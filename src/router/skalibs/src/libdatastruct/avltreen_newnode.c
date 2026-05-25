/* ISC license. */

#include <skalibs/genset.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltreen.h>

uint32_t avltreen_newnode (avltreen *t, uint32_t d)
{
  uint32_t i = genset_new(&t->x) ;
  if (i < avltreen_totalsize(t))
  {
    avlnode *s = avltreen_nodes(t) ;
    s[i].data = d ;
    s[i].child[0] = s[i].child[1] = avltreen_totalsize(t) ;
    s[i].balance = 0 ;
  }
  return i ;
}
