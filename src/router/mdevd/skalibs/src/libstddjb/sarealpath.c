/* ISC license. */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sarealpath (stralloc *sa, char const *path)
{
  if (sa->s)
  {
#ifdef PATH_MAX
    if (!stralloc_readyplus(sa, PATH_MAX)) return -1 ;
    if (!realpath(path, sa->s + sa->len)) return -1 ;
    sa->len += strlen(sa->s + sa->len) ;
#else
    char *p = realpath(path, 0) ;
    if (!p) return -1 ;
    if (!stralloc_cats(sa, p) || !stralloc_0(sa))
    {
      free(p) ;
      return -1 ;
    }
    free(p) ;
#endif
  }
  else
  {
    char *p = realpath(path, 0) ;
    if (!p) return -1 ;
    sa->s = p ; /* XXX: incompatible with alloc() interposition */
    sa->len = strlen(p) ;
    sa->a = sa->len + 1 ;
  }
  return 0 ;
}
