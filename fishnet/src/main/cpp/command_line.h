#ifndef FISHNET_COMMAND_LINE_H
#define FISHNET_COMMAND_LINE_H

#include <string>
#include <sys/types.h>
#include <vector>

std::vector<std::string> get_command_line(pid_t pid);

#endif //FISHNET_COMMAND_LINE_H
