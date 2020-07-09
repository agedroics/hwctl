#pragma once
#include <hwctl/vec.h>

extern struct vec *hwctl_dev_dets;

void hwctl_load_plugins(void);

void hwctl_unload_plugins(void);
