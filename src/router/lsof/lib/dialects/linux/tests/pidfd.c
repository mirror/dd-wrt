#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#ifndef __NR_pidfd_open
#    define __NR_pidfd_open 434 /* System call # on most architectures */
#endif

int main(void) {
    pid_t self = getpid();
    int pidfd = syscall(__NR_pidfd_open, self, 0);
    if (pidfd < 0) {
        if (errno == ENOSYS) {
            printf("%d %d\n", -1, -1);
        }
        perror("pidfd_open");
        return 1;
    }
    printf("%d %d\n\n\n", self, pidfd);
    fclose(stdout);
    pause();
    return 0;
}
