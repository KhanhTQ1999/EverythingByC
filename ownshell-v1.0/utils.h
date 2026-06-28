#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "command_types.h"

void redirect_start(RedirectList *redirects);
void redirect_end(RedirectList *redirects);
void disableRawMode();
void enableRawMode();
bool is_digit(char c);
int stoi(const char *str);