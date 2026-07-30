#include <stdlib.h>
#include "dispose.h"

void dispose_command(Command *cmd) {
    if (cmd == NULL) {
        return;
    }

    if (cmd->type == CM_CONNECTION) {
        dispose_command(cmd->value.connection.left);
        dispose_command(cmd->value.connection.right);
    } else if (cmd->type == CM_SIMPLE) {
        for (size_t i = 0; i < cmd->value.simple.size; ++i) {
            cstr_free(&cmd->value.simple.data[i]);
        }
        vec_free(&cmd->value.simple);
    }

    for (size_t i = 0; i < cmd->redirs.size; ++i) {
        dispose_redir(&cmd->redirs.data[i]);
    }
    vec_free(&cmd->redirs);

    free(cmd);
}

void dispose_token_list(TokenList *tokens) {
    if (tokens == NULL) {
        return;
    }

    for (size_t i = 0; i < tokens->size; ++i) {
        dispose_token(&tokens->data[i]);
    }
    vec_free(tokens);
}

void dispose_token(Token *token) {
    if (token == NULL) {
        return;
    }
    cstr_free(&token->value);
}

void dispose_redir(REDIRECT *redir) {
    if (redir == NULL) {
        return;
    }
    cstr_free(&redir->filename);
}
