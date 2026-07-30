#pragma once

#include "typedef.h"
#include "vec.h"
#include "cstr.h"

enum token_type {
    TOKEN_STRING,
    TOKEN_REDIRECT_FD,
    TOKEN_REDIRECT_OUTPUT,
    TOKEN_REDIRECT_OUTPUT_APPEND,
    TOKEN_PIPE,
    TOKEN_OR,
    TOKEN_AMPERSAND,
    TOKEN_AND_AND,
};

typedef struct {
    enum token_type type;
    cstr value;
} Token;

typedef struct {
    Token *data;
    u32 size;
    u32 capacity;
} TokenList;

i32 lex(cstr *input, TokenList *tokens);
void token_copy(Token *dest, const Token *src);
void tokens_list_copy(TokenList *dest, const TokenList *src);