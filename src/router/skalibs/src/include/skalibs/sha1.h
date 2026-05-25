/* ISC license. */

#ifndef SKALIBS_SHA1_H
#define SKALIBS_SHA1_H

#include <sys/types.h>
#include <stdint.h>

typedef struct SHA1Schedule SHA1Schedule, *SHA1Schedule_ref ;
struct SHA1Schedule
{
  uint32_t buf[5] ;
  uint32_t bits[2] ;
  uint32_t in[16] ;
  unsigned int b ;
} ;

#define SHA1_INIT() { .buf = { 0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U }, .bits = { 0, 0 }, .in = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, .b = 0 }
extern void sha1_init (SHA1Schedule *) ;
extern void sha1_update (SHA1Schedule *, char const *, size_t) ;
extern void sha1_final (SHA1Schedule *, char * /* 20 chars */) ;

#endif
