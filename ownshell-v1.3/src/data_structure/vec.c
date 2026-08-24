#include "../../inc/data_structure/vec.h"

#define VECTOR_INITIAL_CAPACITY 4

int vec_init(struct Vec *self, size_t size, size_t esize) {
    if(self == NULL || esize == 0) { return VEC_ERR_NULL; }

    self->data = malloc(VECTOR_INITIAL_CAPACITY * esize);
    if(self->data == NULL) { return VEC_ERR_ALLOC; }
    self->esize = esize;
    self->size = size;
    self->cap = VECTOR_INITIAL_CAPACITY;
    return VEC_SUCCESS;
}

int vec_push(struct Vec *self, void *item) {
    if(self == NULL || item == NULL) { return VEC_ERR_NULL; }

    if(self->size >= self->cap) {
        self->cap *= 2;
        void *new_data = realloc(self->data, self->cap * self->esize);
        if(new_data == NULL) { return VEC_ERR_ALLOC; }
        self->data = new_data;
    }

    memcpy((char *)self->data + (self->size * self->esize), item, self->esize);
    self->size++;
    return VEC_SUCCESS;
}

void* vec_at(struct Vec *self, size_t index) {
    if(self == NULL) { return NULL; }
    if(index >= self->size) { return NULL; }

    return (char *)self->data + (index * self->esize);
}

int vec_remove(struct Vec *self, size_t index) {
    if(self == NULL) { return VEC_ERR_NULL; }
    if(index >= self->size) { return VEC_ERR_OUT_OF_BOUNDS; }

    memmove((char *)self->data + (index * self->esize),
            (char *)self->data + ((index + 1) * self->esize),
            (self->size - index - 1) * self->esize);
    self->size--;
    return VEC_SUCCESS;
}

int vec_clear(struct Vec *self) {
    if(self == NULL) { return VEC_ERR_NULL; }

    self->size = 0;
    return VEC_SUCCESS;
}

int vec_free(struct Vec *self) {
    if(self == NULL) { return VEC_ERR_NULL; }
    if(self->data != NULL) {
        free(self->data);
        self->data = NULL;
    }
    self->size = 0;
    self->cap = 0;
    return VEC_SUCCESS;
}
