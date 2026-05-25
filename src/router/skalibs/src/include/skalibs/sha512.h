/* ISC license. */

#ifndef SKALIBS_SHA512_H
#define SKALIBS_SHA512_H

#include <sys/types.h>

#include <skalibs/uint64.h>

typedef struct SHA512Schedule_s SHA512Schedule, *SHA512Schedule_ref ;
struct SHA512Schedule_s
{
  uint64_t len ;
  uint64_t h[8] ;
  unsigned char buf[128] ;
} ;

#define SHA512_INIT() { .len = 0, .h = { 0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL } }
extern void sha512_init (SHA512Schedule *) ;
extern void sha512_update (SHA512Schedule *, char const *, size_t) ;
extern void sha512_final (SHA512Schedule *, char *digest) ;

#endif
