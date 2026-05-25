/* ISC license. */

#include <skalibs/avlnode.h>
#include "avlnode-internal.h"

uint32_t avlnode_rotate (avlnode *s, uint32_t max, uint32_t i, int h)
{
  uint32_t j = s[i].child[!h] ;
  s[i].child[!h] = s[j].child[h] ;
  s[j].child[h] = i ;
  if (s[j].balance * avlnode_sfromu(h) < 0) s[i].balance = s[j].balance = 0 ;
  else s[j].balance = avlnode_sfromu(h) ;
  (void)max ;
  return j ;
}
