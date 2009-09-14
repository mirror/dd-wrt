/*
 *	BIRD Library -- Generic Bit Operations
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "bitops.h"

/**
 * u32_mkmask - create a bit mask
 * @n: number of bits
 *
 * u32_mkmask() returns an unsigned 32-bit integer which binary
 * representation consists of @n ones followed by zeroes.
 */
u32
u32_mkmask(unsigned n)
{
  return n ? ~((1 << (32 - n)) - 1) : 0;
}

/**
 * u32_masklen - calculate length of a bit mask
 * @x: bit mask
 *
 * This function checks whether the given integer @x represents
 * a valid bit mask (binary representation contains first ones, then
 * zeroes) and returns the number of ones or -1 if the mask is invalid.
 */
int
u32_masklen(u32 x)
{
  int l = 0;
  u32 n = ~x;

  if (n & (n+1)) return -1;
  if (x & 0x0000ffff) { x &= 0x0000ffff; l += 16; }
  if (x & 0x00ff00ff) { x &= 0x00ff00ff; l += 8; }
  if (x & 0x0f0f0f0f) { x &= 0x0f0f0f0f; l += 4; }
  if (x & 0x33333333) { x &= 0x33333333; l += 2; }
  if (x & 0x55555555) l++;
  if (x & 0xaaaaaaaa) l++;
  return l;
}

/**
 * u32_log2 - compute a binary logarithm.
 * @v: number
 *
 * This function computes a integral part of binary logarithm of given
 * integer @v and returns it. The computed value is also an index of the
 * most significant non-zero bit position.
 */

u32
u32_log2(u32 v)
{
  /* The code from http://www-graphics.stanford.edu/~seander/bithacks.html */
  u32 r, shift;
  r =     (v > 0xFFFF) << 4; v >>= r;
  shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
  shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
  shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
  r |= (v >> 1);
  return r;
}

