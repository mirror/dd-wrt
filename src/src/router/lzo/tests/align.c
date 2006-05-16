/* align.c -- test alignment (important for 16-bit systems)

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


#include <lzoconf.h>
#include <lzoutil.h>
#if 1
#include "lzo_conf.h"
#include "lzo_ptr.h"
#endif
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NDEBUG
#undef NDEBUG
#include <assert.h>


int opt_verbose = 0;


/*************************************************************************
//
**************************************************************************/

long align_test(lzo_byte *block, lzo_uint len, lzo_uint step)
{
	lzo_byte *b1 = block;
	lzo_byte *b2 = block;
	lzo_byte *k1 = NULL;
	lzo_byte *k2 = NULL;
	lzo_byte *k;
	lzo_byte *x;
	lzo_uint offset = 0;
	long i = 0;

	assert(step > 0);
	assert(step <= 65536L);
	assert((step & (step - 1)) == 0);

	for (offset = step; offset < len; offset += step)
	{
		k1 = LZO_PTR_ALIGN_UP(b1+1,step);
		k2 = b2 + offset;
		if (k1 != k2)
		{
			printf("error 1: i %ld step %ld offset %ld: "
					"%p (%ld) %p (%ld)\n",
				    i, (long) step, (long) offset,
			        k1, (long) (k1 - block),
			        k2, (long) (k2 - block));
			return 0;
		}
		if (k1 - step != b1)
		{
			printf("error 2: i %ld step %ld offset %ld: "
					"%p (%ld) %p (%ld)\n",
				    i, (long) step, (long) offset,
			        b1, (long) (b1 - block),
			        k1, (long) (k1 - block));
			return 0;
		}

		assert(k1 > b1);
		assert(k2 > b2);
		assert((lzo_uint)(k2 - b2) == offset);
		assert(k1 - offset == b2);
#if defined(PTR_ALIGNED_4)
		if (step == 4)
		{
			assert(PTR_ALIGNED_4(k1));
			assert(PTR_ALIGNED_4(k2));
			assert(PTR_ALIGNED2_4(k1,k2));
		}
#endif
#if defined(PTR_ALIGNED_8)
		if (step == 8)
		{
			assert(PTR_ALIGNED_8(k1));
			assert(PTR_ALIGNED_8(k2));
			assert(PTR_ALIGNED2_8(k1,k2));
		}
#endif
#if defined(PTR_LINEAR)
		assert((PTR_LINEAR(k1) & (step-1)) == 0);
		assert((PTR_LINEAR(k2) & (step-1)) == 0);
#endif

		for (k = b1 + 1; k <= k1; k++)
		{
			x = LZO_PTR_ALIGN_UP(k,step);
			if (x != k1)
			{
				printf("error 3: base: %p %p %p  i %ld step %ld offset %ld: "
						"%p (%ld) %p (%ld) %p (%ld)\n",
					    block, b1, b2,
					    i, (long) step, (long) offset,
				        k1, (long) (k1 - block),
				        k, (long) (k - block),
				        x, (long) (x - block));
				return 0;
			}
		}

		b1 = k1;
		i++;
	}

	return i;
}


/*************************************************************************
//
**************************************************************************/

#define BLOCK_LEN	(128*1024L)

static lzo_byte _block[ 2*BLOCK_LEN + 256 ];

int main(int argc, char *argv[])
{
	lzo_uint step;

	if (argc >= 2 && strcmp(argv[1],"-v") == 0)
		opt_verbose = 1;

	if (lzo_init() != LZO_E_OK)
	{
		printf("lzo_init() failed !!!\n");
		return 2;
	}

	printf("Align init: %p ( 0x%lx )\n", _block, (unsigned long) _block);

	for (step = 1; step <= 65536L; step *= 2)
	{
		lzo_byte *block = _block;
		long n;
		unsigned gap;

		block = LZO_PTR_ALIGN_UP(block,step);
		gap = __lzo_align_gap(block,step);
		if (opt_verbose >= 1)
			printf("STEP %5ld: GAP: %5lu  %p %p %5ld\n",
			        (long) step, (long) gap, _block, block,
			        (long) (block - _block));
		n = align_test(block,BLOCK_LEN,step);
		if (n == 0)
			return 1;
		if ((n + 1) * step != BLOCK_LEN)
		{
			printf("error 4: %ld %ld\n",(long)step,n);
			return 1;
		}
	}

	printf("Alignment test passed.\n");
	return 0;
}


/*
vi:ts=4
*/

