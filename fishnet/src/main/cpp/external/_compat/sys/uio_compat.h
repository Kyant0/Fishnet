#ifndef UIO_COMPAT_H
#define UIO_COMPAT_H

#include <fcntl.h>
#include <stdio.h>
#include <sys/uio.h>
#include <unistd.h>

static ssize_t process_vm_readv_compat(pid_t pid,
                                         const struct iovec *local_iov, unsigned long liovcnt,
                                         const struct iovec *remote_iov, unsigned long riovcnt,
                                         unsigned long flags) {
    if (__builtin_available(android 23, *)) {
        return process_vm_readv(pid, local_iov, liovcnt, remote_iov, riovcnt, flags);
    } else {
        char mem_path[18];
        snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);

        const int mem_fd = open(mem_path, O_RDONLY);
        if (mem_fd < 0) {
            return -1;
        }

        ssize_t total_read = 0;

        for (int i = 0; i < riovcnt; ++i) {
            const struct iovec *remote = &remote_iov[i];
            void *remote_addr = remote->iov_base;
            size_t remote_len = remote->iov_len;

            for (int j = 0; j < liovcnt; ++j) {
                const struct iovec *local = &local_iov[j];
                void *local_addr = local->iov_base;
                const size_t local_len = local->iov_len;

                const size_t to_read = (remote_len < local_len) ? remote_len : local_len;
                const ssize_t bytes_read =
                        pread(mem_fd, local_addr, to_read, (off_t) remote_addr);
                if (bytes_read < 0) {
                    close(mem_fd);
                    return -1;
                }

                total_read += bytes_read;
                remote_addr = (char *) remote_addr + bytes_read;
                remote_len -= bytes_read;
                if (remote_len == 0) {
                    break;
                }
            }
        }

        close(mem_fd);
        return total_read;
    }
}

#endif //UIO_COMPAT_H
