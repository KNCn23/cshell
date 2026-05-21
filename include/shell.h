#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>

/* ── Constants ──────────────────────────────────────────────────────────── */
#define SHELL_NAME      "cshell"
#define MAX_LINE        4096
#define MAX_ARGS        256
#define MAX_PIPES       16
#define HIST_SIZE       100

/* ── Command representation ─────────────────────────────────────────────── */
typedef struct {
    char **argv;       /* NULL-terminated argument list  */
    int    argc;
    char  *redir_in;   /* < filename, or NULL            */
    char  *redir_out;  /* > filename, or NULL            */
    int    append_out; /* >> instead of >                */
    int    background; /* trailing &                     */
} Command;

typedef struct {
    Command cmds[MAX_PIPES];
    int     n;         /* number of commands in pipeline */
} Pipeline;

/* ── Parser ─────────────────────────────────────────────────────────────── */
Pipeline *parse_line(char *line);
void      pipeline_free(Pipeline *p);

/* ── Executor ────────────────────────────────────────────────────────────── */
int execute_pipeline(Pipeline *p);

/* ── Built-ins ───────────────────────────────────────────────────────────── */
int  is_builtin(const char *cmd);
int  run_builtin(Command *cmd);

/* ── History ─────────────────────────────────────────────────────────────── */
void history_add(const char *line);
void history_print(void);
const char *history_get(int n);   /* 1-based */

/* ── Prompt ──────────────────────────────────────────────────────────────── */
void print_prompt(void);

#endif /* SHELL_H */
