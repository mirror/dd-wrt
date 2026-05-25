/* ISC license. */

#ifndef SHA256_INTERNAL_H
#define SHA256_INTERNAL_H

#include <stdint.h>
#include <skalibs/sha256.h>

extern void sha256_feed (SHA256Schedule *, unsigned char) ;
extern void sha256_transform (uint32_t *, uint32_t const *) ;

#endif
