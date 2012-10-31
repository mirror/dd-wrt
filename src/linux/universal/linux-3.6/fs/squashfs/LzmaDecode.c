/*
 * LzmaDecode.c - Small LZMA Wrapper for Kernel LZMA Decoder
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <linux/vmalloc.h>

#include "LzmaDec.h"
#include "LzmaDecode.h"

#ifndef LZMA_BASE_SIZE 
#define LZMA_BASE_SIZE 1846
#endif
#ifndef LZMA_LIT_SIZE
#define LZMA_LIT_SIZE 768
#endif

/* default LZMA settings, should be in sync with mksquashfs */
#define LZMA_LC 3
#define LZMA_LP 3
#define LZMA_PB 2

#define LZMA_WORKSPACE_SIZE ((LZMA_BASE_SIZE + \
      (LZMA_LIT_SIZE << (LZMA_LC + LZMA_LP))) * sizeof(CLzmaProb))

static unsigned char lzma_workspace[LZMA_WORKSPACE_SIZE];

static void *SzAlloc(void *p, size_t size) { p = p; return lzma_workspace; }
static void SzFree(void *p, void *address) { p = p; }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

int LzmaUncompress(unsigned char *dest, unsigned int *destLen, const unsigned char *src, unsigned int  *srcLen,
  const unsigned char *props, size_t propsSize)
{
  ELzmaStatus status;
  return LzmaDecode(dest, destLen, src, srcLen, props, (unsigned)propsSize, LZMA_FINISH_ANY, &status, &g_Alloc);
}

int LzmaDecode2(int lc, int lp, int pb,
		unsigned char *inStream, unsigned int inSize,
		unsigned char *outStream, unsigned int outSize,
		unsigned int * outSizeProcessed)
{
	unsigned int dictSize = 1 << 23;
	int i;
	unsigned char props[LZMA_PROPS_SIZE];
	props[0] = (unsigned char)((pb * 5 + lp) * 9 + lc);
	for (i = 0; i < 4; i++)
		props[1 + i] = (unsigned char)(dictSize >> (8 * i));
	return LzmaUncompress(outStream, &outSize, inStream, &inSize, &props[0],
		       LZMA_PROPS_SIZE);
}
