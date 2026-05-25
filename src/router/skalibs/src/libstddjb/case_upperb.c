/* ISC license. */

#include <ctype.h>

#include <skalibs/bytestr.h>

void case_upperb (char *s, size_t len)
{
  while (len--)
  {
    int c = toupper(*s) ;
    *s++ = (unsigned char)c ;
  }
}
