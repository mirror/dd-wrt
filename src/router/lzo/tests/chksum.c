/* chksum.c -- compute a checksum

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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>


/*************************************************************************
//
**************************************************************************/

static lzo_byte block[128*1024L];


int main(int argc, char *argv[])
{
	lzo_uint block_len;
	lzo_uint32 adler, crc;

	if (argc < 0 && argv == NULL)	/* avoid warning about unused args */
		return 0;

	if (lzo_init() != LZO_E_OK)
	{
		printf("lzo_init() failed !!!\n");
		return 3;
	}

/* prepare the block */
	block_len = sizeof(block);
	lzo_memset(block, 0, block_len);

/* adler32 checksum */
	adler = lzo_adler32(0, NULL, 0);
	adler = lzo_adler32(adler, block, block_len);
	if (adler != 0x001e0001UL)
	{
		printf("adler32 checksum error !!! (0x%08lx)\n", (long) adler);
		return 2;
	}

/* crc32 checksum */
	crc = lzo_crc32(0, NULL, 0);
	crc = lzo_crc32(crc, block, block_len);
	if (crc != 0x7ee8cdcdUL)
	{
		printf("crc32 checksum error !!! (0x%08lx)\n", (long) crc);
		return 1;
	}

	printf("Checksum test passed.\n");
	return 0;
}


/*
vi:ts=4
*/

