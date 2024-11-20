#ifndef FISHNET_THREAD_H
#define FISHNET_THREAD_H

#include <string>

#include "unwindstack/AndroidUnwinder.h"

std::string get_thread_name(pid_t tid);

void print_thread_header(pid_t pid, pid_t tid, uid_t uid);

void print_main_thread(pid_t pid, pid_t tid, uid_t uid, siginfo_t *si, int word_size,
                       unwindstack::ArchEnum arch, unwindstack::AndroidUnwinder *unwinder,
                       unwindstack::AndroidUnwinderData *data, unwindstack::Regs *regs,
                       bool dump_memory);

void print_thread(pid_t pid, pid_t tid, uid_t uid, unwindstack::ThreadUnwinder *unwinder, bool dump_memory);

void print_guest_thread(pid_t tid, unwindstack::ThreadUnwinder *unwinder, bool dump_memory);

#endif //FISHNET_THREAD_H
