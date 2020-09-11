#pragma once
#include <stdint.h>
#include <hwctl/export.h>
#include <hwctl/vec.h>

struct hwctl_dev;

struct hwctl_dev {
    void *data;
    void (*destroy_data)(void*);
    char* (*get_id)(struct hwctl_dev*);
    char* (*get_desc)(struct hwctl_dev*);
    double (*read_sen)(struct hwctl_dev*);
    void (*write_act)(struct hwctl_dev*, double);
    struct vec *subdevs;
};

HWCTL_EXPORT void hwctl_dev_init(struct hwctl_dev*);

HWCTL_EXPORT void hwctl_dev_destroy(struct hwctl_dev*);

struct hwctl_dev_det {
    void (*det_devs)(struct vec*);
};
