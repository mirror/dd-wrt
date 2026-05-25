 /* ISC license. */

#ifndef SKALIBS_BLAKE2S_INTERNAL_H
#define SKALIBS_BLAKE2S_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/blake2s.h>

extern void blake2s_transform (blake2s_ctx *, char const *, size_t, uint32_t) ;

#endif
