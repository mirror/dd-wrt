/* simple.c -- the annotated simple example program for the LZO library

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


/*************************************************************************
// This program shows the basic usage of the LZO library.
// We will compress a block of data and decompress again.
//
// See also LZO.FAQ
**************************************************************************/

/* We will be using the LZO1X-1 algorithm, so we have
 * to include <lzo1x.h>
 */

#include <lzo1x.h>


/* We want to compress the data block at `in' with length `IN_LEN' to
 * the block at `out'. Because the input block may be incompressible,
 * we must provide a little more output space in case that compression
 * is not possible.
 */

#ifndef IN_LEN
#define IN_LEN		(128*1024L)
#endif
#define OUT_LEN		(IN_LEN + IN_LEN / 64 + 16 + 3)


/*************************************************************************
//
**************************************************************************/

#include "lutil.h"

int main(int argc, char *argv[])
{
	int r;
	lzo_byte *in;
	lzo_byte *out;
	lzo_byte *wrkmem;
	lzo_uint in_len;
	lzo_uint out_len;
	lzo_uint new_len;

#if defined(__EMX__)
	_response(&argc,&argv);
	_wildcard(&argc,&argv);
#endif

	if (argc < 0 && argv == NULL)	/* avoid warning about unused args */
		return 0;

	printf("\nLZO real-time data compression library (v%s, %s).\n",
	        lzo_version_string(), lzo_version_date());
	printf("Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer\n\n");

/*
 * Step 1: initialize the LZO library
 */
	if (lzo_init() != LZO_E_OK)
	{
		printf("lzo_init() failed !!!\n");
		return 4;
	}

/*
 * Step 2: allocate blocks and the work-memory
 */
	in = (lzo_bytep) lzo_malloc(IN_LEN);
	out = (lzo_bytep) lzo_malloc(OUT_LEN);
	wrkmem = (lzo_bytep) lzo_malloc(LZO1X_1_MEM_COMPRESS);
	if (in == NULL || out == NULL || wrkmem == NULL)
	{
		printf("out of memory\n");
		return 3;
	}

/*
 * Step 3: prepare the input block that will get compressed.
 *         We just fill it with zeros in this example program,
 *         but you would use your real-world data here.
 */
	in_len = IN_LEN;
	lzo_memset(in,0,in_len);

/*
 * Step 4: compress from `in' to `out' with LZO1X-1
 */
	r = lzo1x_1_compress(in,in_len,out,&out_len,wrkmem);
	if (r == LZO_E_OK)
		printf("compressed %lu bytes into %lu bytes\n",
			(long) in_len, (long) out_len);
	else
	{
		/* this should NEVER happen */
		printf("internal error - compression failed: %d\n", r);
		return 2;
	}
	/* check for an incompressible block */
	if (out_len >= in_len)
	{
		printf("This block contains incompressible data.\n");
		return 0;
	}

/*
 * Step 5: decompress again, now going from `out' to `in'
 */
	r = lzo1x_decompress(out,out_len,in,&new_len,NULL);
	if (r == LZO_E_OK && new_len == in_len)
		printf("decompressed %lu bytes back into %lu bytes\n",
			(long) out_len, (long) in_len);
	else
	{
		/* this should NEVER happen */
		printf("internal error - decompression failed: %d\n", r);
		return 1;
	}

	lzo_free(wrkmem);
	lzo_free(out);
	lzo_free(in);
	printf("Simple compression test passed.\n");
	return 0;
}

/*
vi:ts=4
*/

