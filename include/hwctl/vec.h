#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <hwctl/export.h>

struct vec;

LIBRARY_EXPORT void vec_init(struct vec**, size_t item_size);

LIBRARY_EXPORT void vec_destroy(struct vec*, int keep_data);

LIBRARY_EXPORT size_t vec_item_size(const struct vec*);

LIBRARY_EXPORT size_t vec_size(const struct vec*);

LIBRARY_EXPORT void *vec_push_back(struct vec*, void*);

LIBRARY_EXPORT int vec_pop_back(struct vec*, void *result);

LIBRARY_EXPORT void *vec_at(const struct vec*, size_t);

LIBRARY_EXPORT int vec_replace(struct vec*, void *old_item, const void *item);

LIBRARY_EXPORT int vec_replace_at(struct vec*, size_t, const void *item, void *old_item);

LIBRARY_EXPORT void *vec_head(const struct vec*);

LIBRARY_EXPORT void *vec_last(const struct vec*);

LIBRARY_EXPORT ssize_t vec_index_of(const struct vec*, const void*);

LIBRARY_EXPORT void *vec_data(const struct vec*);

LIBRARY_EXPORT int vec_remove(struct vec*, void*);

LIBRARY_EXPORT void vec_sort(struct vec*, int (*compare_items)(const void*, const void*));
