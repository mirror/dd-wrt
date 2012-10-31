/* LzmaLib.c -- LZMA library wrapper
2008-08-05
Igor Pavlov
Public domain */

#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"
#include "LzmaLib.h"

static void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

static void lzma_free_workspace(CLzmaEncHandle *p)
{
	LzmaEnc_Destroy(p, &g_Alloc, &g_Alloc);
}

static int lzma_alloc_workspace(CLzmaEncProps *props,CLzmaEncHandle **p)
{
	Byte propsEncoded[LZMA_PROPS_SIZE];
	SizeT propsSize = sizeof(propsEncoded);
	if ((*p = (CLzmaEncHandle *)LzmaEnc_Create(&g_Alloc)) == NULL)
	{
		printf("Failed to allocate lzma deflate workspace\n");
		return -1;
	}

	if (LzmaEnc_SetProps(*p, props) != SZ_OK)
	{
		printf("setprops failed\n");
		lzma_free_workspace(*p);
		return -1;
	}
	
	if (LzmaEnc_WriteProperties(*p, propsEncoded, &propsSize) != SZ_OK)
	{
		printf("writeproperties failed\n");
		lzma_free_workspace(*p);
		return -1;
	}
	
        return 0;
}


MY_STDAPI LzmaCompress(unsigned char *dest, size_t  *destLen, const unsigned char *src, size_t  srcLen,
  unsigned char *outProps, size_t *outPropsSize,
  int level, /* 0 <= level <= 9, default = 5 */
  unsigned dictSize, /* use (1 << N) or (3 << N). 4 KB < dictSize <= 128 MB */
  int lc, /* 0 <= lc <= 8, default = 3  */
  int lp, /* 0 <= lp <= 4, default = 0  */
  int pb, /* 0 <= pb <= 4, default = 2  */
  int fb,  /* 5 <= fb <= 273, default = 32 */
  int numThreads /* 1 or 2, default = 2 */
)
{
	CLzmaEncHandle *p;
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);

        props.dictSize = dictSize;
        props.level = 9;
        props.lc = lc;
        props.lp = lp;
        props.pb = pb;
        props.fb = fb;
	int r = lzma_alloc_workspace(&props,&p);
        if (r < 0)
                return;
	int ret = LzmaEnc_MemEncode(p, dest, destLen, src, srcLen,
		1, NULL, &g_Alloc, &g_Alloc);
	lzma_free_workspace(p);
}


MY_STDAPI LzmaUncompress(unsigned char *dest, size_t  *destLen, const unsigned char *src, size_t  *srcLen,
  const unsigned char *props, size_t propsSize)
{
  ELzmaStatus status;
  return LzmaDecode(dest, destLen, src, srcLen, props, (unsigned)propsSize, LZMA_FINISH_ANY, &status, &g_Alloc);
}
