/* ISC license. */

#include <errno.h>
#include <skalibs/error.h>
#include <skalibs/allreadwrite.h>

ssize_t sanitize_read (ssize_t r)
{
  return r == -1 ? error_isagain(errno) ? (errno = 0, 0) : -1 :
         !r ? (errno = EPIPE, -1) : r ;
}
