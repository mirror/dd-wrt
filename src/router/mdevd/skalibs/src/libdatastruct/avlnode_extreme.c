/* ISC license. */

#include <errno.h>
#include <skalibs/avlnode.h>

int avlnode_extreme (avlnode const *s, uint32_t max, uint32_t r, int h, uint32_t *k)
{
  uint32_t i = avlnode_extremenode(s, max, r, h) ;
  if (i >= max) return (errno = ESRCH, 0) ;
  *k = s[i].data ;
  return 1 ;
}
