/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/genset.h>

int genset_iter_withcancel (genset *g, iter_func_ref f, iter_func_ref cancelf, void *stuff)
{
  uint32_t n = genset_iter(g, f, stuff) ;
  if (n < g->max)
  {
    int e = errno ;
    genset_iter_nocancel(g, n, cancelf, stuff) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}
