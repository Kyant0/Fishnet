#include "abort_message.h"

#include "android-base/logging.h"
#include "log.h"

static char *abort_message = nullptr;

void set_aborter() {
    android::base::SetAborter([](const char *abort_msg) {
        abort_message = strdup(abort_msg);
    });
}

void try_read_libc_abort_logs() {
    if (abort_message != nullptr) return;

    FILE *fp = popen("logcat -d -s libc", "r");
    if (fp == nullptr) return;

    char buffer[1024];
    const char *abort_level = " F ";
    const char *divider = ": ";

    int abort_found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        if (strstr(buffer, abort_level)) {
            // remove line break
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            // 11-18 20:46:27.497 31300 31300 F libc    : FORTIFY: memcpy: prevented 10-byte write into 8-byte buffer
            const char *pos = strstr(buffer, divider);
            if (pos) {
                abort_message = strdup(pos + 2);
            } else {
                abort_message = strdup(buffer);
            }
            abort_found = 1;
        } else if (abort_found) {
            break;
        }
    }

    pclose(fp);
}

void dump_abort_message() {
    if (abort_message) {
        LOG_FISHNET("Abort message: '%s'", abort_message);
    }
}
