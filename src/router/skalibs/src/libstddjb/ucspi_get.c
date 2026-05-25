/* ISC license. */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <skalibs/env.h>

char const *ucspi_get (char const *s)
{
  char const *x = getenv("PROTO") ;
  if (x)
  {
    size_t len = strlen(s) ;
    size_t xlen = strlen(x) ;
    char tmp[len + xlen + 1] ;
    memcpy(tmp, x, xlen) ;
    memcpy(tmp + xlen, s, len + 1) ;
    x = getenv(tmp) ;
    if (!x) errno = ENOENT ;
  }
  else errno = EINVAL ;
  return x ;
}
