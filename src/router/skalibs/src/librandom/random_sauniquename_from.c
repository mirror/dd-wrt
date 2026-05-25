/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>
#include <skalibs/random.h>

int random_sauniquename_from (stralloc *sa, size_t n, randomgen_func_ref f)
{
  size_t base = sa->len ;
  int wasnull = !sa->s ;
  if (!sauniquename(sa)) return 0 ;
  if (!stralloc_readyplus(sa, n+1)) goto err ;
  stralloc_catb(sa, ":", 1) ;
  random_name_from(sa->s + sa->len, n, f) ;
  sa->len += n ;
  return 1 ;

err:
  if (wasnull) stralloc_free(sa) ; else sa->len = base ;
  return 0 ;
}
