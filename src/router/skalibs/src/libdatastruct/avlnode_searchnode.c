/* ISC license. */

#include <skalibs/avlnode.h>
#include "avlnode-internal.h"

uint32_t avlnode_searchnode (avlnode const *s, uint32_t max, uint32_t r, void const *k, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  while (r < max)
  {
    int h = (*f)(k, (*dtok)(s[r].data, p), p) ;
    if (!h) break ;
    r = s[r].child[avlnode_ufroms(h)] ;
  }
  return r ;
}
