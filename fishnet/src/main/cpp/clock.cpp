#include "clock.h"

#include <cstdio>
#include <ctime>

std::string get_timestamp() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    tm tm;
    localtime_r(&ts.tv_sec, &tm);

    char buf[35];
    char *s = buf;
    size_t sz = sizeof(buf), n;
    n = strftime(s, sz, "%F %H:%M", &tm), s += n, sz -= n;
    n = snprintf(s, sz, ":%02d.%09ld", tm.tm_sec, ts.tv_nsec), s += n, sz -= n;
    n = strftime(s, sz, "%z", &tm), s += n, sz -= n;
    return buf;
}
