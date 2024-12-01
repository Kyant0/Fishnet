#include "java_dump.h"

#include <cinttypes>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include "abi.h"
#include "human_readable.h"
#include "duration.h"
#include "logcat.h"
#include "process.h"
#include "tasks.h"
#include "thread.h"
#include "property.h"
#include "backtrace.h"
#include "fds.h"
#include "root.h"
#include "anr.h"
#include "java_dump.h"
#include "version.h"

void fishnet_dump_with_java(const char *java_stack_traces, bool dump_main_thread) {
    const pid_t pid = getpid();
    const pid_t tid = gettid();
    const uid_t uid = getuid();

    FISHNET_RECORD(dump_main_thread ? Java : ANR);

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

    const ApkInfo log_info = get_apk_info();

    struct utsname name_buffer{};
    uname(&name_buffer);

    std::string kernel_version = name_buffer.release;
    kernel_version += ' ';
    kernel_version += name_buffer.version;

    struct sysinfo s_info{};
    sysinfo(&s_info);

    LOG_FISHNET("****** Fishnet crash report %s ******", FISHNET_VERSION);
    LOG_FISHNET("");
    LOG_FISHNET("Log type: %s", dump_main_thread ? "Java" : "ANR");
    LOG_FISHNET("");
    LOG_FISHNET("APK info:");
    LOG_FISHNET("    Package: '%s'", log_info.package_name);
    LOG_FISHNET("    Version: '%s' (%" PRId64 ")", log_info.version_name, log_info.version_code);
    LOG_FISHNET("    Cert: '%s'", log_info.cert);
    LOG_FISHNET("");
    LOG_FISHNET("Device info:");
    LOG_FISHNET("    Build fingerprint: '%s'", get_property("ro.build.fingerprint", "unknown").c_str());
    LOG_FISHNET("    Revision: '%s'", get_property("ro.revision", "unknown").c_str());
    LOG_FISHNET("    Security patch: '%s'", get_property("ro.build.version.security_patch", "unknown").c_str());
    LOG_FISHNET("    Build date: '%s'", get_property("ro.system.build.date", "unknown").c_str());
    LOG_FISHNET("    Kernel version: '%s'", kernel_version.c_str());
    LOG_FISHNET("    ABI: '%s'", abi_string());
    LOG_FISHNET("    Locale: '%s'", get_property("ro.product.locale", "unknown").c_str());
    LOG_FISHNET("    Debuggable: %s", get_bool_property("ro.debuggable", false) ? "yes" : "no");
    LOG_FISHNET("    Rooted (guessed): %s", is_rooted() ? "yes" : "no");
    LOG_FISHNET("    System uptime: %s", seconds_to_human_readable_time(s_info.uptime).c_str());
    LOG_FISHNET("");
    LOG_FISHNET("Timestamp: %s", record.timestamp.c_str());
    LOG_FISHNET("Process uptime: %s", seconds_to_human_readable_time(get_process_uptime(pid)).c_str());

    // only print this info if the page size is not 4k or has been in 16k mode
    const size_t page_size = getpagesize();
    const bool has_been_16kb_mode = get_bool_property("ro.misctrl.16kb_before", false);
    if (page_size != 4096) {
        LOG_FISHNET("Page size: %zu bytes", page_size);
    } else if (has_been_16kb_mode) {
        LOG_FISHNET("Has been in 16kb mode: yes");
    }

    if (dump_main_thread) {
        print_main_thread_header(record, pid, tid, uid);
    }
    LOG_FISHNET("");

    if (java_stack_traces != nullptr) {
        LOG_FISHNET("Java stack traces:");
        LOG_FISHNET("%s", java_stack_traces);
    }

    print_memory_info(record);

    print_process_status(record, pid);

    print_tasks(record, pid);

    for (const pid_t &thread_id: tids) {
        if (!dump_main_thread && thread_id == pid) {
            continue;
        }
        LOG_FISHNET("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
        print_thread(record, thread_id, word_size, arch, &thread_unwinder, false);
    }

    dump_open_fds(record, pid);

    print_logs(record);

    FISHNET_WRITE();
}
