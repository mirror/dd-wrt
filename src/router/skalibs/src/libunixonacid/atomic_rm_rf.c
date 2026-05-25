/* ISC license. */

#include <skalibs/skamisc.h>
#include <skalibs/unix-transactional.h>

int atomic_rm_rf (char const *filename)
{
  return atomic_rm_rf_tmp(filename, &satmp) ;
}
