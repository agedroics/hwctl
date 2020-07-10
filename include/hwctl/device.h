#pragma once
#include <stdint.h>
#include <hwctl/vec.h>
#include <hwctl/export.h>

struct hwctl_dev;

struct hwctl_temp_sen {
    float (*read_temp)(struct hwctl_dev*);
};

struct hwctl_speed_sen {
    int32_t (*read_duty)(struct hwctl_dev*);
    int32_t (*read_speed)(struct hwctl_dev*);
};

struct hwctl_speed_act {
    void (*write_duty)(struct hwctl_dev*, int32_t);
};

struct hwctl_dev {
    void *data;
    void (*destroy_data)(void*);
    char* (*get_id)(struct hwctl_dev*);
    char* (*get_desc)(struct hwctl_dev*);
    struct hwctl_temp_sen *temp_sen;
    struct hwctl_speed_sen *speed_sen;
    struct hwctl_speed_act *speed_act;
    struct vec *subdevs;
};

HWCTL_EXPORT void hwctl_dev_init(struct hwctl_dev*);

HWCTL_EXPORT void hwctl_dev_destroy(struct hwctl_dev*);

struct hwctl_dev_det {
    void (*det_devs)(struct vec*);
};
