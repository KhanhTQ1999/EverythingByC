#pragma once

typedef struct ast {
    struct ast *right;
    struct ast *left;
} Ast;

void ast_init(struct ast *self);
void ast_clear(struct ast *self);
void ast_free(struct ast *self);