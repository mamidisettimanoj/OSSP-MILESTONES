#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "parser.h"

// Function declarations
int execute_command(Command *cmd);
int execute_pipeline(Command **commands, int num_commands);
int is_builtin(const char *cmd);
int execute_builtin(Command *cmd);
char* expand_cd_path(const char *path);
int is_absolute_path(const char *path);
int setup_redirections(Command *cmd);

#endif

int execute_foreground(int job_id);

int execute_foreground(int job_id);

int execute_background(int job_id);
