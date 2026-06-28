#pragma once

#include "typedef.h"
#include "builtin_types.h"

BuiltinHandler get_builtin_handler(s8 *name);
u32 complete_builtin(s8_list *candidates, char *buffer, u32 buffer_len);