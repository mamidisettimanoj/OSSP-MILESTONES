#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function declarations
void display_prompt(void);
char* read_input(void);
int should_exit(const char *input);
void run_shell(void);

#endif
