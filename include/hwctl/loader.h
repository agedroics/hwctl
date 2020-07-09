#ifndef HWCTL_LOADER_H
#define HWCTL_LOADER_H

#include <hwctl/vec.h>

extern struct vec *dev_dets;

void hwctl_load_plugins(void);

void hwctl_unload_plugins(void);

#endif
