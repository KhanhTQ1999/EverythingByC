#pragma once

#include <stdbool.h>

typedef struct {
    i32 fd;
    s8 filename;
    bool truncated;
} Redirect;

typedef struct {
    Redirect *data;
    u32 size;
    u32 capacity;
} RedirectList;

typedef struct {
    s8 *data;
    u32 size;
    u32 capacity;
} ArgList;

typedef struct {
    ArgList args;
    RedirectList redirects;
} Command;

typedef struct {
    Command *data;
    i32 size;
    i32 capacity;
} CommandList;