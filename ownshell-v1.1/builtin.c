#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "builtin.h"
#include "utils.h"
#include "history.h"

typedef void (*BuiltinHandler)(SimpleCommand *args);

typedef struct {
  const char *name;
  BuiltinHandler handler;
} Builtin;

static void builtin_echo(SimpleCommand *args);
static void builtin_type(SimpleCommand *args);
static void builtin_pwd(SimpleCommand *args);
static void builtin_cd(SimpleCommand *args);
static void builtin_exit(SimpleCommand *args);
static void builtin_history(SimpleCommand *args);
static void builtin_jobs(SimpleCommand *args);

static Builtin s_builtin_list[] = {
  {.name = "exit", .handler = builtin_exit},
  {.name = "echo", .handler = builtin_echo},
  {.name = "type", .handler = builtin_type},
  {.name = "pwd", .handler = builtin_pwd},
  {.name = "cd", .handler = builtin_cd},
  {.name = "history", .handler = builtin_history},
  {.name = "jobs", .handler = builtin_jobs}
};

static const u32 s_builtin_count = sizeof(s_builtin_list) / sizeof(Builtin);

static void builtin_echo(SimpleCommand *args) {
  cstr output = {0};
  for (u32 i = 1; i < args->size; ++i) {
    cstr_append(&output, args->data[i].data);
    if (i < args->size - 1) {
      cstr_append(&output, " ");
    }
  }
  cstr_append(&output, "\n");
  write(STDOUT_FILENO, output.data, output.size);

  cstr_free(&output); // Free any existing data in output
}

static void builtin_type(SimpleCommand *args) {
  if (args->size < 2) {
        write(STDERR_FILENO, "type: missing argument\n", 23);
        return;
    }

    const char *command_name = args->data[1].data;
    BuiltinHandler handler = NULL;

    for (u32 i = 0; i < s_builtin_count; ++i) {
        if (strcmp(s_builtin_list[i].name, command_name) == 0) {
            write(STDOUT_FILENO, command_name, strlen(command_name));
            write(STDOUT_FILENO, " is a shell builtin\n", 20);
            return;
        }
    }

    if (find_executable_path(command_name, NULL) == 0) {
        write(STDOUT_FILENO, command_name, strlen(command_name));
        write(STDOUT_FILENO, " is an external command\n", 25);
    } else {
        write(STDERR_FILENO, command_name, strlen(command_name));
        write(STDERR_FILENO, " not found\n", 11);
    }
}

static void builtin_pwd(SimpleCommand *args) {
  (void )args; // Unused parameter
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "\n", 1);
  } else {
    write(STDERR_FILENO, "pwd: error retrieving current directory\n", 40);
  }
}

static void builtin_cd(SimpleCommand *args) {
  if (args->size < 2) {
    write(STDERR_FILENO, "cd: missing argument\n", 21);
    return;
  }

  const char *path = args->data[1].data;
  if (chdir(path) != 0) {
    write(STDERR_FILENO, "cd: failed to change directory\n", 32);
  }
}

static void builtin_history(SimpleCommand *args) {
  if(args->size < 3){
    cstr entry = {0};
    u32 size = history_size();
    i32 limit = args->size == 1 ? -1 : cstr_to_int(&args->data[1]);
    
    //
    for(u32 i = 0; i < size; ++i){
      if(limit > 0 && i < size - limit){
        continue;
      }
      history_get(i, &entry);
      printf("    %d %s\n", i + 1, entry.data);
    }
  }else if(args->size == 3){
    if(strcmp(args->data[1].data, "-r") == 0){
      history_load(&args->data[2]);
    }else if(strcmp(args->data[1].data, "-w") == 0){
      history_save(&args->data[2]);
    } else if(strcmp(args->data[1].data, "-a") == 0){
      history_append(&args->data[2]);
    } else {
      // Handle other cases or invalid arguments
    }
  }else{
    // Handle other cases or invalid arguments
  }
}

static void builtin_jobs(SimpleCommand *args) {
  // Implement the logic for the jobs command
}

static void builtin_exit(SimpleCommand *args) {
  // Implement the logic for the exit command
}

static BuiltinHandler get_builtin_handler(const char *name) {
    for (u32 i = 0; i < s_builtin_count; ++i) {
        if (strcmp(s_builtin_list[i].name, name) == 0) {
            return s_builtin_list[i].handler;
        }
    }
    return NULL;
}

u32 execute_builtin(SimpleCommand *args, int async) {
    if (args->size == 0) {
        return 1; // No command to execute
    }

    if(strncmp(args->data[0].data, "exit", 4) == 0) {
        history_exit();
        exit(0); // Exit the shell
    }

    BuiltinHandler handler = get_builtin_handler(args->data[0].data);
    if (handler == NULL) {
        return 1;
    }
    if(async) {
        pid_t pid = fork();
        if(pid == 0) {
            printf("[1] %d\n", getpid());
            handler(args);
            exit(0); // Exit child process after execution
        } else if(pid > 0) {
            // Parent process continues without waiting
            return 0; // Command executed successfully in background
        } else {
            // Handle fork error
            return 1; // Indicate failure to execute command
        }
    }
    handler(args);
    return 0; // Command executed successfully
}

