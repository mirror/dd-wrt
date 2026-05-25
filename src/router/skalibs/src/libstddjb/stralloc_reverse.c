/* ISC license. */

#include <string.h>
#include <skalibs/stralloc.h>

void stralloc_reverse (stralloc *sa)
{
  size_t i = 0 ;
  for (; i < sa->len >> 1 ; i++)
  {
    char tmp = sa->s[i] ;
    sa->s[i] = sa->s[sa->len - 1 - i] ;
    sa->s[sa->len - 1 - i] = tmp ;
  }
}
