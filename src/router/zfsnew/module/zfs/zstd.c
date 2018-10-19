/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 2016-2018 by Klara Systems Inc.
 * Copyright (c) 2016-2018 Allan Jude <allanjude@freebsd.org>.
 */

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/zfs_context.h>
#include <sys/zio_compress.h>
#include <sys/spa.h>

#define	ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#include <zstd_errors.h>

#define	ZSTD_KMEM_MAGIC		0x20160831

/* for BSD compat */
#define	roundup2(x, m)	((((x) - 1) | ((m) - 1)) + 1)
#define	__unused			__attribute__((unused))
typedef int		 cmp_t(const void *, const void *);
static inline char	*med3(char *, char *, char *, cmp_t *, void *);
#ifndef MIN
#define	MIN(a, b)	((a) < (b) ? a : b)
#endif
static inline void
swapfunc(char *a, char *b, size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc(a, b, n)
#define	CMP(t, x, y) (cmp((x), (y)))

static inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk __unused)
{
	return CMP(thunk, a, b) < 0 ?
	    (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a))
	    :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c));
}

#define	thunk NULL
void
zstd_qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d1, d2;
	int cmp_result;
	int swap_cnt;

loop:
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm;
			    pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			    pl -= es)
				swapfunc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			size_t d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swapfunc(a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swapfunc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swapfunc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc(pb, pc, es);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm;
			    pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			    pl -= es)
				swapfunc(pl, pl - es, es);
		return;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	d1 = MIN(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			zstd_qsort(a, d1 / es, es, cmp);
		}
		if (d2 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	} else {
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es) {
			zstd_qsort(pn - d2, d2 / es, es, cmp);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}


static size_t real_zstd_compress(const char *source, char *dest, int isize,
    int osize, int level);
static size_t real_zstd_decompress(const char *source, char *dest, int isize,
    int maxosize);

void *zstd_alloc(void *opaque, size_t size);
void zstd_free(void *opaque, void *ptr);

static const ZSTD_customMem zstd_malloc = {
	zstd_alloc,
	zstd_free,
	NULL,
};

enum zstd_kmem_type {
	ZSTD_KMEM_UNKNOWN = 0,
	ZSTD_KMEM_CCTX,
	ZSTD_KMEM_WRKSPC_4K_MIN,
	ZSTD_KMEM_WRKSPC_4K_DEF,
	ZSTD_KMEM_WRKSPC_4K_MAX,
	ZSTD_KMEM_WRKSPC_16K_MIN,
	ZSTD_KMEM_WRKSPC_16K_DEF,
	ZSTD_KMEM_WRKSPC_16K_MAX,
	ZSTD_KMEM_WRKSPC_128K_MIN,
	ZSTD_KMEM_WRKSPC_128K_DEF,
	ZSTD_KMEM_WRKSPC_128K_MAX,
	/* SPA_MAXBLOCKSIZE */
	ZSTD_KMEM_WRKSPC_MBS_MIN,
	ZSTD_KMEM_WRKSPC_MBS_DEF,
	ZSTD_KMEM_WRKSPC_MBS_MAX,
	ZSTD_KMEM_DCTX,
	ZSTD_KMEM_COUNT,
};

struct zstd_kmem {
	uint_t			kmem_magic;
	enum zstd_kmem_type	kmem_type;
	size_t			kmem_size;
};

struct zstd_kmem_config {
	size_t			block_size;
	int			compress_level;
	char			*cache_name;
};

static kmem_cache_t *zstd_kmem_cache[ZSTD_KMEM_COUNT] = { NULL };
static struct zstd_kmem zstd_cache_size[ZSTD_KMEM_COUNT] = {
	{ ZSTD_KMEM_MAGIC, 0, 0 } };
static struct zstd_kmem_config zstd_cache_config[ZSTD_KMEM_COUNT] = {
	{ 0, 0, "zstd_unknown" },
	{ 0, 0, "zstd_cctx" },
	{ 4096, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_4k_min" },
	{ 4096, ZIO_ZSTD_LEVEL_DEFAULT, "zstd_wrkspc_4k_def" },
	{ 4096, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_4k_max" },
	{ 16384, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_16k_min" },
	{ 16384, ZIO_ZSTD_LEVEL_DEFAULT, "zstd_wrkspc_16k_def" },
	{ 16384, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_16k_max" },
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_128k_min" },
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_DEFAULT,
	    "zstd_wrkspc_128k_def" },
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_128k_max" },
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_mbs_min" },
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_DEFAULT, "zstd_wrkspc_mbs_def" },
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_mbs_max" },
	{ 0, 0, "zstd_dctx" },
};

static int
zstd_compare(const void *a, const void *b)
{
	struct zstd_kmem *x, *y;

	x = (struct zstd_kmem *)a;
	y = (struct zstd_kmem *)b;

	ASSERT(x->kmem_magic == ZSTD_KMEM_MAGIC);
	ASSERT(y->kmem_magic == ZSTD_KMEM_MAGIC);

	if (x->kmem_size > y->kmem_size) {
		return (1);
	} else if (x->kmem_size == y->kmem_size) {
		return (0);
	} else {
		return (-1);
	}
}

static enum zio_zstd_levels
zstd_level_to_enum(int level)
{
	enum zio_zstd_levels elevel = ZIO_ZSTDLVL_INHERIT;

	if (level > 0 && level <= ZIO_ZSTDLVL_MAX) {
		elevel = level;
		return (elevel);
	} else if (level < 0) {
		switch (level) {
			case -1:
				return (ZIO_ZSTDLVL_FAST_1);
			case -2:
				return (ZIO_ZSTDLVL_FAST_2);
			case -3:
				return (ZIO_ZSTDLVL_FAST_3);
			case -4:
				return (ZIO_ZSTDLVL_FAST_4);
			case -5:
				return (ZIO_ZSTDLVL_FAST_5);
			case -6:
				return (ZIO_ZSTDLVL_FAST_6);
			case -7:
				return (ZIO_ZSTDLVL_FAST_7);
			case -8:
				return (ZIO_ZSTDLVL_FAST_8);
			case -9:
				return (ZIO_ZSTDLVL_FAST_9);
			case -10:
				return (ZIO_ZSTDLVL_FAST_10);
			case -20:
				return (ZIO_ZSTDLVL_FAST_20);
			case -30:
				return (ZIO_ZSTDLVL_FAST_30);
			case -40:
				return (ZIO_ZSTDLVL_FAST_40);
			case -50:
				return (ZIO_ZSTDLVL_FAST_50);
			case -60:
				return (ZIO_ZSTDLVL_FAST_60);
			case -70:
				return (ZIO_ZSTDLVL_FAST_70);
			case -80:
				return (ZIO_ZSTDLVL_FAST_80);
			case -90:
				return (ZIO_ZSTDLVL_FAST_90);
			case -100:
				return (ZIO_ZSTDLVL_FAST_100);
			case -500:
				return (ZIO_ZSTDLVL_FAST_500);
			case -1000:
				return (ZIO_ZSTDLVL_FAST_1000);
		}
	}

	/* This shouldn't happen. Cause a panic. */
	panic("Invalid ZSTD level encountered: %d", level);

	return (ZIO_ZSTDLVL_INHERIT);
}

static int
zstd_enum_to_level(enum zio_zstd_levels elevel)
{
	int level = 0;

	if (elevel > ZIO_ZSTDLVL_INHERIT && elevel <= ZIO_ZSTDLVL_MAX) {
		level = elevel;
		return (level);
	} else if (elevel > ZIO_ZSTDLVL_FAST) {
		switch (elevel) {
			case ZIO_ZSTDLVL_FAST_1:
				return (-1);
			case ZIO_ZSTDLVL_FAST_2:
				return (-2);
			case ZIO_ZSTDLVL_FAST_3:
				return (-3);
			case ZIO_ZSTDLVL_FAST_4:
				return (-4);
			case ZIO_ZSTDLVL_FAST_5:
				return (-5);
			case ZIO_ZSTDLVL_FAST_6:
				return (-6);
			case ZIO_ZSTDLVL_FAST_7:
				return (-7);
			case ZIO_ZSTDLVL_FAST_8:
				return (-8);
			case ZIO_ZSTDLVL_FAST_9:
				return (-9);
			case ZIO_ZSTDLVL_FAST_10:
				return (-10);
			case ZIO_ZSTDLVL_FAST_20:
				return (-20);
			case ZIO_ZSTDLVL_FAST_30:
				return (-30);
			case ZIO_ZSTDLVL_FAST_40:
				return (-40);
			case ZIO_ZSTDLVL_FAST_50:
				return (-50);
			case ZIO_ZSTDLVL_FAST_60:
				return (-60);
			case ZIO_ZSTDLVL_FAST_70:
				return (-70);
			case ZIO_ZSTDLVL_FAST_80:
				return (-80);
			case ZIO_ZSTDLVL_FAST_90:
				return (-90);
			case ZIO_ZSTDLVL_FAST_100:
				return (-100);
			case ZIO_ZSTDLVL_FAST_500:
				return (-500);
			case ZIO_ZSTDLVL_FAST_1000:
				return (-1000);
			default:
			break;
		}
	}

	return (ZIO_ZSTD_LEVEL_DEFAULT);
}

size_t
zstd_compress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	size_t c_len;
	uint32_t bufsiz;
	int32_t zstdlevel;
	char *dest = d_start;

	ASSERT(d_len >= sizeof (bufsiz));
	ASSERT(d_len <= s_len);

	zstdlevel = zstd_enum_to_level(n);

	/* XXX: this could overflow, but we never have blocks that big */
	c_len = real_zstd_compress(s_start,
	    &dest[sizeof (bufsiz) + sizeof (zstdlevel)], s_len,
	    d_len - sizeof (bufsiz) - sizeof (zstdlevel), zstdlevel);

	/* Signal an error if the compression routine returned an error. */
	if (ZSTD_isError(c_len))
		return (s_len);

	/*
	 * Encode the compresed buffer size at the start. We'll need this in
	 * decompression to counter the effects of padding which might be
	 * added to the compressed buffer and which, if unhandled, would
	 * confuse the hell out of our decompression function.
	 */
	bufsiz = c_len;
	*(uint32_t *)dest = BE_32(bufsiz);
	/*
	 * Encode the compression level as well. We may need to know the
	 * original compression level if compressed_arc is disabled, to match
	 * the compression settings to write this block to the L2ARC.
	 * Encode the actual level, so if the enum changes in the future,
	 * we will be compatible.
	 */
	*(uint32_t *)(&dest[sizeof (bufsiz)]) = BE_32(zstdlevel);

	return (c_len + sizeof (bufsiz) + sizeof (zstdlevel));
}

int
zstd_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	const char *src = s_start;
	uint32_t bufsiz = BE_IN32(src);
	uint32_t cookie = BE_IN32(&src[sizeof (bufsiz)]);
	int32_t zstdlevel = zstd_level_to_enum(cookie);

	ASSERT(d_len >= s_len);
	ASSERT(zstdlevel > ZIO_ZSTDLVL_INHERIT);
	ASSERT(zstdlevel < ZIO_ZSTDLVL_LEVELS);

	/* invalid compressed buffer size encoded at start */
	if (bufsiz + sizeof (bufsiz) > s_len)
		return (1);

	/*
	 * Returns 0 on success (decompression function returned non-negative)
	 * and non-zero on failure (decompression function returned negative.
	 */
	if (ZSTD_isError(real_zstd_decompress(
	    &src[sizeof (bufsiz) + sizeof (zstdlevel)], \
	    d_start, bufsiz, d_len)))
		return (1);

	return (0);
}

int
zstd_getlevel(void *s_start, size_t s_len __unused)
{
	const char *src = s_start;
	uint32_t cookie = BE_IN32(&src[sizeof (uint32_t)]);
	int32_t zstdlevel = zstd_level_to_enum(cookie);

	ASSERT(zstdlevel > ZIO_ZSTDLVL_INHERIT);
	ASSERT(zstdlevel < ZIO_ZSTDLVL_LEVELS);

	return (zstdlevel);
}

static size_t
real_zstd_compress(const char *source, char *dest, int isize, int osize,
    int level)
{
	size_t result;
	ZSTD_CCtx *cctx;

	ASSERT(level != 0);
	if (level == ZIO_ZSTDLVL_DEFAULT)
		level = ZIO_ZSTD_LEVEL_DEFAULT;

	cctx = ZSTD_createCCtx_advanced(zstd_malloc);
	/*
	 * out of kernel memory, gently fall through - this will disable
	 * compression in zio_compress_data
	 */
	if (cctx == NULL)
		return (0);

	result = ZSTD_compressCCtx(cctx, dest, osize, source, isize, level);

	ZSTD_freeCCtx(cctx);
	return (result);
}


static size_t
real_zstd_decompress(const char *source, char *dest, int isize, int maxosize)
{
	size_t result;
	ZSTD_DCtx *dctx;

	dctx = ZSTD_createDCtx_advanced(zstd_malloc);
	if (dctx == NULL)
		return (ZSTD_error_memory_allocation);

	result = ZSTD_decompressDCtx(dctx, dest, maxosize, source, isize);

	ZSTD_freeDCtx(dctx);
	return (result);
}

extern void *
zstd_alloc(void *opaque __unused, size_t size)
{
	size_t nbytes = sizeof (struct zstd_kmem) + size;
	struct zstd_kmem *z;
	enum zstd_kmem_type type;
	int i;

	type = ZSTD_KMEM_UNKNOWN;
	for (i = 0; i < ZSTD_KMEM_COUNT; i++) {
		if (nbytes <= zstd_cache_size[i].kmem_size) {
			type = zstd_cache_size[i].kmem_type;
			z = kmem_cache_alloc(zstd_kmem_cache[type], \
			    KM_NOSLEEP);
			if (z)
				memset(z, 0, nbytes);

			break;
		}
	}
	/* No matching cache */
	if (type == ZSTD_KMEM_UNKNOWN) {
		z = kmem_zalloc(nbytes, KM_NOSLEEP);
	}
	if (z == NULL) {
		return (NULL);
	}

	z->kmem_magic = ZSTD_KMEM_MAGIC;
	z->kmem_type = type;
	z->kmem_size = nbytes;

	return ((void*)z + (sizeof (struct zstd_kmem)));
}

extern void
zstd_free(void *opaque __unused, void *ptr)
{
	struct zstd_kmem *z = ptr - sizeof (struct zstd_kmem);

	ASSERT(z->kmem_magic == ZSTD_KMEM_MAGIC);
	ASSERT(z->kmem_type < ZSTD_KMEM_COUNT);
	ASSERT(z->kmem_type >= ZSTD_KMEM_UNKNOWN);

	if (z->kmem_type == ZSTD_KMEM_UNKNOWN) {
		kmem_free(z, z->kmem_size);
	} else {
		kmem_cache_free(zstd_kmem_cache[z->kmem_type], z);
	}
}

extern void
zstd_init(void)
{
	int i;

	/* There is no estimate function for the CCtx itself */
	zstd_cache_size[1].kmem_magic = ZSTD_KMEM_MAGIC;
	zstd_cache_size[1].kmem_type = 1;
	zstd_cache_size[1].kmem_size = roundup2(zstd_cache_config[1].block_size
	    + sizeof (struct zstd_kmem), PAGESIZE);
	zstd_kmem_cache[1] = kmem_cache_create(
	    zstd_cache_config[1].cache_name, zstd_cache_size[1].kmem_size,
	    0, NULL, NULL, NULL, NULL, NULL, 0);

	/*
	 * Estimate the size of the ZSTD CCtx workspace required for each record
	 * size at each compression level.
	 */
	for (i = 2; i < ZSTD_KMEM_DCTX; i++) {
		ASSERT(zstd_cache_config[i].cache_name != NULL);
		zstd_cache_size[i].kmem_magic = ZSTD_KMEM_MAGIC;
		zstd_cache_size[i].kmem_type = i;
		zstd_cache_size[i].kmem_size = roundup2(
		    ZSTD_estimateCCtxSize_usingCParams(
		    ZSTD_getCParams(zstd_cache_config[i].compress_level,
		    zstd_cache_config[i].block_size, 0)) +
		    sizeof (struct zstd_kmem), PAGESIZE);
		zstd_kmem_cache[i] = kmem_cache_create(
		    zstd_cache_config[i].cache_name,
		    zstd_cache_size[i].kmem_size,
		    0, NULL, NULL, NULL, NULL, NULL, 0);
	}
	/* Estimate the size of the decompression context */
	zstd_cache_size[i].kmem_magic = ZSTD_KMEM_MAGIC;
	zstd_cache_size[i].kmem_type = i;
	zstd_cache_size[i].kmem_size = roundup2(ZSTD_estimateDCtxSize() +
	    sizeof (struct zstd_kmem), PAGESIZE);
	zstd_kmem_cache[i] = kmem_cache_create(zstd_cache_config[i].cache_name,
	    zstd_cache_size[i].kmem_size, 0, NULL, NULL, NULL, NULL, NULL, 0);

	/* Sort the kmem caches for later searching */
	zstd_qsort(zstd_cache_size, ZSTD_KMEM_COUNT, sizeof (struct zstd_kmem),
	    zstd_compare);

}

extern void
zstd_fini(void)
{
	int i, type;

	for (i = 0; i < ZSTD_KMEM_COUNT; i++) {
		type = zstd_cache_size[i].kmem_type;
		if (zstd_kmem_cache[type] != NULL) {
			kmem_cache_destroy(zstd_kmem_cache[type]);
		}
	}
}
