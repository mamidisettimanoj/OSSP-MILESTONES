#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_ARGS 64
#define MAX_COMMANDS 16
#define MAX_REDIRECTS 8

typedef enum {
    REDIRECT_NONE,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND,
    REDIRECT_ERR,
    REDIRECT_ERR_APPEND,
} RedirectType;

typedef struct {
    RedirectType type;
    char *filename;
} Redirection;

typedef struct {
    char **args;
    int *quoted;
    int count;
    int capacity;
    Redirection *redirects;
    int num_redirects;
    int is_background;  // NEW: true if command ends with &
} Command;

// Function declarations
Command* parse_command(const char *input);
Command** parse_pipeline(const char *input, int *num_commands);
void print_tokens(const Command *cmd);
void free_command(Command *cmd);
void free_pipeline(Command **commands, int num_commands);
int is_whitespace(char c);
char* extract_token_with_quotes(const char *input, int *pos, int *was_quoted);

#endif
