#ifndef FISHNET_CAUSE_H
#define FISHNET_CAUSE_H

#include "unwindstack/Unwinder.h"

#include "log.h"

void dump_probable_cause(LogRecord &record, const siginfo_t *info, unwindstack::Maps *maps,
                         const std::unique_ptr<unwindstack::Regs> &regs);

#endif //FISHNET_CAUSE_H
