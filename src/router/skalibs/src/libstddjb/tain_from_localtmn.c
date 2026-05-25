/* ISC license. */

#include <skalibs/djbtime.h>

int tain_from_localtmn (tain *a, localtmn const *l)
{
  tai t ;
  if (!tai_from_localtm(&t, &l->tm)) return 0 ;
  a->sec = t ;
  a->nano = l->nano ;
  return 1 ;
}
