#include <stdio.h>

#include "typedef.h"
#include "utils.h"
#include "app_state.h"
#include "parser.h"
#include "executor.h"

char *g_prompt = "$ ";

void doREPL()
{
  i32 ret;
  char raw[IN_LEN+1];
  Command command;

  while(get_app_state() == APP_STATE_RUNNING){
    memset(&command, 0x00, sizeof(command));
    memset(raw, 0x00, sizeof(raw));

    printf("%s", g_prompt);
    do{
      u32 offset = strlen(raw);
      if(offset > 0){
        printf("> ");
      }
      if(get_input(raw + offset, IN_LEN - offset) < 0){
        catch_error();
        continue;
      }

      ret = parse_command(raw, IN_LEN, &command);
    }while(ret < 0);

    if(command.argc == 0){
      continue;
    }

    if(execute(&command) < 0){
      catch_error();
      continue;
    }
  }
}

int main(){
  set_app_state(APP_STATE_RUNNING);
  doREPL();
  return 0;
}