/* ISC license. */

#include <errno.h>
#include <skalibs/genqdyn.h>
#include "genqdyn-internal.h"

int genqdyn_pop (genqdyn *g)
{
  if (g->head >= g->queue.len) return (errno = EINVAL, 0) ;
  g->head += g->esize ;
  if (g->den * (g->queue.len - g->head) <= g->num * g->queue.len) genqdyn_clean(g) ;
  return 1 ;
}
