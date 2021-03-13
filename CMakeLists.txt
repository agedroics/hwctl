cmake_minimum_required(VERSION 3.10)

project(hwctl C)

set(CMAKE_INSTALL_PREFIX "/usr")

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "-O2 -s -Wall -Wextra -Wpedantic")

set(DAEMON_SOURCE_DIR ${PROJECT_SOURCE_DIR}/daemon)

set(HEADERS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/include)

set(PLUGINS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/plugins)

set(LIBRARY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(UTILS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/utils)

add_subdirectory(${DAEMON_SOURCE_DIR})

add_subdirectory(${HEADERS_SOURCE_DIR})

add_subdirectory(${PLUGINS_SOURCE_DIR})

add_subdirectory(${LIBRARY_SOURCE_DIR})

add_subdirectory(${UTILS_SOURCE_DIR})