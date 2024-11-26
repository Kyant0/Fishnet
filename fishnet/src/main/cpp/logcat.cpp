#include "logcat.h"

#include <vector>

#include "log.h"

static std::vector<std::string> logs{};

void dump_logcat(pid_t pid) {
    char command[24];
    snprintf(command, sizeof(command), "logcat -d --pid=%d", pid);
    FILE *fp = popen(command, "r");
    char line[1024];
    if (fp) {
        // 11-19 22:37:54.578 10167 10167 D Com...
        while (fgets(line, sizeof(line), fp) != nullptr) {
            // Remove the line:
            // --------- beginning of main
            if (line[0] < '0' || line[0] > '9') continue;
            logs.emplace_back(line);
        }
        pclose(fp);
    }
}

void print_logs() {
    LOG_FISHNET("");
    LOG_FISHNET("--------- log main");

    const size_t size = logs.size();

    for (size_t i = 0; i < size; ++i) {
        LOG_FISHNET_LN("%s", logs[i].c_str());
    }
}
