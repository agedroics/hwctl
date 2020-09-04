cmake_minimum_required(VERSION 3.10)

project(hwctl C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "-O2 -s -Wall -Wextra -Wpedantic")

add_subdirectory(${PROJECT_SOURCE_DIR}/daemon)

add_subdirectory(${PROJECT_SOURCE_DIR}/plugins)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(${PROJECT_SOURCE_DIR}/util)

set(CPACK_PACKAGE_NAME hwctl)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tool for controlling and monitoring hardware components")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "hwctl")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)

set (CPACK_COMPONENTS_ALL
        daemon
        kraken_xx2_plugin
        hidapi_library
        nvidia_plugin
        import_library
        library
        headers)

set (CPACK_COMPONENT_DAEMON_DISPLAY_NAME "Daemon")
set (CPACK_COMPONENT_KRAKEN_XX2_PLUGIN_DISPLAY_NAME "Kraken Xx2 Support")
set (CPACK_COMPONENT_HIDAPI_LIBRARY_DISPLAY_NAME "hidapi")
set (CPACK_COMPONENT_NVIDIA_PLUGIN_DISPLAY_NAME "nVidia GPU Support")
set (CPACK_COMPONENT_IMPORT_LIBRARY_DISPLAY_NAME "Import Library")
set (CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "Shared Library")
set (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C Headers")

set (CPACK_COMPONENT_DAEMON_DESCRIPTION
        "Background process that enforces user-configurable rules like fan curves and RGB profiles")
set (CPACK_COMPONENT_KRAKEN_XX2_PLUGIN_DESCRIPTION
        "Plugin for monitoring and controlling NZXT Kraken X42, X52, X62 and X72 AIO liquid coolers")
set (CPACK_COMPONENT_HIDAPI_LIBRARY_DESCRIPTION
        "hidapi")
set (CPACK_COMPONENT_NVIDIA_PLUGIN_DESCRIPTION
        "Plugin for monitoring nVidia GPU temperatures and fan speeds")
set (CPACK_COMPONENT_IMPORT_LIBRARY_DESCRIPTION
        "Build dependency for programs that make use of hwctl")
set (CPACK_COMPONENT_LIBRARY_DESCRIPTION
        "Runtime dependency for programs that make use of hwctl")
set (CPACK_COMPONENT_HEADERS_DESCRIPTION
        "C/C++ header files for use with hwctl")

set (CPACK_COMPONENT_DAEMON_DEPENDS library)
set (CPACK_COMPONENT_KRAKEN_XX2_PLUGIN_DEPENDS hidapi_library)
set (CPACK_COMPONENT_IMPORT_LIBRARY_DEPENDS headers)
set (CPACK_COMPONENT_HEADERS_DEPENDS import_library)

set(CPACK_COMPONENT_DAEMON_GROUP "Runtime")
set(CPACK_COMPONENT_KRAKEN_XX2_PLUGIN_GROUP "Plugins")
set(CPACK_COMPONENT_NVIDIA_PLUGIN_GROUP "Plugins")
set(CPACK_COMPONENT_IMPORT_LIBRARY_GROUP "Development")
set(CPACK_COMPONENT_LIBRARY_GROUP "Runtime")
set(CPACK_COMPONENT_HEADERS_GROUP "Development")

set(CPACK_ALL_INSTALL_TYPES User Full Developer)
set(CPACK_COMPONENT_DAEMON_INSTALL_TYPES Full User)
set(CPACK_COMPONENT_KRAKEN_XX2_PLUGIN_INSTALL_TYPES Full User)
set(CPACK_COMPONENT_NVIDIA_PLUGIN_INSTALL_TYPES Full User)
set(CPACK_COMPONENT_HIDAPI_LIBRARY_INSTALL_TYPES Full User)
set(CPACK_COMPONENT_IMPORT_LIBRARY_INSTALL_TYPES Full Developer)
set(CPACK_COMPONENT_LIBRARY_INSTALL_TYPES Full User Developer)
set(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Full Developer)

include(CPack)