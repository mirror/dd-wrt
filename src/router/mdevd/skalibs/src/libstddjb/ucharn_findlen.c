/* ISC license. */

#include <skalibs/fmtscan.h>

size_t ucharn_findlen (char const *s)
{
  size_t i = 0 ;
  while (fmtscan_num(s[i], 16) <= 0xF) i++ ;
  return i ;
}
