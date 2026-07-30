#pragma once

#include "typedef.h"
#include "strlist.h"
#include "cstr.h"
#include "vec.h"

#define HISTORY_APPEND 0
#define HISTORY_TRUNCATE 1

i32 history_add(cstr *entry);
i32 history_get(u32 idx, cstr *entry);
u32 history_size();
u32 history_load(cstr *file_path);
u32 history_save(cstr *file_path);
u32 history_append(cstr *file_path);
u32 history_startup();
u32 history_exit();
