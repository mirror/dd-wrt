/* ISC license. */

#include <sys/stat.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/siovec.h>
#include <skalibs/djbunix.h>

int writevnclose_unsafe5 (int fd, struct iovec const *v, unsigned int vlen, devino *devino, unsigned int options)
{
  if (allwritev(fd, v, vlen) < siovec_len(v, vlen)) return 0 ;
  if (options & 1 && fd_sync(fd) == -1 && errno != EINVAL) return 0 ;
  if (devino)
  {
    struct stat st ;
    if (fstat(fd, &st) == -1) return 0 ;
    devino->dev = st.st_dev ;
    devino->ino = st.st_ino ;
  }
  fd_close(fd) ;
  return 1 ;
}
