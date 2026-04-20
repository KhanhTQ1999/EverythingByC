#pragma once

#include "typedef.h"

i32 find_charactor_in_string(char *s, char c);
char *find_last_char_in_string(char *s, char c);
b8 is_numeric_string(char *s);
void throw_error(Errno err, char *msg);
void catch_error();
i32 get_input(char *raw, u32 size);
i32 str2num(char *s);
