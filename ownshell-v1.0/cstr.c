#include "vec.h"
#include "cstr.h"

void s8_create_by_cptr(s8* str, const char* cptr) {
    size_t len = strlen(cptr);
    str->data = malloc(len + 1);
    assert(str->data != NULL);
    memcpy(str->data, cptr, len);
    str->data[len] = '\0';
    str->size = len;
    str->capacity = len + 1;
}

void s8_create_by_another(s8* str, const s8* src) {
    str->data = malloc(src->capacity);
    assert(str->data != NULL);
    memcpy(str->data, src->data, src->size);
    str->data[src->size] = '\0';
    str->size = src->size;
    str->capacity = src->capacity;
}

void s8_append(s8* str, const char* s){
    size_t len = strlen(s);
    /* Ensure there is room for len bytes plus terminating NUL */
    size_t needed = str->size + len + 1;
    if (str->capacity < needed) {
        size_t new_capacity = (str->capacity == 0) ? VECTOR_INITIAL_CAPACITY : str->capacity;
        while (new_capacity < needed) new_capacity <<= 1;
        char *new_data = realloc(str->data, new_capacity);
        assert(new_data != NULL);
        str->data = new_data;
        str->capacity = new_capacity;
    }
    /* Copy the bytes and maintain NUL termination */
    memcpy(str->data + str->size, s, len);
    str->size += len;
    str->data[str->size] = '\0';
}

void s8_appendn(s8* str, const char* s, size_t n){
    size_t len = strnlen(s, n);
    /* Ensure there is room for len bytes plus terminating NUL */
    size_t needed = str->size + len + 1;
    if (str->capacity < needed) {
        size_t new_capacity = (str->capacity == 0) ? VECTOR_INITIAL_CAPACITY : str->capacity;
        while (new_capacity < needed) new_capacity <<= 1;
        char *new_data = realloc(str->data, new_capacity);
        assert(new_data != NULL);
        str->data = new_data;
        str->capacity = new_capacity;
    }
    /* Copy the bytes and maintain NUL termination */
    memcpy(str->data + str->size, s, len);
    str->size += len;
    str->data[str->size] = '\0';
}

int s8_to_int(s8* str){
    int num = 0;
    for(size_t i=0; i<str->size; i++){
        if(str->data[i] < '0' || str->data[i] > '9') break;
        num = num * 10 + (str->data[i] - '0');
    }
    return num;
}

void s8_free(s8* str){
    if(str->data) {
        free(str->data);
        str->data = NULL;
    }
    str->size = 0;
    str->capacity = 0;
}

int s8_copy(s8* dest, const s8* src) {
    if (dest->capacity < src->size + 1) {
        char* new_data = realloc(dest->data, src->size + 1);
        if (new_data == NULL) {
            return -1; // Allocation failed
        }
        dest->data = new_data;
        dest->capacity = src->size + 1;
    }
    memcpy(dest->data, src->data, src->size);
    dest->data[src->size] = '\0';
    dest->size = src->size;
    return 0;
}

int s8_substring(s8* dest, const s8* src, size_t start, size_t length) {
    if (start >= src->size) {
        return -1; // Start index out of bounds
    }
    size_t actual_length = (start + length > src->size) ? (src->size - start) : length;
    if (dest->capacity < actual_length + 1) {
        char* new_data = realloc(dest->data, actual_length + 1);
        if (new_data == NULL) {
            return -1; // Allocation failed
        }
        dest->data = new_data;
        dest->capacity = actual_length + 1;
    }
    memcpy(dest->data, src->data + start, actual_length);
    dest->data[actual_length] = '\0';
    dest->size = actual_length;
    return 0;
}

void s8_list_free(s8_list* list) {
    for(size_t i=0; i<list->size; i++){
        s8_free(&list->data[i]);
    }
    vector_free(list);
}