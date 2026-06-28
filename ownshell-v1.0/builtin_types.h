#pragma once

#include "typedef.h"
#include "command_types.h"

typedef void (*BuiltinHandler)(ArgList *args);

typedef struct {
  const char *name;
  BuiltinHandler handler;
} Builtin;