/* ISC license. */

#include <string.h>
#include <strings.h>

#include <skalibs/types.h>
#include <skalibs/sig.h>

static size_t sig0_scan_norec (char const *s, int *sig)
{
  int r = sig_number(s) ;
  if (r)
  {
    *sig = r ;
    return strlen(s) ;
  }
  {
    unsigned int u ;
    size_t len = uint0_scan(s, &u) ;
    if (len) *sig = u ;
    return len ;
  }
}

size_t sig0_scan (char const *s, int *sig)
{
  size_t len = sig0_scan_norec(s, sig) ;
  if (len) return len ;
  if (!strncasecmp(s, "SIG", 3))
  {
    len = sig0_scan_norec(s+3, sig) ;
    if (len) return 3+len ;
  }
  return 0 ;
}
