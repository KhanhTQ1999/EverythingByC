#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtin.h"
#include "utils.h"
#include "history.h"
#include "typedef.h"
#include "jobs.h"
#include "completions.h"

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
static void builtin_complete(SimpleCommand *args); // Placeholder for the complete command

static Builtin s_builtin_list[] = {
  {.name = "exit", .handler = builtin_exit},
  {.name = "echo", .handler = builtin_echo},
  {.name = "type", .handler = builtin_type},
  {.name = "pwd", .handler = builtin_pwd},
  {.name = "cd", .handler = builtin_cd},
  {.name = "history", .handler = builtin_history},
  {.name = "jobs", .handler = builtin_jobs},
  {.name = "complete", .handler = builtin_complete}, // Placeholder for the complete command
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

    cstr full_path = {0};
    if (find_executable_path(command_name, &full_path) == 0) {
        printf("%s is %s\n", command_name, full_path.data);
    } else {
        printf("%s: not found\n", command_name);
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

  if(args->data[1].size == 0) {
    write(STDERR_FILENO, "cd: empty directory path\n", 25);
    return;
  }

  if(args->data[1].data[0] == '~') {
    // Handle home directory expansion
    const char *home = getenv("HOME");
    if(home == NULL) {
      write(STDERR_FILENO, "cd: HOME environment variable not set\n", 38);
      return;
    }
    cstr new_path = {0};
    cstr_append(&new_path, home);
    cstr_append(&new_path, args->data[1].data + 1); // Skip the '~' character
    if (chdir(new_path.data) != 0) {
      printf("cd: %s: No such file or directory\n", new_path.data);
    }
    cstr_free(&new_path);
    return;
  }

  const char *path = args->data[1].data;
  if (chdir(path) != 0) {
    printf("cd: %s: No such file or directory\n", path);
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
    (void)args; // Unused parameter

    JobList job_list = jobs_list();
    for(i32 i = job_list.size - 1; i >= 0; --i) {
      i32 status = 0;
      if(waitpid(job_list.data[i].pid, &status, WNOHANG) > 0) {
        if(WIFEXITED(status) || WIFSIGNALED(status)) {
          job_list.data[i].status = JOB_DONE;
        }
      }
    }

    char marker = ' ';
    for (u32 i = 0; i < job_list.size; ++i) {
      if(i == job_list.size - 1) {
          marker = '+';
      } else if(i == job_list.size - 2) {
          marker = '-';
      } else {
          marker = ' ';
      }
      Job *job = &job_list.data[i];
      if(job->status == JOB_DONE) {
          printf("[%d]%c %s \t%s\n",
              job->job_id,
              marker,
              "Done",
              job->command.data);
          continue;
      }
      if(job->status == JOB_RUNNING) {
          printf("[%d]%c %s \t%s %c\n",
              job->job_id,
              marker,
              "Running",
              job->command.data, '&');
      } else if(job->status == JOB_STOPPED) {
          printf("[%d]%c %s \t%s %c\n",
              job->job_id,
              marker,
              "Stopped",
              job->command.data);
      }
  }
  //remove done jobs
  for(i32 i = job_list.size - 1; i >= 0; --i) {
      if(job_list.data[i].status == JOB_DONE) {
          jobs_remove(job_list.data[i].job_id);
      }
  }
}

static void builtin_exit(SimpleCommand *args) {
  // Implement the logic for the exit command
}

static void builtin_complete(SimpleCommand *args) {
    // Placeholder for the complete command implementation
    (void)args; // Unused parameter
    if(args->size < 2) {
        write(STDERR_FILENO, "complete: missing argument\n", 27);
        return;
    }

    if(strcmp(args->data[1].data, "-p") == 0) {
      if(args->size < 3) {
          write(STDERR_FILENO, "complete: missing argument for -p\n", 34);
          return;
      }
      cstr command = args->data[2];
      CompleterScript *script = get_completer_script(&command);
      if(script) {
          printf("complete -C '%s' %s\n", script->script.data, command.data);
      } else {
          printf("complete: %s: no completion specification\n", command.data);
      }
    }else if(strcmp(args->data[1].data, "-C") == 0) {
      if(args->size < 4) {
          write(STDERR_FILENO, "complete: missing argument for -C\n", 34);
          return;
      }
      cstr script = args->data[2];
      cstr command = args->data[3];
      register_completer_script(&script, &command);
    } else {
        write(STDERR_FILENO, "complete: invalid option\n", 26);
    }
}

static BuiltinHandler get_builtin_handler(const char *name) {
    for (u32 i = 0; i < s_builtin_count; ++i) {
        if (strcmp(s_builtin_list[i].name, name) == 0) {
            return s_builtin_list[i].handler;
        }
    }
    return NULL;
}

u32 execute_builtin(SimpleCommand *args, i32 async) {
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

StrList find_builtin(cstr *hint) {
    StrList matches = { .data = NULL, .size = 0, .capacity = 0 };
    for(u32 i = 0; i < s_builtin_count; ++i) {
        if(strncmp(s_builtin_list[i].name, hint->data, hint->size) == 0) {
            cstr match_str = { .data = NULL, .size = 0, .capacity = 0 };
            cstr_appendn(&match_str, s_builtin_list[i].name, strlen(s_builtin_list[i].name));
            vec_append(&matches, match_str);
        }
    }
    return matches;
}