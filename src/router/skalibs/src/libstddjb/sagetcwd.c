/* ISC license. */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagetcwd (stralloc *sa)
{
  size_t n = 128 ;
  int wasnull = !sa->s ;

  for (;;)
  {
    if (!stralloc_readyplus(sa, n)) goto err ;
    if (getcwd(sa->s + sa->len, n)) break ;
    if (errno != ERANGE) goto err ;
    n += 128 ;
  }
  sa->len += strlen(sa->s + sa->len) ;
  return 0 ;

err:
  if (wasnull) stralloc_free(sa) ;
  return -1 ;
}
