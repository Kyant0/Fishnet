#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fishnet/fishnet.h"

#include <err.h>
#include <unistd.h>

#include <chrono>
#include <thread>
#include <vector>

#include "unique_fd.h"

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

void victim() {
    sleep_for(300ms);
    int fd = dup(STDOUT_FILENO);
    sleep_for(200ms);
    ssize_t rc = write(fd, "good\n", 5);
    if (rc == -1) {
        err(1, "good failed to write?!");
    }
    close(fd);
}

void bystander() {
    sleep_for(100ms);
    int fd = dup(STDOUT_FILENO);
    sleep_for(300ms);
    close(fd);
}

void offender() {
    int fd = dup(STDOUT_FILENO);
    close(fd);
    sleep_for(200ms);
    close(fd);
}

void fdsanCrash() {
    std::vector<std::thread> threads;
    for (auto function: {victim, bystander, offender}) {
        threads.emplace_back(function);
    }
    for (auto &thread: threads) {
        thread.join();
    }
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashFragment_signal(JNIEnv *env, jobject, jint signal) {
    kill(getpid(), signal);
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashFragment_nativeCrash(JNIEnv *env, jobject, jstring type) {
    const char *type_str = env->GetStringUTFChars(type, nullptr);
    if (false) { // SIGSEGV
        char *ptr = (char *) 0x114514;
        *ptr = 0;
    } else if (false) { // ANR
        for (int i = 0; i < 100000; ++i) {
            dup(STDOUT_FILENO);
        }
    } else if (true) { // buffer overflow
        char buffer[8]; // Small buffer of 8 bytes

        // Copy more data than the buffer can hold (intentional overflow)
        const char *input = "Overflow!"; // This is 9 bytes, including the null terminator
        strcpy(buffer, input); // Compiler won't catch this as an overflow at compile time

        printf("Buffer contains: %s\n", buffer);
    } else if (false) { // SIGFPE
        int a = 1 / 0;
    } else if (false) { // SIGABRT
        abort();
    } else if (false) { // SIGBUS
        char *ptr = (char *) 0x114514;
        *ptr = 0;
    } else if (false) { // SIGILL
        __builtin_trap();
    } else if (false) { // SIGTRAP
        __builtin_trap();
    } else if (false) { // SIGSYS
        syscall(0);
    }
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashFragment_jniAbort(JNIEnv *env, jobject) {
    char invalid_utf8[] = {0xC3, 0x28, 0x00}; // 0xC3 followed by 0x28 is not valid UTF-8
    env->NewStringUTF(invalid_utf8);
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashFragment_nativeFdsanCrash(JNIEnv *env, jobject) {
    fdsanCrash();
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    Fishnet_init(true);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }
}

}
