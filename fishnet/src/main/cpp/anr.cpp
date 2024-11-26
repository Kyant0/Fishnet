#include "anr.h"

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <syscall.h>
#include <unistd.h>

#include "log.h"

#define MAX_BUFFER_SIZE 64

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
        LOG_FISHNET("Read proc path fail, read buffer size: %zu", size);
        return -1;
    }
    DIR *processDir = opendir(processPath);
    if (processDir) {
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
    } else {
        LOG_FISHNET("Read process dir fail, error: %s", strerror(errno));
    }
    return -1;
}

void anr_signal_handler(int signal_number, siginfo_t *info, void *context) {
    int fromPid1 = info->_si_pad[3];
    int fromPid2 = info->_si_pad[4];
    int myPid = getpid();
    LOG_FISHNET("Receive ANR signal, fromPid1: %d, fromPid2: %d, myPid: %d", fromPid1, fromPid2, myPid);
    if (fromPid1 != myPid && fromPid2 != myPid) {
    }
    int sc_tid = get_signal_catcher_tid(myPid);
    syscall(SYS_tgkill, myPid, sc_tid, SIGQUIT);
}
