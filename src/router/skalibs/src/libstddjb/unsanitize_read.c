/* ISC license. */

#include <errno.h>
#include <skalibs/error.h>
#include <skalibs/allreadwrite.h>

ssize_t unsanitize_read (ssize_t r)
{
  return r == -1 ? errno == EPIPE ? (errno = 0, 0) : -1 :
         !r ? (errno = EWOULDBLOCK, -1) : r ;
}
