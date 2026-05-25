/* ISC license. */

#ifndef SKALIBS_DIUINT_H
#define SKALIBS_DIUINT_H

typedef struct diuint diuint, *diuint_ref ;
struct diuint
{
  unsigned int left ;
  unsigned int right ;
} ;

#define DIUINT_ZERO { .left = 0, .right = 0 }

#endif
