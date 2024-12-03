#include "signal_handler.h"

#include <sys/mman.h>

#include "abort_message.h"
#include "human_readable.h"
#include "dump.h"

static struct sigaction old_actions[NSIG];

static void *thread_stack = nullptr;

static void *fishnet_dispatch_thread(void *arg) {
    const auto *info = (const DebuggerThreadInfo *) arg;
    fishnet_dump_native(info);
    return nullptr;
}

static bool is_entered = false;

static void fishnet_signal_handler(int signal_number, siginfo_t *info, void *context) {
    if (is_entered) {
        return;
    }
    is_entered = true;

    LOGE("Received signal %s, code %s", get_signame(info), get_sigcode(info));

    DebuggerThreadInfo thread_info = {
            .crashing_tid = gettid(),
            .siginfo = info,
            .ucontext = context,
    };

    pthread_t thread_id;
    pthread_attr_t thread_attr;
    if (pthread_attr_init(&thread_attr) != 0) {
        goto resend;
    }
    if (pthread_attr_setstack(&thread_attr, thread_stack, 8 * getpagesize()) != 0) {
        LOGE("pthread_attr_setstack failed");
        munmap(thread_stack, 10 * getpagesize());
        goto resend;
    }
    if (pthread_create(&thread_id, &thread_attr, fishnet_dispatch_thread, &thread_info) != 0) {
        goto resend;
    }
    pthread_join(thread_id, nullptr);
    munmap(thread_stack, 10 * getpagesize());

    resend:
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

void init_signal_handler() {
    const size_t thread_stack_pages = 8;
    const void *thread_stack_allocation = mmap(nullptr, getpagesize() * (thread_stack_pages + 2),
                                               PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (thread_stack_allocation == MAP_FAILED) {
        LOGE("Failed to allocate thread stack");
    }

    char *stack = (char *) thread_stack_allocation + getpagesize();
    if (mprotect(stack, getpagesize() * thread_stack_pages, PROT_READ | PROT_WRITE) != 0) {
        LOGE("Failed to mprotect thread stack");
    }
    thread_stack = stack;

    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_sigaction = fishnet_signal_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    register_handlers(&action);

    set_aborter();
}

void deinit_signal_handler() {
    for (int i = 0; i < NSIG; i++) {
        sigaction(i, old_actions + i, nullptr);
    }
}
