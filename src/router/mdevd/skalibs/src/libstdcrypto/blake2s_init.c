 /* ISC license. */

#include <skalibs/blake2s.h>

void blake2s_init (blake2s_ctx *ctx, size_t outlen)
{
  blake2s_ctx c = BLAKE2S_INIT(outlen) ;
  *ctx = c ;
}
