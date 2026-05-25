/* ISC license. */

#include <errno.h>
#include <skalibs/genalloc.h>
#include <skalibs/gensetdyn.h>

int gensetdyn_delete (gensetdyn *g, uint32_t i)
{
  return (i >= g->storage.len) ? (errno = EINVAL, 0) :
    genalloc_catb(uint32_t, &g->freelist, &i, 1) ;
}
