#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

static int open_redir_in(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); return -1; }
    return fd;
}

static int open_redir_out(const char *path, int append) {
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int fd = open(path, flags, 0644);
    if (fd < 0) { perror(path); return -1; }
    return fd;
}

int execute_pipeline(Pipeline *p) {
    /* Single built-in: run directly in the shell process */
    if (p->n == 1 && is_builtin(p->cmds[0].argv[0]))
        return run_builtin(&p->cmds[0]);

    int n = p->n;
    int pipes[MAX_PIPES][2];

    /* Create n-1 pipes */
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) { perror("pipe"); return 1; }
    }

    pid_t pids[MAX_PIPES];

    for (int i = 0; i < n; i++) {
        Command *cmd = &p->cmds[i];

        /* Built-in inside a pipeline: fork + exec emulation */
        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); return 1; }

        if (pids[i] == 0) {
            /* Child */

            /* Input side */
            if (i > 0)
                dup2(pipes[i-1][0], STDIN_FILENO);
            if (cmd->redir_in) {
                int fd = open_redir_in(cmd->redir_in);
                if (fd < 0) exit(1);
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            /* Output side */
            if (i < n - 1)
                dup2(pipes[i][1], STDOUT_FILENO);
            if (cmd->redir_out) {
                int fd = open_redir_out(cmd->redir_out, cmd->append_out);
                if (fd < 0) exit(1);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            /* Close all pipe fds in child */
            for (int k = 0; k < n - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            /* Reset signal dispositions */
            signal(SIGINT,  SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            execvp(cmd->argv[0], cmd->argv);
            fprintf(stderr, "%s: %s: command not found\n",
                    "cshell", cmd->argv[0]);
            exit(127);
        }
    }

    /* Parent: close all pipe ends */
    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Wait for all children */
    int last_status = 0;
    for (int i = 0; i < n; i++) {
        if (p->cmds[i].background) continue;
        int status;
        waitpid(pids[i], &status, 0);
        if (i == n - 1)
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
    return last_status;
}
