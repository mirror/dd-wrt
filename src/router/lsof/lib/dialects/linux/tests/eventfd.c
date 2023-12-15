#include <sys/eventfd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

static int fd = -1;

int main(int argc, char **argv) {
    fd = eventfd(0, 0);
    if (fd < 0) {
        perror("eventfd");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        pause();
    } else {
        printf("%d %d %d\n", getpid(), pid, fd);
        fflush(stdout);
        wait(NULL);
    }
    return 0;
}
