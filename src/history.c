#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/history.h"

History shell_history;

void history_init(void) {
    shell_history.capacity = MAX_HISTORY;
    shell_history.count = 0;
    shell_history.current_index = 0;
    shell_history.commands = (char **)malloc(MAX_HISTORY * sizeof(char *));
    
    if (shell_history.commands == NULL) {
        perror("malloc failed for history");
        exit(1);
    }
    
    // Initialize all pointers to NULL
    for (int i = 0; i < MAX_HISTORY; i++) {
        shell_history.commands[i] = NULL;
    }
}

void history_add(const char *cmd) {
    if (cmd == NULL || strlen(cmd) == 0) {
        return;
    }
    
    // If history is full, shift commands up and remove oldest
    if (shell_history.count >= MAX_HISTORY) {
        free(shell_history.commands[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            shell_history.commands[i] = shell_history.commands[i + 1];
        }
        shell_history.count = MAX_HISTORY - 1;
    }
    
    // Add new command
    shell_history.commands[shell_history.count] = (char *)malloc(strlen(cmd) + 1);
    if (shell_history.commands[shell_history.count] == NULL) {
        perror("malloc failed for command");
        return;
    }
    strcpy(shell_history.commands[shell_history.count], cmd);
    shell_history.count++;
    shell_history.current_index = shell_history.count;
}

char* history_get(int index) {
    if (index < 0 || index >= shell_history.count) {
        return NULL;
    }
    return shell_history.commands[index];
}

void history_free(void) {
    for (int i = 0; i < shell_history.count; i++) {
        if (shell_history.commands[i] != NULL) {
            free(shell_history.commands[i]);
        }
    }
    free(shell_history.commands);
}

int history_size(void) {
    return shell_history.count;
}

void history_reset_index(void) {
    shell_history.current_index = shell_history.count;
}
