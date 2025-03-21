#include "logcat.h"

#include <vector>

static std::vector<std::string> logs{};

void dump_logcat(pid_t pid) {
    char command[28];
    snprintf(command, sizeof(command), "logcat -t 100 --pid=%d", pid);
    FILE *fp = popen(command, "r");
    if (fp == nullptr) return;

    char line[1024];
    if (fp) {
        fgets(line, sizeof(line), fp); // skip the first line
        // 11-19 22:37:54.578 10167 10167 D Com...
        while (fgets(line, sizeof(line), fp) != nullptr) {
            logs.emplace_back(line);
        }
        pclose(fp);
    }
}

void print_logs(LogRecord &record) {
    LOG_FISHNET("");
    LOG_FISHNET("log main:");

    const size_t size = logs.size();

    for (size_t i = 0; i < size; ++i) {
        LOG_FISHNET_LN("    %s", logs[i].c_str());
    }
}
