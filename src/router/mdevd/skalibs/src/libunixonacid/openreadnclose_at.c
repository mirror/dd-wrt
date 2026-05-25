/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>

ssize_t openreadnclose_at (int dirfd, char const *file, char *s, size_t n)
{
  int fd = openc_readatb(dirfd, file) ;
  return fd == -1 ? -1 : readnclose(fd, s, n) ;
}
