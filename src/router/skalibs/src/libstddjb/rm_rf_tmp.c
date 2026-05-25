/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

int rm_rf_tmp (char const *filename, stralloc *tmp)
{
  size_t tmpbase = tmp->len ;
  if (!stralloc_cats(tmp, filename)) return -1 ;
  if (!stralloc_0(tmp)) goto err ;
  if (rm_rf_in_tmp(tmp, tmpbase) == -1) goto err ;
  tmp->len = tmpbase ;
  return 0 ;

err:
  tmp->len = tmpbase ;
  return -1 ;
}
