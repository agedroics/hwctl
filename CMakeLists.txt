cmake_minimum_required(VERSION 3.10)

project(hwctl C)

set(CMAKE_INSTALL_PREFIX "/usr")

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "-O2 -s -Wall -Wextra -Wpedantic")

add_subdirectory(${PROJECT_SOURCE_DIR}/daemon)

add_subdirectory(${PROJECT_SOURCE_DIR}/plugins)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(${PROJECT_SOURCE_DIR}/util)