#ifndef HWCTL_MODULE_H
#define HWCTL_MODULE_H

#include <hwctl/device.h>

void init_plugin(void);

void shutdown_plugin(void);

void init_dev_det(struct dev_det*);

#endif
