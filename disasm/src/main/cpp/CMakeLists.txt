cmake_minimum_required(VERSION 3.31.0)

project("com.kyant.disasm")

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden -flto -fdata-sections -ffunction-sections -fomit-frame-pointer")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,-s -Wl,--pack-dyn-relocs=android -Wl,--build-id=none -Wl,-z,max-page-size=16384")

add_subdirectory("external/capstone" capstone)

add_library(${CMAKE_PROJECT_NAME} SHARED
        disasm.c)

target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE "external/capstone/include")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        --remove-section .comment
        --remove-section .note
        --strip-debug $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        capstone_static)
