/* ISC license. */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/random.h>
#include <skalibs/unix-transactional.h>

int atomic_rm_rf_tmp (char const *filename, stralloc *tmp)
{
  size_t tmpbase = tmp->len ;
  size_t start ;
  if (!stralloc_cats(tmp, ".skalibs-rmrf-")
   || !stralloc_cats(tmp, filename)) return -1 ;
  start = tmp->len ;
  for (;;)
  {
    if (!random_sauniquename(tmp, 64)) goto err ;
    if (!stralloc_0(tmp)) goto err ;
    if (!rename(filename, tmp->s + tmpbase)) break ;
    if (errno != EEXIST && errno != ENOTEMPTY) goto err ;
    tmp->len = start ;
  }
  if (rm_rf_in_tmp(tmp, tmpbase) < 0) goto err ;
  tmp->len = tmpbase ;
  return 0 ;

err:
  tmp->len = tmpbase ;
  return -1 ;
}
