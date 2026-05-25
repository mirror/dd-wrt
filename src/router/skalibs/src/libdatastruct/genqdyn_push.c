/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genqdyn.h>

int genqdyn_push (genqdyn *g, void const *p)
{
  return stralloc_catb(&g->queue, (char const *)p, g->esize) ;
}
