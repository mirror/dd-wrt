/* ISC license. */

#include <sys/stat.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>

int writenclose_unsafe5 (int fd, char const *s, size_t len, devino *devino, unsigned int options)
{
  if (allwrite(fd, s, len) < len) return 0 ;
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
