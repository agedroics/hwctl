#pragma once
#include <hwctl/export.h>
#include <hwctl/vec.h>

LIBRARY_EXPORT struct vec *get_hwctl_dev_dets(void);

LIBRARY_EXPORT void hwctl_load_plugins(void);

LIBRARY_EXPORT void hwctl_unload_plugins(void);
