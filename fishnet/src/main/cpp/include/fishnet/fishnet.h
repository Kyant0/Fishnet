#ifndef FISHNET_H
#define FISHNET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

#define FISHNET_EXPORT __attribute__((visibility("default")))

FISHNET_EXPORT void Fishnet_init(JavaVM *vm, JNIEnv *env, bool enabled);

FISHNET_EXPORT void Fishnet_deinit();

#ifdef __cplusplus
}
#endif

#endif //FISHNET_H
