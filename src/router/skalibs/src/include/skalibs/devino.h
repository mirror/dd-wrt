/* ISC license. */

#ifndef SKALIBS_DEVINO_H
#define SKALIBS_DEVINO_H

#include <sys/types.h>

typedef struct devino_s devino, *devino_ref ;
struct devino_s
{
  dev_t dev ;
  ino_t ino ;
} ;
#define DEVINO_ZERO { .dev = 0, .ino = 0 }

extern int devino_cmp (void const *, void const *) ;

#endif
