#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include "typedef.h"
#include "cstr.h"
#include "vec.h"
#include "command.h"
#include "completions.h"
#include "shell.h"

#define PROMPT "$ "

static void s_disable_raw_mode();
static void s_enable_raw_mode();
static i32 s_readline(cstr *input);
static void s_shell_exit(i32 status);

static struct termios orig_termios;

static void s_handle_sigint(int sig) {
    (void)sig;
    s_disable_raw_mode();
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

    s_enable_raw_mode();
    while(1){
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        /* Handle read errors or EOF */
        if(n <= 0){
            return -1;
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
            }else if(vec_size(&matches) > 1) {
                if(!tab_pressed) {
                    write(STDOUT_FILENO, "\a", 1); //bell sound
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
            cstr_append(input, &c);
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
    cstr input = {0};

    while(running){
        write(STDOUT_FILENO, PROMPT, sizeof(PROMPT) - 1);
        memset(&input, 0, sizeof(cstr));
        i32 bytes_read = s_readline(&input);
        if(bytes_read <= 0 || input.size == 0){
            continue;
        }
        Command cmd = parse_command(&input);
        printf("You entered: %s\n", input.data);
        fflush(stdout);
    }
    cstr_free(&input);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, s_handle_sigint);
    do_repl();
    return 0;
}