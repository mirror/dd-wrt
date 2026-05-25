/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genqdyn.h>

void genqdyn_free (genqdyn *g)
{
  stralloc_free(&g->queue) ;
  *g = genqdyn_zero ;
}
