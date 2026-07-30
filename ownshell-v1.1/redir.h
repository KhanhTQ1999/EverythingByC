#pragma once

#include "typedef.h"
#include "cstr.h"

typedef struct redirect {
    u32 fd;
    cstr filename;
    b8 truncated;
} REDIRECT;

typedef struct redirect_list {
    REDIRECT *data;
    u32 size;
    u32 capacity;
} REDIRECT_LIST;

void start_redirection(REDIRECT_LIST *redirs);
void end_redirection(REDIRECT_LIST *redirs);