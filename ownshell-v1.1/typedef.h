#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int8_t i8;
typedef uint8_t u8;
typedef int64_t i64;
typedef uint64_t u64;
typedef bool b8;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))