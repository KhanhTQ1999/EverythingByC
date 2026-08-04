#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include "dispose.h"

static void parse_str_tok(Command *cmd, TokenList *toks, u32 idx);
static void parse_redirfd_tok(Command *cmd, TokenList *toks, u32 idx);
static void parse_redirout_tok(Command *cmd, TokenList *toks, u32 idx);
static void parse_redirout_append_tok(Command *cmd, TokenList *toks, u32 idx);
static void parse_conn_tok(Command *cmd, TokenList *toks, u32 idx);

Command* parse(TokenList *tokens, u32 idx){
    if(tokens == NULL){
        return NULL;
    }

    Command *root = malloc(sizeof(Command));
    if(root == NULL){
        return NULL;
    }

    Command *cur = malloc(sizeof(Command));
    if(cur == NULL){
        free(root);
        return NULL;
    }

    memset(root, 0, sizeof(Command));
    memset(cur, 0, sizeof(Command));

    for(u32 i = idx; i < tokens->size; ++i){
        Token token = tokens->data[i];
        switch(token.type){
            case TOKEN_STRING:
                // Create a new simple command
                parse_str_tok(cur, tokens, i);
                break;
            case TOKEN_REDIRECT_FD:
                parse_redirfd_tok(cur, tokens, i);
                i += 2; // Skip the next two tokens as they are part of the redirection
                break;
            case TOKEN_REDIRECT_OUTPUT:
            case TOKEN_REDIRECT_OUTPUT_APPEND:
                parse_redirout_tok(cur, tokens, i);
                ++i; // Skip the next token as it's part of the redirection
                break;
            case TOKEN_AMPERSAND:
            case TOKEN_PIPE:
                if(cur->type == CM_NONE){
                    free(cur);
                    free(root);
                    return NULL;
                }
                root->value.connection.left = cur;
                parse_conn_tok(root, tokens, i);
                return root;
            default:
                break;
        }
    }

    free(root);
    return cur;
}

static void parse_str_tok(Command *cmd, TokenList *toks, u32 idx){
    cmd->type = CM_SIMPLE;
    cstr token_value = {0};
    cstr_copy(&token_value, &toks->data[idx].value);
    vec_append(&cmd->value.simple, token_value);
}

static void parse_redirfd_tok(Command *cmd, TokenList *toks, u32 idx){
    if(idx > toks->size - 2 || toks == NULL 
        || (toks->data[idx + 1].type != TOKEN_REDIRECT_OUTPUT
            && toks->data[idx + 1].type != TOKEN_REDIRECT_OUTPUT_APPEND)
            || toks->data[idx + 2].type != TOKEN_STRING
    ){
        return;
    }
    REDIRECT redir;
    memset(&redir, 0, sizeof(REDIRECT));

    redir.fd = atoi(toks->data[idx].value.data);
    if(toks->data[idx + 1].type == TOKEN_REDIRECT_OUTPUT){
        redir.truncated = 1;
    } else if(toks->data[idx + 1].type == TOKEN_REDIRECT_OUTPUT_APPEND){
        redir.truncated = 0;
    }

    cstr_copy(&redir.filename, &toks->data[idx + 2].value);
    vec_append(&cmd->redirs, redir);
}

static void parse_redirout_tok(Command *cmd, TokenList *toks, u32 idx){
    if(idx > toks->size - 1 || toks == NULL
            || toks->data[idx + 1].type != TOKEN_STRING
    ){
        return;
    }
    
    REDIRECT redir;
    memset(&redir, 0, sizeof(REDIRECT));

    redir.fd = STDOUT_FILENO;
    if(toks->data[idx].type == TOKEN_REDIRECT_OUTPUT){
        redir.truncated = 1;
    } else if(toks->data[idx].type == TOKEN_REDIRECT_OUTPUT_APPEND){
        redir.truncated = 0;
    }
    cstr_copy(&redir.filename, &toks->data[idx + 1].value);
    vec_append(&cmd->redirs, redir);
}

static void parse_conn_tok(Command *cmd, TokenList *toks, u32 idx){
    if(idx > toks->size - 1 || toks == NULL
            || (toks->data[idx].type != TOKEN_PIPE
                && toks->data[idx].type != TOKEN_AND_AND
                && toks->data[idx].type != TOKEN_OR
                && toks->data[idx].type != TOKEN_AMPERSAND)
    ){
        return;
    }

    cmd->type = CM_CONNECTION;
    switch(toks->data[idx].type){
        case TOKEN_PIPE:
            cmd->value.connection.type = CONN_PIPE;
            break;
        case TOKEN_AND_AND:
            cmd->value.connection.type = CONN_AND;
            break;
        case TOKEN_OR:
            cmd->value.connection.type = CONN_OR;
            break;
        case TOKEN_AMPERSAND:
            cmd->value.connection.type = CONN_AMPERSAND;
            break;
        default:
            break;
    }
    cmd->value.connection.right = parse(toks, idx + 1);
}