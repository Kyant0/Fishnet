#ifndef FISHNET_MEMORY_H
#define FISHNET_MEMORY_H

#include "unwindstack/Maps.h"
#include "unwindstack/Regs.h"

void print_tag_dump(uint64_t fault_addr, unwindstack::ArchEnum arch,
                    std::shared_ptr<unwindstack::Memory> &process_memory);

void print_thread_memory_dump(int word_size, const std::unique_ptr<unwindstack::Regs> &regs, unwindstack::Maps *maps,
                              unwindstack::Memory *memory);

void print_memory_maps(uint64_t fault_addr, int word_size, unwindstack::Maps *maps,
                       std::shared_ptr<unwindstack::Memory> &process_memory);

#endif //FISHNET_MEMORY_H
