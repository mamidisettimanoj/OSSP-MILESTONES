#include "../include/shell.h"

void display_prompt(void) {
    printf("shell> ");
    fflush(stdout);
}

char* read_input(void) {
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return NULL;
    }
    
    // Remove newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    // Allocate memory and copy
    char *input = malloc(strlen(buffer) + 1);
    if (input == NULL) {
        perror("malloc failed");
        exit(1);
    }
    strcpy(input, buffer);
    return input;
}

int should_exit(const char *input) {
    return (input != NULL && strcmp(input, "exit") == 0);
}

void run_shell(void) {
    printf("ShellForge - Simple Unix Shell\n");
    printf("Type 'exit' to quit\n\n");
    
    while (1) {
        display_prompt();
        char *input = read_input();
        
        if (input == NULL) {
            // EOF reached
            printf("\n");
            break;
        }
        
        // Skip empty lines
        if (strlen(input) == 0) {
            free(input);
            continue;
        }
        
        // Check for exit
        if (should_exit(input)) {
            printf("Goodbye!\n");
            free(input);
            break;
        }
        
        // Placeholder: will implement command execution in Session 3
        printf("Command received: %s (not yet implemented)\n", input);
        free(input);
    }
}

int main(void) {
    run_shell();
    return 0;
}
