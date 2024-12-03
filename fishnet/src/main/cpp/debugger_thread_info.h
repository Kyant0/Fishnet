#ifndef FISHNET_DEBUGGER_THREAD_INFO_H
#define FISHNET_DEBUGGER_THREAD_INFO_H

#include <csignal>
#include <sys/types.h>

typedef struct DebuggerThreadInfo {
    pid_t crashing_tid;
    siginfo_t *siginfo;
    void *ucontext;
} DebuggerThreadInfo;

#endif //FISHNET_DEBUGGER_THREAD_INFO_H
