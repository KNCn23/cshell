#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

/* ── History ─────────────────────────────────────────────────────────────── */
static char *hist[HIST_SIZE];
static int   hist_n = 0;

void history_add(const char *line) {
    if (hist_n < HIST_SIZE) {
        hist[hist_n++] = strdup(line);
    } else {
        free(hist[0]);
        memmove(hist, hist+1, (HIST_SIZE-1) * sizeof(char *));
        hist[HIST_SIZE-1] = strdup(line);
    }
}

void history_print(void) {
    for (int i = 0; i < hist_n; i++)
        printf("%4d  %s\n", i+1, hist[i]);
}

const char *history_get(int n) {
    if (n < 1 || n > hist_n) return NULL;
    return hist[n-1];
}

/* ── Built-in table ──────────────────────────────────────────────────────── */
static const char *BUILTINS[] = {
    "cd", "exit", "pwd", "history", "help", NULL
};

int is_builtin(const char *cmd) {
    for (int i = 0; BUILTINS[i]; i++)
        if (strcmp(cmd, BUILTINS[i]) == 0) return 1;
    return 0;
}

int run_builtin(Command *cmd) {
    const char *name = cmd->argv[0];

    /* cd */
    if (strcmp(name, "cd") == 0) {
        const char *dir = cmd->argc > 1 ? cmd->argv[1] : getenv("HOME");
        if (!dir) dir = "/";
        if (chdir(dir) < 0) { perror("cd"); return 1; }
        return 0;
    }

    /* pwd */
    if (strcmp(name, "pwd") == 0) {
        char buf[PATH_MAX];
        if (getcwd(buf, sizeof(buf))) puts(buf);
        else perror("pwd");
        return 0;
    }

    /* history */
    if (strcmp(name, "history") == 0) {
        history_print();
        return 0;
    }

    /* help */
    if (strcmp(name, "help") == 0) {
        puts("cshell — a minimal Unix shell");
        puts("Built-in commands:");
        puts("  cd [dir]     Change directory (default: $HOME)");
        puts("  pwd          Print working directory");
        puts("  history      Show command history");
        puts("  help         Show this message");
        puts("  exit [code]  Exit the shell");
        puts("\nFeatures: pipes ( | ), I/O redirection ( < > >> ), background ( & )");
        return 0;
    }

    /* exit */
    if (strcmp(name, "exit") == 0) {
        int code = cmd->argc > 1 ? atoi(cmd->argv[1]) : 0;
        exit(code);
    }

    return 127;
}
