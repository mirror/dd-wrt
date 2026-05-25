/* ISC license. */

#include <skalibs/posixplz.h>

int mkptemp (char *s)
{
  return mkptemp2(s, 0) ;
}
