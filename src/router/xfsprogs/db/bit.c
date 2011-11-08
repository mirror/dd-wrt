/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <xfs/libxfs.h>
#include "bit.h"

#undef setbit	/* defined in param.h on Linux */

static int	getbit(char *ptr, int bit);
static void	setbit(char *ptr, int bit, int val);

static int
getbit(
	char	*ptr,
	int	bit)
{
	int	mask;
	int	shift;

	ptr += byteize(bit);
	bit = bitoffs(bit);
	shift = 7 - bit;
	mask = 1 << shift;
	return (*ptr & mask) >> shift;
}

static void
setbit(
	char *ptr,
	int  bit,
	int  val)
{
	int	mask;
	int	shift;

	ptr += byteize(bit);
	bit = bitoffs(bit);
	shift = 7 - bit;
	mask = (1 << shift);
	if (val) {
		*ptr |= mask;
	} else {
		mask = ~mask;
		*ptr &= mask;
	}
}

__int64_t
getbitval(
	void		*obj,
	int		bitoff,
	int		nbits,
	int		flags)
{
	int		bit;
	int		i;
	char		*p;
	__int64_t	rval;
	int		signext;
	int		z1, z2, z3, z4;

	ASSERT(nbits<=64);

	p = (char *)obj + byteize(bitoff);
	bit = bitoffs(bitoff);
	signext = (flags & BVSIGNED) != 0;
	z4 = ((__psint_t)p & 0xf) == 0 && bit == 0;
	if (nbits == 64 && z4)
		return be64_to_cpu(*(__be64 *)p);
	z3 = ((__psint_t)p & 0x7) == 0 && bit == 0;
	if (nbits == 32 && z3) {
		if (signext)
			return (__s32)be32_to_cpu(*(__be32 *)p);
		else
			return (__u32)be32_to_cpu(*(__be32 *)p);
	}
	z2 = ((__psint_t)p & 0x3) == 0 && bit == 0;
	if (nbits == 16 && z2) {
		if (signext)
			return (__s16)be16_to_cpu(*(__be16 *)p);
		else
			return (__u16)be16_to_cpu(*(__be16 *)p);
	}
	z1 = ((__psint_t)p & 0x1) == 0 && bit == 0;
	if (nbits == 8 && z1) {
		if (signext)
			return *(__s8 *)p;
		else
			return *(__u8 *)p;
	}


	for (i = 0, rval = 0LL; i < nbits; i++) {
		if (getbit(p, bit + i)) {
			/* If the last bit is on and we care about sign
			 * bits and we don't have a full 64 bit
			 * container, turn all bits on between the
			 * sign bit and the most sig bit.
			 */

			/* handle endian swap here */
#if __BYTE_ORDER == LITTLE_ENDIAN
			if (i == 0 && signext && nbits < 64)
				rval = -1LL << nbits;
			rval |= 1LL << (nbits - i - 1);
#else
			if ((i == (nbits - 1)) && signext && nbits < 64)
				rval |= (-1LL << nbits);
			rval |= 1LL << (nbits - i - 1);
#endif
		}
	}
	return rval;
}

void
setbitval(
	void *obuf,      /* buffer to write into */
	int bitoff,      /* bit offset of where to write */
	int nbits,       /* number of bits to write */
	void *ibuf)      /* source bits */
{
	char    *in           = (char *)ibuf;
	char    *out          = (char *)obuf;

	int     bit;

#if BYTE_ORDER == LITTLE_ENDIAN
	int     big           = 0;
#else
	int     big           = 1;
#endif

	/* only need to swap LE integers */
	if (big || (nbits!=16 && nbits!=32 && nbits!=64) ) {
		/* We don't have type info, so we can only assume
		 * that 2,4 & 8 byte values are integers. sigh.
		 */

		/* byte aligned ? */
		if (bitoff%NBBY) {
			/* no - bit copy */
			for (bit=0; bit<nbits; bit++)
				setbit(out, bit+bitoff, getbit(in, bit));
		} else {
			/* yes - byte copy */
			memcpy(out+byteize(bitoff), in, byteize(nbits));
		}

	} else {
		int     ibit;
		int     obit;

		/* we need to endian swap this value */

		out+=byteize(bitoff);
		obit=bitoffs(bitoff);

		ibit=nbits-NBBY;

		for (bit=0; bit<nbits; bit++) {
			setbit(out, bit+obit, getbit(in, ibit));
			if (ibit%NBBY==NBBY-1)
				ibit-=NBBY*2-1;
			else
				ibit++;
		}
	}
}
