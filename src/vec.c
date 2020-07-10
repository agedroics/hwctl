#include <stdlib.h>
#include <hwctl/vec.h>

#define INITIAL_CAP 4

struct vec {
    void *data;
    size_t item_size;
    size_t size;
    size_t cap;
};

void vec_init(struct vec **vec, size_t item_size) {
    *vec = malloc(sizeof(struct vec));
    (*vec)->data = malloc(item_size * INITIAL_CAP);
    (*vec)->item_size = item_size;
    (*vec)->size = 0;
    (*vec)->cap = INITIAL_CAP;
}

void vec_destroy(struct vec *vec) {
    free(vec->data);
    free(vec);
}

size_t vec_size(const struct vec *vec) {
    return vec->size;
}

void *vec_push_back(struct vec *vec) {
    if (vec->size == vec->cap) {
        vec->cap <<= 1u;
        vec->data = realloc(vec->data, vec->cap * vec->item_size);
    }

    ++vec->size;
    return ((char*) vec->data) + (vec->size - 1) * vec->item_size;
}

void *vec_data(const struct vec *vec) {
    return vec->data;
}
