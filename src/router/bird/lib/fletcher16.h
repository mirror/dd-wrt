/*
 *	BIRD Library -- Fletcher-16 checksum
 *
 *	(c) 2015 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Fletcher-16 checksum
 *
 * Fletcher-16 checksum is a position-dependent checksum algorithm used for
 * error-detection e.g. in OSPF LSAs.
 *
 * To generate Fletcher-16 checksum, zero the checksum field in data, initialize
 * the context by fletcher16_init(), process the data by fletcher16_update(),
 * compute the checksum value by fletcher16_final() and store it to the checksum
 * field in data by put_u16() (or other means involving htons() conversion).
 *
 * To verify Fletcher-16 checksum, initialize the context by fletcher16_init(),
 * process the data by fletcher16_update(), compute a passing checksum by
 * fletcher16_compute() and check if it is zero.
 */

#ifndef _BIRD_FLETCHER16_H_
#define _BIRD_FLETCHER16_H_

#include "nest/bird.h"


struct fletcher16_context
{
  int c0, c1;
};


/**
 * fletcher16_init - initialize Fletcher-16 context
 * @ctx: the context
 */
static inline void
fletcher16_init(struct fletcher16_context *ctx)
{
  ctx->c0 = ctx->c1 = 0;
}

/**
 * fletcher16_update - process data to Fletcher-16 context
 * @ctx: the context
 * @buf: data buffer
 * @len: data length
 *
 * fletcher16_update() reads data from the buffer @buf and updates passing sums
 * in the context @ctx. It may be used multiple times for multiple blocks of
 * checksummed data.
 */
static inline void
fletcher16_update(struct fletcher16_context *ctx, const u8* buf, int len)
{
  /*
   * The Fletcher-16 sum is essentially a sequence of
   * ctx->c1 += ctx->c0 += *buf++, modulo 255.
   *
   * In the inner loop, we eliminate modulo operation and we do some loop
   * unrolling. MODX is the maximal number of steps that can be done without
   * modulo before overflow, see RFC 1008 for details. We use a bit smaller
   * value to cover for initial steps due to loop unrolling.
   */

#define MODX 4096

  int blen, i;

  blen = len % 4;
  len -= blen;

  for (i = 0; i < blen; i++)
    ctx->c1 += ctx->c0 += *buf++;

  do {
    blen = MIN(len, MODX);
    len -= blen;

    for (i = 0; i < blen; i += 4)
    {
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
    }

    ctx->c0 %= 255;
    ctx->c1 %= 255;

  } while (len);
}


/**
 * fletcher16_update_n32 - process data to Fletcher-16 context, with endianity adjustment
 * @ctx: the context
 * @buf: data buffer
 * @len: data length
 *
 * fletcher16_update_n32() works like fletcher16_update(), except it applies
 * 32-bit host/network endianity swap to the data before they are processed.
 * I.e., it assumes that the data is a sequence of u32 that must be converted by
 * ntohl() or htonl() before processing. The @buf need not to be aligned, but
 * its length (@len) must be multiple of 4. Note that on big endian systems the
 * host endianity is the same as the network endianity, therefore there is no
 * endianity swap.
 */
static inline void
fletcher16_update_n32(struct fletcher16_context *ctx, const u8* buf, int len)
{
  /* See fletcher16_update() for details */

  int blen, i;

  do {
    blen = MIN(len, MODX);
    len -= blen;

    for (i = 0; i < blen; i += 4)
    {
#ifdef CPU_BIG_ENDIAN
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
      ctx->c1 += ctx->c0 += *buf++;
#else
      ctx->c1 += ctx->c0 += buf[3];
      ctx->c1 += ctx->c0 += buf[2];
      ctx->c1 += ctx->c0 += buf[1];
      ctx->c1 += ctx->c0 += buf[0];
      buf += 4;
#endif
    }

    ctx->c0 %= 255;
    ctx->c1 %= 255;

  } while (len);
}

/**
 * fletcher16_final - compute final Fletcher-16 checksum value
 * @ctx: the context
 * @len: total data length
 * @pos: offset in data where the checksum will be stored
 *
 * fletcher16_final() computes the final checksum value and returns it.
 * The caller is responsible for storing it in the appropriate position.
 * The checksum value depends on @len and @pos, but only their difference
 * (i.e. the offset from the end) is significant.
 *
 * The checksum value is represented as u16, although it is defined as two
 * consecutive bytes. We treat them as one u16 in big endian / network order.
 * I.e., the returned value is in the form that would be returned by get_u16()
 * from the checksum field in the data buffer, therefore the caller should use
 * put_u16() or an explicit host-to-network conversion when storing it to the
 * checksum field in the data buffer.
 *
 * Note that the returned checksum value is always nonzero.
 */
static inline u16
fletcher16_final(struct fletcher16_context *ctx, int len, int pos)
{
  int x = ((len - pos - 1) * ctx->c0 - ctx->c1) % 255;
  if (x <= 0)
    x += 255;

  int y = 510 - ctx->c0 - x;
  if (y > 255)
    y -= 255;

  return (x << 8) | y;
}


/**
 * fletcher16_compute - compute Fletcher-16 sum for verification
 * @ctx: the context
 *
 * fletcher16_compute() returns a passing Fletcher-16 sum for processed data.
 * If the data contains the proper Fletcher-16 checksum value, the returned
 * value is zero.
 */
static inline u16
fletcher16_compute(struct fletcher16_context *ctx)
{
  return (ctx->c0 << 8) | ctx->c1;
}

#endif
