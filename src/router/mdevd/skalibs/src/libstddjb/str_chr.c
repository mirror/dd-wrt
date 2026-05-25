/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

size_t str_chr (char const *s, int c)
{
  char *p = strchr(s, c) ;
  return p ? p - s : strlen(s) ;
}
