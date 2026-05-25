/* ISC license. */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <skalibs/types.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/strerr.h>

#include "mdevd-internal.h"

static inline ssize_t fd_recvmsg (int fd, struct msghdr *hdr)
{
  int e = errno ;
  ssize_t r ;
  do r = recvmsg(fd, hdr, 0) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}

static inline size_t netlink_read (int fd, char *s, uint32_t options, unsigned int verbosity)
{
  struct sockaddr_nl nl;
  struct iovec v = { .iov_base = s, .iov_len = UEVENT_MAX_SIZE } ;
  struct msghdr msg =
  {
    .msg_name = &nl,
    .msg_namelen = sizeof(struct sockaddr_nl),
    .msg_iov = &v,
    .msg_iovlen = 1,
    .msg_control = 0,
    .msg_controllen = 0,
    .msg_flags = 0
  } ;
  ssize_t r ;
  for (;;)
  {
    r = sanitize_read(fd_recvmsg(fd, &msg)) ;
    if (r > 0) break ;
    if (!r) return 0 ;
    if (errno != ENOBUFS) strerr_diefu1sys(111, "read netlink message") ;
    strerr_warnw1x("missed events! you should increase the -b kbufsz value") ;
  }
  if (msg.msg_flags & MSG_TRUNC)
    strerr_diefu1x(111, "buffer too small for netlink message") ;
  if (options & 1 && nl.nl_pid)
  {
    if (verbosity >= 3)
    {
      char fmt[PID_FMT] ;
      fmt[pid_fmt(fmt, nl.nl_pid)] = 0 ;
      strerr_warnw2x("received netlink message from userspace process ", fmt) ;
    }
    return 0 ;
  }
  if (s[r-1])
  {
    if (verbosity) strerr_warnw2x("received invalid event: ", "improperly terminated") ;
    return 0 ;
  }
  if (!strstr(s, "@/"))
  {
    if (verbosity) strerr_warnw2x("received invalid event: ", "bad initial summary") ;
    return 0 ;
  }
  return r ;
}

int mdevd_uevent_read (int fd, struct uevent_s *event, uint32_t options, unsigned int verbosity)
{
  unsigned short len = 0 ;
  event->len = netlink_read(fd, event->buf, options, verbosity) ;
  if (!event->len) return 0 ;
  event->varn = 0 ;
  while (len < event->len)
  {
    if (event->varn >= UEVENT_MAX_VARS)
    {
      if (verbosity) strerr_warnw2x("received invalid event: ", "too many variables") ;
      return 0 ;
    }
    event->vars[event->varn++] = len ;
    len += strlen(event->buf + len) + 1 ;
  }
  return 1 ;
}
