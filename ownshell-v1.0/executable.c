#include "executable.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

// Search PATH for an executable named `name` and copy the full path into `cmd`.
// Return SUCCESS if found, otherwise FAILURE.
i32 search_executable(s8 *cmd, s8 *name){
  char *path_env = getenv("PATH");
  if(path_env == NULL){
    return FAILURE;
  }

  s8 path_env_s8 = {0};
  s8_append(&path_env_s8, path_env);

  char *token = strtok(path_env_s8.data, ":");
  while(token != NULL){
    char full[PATH_LEN_MAX + 1];
    snprintf(full, sizeof(full), "%s/%s", token, name->data);
    struct stat st;
    if(stat(full, &st) == 0 && (st.st_mode & S_IXUSR)){
        s8 path = {0};
        s8_append(&path, full);
        s8_copy(cmd, &path);
        s8_free(&path);
        s8_free(&path_env_s8);
        return SUCCESS;
    }
    token = strtok(NULL, ":");
  }
  s8_free(&path_env_s8);
  return FAILURE;
}

u32 complete_executable(s8_list *candidates, char *buffer, u32 buffer_len){
  if(candidates == NULL || buffer == NULL || buffer_len == 0) return 0;
  u32 candidates_count = 0;

  char *prefix = malloc(buffer_len + 1);
  if(!prefix) return 0;
  memcpy(prefix, buffer, buffer_len);
  prefix[buffer_len] = '\0';

  char *path_env = getenv("PATH");
  if(path_env == NULL){ free(prefix); return 0; }

  s8 path_env_s8 = {0};
  s8_append(&path_env_s8, path_env);
  char *token = strtok(path_env_s8.data, ":");
  while(token != NULL){
    DIR *d = opendir(token);
    if(d){
      struct dirent *ent;
      while((ent = readdir(d)) != NULL){
        if(strncmp(ent->d_name, prefix, buffer_len) != 0) continue;
        char full[PATH_LEN_MAX + 1];
        snprintf(full, sizeof(full), "%s/%s", token, ent->d_name);
        struct stat st;
        if(stat(full, &st) != 0) continue;
        if(!(st.st_mode & S_IXUSR)) continue;

        int exists = 0;
        for(size_t i=0;i<candidates->size;i++){
          if(strcmp(candidates->data[i].data, ent->d_name) == 0){ exists = 1; break; }
        }
        if(exists) continue;

        s8 entry = {0};
        s8_append(&entry, ent->d_name);
        vector_append(candidates, entry);
        candidates_count++;
      }
      closedir(d);
    }
    token = strtok(NULL, ":");
  }
  s8_free(&path_env_s8);
  free(prefix);
  return candidates_count;
}