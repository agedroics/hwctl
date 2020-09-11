set(STR_UTIL_SOURCES
        ${PROJECT_SOURCE_DIR}/util/str_util.c)

set(STR_UTIL_HEADERS
        ${PROJECT_SOURCE_DIR}/util/str_util.h)

set(STR_UTIL_INCLUDES
        ${PROJECT_SOURCE_DIR}/util)

add_library(str_util STATIC ${STR_UTIL_SOURCES} ${STR_UTIL_HEADERS})

target_include_directories(str_util
        PUBLIC ${STR_UTIL_INCLUDES})

set_property(TARGET str_util PROPERTY POSITION_INDEPENDENT_CODE ON)


find_library(HIDAPI
        NAMES hidapi libhidapi hidapi-libusb libhidapi-libusb hidapi-hidraw libhidapi-hidraw)

if(NOT HIADPI STREQUAL "HIDAPI-NOTFOUND")
    set(HID_UTIL_SOURCES
            ${PROJECT_SOURCE_DIR}/util/hid_util.c)

    set(HID_UTIL_HEADERS
            ${PROJECT_SOURCE_DIR}/util/hid_util.h)

    set(HID_UTIL_INCLUDES
            ${PROJECT_SOURCE_DIR}/util)

    add_library(hid_util STATIC ${HID_UTIL_SOURCES} ${HID_UTIL_HEADERS})

    target_include_directories(hid_util
            PUBLIC ${HID_UTIL_INCLUDES})

    set_property(TARGET hid_util PROPERTY POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(hid_util
            PRIVATE str_util
            PRIVATE ${HIDAPI})
endif()