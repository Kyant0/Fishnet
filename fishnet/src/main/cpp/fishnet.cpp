#include <malloc.h>

#include "log.h"
#include "signal_handler.h"
#include "java_dump.h"
#include "anr.h"

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_kyant_fishnet_Fishnet_nativeInit(JNIEnv *env, jobject, jstring path,
                                          jstring packageName, jstring versionName, jlong versionCode,
                                          jstring cert) {
    const char *log_path = env->GetStringUTFChars(path, nullptr);
    set_log_path(log_path);
    env->ReleaseStringUTFChars(path, log_path);

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

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_Fishnet_nativeDumpJavaCrash(JNIEnv *env, jobject, jstring java_stack_traces) {
    const char *stack_traces = env->GetStringUTFChars(java_stack_traces, nullptr);
    fishnet_dump_with_java(stack_traces, true);
    env->ReleaseStringUTFChars(java_stack_traces, stack_traces);
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    init_signal_handler();
    init_anr_signal_handler(vm, env);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env;

    deinit_signal_handler();
    deinit_anr_signal_handler();

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }
}

}
