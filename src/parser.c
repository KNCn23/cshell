#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Split a string into tokens, respecting single/double quotes. */
static char **tokenize(char *s, int *out_count) {
    char **toks = malloc(MAX_ARGS * sizeof(char *));
    int n = 0;
    char buf[MAX_LINE];
    int bi = 0;
    int in_sq = 0, in_dq = 0;

    for (; *s; s++) {
        char c = *s;
        if (in_sq) {
            if (c == '\'') { in_sq = 0; }
            else           { buf[bi++] = c; }
        } else if (in_dq) {
            if (c == '"')  { in_dq = 0; }
            else if (c == '\\' && *(s+1) == '"') { buf[bi++] = '"'; s++; }
            else           { buf[bi++] = c; }
        } else {
            if      (c == '\'') { in_sq = 1; }
            else if (c == '"')  { in_dq = 1; }
            else if (isspace((unsigned char)c)) {
                if (bi > 0) { buf[bi] = '\0'; toks[n++] = strdup(buf); bi = 0; }
            } else if (c == '|' || c == '<' || c == '>' || c == '&') {
                if (bi > 0) { buf[bi] = '\0'; toks[n++] = strdup(buf); bi = 0; }
                if (c == '>' && *(s+1) == '>') {
                    toks[n++] = strdup(">>"); s++;
                } else {
                    char tmp[2] = { c, '\0' }; toks[n++] = strdup(tmp);
                }
            } else {
                buf[bi++] = c;
            }
        }
    }
    if (bi > 0) { buf[bi] = '\0'; toks[n++] = strdup(buf); }
    toks[n] = NULL;
    *out_count = n;
    return toks;
}

Pipeline *parse_line(char *line) {
    Pipeline *p = calloc(1, sizeof(Pipeline));
    if (!p) return NULL;

    int tc;
    char **toks = tokenize(line, &tc);

    /* Split tokens into commands at '|' */
    int cmd_idx = 0;
    int arg_start = 0;
    Command *cur = &p->cmds[0];
    cur->argv = malloc(MAX_ARGS * sizeof(char *));
    cur->argc = 0;

    for (int i = 0; i <= tc; i++) {
        if (i == tc || strcmp(toks[i], "|") == 0) {
            cur->argv[cur->argc] = NULL;
            cmd_idx++;
            if (i < tc) {
                cur = &p->cmds[cmd_idx];
                cur->argv = malloc(MAX_ARGS * sizeof(char *));
                cur->argc = 0;
                cur->redir_in = cur->redir_out = NULL;
            }
        } else if (strcmp(toks[i], "<") == 0) {
            if (i+1 < tc) cur->redir_in  = strdup(toks[++i]);
        } else if (strcmp(toks[i], ">") == 0) {
            if (i+1 < tc) { cur->redir_out = strdup(toks[++i]); cur->append_out = 0; }
        } else if (strcmp(toks[i], ">>") == 0) {
            if (i+1 < tc) { cur->redir_out = strdup(toks[++i]); cur->append_out = 1; }
        } else if (strcmp(toks[i], "&") == 0) {
            cur->background = 1;
        } else {
            cur->argv[cur->argc++] = strdup(toks[i]);
        }
        free(toks[i]);
    }
    free(toks);

    p->n = cmd_idx;
    if (p->n == 0 || p->cmds[0].argc == 0) {
        pipeline_free(p);
        return NULL;
    }
    return p;
}

void pipeline_free(Pipeline *p) {
    if (!p) return;
    for (int i = 0; i < p->n; i++) {
        Command *c = &p->cmds[i];
        if (c->argv) {
            for (int j = 0; j < c->argc; j++) free(c->argv[j]);
            free(c->argv);
        }
        free(c->redir_in);
        free(c->redir_out);
    }
    free(p);
}
