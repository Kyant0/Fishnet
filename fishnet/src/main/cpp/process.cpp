#include "process.h"

#include <bits/sysconf.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/sysinfo.h>

std::string get_process_name(pid_t pid) {
    char process_name[256];
    char path[22];

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE *file = fopen(path, "r");
    if (!file) {
        return "<unknown>";
    }
    if (fgets(process_name, sizeof(process_name), file) == nullptr) {
        fclose(file);
        return "<unknown>";
    }
    fclose(file);

    process_name[sizeof(process_name) - 1] = '\0';

    return process_name;
}

uint64_t get_process_start_time(pid_t pid) {
    char path[19];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;

    static constexpr const char *pattern =
            "%c "    // state
            "%d "    // ppid
            "%*d "   // pgrp
            "%*d "   // session
            "%*d "   // tty_nr
            "%*d "   // tpgid
            "%*u "   // flags
            "%*lu "  // minflt
            "%*lu "  // cminflt
            "%*lu "  // majflt
            "%*lu "  // cmajflt
            "%*lu "  // utime
            "%*lu "  // stime
            "%*ld "  // cutime
            "%*ld "  // cstime
            "%*ld "  // priority
            "%*ld "  // nice
            "%*ld "  // num_threads
            "%*ld "  // itrealvalue
            "%llu "  // starttime
    ;

    char stat[512];
    if (fgets(stat, sizeof(stat), file) == nullptr) {
        fclose(file);
        return -1;
    }
    fclose(file);

    char state = '\0';
    int ppid = 0;
    unsigned long long start_time = 0;
    const char *end_of_comm = strrchr(stat, ')');
    const int rc = sscanf(end_of_comm + 2, pattern, &state, &ppid, &start_time);
    return rc == 3 ? start_time : -1;
}

uint64_t get_process_uptime(pid_t pid) {
    struct sysinfo si{};
    sysinfo(&si);
    const long uptime = si.uptime;
    if (uptime == -1) return -1;

    const long clock_ticks = sysconf(_SC_CLK_TCK);
    const uint64_t start_time = get_process_start_time(pid);
    if (start_time == -1) return -1;

    return uptime - (start_time / clock_ticks);
}

void get_process_tids(pid_t pid, std::vector<pid_t> &tids) {
    char path[22];
    snprintf(path, sizeof(path), "/proc/%d/task", pid);
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        tids.emplace_back(atoi(entry->d_name));
    }

    closedir(dir);
}

void print_process_status(LogRecord &record, pid_t pid) {
    char path[21];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *file = fopen(path, "r");
    if (!file) return;

    LOG_FISHNET("Process status for pid %d:", pid);
    std::string status;
    char line[256];
    while (fgets(line, sizeof(line), file) != nullptr) {
        status += "    ";
        status += line;
    }
    fclose(file);

    LOG_FISHNET("%s", status.c_str());
}

void print_memory_info(LogRecord &record) {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) return;

    LOG_FISHNET("Memory info:");
    std::string status;
    char line[256];
    while (fgets(line, sizeof(line), file) != nullptr) {
        status += "    ";
        status += line;
    }
    fclose(file);

    LOG_FISHNET("%s", status.c_str());
}
