/* ISC license. */

#ifndef SKALIBS_ENVALLOC_H
#define SKALIBS_ENVALLOC_H

#include <sys/types.h>

#include <skalibs/genalloc.h>

#define ENVALLOC_ZERO GENALLOC_ZERO

extern int envalloc_make (genalloc *, size_t, char const *, size_t) ;
extern int envalloc_uniq (genalloc *, char) ;
extern int envalloc_merge (genalloc *, char const *const *, size_t, char const *, size_t) ;
extern int envalloc_0 (genalloc *) ;

#endif
