/* ISC license. */

#include <string.h>
#include <skalibs/bufalloc.h>

void bufalloc_clean (bufalloc *ba)
{
  if (ba->p)
  {
    memmove(ba->x.s, ba->x.s + ba->p, ba->x.len - ba->p) ;
    ba->x.len -= ba->p ;
    ba->p = 0 ;
  }
}
