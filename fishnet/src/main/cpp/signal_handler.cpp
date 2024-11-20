#include "signal_handler.h"

#include <sys/mman.h>

#include "abi.h"
#include "log.h"
#include "clock.h"
#include "abort_message.h"
#include "logcat.h"
#include "process.h"
#include "thread.h"
#include "property.h"
#include "backtrace.h"
#include "fd.h"
#include "anr.h"

static struct sigaction old_actions[NSIG];

void signal_handler(int s, struct siginfo *si, void *uc) {
    pid_t pid = getpid();
    pid_t tid = gettid();
    uid_t uid = getuid();

    unwindstack::AndroidUnwinder *unwinder = unwindstack::AndroidUnwinder::Create(pid);
    unwindstack::ErrorData error_data{};
    unwinder->Initialize(error_data);
    unwindstack::ArchEnum arch = unwindstack::Regs::CurrentArch();
    int word_size = pointer_width(arch);
    unwindstack::Regs *regs = unwindstack::Regs::CreateFromUcontext(arch, uc);
    unwindstack::AndroidUnwinderData data{};
    unwinder->Unwind(uc, data);

    // Do not attempt to dump logs of the logd process because the gathering
    // of logs can hang until a timeout occurs.
    if (get_thread_name(tid) != "logd") {
        dump_logcat();
        try_read_libc_abort_logs();
    }

    dump_thread_backtrace(data.frames);

    LOG_FISHNET("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
    LOG_FISHNET("Build fingerprint: '%s'", get_property("ro.build.fingerprint", "unknown").c_str());
    LOG_FISHNET("Revision: '%s'", get_property("ro.revision", "unknown").c_str());
    LOG_FISHNET("ABI: '%s'", abi_string(arch));
    LOG_FISHNET("Timestamp: %s", get_timestamp().c_str());
    LOG_FISHNET("Process uptime: %lus", get_process_uptime(pid));

    // only print this info if the page size is not 4k or has been in 16k mode
    size_t page_size = get_page_size();
    bool has_been_16kb_mode = get_bool_property("ro.misctrl.16kb_before", false);
    if (page_size != 4096) {
        LOG_FISHNET("Page size: %zu bytes", page_size);
    } else if (has_been_16kb_mode) {
        LOG_FISHNET("Has been in 16kb mode: yes");
    }

    print_main_thread(pid, tid, uid, si, word_size, arch, unwinder, &data, regs, false);

    std::vector<pid_t> tids;
    get_process_tids(pid, tids);
    for (pid_t thread_id: tids) {
        if (thread_id == tid) continue;
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(pid, thread_id, uid, unwinder, false);
        // print_guest_thread(thread_id, unwinder, false);
    }

    dump_open_fds(pid);

    print_logs();

    write_log();

    if (old_actions[s].sa_flags & SA_SIGINFO) {
        if (old_actions[s].sa_sigaction) {
            old_actions[s].sa_sigaction(s, si, uc);
        }
    } else {
        if (old_actions[s].sa_handler) {
            old_actions[s].sa_handler(s);
        }
    }
}

void register_handlers(struct sigaction *action) {
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

    size_t thread_stack_pages = 8;
    void *thread_stack_allocation = mmap(nullptr, getpagesize() * (thread_stack_pages + 2), PROT_NONE,
                                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (thread_stack_allocation == MAP_FAILED) {
        LOG_FISHNET("failed to allocate fishnet thread stack");
    }

    char *stack = static_cast<char *>(thread_stack_allocation) + getpagesize();
    if (mprotect(stack, getpagesize() * thread_stack_pages, PROT_READ | PROT_WRITE) != 0) {
        LOG_FISHNET("failed to mprotect fishnet thread stack");
    }

    // Stack grows negatively, set it to the last byte in the page...
    stack = (stack + thread_stack_pages * getpagesize() - 1);
    // and align it.
    stack -= 15;


    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_sigaction = signal_handler;
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
