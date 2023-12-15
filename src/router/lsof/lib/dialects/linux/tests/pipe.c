#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int no_close = 0;

    if (argc > 1 && strcmp(argv[1], "no-close") == 0)
        no_close = 1;

    int pd[2];

    if (pipe(pd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t self, child;

    self = getpid();
    child = fork();

    if (child == 0) {
        if (!no_close)
            close(pd[0]);
        pause();
        return 0;
    } else if (child < 0) {
        perror("fork");
        return 1;
    }

    if (!no_close)
        close(pd[1]);
    printf("%d %d %d %d\n", self, child, pd[0], pd[1]);
    fflush(stdout);
    wait(NULL);
    return 0;
}
