/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <errno.h>

#include <skalibs/error.h>
#include <skalibs/iopause.h>
#include <skalibs/siovec.h>
#include <skalibs/unix-timed.h>
#include <skalibs/posixishard.h>

int ipc_timed_sendv (int fd, struct iovec const *v, unsigned int n, tain const *deadline, tain *stamp)
{
  struct msghdr hdr =
  {
    .msg_name = 0,
    .msg_namelen = 0,
    .msg_iov = (struct iovec *)v,
    .msg_iovlen = n,
    .msg_control = 0,
    .msg_controllen = 0,
    .msg_flags = 0
  } ;
  size_t len = siovec_len(v, n) ;
  iopause_fd x = { .fd = fd, .events = IOPAUSE_WRITE, .revents = 0 } ;
  for (;;)
  {
    int r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (r < 0) return 0 ;
    else if (!r) return (errno = ETIMEDOUT, 0) ;
    else if (x.revents & IOPAUSE_WRITE)
    {
      if (sendmsg(fd, &hdr, MSG_NOSIGNAL) == (ssize_t)len) break ;
      if (!error_isagain(errno)) return 0 ;
    }
    else if (x.revents & IOPAUSE_EXCEPT) return (sendmsg(fd, &hdr, MSG_NOSIGNAL) == (ssize_t)len) ;
  }
  return 1 ;
}
