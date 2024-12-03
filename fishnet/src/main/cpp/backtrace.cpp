#include "backtrace.h"

void print_backtrace(LogRecord &record, unwindstack::ArchEnum arch,
                     const std::vector<unwindstack::FrameData> &frames) {
    LOG_FISHNET("");
    LOG_FISHNET("backtrace:");
    for (const auto &frame: frames) {
        LOG_FISHNET("  %s", unwindstack::Unwinder::FormatFrame(arch, frame, false).c_str());
    }
}
