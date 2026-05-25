/* ISC license. */

#include <stdint.h>
#include <skalibs/avlnode.h>

struct avlnode_iter_s
{
  avlnode const *s ;
  uint32_t max ;
  uint32_t cut ;
  avliter_func_ref f ;
  void *p ;
} ;

static uint32_t avlnode_iter_rec (struct avlnode_iter_s const *blah, uint32_t r, unsigned int h)
{
  if (r >= blah->max) return blah->max ;
  {
    uint32_t res = avlnode_iter_rec(blah, blah->s[r].child[0], h+1) ;
    if (res != blah->max) return res ;
  }
  if (r == blah->cut) return blah->max ;
  if (!(*blah->f)(blah->s[r].data, h, blah->p)) return r ;
  return avlnode_iter_rec(blah, blah->s[r].child[1], h+1) ;
}

uint32_t avlnode_iter_nocancel (avlnode *s, uint32_t max, uint32_t cut, uint32_t r, avliter_func_ref f, void *p)
{
  struct avlnode_iter_s blah = { .s = s, .max = max, .cut = cut, .f = f, .p = p } ;
  return avlnode_iter_rec(&blah, r, 0) ;
}
