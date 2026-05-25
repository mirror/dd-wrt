/* ISC license. */

#include <string.h>
#include <stdint.h>

#include <skalibs/uint32.h>
#include <skalibs/surf.h>

#define ROTATE(x, b) (((x) << (b)) | ((x) >> (32 - (b))))
#define MUSH(i, b) x = t[i] += (((x ^ ctx->seed[i]) + sum) ^ ROTATE(x, b))

static void surfit (SURFSchedule *ctx)
{
  uint32_t t[12] ;
  uint32_t z[8] ;
  uint32_t x ;
  uint32_t sum = 0 ;
  uint32_t i = 0, loop = 0 ; ;

  if (!++ctx->in[0] && !++ctx->in[1] && !++ctx->in[2]) ++ctx->in[3] ;
  for (; i < 12 ; i++) t[i] = ctx->in[i] ^ ctx->seed[12+i] ;
  for (i = 0 ; i < 8 ; i++) z[i] = ctx->seed[24+i] ;
  x = t[11] ;
  for (; loop < 2 ; loop++)
  {
    for (i = 0 ; i < 16 ; i++)
    {
      sum += 0x9e3779b9 ;
      MUSH(0, 5) ; MUSH(1, 7) ; MUSH(2, 9) ;  MUSH(3, 13) ;
      MUSH(4, 5) ; MUSH(5, 7) ; MUSH(6, 9) ;  MUSH(7, 13) ;
      MUSH(8, 5) ; MUSH(9, 7) ; MUSH(10, 9) ; MUSH(11, 13) ;
    }
    for (i = 0 ; i < 8 ; i++) z[i] ^= t[i+4] ;
  }
  for (i = 0 ; i < 8 ; i++) uint32_pack(ctx->out + (i<<2), z[i]) ;
}

void surf (SURFSchedule *ctx, char *s, size_t n)
{
  {
    size_t i = 32 - ctx->pos ;
    if (n < i) i = n ;
    memcpy(s, ctx->out + ctx->pos, i) ;
    s += i ; n -= i ; ctx->pos += i ;
  }
  while (n > 32)
  {
    surfit(ctx) ;
    memcpy(s, ctx->out, 32) ;
    s += 32 ; n -= 32 ;
  }
  if (!n) return ;
  surfit(ctx) ;
  memcpy(s, ctx->out, n) ;
  ctx->pos = n ;
}
