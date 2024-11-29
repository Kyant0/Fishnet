#include "fishnet/fishnet.h"

#include <malloc.h>

#include "log.h"
#include "signal_handler.h"
#include "dump.h"
#include "anr.h"

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_kyant_fishnet_Fishnet_nativeInit(JNIEnv *env, jobject, jint fd,
                                          jstring packageName, jstring versionName, jlong versionCode,
                                          jstring cert) {
    set_log_fd(fd);

    const char *package_name = env->GetStringUTFChars(packageName, nullptr);
    const char *version_name = env->GetStringUTFChars(versionName, nullptr);
    const char *cert_str = env->GetStringUTFChars(cert, nullptr);
    set_apk_info({
                         strdup(package_name),
                         strdup(version_name),
                         (uint64_t) versionCode,
                         strdup(cert_str),
                 });

    env->ReleaseStringUTFChars(packageName, package_name);
    env->ReleaseStringUTFChars(versionName, version_name);
    env->ReleaseStringUTFChars(cert, cert_str);
    return true;
}

JNIEXPORT jstring JNICALL
Java_com_kyant_fishnet_Fishnet_nativeDump(JNIEnv *env, jobject, jstring java_stack_traces) {
    const char *stack_traces = env->GetStringUTFChars(java_stack_traces, nullptr);
    char *dump = fishnet_dump_with_java(stack_traces, true);
    env->ReleaseStringUTFChars(java_stack_traces, stack_traces);
    jstring result = env->NewStringUTF(dump);
    free(dump);
    return result;
}

void Fishnet_init(JavaVM *vm, JNIEnv *env, bool enabled) {
    init_signal_handler(enabled);
    init_anr_signal_handler(vm, env);
}

void Fishnet_deinit() {
    deinit_signal_handler();
    deinit_anr_signal_handler();
}
}
