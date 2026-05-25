/* ISC license. */

#include <string.h>
#include <skalibs/uint64.h>
#include <skalibs/bytestr.h>
#include <skalibs/siovec.h>
#include <skalibs/stralloc.h>
#include <skalibs/netstring.h>

int netstring_appendv (stralloc *sa, struct iovec const *v, unsigned int n)
{
  char fmt[UINT64_FMT] ;
  size_t len = 0, pos ;
  unsigned int i = 0 ;
  for (; i < n ; i++) len += v[i].iov_len ;
  pos = uint64_fmt(fmt, len) ;
  if (!stralloc_readyplus(sa, len + pos + 2)) return 0 ;
  fmt[pos] = ':' ;
  memcpy(sa->s + sa->len, fmt, pos+1) ;
  sa->len += pos+1 ;
  for (i = 0 ; i < n ; i++)
  {
    memcpy(sa->s + sa->len, v[i].iov_base, v[i].iov_len) ;
    sa->len += v[i].iov_len ;
  }
  sa->s[sa->len++] = ',' ;
  return 1 ;
}
