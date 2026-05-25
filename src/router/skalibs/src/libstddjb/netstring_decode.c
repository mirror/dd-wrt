/* ISC license. */

#include <errno.h>
#include <skalibs/uint64.h>
#include <skalibs/stralloc.h>
#include <skalibs/netstring.h>

ssize_t netstring_decode (stralloc *sa, char const *s, size_t len)
{
  uint64_t nlen ;
  size_t pos ;
  if (!len) return 0 ;
  pos = uint64_scan(s, &nlen) ;
  if (pos >= len) return (errno = EINVAL, -1) ;
  if (s[pos] != ':') return (errno = EINVAL, -1) ;
  s += pos+1 ; len -= pos+1 ;
  if (len <= nlen) return (errno = EINVAL, -1) ;
  if (s[nlen] != ',') return (errno = EINVAL, -1) ;
  if (!stralloc_catb(sa, s, nlen)) return -1 ;
  return pos + nlen + 2 ;
}
