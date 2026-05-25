/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genwrite.h>

ssize_t genwrite_put_stralloc (void *target, char const *s, size_t len)
{
  return stralloc_catb((stralloc *)target, s, len) ? len : -1 ;
}
