set(HEADERS_HEADERS
        ${HEADERS_SOURCE_DIR}/hwctl/device.h
        ${HEADERS_SOURCE_DIR}/hwctl/loader.h
        ${HEADERS_SOURCE_DIR}/hwctl/plugin.h
        ${HEADERS_SOURCE_DIR}/hwctl/vec.h
        ${HEADERS_SOURCE_DIR}/hwctl/export.h)

add_library(headers INTERFACE)

install(FILES ${HEADERS_HEADERS}
        DESTINATION include/hwctl
        COMPONENT headers)