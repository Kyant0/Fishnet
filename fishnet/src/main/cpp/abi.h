#ifndef FISHNET_ABI_H
#define FISHNET_ABI_H

#include <cstdint>

#include "unwindstack/Arch.h"

#if defined(__arm__)
#define ABI_STRING "arm"
#elif defined(__aarch64__)
#define ABI_STRING "arm64"
#elif defined(__i386__)
#define ABI_STRING "x86"
#elif defined(__riscv)
#define ABI_STRING "riscv64"
#elif defined(__x86_64__)
#define ABI_STRING "x86_64"
#endif

static inline uintptr_t untag_address(uintptr_t p) {
#if defined(__aarch64__)
    return p & ((1ULL << 56) - 1);
#else
    return p;
#endif
}

int pointer_width(const unwindstack::ArchEnum &arch);

const char *abi_string(const unwindstack::ArchEnum &arch);

#endif //FISHNET_ABI_H
