/* ISC license. */

/* MT-unsafe */

#include <skalibs/skamisc.h>
#include <skalibs/djbunix.h>

int rm_rf (char const *filename)
{
  return rm_rf_tmp(filename, &satmp) ;
}
