/* ISC license. */

#include <skalibs/fmtscan.h>

unsigned char fmtscan_num (unsigned char c, unsigned char n)
{
  return
    ((c < '0') || (n > 36)) ? n :
    (n <= 10) ? (c - '0' <= n) ? c - '0' : n :
    (c - '0' <= 9) ? c - '0' :
    (c < 'A') ? n :
    (c - 'A' < n - 10) ? c - 'A' + 10 :
    (c < 'a') ? n :
    (c - 'a' < n - 10) ? c - 'a' + 10 :
    n ;
}
