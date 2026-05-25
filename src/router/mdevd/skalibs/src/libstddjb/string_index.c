/* ISC license. */

#include <skalibs/genalloc.h>
#include <skalibs/skamisc.h>

int string_index (char *s, size_t start, size_t len, char delim, genalloc *indices)
{
  size_t origlen = genalloc_len(size_t, indices) ;
  size_t pos = start ;
  int wasnull = !indices->s ;
  int inword = 0 ;

  len += start ;
  for (size_t i = start ; i < len ; i++)
  {
    if (s[i] == delim)
    {
      s[i] = 0 ;
      if (!genalloc_append(size_t, indices, &pos)) goto err ;
      inword = 0 ;
    }
    else if (!inword) { pos = i ; inword = 1 ; }
  }
  if (inword) goto err ;
  return 1 ;

 err:
  if (wasnull) genalloc_free(size_t, indices) ;
  else genalloc_setlen(size_t, indices, origlen) ;
  return 0 ;
}
