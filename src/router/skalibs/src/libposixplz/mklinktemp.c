/* ISC license. */

#include <unistd.h>

#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

#include "posixplz-internal.h"

static int f (char const *dst, mode_t mode, void *data)
{
  linkarg *la = data ;
  (void)mode ;
  return (*la->lf)(la->src, dst) ;
}

int mklinktemp (char const *src, char *dst, link_func_ref lf)
{
  linkarg la = { .lf = lf, .src = src } ;
  return mkfiletemp(dst, &f, 0600, &la) ;
}
