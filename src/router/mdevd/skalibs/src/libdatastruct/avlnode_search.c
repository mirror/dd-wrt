/* ISC license. */

#include <errno.h>
#include <skalibs/avlnode.h>

int avlnode_search (avlnode const *s, uint32_t max, uint32_t r, void const *k, uint32_t *data, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  uint32_t i = avlnode_searchnode(s, max, r, k, dtok, f, p) ;
  if (i >= max) return (errno = ESRCH, 0) ;
  *data = s[i].data ;
  return 1 ;
}
