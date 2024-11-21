#include "abort_message.h"

#include "android-base/logging.h"
#include "log.h"

static char *abort_message = nullptr;

void set_aborter() {
    android::base::SetAborter([](const char *abort_msg) {
        abort_message = strdup(abort_msg);
    });
}

void try_read_abort_message_from_logcat() {
    if (abort_message != nullptr) return;

    FILE *fp = popen("logcat -d -s libc", "r");
    if (fp == nullptr) return;

    char buffer[1024];
    const char *divider = ": ";

    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
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
    }

    pclose(fp);
}

void get_scudo_message_if_needed(unwindstack::ArchEnum &arch, unwindstack::Regs *regs,
                                 const std::vector<unwindstack::FrameData> &frames) {
    if (abort_message != nullptr) return;

    if (frames.size() < 2) return;

    static const char *scudo_die = "_ZN5scudo3dieEv";
    if (frames[1].function_name != scudo_die) return;

    uint64_t fp;
    switch (arch) {
        case unwindstack::ARCH_ARM:
            fp = ((const uint32_t *) regs->RawData())[7]; // r7
            break;
        case unwindstack::ARCH_ARM64:
            fp = ((const uint64_t *) regs->RawData())[29]; // x29
            break;
        case unwindstack::ARCH_X86:
            fp = ((const uint32_t *) regs->RawData())[5]; // ebp
            break;
        case unwindstack::ARCH_X86_64:
            fp = ((const uint64_t *) regs->RawData())[6]; // rbp
            break;
        case unwindstack::ARCH_RISCV64:
            fp = ((const uint64_t *) regs->RawData())[8]; // s0 (x8)
            break;
        default:
            return;
    }

    const char *scudo_message = (const char *) fp + 0x68;
    if (scudo_message[0] != '\0') {
        abort_message = strdup(scudo_message);
        size_t len = strlen(abort_message);
        if (len > 0 && abort_message[len - 1] == '\n') {
            abort_message[len - 1] = '\0';
        }
    }
}

void dump_abort_message() {
    if (abort_message) {
        LOG_FISHNET("Abort message: '%s'", abort_message);
    }
}
