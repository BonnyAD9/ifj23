#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stddef.h>  // size_t
#include <stdbool.h> // bool

/// Creates new vector of the given type
#define VEC_NEW(type) vec_new(sizeof(type))

/// Gets the value at the given index, DOESN'T CHECK BOUNDS
#define VEC_AT(vec, type, index) (((type *)((vec)->items))[index])

/// Gets the last value, segfault if vector is empty
#define VEC_LAST(vec, type) (*(type *)vec_last(vec))

/// removes the last item from the vector and returns it.
/// segfault if vector is empty.
#define VEC_POP_BACK(vec, type) (*(type *)vec_pop_back(vec))

/// Adds value to the vector
#define VEC_PUSH_BACK(_vec, type, value) do { \
    Vec *vec = _vec; \
    if (vec_reserve(vec, vec->len + 1)) { \
        VEC_AT(vec, type, vec->len) = value; \
        ++vec->len; \
    } \
} while (false);

/// For each loop
#define VEC_FOR_EACH(_vec, type, name) for ( \
    struct { size_t i; type *v; Vec *vec; } name = { .i = 0, .vec = _vec }; \
    name.i < name.vec->len \
        && (name.v = &VEC_AT(name.vec, type, name.i), true); \
    ++name.i \
)

/// generic vector
typedef struct {
    char *items;
    size_t item_size;
    /// number of allocated items
    size_t allocated;
    /// number of items in the vector
    size_t len;
} Vec;

struct VecIterator {
    size_t index;
};

typedef void (*IterFun)(void *);

/// creates new vector
Vec vec_new(size_t item_size);

/// frees the vector
void vec_free(Vec *vec);

/// frees all items and the vector
void vec_free_with(Vec *vec, IterFun free);

/// adds item to the end of the vector, returns false on failure
bool vec_push_back(Vec *vec, void *item);

/// gets pointer to item at the given index, DOESN'T CHECK BOUNDS
void *vec_at(Vec *vec, size_t index);

/// removes the last item from the vector and returns pointer to it.
/// Returns NULL if vector is empty.
/// THE POINTER IS VALID ONLY UNTIL PUSH OR FREE.
void *vec_pop_back(Vec *vec);

/// gets pointer to the last item in the vector, NULL if vector is empty
void *vec_last(Vec *vec);

/// pops all items from the vector
void vec_clear(Vec *vec);

/// runs the given function for each item
void vec_for_each(Vec *vec, IterFun fun);

/// makes sure that the vector can contain at least len elements before realloc
/// returns false if fails
bool vec_reserve(Vec *vec, size_t len);

#endif // VECTOR_H_INCLUDED
