set(DL_UTIL_SOURCES
        ${PROJECT_SOURCE_DIR}/util/dl_util.c)

set(DL_UTIL_HEADERS
        ${PROJECT_SOURCE_DIR}/util/dl_util.h)

set(DL_UTIL_INCLUDES
        ${PROJECT_SOURCE_DIR}/util)

add_library(dl_util STATIC ${DL_UTIL_SOURCES} ${DL_UTIL_HEADERS})

target_include_directories(dl_util
        PUBLIC ${DL_UTIL_INCLUDES})

set_property(TARGET dl_util PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT WIN32)
    target_link_libraries(dl_util ${CMAKE_DL_LIBS})
endif()


set(FS_UTIL_SOURCES
        ${PROJECT_SOURCE_DIR}/util/fs_util.c)

set(FS_UTIL_HEADERS
        ${PROJECT_SOURCE_DIR}/util/fs_util.h)

set(FS_UTIL_INCLUDES
        ${PROJECT_SOURCE_DIR}/util)

add_library(fs_util STATIC ${FS_UTIL_SOURCES} ${FS_UTIL_HEADERS})

target_include_directories(fs_util
        PUBLIC ${FS_UTIL_INCLUDES})

set_property(TARGET fs_util PROPERTY POSITION_INDEPENDENT_CODE ON)


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
        NAMES hidapi hidapi-libusb hidapi-hidraw libhidapi libhidapi-dev)

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

    if(WIN32)
        find_file(HIDAPI_RUNTIME hidapi.dll)

        if(NOT HIDAPI_RUNTIME STREQUAL "HIDAPI_RUNTIME-NOTFOUND")
            install(FILES ${HIDAPI_RUNTIME}
                    DESTINATION bin
                    COMPONENT hidapi_library)
        endif()
    endif()
endif()