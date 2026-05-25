/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int string_quote_options (stralloc *sa, char const *s, size_t len, uint32_t options)
{
  size_t base = sa->len ;
  int wasnull = !sa->s ;
  if (!stralloc_catb(sa, "\"", 1)) return 0 ;
  if (!string_quote_nodelim_mustquote_options(sa, s, len, "\"", 1, options) || !stralloc_catb(sa, "\"", 1))
  {
    if (wasnull) stralloc_free(sa) ; else sa->len = base ;
    return 0 ;
  }
  return 1 ;
}
