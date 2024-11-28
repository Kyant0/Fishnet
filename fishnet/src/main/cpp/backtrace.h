#ifndef FISHNET_BACKTRACE_H
#define FISHNET_BACKTRACE_H

#include "unwindstack/Unwinder.h"

void print_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames);

void print_backtrace_with_bytes(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames);

#endif //FISHNET_BACKTRACE_H
