set(HWCTLD_SOURCES
        ${PROJECT_SOURCE_DIR}/daemon/hwctld.c)

add_executable(hwctld ${HWCTLD_SOURCES})

target_include_directories(hwctld
        PRIVATE ${HWCTL_INCLUDES})

target_link_libraries(hwctld
        PRIVATE hwctl)

set_target_properties(hwctld
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS hwctld
        RUNTIME DESTINATION bin
        COMPONENT daemon)