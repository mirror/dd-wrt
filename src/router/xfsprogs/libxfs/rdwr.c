// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 */


#include "libxfs_priv.h"
#include "init.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_inode_buf.h"
#include "xfs_inode_fork.h"
#include "xfs_inode.h"
#include "xfs_trans.h"
#include "libfrog/platform.h"

#include "libxfs.h"

static void libxfs_brelse(struct cache_node *node);

/*
 * Important design/architecture note:
 *
 * The userspace code that uses the buffer cache is much less constrained than
 * the kernel code. The userspace code is pretty nasty in places, especially
 * when it comes to buffer error handling.  Very little of the userspace code
 * outside libxfs clears bp->b_error - very little code even checks it - so the
 * libxfs code is tripping on stale errors left by the userspace code.
 *
 * We can't clear errors or zero buffer contents in libxfs_buf_get-* like we do
 * in the kernel, because those functions are used by the libxfs_readbuf_*
 * functions and hence need to leave the buffers unchanged on cache hits. This
 * is actually the only way to gather a write error from a libxfs_writebuf()
 * call - you need to get the buffer again so you can check bp->b_error field -
 * assuming that the buffer is still in the cache when you check, that is.
 *
 * This is very different to the kernel code which does not release buffers on a
 * write so we can wait on IO and check errors. The kernel buffer cache also
 * guarantees a buffer of a known initial state from xfs_buf_get() even on a
 * cache hit.
 *
 * IOWs, userspace is behaving quite differently to the kernel and as a result
 * it leaks errors from reads, invalidations and writes through
 * libxfs_buf_get/libxfs_buf_read.
 *
 * The result of this is that until the userspace code outside libxfs is cleaned
 * up, functions that release buffers from userspace control (i.e
 * libxfs_writebuf/libxfs_buf_relse) need to zero bp->b_error to prevent
 * propagation of stale errors into future buffer operations.
 */

#define BDSTRAT_SIZE	(256 * 1024)

#define IO_BCOMPARE_CHECK

/* XXX: (dgc) Propagate errors, only exit if fail-on-error flag set */
int
libxfs_device_zero(struct xfs_buftarg *btp, xfs_daddr_t start, uint len)
{
	xfs_off_t	start_offset, end_offset, offset;
	ssize_t		zsize, bytes;
	size_t		len_bytes;
	char		*z;
	int		error, fd;

	fd = libxfs_device_to_fd(btp->bt_bdev);
	start_offset = LIBXFS_BBTOOFF64(start);

	/* try to use special zeroing methods, fall back to writes if needed */
	len_bytes = LIBXFS_BBTOOFF64(len);
	error = platform_zero_range(fd, start_offset, len_bytes);
	if (!error) {
		xfs_buftarg_trip_write(btp);
		return 0;
	}

	zsize = min(BDSTRAT_SIZE, BBTOB(len));
	if ((z = memalign(libxfs_device_alignment(), zsize)) == NULL) {
		fprintf(stderr,
			_("%s: %s can't memalign %d bytes: %s\n"),
			progname, __FUNCTION__, (int)zsize, strerror(errno));
		exit(1);
	}
	memset(z, 0, zsize);

	if ((lseek(fd, start_offset, SEEK_SET)) < 0) {
		fprintf(stderr, _("%s: %s seek to offset %llu failed: %s\n"),
			progname, __FUNCTION__,
			(unsigned long long)start_offset, strerror(errno));
		exit(1);
	}

	end_offset = LIBXFS_BBTOOFF64(start + len) - start_offset;
	for (offset = 0; offset < end_offset; ) {
		bytes = min((ssize_t)(end_offset - offset), zsize);
		if ((bytes = write(fd, z, bytes)) < 0) {
			fprintf(stderr, _("%s: %s write failed: %s\n"),
				progname, __FUNCTION__, strerror(errno));
			exit(1);
		} else if (bytes == 0) {
			fprintf(stderr, _("%s: %s not progressing?\n"),
				progname, __FUNCTION__);
			exit(1);
		}
		xfs_buftarg_trip_write(btp);
		offset += bytes;
	}
	free(z);
	return 0;
}

static void unmount_record(void *p)
{
	xlog_op_header_t	*op = (xlog_op_header_t *)p;
	/* the data section must be 32 bit size aligned */
	struct {
	    uint16_t magic;
	    uint16_t pad1;
	    uint32_t pad2; /* may as well make it 64 bits */
	} magic = { XLOG_UNMOUNT_TYPE, 0, 0 };

	memset(p, 0, BBSIZE);
	/* dummy tid to mark this as written from userspace */
	op->oh_tid = cpu_to_be32(0xb0c0d0d0);
	op->oh_len = cpu_to_be32(sizeof(magic));
	op->oh_clientid = XFS_LOG;
	op->oh_flags = XLOG_UNMOUNT_TRANS;
	op->oh_res2 = 0;

	/* and the data for this op */
	memcpy((char *)p + sizeof(xlog_op_header_t), &magic, sizeof(magic));
}

static char *next(
	char		*ptr,
	int		offset,
	void		*private)
{
	struct xfs_buf	*buf = (struct xfs_buf *)private;

	if (buf &&
	    (BBTOB(buf->b_length) < (int)(ptr - (char *)buf->b_addr) + offset))
		abort();

	return ptr + offset;
}

struct xfs_buf *
libxfs_getsb(
	struct xfs_mount	*mp)
{
	struct xfs_buf		*bp;

	libxfs_buf_read(mp->m_ddev_targp, XFS_SB_DADDR, XFS_FSS_TO_BB(mp, 1),
			0, &bp, &xfs_sb_buf_ops);
	return bp;
}

struct kmem_cache			*xfs_buf_cache;

static struct cache_mru		xfs_buf_freelist =
	{{&xfs_buf_freelist.cm_list, &xfs_buf_freelist.cm_list},
	 0, PTHREAD_MUTEX_INITIALIZER };

/*
 * The bufkey is used to pass the new buffer information to the cache object
 * allocation routine. Because discontiguous buffers need to pass different
 * information, we need fields to pass that information. However, because the
 * blkno and bblen is needed for the initial cache entry lookup (i.e. for
 * bcompare) the fact that the map/nmaps is non-null to switch to discontiguous
 * buffer initialisation instead of a contiguous buffer.
 */
struct xfs_bufkey {
	struct xfs_buftarg	*buftarg;
	xfs_daddr_t		blkno;
	unsigned int		bblen;
	struct xfs_buf_map	*map;
	int			nmaps;
};

/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME	0x9e37fffffffc0001UL
#define CACHE_LINE_SIZE		64
static unsigned int
libxfs_bhash(cache_key_t key, unsigned int hashsize, unsigned int hashshift)
{
	uint64_t	hashval = ((struct xfs_bufkey *)key)->blkno;
	uint64_t	tmp;

	tmp = hashval ^ (GOLDEN_RATIO_PRIME + hashval) / CACHE_LINE_SIZE;
	tmp = tmp ^ ((tmp ^ GOLDEN_RATIO_PRIME) >> hashshift);
	return tmp % hashsize;
}

static int
libxfs_bcompare(struct cache_node *node, cache_key_t key)
{
	struct xfs_buf		*bp = container_of(node, struct xfs_buf,
						   b_node);
	struct xfs_bufkey	*bkey = (struct xfs_bufkey *)key;

	if (bp->b_target->bt_bdev == bkey->buftarg->bt_bdev &&
	    bp->b_cache_key == bkey->blkno) {
		if (bp->b_length == bkey->bblen)
			return CACHE_HIT;
#ifdef IO_BCOMPARE_CHECK
		if (!(libxfs_bcache->c_flags & CACHE_MISCOMPARE_PURGE)) {
			fprintf(stderr,
	"%lx: Badness in key lookup (length)\n"
	"bp=(bno 0x%llx, len %u bytes) key=(bno 0x%llx, len %u bytes)\n",
				pthread_self(),
				(unsigned long long)xfs_buf_daddr(bp),
				BBTOB(bp->b_length),
				(unsigned long long)bkey->blkno,
				BBTOB(bkey->bblen));
		}
#endif
		return CACHE_PURGE;
	}
	return CACHE_MISS;
}

static void
__initbuf(struct xfs_buf *bp, struct xfs_buftarg *btp, xfs_daddr_t bno,
		unsigned int bytes)
{
	bp->b_flags = 0;
	bp->b_cache_key = bno;
	bp->b_length = BTOBB(bytes);
	bp->b_target = btp;
	bp->b_mount = btp->bt_mount;
	bp->b_error = 0;
	if (!bp->b_addr)
		bp->b_addr = memalign(libxfs_device_alignment(), bytes);
	if (!bp->b_addr) {
		fprintf(stderr,
			_("%s: %s can't memalign %u bytes: %s\n"),
			progname, __FUNCTION__, bytes,
			strerror(errno));
		exit(1);
	}
	memset(bp->b_addr, 0, bytes);
	pthread_mutex_init(&bp->b_lock, NULL);
	bp->b_holder = 0;
	bp->b_recur = 0;
	bp->b_ops = NULL;
	INIT_LIST_HEAD(&bp->b_li_list);

	if (!bp->b_maps)
		bp->b_maps = &bp->__b_map;

	if (bp->b_maps == &bp->__b_map) {
		bp->b_nmaps = 1;
		bp->b_maps[0].bm_bn = bno;
		bp->b_maps[0].bm_len = bp->b_length;
	}
}

static void
libxfs_initbuf(struct xfs_buf *bp, struct xfs_buftarg *btp, xfs_daddr_t bno,
		unsigned int bytes)
{
	__initbuf(bp, btp, bno, bytes);
}

static void
libxfs_initbuf_map(struct xfs_buf *bp, struct xfs_buftarg *btp,
		struct xfs_buf_map *map, int nmaps)
{
	unsigned int bytes = 0;
	int i;

	bytes = sizeof(struct xfs_buf_map) * nmaps;
	bp->b_maps = malloc(bytes);
	if (!bp->b_maps) {
		fprintf(stderr,
			_("%s: %s can't malloc %u bytes: %s\n"),
			progname, __FUNCTION__, bytes,
			strerror(errno));
		exit(1);
	}
	bp->b_nmaps = nmaps;

	bytes = 0;
	for ( i = 0; i < nmaps; i++) {
		bp->b_maps[i].bm_bn = map[i].bm_bn;
		bp->b_maps[i].bm_len = map[i].bm_len;
		bytes += BBTOB(map[i].bm_len);
	}

	__initbuf(bp, btp, map[0].bm_bn, bytes);
	bp->b_flags |= LIBXFS_B_DISCONTIG;
}

static struct xfs_buf *
__libxfs_getbufr(int blen)
{
	struct xfs_buf	*bp;

	/*
	 * first look for a buffer that can be used as-is,
	 * if one cannot be found, see if there is a buffer,
	 * and if so, free its buffer and set b_addr to NULL
	 * before calling libxfs_initbuf.
	 */
	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	if (!list_empty(&xfs_buf_freelist.cm_list)) {
		list_for_each_entry(bp, &xfs_buf_freelist.cm_list, b_node.cn_mru) {
			if (bp->b_length == BTOBB(blen)) {
				list_del_init(&bp->b_node.cn_mru);
				break;
			}
		}
		if (&bp->b_node.cn_mru == &xfs_buf_freelist.cm_list) {
			bp = list_entry(xfs_buf_freelist.cm_list.next,
					struct xfs_buf, b_node.cn_mru);
			list_del_init(&bp->b_node.cn_mru);
			free(bp->b_addr);
			bp->b_addr = NULL;
			if (bp->b_maps != &bp->__b_map)
				free(bp->b_maps);
			bp->b_maps = NULL;
		}
	} else
		bp = kmem_cache_zalloc(xfs_buf_cache, 0);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);
	bp->b_ops = NULL;
	if (bp->b_flags & LIBXFS_B_DIRTY)
		fprintf(stderr, "found dirty buffer (bulk) on free list!\n");

	return bp;
}

static struct xfs_buf *
libxfs_getbufr(struct xfs_buftarg *btp, xfs_daddr_t blkno, int bblen)
{
	struct xfs_buf	*bp;
	int		blen = BBTOB(bblen);

	bp =__libxfs_getbufr(blen);
	if (bp)
		libxfs_initbuf(bp, btp, blkno, blen);
	return bp;
}

static struct xfs_buf *
libxfs_getbufr_map(struct xfs_buftarg *btp, xfs_daddr_t blkno, int bblen,
		struct xfs_buf_map *map, int nmaps)
{
	struct xfs_buf	*bp;
	int		blen = BBTOB(bblen);

	if (!map || !nmaps) {
		fprintf(stderr,
			_("%s: %s invalid map %p or nmaps %d\n"),
			progname, __FUNCTION__, map, nmaps);
		exit(1);
	}

	if (blkno != map[0].bm_bn) {
		fprintf(stderr,
			_("%s: %s map blkno 0x%llx doesn't match key 0x%llx\n"),
			progname, __FUNCTION__, (long long)map[0].bm_bn,
			(long long)blkno);
		exit(1);
	}

	bp =__libxfs_getbufr(blen);
	if (bp)
		libxfs_initbuf_map(bp, btp, map, nmaps);
	return bp;
}

void
xfs_buf_lock(
	struct xfs_buf	*bp)
{
	if (use_xfs_buf_lock)
		pthread_mutex_lock(&bp->b_lock);
}

void
xfs_buf_unlock(
	struct xfs_buf	*bp)
{
	if (use_xfs_buf_lock)
		pthread_mutex_unlock(&bp->b_lock);
}

static int
__cache_lookup(
	struct xfs_bufkey	*key,
	unsigned int		flags,
	struct xfs_buf		**bpp)
{
	struct cache_node	*cn = NULL;
	struct xfs_buf		*bp;

	*bpp = NULL;

	cache_node_get(libxfs_bcache, key, &cn);
	if (!cn)
		return -ENOMEM;
	bp = container_of(cn, struct xfs_buf, b_node);

	if (use_xfs_buf_lock) {
		int		ret;

		ret = pthread_mutex_trylock(&bp->b_lock);
		if (ret) {
			ASSERT(ret == EAGAIN);
			if (flags & LIBXFS_GETBUF_TRYLOCK) {
				cache_node_put(libxfs_bcache, cn);
				return -EAGAIN;
			}

			if (pthread_equal(bp->b_holder, pthread_self())) {
				fprintf(stderr,
	_("Warning: recursive buffer locking at block %" PRIu64 " detected\n"),
					key->blkno);
				bp->b_recur++;
				*bpp = bp;
				return 0;
			} else {
				pthread_mutex_lock(&bp->b_lock);
			}
		}

		bp->b_holder = pthread_self();
	}

	cache_node_set_priority(libxfs_bcache, cn,
			cache_node_get_priority(cn) - CACHE_PREFETCH_PRIORITY);
	*bpp = bp;
	return 0;
}

static int
libxfs_getbuf_flags(
	struct xfs_buftarg	*btp,
	xfs_daddr_t		blkno,
	int			len,
	unsigned int		flags,
	struct xfs_buf		**bpp)
{
	struct xfs_bufkey	key = {NULL};
	int			ret;

	key.buftarg = btp;
	key.blkno = blkno;
	key.bblen = len;

	ret = __cache_lookup(&key, flags, bpp);
	if (ret)
		return ret;

	if (btp == btp->bt_mount->m_ddev_targp) {
		(*bpp)->b_pag = xfs_perag_get(btp->bt_mount,
				xfs_daddr_to_agno(btp->bt_mount, blkno));
	}

	return 0;
}

/*
 * Clean the buffer flags for libxfs_getbuf*(), which wants to return
 * an unused buffer with clean state.  This prevents CRC errors on a
 * re-read of a corrupt block that was prefetched and freed.  This
 * can happen with a massively corrupt directory that is discarded,
 * but whose blocks are then recycled into expanding lost+found.
 *
 * Note however that if the buffer's dirty (prefetch calls getbuf)
 * we'll leave the state alone because we don't want to discard blocks
 * that have been fixed.
 */
static void
reset_buf_state(
	struct xfs_buf	*bp)
{
	if (bp && !(bp->b_flags & LIBXFS_B_DIRTY))
		bp->b_flags &= ~(LIBXFS_B_UNCHECKED | LIBXFS_B_STALE |
				LIBXFS_B_UPTODATE);
}

static int
__libxfs_buf_get_map(
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps,
	int			flags,
	struct xfs_buf		**bpp)
{
	struct xfs_bufkey	key = {NULL};
	int			i;

	if (nmaps == 1)
		return libxfs_getbuf_flags(btp, map[0].bm_bn, map[0].bm_len,
				flags, bpp);

	key.buftarg = btp;
	key.blkno = map[0].bm_bn;
	for (i = 0; i < nmaps; i++) {
		key.bblen += map[i].bm_len;
	}
	key.map = map;
	key.nmaps = nmaps;

	return __cache_lookup(&key, flags, bpp);
}

int
libxfs_buf_get_map(
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps,
	int			flags,
	struct xfs_buf		**bpp)
{
	int			error;

	error = __libxfs_buf_get_map(btp, map, nmaps, flags, bpp);
	if (error)
		return error;

	reset_buf_state(*bpp);
	return 0;
}

void
libxfs_buf_relse(
	struct xfs_buf	*bp)
{
	/*
	 * ensure that any errors on this use of the buffer don't carry
	 * over to the next user.
	 */
	bp->b_error = 0;
	if (use_xfs_buf_lock) {
		if (bp->b_recur) {
			bp->b_recur--;
		} else {
			bp->b_holder = 0;
			pthread_mutex_unlock(&bp->b_lock);
		}
	}

	if (!list_empty(&bp->b_node.cn_hash))
		cache_node_put(libxfs_bcache, &bp->b_node);
	else if (--bp->b_node.cn_count == 0) {
		if (bp->b_flags & LIBXFS_B_DIRTY)
			libxfs_bwrite(bp);
		libxfs_brelse(&bp->b_node);
	}
}

static struct cache_node *
libxfs_balloc(
	cache_key_t		key)
{
	struct xfs_bufkey	*bufkey = (struct xfs_bufkey *)key;
	struct xfs_buf		*bp;

	if (bufkey->map)
		bp = libxfs_getbufr_map(bufkey->buftarg, bufkey->blkno,
				bufkey->bblen, bufkey->map, bufkey->nmaps);
	else
		bp = libxfs_getbufr(bufkey->buftarg, bufkey->blkno,
				bufkey->bblen);
	return &bp->b_node;
}


static int
__read_buf(int fd, void *buf, int len, off64_t offset, int flags)
{
	int	sts;

	sts = pread(fd, buf, len, offset);
	if (sts < 0) {
		int error = errno;
		fprintf(stderr, _("%s: read failed: %s\n"),
			progname, strerror(error));
		return -error;
	} else if (sts != len) {
		fprintf(stderr, _("%s: error - read only %d of %d bytes\n"),
			progname, sts, len);
		return -EIO;
	}
	return 0;
}

int
libxfs_readbufr(struct xfs_buftarg *btp, xfs_daddr_t blkno, struct xfs_buf *bp,
		int len, int flags)
{
	int	fd = libxfs_device_to_fd(btp->bt_bdev);
	int	bytes = BBTOB(len);
	int	error;

	ASSERT(len <= bp->b_length);

	error = __read_buf(fd, bp->b_addr, bytes, LIBXFS_BBTOOFF64(blkno), flags);
	if (!error &&
	    bp->b_target->bt_bdev == btp->bt_bdev &&
	    bp->b_cache_key == blkno &&
	    bp->b_length == len)
		bp->b_flags |= LIBXFS_B_UPTODATE;
	bp->b_error = error;
	return error;
}

int
libxfs_readbuf_verify(
	struct xfs_buf		*bp,
	const struct xfs_buf_ops *ops)
{
	if (!ops)
		return bp->b_error;

	bp->b_ops = ops;
	bp->b_ops->verify_read(bp);
	bp->b_flags &= ~LIBXFS_B_UNCHECKED;
	return bp->b_error;
}

int
libxfs_readbufr_map(struct xfs_buftarg *btp, struct xfs_buf *bp, int flags)
{
	int	fd;
	int	error = 0;
	void	*buf;
	int	i;

	fd = libxfs_device_to_fd(btp->bt_bdev);
	buf = bp->b_addr;
	for (i = 0; i < bp->b_nmaps; i++) {
		off64_t	offset = LIBXFS_BBTOOFF64(bp->b_maps[i].bm_bn);
		int len = BBTOB(bp->b_maps[i].bm_len);

		error = __read_buf(fd, buf, len, offset, flags);
		if (error) {
			bp->b_error = error;
			break;
		}
		buf += len;
	}

	if (!error)
		bp->b_flags |= LIBXFS_B_UPTODATE;
	return error;
}

int
libxfs_buf_read_map(
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps,
	int			flags,
	struct xfs_buf		**bpp,
	const struct xfs_buf_ops *ops)
{
	struct xfs_buf		*bp;
	bool			salvage = flags & LIBXFS_READBUF_SALVAGE;
	int			error = 0;

	*bpp = NULL;
	if (nmaps == 1)
		error = libxfs_getbuf_flags(btp, map[0].bm_bn, map[0].bm_len,
				0, &bp);
	else
		error = __libxfs_buf_get_map(btp, map, nmaps, 0, &bp);
	if (error)
		return error;

	/*
	 * If the buffer was prefetched, it is likely that it was not validated.
	 * Hence if we are supplied an ops function and the buffer is marked as
	 * unchecked, we need to validate it now.
	 *
	 * We do this verification even if the buffer is dirty - the
	 * verification is almost certainly going to fail the CRC check in this
	 * case as a dirty buffer has not had the CRC recalculated. However, we
	 * should not be dirtying unchecked buffers and therefore failing it
	 * here because it's dirty and unchecked indicates we've screwed up
	 * somewhere else.
	 *
	 * Note that if the caller passes in LIBXFS_READBUF_SALVAGE, that means
	 * they want the buffer even if it fails verification.
	 */
	bp->b_error = 0;
	if (bp->b_flags & (LIBXFS_B_UPTODATE | LIBXFS_B_DIRTY)) {
		if (bp->b_flags & LIBXFS_B_UNCHECKED)
			error = libxfs_readbuf_verify(bp, ops);
		if (error && !salvage)
			goto err;
		goto ok;
	}

	/*
	 * Set the ops on a cache miss (i.e. first physical read) as the
	 * verifier may change the ops to match the type of buffer it contains.
	 * A cache hit might reset the verifier to the original type if we set
	 * it again, but it won't get called again and set to match the buffer
	 * contents. *cough* xfs_da_node_buf_ops *cough*.
	 */
	if (nmaps == 1)
		error = libxfs_readbufr(btp, map[0].bm_bn, bp, map[0].bm_len,
				flags);
	else
		error = libxfs_readbufr_map(btp, bp, flags);
	if (error)
		goto err;

	error = libxfs_readbuf_verify(bp, ops);
	if (error && !salvage)
		goto err;

ok:
	*bpp = bp;
	return 0;
err:
	libxfs_buf_relse(bp);
	return error;
}

/* Allocate a raw uncached buffer. */
static inline struct xfs_buf *
libxfs_getbufr_uncached(
	struct xfs_buftarg	*targ,
	xfs_daddr_t		daddr,
	size_t			bblen)
{
	struct xfs_buf		*bp;

	bp = libxfs_getbufr(targ, daddr, bblen);
	if (!bp)
		return NULL;

	INIT_LIST_HEAD(&bp->b_node.cn_hash);
	bp->b_node.cn_count = 1;
	return bp;
}

/*
 * Allocate an uncached buffer that points nowhere.  The refcount will be 1,
 * and the cache node hash list will be empty to indicate that it's uncached.
 */
int
libxfs_buf_get_uncached(
	struct xfs_buftarg	*targ,
	size_t			bblen,
	int			flags,
	struct xfs_buf		**bpp)
{
	*bpp = libxfs_getbufr_uncached(targ, XFS_BUF_DADDR_NULL, bblen);
	return *bpp != NULL ? 0 : -ENOMEM;
}

/*
 * Allocate and read an uncached buffer.  The refcount will be 1, and the cache
 * node hash list will be empty to indicate that it's uncached.
 */
int
libxfs_buf_read_uncached(
	struct xfs_buftarg	*targ,
	xfs_daddr_t		daddr,
	size_t			bblen,
	int			flags,
	struct xfs_buf		**bpp,
	const struct xfs_buf_ops *ops)
{
	struct xfs_buf		*bp;
	int			error;

	*bpp = NULL;
	bp = libxfs_getbufr_uncached(targ, daddr, bblen);
	if (!bp)
		return -ENOMEM;

	error = libxfs_readbufr(targ, daddr, bp, bblen, flags);
	if (error)
		goto err;

	error = libxfs_readbuf_verify(bp, ops);
	if (error)
		goto err;

	*bpp = bp;
	return 0;
err:
	libxfs_buf_relse(bp);
	return error;
}

static int
__write_buf(int fd, void *buf, int len, off64_t offset, int flags)
{
	int	sts;

	sts = pwrite(fd, buf, len, offset);
	if (sts < 0) {
		int error = errno;
		fprintf(stderr, _("%s: pwrite failed: %s\n"),
			progname, strerror(error));
		return -error;
	} else if (sts != len) {
		fprintf(stderr, _("%s: error - pwrite only %d of %d bytes\n"),
			progname, sts, len);
		return -EIO;
	}
	return 0;
}

int
libxfs_bwrite(
	struct xfs_buf	*bp)
{
	int		fd = libxfs_device_to_fd(bp->b_target->bt_bdev);

	/*
	 * we never write buffers that are marked stale. This indicates they
	 * contain data that has been invalidated, and even if the buffer is
	 * dirty it must *never* be written. Verifiers are wonderful for finding
	 * bugs like this. Make sure the error is obvious as to the cause.
	 */
	if (bp->b_flags & LIBXFS_B_STALE) {
		bp->b_error = -ESTALE;
		return bp->b_error;
	}

	/* Trigger the writeback hook if there is one. */
	if (bp->b_mount->m_buf_writeback_fn)
		bp->b_mount->m_buf_writeback_fn(bp);

	/*
	 * clear any pre-existing error status on the buffer. This can occur if
	 * the buffer is corrupt on disk and the repair process doesn't clear
	 * the error before fixing and writing it back.
	 */
	bp->b_error = 0;
	if (bp->b_ops) {
		bp->b_ops->verify_write(bp);
		if (bp->b_error) {
			fprintf(stderr,
	_("%s: write verifier failed on %s bno 0x%llx/0x%x\n"),
				__func__, bp->b_ops->name,
				(unsigned long long)xfs_buf_daddr(bp),
				bp->b_length);
			return bp->b_error;
		}
	}

	if (!(bp->b_flags & LIBXFS_B_DISCONTIG)) {
		bp->b_error = __write_buf(fd, bp->b_addr, BBTOB(bp->b_length),
				    LIBXFS_BBTOOFF64(xfs_buf_daddr(bp)),
				    bp->b_flags);
	} else {
		int	i;
		void	*buf = bp->b_addr;

		for (i = 0; i < bp->b_nmaps; i++) {
			off64_t	offset = LIBXFS_BBTOOFF64(bp->b_maps[i].bm_bn);
			int len = BBTOB(bp->b_maps[i].bm_len);

			bp->b_error = __write_buf(fd, buf, len, offset,
						  bp->b_flags);
			if (bp->b_error)
				break;
			buf += len;
		}
	}

	if (bp->b_error) {
		fprintf(stderr,
	_("%s: write failed on %s bno 0x%llx/0x%x, err=%d\n"),
			__func__, bp->b_ops ? bp->b_ops->name : "(unknown)",
			(unsigned long long)xfs_buf_daddr(bp),
			bp->b_length, -bp->b_error);
	} else {
		bp->b_flags |= LIBXFS_B_UPTODATE;
		bp->b_flags &= ~(LIBXFS_B_DIRTY | LIBXFS_B_UNCHECKED);
		xfs_buftarg_trip_write(bp->b_target);
	}
	return bp->b_error;
}

/*
 * Mark a buffer dirty.  The dirty data will be written out when the cache
 * is flushed (or at release time if the buffer is uncached).
 */
void
libxfs_buf_mark_dirty(
	struct xfs_buf	*bp)
{
	/*
	 * Clear any error hanging over from reading the buffer. This prevents
	 * subsequent reads after this write from seeing stale errors.
	 */
	bp->b_error = 0;
	bp->b_flags &= ~LIBXFS_B_STALE;
	bp->b_flags |= LIBXFS_B_DIRTY;
}

/* Prepare a buffer to be sent to the MRU list. */
static inline void
libxfs_buf_prepare_mru(
	struct xfs_buf		*bp)
{
	if (bp->b_pag)
		xfs_perag_put(bp->b_pag);
	bp->b_pag = NULL;

	if (!(bp->b_flags & LIBXFS_B_DIRTY))
		return;

	/* Complain about (and remember) dropping dirty buffers. */
	fprintf(stderr, _("%s: Releasing dirty buffer to free list!\n"),
			progname);

	if (bp->b_error == -EFSCORRUPTED)
		bp->b_target->flags |= XFS_BUFTARG_CORRUPT_WRITE;
	bp->b_target->flags |= XFS_BUFTARG_LOST_WRITE;
}

static void
libxfs_brelse(
	struct cache_node	*node)
{
	struct xfs_buf		*bp = container_of(node, struct xfs_buf,
						   b_node);

	if (!bp)
		return;
	libxfs_buf_prepare_mru(bp);

	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	list_add(&bp->b_node.cn_mru, &xfs_buf_freelist.cm_list);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);
}

static unsigned int
libxfs_bulkrelse(
	struct cache		*cache,
	struct list_head	*list)
{
	struct xfs_buf		*bp;
	int			count = 0;

	if (list_empty(list))
		return 0 ;

	list_for_each_entry(bp, list, b_node.cn_mru) {
		libxfs_buf_prepare_mru(bp);
		count++;
	}

	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	list_splice(list, &xfs_buf_freelist.cm_list);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);

	return count;
}

/*
 * Free everything from the xfs_buf_freelist MRU, used at final teardown
 */
void
libxfs_bcache_free(void)
{
	struct list_head	*cm_list;
	struct xfs_buf		*bp, *next;

	cm_list = &xfs_buf_freelist.cm_list;
	list_for_each_entry_safe(bp, next, cm_list, b_node.cn_mru) {
		free(bp->b_addr);
		if (bp->b_maps != &bp->__b_map)
			free(bp->b_maps);
		kmem_cache_free(xfs_buf_cache, bp);
	}
}

/*
 * When a buffer is marked dirty, the error is cleared. Hence if we are trying
 * to flush a buffer prior to cache reclaim that has an error on it it means
 * we've already tried to flush it and it failed. Prevent repeated corruption
 * errors from being reported by skipping such buffers - when the corruption is
 * fixed the buffer will be marked dirty again and we can write it again.
 */
static int
libxfs_bflush(
	struct cache_node	*node)
{
	struct xfs_buf		*bp = container_of(node, struct xfs_buf,
						   b_node);

	if (!bp->b_error && bp->b_flags & LIBXFS_B_DIRTY)
		return libxfs_bwrite(bp);
	return bp->b_error;
}

void
libxfs_bcache_purge(void)
{
	cache_purge(libxfs_bcache);
}

void
libxfs_bcache_flush(void)
{
	cache_flush(libxfs_bcache);
}

int
libxfs_bcache_overflowed(void)
{
	return cache_overflowed(libxfs_bcache);
}

struct cache_operations libxfs_bcache_operations = {
	.hash		= libxfs_bhash,
	.alloc		= libxfs_balloc,
	.flush		= libxfs_bflush,
	.relse		= libxfs_brelse,
	.compare	= libxfs_bcompare,
	.bulkrelse	= libxfs_bulkrelse
};

/*
 * Verify an on-disk magic value against the magic value specified in the
 * verifier structure. The verifier magic is in disk byte order so the caller is
 * expected to pass the value directly from disk.
 */
bool
xfs_verify_magic(
	struct xfs_buf		*bp,
	__be32			dmagic)
{
	struct xfs_mount	*mp = bp->b_mount;
	int			idx;

	idx = xfs_has_crc(mp);
	if (unlikely(WARN_ON(!bp->b_ops || !bp->b_ops->magic[idx])))
		return false;
	return dmagic == bp->b_ops->magic[idx];
}

/*
 * Verify an on-disk magic value against the magic value specified in the
 * verifier structure. The verifier magic is in disk byte order so the caller is
 * expected to pass the value directly from disk.
 */
bool
xfs_verify_magic16(
	struct xfs_buf		*bp,
	__be16			dmagic)
{
	struct xfs_mount	*mp = bp->b_mount;
	int			idx;

	idx = xfs_has_crc(mp);
	if (unlikely(WARN_ON(!bp->b_ops || !bp->b_ops->magic16[idx])))
		return false;
	return dmagic == bp->b_ops->magic16[idx];
}

/*
 * Inode cache stubs.
 */

struct kmem_cache		*xfs_inode_cache;
extern struct kmem_cache	*xfs_ili_cache;

int
libxfs_iget(
	struct xfs_mount	*mp,
	struct xfs_trans	*tp,
	xfs_ino_t		ino,
	uint			lock_flags,
	struct xfs_inode	**ipp)
{
	struct xfs_inode	*ip;
	struct xfs_buf		*bp;
	struct xfs_perag	*pag;
	int			error = 0;

	/* reject inode numbers outside existing AGs */
	if (!ino || XFS_INO_TO_AGNO(mp, ino) >= mp->m_sb.sb_agcount)
		return -EINVAL;

	ip = kmem_cache_zalloc(xfs_inode_cache, 0);
	if (!ip)
		return -ENOMEM;

	VFS_I(ip)->i_count = 1;
	ip->i_ino = ino;
	ip->i_mount = mp;
	ip->i_af.if_format = XFS_DINODE_FMT_EXTENTS;
	spin_lock_init(&VFS_I(ip)->i_lock);

	pag = xfs_perag_get(mp, XFS_INO_TO_AGNO(mp, ip->i_ino));
	error = xfs_imap(pag, tp, ip->i_ino, &ip->i_imap, 0);
	xfs_perag_put(pag);

	if (error)
		goto out_destroy;

	error = xfs_imap_to_bp(mp, tp, &ip->i_imap, &bp);
	if (error)
		goto out_destroy;

	error = xfs_inode_from_disk(ip,
			xfs_buf_offset(bp, ip->i_imap.im_boffset));
	if (!error)
		xfs_buf_set_ref(bp, XFS_INO_REF);
	xfs_trans_brelse(tp, bp);

	if (error)
		goto out_destroy;

	*ipp = ip;
	return 0;

out_destroy:
	kmem_cache_free(xfs_inode_cache, ip);
	*ipp = NULL;
	return error;
}

static void
libxfs_idestroy(xfs_inode_t *ip)
{
	switch (VFS_I(ip)->i_mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFLNK:
			libxfs_idestroy_fork(&ip->i_df);
			break;
	}

	libxfs_ifork_zap_attr(ip);

	if (ip->i_cowfp) {
		libxfs_idestroy_fork(ip->i_cowfp);
		kmem_cache_free(xfs_ifork_cache, ip->i_cowfp);
	}
}

void
libxfs_irele(
	struct xfs_inode	*ip)
{
	VFS_I(ip)->i_count--;

	if (VFS_I(ip)->i_count == 0) {
		ASSERT(ip->i_itemp == NULL);
		libxfs_idestroy(ip);
		kmem_cache_free(xfs_inode_cache, ip);
	}
}

/*
 * Flush everything dirty in the kernel and disk write caches to stable media.
 * Returns 0 for success or a negative error code.
 */
int
libxfs_blkdev_issue_flush(
	struct xfs_buftarg	*btp)
{
	int			fd, ret;

	if (btp->bt_bdev == 0)
		return 0;

	fd = libxfs_device_to_fd(btp->bt_bdev);
	ret = platform_flush_device(fd, btp->bt_bdev);
	return ret ? -errno : 0;
}

/*
 * Write out a buffer list synchronously.
 *
 * This will take the @buffer_list, write all buffers out and wait for I/O
 * completion on all of the buffers. @buffer_list is consumed by the function,
 * so callers must have some other way of tracking buffers if they require such
 * functionality.
 */
int
xfs_buf_delwri_submit(
	struct list_head	*buffer_list)
{
	struct xfs_buf		*bp, *n;
	int			error = 0, error2;

	list_for_each_entry_safe(bp, n, buffer_list, b_list) {
		list_del_init(&bp->b_list);
		error2 = libxfs_bwrite(bp);
		if (!error)
			error = error2;
		libxfs_buf_relse(bp);
	}

	return error;
}

/*
 * Cancel a delayed write list.
 *
 * Remove each buffer from the list, clear the delwri queue flag and drop the
 * associated buffer reference.
 */
void
xfs_buf_delwri_cancel(
	struct list_head	*list)
{
	struct xfs_buf		*bp;

	while (!list_empty(list)) {
		bp = list_first_entry(list, struct xfs_buf, b_list);

		list_del_init(&bp->b_list);
		libxfs_buf_relse(bp);
	}
}

/*
 * Format the log. The caller provides either a buftarg which is used to access
 * the log via buffers or a direct pointer to a buffer that encapsulates the
 * entire log.
 */
int
libxfs_log_clear(
	struct xfs_buftarg	*btp,
	char			*dptr,
	xfs_daddr_t		start,
	uint			length,		/* basic blocks */
	uuid_t			*fs_uuid,
	int			version,
	int			sunit,		/* bytes */
	int			fmt,
	int			cycle,
	bool			max)
{
	struct xfs_buf		*bp = NULL;
	int			len;
	xfs_lsn_t		lsn;
	xfs_lsn_t		tail_lsn;
	xfs_daddr_t		blk;
	xfs_daddr_t		end_blk;
	char			*ptr;

	if (((btp && dptr) || (!btp && !dptr)) ||
	    (btp && !btp->bt_bdev) || !fs_uuid)
		return -EINVAL;

	/* first zero the log */
	if (btp)
		libxfs_device_zero(btp, start, length);
	else
		memset(dptr, 0, BBTOB(length));

	/*
	 * Initialize the log record length and LSNs. XLOG_INIT_CYCLE is a
	 * special reset case where we only write a single record where the lsn
	 * and tail_lsn match. Otherwise, the record lsn starts at block 0 of
	 * the specified cycle and points tail_lsn at the last record of the
	 * previous cycle.
	 */
	len = ((version == 2) && sunit) ? BTOBB(sunit) : 2;
	len = max(len, 2);
	lsn = xlog_assign_lsn(cycle, 0);
	if (cycle == XLOG_INIT_CYCLE)
		tail_lsn = lsn;
	else
		tail_lsn = xlog_assign_lsn(cycle - 1, length - len);

	/* write out the first log record */
	ptr = dptr;
	if (btp) {
		bp = libxfs_getbufr_uncached(btp, start, len);
		ptr = bp->b_addr;
	}
	libxfs_log_header(ptr, fs_uuid, version, sunit, fmt, lsn, tail_lsn,
			  next, bp);
	if (bp) {
		libxfs_buf_mark_dirty(bp);
		libxfs_buf_relse(bp);
	}

	/*
	 * There's nothing else to do if this is a log reset. The kernel detects
	 * the rest of the log is zeroed and starts at cycle 1.
	 */
	if (cycle == XLOG_INIT_CYCLE)
		return 0;

	/*
	 * Bump the record size for a full log format if the caller allows it.
	 * This is primarily for performance reasons and most callers don't care
	 * about record size since the log is clean after we're done.
	 */
	if (max)
		len = BTOBB(BDSTRAT_SIZE);

	/*
	 * Otherwise, fill everything beyond the initial record with records of
	 * the previous cycle so the kernel head/tail detection works correctly.
	 *
	 * We don't particularly care about the record size or content here.
	 * It's only important that the headers are in place such that the
	 * kernel finds 1.) a clean log and 2.) the correct current cycle value.
	 * Therefore, bump up the record size to the max to use larger I/Os and
	 * improve performance.
	 */
	cycle--;
	blk = start + len;
	if (dptr)
		dptr += BBTOB(len);
	end_blk = start + length;

	len = min(end_blk - blk, len);
	while (blk < end_blk) {
		lsn = xlog_assign_lsn(cycle, blk - start);
		tail_lsn = xlog_assign_lsn(cycle, blk - start - len);

		ptr = dptr;
		if (btp) {
			bp = libxfs_getbufr_uncached(btp, blk, len);
			ptr = bp->b_addr;
		}
		/*
		 * Note: pass the full buffer length as the sunit to initialize
		 * the entire buffer.
		 */
		libxfs_log_header(ptr, fs_uuid, version, BBTOB(len), fmt, lsn,
				  tail_lsn, next, bp);
		if (bp) {
			libxfs_buf_mark_dirty(bp);
			libxfs_buf_relse(bp);
		}

		blk += len;
		if (dptr)
			dptr += BBTOB(len);
		len = min(end_blk - blk, len);
	}

	return 0;
}

int
libxfs_log_header(
	char			*caddr,
	uuid_t			*fs_uuid,
	int			version,
	int			sunit,
	int			fmt,
	xfs_lsn_t		lsn,
	xfs_lsn_t		tail_lsn,
	libxfs_get_block_t	*nextfunc,
	void			*private)
{
	xlog_rec_header_t	*head = (xlog_rec_header_t *)caddr;
	char			*p = caddr;
	__be32			cycle_lsn;
	int			i, len;
	int			hdrs = 1;

	if (lsn == NULLCOMMITLSN)
		lsn = xlog_assign_lsn(XLOG_INIT_CYCLE, 0);
	if (tail_lsn == NULLCOMMITLSN)
		tail_lsn = lsn;

	len = ((version == 2) && sunit) ? BTOBB(sunit) : 1;

	memset(p, 0, BBSIZE);
	head->h_magicno = cpu_to_be32(XLOG_HEADER_MAGIC_NUM);
	head->h_cycle = cpu_to_be32(CYCLE_LSN(lsn));
	head->h_version = cpu_to_be32(version);
	head->h_crc = cpu_to_le32(0);
	head->h_prev_block = cpu_to_be32(-1);
	head->h_num_logops = cpu_to_be32(1);
	head->h_fmt = cpu_to_be32(fmt);
	head->h_size = cpu_to_be32(max(sunit, XLOG_BIG_RECORD_BSIZE));

	head->h_lsn = cpu_to_be64(lsn);
	head->h_tail_lsn = cpu_to_be64(tail_lsn);

	memcpy(&head->h_fs_uuid, fs_uuid, sizeof(uuid_t));

	/*
	 * The kernel expects to see either a log record header magic value or
	 * the LSN cycle at the top of every log block. The first word of each
	 * non-header block is copied to the record headers and replaced with
	 * the cycle value (see xlog_[un]pack_data() and xlog_get_cycle() for
	 * details).
	 *
	 * Even though we only ever write an unmount record (one block), we
	 * support writing log records up to the max log buffer size of 256k to
	 * improve log format performance. This means a record can require up
	 * to 8 headers (1 rec. header + 7 ext. headers) for the packed cycle
	 * data (each header supports 32k of data).
	 */
	cycle_lsn = CYCLE_LSN_DISK(head->h_lsn);
	if (version == 2 && sunit > XLOG_HEADER_CYCLE_SIZE) {
		hdrs = sunit / XLOG_HEADER_CYCLE_SIZE;
		if (sunit % XLOG_HEADER_CYCLE_SIZE)
			hdrs++;
	}

	/*
	 * A fixed number of extended headers is expected based on h_size. If
	 * required, format those now so the unmount record is located
	 * correctly.
	 *
	 * Since we only write an unmount record, we only need one h_cycle_data
	 * entry for the unmount record block. The subsequent record data
	 * blocks are zeroed, which means we can stamp them directly with the
	 * cycle and zero the rest of the cycle data in the extended headers.
	 */
	if (hdrs > 1) {
		for (i = 1; i < hdrs; i++) {
			p = nextfunc(p, BBSIZE, private);
			memset(p, 0, BBSIZE);
			/* xlog_rec_ext_header.xh_cycle */
			*(__be32 *)p = cycle_lsn;
		}
	}

	/*
	 * The total length is the max of the stripe unit or 2 basic block
	 * minimum (1 hdr blk + 1 data blk). The record length is the total
	 * minus however many header blocks are required.
	 */
	head->h_len = cpu_to_be32(max(BBTOB(2), sunit) - hdrs * BBSIZE);

	/*
	 * Write out the unmount record, pack the first word into the record
	 * header and stamp the block with the cycle.
	 */
	p = nextfunc(p, BBSIZE, private);
	unmount_record(p);

	head->h_cycle_data[0] = *(__be32 *)p;
	*(__be32 *)p = cycle_lsn;

	/*
	 * Finally, zero all remaining blocks in the record and stamp each with
	 * the cycle. We don't need to pack any of these blocks because the
	 * cycle data in the headers has already been zeroed.
	 */
	len = max(len, hdrs + 1);
	for (i = hdrs + 1; i < len; i++) {
		p = nextfunc(p, BBSIZE, private);
		memset(p, 0, BBSIZE);
		*(__be32 *)p = cycle_lsn;
	}

	return BBTOB(len);
}

void
libxfs_buf_set_priority(
	struct xfs_buf	*bp,
	int		priority)
{
	cache_node_set_priority(libxfs_bcache, &bp->b_node, priority);
}

int
libxfs_buf_priority(
	struct xfs_buf	*bp)
{
	return cache_node_get_priority(&bp->b_node);
}

/*
 * Log a message about and stale a buffer that a caller has decided is corrupt.
 *
 * This function should be called for the kinds of metadata corruption that
 * cannot be detect from a verifier, such as incorrect inter-block relationship
 * data.  Do /not/ call this function from a verifier function.
 *
 * The buffer must be XBF_DONE prior to the call.  Afterwards, the buffer will
 * be marked stale, but b_error will not be set.  The caller is responsible for
 * releasing the buffer or fixing it.
 */
void
__xfs_buf_mark_corrupt(
	struct xfs_buf		*bp,
	xfs_failaddr_t		fa)
{
	ASSERT(bp->b_flags & XBF_DONE);

	xfs_buf_corruption_error(bp, fa);
	xfs_buf_stale(bp);
}
