/* ISC license. */

#include <stdint.h>
#include <errno.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltree.h>

void avltree_init (avltree *t, uint32_t base, uint32_t fracnum, uint32_t fracden, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  gensetdyn_init(&t->x, sizeof(avlnode), base, fracnum, fracden) ;
  t->root = UINT32_MAX ;
  t->dtok = dtok ;
  t->kcmp = f ;
  t->external = p ;
}
