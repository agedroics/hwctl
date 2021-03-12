#pragma once
#include <time.h>
#include <hwctl/vec.h>

struct profile;

struct profile *profile_open(const char *path, const struct vec *devs);

void profile_close(struct profile*);

int profile_exec(const struct profile*);

struct timespec profile_get_period(const struct profile*);

void profile_mark_for_stop(struct profile*);

int profile_marked_for_stop(const struct profile*);
