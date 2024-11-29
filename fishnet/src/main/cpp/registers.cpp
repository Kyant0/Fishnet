#include "registers.h"

#include <cinttypes>
#include <unordered_set>

static void print_register_row(LogRecord &record, int word_size,
                               const std::vector<std::pair<std::string, uint64_t>> &row) {
    std::string output = "  ";
    for (const auto &[name, value]: row) {
        output += StringPrintf("  %-3s %0*" PRIx64, name.c_str(), 2 * word_size, (uint64_t) value);
    }
    LOG_FISHNET("%s", output.c_str());
}

void print_thread_registers(LogRecord &record, unwindstack::ArchEnum arch, int word_size,
                            const std::unique_ptr<unwindstack::Regs> &regs) {
    static constexpr size_t column_count = 4;
    std::vector<std::pair<std::string, uint64_t>> current_row;
    std::vector<std::pair<std::string, uint64_t>> special_row;
    std::unordered_set<std::string> special_registers;

    switch (arch) {
        case unwindstack::ArchEnum::ARCH_ARM:
        case unwindstack::ArchEnum::ARCH_ARM64:
            special_registers = {"ip", "lr", "sp", "pc", "pst"};
            break;

        case unwindstack::ArchEnum::ARCH_RISCV64:
            special_registers = {"ra", "sp", "pc"};
            break;

        case unwindstack::ArchEnum::ARCH_X86:
            special_registers = {"ebp", "esp", "eip"};
            break;

        case unwindstack::ArchEnum::ARCH_X86_64:
            special_registers = {"rbp", "rsp", "rip"};
            break;

        default:
            LOG_FISHNET("Unknown architecture %d printing thread registers", arch);
            return;
    }

    LOG_FISHNET("");
    LOG_FISHNET("registers:");

    regs->IterateRegisters([&](const char *name, uint64_t value) {
        auto row = &current_row;
        if (special_registers.count(name) == 1) {
            row = &special_row;
        }

        row->emplace_back(name, value);
        if (current_row.size() == column_count) {
            print_register_row(record, word_size, current_row);
            current_row.clear();
        }
    });

    if (!current_row.empty()) {
        print_register_row(record, word_size, current_row);
    }

    print_register_row(record, word_size, special_row);
}
