#ifndef FISHNET_DURATION_H
#define FISHNET_DURATION_H

#include <string>

static std::string seconds_to_human_readable_time(uint64_t total_seconds) {
    uint64_t hours = total_seconds / 3600;
    uint64_t minutes = (total_seconds % 3600) / 60;
    uint64_t seconds = total_seconds % 60;

    std::string result;

    if (hours > 0) {
        result += (hours < 10 ? "0" : "") + std::to_string(hours) + ":";
    }

    result += (minutes < 10 ? "0" : "") + std::to_string(minutes) + ".";

    result += (seconds < 10 ? "0" : "") + std::to_string(seconds);

    return result;
}

#endif //FISHNET_DURATION_H
