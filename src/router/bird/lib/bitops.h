/*
 *	BIRD Library -- Generic Bit Operations
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BITOPTS_H_
#define _BIRD_BITOPTS_H_

/*
 *	Bit mask operations:
 *
 *	u32_mkmask	Make bit mask consisting of <n> consecutive ones
 *			from the left and the rest filled with zeroes.
 *			E.g., u32_mkmask(5) = 0xf8000000.
 *	u32_masklen	Inverse operation to u32_mkmask, -1 if not a bitmask.
 */

u32 u32_mkmask(unsigned n);
int u32_masklen(u32 x);

u32 u32_log2(u32 v);

static inline u32 u32_hash(u32 v) { return v * 2902958171u; }

#endif

