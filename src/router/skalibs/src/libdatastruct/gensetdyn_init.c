/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/gensetdyn.h>

void gensetdyn_init (gensetdyn *g, uint32_t esize, uint32_t base, uint32_t fracnum, uint32_t fracden)
{
  g->storage = stralloc_zero ;
  g->freelist = genalloc_zero ;
  g->esize = esize ;
  g->base = base ;
  g->fracnum = fracnum ;
  g->fracden = fracden ;
}
