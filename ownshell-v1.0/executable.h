#pragma once

#include "typedef.h"

i32 search_executable(s8 *cmd, s8 *name);
u32 complete_executable(s8_list *candidates, char *buffer, u32 buffer_len);