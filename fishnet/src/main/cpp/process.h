#ifndef FISHNET_PROCESS_H
#define FISHNET_PROCESS_H

#include <string>

std::string get_process_name(pid_t pid);

long get_process_uptime(pid_t pid);

#endif //FISHNET_PROCESS_H
