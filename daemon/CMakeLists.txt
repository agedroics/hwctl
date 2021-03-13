set(DAEMON_SOURCES
        ${DAEMON_SOURCE_DIR}/heap.c
        ${DAEMON_SOURCE_DIR}/hwctld.c
        ${DAEMON_SOURCE_DIR}/profile.c)

set(DAEMON_HEADERS
        ${DAEMON_SOURCE_DIR}/heap.h
        ${DAEMON_SOURCE_DIR}/profile.h)

set(DAEMON_INCLUDES
        ${DAEMON_SOURCE_DIR})

add_executable(daemon ${DAEMON_SOURCES} ${DAEMON_HEADERS})

target_include_directories(daemon
        PRIVATE ${LIBRARY_INCLUDES}
        PRIVATE ${DAEMON_INCLUDES}
        PRIVATE ${STR_UTIL_INCLUDES}
        PRIVATE ${TIME_UTIL_INCLUDES})

target_link_libraries(daemon
        PRIVATE library
        PRIVATE str_util
        PRIVATE time_util)

set_target_properties(daemon
        PROPERTIES
        OUTPUT_NAME hwctld
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS daemon
        RUNTIME DESTINATION bin
        COMPONENT daemon)