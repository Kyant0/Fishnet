#ifndef FISHNET_ANR_H
#define FISHNET_ANR_H

#include <csignal>

void anr_signal_handler(int signal_number, siginfo_t *info, void *context);

#endif //FISHNET_ANR_H
