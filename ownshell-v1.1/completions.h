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

typedef struct completer_script {
    cstr script; // The script to be executed for completion
    cstr command; // The command to be completed
    StrList args; // Arguments for the completion script
} CompleterScript;

typedef struct{
    CompleterScript *data;
    u32 size;
    u32 capacity;
}CompleterScriptList;

StrList complete_command_word(cstr *input);
CompleterScript* get_completer_script(const cstr *command);
i32 register_completer_script(const cstr *script, const cstr *command);