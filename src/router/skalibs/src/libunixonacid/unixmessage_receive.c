/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <skalibs/uint16.h>
#include <skalibs/uint32.h>
#include <skalibs/cbuffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/stralloc.h>
#include <skalibs/unixmessage.h>
#include <skalibs/posixishard.h>

union aligner_u
{
  struct cmsghdr cmsghdr ;
  int i ;
} ;

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

static int unixmessage_receiver_fill (unixmessage_receiver *b)
{
  union aligner_u ancilbuf[1 + CMSG_SPACE(b->auxb.a - 1) / sizeof(union aligner_u)] ;
  struct iovec iov[2] ;
  struct msghdr msghdr =
  {
    .msg_name = 0,
    .msg_namelen = 0,
    .msg_iov = iov,
    .msg_iovlen = 2,
    .msg_flags = 0,
    .msg_control = b->fds_ok & 1 ? ancilbuf : 0,
    .msg_controllen = b->fds_ok & 1 ? CMSG_SPACE(b->auxb.a - 1) : 0
  } ;
  int e = errno ;
  ssize_t r = -1 ;
  if (cbuffer_isfull(&b->mainb) || ((b->fds_ok & 1) && cbuffer_isfull(&b->auxb)))
    return (errno = ENOBUFS, -1) ;
  cbuffer_wpeek(&b->mainb, iov) ;
  while (r == -1)
  {
    r = recvmsg(b->fd, &msghdr, awesomeflags) ;
    if (!r || (r == -1 && errno != EINTR)) return r ;
  }
  if (r > 0) errno = e ;
  if (b->fds_ok & 1)
  {
    struct cmsghdr *c = CMSG_FIRSTHDR(&msghdr) ;
    if (c)
    {
      size_t auxlen ;
      if (c->cmsg_level != SOL_SOCKET
       || c->cmsg_type != SCM_RIGHTS) return (errno = EPROTO, -1) ;
      auxlen = (size_t)(c->cmsg_len - (CMSG_DATA(c) - (unsigned char *)c)) ;
      if (auxlen && !(b->fds_ok & 2))
      {
        size_t i = auxlen / sizeof(int) ;
        while (i--) fd_close(((int *)CMSG_DATA(c))[i]) ;
        return (errno = EPROTO, -1) ;
      }
#ifndef SKALIBS_HASCMSGCLOEXEC
      {
        size_t i = 0 ;
        for (; i < auxlen / sizeof(int) ; i++)
          if (coe(((int *)CMSG_DATA(c))[i]) == -1)
          {
            i++ ;
            while (i--) fd_close(((int *)CMSG_DATA(c))[i]) ;
            return -1 ;
          }
      }
#endif
      if ((msghdr.msg_flags & MSG_CTRUNC) || cbuffer_put(&b->auxb, (char *)CMSG_DATA(c), auxlen) < auxlen)
      {
        size_t i = auxlen/sizeof(int) ;
        while (i--) fd_close(((int *)CMSG_DATA(c))[i]) ;
        return (errno = ENOBUFS, -1) ;
      }
    }
  }
  cbuffer_WSEEK(&b->mainb, r) ;
  return 1 ;
}

int unixmessage_receive (unixmessage_receiver *b, unixmessage *m)
{
  if (b->maindata.len == b->mainlen && b->auxdata.len == b->auxlen)
  {
    char pack[6] ;
    if (cbuffer_len(&b->mainb) < 6)
    {
      ssize_t r = sanitize_read(unixmessage_receiver_fill(b)) ;
      if (r <= 0) return r ;
      if (cbuffer_len(&b->mainb) < 6) return (errno = EWOULDBLOCK, 0) ;
    }
    cbuffer_get(&b->mainb, pack, 6) ;
    uint32_unpack_big(pack, &b->mainlen) ;
    if (b->fds_ok & 1) uint16_unpack_big(pack + 4, &b->auxlen) ;
    else b->auxlen = 0 ;
    b->auxlen *= sizeof(int) ;
    if (b->mainlen > UNIXMESSAGE_MAXSIZE
     || b->auxlen > ((b->fds_ok & 2) ? UNIXMESSAGE_MAXFDS * sizeof(int) : 0))
      return (errno = EPROTO, -1) ;
    if (!stralloc_ready(&b->maindata, b->mainlen)
     || !stralloc_ready(&b->auxdata, b->auxlen))
      return -1 ;
    b->maindata.len = 0 ;
    b->auxdata.len = 0 ;
  }

  for (;;)
  {
    ssize_t r ;
    size_t n = cbuffer_len(&b->mainb) ;
    if (n > b->mainlen - b->maindata.len) n = b->mainlen - b->maindata.len ;
    b->maindata.len += cbuffer_get(&b->mainb, b->maindata.s + b->maindata.len, n) ;
    n = cbuffer_len(&b->auxb) ;
    if (n > b->auxlen - b->auxdata.len) n = b->auxlen - b->auxdata.len ;
    b->auxdata.len += cbuffer_get(&b->auxb, b->auxdata.s + b->auxdata.len, n) ;
    if (b->maindata.len == b->mainlen && b->auxdata.len == b->auxlen) break ;
    r = sanitize_read(unixmessage_receiver_fill(b)) ;
    if (r <= 0) return r ;
  }

  m->s = b->maindata.s ;
  m->len = b->maindata.len ;
  m->fds = (int *)b->auxdata.s ;
  m->nfds = b->auxdata.len / sizeof(int) ;
  return 1 ;
}
