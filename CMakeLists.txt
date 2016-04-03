cmake_minimum_required(VERSION 2.8.3)
project(ping_clone)

include_directories(../shared)

set(TARGET "ping")

file(GLOB_RECURSE SRC_NAMES src/*.c)

add_executable(${TARGET} ${SRC_NAMES})
# tells the linker to link the dl library, which is located at /usr/lib/libdl.so (-ldl)
# Historically, functions in libdl provided for dynamic linking support. This functionality now resides in libc(3LIB).
# target_link_libraries(${TARGET} dl)


