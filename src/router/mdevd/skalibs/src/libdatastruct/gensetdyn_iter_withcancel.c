/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/gensetdyn.h>

int gensetdyn_iter_withcancel (gensetdyn *g, iter_func_ref f, iter_func_ref cancelf, void *stuff)
{
  uint32_t n = gensetdyn_iter_nocancel(g, gensetdyn_n(g), f, stuff) ;
  if (n < gensetdyn_n(g))
  {
    int e = errno ;
    gensetdyn_iter_nocancel(g, n, cancelf, stuff) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}
