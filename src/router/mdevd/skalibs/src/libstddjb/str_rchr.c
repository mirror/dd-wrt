/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

size_t str_rchr (char const *s, int c)
{
  char *p = strrchr(s, c) ;
  return p ? p - s : strlen(s) ;
}

