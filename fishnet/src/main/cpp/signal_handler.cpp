#include "signal_handler.h"

#include <sys/mman.h>

#include "abi.h"
#include "log.h"
#include "clock.h"
#include "abort_message.h"
#include "logcat.h"
#include "process.h"
#include "tasks.h"
#include "thread.h"
#include "property.h"
#include "backtrace.h"
#include "fd.h"
#include "anr.h"

static struct sigaction old_actions[NSIG];

struct debugger_thread_info {
    pid_t crashing_tid;
    siginfo_t *siginfo;
    void *ucontext;
};

static void *thread_stack = nullptr;

static void *fishnet_dispatch_thread(void *arg) {
    const debugger_thread_info *thread_info = (const debugger_thread_info *) arg;

    LOGE("fishnet_dispatch_thread started");

    const pid_t pid = getpid();
    const pid_t tid = thread_info->crashing_tid;
    const pid_t the_tid = gettid();
    const uid_t uid = getuid();
    const siginfo_t *info = thread_info->siginfo;
    void *context = thread_info->ucontext;

    std::shared_ptr<unwindstack::Memory> process_memory = unwindstack::Memory::CreateProcessMemoryCached(pid);
    unwindstack::AndroidLocalUnwinder unwinder(process_memory);
    unwindstack::ErrorData error{};
    if (!unwinder.Initialize(error)) {
        LOGE("failed to init unwinder object: %s", unwindstack::GetErrorCodeString(error.code));
        close_log_fd();
        return nullptr;
    }

    const unwindstack::ArchEnum arch = unwindstack::Regs::CurrentArch();
    const int word_size = pointer_width(arch);
    std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::CreateFromUcontext(arch, context));
    unwindstack::AndroidUnwinderData data{};
    unwinder.Unwind(regs.get(), data);

    unwindstack::ThreadUnwinder thread_unwinder(128);
    thread_unwinder.Init();

    std::vector<pid_t> tids;
    get_process_tids(pid, tids);

    get_scudo_message_if_needed(arch, regs, data.frames);

    dump_logcat(pid);
    try_read_abort_message_from_logcat();

    dump_thread_backtrace(data.frames);

    LOG_FISHNET("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
    LOG_FISHNET("Build fingerprint: '%s'", get_property("ro.build.fingerprint", "unknown").c_str());
    LOG_FISHNET("Revision: '%s'", get_property("ro.revision", "unknown").c_str());
    LOG_FISHNET("ABI: '%s'", abi_string(arch));
    LOG_FISHNET("Timestamp: %s", get_timestamp().c_str());
    LOG_FISHNET("Process uptime: %lus", get_process_uptime(pid));

    // only print this info if the page size is not 4k or has been in 16k mode
    const size_t page_size = getpagesize();
    const bool has_been_16kb_mode = get_bool_property("ro.misctrl.16kb_before", false);
    if (page_size != 4096) {
        LOG_FISHNET("Page size: %zu bytes", page_size);
    } else if (has_been_16kb_mode) {
        LOG_FISHNET("Has been in 16kb mode: yes");
    }

    print_main_thread(pid, tid, uid, info, word_size, arch, &unwinder, regs, data.frames,
                      true, false);

    print_tasks(pid);

    for (const pid_t &thread_id: tids) {
        if (thread_id == tid || thread_id == the_tid) {
            continue;
        }
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(thread_id, word_size, arch, &thread_unwinder, false);
    }

    dump_open_fds(pid);

    print_logs();

    write_log_to_fd();

    return nullptr;
}

static volatile bool is_entered = false;

__attribute__((optnone))
static void fishnet_signal_handler(int signal_number, siginfo_t *info, void *context) {
    if (is_entered) {
        return;
    }
    is_entered = true;

    debugger_thread_info thread_info = {
            .crashing_tid = gettid(),
            .siginfo = info,
            .ucontext = context,
    };

    pthread_t thread_id;
    pthread_attr_t thread_attr;
    if (pthread_attr_init(&thread_attr) != 0) {
        LOGE("pthread_attr_init failed");
        goto resend;
    }
    if (pthread_attr_setstack(&thread_attr, thread_stack, 8 * getpagesize()) != 0) {
        LOGE("pthread_attr_setstack failed");
        if (munmap(thread_stack, 10 * getpagesize()) != 0) {
            LOGE("munmap failed");
        }
        goto resend;
    }
    if (pthread_create(&thread_id, &thread_attr, fishnet_dispatch_thread, &thread_info) != 0) {
        LOGE("pthread_create failed");
        goto resend;
    }
    pthread_join(thread_id, nullptr);
    if (munmap(thread_stack, 10 * getpagesize()) != 0) {
        LOGE("munmap failed");
    }

    resend:
    close_log_fd();

    if (old_actions[signal_number].sa_flags & SA_SIGINFO) {
        if (old_actions[signal_number].sa_sigaction) {
            old_actions[signal_number].sa_sigaction(signal_number, info, context);
        }
    } else {
        if (old_actions[signal_number].sa_handler) {
            old_actions[signal_number].sa_handler(signal_number);
        }
    }
}

static void register_handlers(struct sigaction *action) {
    sigaction(SIGABRT, action, old_actions + SIGABRT);
    sigaction(SIGBUS, action, old_actions + SIGBUS);
    sigaction(SIGFPE, action, old_actions + SIGFPE);
    sigaction(SIGILL, action, old_actions + SIGILL);
    sigaction(SIGSEGV, action, old_actions + SIGSEGV);
    sigaction(SIGSTKFLT, action, old_actions + SIGSTKFLT);
    sigaction(SIGSYS, action, old_actions + SIGSYS);
    sigaction(SIGTRAP, action, old_actions + SIGTRAP);
}

void init_signal_handler(bool enabled) {
    if (!enabled) return;

    const size_t thread_stack_pages = 8;
    const void *thread_stack_allocation = mmap(nullptr, getpagesize() * (thread_stack_pages + 2),
                                               PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (thread_stack_allocation == MAP_FAILED) {
        LOGE("failed to allocate fishnet thread stack");
    }

    char *stack = (char *) thread_stack_allocation + getpagesize();
    if (mprotect(stack, getpagesize() * thread_stack_pages, PROT_READ | PROT_WRITE) != 0) {
        LOGE("failed to mprotect fishnet thread stack");
    }
    thread_stack = stack;

    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_sigaction = fishnet_signal_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    register_handlers(&action);

    set_aborter();

    sigset_t sig_sets;
    sigemptyset(&sig_sets);
    sigaddset(&sig_sets, SIGQUIT);
    pthread_sigmask(SIG_UNBLOCK, &sig_sets, nullptr);

    struct sigaction sigAction{};
    sigfillset(&sigAction.sa_mask);
    sigAction.sa_flags = SA_RESTART | SA_ONSTACK | SA_SIGINFO;
    sigAction.sa_sigaction = anr_signal_handler;
    sigaction(SIGQUIT, &sigAction, nullptr);
}