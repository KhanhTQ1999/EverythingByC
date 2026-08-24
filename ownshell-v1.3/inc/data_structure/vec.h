#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    VEC_SUCCESS = 0,
    VEC_ERR_ALLOC = 1,
    VEC_ERR_NULL = 2,
    VEC_ERR_OUT_OF_BOUNDS = 3
};

struct Vec {
    void *data;
    size_t esize;
    size_t size;
    size_t cap;
};

int vec_init(struct Vec *self, size_t size, size_t esize);
int vec_push(struct Vec *self, void *elem);
void* vec_at(struct Vec *self, size_t index);
int vec_remove(struct Vec *self, size_t index);
int vec_clear(struct Vec *self);
int vec_free(struct Vec *self);