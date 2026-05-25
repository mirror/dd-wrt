/* ISC license. */

#include <ctype.h>

#include <skalibs/bytestr.h>

void case_lowers (char *s)
{
  while (*s)
  {
    int c = tolower(*s) ;
    *s++ = (unsigned char)c ;
  }
}
