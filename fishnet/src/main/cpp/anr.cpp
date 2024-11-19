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
    char *processPath = new char[MAX_BUFFER_SIZE];
    size_t size = sprintf(processPath, "/proc/%d/task", myPid);
    if (size >= MAX_BUFFER_SIZE) {
        LOG_FISHNET("Read proc path fail, read buffer size: %zu", size);
        return -1;
    }
    DIR *processDir = opendir(processPath);
    if (processDir) {
        int tid = -1;
        dirent *child = readdir(processDir);
        while (child != nullptr) {
            if (is_number_str(child->d_name, MAX_BUFFER_SIZE - 1)) {
                char *filePath = new char[MAX_BUFFER_SIZE];
                size = sprintf(filePath, "%s/%s/comm", processPath, child->d_name);
                if (size >= MAX_BUFFER_SIZE) {
                    continue;
                }
                char *threadName = new char[MAX_BUFFER_SIZE];
                int fd = open(filePath, O_RDONLY);
                delete[] filePath;
                size = read(fd, threadName, MAX_BUFFER_SIZE);
                close(fd);
                threadName[size - 1] = '\0';
                if (strcmp(threadName, "Signal Catcher") == 0) {
                    tid = atoi(child->d_name);
                    delete[] threadName;
                    break;
                }
            }
            child = readdir(processDir);
        }
        closedir(processDir);
        delete[] processPath;
        return tid;
    } else {
        delete[] processPath;
        LOG_FISHNET("Read process dir fail, error: %s", strerror(errno));
    }
    return -1;
}

void anr_signal_handler(int s, siginfo_t *si, void *uc) {
    int fromPid1 = si->_si_pad[3];
    int fromPid2 = si->_si_pad[4];
    int myPid = getpid();
    LOG_FISHNET("Receive ANR signal, fromPid1: %d, fromPid2: %d, myPid: %d", fromPid1, fromPid2, myPid);
    if (fromPid1 != myPid && fromPid2 != myPid) {
    }
    int sc_tid = get_signal_catcher_tid(myPid);
    syscall(SYS_tgkill, myPid, sc_tid, SIGQUIT);
}
