#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin_types.h"
#include "executor.h"
#include "command.h"
#include "utils.h"

static i32 exec_builtin(ArgList *args);
static i32 exec_executable(ArgList *args);

i32 exec_command(ArgList *args){
  if(args == NULL || args->size == 0){
    return FAILURE;
  }
  if(exec_builtin(args) == 0){
    return SUCCESS;
  }

  i32 ret = exec_executable(args);
  if(ret == 0){
    return SUCCESS;
  }else if (ret == -2){
    printf("%s: command not found\n", args->data[0].data);
    ret = FAILURE;
  }
  return ret;
}

i32 exec_recursive(CommandList *cmdlist, u32 idx){
  if(cmdlist == NULL || cmdlist->size == 0){
    return FAILURE;
  }

  ArgList *args = &cmdlist->data[idx].args;
  RedirectList *redirs = &cmdlist->data[idx].redirects;
  redirect_start(redirs);

  if(idx == cmdlist->size - 1){
    i32 rc = exec_command(args);
    redirect_end(redirs);
    return rc == SUCCESS ? SUCCESS : FAILURE;
  }

  int pfd[2];
  if(pipe(pfd) == -1){
    redirect_end(redirs);
    return FAILURE;
  }

  pid_t pid = fork();
  if(pid == -1){
    close(pfd[0]);
    close(pfd[1]);
    redirect_end(redirs);
    return FAILURE;
  }

  if(pid == 0){
    close(pfd[0]);
    if(dup2(pfd[1], fileno(stdout)) < 0){
      perror("dup2");
      close(pfd[1]);
      _exit(127);
    }
    close(pfd[1]);
    exec_command(args);
    _exit(0);
  } else {
    close(pfd[1]);
    if(dup2(pfd[0], fileno(stdin)) < 0){
      perror("dup2");
      close(pfd[0]);
      waitpid(pid, NULL, 0);
      redirect_end(redirs);
      return FAILURE;
    }

    i32 rc = exec_recursive(cmdlist, idx + 1);

    close(pfd[0]);
    waitpid(pid, NULL, 0);
    redirect_end(redirs);
    return rc;
  }
}

i32 execute(CommandList *cmdlist){
  int stdinfd = dup(fileno(stdin));
  int stdoutfd = dup(fileno(stdout));
  int stderrfd = dup(fileno(stderr));

  exec_recursive(cmdlist, 0);

  dup2(stdinfd, fileno(stdin));
  dup2(stdoutfd, fileno(stdout));
  dup2(stderrfd, fileno(stderr));
  close(stdinfd);
  close(stdoutfd);
  close(stderrfd);
}

static i32 exec_builtin(ArgList *args){
  BuiltinHandler handler = get_builtin_handler(&args->data[0]);
  if(handler == NULL){
    return -1;
  }

  handler(args);
  return 0;
}

static i32 exec_executable(ArgList *args){
  if(args->size == 0){
    return FAILURE;
  }
  s8 command = {0};
  i32 ret = search_executable(&command, &args->data[0]);
  if(ret < 0 || command.size == 0){
    return -2;
  }

  char **argv = malloc((args->size + 1) * sizeof(char*));
  if(argv == NULL){
    s8_free(&command);
    return FAILURE;
  }
  for(u32 i = 0; i < args->size; ++i){
    argv[i] = args->data[i].data;
  }
  argv[args->size] = NULL;

  pid_t pid = fork();
  if(pid == 0){
    /* child */
    execv(command.data, argv);
    perror("execv");
    free(argv);
    s8_free(&command);
    free(argv);
    exit(127);
  }else if(pid > 0){
    int status;
    if(waitpid(pid, &status, 0) < 0){
      perror("waitpid");
      free(argv);
      s8_free(&command);
      return FAILURE;
    }
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : FAILURE;
    free(argv);
    s8_free(&command);
    return exit_code;
  }else{
    perror("fork");
    free(argv);
    s8_free(&command);
    return FAILURE;
  }
}