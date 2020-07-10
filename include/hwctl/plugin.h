#pragma once
#include <hwctl/device.h>
#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT
#endif

PLUGIN_EXPORT void hwctl_init_plugin(void);

PLUGIN_EXPORT void hwctl_shutdown_plugin(void);

PLUGIN_EXPORT void hwctl_init_dev_det(struct hwctl_dev_det*);
