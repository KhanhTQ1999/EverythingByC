#pragma once
#include "typedef.h"
#include "cstr.h"

u32 skip_whitespace(cstr *input);
b8 is_digit(char c);
u32 find_executable_path(const char *command_name, cstr *full_path);