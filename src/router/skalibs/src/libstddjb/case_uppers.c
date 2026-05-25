/* ISC license. */

#include <ctype.h>

#include <skalibs/bytestr.h>

void case_uppers (char *s)
{
  while (*s)
  {
    int c = toupper(*s) ;
    *s++ = (unsigned char)c ;
  }
}
