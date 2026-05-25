/* ISC license. */

#include <stdint.h>
#include <errno.h>
#include <skalibs/genset.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltreen.h>

int avltreen_delete (avltreen *t, void const *k)
{
  uint32_t r = avltreen_root(t) ;
  uint32_t i = avlnode_delete(avltreen_nodes(t), avltreen_totalsize(t), &r, k, t->dtok, t->kcmp, t->external) ;
  if (i >= avltreen_totalsize(t)) return (errno = ESRCH, 0) ;
  avltreen_setroot(t, r) ;
  if (!genset_delete(&t->x, i)) return 0 ;
  return 1 ;
}
