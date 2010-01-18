/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2008 Nikos Mavrogiannopoulos.
 *
 * compr_lzma is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *               
 * compr_lzma is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *                               
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/mtd/compatmac.h> /* for min() */
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/jffs2.h>
#include <linux/zlib.h>
#include "nodelist.h"
#include "lzma/LzmaDec.h"
#include "lzma/LzmaEnc.h"

#ifndef JFFS2_COMPR_LZMA
# define JFFS2_COMPR_LZMA 0x15
#endif

static void* my_alloc( void* p, size_t size)
{
//printk(KERN_EMERG "alloc %d\n",size);
  return vmalloc(size);
}

static void my_free( void* p, void* addr)
{
  vfree(addr);
}

static void* my_bigalloc( void* p, size_t size)
{
//printk(KERN_EMERG "bigalloc %d\n",size);
  return vmalloc(size);
}

static void my_bigfree( void* p, void* addr)
{
  vfree(addr);
}

const CLzmaEncProps lzma_props = {
  .level = 1, /* unused */
  .lc = 3,
  .lp = 0,
  .pb = 2,
  .algo = 1,
  .fb = 32,
  .btMode = 1,
  .mc = 32,
  .writeEndMark = 0,
  .numThreads = 1,
  .numHashBytes = 4,
  /* something more than this doesn't seem to offer anything
   * practical and increases memory size.
   */
  .dictSize = 1 << 13,
};

ISzAlloc xalloc = { my_alloc, my_free };
ISzAlloc xbigalloc = { my_bigalloc, my_bigfree };

int lzma_compress(unsigned char *data_in, unsigned char *cpage_out,
		uint32_t *sourcelen, uint32_t *dstlen)
{
	int ret;
//	printk(KERN_EMERG "compress lzma %d destlen %d\n",*sourcelen,*dstlen);
	size_t props_size = LZMA_PROPS_SIZE;
	size_t data_out_size = (*dstlen)-LZMA_PROPS_SIZE;

	if (*dstlen < LZMA_PROPS_SIZE)
		return -1;

        ret = LzmaEncode( cpage_out+LZMA_PROPS_SIZE, &data_out_size, 
        	data_in, *sourcelen, &lzma_props, 
        	cpage_out, &props_size, 0, NULL, &xalloc, &xbigalloc);
        	
	if (ret != SZ_OK)
		{
		return -1;
		}

	*dstlen = data_out_size + LZMA_PROPS_SIZE;
	
	return 0;
}

int lzma_decompress(unsigned char *data_in, unsigned char *cpage_out,
		uint32_t srclen, uint32_t destlen)
{
	int ret;
	size_t data_out_size = destlen;
	size_t data_in_size = srclen - LZMA_PROPS_SIZE;
	ELzmaStatus status;
	
	if (srclen < LZMA_PROPS_SIZE)
		return -1;
	
	ret = LzmaDecode( cpage_out, &data_out_size, data_in+LZMA_PROPS_SIZE, &data_in_size, 
		data_in, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &xalloc);
	
	if (ret != SZ_OK)
		return -1;

	return 0;
}

