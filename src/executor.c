#include "../include/executor.h"
#include "../include/expander.h"
#include <libgen.h>

// Track previous directory for cd -
static char previous_dir[1024] = "";

int is_absolute_path(const char *path) {
    return (path && path[0] == '/');
}

char* expand_cd_path(const char *path) {
    if (path == NULL) {
        return NULL;
    }
    
    char buffer[2048];  // Larger buffer to prevent truncation
    
    // Handle ~ (home directory)
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (home == NULL) {
            return NULL;
        }
        
        if (path[1] == '\0') {
            // Just ~ → home directory
            strcpy(buffer, home);
        } else if (path[1] == '/') {
            // ~/something → home/something
            snprintf(buffer, sizeof(buffer), "%s%s", home, &path[1]);
        } else {
            // ~user/something (not implemented, just return as-is)
            strcpy(buffer, path);
        }
    } else if (is_absolute_path(path)) {
        // Absolute path, use as-is
        strcpy(buffer, path);
    } else if (strcmp(path, "-") == 0) {
        // cd - → go to previous directory
        if (previous_dir[0] == '\0') {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return NULL;
        }
        strcpy(buffer, previous_dir);
    } else {
        // Relative path: prepend current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return NULL;
        }
        snprintf(buffer, sizeof(buffer), "%s/%s", cwd, path);
    }
    
    // Allocate and return
    char *result = (char *)malloc(strlen(buffer) + 1);
    if (result == NULL) {
        perror("malloc failed");
        return NULL;
    }
    strcpy(result, buffer);
    return result;
}

// Check if command is a built-in
int is_builtin(const char *cmd) {
    if (cmd == NULL) return 0;
    
    if (strcmp(cmd, "cd") == 0) return 1;
    if (strcmp(cmd, "pwd") == 0) return 1;
    if (strcmp(cmd, "exit") == 0) return 1;
    if (strcmp(cmd, "export") == 0) return 1;
    if (strcmp(cmd, "history") == 0) return 1;
    
    return 0;
}

// Execute built-in commands
int execute_builtin(Command *cmd) {
    if (cmd == NULL || cmd->count == 0) {
        return 1;
    }
    
    const char *builtin = cmd->args[0];
    
    // pwd: print working directory
    if (strcmp(builtin, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
            return 1;
        }
        return 0;
    }
    
    // cd: change directory
    if (strcmp(builtin, "cd") == 0) {
        const char *target = NULL;
        
        // Determine target directory
        if (cmd->count < 2) {
            // No argument: go to home
            target = getenv("HOME");
            if (target == NULL) {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
        } else {
            target = cmd->args[1];
        }
        
        // Expand the path
        char *expanded = expand_cd_path(target);
        if (expanded == NULL) {
            return 1;
        }
        
        // Save current directory before changing
        char current_dir[1024];
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            strcpy(previous_dir, current_dir);
        }
        
        // Change directory
        if (chdir(expanded) != 0) {
            perror("cd failed");
            free(expanded);
            return 1;
        }
        
        free(expanded);
        return 0;
    }
    
    // exit: exit shell with optional status code
    if (strcmp(builtin, "exit") == 0) {
        int status = 0;
        
        if (cmd->count > 1) {
            status = atoi(cmd->args[1]);
        }
        
        printf("Goodbye!\n");
        exit(status);
        return 0;  // Never reached
    }
    
    // Other built-ins not yet implemented
    printf("Built-in '%s' not yet implemented\n", builtin);
    return 1;
}

// Execute external command via fork/exec/wait
int execute_command(Command *cmd) {
    if (cmd == NULL || cmd->count == 0) {
        return 1;
    }
    
    // Expand environment variables in arguments (respects single quotes)
    expand_command(cmd);
    
    // Check if it's a built-in command
    if (is_builtin(cmd->args[0])) {
        return execute_builtin(cmd);
    }
    
    // Fork a child process
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process: execute the command
        // cmd->args is already NULL-terminated by parse_command()
        execvp(cmd->args[0], cmd->args);
        
        // execvp only returns if there's an error
        perror("execvp failed");
        exit(127);  // Command not found
    } else {
        // Parent process: wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        // Check if child exited normally
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }
}
