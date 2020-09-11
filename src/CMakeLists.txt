include(GenerateExportHeader)

set(HWCTL_SOURCES
        ${PROJECT_SOURCE_DIR}/src/device.c
        ${PROJECT_SOURCE_DIR}/src/loader.c
        ${PROJECT_SOURCE_DIR}/src/vec.c)

set(HWCTL_HEADERS
        ${PROJECT_SOURCE_DIR}/include/hwctl/device.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/loader.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/plugin.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/vec.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/export.h)

set(HWCTL_INCLUDES ${PROJECT_SOURCE_DIR}/include)

add_library(hwctl SHARED ${HWCTL_SOURCES} ${HWCTL_HEADERS})

GENERATE_EXPORT_HEADER(hwctl
        EXPORT_FILE_NAME ${PROJECT_SOURCE_DIR}/include/hwctl/export.h)

target_include_directories(hwctl
        PUBLIC ${HWCTL_INCLUDES}
        PRIVATE ${STR_UTIL_INCLUDES})

target_link_libraries(hwctl
        PRIVATE str_util
        PRIVATE ${CMAKE_DL_LIBS})

set_target_properties(hwctl
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS hwctl
        ARCHIVE DESTINATION lib
        COMPONENT import_library
        LIBRARY DESTINATION lib
        COMPONENT library
        RUNTIME DESTINATION bin
        COMPONENT library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include
        COMPONENT headers)