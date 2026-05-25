/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/direntry.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int sals (char const *fn, stralloc *sa, size_t *x)
{
  int n = 0 ;
  size_t sabase = sa->len ;
  size_t maxlen = 0 ;
  int wasnull = !sa->s ;
  DIR *dir = opendir(fn) ;
  if (!dir) return -1 ;
  for (;;)
  {
    direntry *d ;
    size_t len ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.')
      if (((d->d_name[1] == '.') && !d->d_name[2]) || !d->d_name[1])
        continue ;
    len = strlen(d->d_name) ;
    if (len > maxlen) maxlen = len ;
    if (!stralloc_catb(sa, d->d_name, len+1)) goto err ;
    n++ ;
  }
  if (errno) goto err ;
  dir_close(dir) ;
  if (x) *x = maxlen ;
  return n ;

 err:
  dir_close(dir) ;
  if (wasnull) stralloc_free(sa) ;
  else sa->len = sabase ;
  return -1 ;
}
