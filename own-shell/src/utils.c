#include "utils.h"
#include "app_state.h"

static char s_errmsg[ERRMSG_LEN+1];
static Errno s_errno = E_SUCCESS;

i32 find_charactor_in_string(char *s, char c){
  if(s == NULL){
    return -1;
  }

  for(u32 i = 0; s[i] != '\0'; ++i){
    if(s[i] == c){
      return i;
    }
  }
  return -1;
}

char *find_last_char_in_string(char *s, char c){
  if(s == NULL){
    return NULL;
  }

  char *result = NULL;
  for(u32 i = 0; s[i] != '\0'; ++i){
    if(s[i] == c){
      result = &s[i];
    }
  }
  return result;
}

i32 str2num(char *s){
  i32 ret = 0;
  if(s == NULL || s[0] == '\0'){
    return -1;
  }

  for(u32 i = 0; s[i] != '\0'; ++i){
    if(s[i] < '0' || s[i] > '9'){
      return -1;
    }
    ret = ret * 10 + (s[i] - '0');
  }

  return ret;
}

i32 get_input(char *raw, u32 len){
  if(raw == NULL){
    throw_error(E_INVALID_PARAMETER, "sh: Invalid parameter");
    return -1;
  }

  if(fgets(raw, len, stdin) == NULL){
    throw_error(E_EOF, NULL);
    return -1;
  }

  if(len < 1 || (raw[len - 1] != '\n' && raw[len - 1] != '\0')){
    throw_error(E_INPUT_TOO_LONG, "sh: Command too long");
    return -1;
  }

  return 0;
}

void throw_error(Errno err, char *msg){
  s_errno = err;
  u32 msg_len = 0;
  
  if(msg == NULL){
    return;
  }
  msg_len = MIN(strlen(msg), ERRMSG_LEN);
  strncpy(s_errmsg, msg, msg_len);
}

void catch_error(){
  switch (s_errno)
  {
    case E_EOF:
      printf("\nsh: EOF received, exiting...");
      set_app_state(APP_STATE_RUNNING);
      break;

    default:
      printf("%s", s_errmsg);
      break;
  }

  s_errno = E_SUCCESS;
  memset(s_errmsg, 0x00, sizeof(s_errmsg));
}