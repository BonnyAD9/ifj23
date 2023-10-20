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
#define VEC_POP(vec, type) (*(type *)vec_pop(vec))

/// Adds value to the vector
#define VEC_PUSH(_vec, type, value) do { \
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

/// Creates new span
#define SPAN_NEW(data, len) span_new(data, sizeof(*data), len)

/// Creates new span from array with size known at compile time
#define SPAN_ARR(arr) span_new(arr, sizeof(*arr), sizeof(arr) / sizeof(*arr))

/// Gets the item at the given index
#define SPAN_AT(span, type, index) (((type *)((span).items))[index])

/// For each loop
#define SPAN_FOR_EACH(_span, type, name) for ( \
    struct { size_t i; type *v; Span span; } name \
        = { .i = 0, .span = _span }; \
    name.i < name.span.len \
        && (name.v = &SPAN_AT(name.span, type, name.i), true); \
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

/// Span of borrowed generic data
typedef struct {
    char *items;
    size_t item_size;
    size_t len;
} Span;

/// Function that frees object
typedef void (*FreeFun)(void *data);

/// creates new vector
Vec vec_new(size_t item_size);

/// frees the vector
void vec_free(Vec *vec);

/// frees all items and the vector
void vec_free_with(Vec *vec, FreeFun free);

/// adds item to the end of the vector, returns false on failure
bool vec_push(Vec *vec, void *item);

/// gets pointer to item at the given index, DOESN'T CHECK BOUNDS
void *vec_at(Vec *vec, size_t index);

/// removes the last item from the vector and returns pointer to it.
/// Returns NULL if vector is empty.
/// THE POINTER IS VALID ONLY UNTIL PUSH OR FREE.
void *vec_pop(Vec *vec);

/// gets pointer to the last item in the vector, NULL if vector is empty
void *vec_last(Vec *vec);

/// pops all items from the vector
void vec_clear(Vec *vec);

/// makes sure that the vector can contain at least len elements before realloc
/// returns false if fails
bool vec_reserve(Vec *vec, size_t len);

/// creates span of the whole vector. THE SPAN IS VALID ONLY UNTIL PUSH
Span vec_as_span(Vec *vec);

/// creates slice of the vector. THE SPAN IS VALID ONLY UNTIL PUSH.
/// It doesn't check the bounds.
Span vec_slice(Vec *vec, size_t start, size_t len);

/// Pushes a span to the end of te vector.
/// THIS IS NOT VALID FOR SPAN OF THE SAME VEC
bool vec_push_span(Vec *vec, Span span);

/// Copies the vector, returns empty vector when fails
Vec vec_clone(Vec *vec);

/// creates new span
Span span_new(void *data, size_t item_size, size_t len);

/// gets pointer to value at the given index. DOESN'T CHECK BOUNDS
void *span_at(Span span, size_t index);

/// Slices te span
Span span_slice(Span span, size_t start, size_t len);

/// Copies the data into a new vector, returns empty vector if fails
Vec span_to_vec(Span span);

#endif // VECTOR_H_INCLUDED
