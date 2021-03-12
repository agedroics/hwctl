#pragma once
#include <stdint.h>
#include <hwctl/export.h>
#include <hwctl/vec.h>

struct hwctl_dev;

struct hwctl_dev {
    void *data;
    void (*destroy_data)(void*);
    const char* (*get_id)(const struct hwctl_dev*);
    const char* (*get_desc)(const struct hwctl_dev*);
    int (*read_sen)(struct hwctl_dev*, double*);
    int (*write_act)(struct hwctl_dev*, double);
    struct vec *subdevs;
};

LIBRARY_EXPORT void hwctl_dev_init(struct hwctl_dev*);

LIBRARY_EXPORT void hwctl_dev_destroy(struct hwctl_dev*);

struct hwctl_dev_det {
    void (*det_devs)(struct vec*);
};
