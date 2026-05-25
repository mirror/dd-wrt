 /* ISC license. */

#ifndef SKALIBS_BLAKE2S_H
#define SKALIBS_BLAKE2S_H

#include <stddef.h>
#include <stdint.h>

typedef struct blake2s_ctx_s blake2s_ctx, *blake2s_ctx_ref ;
struct blake2s_ctx_s
{
  size_t buflen ;
  size_t outlen ;
  uint32_t h[8] ;
  uint32_t t[2] ;
  uint32_t f[2] ;
  char buf[64] ;
} ;

#define BLAKE2S_INIT(len) { \
  .buflen = 0, \
  .outlen = len, \
  .h = { 0x6A09E667UL ^ (0x01010000 | len), 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL }, \
  .t = { 0, 0 }, \
  .f = { 0, 0 }, \
  .buf = { 0 } }

extern void blake2s_init (blake2s_ctx *, size_t) ;  /* outlen <= 32 */
extern void blake2s_update (blake2s_ctx *, char const *, size_t) ;
extern void blake2s_final (blake2s_ctx *, char *) ; /* outlen chars */

#endif
