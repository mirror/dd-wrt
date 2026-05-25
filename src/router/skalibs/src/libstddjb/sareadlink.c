/* ISC license. */

#include <unistd.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sareadlink (stralloc *sa, char const *path)
{
  size_t n = 128 ;
  int wasnull = !sa->s ;
  ssize_t r ;

  for (;;)
  {
    if (!stralloc_readyplus(sa, n)) goto err ;
    r = readlink(path, sa->s + sa->len, n) ;
    if (r < 0) goto err ;
    if ((size_t)r < n) break ;
    n += 128 ;
  }
  sa->len += r ;
  return 0 ;

err:
  if (wasnull) stralloc_free(sa) ;
  return -1 ;
}
