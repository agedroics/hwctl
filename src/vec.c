#include <stdlib.h>
#include <string.h>
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

void vec_destroy(struct vec *vec, int keep_data) {
    if (!keep_data) {
        free(vec->data);
    }
    free(vec);
}

size_t vec_item_size(const struct vec *vec) {
    return vec->item_size;
}

size_t vec_size(const struct vec *vec) {
    return vec->size;
}

void *vec_push_back(struct vec *vec, void *data) {
    if (vec->size == vec->cap) {
        vec->cap <<= 1u;
        vec->data = realloc(vec->data, vec->cap * vec->item_size);
    }

    ++vec->size;
    void *item = vec_last(vec);
    if (data) {
        memcpy(item, data, vec->item_size);
    } else {
        memset(item, 0, vec->item_size);
    }
    return item;
}

int vec_pop_back(struct vec *vec, void *result) {
    if (!vec->size) {
        return 1;
    }

    if (result && result != vec_last(vec)) {
        memcpy(result, vec_last(vec), vec->item_size);
    }

    --vec->size;
    return 0;
}

void *vec_at(const struct vec *vec, size_t i) {
    if (i >= vec->size) {
        return NULL;
    }

    return ((char*) vec->data) + i * vec->item_size;
}

int vec_replace(struct vec *vec, void *old_item, const void *item) {
    ssize_t i = vec_index_of(vec, old_item);
    if (i == -1) {
        return 1;
    }

    if (item) {
        if (item != old_item) {
            memcpy(old_item, item, vec->item_size);
        }
    } else {
        memset(old_item, 0, vec->item_size);
    }

    return 0;
}

int vec_replace_at(struct vec *vec, size_t i, const void *item, void *result) {
    if (i >= vec->size) {
        return 1;
    }

    void *old_item = vec_at(vec, i);

    void *temp;
    int temp_created = 0;
    if (result && result != old_item) {
        if (result == item) {
            temp = malloc(vec->item_size);
            memcpy(temp, old_item, vec->item_size);
            temp_created = 1;
        } else {
            memcpy(result, old_item, vec->item_size);
        }
    }

    if (item) {
        if (item != old_item) {
            memcpy(old_item, item, vec->item_size);
        }
    } else {
        memset(old_item, 0, vec->item_size);
    }

    if (temp_created) {
        memcpy(result, temp, vec->item_size);
        free(temp);
    }

    return 0;
}

void *vec_head(const struct vec *vec) {
    return vec_at(vec, 0);
}

void *vec_last(const struct vec *vec) {
    return vec_at(vec, vec->size - 1);
}

ssize_t vec_index_of(const struct vec *vec, const void *item) {
    if (item < vec->data) {
        return -1;
    }

    size_t diff = ((char*) item) - ((char*) vec->data);
    if (diff % vec->item_size) {
        return -1;
    }

    size_t i = diff / vec->item_size;
    if (i >= vec->size) {
        return -1;
    }

    return i;
}

void *vec_data(const struct vec *vec) {
    return vec->data;
}

int vec_remove(struct vec *vec, void *item) {
    if (!vec->size) {
        return 1;
    }

    ssize_t i = vec_index_of(vec, item);
    if (i == -1) {
        return 1;
    }

    if (i == vec->size - 1) {
        return vec_pop_back(vec, NULL);
    }

    memmove(item, vec_at(vec, i + 1), (vec->size - i - 1) * vec->item_size);
    --vec->size;
    return 0;
}

void vec_sort(struct vec *vec, int (*compare_items)(const void*, const void*)) {
    qsort(vec->data, vec->size, vec->item_size, compare_items);
}
