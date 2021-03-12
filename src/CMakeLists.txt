include(GenerateExportHeader)

set(LIBRARY_SOURCES
        ${PROJECT_SOURCE_DIR}/src/device.c
        ${PROJECT_SOURCE_DIR}/src/loader.c
        ${PROJECT_SOURCE_DIR}/src/vec.c)

set(LIBRARY_HEADERS
        ${PROJECT_SOURCE_DIR}/include/hwctl/device.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/loader.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/plugin.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/vec.h
        ${PROJECT_SOURCE_DIR}/include/hwctl/export.h)

set(LIBRARY_INCLUDES ${PROJECT_SOURCE_DIR}/include)

add_library(library SHARED ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

GENERATE_EXPORT_HEADER(library
        EXPORT_FILE_NAME ${PROJECT_SOURCE_DIR}/include/hwctl/export.h)

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

#install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
#        DESTINATION include
#        COMPONENT headers)