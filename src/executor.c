#include "../include/executor.h"
#include "../include/expander.h"
#include "../include/history.h"
#include <libgen.h>

static char previous_dir[1024] = "";

int is_absolute_path(const char *path) {
    return (path && path[0] == '/');
}

char* expand_cd_path(const char *path) {
    if (path == NULL) {
        return NULL;
    }
    
    char buffer[2048];
    
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (home == NULL) {
            return NULL;
        }
        
        if (path[1] == '\0') {
            strcpy(buffer, home);
        } else if (path[1] == '/') {
            snprintf(buffer, sizeof(buffer), "%s%s", home, &path[1]);
        } else {
            strcpy(buffer, path);
        }
    } else if (is_absolute_path(path)) {
        strcpy(buffer, path);
    } else if (strcmp(path, "-") == 0) {
        if (previous_dir[0] == '\0') {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return NULL;
        }
        strcpy(buffer, previous_dir);
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return NULL;
        }
        snprintf(buffer, sizeof(buffer), "%s/%s", cwd, path);
    }
    
    char *result = (char *)malloc(strlen(buffer) + 1);
    if (result == NULL) {
        perror("malloc failed");
        return NULL;
    }
    strcpy(result, buffer);
    return result;
}

int setup_redirections(Command *cmd) {
    if (cmd == NULL || cmd->num_redirects == 0) {
        return 0;
    }
    
    for (int i = 0; i < cmd->num_redirects; i++) {
        Redirection *redir = &cmd->redirects[i];
        int fd = -1;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        
        switch (redir->type) {
            case REDIRECT_IN:
                fd = open(redir->filename, O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    return 1;
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                break;
                
            case REDIRECT_OUT:
                fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (fd < 0) {
                    perror("open");
                    return 1;
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;
                
            case REDIRECT_APPEND:
                fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, mode);
                if (fd < 0) {
                    perror("open");
                    return 1;
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;
                
            case REDIRECT_ERR:
                fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (fd < 0) {
                    perror("open");
                    return 1;
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;
                
            case REDIRECT_ERR_APPEND:
                fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, mode);
                if (fd < 0) {
                    perror("open");
                    return 1;
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

int is_builtin(const char *cmd) {
    if (cmd == NULL) return 0;
    
    if (strcmp(cmd, "cd") == 0) return 1;
    if (strcmp(cmd, "pwd") == 0) return 1;
    if (strcmp(cmd, "exit") == 0) return 1;
    if (strcmp(cmd, "export") == 0) return 1;
    if (strcmp(cmd, "history") == 0) return 1;
    
    return 0;
}

int execute_builtin_history(Command *cmd) {
    int start_index = 0;
    
    if (cmd->count > 1) {
        int num_commands = atoi(cmd->args[1]);
        if (num_commands > 0) {
            start_index = shell_history.count - num_commands;
            if (start_index < 0) {
                start_index = 0;
            }
        }
    }
    
    for (int i = start_index; i < shell_history.count; i++) {
        printf("%3d  %s\n", i, shell_history.commands[i]);
    }
    
    return 0;
}

int execute_builtin(Command *cmd) {
    if (cmd == NULL || cmd->count == 0) {
        return 1;
    }
    
    const char *builtin = cmd->args[0];
    
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
    
    if (strcmp(builtin, "cd") == 0) {
        const char *target = NULL;
        
        if (cmd->count < 2) {
            target = getenv("HOME");
            if (target == NULL) {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
        } else {
            target = cmd->args[1];
        }
        
        char *expanded = expand_cd_path(target);
        if (expanded == NULL) {
            return 1;
        }
        
        char current_dir[1024];
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            strcpy(previous_dir, current_dir);
        }
        
        if (chdir(expanded) != 0) {
            perror("cd failed");
            free(expanded);
            return 1;
        }
        
        free(expanded);
        return 0;
    }
    
    if (strcmp(builtin, "exit") == 0) {
        int status = 0;
        
        if (cmd->count > 1) {
            status = atoi(cmd->args[1]);
        }
        
        printf("Goodbye!\n");
        exit(status);
        return 0;
    }
    
    if (strcmp(builtin, "history") == 0) {
        return execute_builtin_history(cmd);
    }
    
    printf("Built-in '%s' not yet implemented\n", builtin);
    return 1;
}

int execute_pipeline(Command **commands, int num_commands) {
    if (commands == NULL || num_commands == 0) {
        return 1;
    }
    
    if (num_commands == 1) {
        return execute_command(commands[0]);
    }
    
    for (int i = 0; i < num_commands; i++) {
        expand_command(commands[i]);
    }
    
    pid_t pids[num_commands];
    int pipes[num_commands - 1][2];
    
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            return 1;
        }
    }
    
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            if (setup_redirections(commands[i]) != 0) {
                exit(1);
            }
            
            execvp(commands[i]->args[0], commands[i]->args);
            perror("execvp failed");
            exit(127);
        } else {
            pids[i] = pid;
        }
    }
    
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    int status = 0;
    for (int i = 0; i < num_commands; i++) {
        int child_status;
        waitpid(pids[i], &child_status, 0);
        if (i == num_commands - 1) {
            status = WIFEXITED(child_status) ? WEXITSTATUS(child_status) : 1;
        }
    }
    
    return status;
}

int execute_command(Command *cmd) {
    if (cmd == NULL || cmd->count == 0) {
        return 1;
    }
    
    expand_command(cmd);
    
    if (is_builtin(cmd->args[0])) {
        return execute_builtin(cmd);
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        if (setup_redirections(cmd) != 0) {
            exit(1);
        }
        
        execvp(cmd->args[0], cmd->args);
        perror("execvp failed");
        exit(127);
    } else {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }
}
