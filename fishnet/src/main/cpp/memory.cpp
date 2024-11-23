#include "memory.h"

#include <cinttypes>
#include <sys/mman.h>

#include "abi.h"
#include "log.h"

// Number of bytes per MTE granule.
constexpr size_t kTagGranuleSize = 16;

// Number of rows and columns to display in an MTE tag dump.
constexpr size_t kNumTagColumns = 16;
constexpr size_t kNumTagRows = 16;

#define MEMORY_BYTES_TO_DUMP 256
#define MEMORY_BYTES_PER_LINE 16
static_assert(MEMORY_BYTES_PER_LINE == kTagGranuleSize);

ssize_t dump_memory(void *out, size_t len, uint8_t *tags, size_t tags_len, uint64_t *addr,
                    unwindstack::Memory *memory) {
    // Align the address to the number of bytes per line to avoid confusing memory tag output if
    // memory is tagged and we start from a misaligned address. Start 32 bytes before the address.
    *addr &= ~(MEMORY_BYTES_PER_LINE - 1);
    if (*addr >= 4128) {
        *addr -= 32;
    }

    // We don't want the address tag to appear in the addresses in the memory dump.
    *addr = untag_address(*addr);

    // Don't bother if the address would overflow, taking tag bits into account. Note that
    // untag_address truncates to 32 bits on 32-bit platforms as a side effect of returning a
    // uintptr_t, so this also checks for 32-bit overflow.
    if (untag_address(*addr + MEMORY_BYTES_TO_DUMP - 1) < *addr) {
        return -1;
    }

    memset(out, 0, len);

    size_t bytes = memory->Read(*addr, (uint8_t *) out, len);
    if (bytes % sizeof(uintptr_t) != 0) {
        // This should never happen, but just in case.
        // ALOGE("Bytes read %zu, is not a multiple of %zu", bytes, sizeof(uintptr_t));
        bytes &= ~(sizeof(uintptr_t) - 1);
    }

    bool skip_2nd_read = false;
    if (bytes == 0) {
        // In this case, we might want to try another read at the beginning of
        // the next page only if it's within the amount of memory we would have
        // read.
        const size_t page_size = sysconf(_SC_PAGE_SIZE);
        const uint64_t next_page = (*addr + (page_size - 1)) & ~(page_size - 1);
        if (next_page == *addr || next_page >= *addr + len) {
            skip_2nd_read = true;
        }
        *addr = next_page;
    }

    if (bytes < len && !skip_2nd_read) {
        // Try to do one more read. This could happen if a read crosses a map,
        // but the maps do not have any break between them. Or it could happen
        // if reading from an unreadable map, but the read would cross back
        // into a readable map. Only requires one extra read because a map has
        // to contain at least one page, and the total number of bytes to dump
        // is smaller than a page.
        const size_t bytes2 = memory->Read(*addr + bytes, (uint8_t *) out + bytes, len - bytes);
        bytes += bytes2;
        if (bytes2 > 0 && bytes % sizeof(uintptr_t) != 0) {
            // This should never happen, but we'll try and continue any way.
            // ALOGE("Bytes after second read %zu, is not a multiple of %zu", bytes, sizeof(uintptr_t));
            bytes &= ~(sizeof(uintptr_t) - 1);
        }
    }

    // If we were unable to read anything, it probably means that the register doesn't contain a
    // valid pointer.
    if (bytes == 0) {
        return -1;
    }

    for (uint64_t tag_granule = 0; tag_granule < bytes / kTagGranuleSize; ++tag_granule) {
        const long tag = memory->ReadTag(*addr + kTagGranuleSize * tag_granule);
        if (tag_granule < tags_len) {
            tags[tag_granule] = tag >= 0 ? tag : 0;
        } else {
            // ALOGE("Insufficient space for tags");
        }
    }

    return (ssize_t) bytes;
}

void print_tag_dump(uint64_t fault_addr, unwindstack::ArchEnum arch,
                    std::shared_ptr<unwindstack::Memory> &process_memory) {
    if (arch != unwindstack::ARCH_ARM64) {
        return;
    }

    /// FUNCTION_START: dump_tags_around_fault_addr

    fault_addr = untag_address(fault_addr);
    constexpr size_t kNumGranules = kNumTagRows * kNumTagColumns;
    constexpr size_t kBytesToRead = kNumGranules * kTagGranuleSize;

    // If the low part of the tag dump would underflow to the high address space, it's probably not
    // a valid address for us to dump tags from.
    if (fault_addr < kBytesToRead / 2) return;

    constexpr uintptr_t kRowStartMask = ~(kNumTagColumns * kTagGranuleSize - 1);
    size_t start_address = (fault_addr & kRowStartMask) - kBytesToRead / 2;
    size_t granules_to_read = kNumGranules;

    // Attempt to read the first tag. If reading fails, this likely indicates the
    // lowest touched page is inaccessible or not marked with PROT_MTE.
    // Fast-forward over pages until one has tags, or we exhaust the search range.
    while (process_memory->ReadTag(start_address) < 0) {
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        size_t bytes_to_next_page = page_size - (start_address % page_size);
        if (bytes_to_next_page >= granules_to_read * kTagGranuleSize) return;
        start_address += bytes_to_next_page;
        granules_to_read -= bytes_to_next_page / kTagGranuleSize;
    }
    const uintptr_t begin_address = start_address;

    std::string mte_tags{};

    for (size_t i = 0; i < granules_to_read; ++i) {
        const long tag = process_memory->ReadTag(start_address + i * kTagGranuleSize);
        if (tag < 0) break;
        mte_tags.push_back((uint8_t) tag);
    }

    if (mte_tags.empty()) {
        return;
    }

    /// FUNCTION_END: dump_tags_around_fault_addr

    const std::string &tags = mte_tags;

    LOG_FISHNET("");
    LOG_FISHNET("Memory tags around the fault address (0x%" PRIx64 "), one tag per %zu bytes:",
                fault_addr, kTagGranuleSize);

    size_t tag_index = 0;
    const size_t num_tags = tags.length();
    const uintptr_t fault_granule = untag_address(fault_addr) & ~(kTagGranuleSize - 1);
    for (size_t row = 0; tag_index < num_tags; ++row) {
        const uintptr_t row_addr = (begin_address + row * kNumTagColumns * kTagGranuleSize) & kRowStartMask;
        std::string row_contents;
        bool row_has_fault = false;

        for (size_t column = 0; column < kNumTagColumns; ++column) {
            uintptr_t granule_addr = row_addr + column * kTagGranuleSize;
            if (granule_addr < begin_address ||
                granule_addr >= begin_address + num_tags * kTagGranuleSize) {
                row_contents += " . ";
            } else if (granule_addr == fault_granule) {
                row_contents += StringPrintf("[%1hhx]", tags[tag_index++]);
                row_has_fault = true;
            } else {
                row_contents += StringPrintf(" %1hhx ", tags[tag_index++]);
            }
        }

        if (row_contents.back() == ' ') row_contents.pop_back();

        if (row_has_fault) {
            LOG_FISHNET("    =>0x%" PRIxPTR ":%s", row_addr, row_contents.c_str());
        } else {
            LOG_FISHNET("      0x%" PRIxPTR ":%s", row_addr, row_contents.c_str());
        }
    }
}

void print_thread_memory_dump(int word_size, const std::unique_ptr<unwindstack::Regs> &regs, unwindstack::Maps *maps,
                              unwindstack::Memory *memory) {
    static constexpr size_t bytes_per_line = 16;
    static_assert(bytes_per_line == kTagGranuleSize);

    regs->IterateRegisters([&word_size, maps, memory](const char *name, uint64_t value) {
        const std::shared_ptr<unwindstack::MapInfo> map_info = maps->Find(untag_address(value));
        std::string mapping_name;
        if (map_info) {
            mapping_name = map_info->name();
        }

        constexpr size_t kNumBytesAroundRegister = 256;
        constexpr size_t kNumTagsAroundRegister = kNumBytesAroundRegister / kTagGranuleSize;
        char buf[kNumBytesAroundRegister];
        uint8_t tags[kNumTagsAroundRegister];
        const ssize_t memory_size =
                dump_memory(buf, sizeof(buf), tags, sizeof(tags), &value, memory);
        if (memory_size == -1) {
            return;
        }

        bool has_tags = false;
#if defined(__aarch64__)
        for (unsigned char tag: tags) {
            if (tag != 0) {
                has_tags = true;
                break;
            }
        }
#endif  // defined(__aarch64__)

        LOG_FISHNET("");
        if (mapping_name.empty()) {
            LOG_FISHNET("memory near %s:", name);
        } else {
            LOG_FISHNET("memory near %s (%s):", name, mapping_name.c_str());
        }
        const uint64_t addr = value;
        for (size_t offset = 0; offset < memory_size; offset += bytes_per_line) {
            uint64_t tagged_addr = addr;
            if (has_tags && kNumTagsAroundRegister > offset / kTagGranuleSize) {
                tagged_addr |= (uint64_t) tags[offset / kTagGranuleSize] << 56;
            }
            std::string line = StringPrintf("    %0*" PRIx64, word_size * 2, tagged_addr + offset);

            const size_t bytes = std::min(bytes_per_line, memory_size - offset);
            for (size_t i = 0; i < bytes; i += word_size) {
                uint64_t word = 0;

                // Assumes little-endian, but what doesn't?
                memcpy(&word, buf + offset + i, word_size);

                StringAppendF(&line, " %0*" PRIx64, word_size * 2, word);
            }

            char ascii[bytes_per_line + 1];

            memset(ascii, '.', sizeof(ascii));
            ascii[bytes_per_line] = '\0';

            for (size_t i = 0; i < bytes; ++i) {
                const uint8_t byte = buf[offset + i];
                if (byte >= 0x20 && byte < 0x7f) {
                    ascii[i] = byte;
                }
            }

            LOG_FISHNET("%s  %s", line.c_str(), ascii);
        }
    });
}

void print_memory_maps(uint64_t fault_addr, int word_size, unwindstack::Maps *maps,
                       std::shared_ptr<unwindstack::Memory> &process_memory) {
    const auto format_pointer = [word_size](uint64_t ptr) -> std::string {
        if (word_size == 8) {
            const uint64_t top = ptr >> 32;
            const uint64_t bottom = ptr & 0xFFFFFFFF;
            return StringPrintf("%08" PRIx64 "'%08" PRIx64, top, bottom);
        }

        return StringPrintf("%0*" PRIx64, word_size * 2, ptr);
    };

    std::string memory_map_header =
            StringPrintf("memory map (%zu %s):", maps->Total(), maps->Total() == 1 ? "entry" : "entries");

    const bool has_fault_address = fault_addr != 0;
    const uint64_t fault_address = untag_address(fault_addr);
    bool preamble_printed = false;
    bool printed_fault_address_marker = false;

    for (const auto &map_info: *maps) {
        const uint64_t begin_address = map_info->start();
        const uint64_t end_address = map_info->end();
        const uint64_t offset = map_info->offset();

        const bool readable = map_info->flags() & PROT_READ;
        const bool writable = map_info->flags() & PROT_WRITE;
        const bool executable = map_info->flags() & PROT_EXEC;

        const std::string mapping_name = map_info->name();

        const std::string build_id = map_info->GetPrintableBuildID();

        const uint64_t load_bias = map_info->GetLoadBias(process_memory);

        if (!preamble_printed) {
            preamble_printed = true;
            if (has_fault_address) {
                if (fault_address < begin_address) {
                    memory_map_header +=
                            StringPrintf("\n--->Fault address falls at %s before any mapped regions",
                                         format_pointer(fault_address).c_str());
                    printed_fault_address_marker = true;
                } else {
                    memory_map_header += " (fault address prefixed with --->)";
                }
            }
            LOG_FISHNET("%s", memory_map_header.c_str());
        }

        std::string line = "    ";
        if (has_fault_address && !printed_fault_address_marker) {
            if (fault_address < begin_address) {
                printed_fault_address_marker = true;
                LOG_FISHNET("--->Fault address falls at %s between mapped regions",
                            format_pointer(fault_address).c_str());
            } else if (fault_address >= begin_address && fault_address < end_address) {
                printed_fault_address_marker = true;
                line = "--->";
            }
        }
        StringAppendF(&line, "%s-%s", format_pointer(begin_address).c_str(),
                      format_pointer(end_address - 1).c_str());
        StringAppendF(&line, " %s%s%s", readable ? "r" : "-", writable ? "w" : "-",
                      executable ? "x" : "-");
        StringAppendF(&line, "  %8" PRIx64 "  %8" PRIx64, offset,
                      end_address - begin_address);

        if (!mapping_name.empty()) {
            StringAppendF(&line, "  %s", mapping_name.c_str());

            if (!build_id.empty()) {
                StringAppendF(&line, " (BuildId: %s)", build_id.c_str());
            }

            if (load_bias != 0) {
                StringAppendF(&line, " (load bias 0x%" PRIx64 ")", load_bias);
            }
        }

        LOG_FISHNET("%s", line.c_str());
    }

    if (has_fault_address && !printed_fault_address_marker) {
        LOG_FISHNET("--->Fault address falls at %s after any mapped regions",
                    format_pointer(fault_address).c_str());
    }
}
