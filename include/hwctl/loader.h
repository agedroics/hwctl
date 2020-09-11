#pragma once
#include <hwctl/export.h>
#include <hwctl/vec.h>

HWCTL_EXPORT const struct vec *get_hwctl_dev_dets(void);

HWCTL_EXPORT void hwctl_load_plugins(void);

HWCTL_EXPORT void hwctl_unload_plugins(void);
