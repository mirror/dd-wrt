/* ISC license. */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

static int rmstar_in_tmp (stralloc *tmp, size_t ipos)
{
  size_t tmpbase = tmp->len, tmpstop ;
  size_t fnbase = strlen(tmp->s + ipos) ;
  size_t i ;
  if (sals(tmp->s + ipos, tmp, &i) == -1) return -1 ;
  tmpstop = tmp->len ;
  if (!stralloc_readyplus(tmp, fnbase + 2 + i)) goto err ;
  stralloc_catb(tmp, tmp->s + ipos, fnbase) ;
  stralloc_catb(tmp, "/", 1) ;
  fnbase = tmp->len ;
  for (i = tmpbase ; i < tmpstop ; i += tmp->len - fnbase)
  {
    size_t n = strlen(tmp->s + i) ;
    tmp->len = fnbase ;
     stralloc_catb(tmp, tmp->s + i, n+1) ;
    if (rm_rf_in_tmp(tmp, tmpstop) == -1) goto err ;
  }
  tmp->len = tmpbase ;
  return 0 ;

err:
  tmp->len = tmpbase ;
  return -1 ;
}

int rm_rf_in_tmp (stralloc *tmp, size_t ipos)
{
  int isadir ;
  if (unlink(tmp->s + ipos) == 0) return 0 ;
  if (errno == ENOENT) return 0 ;
  if ((errno != EISDIR) && (errno != EPERM)) return -1 ;
  isadir = errno == EPERM ;
  if (rmstar_in_tmp(tmp, ipos) == -1)
  {
    if (isadir && errno == ENOTDIR) errno = EPERM ;
    return -1 ;
  }
  return rmdir(tmp->s + ipos) ;
}

int rmstar_tmp (char const *dirname, stralloc *tmp)
{
  size_t tmpbase = tmp->len ;
  if (!stralloc_cats(tmp, dirname)) return -1 ;
  if (!stralloc_0(tmp)) goto err ;
  if (rmstar_in_tmp(tmp, tmpbase) == -1) goto err ;
  tmp->len = tmpbase ;
  return 0 ;

err:
  tmp->len = tmpbase ;
  return -1 ;
}
