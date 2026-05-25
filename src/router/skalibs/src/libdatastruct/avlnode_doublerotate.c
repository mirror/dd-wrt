/* ISC license. */

#include <skalibs/avlnode.h>
#include "avlnode-internal.h"

uint32_t avlnode_doublerotate (avlnode_ref s, uint32_t max, uint32_t i, int h)
{
  uint32_t j = s[i].child[!h] ;
  uint32_t k = s[j].child[h] ;
  s[i].child[!h] = s[k].child[h] ;
  s[j].child[h] = s[k].child[!h] ;
  s[k].child[!h] = j ;
  s[k].child[h] = i ;
  s[h ? i : j].balance = (s[k].balance < 0) ;
  s[h ? j : i].balance = -(s[k].balance > 0) ;
  s[k].balance = 0 ;
  (void)max ;
  return k ;
}
