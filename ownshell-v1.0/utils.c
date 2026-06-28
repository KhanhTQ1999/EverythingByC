#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "typedef.h"
#include "utils.h"
#include "command_types.h"

static struct termios orig_termios;

void redirect_start(RedirectList *redirects){
  for(i32 i = 0; i < redirects->size; ++i){
    Redirect *redirect = &redirects->data[i];
    int flags = O_WRONLY | O_CREAT;
    if(redirect->truncated){
      flags |= O_TRUNC;
    }else{
      flags |= O_APPEND;
    }
    int fd = open(redirect->filename.data, flags, 0644);
    if(fd < 0){
      perror("open");
      continue;
    }
    if(dup2(fd, redirect->fd) < 0){
      perror("dup2");
      close(fd);
      continue;
    }
    close(fd);
  }
}

void redirect_end(RedirectList *redirects){
  fflush(stdout);
  fflush(stderr);
  for(u32 i = 0; i < redirects->size; ++i){
    close(redirects->data[i].fd);
  }
}

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

bool is_digit(char c){
    return c >= '0' && c <= '9';
}

int string_to_int(const char *str){
    int num = 0;
    while(*str){
        if(!is_digit(*str)){
            break;
        }
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}