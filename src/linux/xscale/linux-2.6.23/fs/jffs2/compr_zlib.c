/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright © 2001-2007 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 */

#if !defined(__KERNEL__) && !defined(__ECOS)
#error "The userspace support got too messy and was removed. Update your mkfs.jffs2"
#endif

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/zlib.h>
#include <linux/zutil.h>
#include "nodelist.h"
#include "compr.h"

	/* Plan: call deflate() with avail_in == *sourcelen,
		avail_out = *dstlen - 12 and flush == Z_FINISH.
		If it doesn't manage to finish,	call it again with
		avail_in == 0 and avail_out set to the remaining 12
		bytes for it to clean up.
	   Q: Is 12 bytes sufficient?
	*/
#define STREAM_END_SPACE 12


#ifdef __KERNEL__ /* Linux-only */
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/mutex.h>

#else
#define alloc_workspaces() (0)
#define free_workspaces() do { } while(0)
#endif /* __KERNEL__ */

int lzma_compress(unsigned char *data_in, unsigned char *cpage_out, __u32 *sourcelen, __u32 *dstlen);
void lzma_decompress(unsigned char *data_in, unsigned char *cpage_out, __u32 srclen, __u32 destlen);

static int jffs2_zlib_compress(unsigned char *data_in,
			       unsigned char *cpage_out,
			       uint32_t *sourcelen, uint32_t *dstlen,
			       void *model)
{
return lzma_compress(data_in,cpage_out,sourcelen,dstlen);
}

static int jffs2_zlib_decompress(unsigned char *data_in,
				 unsigned char *cpage_out,
				 uint32_t srclen, uint32_t destlen,
				 void *model)
{
lzma_decompress(data_in,cpage_out,srclen,destlen);
        return 0;
}

static struct jffs2_compressor jffs2_zlib_comp = {
    .priority = JFFS2_ZLIB_PRIORITY,
    .name = "zlib",
    .compr = JFFS2_COMPR_ZLIB,
    .compress = &jffs2_zlib_compress,
    .decompress = &jffs2_zlib_decompress,
#ifdef JFFS2_ZLIB_DISABLED
    .disabled = 1,
#else
    .disabled = 0,
#endif
};

int __init jffs2_zlib_init(void)
{
    int ret;


    ret = jffs2_register_compressor(&jffs2_zlib_comp);

    return ret;
}

void jffs2_zlib_exit(void)
{
    jffs2_unregister_compressor(&jffs2_zlib_comp);
}
