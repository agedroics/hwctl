#ifndef HWCTL_DEVICE_H
#define HWCTL_DEVICE_H

#include <stdint.h>
#include <hwctl/vec.h>

struct dev;

struct temp_sen {
    float (*read_temp)(struct dev*);
};

struct speed_sen {
    int32_t (*read_duty)(struct dev*);
    int32_t (*read_speed)(struct dev*);
};

struct speed_act {
    void (*write_duty)(struct dev*, int32_t);
};

struct dev {
    void *data;
    void (*destroy_data)(void*);
    char* (*get_name)(struct dev*);
    char* (*get_desc)(struct dev*);
    struct temp_sen *temp_sen;
    struct speed_sen *speed_sen;
    struct speed_act *speed_act;
    struct vec *children;
};

void dev_init(struct dev*);

void dev_destroy(struct dev*);

struct dev_det {
    void (*det_devs)(struct vec*);
};

#endif
