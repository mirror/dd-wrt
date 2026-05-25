/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/posixplz.h>

void *bsearchr (void const *key, void const *base, size_t n, size_t width, cmp_func_ref cmp, void *aux)
{
  while (n)
  {
    void *cur = (char *)base + width * (n >> 1) ;
    int h = (*cmp)(key, cur, aux) ;
    if (h < 0) n >>= 1 ;
    else if (h > 0) { base = (char *)cur + width ; n = (n-1) >> 1 ; }
    else return cur ;
  }
  return 0 ;
}
