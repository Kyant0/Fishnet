#ifndef FISHNET_THREAD_H
#define FISHNET_THREAD_H

#include "unwindstack/AndroidUnwinder.h"

void print_main_thread_header(pid_t pid, pid_t tid, uid_t uid);

void print_main_thread(pid_t pid, pid_t tid, uid_t uid, const siginfo_t *info, int word_size,
                       const unwindstack::ArchEnum &arch, unwindstack::AndroidUnwinder *unwinder,
                       const std::unique_ptr<unwindstack::Regs> &regs,
                       const std::vector<unwindstack::FrameData> &frames,
                       bool dump_memory, bool dump_memory_maps);

void print_thread(pid_t tid, int word_size, const unwindstack::ArchEnum &arch, unwindstack::ThreadUnwinder *unwinder,
                  bool dump_memory);

#endif //FISHNET_THREAD_H
