/* ISC license. */

#include <sys/types.h>
#include <skalibs/bufalloc.h>
#include <skalibs/genwrite.h>

ssize_t genwrite_put_bufalloc (void *target, char const *s, size_t len)
{
  return bufalloc_put((bufalloc *)target, s, len) ? (ssize_t)len : -1 ;
}
