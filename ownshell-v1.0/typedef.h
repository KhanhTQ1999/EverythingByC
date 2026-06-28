#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "cstr.h"
#include "vec.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define SUCCESS 0
#define FAILURE 1

#define PATH_LEN_MAX 1024