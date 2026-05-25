/* ISC license. */

#include <string.h>
#include <errno.h>
#include <skalibs/env.h>

int env_make (char const **v, size_t argc, char const *s, size_t len)
{
  while (argc--)
  {
    size_t n = strlen(s) + 1 ;
    if (n > len) return (errno = EINVAL, 0) ;
    *v++ = s ; s += n ; len -= n ;
  }
  return 1 ;
}
