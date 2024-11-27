#include "fishnet/fishnet.h"

#include <jni.h>

#include "signal_handler.h"
#include "log.h"

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

void Fishnet_init(bool enabled) {
    init_signal_handler(enabled);
}
}
