/* ISC license. */

#include <errno.h>
#include <unistd.h>

#include <skalibs/posixplz.h>
#include <skalibs/djbunix.h>

int openwritenclose_unsafe5 (char const *fn, char const *s, size_t len, devino *devino, unsigned int options)
{
  int fd = openc_trunc(fn) ;
  if (fd < 0) return 0 ;
  if (!writenclose_unsafe5(fd, s, len, devino, options))
  {
    fd_close(fd) ;
    unlink_void(fn) ;
    return 0 ;
  }
  return 1 ;
}
