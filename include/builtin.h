#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

int is_builtin(const char *cmd);
int execute_builtin(Command *cmd);
void history_display(int count);
int get_last_job_id(void);

#endif
