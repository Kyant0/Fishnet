#include "logcat.h"

#include <vector>

#include "log.h"

static std::vector<std::string> logs{};

void dump_logcat() {
    FILE *fp = popen("logcat -d", "r");
    char line[1024];
    if (fp) {
        // 11-19 22:37:54.578 10167 10167 D Com...
        while (fgets(line, sizeof(line), fp) != nullptr) {
            logs.emplace_back(line);
        }
        pclose(fp);
    }
}

void print_logs() {
    LOG_FISHNET("");
    LOG_FISHNET("--------- log main");

    int size = (int) logs.size();

    for (int i = 0; i < size; ++i) {
        LOG_FISHNET_LN("%s", logs[i].c_str());
    }
}
