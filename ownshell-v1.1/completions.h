#pragma once

#include "typedef.h"
#include "cstr.h"
#include "strlist.h"

enum comp_type {
    COMPLETION_NONE = 0,
    COMPLETION_COMMAND_WORD = 1,
    COMPLETION_ARGUMENT = 2
};

typedef struct comp_context {
    cstr *input; // The input string being completed
    u32 hint_index; // Index of the hint in the input string
    enum comp_type type; // 0: no context, 1: command word, 2: argument
} CompContext;

StrList complete_command_word(cstr *input);