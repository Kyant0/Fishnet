#include "fishnet/fishnet.h"

#include <jni.h>

#include "signal_handler.h"
#include "log.h"

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_kyant_fishnet_Fishnet_init(JNIEnv *env, jobject, jint fd) {
    set_log_fd(fd);
    return true;
}

void Fishnet_init(bool enabled) {
    init_signal_handler(enabled);
}
}
