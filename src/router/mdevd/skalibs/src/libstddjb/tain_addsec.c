/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/tai.h>

int tain_addsec (tain *b, tain const *a, int c)
{
  if (c >= 0)
  {
    tai t = { .x = (uint64_t)c } ;
    tai_add(&b->sec, &a->sec, &t) ;
  }
  else
  {
    tai t = { .x = (uint64_t)-c } ;
    tai_sub(&b->sec, &a->sec, &t) ;
  }
  b->nano = a->nano ;
  return 1 ;
}
