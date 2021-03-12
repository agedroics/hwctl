#pragma once
#include <stddef.h>
#include <hwctl/export.h>

struct vec;

LIBRARY_EXPORT void vec_init(struct vec**, size_t item_size);

LIBRARY_EXPORT void vec_destroy(struct vec*, int keep_data);

LIBRARY_EXPORT size_t vec_size(const struct vec*);

LIBRARY_EXPORT void *vec_push_back(struct vec*);

LIBRARY_EXPORT void *vec_data(const struct vec*);

LIBRARY_EXPORT void vec_remove(struct vec*, void*);
