#include <jni.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <err.h>
#include <signal.h>
#include <dlfcn.h>

void deadlock();

void fdsanCrash();

__attribute__((optnone))
JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashingTestFragment_nativeCrash(JNIEnv *env, jobject obj, jstring type) {
    const char *type_str = (*env)->GetStringUTFChars(env, type, nullptr);

    if (strcmp(type_str, "nullptr") == 0) {
        char *ptr = (char *) 0;
        *ptr = 0;
    } else if (strcmp(type_str, "deadlock") == 0) {
        deadlock();
    } else if (strcmp(type_str, "too_many_open_files") == 0) {
        for (int i = 0; i < 100000; ++i) {
            dup(STDOUT_FILENO);
        }
    } else if (strcmp(type_str, "buffer_overflow") == 0) {
        char buffer[8];
        const char *input = "Overflow!";
        strcpy(buffer, input);
    } else if (strcmp(type_str, "scudo_error") == 0) { // Scudo ERROR
        jbyteArray array = (*env)->NewByteArray(env, 10);
        jbyte *elements = (*env)->GetByteArrayElements(env, array, nullptr);
        elements[20] = 42; // Access out of bounds; will likely crash
        (*env)->ReleaseByteArrayElements(env, array, elements, 0);
        (*env)->NewStringUTF(env, (const char *) elements);
    } else if (strcmp(type_str, "jni") == 0) {
        char invalid_utf8[] = {(char) 0xC3, (char) 0x28, (char) 0x00}; // 0xC3 followed by 0x28 is not valid UTF-8
        (*env)->NewStringUTF(env, invalid_utf8);
    }

    (*env)->ReleaseStringUTFChars(env, type, type_str);
}

JNIEXPORT void JNICALL
Java_com_kyant_fishnet_demo_CrashingTestFragment_nativeFdsanCrash(JNIEnv *env, jobject obj) {
    fdsanCrash();
}


void *deadlock_thread_function(void *) {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);
    return nullptr;
}

void deadlock() {
    pthread_t thread;
    pthread_create(&thread, nullptr, deadlock_thread_function, nullptr);
    pthread_join(thread, nullptr);
}


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
