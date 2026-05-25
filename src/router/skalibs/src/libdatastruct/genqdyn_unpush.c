/* ISC license. */

#include <errno.h>
#include <skalibs/stralloc.h>
#include <skalibs/genqdyn.h>

int genqdyn_unpush (genqdyn *g)
{
  if (g->queue.len < g->esize) return (errno = EINVAL, 0) ;
  g->queue.len -= g->esize ;
  return 1 ;
}
