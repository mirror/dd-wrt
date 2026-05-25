/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int open_writeat (int fd, char const *name)
{
  return open2_at(fd, name, O_WRONLY | O_NONBLOCK) ;
}
