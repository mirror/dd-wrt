#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define NAME "/xxx"

static void do_nothing(int n) { signal(SIGCONT, do_nothing); }

int main(void) {
    mqd_t t = mq_open(NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    if ((mqd_t)t == -1) {
        perror("open[" NAME "]");
        return 1;
    }

    printf("pid: %d / fd: %d\n", getpid(), t);
    fflush(stdout);
    signal(SIGCONT, do_nothing);
    pause();
    return 0;
}
