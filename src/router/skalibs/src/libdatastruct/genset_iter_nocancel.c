/* ISC license. */

#include <stdint.h>

#include <skalibs/bitarray.h>
#include <skalibs/genset.h>

uint32_t genset_iter_nocancel (genset *g, uint32_t n, iter_func_ref f, void *stuff)
{
  unsigned char bits[bitarray_div8(n)] ;
  uint32_t i = 0, j = 0, m = genset_n(g) ;
  bitarray_setn(bits, 0, n) ;
  for (; i < g->sp ; i++) if (g->freelist[i] < n) bitarray_clear(bits, g->freelist[i]) ;
  for (i = 0 ; (i < n) && (j < m) ; i++) if (bitarray_peek(bits, i))
  {
    j++ ;
    if (!(*f)(g->storage + i * g->esize, stuff)) break ;
  }
  return i ;
}
