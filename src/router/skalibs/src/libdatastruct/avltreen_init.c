/* ISC license. */

#include <skalibs/genset.h>
#include <skalibs/avlnode.h>
#include <skalibs/avltreen.h>

void avltreen_init (avltreen *t, avlnode *storage, uint32_t *freelist, uint32_t size, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  GENSET_init(&t->x, avlnode, storage, freelist, size) ;
  t->root = size ;
  t->dtok = dtok ;
  t->kcmp = f ;
  t->external = p ;
}
