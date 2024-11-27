#ifndef FISHNET_LOG_H
#define FISHNET_LOG_H

#include <android/log.h>
#include <string>

#define LOG_TAG "Fishnet"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOG_FISHNET(...) log_fishnet(true, __VA_ARGS__)

#define LOG_FISHNET_LN(...) log_fishnet(false, __VA_ARGS__)

struct ApkInfo {
    const char *package_name;
    const char *version_name;
    uint64_t version_code;
    const char *cert;
};

void set_log_fd(int fd);

void write_log_to_fd();

void close_log_fd();

void set_apk_info(const ApkInfo &info);

const ApkInfo &get_apk_info();

__attribute__((__format__(printf, 1, 2)))
std::string StringPrintf(const char *fmt, ...);

__attribute__((__format__(printf, 2, 3)))
void StringAppendF(std::string *dst, const char *format, ...);

__attribute__((__format__(printf, 2, 3)))
void log_fishnet(bool linebreak, const char *fmt, ...);

#endif //FISHNET_LOG_H
