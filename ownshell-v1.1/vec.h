#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VECTOR_INITIAL_CAPACITY 4

// Vector MUST follow the prototype
// to use the functionalties.
// typedef struct{
//     Base_Type* data;
//     size_t size;
//     size_t capacity;
// } Your_Type;

#define vec_dispatch(_1, _2, _3, _4, NAME, ...) NAME

#define vec_init_without_size(vec) \
    do { \
        memset((vec), 0, sizeof(*(vec))); \
    } while(0)

#define vec_init_with_size(vec, size) \
    do { \
        (vec)->size = (size); \
        (vec)->capacity = (size) > VECTOR_INITIAL_CAPACITY ? (size) : VECTOR_INITIAL_CAPACITY; \
        (vec)->data = malloc((vec)->capacity * sizeof(*(vec)->data)); \
        assert((vec)->data != NULL); \
    } while(0)

#define vec_init_with_size_value(vec, size, value) \
    do { \
        vec_init_with_size((vec), (size)); \
        for (size_t i = 0; i < (vec)->size; i++) { \
            (vec)->data[i] = (value); \
        } \
    } while(0)

#define vec_init(...) \
    vec_dispatch(dummy, ##__VA_ARGS__, vec_init_with_size_value, vec_init_with_size, vec_init_without_size)(__VA_ARGS__)

#define vec_free(vec) \
    do { \
        if((vec)->data) { \
            free((vec)->data); \
            (vec)->data = NULL; \
        } \
        (vec)->size = 0; \
        (vec)->capacity = 0; \
    } while(0)

#define vec_append(vec, item) \
    do { \
        if ((vec)->size + 1 >= (vec)->capacity) { \
            (vec)->capacity = ((vec)->capacity == 0) ? VECTOR_INITIAL_CAPACITY : ((vec)->capacity << 1); \
            (vec)->data = realloc((vec)->data, (vec)->capacity * sizeof(*(vec)->data)); \
            assert((vec)->data != NULL); \
        } \
        (vec)->data[(vec)->size++] = (item); \
    } while(0)

#define vec_appendn(vec, items, count) \
    do { \
        if ((vec)->size + (count) >= (vec)->capacity) { \
            (vec)->capacity = ((vec)->capacity == 0) ? VECTOR_INITIAL_CAPACITY : ((vec)->capacity << 1); \
            while ((vec)->size + (count) >= (vec)->capacity) \
                (vec)->capacity = ((vec)->capacity << 1); \
            (vec)->data = realloc((vec)->data, (vec)->capacity * sizeof(*(vec)->data)); \
            assert((vec)->data != NULL); \
        } \
        memcpy((vec)->data + (vec)->size, (items), (count) * sizeof(*(vec)->data)); \
        (vec)->size += (count); \
    } while(0)

#define vec_clear(vec) \
    do { \
        (vec)->size = 0; \
    } while(0)

#define vec_size(vec) ((vec)->size)