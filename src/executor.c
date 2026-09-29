#include "builtin.h"
#include "../include/executor.h"
#include "../include/expander.h"
#include "../include/history.h"
#include "../include/job.h"
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

int execute_background(int job_id) {
    Job *job = find_job_by_id(job_id);
    
    if (job == NULL) {
        fprintf(stderr, "bg: job %d not found\n", job_id);
        return 1;
    }
    
    // If the job is stopped, send SIGCONT to resume it
    if (job->status == JOB_STOPPED) {
        if (kill(job->pid, SIGCONT) == -1) {
            perror("kill SIGCONT failed");
            return 1;
        }
    }
    
    printf("[%d]+ Continued            %s\n", job->job_id, job->command);
    job->status = JOB_RUNNING;
    
    // DON'T wait — shell returns to prompt immediately
    return 0;
}

int execute_foreground(int job_id) {
    Job *job = find_job_by_id(job_id);
    
    if (job == NULL) {
        fprintf(stderr, "fg: job %d not found\n", job_id);
        return 1;
    }
    
    // If the job is stopped, send SIGCONT to resume it
    if (job->status == JOB_STOPPED) {
        if (kill(job->pid, SIGCONT) == -1) {
            perror("kill SIGCONT failed");
            return 1;
        }
    }
    
    printf("[%d]+ Resumed            %s\n", job->job_id, job->command);
    job->status = JOB_RUNNING;
    
    // Wait for the foreground job to complete
    int status;
    if (waitpid(job->pid, &status, 0) == -1) {
        perror("waitpid failed");
        return 1;
    }
    
    // Print completion status
    if (WIFEXITED(status)) {
        printf("[%d]+  Done                    %s\n", job->job_id, job->command);
        int exit_code = WEXITSTATUS(status);
        remove_job(job->job_id);
        return exit_code;
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("[%d]+  Terminated by signal %d %s\n", job->job_id, sig, job->command);
        remove_job(job->job_id);
        return 1;
    }
    
    remove_job(job->job_id);
    return 0;
}

void sigchld_handler(int sig) {
    (void)sig;
    
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job *job = find_job_by_pid(pid);
        
        if (job != NULL) {
            if (WIFEXITED(status)) {
                printf("[%d]+  Done                    %s\n", job->job_id, job->command);
            } else if (WIFSIGNALED(status)) {
                int sig_num = WTERMSIG(status);
                printf("[%d]+  Terminated by signal %d %s\n", job->job_id, sig_num, job->command);
            }
            remove_job(job->job_id);
        }
    }
}

void sigint_handler(int sig) {
    (void)sig;
}

void sigtstp_handler(int sig) {
    (void)sig;
}

int execute_pipeline(Command **commands, int num_commands) {
    if (commands == NULL || num_commands == 0) {
        return 1;
    }
    
    // Check if last command is background
    int is_background = (commands[num_commands - 1]->is_background);
    
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
    
    if (is_background) {
        // Background: add to job table and return immediately
        char cmd_str[1024] = "";
        for (int i = 0; i < num_commands; i++) {
            for (int j = 0; j < commands[i]->count; j++) {
                strcat(cmd_str, commands[i]->args[j]);
                if (j < commands[i]->count - 1) {
                    strcat(cmd_str, " ");
                }
            }
            if (i < num_commands - 1) {
                strcat(cmd_str, " | ");
            }
        }
        add_job(pids[num_commands - 1], cmd_str);
        status = 0;
    } else {
        // Foreground: wait for all children
        for (int i = 0; i < num_commands; i++) {
            int child_status;
            waitpid(pids[i], &child_status, 0);
            if (i == num_commands - 1) {
                status = WIFEXITED(child_status) ? WEXITSTATUS(child_status) : 1;
            }
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
        if (cmd->is_background) {
            // Background job: add to table and return
            char cmd_str[1024] = "";
            for (int i = 0; i < cmd->count; i++) {
                strcat(cmd_str, cmd->args[i]);
                if (i < cmd->count - 1) {
                    strcat(cmd_str, " ");
                }
            }
            add_job(pid, cmd_str);
            return 0;
        } else {
            // Foreground: wait for child
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                return 1;
            }
        }
    }
}
