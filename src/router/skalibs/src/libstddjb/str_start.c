/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

int str_start (char const *s, char const *t)
{
  return !strncmp(s, t, strlen(t)) ;
}
