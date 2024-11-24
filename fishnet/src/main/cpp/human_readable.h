#ifndef FISHNET_HUMAN_READABLE_H
#define FISHNET_HUMAN_READABLE_H

#include <csignal>

const char *get_signame(const siginfo_t *info);

const char *get_sigcode(const siginfo_t *info);

#endif //FISHNET_HUMAN_READABLE_H
