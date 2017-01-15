/*
 *	BIRD Library -- SHA-512 and SHA-384 Hash Functions
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Based on the code from libgcrypt-1.6.0, which is
 *	(c) 2003, 2006, 2008, 2009 Free Software Foundation, Inc.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "lib/sha512.h"
#include "lib/unaligned.h"


// #define SHA512_UNROLLED

void
sha512_init(struct hash_context *CTX)
{
  struct sha512_context *ctx = (void *) CTX;

  ctx->h0 = U64(0x6a09e667f3bcc908);
  ctx->h1 = U64(0xbb67ae8584caa73b);
  ctx->h2 = U64(0x3c6ef372fe94f82b);
  ctx->h3 = U64(0xa54ff53a5f1d36f1);
  ctx->h4 = U64(0x510e527fade682d1);
  ctx->h5 = U64(0x9b05688c2b3e6c1f);
  ctx->h6 = U64(0x1f83d9abfb41bd6b);
  ctx->h7 = U64(0x5be0cd19137e2179);

  ctx->nblocks = 0;
  ctx->count = 0;
}

void
sha384_init(struct hash_context *CTX)
{
  struct sha384_context *ctx = (void *) CTX;

  ctx->h0 = U64(0xcbbb9d5dc1059ed8);
  ctx->h1 = U64(0x629a292a367cd507);
  ctx->h2 = U64(0x9159015a3070dd17);
  ctx->h3 = U64(0x152fecd8f70e5939);
  ctx->h4 = U64(0x67332667ffc00b31);
  ctx->h5 = U64(0x8eb44a8768581511);
  ctx->h6 = U64(0xdb0c2e0d64f98fa7);
  ctx->h7 = U64(0x47b5481dbefa4fa4);

  ctx->nblocks = 0;
  ctx->count = 0;
}

static inline u64
ROTR(u64 x, u64 n)
{
  return ((x >> n) | (x << (64 - n)));
}

static inline u64
Ch(u64 x, u64 y, u64 z)
{
  return ((x & y) ^ ( ~x & z));
}

static inline u64
Maj(u64 x, u64 y, u64 z)
{
  return ((x & y) ^ (x & z) ^ (y & z));
}

static inline u64
sum0(u64 x)
{
  return (ROTR(x, 28) ^ ROTR(x, 34) ^ ROTR(x, 39));
}

static inline u64
sum1(u64 x)
{
  return (ROTR(x, 14) ^ ROTR(x, 18) ^ ROTR(x, 41));
}

static const u64 k[] =
{
    U64(0x428a2f98d728ae22), U64(0x7137449123ef65cd),
    U64(0xb5c0fbcfec4d3b2f), U64(0xe9b5dba58189dbbc),
    U64(0x3956c25bf348b538), U64(0x59f111f1b605d019),
    U64(0x923f82a4af194f9b), U64(0xab1c5ed5da6d8118),
    U64(0xd807aa98a3030242), U64(0x12835b0145706fbe),
    U64(0x243185be4ee4b28c), U64(0x550c7dc3d5ffb4e2),
    U64(0x72be5d74f27b896f), U64(0x80deb1fe3b1696b1),
    U64(0x9bdc06a725c71235), U64(0xc19bf174cf692694),
    U64(0xe49b69c19ef14ad2), U64(0xefbe4786384f25e3),
    U64(0x0fc19dc68b8cd5b5), U64(0x240ca1cc77ac9c65),
    U64(0x2de92c6f592b0275), U64(0x4a7484aa6ea6e483),
    U64(0x5cb0a9dcbd41fbd4), U64(0x76f988da831153b5),
    U64(0x983e5152ee66dfab), U64(0xa831c66d2db43210),
    U64(0xb00327c898fb213f), U64(0xbf597fc7beef0ee4),
    U64(0xc6e00bf33da88fc2), U64(0xd5a79147930aa725),
    U64(0x06ca6351e003826f), U64(0x142929670a0e6e70),
    U64(0x27b70a8546d22ffc), U64(0x2e1b21385c26c926),
    U64(0x4d2c6dfc5ac42aed), U64(0x53380d139d95b3df),
    U64(0x650a73548baf63de), U64(0x766a0abb3c77b2a8),
    U64(0x81c2c92e47edaee6), U64(0x92722c851482353b),
    U64(0xa2bfe8a14cf10364), U64(0xa81a664bbc423001),
    U64(0xc24b8b70d0f89791), U64(0xc76c51a30654be30),
    U64(0xd192e819d6ef5218), U64(0xd69906245565a910),
    U64(0xf40e35855771202a), U64(0x106aa07032bbd1b8),
    U64(0x19a4c116b8d2d0c8), U64(0x1e376c085141ab53),
    U64(0x2748774cdf8eeb99), U64(0x34b0bcb5e19b48a8),
    U64(0x391c0cb3c5c95a63), U64(0x4ed8aa4ae3418acb),
    U64(0x5b9cca4f7763e373), U64(0x682e6ff3d6b2b8a3),
    U64(0x748f82ee5defb2fc), U64(0x78a5636f43172f60),
    U64(0x84c87814a1f0ab72), U64(0x8cc702081a6439ec),
    U64(0x90befffa23631e28), U64(0xa4506cebde82bde9),
    U64(0xbef9a3f7b2c67915), U64(0xc67178f2e372532b),
    U64(0xca273eceea26619c), U64(0xd186b8c721c0c207),
    U64(0xeada7dd6cde0eb1e), U64(0xf57d4f7fee6ed178),
    U64(0x06f067aa72176fba), U64(0x0a637dc5a2c898a6),
    U64(0x113f9804bef90dae), U64(0x1b710b35131c471b),
    U64(0x28db77f523047d84), U64(0x32caab7b40c72493),
    U64(0x3c9ebe0a15c9bebc), U64(0x431d67c49c100d4c),
    U64(0x4cc5d4becb3e42b6), U64(0x597f299cfc657e2a),
    U64(0x5fcb6fab3ad6faec), U64(0x6c44198c4a475817)
};

/*
 * Transform the message W which consists of 16 64-bit-words
 */
static uint
sha512_transform(struct sha512_context *ctx, const byte *data)
{
  u64 a, b, c, d, e, f, g, h;
  u64 w[16];
  uint t;

  /* get values from the chaining vars */
  a = ctx->h0;
  b = ctx->h1;
  c = ctx->h2;
  d = ctx->h3;
  e = ctx->h4;
  f = ctx->h5;
  g = ctx->h6;
  h = ctx->h7;

  for (t = 0; t < 16; t++)
    w[t] = get_u64(data + t * 8);

#define S0(x) (ROTR((x),1) ^ ROTR((x),8) ^ ((x)>>7))
#define S1(x) (ROTR((x),19) ^ ROTR((x),61) ^ ((x)>>6))

  for (t = 0; t < 80 - 16; )
  {
    u64 t1, t2;

    /* Performance on a AMD Athlon(tm) Dual Core Processor 4050e
         with gcc 4.3.3 using gcry_md_hash_buffer of each 10000 bytes
         initialized to 0,1,2,3...255,0,... and 1000 iterations:

         Not unrolled with macros:  440ms
         Unrolled with macros:      350ms
         Unrolled with inline:      330ms
     */
#ifndef SHA512_UNROLLED
    t1 = h + sum1(e) + Ch(e, f, g) + k[t] + w[t%16];
    w[t%16] += S1(w[(t - 2)%16]) + w[(t - 7)%16] + S0(w[(t - 15)%16]);
    t2 = sum0(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
    t++;
#else /* Unrolled */
    t1 = h + sum1(e) + Ch(e, f, g) + k[t] + w[0];
    w[0] += S1(w[14]) + w[9] + S0(w[1]);
    t2 = sum0(a) + Maj(a, b, c);
    d += t1;
    h = t1 + t2;

    t1 = g + sum1(d) + Ch(d, e, f) + k[t+1] + w[1];
    w[1] += S1(w[15]) + w[10] + S0(w[2]);
    t2 = sum0(h) + Maj(h, a, b);
    c += t1;
    g  = t1 + t2;

    t1 = f + sum1(c) + Ch(c, d, e) + k[t+2] + w[2];
    w[2] += S1(w[0]) + w[11] + S0(w[3]);
    t2 = sum0(g) + Maj(g, h, a);
    b += t1;
    f  = t1 + t2;

    t1 = e + sum1(b) + Ch(b, c, d) + k[t+3] + w[3];
    w[3] += S1(w[1]) + w[12] + S0(w[4]);
    t2 = sum0(f) + Maj(f, g, h);
    a += t1;
    e  = t1 + t2;

    t1 = d + sum1(a) + Ch(a, b, c) + k[t+4] + w[4];
    w[4] += S1(w[2]) + w[13] + S0(w[5]);
    t2 = sum0(e) + Maj(e, f, g);
    h += t1;
    d  = t1 + t2;

    t1 = c + sum1(h) + Ch(h, a, b) + k[t+5] + w[5];
    w[5] += S1(w[3]) + w[14] + S0(w[6]);
    t2 = sum0(d) + Maj(d, e, f);
    g += t1;
    c  = t1 + t2;

    t1 = b + sum1(g) + Ch(g, h, a) + k[t+6] + w[6];
    w[6] += S1(w[4]) + w[15] + S0(w[7]);
    t2 = sum0(c) + Maj(c, d, e);
    f += t1;
    b  = t1 + t2;

    t1 = a + sum1(f) + Ch(f, g, h) + k[t+7] + w[7];
    w[7] += S1(w[5]) + w[0] + S0(w[8]);
    t2 = sum0(b) + Maj(b, c, d);
    e += t1;
    a  = t1 + t2;

    t1 = h + sum1(e) + Ch(e, f, g) + k[t+8] + w[8];
    w[8] += S1(w[6]) + w[1] + S0(w[9]);
    t2 = sum0(a) + Maj(a, b, c);
    d += t1;
    h  = t1 + t2;

    t1 = g + sum1(d) + Ch(d, e, f) + k[t+9] + w[9];
    w[9] += S1(w[7]) + w[2] + S0(w[10]);
    t2 = sum0(h) + Maj(h, a, b);
    c += t1;
    g  = t1 + t2;

    t1 = f + sum1(c) + Ch(c, d, e) + k[t+10] + w[10];
    w[10] += S1(w[8]) + w[3] + S0(w[11]);
    t2 = sum0(g) + Maj(g, h, a);
    b += t1;
    f  = t1 + t2;

    t1 = e + sum1(b) + Ch(b, c, d) + k[t+11] + w[11];
    w[11] += S1(w[9]) + w[4] + S0(w[12]);
    t2 = sum0(f) + Maj(f, g, h);
    a += t1;
    e  = t1 + t2;

    t1 = d + sum1(a) + Ch(a, b, c) + k[t+12] + w[12];
    w[12] += S1(w[10]) + w[5] + S0(w[13]);
    t2 = sum0(e) + Maj(e, f, g);
    h += t1;
    d  = t1 + t2;

    t1 = c + sum1(h) + Ch(h, a, b) + k[t+13] + w[13];
    w[13] += S1(w[11]) + w[6] + S0(w[14]);
    t2 = sum0(d) + Maj(d, e, f);
    g += t1;
    c  = t1 + t2;

    t1 = b + sum1(g) + Ch(g, h, a) + k[t+14] + w[14];
    w[14] += S1(w[12]) + w[7] + S0(w[15]);
    t2 = sum0(c) + Maj(c, d, e);
    f += t1;
    b  = t1 + t2;

    t1 = a + sum1(f) + Ch(f, g, h) + k[t+15] + w[15];
    w[15] += S1(w[13]) + w[8] + S0(w[0]);
    t2 = sum0(b) + Maj(b, c, d);
    e += t1;
    a  = t1 + t2;

    t += 16;
#endif
  }

  for (; t < 80; )
  {
    u64 t1, t2;

#ifndef SHA512_UNROLLED
    t1 = h + sum1(e) + Ch(e, f, g) + k[t] + w[t%16];
    t2 = sum0(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
    t++;
#else /* Unrolled */
    t1 = h + sum1(e) + Ch(e, f, g) + k[t] + w[0];
    t2 = sum0(a) + Maj(a, b, c);
    d += t1;
    h  = t1 + t2;

    t1 = g + sum1(d) + Ch(d, e, f) + k[t+1] + w[1];
    t2 = sum0(h) + Maj(h, a, b);
    c += t1;
    g  = t1 + t2;

    t1 = f + sum1(c) + Ch(c, d, e) + k[t+2] + w[2];
    t2 = sum0(g) + Maj(g, h, a);
    b += t1;
    f  = t1 + t2;

    t1 = e + sum1(b) + Ch(b, c, d) + k[t+3] + w[3];
    t2 = sum0(f) + Maj(f, g, h);
    a += t1;
    e  = t1 + t2;

    t1 = d + sum1(a) + Ch(a, b, c) + k[t+4] + w[4];
    t2 = sum0(e) + Maj(e, f, g);
    h += t1;
    d  = t1 + t2;

    t1 = c + sum1(h) + Ch(h, a, b) + k[t+5] + w[5];
    t2 = sum0(d) + Maj(d, e, f);
    g += t1;
    c  = t1 + t2;

    t1 = b + sum1(g) + Ch(g, h, a) + k[t+6] + w[6];
    t2 = sum0(c) + Maj(c, d, e);
    f += t1;
    b  = t1 + t2;

    t1 = a + sum1(f) + Ch(f, g, h) + k[t+7] + w[7];
    t2 = sum0(b) + Maj(b, c, d);
    e += t1;
    a  = t1 + t2;

    t1 = h + sum1(e) + Ch(e, f, g) + k[t+8] + w[8];
    t2 = sum0(a) + Maj(a, b, c);
    d += t1;
    h  = t1 + t2;

    t1 = g + sum1(d) + Ch(d, e, f) + k[t+9] + w[9];
    t2 = sum0(h) + Maj(h, a, b);
    c += t1;
    g  = t1 + t2;

    t1 = f + sum1(c) + Ch(c, d, e) + k[t+10] + w[10];
    t2 = sum0(g) + Maj(g, h, a);
    b += t1;
    f  = t1 + t2;

    t1 = e + sum1(b) + Ch(b, c, d) + k[t+11] + w[11];
    t2 = sum0(f) + Maj(f, g, h);
    a += t1;
    e  = t1 + t2;

    t1 = d + sum1(a) + Ch(a, b, c) + k[t+12] + w[12];
    t2 = sum0(e) + Maj(e, f, g);
    h += t1;
    d  = t1 + t2;

    t1 = c + sum1(h) + Ch(h, a, b) + k[t+13] + w[13];
    t2 = sum0(d) + Maj(d, e, f);
    g += t1;
    c  = t1 + t2;

    t1 = b + sum1(g) + Ch(g, h, a) + k[t+14] + w[14];
    t2 = sum0(c) + Maj(c, d, e);
    f += t1;
    b  = t1 + t2;

    t1 = a + sum1(f) + Ch(f, g, h) + k[t+15] + w[15];
    t2 = sum0(b) + Maj(b, c, d);
    e += t1;
    a  = t1 + t2;

    t += 16;
#endif
  }

  /* Update chaining vars.  */
  ctx->h0 += a;
  ctx->h1 += b;
  ctx->h2 += c;
  ctx->h3 += d;
  ctx->h4 += e;
  ctx->h5 += f;
  ctx->h6 += g;
  ctx->h7 += h;

  return /* burn_stack */ (8 + 16) * sizeof(u64) + sizeof(u32) + 3 * sizeof(void*);
}

void
sha512_update(struct hash_context *CTX, const byte *buf, uint len)
{
  struct sha512_context *ctx = (void *) CTX;

  if (ctx->count)
  {
    /* Fill rest of internal buffer */
    for (; len && ctx->count < SHA512_BLOCK_SIZE; len--)
      ctx->buf[ctx->count++] = *buf++;

    if (ctx->count < SHA512_BLOCK_SIZE)
      return;

    /* Process data from internal buffer */
    sha512_transform(ctx, ctx->buf);
    ctx->nblocks++;
    ctx->count = 0;
  }

  if (!len)
    return;

  /* Process data from input buffer */
  while (len >= SHA512_BLOCK_SIZE)
  {
    sha512_transform(ctx, buf);
    ctx->nblocks++;
    buf += SHA512_BLOCK_SIZE;
    len -= SHA512_BLOCK_SIZE;
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
 * Returns: 64 bytes representing the digest. When used for sha384, we take the
 * first 48 of those bytes.
 */
byte *
sha512_final(struct hash_context *CTX)
{
  struct sha512_context *ctx = (void *) CTX;
  u64 t, th, msb, lsb;

  sha512_update(CTX, NULL, 0);	/* flush */

  t = ctx->nblocks;
  th = 0;

  /* multiply by 128 to make a byte count */
  lsb = t << 7;
  msb = (th << 7) | (t >> 57);
  /* add the count */
  t = lsb;
  if ((lsb += ctx->count) < t)
    msb++;
  /* multiply by 8 to make a bit count */
  t = lsb;
  lsb <<= 3;
  msb <<= 3;
  msb |= t >> 61;

  if (ctx->count < 112)
  {
    /* enough room */
    ctx->buf[ctx->count++] = 0x80;	/* pad */
    while(ctx->count < 112)
      ctx->buf[ctx->count++] = 0;	/* pad */
  }
  else
  {
    /* need one extra block */
    ctx->buf[ctx->count++] = 0x80;	/* pad character */
    while(ctx->count < 128)
      ctx->buf[ctx->count++] = 0;
    sha512_update(CTX, NULL, 0); 	/* flush */
    memset(ctx->buf, 0, 112);		/* fill next block with zeroes */
  }

  /* append the 128 bit count */
  put_u64(ctx->buf + 112, msb);
  put_u64(ctx->buf + 120, lsb);
  sha512_transform(ctx, ctx->buf);

  byte *p = ctx->buf;
#define X(a) do { put_u64(p, ctx->h##a); p += 8; } while(0)
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
