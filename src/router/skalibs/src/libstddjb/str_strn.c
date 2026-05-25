/* ISC license. */

#include <string.h>
#include <skalibs/bytestr.h>

size_t str_strn (char const *haystack, size_t hlen, char const *needle, size_t nlen)
{
  char haystack2[hlen+1] ;
  char needle2[nlen+1] ;
  char *p ;
  memcpy(haystack2, haystack, hlen) ; haystack2[hlen] = 0 ;
  memcpy(needle2, needle, nlen) ; needle2[nlen] = 0 ;
  p = strstr(haystack2, needle2) ;
  return p ? p - haystack2 : hlen ;
}
