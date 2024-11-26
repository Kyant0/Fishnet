#include "clock.h"

std::string get_timestamp() {
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);

    tm tm{};
    localtime_r(&ts.tv_sec, &tm);

    // 2024-11-19 21:55:28.269116356+0800
    char buf[35];
    char *s = buf;
    size_t sz = sizeof(buf), n;
    n = strftime(s, sz, "%F %H:%M", &tm), s += n, sz -= n;
    n = snprintf(s, sz, ":%02d.%09ld", tm.tm_sec, ts.tv_nsec), s += n, sz -= n;
    strftime(s, sz, "%z", &tm);
    return buf;
}
