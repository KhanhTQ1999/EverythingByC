#pragma once
#include "lexer.h"
#include "parser.h"

void dispose_command(Command *cmd);
void dispose_token_list(TokenList *tokens);
void dispose_token(Token *token);
void dispose_redir(REDIRECT *redir);