#ifndef FISHNET_BACKTRACE_H
#define FISHNET_BACKTRACE_H

#include "unwindstack/Unwinder.h"

void dump_thread_backtrace(const std::vector<unwindstack::FrameData> &frames);

void print_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames);

void print_thread_backtrace(unwindstack::ArchEnum arch, const std::vector<unwindstack::FrameData> &frames);

#endif //FISHNET_BACKTRACE_H
