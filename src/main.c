#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "shell.h"

/* Ignore Ctrl-C in the shell process itself; child groups get it. */
static void sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

int main(void) {
    signal(SIGINT,  sigint_handler);
    signal(SIGTSTP, SIG_IGN);   /* Ctrl-Z: ignore at shell level */
    signal(SIGCHLD, SIG_DFL);   /* let waitpid reap children     */

    char *line;
    while ((line = readline("")) != NULL) {
        /* Trim leading/trailing whitespace */
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        size_t len = strlen(s);
        while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\n' ||
                           s[len-1] == '\t')) s[--len] = '\0';

        if (*s == '\0') { free(line); continue; }

        history_add(s);
        add_history(s);   /* readline history */

        Pipeline *p = parse_line(s);
        if (p) {
            execute_pipeline(p);
            pipeline_free(p);
        }
        free(line);
    }

    printf("\nexit\n");
    return 0;
}
