#include "vec.h" // Vec, bool, size_t, NULL, true, false

#include <stdlib.h> // free, realloc
#include <string.h> // memcpy

Vec vec_new(size_t item_size) {
    return (Vec) {
        .items = NULL,
        .item_size = item_size,
        .allocated = 0,
        .len = 0,
    };
}

void vec_free(Vec *vec) {
    free(vec->items);
    vec->items = NULL;
    vec->allocated = 0;
    vec->len = 0;
}

/// adds item to the end of the vector
bool vec_push_back(Vec *vec, void *item) {
    if (!vec_reserve(vec, vec->len + 1)) {
        return false;
    }

    memcpy(vec_at(vec, vec->len), item, vec->item_size);
    ++vec->len;

    return true;
}

/// gets pointer to item at the given index, NULL if index is out of bounds
void *vec_at(Vec *vec, size_t index) {
    return vec->items + index * vec->len;
}

/// removes the last item from the vector and writes its data to popped
void vec_pop_back(Vec *vec, void *popped) {
    if (popped) {
        memcpy(vec_last(vec), popped, vec->item_size);
    }
    --vec->len;
}

/// gets pointer to the last item in the vector
void *vec_last(Vec *vec) {
    return vec_at(vec, vec->len - 1);
}

/// pops all items from the vector
void vec_clear(Vec *vec) {
    vec->len = 0;
}

/// runs the given function for each item
void vec_for_each(Vec *vec, void *data, void (*fun)(void *item)) {
    for (size_t i = 0; i < vec->len; ++i) {
        fun(vec_at(vec, i));
    }
}

bool vec_reserve(Vec *vec, size_t len) {
    if (len <= vec->allocated) {
        return true;
    }

    size_t new_size = vec->allocated * 2;

    if (new_size == 0) {
        new_size = 1;
    }

    while (new_size < len) {
        new_size *= 2;
    }


    char *new_data = realloc(vec->items, vec->item_size * new_size);
    if (!new_data) {
        return false;
    }

    vec->allocated = new_size;
    vec->items = new_data;

    return true;
}
