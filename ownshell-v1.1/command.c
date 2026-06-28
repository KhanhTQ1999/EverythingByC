#include "command.h"
#include "utils.h"

b8 scanner_end(Scanner *scanner) {
    return scanner->pos >= scanner->raw.size;
}

char scanner_next(Scanner *scanner) {
    if (scanner_end(scanner)) {
        return '\0';
    }
    return scanner->raw.data[scanner->pos++];
}


/*
 * Precedence:
 *   lowest  : ||
 *   middle  : &&
 *   highest : |   (pipe)
 *
 * parse_expr  → handles ||
 * parse_and   → handles &&
 * parse_pipe  → handles |
 * parse_atom  → handles a simple command
 */

b8 is_special_char(char c) {
    return c == '|' || c == '&' || c == '<' || c == '>';
}

b8 is_connector_char(char c) {
    return c == '|' || c == '&';
}

b8 is_redirect_char(char c) {
    return c == '<' || c == '>';
}

Command parse_simple(Scanner *scanner) {
    Command cmd = {0};
    cmd.type = CM_SIMPLE;
    while(scanner_end(scanner) == false) {
        char c = scanner_next(scanner);
        switch (c) {
            case '\0':
            case '\n':
                break;
            case ' ':
                break;
            case '\'':
                break;
            case '"':
                break;
            case '\\':
                break;
            case '>':
                break;
            case '<':
                break;
            default:
                break;
        }
    }

    return cmd;
}

Command parse_atom(Scanner *scanner) {
    return parse_simple(scanner);
}

Command parse_pipe(Scanner *scanner) {
    Command left = parse_atom(scanner);
    return left;
}

Command parse_and(Scanner *scanner){
    Command left = parse_pipe(scanner);
    return left;
}

Command parse_expr(Scanner *scanner){
    Command left = parse_and(scanner);

    while (1) {

    }
    return left;
}

Command parse_command(cstr *input) {
    Scanner scanner = { .raw = {0}, .pos = 0 };
    u32 st = skip_whitespace(input);
    cstr_substr(&scanner.raw, input, st, input->size - st);
    Command cmd = parse_expr(&scanner);
    cstr_free(&scanner.raw);
    return cmd;
}