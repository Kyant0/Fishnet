#ifndef FISHNET_LOG_H
#define FISHNET_LOG_H

#include <android/log.h>
#include <string>

#define LOG_FISHNET(...) log_fishnet(__VA_ARGS__)

void set_log_fd(int fd);

void write_log();

__attribute__((__format__(printf, 1, 2)))
std::string StringPrintf(const char *fmt, ...);

__attribute__((__format__(printf, 2, 3)))
void StringAppendF(std::string *dst, const char *format, ...);

__attribute__((__format__(printf, 1, 2)))
void log_fishnet(const char *fmt, ...);

#endif //FISHNET_LOG_H
