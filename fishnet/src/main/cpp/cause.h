#ifndef FISHNET_CAUSE_H
#define FISHNET_CAUSE_H

#include "unwindstack/Unwinder.h"

void dump_probable_cause(const siginfo *si, unwindstack::Maps *maps, unwindstack::Regs *regs);

#endif //FISHNET_CAUSE_H
