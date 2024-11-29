#include "anr.h"

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <syscall.h>
#include <unistd.h>

#include "log.h"
#include "dump.h"

#define MAX_BUFFER_SIZE 64

static JavaVM *g_vm = nullptr;
static jclass exception_handler_class = nullptr;

void init_anr_signal_handler(JavaVM *vm, JNIEnv *env) {
    g_vm = vm;

    jclass _exception_handler_class = env->FindClass("com/kyant/fishnet/JavaExceptionHandler");
    if (_exception_handler_class == nullptr) {
        LOGE("FindClass failed");
        return;
    }

    exception_handler_class = (jclass) env->NewGlobalRef(_exception_handler_class);
    if (exception_handler_class == nullptr) {
        LOGE("NewGlobalRef failed");
        return;
    }

    sigset_t sig_sets;
    sigemptyset(&sig_sets);
    sigaddset(&sig_sets, SIGQUIT);
    pthread_sigmask(SIG_UNBLOCK, &sig_sets, nullptr);

    struct sigaction sigAction{};
    sigfillset(&sigAction.sa_mask);
    sigAction.sa_flags = SA_RESTART | SA_ONSTACK | SA_SIGINFO;
    sigAction.sa_sigaction = anr_signal_handler;
    sigaction(SIGQUIT, &sigAction, nullptr);
}

void deinit_anr_signal_handler() {
    sigset_t sig_sets;
    sigemptyset(&sig_sets);
    sigaddset(&sig_sets, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &sig_sets, nullptr);

    g_vm = nullptr;
}

static bool is_number_str(const char *str, size_t max_len) {
    if (str == nullptr || strlen(str) > max_len) {
        return false;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

int get_signal_catcher_tid(pid_t myPid) {
    char processPath[MAX_BUFFER_SIZE];
    size_t size = snprintf(processPath, sizeof(processPath), "/proc/%d/task", myPid);
    if (size >= MAX_BUFFER_SIZE) {
        return -1;
    }
    DIR *processDir = opendir(processPath);
    if (!processDir) {
        return -1;
    }

    int tid = -1;
    char filePath[MAX_BUFFER_SIZE];
    char threadName[MAX_BUFFER_SIZE];
    dirent *child = readdir(processDir);
    while (child != nullptr) {
        if (is_number_str(child->d_name, MAX_BUFFER_SIZE - 1)) {
            size = snprintf(filePath, sizeof(filePath), "%s/%s/comm", processPath, child->d_name);
            if (size >= MAX_BUFFER_SIZE) {
                continue;
            }
            int fd = open(filePath, O_RDONLY);
            size = read(fd, threadName, MAX_BUFFER_SIZE);
            close(fd);
            threadName[size - 1] = '\0';
            if (strcmp(threadName, "Signal Catcher") == 0) {
                tid = atoi(child->d_name);
                break;
            }
        }
        child = readdir(processDir);
    }
    closedir(processDir);
    return tid;
}

void *dump_java_threads_thread(void *arg) {
    LOGE("dump_java_threads_thread started");

    if (g_vm == nullptr) {
        LOGE("JavaVM is null");
        return nullptr;
    }

    JNIEnv *env;
    jint status = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (status == JNI_EDETACHED) {
        status = g_vm->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            LOGE("AttachCurrentThread failed: %d", status);
            return nullptr;
        }
    } else if (status != JNI_OK) {
        LOGE("GetEnv failed: %d", status);
        return nullptr;
    }

    jmethodID dump_method = env->GetStaticMethodID(exception_handler_class, "dumpJavaThreads", "()Ljava/lang/String;");
    if (dump_method == nullptr) {
        LOGE("GetMethodID failed");
        return nullptr;
    }

    jstring thread_dump = (jstring) env->CallStaticObjectMethod(exception_handler_class, dump_method);
    if (thread_dump == nullptr) {
        LOGE("CallObjectMethod failed");
        return nullptr;
    }

    const char *thread_dump_chars = env->GetStringUTFChars(thread_dump, nullptr);
    char *dump = fishnet_dump_with_java(thread_dump_chars, false);
    free(dump);
    env->ReleaseStringUTFChars(thread_dump, thread_dump_chars);

    g_vm->DetachCurrentThread();

    return nullptr;
}

void anr_signal_handler(int signal_number, siginfo_t *info, void *context) {
    const int fromPid1 = info->_si_pad[3];
    const int fromPid2 = info->_si_pad[4];
    const int pid = getpid();
    LOGE("Receive ANR signal, fromPid1: %d, fromPid2: %d, pid: %d", fromPid1, fromPid2, pid);

    if (fromPid1 != pid && fromPid2 != pid) {
        pthread_t thread_id;
        if (pthread_create(&thread_id, nullptr, dump_java_threads_thread, nullptr) != 0) {
            LOGE("pthread_create failed");
            goto resend;
        }
        pthread_join(thread_id, nullptr);
    }

    resend:
    const int sc_tid = get_signal_catcher_tid(pid);
    syscall(SYS_tgkill, pid, sc_tid, SIGQUIT);
}
