#include "parser.h"
#include "utils.h"

typedef enum { 
    STATE_DEFAULT,
    STATE_SINGLE_QUOTE,
    STATE_DOUBLE_QUOTE,
    STATE_BACKSLASH,
    STATE_NUMBER,
} ScannerState;

typedef enum {
    TOKEN_STRING,
    TOKEN_REDIRECT_FD,
    TOKEN_REDIRECT_OUTPUT,
    TOKEN_REDIRECT_OUTPUT_APPEND,
    TOKEN_PIPE,
} TokenType;

typedef struct {
    TokenType type;
    s8 str;
} Token;

typedef struct {
    Token *data;
    i32 size;
    i32 capacity;
} TokenList;

typedef struct {
    ScannerState state;
    ScannerState prev_state;
    s8 *raw;
    u32 pos;
    Token current_token;
    TokenList tokens;
} Scanner;

static void s_scanner_reset_current_token(Scanner *scanner) {
    scanner->current_token = (Token){0};
}

static i32 s_scanner_end(Scanner *scanner) {
    return scanner->pos >= scanner->raw->size;
}

static char s_scanner_next(Scanner *scanner) {
    if (s_scanner_end(scanner)) {
        return '\0';
    }
    return scanner->raw->data[scanner->pos++];
}

static char s_scanner_back(Scanner *scanner) {
    if (scanner->pos == 0) {
        return '\0';
    }
    return scanner->raw->data[--scanner->pos];
}

static char s_scanner_peek(Scanner *scanner){
    if (s_scanner_end(scanner)) {
        return '\0';
    }
    return scanner->raw->data[scanner->pos]; 
}

static void s_scanner_make_token(Scanner *scanner, TokenType type) {
    scanner->current_token.type = type;
    if(type == TOKEN_REDIRECT_OUTPUT || type == TOKEN_REDIRECT_OUTPUT_APPEND 
        || type == TOKEN_REDIRECT_FD || type == TOKEN_PIPE 
        || scanner->current_token.str.size > 0) {
        vector_append(&scanner->tokens, scanner->current_token);
        s_scanner_reset_current_token(scanner);
    }
}

static void s_scanner_cleanup(Scanner *scanner) {
    for(i32 i=0; i<scanner->tokens.size; i++){
        s8_free(&scanner->tokens.data[i].str);
    }
    vector_free(&scanner->tokens);
}

static void s_parse_default(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    char next_c;
    switch (c)
    {
    case '\0':
    case '\n':
        s_scanner_make_token(scanner, TOKEN_STRING);
        return;
    case ' ':
        s_scanner_make_token(scanner, TOKEN_STRING);
        return;
    case '\'':
        scanner->state = STATE_SINGLE_QUOTE;
        return;
    case '"':
        scanner->state = STATE_DOUBLE_QUOTE;
        return;
    case '\\':
        scanner->prev_state = STATE_DEFAULT;
        scanner->state = STATE_BACKSLASH;
        return;
    case '>':
        s_scanner_make_token(scanner, TOKEN_STRING);
        next_c = s_scanner_next(scanner);
        if(next_c == '>') {
            s_scanner_make_token(scanner, TOKEN_REDIRECT_OUTPUT_APPEND);
        } else {
            s_scanner_back(scanner);
            s_scanner_make_token(scanner, TOKEN_REDIRECT_OUTPUT);
        }
        return;
    case '|':
        s_scanner_make_token(scanner, TOKEN_STRING);
        s_scanner_make_token(scanner, TOKEN_PIPE);
        return;
    default:
        if(is_digit(c)){
            scanner->state = STATE_NUMBER;
            s_scanner_back(scanner);
        }else{
            s8_appendn(&scanner->current_token.str, &c, 1);
        }
        break;
    }
}

static void s_parse_single_quote(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    switch (c)
    {
    case '\'':
        // s_scanner_make_token(scanner, TOKEN_STRING);
        scanner->state = STATE_DEFAULT;
        break;
    
    // case '\\':
    //     scanner->prev_state = STATE_SINGLE_QUOTE;
    //     scanner->state = STATE_BACKSLASH;
    //     break;

    default:
        s8_appendn(&scanner->current_token.str, &c, 1);
        break;
    }
}

static void s_parse_double_quote(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    switch (c)
    {
    case '"':
        // s_scanner_make_token(scanner, TOKEN_STRING);
        scanner->state = STATE_DEFAULT;
        break;
    
    case '\\':
        scanner->prev_state = STATE_DOUBLE_QUOTE;
        scanner->state = STATE_BACKSLASH;
        break;

    default:
        s8_appendn(&scanner->current_token.str, &c, 1);
        break;
    }
}

static void s_parse_number(Scanner *scanner){
    char c = s_scanner_next(scanner);
    while(is_digit(c)){
        s8_appendn(&scanner->current_token.str, &c, 1);
        c = s_scanner_next(scanner);
    }
    if(c == '>'){
        s_scanner_make_token(scanner, TOKEN_REDIRECT_FD);
    }
    s_scanner_back(scanner);
    scanner->state = STATE_DEFAULT;
}

static void s_parse_backslash(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    if (c != '\0') {
        s8_appendn(&scanner->current_token.str, &c, 1);
    }
    scanner->state = scanner->prev_state;
}

i32 parse_commands(s8 *input, CommandList *cmd_list) {
    Scanner scanner = {
        .state = STATE_DEFAULT,
        .raw = input,
        .pos = 0,
        .current_token = {0},
        .tokens = {0},
    };

    while(!s_scanner_end(&scanner)){
        switch (scanner.state)
        {
        case STATE_DEFAULT:
            s_parse_default(&scanner);
            break;
        
        case STATE_DOUBLE_QUOTE:
            s_parse_double_quote(&scanner);
            break;

        case STATE_SINGLE_QUOTE:
            s_parse_single_quote(&scanner);
            break;
        
        case STATE_BACKSLASH:
            s_parse_backslash(&scanner);
            break;
        
        case STATE_NUMBER:
            s_parse_number(&scanner);
        default:
            break;
        }
    }

    Command cmd = {0};
    for(i32 i=0; i<scanner.tokens.size; i++){
        Token token = scanner.tokens.data[i];

        if(token.type == TOKEN_STRING){
            s8 arg = {0};
            s8_copy(&arg, &token.str);
            vector_append(&cmd.args, arg);
        }else if(token.type == TOKEN_REDIRECT_OUTPUT || token.type == TOKEN_REDIRECT_OUTPUT_APPEND){
            Redirect redirect = {0};
            redirect.truncated = (token.type == TOKEN_REDIRECT_OUTPUT);
            if(i > 0 && scanner.tokens.data[i - 1].type == TOKEN_REDIRECT_FD){
                redirect.fd = s8_to_int(&scanner.tokens.data[i - 1].str);
            }else{
                redirect.fd = 1;
            }
            if(i + 1 < scanner.tokens.size && scanner.tokens.data[i + 1].type == TOKEN_STRING){
                s8_copy(&redirect.filename, &scanner.tokens.data[i + 1].str);
                i++;
            }
            vector_append(&cmd.redirects, redirect);
        }else if(token.type == TOKEN_PIPE){
            vector_append(cmd_list, cmd);
            cmd = (Command){0};
        }
    }

    vector_append(cmd_list, cmd);
    s_scanner_cleanup(&scanner);
    return SUCCESS;
}