#include <string.h>
#include <dirent.h>
#include "vec.h"
#include "utils.h"
#include "completions.h"
#include "builtin.h"

static CompleterScriptList s_completers = {0};

static CompContext s_detect_completion_context(cstr *input);
static StrList find_executable_in_path(cstr *hint);
static void complete_executable(StrList *result, CompContext *context);
static b8 completion_command_compare(const cstr *a, const cstr *b);
static u32 find_min_common_prefix(StrList *result);
static u32 find_common_prefix(const cstr *a, const cstr *b);
static StrList find_in_completer_scripts(cstr *hint);

StrList complete_command_word(cstr *input) {
    StrList matches = { .data = NULL, .size = 0, .capacity = 0 };

    if(input == NULL){ return matches; }

    CompContext context = s_detect_completion_context(input);
    if(context.type == COMPLETION_NONE) {
        return matches;
    }

    cstr hint = {0};
    cstr_substring(&hint, input, context.hint_index, input->size - context.hint_index);

    matches = find_in_completer_scripts(&hint);
    if(matches.size > 0) {
        complete_executable(&matches, &context);
        cstr_free(&hint);
        return matches;
    }
   
    matches = find_executable_in_path(&hint);
    StrList builtin_matches = find_builtin(&hint);

    strlist_concat(&matches, &builtin_matches);
    strlist_free(&builtin_matches);
    strlist_to_set(&matches, completion_command_compare);
    complete_executable(&matches, &context);

    cstr_free(&hint);
    return matches;
}

static CompContext s_detect_completion_context(cstr *input) {
    CompContext context = { .input = input, .type = COMPLETION_NONE };
    char *iter = cstr_begin(input) + skip_whitespace(input);

    /* If we reached the end of the input, return no context */
    if(iter == cstr_end(input)) {
        return context;
    }

    /* If we found a non-whitespace character, check if it's the first word or an argument */
    context.type = COMPLETION_COMMAND_WORD;
    context.hint_index = iter - cstr_begin(input);
    while(iter != cstr_end(input)) {
        if(*iter == ' ' || *iter == '\t') {
            context.type = COMPLETION_ARGUMENT;
            context.hint_index = iter - cstr_begin(input) + 1;
        }
        ++iter;
    }
    return context;
}

static StrList find_folder_in_path(cstr *hint) {
    StrList matches = { .data = NULL, .size = 0, .capacity = 0 };
    char *path_env = getenv("PATH");
    if(path_env == NULL) {
        return matches;
    }

    char *path_env_copy = strdup(path_env);
    char *token = strtok(path_env_copy, ":");
    while(token != NULL) {
        DIR *dir = opendir(token);
        if(dir != NULL) {
            struct dirent *entry;
            while((entry = readdir(dir)) != NULL) {
                if(entry->d_type == DT_DIR && (strncmp(entry->d_name, hint->data, hint->size) == 0) && strlist_find(&matches, entry->d_name) == -1) {
                    cstr match_str = { .data = NULL, .size = 0, .capacity = 0 };
                    cstr_appendn(&match_str, entry->d_name, strlen(entry->d_name));
                    vec_append(&matches, match_str);
                }
            }
            closedir(dir);
        }
        token = strtok(NULL, ":");
    }

    free(path_env_copy);
    return matches;
}

static StrList find_executable_in_path(cstr *hint) {
    StrList matches = { .data = NULL, .size = 0, .capacity = 0 };
    char *path_env = getenv("PATH");
    if(path_env == NULL) {
        return matches;
    }

    char *path_env_copy = strdup(path_env);
    char *token = strtok(path_env_copy, ":");
    while(token != NULL) {
        DIR *dir = opendir(token);
        if(dir != NULL) {
            struct dirent *entry;
            while((entry = readdir(dir)) != NULL) {
                if((entry->d_type == DT_REG || entry->d_type == DT_LNK) && (strncmp(entry->d_name, hint->data, hint->size) == 0) && strlist_find(&matches, entry->d_name) == -1) {
                    cstr match_str = { .data = NULL, .size = 0, .capacity = 0 };
                    cstr_appendn(&match_str, entry->d_name, strlen(entry->d_name));
                    vec_append(&matches, match_str);
                }
            }
            closedir(dir);
        }
        token = strtok(NULL, ":");
    }

    free(path_env_copy);
    return matches;
}

static void complete_executable(StrList *result, CompContext *context) {
    if(context->type != COMPLETION_COMMAND_WORD || result->size == 0) {
        return;
    }
    u32 hint_len = context->input->size - context->hint_index;
    u32 common_prefix_len = find_min_common_prefix(result);
    if(common_prefix_len > hint_len) {
        cstr_appendn(context->input, result->data[0].data + hint_len, common_prefix_len - hint_len);
    }
}
static u32 find_min_common_prefix(StrList *result){
    u32 common_prefix_len = result->data[0].size;
    for(u32 i = 1; i < result->size; ++i) {
        u32 match_len = find_common_prefix(&result->data[0], &result->data[i]);
        common_prefix_len = MIN(common_prefix_len, match_len);
    }
    return common_prefix_len;
}

static u32 find_common_prefix(const cstr *a, const cstr *b) {
    u32 i = 0;
    while(i < a->size && i < b->size && a->data[i] == b->data[i]) {
        ++i;
    }
    return i;
}

static b8 completion_command_compare(const cstr *a, const cstr *b) {
    return strcmp(a->data, b->data) > 0;
}

i32 register_completer_script(const cstr *script, const cstr *command) {
    CompleterScript new_script = {0};
    cstr_copy(&new_script.script, script);
    cstr_copy(&new_script.command, command);
    vec_append(&s_completers, new_script);

    return 0; // Success
}

CompleterScript* get_completer_script(const cstr *command) {
    for(u32 i = 0; i < s_completers.size; ++i) {
        if(strcmp(s_completers.data[i].command.data, command->data) == 0) {
            return &s_completers.data[i];
        }
    }
    return NULL; // Not found
}

// StrList find_in_completer_scripts(cstr *hint) {
//     StrList matches = { .data = NULL, .size = 0, .capacity = 0 };
//     CompleterScript *script = get_completer_script(hint);
//     if(script == NULL) {
//         return matches; // No matching script found
//     }

//     // Run script and readstdout for completions
//     FILE *fp = popen(script->script.data, "r");
//     if(fp == NULL) {
//         return matches; // Failed to run script
//     }

//     char buffer[256];
//     while(fgets(buffer, sizeof(buffer), fp) != NULL) {
//         // Remove newline character from the end of the buffer
//         size_t len = strlen(buffer);
//         if(len > 0 && buffer[len - 1] == '\n') {
//             buffer[len - 1] = '\0';
//         }

//     return matches;
// }