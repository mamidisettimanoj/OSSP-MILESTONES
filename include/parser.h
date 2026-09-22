#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_ARGS 64

typedef struct {
    char **args;
    int *quoted;        // NEW: track which args were single-quoted
    int count;
    int capacity;
} Command;

// Function declarations
Command* parse_command(const char *input);
void print_tokens(const Command *cmd);
void free_command(Command *cmd);
int is_whitespace(char c);
char* extract_token_with_quotes(const char *input, int *pos, int *was_quoted);

#endif
