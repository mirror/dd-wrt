/*
  LzmaDecode.c
  LZMA Decoder

  LZMA SDK 4.05 Copyright (c) 1999-2004 Igor Pavlov (2004-08-25)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to
  statically or dynamically link your code (or bind by name) to the
  interfaces of this file without subjecting your linked code to the
  terms of the CPL or GNU LGPL. Any modifications or additions
  to this file, however, are subject to the LGPL or CPL terms.
*/

#include <linux/vmalloc.h>

#include "LzmaDec.h"
#include "LzmaDecode.h"


static void *SzAlloc(void *p, size_t size) { p = p; return vmalloc(size); }
static void SzFree(void *p, void *address) { p = p; vfree(address); }
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
