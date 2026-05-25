/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/gensetdyn.h>

int gensetdyn_ready (gensetdyn *g, uint32_t n)
{
  int wasnull = !g->storage.s ;
  uint32_t i = g->storage.len ;
  if (n < i) return 1 ;
  n += g->base + (n * g->fracnum) / g->fracden ;
  if (!stralloc_ready_tuned(&g->storage, n * g->esize, 0, 0, 1)) return 0 ;
  if (!genalloc_ready(uint32_t, &g->freelist, n))
  {
    if (wasnull) stralloc_free(&g->storage) ;
    return 0 ;
  }
  for (; i < n ; i++)
  {
    uint32_t j = n - 1 - i + g->storage.len ;
    genalloc_catb(uint32_t, &g->freelist, &j, 1) ;
  }
  g->storage.len = n ;
  return 1 ;
}
