#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../include/builtin.h"
#include "../include/history.h"
#include "../include/job.h"
#include "../include/executor.h"

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    return strcmp(cmd, "pwd") == 0 || strcmp(cmd, "cd") == 0 ||
           strcmp(cmd, "exit") == 0 || strcmp(cmd, "export") == 0 ||
           strcmp(cmd, "history") == 0 || strcmp(cmd, "jobs") == 0 ||
           strcmp(cmd, "fg") == 0 || strcmp(cmd, "bg") == 0;
}

int execute_builtin(Command *cmd) {
    if (cmd == NULL || cmd->count == 0) {
        return 1;
    }

    if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
        return 0;
    }

    if (strcmp(cmd->args[0], "cd") == 0) {
        const char *path = (cmd->count > 1) ? cmd->args[1] : getenv("HOME");
        
        if (path == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }

        char old_pwd[1024];
        getcwd(old_pwd, sizeof(old_pwd));

        if (strcmp(path, "-") == 0) {
            path = getenv("OLDPWD");
            if (path == NULL) {
                fprintf(stderr, "cd: OLDPWD not set\n");
                return 1;
            }
        }

        if (chdir(path) != 0) {
            perror("cd");
            return 1;
        }

        setenv("OLDPWD", old_pwd, 1);
        return 0;
    }

    if (strcmp(cmd->args[0], "exit") == 0) {
        int status = 0;
        if (cmd->count > 1) {
            status = atoi(cmd->args[1]);
        }
        printf("Goodbye!\n");
        exit(status);
    }

    if (strcmp(cmd->args[0], "export") == 0) {
        if (cmd->count < 2) {
            fprintf(stderr, "export: missing operand\n");
            return 1;
        }
        char *eq = strchr(cmd->args[1], '=');
        if (eq) {
            *eq = '\0';
            setenv(cmd->args[1], eq + 1, 1);
            *eq = '=';
        }
        return 0;
    }

    if (strcmp(cmd->args[0], "history") == 0) {
        int count = -1;
        if (cmd->count > 1) {
            count = atoi(cmd->args[1]);
        }
        history_display(count);
        return 0;
    }

    if (strcmp(cmd->args[0], "jobs") == 0) {
        print_jobs();
        return 0;
    }

    if (strcmp(cmd->args[0], "fg") == 0) {
        int job_id = -1;
        if (cmd->count > 1) {
            job_id = atoi(cmd->args[1]);
        } else {
            job_id = get_last_job_id();
        }
        
        if (job_id == -1) {
            fprintf(stderr, "fg: no current job\n");
            return 1;
        }
        
        return execute_foreground(job_id);
    }

    if (strcmp(cmd->args[0], "bg") == 0) {
        int job_id = -1;
        if (cmd->count > 1) {
            job_id = atoi(cmd->args[1]);
        } else {
            job_id = get_last_job_id();
        }
        
        if (job_id == -1) {
            fprintf(stderr, "bg: no current job\n");
            return 1;
        }
        
        return execute_background(job_id);
    }

    return 1;
}

// Helper function to display history
void history_display(int count) {
    extern History shell_history;
    int start_index = 0;

    if (count > 0) {
        start_index = shell_history.count - count;
        if (start_index < 0) {
            start_index = 0;
        }
    }

    for (int i = start_index; i < shell_history.count; i++) {
        printf("%3d  %s\n", i, shell_history.commands[i]);
    }
}

// Helper function to get last job ID
int get_last_job_id(void) {
    extern JobTable job_table;
    if (job_table.num_jobs > 0) {
        return job_table.jobs[job_table.num_jobs - 1].job_id;
    }
    return -1;
}
