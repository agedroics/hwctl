find_package(Threads)

set(DAEMON_SOURCES
        ${PROJECT_SOURCE_DIR}/daemon/hwctld.c
        ${PROJECT_SOURCE_DIR}/daemon/profile.c)

set(DAEMON_HEADERS
        ${PROJECT_SOURCE_DIR}/daemon/profile.h)

set(DAEMON_INCLUDES
        ${PROJECT_SOURCE_DIR}/daemon)

add_executable(daemon ${DAEMON_SOURCES} ${DAEMON_HEADERS})

target_include_directories(daemon
        PRIVATE ${LIBRARY_INCLUDES}
        PRIVATE ${DAEMON_INCLUDES}
        PRIVATE ${STR_UTIL_INCLUDES}
        PRIVATE ${TIME_UTIL_INCLUDES})

target_link_libraries(daemon
        PRIVATE library
        PRIVATE str_util
        PRIVATE time_util
        PRIVATE ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(daemon
        PROPERTIES
        OUTPUT_NAME hwctld
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS daemon
        RUNTIME DESTINATION bin
        COMPONENT daemon)