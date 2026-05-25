/* ISC license. */

#include <skalibs/sysdeps.h>

#ifndef SKALIBS_HASSTRCASESTR

#include <string.h>

#include <skalibs/bytestr.h>

 /* XXX: copies strings on the stack, careful with long strings */

char *strcasestr (char const *haystack, char const *needle)
{
  size_t nlen = strlen(needle) ;
  size_t hlen = strlen(haystack) ;
  char *p ;
  char lneedle[nlen + 1] ;
  char lhaystack[hlen + 1] ;
  memcpy(lneedle, needle, nlen + 1) ;
  memcpy(lhaystack, haystack, hlen + 1) ;
  case_lowerb(lneedle, nlen) ;
  case_lowerb(lhaystack, hlen) ;
  p = strstr(lhaystack, lneedle) ;
  return p ? haystack + (p - lhaystack) : 0 ;
}

#endif
