#include "dump.h"

#include <cinttypes>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include "abi.h"
#include "human_readable.h"
#include "logcat.h"
#include "process.h"
#include "tasks.h"
#include "thread.h"
#include "backtrace.h"
#include "fds.h"
#include "anr_signal_handler.h"
#include "log_header.h"
#include "abort_message.h"

void fishnet_dump_native(const DebuggerThreadInfo *info) {
    const pid_t pid = getpid();
    const pid_t tid = info->crashing_tid;
    const pid_t the_tid = gettid();
    const uid_t uid = getuid();

    FISHNET_RECORD(Native);

    std::shared_ptr<unwindstack::Memory> process_memory = unwindstack::Memory::CreateProcessMemoryCached(pid);
    unwindstack::AndroidLocalUnwinder unwinder(process_memory);
    unwindstack::ErrorData error{};
    if (!unwinder.Initialize(error)) {
        LOGE("Failed to init unwinder: %s", unwindstack::GetErrorCodeString(error.code));
        return;
    }

    constexpr unwindstack::ArchEnum arch = current_arch();
    constexpr int word_size = pointer_width();
    std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::CreateFromUcontext(arch, info->ucontext));
    unwindstack::AndroidUnwinderData data{};
    unwinder.Unwind(regs.get(), data);

    unwindstack::ThreadUnwinder thread_unwinder(128);
    if (!thread_unwinder.Init()) {
        LOGE("Failed to init thread unwinder");
        return;
    }

    std::vector<pid_t> tids;
    get_process_tids(pid, tids);

    get_scudo_message_if_needed(arch, regs, data.frames);

    dump_logcat(pid);
    try_read_abort_message_from_logcat();

    print_log_header(record, LogType::Native, pid);

    print_main_thread(record, pid, tid, uid, info->siginfo, word_size, arch, &unwinder, regs, data.frames,
                      true, false);

    print_memory_info(record);

    print_process_status(record, pid);

    print_tasks(record, pid);

    for (const pid_t &thread_id: tids) {
        if (thread_id == tid || thread_id == the_tid) {
            continue;
        }
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(record, thread_id, word_size, arch, &thread_unwinder, false);
    }

    dump_open_fds(record, pid);

    print_logs(record);

    FISHNET_WRITE();
}

void fishnet_dump_java(const char *java_stack_traces) {
    const pid_t pid = getpid();
    const pid_t tid = gettid();
    const uid_t uid = getuid();

    FISHNET_RECORD(Java);

    constexpr unwindstack::ArchEnum arch = current_arch();
    constexpr int word_size = pointer_width();

    unwindstack::ThreadUnwinder thread_unwinder(128);
    if (!thread_unwinder.Init()) {
        LOGE("Failed to init thread unwinder");
        return;
    }

    std::vector<pid_t> tids;
    get_process_tids(pid, tids);

    dump_logcat(pid);

    print_log_header(record, LogType::Java, pid);

    print_main_thread_header(record, pid, tid, uid);
    LOG_FISHNET("");

    if (java_stack_traces != nullptr) {
        LOG_FISHNET("Java stack traces:");
        LOG_FISHNET("%s", java_stack_traces);
    }

    print_memory_info(record);

    print_process_status(record, pid);

    print_tasks(record, pid);

    for (const pid_t &thread_id: tids) {
        if (thread_id == tid) {
            continue;
        }
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(record, thread_id, word_size, arch, &thread_unwinder, false);
    }

    dump_open_fds(record, pid);

    print_logs(record);

    FISHNET_WRITE();
}

void fishnet_dump_anr(const char *java_stack_traces, const DebuggerThreadInfo *info) {
    const pid_t pid = getpid();
    const pid_t tid = info->crashing_tid;
    const pid_t the_tid = gettid();
    const uid_t uid = getuid();

    FISHNET_RECORD(ANR);

    constexpr unwindstack::ArchEnum arch = current_arch();
    constexpr int word_size = pointer_width();

    unwindstack::ThreadUnwinder thread_unwinder(128);
    if (!thread_unwinder.Init()) {
        LOGE("Failed to init thread unwinder");
        return;
    }

    std::vector<pid_t> tids;
    get_process_tids(pid, tids);

    dump_logcat(pid);

    print_log_header(record, LogType::ANR, pid);

    print_main_thread_header(record, pid, tid, uid);
    LOG_FISHNET("");

    if (java_stack_traces != nullptr) {
        LOG_FISHNET("Java stack traces:");
        LOG_FISHNET("%s", java_stack_traces);
    }

    print_memory_info(record);

    print_process_status(record, pid);

    print_tasks(record, pid);

    for (const pid_t &thread_id: tids) {
        if (thread_id == pid) {
            continue;
        }
        if (thread_id == tid || thread_id == the_tid) {
            continue;
        }
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(record, thread_id, word_size, arch, &thread_unwinder, false);
    }

    dump_open_fds(record, pid);

    print_logs(record);

    FISHNET_WRITE();
}
