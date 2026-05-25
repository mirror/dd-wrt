/* ISC license. */

#include <string.h>
#include <skalibs/stralloc.h>

void stralloc_reverse_blocks (stralloc *sa, size_t size)
{
  size_t n = sa->len / (size << 1) ;
  size_t i = 0 ;
  char tmp[size] ;
  for (; i < n ; i++)
  {
    size_t k = sa->len - (i + 1) * size ;
    memcpy(tmp, sa->s + i * size, size) ;
    memcpy(sa->s + i * size, sa->s + k, size) ;
    memcpy(sa->s + k, tmp, size) ;
  }
}
