/* ISC license. */

#include <skalibs/avltreen.h>

int avltreen_insert (avltreen *t, uint32_t d)
{
  uint32_t i = avltreen_newnode(t, d) ;
  if (i >= avltreen_totalsize(t)) return 0 ;
  avltreen_insertnode(t, i) ;
  return 1 ;
}
