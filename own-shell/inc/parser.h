#pragma once

#include "typedef.h"

/**
 * Parse command line input into arguments and redirect information
 * 
 * Supports:
 * - Space-separated arguments
 * - Single quotes (preserve spaces)
 * - Double quotes (preserve spaces)
 * - Backslash escaping
 * - Output redirect: > file (stdout)
 * - FD redirect: 1> file (stdout), 2> file (stderr), etc.
 * 
 * Examples:
 * - "echo Hello World" -> argc=3, arguments=["echo", "Hello", "World"]
 * - "echo \"Hello World\"" -> argc=2, arguments=["echo", "Hello World"]
 * - "echo text > file.txt" -> argc=2, arguments=["echo", "text"], redirect_fd=1, redirect_file="file.txt"
 * - "ls 2> errors.txt" -> argc=1, arguments=["ls"], redirect_fd=2, redirect_file="errors.txt"
 * 
 * Returns 0 on success, -1 on error
 */
i32 parse_command(char *raw, u32 raw_size, Command *command);