#include "human_readable.h"

#include <linux/ptrace.h>

const char *get_signame(const siginfo_t *info) {
    switch (info->si_signo) {
        case SIGABRT:
            return "SIGABRT";
        case SIGBUS:
            return "SIGBUS";
        case SIGFPE:
            return "SIGFPE";
        case SIGILL:
            return "SIGILL";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGSTKFLT:
            return "SIGSTKFLT";
        case SIGSTOP:
            return "SIGSTOP";
        case SIGSYS:
            return "SIGSYS";
        case SIGTRAP:
            return "SIGTRAP";
        default:
            return "?";
    }
}

const char *get_sigcode(const siginfo_t *info) {
    // Try the signal-specific codes...
    switch (info->si_signo) {
        case SIGILL:
            switch (info->si_code) {
                case ILL_ILLOPC:
                    return "ILL_ILLOPC";
                case ILL_ILLOPN:
                    return "ILL_ILLOPN";
                case ILL_ILLADR:
                    return "ILL_ILLADR";
                case ILL_ILLTRP:
                    return "ILL_ILLTRP";
                case ILL_PRVOPC:
                    return "ILL_PRVOPC";
                case ILL_PRVREG:
                    return "ILL_PRVREG";
                case ILL_COPROC:
                    return "ILL_COPROC";
                case ILL_BADSTK:
                    return "ILL_BADSTK";
                case ILL_BADIADDR:
                    return "ILL_BADIADDR";
                case __ILL_BREAK:
                    return "ILL_BREAK";
                case __ILL_BNDMOD:
                    return "ILL_BNDMOD";
            }
            static_assert(NSIGILL == __ILL_BNDMOD, "missing ILL_* si_code");
            break;
        case SIGBUS:
            switch (info->si_code) {
                case BUS_ADRALN:
                    return "BUS_ADRALN";
                case BUS_ADRERR:
                    return "BUS_ADRERR";
                case BUS_OBJERR:
                    return "BUS_OBJERR";
                case BUS_MCEERR_AR:
                    return "BUS_MCEERR_AR";
                case BUS_MCEERR_AO:
                    return "BUS_MCEERR_AO";
            }
            static_assert(NSIGBUS == BUS_MCEERR_AO, "missing BUS_* si_code");
            break;
        case SIGFPE:
            switch (info->si_code) {
                case FPE_INTDIV:
                    return "FPE_INTDIV";
                case FPE_INTOVF:
                    return "FPE_INTOVF";
                case FPE_FLTDIV:
                    return "FPE_FLTDIV";
                case FPE_FLTOVF:
                    return "FPE_FLTOVF";
                case FPE_FLTUND:
                    return "FPE_FLTUND";
                case FPE_FLTRES:
                    return "FPE_FLTRES";
                case FPE_FLTINV:
                    return "FPE_FLTINV";
                case FPE_FLTSUB:
                    return "FPE_FLTSUB";
                case __FPE_DECOVF:
                    return "FPE_DECOVF";
                case __FPE_DECDIV:
                    return "FPE_DECDIV";
                case __FPE_DECERR:
                    return "FPE_DECERR";
                case __FPE_INVASC:
                    return "FPE_INVASC";
                case __FPE_INVDEC:
                    return "FPE_INVDEC";
                case FPE_FLTUNK:
                    return "FPE_FLTUNK";
                case FPE_CONDTRAP:
                    return "FPE_CONDTRAP";
            }
            static_assert(NSIGFPE == FPE_CONDTRAP, "missing FPE_* si_code");
            break;
        case SIGSEGV:
            switch (info->si_code) {
                case SEGV_MAPERR:
                    return "SEGV_MAPERR";
                case SEGV_ACCERR:
                    return "SEGV_ACCERR";
                case SEGV_BNDERR:
                    return "SEGV_BNDERR";
                case SEGV_PKUERR:
                    return "SEGV_PKUERR";
                case SEGV_ACCADI:
                    return "SEGV_ACCADI";
                case SEGV_ADIDERR:
                    return "SEGV_ADIDERR";
                case SEGV_ADIPERR:
                    return "SEGV_ADIPERR";
                case SEGV_MTEAERR:
                    return "SEGV_MTEAERR";
                case SEGV_MTESERR:
                    return "SEGV_MTESERR";
                case SEGV_CPERR:
                    return "SEGV_CPERR";
            }
            static_assert(NSIGSEGV == SEGV_CPERR, "missing SEGV_* si_code");
            break;
        case SIGSYS:
            switch (info->si_code) {
                case SYS_SECCOMP:
                    return "SYS_SECCOMP";
                case SYS_USER_DISPATCH:
                    return "SYS_USER_DISPATCH";
            }
            static_assert(NSIGSYS == SYS_USER_DISPATCH, "missing SYS_* si_code");
            break;
        case SIGTRAP:
            switch (info->si_code) {
                case TRAP_BRKPT:
                    return "TRAP_BRKPT";
                case TRAP_TRACE:
                    return "TRAP_TRACE";
                case TRAP_BRANCH:
                    return "TRAP_BRANCH";
                case TRAP_HWBKPT:
                    return "TRAP_HWBKPT";
                case TRAP_UNK:
                    return "TRAP_UNDIAGNOSED";
                case TRAP_PERF:
                    return "TRAP_PERF";
            }
            if ((info->si_code & 0xff) == SIGTRAP) {
                switch ((info->si_code >> 8) & 0xff) {
                    case PTRACE_EVENT_FORK:
                        return "PTRACE_EVENT_FORK";
                    case PTRACE_EVENT_VFORK:
                        return "PTRACE_EVENT_VFORK";
                    case PTRACE_EVENT_CLONE:
                        return "PTRACE_EVENT_CLONE";
                    case PTRACE_EVENT_EXEC:
                        return "PTRACE_EVENT_EXEC";
                    case PTRACE_EVENT_VFORK_DONE:
                        return "PTRACE_EVENT_VFORK_DONE";
                    case PTRACE_EVENT_EXIT:
                        return "PTRACE_EVENT_EXIT";
                    case PTRACE_EVENT_SECCOMP:
                        return "PTRACE_EVENT_SECCOMP";
                    case PTRACE_EVENT_STOP:
                        return "PTRACE_EVENT_STOP";
                }
            }
            static_assert(NSIGTRAP == TRAP_PERF, "missing TRAP_* si_code");
            break;
    }
    // Then the other codes...
    switch (info->si_code) {
        case SI_USER:
            return "SI_USER";
        case SI_KERNEL:
            return "SI_KERNEL";
        case SI_QUEUE:
            return "SI_QUEUE";
        case SI_TIMER:
            return "SI_TIMER";
        case SI_MESGQ:
            return "SI_MESGQ";
        case SI_ASYNCIO:
            return "SI_ASYNCIO";
        case SI_SIGIO:
            return "SI_SIGIO";
        case SI_TKILL:
            return "SI_TKILL";
        case SI_DETHREAD:
            return "SI_DETHREAD";
    }
    // Then give up...
    return "?";
}
