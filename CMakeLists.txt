cmake_minimum_required(VERSION 2.8.3)
project(ping_clone)

include_directories(../shared)

set(TARGET "ping")

file(GLOB_RECURSE SRC_NAMES src/*.c)

add_executable(${TARGET} ${SRC_NAMES})


# VERBATIM
#   arguments to the commands will be escaped properly for the build tool
#
# NOTE: when VERBATIM is not given the behavior is platform specific because
# there is no protection of tool-specific special characters.

add_custom_command(
  TARGET ${TARGET}
  POST_BUILD
  COMMAND sudo chown root ${TARGET}
  # set user or group ID on execution (+s) for all (a)
  COMMAND sudo chmod a+s  ${TARGET}
  COMMENT "Grant root permissions to use RAW sockets"
  VERBATIM
)

# tells the linker to link the dl library, which is located at /usr/lib/libdl.so (-ldl)
# Historically, functions in libdl provided for dynamic linking support. This functionality now resides in libc(3LIB).
# target_link_libraries(${TARGET} dl)
