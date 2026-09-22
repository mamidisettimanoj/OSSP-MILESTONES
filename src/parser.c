#include "../include/parser.h"

int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// Extract a single token, handling single quotes and backslash escapes
char* extract_token_with_quotes(const char *input, int *pos) {
    char buffer[1024];
    int buf_pos = 0;
    int i = *pos;
    
    // Skip leading whitespace
    while (input[i] && is_whitespace(input[i])) {
        i++;
    }
    
    if (!input[i]) {
        *pos = i;
        return NULL;
    }
    
    // Read token character by character, handling quotes and escapes
    while (input[i] && !is_whitespace(input[i])) {
        if (input[i] == '\'') {
            // Single quote: collect everything until closing quote
            i++;  // skip opening quote
            while (input[i] && input[i] != '\'') {
                buffer[buf_pos++] = input[i];
                i++;
            }
            if (input[i] == '\'') {
                i++;  // skip closing quote
            }
        } else if (input[i] == '\\') {
            // Backslash: escape next character
            i++;  // skip backslash
            if (input[i]) {
                buffer[buf_pos++] = input[i];  // add escaped character literally
                i++;
            }
        } else {
            // Regular character
            buffer[buf_pos++] = input[i];
            i++;
        }
    }
    
    if (buf_pos == 0) {
        *pos = i;
        return NULL;
    }
    
    buffer[buf_pos] = '\0';
    *pos = i;
    
    // Allocate and return token
    char *token = (char *)malloc(buf_pos + 1);
    if (token == NULL) {
        perror("malloc failed for token");
        return NULL;
    }
    strcpy(token, buffer);
    return token;
}

Command* parse_command(const char *input) {
    if (input == NULL) {
        return NULL;
    }
    
    Command *cmd = (Command *)malloc(sizeof(Command));
    if (cmd == NULL) {
        perror("malloc failed for Command");
        return NULL;
    }
    
    cmd->capacity = MAX_ARGS;
    cmd->count = 0;
    cmd->args = (char **)malloc(MAX_ARGS * sizeof(char *));
    
    if (cmd->args == NULL) {
        perror("malloc failed for args");
        free(cmd);
        return NULL;
    }
    
    // Initialize all pointers to NULL
    for (int i = 0; i < MAX_ARGS; i++) {
        cmd->args[i] = NULL;
    }
    
    // Tokenize with quote and escape support
    int pos = 0;
    while (cmd->count < MAX_ARGS - 1) {
        char *token = extract_token_with_quotes(input, &pos);
        if (token == NULL) {
            break;
        }
        cmd->args[cmd->count] = token;
        cmd->count++;
    }
    
    // NULL-terminate the array (required for exec*)
    cmd->args[cmd->count] = NULL;
    
    return cmd;
}

void print_tokens(const Command *cmd) {
    if (cmd == NULL) {
        printf("(empty command)\n");
        return;
    }
    
    printf("Tokens (%d):\n", cmd->count);
    for (int i = 0; i < cmd->count; i++) {
        printf("  [%d] '%s'\n", i, cmd->args[i]);
    }
}

void free_command(Command *cmd) {
    if (cmd == NULL) {
        return;
    }
    
    for (int i = 0; i < cmd->count; i++) {
        if (cmd->args[i] != NULL) {
            free(cmd->args[i]);
        }
    }
    free(cmd->args);
    free(cmd);
}
