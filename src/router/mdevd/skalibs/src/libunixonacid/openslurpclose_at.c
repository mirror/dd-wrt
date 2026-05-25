/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>

int openslurpclose_at (int dirfd, char const *fn, stralloc *sa)
{
  int fd = openc_readatb(dirfd, fn) ;
  if (fd < 0) return 0 ;
  if (!slurp(sa, fd))
  {
    fd_close(fd) ;
    return 0 ;
  }
  fd_close(fd) ;
  return 1 ;
}
