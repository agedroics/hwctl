#pragma once
#include <hwctl/vec.h>

struct profile;

struct profile *profile_open(char *path, struct vec *devs);

void profile_close(struct profile*);

void profile_exec(struct profile*);