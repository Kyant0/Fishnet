#include <jni.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <err.h>
#include <signal.h>
#include <dlfcn.h>

void victim() {
    usleep(300000); // 300ms
    int fd = dup(STDOUT_FILENO);
    usleep(200000); // 200ms
    ssize_t rc = write(fd, "good\n", 5);
    if (rc == -1) {
        err(1, "good failed to write?!");
    }
    close(fd);
}

void bystander() {
    usleep(100000); // 100ms
    int fd = dup(STDOUT_FILENO);
    usleep(300000); // 300ms
    close(fd);
}

void offender() {
    int fd = dup(STDOUT_FILENO);
    close(fd);
    usleep(200000); // 200ms
    close(fd);
}

void *thread_function(void *arg) {
    void (*function)() = (void (*)()) arg;
    function();
    return nullptr;
}

void fdsanCrash() {
    pthread_t threads[3];
    void (*functions[3])() = {victim, bystander, offender};

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], nullptr, thread_function, functions[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }
}

__attribute__((optnone))
void *crash_thread_function(void *arg) {
    const char *type_str = (const char *) arg;

    if (strcmp(type_str, "SIGSEGV") == 0) {
        char *ptr = (char *) 0;
        *ptr = 0;
    } else if (strcmp(type_str, "ANR") == 0) {
        for (int i = 0; i < 100000; ++i) {
            dup(STDOUT_FILENO);
        }
    } else if (strcmp(type_str, "buffer overflow") == 0) {
        char buffer[8]; // Small buffer of 8 bytes

        // Copy more data than the buffer can hold (intentional overflow)
        const char *input = "Overflow!"; // This is 9 bytes, including the null terminator
        strcpy(buffer, input); // Compiler won't catch this as an overflow at compile time

        printf("Buffer contains: %s\n", buffer);
    }

    return nullptr;
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashingTestFragment_nativeCrash(JNIEnv *env, jobject obj, jstring type) {
    const char *type_str = (*env)->GetStringUTFChars(env, type, nullptr);
    pthread_t crash_thread;

    pthread_create(&crash_thread, nullptr, crash_thread_function, (void *) type_str);
    pthread_join(crash_thread, nullptr);
    (*env)->ReleaseStringUTFChars(env, type, type_str);
}

JNIEXPORT jstring JNICALL
Java_com_kyant_fishnet_demo_CrashingTestFragment_jniAbort(JNIEnv *env, jobject obj) {
    if (false) {
        char invalid_utf8[] = {(char) 0xC3, (char) 0x28, (char) 0x00}; // 0xC3 followed by 0x28 is not valid UTF-8
        return (*env)->NewStringUTF(env, invalid_utf8);
    } else { // Scudo ERROR
        jbyteArray array = (*env)->NewByteArray(env, 10);
        jbyte *elements = (*env)->GetByteArrayElements(env, array, nullptr);
        elements[20] = 42; // Access out of bounds; will likely crash
        (*env)->ReleaseByteArrayElements(env, array, elements, 0);
        return (*env)->NewStringUTF(env, (const char *) elements);
    }
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashingTestFragment_nativeFdsanCrash(JNIEnv *env, jobject obj) {
    fdsanCrash();
}
