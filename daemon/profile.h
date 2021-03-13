#pragma once
#include <time.h>
#include <hwctl/vec.h>

struct profile {
    struct timespec period;
    struct hwctl_dev *dev_in;
    struct hwctl_dev *dev_out;
    struct vec *pairs;
};

int profile_open(struct profile*, const char *path, const struct vec *devs);

void profile_close(struct profile*);

int profile_exec(const struct profile*);
