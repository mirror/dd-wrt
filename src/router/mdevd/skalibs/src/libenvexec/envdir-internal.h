/* ISC license. */

#ifndef ENVDIR_INTERNAL_H
#define ENVDIR_INTERNAL_H

#include <skalibs/stralloc.h>

extern int envdir_internal_clamp (char const *, stralloc *, unsigned int, char) ;
extern int envdir_internal_noclamp (char const *, stralloc *, unsigned int, char) ;

#endif
