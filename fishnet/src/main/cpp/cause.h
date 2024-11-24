#ifndef FISHNET_CAUSE_H
#define FISHNET_CAUSE_H

#include "unwindstack/Unwinder.h"

void dump_probable_cause(const siginfo_t *info, unwindstack::Maps *maps,
                         const std::unique_ptr<unwindstack::Regs> &regs);

#endif //FISHNET_CAUSE_H
