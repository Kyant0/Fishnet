#include "thread.h"

#include <cinttypes>
#include <numeric>
#include <sys/prctl.h>
#include <sys/ptrace.h>

#include "abi.h"
#include "human_readable.h"
#include "log.h"
#include "cause.h"
#include "abort_message.h"
#include "command_line.h"
#include "process.h"
#include "thread.h"
#include "property.h"
#include "registers.h"
#include "backtrace.h"
#include "memory.h"

#define BIONIC_SIGNAL_BACKTRACE (__SIGRTMIN + 1)

static bool signal_has_si_addr(const siginfo_t *si) {
    // Manually sent signals won't have si_addr.
    if (si->si_code == SI_USER || si->si_code == SI_QUEUE || si->si_code == SI_TKILL) {
        return false;
    }

    switch (si->si_signo) {
        case SIGBUS:
        case SIGFPE:
        case SIGILL:
        case SIGTRAP:
            return true;
        case SIGSEGV:
            return si->si_code != SEGV_MTEAERR;
        default:
            return false;
    }
}

static bool signal_has_sender(const siginfo_t *si, pid_t caller_pid) {
    return SI_FROMUSER(si) && (si->si_pid != 0) && (si->si_pid != caller_pid);
}

std::string get_thread_name(pid_t tid) {
    char thread_name[16];
    char path[19];

    snprintf(path, sizeof(path), "/proc/%d/comm", tid);
    FILE *file = fopen(path, "r");
    if (!file) {
        return "<unknown>";
    }
    if (fgets(thread_name, sizeof(thread_name), file) == nullptr) {
        fclose(file);
        return "<unknown>";
    }
    fclose(file);

    size_t len = strlen(thread_name);
    if (len > 0 && thread_name[len - 1] == '\n') {
        thread_name[len - 1] = '\0';
    }

    return thread_name;
}

#define DESCRIBE_FLAG(flag) \
  if (value & flag) {       \
    desc += ", ";           \
    desc += #flag;          \
    value &= ~flag;         \
  }

static std::string describe_end(long value, std::string& desc) {
    if (value) {
        desc += StringPrintf(", unknown 0x%lx", value);
    }
    return desc.empty() ? "" : " (" + desc.substr(2) + ")";
}

static std::string describe_tagged_addr_ctrl(long value) {
    std::string desc;
    DESCRIBE_FLAG(PR_TAGGED_ADDR_ENABLE)
    DESCRIBE_FLAG(PR_MTE_TCF_SYNC)
    DESCRIBE_FLAG(PR_MTE_TCF_ASYNC)
    if (value & PR_MTE_TAG_MASK) {
        desc += StringPrintf(", mask 0x%04lx", (value & PR_MTE_TAG_MASK) >> PR_MTE_TAG_SHIFT);
        value &= ~PR_MTE_TAG_MASK;
    }
    return describe_end(value, desc);
}

static std::string describe_pac_enabled_keys(long value) {
    std::string desc;
    DESCRIBE_FLAG(PR_PAC_APIAKEY)
    DESCRIBE_FLAG(PR_PAC_APIBKEY)
    DESCRIBE_FLAG(PR_PAC_APDAKEY)
    DESCRIBE_FLAG(PR_PAC_APDBKEY)
    DESCRIBE_FLAG(PR_PAC_APGAKEY)
    return describe_end(value, desc);
}

void print_thread_header(pid_t pid, pid_t tid, uid_t uid) {
    std::vector<std::string> command_line = get_command_line(pid);
    if (!command_line.empty()) {
        if (command_line.size() == 1) {
            LOG_FISHNET("Cmdline: %s", command_line[0].c_str());
        } else {
            std::string command_line_string = std::accumulate(
                    std::next(command_line.begin()), command_line.end(), command_line[0],
                    [](const std::string &a, const std::string &b) {
                        return a + ' ' + b;
                    });
            LOG_FISHNET("Cmdline: %s", command_line_string.c_str());
        }
    } else {
        LOG_FISHNET("Cmdline: <unknown>");
    }
    std::string thread_name = get_thread_name(tid);
    std::string process_name = get_process_name(pid);
    LOG_FISHNET("pid: %d, tid: %d, name: %s  >>> %s <<<", pid, tid, thread_name.c_str(), process_name.c_str());
    LOG_FISHNET("uid: %d", uid);
    // Only supported on aarch64 for now.
#if defined(__aarch64__)
    long tagged_addr_ctrl = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
    long pac_enabled_keys = prctl(PR_PAC_GET_ENABLED_KEYS, 0, 0, 0, 0);
    if (tagged_addr_ctrl != -1) {
        LOG_FISHNET("tagged_addr_ctrl: %016" PRIx64 "%s", tagged_addr_ctrl,
                    describe_tagged_addr_ctrl(tagged_addr_ctrl).c_str());
    }
    if (pac_enabled_keys != -1) {
        LOG_FISHNET("pac_enabled_keys: %016" PRIx64 "%s", pac_enabled_keys,
                    describe_pac_enabled_keys(pac_enabled_keys).c_str());
    }
#endif
}

void print_main_thread(pid_t pid, pid_t tid, uid_t uid, siginfo_t *si, int word_size,
                       unwindstack::ArchEnum arch, unwindstack::AndroidUnwinder *unwinder,
                       unwindstack::AndroidUnwinderData *data, unwindstack::Regs *regs,
                       bool dump_memory, bool dump_memory_maps) {
    const bool has_fault_addr = signal_has_si_addr(si);
    auto fault_addr = (uintptr_t) si->si_addr;
    print_thread_header(pid, tid, uid);

    std::string sender_desc;
    if (signal_has_sender(si, pid)) {
        sender_desc = StringPrintf(" from pid %d, uid %d", si->si_pid, si->si_uid);
    }

    bool is_async_mte_crash = false;
    bool is_mte_crash = false;
    std::string fault_addr_desc;
    if (has_fault_addr) {
        fault_addr_desc = StringPrintf("0x%0*" PRIx64, 2 * word_size, fault_addr);
    } else {
        fault_addr_desc = "--------";
    }

    LOG_FISHNET("signal %d (%s), code %d (%s%s), fault addr %s",
                si->si_signo, get_signame(si),
                si->si_code, get_sigcode(si), sender_desc.c_str(),
                fault_addr_desc.c_str());
#ifdef SEGV_MTEAERR
    is_async_mte_crash = si->si_signo == SIGSEGV && si->si_code == SEGV_MTEAERR;
    is_mte_crash = is_async_mte_crash || (si->si_signo == SIGSEGV && si->si_code == SEGV_MTESERR);
#endif

    dump_probable_cause(si, unwinder->GetMaps(), regs);

    dump_abort_message();

    print_thread_registers(arch, word_size, regs);
    if (is_async_mte_crash) {
        LOG_FISHNET("Note: This crash is a delayed async MTE crash. Memory corruption has occurred");
        LOG_FISHNET("      in this process. The stack trace below is the first system call or context");
        LOG_FISHNET("      switch that was executed after the memory corruption happened.");
    }
    print_thread_backtrace(arch, data->frames);

    if (has_fault_addr) {
        print_tag_dump(fault_addr, arch, unwinder->GetProcessMemory());
    }

    if (is_mte_crash) {
        LOG_FISHNET("");
        LOG_FISHNET("Learn more about MTE reports: "
                    "https://source.android.com/docs/security/test/memory-safety/mte-reports");
    }

    if (dump_memory) {
        unwindstack::Maps *maps = unwinder->GetMaps();
        unwindstack::Memory *memory = unwinder->GetProcessMemory().get();
        print_thread_memory_dump(word_size, regs, maps, memory);

        LOG_FISHNET("");
    }

    if (dump_memory_maps) {
        // No memory maps to print.
        if (unwinder->GetMaps()->Total() > 0) {
            print_memory_maps(fault_addr, word_size, unwinder->GetMaps(), unwinder->GetProcessMemory());
        } else {
            LOG_FISHNET("No memory maps found");
        }
    }
}

void print_thread(pid_t pid, pid_t tid, uid_t uid, unwindstack::ThreadUnwinder *unwinder, bool dump_memory) {
    std::unique_ptr<unwindstack::Regs> regs;
    unwinder->UnwindWithSignal(BIONIC_SIGNAL_BACKTRACE, tid, &regs);
    unwindstack::ArchEnum arch = unwindstack::Regs::CurrentArch();
    int word_size = pointer_width(arch);

    print_thread_header(pid, tid, uid);
    if (regs) {
        print_thread_registers(arch, word_size, regs.get());
    }
    print_thread_backtrace(arch, unwinder->ConsumeFrames());
    if (dump_memory) {
        unwindstack::Maps *maps = unwinder->GetMaps();
        unwindstack::Memory *memory = unwinder->GetProcessMemory().get();
        if (regs) {
            print_thread_memory_dump(word_size, regs.get(), maps, memory);
        }
    }
}

void print_guest_thread(pid_t tid, unwindstack::ThreadUnwinder *unwinder, bool dump_memory) {
    std::unique_ptr<unwindstack::Regs> regs;
    unwinder->UnwindWithSignal(BIONIC_SIGNAL_BACKTRACE, tid, &regs);
    unwindstack::ArchEnum arch = unwindstack::Regs::CurrentArch();
    int word_size = pointer_width(arch);

    LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
    LOG_FISHNET("Guest thread information for tid: %d", tid);
    print_thread_registers(arch, word_size, regs.get());

    LOG_FISHNET("");
    LOG_FISHNET("%zu total frames", unwinder->frames().size());
    LOG_FISHNET("backtrace:");
    print_backtrace(arch, unwinder->ConsumeFrames());

    if (dump_memory) {
        unwindstack::Maps *maps = unwinder->GetMaps();
        unwindstack::Memory *memory = unwinder->GetProcessMemory().get();
        print_thread_memory_dump(word_size, regs.get(), maps, memory);
    }
}
