#ifndef FISHNET_LOGCAT_H
#define FISHNET_LOGCAT_H

#include <sys/types.h>

#include "log.h"

void dump_logcat(pid_t pid);

void print_logs(LogRecord &record);

#endif //FISHNET_LOGCAT_H
