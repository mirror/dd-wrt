/* ISC license. */

/* MT-unsafe */

#include <skalibs/skamisc.h>
#include <skalibs/djbunix.h>

int rmstar (char const *dirname)
{
  return rmstar_tmp(dirname, &satmp) ;
}
