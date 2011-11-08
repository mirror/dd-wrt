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

#include <xfs/libxfs.h>
#include <xfs/xfs_log.h>
#include <xfs/xfs_log_priv.h>
#include "init.h"

#define BDSTRAT_SIZE	(256 * 1024)
#define min(x, y)	((x) < (y) ? (x) : (y))

#define IO_BCOMPARE_CHECK

void
libxfs_device_zero(dev_t dev, xfs_daddr_t start, uint len)
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

	fd = libxfs_device_to_fd(dev);
	start_offset = LIBXFS_BBTOOFF64(start);

	if ((lseek64(fd, start_offset, SEEK_SET)) < 0) {
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
}

static void unmount_record(void *p)
{
	xlog_op_header_t	*op = (xlog_op_header_t *)p;
	/* the data section must be 32 bit size aligned */
	struct {
	    __uint16_t magic;
	    __uint16_t pad1;
	    __uint32_t pad2; /* may as well make it 64 bits */
	} magic = { XLOG_UNMOUNT_TYPE, 0, 0 };

	memset(p, 0, BBSIZE);
	op->oh_tid = cpu_to_be32(1);
	op->oh_len = cpu_to_be32(sizeof(magic));
	op->oh_clientid = XFS_LOG;
	op->oh_flags = XLOG_UNMOUNT_TRANS;
	op->oh_res2 = 0;

	/* and the data for this op */
	memcpy((char *)p + sizeof(xlog_op_header_t), &magic, sizeof(magic));
}

static xfs_caddr_t next(xfs_caddr_t ptr, int offset, void *private)
{
	xfs_buf_t	*buf = (xfs_buf_t *)private;

	if (XFS_BUF_COUNT(buf) < (int)(ptr - XFS_BUF_PTR(buf)) + offset)
		abort();
	return ptr + offset;
}

int
libxfs_log_clear(
	dev_t			device,
	xfs_daddr_t		start,
	uint			length,
	uuid_t			*fs_uuid,
	int			version,
	int			sunit,
	int			fmt)
{
	xfs_buf_t		*bp;
	int			len;

	if (!device || !fs_uuid)
		return -EINVAL;

	/* first zero the log */
	libxfs_device_zero(device, start, length);

	/* then write a log record header */
	len = ((version == 2) && sunit) ? BTOBB(sunit) : 2;
	len = MAX(len, 2);
	bp = libxfs_getbufr(device, start, len);
	libxfs_log_header(XFS_BUF_PTR(bp),
			  fs_uuid, version, sunit, fmt, next, bp);
	bp->b_flags |= LIBXFS_B_DIRTY;
	libxfs_putbufr(bp);
	return 0;
}

int
libxfs_log_header(
	xfs_caddr_t		caddr,
	uuid_t			*fs_uuid,
	int			version,
	int			sunit,
	int			fmt,
	libxfs_get_block_t	*nextfunc,
	void			*private)
{
	xlog_rec_header_t	*head = (xlog_rec_header_t *)caddr;
	xfs_caddr_t		p = caddr;
	__be32			cycle_lsn;
	int			i, len;

	len = ((version == 2) && sunit) ? BTOBB(sunit) : 1;

	/* note that oh_tid actually contains the cycle number
	 * and the tid is stored in h_cycle_data[0] - that's the
	 * way things end up on disk.
	 */
	memset(p, 0, BBSIZE);
	head->h_magicno = cpu_to_be32(XLOG_HEADER_MAGIC_NUM);
	head->h_cycle = cpu_to_be32(1);
	head->h_version = cpu_to_be32(version);
	if (len != 1)
		head->h_len = cpu_to_be32(sunit - BBSIZE);
	else
		head->h_len = cpu_to_be32(20);
	head->h_chksum = cpu_to_be32(0);
	head->h_prev_block = cpu_to_be32(-1);
	head->h_num_logops = cpu_to_be32(1);
	head->h_cycle_data[0] = cpu_to_be32(0xb0c0d0d0);
	head->h_fmt = cpu_to_be32(fmt);
	head->h_size = cpu_to_be32(XLOG_HEADER_CYCLE_SIZE);

	head->h_lsn = cpu_to_be64(xlog_assign_lsn(1, 0));
	head->h_tail_lsn = cpu_to_be64(xlog_assign_lsn(1, 0));

	memcpy(&head->h_fs_uuid, fs_uuid, sizeof(uuid_t));

	len = MAX(len, 2);
	p = nextfunc(p, BBSIZE, private);
	unmount_record(p);

	cycle_lsn = CYCLE_LSN_DISK(head->h_lsn);
	for (i = 2; i < len; i++) {
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
#undef libxfs_writebuf
#undef libxfs_getbuf
#undef libxfs_putbuf

xfs_buf_t 	*libxfs_readbuf(dev_t, xfs_daddr_t, int, int);
int		libxfs_writebuf(xfs_buf_t *, int);
xfs_buf_t 	*libxfs_getbuf(dev_t, xfs_daddr_t, int);
void		libxfs_putbuf (xfs_buf_t *);

xfs_buf_t *
libxfs_trace_readbuf(const char *func, const char *file, int line, dev_t dev, xfs_daddr_t blkno, int len, int flags)
{
	xfs_buf_t	*bp = libxfs_readbuf(dev, blkno, len, flags);

       if (bp){
               bp->b_func = func;
               bp->b_file = file;
               bp->b_line = line;
       }

	return bp;
}

int
libxfs_trace_writebuf(const char *func, const char *file, int line, xfs_buf_t *bp, int flags)
{
	bp->b_func = func;
	bp->b_file = file;
	bp->b_line = line;

	return libxfs_writebuf(bp, flags);
}

xfs_buf_t *
libxfs_trace_getbuf(const char *func, const char *file, int line, dev_t device, xfs_daddr_t blkno, int len)
{
	xfs_buf_t	*bp = libxfs_getbuf(device, blkno, len);

	bp->b_func = func;
	bp->b_file = file;
	bp->b_line = line;

	return bp;
}

void
libxfs_trace_putbuf(const char *func, const char *file, int line, xfs_buf_t *bp)
{
	bp->b_func = func;
	bp->b_file = file;
	bp->b_line = line;

	libxfs_putbuf(bp);
}


#endif


xfs_buf_t *
libxfs_getsb(xfs_mount_t *mp, int flags)
{
	return libxfs_readbuf(mp->m_dev, XFS_SB_DADDR,
				XFS_FSS_TO_BB(mp, 1), flags);
}

kmem_zone_t			*xfs_buf_zone;

static struct cache_mru		xfs_buf_freelist =
	{{&xfs_buf_freelist.cm_list, &xfs_buf_freelist.cm_list},
	 0, PTHREAD_MUTEX_INITIALIZER };

typedef struct {
	dev_t		device;
	xfs_daddr_t	blkno;
	unsigned int	bblen;
} xfs_bufkey_t;

static unsigned int
libxfs_bhash(cache_key_t key, unsigned int hashsize)
{
	return (((unsigned int)((xfs_bufkey_t *)key)->blkno) >> 5) % hashsize;
}

static int
libxfs_bcompare(struct cache_node *node, cache_key_t key)
{
	xfs_buf_t	*bp = (xfs_buf_t *)node;
	xfs_bufkey_t	*bkey = (xfs_bufkey_t *)key;

#ifdef IO_BCOMPARE_CHECK
	if (bp->b_dev == bkey->device &&
	    bp->b_blkno == bkey->blkno &&
	    bp->b_bcount != BBTOB(bkey->bblen))
		fprintf(stderr, "%lx: Badness in key lookup (length)\n"
			"bp=(bno %llu, len %u bytes) key=(bno %llu, len %u bytes)\n",
			pthread_self(),
			(unsigned long long)bp->b_blkno, (int)bp->b_bcount,
			(unsigned long long)bkey->blkno, BBTOB(bkey->bblen));
#endif

	return (bp->b_dev == bkey->device &&
		bp->b_blkno == bkey->blkno &&
		bp->b_bcount == BBTOB(bkey->bblen));
}

void
libxfs_bprint(xfs_buf_t *bp)
{
	fprintf(stderr, "Buffer 0x%p blkno=%llu bytes=%u flags=0x%x count=%u\n",
		bp, (unsigned long long)bp->b_blkno, (unsigned)bp->b_bcount,
		bp->b_flags, bp->b_node.cn_count);
}

static void
libxfs_initbuf(xfs_buf_t *bp, dev_t device, xfs_daddr_t bno, unsigned int bytes)
{
	bp->b_flags = 0;
	bp->b_blkno = bno;
	bp->b_bcount = bytes;
	bp->b_dev = device;
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
#ifdef XFS_BUF_TRACING
	list_head_init(&bp->b_lock_list);
#endif
	pthread_mutex_init(&bp->b_lock, NULL);
}

xfs_buf_t *
libxfs_getbufr(dev_t device, xfs_daddr_t blkno, int bblen)
{
	xfs_buf_t	*bp;
	int		blen = BBTOB(bblen);

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
		}
	} else
		bp = kmem_zone_zalloc(xfs_buf_zone, 0);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);

	if (bp != NULL)
		libxfs_initbuf(bp, device, blkno, blen);
#ifdef IO_DEBUG
	printf("%lx: %s: allocated %u bytes buffer, key=%llu(%llu), %p\n",
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

xfs_buf_t *
libxfs_getbuf(dev_t device, xfs_daddr_t blkno, int len)
{
	xfs_buf_t	*bp;
	xfs_bufkey_t	key;
	int		miss;

	key.device = device;
	key.blkno = blkno;
	key.bblen = len;

	miss = cache_node_get(libxfs_bcache, &key, (struct cache_node **)&bp);
	if (bp) {
		if (use_xfs_buf_lock)
			pthread_mutex_lock(&bp->b_lock);
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
		printf("%lx %s: %s buffer %p for bno = %llu\n",
			pthread_self(), __FUNCTION__, miss ? "miss" : "hit",
			bp, (long long)LIBXFS_BBTOOFF64(blkno));
#endif
	}

	return bp;
}

void
libxfs_putbuf(xfs_buf_t *bp)
{
#ifdef XFS_BUF_TRACING
	pthread_mutex_lock(&libxfs_bcache->c_mutex);
	lock_buf_count--;
	ASSERT(lock_buf_count >= 0);
	list_del_init(&bp->b_lock_list);
	pthread_mutex_unlock(&libxfs_bcache->c_mutex);
#endif
	if (use_xfs_buf_lock)
		pthread_mutex_unlock(&bp->b_lock);
	cache_node_put(libxfs_bcache, (struct cache_node *)bp);
}

void
libxfs_purgebuf(xfs_buf_t *bp)
{
	xfs_bufkey_t	key;

	key.device = bp->b_dev;
	key.blkno = bp->b_blkno;
	key.bblen = bp->b_bcount >> BBSHIFT;

	cache_node_purge(libxfs_bcache, &key, (struct cache_node *)bp);
}

static struct cache_node *
libxfs_balloc(cache_key_t key)
{
	xfs_bufkey_t	*bufkey = (xfs_bufkey_t *)key;

	return (struct cache_node *)libxfs_getbufr(bufkey->device,
					bufkey->blkno, bufkey->bblen);
}

int
libxfs_readbufr(dev_t dev, xfs_daddr_t blkno, xfs_buf_t *bp, int len, int flags)
{
	int	fd = libxfs_device_to_fd(dev);
	int	bytes = BBTOB(len);
	int	error;
	int	sts;

	ASSERT(BBTOB(len) <= bp->b_bcount);

	sts = pread64(fd, bp->b_addr, bytes, LIBXFS_BBTOOFF64(blkno));
	if (sts < 0) {
		error = errno;
		fprintf(stderr, _("%s: read failed: %s\n"),
			progname, strerror(error));
		if (flags & LIBXFS_EXIT_ON_FAILURE)
			exit(1);
		return error;
	} else if (sts != bytes) {
		fprintf(stderr, _("%s: error - read only %d of %d bytes\n"),
			progname, sts, bytes);
		if (flags & LIBXFS_EXIT_ON_FAILURE)
			exit(1);
		return EIO;
	}
#ifdef IO_DEBUG
	printf("%lx: %s: read %u bytes, blkno=%llu(%llu), %p\n",
		pthread_self(), __FUNCTION__, bytes,
		(long long)LIBXFS_BBTOOFF64(blkno), (long long)blkno, bp);
#endif
	if (bp->b_dev == dev &&
	    bp->b_blkno == blkno &&
	    bp->b_bcount == bytes)
		bp->b_flags |= LIBXFS_B_UPTODATE;
	return 0;
}

xfs_buf_t *
libxfs_readbuf(dev_t dev, xfs_daddr_t blkno, int len, int flags)
{
	xfs_buf_t	*bp;
	int		error;

	bp = libxfs_getbuf(dev, blkno, len);
	if (bp && !(bp->b_flags & (LIBXFS_B_UPTODATE|LIBXFS_B_DIRTY))) {
		error = libxfs_readbufr(dev, blkno, bp, len, flags);
		if (error)
			bp->b_error = error;
	}
	return bp;
}

int
libxfs_writebufr(xfs_buf_t *bp)
{
	int	sts;
	int	fd = libxfs_device_to_fd(bp->b_dev);
	int	error;

	sts = pwrite64(fd, bp->b_addr, bp->b_bcount, LIBXFS_BBTOOFF64(bp->b_blkno));
	if (sts < 0) {
		error = errno;
		fprintf(stderr, _("%s: pwrite64 failed: %s\n"),
			progname, strerror(error));
		if (bp->b_flags & LIBXFS_B_EXIT)
			exit(1);
		return error;
	} else if (sts != bp->b_bcount) {
		fprintf(stderr, _("%s: error - wrote only %d of %d bytes\n"),
			progname, sts, bp->b_bcount);
		if (bp->b_flags & LIBXFS_B_EXIT)
			exit(1);
		return EIO;
	}
#ifdef IO_DEBUG
	printf("%lx: %s: wrote %u bytes, blkno=%llu(%llu), %p\n",
			pthread_self(), __FUNCTION__, bp->b_bcount,
			(long long)LIBXFS_BBTOOFF64(bp->b_blkno),
			(long long)bp->b_blkno, bp);
#endif
	bp->b_flags |= LIBXFS_B_UPTODATE;
	bp->b_flags &= ~(LIBXFS_B_DIRTY | LIBXFS_B_EXIT);
	return 0;
}

int
libxfs_writebuf_int(xfs_buf_t *bp, int flags)
{
	bp->b_flags |= (LIBXFS_B_DIRTY | flags);
	return 0;
}

int
libxfs_writebuf(xfs_buf_t *bp, int flags)
{
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
			"bp=(bno %llu, bytes %u) range=(boff %u, bytes %u)\n",
			(long long)bp->b_blkno, bp->b_bcount, boff, len);
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
libxfs_brelse(struct cache_node *node)
{
	xfs_buf_t		*bp = (xfs_buf_t *)node;

	if (bp != NULL) {
		if (bp->b_flags & LIBXFS_B_DIRTY)
			libxfs_writebufr(bp);
		pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
		list_add(&bp->b_node.cn_mru, &xfs_buf_freelist.cm_list);
		pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);
	}
}

static unsigned int
libxfs_bulkrelse(
	struct cache 		*cache,
	struct list_head 	*list)
{
	xfs_buf_t		*bp;
	int			count = 0;

	if (list_empty(list))
		return 0 ;

	list_for_each_entry(bp, list, b_node.cn_mru) {
		if (bp->b_flags & LIBXFS_B_DIRTY)
			libxfs_writebufr(bp);
		count++;
	}

	pthread_mutex_lock(&xfs_buf_freelist.cm_mutex);
	__list_splice(list, &xfs_buf_freelist.cm_list);
	pthread_mutex_unlock(&xfs_buf_freelist.cm_mutex);

	return count;
}

static void
libxfs_bflush(struct cache_node *node)
{
	xfs_buf_t		*bp = (xfs_buf_t *)node;

	if ((bp != NULL) && (bp->b_flags & LIBXFS_B_DIRTY))
		libxfs_writebufr(bp);
}

void
libxfs_putbufr(xfs_buf_t *bp)
{
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
	/* .hash */	libxfs_bhash,
	/* .alloc */	libxfs_balloc,
	/* .flush */	libxfs_bflush,
	/* .relse */	libxfs_brelse,
	/* .compare */	libxfs_bcompare,
	/* .bulkrelse */libxfs_bulkrelse
};


/*
 * Inode cache interfaces
 */

extern kmem_zone_t	*xfs_ili_zone;
extern kmem_zone_t	*xfs_inode_zone;

static unsigned int
libxfs_ihash(cache_key_t key, unsigned int hashsize)
{
	return ((unsigned int)*(xfs_ino_t *)key) % hashsize;
}

static int
libxfs_icompare(struct cache_node *node, cache_key_t key)
{
	xfs_inode_t	*ip = (xfs_inode_t *)node;

	return (ip->i_ino == *(xfs_ino_t *)key);
}

int
libxfs_iget(xfs_mount_t *mp, xfs_trans_t *tp, xfs_ino_t ino, uint lock_flags,
		xfs_inode_t **ipp, xfs_daddr_t bno)
{
	xfs_inode_t	*ip;
	int		error = 0;

	if (cache_node_get(libxfs_icache, &ino, (struct cache_node **)&ip)) {
#ifdef INO_DEBUG
		fprintf(stderr, "%s: allocated inode, ino=%llu(%llu), %p\n",
			__FUNCTION__, (unsigned long long)ino, bno, ip);
#endif
		if ((error = libxfs_iread(mp, tp, ino, ip, bno))) {
			cache_node_purge(libxfs_icache, &ino,
					(struct cache_node *)ip);
			ip = NULL;
		}
	}
	*ipp = ip;
	return error;
}

void
libxfs_iput(xfs_inode_t *ip, uint lock_flags)
{
	cache_node_put(libxfs_icache, (struct cache_node *)ip);
}

static struct cache_node *
libxfs_ialloc(cache_key_t key)
{
	return kmem_zone_zalloc(xfs_inode_zone, 0);
}

static void
libxfs_idestroy(xfs_inode_t *ip)
{
	switch (ip->i_d.di_mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFLNK:
			libxfs_idestroy_fork(ip, XFS_DATA_FORK);
			break;
	}
	if (ip->i_afp)
		libxfs_idestroy_fork(ip, XFS_ATTR_FORK);
}

static void
libxfs_irelse(struct cache_node *node)
{
	xfs_inode_t	*ip = (xfs_inode_t *)node;

	if (ip != NULL) {
		if (ip->i_itemp)
			kmem_zone_free(xfs_ili_zone, ip->i_itemp);
		ip->i_itemp = NULL;
		libxfs_idestroy(ip);
		kmem_zone_free(xfs_inode_zone, ip);
		ip = NULL;
	}
}

void
libxfs_icache_purge(void)
{
	cache_purge(libxfs_icache);
}

struct cache_operations libxfs_icache_operations = {
	/* .hash */	libxfs_ihash,
	/* .alloc */	libxfs_ialloc,
	/* .flush */	NULL,
	/* .relse */	libxfs_irelse,
	/* .compare */	libxfs_icompare,
	/* .bulkrelse */ NULL
};
