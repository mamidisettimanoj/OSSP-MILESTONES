#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char command[256];

    printf("Enter a Linux command: ");
    fgets(command, sizeof(command), stdin);

    for (int i = 0; command[i]; i++) {
        if (command[i] == '\n') { command[i] = '\0'; break; }
    }

    printf("Parent PID: %d\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
        execlp(command, command, NULL);
        perror("exec failed");
        exit(1);
    } else {
        wait(NULL);
        printf("Child finished. Parent PID: %d\n", getpid());
    }

    return 0;
}
