/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <sys/uio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/uint32.h>
#include <skalibs/disize.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/genalloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/unixmessage.h>
#include <skalibs/posixishard.h>

union aligner_u
{
  struct cmsghdr cmsghdr ;
  int i ;
} ;


 /*
    XXX: sendmsg/recvmsg is badly, badly specified.
    XXX: We assume ancillary data is attached to the first byte.
 */

int unixmessage_sender_flush (unixmessage_sender *b)
{
  disize last = { .left = b->data.len, .right = genalloc_len(int, &b->fds) } ;
  disize *offsets = genalloc_s(disize, &b->offsets) ;
  size_t n = genalloc_len(disize, &b->offsets) ;
  ssize_t r ;

  if (b->shorty) /* we had a short write, gotta send the remainder first */
  {
    disize *next = b->head+1 < n ? offsets + b->head+1 : &last ;
    size_t len = next->left - offsets[b->head].left ;
    if (b->shorty <= len)
      r = fd_write(b->fd, b->data.s + offsets[b->head].left + (len - b->shorty), b->shorty) ;
    else
    {
      size_t nfds = next->right - offsets[b->head].right ;
      char pack[6] ;
      struct iovec v[2] =
      {
        { .iov_base = pack + 6 - (b->shorty - len), .iov_len = b->shorty - len },
        { .iov_base = b->data.s + offsets[b->head].left, .iov_len = len }
      } ;
      uint32_pack_big(pack, (uint32_t)len) ;
      uint16_pack_big(pack + 4, (uint16_t)nfds) ;
      r = fd_writev(b->fd, v, 2) ;
    }
    if (r <= 0) return 0 ;
    b->shorty -= r ;
    if (b->shorty) return (errno = EWOULDBLOCK, 0) ;
  }

  for (; b->head < n ; b->head++)
  {
    disize *next = b->head+1 < n ? offsets + b->head+1 : &last ;
    size_t len = next->left - offsets[b->head].left ;
    size_t nfds = next->right - offsets[b->head].right ;
    char pack[6] ;
    struct iovec v[2] =
    {
      { .iov_base = pack, .iov_len = 6 },
      { .iov_base = b->data.s + offsets[b->head].left, .iov_len = len }
    } ;
    union aligner_u ancilbuf[1 + CMSG_SPACE(nfds * sizeof(int)) / sizeof(union aligner_u)] ;
    struct msghdr hdr =
    {
      .msg_name = 0,
      .msg_namelen = 0,
      .msg_iov = v,
      .msg_iovlen = 2,
      .msg_control = nfds ? ancilbuf : 0,
      .msg_controllen = nfds ? CMSG_SPACE(nfds * sizeof(int)) : 0
    } ;
    uint32_pack_big(pack, (uint32_t)len) ;
    uint16_pack_big(pack + 4, (uint16_t)nfds) ;
    if (nfds)
    {
      struct cmsghdr *cp = CMSG_FIRSTHDR(&hdr) ;
      size_t i = 0 ;
      memset(hdr.msg_control, 0, hdr.msg_controllen) ;
      cp->cmsg_level = SOL_SOCKET ;
      cp->cmsg_type = SCM_RIGHTS ;
      cp->cmsg_len = CMSG_LEN(nfds * sizeof(int)) ;
      for (; i < nfds ; i++)
      {
        int fd = genalloc_s(int, &b->fds)[offsets[b->head].right + i] ;
        ((int *)CMSG_DATA(cp))[i] = fd < 0 ? -(fd+1) : fd ;
      }
    }
    int e = errno ;
    do r = sendmsg(b->fd, &hdr, MSG_NOSIGNAL) ;
    while (r == -1 && errno == EINTR) ;
    if (r <= 0) return 0 ;
    errno = e ;
    if (nfds)
    {
      size_t i = 0 ;
      for (; i < nfds ; i++)
      {
        int fd = genalloc_s(int, &b->fds)[offsets[b->head].right + i] ;
        if (fd < 0) (*b->closecb)(-(fd+1), b->closecbdata) ;
      }
    }
    if ((size_t)r < 6 + len)
    {
      b->shorty = 6 + len - r ;
      return (errno = EWOULDBLOCK, 0) ;
    }
  }
  b->data.len = 0 ;
  genalloc_setlen(int, &b->fds, 0) ;
  genalloc_setlen(disize, &b->offsets, 0) ;
  b->head = 0 ;
  return 1 ;
}
