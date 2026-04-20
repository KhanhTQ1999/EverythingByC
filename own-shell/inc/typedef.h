#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b
#define IN_LEN       1024
#define PATH_LEN_MAX 1024
#define ERRMSG_LEN   256
#define ARGC_MAX     20
#define REDIRECT_FILE_MAX 10

typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint8_t     u8;
typedef int64_t     i64;
typedef int32_t     i32;
typedef int8_t      i8;
typedef bool        b8;

typedef enum {
    E_SUCCESS = 0,
    E_INPUT_TOO_LONG,
    E_UNKNOWN_COMMAND,
    E_INVALID_PARAMETER,
    E_SYNTAX_ERROR,
    E_EOF
} Errno;

typedef struct {
  char *redirect_file[REDIRECT_FILE_MAX];
  i32 redirect_fd[REDIRECT_FILE_MAX];
  u32 redirect_count;
} Redirect;

typedef struct{
    u32 argc;
    char buffer[IN_LEN+1];
    char *arguments[ARGC_MAX];
    Redirect redirect;
} Command;

typedef void (*BuildinHandler)(Command *cmd_ptr);

typedef struct {
  const char *name;
  BuildinHandler handler;
} Buildin;