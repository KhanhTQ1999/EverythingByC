#pragma once

#include "typedef.h"
#include "vec.h"
#include "cstr.h"

typedef struct str_list {
    cstr *data;
    u32 size;
    u32 capacity;
} StrList;

#define strlist_init(strlist) \
    do { \
        (strlist)->data = NULL; \
        (strlist)->size = 0; \
        (strlist)->capacity = 0; \
    } while(0)

#define strlist_free(strlist) \
    do { \
        if((strlist)->data) { \
            for(u32 i = 0; i < (strlist)->size; i++) { \
                cstr_free(&(strlist)->data[i]); \
            } \
        } \
        vec_free((strlist)); \
    } while(0)

#define strlist_bubble_sort(strlist, cmp) \
    do { \
        for(u32 i = 0; i < (strlist)->size; i++) { \
            for(u32 j = 0; j < (strlist)->size - 1 - i; j++) { \
                if(cmp(&(strlist)->data[j], &(strlist)->data[j + 1]) > 0) { \
                    cstr temp = (strlist)->data[j]; \
                    (strlist)->data[j] = (strlist)->data[j + 1]; \
                    (strlist)->data[j + 1] = temp; \
                } \
            } \
        } \
    } while(0)

#define strlist_find(strlist, str) \
    ({ \
        int index = -1; \
        for(u32 i = 0; i < (strlist)->size; i++) { \
            if(strcmp((strlist)->data[i].data, (str)) == 0) { \
                index = i; \
                break; \
            } \
        } \
        index; \
    })