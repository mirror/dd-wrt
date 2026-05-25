/* ISC license. */

#include <sys/types.h>
#include <skalibs/genalloc.h>
#include <skalibs/gensetdyn.h>

int gensetdyn_new (gensetdyn *g, uint32_t *i)
{
  size_t n ;
  if (!genalloc_len(uint32_t, &g->freelist) && !gensetdyn_readyplus(g, 1)) return 0 ;
  n = genalloc_len(uint32_t, &g->freelist) ;
  *i = genalloc_s(uint32_t, &g->freelist)[n-1] ;
  genalloc_setlen(uint32_t, &g->freelist, n-1) ;
  return 1 ;
}
