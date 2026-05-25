/* ISC license. */

#include <unistd.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>

size_t openwritenclose_at (int dirfd, char const *file, char const *s, size_t n)
{
  size_t r ;
  int fd = openc_truncatb(dirfd, file) ;
  if (fd < 0) return 0 ;
  r = allwrite(fd, s, n) ;
  if (r >= n) fsync(fd) ;
  fd_close(fd) ;
  return r ;
}
