#include "backtrace.h"

#include <set>

#include "log.h"

void dump_thread_backtrace(const std::vector<unwindstack::FrameData> &frames) {
    std::set<std::string> unreadable_elf_files;
    for (const auto &frame: frames) {
        if (frame.map_info != nullptr && frame.map_info->ElfFileNotReadable()) {
            unreadable_elf_files.emplace(frame.map_info->name());
        }
    }

    if (!unreadable_elf_files.empty()) {
        LOG_FISHNET("  NOTE: %s", "Function names and BuildId information is missing for some frames due");
        LOG_FISHNET("\n  NOTE: %s", "to unreadable libraries. For unwinds of apps, only shared libraries");
        LOG_FISHNET("\n  NOTE: %s", "found under the lib/ directory are readable.");
        LOG_FISHNET("\n  NOTE: %s", "On this device, run setenforce 0 to make the libraries readable.");
        LOG_FISHNET("\n  NOTE: %s", "Unreadable libraries:");
        for (auto &name: unreadable_elf_files) {
            LOG_FISHNET("\n  NOTE:   %s", name.c_str());
        }
    }
}

void print_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames) {
    for (const auto &frame: frames) {
        LOG_FISHNET("    %s", unwindstack::Unwinder::FormatFrame(arch, frame, true).c_str());
    }
}

void print_thread_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames) {
    LOG_FISHNET("");
    LOG_FISHNET("%zu total frames", frames.size());
    LOG_FISHNET("backtrace:");
    print_backtrace(arch, frames);
}
