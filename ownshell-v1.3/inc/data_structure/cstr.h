#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CSTR_SUCCESS = 0,
    CSTR_ERR_ALLOC = 1,
    CSTR_ERR_NULL = 2,
    CSTR_ERR_OUT_OF_BOUNDS = 3
};

struct Cstr{
    char *data;
    size_t len;
    size_t cap;
} ;

void cstr_init(struct Cstr *self);
int cstr_append(struct Cstr *self, const char *str);
int cstr_appendn(struct Cstr *self, const char *str, size_t n);
int cstr_pop(struct Cstr *self);
int cstr_copy(struct Cstr *self, const char *str, size_t n);
char cstr_at(struct Cstr *self, size_t index);
int cstr_substr(struct Cstr *self, size_t start, size_t end, struct Cstr *out);
int cstr_reverse(struct Cstr *self);
int cstr_remove(struct Cstr *self, size_t start, size_t len);
int cstr_split(struct Cstr *self, const char *delim, struct Cstr **out, size_t *count);
int cstr_clear(struct Cstr *self);
int cstr_free(struct Cstr *self);