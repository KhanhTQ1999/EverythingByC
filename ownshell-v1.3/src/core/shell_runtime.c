#include "../../inc/types.h"
#include "../../inc/core/shell.h"

int lex_tokanize(struct Cstr *input, struct Vec *tok_list) {
    (void)input;
    (void)tok_list;
    return 0;
}

int parse_tokens(struct Vec *tok_list, Ast *ast) {
    (void)tok_list;
    ast_init(ast);
    return 0;
}

int execute_ast(Ast *ast) {
    (void)ast;
    return 0;
}
