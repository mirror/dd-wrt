/* ISC license. */

#include <string.h>
#include <skalibs/uint64.h>
#include <skalibs/stralloc.h>
#include <skalibs/netstring.h>

int netstring_appendb (stralloc *sa, char const *s, size_t len)
{
  char fmt[UINT64_FMT] ;
  size_t n = uint64_fmt(fmt, len) ;
  if (!stralloc_readyplus(sa, len + n + 2)) return 0 ;
  fmt[n] = ':' ;
  memcpy(sa->s + sa->len, fmt, n+1) ;
  memcpy(sa->s + sa->len + n+1, s, len) ;
  sa->s[sa->len + n+1 + len] = ',' ;
  sa->len += n + 2 + len ;
  return 1 ;
}
