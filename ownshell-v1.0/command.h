#pragma once

#include "typedef.h"
#include "command_types.h"
#include "builtin.h"
#include "executable.h"

void command_list_free(CommandList *cmd_list);
u32 complete_command(s8_list *candidates, char *buffer, u32 buffer_len);