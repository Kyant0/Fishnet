#ifndef FISHNET_ABORT_MESSAGE_H
#define FISHNET_ABORT_MESSAGE_H

#include "unwindstack/Unwinder.h"

void set_aborter();

void try_read_abort_message_from_logcat();

void get_scudo_message_if_needed(unwindstack::ArchEnum &arch, unwindstack::Regs *regs,
                                 const std::vector<unwindstack::FrameData> &frames);

void dump_abort_message();

#endif //FISHNET_ABORT_MESSAGE_H
