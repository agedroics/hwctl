include(GenerateExportHeader)

set(LIBRARY_SOURCES
        ${LIBRARY_SOURCE_DIR}/device.c
        ${LIBRARY_SOURCE_DIR}/loader.c
        ${LIBRARY_SOURCE_DIR}/vec.c)

set(LIBRARY_HEADERS
        ${HEADERS_SOURCE_DIR}/hwctl/device.h
        ${HEADERS_SOURCE_DIR}/hwctl/loader.h
        ${HEADERS_SOURCE_DIR}/hwctl/plugin.h
        ${HEADERS_SOURCE_DIR}/hwctl/vec.h
        ${HEADERS_SOURCE_DIR}/hwctl/export.h)

set(LIBRARY_INCLUDES ${HEADERS_SOURCE_DIR})

add_library(library SHARED ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

GENERATE_EXPORT_HEADER(library
        EXPORT_FILE_NAME ${HEADERS_SOURCE_DIR}/hwctl/export.h)

target_include_directories(library
        PUBLIC ${LIBRARY_INCLUDES}
        PRIVATE ${STR_UTIL_INCLUDES})

target_link_libraries(library
        PRIVATE str_util
        PRIVATE ${CMAKE_DL_LIBS})

set_target_properties(library
        PROPERTIES
        OUTPUT_NAME hwctl
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS library
        ARCHIVE DESTINATION lib
        COMPONENT import_library
        LIBRARY DESTINATION lib
        COMPONENT library
        RUNTIME DESTINATION bin
        COMPONENT library)