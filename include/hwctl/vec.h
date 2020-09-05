#pragma once
#include <stddef.h>
#include <hwctl/export.h>

struct vec;

HWCTL_EXPORT void vec_init(struct vec**, size_t item_size);

HWCTL_EXPORT void vec_destroy(struct vec*, int keep_data);

HWCTL_EXPORT size_t vec_size(const struct vec*);

HWCTL_EXPORT void *vec_push_back(struct vec*);

HWCTL_EXPORT void *vec_data(const struct vec*);
