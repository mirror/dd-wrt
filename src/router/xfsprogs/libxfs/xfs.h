/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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

/*
 * This header is effectively a "namespace multiplexor" for the
 * user level XFS code.  It provides all of the necessary stuff
 * such that we can build some parts of the XFS kernel code in
 * user space in a controlled fashion, and translates the names
 * used in the kernel into the names which libxfs is going to
 * make available to user tools.
 *
 * It should only ever be #include'd by XFS "kernel" code being
 * compiled in user space.
 *
 * Our goals here are to...
 *      o  "share" large amounts of complex code between user and
 *         kernel space;
 *      o  shield the user tools from changes in the bleeding
 *         edge kernel code, merging source changes when
 *         convenient and not immediately (no symlinks);
 *      o  i.e. be able to merge changes to the kernel source back
 *         into the affected user tools in a controlled fashion;
 *      o  provide a _minimalist_ life-support system for kernel
 *         code in user land, not the "everything + the kitchen
 *         sink" model which libsim had mutated into;
 *      o  allow the kernel code to be completely free of code
 *         specifically there to support the user level build.
 */

#include <xfs/libxfs.h>

typedef struct { dev_t dev; }	xfs_buftarg_t;

typedef __uint32_t 		uint_t;
typedef __uint32_t		inst_t;		/* an instruction */


#define m_ddev_targp 			m_dev
#define xfs_error_level			0

#define STATIC				static

#define ATTR_ROOT			LIBXFS_ATTR_ROOT
#define ATTR_SECURE			LIBXFS_ATTR_SECURE
#define ATTR_CREATE			LIBXFS_ATTR_CREATE
#define ATTR_REPLACE			LIBXFS_ATTR_REPLACE
#define ATTR_KERNOTIME			0
#define ATTR_KERNOVAL			0

#define IHOLD(ip)			((void) 0)

#define XFS_CORRUPTION_ERROR(e,l,mp,m)	((void) 0)
#define XFS_QM_DQATTACH(mp,ip,flags)	0
#define XFS_ERROR(e)			(e)
#define XFS_ERROR_REPORT(e,l,mp)	((void) 0)
#define XFS_ERRLEVEL_LOW		1
#define XFS_FORCED_SHUTDOWN(mp)		0
#define XFS_ILOCK_EXCL			0
#define XFS_STATS_INC(count)		do { } while (0)
#define XFS_STATS_DEC(count, x)		do { } while (0)
#define XFS_STATS_ADD(count, x)		do { } while (0)
#define XFS_TRANS_MOD_DQUOT_BYINO(mp,tp,ip,field,delta)	do { } while (0)
#define XFS_TRANS_RESERVE_QUOTA_NBLKS(mp,tp,ip,nblks,ninos,fl)	0
#define XFS_TRANS_UNRESERVE_QUOTA_NBLKS(mp,tp,ip,nblks,ninos,fl)	0
#define XFS_TEST_ERROR(expr,a,b,c)	( expr )
#define XFS_WANT_CORRUPTED_GOTO(expr,l)	\
		{ if (!(expr)) { error = EFSCORRUPTED; goto l; } }
#define XFS_WANT_CORRUPTED_RETURN(expr)	\
		{ if (!(expr)) { return EFSCORRUPTED; } }

#ifdef __GNUC__
#define __return_address	__builtin_return_address(0)
#endif

/* miscellaneous kernel routines not in user space */
#define down_read(a)		((void) 0)
#define up_read(a)		((void) 0)
#define spin_lock_init(a)	((void) 0)
#define spin_lock(a)		((void) 0)
#define spin_unlock(a)		((void) 0)
#define likely(x)		(x)
#define unlikely(x)		(x)
#define rcu_read_lock()		((void) 0)
#define rcu_read_unlock()	((void) 0)

/*
 * random32 is used for di_gen inode allocation, it must be zero for libxfs
 * or all sorts of badness can occur!
 */
#define random32()		0	

#define PAGE_CACHE_SIZE 	getpagesize()

static inline int __do_div(unsigned long long *n, unsigned base)
{
	int __res;
	__res = (int)(((unsigned long) *n) % (unsigned) base);
	*n = ((unsigned long) *n) / (unsigned) base;
	return __res;
}

#define do_div(n,base)	(__do_div((unsigned long long *)&(n), (base)))
#define do_mod(a, b)		((a) % (b))
#define rol32(x,y)		(((x) << (y)) | ((x) >> (32 - (y))))

#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })


static inline __uint32_t __get_unaligned_be32(const __uint8_t *p)
{
        return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}

static inline __uint64_t get_unaligned_be64(void *p)
{
	return (__uint64_t)__get_unaligned_be32(p) << 32 |
			   __get_unaligned_be32(p + 4);
}

static inline void __put_unaligned_be16(__uint16_t val, __uint8_t *p)
{
	*p++ = val >> 8;
	*p++ = val;
}

static inline void __put_unaligned_be32(__uint32_t val, __uint8_t *p)
{
	__put_unaligned_be16(val >> 16, p);
	__put_unaligned_be16(val, p + 2);
}

static inline void put_unaligned_be64(__uint64_t val, void *p)
{
	__put_unaligned_be32(val >> 32, p);
	__put_unaligned_be32(val, p + 4);
}


static inline __attribute__((const))
int is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/*
 * xfs_iroundup: round up argument to next power of two
 */
static inline uint
roundup_pow_of_two(uint v)
{
	int	i;
	uint	m;

	if ((v & (v - 1)) == 0)
		return v;
	ASSERT((v & 0x80000000) == 0);
	if ((v & (v + 1)) == 0)
		return v + 1;
	for (i = 0, m = 1; i < 31; i++, m <<= 1) {
		if (v & m)
			continue;
		v |= m;
		if ((v & (v + 1)) == 0)
			return v + 1;
	}
	ASSERT(0);
	return 0;
}

/* buffer management */
#define XFS_BUF_LOCK			0
#define XFS_BUF_TRYLOCK			0
#define XBF_LOCK			XFS_BUF_LOCK
#define XBF_TRYLOCK			XFS_BUF_TRYLOCK
#define XBF_DONT_BLOCK			0
#define XFS_BUF_GETERROR(bp)		0
#define XFS_BUF_DONE(bp)		((bp)->b_flags |= LIBXFS_B_UPTODATE)
#define XFS_BUF_ISDONE(bp)		((bp)->b_flags & LIBXFS_B_UPTODATE)
#define XFS_BUF_STALE(bp)		((bp)->b_flags |= LIBXFS_B_STALE)
#define XFS_BUF_UNDELAYWRITE(bp)	((bp)->b_flags &= ~LIBXFS_B_DIRTY)
#define XFS_BUF_SET_VTYPE(a,b)		((void) 0)
#define XFS_BUF_SET_VTYPE_REF(a,b,c)	((void) 0)
#define XFS_BUF_SET_BDSTRAT_FUNC(a,b)	((void) 0)

#define xfs_incore(bt,blkno,len,lockit)	0
#define xfs_buf_relse(bp)		libxfs_putbuf(bp)
#define xfs_read_buf(mp,devp,blkno,len,f,bpp)	\
					(*(bpp) = libxfs_readbuf((devp), \
							(blkno), (len), 1), 0)
#define xfs_buf_get(devp,blkno,len,f)	\
					(libxfs_getbuf((devp), (blkno), (len)))
#define xfs_bwrite(mp,bp)		libxfs_writebuf((bp), 0)

#define XBRW_READ			LIBXFS_BREAD
#define XBRW_WRITE			LIBXFS_BWRITE
#define xfs_buf_iomove(bp,off,len,data,f)	libxfs_iomove(bp,off,len,data,f)
#define xfs_buf_zero(bp,off,len)	libxfs_iomove(bp,off,len,0,LIBXFS_BZERO)

/* mount stuff */
#define XFS_MOUNT_32BITINODES		LIBXFS_MOUNT_32BITINODES
#define XFS_MOUNT_ATTR2			LIBXFS_MOUNT_ATTR2
#define XFS_MOUNT_SMALL_INUMS		0	/* ignored in userspace */
#define XFS_MOUNT_WSYNC			0	/* ignored in userspace */
#define XFS_MOUNT_NOALIGN		0	/* ignored in userspace */

#define xfs_icsb_modify_counters(mp, field, delta, rsvd) \
	xfs_mod_incore_sb(mp, field, delta, rsvd)

/*
 * Map XFS kernel routine names to libxfs versions
 */

#define xfs_alloc_fix_freelist		libxfs_alloc_fix_freelist
#define xfs_attr_get			libxfs_attr_get
#define xfs_attr_set			libxfs_attr_set
#define xfs_attr_remove			libxfs_attr_remove
#define xfs_rtfree_extent		libxfs_rtfree_extent

#define xfs_fs_repair_cmn_err		libxfs_fs_repair_cmn_err
#define xfs_fs_cmn_err			libxfs_fs_cmn_err

#define xfs_bmap_finish			libxfs_bmap_finish
#define xfs_trans_ichgtime		libxfs_trans_ichgtime
#define xfs_mod_incore_sb		libxfs_mod_incore_sb

#define xfs_trans_alloc			libxfs_trans_alloc
#define xfs_trans_bhold			libxfs_trans_bhold
#define xfs_trans_binval		libxfs_trans_binval
#define xfs_trans_bjoin			libxfs_trans_bjoin
#define xfs_trans_brelse		libxfs_trans_brelse
#define xfs_trans_commit		libxfs_trans_commit
#define xfs_trans_cancel		libxfs_trans_cancel
#define xfs_trans_dup			libxfs_trans_dup
#define xfs_trans_get_buf		libxfs_trans_get_buf
#define xfs_trans_getsb			libxfs_trans_getsb
#define xfs_trans_iget			libxfs_trans_iget
#define xfs_trans_ihold			libxfs_trans_ihold
#define xfs_trans_ijoin			libxfs_trans_ijoin
#define xfs_trans_ijoin_ref		libxfs_trans_ijoin_ref
#define xfs_trans_inode_alloc_buf	libxfs_trans_inode_alloc_buf
#define xfs_trans_log_buf		libxfs_trans_log_buf
#define xfs_trans_log_inode		libxfs_trans_log_inode
#define xfs_trans_mod_sb		libxfs_trans_mod_sb
#define xfs_trans_read_buf		libxfs_trans_read_buf
#define xfs_trans_reserve		libxfs_trans_reserve

#define xfs_trans_get_block_res(tp)	1
#define xfs_trans_set_sync(tp)		((void) 0)
#define	xfs_trans_agblocks_delta(tp, d)
#define	xfs_trans_agflist_delta(tp, d)
#define	xfs_trans_agbtree_delta(tp, d)

#define xfs_buf_readahead(a,b,c)	((void) 0)	/* no readahead */
#define xfs_btree_reada_bufl(m,fsb,c)	((void) 0)
#define xfs_btree_reada_bufs(m,fsb,c,x)	((void) 0)
#define xfs_buftrace(x,y)		((void) 0)	/* debug only */

#define xfs_cmn_err(tag,level,mp,fmt,args...)	cmn_err(level,fmt, ## args)

#define xfs_dir2_trace_args(where, args)		((void) 0)
#define xfs_dir2_trace_args_b(where, args, bp)		((void) 0)
#define xfs_dir2_trace_args_bb(where, args, lbp, dbp)	((void) 0)
#define xfs_dir2_trace_args_bibii(where, args, bs, ss, bd, sd, c) ((void) 0)
#define xfs_dir2_trace_args_db(where, args, db, bp)	((void) 0)
#define xfs_dir2_trace_args_i(where, args, i)		((void) 0)
#define xfs_dir2_trace_args_s(where, args, s)		((void) 0)
#define xfs_dir2_trace_args_sb(where, args, s, bp)	((void) 0)
#define xfs_sort					qsort

#define xfs_icsb_reinit_counters(mp)			do { } while (0)
#define xfs_initialize_perag_icache(pag)		((void) 0)

#define xfs_ilock(ip,mode)				((void) 0)
#define xfs_iunlock(ip,mode)				((void) 0)

/* space allocation */
#define xfs_alloc_busy_search(tp,ag,b,len)	0
/* avoid unused variable warning */
#define xfs_alloc_busy_insert(tp,ag,b,len)	({	\
	xfs_agnumber_t __foo = ag;			\
	__foo = 0;					\
})
#define xfs_rotorstep				1
#define xfs_bmap_rtalloc(a)			(ENOSYS)
#define xfs_rtpick_extent(mp,tp,len,p)		(ENOSYS)
#define xfs_get_extsz_hint(ip)			(0)
#define xfs_inode_is_filestream(ip)		(0)
#define xfs_filestream_lookup_ag(ip)		(0)
#define xfs_filestream_new_ag(ip,ag)		(0)

/*
 * Prototypes for kernel static functions that are aren't in their
 * associated header files
 */

/* xfs_attr.c */
int xfs_attr_rmtval_get(struct xfs_da_args *);

/* xfs_bmap.c */
void xfs_bmap_del_free(xfs_bmap_free_t *, xfs_bmap_free_item_t *,
			xfs_bmap_free_item_t *);

/* xfs_da_btree.c */
int  xfs_da_do_buf(xfs_trans_t *, xfs_inode_t *, xfs_dablk_t, xfs_daddr_t *,
			xfs_dabuf_t **, int, int, inst_t *);

/* xfs_inode.c */
void xfs_iflush_fork(xfs_inode_t *, xfs_dinode_t *, xfs_inode_log_item_t *,
			int, xfs_buf_t *);
int xfs_iformat(xfs_inode_t *, xfs_dinode_t *);

/* xfs_mount.c */
int xfs_initialize_perag_data(xfs_mount_t *, xfs_agnumber_t);
void xfs_mount_common(xfs_mount_t *, xfs_sb_t *);

/*
 * logitem.c and trans.c prototypes
 */

/* xfs_trans_item.c */
void xfs_trans_add_item(struct xfs_trans *, struct xfs_log_item *);
void xfs_trans_del_item(struct xfs_log_item *);
void xfs_trans_free_items(struct xfs_trans *, int);

/* xfs_inode_item.c */
void xfs_inode_item_init (xfs_inode_t *, xfs_mount_t *);

/* xfs_buf_item.c */
void xfs_buf_item_init (xfs_buf_t *, xfs_mount_t *);
void xfs_buf_item_log (xfs_buf_log_item_t *, uint, uint);

/* xfs_trans_buf.c */
xfs_buf_t *xfs_trans_buf_item_match (xfs_trans_t *, xfs_buftarg_t *,
			xfs_daddr_t, int);

/* local source files */
int  xfs_mod_incore_sb(xfs_mount_t *, xfs_sb_field_t, int64_t, int);
void xfs_trans_mod_sb(xfs_trans_t *, uint, long);
