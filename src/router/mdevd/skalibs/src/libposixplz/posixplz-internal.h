/* ISC license. */

#ifndef SKALIBS_POSIXPLZ_INTERNAL_H
#define SKALIBS_POSIXPLZ_INTERNAL_H

typedef struct linkarg_s linkarg, *linkarg_ref ;
struct linkarg_s
{
  link_func_ref lf ;
  char const *src ;
} ;

extern void execvep_internal (char const *, char const *const *, char const *const *, char const *) ;

#endif
