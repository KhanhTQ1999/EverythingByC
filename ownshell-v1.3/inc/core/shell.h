#pragma once

#include "../types.h"

enum shell_state {
    SHELL_STATE_INIT = 0,
    SHELL_STATE_INTERACTIVE,
    SHELL_STATE_LEXING,
    SHELL_STATE_PARSING,
    SHELL_STATE_EXECUTING,
    SHELL_STATE_CLEANUP,
    SHELL_STATE_ERROR,
    SHELL_STATE_EXIT,
    SHELL_STATE_COUNT
};

struct shell_ctx{
    enum shell_state current_state;
    enum shell_state prev_state;

    // Data
    struct Vec tok_list;
    struct Cstr input_buf;
    Ast ast;

    //State
    b8 is_running;
    i32 exit_code;
};

void shell_context_init(struct shell_ctx *self);
void shell_context_free(struct shell_ctx *self);

i32 shell_run(struct shell_ctx *ctx);

int lex_tokanize(struct Cstr *input, struct Vec *tok_list);
int parse_tokens(struct Vec *tok_list, Ast *ast);
int execute_ast(Ast *ast);