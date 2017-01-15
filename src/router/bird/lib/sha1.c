/*
 *	BIRD Library -- SHA-1 Hash Function (FIPS 180-1, RFC 3174)
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Based on the code from libucw-6.4
 *	(c) 2008--2009 Martin Mares <mj@ucw.cz>
 *
 *	Based on the code from libgcrypt-1.2.3, which is
 *	(c) 1998, 2001, 2002, 2003 Free Software Foundation, Inc.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "lib/sha1.h"
#include "lib/unaligned.h"


void
sha1_init(struct hash_context *CTX)
{
  struct sha1_context *ctx = (void *) CTX;

  ctx->h0 = 0x67452301;
  ctx->h1 = 0xefcdab89;
  ctx->h2 = 0x98badcfe;
  ctx->h3 = 0x10325476;
  ctx->h4 = 0xc3d2e1f0;

  ctx->nblocks = 0;
  ctx->count = 0;
}

/*
 * Transform the message X which consists of 16 32-bit-words
 */
static void
sha1_transform(struct sha1_context *ctx, const byte *data)
{
  u32 a,b,c,d,e,tm;
  u32 x[16];

  /* Get values from the chaining vars. */
  a = ctx->h0;
  b = ctx->h1;
  c = ctx->h2;
  d = ctx->h3;
  e = ctx->h4;

#ifdef CPU_BIG_ENDIAN
  memcpy(x, data, 64);
#else
  int i;
  for (i = 0; i < 16; i++)
    x[i] = get_u32(data+4*i);
#endif

#define K1		0x5A827999L
#define K2		0x6ED9EBA1L
#define K3		0x8F1BBCDCL
#define K4  		0xCA62C1D6L
#define F1(x,y,z)	( z ^ ( x & ( y ^ z ) ) )
#define F2(x,y,z)	( x ^ y ^ z )
#define F3(x,y,z)	( ( x & y ) | ( z & ( x | y ) ) )
#define F4(x,y,z)	( x ^ y ^ z )

#define M(i) (tm = x[i&0x0f] ^ x[(i-14)&0x0f] ^ x[(i-8)&0x0f] ^ x[(i-3)&0x0f], (x[i&0x0f] = ROL(tm, 1)))

/* Bitwise rotation of an unsigned int to the left **/
#define	ROL(x, bits) (((x) << (bits)) | ((uint)(x) >> (sizeof(uint)*8 - (bits))))

  #define R(a, b, c, d, e, f, k, m)		\
    do 						\
    {						\
      e += ROL(a, 5) + f(b, c, d) + k + m;	\
      b = ROL(b, 30);				\
    } while(0)

  R( a, b, c, d, e, F1, K1, x[ 0] );
  R( e, a, b, c, d, F1, K1, x[ 1] );
  R( d, e, a, b, c, F1, K1, x[ 2] );
  R( c, d, e, a, b, F1, K1, x[ 3] );
  R( b, c, d, e, a, F1, K1, x[ 4] );
  R( a, b, c, d, e, F1, K1, x[ 5] );
  R( e, a, b, c, d, F1, K1, x[ 6] );
  R( d, e, a, b, c, F1, K1, x[ 7] );
  R( c, d, e, a, b, F1, K1, x[ 8] );
  R( b, c, d, e, a, F1, K1, x[ 9] );
  R( a, b, c, d, e, F1, K1, x[10] );
  R( e, a, b, c, d, F1, K1, x[11] );
  R( d, e, a, b, c, F1, K1, x[12] );
  R( c, d, e, a, b, F1, K1, x[13] );
  R( b, c, d, e, a, F1, K1, x[14] );
  R( a, b, c, d, e, F1, K1, x[15] );
  R( e, a, b, c, d, F1, K1, M(16) );
  R( d, e, a, b, c, F1, K1, M(17) );
  R( c, d, e, a, b, F1, K1, M(18) );
  R( b, c, d, e, a, F1, K1, M(19) );
  R( a, b, c, d, e, F2, K2, M(20) );
  R( e, a, b, c, d, F2, K2, M(21) );
  R( d, e, a, b, c, F2, K2, M(22) );
  R( c, d, e, a, b, F2, K2, M(23) );
  R( b, c, d, e, a, F2, K2, M(24) );
  R( a, b, c, d, e, F2, K2, M(25) );
  R( e, a, b, c, d, F2, K2, M(26) );
  R( d, e, a, b, c, F2, K2, M(27) );
  R( c, d, e, a, b, F2, K2, M(28) );
  R( b, c, d, e, a, F2, K2, M(29) );
  R( a, b, c, d, e, F2, K2, M(30) );
  R( e, a, b, c, d, F2, K2, M(31) );
  R( d, e, a, b, c, F2, K2, M(32) );
  R( c, d, e, a, b, F2, K2, M(33) );
  R( b, c, d, e, a, F2, K2, M(34) );
  R( a, b, c, d, e, F2, K2, M(35) );
  R( e, a, b, c, d, F2, K2, M(36) );
  R( d, e, a, b, c, F2, K2, M(37) );
  R( c, d, e, a, b, F2, K2, M(38) );
  R( b, c, d, e, a, F2, K2, M(39) );
  R( a, b, c, d, e, F3, K3, M(40) );
  R( e, a, b, c, d, F3, K3, M(41) );
  R( d, e, a, b, c, F3, K3, M(42) );
  R( c, d, e, a, b, F3, K3, M(43) );
  R( b, c, d, e, a, F3, K3, M(44) );
  R( a, b, c, d, e, F3, K3, M(45) );
  R( e, a, b, c, d, F3, K3, M(46) );
  R( d, e, a, b, c, F3, K3, M(47) );
  R( c, d, e, a, b, F3, K3, M(48) );
  R( b, c, d, e, a, F3, K3, M(49) );
  R( a, b, c, d, e, F3, K3, M(50) );
  R( e, a, b, c, d, F3, K3, M(51) );
  R( d, e, a, b, c, F3, K3, M(52) );
  R( c, d, e, a, b, F3, K3, M(53) );
  R( b, c, d, e, a, F3, K3, M(54) );
  R( a, b, c, d, e, F3, K3, M(55) );
  R( e, a, b, c, d, F3, K3, M(56) );
  R( d, e, a, b, c, F3, K3, M(57) );
  R( c, d, e, a, b, F3, K3, M(58) );
  R( b, c, d, e, a, F3, K3, M(59) );
  R( a, b, c, d, e, F4, K4, M(60) );
  R( e, a, b, c, d, F4, K4, M(61) );
  R( d, e, a, b, c, F4, K4, M(62) );
  R( c, d, e, a, b, F4, K4, M(63) );
  R( b, c, d, e, a, F4, K4, M(64) );
  R( a, b, c, d, e, F4, K4, M(65) );
  R( e, a, b, c, d, F4, K4, M(66) );
  R( d, e, a, b, c, F4, K4, M(67) );
  R( c, d, e, a, b, F4, K4, M(68) );
  R( b, c, d, e, a, F4, K4, M(69) );
  R( a, b, c, d, e, F4, K4, M(70) );
  R( e, a, b, c, d, F4, K4, M(71) );
  R( d, e, a, b, c, F4, K4, M(72) );
  R( c, d, e, a, b, F4, K4, M(73) );
  R( b, c, d, e, a, F4, K4, M(74) );
  R( a, b, c, d, e, F4, K4, M(75) );
  R( e, a, b, c, d, F4, K4, M(76) );
  R( d, e, a, b, c, F4, K4, M(77) );
  R( c, d, e, a, b, F4, K4, M(78) );
  R( b, c, d, e, a, F4, K4, M(79) );

  /* Update chaining vars. */
  ctx->h0 += a;
  ctx->h1 += b;
  ctx->h2 += c;
  ctx->h3 += d;
  ctx->h4 += e;
}

/*
 * Update the message digest with the contents of BUF with length LEN.
 */
void
sha1_update(struct hash_context *CTX, const byte *buf, uint len)
{
  struct sha1_context *ctx = (void *) CTX;

  if (ctx->count)
  {
    /* Fill rest of internal buffer */
    for (; len && ctx->count < SHA1_BLOCK_SIZE; len--)
      ctx->buf[ctx->count++] = *buf++;

    if (ctx->count < SHA1_BLOCK_SIZE)
      return;

    /* Process data from internal buffer */
    sha1_transform(ctx, ctx->buf);
    ctx->nblocks++;
    ctx->count = 0;
  }

  if (!len)
    return;

  /* Process data from input buffer */
  while (len >= SHA1_BLOCK_SIZE)
  {
    sha1_transform(ctx, buf);
    ctx->nblocks++;
    buf += SHA1_BLOCK_SIZE;
    len -= SHA1_BLOCK_SIZE;
  }

  /* Copy remaining data to internal buffer */
  memcpy(ctx->buf, buf, len);
  ctx->count = len;
}

/*
 * The routine final terminates the computation and returns the digest. The
 * handle is prepared for a new cycle, but adding bytes to the handle will the
 * destroy the returned buffer.
 *
 * Returns: 20 bytes representing the digest.
 */
byte *
sha1_final(struct hash_context *CTX)
{
  struct sha1_context *ctx = (void *) CTX;
  u32 t, msb, lsb;

  sha1_update(CTX, NULL, 0);	/* flush */

  t = ctx->nblocks;
  /* multiply by 64 to make a byte count */
  lsb = t << 6;
  msb = t >> 26;
  /* add the count */
  t = lsb;
  if ((lsb += ctx->count) < t)
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 29;

  if (ctx->count < 56)
  {
    /* enough room */
    ctx->buf[ctx->count++] = 0x80; /* pad */
    while (ctx->count < 56)
      ctx->buf[ctx->count++] = 0;  /* pad */
  }
  else
  {
    /* need one extra block */
    ctx->buf[ctx->count++] = 0x80; /* pad character */
    while (ctx->count < 64)
      ctx->buf[ctx->count++] = 0;
    sha1_update(CTX, NULL, 0);	/* flush */
    memset(ctx->buf, 0, 56); /* fill next block with zeroes */
  }

  /* append the 64 bit count */
  ctx->buf[56] = msb >> 24;
  ctx->buf[57] = msb >> 16;
  ctx->buf[58] = msb >>  8;
  ctx->buf[59] = msb;
  ctx->buf[60] = lsb >> 24;
  ctx->buf[61] = lsb >> 16;
  ctx->buf[62] = lsb >>  8;
  ctx->buf[63] = lsb;
  sha1_transform(ctx, ctx->buf);

  byte *p = ctx->buf;
#define X(a) do { put_u32(p, ctx->h##a); p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
#undef X

  return ctx->buf;
}
