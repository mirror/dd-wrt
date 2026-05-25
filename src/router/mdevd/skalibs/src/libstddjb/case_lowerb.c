/* ISC license. */

#include <ctype.h>

#include <skalibs/bytestr.h>

void case_lowerb (char *s, size_t len)
{
  while (len--)
  {
    int c = tolower(*s) ;
    *s++ = (unsigned char)c ;
  }
}
