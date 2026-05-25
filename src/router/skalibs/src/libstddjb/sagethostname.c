/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sagethostname (stralloc *sa)
{
  size_t n = 32 ;
  int e = errno ;
  int wasnull = !sa->s ;

  for (;;)
  {
    if (!stralloc_readyplus(sa, n)) goto err ;
    sa->s[sa->len + n - 2] = 0 ;
    errno = 0 ;
    if (gethostname(sa->s + sa->len, n) < 0)
    {
      if (errno != ENAMETOOLONG) goto err ;
    }
    else if (!sa->s[sa->len + n - 2]) break ;
    n += 32 ;
  }
  sa->len += strlen(sa->s + sa->len) ;
  errno = e ;
  return 0 ;

err:
  if (wasnull) stralloc_free(sa) ;
  return -1 ;
}
