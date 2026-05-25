/* ISC license. */

#ifndef SKALIBS_CDBMAKE_INTERNAL_H
#define SKALIBS_CDBMAKE_INTERNAL_H

#include <stdint.h>

#include <skalibs/cdbmake.h>

extern int cdbmake_posplus (cdbmaker *, uint32_t) ;
extern int cdbmake_addend (cdbmaker *, uint32_t, uint32_t, uint32_t) ;
extern int cdbmake_addbegin (cdbmaker *, uint32_t, uint32_t) ;

#endif
