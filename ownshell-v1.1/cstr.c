#include <string.h>
#include "cstr.h"

#define CSTR_INITIAL_CAPACITY 16

void cstr_append(cstr* str, const char* s) {
    size_t len = strlen(s);
    if (str->size + len + 1 > str->capacity) {
        size_t new_capacity = (str->capacity == 0) ? CSTR_INITIAL_CAPACITY : str->capacity * 2;
        while (new_capacity < str->size + len + 1) {
            new_capacity *= 2;
        }
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) {
            return;
        }
        str->data = new_data;
        str->capacity = new_capacity;
    }
    memcpy(str->data + str->size, s, len);
    str->size += len;
    str->data[str->size] = '\0';
}

void cstr_appendn(cstr* str, const char* s, size_t n) {
    if (str->size + n + 1 > str->capacity) {
        size_t new_capacity = (str->capacity == 0) ? CSTR_INITIAL_CAPACITY : str->capacity * 2;
        while (new_capacity < str->size + n + 1) {
            new_capacity *= 2;
        }
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) {
            return;
        }
        str->data = new_data;
        str->capacity = new_capacity;
    }
    memcpy(str->data + str->size, s, n);
    str->size += n;
    str->data[str->size] = '\0';
}

int cstr_pop(cstr* str) {
    if (str->size > 0) {
        str->size--;
        str->data[str->size] = '\0';
        return 0;
    }
    return -1;
}

void cstr_free(cstr* str) {
    if(str == NULL) { return; }
    if(str->data == NULL) { return; }

    free(str->data);
    str->data = NULL;
    str->size = 0;
    str->capacity = 0;
}

int cstr_to_int(cstr* str) {
    return atoi(str->data);
}

int cstr_copy(cstr* dest, const cstr* src) {
    if (dest->capacity < src->size + 1) {
        char* new_data = realloc(dest->data, src->size + 1);
        if (!new_data) {
            return -1;
        }
        dest->data = new_data;
        dest->capacity = src->size + 1;
    }
    memcpy(dest->data, src->data, src->size + 1);
    dest->size = src->size;
    return 0;
}

int cstr_substring(cstr* dest, const cstr* src, size_t start, size_t length) {
    if (start >= src->size) {
        return -1;
    }
    size_t actual_length = (start + length > src->size) ? (src->size - start) : length;
    if (dest->capacity < actual_length + 1) {
        char* new_data = realloc(dest->data, actual_length + 1);
        if (!new_data) {
            return -1;
        }
        dest->data = new_data;
        dest->capacity = actual_length + 1;
    }
    memcpy(dest->data, src->data + start, actual_length);
    dest->data[actual_length] = '\0';
    dest->size = actual_length;
    return 0;
}

char *cstr_begin(cstr* str) {
    return str->data;
}

char *cstr_end(cstr* str) {
    return str->data + str->size;
}

void cstr_remove(cstr* str, size_t start, size_t length) {
    if (start >= str->size || length == 0) {
        return;
    }
    size_t actual_length = (start + length > str->size) ? (str->size - start) : length;
    memmove(str->data + start, str->data + start + actual_length, str->size - (start + actual_length) + 1);
    str->size -= actual_length;
}