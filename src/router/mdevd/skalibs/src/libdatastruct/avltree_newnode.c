/* ISC license. */

#include <errno.h>

#include <skalibs/gensetdyn.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltree.h>

int avltree_newnode (avltree *t, uint32_t data, uint32_t *i)
{
  if (!gensetdyn_new(&t->x, i)) return 0 ;
  {
    avlnode *s = avltree_nodes(t) ;
    s[*i].data = data ;
    s[*i].child[0] = s[*i].child[1] = UINT32_MAX ;
    s[*i].balance = 0 ;
  }
  return 1 ;
}
