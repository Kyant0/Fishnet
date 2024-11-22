#include "log.h"

#include <unistd.h>

static int log_fd = -1;

static std::string log_buffer{};

void set_log_fd(int fd) {
    log_fd = fd;
}

void write_log() {
    if (log_fd != -1) {
        ftruncate(log_fd, 0);
        write(log_fd, log_buffer.c_str(), log_buffer.size());
        close(log_fd);
        log_fd = -1;
    }
}

void StringAppendV(std::string *dst, const char *format, va_list ap) {
    // First try with a small fixed size buffer
    char space[256] __attribute__((__uninitialized__));

    // It's possible for methods that use a va_list to invalidate
    // the data in it upon use.  The fix is to make a copy
    // of the structure before using it and use that copy instead.
    va_list backup_ap;
    va_copy(backup_ap, ap);
    int result = vsnprintf(space, sizeof(space), format, backup_ap);
    va_end(backup_ap);

    if (result < static_cast<int>(sizeof(space))) {
        if (result >= 0) {
            // Normal case -- everything fit.
            dst->append(space, result);
            return;
        }

        // Just an error.
        return;
    }

    // Increase the buffer size to the size requested by vsnprintf,
    // plus one for the closing \0.
    int length = result + 1;
    char *buf = new char[length];

    // Restore the va_list before we use it again
    va_copy(backup_ap, ap);
    result = vsnprintf(buf, length, format, backup_ap);
    va_end(backup_ap);

    if (result >= 0 && result < length) {
        // It fit
        dst->append(buf, result);
    }
    delete[] buf;
}

std::string StringPrintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string result;
    StringAppendV(&result, fmt, ap);
    va_end(ap);
    return result;
}

void StringAppendF(std::string *dst, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    StringAppendV(dst, format, ap);
    va_end(ap);
}

void log_fishnet(bool linebreak, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // size_t next_start = log_buffer.size();
    StringAppendV(&log_buffer, strdup(fmt), args);
    va_end(args);
    // __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", log_buffer.c_str() + next_start);
    if (linebreak) {
        log_buffer.append("\n");
    }
}
