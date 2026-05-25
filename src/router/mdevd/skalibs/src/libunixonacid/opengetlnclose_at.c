/* ISC license. */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/skamisc.h>
#include <skalibs/unix-transactional.h>

int opengetlnclose_at (int dirfd, char const *fn, stralloc *sa, int sep)
{
  char buf[BUFFER_INSIZE] ;
  buffer b ;
  int r ;
  int fd = openc_readatb(dirfd, fn) ;
  if (fd < 0) return -1 ;
  buffer_init(&b, &buffer_read, fd, buf, BUFFER_INSIZE) ;
  r = skagetln(&b, sa, sep) ;
  fd_close(fd) ;
  return r ;
}
