cmake_minimum_required(VERSION 3.31.0)

project(com.kyant.fishnet)

enable_language(C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden -flto -fdata-sections -ffunction-sections -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden -flto -fdata-sections -ffunction-sections -fomit-frame-pointer")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,-s -Wl,--build-id=none -Wl,-z,max-page-size=16384")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")


set(LIBC_SRC "external/_libc")
set(ASYNCSAFE_SRC "external/_libc/async_safe")
set(ANDROIDBASE_SRC "external/libbase")
set(LIBLOG_SRC "external/logging/liblog")
set(ARTBASE_SRC "external/art/libartbase")
set(ARTPALETTE_SRC "external/art/libartpalette")
set(DEXFILE_SRC "external/art/libdexfile")
set(PROCINFO_SRC "external/libprocinfo")
set(ZIPARCHIVE_SRC "external/libziparchive")
set(ZSTD_SRC "external/zstd")
set(LZMA_SRC "external/lzma")
set(LIBIBERTY_SRC "external/libiberty")
set(UNWINDSTACK_SRC "external/unwinding/libunwindstack")


if (ANDROID_ABI STREQUAL "x86")
    set(UNWINDSTACK_ARCH "x86")
elseif (ANDROID_ABI STREQUAL "x86_64")
    set(UNWINDSTACK_ARCH "x86_64")
elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
    set(UNWINDSTACK_ARCH "arm")
elseif (ANDROID_ABI STREQUAL "arm64-v8a")
    set(UNWINDSTACK_ARCH "arm64")
elseif (ANDROID_ABI STREQUAL "riscv64")
    set(UNWINDSTACK_ARCH "riscv64")
else ()
    message(FATAL_ERROR "Unsupported ANDROID_ABI: ${ANDROID_ABI}")
endif ()

if (UNWINDSTACK_ARCH STREQUAL "x86")
    set(UNWINDSTACK_ARCH_SRC "${UNWINDSTACK_SRC}/AsmGetRegsX86.S")
elseif (UNWINDSTACK_ARCH STREQUAL "x86_64")
    set(UNWINDSTACK_ARCH_SRC "${UNWINDSTACK_SRC}/AsmGetRegsX86_64.S")
else ()
    set(UNWINDSTACK_ARCH_SRC "")
endif ()

if (UNWINDSTACK_ARCH STREQUAL "x86")
    list(APPEND ART_COMPILE_C_FLAGS "-DART_ENABLE_CODEGEN_x86")
elseif (UNWINDSTACK_ARCH STREQUAL "x86_64")
    list(APPEND ART_COMPILE_C_FLAGS "-DART_ENABLE_CODEGEN_x86_64" "-DART_ENABLE_CODEGEN_x86")
elseif (UNWINDSTACK_ARCH STREQUAL "arm")
    list(APPEND ART_COMPILE_C_FLAGS "-DART_ENABLE_CODEGEN_arm")
elseif (UNWINDSTACK_ARCH STREQUAL "arm64")
    list(APPEND ART_COMPILE_C_FLAGS "-DART_ENABLE_CODEGEN_arm64" "-DART_ENABLE_CODEGEN_arm")
endif ()


set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_CONTRIB OFF)
set(ZSTD_BUILD_TESTS OFF)

set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_COMPRESSION OFF)
set(ZSTD_BUILD_DECOMPRESSION ON)
set(ZSTD_BUILD_DICTBUILDER OFF)
set(ZSTD_BUILD_DEPRECATED OFF)

add_subdirectory("${ZSTD_SRC}/build/cmake" zstd)


add_library(liblog STATIC
        "${LIBLOG_SRC}/include/log/log.h")

set_target_properties(liblog PROPERTIES
        LINKER_LANGUAGE C)

target_include_directories(liblog PUBLIC
        "${LIBLOG_SRC}/include")


add_library(bionic_libc STATIC
        "${LIBC_SRC}/platform/bionic/mte.h")

set_target_properties(bionic_libc PROPERTIES
        LINKER_LANGUAGE CXX)

target_include_directories(bionic_libc PUBLIC
        "${LIBC_SRC}"
        "${LIBC_SRC}/platform")


add_library(async_safe STATIC
        "${ASYNCSAFE_SRC}/async_safe_log.cpp")

target_include_directories(async_safe PUBLIC
        "${ASYNCSAFE_SRC}/include")

target_link_libraries(async_safe
        log bionic_libc)


add_definitions(-DART_BASE_ADDRESS=0x70000000)

add_library(androidbase STATIC
        "${ANDROIDBASE_SRC}/file.cpp"
        "${ANDROIDBASE_SRC}/logging.cpp"
        "${ANDROIDBASE_SRC}/stringprintf.cpp"
        "${ANDROIDBASE_SRC}/strings.cpp"
        "${ANDROIDBASE_SRC}/threads.cpp")

target_include_directories(androidbase PUBLIC
        "${ANDROIDBASE_SRC}/include")

target_compile_options(androidbase PRIVATE
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)


add_library(procinfo STATIC
        "${PROCINFO_SRC}/process.cpp")

target_include_directories(procinfo PUBLIC
        "${PROCINFO_SRC}/include")

target_compile_options(procinfo PRIVATE
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

target_link_libraries(procinfo
        androidbase)


add_library(ziparchive STATIC
        "${ZIPARCHIVE_SRC}/zip_archive.cc"
        "${ZIPARCHIVE_SRC}/zip_archive_stream_entry.cc"
        "${ZIPARCHIVE_SRC}/zip_cd_entry_map.cc"
        "${ZIPARCHIVE_SRC}/zip_error.cpp")

target_include_directories(ziparchive
        PUBLIC "${ZIPARCHIVE_SRC}/include"
        PRIVATE "${ZIPARCHIVE_SRC}/incfs_support/include")

target_compile_definitions(ziparchive PRIVATE
        ZLIB_CONST=const)

target_compile_options(ziparchive PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-field-initializers -Wconversion -Wno-sign-conversion -Wold-style-cast>
        $<$<COMPILE_LANGUAGE:C>:-DZLIB_CONST -Werror -D_FILE_OFFSET_BITS=64 -DINCFS_SUPPORT_DISABLED=1>
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

target_link_libraries(ziparchive
        androidbase z liblog)


add_library(dexfile STATIC
        "${DEXFILE_SRC}/dex/compact_dex_file.cc"
        "${DEXFILE_SRC}/dex/compact_offset_table.cc"
        "${DEXFILE_SRC}/dex/descriptors_names.cc"
        "${DEXFILE_SRC}/dex/dex_file.cc"
        "${DEXFILE_SRC}/dex/dex_file_loader.cc"
        "${DEXFILE_SRC}/dex/dex_file_verifier.cc"
        "${DEXFILE_SRC}/dex/modifiers.cc"
        "${DEXFILE_SRC}/dex/primitive.cc"
        "${DEXFILE_SRC}/dex/signature.cc"
        "${DEXFILE_SRC}/dex/standard_dex_file.cc"
        "${DEXFILE_SRC}/dex/utf.cc"
        "${DEXFILE_SRC}/external/dex_file_ext.cc"
        "${DEXFILE_SRC}/external/dex_file_supp.cc"
        "${ARTBASE_SRC}/base/unix_file/fd_file.cc"
        "${ARTBASE_SRC}/base/logging.cc"
        "${ARTBASE_SRC}/base/utils.cc"
        "${ARTBASE_SRC}/base/file_magic.cc"
        "${ARTBASE_SRC}/base/mem_map.cc"
        "${ARTBASE_SRC}/base/mem_map_unix.cc"
        "${ARTBASE_SRC}/base/zip_archive.cc")

target_include_directories(dexfile BEFORE
        PUBLIC "${DEXFILE_SRC}"
        PUBLIC "${DEXFILE_SRC}/external/include"
        PUBLIC "${ARTPALETTE_SRC}/include"
        PRIVATE "${ARTBASE_SRC}")

target_compile_definitions(dexfile PUBLIC
        -DART_TARGET -DSTATIC_LIB=1 -DART_TARGET_ANDROID)

target_compile_options(dexfile PRIVATE
        ${ART_COMPILE_C_FLAGS} -Wno-deprecated-declarations
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

target_link_libraries(dexfile
        androidbase ziparchive)


add_library(lzma STATIC
        "${LZMA_SRC}/C/7zAlloc.c"
        "${LZMA_SRC}/C/7zArcIn.c"
        "${LZMA_SRC}/C/7zBuf2.c"
        "${LZMA_SRC}/C/7zBuf.c"
        "${LZMA_SRC}/C/7zCrc.c"
        "${LZMA_SRC}/C/7zCrcOpt.c"
        "${LZMA_SRC}/C/7zDec.c"
        "${LZMA_SRC}/C/7zFile.c"
        "${LZMA_SRC}/C/7zStream.c"
        "${LZMA_SRC}/C/Aes.c"
        "${LZMA_SRC}/C/AesOpt.c"
        "${LZMA_SRC}/C/Alloc.c"
        "${LZMA_SRC}/C/Bcj2.c"
        "${LZMA_SRC}/C/Bra86.c"
        "${LZMA_SRC}/C/Bra.c"
        "${LZMA_SRC}/C/BraIA64.c"
        "${LZMA_SRC}/C/CpuArch.c"
        "${LZMA_SRC}/C/Delta.c"
        "${LZMA_SRC}/C/LzFind.c"
        "${LZMA_SRC}/C/Lzma2Dec.c"
        "${LZMA_SRC}/C/Lzma2Enc.c"
        "${LZMA_SRC}/C/Lzma86Dec.c"
        "${LZMA_SRC}/C/Lzma86Enc.c"
        "${LZMA_SRC}/C/LzmaDec.c"
        "${LZMA_SRC}/C/LzmaEnc.c"
        "${LZMA_SRC}/C/LzmaLib.c"
        "${LZMA_SRC}/C/Ppmd7.c"
        "${LZMA_SRC}/C/Ppmd7Dec.c"
        "${LZMA_SRC}/C/Ppmd7Enc.c"
        "${LZMA_SRC}/C/Sha256.c"
        "${LZMA_SRC}/C/Sort.c"
        "${LZMA_SRC}/C/Xz.c"
        "${LZMA_SRC}/C/XzCrc64.c"
        "${LZMA_SRC}/C/XzCrc64Opt.c"
        "${LZMA_SRC}/C/XzDec.c"
        "${LZMA_SRC}/C/XzEnc.c"
        "${LZMA_SRC}/C/XzIn.c")

target_include_directories(lzma PUBLIC "${LZMA_SRC}/C")

target_compile_options(lzma PRIVATE
        -D_Z7_ST -Wall -Werror -Wno-empty-body -Wno-enum-conversion -Wno-logical-op-parentheses -Wno-self-assign)


add_library(unwindstack STATIC
        "${UNWINDSTACK_SRC}/AndroidUnwinder.cpp"
        "${UNWINDSTACK_SRC}/ArmExidx.cpp"
        "${UNWINDSTACK_SRC}/Demangle.cpp"
        "${UNWINDSTACK_SRC}/DexFiles.cpp"
        "${UNWINDSTACK_SRC}/DwarfCfa.cpp"
        "${UNWINDSTACK_SRC}/DwarfEhFrameWithHdr.cpp"
        "${UNWINDSTACK_SRC}/DwarfMemory.cpp"
        "${UNWINDSTACK_SRC}/DwarfOp.cpp"
        "${UNWINDSTACK_SRC}/DwarfSection.cpp"
        "${UNWINDSTACK_SRC}/Elf.cpp"
        "${UNWINDSTACK_SRC}/ElfInterface.cpp"
        "${UNWINDSTACK_SRC}/ElfInterfaceArm.cpp"
        "${UNWINDSTACK_SRC}/Global.cpp"
        "${UNWINDSTACK_SRC}/JitDebug.cpp"
        "${UNWINDSTACK_SRC}/MapInfo.cpp"
        "${UNWINDSTACK_SRC}/Maps.cpp"
        "${UNWINDSTACK_SRC}/Memory.cpp"
        "${UNWINDSTACK_SRC}/MemoryMte.cpp"
        "${UNWINDSTACK_SRC}/MemoryXz.cpp"
        "${UNWINDSTACK_SRC}/Regs.cpp"
        "${UNWINDSTACK_SRC}/RegsArm.cpp"
        "${UNWINDSTACK_SRC}/RegsArm64.cpp"
        "${UNWINDSTACK_SRC}/RegsX86.cpp"
        "${UNWINDSTACK_SRC}/RegsX86_64.cpp"
        "${UNWINDSTACK_SRC}/RegsRiscv64.cpp"
        "${UNWINDSTACK_SRC}/Symbols.cpp"
        "${UNWINDSTACK_SRC}/ThreadEntry.cpp"
        "${UNWINDSTACK_SRC}/ThreadUnwinder.cpp"
        "${UNWINDSTACK_SRC}/Unwinder.cpp"
        "${UNWINDSTACK_ARCH_SRC}"
        "${UNWINDSTACK_SRC}/DexFile.cpp"
        "${UNWINDSTACK_SRC}/LogAndroid.cpp"
        "${LIBIBERTY_SRC}/cp-demangle.c"
        "${LIBIBERTY_SRC}/cplus-dem.c"
        "${LIBIBERTY_SRC}/d-demangle.c"
        "${LIBIBERTY_SRC}/rust-demangle.c"
        "${LIBIBERTY_SRC}/safe-ctype.c")

target_include_directories(unwindstack BEFORE
        PUBLIC "${UNWINDSTACK_SRC}/include"
        PRIVATE "platform"
        PRIVATE "${LIBIBERTY_SRC}"
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/_compat")

target_compile_options(unwindstack PRIVATE
        -Wno-deprecated-volatile -Wno-reorder-init-list
        -Wexit-time-destructors -fno-c++-static-destructors
        -DDEXFILE_SUPPORT
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

target_link_libraries(unwindstack
        lzma libzstd_static androidbase dexfile procinfo async_safe bionic_libc)


add_library(${CMAKE_PROJECT_NAME} SHARED
        fishnet.cpp
        abi.cpp
        human_readable.cpp
        log.cpp
        cause.cpp
        clock.cpp
        abort_message.cpp
        logcat.cpp
        command_line.cpp
        process.cpp
        thread.cpp
        tasks.cpp
        property.cpp
        registers.cpp
        backtrace.cpp
        memory.cpp
        fd.cpp
        root.cpp
        anr.cpp
        signal_handler.cpp)

target_include_directories(${CMAKE_PROJECT_NAME}
        PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        --remove-section .comment
        --remove-section .note
        --strip-debug $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        android
        log
        unwindstack)
