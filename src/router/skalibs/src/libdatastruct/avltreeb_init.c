/* ISC license. */

#include <stdint.h>

#include <skalibs/avlnode.h>
#include <skalibs/avltreen.h>

void avltreeb_init (void *blob, uint32_t size, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  avltreen *t = blob ;
  avlnode *storage = (avlnode *)(t + 1) ;
  uint32_t *freelist = (uint32_t *)(storage + size) ;
  avltreen_init(t, storage, freelist, size, dtok, f, p) ;
}
