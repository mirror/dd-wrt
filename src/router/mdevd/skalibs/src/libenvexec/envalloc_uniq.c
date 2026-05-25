/* ISC license. */

#include <string.h>
#include <errno.h>
#include <skalibs/bytestr.h>
#include <skalibs/genalloc.h>
#include <skalibs/envalloc.h>

int envalloc_uniq (genalloc *v, char delim)
{
  unsigned int m = 0 ;
  size_t i = 0 ;
  for (; i < genalloc_len(char const *, v) ; i++)
  {
    size_t j = i+1 ;
    char const *s = genalloc_s(char const *, v)[i] ;
    size_t n = str_chr(s, delim) ;
    if (delim && !s[n]) return (errno = EINVAL, -1) ;
    for (; j < genalloc_len(char const *, v) ; j++)
    {
      char const **p = genalloc_s(char const *, v) ;
      if (!strncmp(s, p[j], n))
      {
        size_t len = genalloc_len(char const *, v) - 1 ;
        genalloc_setlen(char const *, v, len) ;
        p[j] = p[len] ;
        m++ ;
      }
    }
  }
  return m ;
}
