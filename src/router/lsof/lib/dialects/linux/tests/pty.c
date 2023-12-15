#include <fcntl.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int tfd[2];
    int s;

    if ((tfd[0] = open("/dev/ptmx", O_RDONLY)) < 0)
        err(1, "failed to open ptmx");

    int unlock = 0;
    if (ioctl(tfd[0], TIOCSPTLCK, &unlock) < 0)
        err(1, "failed to do ioctl TIOCSPTLCK");

    if (ioctl(tfd[0], TIOCGPTN, &s) < 0)
        err(1, "failed to do ioctl TIOCGPTN");

    char slave_name[128];
    snprintf(slave_name, 128, "/dev/pts/%d", s);
    if ((tfd[1] = open(slave_name, O_RDONLY)) < 0)
        err(1, "failed to open %s", slave_name);

    pid_t self, child;
    self = getpid();
    child = fork();

    if (child == 0) {
        if (dup2(tfd[1], tfd[0]) < 0)
            err(1, "failed to dup2(%d, %d)", tfd[1], tfd[0]);
        close(tfd[1]);
        pause();
        return 0;
    } else if (child < 0) {
        perror("fork");
        return 1;
    }

    printf("%d %d %d %d %d\n", self, child, tfd[0], tfd[1], s);
    fflush(stdout);
    wait(NULL);
    return 0;
}
