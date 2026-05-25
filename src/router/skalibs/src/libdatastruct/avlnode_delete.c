/* ISC license. */

#include <stdint.h>

#include <skalibs/avlnode.h>
#include "avlnode-internal.h"

uint32_t avlnode_delete (avlnode *s, uint32_t max, uint32_t *root, void const *k, dtok_func_ref dtok, cmp_func_ref f, void *p)
{
  uint32_t r = *root ;
  uint32_t itodel ;
  uint32_t stack[AVLNODE_MAXDEPTH] ;
  uint8_t spin[AVLNODE_MAXDEPTH] ;
  uint8_t sp = 0 ;

  for (; r < max ; sp++)
  {
    int c = (*f)(k, (*dtok)(s[r].data, p), p) ;
    if (!c) break ;
    spin[sp] = avlnode_ufroms(c) ;
    stack[sp] = r ;
    r = s[r].child[spin[sp]] ;
  }
  if (r >= max) return max ;
  itodel = r ;

  if ((s[r].child[0] < max) || (s[r].child[1] < max))
  {
    uint8_t subspin = s[r].child[1] < max ;
    stack[sp] = r ;
    spin[sp++] = subspin ;
    r = s[r].child[subspin] ;
    for (; r < max ; sp++)
    {
      stack[sp] = r ;
      spin[sp] = !subspin ;
      r = s[r].child[!subspin] ;
    }
    r = stack[--sp] ;
    s[itodel].data = s[r].data ;
    itodel = s[r].child[subspin] ;
    if (itodel < max)
    {
      s[r].data = s[itodel].data ;
      stack[sp] = r ;
      spin[sp++] = subspin ;
    }
    else itodel = r ;
  }

  r = max ; 
  while (sp--)
  {
    s[stack[sp]].child[spin[sp]] = r ;
    r = stack[sp] ;
    if (!s[r].balance) goto easyfix ;
    else if (spin[sp] == avlnode_ufroms(s[r].balance)) s[r].balance = 0 ;
    else if (!s[s[r].child[!spin[sp]]].balance) goto hardfix ;
    else r = avlnode_rotate_maydouble(s, max, r, spin[sp], spin[sp] == avlnode_ufroms(s[s[r].child[!spin[sp]]].balance)) ;
  }
  *root = r ;
  return itodel ;

 easyfix:
  s[r].balance = -avlnode_sfromu(spin[sp]) ;
  return itodel ;

 hardfix:
  r = avlnode_rotate(s, max, r, spin[sp]) ;
  if (!sp--) *root = r ;
  else s[stack[sp]].child[spin[sp]] = r ;
  return itodel ;
}
