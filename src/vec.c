/**
 * IFJ23
 *
 * xsleza23 Martin Slezák
 * xstigl00 Jakub Antonín Štigler
 */

#include "vec.h" // Vec, bool, size_t, NULL, true, false, Span, FreeFun
#include "errors.h"

#include <stdlib.h> // free, realloc, malloc
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
bool vec_push(Vec *vec, void *item) {
    if (!vec_reserve(vec, vec->len + 1)) {
        return false;
    }

    memcpy(vec_at(vec, vec->len), item, vec->item_size);
    ++vec->len;

    return true;
}

void *vec_at(Vec *vec, size_t index) {
    return vec->items + index * vec->item_size;
}

void *vec_pop(Vec *vec) {
    --vec->len;
    return vec_at(vec, vec->len);
}

Span vec_pop_range(Vec *vec, size_t count) {
    Span res = vec_slice(vec, vec->len - count, count);
    vec->len -= count;
    return res;
}

void *vec_last(Vec *vec) {
    return vec_at(vec, vec->len - 1);
}

void vec_clear(Vec *vec) {
    vec->len = 0;
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
        return OTHER_ERR_FALSE;
    }

    vec->allocated = new_size;
    vec->items = new_data;

    return true;
}

void vec_free_with(Vec *vec, FreeFun free) {
    for (size_t i = 0; i < vec->len; ++i) {
        free(vec_at(vec, i));
    }
    vec_free(vec);
}

Span vec_as_span(Vec *vec) {
    return span_new(vec->items, vec->item_size, vec->len);
}

Span vec_slice(Vec *vec, size_t start, size_t len) {
    return span_new(vec_at(vec, start), vec->item_size, len);
}

bool vec_push_span(Vec *vec, Span span) {
    if (vec->item_size != span.item_size
        || !vec_reserve(vec, vec->len + span.len)
    ) {
        return false;
    }

    memcpy(vec_at(vec, vec->len), span.items, span.len * span.item_size);
    vec->len += span.len;

    return true;
}

Vec vec_clone(Vec *vec) {
    return span_to_vec(vec_as_span(vec));
}

bool vec_insert(Vec *vec, size_t index, void *item) {
    if (index == vec->len) {
        return vec_push(vec, item);
    }

    if (!vec_reserve(vec, vec->len + 1)) {
        return false;
    }

    memmove(
        vec_at(vec, index + 1),
        vec_at(vec, index),
        (vec->len - index) * vec->item_size
    );
    memcpy(vec_at(vec, index), item, vec->item_size);
    ++vec->len;
    return true;
}

void vec_remove(Vec *vec, size_t index) {
    if (index == vec->len - 1) {
        vec_pop(vec);
        return;
    }

    memmove(
        vec_at(vec, index),
        vec_at(vec, index + 1),
        (vec->len - index - 1) * vec->item_size
    );
    --vec->len;
}

Span span_new(void *data, size_t item_size, size_t len) {
    return (Span) {
        .items = data,
        .item_size = item_size,
        .len = len,
    };
}

void *span_at(Span span, size_t index) {
    return span.items + index * span.item_size;
}

Span span_slice(Span span, size_t start, size_t len) {
    return span_new(span_at(span, start), span.item_size, len);
}

Vec span_to_vec(Span span) {
    Vec res = {
        .allocated = span.len,
        .item_size = span.item_size,
        .items = malloc(span.item_size * span.len),
        .len = span.len,
    };

    if (!res.items) {
        (void)OTHER_ERR_FALSE;
        return vec_new(span.item_size);
    }

    memcpy(res.items, span.items, res.len * res.item_size);
    return res;
}
