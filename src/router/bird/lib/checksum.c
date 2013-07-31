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

static inline u32
add32(u32 sum, u32 x)
{
  u32 z = sum + x;
//  return z + (z < sum);

 /* add carry */
  if (z < x)
    z++;
  return z;
}

static u16
ipsum_calc_block(u32 *buf, unsigned len, u16 isum)
{
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

  ASSERT(!(len % 4));
  if (!len)
    return isum;

  u32 *end = buf + (len >> 2);
  u32 sum = isum;
  while (buf < end)
    sum = add32(sum, *buf++);

  sum = (sum >> 16) + (sum & 0xffff);    /* add high-16 to low-16 */
  sum += (sum >> 16); /* add carry */
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
