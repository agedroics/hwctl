find_library(HIDAPI
        NAMES hidapi hidapi-libusb hidapi-hidraw libhidapi libhidapi-dev)

if(NOT HIDAPI STREQUAL "HIDAPI-NOTFOUND")
    set(KRAKEN_XX2_SOURCES
            ${PROJECT_SOURCE_DIR}/plugins/kraken_xx2.c)

    add_library(kraken_xx2 MODULE ${KRAKEN_XX2_SOURCES})

    target_include_directories(kraken_xx2
            PRIVATE ${HWCTL_INCLUDES}
            PRIVATE ${STR_UTIL_INCLUDES}
            PRIVATE ${HID_UTIL_INCLUDES})

    target_link_libraries(kraken_xx2
            PRIVATE hwctl
            PRIVATE str_util
            PRIVATE hid_util
            PRIVATE ${HIDAPI})

    set_target_properties(kraken_xx2
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/hwctl)

    install(TARGETS kraken_xx2
            LIBRARY DESTINATION lib/hwctl
            COMPONENT kraken_xx2_plugin)

    if(WIN32)
        find_file(HIDAPI_RUNTIME hidapi.dll)

        if(NOT HIDAPI_RUNTIME STREQUAL "HIDAPI_RUNTIME-NOTFOUND")
            install(FILES ${HIDAPI_RUNTIME}
                    DESTINATION bin
                    COMPONENT hidapi_library)
        endif()
    endif()
endif()


find_package(CUDA)

if(CUDA_FOUND)
    set(NVIDIA_SOURCES
            ${PROJECT_SOURCE_DIR}/plugins/nvidia.c)

    add_library(nvidia MODULE ${NVIDIA_SOURCES})

    target_include_directories(nvidia
            PRIVATE ${HWCTL_INCLUDES}
            PRIVATE ${STR_UTIL_INCLUDES}
            PRIVATE ${CUDA_INCLUDE_DIRS})

    find_library(NVML
            NAMES nvml libnvml nvidia-ml libnvidia-ml
            PATHS ${CUDA_TOOLKIT_ROOT_DIR}/lib ${CUDA_TOOLKIT_ROOT_DIR}/lib64 ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64)

    target_link_libraries(nvidia
            PRIVATE hwctl
            PRIVATE str_util
            PRIVATE ${NVML})

    set_target_properties(nvidia
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/hwctl)

    install(TARGETS nvidia
            LIBRARY DESTINATION lib/hwctl
            COMPONENT nvidia_plugin)
endif()