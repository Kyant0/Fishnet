#include "log.h"

#include <algorithm>
#include <unistd.h>

#include "clock.h"

#define ENABLE_LOG 0

static std::string log_path{};

static ApkInfo apk_info{};

void set_log_path(const char *path) {
    log_path = path;
}

void set_apk_info(const ApkInfo &info) {
    apk_info = info;
}

const ApkInfo &get_apk_info() {
    return apk_info;
}

LogRecord start_recording(LogType type) {
    return {
            .type = type,
            .timestamp = get_timestamp(),
    };
}

static std::string log_type_to_string(LogType type) {
    switch (type) {
        case None:
            return "none";
        case Native:
            return "native";
        case Java:
            return "java";
        case ANR:
            return "anr";
    }
    return "unknown";
}

void write_log(const LogRecord &record) {
    if (log_path.empty()) {
        return;
    }
    std::string timestamp = record.timestamp;
    std::replace(timestamp.begin(), timestamp.end(), ' ', '_');
    std::replace(timestamp.begin(), timestamp.end(), ':', '-');
    std::string path = log_path + '/' + log_type_to_string(record.type) + '_' + timestamp + ".log";
    FILE *file = fopen(path.c_str(), "w");
    if (file == nullptr) {
        LOGE("Failed to open %s, %s", path.c_str(), strerror(errno));
        return;
    }
    LOGE("Write log to %s", path.c_str());
    fwrite(record.content.c_str(), 1, record.content.size(), file);
    fclose(file);
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

    if (result < sizeof(space)) {
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
    const int length = result + 1;
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

void log_fishnet(LogRecord &record, bool linebreak, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
#if ENABLE_LOG
    const size_t next_start = record.buffer.size();
#endif
    StringAppendV(&record.content, strdup(fmt), args);
    va_end(args);
#if ENABLE_LOG
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", record.buffer.c_str() + next_start);
#endif
    if (linebreak) {
        record.content += '\n';
    }
}
