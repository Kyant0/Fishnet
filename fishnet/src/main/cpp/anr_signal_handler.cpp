#include "anr_signal_handler.h"

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <syscall.h>
#include <unistd.h>

#include "log.h"
#include "dump.h"

static sigset_t old_set;
static struct sigaction old_action;

static JavaVM *g_vm = nullptr;
static jclass exception_handler_class = nullptr;

void init_anr_signal_handler(JavaVM *vm, JNIEnv *env) {
    sigset_t sig_sets;
    sigemptyset(&sig_sets);
    sigaddset(&sig_sets, SIGQUIT);
    pthread_sigmask(SIG_UNBLOCK, &sig_sets, &old_set);

    struct sigaction action{};
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART | SA_ONSTACK | SA_SIGINFO;
    action.sa_sigaction = anr_signal_handler;
    sigaction(SIGQUIT, &action, &old_action);

    g_vm = vm;

    jclass _exception_handler_class = env->FindClass("com/kyant/fishnet/JavaExceptionHandler");
    exception_handler_class = (jclass) env->NewGlobalRef(_exception_handler_class);
    env->DeleteLocalRef(_exception_handler_class);
}

void deinit_anr_signal_handler() {
    pthread_sigmask(SIG_SETMASK, &old_set, nullptr);
    sigaction(SIGQUIT, &old_action, nullptr);

    JNIEnv *env;
    if (g_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        g_vm = nullptr;
        return;
    }

    env->DeleteGlobalRef(exception_handler_class);

    g_vm = nullptr;
}

static int get_signal_catcher_tid(pid_t myPid) {
    char task[19];
    snprintf(task, sizeof(task), "/proc/%d/task", myPid);
    DIR *processDir = opendir(task);
    if (!processDir) {
        return -1;
    }

    int tid = -1;
    char path[32];
    char name[16];
    const dirent *child;
    constexpr char signal_catcher[] = "Signal Catcher";
    while ((child = readdir(processDir)) != nullptr) {
        snprintf(path, sizeof(path), "%s/%s/comm", task, child->d_name);
        const int fd = open(path, O_RDONLY);
        ssize_t size = read(fd, name, sizeof(name));
        close(fd);
        if (size <= 0) {
            continue;
        }

        name[size - 1] = '\0';
        if (strcmp(name, signal_catcher) == 0) {
            tid = atoi(child->d_name);
            break;
        }
    }
    closedir(processDir);
    return tid;
}

static void *fishnet_anr_dispatch_thread(void *arg) {
    const auto *info = (const DebuggerThreadInfo *) arg;

    JNIEnv *env;
    jint status = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (status == JNI_EDETACHED) {
        status = g_vm->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            return nullptr;
        }
    } else if (status != JNI_OK) {
        return nullptr;
    }

    jmethodID dump_method = env->GetStaticMethodID(exception_handler_class, "dumpJavaThreads",
                                                   "()Ljava/lang/String;");
    auto thread_dump = (jstring) env->CallStaticObjectMethod(exception_handler_class, dump_method);

    const char *thread_dump_chars = env->GetStringUTFChars(thread_dump, nullptr);
    fishnet_dump_anr(thread_dump_chars, info);
    env->ReleaseStringUTFChars(thread_dump, thread_dump_chars);

    g_vm->DetachCurrentThread();

    return nullptr;
}

void anr_signal_handler(int signal_number, siginfo_t *info, void *context) {
    const int from_pid_1 = info->_si_pad[3];
    const int from_pid_2 = info->_si_pad[4];
    const int pid = getpid();

    LOGE("Received ANR signal from_pid_1: %d, from_pid_2: %d, pid: %d", from_pid_1, from_pid_2, pid);

    if (from_pid_1 != pid && from_pid_2 != pid) {
        DebuggerThreadInfo thread_info = {
                .crashing_tid = gettid(),
                .siginfo = info,
                .ucontext = context,
        };

        pthread_t thread_id;
        if (pthread_create(&thread_id, nullptr, fishnet_anr_dispatch_thread, &thread_info) != 0) {
            goto resend;
        }
        pthread_join(thread_id, nullptr);
    }

    resend:
    const int sc_tid = get_signal_catcher_tid(pid);
    if (sc_tid != -1) {
        syscall(SYS_tgkill, pid, sc_tid, SIGQUIT);
    }
}
