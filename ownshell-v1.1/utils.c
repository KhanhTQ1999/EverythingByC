#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

u32 skip_whitespace(cstr *input) {
    u32 count = 0;
    char *iter = cstr_begin(input);
    while(iter != cstr_end(input) && (*iter == ' ' || *iter == '\t')) {
        ++iter;
        ++count;
    }
    return count;
}

b8 is_digit(char c) {
    return c >= '0' && c <= '9';
}

u32 find_executable_path(const char *command_name, cstr *full_path) {
    // const char *paths[] = {"/bin/cat", "/usr/bin/cat", NULL};
    // for (int i = 0; paths[i] != NULL; i++) {
    //     if (access(paths[i], X_OK) == 0) {
    //         printf("[DEBUG] cat tồn tại tại: %s\n", paths[i]);
    //     } else {
    //         printf("[DEBUG] Không tìm thấy hoặc không có quyền thực thi: %s\n", paths[i]);
    //     }
    // }

    // const char *path_env = getenv("PATH");
    // if (path_env) {
    //     printf("[DEBUG] PATH hiện tại: %s\n", path_env);
    // } else {
    //     printf("[DEBUG] PATH chưa được set\n");
    // }

    // // Thử chạy which cat để in ra đường dẫn thực tế
    // printf("[DEBUG] Kết quả system(\"which cat\"):\n");
    // fflush(stdout);
    // system("which cat");
    // return 1;
    if (access(command_name, X_OK) == 0) {
        if (full_path) {
            cstr_append(full_path, command_name);
        }
        return 0; // Found in current directory or absolute path
    }

    const char *path_env = getenv("PATH");
    if (!path_env) {
        return 1; // PATH not set
    }

    char *path_copy_data = strdup(path_env);
    if (!path_copy_data) {
        return 1; // Memory allocation failed
    }

    char *token = strtok(path_copy_data, ":");
    while(token != NULL) {
        cstr dir = {0};
        cstr_append(&dir, token);
        cstr_append(&dir, "/");
        cstr_append(&dir, command_name);

        if (access(dir.data, X_OK) == 0) {
            if (full_path) {
                cstr_append(full_path, dir.data);
            }
            cstr_free(&dir);
            free(path_copy_data);
            return 0; // Found in PATH
        }

        cstr_free(&dir);
        token = strtok(NULL, ":");
    }

    free(path_copy_data);
    return 1; // Not found
}