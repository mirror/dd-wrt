/* ISC license. */

#include <skalibs/posixplz.h>

int mkptemp2 (char *s, unsigned int flags)
{
  return mkptemp3(s, 0600, flags) ;
}
