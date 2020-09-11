#pragma once
#include <hwctl/device.h>

int hwctl_init_plugin(void);

int hwctl_shutdown_plugin(void);

void hwctl_init_dev_det(struct hwctl_dev_det*);
