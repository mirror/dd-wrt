#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  int epfd = epoll_create (1);
  if (epfd < 0)
    {
      perror ("epoll_create");
      return 1;
    }

  struct epoll_event ev;

  ev.events = EPOLLOUT;
  ev.data.fd = 2;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0)
    {
      perror ("epoll_ctl");
      return 1;
    }

  ev.events = EPOLLOUT;
  ev.data.fd = 1;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0)
    {
      perror ("epoll_ctl");
      return 1;
    }

  ev.events = EPOLLIN;
  ev.data.fd = 0;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) < 0)
    {
      perror ("epoll_ctl");
      return 1;
    }

  printf ("%d %d\n", getpid(), epfd);
  fflush (stdout);
  getchar ();
  return 0;
}
