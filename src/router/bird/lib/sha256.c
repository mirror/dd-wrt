/*
 *	BIRD Library -- SHA-256 and SHA-224 Hash Functions
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Based on the code from libgcrypt-1.6.0, which is
 *	(c) 2003, 2006, 2008, 2009 Free Software Foundation, Inc.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "lib/sha256.h"
#include "lib/unaligned.h"


// #define SHA256_UNROLLED

void
sha256_init(struct hash_context *CTX)
{
  struct sha256_context *ctx = (void *) CTX;

  ctx->h0 = 0x6a09e667;
  ctx->h1 = 0xbb67ae85;
  ctx->h2 = 0x3c6ef372;
  ctx->h3 = 0xa54ff53a;
  ctx->h4 = 0x510e527f;
  ctx->h5 = 0x9b05688c;
  ctx->h6 = 0x1f83d9ab;
  ctx->h7 = 0x5be0cd19;

  ctx->nblocks = 0;
  ctx->count = 0;
}

void
sha224_init(struct hash_context *CTX)
{
  struct sha224_context *ctx = (void *) CTX;

  ctx->h0 = 0xc1059ed8;
  ctx->h1 = 0x367cd507;
  ctx->h2 = 0x3070dd17;
  ctx->h3 = 0xf70e5939;
  ctx->h4 = 0xffc00b31;
  ctx->h5 = 0x68581511;
  ctx->h6 = 0x64f98fa7;
  ctx->h7 = 0xbefa4fa4;

  ctx->nblocks = 0;
  ctx->count = 0;
}

/* (4.2) same as SHA-1's F1.  */
static inline u32
f1(u32 x, u32 y, u32 z)
{
  return (z ^ (x & (y ^ z)));
}

/* (4.3) same as SHA-1's F3 */
static inline u32
f3(u32 x, u32 y, u32 z)
{
  return ((x & y) | (z & (x|y)));
}

/* Bitwise rotation of an uint to the right */
static inline u32 ror(u32 x, int n)
{
  return ((x >> (n&(32-1))) | (x << ((32-n)&(32-1))));
}

/* (4.4) */
static inline u32
sum0(u32 x)
{
  return (ror(x, 2) ^ ror(x, 13) ^ ror(x, 22));
}

/* (4.5) */
static inline u32
sum1(u32 x)
{
  return (ror(x, 6) ^ ror(x, 11) ^ ror(x, 25));
}

/*
  Transform the message X which consists of 16 32-bit-words. See FIPS
  180-2 for details.  */
#define S0(x) (ror((x),  7) ^ ror((x), 18) ^ ((x) >>  3))	/* (4.6) */
#define S1(x) (ror((x), 17) ^ ror((x), 19) ^ ((x) >> 10))	/* (4.7) */
#define R(a,b,c,d,e,f,g,h,k,w)					\
    do								\
    {								\
      t1 = (h) + sum1((e)) + f1((e),(f),(g)) + (k) + (w);	\
      t2 = sum0((a)) + f3((a),(b),(c));				\
      h = g;							\
      g = f;							\
      f = e;							\
      e = d + t1;						\
      d = c;							\
      c = b;							\
      b = a;							\
      a = t1 + t2;						\
    } while (0)

/*
    The SHA-256 core: Transform the message X which consists of 16
    32-bit-words. See FIPS 180-2 for details.
 */
static uint
sha256_transform(struct sha256_context *ctx, const byte *data)
{
  static const u32 K[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
      0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
      0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
      0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
      0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
      0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };

  u32 a,b,c,d,e,f,g,h,t1,t2;
  u32 w[64];
  int i;

  a = ctx->h0;
  b = ctx->h1;
  c = ctx->h2;
  d = ctx->h3;
  e = ctx->h4;
  f = ctx->h5;
  g = ctx->h6;
  h = ctx->h7;

  for (i = 0; i < 16; i++)
    w[i] = get_u32(data + i * 4);

  for (; i < 64; i++)
    w[i] = S1(w[i-2]) + w[i-7] + S0(w[i-15]) + w[i-16];

  for (i = 0; i < 64;)
  {
#ifndef SHA256_UNROLLED
    R(a,b,c,d,e,f,g,h,K[i],w[i]);
    i++;
#else /* Unrolled */
    t1 = h + sum1(e) + f1(e, f, g) + K[i] + w[i];
    t2 = sum0(a) + f3(a, b, c);
    d += t1;
    h  = t1 + t2;

    t1 = g + sum1(d) + f1(d, e, f) + K[i+1] + w[i+1];
    t2 = sum0(h) + f3(h, a, b);
    c += t1;
    g  = t1 + t2;

    t1 = f + sum1(c) + f1(c, d, e) + K[i+2] + w[i+2];
    t2 = sum0(g) + f3(g, h, a);
    b += t1;
    f  = t1 + t2;

    t1 = e + sum1(b) + f1(b, c, d) + K[i+3] + w[i+3];
    t2 = sum0(f) + f3(f, g, h);
    a += t1;
    e  = t1 + t2;

    t1 = d + sum1(a) + f1(a, b, c) + K[i+4] + w[i+4];
    t2 = sum0(e) + f3(e, f, g);
    h += t1;
    d  = t1 + t2;

    t1 = c + sum1(h) + f1(h, a, b) + K[i+5] + w[i+5];
    t2 = sum0(d) + f3(d, e, f);
    g += t1;
    c  = t1 + t2;

    t1 = b + sum1(g) + f1(g, h, a) + K[i+6] + w[i+6];
    t2 = sum0(c) + f3(c, d, e);
    f += t1;
    b  = t1 + t2;

    t1 = a + sum1(f) + f1(f, g, h) + K[i+7] + w[i+7];
    t2 = sum0(b) + f3(b, c, d);
    e += t1;
    a  = t1 + t2;

    i += 8;
#endif
  }

  ctx->h0 += a;
  ctx->h1 += b;
  ctx->h2 += c;
  ctx->h3 += d;
  ctx->h4 += e;
  ctx->h5 += f;
  ctx->h6 += g;
  ctx->h7 += h;

  return /*burn_stack*/ 74*4+32;
}
#undef S0
#undef S1
#undef R

/* Common function to write a chunk of data to the transform function
   of a hash algorithm.  Note that the use of the term "block" does
   not imply a fixed size block.  Note that we explicitly allow to use
   this function after the context has been finalized; the result does
   not have any meaning but writing after finalize is sometimes
   helpful to mitigate timing attacks. */
void
sha256_update(struct hash_context *CTX, const byte *buf, uint len)
{
  struct sha256_context *ctx = (void *) CTX;

  if (ctx->count)
  {
    /* Fill rest of internal buffer */
    for (; len && ctx->count < SHA256_BLOCK_SIZE; len--)
      ctx->buf[ctx->count++] = *buf++;

    if (ctx->count < SHA256_BLOCK_SIZE)
      return;

    /* Process data from internal buffer */
    sha256_transform(ctx, ctx->buf);
    ctx->nblocks++;
    ctx->count = 0;
  }

  if (!len)
    return;

  /* Process data from input buffer */
  while (len >= SHA256_BLOCK_SIZE)
  {
    sha256_transform(ctx, buf);
    ctx->nblocks++;
    buf += SHA256_BLOCK_SIZE;
    len -= SHA256_BLOCK_SIZE;
  }

  /* Copy remaining data to internal buffer */
  memcpy(ctx->buf, buf, len);
  ctx->count = len;
}

/*
 * The routine finally terminates the computation and returns the digest.  The
 * handle is prepared for a new cycle, but adding bytes to the handle will the
 * destroy the returned buffer.
 *
 * Returns: 32 bytes with the message the digest. 28 bytes for SHA-224.
 */
byte *
sha256_final(struct hash_context *CTX)
{
  struct sha256_context *ctx = (void *) CTX;
  u32 t, th, msb, lsb;

  sha256_update(CTX, NULL, 0);	/* flush */

  t = ctx->nblocks;
  th = 0;

  /* multiply by 64 to make a byte count */
  lsb = t << 6;
  msb = (th << 6) | (t >> 26);
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
    sha256_update(CTX, NULL, 0);  /* flush */;
    memset(ctx->buf, 0, 56 ); /* fill next block with zeroes */
  }

  /* append the 64 bit count */
  put_u32(ctx->buf + 56, msb);
  put_u32(ctx->buf + 60, lsb);
  sha256_transform(ctx, ctx->buf);

  byte *p = ctx->buf;
#define X(a) do { put_u32(p, ctx->h##a); p += 4; } while(0)
  X(0);
  X(1);
  X(2);
  X(3);
  X(4);
  X(5);
  X(6);
  X(7);
#undef X

  return ctx->buf;
}
