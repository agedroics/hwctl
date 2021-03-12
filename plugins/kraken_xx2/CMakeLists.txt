find_library(HIDAPI
        NAMES hidapi libhidapi hidapi-libusb libhidapi-libusb hidapi-hidraw libhidapi-hidraw)

if(NOT HIDAPI STREQUAL "HIDAPI-NOTFOUND")
    set(KRAKEN_XX2_PLUGIN_SOURCES
            ${PLUGIN_SOURCE_DIR}/kraken_xx2/kraken_xx2.c)

    add_library(kraken-xx2-plugin MODULE ${KRAKEN_XX2_PLUGIN_SOURCES})

    target_include_directories(kraken-xx2-plugin
            PRIVATE ${LIBRARY_INCLUDES}
            PRIVATE ${STR_UTIL_INCLUDES}
            PRIVATE ${HID_UTIL_INCLUDES}
            PRIVATE ${TIME_UTIL_INCLUDES})

    target_link_libraries(kraken-xx2-plugin
            PRIVATE library
            PRIVATE str_util
            PRIVATE hid_util
            PRIVATE time_util
            PRIVATE ${HIDAPI})

    set_target_properties(kraken-xx2-plugin
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/hwctl)

    install(TARGETS kraken-xx2-plugin
            LIBRARY DESTINATION lib/hwctl
            COMPONENT kraken-xx2-plugin)
endif()