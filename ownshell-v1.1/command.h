#pragma once

#include "typedef.h"
#include "cstr.h"

enum connector_type {
    CONN_PIPE,
    CONN_AND,
    CONN_OR
};

/* Command Types: */
enum command_type { 
    CM_SIMPLE,
    CM_CONNECTION
};

typedef struct redirect {
    u32 fd;
    cstr filename;
    b8 truncated;
} REDIRECT;

typedef struct simple_com {
    cstr args;
    u32 argc;
} SimpleCommand;

typedef struct connection {
    enum connector_type type;
    struct command *left;
    struct command *right;
} Connection;

typedef struct command {
    enum command_type type;
    REDIRECT *redirects;
    union {
        struct connection *connection;
        struct simple_com *simple;
    } value;
} Command;

enum scanner_state {
    SCAN_NORMAL,
    SCAN_IN_SINGLE_QUOTE,
    SCAN_IN_DOUBLE_QUOTE,
    SCAN_IN_BACKSLASH,
    SCAN_IN_DIGIT,
};
typedef struct {
    cstr raw;
    enum scanner_state state;
    enum scanner_state prev_state;
    u32 pos;
} Scanner;

Command parse_command(cstr *input);