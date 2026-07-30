#include "history.h"

#define SUCCESS 0
#define FAILURE 1

static StrList s_history = {0};
static u32 history_startup_num = 0;

i32 history_add(cstr *entry){
  if(entry == NULL){
    return FAILURE;
  }

  cstr new_entry = {0};
  cstr_copy(&new_entry, entry);
  vec_append(&s_history, new_entry);
  return SUCCESS;
}

i32 history_get(u32 idx, cstr *entry){
  if(idx >= s_history.size || entry == NULL){
    return FAILURE;
  }
  cstr_copy(entry, &s_history.data[idx]);
  return SUCCESS;
}

u32 history_size(){
  return s_history.size;
}

u32 history_load(cstr *file_path){
  if(file_path == NULL){
    return FAILURE;
  }
  FILE *file = fopen(file_path->data, "r");
  if(file == NULL){
    return FAILURE;
  }
  char line[USR_INPUT_MAX];
  while(fgets(line, sizeof(line), file)){
    u32 len = strlen(line);
    if(len > 0 && line[len - 1] == '\n'){
      line[len - 1] = '\0'; // Remove the newline character
    }
    cstr entry = {0};
    cstr_append(&entry, line);
    history_add(&entry);
    cstr_free(&entry);
  }
  fclose(file);
  return SUCCESS;
}

u32 history_save(cstr *file_path){
  if(file_path == NULL){
    return FAILURE;
  }
  FILE *file = fopen(file_path->data, "w");
  if(file == NULL){
    return FAILURE;
  }
  for(u32 i = 0; i < s_history.size; ++i){
    fprintf(file, "%s\n", s_history.data[i].data);
  }
  fclose(file);
  return SUCCESS;
}

u32 history_append(cstr *file_path){
  if(file_path == NULL){
    return FAILURE;
  }
  FILE *file = fopen(file_path->data, "a");
  if(file == NULL){
    return FAILURE;
  }
  i32 start = history_startup_num;
  for(i32 i = s_history.size - 2; i >= 0; --i){
    /* find the last "history -a <file-path>" and "exit" command */
    if(strncmp(s_history.data[i].data, "history -a ", 11) == 0 || strncmp(s_history.data[i].data, "exit", 4) == 0){
      start = i + 1;
      break;
    }
  }

  for(i32 i = start; i < s_history.size; ++i){
    fprintf(file, "%s\n", s_history.data[i].data);
  }
  fclose(file);
  return SUCCESS;
}

u32 history_startup(){
  /* load history from HISTFILE */
  const char *histfile = getenv("HISTFILE");
  if(histfile == NULL){
    return FAILURE;
  }
  cstr file_path = {0};
  cstr_append(&file_path, histfile);
  history_load(&file_path);
  cstr_free(&file_path);
  history_startup_num = s_history.size;
  return SUCCESS;
}

u32 history_exit(){
  /* save history to HISTFILE */
  const char *histfile = getenv("HISTFILE");
  if(histfile == NULL){
    return FAILURE;
  }
  cstr file_path = {0};
  cstr_append(&file_path, histfile);
  history_append(&file_path);
  cstr_free(&file_path);
  return SUCCESS;
}