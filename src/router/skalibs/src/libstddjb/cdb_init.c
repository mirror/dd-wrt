/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/cdb.h>

int cdb_init (cdb *c, char const *file)
{
  int fd = openc_read(file) ;
  if (fd < 0) return 0 ;
  if (!cdb_init_fromfd(c, fd))
  {
    fd_close(fd) ;
    return 0 ;
  }
  fd_close(fd) ;
  return 1 ;
}
