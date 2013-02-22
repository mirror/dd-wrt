/*
 * Copyright (C) 2008 Nokia Corporation.
 * Copyright (C) 2008 University of Szeged, Hungary
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: Artem Bityutskiy
 *          Adrian Hunter
 *          Zoltan Sogor
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/types.h>

#define crc32 __zlib_crc32
#include <zlib.h>
#undef crc32

#include "compr.h"
#include "ubifs-media.h"
#include "mkfs.ubifs.h"

static unsigned long long errcnt = 0;
static struct ubifs_info *c = &info_;

#define DEFLATE_DEF_LEVEL     Z_DEFAULT_COMPRESSION
#define DEFLATE_DEF_WINBITS   11
#define DEFLATE_DEF_MEMLEVEL  8

static int zlib_deflate(void *in_buf, size_t in_len, void *out_buf,
			size_t *out_len)
{
	z_stream strm;

	strm.zalloc = NULL;
	strm.zfree = NULL;

	/*
	 * Match exactly the zlib parameters used by the Linux kernel crypto
	 * API.
	 */
        if (deflateInit2(&strm, DEFLATE_DEF_LEVEL, Z_DEFLATED,
			 -DEFLATE_DEF_WINBITS, DEFLATE_DEF_MEMLEVEL,
			 Z_DEFAULT_STRATEGY)) {
		errcnt += 1;
		return -1;
	}

	strm.next_in = in_buf;
	strm.avail_in = in_len;
	strm.total_in = 0;

	strm.next_out = out_buf;
	strm.avail_out = *out_len;
	strm.total_out = 0;

	if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
		deflateEnd(&strm);
		errcnt += 1;
		return -1;
	}

	if (deflateEnd(&strm) != Z_OK) {
		errcnt += 1;
		return -1;
	}

	*out_len = strm.total_out;

	return 0;
}

#ifndef WITHOUT_LZO
#include <lzo/lzo1x.h>

static void *lzo_mem;

static int lzo_init(void)
{
	lzo_mem = malloc(LZO1X_999_MEM_COMPRESS);
	if (!lzo_mem)
		return -1;

	return 0;
}

static void lzo_fini(void)
{
	free(lzo_mem);
}

static int lzo_compress(void *in_buf, size_t in_len, void *out_buf,
			size_t *out_len)
{
	lzo_uint len;
	int ret;

	len = *out_len;
	ret = lzo1x_999_compress(in_buf, in_len, out_buf, &len, lzo_mem);
	*out_len = len;

	if (ret != LZO_E_OK) {
		errcnt += 1;
		return -1;
	}

	return 0;
}
#else
static inline int lzo_compress(void *in_buf, size_t in_len, void *out_buf,
			       size_t *out_len) { return -1; }
static inline int lzo_init(void) { return 0; }
static inline void lzo_fini(void) { }
#endif

#ifndef WITHOUT_XZ

#include <lzma.h>

struct xz_ctx {
	lzma_filter	filters[3];
	lzma_options_lzma opts;
};

static struct xz_ctx *xz_ctx;

#define LZMA_COMPRESSION_LEVEL	9

static struct xz_ctx *xz_ctx_init(void)
{
	struct xz_ctx *ctx;
	lzma_options_lzma *opts_lzma;
	uint32_t preset;
	int ret;

	ctx = malloc(sizeof(struct xz_ctx));
	if (ctx == NULL)
		goto err;

	memset(ctx, 0, sizeof(struct xz_ctx));

	opts_lzma = &ctx->opts;

	preset = LZMA_COMPRESSION_LEVEL | LZMA_PRESET_EXTREME;
	ret = lzma_lzma_preset(opts_lzma, preset);
	if (ret)
		goto err_free_ctx;

	/* TODO: allow to specify LZMA options via command line */
#if 0
	opts_lzma->lc = 3;
	opts_lzma->lp = 0;
	opts_lzma->pb = 2;
	opts_lzma->nice_len = 64;
#else
	opts_lzma->lc = 0;
	opts_lzma->lp = 2;
	opts_lzma->pb = 2;
	opts_lzma->nice_len = 64;
#endif

	ctx->filters[0].id = LZMA_FILTER_LZMA2;
	ctx->filters[0].options = opts_lzma;
	ctx->filters[1].id = LZMA_VLI_UNKNOWN;

	return ctx;

err_free_ctx:
	free(ctx);
err:
	return NULL;
}

static void xz_ctx_free(struct xz_ctx *ctx)
{
	free(ctx);
}

static int xz_init(void)
{
	xz_ctx = xz_ctx_init();
	if (xz_ctx == NULL)
		return -1;

	return 0;
}

static void xz_fini(void)
{
	xz_ctx_free(xz_ctx);
}

static int xz_compress(void *in_buf, size_t in_len, void *out_buf,
		       size_t *out_len)
{
	size_t ret_len;
	lzma_ret ret_xz;
	int ret;

	ret = -1;

	ret_len = 0;
	ret_xz = lzma_stream_buffer_encode(xz_ctx->filters, LZMA_CHECK_CRC32,
					   NULL, in_buf, in_len, out_buf,
					   &ret_len, *out_len);
	if (ret_xz != LZMA_OK) {
		fprintf(stderr, "XZ error: %d\n", (int) ret_xz);
		goto out;
	}

	*out_len = ret_len;

	ret = 0;
out:
	return ret;
}
#else
static inline int xz_init(void) { return 0; }
static inline void xz_fini(void) { }
static inline int xz_compress(void *in_buf, size_t in_len, void *out_buf,
			      size_t *out_len) { return -1; }
#endif

static int no_compress(void *in_buf, size_t in_len, void *out_buf,
		       size_t *out_len)
{
	memcpy(out_buf, in_buf, in_len);
	*out_len = in_len;
	return 0;
}

static char *zlib_buf;

static int favor_lzo_compress(void *in_buf, size_t in_len, void *out_buf,
			       size_t *out_len, int *type)
{
	int lzo_ret, zlib_ret;
	size_t lzo_len, zlib_len;

	lzo_len = zlib_len = *out_len;
	lzo_ret = lzo_compress(in_buf, in_len, out_buf, &lzo_len);
	zlib_ret = zlib_deflate(in_buf, in_len, zlib_buf, &zlib_len);
	if (lzo_ret && zlib_ret)
		/* Both compressors failed */
		return -1;

	if (!lzo_ret && !zlib_ret) {
		double percent;

		/* Both compressors succeeded */
		if (lzo_len <= zlib_len )
			goto select_lzo;

		percent = (double)zlib_len / (double)lzo_len;
		percent *= 100;
		if (percent > 100 - c->favor_percent)
			goto select_lzo;
		goto select_zlib;
	}

	if (lzo_ret)
		/* Only zlib compressor succeeded */
		goto select_zlib;

	/* Only LZO compressor succeeded */

select_lzo:
	*out_len = lzo_len;
	*type = MKFS_UBIFS_COMPR_LZO;
	return 0;

select_zlib:
	*out_len = zlib_len;
	*type = MKFS_UBIFS_COMPR_ZLIB;
	memcpy(out_buf, zlib_buf, zlib_len);
	return 0;
}

int compress_data(void *in_buf, size_t in_len, void *out_buf, size_t *out_len,
		  int type)
{
	int ret;

	if (in_len < UBIFS_MIN_COMPR_LEN) {
		no_compress(in_buf, in_len, out_buf, out_len);
		return MKFS_UBIFS_COMPR_NONE;
	}

	if (c->favor_lzo)
		ret = favor_lzo_compress(in_buf, in_len, out_buf, out_len, &type);
	else {
		switch (type) {
		case MKFS_UBIFS_COMPR_LZO:
			ret = lzo_compress(in_buf, in_len, out_buf, out_len);
			break;
		case MKFS_UBIFS_COMPR_XZ:
			ret = xz_compress(in_buf, in_len, out_buf, out_len);
			break;
		case MKFS_UBIFS_COMPR_ZLIB:
			ret = zlib_deflate(in_buf, in_len, out_buf, out_len);
			break;
		case MKFS_UBIFS_COMPR_NONE:
			ret = 1;
			break;
		default:
			errcnt += 1;
			ret = 1;
			break;
		}
	}
	if (ret || *out_len >= in_len) {
		no_compress(in_buf, in_len, out_buf, out_len);
		return MKFS_UBIFS_COMPR_NONE;
	}
	return type;
}

int init_compression(void)
{
	int ret;

	ret = lzo_init();
	if (ret)
		goto err;

	ret = xz_init();
	if (ret)
		goto err_lzo;

	zlib_buf = malloc(UBIFS_BLOCK_SIZE * WORST_COMPR_FACTOR);
	if (!zlib_buf)
		goto err_xz;

	return 0;

err_xz:
	xz_fini();
err_lzo:
	lzo_fini();
err:
	return ret;
}

void destroy_compression(void)
{
	free(zlib_buf);
	xz_fini();
	lzo_fini();
	if (errcnt)
		fprintf(stderr, "%llu compression errors occurred\n", errcnt);
}
