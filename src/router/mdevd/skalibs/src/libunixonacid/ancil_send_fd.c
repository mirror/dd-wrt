/* ISC license. */

#include <skalibs/nonposix.h>

#include <errno.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <skalibs/ancil.h>
#include <skalibs/posixishard.h>

union aligner_u
{
  struct cmsghdr cmsghdr ;
  int i ;
} ;

int ancil_send_fd (int sock, int fd, char ch)
{
  ssize_t r ;
  struct iovec v = { .iov_base = &ch, .iov_len = 1 } ;
  union aligner_u ancilbuf[1 + (CMSG_SPACE(sizeof(int)) - 1) / sizeof(union aligner_u)] ;
  struct msghdr hdr =
  {
    .msg_name = 0,
    .msg_namelen = 0,
    .msg_iov = &v,
    .msg_iovlen = 1,
    .msg_control = ancilbuf,
    .msg_controllen = CMSG_SPACE(sizeof(int))
  } ;
  struct cmsghdr *c = CMSG_FIRSTHDR(&hdr) ;
  int e = errno ;
  memset(hdr.msg_control, 0, hdr.msg_controllen) ;
  c->cmsg_level = SOL_SOCKET ;
  c->cmsg_type = SCM_RIGHTS ;
  c->cmsg_len = CMSG_LEN(sizeof(int)) ;
  memcpy(CMSG_DATA(c), &fd, sizeof(int)) ;
  do r = sendmsg(sock, &hdr, MSG_NOSIGNAL) ;
  while (r == -1 && errno == EINTR) ;
  if (r <= 0) return 0 ;
  errno = e ;
  return 1 ;
}
