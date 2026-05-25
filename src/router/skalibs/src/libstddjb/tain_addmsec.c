/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/tai.h>

int tain_addmsec (tain *b, tain const *a, unsigned int c)
{
  tain tn = { .sec = { .x = (uint64_t)c / 1000 }, .nano = (c % 1000) * 1000000U } ;
  return tain_add(b, a, &tn) ;
}
