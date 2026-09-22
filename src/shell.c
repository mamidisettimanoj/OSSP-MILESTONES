#include "../include/shell.h"
#include "../include/history.h"
#include "../include/parser.h"
#include <termios.h>

void display_prompt(void) {
    printf("shell> ");
    fflush(stdout);
}

char* read_input_with_history(void) {
    char buffer[MAX_INPUT];
    int pos = 0;
    int c;
    
    // Disable canonical mode to read arrow keys
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    while (1) {
        c = getchar();
        
        // Detect escape sequence for arrow keys
        if (c == 27) {  // ESC character
            int bracket = getchar();
            if (bracket == '[') {
                int arrow = getchar();
                if (arrow == 'A') {  // UP arrow
                    if (shell_history.current_index > 0) {
                        shell_history.current_index--;
                        char *hist_cmd = history_get(shell_history.current_index);
                        if (hist_cmd != NULL) {
                            printf("\r");
                            printf("shell> ");
                            printf("%s", hist_cmd);
                            strncpy(buffer, hist_cmd, MAX_INPUT - 1);
                            pos = strlen(hist_cmd);
                            fflush(stdout);
                        }
                    }
                    continue;
                } else if (arrow == 'B') {  // DOWN arrow
                    if (shell_history.current_index < shell_history.count) {
                        shell_history.current_index++;
                        if (shell_history.current_index < shell_history.count) {
                            char *hist_cmd = history_get(shell_history.current_index);
                            if (hist_cmd != NULL) {
                                printf("\r");
                                printf("shell> ");
                                printf("%s", hist_cmd);
                                strncpy(buffer, hist_cmd, MAX_INPUT - 1);
                                pos = strlen(hist_cmd);
                                fflush(stdout);
                            }
                        } else {
                            printf("\r");
                            printf("shell> ");
                            buffer[0] = '\0';
                            pos = 0;
                            fflush(stdout);
                        }
                    }
                    continue;
                }
            }
            continue;
        }
        
        // Handle regular characters
        if (c == '\n') {
            printf("\n");
            buffer[pos] = '\0';
            tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
            
            char *input = (char *)malloc(strlen(buffer) + 1);
            if (input == NULL) {
                perror("malloc failed");
                exit(1);
            }
            strcpy(input, buffer);
            return input;
        } else if (c == 127) {  // Backspace
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        } else if (pos < MAX_INPUT - 1) {
            buffer[pos++] = c;
            printf("%c", c);
            fflush(stdout);
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    return NULL;
}

int should_exit(const char *input) {
    return (input != NULL && strcmp(input, "exit") == 0);
}

void run_shell(void) {
    history_init();
    
    printf("ShellForge - Simple Unix Shell\n");
    printf("Type 'exit' to quit. Use UP/DOWN arrows for history.\n\n");
    
    while (1) {
        display_prompt();
        char *input = read_input_with_history();
        
        if (input == NULL) {
            printf("\n");
            break;
        }
        
        // Skip empty lines
        if (strlen(input) == 0) {
            free(input);
            history_reset_index();
            continue;
        }
        
        // Add to history
        history_add(input);
        
        // Check for exit
        if (should_exit(input)) {
            printf("Goodbye!\n");
            free(input);
            break;
        }
        
        // Parse the command
        Command *cmd = parse_command(input);
        if (cmd && cmd->count > 0) {
            print_tokens(cmd);
        } else {
            printf("Parse error or empty command\n");
        }
        
        if (cmd) {
            free_command(cmd);
        }
        
        history_reset_index();
        free(input);
    }
    
    history_free();
}

int main(void) {
    run_shell();
    return 0;
}
