#ifndef FISHNET_FD_H
#define FISHNET_FD_H

#include <sys/types.h>

#include "log.h"

void dump_open_fds(LogRecord &record, pid_t pid);

#endif //FISHNET_FD_H
