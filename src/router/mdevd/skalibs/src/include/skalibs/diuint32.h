/* ISC license. */

#ifndef SKALIBS_DIUINT32_H
#define SKALIBS_DIUINT32_H

#include <stdint.h>

typedef struct diuint32_s diuint32, *diuint32_ref ;
struct diuint32_s
{
  uint32_t left ;
  uint32_t right ;
} ;

#define DIUINT32_ZERO { .left = 0, .right = 0 }

#endif
