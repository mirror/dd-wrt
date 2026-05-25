/* ISC license. */

#ifndef SKALIBS_DISIZE_H
#define SKALIBS_DISIZE_H

#include <stddef.h>

typedef struct disize_s disize, *disize_ref ;
struct disize_s
{
  size_t left ;
  size_t right ;
} ;

#define DISIZE_ZERO { .left = 0, .right = 0 }

#endif
