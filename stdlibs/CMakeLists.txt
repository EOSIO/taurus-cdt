cmake_minimum_required(VERSION 3.16)

project(stdlibs)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
  message(STATUS "Using ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()
set(CMAKE_INSTALL_MESSAGE NEVER)

add_subdirectory(libc)
add_subdirectory(libc++)

configure_file(${musl_SOURCE_DIR}/COPYRIGHT ${CMAKE_INSTALL_PREFIX}/licenses/musl.license COPYONLY)
