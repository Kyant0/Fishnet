#ifndef FISHNET_REGISTERS_H
#define FISHNET_REGISTERS_H

#include "unwindstack/Regs.h"

void print_thread_registers(unwindstack::ArchEnum arch, int word_size, const std::unique_ptr<unwindstack::Regs> &regs);

#endif //FISHNET_REGISTERS_H
