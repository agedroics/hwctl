#include <stdlib.h>
#include <string.h>
#include <hwctl/vec.h>
#include <heap.h>

struct heap {
    struct vec *vec;
    int (*compare_items)(const void*, const void*);
};

void heap_init(struct heap** heap, size_t item_size, int (*compare_items)(const void*, const void*)) {
    *heap = malloc(sizeof(struct heap));
    vec_init(&(*heap)->vec, item_size);
    (*heap)->compare_items = compare_items;
}

void heap_destroy(struct heap *heap) {
    vec_destroy(heap->vec, 0);
    free(heap);
}

static inline size_t get_lchild_i(size_t i) {
    return (i << 1) + 1;
}

static inline size_t get_rchild_i(size_t i) {
    return (i << 1) + 2;
}

static inline size_t get_parent_i(size_t i) {
    return (i - (2 - (i & 1))) >> 1;
}

void heap_push(struct heap *heap, const void *item) {
    size_t i = vec_size(heap->vec);
    vec_push_back(heap->vec, NULL);

    while (i) {
        size_t parent_i = get_parent_i(i);
        void *parent = vec_at(heap->vec, parent_i);

        if (heap->compare_items(item, parent) >= 0) {
            break;
        }

        vec_replace_at(heap->vec, i, parent, NULL);
        i = parent_i;
    }

    vec_replace_at(heap->vec, i, item, NULL);
}

static void heapify(struct heap *heap, size_t size, const void *item) {
    size_t i = 0;

    for (;;) {
        size_t lchild_i = get_lchild_i(i);
        size_t rchild_i = lchild_i + 1;

        const void *smallest = item;

        if (lchild_i < size && heap->compare_items(vec_at(heap->vec, lchild_i), smallest) < 0) {
            smallest = vec_at(heap->vec, lchild_i);
        }
        if (rchild_i < size && heap->compare_items(vec_at(heap->vec, rchild_i), smallest) < 0) {
            smallest = vec_at(heap->vec, rchild_i);
        }

        if (smallest == item) {
            break;
        } else {
            vec_replace_at(heap->vec, i, smallest, NULL);
            i = vec_index_of(heap->vec, smallest);
        }
    }

    vec_replace_at(heap->vec, i, item, NULL);
}

int heap_pop(struct heap *heap, void *result) {
    if (!vec_size(heap->vec)) {
        return 1;
    }

    if (result) {
        memcpy(result, vec_head(heap->vec), vec_item_size(heap->vec));
    }

    void *item = vec_last(heap->vec);
    heapify(heap, vec_size(heap->vec) - 1, item);
    vec_pop_back(heap->vec, NULL);
    return 0;
}

int heap_push_pop(struct heap *heap, const void *item, void *result) {
    if (vec_size(heap->vec) == 0) {
        return 1;
    }

    if (heap->compare_items(vec_head(heap->vec), item) >= 0) {
        return 1;
    }

    if (result) {
        memcpy(result, vec_head(heap->vec), vec_item_size(heap->vec));
    }
    heapify(heap, vec_size(heap->vec), item);
    return 0;
}

void *heap_head(struct heap *heap) {
    return vec_head(heap->vec);
}
