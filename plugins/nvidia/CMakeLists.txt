find_package(CUDA)

if(CUDA_FOUND)
    set(NVIDIA_PLUGIN_SOURCES
            ${NVIDIA_PLUGIN_SOURCE_DIR}/nvidia.c)

    add_library(nvidia-plugin MODULE ${NVIDIA_PLUGIN_SOURCES})

    target_include_directories(nvidia-plugin
            PRIVATE ${LIBRARY_INCLUDES}
            PRIVATE ${STR_UTIL_INCLUDES}
            PRIVATE ${CUDA_INCLUDE_DIRS})

    find_library(NVML
            NAMES nvml libnvml nvidia-ml libnvidia-ml
            PATHS ${CUDA_TOOLKIT_ROOT_DIR}/lib ${CUDA_TOOLKIT_ROOT_DIR}/lib64 ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64)

    target_link_libraries(nvidia-plugin
            PRIVATE library
            PRIVATE str_util
            PRIVATE ${NVML})

    set_target_properties(nvidia-plugin
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/hwctl)

    install(TARGETS nvidia-plugin
            LIBRARY DESTINATION lib/hwctl
            COMPONENT nvidia-plugin)
endif()