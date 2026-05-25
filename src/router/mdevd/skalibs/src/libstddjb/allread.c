/* ISC license. */

#include <skalibs/allreadwrite.h>

size_t allread (int fd, char *buf, size_t len)
{
  return allreadwrite(&fd_read, fd, buf, len) ;
}
