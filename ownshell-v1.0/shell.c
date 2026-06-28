#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "parser.h"
#include "shell.h"
#include "command.h"
#include "executor.h"
#include "utils.h"

#define USR_INPUT_MAX 1024

static void sort_candidates(s8_list candidates){
    for(u32 i = 0; i < candidates.size - 1; ++i){
        for(u32 j = 0; j < candidates.size - i - 1; ++j){
            if(strcmp(candidates.data[j].data, candidates.data[j+1].data) > 0){
                s8 temp = candidates.data[j];
                candidates.data[j] = candidates.data[j+1];
                candidates.data[j+1] = temp;
            }
        }
    }
}

static i32 s_read_line(char *buffer, i32 size) {
    if (size <= 0 || buffer == NULL) return FAILURE;
    i32 ret = SUCCESS;
    i32 idx = 0;
    bool tab_pressed = false;

    enableRawMode();
    while (1) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n < 0) {
            if (errno == EINTR) continue;
            ret = FAILURE;
            break;
        }
        if (n == 0) {
            continue;
        }

        /* Handle newline / enter */
        if (c == '\r' || c == '\n') {
            write(STDOUT_FILENO, "\n", 1);
            buffer[idx] = '\n';
            ret = SUCCESS;
            break;
        }

        /* Backspace / DEL */
        if (c == 0x7f || c == 0x08) {
            if (idx > 0) {
                idx--;
                write(STDOUT_FILENO, "\b \b", 3);
            }
            tab_pressed = false;
            continue;
        }

        /* Tab completion */
        if (c == '\t') {
            s8_list candidates = {0};
            i32 completion_len = complete_command(&candidates, buffer, idx);
            if (completion_len == 1) {
                const s8 *completion = &candidates.data[0];
                write(STDOUT_FILENO, "\x1B[1K\r", 5);
                write(STDOUT_FILENO, "$ ", 2);
                write(STDOUT_FILENO, completion->data, completion->size);
                write(STDOUT_FILENO, " ", 1);

                memcpy(buffer, completion->data, completion->size < USR_INPUT_MAX - 1 ? completion->size : USR_INPUT_MAX - 2);
                idx = completion->size < USR_INPUT_MAX - 1 ? completion->size : USR_INPUT_MAX - 2;
                buffer[idx++] = ' ';
            }else if(completion_len > 1){
                if(!tab_pressed){
                    //find longest lenght matched of candidate
                    if(candidates.size > 0){
                        u32 min_len = candidates.data[0].size;
                        for(u32 i = 1; i < candidates.size; ++i){
                            if(candidates.data[i].size < min_len){
                                min_len = candidates.data[i].size;
                            }
                            u32 common_end = idx;
                            for(u32 pos = idx; pos < min_len; ++pos){
                                char ch = candidates.data[0].data[pos];
                                bool all_same = true;
                                for(u32 j = 1; j < candidates.size; ++j){
                                    if(candidates.data[j].data[pos] != ch){
                                        all_same = false;
                                        break;
                                    }
                                }
                                if(!all_same) break;
                                common_end++;
                            }
                            u32 add_len = common_end - idx;
                            if(add_len > 0){
                                u32 avail = (USR_INPUT_MAX - 1) - idx;
                                if(add_len > avail) add_len = avail;
                                write(STDOUT_FILENO, candidates.data[0].data + idx, add_len);
                                memcpy(buffer + idx, candidates.data[0].data + idx, add_len);
                                idx += add_len;
                                buffer[idx] = '\0';
                            }else{
                                write(STDOUT_FILENO, "\a", 1);
                                tab_pressed = true;
                            }
                        }
                    }
                }else{
                    tab_pressed = false;
                    sort_candidates(candidates);
                    write(STDOUT_FILENO, "\n", 1);
                    for(u32 i=0; i<candidates.size; i++){
                        write(STDOUT_FILENO, candidates.data[i].data, candidates.data[i].size);
                        write(STDOUT_FILENO, " ", 1);
                    }
                    write(STDOUT_FILENO, "\n", 1);
                    write(STDOUT_FILENO, "$ ", 2);
                    write(STDOUT_FILENO, buffer, idx);
                }
            }
            s8_list_free(&candidates);
            continue;
        }else{
            write(STDOUT_FILENO, "\a", 1);
        }

        if (isprint((unsigned char)c)) {
            if (idx < size - 1) {
                buffer[idx++] = c;
                write(STDOUT_FILENO, &c, 1);
            } else {
                const char bell = '\a';
                write(STDOUT_FILENO, &bell, 1);
            }
        }
        tab_pressed = false;
        usleep(10000);
    }
    disableRawMode();
    return ret;
}

static i32 s_do_repl(void) {
    char input[USR_INPUT_MAX];
    CommandList cmd_list = {0};
    while(1){
        // write(STDOUT_FILENO, "\x1B[0G", 4);
        write(STDOUT_FILENO, "$ ", 2);
        memset(input, 0x00, sizeof(input));
        i32 rc = s_read_line(input, USR_INPUT_MAX);
        if (rc == FAILURE) {
            break;
        }
        s8 str_input;
        s8_create(&str_input, input);
        parse_commands(&str_input, &cmd_list);
        execute(&cmd_list);

        s8_free(&str_input);
        command_list_free(&cmd_list);
    }
    return 0;
}

i32 shell_run(void) {
    i32 rc = s_do_repl();
    return rc;
}