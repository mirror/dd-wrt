#include <linux/version.h>
#include <linux/uio.h>
#include <linux/swap.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <asm/pgtable.h>

#ifndef SECTOR_SHIFT
#define SECTOR_SHIFT 9
#endif
#ifndef SECTOR_SIZE
#define SECTOR_SIZE (1 << SECTOR_SHIFT)
#endif

#ifndef PAGE_KERNEL_RO
# define PAGE_KERNEL_RO PAGE_KERNEL
#endif

#define static_assert(expr, ...) __static_assert(expr, ##__VA_ARGS__, #expr)
#define __static_assert(expr, msg, ...) _Static_assert(expr, msg)
#ifdef __GENKSYMS__
/* genksyms gets confused by _Static_assert */
#define _Static_assert(expr, ...)
#endif
#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

#ifndef SB_SYNCHRONOUS
#define SB_SYNCHRONOUS MS_SYNCHRONOUS
#define SB_ACTIVE MS_ACTIVE
#define SB_NOATIME S_NOATIME
#define SB_NODIRATIME ST_NODIRATIME
#define SB_LAZYTIME MS_LAZYTIME
#define SB_RDONLY ST_RDONLY
#define SB_POSIXACL MS_POSIXACL
static inline bool sb_rdonly(const struct super_block *sb) { return sb->s_flags & MS_RDONLY; }
static inline void
bio_set_dev(struct bio *bio, struct block_device *bdev)
{
	bio->bi_bdev = bdev;
}
#endif
extern void __bitmap_set(unsigned long *map, unsigned int start, int len);
extern void __bitmap_clear(unsigned long *map, unsigned int start, int len);

#ifndef SLAB_ACCOUNT
#define SLAB_ACCOUNT 0
#endif

#define discard_new_inode unlock_new_inode
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
#define bi_opf bi_rw
#define REQ_OP_WRITE REQ_WRITE
#define REQ_OP_READ READ
#define compat_submit_bio(bio) submit_bio(WRITE, bio)
#define compat_submit_bio_wait(bio) submit_bio_wait(WRITE, bio)
#define current_time(inode) CURRENT_TIME_SEC
#define ll_rw_block(rw, flags, nr, bh) ll_rw_block(rw, nr, bh)
#else
#define compat_submit_bio(bio) submit_bio(bio)
#define compat_submit_bio_wait(bio) submit_bio_wait(bio)
#endif