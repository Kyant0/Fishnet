#include "backtrace.h"

#include "log.h"

void print_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames) {
    LOG_FISHNET("");
    LOG_FISHNET("backtrace:");
    for (const auto &frame: frames) {
        LOG_FISHNET("  %s", unwindstack::Unwinder::FormatFrame(arch, frame, false).c_str());
    }
}

void print_backtrace_with_bytes(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames) {
    LOG_FISHNET("");
    LOG_FISHNET("backtrace:");
    bool is_first_frame = true;
    for (const auto &frame: frames) {
        LOG_FISHNET("  %s", unwindstack::Unwinder::FormatFrame(arch, frame, false).c_str());

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
                hexdump += StringPrintf("%02x", bytes[i]);
                if (i + 1 < size) {
                    if ((i + 1) % 16 == 0) {
                        hexdump += "\n                             ";
                    } else if ((i + 1) % 8 == 0) {
                        hexdump += ' ';
                    }
                }
            }
            LOG_FISHNET("                 bytes here: %s", hexdump.c_str());
        }
    }
}
