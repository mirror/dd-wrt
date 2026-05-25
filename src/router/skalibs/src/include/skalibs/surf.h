/* ISC license. */

#ifndef SKALIBS_SURF_H
#define SKALIBS_SURF_H

#include <sys/types.h>
#include <stdint.h>

typedef struct SURFSchedule SURFSchedule, *SURFSchedule_ref ;
struct SURFSchedule
{
  uint32_t seed[32] ;
  uint32_t in[12] ;
  uint32_t pos ;
  char out[32] ;
} ;

#define SURFSCHEDULE_ZERO { .seed = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, .in = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, .pos = 32, .out = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" }

extern void surf_init (SURFSchedule *, char const *) ; /* 160 chars */
extern void surf (SURFSchedule *, char *, size_t) ;
extern void autosurf (char *, size_t) ;
extern void autosurf_name (char *, size_t) ;

#endif
