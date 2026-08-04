#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "typedef.h"
#include "cstr.h"
#include "vec.h"
#include "lexer.h"
#include "parser.h"
#include "dispose.h"
#include "completions.h"
#include "shell.h"
#include "execute_cmd.h"
#include "history.h"
#include "jobs.h"

#define PROMPT "$ "

static void s_disable_raw_mode();
static void s_enable_raw_mode();
static i32 s_readline(cstr *input);
static void s_shell_exit(i32 status);

static struct termios orig_termios;

static void s_handle_sigint(int sig) {
    (void)sig;
    s_disable_raw_mode();
    s_shell_exit(0);
}

static void s_shell_exit(i32 status) {
    fflush(stdout);
    fflush(stderr);
    exit(status);
}

static i32 s_readline(cstr *input) {
    if(input == NULL){ return -1; }

    b8 tab_pressed = false;
    b8 double_quote = false;
    b8 single_quote = false;
    b8 backslash = false;
    u32 history_idx = 0;

    s_enable_raw_mode();
    while(1){
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        /* Handle read errors or EOF */
        if(n <= 0){
            return -1;
        }
        /* Handle up-arrow and down-arrow */
        if(c == 0x1b) {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;

            if (seq[0] == '[' && seq[1] == 'A') {
                cstr entry = {0};
                history_idx = history_idx > 0 ? history_idx - 1 : history_size() - 1;
                if(history_idx >= 0 &&  history_get(history_idx, &entry) == 0){
                    printf("\x1B[1K\r$ %s", entry.data);
                    fflush(stdout);
                    cstr_copy(input, &entry);
                }
                cstr_free(&entry);
                continue;
            }

            if (seq[0] == '[' && seq[1] == 'B') {
                cstr entry = {0};
                history_idx = history_idx < history_size() - 1 ? history_idx + 1 : 0;
                if(history_idx < history_size() && history_get(history_idx, &entry) == 0){
                    printf("\x1B[1K\r$ %s", entry.data);
                    fflush(stdout);
                    cstr_copy(input, &entry);
                }
                cstr_free(&entry);
                continue;
            }
        }
        /* Handle newline / enter */
        if(c == '\n'){
            write(STDOUT_FILENO, "\n", 1);
            break;
        }
        /* Handle backspace / delete */
        if (c == 0x7f || c == 0x08) {
            if(cstr_pop(input) == 0){
                write(STDOUT_FILENO, "\b \b", 3);
            }
            tab_pressed = false;
            continue;
        }
        /* Handle tab completion */
        if (c == '\t') {
            StrList matches = complete_command_word(input);
            if(vec_size(&matches) == 1) {
                write(STDOUT_FILENO, "\x1B[1K\r", 5);
                write(STDOUT_FILENO, "$ ", 2);
                write(STDOUT_FILENO, input->data, input->size);
                write(STDOUT_FILENO, " ", 1);
                cstr_append(input, " ");
            }else if(vec_size(&matches) > 1) {
                if(!tab_pressed) {
                    write(STDOUT_FILENO, "\a", 1); //bell sound
                    write(STDOUT_FILENO, "\x1B[1K\r", 5);
                    write(STDOUT_FILENO, "$ ", 2);
                    write(STDOUT_FILENO, input->data, input->size);
                }else{
                    write(STDOUT_FILENO, "\n", 1);
                    for(u32 i = 0; i < matches.size; i++) {
                        write(STDOUT_FILENO, matches.data[i].data, matches.data[i].size);
                        write(STDOUT_FILENO, " ", 1);
                    }
                    write(STDOUT_FILENO, "\n", 1);
                    write(STDOUT_FILENO, "$ ", 2);
                    write(STDOUT_FILENO, input->data, input->size);
                }
                tab_pressed = !tab_pressed;
            }else{
                write(STDOUT_FILENO, "\a", 1); // bell sound
            }
            strlist_free(&matches);
            continue;
        }
        /* Handle printable characters */
        if (isprint((unsigned char)c)) {
            write(STDOUT_FILENO, &c, 1);
            char buf[2] = {c, '\0'};
            cstr_append(input, buf);
            // for(u32 i = 0; i < input->size; ++i) {
            //     printf("[DEBUG] input->data[%d]: %c\n", i, input->data[i]);
            // }
            tab_pressed = false;
        }
    }
    s_disable_raw_mode();
    return input->size;
}

static void s_enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(s_disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void s_disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void do_repl(){
    b8 running = true;
    

    while(running){
        JobList job_list = jobs_list();
        for(i32 i = job_list.size - 1; i >= 0; --i) {
            i32 status = 0;
            if(waitpid(job_list.data[i].pid, &status, WNOHANG) > 0) {
                char marker = ' ';
                if(i == job_list.size - 1) {
                    marker = '+';
                } else if(i == job_list.size - 2) {
                    marker = '-';
                } else {
                    marker = ' ';
                }
                if(WIFEXITED(status) || WIFSIGNALED(status)) {
                    printf("[%d]%c %s \t%s\n",
                        job_list.data[i].job_id,
                        marker,
                        "Done",
                        job_list.data[i].command.data);
                    jobs_remove(job_list.data[i].job_id);
                }
            }
        }
        write(STDOUT_FILENO, PROMPT, sizeof(PROMPT) - 1);
        cstr input = {0};
        i32 bytes_read = s_readline(&input);
        if(bytes_read <= 0 || input.size == 0){
            continue;
        }

        history_add(&input);

        TokenList tokens = {0};
        if(lex(&input, &tokens) != 0){
            printf("Lexing error\n");
            cstr_free(&input);
            continue;
        }

        Command *cmd = parse(&tokens, 0);
        if(cmd == NULL){
            printf("Parsing error\n");
            dispose_token_list(&tokens);
            cstr_free(&input);
            continue;
        }

        execute(cmd);
        fflush(stdout);

        dispose_command(cmd);
        dispose_token_list(&tokens);
        cstr_free(&input);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, s_handle_sigint);
    history_startup();
    do_repl();
    return 0;
}