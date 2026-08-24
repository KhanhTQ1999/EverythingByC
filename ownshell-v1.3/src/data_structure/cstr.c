#include <stdint.h>
#include "../../inc/data_structure/cstr.h"

#define CSTR_INITIAL_CAPACITY 4

void cstr_init(struct Cstr *self) {
    if (self == NULL) {
        return;
    }

    self->data = NULL;
    self->len = 0;
    self->cap = 0;
}

int cstr_append(struct Cstr *self, const char *str) {
    if (self == NULL || str == NULL) {
        return CSTR_ERR_NULL;
    }

    return cstr_appendn(self, str, strlen(str));
}

int cstr_appendn(struct Cstr *self, const char *str, size_t n) {
    if(self == NULL || str == NULL) return CSTR_ERR_NULL;
    if(n == 0) return CSTR_SUCCESS;
    if(n > SIZE_MAX - self->len - 1) return CSTR_ERR_OUT_OF_BOUNDS;
    
    size_t new_size = self->len + n + 1;
    if(new_size >= self->cap) {
        size_t new_cap = self->cap == 0 ? CSTR_INITIAL_CAPACITY : self->cap;
        while(new_size > new_cap) {
            new_cap *= 2;
        }
        char *new_data = (char *)realloc(self->data, new_cap);
        if(new_data == NULL) return CSTR_ERR_ALLOC;
        self->data = new_data;
        self->cap = new_cap;
    }

    memcpy(self->data + self->len, str, n);
    self->len += n;
    self->data[self->len] = '\0';
    return CSTR_SUCCESS;
}

int cstr_pop(struct Cstr *self) {
    if(self == NULL || self->data == NULL) { return CSTR_ERR_NULL; }
    if(self->len == 0) { return CSTR_ERR_OUT_OF_BOUNDS; }

    self->len--;
    self->data[self->len] = '\0';
    return CSTR_SUCCESS;
}

int cstr_copy(struct Cstr *self, const char *str, size_t n) {
    if(self == NULL || str == NULL) { return CSTR_ERR_NULL; }
    if(n == 0) {
        if(self->data) {
            self->data[0] = '\0';
        }
        self->len = 0;
        return CSTR_SUCCESS; 
    }
    if(n > SIZE_MAX - 1) { return CSTR_ERR_OUT_OF_BOUNDS; }

    size_t new_size = n + 1;

    if(new_size > self->cap) {
        size_t new_cap = self->cap == 0 ? CSTR_INITIAL_CAPACITY : self->cap;
        while(new_size > new_cap) {
            new_cap *= 2;
        }
        char *new_data = (char *)realloc(self->data, new_cap);
        if(new_data == NULL) { return CSTR_ERR_ALLOC; }
        self->data = new_data;
        self->cap = new_cap;
    }

    memcpy(self->data, str, n);
    self->len = n;
    self->data[self->len] = '\0';
    return CSTR_SUCCESS;
}

char cstr_at(struct Cstr *self, size_t index) {
    if(self == NULL || index >= self->len) { return '\0'; }

    return self->data[index];
}

int cstr_substr(struct Cstr *self, size_t start, size_t end, struct Cstr *out) {
    if(self == NULL || self->data == NULL || out == NULL) { 
        return CSTR_ERR_NULL; 
    }
    
    if(start >= self->len || end > self->len || start >= end) { 
        return CSTR_ERR_OUT_OF_BOUNDS; 
    }

    size_t substr_len = end - start;
    int ret = cstr_copy(out, self->data + start, substr_len);
    if(ret != CSTR_SUCCESS) { return ret; }

    return CSTR_SUCCESS;
}

int cstr_reverse(struct Cstr *self) {
    if(self == NULL || self->data == NULL) { return CSTR_ERR_NULL; }

    for(size_t i = 0; i < self->len / 2; i++) {
        char temp = self->data[i];
        self->data[i] = self->data[self->len - 1 - i];
        self->data[self->len - 1 - i] = temp;
    }

    return CSTR_SUCCESS;
}

int cstr_remove(struct Cstr *self, size_t start, size_t len) {
    if(self == NULL || self->data == NULL) { return CSTR_ERR_NULL; }
    if(start >= self->len || start + len > self->len) { return CSTR_ERR_OUT_OF_BOUNDS; }

    memmove(self->data + start, self->data + start + len, self->len - (start + len));
    self->len -= len;
    self->data[self->len] = '\0';
    return CSTR_SUCCESS;
}

int cstr_clear(struct Cstr *self) {
    if(self == NULL || self->data == NULL) { return CSTR_ERR_NULL; }
    self->len = 0;
    self->data[0] = '\0';
    return CSTR_SUCCESS;
}

int cstr_free(struct Cstr *self) {
    if(self == NULL) { return CSTR_ERR_NULL; }
    if(self->data != NULL) {
        free(self->data);
        self->data = NULL;
    }
    self->len = 0;
    self->cap = 0;
    return CSTR_SUCCESS;
}