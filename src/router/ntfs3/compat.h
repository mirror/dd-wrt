#include <asm/pgtable.h>
#include <linux/version.h>
#include <linux/uio.h>
#include <linux/swap.h>


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

#define SB_SYNCHRONOUS MS_SYNCHRONOUS
#define SB_ACTIVE MS_ACTIVE
#define SB_NOATIME S_NOATIME
#define SB_NODIRATIME ST_NODIRATIME
#define SB_LAZYTIME MS_LAZYTIME
#define SB_RDONLY ST_RDONLY
#define SB_POSIXACL MS_POSIXACL

extern void __bitmap_set(unsigned long *map, unsigned int start, int len);
extern void __bitmap_clear(unsigned long *map, unsigned int start, int len);
static inline bool sb_rdonly(const struct super_block *sb) { return sb->s_flags & MS_RDONLY; }

static inline void
bio_set_dev(struct bio *bio, struct block_device *bdev)
{
	bio->bi_bdev = bdev;
}

#define discard_new_inode unlock_new_inode
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
//#define current_time(inode) CURRENT_TIME_SEC
#endif