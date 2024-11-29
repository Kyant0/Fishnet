#ifndef FISHNET_PROCESS_H
#define FISHNET_PROCESS_H

#include <vector>

#include "log.h"

std::string get_process_name(pid_t pid);

uint64_t get_process_uptime(pid_t pid);

void get_process_tids(pid_t pid, std::vector<pid_t> &tids);

void print_process_status(LogRecord &record, pid_t pid);

void print_memory_info(LogRecord &record);

#endif //FISHNET_PROCESS_H
