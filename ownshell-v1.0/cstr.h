#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} s8;

typedef struct {
    s8* data;
    size_t size;
    size_t capacity;
} s8_list;

#define S8_DISPATCH(_1, _2, _3, NAME, ...) NAME
#define s8_create(...) \
    S8_DISPATCH(dummy, ##__VA_ARGS__, s8_create_by_cptr, s8_create_by_another)(__VA_ARGS__)

void s8_create_by_cptr(s8* str, const char* cptr);
void s8_create_by_another(s8* str, const s8* src);
void s8_append(s8* str, const char* s);
void s8_appendn(s8* str, const char* s, size_t n);
int s8_to_int(s8* str);
void s8_free(s8* str);
int s8_copy(s8* dest, const s8* src);
int s8_substring(s8* dest, const s8* src, size_t start, size_t length);
;
void s8_list_free(s8_list* list);