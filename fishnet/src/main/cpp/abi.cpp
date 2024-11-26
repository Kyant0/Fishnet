#include "abi.h"

int pointer_width(const unwindstack::ArchEnum &arch) {
    return unwindstack::ArchIs32Bit(arch) ? 4 : 8;
}

const char *abi_string(const unwindstack::ArchEnum &arch) {
    switch (arch) {
        case unwindstack::ArchEnum::ARCH_ARM:
            return "arm";
        case unwindstack::ArchEnum::ARCH_ARM64:
            return "arm64";
        case unwindstack::ArchEnum::ARCH_RISCV64:
            return "riscv64";
        case unwindstack::ArchEnum::ARCH_X86:
            return "x86";
        case unwindstack::ArchEnum::ARCH_X86_64:
            return "x86_64";
        default:
            return "<unknown>";
    }
}
