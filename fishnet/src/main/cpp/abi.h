#ifndef FISHNET_ABI_H
#define FISHNET_ABI_H

#include "unwindstack/Arch.h"

#if defined(__arm__)
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_ARM
#define ABI_STRING "arm"
#elif defined(__aarch64__)
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_ARM64
#define ABI_STRING "arm64"
#elif defined(__i386__)
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_X86;
#define ABI_STRING "x86"
#elif defined(__x86_64__)
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_X86_64;
#define ABI_STRING "x86_64"
#elif defined(__riscv)
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_RISCV64;
#define ABI_STRING "riscv64"
#else
#define CURRENT_ARCH unwindstack::ArchEnum::ARCH_UNKNOWN;
#define ABI_STRING "<unknown>"
#endif

constexpr static inline uintptr_t untag_address(uintptr_t p) {
#if defined(__aarch64__)
    return p & ((1ULL << 56) - 1);
#else
    return p;
#endif
}

consteval static inline unwindstack::ArchEnum current_arch() {
    return CURRENT_ARCH;
}

consteval static inline int pointer_width() {
#if defined(__arm__) || defined(__i386__)
    return 4;
#else
    return 8;
#endif
}

consteval static inline const char *abi_string() {
    return ABI_STRING;
}

#endif //FISHNET_ABI_H
