/* ISC license. */

#include <skalibs/avltree.h>

int avltree_insert (avltree *t, uint32_t d)
{
  uint32_t i ;
  if (!avltree_newnode(t, d, &i)) return 0 ;
  avltree_insertnode(t, i) ;
  return 1 ;
}
