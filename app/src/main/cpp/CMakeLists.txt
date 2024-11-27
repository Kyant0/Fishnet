cmake_minimum_required(VERSION 3.31.0)

project(com.kyant.fishnet.demo)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden -flto -fdata-sections -ffunction-sections -fomit-frame-pointer")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,-s -Wl,--build-id=none -Wl,-z,max-page-size=16384")

add_library(fishnet SHARED IMPORTED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FISHNET_BUILD_TYPE "debug")
else ()
    set(FISHNET_BUILD_TYPE "release")
endif ()

set_target_properties(fishnet PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../../fishnet/build/intermediates/merged_native_libs/${FISHNET_BUILD_TYPE}/merge${FISHNET_BUILD_TYPE}NativeLibs/out/lib/${ANDROID_ABI}/libcom.kyant.fishnet.so"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/../../../../fishnet/src/main/cpp/include")

add_library(${CMAKE_PROJECT_NAME} SHARED
        lib.c)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall
        -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        --remove-section .comment
        --remove-section .note
        --strip-debug $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        android
        log
        fishnet)
