#include "backtrace.h"

#include <set>

#include "log.h"

#if defined(__arm__)
#define CS_ARCH CS_ARCH_ARM
#define CS_MODE CS_MODE_ARM
#elif defined(__aarch64__)
#define CS_ARCH CS_ARCH_AARCH64
#define CS_MODE CS_MODE_LITTLE_ENDIAN
#elif defined(__i386__)
#define CS_ARCH CS_ARCH_X86
#define CS_MODE CS_MODE_32
#elif defined(__riscv)
#define CS_ARCH CS_ARCH_RISCV
#define CS_MODE CS_MODE_64
#elif defined(__x86_64__)
#define CS_ARCH CS_ARCH_X86
#define CS_MODE CS_MODE_64
#endif

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
    bool is_first_frame = true;
    for (const auto &frame: frames) {
        LOG_FISHNET("    %s", unwindstack::Unwinder::FormatFrame(arch, frame, true).c_str());

        if (is_first_frame) {
            is_first_frame = false;

            if (!frame.map_info) {
                continue;
            }

            const std::shared_ptr<unwindstack::Elf> elf = frame.map_info->elf();
            if (!elf) {
                continue;
            }

            const size_t size = std::min(frame.map_info->end() - frame.pc, (uint64_t) 128);
            const std::shared_ptr<unwindstack::Memory> memory = elf->memory();
            if (!memory) {
                continue;
            }

            uint8_t bytes[size];
            if (!memory->ReadFully(frame.rel_pc, bytes, sizeof(bytes))) {
                continue;
            }

            std::string hexdump;
            for (size_t i = 0; i < size; i++) {
                hexdump += StringPrintf("%02x ", bytes[i]);
            }
            LOG_FISHNET("                   bytes here: %s", hexdump.c_str());
        }
    }
}

void print_thread_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames) {
    LOG_FISHNET("");
    LOG_FISHNET("%zu total frames", frames.size());
    LOG_FISHNET("backtrace:");
    print_backtrace(arch, frames);
}
