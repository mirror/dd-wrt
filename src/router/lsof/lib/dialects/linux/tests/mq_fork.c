#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int pid;
    char *fname;

    if (argc != 2) {
        fprintf(stderr, "wrong number of arguments: %d\n", argc);
        return 1;
    } else if (argv[1][0] != '/') {
        fprintf(stderr, "name must starts with '/': %c\n", argv[1][0]);
        return 1;
    }

    fname = argv[1];
    mqd_t t = mq_open(fname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    ;

    if (t == -1) {
        perror("mq_open");
        return 1;
    }

    pid = fork();
    if (pid == 0)
        pause();
    else if (pid > 0) {
        printf("%d %d %d\n", getpid(), pid, t);
        fflush(stdout);
        wait(NULL);
        mq_unlink(fname);
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}
