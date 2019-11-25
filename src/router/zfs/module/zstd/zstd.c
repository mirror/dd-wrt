/*
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 2016-2018 by Klara Systems Inc.
 * Copyright (c) 2016-2018 Allan Jude <allanjude@freebsd.org>
 * Copyright (c) 2018-2019 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 */

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/zfs_context.h>
#include <sys/zio_compress.h>
#include <sys/spa.h>

#define	ZSTD_STATIC_LINKING_ONLY
#include <sys/zstd/zstd.h>
#include <sys/zstd/zstd_errors.h>
#include <sys/zstd/error_private.h>

#include "common/zstd_common.c"
#include "common/fse_decompress.c"
#include "common/entropy_common.c"
#include "common/xxhash.c"
#include "compress/hist.c"
#include "compress/zstd_compress.c"
#include "compress/zstd_compress_literals.c"
#include "compress/zstd_compress_sequences.c"
#include "compress/fse_compress.c"
#include "compress/huf_compress.c"
#include "compress/zstd_double_fast.c"
#include "compress/zstd_fast.c"
#include "compress/zstd_lazy.c"
#include "compress/zstd_ldm.c"
#include "compress/zstd_opt.c"
#include "decompress/zstd_ddict.c"
#include "decompress/zstd_decompress.c"
#include "decompress/zstd_decompress_block.c"
#include "decompress/huf_decompress.c"

#define	ZSTD_KMEM_MAGIC		0x20160831

/* for BSD compat */
#define	__unused			__attribute__((unused))


#ifdef _KERNEL
#include <linux/sort.h>
#define	zstd_qsort(a, n, es, cmp) sort(a, n, es, cmp, NULL)
#else
#define	zstd_qsort qsort
#endif

static size_t real_zstd_compress(const char *source, char *dest, int isize,
    int osize, int level);
static size_t real_zstd_decompress(const char *source, char *dest, int isize,
    int maxosize);

static void *zstd_alloc(void *opaque, size_t size);
static void zstd_free(void *opaque, void *ptr);

static const ZSTD_customMem zstd_malloc = {
	zstd_alloc,
	zstd_free,
	NULL,
};
/* these enums are index references to zstd_cache_config */

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
	ZSTD_KMEM_WRKSPC_16M_MIN,
	ZSTD_KMEM_WRKSPC_16M_DEF,
	ZSTD_KMEM_WRKSPC_16M_MAX,
	ZSTD_KMEM_DCTX,
	ZSTD_KMEM_COUNT,
};

#define	ZSTD_POOL_MAX		16
#define	ZSTD_POOL_TIMEOUT	60 * 2

struct zstd_kmem;

struct zstd_pool {
	struct zstd_kmem *mem;
	size_t size;
	kmutex_t 		barrier;
	time_t timeout;
};

struct zstd_kmem {
	uint_t			kmem_magic;
	enum zstd_kmem_type	kmem_type;
	size_t			kmem_size;
	int			kmem_flags;
	struct zstd_pool	*pool;
	boolean_t		isvm;
};


static struct zstd_pool *zstd_mempool_cctx;
static struct zstd_pool *zstd_mempool_dctx;

/* initializes memory pool barrier mutexes */
void
zstd_mempool_init(void)
{
	int i;
	zstd_mempool_cctx = (struct zstd_pool *)
	    kmem_zalloc(ZSTD_POOL_MAX * sizeof (struct zstd_pool), KM_SLEEP);
	zstd_mempool_dctx = (struct zstd_pool *)
	    kmem_zalloc(ZSTD_POOL_MAX * sizeof (struct zstd_pool), KM_SLEEP);
	for (i = 0; i < ZSTD_POOL_MAX; i++) {
		mutex_init(&zstd_mempool_cctx[i].barrier, NULL,
		    MUTEX_DEFAULT, NULL);
		mutex_init(&zstd_mempool_dctx[i].barrier, NULL,
		    MUTEX_DEFAULT, NULL);
	}
}

/* release object from pool and free memory */
static void
release_pool(struct zstd_pool *pool)
{
	mutex_tryenter(&pool->barrier);
	mutex_exit(&pool->barrier);
	kmem_free(pool->mem, pool->size);
	pool->mem = NULL;
	pool->size = 0;
}

/* releases memory pool objects */
void
zstd_mempool_deinit(void)
{
	int i;
	for (i = 0; i < ZSTD_POOL_MAX; i++) {
		release_pool(&zstd_mempool_cctx[i]);
		release_pool(&zstd_mempool_dctx[i]);
	}
	kmem_free(zstd_mempool_dctx, ZSTD_POOL_MAX * sizeof (struct zstd_pool));
	kmem_free(zstd_mempool_cctx, ZSTD_POOL_MAX * sizeof (struct zstd_pool));
}

/*
 * tries to get cached allocated buffer from memory pool and allocate new one
 * if neccessary if a object is older than 2 minutes and does not fit to the
 * requested size, it will be released and a new cached entry will be allocated
 */
struct zstd_kmem *
zstd_mempool_alloc(struct zstd_pool *zstd_mempool, size_t size)
{
	int i;
	struct zstd_pool *pool;
	boolean_t reclaimed = B_FALSE;
	struct zstd_kmem *mem = NULL;

	for (i = 0; i < ZSTD_POOL_MAX; i++) {
		pool = &zstd_mempool[i];
		if (mutex_tryenter(&pool->barrier)) {
			if (!pool->mem) {
				struct zstd_kmem *z =
				    kvmem_zalloc(size, KM_SLEEP);
				pool->mem = z;
				if (!pool->mem) {
					mutex_exit(&pool->barrier);
					return (NULL);
				}
				z->pool = pool;
				pool->size = size;
				pool->timeout = gethrestime_sec() +
				    ZSTD_POOL_TIMEOUT;
				return (z);
			} else {
				if (size <= pool->size) {
					pool->timeout = gethrestime_sec() +
					    ZSTD_POOL_TIMEOUT;
					return (pool->mem);
				}
			}
			/*
			 * free memory if size doesnt fit and object
			 * is older than 2 minutes
			 */
			if (pool->mem && gethrestime_sec() > pool->timeout) {
				kmem_free(pool->mem, pool->size);
				pool->mem = NULL;
				pool->size = 0;
				pool->timeout = 0;
				reclaimed = B_TRUE;
			}
			mutex_exit(&pool->barrier);
		}
	}
	/*
	 * if a object was released from slot, we try a second attempt,
	 * since we can now reallocate the slot with a new size
	 */
	if (reclaimed) {
		mem = zstd_mempool_alloc(zstd_mempool, size);
	}

	return (mem ? mem : kvmem_zalloc(size, KM_NOSLEEP));
}

/*
 * mark object as released by releasing the barrier
 * mutex and clear the buffer
 */
void
zstd_mempool_free(struct zstd_kmem *z)
{
	struct zstd_pool *pool = z->pool;
	memset(pool->mem + sizeof (struct zstd_kmem), 0,
	    pool->size - sizeof (struct zstd_kmem));
	mutex_exit(&pool->barrier);
}


struct zstd_vmem {
	size_t			vmem_size;
	void			*vm;
	kmutex_t 		barrier;
	boolean_t		inuse;
};

struct zstd_kmem_config {
	size_t			block_size;
	int			compress_level;
	char			*cache_name;
	int 			flags;
};

static kmem_cache_t *zstd_kmem_cache[ZSTD_KMEM_COUNT] = { NULL };
static struct zstd_kmem zstd_cache_size[ZSTD_KMEM_COUNT] = {
	{ ZSTD_KMEM_MAGIC, 0, 0, KM_NOSLEEP, B_FALSE} };

static struct zstd_vmem zstd_vmem_cache[ZSTD_KMEM_COUNT] = {
		{
		.vmem_size = 0,
		.vm = NULL,
		.inuse = B_FALSE
		}
	};
static struct zstd_kmem_config zstd_cache_config[ZSTD_KMEM_COUNT] = {
	{ 0, 0, "zstd_unknown", KM_NOSLEEP},
	{ 0, 0, "zstd_cctx", KM_NOSLEEP },
	{ 4096, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_4k_min", KM_NOSLEEP},
	{ 4096, ZIO_ZSTD_LEVEL_DEFAULT, "zstd_wrkspc_4k_def", KM_NOSLEEP},
	{ 4096, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_4k_max", KM_NOSLEEP},
	{ 16384, ZIO_ZSTD_LEVEL_MIN, "zstd_wrkspc_16k_min", KM_NOSLEEP},
	{ 16384, ZIO_ZSTD_LEVEL_DEFAULT, "zstd_wrkspc_16k_def", KM_NOSLEEP},
	{ 16384, ZIO_ZSTD_LEVEL_MAX, "zstd_wrkspc_16k_max", KM_NOSLEEP},
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MIN, \
	    "zstd_wrkspc_128k_min", KM_NOSLEEP},
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_DEFAULT,
	    "zstd_wrkspc_128k_def", KM_NOSLEEP},
	{ SPA_OLD_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MAX, \
	    "zstd_wrkspc_128k_max", KM_NOSLEEP},
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MIN, \
	    "zstd_wrkspc_16m_min", KM_NOSLEEP},
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_DEFAULT, \
	    "zstd_wrkspc_16m_def", KM_NOSLEEP},
	{ SPA_MAXBLOCKSIZE, ZIO_ZSTD_LEVEL_MAX, \
	    "zstd_wrkspc_16m_max", KM_NOSLEEP},
	{ 0, 0, "zstd_dctx", KM_NOSLEEP},
};

static int
zstd_compare(const void *a, const void *b)
{
	struct zstd_kmem *x, *y;

	x = (struct zstd_kmem *)a;
	y = (struct zstd_kmem *)b;

	ASSERT3U(x->kmem_magic, ==, ZSTD_KMEM_MAGIC);
	ASSERT3U(y->kmem_magic, ==, ZSTD_KMEM_MAGIC);

	return (TREE_CMP(x->kmem_size, y->kmem_size));
}

static enum zio_zstd_levels
zstd_cookie_to_enum(int32_t level)
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
			default:
				/* This shouldn't happen. Cause a panic. */
#ifdef _KERNEL
			printk(KERN_ERR
			    "%s:Invalid ZSTD level encountered: %d",
			    __func__, level);
#endif
			return (ZIO_ZSTD_LEVEL_DEFAULT);
		}
	}

	/* This shouldn't happen. Cause a panic. */
#ifdef _KERNEL
	printk(KERN_ERR "%s:Invalid ZSTD level encountered: %d",
	    __func__, level);
#endif
	return (ZIO_ZSTD_LEVEL_DEFAULT);
}

static int32_t
zstd_enum_to_cookie(enum zio_zstd_levels elevel)
{
	int level = 0;

	if (elevel > ZIO_ZSTDLVL_INHERIT && elevel <= ZIO_ZSTDLVL_MAX) {
		level = elevel;
		return (level);
	} else if (elevel > ZIO_ZSTDLVL_FAST &&
	    elevel <= ZIO_ZSTDLVL_FAST_MAX) {
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
				/* This shouldn't happen. Cause a panic. */
#ifdef _KERNEL
			printk(KERN_ERR
			    "%s:Invalid ZSTD enum level encountered: %d",
			    __func__, elevel);
#endif
				return (3);
		}
	}

	/* This shouldn't happen. Cause a panic. */
#ifdef _KERNEL
	printk(KERN_ERR
	    "%s:Invalid ZSTD enum level encountered: %d",
	    __func__, elevel);
#endif

	return (3);
}

size_t
zstd_compress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	size_t c_len;
	uint32_t bufsiz;
	int32_t levelcookie;
	char *dest = d_start;

	ASSERT3U(d_len, >=, sizeof (bufsiz));
	ASSERT3U(d_len, <=, s_len);

	levelcookie = zstd_enum_to_cookie(n);

	/* XXX: this could overflow, but we never have blocks that big */
	c_len = real_zstd_compress(s_start,
	    &dest[sizeof (bufsiz) + sizeof (levelcookie)], s_len,
	    d_len - sizeof (bufsiz) - sizeof (levelcookie), levelcookie);

	/* Signal an error if the compression routine returned an error. */
	if (ZSTD_isError(c_len)) {
		return (s_len);
	}

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
	*(uint32_t *)(&dest[sizeof (bufsiz)]) = BE_32(levelcookie);

	return (c_len + sizeof (bufsiz) + sizeof (levelcookie));
}

int
zstd_get_level(void *s_start, size_t s_len, uint8_t *level)
{
	const char *src = s_start;
	uint32_t levelcookie = BE_IN32(&src[sizeof (levelcookie)]);
	uint8_t zstdlevel = zstd_cookie_to_enum(levelcookie);

	ASSERT3U(zstdlevel, !=, ZIO_ZSTDLVL_INHERIT);

	if (level != NULL) {
		*level = zstdlevel;
	}

	return (0);
}

int
zstd_decompress_level(void *s_start, void *d_start, size_t s_len, size_t d_len,
    uint8_t *level)
{
	const char *src = s_start;
	uint32_t bufsiz = BE_IN32(src);
	int32_t levelcookie = (int32_t)BE_IN32(&src[sizeof (bufsiz)]);
	uint8_t zstdlevel = zstd_cookie_to_enum(levelcookie);

	ASSERT3U(d_len, >=, s_len);
	ASSERT3U(zstdlevel, !=, ZIO_ZSTDLVL_INHERIT);

	/* invalid compressed buffer size encoded at start */
	if (bufsiz + sizeof (bufsiz) > s_len) {
		return (1);
	}

	/*
	 * Returns 0 on success (decompression function returned non-negative)
	 * and non-zero on failure (decompression function returned negative.
	 */
	if (ZSTD_isError(real_zstd_decompress(
	    &src[sizeof (bufsiz) + sizeof (levelcookie)], d_start, bufsiz,
	    d_len))) {
		return (1);
	}

	if (level != NULL) {
		*level = zstdlevel;
	}

	return (0);
}

int
zstd_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{

	return (zstd_decompress_level(s_start, d_start, s_len, d_len, NULL));
}

static size_t
real_zstd_compress(const char *source, char *dest, int isize, int osize,
    int level)
{
	size_t result;
	ZSTD_CCtx *cctx;

	ASSERT3U(level, !=, 0);
	if (level == ZIO_COMPLEVEL_DEFAULT) {
		level = ZIO_ZSTD_LEVEL_DEFAULT;
	}
	if (level == ZIO_ZSTDLVL_DEFAULT) {
		level = ZIO_ZSTD_LEVEL_DEFAULT;
	}

	cctx = ZSTD_createCCtx_advanced(zstd_malloc);
	/*
	 * out of kernel memory, gently fall through - this will disable
	 * compression in zio_compress_data
	 */
	if (cctx == NULL) {
		return (0);
	}

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

static int zstd_meminit(void);

extern void *
zstd_alloc(void *opaque __unused, size_t size)
{
	size_t nbytes = sizeof (struct zstd_kmem) + size;
	struct zstd_kmem *z = NULL;
	enum zstd_kmem_type type;
	enum zstd_kmem_type newtype;
	int i;
	type = ZSTD_KMEM_UNKNOWN;
	for (i = 0; i < ZSTD_KMEM_COUNT; i++) {
		if (nbytes <= zstd_cache_size[i].kmem_size) {
			type = zstd_cache_size[i].kmem_type;
			if (zstd_kmem_cache[type]) {
				z = kmem_cache_alloc( \
				    zstd_kmem_cache[type], \
				    zstd_cache_size[i].kmem_flags);
				if (z) {
					memset(z, 0, nbytes);
					z->isvm = B_FALSE;
				}
			}
			break;
		}
	}
	newtype = type;
	/* No matching cache */
	if (type == ZSTD_KMEM_UNKNOWN || z == NULL) {
		/*
		 * consider max allocation size
		 * so we need to use standard vmem allocator
		 */
#ifdef _KERNEL
		if (type != ZSTD_KMEM_DCTX)
			z = zstd_mempool_alloc(zstd_mempool_cctx, nbytes);
		else {
			z = zstd_mempool_alloc(zstd_mempool_dctx, nbytes);
			if (!z)
				z = kvmem_zalloc(nbytes, KM_SLEEP);
		}
#else
		z = kmem_zalloc(nbytes, KM_SLEEP);
#endif
		if (z)
			newtype = ZSTD_KMEM_UNKNOWN;
	}
	/* fallback if everything fails (decompression only) */
	if (!z && zstd_vmem_cache[type].vm && type == ZSTD_KMEM_DCTX) {
		mutex_enter(&zstd_vmem_cache[type].barrier);
		mutex_exit(&zstd_vmem_cache[type].barrier);

		mutex_enter(&zstd_vmem_cache[type].barrier);
		newtype = ZSTD_KMEM_DCTX;
		zstd_vmem_cache[type].inuse = B_TRUE;
		z = zstd_vmem_cache[type].vm;
		if (z) {
			memset(z, 0, nbytes);
			z->isvm = B_TRUE;
		}
	}

	/* allocation should always be successful */
	if (z == NULL) {
		return (NULL);
	}

	z->kmem_magic = ZSTD_KMEM_MAGIC;
	z->kmem_type = newtype;
	z->kmem_size = nbytes;

	return ((void*)z + (sizeof (struct zstd_kmem)));
}

extern void
zstd_free(void *opaque __unused, void *ptr)
{
	struct zstd_kmem *z = ptr - sizeof (struct zstd_kmem);
	enum zstd_kmem_type type;

	ASSERT3U(z->kmem_magic, ==, ZSTD_KMEM_MAGIC);
	ASSERT3U(z->kmem_type, <, ZSTD_KMEM_COUNT);
	ASSERT3U(z->kmem_type, >=, ZSTD_KMEM_UNKNOWN);
	type = z->kmem_type;
	if (type == ZSTD_KMEM_UNKNOWN) {
		if (z->pool) {
			zstd_mempool_free(z);
		} else {
			kmem_free(z, z->kmem_size);
		}
	} else {
		if (zstd_kmem_cache[type] && z->isvm == B_FALSE) {
			kmem_cache_free(zstd_kmem_cache[type], z);
		} else if (zstd_vmem_cache[type].vm && \
		    zstd_vmem_cache[type].inuse == B_TRUE) {
			zstd_vmem_cache[type].inuse = B_FALSE;
			/* release barrier */
			mutex_exit(&zstd_vmem_cache[type].barrier);
		}

	}
}
#ifndef _KERNEL
#define	__init
#define	__exit
#endif

static void create_vmem_cache(struct zstd_vmem *mem, char *name, size_t size)
{
#ifdef _KERNEL
	mem->vmem_size = size;
	mem->vm = \
	    vmem_zalloc(mem->vmem_size, \
	    KM_SLEEP);
	mem->inuse = B_FALSE;
	mutex_init(&mem->barrier, \
	    NULL, MUTEX_DEFAULT, NULL);
#endif
}
static int zstd_meminit(void)
{
	int i;

	zstd_mempool_init();
	/* There is no estimate function for the CCtx itself */
	zstd_cache_size[1].kmem_magic = ZSTD_KMEM_MAGIC;
	zstd_cache_size[1].kmem_type = 1;
	zstd_cache_size[1].kmem_size = P2ROUNDUP(zstd_cache_config[1].block_size
	    + sizeof (struct zstd_kmem), PAGESIZE);
	zstd_kmem_cache[1] = kmem_cache_create(
	    zstd_cache_config[1].cache_name, zstd_cache_size[1].kmem_size,
	    0, NULL, NULL, NULL, NULL, NULL, KMC_KVMEM);
	zstd_cache_size[1].kmem_flags = zstd_cache_config[1].flags;

	/*
	 * Estimate the size of the ZSTD CCtx workspace required for each record
	 * size at each compression level.
	 */
	for (i = 2; i < ZSTD_KMEM_DCTX; i++) {
		ASSERT(zstd_cache_config[i].cache_name != NULL);
		zstd_cache_size[i].kmem_magic = ZSTD_KMEM_MAGIC;
		zstd_cache_size[i].kmem_type = i;
		zstd_cache_size[i].kmem_size = P2ROUNDUP(
		    ZSTD_estimateCCtxSize_usingCParams(
		    ZSTD_getCParams(zstd_cache_config[i].compress_level,
		    zstd_cache_config[i].block_size, 0)) +
		    sizeof (struct zstd_kmem), PAGESIZE);
		zstd_cache_size[i].kmem_flags = zstd_cache_config[i].flags;
		zstd_kmem_cache[i] = kmem_cache_create(
		    zstd_cache_config[i].cache_name,
		    zstd_cache_size[i].kmem_size,
		    0, NULL, NULL, NULL, NULL, NULL, KMC_KVMEM);
	}

	/* Estimate the size of the decompression context */
	zstd_cache_size[i].kmem_magic = ZSTD_KMEM_MAGIC;
	zstd_cache_size[i].kmem_type = i;
	zstd_cache_size[i].kmem_size = P2ROUNDUP(ZSTD_estimateDCtxSize() +
	    sizeof (struct zstd_kmem), PAGESIZE);
	zstd_kmem_cache[i] = kmem_cache_create(zstd_cache_config[i].cache_name,
	    zstd_cache_size[i].kmem_size, 0, NULL, NULL, NULL, NULL, NULL,
	    KMC_KVMEM);
	zstd_cache_size[i].kmem_flags = zstd_cache_config[i].flags;


	create_vmem_cache(&zstd_vmem_cache[i], \
	    zstd_cache_config[i].cache_name, \
	    zstd_cache_size[i].kmem_size);

	/* Sort the kmem caches for later searching */
	zstd_qsort(zstd_cache_size, ZSTD_KMEM_COUNT, sizeof (struct zstd_kmem),
	    zstd_compare);

	return (0);
}

extern int __init
zstd_init(void)
{
	zstd_meminit();
	return (0);
}

extern void __exit
zstd_fini(void)
{
	int i, type;

	for (i = 0; i < ZSTD_KMEM_COUNT; i++) {
		type = zstd_cache_size[i].kmem_type;
		if (zstd_vmem_cache[type].vm) {
			kmem_free(zstd_vmem_cache[type].vm, \
			    zstd_vmem_cache[type].vmem_size);
			zstd_vmem_cache[type].vm = NULL;
			zstd_vmem_cache[type].inuse = B_FALSE;
			mutex_destroy(&zstd_vmem_cache[type].barrier);
		} else {
			if (zstd_kmem_cache[type] != NULL) {
				kmem_cache_destroy(zstd_kmem_cache[type]);
			}
		}
	}
	zstd_mempool_deinit();
}


#if defined(_KERNEL)
module_init(zstd_init);
module_exit(zstd_fini);
EXPORT_SYMBOL(zstd_compress);
EXPORT_SYMBOL(zstd_decompress_level);
EXPORT_SYMBOL(zstd_decompress);
EXPORT_SYMBOL(zstd_get_level);

MODULE_DESCRIPTION("ZSTD Compression for ZFS");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.4.4");
#endif