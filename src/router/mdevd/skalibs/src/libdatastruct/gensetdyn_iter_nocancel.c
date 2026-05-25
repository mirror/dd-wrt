/* ISC license. */

#include <stddef.h>
#include <stdint.h>

#include <skalibs/bitarray.h>
#include <skalibs/gensetdyn.h>

uint32_t gensetdyn_iter_nocancel (gensetdyn *g, uint32_t n, iter_func_ref f, void *stuff)
{
 /*
    XXX: we may be called by a freeing function, so we cannot alloc -
    XXX: so pray that the bitarray fits in the stack.
 */
  unsigned char bits[bitarray_div8(g->storage.len) ? bitarray_div8(g->storage.len) : 1] ;
  size_t i = 0 ;
  uint32_t j = 0 ;
  uint32_t *fl = genalloc_s(uint32_t, &g->freelist) ;
  size_t sp = genalloc_len(uint32_t, &g->freelist) ;
  bitarray_setn(bits, 0, g->storage.len) ;
  
  for (; i < sp ; i++) if (fl[i] < g->storage.len) bitarray_clear(bits, fl[i]) ;
  for (i = 0 ; (i < g->storage.len) && (j < n) ; i++) if (bitarray_peek(bits, i))
  {
    j++ ;
    if (!(*f)(gensetdyn_p(g, i), stuff)) break ;
  }
  return j ;
}
