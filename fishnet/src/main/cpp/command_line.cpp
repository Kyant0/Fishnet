#include "command_line.h"

std::vector<std::string> get_command_line(pid_t pid) {
    std::vector<std::string> result;

    char process_name[256];
    char path[22];

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE *file = fopen(path, "r");
    if (!file) {
        result.emplace_back("<unknown>");
        return result;
    }
    if (fgets(process_name, sizeof(process_name), file) == nullptr) {
        fclose(file);
        result.emplace_back("<unknown>");
        return result;
    }

    std::string cmdline(process_name);
    auto it = cmdline.cbegin();
    while (it != cmdline.cend()) {
        auto terminator = std::find(it, cmdline.cend(), '\0');
        result.emplace_back(it, terminator);
        it = std::find_if(terminator, cmdline.cend(), [](char c) { return c != '\0'; });
    }
    if (result.empty()) {
        result.emplace_back("<unknown>");
    }

    return result;
}
