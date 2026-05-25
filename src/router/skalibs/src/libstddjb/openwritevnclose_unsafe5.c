/* ISC license. */

#include <skalibs/posixplz.h>
#include <skalibs/djbunix.h>

int openwritevnclose_unsafe5 (char const *fn, struct iovec const *v, unsigned int vlen, devino *devino, unsigned int options)
{
  int fd = openc_trunc(fn) ;
  if (fd < 0) return 0 ;
  if (!writevnclose_unsafe5(fd, v, vlen, devino, options))
  {
    fd_close(fd) ;
    unlink_void(fn) ;
    return 0 ;
  }
  return 1 ;
}
