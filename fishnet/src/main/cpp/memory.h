#ifndef FISHNET_MEMORY_H
#define FISHNET_MEMORY_H

#include "unwindstack/AndroidUnwinder.h"

void print_tag_dump(uint64_t fault_addr, unwindstack::ArchEnum arch,
                    std::shared_ptr<unwindstack::Memory> &process_memory);

void print_thread_memory_dump(unwindstack::AndroidUnwinder *unwinder, int word_size, unwindstack::Regs *regs);

void print_memory_maps(uint64_t fault_addr, unwindstack::Maps *maps,
                       std::shared_ptr<unwindstack::Memory> &process_memory, int word_size);

#endif //FISHNET_MEMORY_H
