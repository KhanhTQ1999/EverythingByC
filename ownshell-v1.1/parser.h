#pragma once

#include "typedef.h"
#include "cstr.h"
#include "vec.h"
#include "strlist.h"
#include "lexer.h"
#include "redir.h"

enum connector_type {
    CONN_PIPE,
    CONN_AND,
    CONN_OR,
    CONN_AMPERSAND
};

enum command_type {
    CM_NONE = 0,
    CM_SIMPLE,
    CM_CONNECTION
};

typedef struct str_list SimpleCommand;

typedef struct connection {
    enum connector_type type;
    struct command *left;
    struct command *right;
} Connection;

typedef struct command {
    enum command_type type;
    struct redirect_list redirs;
    union {
        Connection connection;
        SimpleCommand simple;
    } value;
} Command;

Command* parse(TokenList *tokens, u32 idx);