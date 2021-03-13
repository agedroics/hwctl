#pragma once
#include <stddef.h>

struct heap;

void heap_init(struct heap**, size_t item_size, int (*compare_items)(const void*, const void*));

void heap_destroy(struct heap*);

void heap_push(struct heap*, const void*);

int heap_pop(struct heap *heap, void *result);

int heap_push_pop(struct heap*, const void*, void *result);

void *heap_head(struct heap*);
