#pragma once
#include <time.h>

void time_from_millis(struct timespec *, long);

struct timespec time_nanos();

struct timespec time_diff(const struct timespec*, const struct timespec*);

void time_add(struct timespec*, const struct timespec *amt);

void time_subtract(struct timespec*, const struct timespec *amt);

int time_is_positive(const struct timespec*);