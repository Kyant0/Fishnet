cmake_minimum_required(VERSION 4.0.2)

project(fishnet_demo)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -fvisibility=hidden -fdata-sections -ffunction-sections -fomit-frame-pointer")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,-s -Wl,--pack-dyn-relocs=relr -Wl,--build-id=none")
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_library(${CMAKE_PROJECT_NAME} SHARED
        lib.c)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        --remove-section .comment
        --remove-section .note
        --strip-debug $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        android)
