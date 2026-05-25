/* ISC license. */

#include <skalibs/djbunix.h>

int openslurpnclose (char const *fn, stralloc *sa, size_t max)
{
  int r ;
  int fd = openbc_read(fn) ;
  if (fd == -1) return 0 ;
  r = slurpn(fd, sa, max) ;
  fd_close(fd) ;
  return r ;
}
