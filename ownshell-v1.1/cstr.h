#pragma once
#include <stdlib.h>

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} cstr;

char* cstr_begin(cstr* str);
char* cstr_end(cstr* str);
void cstr_append(cstr* str, const char* s);
void cstr_appendn(cstr* str, const char* s, size_t n);
int cstr_pop(cstr* str);
void cstr_free(cstr* str);
int cstr_to_int(cstr* str);
int cstr_copy(cstr* dest, const cstr* src);
int cstr_substring(cstr* dest, const cstr* src, size_t start, size_t length);
void cstr_remove(cstr* str, size_t start, size_t length);