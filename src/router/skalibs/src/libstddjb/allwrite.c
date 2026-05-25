/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>

size_t allwrite (int fd, char const *buf, size_t len)
{
  return allreadwrite((io_func_ref)&fd_write, fd, (char *)buf, len) ;
}
