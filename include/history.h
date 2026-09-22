#ifndef HISTORY_H
#define HISTORY_H

#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 100
#define MAX_INPUT 1024

typedef struct {
    char **commands;
    int count;
    int capacity;
    int current_index;
} History;

// Global history object
extern History shell_history;

// Function declarations
void history_init(void);
void history_add(const char *cmd);
char* history_get(int index);
void history_free(void);
int history_size(void);
void history_reset_index(void);

#endif
