#include "../include/parser.h"

int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
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
    
    // Skip leading whitespace
    int i = 0;
    while (input[i] && is_whitespace(input[i])) {
        i++;
    }
    
    // Tokenize
    while (input[i] && cmd->count < MAX_ARGS - 1) {
        // Skip whitespace between tokens
        while (input[i] && is_whitespace(input[i])) {
            i++;
        }
        
        if (!input[i]) {
            break;
        }
        
        // Find end of token
        int start = i;
        while (input[i] && !is_whitespace(input[i])) {
            i++;
        }
        int end = i;
        
        // Extract token
        int token_len = end - start;
        char *token = (char *)malloc(token_len + 1);
        if (token == NULL) {
            perror("malloc failed for token");
            free_command(cmd);
            return NULL;
        }
        
        strncpy(token, &input[start], token_len);
        token[token_len] = '\0';
        
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
