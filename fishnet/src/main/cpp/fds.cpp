#include "fds.h"

#include <android/fdsan.h>
#include <cinttypes>
#include <dirent.h>
#include <vector>
#include <unistd.h>

struct FD {
    int32_t fd;
    std::string path;
    std::string owner;
    uint64_t tag;
};

void dump_open_fds(LogRecord &record, pid_t pid) {
    char fd_dir[17];
    snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", pid);

    DIR *dir = opendir(fd_dir);
    if (!dir) return;

    std::vector<FD> open_fds;

    struct dirent *entry;
    char fd_path[24], path[PATH_MAX];
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;

        const int fd = atoi(entry->d_name);
        FD fd_info = {.fd = fd};

        snprintf(fd_path, sizeof(fd_path), "%s/%s", fd_dir, entry->d_name);
        ssize_t len = readlink(fd_path, path, sizeof(path) - 1);
        if (len) {
            path[len] = '\0';
            fd_info.path = path;
        } else {
            fd_info.path = StringPrintf("??? (%s)", strerror(errno));
        }

        if (__builtin_available(android 29, *)) {
            const uint64_t tag = android_fdsan_get_owner_tag(fd);
            fd_info.tag = android_fdsan_get_tag_value(tag);
            if (fd_info.tag) {
                fd_info.owner = android_fdsan_get_tag_type(tag);
            }
        }

        open_fds.emplace_back(fd_info);
    }

    closedir(dir);

    if (!open_fds.empty()) {
        LOG_FISHNET("");
        LOG_FISHNET("open files:");
        if (__builtin_available(android 29, *)) {
            for (const FD &fd: open_fds) {
                if (!fd.owner.empty()) {
                    LOG_FISHNET("    fd %d: %s (owned by %s 0x%" PRIx64 ")",
                                fd.fd, fd.path.c_str(), fd.owner.c_str(), fd.tag);
                } else {
                    LOG_FISHNET("    fd %d: %s (unowned)", fd.fd, fd.path.c_str());
                }
            }
        } else {
            for (const FD &fd: open_fds) {
                LOG_FISHNET("    fd %d: %s", fd.fd, fd.path.c_str());
            }
        }
    }
}
