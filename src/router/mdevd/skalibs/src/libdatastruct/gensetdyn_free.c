/* ISC license. */

#include <stdint.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/gensetdyn.h>

void gensetdyn_free (gensetdyn *g)
{
  stralloc_free(&g->storage) ;
  genalloc_free(uint32_t, &g->freelist) ;
  *g = gensetdyn_zero ;
}
