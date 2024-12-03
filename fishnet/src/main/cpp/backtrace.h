#ifndef FISHNET_BACKTRACE_H
#define FISHNET_BACKTRACE_H

#include "unwindstack/Unwinder.h"

#include "log.h"

void print_backtrace(LogRecord &record, unwindstack::ArchEnum arch,
                     const std::vector<unwindstack::FrameData> &frames);

#endif //FISHNET_BACKTRACE_H
