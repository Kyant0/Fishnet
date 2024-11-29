#include "abort_message.h"

#include "android-base/logging.h"

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
    constexpr char divider[] = ": ";

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

void get_scudo_message_if_needed(const unwindstack::ArchEnum &arch, const std::unique_ptr<unwindstack::Regs> &regs,
                                 const std::vector<unwindstack::FrameData> &frames) {
    if (abort_message != nullptr) return;

    if (frames.size() < 2) return;

    constexpr char scudo_die[] = "_ZN5scudo3dieEv";
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

    const char *scudo_block = (const char *) fp;
    size_t offset = 0x68;
    constexpr size_t max_offset = 0x100;
    constexpr char scudo_error[] = "Scudo ERROR";

    // Scudo ERROR: corrupted chunk header at address 0x73485fd60910
    while (offset <= max_offset) {
        if (memcmp(&scudo_block[offset], scudo_error, 11) == 0) {
            abort_message = strdup(&scudo_block[offset]);
            const size_t len = strlen(abort_message);
            if (len > 0 && abort_message[len - 1] == '\n') {
                abort_message[len - 1] = '\0';
            }
            break;
        }
        offset++;
    }
}

void dump_abort_message(LogRecord &record) {
    if (abort_message) {
        LOG_FISHNET("Abort message: '%s'", abort_message);
    }
}
