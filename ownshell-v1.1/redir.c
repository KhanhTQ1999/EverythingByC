#include <fcntl.h>
#include <unistd.h>
#include "redir.h"

void start_redirection(REDIRECT_LIST *redirs) {
    if(redirs == NULL) {
        return;
    }

    for(u32 i = 0; i < redirs->size; ++i) {
        REDIRECT *redir = &redirs->data[i];
        if(redir == NULL) {
            continue;
        }

        int flags = O_WRONLY | O_CREAT;
        if(redir->truncated) {
            flags |= O_TRUNC;
        } else {
            flags |= O_APPEND;
        }

        int fd = open(redir->filename.data, flags, 0644);
        if(fd == -1) {
            continue;
        }

        if(dup2(fd, redir->fd) == -1) {
            close(fd);
            continue;
        }

        close(fd);
    }
}

void end_redirection(REDIRECT_LIST *redirs) {
    if(redirs == NULL) {
        return;
    }

    for(u32 i = 0; i < redirs->size; ++i) {
        REDIRECT *redir = &redirs->data[i];
        if(redir == NULL) {
            continue;
        }
        close(redir->fd);
    }
}