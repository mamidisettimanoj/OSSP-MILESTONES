#ifndef EXPANDER_H
#define EXPANDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

// Function declarations
char* expand_variables(const char *token);
char* get_env_variable(const char *name);
int is_variable_name_char(char c);
void expand_command(Command *cmd);

#endif
