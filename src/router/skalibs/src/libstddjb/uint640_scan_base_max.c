/* ISC license. */

#include <errno.h>

#include <skalibs/uint64.h>
#include <skalibs/fmtscan.h>

size_t uint640_scan_base_max (char const *s, uint64_t *u, uint8_t base, uint64_t max)
{
  size_t pos = uint64_scan_base_max(s, u, base, max) ;
  if (!pos) return (errno = EINVAL, 0) ;
  if (!s[pos]) return pos ;
  errno = fmtscan_num(s[pos], base) < base ? ERANGE : EINVAL ;
  return 0 ;
}
