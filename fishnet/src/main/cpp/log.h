#ifndef FISHNET_LOG_H
#define FISHNET_LOG_H

#include <android/log.h>
#include <string>

#define LOG_TAG "Fishnet"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOG_FISHNET(...) log_fishnet(record, true, __VA_ARGS__)

#define LOG_FISHNET_LN(...) log_fishnet(record, false, __VA_ARGS__)

#define FISHNET_RECORD(type) LogRecord record = start_recording(type)

#define FISHNET_WRITE() write_log(record)

struct ApkInfo {
    const char *package_name;
    const char *version_name;
    uint64_t version_code;
    const char *cert;
};

enum LogType {
    None,
    Native,
    Java,
    ANR,
};

struct LogRecord {
    LogType type;
    std::string timestamp;
    std::string content;
};

void set_log_path(const char *path);

void set_apk_info(const ApkInfo &info);

const ApkInfo &get_apk_info();

LogRecord start_recording(LogType type);

void write_log(const LogRecord &record);

__attribute__((__format__(printf, 1, 2)))
std::string StringPrintf(const char *fmt, ...);

__attribute__((__format__(printf, 2, 3)))
void StringAppendF(std::string *dst, const char *format, ...);

__attribute__((__format__(printf, 3, 4)))
void log_fishnet(LogRecord &record, bool linebreak, const char *fmt, ...);

#endif //FISHNET_LOG_H
