#include "../include/executor.h"
#include "../include/expander.h"

// Check if command is a built-in
int is_builtin(const char *cmd) {
    if (cmd == NULL) return 0;
    
    // Built-in commands we'll implement later
    if (strcmp(cmd, "cd") == 0) return 1;
    if (strcmp(cmd, "pwd") == 0) return 1;
    if (strcmp(cmd, "exit") == 0) return 1;
    if (strcmp(cmd, "export") == 0) return 1;
    if (strcmp(cmd, "history") == 0) return 1;
    
    return 0;
}

// Execute built-in commands (placeholder for now)
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
        }
        return 0;
    }
    
    // cd: change directory (placeholder)
    if (strcmp(builtin, "cd") == 0) {
        if (cmd->count < 2) {
            printf("cd: missing argument\n");
            return 1;
        }
        if (chdir(cmd->args[1]) != 0) {
            perror("cd failed");
            return 1;
        }
        return 0;
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
