#include "lexer.h"
#include <stdio.h>
#include "utils.h"
#include "dispose.h"

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
    Token current_token;
    TokenList tokens;
} Scanner;

static void s_set_scanner_state(Scanner *scanner, enum scanner_state state) {
    scanner->prev_state = scanner->state;
    scanner->state = state;
}

static b8 s_scanner_end(Scanner *scanner) {
    return scanner->pos >= scanner->raw.size;
}

static char s_scanner_next(Scanner *scanner) {
    if (s_scanner_end(scanner)) {
        return '\0';
    }
    return scanner->raw.data[scanner->pos++];
}

static char s_scanner_back(Scanner *scanner) {
    if (scanner->pos == 0) {
        return '\0';
    }
    return scanner->raw.data[--scanner->pos];
}

static char s_scanner_peek(Scanner *scanner) {
    if (s_scanner_end(scanner)) {
        return '\0';
    }
    return scanner->raw.data[scanner->pos];
}

static void s_scanner_reset_current_token(Scanner *scanner) {
    scanner->current_token = (Token){0};
}

static void s_scanner_make_token(Scanner *scanner, enum token_type type) {
    scanner->current_token.type = type;
    if(type == TOKEN_REDIRECT_OUTPUT || type == TOKEN_REDIRECT_OUTPUT_APPEND 
        || type == TOKEN_REDIRECT_FD || type == TOKEN_PIPE || type == TOKEN_AMPERSAND
        || scanner->current_token.value.size > 0) {
        vec_append(&scanner->tokens, scanner->current_token);
        s_scanner_reset_current_token(scanner);
    }
}

static void s_scanner_cleanup(Scanner *scanner) {
    dispose_token_list(&scanner->tokens);
    dispose_token(&scanner->current_token);
    cstr_free(&scanner->raw);
}

static void s_lex_default(Scanner *scanner) {
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
        s_set_scanner_state(scanner, SCAN_IN_SINGLE_QUOTE);
        return;
    case '"':
        s_set_scanner_state(scanner, SCAN_IN_DOUBLE_QUOTE);
        return;
    case '\\':
        s_set_scanner_state(scanner, SCAN_IN_BACKSLASH);
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
    case '&':
        s_scanner_make_token(scanner, TOKEN_AMPERSAND);
        return;
    default:
        if(is_digit(c)){
            s_set_scanner_state(scanner, SCAN_IN_DIGIT);
            s_scanner_back(scanner);
        }else{
            cstr_appendn(&scanner->current_token.value, &c, 1);
        }
        break;
    }

    if(s_scanner_end(scanner)){
        s_scanner_make_token(scanner, TOKEN_STRING);
    }
}

static void s_lex_double_quote(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    switch (c)
    {
    case '"':
        s_set_scanner_state(scanner, SCAN_NORMAL);
        break;
    
    case '\\':
        s_set_scanner_state(scanner, SCAN_IN_BACKSLASH);
        break;

    default:
        cstr_appendn(&scanner->current_token.value, &c, 1);
        break;
    }
}

static void s_lex_single_quote(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    switch (c)
    {
    case '\'':
        s_set_scanner_state(scanner, SCAN_NORMAL);
        break;

    default:
        cstr_appendn(&scanner->current_token.value, &c, 1);
        break;
    }
}

static void s_lex_number(Scanner *scanner){
    char c = s_scanner_next(scanner);
    while(is_digit(c)){
        cstr_appendn(&scanner->current_token.value, &c, 1);
        c = s_scanner_next(scanner);
    }

    if(s_scanner_end(scanner)){
        s_scanner_make_token(scanner, TOKEN_STRING);
        return;
    }

    if(c == '>'){
        s_scanner_make_token(scanner, TOKEN_REDIRECT_FD);
    }

    s_scanner_back(scanner);
    s_set_scanner_state(scanner, SCAN_NORMAL);
}

static void s_lex_backslash(Scanner *scanner) {
    char c = s_scanner_next(scanner);
    if (c != '\0') {
        cstr_appendn(&scanner->current_token.value, &c, 1);
    }
    scanner->state = scanner->prev_state;
}

i32 lex(cstr *input, TokenList *tokens) {
    Scanner scanner = {
        .state = SCAN_NORMAL,
        .raw = {0},
        .pos = 0,
        .current_token = {0},
        .tokens = {0},
    };

    /* Skip leading whitespace and initialize the scanner with the input string */
    u32 st = skip_whitespace(input);
    cstr_substring(&scanner.raw, input, st, input->size - st);

    while(!s_scanner_end(&scanner)){
        switch (scanner.state)
        {
            case SCAN_NORMAL:           s_lex_default(&scanner); break;
            case SCAN_IN_DOUBLE_QUOTE:  s_lex_double_quote(&scanner); break;
            case SCAN_IN_SINGLE_QUOTE:  s_lex_single_quote(&scanner); break;
            case SCAN_IN_BACKSLASH:     s_lex_backslash(&scanner); break;
            case SCAN_IN_DIGIT:         s_lex_number(&scanner); break;
            default: break;
        }
    }

    tokens_list_copy(tokens, &scanner.tokens);
    s_scanner_cleanup(&scanner);
    return 0; // Success
}

void token_copy(Token *dest, const Token *src) {
    dest->type = src->type;
    if(src->value.data != NULL && src->value.size > 0) {
        cstr_copy(&dest->value, &src->value);
    }
}

void tokens_list_copy(TokenList *dest, const TokenList *src) {
    for(u32 i = 0; i < src->size; ++i){
        Token tok = {0};
        token_copy(&tok, &src->data[i]);
        vec_append(dest, tok);
    }
}