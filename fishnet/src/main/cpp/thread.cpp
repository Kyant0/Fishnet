#include "thread.h"

#include <cinttypes>
#include <numeric>

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
}

void print_thread(pid_t pid, pid_t tid, uid_t uid, int word_size, unwindstack::ArchEnum arch,
                  unwindstack::AndroidUnwinder *unwinder, unwindstack::AndroidUnwinderData *data,
                  unwindstack::Regs *regs) {
    print_thread_header(pid, tid, uid);
    print_thread_registers(arch, word_size, regs);
    print_thread_backtrace(arch, data->frames);
    print_thread_memory_dump(unwinder, word_size, regs);
}

void print_main_thread(pid_t pid, pid_t tid, uid_t uid, siginfo_t *si, int word_size,
                       unwindstack::ArchEnum arch, unwindstack::AndroidUnwinder *unwinder,
                       unwindstack::AndroidUnwinderData *data, unwindstack::Regs *regs,
                       bool dump_memory) {
    auto fault_addr = (uintptr_t) si->si_addr;
    print_thread_header(pid, tid, uid);

    std::string sender_desc;
    if (signal_has_sender(si, pid)) {
        sender_desc = StringPrintf(" from pid %d, uid %d", si->si_pid, si->si_uid);
    }

    bool is_async_mte_crash = false;
    bool is_mte_crash = false;
    std::string fault_addr_desc;
    if (fault_addr) {
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

    print_tag_dump(fault_addr, arch, unwinder->GetProcessMemory());

    if (is_mte_crash) {
        LOG_FISHNET("");
        LOG_FISHNET("Learn more about MTE reports: "
                    "https://source.android.com/docs/security/test/memory-safety/mte-reports");
    }

    if (dump_memory) {
        print_thread_memory_dump(unwinder, word_size, regs);

        LOG_FISHNET("");

        // No memory maps to print.
        if (unwinder->GetMaps()->Total() > 0) {
            print_memory_maps(fault_addr, unwinder->GetMaps(), unwinder->GetProcessMemory(), word_size);
        } else {
            LOG_FISHNET("No memory maps found");
        }
    }
}
