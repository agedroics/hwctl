find_package(Threads)

set(HWCTLD_SOURCES
        ${PROJECT_SOURCE_DIR}/daemon/hwctld.c
        ${PROJECT_SOURCE_DIR}/daemon/profile.c)

set(HWCTLD_HEADERS
        ${PROJECT_SOURCE_DIR}/daemon/profile.h)

set(HWCTLD_INCLUDES
        ${PROJECT_SOURCE_DIR}/daemon)

add_executable(hwctld ${HWCTLD_SOURCES} ${HWCTLD_HEADERS})

target_include_directories(hwctld
        PRIVATE ${HWCTL_INCLUDES}
        PRIVATE ${HWCTLD_INCLUDES}
        PRIVATE ${STR_UTIL_INCLUDES})

target_link_libraries(hwctld
        PRIVATE hwctl
        PRIVATE str_util
        PRIVATE ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(hwctld
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS hwctld
        RUNTIME DESTINATION bin
        COMPONENT daemon)