#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "execute_cmd.h"
#include "builtin.h"
#include "utils.h"

static i32 execute_external_command(SimpleCommand *cmd, int async);
static void execute_simple_command(SimpleCommand *cmd, int async);
static void execute_connection_command(Connection *cmd);
static void execute_pipe_command(Connection *cmd);
static void execute_and_command(Connection *cmd);
static void execute_ampersand_command(Connection *cmd);

void execute(Command *cmd) {
    int stdinfd = dup(STDIN_FILENO);
    int stdoutfd = dup(STDOUT_FILENO);
    int stderrfd = dup(STDERR_FILENO);

    start_redirection(&cmd->redirs);

    if (cmd->type == CM_SIMPLE) {
        execute_simple_command((SimpleCommand *)&cmd->value.simple, 0);
    } else if (cmd->type == CM_CONNECTION) {
        execute_connection_command((Connection *)&cmd->value.connection);
    }

    end_redirection(&cmd->redirs);

    dup2(stdinfd, STDIN_FILENO);
    dup2(stdoutfd, STDOUT_FILENO);
    dup2(stderrfd, STDERR_FILENO);
    close(stdinfd);
    close(stdoutfd);
    close(stderrfd);
}

static void execute_simple_command(SimpleCommand *cmd, int async) {
    if(cmd == NULL || cmd->size == 0) {
        return;
    }

    if(execute_builtin(cmd, async) == 0) {
        return; // Builtin command executed successfully
    }

    execute_external_command(cmd, async); // Execute external command if not a builtin
}

static i32 execute_external_command(SimpleCommand *cmd, int async) {
    if(cmd == NULL || cmd->size == 0) {
        return 1;
    }
    i32 ret = 0;
    cstr full_path = {0};

    if(find_executable_path(cmd->data[0].data, &full_path) != 0) {
        printf("%s: command not found\n", cmd->data[0].data);
        return 1;
    }

    cstr *args = &cmd->data[0];
    char **argv = malloc((cmd->size + 1) * sizeof(char*));
    memset(argv, 0, (cmd->size + 1) * sizeof(char*));
    for(u32 i = 0; i < cmd->size; ++i) {
        argv[i] = cmd->data[i].data;
    }

    pid_t pid = fork();
    if(pid == 0) {
        execv(full_path.data, argv);
        exit(EXIT_FAILURE); // If execv fails
    } else if(pid > 0) {
        int status;
        if(async) {
            printf("[1] %d\n", pid);
        }else{
            waitpid(pid, &status, 0);
            ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        }
        // Handle exit status if needed
    } else {
        printf("Fork failed\n");
        // Handle fork error
    }

    free(argv);
    cstr_free(&full_path);
    return ret;
}

static void execute_connection_command(Connection *cmd) {
    if(cmd == NULL) {
        return;
    }

    switch(cmd->type) {
        case CONN_PIPE:
            execute_pipe_command(cmd);
            break;
        case CONN_AND:
            execute_and_command(cmd);
            break;
        case CONN_AMPERSAND:
            execute_ampersand_command(cmd);
            break;
        default:
            break;
    }
}

static void execute_pipe_command(Connection *cmd) {
    if(cmd == NULL) {
        return;
    }

    int pipefd[2];
    if(pipe(pipefd) == -1) {
        // Handle pipe error
        return;
    }

    pid_t pid = fork();
    if(pid == 0) {
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        execute(cmd->left);
        close(pipefd[1]); // Close write end after use
        exit(0);
    } else if(pid > 0) {
        close(pipefd[1]); // Close write end
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin from pipe
        execute(cmd->right);
        close(pipefd[0]); // Close read end after use
        waitpid(pid, NULL, 0); // Wait for the child process to finish
    } else {
        close(pipefd[0]);
        close(pipefd[1]);
    }
}

static void execute_and_command(Connection *cmd) {
    if(cmd == NULL) {
        return;
    }

    execute(cmd->left);
    // Check exit status of left command
    if(WIFEXITED(0) && WEXITSTATUS(0) == 0) {
        execute(cmd->right);
    }
}

static void execute_ampersand_command(Connection *cmd) {
    if(cmd == NULL) {
        return;
    }

    if(cmd->left->type == CM_SIMPLE) {
        execute_simple_command((SimpleCommand *)&cmd->left->value.simple, 1); // Execute left command asynchronously
    } 

    execute(cmd->right);
}