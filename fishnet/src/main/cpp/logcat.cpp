#include "logcat.h"

#include "log.h"

static std::string logs{};

void dump_logcat() {
    FILE *fp = popen("logcat -d -t 50", "r");
    if (fp == nullptr) return;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        StringAppendF(&logs, "%s", buffer);
    }
    pclose(fp);
}

void print_logs() {
    LOG_FISHNET("");
    LOG_FISHNET("--------- log %s", "system & main");
    LOG_FISHNET("%s", logs.c_str());
    LOG_FISHNET("--------- tail end of log %s", "system & main");
}
