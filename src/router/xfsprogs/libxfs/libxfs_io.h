// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef __LIBXFS_IO_H_
#define __LIBXFS_IO_H_

/*
 * Kernel equivalent buffer based I/O interface
 */

struct xfs_buf;
struct xfs_mount;
struct xfs_perag;

/*
 * IO verifier callbacks need the xfs_mount pointer, so we have to behave
 * somewhat like the kernel now for userspace IO in terms of having buftarg
 * based devices...
 */
struct xfs_buftarg {
	struct xfs_mount	*bt_mount;
	pthread_mutex_t		lock;
	unsigned long		writes_left;
	dev_t			bt_bdev;
	unsigned int		flags;
};

/* We purged a dirty buffer and lost a write. */
#define XFS_BUFTARG_LOST_WRITE		(1 << 0)
/* A dirty buffer failed the write verifier. */
#define XFS_BUFTARG_CORRUPT_WRITE	(1 << 1)
/* Simulate failure after a certain number of writes. */
#define XFS_BUFTARG_INJECT_WRITE_FAIL	(1 << 2)

/* Simulate the system crashing after a certain number of writes. */
static inline void
xfs_buftarg_trip_write(
	struct xfs_buftarg	*btp)
{
	if (!(btp->flags & XFS_BUFTARG_INJECT_WRITE_FAIL))
		return;

	pthread_mutex_lock(&btp->lock);
	btp->writes_left--;
	if (!btp->writes_left)
		platform_crash();
	pthread_mutex_unlock(&btp->lock);
}

extern void	libxfs_buftarg_init(struct xfs_mount *mp, dev_t ddev,
				    dev_t logdev, dev_t rtdev);
int libxfs_blkdev_issue_flush(struct xfs_buftarg *btp);

#define LIBXFS_BBTOOFF64(bbs)	(((xfs_off_t)(bbs)) << BBSHIFT)

#define XB_PAGES        2
struct xfs_buf_map {
	xfs_daddr_t		bm_bn;  /* block number for I/O */
	int			bm_len; /* size of I/O */
};

#define DEFINE_SINGLE_BUF_MAP(map, blkno, numblk) \
	struct xfs_buf_map (map) = { .bm_bn = (blkno), .bm_len = (numblk) };

struct xfs_buf_ops {
	char *name;
	union {
		__be32 magic[2];	/* v4 and v5 on disk magic values */
		__be16 magic16[2];	/* v4 and v5 on disk magic values */
	};
	void (*verify_read)(struct xfs_buf *);
	void (*verify_write)(struct xfs_buf *);
	xfs_failaddr_t (*verify_struct)(struct xfs_buf *);
};

struct xfs_buf {
	struct cache_node	b_node;
	unsigned int		b_flags;
	xfs_daddr_t		b_cache_key;	/* buffer cache index */
	unsigned int		b_length;
	struct xfs_buftarg	*b_target;
	pthread_mutex_t		b_lock;
	pthread_t		b_holder;
	unsigned int		b_recur;
	void			*b_log_item;
	struct list_head	b_li_list;	/* Log items list head */
	void			*b_transp;
	void			*b_addr;
	int			b_error;
	const struct xfs_buf_ops *b_ops;
	struct xfs_perag	*b_pag;
	struct xfs_mount	*b_mount;
	struct xfs_buf_map	*b_maps;
	struct xfs_buf_map	__b_map;
	int			b_nmaps;
	struct list_head	b_list;
};

bool xfs_verify_magic(struct xfs_buf *bp, __be32 dmagic);
bool xfs_verify_magic16(struct xfs_buf *bp, __be16 dmagic);

/* b_flags bits */
#define LIBXFS_B_DIRTY		0x0002	/* buffer has been modified */
#define LIBXFS_B_STALE		0x0004	/* buffer marked as invalid */
#define LIBXFS_B_UPTODATE	0x0008	/* buffer is sync'd to disk */
#define LIBXFS_B_DISCONTIG	0x0010	/* discontiguous buffer */
#define LIBXFS_B_UNCHECKED	0x0020	/* needs verification */

typedef unsigned int xfs_buf_flags_t;

#define XFS_BUF_DADDR_NULL		((xfs_daddr_t) (-1LL))

#define xfs_buf_offset(bp, offset)	((bp)->b_addr + (offset))

static inline xfs_daddr_t xfs_buf_daddr(struct xfs_buf *bp)
{
	return bp->b_maps[0].bm_bn;
}

static inline void xfs_buf_set_daddr(struct xfs_buf *bp, xfs_daddr_t blkno)
{
	assert(bp->b_cache_key == XFS_BUF_DADDR_NULL);
	bp->b_maps[0].bm_bn = blkno;
}

void libxfs_buf_set_priority(struct xfs_buf *bp, int priority);
int libxfs_buf_priority(struct xfs_buf *bp);

#define xfs_buf_set_ref(bp,ref)		((void) 0)
#define xfs_buf_ioerror(bp,err)		((bp)->b_error = (err))

#define xfs_daddr_to_agno(mp,d) \
	((xfs_agnumber_t)(XFS_BB_TO_FSBT(mp, d) / (mp)->m_sb.sb_agblocks))
#define xfs_daddr_to_agbno(mp,d) \
	((xfs_agblock_t)(XFS_BB_TO_FSBT(mp, d) % (mp)->m_sb.sb_agblocks))

/* Buffer Cache Interfaces */

extern struct cache	*libxfs_bcache;
extern struct cache_operations	libxfs_bcache_operations;

#define LIBXFS_GETBUF_TRYLOCK	(1 << 0)

/* Return the buffer even if the verifiers fail. */
#define LIBXFS_READBUF_SALVAGE		(1 << 1)

int libxfs_buf_read_map(struct xfs_buftarg *btp, struct xfs_buf_map *maps,
			int nmaps, int flags, struct xfs_buf **bpp,
			const struct xfs_buf_ops *ops);
void libxfs_buf_mark_dirty(struct xfs_buf *bp);
int libxfs_buf_get_map(struct xfs_buftarg *btp, struct xfs_buf_map *maps,
			int nmaps, int flags, struct xfs_buf **bpp);
void	libxfs_buf_relse(struct xfs_buf *bp);

static inline int
libxfs_buf_get(
	struct xfs_buftarg	*target,
	xfs_daddr_t		blkno,
	size_t			numblks,
	struct xfs_buf		**bpp)
{
	DEFINE_SINGLE_BUF_MAP(map, blkno, numblks);

	return libxfs_buf_get_map(target, &map, 1, 0, bpp);
}

static inline int
libxfs_buf_read(
	struct xfs_buftarg	*target,
	xfs_daddr_t		blkno,
	size_t			numblks,
	xfs_buf_flags_t		flags,
	struct xfs_buf		**bpp,
	const struct xfs_buf_ops *ops)
{
	DEFINE_SINGLE_BUF_MAP(map, blkno, numblks);

	return libxfs_buf_read_map(target, &map, 1, flags, bpp, ops);
}

int libxfs_readbuf_verify(struct xfs_buf *bp, const struct xfs_buf_ops *ops);
struct xfs_buf *libxfs_getsb(struct xfs_mount *mp);
extern void	libxfs_bcache_purge(void);
extern void	libxfs_bcache_free(void);
extern void	libxfs_bcache_flush(void);
extern int	libxfs_bcache_overflowed(void);

/* Buffer (Raw) Interfaces */
int		libxfs_bwrite(struct xfs_buf *bp);
extern int	libxfs_readbufr(struct xfs_buftarg *, xfs_daddr_t, struct xfs_buf *, int, int);
extern int	libxfs_readbufr_map(struct xfs_buftarg *, struct xfs_buf *, int);

extern int	libxfs_device_zero(struct xfs_buftarg *, xfs_daddr_t, uint);

extern int libxfs_bhash_size;

static inline int
xfs_buf_verify_cksum(struct xfs_buf *bp, unsigned long cksum_offset)
{
	return xfs_verify_cksum(bp->b_addr, BBTOB(bp->b_length),
				cksum_offset);
}

static inline void
xfs_buf_update_cksum(struct xfs_buf *bp, unsigned long cksum_offset)
{
	xfs_update_cksum(bp->b_addr, BBTOB(bp->b_length),
			 cksum_offset);
}

static inline int
xfs_buf_associate_memory(struct xfs_buf *bp, void *mem, size_t len)
{
	bp->b_addr = mem;
	bp->b_length = BTOBB(len);
	return 0;
}

static inline void
xfs_buf_hold(struct xfs_buf *bp)
{
	bp->b_node.cn_count++;
}

void xfs_buf_lock(struct xfs_buf *bp);
void xfs_buf_unlock(struct xfs_buf *bp);

int libxfs_buf_get_uncached(struct xfs_buftarg *targ, size_t bblen, int flags,
		struct xfs_buf **bpp);
int libxfs_buf_read_uncached(struct xfs_buftarg *targ, xfs_daddr_t daddr,
		size_t bblen, int flags, struct xfs_buf **bpp,
		const struct xfs_buf_ops *ops);

/* Push a single buffer on a delwri queue. */
static inline bool
xfs_buf_delwri_queue(struct xfs_buf *bp, struct list_head *buffer_list)
{
	xfs_buf_hold(bp);
	list_add_tail(&bp->b_list, buffer_list);
	return true;
}

int xfs_buf_delwri_submit(struct list_head *buffer_list);
void xfs_buf_delwri_cancel(struct list_head *list);

#endif	/* __LIBXFS_IO_H__ */
