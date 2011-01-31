/*
 * poll2select() -- function to convert poll() to select()
 * until poll() gets fixed and is usable again
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#if __GLIBC__ >= 2
#include <sys/select.h>
#include <sys/poll.h>
#else /* ugly hack to make it compile on RH 4.2 - WA */
#include <linux/poll.h>
#endif

int poll2select(struct pollfd *fds, int fds_used, int timeout)
{
        int i, retval, maxfd = 0;
        struct timeval tv, *tvp;
        fd_set rset, wset;

        FD_ZERO(&rset);
        FD_ZERO(&wset);

        for (i = 0; i < fds_used; i++) {
                if (fds[i].fd == -1) continue;
                if (fds[i].events & POLLIN)  FD_SET(fds[i].fd, &rset);
                if (fds[i].events & POLLOUT) FD_SET(fds[i].fd, &wset);
                if (fds[i].fd > maxfd) maxfd = fds[i].fd;
        }

        tvp = &tv;
        if (timeout == -1) tvp = NULL;
        else {
                tv.tv_sec  = timeout/1000;
                tv.tv_usec = 0;
        }

        retval = select(maxfd + 1, &rset, &wset, NULL, tvp);

        for (i = 0; i < fds_used; i++) {
                fds[i].revents = 0;
                if (fds[i].fd == -1) continue;
                if (FD_ISSET(fds[i].fd, &rset)) fds[i].revents = POLLIN;
                if (FD_ISSET(fds[i].fd, &wset)) fds[i].revents |= POLLOUT;
        }

        return retval;
}
