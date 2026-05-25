/* ISC license. */

#ifndef DJBTIME_INTERNAL_H
#define DJBTIME_INTERNAL_H

#include <skalibs/uint64.h>

extern unsigned int const leapsecs_table_len ;
extern uint64_t const *const leapsecs_table ;

extern void leapsecs_add (uint64_t *, int) ;
extern int leapsecs_sub (uint64_t *) ;
extern int skalibs_tzisright (void) ;

#endif
