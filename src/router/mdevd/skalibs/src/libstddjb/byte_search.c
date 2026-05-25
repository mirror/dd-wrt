/* ISC license. */

#include <string.h>
#include <skalibs/posixplz.h>
#include <skalibs/bytestr.h>

size_t byte_search (char const *haystack, size_t hlen, char const *needle, size_t nlen)
{
  char *p = memmem(haystack, hlen, needle, nlen) ;
  return p ? p - haystack : hlen + 1 - nlen ;
}
