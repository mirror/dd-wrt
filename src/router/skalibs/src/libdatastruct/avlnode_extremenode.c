/* ISC license. */

#include <skalibs/avlnode.h>

uint32_t avlnode_extremenode (avlnode const *s, uint32_t max, uint32_t r, int h)
{
  uint32_t oldr = r ;
  for (; r < max ; oldr = r, r = s[r].child[h]) ;
  return oldr ;
}
