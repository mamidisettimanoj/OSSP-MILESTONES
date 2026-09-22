#include "../include/expander.h"
#include "../include/parser.h"

int is_variable_name_char(char c) {
    return (isalnum(c) || c == '_');
}

char* get_env_variable(const char *name) {
    if (name == NULL) {
        return "";
    }
    
    char *value = getenv(name);
    return (value != NULL) ? value : "";
}

char* expand_variables(const char *token) {
    if (token == NULL) {
        return NULL;
    }
    
    // Allocate buffer for expanded string (worst case: no expansion)
    char buffer[4096];
    int buf_pos = 0;
    int i = 0;
    
    while (token[i] && buf_pos < 4095) {
        if (token[i] == '$') {
            // Found a variable reference
            i++;  // skip $
            
            // Read variable name (alphanumeric + underscore)
            char var_name[256];
            int var_pos = 0;
            
            while (token[i] && is_variable_name_char(token[i]) && var_pos < 255) {
                var_name[var_pos++] = token[i];
                i++;
            }
            var_name[var_pos] = '\0';
            
            // If we found a variable name, expand it
            if (var_pos > 0) {
                char *value = get_env_variable(var_name);
                int value_len = strlen(value);
                
                // Check if expansion fits in buffer
                if (buf_pos + value_len < 4096) {
                    strcpy(&buffer[buf_pos], value);
                    buf_pos += value_len;
                }
            }
            // If no variable name after $, keep the $ as-is
            else {
                buffer[buf_pos++] = '$';
            }
        } else if (token[i] == '\\') {
            // Backslash escapes the next character (already handled in parser)
            // But we need to preserve it here if it's not escaping a $
            buffer[buf_pos++] = token[i];
            i++;
        } else {
            buffer[buf_pos++] = token[i];
            i++;
        }
    }
    
    buffer[buf_pos] = '\0';
    
    // Allocate and return the expanded string
    char *result = (char *)malloc(strlen(buffer) + 1);
    if (result == NULL) {
        perror("malloc failed for expanded variable");
        return NULL;
    }
    strcpy(result, buffer);
    return result;
}

// Expand all variables in a command's arguments (skip single-quoted args)
void expand_command(Command *cmd) {
    if (cmd == NULL || cmd->args == NULL) {
        return;
    }
    
    for (int i = 0; cmd->args[i] != NULL; i++) {
        // Skip expansion for single-quoted tokens
        if (cmd->quoted[i]) {
            continue;
        }
        
        char *expanded = expand_variables(cmd->args[i]);
        if (expanded != NULL && expanded != cmd->args[i]) {
            free(cmd->args[i]);
            cmd->args[i] = expanded;
        }
    }
}
