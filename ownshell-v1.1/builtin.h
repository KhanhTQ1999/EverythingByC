#pragma once

#include "parser.h"

u32 execute_builtin(SimpleCommand *args, int async);
StrList find_builtin(cstr *hint);