/* ISC license. */

#include <string.h>
#include <strings.h>

#include <skalibs/bytestr.h>

int case_startb (char const *s, size_t slen, char const *t)
{
  size_t tlen = strlen(t) ;
  return slen < tlen ? 0 : !strncasecmp(s, t, tlen) ;
}
