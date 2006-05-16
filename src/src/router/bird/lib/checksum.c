/*
 *	BIRD Library -- IP One-Complement Checksum
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Miscellaneous functions.
 */

#include <stdarg.h>

#include "nest/bird.h"
#include "checksum.h"

static u16				/* One-complement addition */
add16(u16 sum, u16 x)
{
  u16 z = sum + x;
  return z + (z < sum);
}

static u32
add32(u32 sum, u32 x)
{
  u32 z = sum + x;
  return z + (z < sum);
}

static u16
ipsum_calc_block(u16 *x, unsigned len, u16 sum)
{
  int rest;
  u32 tmp, *xx;

  /*
   *  A few simple facts about the IP checksum (see RFC 1071 for detailed
   *  discussion):
   *
   *	o  It's associative and commutative.
   *	o  It's byte order independent.
   *	o  It's word size independent.
   *
   *  This gives us a neat 32-bits-at-a-time algorithm which respects
   *  usual alignment requirements and is reasonably fast.
   */

  ASSERT(!(len % 2));
  if (!len)
    return sum;
  len >>= 1;
  if ((unsigned long) x & 2)		/* Align to 32-bit boundary */
    {
      sum = add16(sum, *x++);
      len--;
    }
  rest = len & 1;
  len >>= 1;
  tmp = 0;
  xx = (u32 *) x;
  while (len)
    {
      tmp = add32(tmp, *xx++);
      len--;
    }
  sum = add16(sum, add16(tmp & 0xffff, tmp >> 16U));
  if (rest)
    sum = add16(sum, *(u16 *) xx);
  return sum;
}

static u16
ipsum_calc(void *frag, unsigned len, va_list args)
{
  u16 sum = 0;

  for(;;)
    {
      sum = ipsum_calc_block(frag, len, sum);
      frag = va_arg(args, void *);
      if (!frag)
	break;
      len = va_arg(args, unsigned);
    }
  return sum;
}

/**
 * ipsum_verify - verify an IP checksum
 * @frag: first packet fragment
 * @len: length in bytes
 *
 * This function verifies whether a given fragmented packet
 * has correct one's complement checksum as used by the IP
 * protocol.
 *
 * It uses all the clever tricks described in RFC 1071 to speed
 * up checksum calculation as much as possible.
 *
 * Result: 1 if the checksum is correct, 0 else.
 */
int
ipsum_verify(void *frag, unsigned len, ...)
{
  va_list args;
  u16 sum;

  va_start(args, len);
  sum = ipsum_calc(frag, len, args);
  va_end(args);
  return sum == 0xffff;
}

/**
 * ipsum_calculate - compute an IP checksum
 * @frag: first packet fragment
 * @len: length in bytes
 *
 * This function calculates a one's complement checksum of a given fragmented
 * packet.
 *
 * It uses all the clever tricks described in RFC 1071 to speed
 * up checksum calculation as much as possible.
 */
u16
ipsum_calculate(void *frag, unsigned len, ...)
{
  va_list args;
  u16 sum;

  va_start(args, len);
  sum = ipsum_calc(frag, len, args);
  va_end(args);
  return 0xffff - sum;
}
