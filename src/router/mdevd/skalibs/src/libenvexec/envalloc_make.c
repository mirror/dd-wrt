/* ISC license. */

#include <skalibs/env.h>
#include <skalibs/genalloc.h>
#include <skalibs/envalloc.h>

int envalloc_make (genalloc *v, size_t argc, char const *s, size_t len)
{
  int wasnull = !v->s ;
  if (!genalloc_readyplus(char const *, v, argc+1)) return 0 ;
  if (!env_make(genalloc_s(char const *, v) + genalloc_len(char const *, v), argc, s, len))
  {
    if (wasnull) genalloc_free(char const *, v) ;
    return 0 ;
  }
  genalloc_setlen(char const *, v, genalloc_len(char const *, v) + argc) ;
  return 1 ;
}
