add_library(headers INTERFACE)

target_link_libraries(headers
        INTERFACE library)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include
        COMPONENT headers)