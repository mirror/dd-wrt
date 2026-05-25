/* ISC license. */

#include <errno.h>

#include <skalibs/env.h>
#include "envdir-internal.h"

int envdir_internal (char const *path, stralloc *modifs, unsigned int options, char nullis)
{
  return nullis ? options & SKALIBS_ENVDIR_NOCLAMP ?
    envdir_internal_noclamp(path, modifs, options & ~SKALIBS_ENVDIR_NOCLAMP, nullis) :
    envdir_internal_clamp(path, modifs, options, nullis) : (errno = EINVAL, -1) ;
}
