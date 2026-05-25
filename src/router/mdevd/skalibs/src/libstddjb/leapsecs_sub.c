/* ISC license. */

#include <skalibs/uint64.h>
#include "djbtime-internal.h"

int leapsecs_sub (uint64_t *t)
{
  uint64_t u = *t ;
  uint64_t d = 0 ;
  unsigned int i = 0 ;
  int hit = 0 ;
  for (; i < leapsecs_table_len ; i++)
  {
    if (u < leapsecs_table[i]) break ;
    if (u == leapsecs_table[i]) hit = 1 ;
    else d++ ;
  }
  *t = u - d ;
  return hit ;
}
