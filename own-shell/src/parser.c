#include "parser.h"
#include "utils.h"

i32 parse_command(char *raw, u32 raw_len, Command *cmd_ptr){
  char c;
  char *arguments[ARGC_MAX];
  u32 argc = 0;
  u32 idx = 0;
  u32 redirect_count = 0;
  b8 quotes = false;
  b8 dquotes = false;
  b8 backslash = false;

  if(raw_len > IN_LEN){
    throw_error(E_INPUT_TOO_LONG, "Input is longer than command buffer");
    return -1;
  }

  for(u32 raw_idx = 0; raw_idx < raw_len; ++raw_idx){
    if((c = raw[raw_idx]) == '\0'){
      break;
    }
    if(c == '\n'){
      continue;
    }

    if(c == '\\' && backslash == false){
      backslash = true;
      continue;
    }
    if(c == '\'' && dquotes == false && backslash == false){
      quotes = !quotes;
      continue;
    }

    if(c == '\"' && quotes == false && backslash == false){
      dquotes = !dquotes;
      continue;
    }

    if(c == ' ' && quotes == false && dquotes == false && backslash == false){
      cmd_ptr->buffer[idx++] = '\0';
      continue;
    }

    if(c == '>' && quotes == false && dquotes == false && backslash == false){
      if(redirect_count == 0){
        if(idx != 0 ){
          cmd_ptr->buffer[idx++] = '\0';
        }
        arguments[argc++] = cmd_ptr->buffer + idx;
        cmd_ptr->buffer[idx++] = c;
        cmd_ptr->buffer[idx++] = '\0';
      }else{
        cmd_ptr->buffer[idx++] = c;
      }
      redirect_count++;
      continue;
    }

    if(argc == 0 || cmd_ptr->buffer[idx-1] == '\0'){
      arguments[argc++] = cmd_ptr->buffer + idx;
    }
    cmd_ptr->buffer[idx++] = c;

    redirect_count = 0;
    if(backslash){
      backslash = false;
    }
  }

  if(idx > 0 && cmd_ptr->buffer[idx-1] != '\0'){
    cmd_ptr->buffer[idx++] = '\0';
  }

  if(quotes == true || dquotes == true){
    throw_error(E_INVALID_PARAMETER, "Unclosed quotes");
    return -1;
  }

  for(u32 i = 0; i < argc; ++i){
    if(arguments[i][0] == '\0'){
      continue;
    }

    if(i < argc - 1 && arguments[i+1][0] == '>'){
      if(i + 2 >= argc){
        throw_error(E_SYNTAX_ERROR, "syntax error near unexpected token");
        return -1;
      }

      i32 redirect_fd = str2num(arguments[i]);
      if(redirect_fd < 0){
        cmd_ptr->redirect.redirect_fd[cmd_ptr->redirect.redirect_count] = STDOUT_FILENO;
        cmd_ptr->arguments[cmd_ptr->argc++] = arguments[i];
      }else{
        cmd_ptr->redirect.redirect_fd[cmd_ptr->redirect.redirect_count] = redirect_fd;
      }

      cmd_ptr->redirect.redirect_file[cmd_ptr->redirect.redirect_count++] = arguments[i+2];
      i += 2;

      continue;
    }

    if(arguments[i][0] == '>'){
      if(i + 1 >= argc){
        throw_error(E_SYNTAX_ERROR, "syntax error near unexpected token");
        return -1;
      }

      cmd_ptr->redirect.redirect_fd[cmd_ptr->redirect.redirect_count] = STDOUT_FILENO;
      cmd_ptr->redirect.redirect_file[cmd_ptr->redirect.redirect_count++] = arguments[i+1];
      i += 1;

      continue;
    }

    if(argc >= ARGC_MAX){
      break;
    }
    cmd_ptr->arguments[cmd_ptr->argc++] = arguments[i];
  }

  return 0;
}