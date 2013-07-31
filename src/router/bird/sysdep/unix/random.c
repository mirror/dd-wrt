/*
 *	BIRD Internet Routing Daemon -- Random Numbers
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdlib.h>

#include "nest/bird.h"

u32
random_u32(void)
{
  long int rand_low, rand_high;

  rand_low = random();
  rand_high = random();
  return (rand_low & 0xffff) | ((rand_high & 0xffff) << 16);
}
