#include "builtin.h"
#include "executable.h"

static i32 search_builtin(s8 *name);
static i32 find_builtin_index(s8 *name);

static void builtin_exit(ArgList *args);
static void builtin_echo(ArgList *args);
static void builtin_type(ArgList *args);
static void builtin_pwd(ArgList *args);
static void builtin_cd(ArgList *args);

static Builtin s_builtin_list[] = {
  {.name = "exit", .handler = builtin_exit},
  {.name = "echo", .handler = builtin_echo},
  {.name = "type", .handler = builtin_type},
  {.name = "pwd", .handler = builtin_pwd},
  {.name = "cd", .handler = builtin_cd}
};

static void builtin_echo(ArgList *args){
  s8 str = {0};
  u32 argc = args->size;
  if(argc < 2){
    printf("\n");
    return;
  }

  for(u32 i = 1; i < argc; ++i){
    s8_append(&str, args->data[i].data);
    if(i < argc - 1){
      s8_append(&str, " ");
    }
  }
  printf("%s\n", str.data);
  fflush(stdout);
  s8_free(&str);
}

static void builtin_pwd(ArgList *args){
  char cwd[PATH_LEN_MAX+1];
  if(getcwd(cwd, PATH_LEN_MAX) == NULL){
    return;
  }
  printf("%s\n", cwd);
}

static void builtin_cd(ArgList *args){
  u32 argc = args->size;
  s8 path = {0};
  s8 dest = {0};

  if(argc < 2){
    return;
  }

  if(args->data[1].data[0] == '~'){
    const char *home = getenv("HOME");
    if(home == NULL){
      return;
    }

    s8_append(&dest, home);
    s8_substring(&path, &args->data[1], 1, args->data[1].size - 1);
    if(path.size > 0){
      s8_append(&dest, path.data);
    }
  }else{
    s8_append(&dest, args->data[1].data);
  }

  if(chdir(dest.data) < 0){
    printf("cd: %s: No such file or directory\n", dest.data);
  }

  s8_free(&path);
  s8_free(&dest);
}

static void builtin_exit(ArgList *args){
  exit(0);
}

static void builtin_type(ArgList *args){
  u32 argc = args->size;
  if(argc < 2){
    return;
  }

  s8 *name = &args->data[1];
  i32 idx = find_builtin_index(name);
  if(idx >= 0){
    printf("%s is a shell builtin\n", name->data);
    return;
  }

  s8 path = {0};
  if(search_executable(&path, name) == 0){
    printf("%s is %s\n", name->data, path.data);
    s8_free(&path);
    return;
  }
  printf("%s: not found\n", name->data);
  s8_free(&path);
}

static i32 find_builtin_index(s8 *name){
  for(u32 i = 0; i < sizeof(s_builtin_list)/sizeof(Builtin); ++i){
    if(strcmp(name->data, s_builtin_list[i].name) == 0)
    {
      return i;
    }
  }
  return -1;
}

BuiltinHandler get_builtin_handler(s8 *name){
  i32 idx = find_builtin_index(name);
  if(idx < 0){
    return NULL;
  }
  return s_builtin_list[idx].handler;
}

u32 complete_builtin(s8_list *candidates, char *buffer, u32 buffer_len){
  if(candidates == NULL || buffer == NULL || buffer_len == 0) return 0;
  u32 candidates_count = 0;

  char *prefix = malloc(buffer_len + 1);
  if(!prefix) return 0;
  memcpy(prefix, buffer, buffer_len);
  prefix[buffer_len] = '\0';

  for(u32 i = 0; i < sizeof(s_builtin_list) / sizeof(Builtin); ++i){
    if(strncmp(s_builtin_list[i].name, prefix, buffer_len) == 0){
      /* avoid duplicates (unlikely) */
      int exists = 0;
      for(size_t j=0;j<candidates->size;j++){
        if(strcmp(candidates->data[j].data, s_builtin_list[i].name) == 0){ exists = 1; break; }
      }   
      if(exists) continue;

      s8 candidate = {0};
      s8_append(&candidate, s_builtin_list[i].name);
      vector_append(candidates, candidate);
      candidates_count++;
    }
  }

  free(prefix);
  return candidates_count;
}
