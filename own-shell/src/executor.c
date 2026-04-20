#include "utils.h"
#include "app_state.h"
#include "executor.h"

static void buildin_exit(Command *cmd_ptr);
static void buildin_echo(Command *cmd_ptr);
static void buildin_type(Command *cmd_ptr);
static void buildin_pwd(Command *cmd_ptr);
static void buildin_cd(Command *cmd_ptr);

static BuildinHandler get_buildin_handler_by_name(char *name);
static i32 find_buildin_by_name(char *name);
static i32 exec_buildin(Command *cmd_ptr);
static i32 search_binary(char *searched_bin, u32 searched_bin_len, char *cmd, u32 cmd_len);
static i32 exec_binary(Command *cmd_ptr);
static i32 start_redirect(Redirect *redirect_ptr);
static i32 stop_redirect(Redirect *redirect_ptr);
static i32 exec_cmd(Command *cmd_ptr);
static i32 exec_cmd_args(Command *cmd_ptr);

static Buildin s_buildin_list[] = {
  {.name = "exit", .handler = buildin_exit},
  {.name = "echo", .handler = buildin_echo},
  {.name = "type", .handler = buildin_type},
  {.name = "pwd", .handler = buildin_pwd},
  {.name = "cd", .handler = buildin_cd}
};

static i32 start_redirect(Redirect *redirect_ptr){
  if(redirect_ptr == NULL || redirect_ptr->redirect_count == 0){
    return -1;
  }

  for(u32 i = 0; i < redirect_ptr->redirect_count; ++i){
    i32 oldfd = open(redirect_ptr->redirect_file[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(oldfd < 0){
      perror("sh: open");
      return -1;
    }
    dup2(oldfd, redirect_ptr->redirect_fd[i]);
  }

  return 1;
}

static i32 stop_redirect(Redirect *redirect_ptr){
  if(redirect_ptr == NULL || redirect_ptr->redirect_count == 0){
    return -1;
  }

  fflush(stdout);
  fflush(stderr);
  for(u32 i = 0; i < redirect_ptr->redirect_count; ++i){
    close(redirect_ptr->redirect_fd[i]);
  }

  return 0;
}

static i32 exec_cmd_args(Command *cmd_ptr){
  if(cmd_ptr == NULL || cmd_ptr->argc == 0){
    return -1;
  }

  BuildinHandler handler = get_buildin_handler_by_name(cmd_ptr->arguments[0]);
  if(handler != NULL){
    return exec_buildin(cmd_ptr);
  }

  return exec_binary(cmd_ptr);
}

static i32 exec_cmd(Command *cmd_ptr){
  start_redirect(&cmd_ptr->redirect);

  i32 ret = exec_cmd_args(cmd_ptr);

  stop_redirect(&cmd_ptr->redirect);
  return ret;
}

i32 execute(Command *cmd_ptr){
  if(cmd_ptr == NULL || cmd_ptr->argc == 0){
    return -1;
  }
  i32 stdout_fd = dup(STDOUT_FILENO);
  i32 stderr_fd = dup(STDERR_FILENO);
  i32 stdin_fd = dup(STDIN_FILENO);

  i32 ret = exec_cmd(cmd_ptr);
  
  dup2(stdout_fd, STDOUT_FILENO);
  dup2(stderr_fd, STDERR_FILENO);
  dup2(stdin_fd, STDIN_FILENO);
  close(stdout_fd);
  close(stderr_fd);
  close(stdin_fd);
  return ret;
}

static i32 exec_buildin(Command *cmd_ptr){
  BuildinHandler handler = get_buildin_handler_by_name(cmd_ptr->arguments[0]);
  if(handler == NULL){
    return -1;
  }
  handler(cmd_ptr);
  return 0;
}

static BuildinHandler get_buildin_handler_by_name(char *name){
  i32 idx = find_buildin_by_name(name);
  if(idx < 0){
    return NULL;
  }
  return s_buildin_list[idx].handler;
}

static i32 exec_binary(Command *cmd_ptr){
  char searched_bin[PATH_LEN_MAX+1];
  if(search_binary(searched_bin, PATH_LEN_MAX, cmd_ptr->arguments[0], strlen(cmd_ptr->arguments[0])) < 0){
    return -1;
  }
  
  pid_t pid = fork();
  if(pid == 0){
    // Child process - execute binary
    execv(searched_bin, cmd_ptr->arguments);
    perror("sh: execv");
    exit(127);
  }else if(pid > 0){
    // Parent process - wait for child
    int status;
    waitpid(pid, &status, 0);
    return 0;
  }else{
    perror("sh: fork");
    return -1;
  }
}

static i32 search_binary(char *searched_bin, u32 searched_bin_len, char *cmd, u32 cmd_len){
  if(cmd == NULL || searched_bin == NULL || cmd_len == 0 || cmd_len >= PATH_LEN_MAX || searched_bin_len < PATH_LEN_MAX){
    return -1;
  }

  char *env_paths = getenv("PATH");
  if(env_paths == NULL){
    return -1;
  }

  char env_paths_clone[strlen(env_paths)+1];
  strncpy(env_paths_clone, env_paths, strlen(env_paths)+1);

  char *path = strtok(env_paths_clone, ":");
  while(path != NULL){
    char full_path[PATH_LEN_MAX+1];
    snprintf(full_path, PATH_LEN_MAX, "%s/%s", path, cmd);
    if(access(full_path, X_OK) == 0){
      strncpy(searched_bin, full_path, PATH_LEN_MAX);
      return 0;
    }
    path = strtok(NULL, ":");
  }

  return -1;
}

static void buildin_exit(Command *cmd_ptr){
  i32 exit_code = 0;
  
  if(cmd_ptr->argc > 1){
    exit_code = str2num(cmd_ptr->arguments[1]);
    if(exit_code != -1){
      // Valid exit code
      printf("sh: exit: %d\n", exit_code);
      exit(exit_code);
    } else {
      printf("sh: exit: %s: numeric argument required\n", cmd_ptr->arguments[1]);
      return;
    }
  }
  
  set_app_state(APP_STATE_STOPPED);
  exit(exit_code);
}

static void buildin_echo(Command *cmd_ptr){
  u32 str_len = 0;
  char str[IN_LEN+1];
  u32 argc = cmd_ptr->argc;
  char **argv = cmd_ptr->arguments;

  memset(str, 0x00, IN_LEN+1);

  for(u32 i = 1; i < argc; ++i){
    strcat(str + str_len, argv[i]);
    if(i == argc - 1){
      continue;
    }
    str_len = strlen(str);
    str[str_len] = ' ';
    ++str_len;
  }

  printf("%s\n", str);
}

static void buildin_type(Command *cmd_ptr){
  u32 argc = cmd_ptr->argc;
  char **argv = cmd_ptr->arguments;

  if(argc < 2){
    return;
  }

  i32 idx = find_buildin_by_name(argv[1]);
  if(idx >= 0){
    printf("%s is a build-in command\n", argv[1]);
    return;
  }

  char searched_bin[PATH_LEN_MAX+1];
  if(search_binary(searched_bin, PATH_LEN_MAX, argv[1], strlen(argv[1])) == 0){
    printf("%s is %s\n", argv[1], searched_bin);
    return;
  }

  printf("%s: not found\n", argv[1]);
}

static i32 find_buildin_by_name(char *name){
  for(u32 i = 0; i < sizeof(s_buildin_list)/sizeof(Buildin); ++i){
    if(strcmp(name, s_buildin_list[i].name) == 0)
    {
      return i;
    }
  }
  return -1;
}

static void buildin_pwd(Command *cmd_ptr){
  (void)cmd_ptr;  // Unused parameter
  char cwd[PATH_LEN_MAX+1];
  if(getcwd(cwd, PATH_LEN_MAX) == NULL){
    return;
  }
  printf("%s\n", cwd);
}

static void buildin_cd(Command *cmd_ptr){
  u32 argc = cmd_ptr->argc;
  char **argv = cmd_ptr->arguments;

  if(argc < 2){
    return;
  }

  char dest[PATH_LEN_MAX+1];
  if(argv[1][0] == '~'){
    const char *home = getenv("HOME");
    if(home == NULL){
      return;
    }
    snprintf(dest, PATH_LEN_MAX, "%s/%s", home, argv[1]+1);
  }else{
    strncpy(dest, argv[1], PATH_LEN_MAX);
  }

  if(chdir(dest) < 0){
    printf("cd: %s: No such file or directory\n", argv[1]);
  }
}
