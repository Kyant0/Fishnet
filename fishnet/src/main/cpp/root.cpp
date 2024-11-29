#include "root.h"

#include <cstdlib>
#include <cstring>
#include <sys/fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "log.h"

// See https://github.com/vvb2060/MagiskDetector/blob/1570969c1070b372b63aa18c6ff30a3f897a1983/app/src/main/jni/vvb2060.c#L97
bool is_rooted() {
    char *path = getenv("PATH");
    const char *p = strtok(path, ":");
    char su_path[256];
    do {
        snprintf(su_path, sizeof(su_path), "%s/su", p);
        if (access(su_path, F_OK) == 0) {
            return true;
        }
    } while ((p = strtok(nullptr, ":")) != nullptr);
    return false;
}
