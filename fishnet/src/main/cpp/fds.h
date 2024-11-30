#ifndef FISHNET_FDS_H
#define FISHNET_FDS_H

#include <sys/types.h>

#include "log.h"

void dump_open_fds(LogRecord &record, pid_t pid);

#endif //FISHNET_FDS_H
