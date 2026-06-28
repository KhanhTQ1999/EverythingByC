#include "command.h"

static void s_arg_list_free(ArgList *arg_list) {
    for(i32 i=0; i<arg_list->size; i++){
        s8_free(&arg_list->data[i]);
    }
    vector_free(arg_list);
}

static void s_redirect_list_free(RedirectList *redirect_list) {
    for(i32 i=0; i<redirect_list->size; i++){
        s8_free(&redirect_list->data[i].filename);
    }
    vector_free(redirect_list);
}

void command_list_free(CommandList *cmd_list) {
    for(i32 i=0; i<cmd_list->size; i++){
        s_arg_list_free(&cmd_list->data[i].args);
        s_redirect_list_free(&cmd_list->data[i].redirects);
    }
    vector_free(cmd_list);
}

u32 complete_command(s8_list *candidates, char *buffer, u32 buffer_len){
    u32 candidates_count = complete_builtin(candidates, buffer, buffer_len);
    candidates_count += complete_executable(candidates, buffer, buffer_len);
    return candidates_count;
}