/* ISC license. */

#include <string.h>

#include <skalibs/posixplz.h>

int mkdirp (char const *s, mode_t mode)
{
  size_t len = strlen(s) ;
  char tmp[len+1] ;
  memcpy(tmp, s, len+1) ;
  return mkdirp2(tmp, mode) ;
}
