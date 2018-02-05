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

/*
 * define a guard and something we can check to determine what include context
 * we are running from.
 */
#ifndef __LIBXFS_INTERNAL_XFS_H__
#define __LIBXFS_INTERNAL_XFS_H__

/*
 * Repair doesn't have a inode when it calls libxfs_dir2_data_freescan,
 * so we to work around this internally for now.
 */
#define xfs_dir2_data_freescan(ip, hdr, loghead) \
	__xfs_dir2_data_freescan((ip)->i_mount->m_dir_geo, \
				 (ip)->d_ops, hdr, loghead)

#include "libxfs_api_defs.h"
#include "platform_defs.h"
#include "xfs.h"

#include "list.h"
#include "hlist.h"
#include "cache.h"
#include "bitops.h"
#include "kmem.h"
#include "radix-tree.h"
#include "atomic.h"

#include "xfs_types.h"
#include "xfs_arch.h"

#include "xfs_fs.h"

/* CRC stuff, buffer API dependent on it */
extern uint32_t crc32_le(uint32_t crc, unsigned char const *p, size_t len);
extern uint32_t crc32c_le(uint32_t crc, unsigned char const *p, size_t len);

#define crc32(c,p,l)	crc32_le((c),(unsigned char const *)(p),(l))
#define crc32c(c,p,l)	crc32c_le((c),(unsigned char const *)(p),(l))

#include "xfs_cksum.h"

/*
 * This mirrors the kernel include for xfs_buf.h - it's implicitly included in
 * every files via a similar include in the kernel xfs_linux.h.
 */
#include "libxfs_io.h"

/* for all the support code that uses progname in error messages */
extern char    *progname;

#undef ASSERT
#define ASSERT(ex) assert(ex)

#ifndef EWRONGFS
#define EWRONGFS	EINVAL
#endif

#define xfs_error_level			0

#define STATIC				static

/* XXX: need to push these out to make LIBXFS_ATTR defines */
#define ATTR_ROOT			0x0002
#define ATTR_SECURE			0x0008
#define ATTR_CREATE			0x0010
#define ATTR_REPLACE			0x0020
#define ATTR_KERNOTIME			0
#define ATTR_KERNOVAL			0

#define IHOLD(ip)			((void) 0)

#define XFS_IGET_CREATE			0x1
#define XFS_IGET_UNTRUSTED		0x2

extern void cmn_err(int, char *, ...);
enum ce { CE_DEBUG, CE_CONT, CE_NOTE, CE_WARN, CE_ALERT, CE_PANIC };

#define xfs_notice(mp,fmt,args...)		cmn_err(CE_NOTE,fmt, ## args)
#define xfs_warn(mp,fmt,args...)		cmn_err(CE_WARN,fmt, ## args)
#define xfs_hex_dump(d,n)		((void) 0)

#define xfs_force_shutdown(d,n)		((void) 0)

/* stop unused var warnings by assigning mp to itself */
#define XFS_CORRUPTION_ERROR(e,l,mp,m)	do { \
	(mp) = (mp); \
	cmn_err(CE_ALERT, "%s: XFS_CORRUPTION_ERROR", (e));  \
} while (0)

#define XFS_ERROR_REPORT(e,l,mp)	do { \
	(mp) = (mp); \
	cmn_err(CE_ALERT, "%s: XFS_ERROR_REPORT", (e));  \
} while (0)

#define XFS_QM_DQATTACH(mp,ip,flags)	0
#define XFS_ERRLEVEL_LOW		1
#define XFS_FORCED_SHUTDOWN(mp)		0
#define XFS_ILOCK_EXCL			0
#define XFS_STATS_INC(mp, count)	do { (mp) = (mp); } while (0)
#define XFS_STATS_DEC(mp, count, x)	do { (mp) = (mp); } while (0)
#define XFS_STATS_ADD(mp, count, x)	do { (mp) = (mp); } while (0)
#define XFS_TRANS_MOD_DQUOT_BYINO(mp,tp,ip,field,delta)	do { } while (0)
#define XFS_TRANS_RESERVE_QUOTA_NBLKS(mp,tp,ip,nblks,ninos,fl)	0
#define XFS_TRANS_UNRESERVE_QUOTA_NBLKS(mp,tp,ip,nblks,ninos,fl)	0
#define XFS_TEST_ERROR(expr,a,b,c)	( expr )
#define XFS_WANT_CORRUPTED_GOTO(mp, expr, l)				\
{									\
	if (!(expr)) {							\
		if ((mp)->m_flags & LIBXFS_MOUNT_WANT_CORRUPTED)	\
			printf("WANT_CORRUPTED_GOTO at %s:%d\n",	\
				__func__, __LINE__);			\
		error = -EFSCORRUPTED;					\
		goto l;							\
	}								\
}
#define XFS_WANT_CORRUPTED_RETURN(mp, expr)				\
{									\
	if (!(expr)) {							\
		if ((mp)->m_flags & LIBXFS_MOUNT_WANT_CORRUPTED)	\
			printf("WANT_CORRUPTED_RETURN at %s:%d\n",	\
				__func__, __LINE__);			\
		return -EFSCORRUPTED;					\
	}								\
}

#ifdef __GNUC__
#define __return_address	__builtin_return_address(0)
#endif

#define XFS_DQUOT_CLUSTER_SIZE_FSB (xfs_filblks_t)1

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
#define WARN_ON_ONCE(expr)	((void) 0)

#define percpu_counter_read(x)		(*x)
#define percpu_counter_read_positive(x)	((*x) > 0 ? (*x) : 0)
#define percpu_counter_sum(x)		(*x)

/*
 * prandom_u32 is used for di_gen inode allocation, it must be zero for libxfs
 * or all sorts of badness can occur!
 */
#define prandom_u32()		0

#define PAGE_SIZE		getpagesize()

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


#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

/*
 * Handling for kernel bitmap types.
 */
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, NBBY * sizeof(long))
#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))

/*
 * This is a common helper function for find_next_bit and
 * find_next_zero_bit.  The difference is the "invert" argument, which
 * is XORed with each fetched word before searching it for one bits.
 */
static inline unsigned long
_find_next_bit(const unsigned long *addr, unsigned long nbits,
		unsigned long start, unsigned long invert)
{
	unsigned long tmp;

	if (!nbits || start >= nbits)
		return nbits;

	tmp = addr[start / BITS_PER_LONG] ^ invert;

	/* Handle 1st word. */
	tmp &= BITMAP_FIRST_WORD_MASK(start);
	start = round_down(start, BITS_PER_LONG);

	while (!tmp) {
		start += BITS_PER_LONG;
		if (start >= nbits)
			return nbits;

		tmp = addr[start / BITS_PER_LONG] ^ invert;
	}

	return min(start + ffs(tmp), nbits);
}

/*
 * Find the next set bit in a memory region.
 */
static inline unsigned long
find_next_bit(const unsigned long *addr, unsigned long size,
		unsigned long offset)
{
	return _find_next_bit(addr, size, offset, 0UL);
}
static inline unsigned long
find_next_zero_bit(const unsigned long *addr, unsigned long size,
		 unsigned long offset)
{
	return _find_next_bit(addr, size, offset, ~0UL);
}
#define find_first_zero_bit(addr, size) find_next_zero_bit((addr), (size), 0)

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

static inline __uint64_t
roundup_64(__uint64_t x, __uint32_t y)
{
	x += y - 1;
	do_div(x, y);
	return x * y;
}

/* buffer management */
#define XFS_BUF_LOCK			0
#define XFS_BUF_TRYLOCK			0
#define XBF_LOCK			XFS_BUF_LOCK
#define XBF_TRYLOCK			XFS_BUF_TRYLOCK
#define XBF_DONT_BLOCK			0
#define XBF_UNMAPPED			0
#define XBF_DONE			0
#define XFS_BUF_GETERROR(bp)		0
#define XFS_BUF_DONE(bp)		((bp)->b_flags |= LIBXFS_B_UPTODATE)
#define XFS_BUF_ISDONE(bp)		((bp)->b_flags & LIBXFS_B_UPTODATE)
#define xfs_buf_stale(bp)		((bp)->b_flags |= LIBXFS_B_STALE)
#define XFS_BUF_UNDELAYWRITE(bp)	((bp)->b_flags &= ~LIBXFS_B_DIRTY)
#define XFS_BUF_SET_VTYPE(a,b)		((void) 0)
#define XFS_BUF_SET_VTYPE_REF(a,b,c)	((void) 0)
#define XFS_BUF_SET_BDSTRAT_FUNC(a,b)	((void) 0)

/* avoid gcc warning */
#define xfs_incore(bt,blkno,len,lockit)	({		\
	typeof(blkno) __foo = (blkno);			\
	typeof(len) __bar = (len);			\
	(blkno) = __foo;				\
	(len) = __bar; /* no set-but-unused warning */	\
	NULL;						\
})
#define xfs_buf_relse(bp)		libxfs_putbuf(bp)
#define xfs_buf_get(devp,blkno,len,f)	(libxfs_getbuf((devp), (blkno), (len)))
#define xfs_bwrite(bp)			libxfs_writebuf((bp), 0)
#define xfs_buf_delwri_queue(bp, bl)	libxfs_writebuf((bp), 0)

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
#define XFS_MOUNT_IKEEP			0	/* ignored in userspace */
#define XFS_MOUNT_SWALLOC		0	/* ignored in userspace */
#define XFS_MOUNT_RDONLY		0	/* ignored in userspace */


#define xfs_trans_get_block_res(tp)	1
#define xfs_trans_set_sync(tp)		((void) 0)
#define xfs_trans_ordered_buf(tp, bp)	((void) 0)
#define	xfs_trans_agblocks_delta(tp, d)
#define	xfs_trans_agflist_delta(tp, d)
#define	xfs_trans_agbtree_delta(tp, d)
#define xfs_trans_buf_set_type(tp, bp, t)	({	\
	int __t = (t);					\
	__t = __t; /* no set-but-unused warning */	\
	tp = tp;  /* no set-but-unused warning */	\
})

#define xfs_trans_buf_copy_type(dbp, sbp)

/* no readahead, need to avoid set-but-unused var warnings. */
#define xfs_buf_readahead(a,d,c,ops)		({	\
	xfs_daddr_t __d = d;				\
	__d = __d; /* no set-but-unused warning */	\
})
#define xfs_buf_readahead_map(a,b,c,ops)	((void) 0)	/* no readahead */
#define xfs_buftrace(x,y)			((void) 0)	/* debug only */

#define xfs_cmn_err(tag,level,mp,fmt,args...)	cmn_err(level,fmt, ## args)
#define xfs_warn(mp,fmt,args...)		cmn_err(CE_WARN,fmt, ## args)
#define xfs_alert(mp,fmt,args...)		cmn_err(CE_ALERT,fmt, ## args)
#define xfs_alert_tag(mp,tag,fmt,args...)	cmn_err(CE_ALERT,fmt, ## args)

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
#define xfs_ilock_nowait(ip,mode)			((void) 0)
#define xfs_ilock_demote(ip,mode)			((void) 0)
#define xfs_ilock_data_map_shared(ip)			(0)
#define xfs_ilock_attr_map_shared(ip)			(0)
#define xfs_iunlock(ip,mode)				({	\
	typeof(mode) __mode = mode;				\
	__mode = __mode; /* no set-but-unused warning */	\
})
#define __xfs_flock(ip)					((void) 0)

/* space allocation */
#define xfs_extent_busy_reuse(mp,ag,bno,len,user)	((void) 0)
/* avoid unused variable warning */
#define xfs_extent_busy_insert(tp,ag,bno,len,flags)({ 	\
	xfs_agnumber_t __foo = ag; 			\
	__foo = __foo; /* no set-but-unused warning */	\
})
#define xfs_extent_busy_trim(args,fbno,flen,bno,len) \
do { \
	*(bno) = (fbno); \
	*(len) = (flen); \
} while (0)

/* avoid unused variable warning */
#define xfs_alloc_busy_insert(tp,ag,b,len)	({	\
	xfs_agnumber_t __foo = ag;			\
	__foo = 0;					\
})

#define xfs_rotorstep				1
#define xfs_bmap_rtalloc(a)			(-ENOSYS)
#define xfs_get_extsz_hint(ip)			(0)
#define xfs_get_cowextsz_hint(ip)		(0)
#define xfs_inode_is_filestream(ip)		(0)
#define xfs_filestream_lookup_ag(ip)		(0)
#define xfs_filestream_new_ag(ip,ag)		(0)

#define xfs_log_force(mp,flags)			((void) 0)
#define XFS_LOG_SYNC				1

/* quota bits */
#define xfs_trans_mod_dquot_byino(t,i,f,d)		((void) 0)
#define xfs_trans_reserve_quota_nblks(t,i,b,n,f)	(0)
#define xfs_trans_unreserve_quota_nblks(t,i,b,n,f)	((void) 0)
#define xfs_qm_dqattach(i,f)				(0)

#define uuid_copy(s,d)		platform_uuid_copy((s),(d))
#define uuid_equal(s,d)		(platform_uuid_compare((s),(d)) == 0)

#define xfs_icreate_log(tp, agno, agbno, cnt, isize, len, gen) ((void) 0)
#define xfs_sb_validate_fsb_count(sbp, nblks)		(0)

/*
 * Prototypes for kernel static functions that are aren't in their
 * associated header files.
 */
struct xfs_da_args;
struct xfs_bmap_free;
struct xfs_bmap_free_item;
struct xfs_mount;
struct xfs_sb;
struct xfs_trans;
struct xfs_inode;
struct xfs_log_item;
struct xfs_buf;
struct xfs_buf_map;
struct xfs_buf_log_item;
struct xfs_buftarg;

/* xfs_attr.c */
int xfs_attr_rmtval_get(struct xfs_da_args *);

/* xfs_bmap.c */
void xfs_bmap_del_free(struct xfs_bmap_free *, struct xfs_bmap_free_item *);

/* xfs_mount.c */
int xfs_initialize_perag_data(struct xfs_mount *, xfs_agnumber_t);
void xfs_mount_common(struct xfs_mount *, struct xfs_sb *);

/*
 * logitem.c and trans.c prototypes
 */
void xfs_trans_init(struct xfs_mount *);
int xfs_trans_roll(struct xfs_trans **, struct xfs_inode *);

/* xfs_trans_item.c */
void xfs_trans_add_item(struct xfs_trans *, struct xfs_log_item *);
void xfs_trans_del_item(struct xfs_log_item *);

/* xfs_inode_item.c */
void xfs_inode_item_init(struct xfs_inode *, struct xfs_mount *);

/* xfs_buf_item.c */
void xfs_buf_item_init(struct xfs_buf *, struct xfs_mount *);
void xfs_buf_item_log(struct xfs_buf_log_item *, uint, uint);

/* xfs_trans_buf.c */
struct xfs_buf *xfs_trans_buf_item_match(struct xfs_trans *,
			struct xfs_buftarg *, struct xfs_buf_map *, int);

/* local source files */
#define xfs_mod_fdblocks(mp, delta, rsvd) \
	libxfs_mod_incore_sb(mp, XFS_TRANS_SB_FDBLOCKS, delta, rsvd)
#define xfs_mod_frextents(mp, delta) \
	libxfs_mod_incore_sb(mp, XFS_TRANS_SB_FREXTENTS, delta, 0)
int  libxfs_mod_incore_sb(struct xfs_mount *, int, int64_t, int);
/* percpu counters in mp are #defined to the superblock sb_ counters */
#define xfs_reinit_percpu_counters(mp)

void xfs_trans_mod_sb(struct xfs_trans *, uint, long);
void xfs_trans_init(struct xfs_mount *);
int  xfs_trans_roll(struct xfs_trans **, struct xfs_inode *);
void xfs_verifier_error(struct xfs_buf *bp);

/* XXX: this is clearly a bug - a shared header needs to export this */
/* xfs_rtalloc.c */
int libxfs_rtfree_extent(struct xfs_trans *, xfs_rtblock_t, xfs_extlen_t);

int libxfs_zero_extent(struct xfs_inode *ip, xfs_fsblock_t start_fsb,
                        xfs_off_t count_fsb);

bool xfs_log_check_lsn(struct xfs_mount *, xfs_lsn_t);

#endif	/* __LIBXFS_INTERNAL_XFS_H__ */
