/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genqdyn.h>

void genqdyn_init (genqdyn *g, size_t esize, unsigned int num, unsigned int den)
{
  g->queue = stralloc_zero ;
  g->esize = esize ;
  g->head = 0 ;
  g->num = num ;
  g->den = den ;
}
