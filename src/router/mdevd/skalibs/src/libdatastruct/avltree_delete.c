/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/gensetdyn.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltree.h>

int avltree_delete (avltree *t, void const *k)
{
  uint32_t r = avltree_root(t) ;
  uint32_t i = avlnode_delete(avltree_nodes(t), UINT32_MAX, &r, k, t->dtok, t->kcmp, t->external) ;
  if (i >= UINT32_MAX) return (errno = ESRCH, 0) ;
  avltree_setroot(t, r) ;
  if (!gensetdyn_delete(&t->x, i)) return 0 ;
  return 1 ;
}
