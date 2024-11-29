#include "cause.h"

#include <dlfcn.h>
#include <sys/mman.h>

#include "abi.h"

static std::optional<std::string> get_stack_overflow_cause(uint64_t fault_addr, uint64_t sp, unwindstack::Maps *maps) {
    // Under stack MTE the stack pointer and/or the fault address can be tagged.
    // In order to calculate deltas between them, strip off the tags off both
    // addresses.
    fault_addr = untag_address(fault_addr);
    sp = untag_address(sp);
    static constexpr uint64_t kMaxDifferenceBytes = 256;
    uint64_t difference;
    if (sp >= fault_addr) {
        difference = sp - fault_addr;
    } else {
        difference = fault_addr - sp;
    }
    if (difference <= kMaxDifferenceBytes) {
        // The faulting address is close to the current sp, check if the sp
        // indicates a stack overflow.
        // On arm, the sp does not get updated when the instruction faults.
        // In this case, the sp will still be in a valid map, which is the
        // last case below.
        // On aarch64, the sp does get updated when the instruction faults.
        // In this case, the sp will be in either an invalid map if triggered
        // on the main thread, or in a guard map if in another thread, which
        // will be the first case or second case from below.
        std::shared_ptr<unwindstack::MapInfo> map_info = maps->Find(sp);
        if (map_info == nullptr) {
            return "stack pointer is in a non-existent map; likely due to stack overflow.";
        } else if ((map_info->flags() & (PROT_READ | PROT_WRITE)) != (PROT_READ | PROT_WRITE)) {
            return "stack pointer is not in a rw map; likely due to stack overflow.";
        } else if ((sp - map_info->start()) <= kMaxDifferenceBytes) {
            return "stack pointer is close to top of stack; likely stack overflow.";
        }
    }
    return {};
}

void dump_probable_cause(LogRecord &record, const siginfo_t *info, unwindstack::Maps *maps,
                         const std::unique_ptr<unwindstack::Regs> &regs) {
    const auto fault_addr = (const uint64_t) (info->si_addr);

    std::optional<std::string> cause;
    if (info->si_signo == SIGSEGV && info->si_code == SEGV_MAPERR) {
        if (fault_addr < 4096) {
            cause = "null pointer dereference";
        } else if (fault_addr == 0xffff0ffc) {
            cause = "call to kuser_helper_version";
        } else if (fault_addr == 0xffff0fe0) {
            cause = "call to kuser_get_tls";
        } else if (fault_addr == 0xffff0fc0) {
            cause = "call to kuser_cmpxchg";
        } else if (fault_addr == 0xffff0fa0) {
            cause = "call to kuser_memory_barrier";
        } else if (fault_addr == 0xffff0f60) {
            cause = "call to kuser_cmpxchg64";
        } else {
            cause = get_stack_overflow_cause(fault_addr, regs->sp(), maps);
        }
    } else if (info->si_signo == SIGSEGV && info->si_code == SEGV_ACCERR) {
        auto map_info = maps->Find(fault_addr);
        if (map_info != nullptr && map_info->flags() == PROT_EXEC) {
            cause = "execute-only (no-read) memory access error; likely due to data in .text.";
        } else if (fault_addr == regs->pc() && map_info != nullptr && (map_info->flags() & PROT_EXEC) == 0) {
            cause = "trying to execute non-executable memory.";
        } else {
            cause = get_stack_overflow_cause(fault_addr, regs->sp(), maps);
        }
    }
#if defined(__aarch64__) && defined(SEGV_MTESERR)
    else if (info->si_signo == SIGSEGV && info->si_code == SEGV_MTESERR) {
        // If this was a heap MTE crash, it would have been handled by scudo. Checking whether it
        // is a stack one.
        // cause = maybe_stack_mte_cause(tombstone, unwinder, target_thread, threads, fault_addr);
    }
#endif
    else if (info->si_signo == SIGSYS && info->si_code == SYS_SECCOMP) {
        cause = StringPrintf("seccomp prevented call to disallowed %s system call %d", ABI_STRING, info->si_syscall);
    }

    if (cause) {
        LOG_FISHNET("Cause: %s", cause->c_str());
    }
}
