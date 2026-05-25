/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <errno.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/ancil.h>
#include <skalibs/posixishard.h>

union aligner_u
{
  struct cmsghdr cmsghdr ;
  int i ;
} ;

int ancil_recv_fd (int sock, char expected_ch)
{
  static int const awesomeflags =
#ifdef SKALIBS_HASMSGDONTWAIT
    MSG_DONTWAIT
#else
    0
#endif
    |
#ifdef SKALIBS_HASCMSGCLOEXEC
    MSG_CMSG_CLOEXEC
#else
    0
#endif
    ;
  int fd ;
  struct cmsghdr *c ;
  ssize_t r ;
  char ch ;
  struct iovec v = { .iov_base = &ch, .iov_len = 1 } ;
  union aligner_u ancilbuf[1 + (CMSG_SPACE(sizeof(int)) - 1) / sizeof(union aligner_u)] ;
  struct msghdr msghdr =
  {
    .msg_name = 0,
    .msg_namelen = 0,
    .msg_iov = &v,
    .msg_iovlen = 1,
    .msg_flags = 0,
    .msg_control = ancilbuf,
    .msg_controllen = CMSG_SPACE(sizeof(int))
  } ;
  int e = errno ;
  do r = recvmsg(sock, &msghdr, awesomeflags) ;
  while (r < 0 && errno == EINTR) ;
  if (r < 0) return r ;
  if (!r) return (errno = EPIPE, -1) ;
  errno = e ;
  c = CMSG_FIRSTHDR(&msghdr) ;
  if (ch != expected_ch
   || !c
   || c->cmsg_level != SOL_SOCKET
   || c->cmsg_type != SCM_RIGHTS
   || (size_t)(c->cmsg_len - (CMSG_DATA(c) - (unsigned char *)c)) != sizeof(int)) return (errno = EPROTO, -1) ;
  memcpy(&fd, CMSG_DATA(c), sizeof(int)) ;
#ifndef SKALIBS_HASCMSGCLOEXEC
  if (coe(fd) == -1)
  {
    fd_close(fd) ;
    return -1 ;
  }
#endif
  return fd ;
}
