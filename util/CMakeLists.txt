set(DL_UTIL_SOURCES
        ${PROJECT_SOURCE_DIR}/util/dl_util.c)

set(DL_UTIL_HEADERS
        ${PROJECT_SOURCE_DIR}/util/dl_util.h)

set(DL_UTIL_INCLUDES
        ${PROJECT_SOURCE_DIR}/util)

add_library(dl_util STATIC ${DL_UTIL_SOURCES} ${DL_UTIL_HEADERS})

target_include_directories(dl_util
        PUBLIC ${DL_UTIL_INCLUDES})


set(STR_UTIL_SOURCES
        ${PROJECT_SOURCE_DIR}/util/str_util.c)

set(STR_UTIL_HEADERS
        ${PROJECT_SOURCE_DIR}/util/str_util.h)

set(STR_UTIL_INCLUDES
        ${PROJECT_SOURCE_DIR}/util)

add_library(str_util STATIC ${STR_UTIL_SOURCES} ${STR_UTIL_HEADERS})

target_include_directories(str_util
        PUBLIC ${STR_UTIL_INCLUDES})


find_library(LIBUSB
        NAMES usb usb-1.0 libusb libusb-1.0)

if(NOT LIBUSB STREQUAL "LIBUSB-NOTFOUND")
    set(USB_UTIL_SOURCES
            ${PROJECT_SOURCE_DIR}/util/usb_util.c)

    set(USB_UTIL_HEADERS
            ${PROJECT_SOURCE_DIR}/util/usb_util.h)

    set(USB_UTIL_INCLUDES
            ${PROJECT_SOURCE_DIR}/util)

    add_library(usb_util STATIC ${USB_UTIL_SOURCES} ${USB_UTIL_HEADERS})

    target_include_directories(usb_util
            PUBLIC ${USB_UTIL_INCLUDES})

    target_link_libraries(usb_util
            PRIVATE ${LIBUSB})

    if(WIN32)
        find_file(LIBUSB_RUNTIME libusb-1.0.dll)

        if(NOT LIBUSB_RUNTIME STREQUAL "LIBUSB_RUNTIME-NOTFOUND")
            install(FILES ${LIBUSB_RUNTIME}
                    DESTINATION bin
                    COMPONENT libusb_library)
        endif()
    endif()
endif()