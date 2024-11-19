#ifndef FISHNET_ANR_H
#define FISHNET_ANR_H

#include <csignal>

void anr_signal_handler(int s, siginfo_t *si, void *uc);

#endif //FISHNET_ANR_H
