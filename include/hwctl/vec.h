#pragma once
#include <stddef.h>

struct vec;

void vec_init(struct vec**, size_t item_size);

void vec_destroy(struct vec*);

size_t vec_size(struct vec*);

void *vec_push_back(struct vec*);

void *vec_data(struct vec*);
