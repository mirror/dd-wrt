/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/cdb.h>
#include <skalibs/unix-transactional.h>

int cdb_init_at (cdb *c, int dfd, char const *file)
{
  int fd = open_readat(dfd, file) ;
  if (fd < 0) return 0 ;
  if (!cdb_init_fromfd(c, fd))
  {
    fd_close(fd) ;
    return 0 ;
  }
  fd_close(fd) ;
  return 1 ;
}
