/* ISC license. */

#include <skalibs/uint64.h>
#include "djbtime-internal.h"

void leapsecs_add (uint64_t *t, int hit)
{
  uint64_t u = *t - !!hit ;
  unsigned int i = 0 ;
  for (; i < leapsecs_table_len ; i++)
  {
    if (u < leapsecs_table[i]) break ;
    if (!hit || (leapsecs_table[i] < u)) ++u ;
  }
  *t = u ;
}
