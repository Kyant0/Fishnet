#ifndef FISHNET_ANR_SIGNAL_HANDLER_H
#define FISHNET_ANR_SIGNAL_HANDLER_H

#include <csignal>
#include <jni.h>

void init_anr_signal_handler(JavaVM *vm, JNIEnv *env);

void deinit_anr_signal_handler();

void anr_signal_handler(int signal_number, siginfo_t *info, void *context);

#endif //FISHNET_ANR_SIGNAL_HANDLER_H
