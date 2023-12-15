#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int epfd = epoll_create(1);
    if (epfd < 0) {
        perror("epoll_create");
        return 1;
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }

    int evfd[2];
    if ((evfd[0] = dup(pipefd[0])) < 0) {
        perror("dup(pipefd[0])");
        return 1;
    }
    if ((evfd[1] = dup(pipefd[1])) < 0) {
        perror("dup(pipefd[1])");
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLOUT;
    ev.data.fd = evfd[1];
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) {
        perror("epoll_ctl<evfd[1]>");
        return 1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = evfd[0];
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) {
        perror("epoll_ctl<evfd[0]>");
        return 1;
    }

    printf("%d %d %d %d\n", getpid(), epfd, evfd[0], evfd[1]);
    fflush(stdout);
    pause();
    return 0;
}
