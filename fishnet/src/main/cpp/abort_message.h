#ifndef FISHNET_ABORT_MESSAGE_H
#define FISHNET_ABORT_MESSAGE_H

#include "unwindstack/Unwinder.h"

#include "log.h"

void set_aborter();

void try_read_abort_message_from_logcat(pid_t pid);

void get_scudo_message_if_needed(const unwindstack::ArchEnum &arch, const std::unique_ptr<unwindstack::Regs> &regs,
                                 const std::vector<unwindstack::FrameData> &frames);

void dump_abort_message(LogRecord &record);

#endif //FISHNET_ABORT_MESSAGE_H
