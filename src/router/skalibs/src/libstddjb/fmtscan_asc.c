/* ISC license. */

#include <skalibs/fmtscan.h>

unsigned char fmtscan_asc (unsigned char c)
{
  static char const *tab = "0123456789abcdefghijklmnopqrstuvwxyz" ;
  return (c >= 36) ? 0 : tab[c] ;
}
