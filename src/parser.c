#include "../include/parser.h"

int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

char* extract_token_with_quotes(const char *input, int *pos, int *was_quoted) {
    char buffer[1024];
    int buf_pos = 0;
    int i = *pos;
    *was_quoted = 0;
    
    while (input[i] && is_whitespace(input[i])) {
        i++;
    }
    
    if (!input[i]) {
        *pos = i;
        return NULL;
    }
    
    while (input[i] && !is_whitespace(input[i]) && input[i] != '|') {
        if (input[i] == '\'') {
            *was_quoted = 1;
            i++;
            while (input[i] && input[i] != '\'') {
                buffer[buf_pos++] = input[i];
                i++;
            }
            if (input[i] == '\'') {
                i++;
            }
        } else if (input[i] == '\\') {
            i++;
            if (input[i]) {
                buffer[buf_pos++] = input[i];
                i++;
            }
        } else {
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
    cmd->quoted = (int *)malloc(MAX_ARGS * sizeof(int));
    
    if (cmd->args == NULL || cmd->quoted == NULL) {
        perror("malloc failed for args");
        free(cmd->args);
        free(cmd->quoted);
        free(cmd);
        return NULL;
    }
    
    for (int i = 0; i < MAX_ARGS; i++) {
        cmd->args[i] = NULL;
        cmd->quoted[i] = 0;
    }
    
    int pos = 0;
    while (cmd->count < MAX_ARGS - 1) {
        int was_quoted = 0;
        char *token = extract_token_with_quotes(input, &pos, &was_quoted);
        if (token == NULL) {
            break;
        }
        cmd->args[cmd->count] = token;
        cmd->quoted[cmd->count] = was_quoted;
        cmd->count++;
    }
    
    cmd->args[cmd->count] = NULL;
    
    return cmd;
}

// Parse input string by pipes and return array of commands
Command** parse_pipeline(const char *input, int *num_commands) {
    if (input == NULL || num_commands == NULL) {
        return NULL;
    }
    
    // Allocate array of command pointers
    Command **commands = (Command **)malloc(MAX_COMMANDS * sizeof(Command *));
    if (commands == NULL) {
        perror("malloc failed for commands");
        return NULL;
    }
    
    *num_commands = 0;
    int i = 0;
    
    while (input[i] && *num_commands < MAX_COMMANDS) {
        // Find next pipe or end of string
        int pipe_pos = i;
        while (input[pipe_pos] && input[pipe_pos] != '|') {
            // Skip quoted sections to avoid splitting on pipes inside quotes
            if (input[pipe_pos] == '\'') {
                pipe_pos++;
                while (input[pipe_pos] && input[pipe_pos] != '\'') {
                    pipe_pos++;
                }
                if (input[pipe_pos] == '\'') {
                    pipe_pos++;
                }
            } else {
                pipe_pos++;
            }
        }
        
        // Extract substring from i to pipe_pos
        int cmd_len = pipe_pos - i;
        char *cmd_str = (char *)malloc(cmd_len + 1);
        if (cmd_str == NULL) {
            perror("malloc failed for cmd_str");
            free(commands);
            return NULL;
        }
        
        strncpy(cmd_str, &input[i], cmd_len);
        cmd_str[cmd_len] = '\0';
        
        // Parse this command
        Command *cmd = parse_command(cmd_str);
        free(cmd_str);
        
        if (cmd && cmd->count > 0) {
            commands[*num_commands] = cmd;
            (*num_commands)++;
        } else if (cmd) {
            free_command(cmd);
        }
        
        // Skip pipe character if present
        if (input[pipe_pos] == '|') {
            i = pipe_pos + 1;
        } else {
            break;
        }
    }
    
    return commands;
}

void print_tokens(const Command *cmd) {
    if (cmd == NULL) {
        printf("(empty command)\n");
        return;
    }
    
    printf("Tokens (%d):\n", cmd->count);
    for (int i = 0; i < cmd->count; i++) {
        printf("  [%d] '%s'%s\n", i, cmd->args[i], cmd->quoted[i] ? " (quoted)" : "");
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
    free(cmd->quoted);
    free(cmd);
}

void free_pipeline(Command **commands, int num_commands) {
    if (commands == NULL) {
        return;
    }
    
    for (int i = 0; i < num_commands; i++) {
        free_command(commands[i]);
    }
    free(commands);
}
