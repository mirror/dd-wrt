/* ISC license. */

#ifndef SHA1_INTERNAL_H
#define SHA1_INTERNAL_H

#include <stdint.h>
#include <skalibs/sha1.h>

extern void sha1_feed (SHA1Schedule *, unsigned char) ;
extern void sha1_transform (uint32_t * /* 5 uint32s */, uint32_t const * /* 16 uint32s */) ;

#endif
