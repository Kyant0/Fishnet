#include "process.h"

#include <bits/sysconf.h>
#include <cstdlib>

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

long get_uptime() {
    FILE *file = fopen("/proc/uptime", "r");
    if (!file) return -1;

    double uptime_seconds;
    if (fscanf(file, "%lf", &uptime_seconds) != 1) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return (long) uptime_seconds;
}

long get_process_start_time(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;

    long start_time = 0;
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), file)) {
        // Split the stat file contents to find the 22nd field
        char *token = strtok(buffer, " ");
        for (int i = 1; i <= 22; i++) {
            if (!token) break;
            if (i == 22) {
                start_time = atol(token);
            }
            token = strtok(nullptr, " ");
        }
    }
    fclose(file);
    return start_time;
}

long get_process_uptime(pid_t pid) {
    long uptime = get_uptime();
    if (uptime == -1) return -1;

    long clock_ticks = sysconf(_SC_CLK_TCK);
    long start_time = get_process_start_time(pid);
    if (start_time == -1) return -1;

    // Convert start time to seconds and calculate uptime
    long process_uptime = uptime - (start_time / clock_ticks);
    return process_uptime;
}
