/* ISC license. */

#ifndef SKALIBS_ALLOC_H
#define SKALIBS_ALLOC_H

#include <stdlib.h>

#include <skalibs/gccattributes.h>

extern void *alloc (size_t) ;
#define alloc_free(p) free(p)

#define alloc_re(p, old, new) alloc_realloc(p, new)
extern int alloc_realloc (char **, size_t) ;

#endif
