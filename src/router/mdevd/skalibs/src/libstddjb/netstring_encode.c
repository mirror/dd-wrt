/* ISC license. */

#include <string.h>
#include <skalibs/uint64.h>
#include <skalibs/netstring.h>
#include <skalibs/stralloc.h>

int netstring_encode (stralloc *sa, char const *s, size_t len)
{
  char fmt[UINT64_FMT] ;
  size_t pos = uint64_fmt(fmt, len) ;
  if (!stralloc_readyplus(sa, pos + len + 2)) return 0 ;
  stralloc_catb(sa, fmt, pos) ;
  stralloc_catb(sa, ":", 1) ;
  stralloc_catb(sa, s, len) ;
  stralloc_catb(sa, ",", 1) ;
  return 1 ;
}
