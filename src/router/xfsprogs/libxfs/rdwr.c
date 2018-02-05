/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "libxfs.h"		/* for LIBXFS_EXIT_ON_FAILURE */

/*
 * Important design/architecture note:
 *
 * The userspace code that uses the buffer cache is much less constrained than
 * the kernel code. The userspace code is pretty nasty in places, especially
 * when it comes to buffer error handling.  Very little of the userspace code
 * outside libxfs clears bp->b_error - very little code even checks it - so the
 * libxfs code is tripping on stale errors left by the userspace code.
 *
 * We can't clear errors or zero buffer contents in libxfs_getbuf-* like we do
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
 * libxfs_getbuf/libxfs_readbuf.
 *
 * The result of this is that until the userspace code outside libxfs is cleaned
 * up, functions that release buffers from userspace control (i.e
 * libxfs_writebuf/libxfs_putbuf) need to zero bp->b_error to prevent
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
	char		*z;
	int		fd;

	zsize = min(BDSTRAT_SIZE, BBTOB(len));
	if ((z = memalign(libxfs_device_alignment(), zsize)) == NULL) {
		fprintf(stderr,
			_("%s: %s can't memalign %d bytes: %s\n"),
			progname, __FUNCTION__, (int)zsize, strerror(errno));
		exit(1);
	}
	memset(z, 0, zsize);

	fd = libxfs_device_to_fd(btp->dev);
	start_offset = LIBXFS_BBTOOFF64(start);

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
	    (XFS_BUF_COUNT(buf) < (int)(ptr - XFS_BUF_PTR(buf)) + offset))
		abort();

	return ptr + offset;
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
	    (btp && !btp->dev) || !fs_uuid)
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
	len = MAX(len, 2);
	lsn = xlog_assign_lsn(cycle, 0);
	if (cycle == XLOG_INIT_CYCLE)
		tail_lsn = lsn;
	else
		tail_lsn = xlog_assign_lsn(cycle - 1, length - len);

	/* write out the first log record */
	ptr = dptr;
	if (btp) {
		bp = libxfs_getbufr(btp, start, len);
		ptr = XFS_BUF_PTR(bp);
	}
	libxfs_log_header(ptr, fs_uuid, version, sunit, fmt, lsn, tail_lsn,
			  next, bp);
	if (bp) {
		bp->b_flags |= LIBXFS_B_DIRTY;
		libxfs_putbufr(bp);
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
			bp = libxfs_getbufr(btp, blk, len);
			ptr = XFS_BUF_PTR(bp);
		}
		/*
		 * Note: pass the full buffer length as the sunit to initialize
		 * the entire buffer.
		 */
		libxfs_log_header(ptr, fs_uuid, version, BBTOB(len), fmt, lsn,
				  tail_lsn, next, bp);
		if (bp) {
			bp->b_flags |= LIBXFS_B_DIRTY;
			libxfs_putbufr(bp);
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
	head->h_size = cpu_to_be32(MAX(sunit, XLOG_BIG_RECORD_BSIZE));

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
	head->h_len = cpu_to_be32(MAX(BBTOB(2), sunit) - hdrs * BBSIZE);

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
	len = MAX(len, hdrs + 1);
	for (i = hdrs + 1; i < len; i++) {
		p = nextfunc(p, BBSIZE, private);
		memset(p, 0, BBSIZE);
		*(__be32 *)p = cycle_lsn;
	}

	return BBTOB(len);
}

/*
 * Simple I/O (buffer cache) interface
 */


#ifdef XFS_BUF_TRACING

#undef libxfs_readbuf
#undef libxfs_readbuf_map
#undef libxfs_writebuf
#undef libxfs_getbuf
#undef libxfs_getbuf_map
#undef libxfs_getbuf_flags
#undef libxfs_putbuf

xfs_buf_t	*libxfs_readbuf(struct xfs_buftarg *, xfs_daddr_t, int, int,
				const struct xfs_buf_ops *);
xfs_buf_t	*libxfs_readbuf_map(struct xfs_buftarg *, struct xfs_buf_map *,
				int, int, const struct xfs_buf_ops *);
int		libxfs_writebuf(xfs_buf_t *, int);
xfs_buf_t	*libxfs_getbuf(struct xfs_buftarg *, xfs_daddr_t, int);
xfs_buf_t	*libxfs_getbuf_map(struct xfs_buftarg *, struct xfs_buf_map *,
				int, int);
xfs_buf_t	*libxfs_getbuf_flags(struct xfs_buftarg *, xfs_daddr_t, int,
				unsigned int);
void		libxfs_putbuf (xfs_buf_t *);

#define	__add_trace(bp, func, file, line)	\
do {						\
	if (bp) {				\
		(bp)->b_func = (func);		\
		(bp)->b_file = (file);		\
		(bp)->b_line = (line);		\
	}					\
} while (0)

xfs_buf_t *
libxfs_trace_readbuf(const char *func, const char *file, int line,
		struct xfs_buftarg *btp, xfs_daddr_t blkno, int len, int flags,
		const struct xfs_buf_ops *ops)
{
	xfs_buf_t	*bp = libxfs_readbuf(btp, blkno, len, flags, ops);
	__add_trace(bp, func, file, line);
	return bp;
}

xfs_buf_t *
libxfs_trace_readbuf_map(const char *func, const char *file, int line,
		struct xfs_buftarg *btp, struct xfs_buf_map *map, int nmaps, int flags,
		const struct xfs_buf_ops *ops)
{
	xfs_buf_t	*bp = libxfs_readbuf_map(btp, map, nmaps, flags, ops);
	__add_trace(bp, func, file, line);
	return bp;
}

int
libxfs_trace_writebuf(const char *func, const char *file, int line, xfs_buf_t *bp, int flags)
{
	__add_trace(bp, func, file, line);
	return libxfs_writebuf(bp, flags);
}

xfs_buf_t *
libxfs_trace_getbuf(const char *func, const char *file, int line,
		struct xfs_buftarg *btp, xfs_daddr_t blkno, int len)
{
	xfs_buf_t	*bp = libxfs_getbuf(btp, blkno, len);
	__add_trace(bp, func, file, line);
	return bp;
}

xfs_buf_t *
libxfs_trace_getbuf_map(const char *func, const char *file, int line,
		struct xfs_buftarg *btp, struct xfs_buf_map *map, int nmaps,
		int flags)
{
	xfs_buf_t	*bp = libxfs_getbuf_map(btp, map, nmaps, flags);
	__add_trace(bp, func, file, line);
	return bp;
}

xfs_buf_t *
libxfs_trace_getbuf_flags(const char *func, const char *file, int line,
		struct xfs_buftarg *btp, xfs_daddr_t blkno, int len, unsigned int flags)
{
	xfs_buf_t	*bp = libxfs_getbuf_flags(btp, blkno, len, flags);
	__add_trace(bp, func, file, line);
	return bp;
}

void
libxfs_trace_putbuf(const char *func, const char *file, int line, xfs_buf_t *bp)
{
	__add_trace(bp, func, file, line);
	libxfs_putbuf(bp);
}


#endif


xfs_buf_t *
libxfs_getsb(xfs_mount_t *mp, int flags)
{
	return libxfs_readbuf(mp->m_ddev_targp, XFS_SB_DADDR,
				XFS_FSS_TO_BB(mp, 1), flags, &xfs_sb_buf_ops);
}

kmem_zone_t			*xfs_buf_zone;

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
	struct xfs_buf	*bp = (struct xfs_buf *)node;
	struct xfs_bufkey *bkey = (struct xfs_bufkey *)key;

	if (bp->b_target->dev == bkey->buftarg->dev &&
	    bp->b_bn == bkey->blkno) {
		if (bp->b_bcount == BBTOB(bkey->bblen))
			return CACHE_HIT;
#ifdef IO_BCOMPARE_CHECK
		if (!(libxfs_bcache->c_flags & CACHE_MISCOMPARE_PURGE)) {
			fprintf(stderr,
	"%lx: Badness in key lookup (length)\n"
	"bp=(bno 0x%llx, len %u bytes) key=(bno 0x%llx, len %u bytes)\n",
				pthread_self(),
				(unsigned long long)bp->b_bn, (int)bp->b_bcount,
				(unsigned long long)bkey->blkno,
				BBTOB(bkey->bblen));
		}
#endif
		return CACHE_PURGE;
	}
	return CACHE_MISS;
}

void
libxfs_bprint(xfs_buf_t *bp)
{
	fprintf(stderr, "Buffer 0x%p blkno=%llu bytes=%u flags=0x%x count=%u\n",
		bp, (unsigned long long)bp->b_bn, (unsigned)bp->b_bcount,
		bp->b_flags, bp->b_node.cn_count);
}

static void
__initbuf(xfs_buf_t *bp, struct xfs_buftarg *btp, xfs_daddr_t bno,
		unsigned int bytes)
{
	bp->b_flags = 0;
	bp->b_bn = bno;
	bp->b_bcount = bytes;
	bp->b_length = BTOBB(bytes);
	bp->b_target = btp;
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
#ifdef XFS_BUF_TRACING
	list_head_init(&bp->b_lock_list);
#endif
	pthread_mutex_init(&bp->b_lock, NULL);
	bp->b_holder = 0;
	bp->b_recur = 0;
	bp->b_ops = NULL;

	if (!bp->b_maps) {
		bp->b_nmaps = 1;
		bp->b_maps = &bp->__b_map;
		bp->b_maps[0].bm_bn = bp->b_bn;
		bp->b_maps[0].bm_len = bp->b_length;
	}
}

static void
libxfs_initbuf(xfs_buf_t *bp, struct xfs_buftarg *btp, xfs_daddr_t bno,
		unsigned int bytes)
{
	__initbuf(bp, btp, bno, bytes);
}

static void
libxfs_initbuf_map(xfs_buf_t *bp, struct xfs_buftarg *btp,
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

xfs_buf_t *
__libxfs_getbufr(int blen)
{
	xfs_buf_t	*bp;

	/*
	 * first look for a buffer that can be used as-is,
	 * if one cannot be found, see if there is a buffer,
	 * and if so, free its buffer and set b_addr to NULL
	 * before calling libxfs_initbuf.
	 */
	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	if (!list_empty(&xfs_buf_freelist.cm_list)) {
		list_for_each_entry(bp, &xfs_buf_freelist.cm_list, b_node.cn_mru) {
			if (bp->b_bcount == blen) {
				list_del_init(&bp->b_node.cn_mru);
				break;
			}
		}
		if (&bp->b_node.cn_mru == &xfs_buf_freelist.cm_list) {
			bp = list_entry(xfs_buf_freelist.cm_list.next,
					xfs_buf_t, b_node.cn_mru);
			list_del_init(&bp->b_node.cn_mru);
			free(bp->b_addr);
			bp->b_addr = NULL;
			if (bp->b_maps != &bp->__b_map)
				free(bp->b_maps);
			bp->b_maps = NULL;
		}
	} else
		bp = kmem_zone_zalloc(xfs_buf_zone, 0);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);
	bp->b_ops = NULL;
	if (bp->b_flags & LIBXFS_B_DIRTY)
		fprintf(stderr, "found dirty buffer (bulk) on free list!");

	return bp;
}

xfs_buf_t *
libxfs_getbufr(struct xfs_buftarg *btp, xfs_daddr_t blkno, int bblen)
{
	xfs_buf_t	*bp;
	int		blen = BBTOB(bblen);

	bp =__libxfs_getbufr(blen);
	if (bp)
		libxfs_initbuf(bp, btp, blkno, blen);
#ifdef IO_DEBUG
	printf("%lx: %s: allocated %u bytes buffer, key=0x%llx(0x%llx), %p\n",
		pthread_self(), __FUNCTION__, blen,
		(long long)LIBXFS_BBTOOFF64(blkno), (long long)blkno, bp);
#endif

	return bp;
}

xfs_buf_t *
libxfs_getbufr_map(struct xfs_buftarg *btp, xfs_daddr_t blkno, int bblen,
		struct xfs_buf_map *map, int nmaps)
{
	xfs_buf_t	*bp;
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
#ifdef IO_DEBUG
	printf("%lx: %s: allocated %u bytes buffer, key=0x%llx(0x%llx), %p\n",
		pthread_self(), __FUNCTION__, blen,
		(long long)LIBXFS_BBTOOFF64(blkno), (long long)blkno, bp);
#endif

	return bp;
}

#ifdef XFS_BUF_TRACING
struct list_head	lock_buf_list = {&lock_buf_list, &lock_buf_list};
int			lock_buf_count = 0;
#endif

extern int     use_xfs_buf_lock;

static struct xfs_buf *
__cache_lookup(struct xfs_bufkey *key, unsigned int flags)
{
	struct xfs_buf	*bp;

	cache_node_get(libxfs_bcache, key, (struct cache_node **)&bp);
	if (!bp)
		return NULL;

	if (use_xfs_buf_lock) {
		int ret;

		ret = pthread_mutex_trylock(&bp->b_lock);
		if (ret) {
			ASSERT(ret == EAGAIN);
			if (flags & LIBXFS_GETBUF_TRYLOCK)
				goto out_put;

			if (pthread_equal(bp->b_holder, pthread_self())) {
				fprintf(stderr,
	_("Warning: recursive buffer locking at block %" PRIu64 " detected\n"),
					key->blkno);
				bp->b_recur++;
				return bp;
			} else {
				pthread_mutex_lock(&bp->b_lock);
			}
		}

		bp->b_holder = pthread_self();
	}

	cache_node_set_priority(libxfs_bcache, (struct cache_node *)bp,
		cache_node_get_priority((struct cache_node *)bp) -
						CACHE_PREFETCH_PRIORITY);
#ifdef XFS_BUF_TRACING
	pthread_mutex_lock(&libxfs_bcache->c_mutex);
	lock_buf_count++;
	list_add(&bp->b_lock_list, &lock_buf_list);
	pthread_mutex_unlock(&libxfs_bcache->c_mutex);
#endif
#ifdef IO_DEBUG
	printf("%lx %s: hit buffer %p for bno = 0x%llx/0x%llx\n",
		pthread_self(), __FUNCTION__,
		bp, bp->b_bn, (long long)LIBXFS_BBTOOFF64(key->blkno));
#endif

	return bp;
out_put:
	cache_node_put(libxfs_bcache, (struct cache_node *)bp);
	return NULL;
}

struct xfs_buf *
libxfs_getbuf_flags(struct xfs_buftarg *btp, xfs_daddr_t blkno, int len,
		unsigned int flags)
{
	struct xfs_bufkey key = {0};

	key.buftarg = btp;
	key.blkno = blkno;
	key.bblen = len;

	return __cache_lookup(&key, flags);
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

struct xfs_buf *
libxfs_getbuf(struct xfs_buftarg *btp, xfs_daddr_t blkno, int len)
{
	struct xfs_buf	*bp;

	bp = libxfs_getbuf_flags(btp, blkno, len, 0);
	reset_buf_state(bp);
	return bp;
}

static struct xfs_buf *
__libxfs_getbuf_map(struct xfs_buftarg *btp, struct xfs_buf_map *map,
		    int nmaps, int flags)
{
	struct xfs_bufkey key = {0};
	int i;

	if (nmaps == 1)
		return libxfs_getbuf_flags(btp, map[0].bm_bn, map[0].bm_len,
					   flags);

	key.buftarg = btp;
	key.blkno = map[0].bm_bn;
	for (i = 0; i < nmaps; i++) {
		key.bblen += map[i].bm_len;
	}
	key.map = map;
	key.nmaps = nmaps;

	return __cache_lookup(&key, flags);
}

struct xfs_buf *
libxfs_getbuf_map(struct xfs_buftarg *btp, struct xfs_buf_map *map,
		  int nmaps, int flags)
{
	struct xfs_buf	*bp;

	bp = __libxfs_getbuf_map(btp, map, nmaps, flags);
	reset_buf_state(bp);
	return bp;
}

void
libxfs_putbuf(xfs_buf_t *bp)
{
	/*
	 * ensure that any errors on this use of the buffer don't carry
	 * over to the next user.
	 */
	bp->b_error = 0;

#ifdef XFS_BUF_TRACING
	pthread_mutex_lock(&libxfs_bcache->c_mutex);
	lock_buf_count--;
	ASSERT(lock_buf_count >= 0);
	list_del_init(&bp->b_lock_list);
	pthread_mutex_unlock(&libxfs_bcache->c_mutex);
#endif
	if (use_xfs_buf_lock) {
		if (bp->b_recur) {
			bp->b_recur--;
		} else {
			bp->b_holder = 0;
			pthread_mutex_unlock(&bp->b_lock);
		}
	}

	cache_node_put(libxfs_bcache, (struct cache_node *)bp);
}

void
libxfs_purgebuf(xfs_buf_t *bp)
{
	struct xfs_bufkey key = {0};

	key.buftarg = bp->b_target;
	key.blkno = bp->b_bn;
	key.bblen = bp->b_length;

	cache_node_purge(libxfs_bcache, &key, (struct cache_node *)bp);
}

static struct cache_node *
libxfs_balloc(cache_key_t key)
{
	struct xfs_bufkey *bufkey = (struct xfs_bufkey *)key;

	if (bufkey->map)
		return (struct cache_node *)
		       libxfs_getbufr_map(bufkey->buftarg,
					  bufkey->blkno, bufkey->bblen,
					  bufkey->map, bufkey->nmaps);
	return (struct cache_node *)libxfs_getbufr(bufkey->buftarg,
					  bufkey->blkno, bufkey->bblen);
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
		if (flags & LIBXFS_EXIT_ON_FAILURE)
			exit(1);
		return -error;
	} else if (sts != len) {
		fprintf(stderr, _("%s: error - read only %d of %d bytes\n"),
			progname, sts, len);
		if (flags & LIBXFS_EXIT_ON_FAILURE)
			exit(1);
		return -EIO;
	}
	return 0;
}

int
libxfs_readbufr(struct xfs_buftarg *btp, xfs_daddr_t blkno, xfs_buf_t *bp,
		int len, int flags)
{
	int	fd = libxfs_device_to_fd(btp->dev);
	int	bytes = BBTOB(len);
	int	error;

	ASSERT(BBTOB(len) <= bp->b_bcount);

	error = __read_buf(fd, bp->b_addr, bytes, LIBXFS_BBTOOFF64(blkno), flags);
	if (!error &&
	    bp->b_target->dev == btp->dev &&
	    bp->b_bn == blkno &&
	    bp->b_bcount == bytes)
		bp->b_flags |= LIBXFS_B_UPTODATE;
#ifdef IO_DEBUG
	printf("%lx: %s: read %u bytes, error %d, blkno=0x%llx(0x%llx), %p\n",
		pthread_self(), __FUNCTION__, bytes, error,
		(long long)LIBXFS_BBTOOFF64(blkno), (long long)blkno, bp);
#endif
	return error;
}

void
libxfs_readbuf_verify(struct xfs_buf *bp, const struct xfs_buf_ops *ops)
{
	if (!ops)
		return;
	bp->b_ops = ops;
	bp->b_ops->verify_read(bp);
	bp->b_flags &= ~LIBXFS_B_UNCHECKED;
}


xfs_buf_t *
libxfs_readbuf(struct xfs_buftarg *btp, xfs_daddr_t blkno, int len, int flags,
		const struct xfs_buf_ops *ops)
{
	xfs_buf_t	*bp;
	int		error;

	bp = libxfs_getbuf_flags(btp, blkno, len, 0);
	if (!bp)
		return NULL;

	/*
	 * if the buffer was prefetched, it is likely that it was not validated.
	 * Hence if we are supplied an ops function and the buffer is marked as
	 * unchecked, we need to validate it now.
	 *
	 * We do this verification even if the buffer is dirty - the
	 * verification is almost certainly going to fail the CRC check in this
	 * case as a dirty buffer has not had the CRC recalculated. However, we
	 * should not be dirtying unchecked buffers and therefore failing it
	 * here because it's dirty and unchecked indicates we've screwed up
	 * somewhere else.
	 */
	bp->b_error = 0;
	if ((bp->b_flags & (LIBXFS_B_UPTODATE|LIBXFS_B_DIRTY))) {
		if (bp->b_flags & LIBXFS_B_UNCHECKED)
			libxfs_readbuf_verify(bp, ops);
		return bp;
	}

	/*
	 * Set the ops on a cache miss (i.e. first physical read) as the
	 * verifier may change the ops to match the type of buffer it contains.
	 * A cache hit might reset the verifier to the original type if we set
	 * it again, but it won't get called again and set to match the buffer
	 * contents. *cough* xfs_da_node_buf_ops *cough*.
	 */
	error = libxfs_readbufr(btp, blkno, bp, len, flags);
	if (error)
		bp->b_error = error;
	else
		libxfs_readbuf_verify(bp, ops);
	return bp;
}

int
libxfs_readbufr_map(struct xfs_buftarg *btp, struct xfs_buf *bp, int flags)
{
	int	fd;
	int	error = 0;
	char	*buf;
	int	i;

	fd = libxfs_device_to_fd(btp->dev);
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
#ifdef IO_DEBUG
	printf("%lx: %s: read %lu bytes, error %d, blkno=%llu(%llu), %p\n",
		pthread_self(), __FUNCTION__, buf - (char *)bp->b_addr, error,
		(long long)LIBXFS_BBTOOFF64(bp->b_bn), (long long)bp->b_bn, bp);
#endif
	return error;
}

struct xfs_buf *
libxfs_readbuf_map(struct xfs_buftarg *btp, struct xfs_buf_map *map, int nmaps,
		int flags, const struct xfs_buf_ops *ops)
{
	struct xfs_buf	*bp;
	int		error = 0;

	if (nmaps == 1)
		return libxfs_readbuf(btp, map[0].bm_bn, map[0].bm_len,
					flags, ops);

	bp = __libxfs_getbuf_map(btp, map, nmaps, 0);
	if (!bp)
		return NULL;

	bp->b_error = 0;
	if ((bp->b_flags & (LIBXFS_B_UPTODATE|LIBXFS_B_DIRTY))) {
		if (bp->b_flags & LIBXFS_B_UNCHECKED)
			libxfs_readbuf_verify(bp, ops);
		return bp;
	}
	error = libxfs_readbufr_map(btp, bp, flags);
	if (!error)
		libxfs_readbuf_verify(bp, ops);

#ifdef IO_DEBUGX
	printf("%lx: %s: read %lu bytes, error %d, blkno=%llu(%llu), %p\n",
		pthread_self(), __FUNCTION__, buf - (char *)bp->b_addr, error,
		(long long)LIBXFS_BBTOOFF64(bp->b_bn), (long long)bp->b_bn, bp);
#endif
	return bp;
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
		if (flags & LIBXFS_B_EXIT)
			exit(1);
		return -error;
	} else if (sts != len) {
		fprintf(stderr, _("%s: error - pwrite only %d of %d bytes\n"),
			progname, sts, len);
		if (flags & LIBXFS_B_EXIT)
			exit(1);
		return -EIO;
	}
	return 0;
}

int
libxfs_writebufr(xfs_buf_t *bp)
{
	int	fd = libxfs_device_to_fd(bp->b_target->dev);

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
	_("%s: write verifer failed on %s bno 0x%llx/0x%x\n"),
				__func__, bp->b_ops->name,
				(long long)bp->b_bn, bp->b_bcount);
			return bp->b_error;
		}
	}

	if (!(bp->b_flags & LIBXFS_B_DISCONTIG)) {
		bp->b_error = __write_buf(fd, bp->b_addr, bp->b_bcount,
				    LIBXFS_BBTOOFF64(bp->b_bn), bp->b_flags);
	} else {
		int	i;
		char	*buf = bp->b_addr;

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

#ifdef IO_DEBUG
	printf("%lx: %s: wrote %u bytes, blkno=%llu(%llu), %p, error %d\n",
			pthread_self(), __FUNCTION__, bp->b_bcount,
			(long long)LIBXFS_BBTOOFF64(bp->b_bn),
			(long long)bp->b_bn, bp, bp->b_error);
#endif
	if (!bp->b_error) {
		bp->b_flags |= LIBXFS_B_UPTODATE;
		bp->b_flags &= ~(LIBXFS_B_DIRTY | LIBXFS_B_EXIT |
				 LIBXFS_B_UNCHECKED);
	}
	return bp->b_error;
}

int
libxfs_writebuf_int(xfs_buf_t *bp, int flags)
{
	/*
	 * Clear any error hanging over from reading the buffer. This prevents
	 * subsequent reads after this write from seeing stale errors.
	 */
	bp->b_error = 0;
	bp->b_flags &= ~LIBXFS_B_STALE;
	bp->b_flags |= (LIBXFS_B_DIRTY | flags);
	return 0;
}

int
libxfs_writebuf(xfs_buf_t *bp, int flags)
{
#ifdef IO_DEBUG
	printf("%lx: %s: dirty blkno=%llu(%llu)\n",
			pthread_self(), __FUNCTION__,
			(long long)LIBXFS_BBTOOFF64(bp->b_bn),
			(long long)bp->b_bn);
#endif
	/*
	 * Clear any error hanging over from reading the buffer. This prevents
	 * subsequent reads after this write from seeing stale errors.
	 */
	bp->b_error = 0;
	bp->b_flags &= ~LIBXFS_B_STALE;
	bp->b_flags |= (LIBXFS_B_DIRTY | flags);
	libxfs_putbuf(bp);
	return 0;
}

void
libxfs_iomove(xfs_buf_t *bp, uint boff, int len, void *data, int flags)
{
#ifdef IO_DEBUG
	if (boff + len > bp->b_bcount) {
		printf("Badness, iomove out of range!\n"
			"bp=(bno 0x%llx, bytes %u) range=(boff %u, bytes %u)\n",
			(long long)bp->b_bn, bp->b_bcount, boff, len);
		abort();
	}
#endif
	switch (flags) {
	case LIBXFS_BZERO:
		memset(bp->b_addr + boff, 0, len);
		break;
	case LIBXFS_BREAD:
		memcpy(data, bp->b_addr + boff, len);
		break;
	case LIBXFS_BWRITE:
		memcpy(bp->b_addr + boff, data, len);
		break;
	}
}

static void
libxfs_brelse(
	struct cache_node	*node)
{
	struct xfs_buf		*bp = (struct xfs_buf *)node;

	if (!bp)
		return;
	if (bp->b_flags & LIBXFS_B_DIRTY)
		fprintf(stderr,
			"releasing dirty buffer to free list!");

	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	list_add(&bp->b_node.cn_mru, &xfs_buf_freelist.cm_list);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);
}

static unsigned int
libxfs_bulkrelse(
	struct cache		*cache,
	struct list_head	*list)
{
	xfs_buf_t		*bp;
	int			count = 0;

	if (list_empty(list))
		return 0 ;

	list_for_each_entry(bp, list, b_node.cn_mru) {
		if (bp->b_flags & LIBXFS_B_DIRTY)
			fprintf(stderr,
				"releasing dirty buffer (bulk) to free list!");
		count++;
	}

	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	list_splice(list, &xfs_buf_freelist.cm_list);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);

	return count;
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
	struct xfs_buf		*bp = (struct xfs_buf *)node;

	if (!bp->b_error && bp->b_flags & LIBXFS_B_DIRTY)
		return libxfs_writebufr(bp);
	return bp->b_error;
}

void
libxfs_putbufr(xfs_buf_t *bp)
{
	if (bp->b_flags & LIBXFS_B_DIRTY)
		libxfs_writebufr(bp);
	libxfs_brelse((struct cache_node *)bp);
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
 * Inode cache stubs.
 */

extern kmem_zone_t	*xfs_ili_zone;
extern kmem_zone_t	*xfs_inode_zone;

int
libxfs_iget(xfs_mount_t *mp, xfs_trans_t *tp, xfs_ino_t ino, uint lock_flags,
		xfs_inode_t **ipp)
{
	xfs_inode_t	*ip;
	int		error = 0;

	ip = kmem_zone_zalloc(xfs_inode_zone, 0);
	if (!ip)
		return -ENOMEM;

	ip->i_ino = ino;
	ip->i_mount = mp;
	error = xfs_iread(mp, tp, ip, 0);
	if (error) {
		kmem_zone_free(xfs_inode_zone, ip);
		*ipp = NULL;
		return error;
	}

	/*
	 * set up the inode ops structure that the libxfs code relies on
	 */
	if (XFS_ISDIR(ip))
		ip->d_ops = mp->m_dir_inode_ops;
	else
		ip->d_ops = mp->m_nondir_inode_ops;

	*ipp = ip;
	return 0;
}

static void
libxfs_idestroy(xfs_inode_t *ip)
{
	switch (VFS_I(ip)->i_mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFLNK:
			libxfs_idestroy_fork(ip, XFS_DATA_FORK);
			break;
	}
	if (ip->i_afp)
		libxfs_idestroy_fork(ip, XFS_ATTR_FORK);
	if (ip->i_cowfp)
		xfs_idestroy_fork(ip, XFS_COW_FORK);
}

void
libxfs_iput(xfs_inode_t *ip)
{
	if (ip->i_itemp)
		kmem_zone_free(xfs_ili_zone, ip->i_itemp);
	ip->i_itemp = NULL;
	libxfs_idestroy(ip);
	kmem_zone_free(xfs_inode_zone, ip);
}
