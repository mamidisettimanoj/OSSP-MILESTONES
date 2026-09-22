#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "parser.h"

// Function declarations
int execute_command(Command *cmd);
int is_builtin(const char *cmd);
int execute_builtin(Command *cmd);
char* expand_cd_path(const char *path);
int is_absolute_path(const char *path);

#endif
