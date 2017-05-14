/**
 * f2fs_fs.h
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 *
 * The byteswap codes are copied from:
 *   samba_3_master/lib/ccan/endian/endian.h under LGPL 2.1
 */
#ifndef __F2FS_FS_H__
#define __F2FS_FS_H__

#include <inttypes.h>
#include <linux/types.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#include <linux/blkzoned.h>
#endif

typedef u_int64_t	u64;
typedef u_int32_t	u32;
typedef u_int16_t	u16;
typedef u_int8_t	u8;
typedef u32		block_t;
typedef u32		nid_t;
typedef u8		bool;
typedef unsigned long	pgoff_t;
typedef unsigned short	umode_t;

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#else
/**
 * bswap_16 - reverse bytes in a uint16_t value.
 * @val: value whose bytes to swap.
 *
 * Example:
 *	// Output contains "1024 is 4 as two bytes reversed"
 *	printf("1024 is %u as two bytes reversed\n", bswap_16(1024));
 */
static inline uint16_t bswap_16(uint16_t val)
{
	return ((val & (uint16_t)0x00ffU) << 8)
		| ((val & (uint16_t)0xff00U) >> 8);
}

/**
 * bswap_32 - reverse bytes in a uint32_t value.
 * @val: value whose bytes to swap.
 *
 * Example:
 *	// Output contains "1024 is 262144 as four bytes reversed"
 *	printf("1024 is %u as four bytes reversed\n", bswap_32(1024));
 */
static inline uint32_t bswap_32(uint32_t val)
{
	return ((val & (uint32_t)0x000000ffUL) << 24)
		| ((val & (uint32_t)0x0000ff00UL) <<  8)
		| ((val & (uint32_t)0x00ff0000UL) >>  8)
		| ((val & (uint32_t)0xff000000UL) >> 24);
}
#endif /* !HAVE_BYTESWAP_H */

#if defined HAVE_DECL_BSWAP_64 && !HAVE_DECL_BSWAP_64
/**
 * bswap_64 - reverse bytes in a uint64_t value.
 * @val: value whose bytes to swap.
 *
 * Example:
 *	// Output contains "1024 is 1125899906842624 as eight bytes reversed"
 *	printf("1024 is %llu as eight bytes reversed\n",
 *		(unsigned long long)bswap_64(1024));
 */
static inline uint64_t bswap_64(uint64_t val)
{
	return ((val & (uint64_t)0x00000000000000ffULL) << 56)
		| ((val & (uint64_t)0x000000000000ff00ULL) << 40)
		| ((val & (uint64_t)0x0000000000ff0000ULL) << 24)
		| ((val & (uint64_t)0x00000000ff000000ULL) <<  8)
		| ((val & (uint64_t)0x000000ff00000000ULL) >>  8)
		| ((val & (uint64_t)0x0000ff0000000000ULL) >> 24)
		| ((val & (uint64_t)0x00ff000000000000ULL) >> 40)
		| ((val & (uint64_t)0xff00000000000000ULL) >> 56);
}
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le16_to_cpu(x)	((__u16)(x))
#define le32_to_cpu(x)	((__u32)(x))
#define le64_to_cpu(x)	((__u64)(x))
#define cpu_to_le16(x)	((__u16)(x))
#define cpu_to_le32(x)	((__u32)(x))
#define cpu_to_le64(x)	((__u64)(x))
#elif __BYTE_ORDER == __BIG_ENDIAN
#define le16_to_cpu(x)	bswap_16(x)
#define le32_to_cpu(x)	bswap_32(x)
#define le64_to_cpu(x)	bswap_64(x)
#define cpu_to_le16(x)	bswap_16(x)
#define cpu_to_le32(x)	bswap_32(x)
#define cpu_to_le64(x)	bswap_64(x)
#endif

#define typecheck(type,x) \
	({	type __dummy; \
		typeof(x) __dummy2; \
		(void)(&__dummy == &__dummy2); \
		1; \
	 })

#define NULL_SEGNO	((unsigned int)~0)

/*
 * Debugging interfaces
 */
#define FIX_MSG(fmt, ...)						\
	do {								\
		printf("[FIX] (%s:%4d) ", __func__, __LINE__);		\
		printf(" --> "fmt"\n", ##__VA_ARGS__);			\
	} while (0)

#define ASSERT_MSG(fmt, ...)						\
	do {								\
		printf("[ASSERT] (%s:%4d) ", __func__, __LINE__);	\
		printf(" --> "fmt"\n", ##__VA_ARGS__);			\
		c.bug_on = 1;						\
	} while (0)

#define ASSERT(exp)							\
	do {								\
		if (!(exp)) {						\
			printf("[ASSERT] (%s:%4d) " #exp"\n",		\
					__func__, __LINE__);		\
			exit(-1);					\
		}							\
	} while (0)

#define ERR_MSG(fmt, ...)						\
	do {								\
		printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
	} while (0)

#define MSG(n, fmt, ...)						\
	do {								\
		if (c.dbg_lv >= n) {					\
			printf(fmt, ##__VA_ARGS__);			\
		}							\
	} while (0)

#define DBG(n, fmt, ...)						\
	do {								\
		if (c.dbg_lv >= n) {					\
			printf("[%s:%4d] " fmt,				\
				__func__, __LINE__, ##__VA_ARGS__);	\
		}							\
	} while (0)

/* Display on console */
#define DISP(fmt, ptr, member)				\
	do {						\
		printf("%-30s" fmt, #member, ((ptr)->member));	\
	} while (0)

#define DISP_u32(ptr, member)						\
	do {								\
		assert(sizeof((ptr)->member) <= 4);			\
		printf("%-30s" "\t\t[0x%8x : %u]\n",			\
			#member, le32_to_cpu(((ptr)->member)),		\
			le32_to_cpu(((ptr)->member)));			\
	} while (0)

#define DISP_u64(ptr, member)						\
	do {								\
		assert(sizeof((ptr)->member) == 8);			\
		printf("%-30s" "\t\t[0x%8llx : %llu]\n",		\
			#member, le64_to_cpu(((ptr)->member)),		\
			le64_to_cpu(((ptr)->member)));			\
	} while (0)

#define DISP_utf(ptr, member)						\
	do {								\
		printf("%-30s" "\t\t[%s]\n", #member, ((ptr)->member)); \
	} while (0)

/* Display to buffer */
#define BUF_DISP_u32(buf, data, len, ptr, member)			\
	do {								\
		assert(sizeof((ptr)->member) <= 4);			\
		snprintf(buf, len, #member);				\
		snprintf(data, len, "0x%x : %u", ((ptr)->member),	\
						((ptr)->member));	\
	} while (0)

#define BUF_DISP_u64(buf, data, len, ptr, member)			\
	do {								\
		assert(sizeof((ptr)->member) == 8);			\
		snprintf(buf, len, #member);				\
		snprintf(data, len, "0x%llx : %llu", ((ptr)->member),	\
						((ptr)->member));	\
	} while (0)

#define BUF_DISP_utf(buf, data, len, ptr, member)			\
		snprintf(buf, len, #member)

/* these are defined in kernel */
#define PAGE_SIZE		4096
#define PAGE_CACHE_SIZE		4096
#define BITS_PER_BYTE		8
#define F2FS_SUPER_MAGIC	0xF2F52010	/* F2FS Magic Number */
#define CHECKSUM_OFFSET		4092
#define MAX_PATH_LEN		64
#define MAX_DEVICES		8

#define F2FS_BYTES_TO_BLK(bytes)    ((bytes) >> F2FS_BLKSIZE_BITS)
#define F2FS_BLKSIZE_BITS 12

/* for mkfs */
#define	F2FS_NUMBER_OF_CHECKPOINT_PACK	2
#define	DEFAULT_SECTOR_SIZE		512
#define	DEFAULT_SECTORS_PER_BLOCK	8
#define	DEFAULT_BLOCKS_PER_SEGMENT	512
#define DEFAULT_SEGMENTS_PER_SECTION	1

#define VERSION_LEN	256

enum f2fs_config_func {
	FSCK,
	DUMP,
	DEFRAG,
	RESIZE,
	SLOAD,
};

struct device_info {
	char *path;
	int32_t fd;
	u_int32_t sector_size;
	u_int64_t total_sectors;	/* got by get_device_info */
	u_int64_t start_blkaddr;
	u_int64_t end_blkaddr;
	u_int32_t total_segments;

	/* to handle zone block devices */
	int zoned_model;
	u_int32_t nr_zones;
	u_int32_t nr_rnd_zones;
	size_t zone_blocks;
};

struct f2fs_configuration {
	u_int32_t reserved_segments;
	u_int32_t new_reserved_segments;
	int zoned_mode;
	int zoned_model;
	size_t zone_blocks;
	double overprovision;
	double new_overprovision;
	u_int32_t cur_seg[6];
	u_int32_t segs_per_sec;
	u_int32_t secs_per_zone;
	u_int32_t segs_per_zone;
	u_int32_t start_sector;
	u_int32_t total_segments;
	u_int32_t sector_size;
	u_int64_t total_sectors;
	u_int64_t wanted_total_sectors;
	u_int64_t target_sectors;
	u_int32_t sectors_per_blk;
	u_int32_t blks_per_seg;
	__u8 init_version[VERSION_LEN + 1];
	__u8 sb_version[VERSION_LEN + 1];
	__u8 version[VERSION_LEN + 1];
	char *vol_label;
	int heap;
	int32_t kd;
	int32_t dump_fd;
	struct device_info devices[MAX_DEVICES];
	int ndevs;
	char *extension_list;
	const char *rootdev_name;
	int dbg_lv;
	int trim;
	int func;
	void *private;
	int fix_on;
	int bug_on;
	int auto_fix;
	int preen_mode;
	int ro;
	__le32 feature;			/* defined features */

	/* defragmentation parameters */
	int defrag_shrink;
	u_int64_t defrag_start;
	u_int64_t defrag_len;
	u_int64_t defrag_target;

	/* sload parameters */
	char *from_dir;
	char *mount_point;
} __attribute__((packed));

#ifdef CONFIG_64BIT
#define BITS_PER_LONG	64
#else
#define BITS_PER_LONG	32
#endif

#define BIT_MASK(nr)	(1 << (nr % BITS_PER_LONG))
#define BIT_WORD(nr)	(nr / BITS_PER_LONG)

#define set_sb_le64(member, val)		(sb->member = cpu_to_le64(val))
#define set_sb_le32(member, val)		(sb->member = cpu_to_le32(val))
#define set_sb_le16(member, val)		(sb->member = cpu_to_le16(val))
#define get_sb_le64(member)			le64_to_cpu(sb->member)
#define get_sb_le32(member)			le32_to_cpu(sb->member)
#define get_sb_le16(member)			le16_to_cpu(sb->member)
#define get_newsb_le64(member)			le64_to_cpu(new_sb->member)
#define get_newsb_le32(member)			le32_to_cpu(new_sb->member)
#define get_newsb_le16(member)			le16_to_cpu(new_sb->member)

#define set_sb(member, val)	\
			do {						\
				typeof(sb->member) t;			\
				switch (sizeof(t)) {			\
				case 8: set_sb_le64(member, val); break; \
				case 4: set_sb_le32(member, val); break; \
				case 2: set_sb_le16(member, val); break; \
				} \
			} while(0)

#define get_sb(member)		\
			({						\
				typeof(sb->member) t;			\
				switch (sizeof(t)) {			\
				case 8: t = get_sb_le64(member); break; \
				case 4: t = get_sb_le32(member); break; \
				case 2: t = get_sb_le16(member); break; \
				} 					\
				t; \
			})
#define get_newsb(member)		\
			({						\
				typeof(new_sb->member) t;		\
				switch (sizeof(t)) {			\
				case 8: t = get_newsb_le64(member); break; \
				case 4: t = get_newsb_le32(member); break; \
				case 2: t = get_newsb_le16(member); break; \
				} 					\
				t; \
			})

#define set_cp_le64(member, val)		(cp->member = cpu_to_le64(val))
#define set_cp_le32(member, val)		(cp->member = cpu_to_le32(val))
#define set_cp_le16(member, val)		(cp->member = cpu_to_le16(val))
#define get_cp_le64(member)			le64_to_cpu(cp->member)
#define get_cp_le32(member)			le32_to_cpu(cp->member)
#define get_cp_le16(member)			le16_to_cpu(cp->member)

#define set_cp(member, val)	\
			do {						\
				typeof(cp->member) t;			\
				switch (sizeof(t)) {			\
				case 8: set_cp_le64(member, val); break; \
				case 4: set_cp_le32(member, val); break; \
				case 2: set_cp_le16(member, val); break; \
				} \
			} while(0)

#define get_cp(member)		\
			({						\
				typeof(cp->member) t;			\
				switch (sizeof(t)) {			\
				case 8: t = get_cp_le64(member); break; \
				case 4: t = get_cp_le32(member); break; \
				case 2: t = get_cp_le16(member); break; \
				} 					\
				t; \
			})

/*
 * Copied from include/linux/kernel.h
 */
#define __round_mask(x, y)	((__typeof__(x))((y)-1))
#define round_down(x, y)	((x) & ~__round_mask(x, y))
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

/*
 * Copied from fs/f2fs/f2fs.h
 */
#define	NR_CURSEG_DATA_TYPE	(3)
#define NR_CURSEG_NODE_TYPE	(3)
#define NR_CURSEG_TYPE	(NR_CURSEG_DATA_TYPE + NR_CURSEG_NODE_TYPE)

enum {
	CURSEG_HOT_DATA	= 0,	/* directory entry blocks */
	CURSEG_WARM_DATA,	/* data blocks */
	CURSEG_COLD_DATA,	/* multimedia or GCed data blocks */
	CURSEG_HOT_NODE,	/* direct node blocks of directory files */
	CURSEG_WARM_NODE,	/* direct node blocks of normal files */
	CURSEG_COLD_NODE,	/* indirect node blocks */
	NO_CHECK_TYPE
};

#define F2FS_MIN_SEGMENTS	9 /* SB + 2 (CP + SIT + NAT) + SSA + MAIN */

/*
 * Copied from fs/f2fs/segment.h
 */
#define GET_SUM_TYPE(footer) ((footer)->entry_type)
#define SET_SUM_TYPE(footer, type) ((footer)->entry_type = type)

/*
 * Copied from include/linux/f2fs_sb.h
 */
#define F2FS_SUPER_OFFSET		1024	/* byte-size offset */
#define F2FS_MIN_LOG_SECTOR_SIZE	9	/* 9 bits for 512 bytes */
#define F2FS_MAX_LOG_SECTOR_SIZE	12	/* 12 bits for 4096 bytes */
#define F2FS_BLKSIZE			4096	/* support only 4KB block */
#define F2FS_MAX_EXTENSION		64	/* # of extension entries */
#define F2FS_BLK_ALIGN(x)	(((x) + F2FS_BLKSIZE - 1) / F2FS_BLKSIZE)

#define NULL_ADDR		0x0U
#define NEW_ADDR		-1U

#define F2FS_ROOT_INO(sbi)	(sbi->root_ino_num)
#define F2FS_NODE_INO(sbi)	(sbi->node_ino_num)
#define F2FS_META_INO(sbi)	(sbi->meta_ino_num)

/* This flag is used by node and meta inodes, and by recovery */
#define GFP_F2FS_ZERO	(GFP_NOFS | __GFP_ZERO)

/*
 * For further optimization on multi-head logs, on-disk layout supports maximum
 * 16 logs by default. The number, 16, is expected to cover all the cases
 * enoughly. The implementaion currently uses no more than 6 logs.
 * Half the logs are used for nodes, and the other half are used for data.
 */
#define MAX_ACTIVE_LOGS	16
#define MAX_ACTIVE_NODE_LOGS	8
#define MAX_ACTIVE_DATA_LOGS	8

#define F2FS_FEATURE_ENCRYPT	0x0001
#define F2FS_FEATURE_BLKZONED	0x0002

#define MAX_VOLUME_NAME		512

/*
 * For superblock
 */
struct f2fs_device {
	__u8 path[MAX_PATH_LEN];
	__le32 total_segments;
} __attribute__((packed));

struct f2fs_super_block {
	__le32 magic;			/* Magic Number */
	__le16 major_ver;		/* Major Version */
	__le16 minor_ver;		/* Minor Version */
	__le32 log_sectorsize;		/* log2 sector size in bytes */
	__le32 log_sectors_per_block;	/* log2 # of sectors per block */
	__le32 log_blocksize;		/* log2 block size in bytes */
	__le32 log_blocks_per_seg;	/* log2 # of blocks per segment */
	__le32 segs_per_sec;		/* # of segments per section */
	__le32 secs_per_zone;		/* # of sections per zone */
	__le32 checksum_offset;		/* checksum offset inside super block */
	__le64 block_count;		/* total # of user blocks */
	__le32 section_count;		/* total # of sections */
	__le32 segment_count;		/* total # of segments */
	__le32 segment_count_ckpt;	/* # of segments for checkpoint */
	__le32 segment_count_sit;	/* # of segments for SIT */
	__le32 segment_count_nat;	/* # of segments for NAT */
	__le32 segment_count_ssa;	/* # of segments for SSA */
	__le32 segment_count_main;	/* # of segments for main area */
	__le32 segment0_blkaddr;	/* start block address of segment 0 */
	__le32 cp_blkaddr;		/* start block address of checkpoint */
	__le32 sit_blkaddr;		/* start block address of SIT */
	__le32 nat_blkaddr;		/* start block address of NAT */
	__le32 ssa_blkaddr;		/* start block address of SSA */
	__le32 main_blkaddr;		/* start block address of main area */
	__le32 root_ino;		/* root inode number */
	__le32 node_ino;		/* node inode number */
	__le32 meta_ino;		/* meta inode number */
	__u8 uuid[16];			/* 128-bit uuid for volume */
	__le16 volume_name[MAX_VOLUME_NAME];	/* volume name */
	__le32 extension_count;		/* # of extensions below */
	__u8 extension_list[F2FS_MAX_EXTENSION][8];	/* extension array */
	__le32 cp_payload;
	__u8 version[VERSION_LEN];	/* the kernel version */
	__u8 init_version[VERSION_LEN];	/* the initial kernel version */
	__le32 feature;			/* defined features */
	__u8 encryption_level;		/* versioning level for encryption */
	__u8 encrypt_pw_salt[16];	/* Salt used for string2key algorithm */
	struct f2fs_device devs[MAX_DEVICES];	/* device list */
	__u8 reserved[327];		/* valid reserved region */
} __attribute__((packed));

/*
 * For checkpoint
 */
#define CP_FASTBOOT_FLAG	0x00000020
#define CP_FSCK_FLAG		0x00000010
#define CP_ERROR_FLAG		0x00000008
#define CP_COMPACT_SUM_FLAG	0x00000004
#define CP_ORPHAN_PRESENT_FLAG	0x00000002
#define CP_UMOUNT_FLAG		0x00000001

struct f2fs_checkpoint {
	__le64 checkpoint_ver;		/* checkpoint block version number */
	__le64 user_block_count;	/* # of user blocks */
	__le64 valid_block_count;	/* # of valid blocks in main area */
	__le32 rsvd_segment_count;	/* # of reserved segments for gc */
	__le32 overprov_segment_count;	/* # of overprovision segments */
	__le32 free_segment_count;	/* # of free segments in main area */

	/* information of current node segments */
	__le32 cur_node_segno[MAX_ACTIVE_NODE_LOGS];
	__le16 cur_node_blkoff[MAX_ACTIVE_NODE_LOGS];
	/* information of current data segments */
	__le32 cur_data_segno[MAX_ACTIVE_DATA_LOGS];
	__le16 cur_data_blkoff[MAX_ACTIVE_DATA_LOGS];
	__le32 ckpt_flags;		/* Flags : umount and journal_present */
	__le32 cp_pack_total_block_count;	/* total # of one cp pack */
	__le32 cp_pack_start_sum;	/* start block number of data summary */
	__le32 valid_node_count;	/* Total number of valid nodes */
	__le32 valid_inode_count;	/* Total number of valid inodes */
	__le32 next_free_nid;		/* Next free node number */
	__le32 sit_ver_bitmap_bytesize;	/* Default value 64 */
	__le32 nat_ver_bitmap_bytesize; /* Default value 256 */
	__le32 checksum_offset;		/* checksum offset inside cp block */
	__le64 elapsed_time;		/* mounted time */
	/* allocation type of current segment */
	unsigned char alloc_type[MAX_ACTIVE_LOGS];

	/* SIT and NAT version bitmap */
	unsigned char sit_nat_version_bitmap[1];
} __attribute__((packed));

/*
 * For orphan inode management
 */
#define F2FS_ORPHANS_PER_BLOCK	1020

struct f2fs_orphan_block {
	__le32 ino[F2FS_ORPHANS_PER_BLOCK];	/* inode numbers */
	__le32 reserved;	/* reserved */
	__le16 blk_addr;	/* block index in current CP */
	__le16 blk_count;	/* Number of orphan inode blocks in CP */
	__le32 entry_count;	/* Total number of orphan nodes in current CP */
	__le32 check_sum;	/* CRC32 for orphan inode block */
} __attribute__((packed));

/*
 * For NODE structure
 */
struct f2fs_extent {
	__le32 fofs;		/* start file offset of the extent */
	__le32 blk_addr;	/* start block address of the extent */
	__le32 len;		/* lengh of the extent */
} __attribute__((packed));

#define F2FS_NAME_LEN		255
#define F2FS_INLINE_XATTR_ADDRS	50	/* 200 bytes for inline xattrs */
#define DEF_ADDRS_PER_INODE	923	/* Address Pointers in an Inode */
#define ADDRS_PER_INODE(i)	addrs_per_inode(i)
#define DEF_ADDRS_PER_INODE_INLINE_XATTR				\
		(DEF_ADDRS_PER_INODE - F2FS_INLINE_XATTR_ADDRS)
#define ADDRS_PER_BLOCK         1018	/* Address Pointers in a Direct Block */
#define NIDS_PER_BLOCK          1018	/* Node IDs in an Indirect Block */

#define	NODE_DIR1_BLOCK		(DEF_ADDRS_PER_INODE + 1)
#define	NODE_DIR2_BLOCK		(DEF_ADDRS_PER_INODE + 2)
#define	NODE_IND1_BLOCK		(DEF_ADDRS_PER_INODE + 3)
#define	NODE_IND2_BLOCK		(DEF_ADDRS_PER_INODE + 4)
#define	NODE_DIND_BLOCK		(DEF_ADDRS_PER_INODE + 5)

#define F2FS_INLINE_XATTR	0x01	/* file inline xattr flag */
#define F2FS_INLINE_DATA	0x02	/* file inline data flag */
#define F2FS_INLINE_DENTRY	0x04	/* file inline dentry flag */
#define F2FS_DATA_EXIST		0x08	/* file inline data exist flag */
#define F2FS_INLINE_DOTS	0x10	/* file having implicit dot dentries */

#define MAX_INLINE_DATA (sizeof(__le32) *				\
			(DEF_ADDRS_PER_INODE_INLINE_XATTR - 1))

#define INLINE_DATA_OFFSET	(PAGE_CACHE_SIZE - sizeof(struct node_footer) \
				- sizeof(__le32)*(DEF_ADDRS_PER_INODE + 5 - 1))

#define DEF_DIR_LEVEL		0

/*
 * i_advise uses FADVISE_XXX_BIT. We can add additional hints later.
 */
#define FADVISE_COLD_BIT       0x01
#define FADVISE_LOST_PINO_BIT  0x02
#define FADVISE_ENCRYPT_BIT    0x04

#define file_is_encrypt(fi)      ((fi)->i_advise & FADVISE_ENCRYPT_BIT)

struct f2fs_inode {
	__le16 i_mode;			/* file mode */
	__u8 i_advise;			/* file hints */
	__u8 i_inline;			/* file inline flags */
	__le32 i_uid;			/* user ID */
	__le32 i_gid;			/* group ID */
	__le32 i_links;			/* links count */
	__le64 i_size;			/* file size in bytes */
	__le64 i_blocks;		/* file size in blocks */
	__le64 i_atime;			/* access time */
	__le64 i_ctime;			/* change time */
	__le64 i_mtime;			/* modification time */
	__le32 i_atime_nsec;		/* access time in nano scale */
	__le32 i_ctime_nsec;		/* change time in nano scale */
	__le32 i_mtime_nsec;		/* modification time in nano scale */
	__le32 i_generation;		/* file version (for NFS) */
	__le32 i_current_depth;		/* only for directory depth */
	__le32 i_xattr_nid;		/* nid to save xattr */
	__le32 i_flags;			/* file attributes */
	__le32 i_pino;			/* parent inode number */
	__le32 i_namelen;		/* file name length */
	__u8 i_name[F2FS_NAME_LEN];	/* file name for SPOR */
	__u8 i_dir_level;		/* dentry_level for large dir */

	struct f2fs_extent i_ext;	/* caching a largest extent */

	__le32 i_addr[DEF_ADDRS_PER_INODE];	/* Pointers to data blocks */

	__le32 i_nid[5];		/* direct(2), indirect(2),
						double_indirect(1) node id */
} __attribute__((packed));

struct direct_node {
	__le32 addr[ADDRS_PER_BLOCK];	/* array of data block address */
} __attribute__((packed));

struct indirect_node {
	__le32 nid[NIDS_PER_BLOCK];	/* array of data block address */
} __attribute__((packed));

enum {
	COLD_BIT_SHIFT = 0,
	FSYNC_BIT_SHIFT,
	DENT_BIT_SHIFT,
	OFFSET_BIT_SHIFT
};

#define XATTR_NODE_OFFSET	((((unsigned int)-1) << OFFSET_BIT_SHIFT) \
				>> OFFSET_BIT_SHIFT)

struct node_footer {
	__le32 nid;		/* node id */
	__le32 ino;		/* inode nunmber */
	__le32 flag;		/* include cold/fsync/dentry marks and offset */
	__le64 cp_ver;		/* checkpoint version */
	__le32 next_blkaddr;	/* next node page block address */
} __attribute__((packed));

struct f2fs_node {
	/* can be one of three types: inode, direct, and indirect types */
	union {
		struct f2fs_inode i;
		struct direct_node dn;
		struct indirect_node in;
	};
	struct node_footer footer;
} __attribute__((packed));

/*
 * For NAT entries
 */
#define NAT_ENTRY_PER_BLOCK (PAGE_CACHE_SIZE / sizeof(struct f2fs_nat_entry))
#define NAT_BLOCK_OFFSET(start_nid) (start_nid / NAT_ENTRY_PER_BLOCK)

struct f2fs_nat_entry {
	__u8 version;		/* latest version of cached nat entry */
	__le32 ino;		/* inode number */
	__le32 block_addr;	/* block address */
} __attribute__((packed));

struct f2fs_nat_block {
	struct f2fs_nat_entry entries[NAT_ENTRY_PER_BLOCK];
} __attribute__((packed));

/*
 * For SIT entries
 *
 * Each segment is 2MB in size by default so that a bitmap for validity of
 * there-in blocks should occupy 64 bytes, 512 bits.
 * Not allow to change this.
 */
#define SIT_VBLOCK_MAP_SIZE 64
#define SIT_ENTRY_PER_BLOCK (PAGE_CACHE_SIZE / sizeof(struct f2fs_sit_entry))

/*
 * F2FS uses 4 bytes to represent block address. As a result, supported size of
 * disk is 16 TB and it equals to 16 * 1024 * 1024 / 2 segments.
 */
#define F2FS_MAX_SEGMENT       ((16 * 1024 * 1024) / 2)
#define MAX_SIT_BITMAP_SIZE    (SEG_ALIGN(ALIGN(F2FS_MAX_SEGMENT, \
						SIT_ENTRY_PER_BLOCK)) * \
						c.blks_per_seg / 8)

/*
 * Note that f2fs_sit_entry->vblocks has the following bit-field information.
 * [15:10] : allocation type such as CURSEG_XXXX_TYPE
 * [9:0] : valid block count
 */
#define SIT_VBLOCKS_SHIFT	10
#define SIT_VBLOCKS_MASK	((1 << SIT_VBLOCKS_SHIFT) - 1)
#define GET_SIT_VBLOCKS(raw_sit)				\
	(le16_to_cpu((raw_sit)->vblocks) & SIT_VBLOCKS_MASK)
#define GET_SIT_TYPE(raw_sit)					\
	((le16_to_cpu((raw_sit)->vblocks) & ~SIT_VBLOCKS_MASK)	\
	 >> SIT_VBLOCKS_SHIFT)

struct f2fs_sit_entry {
	__le16 vblocks;				/* reference above */
	__u8 valid_map[SIT_VBLOCK_MAP_SIZE];	/* bitmap for valid blocks */
	__le64 mtime;				/* segment age for cleaning */
} __attribute__((packed));

struct f2fs_sit_block {
	struct f2fs_sit_entry entries[SIT_ENTRY_PER_BLOCK];
} __attribute__((packed));

/*
 * For segment summary
 *
 * One summary block contains exactly 512 summary entries, which represents
 * exactly 2MB segment by default. Not allow to change the basic units.
 *
 * NOTE: For initializing fields, you must use set_summary
 *
 * - If data page, nid represents dnode's nid
 * - If node page, nid represents the node page's nid.
 *
 * The ofs_in_node is used by only data page. It represents offset
 * from node's page's beginning to get a data block address.
 * ex) data_blkaddr = (block_t)(nodepage_start_address + ofs_in_node)
 */
#define ENTRIES_IN_SUM		512
#define	SUMMARY_SIZE		(7)	/* sizeof(struct summary) */
#define	SUM_FOOTER_SIZE		(5)	/* sizeof(struct summary_footer) */
#define SUM_ENTRIES_SIZE	(SUMMARY_SIZE * ENTRIES_IN_SUM)

/* a summary entry for a 4KB-sized block in a segment */
struct f2fs_summary {
	__le32 nid;		/* parent node id */
	union {
		__u8 reserved[3];
		struct {
			__u8 version;		/* node version number */
			__le16 ofs_in_node;	/* block index in parent node */
		} __attribute__((packed));
	};
} __attribute__((packed));

/* summary block type, node or data, is stored to the summary_footer */
#define SUM_TYPE_NODE		(1)
#define SUM_TYPE_DATA		(0)

struct summary_footer {
	unsigned char entry_type;	/* SUM_TYPE_XXX */
	__le32 check_sum;		/* summary checksum */
} __attribute__((packed));

#define SUM_JOURNAL_SIZE	(F2FS_BLKSIZE - SUM_FOOTER_SIZE -\
				SUM_ENTRIES_SIZE)
#define NAT_JOURNAL_ENTRIES	((SUM_JOURNAL_SIZE - 2) /\
				sizeof(struct nat_journal_entry))
#define NAT_JOURNAL_RESERVED	((SUM_JOURNAL_SIZE - 2) %\
				sizeof(struct nat_journal_entry))
#define SIT_JOURNAL_ENTRIES	((SUM_JOURNAL_SIZE - 2) /\
				sizeof(struct sit_journal_entry))
#define SIT_JOURNAL_RESERVED	((SUM_JOURNAL_SIZE - 2) %\
				sizeof(struct sit_journal_entry))

/*
 * Reserved area should make size of f2fs_extra_info equals to
 * that of nat_journal and sit_journal.
 */
#define EXTRA_INFO_RESERVED	(SUM_JOURNAL_SIZE - 2 - 8)

/*
 * frequently updated NAT/SIT entries can be stored in the spare area in
 * summary blocks
 */
enum {
	NAT_JOURNAL = 0,
	SIT_JOURNAL
};

struct nat_journal_entry {
	__le32 nid;
	struct f2fs_nat_entry ne;
} __attribute__((packed));

struct nat_journal {
	struct nat_journal_entry entries[NAT_JOURNAL_ENTRIES];
	__u8 reserved[NAT_JOURNAL_RESERVED];
} __attribute__((packed));

struct sit_journal_entry {
	__le32 segno;
	struct f2fs_sit_entry se;
} __attribute__((packed));

struct sit_journal {
	struct sit_journal_entry entries[SIT_JOURNAL_ENTRIES];
	__u8 reserved[SIT_JOURNAL_RESERVED];
} __attribute__((packed));

struct f2fs_extra_info {
	__le64 kbytes_written;
	__u8 reserved[EXTRA_INFO_RESERVED];
} __attribute__((packed));

struct f2fs_journal {
	union {
		__le16 n_nats;
		__le16 n_sits;
	};
	/* spare area is used by NAT or SIT journals or extra info */
	union {
		struct nat_journal nat_j;
		struct sit_journal sit_j;
		struct f2fs_extra_info info;
	};
} __attribute__((packed));

/* 4KB-sized summary block structure */
struct f2fs_summary_block {
	struct f2fs_summary entries[ENTRIES_IN_SUM];
	struct f2fs_journal journal;
	struct summary_footer footer;
} __attribute__((packed));

/*
 * For directory operations
 */
#define F2FS_DOT_HASH		0
#define F2FS_DDOT_HASH		F2FS_DOT_HASH
#define F2FS_MAX_HASH		(~((0x3ULL) << 62))
#define F2FS_HASH_COL_BIT	((0x1ULL) << 63)

typedef __le32	f2fs_hash_t;

/* One directory entry slot covers 8bytes-long file name */
#define F2FS_SLOT_LEN		8
#define F2FS_SLOT_LEN_BITS	3

#define GET_DENTRY_SLOTS(x)	((x + F2FS_SLOT_LEN - 1) >> F2FS_SLOT_LEN_BITS)

/* the number of dentry in a block */
#define NR_DENTRY_IN_BLOCK	214

/* MAX level for dir lookup */
#define MAX_DIR_HASH_DEPTH	63

/* MAX buckets in one level of dir */
#define MAX_DIR_BUCKETS		(1 << ((MAX_DIR_HASH_DEPTH / 2) - 1))

#define SIZE_OF_DIR_ENTRY	11	/* by byte */
#define SIZE_OF_DENTRY_BITMAP	((NR_DENTRY_IN_BLOCK + BITS_PER_BYTE - 1) / \
					BITS_PER_BYTE)
#define SIZE_OF_RESERVED	(PAGE_SIZE - ((SIZE_OF_DIR_ENTRY + \
				F2FS_SLOT_LEN) * \
				NR_DENTRY_IN_BLOCK + SIZE_OF_DENTRY_BITMAP))

/* One directory entry slot representing F2FS_SLOT_LEN-sized file name */
struct f2fs_dir_entry {
	__le32 hash_code;	/* hash code of file name */
	__le32 ino;		/* inode number */
	__le16 name_len;	/* lengh of file name */
	__u8 file_type;		/* file type */
} __attribute__((packed));

/* 4KB-sized directory entry block */
struct f2fs_dentry_block {
	/* validity bitmap for directory entries in each block */
	__u8 dentry_bitmap[SIZE_OF_DENTRY_BITMAP];
	__u8 reserved[SIZE_OF_RESERVED];
	struct f2fs_dir_entry dentry[NR_DENTRY_IN_BLOCK];
	__u8 filename[NR_DENTRY_IN_BLOCK][F2FS_SLOT_LEN];
} __attribute__((packed));

/* for inline dir */
#define NR_INLINE_DENTRY	(MAX_INLINE_DATA * BITS_PER_BYTE / \
				((SIZE_OF_DIR_ENTRY + F2FS_SLOT_LEN) * \
				BITS_PER_BYTE + 1))
#define INLINE_DENTRY_BITMAP_SIZE	((NR_INLINE_DENTRY + \
					BITS_PER_BYTE - 1) / BITS_PER_BYTE)
#define INLINE_RESERVED_SIZE	(MAX_INLINE_DATA - \
				((SIZE_OF_DIR_ENTRY + F2FS_SLOT_LEN) * \
				NR_INLINE_DENTRY + INLINE_DENTRY_BITMAP_SIZE))

/* inline directory entry structure */
struct f2fs_inline_dentry {
	__u8 dentry_bitmap[INLINE_DENTRY_BITMAP_SIZE];
	__u8 reserved[INLINE_RESERVED_SIZE];
	struct f2fs_dir_entry dentry[NR_INLINE_DENTRY];
	__u8 filename[NR_INLINE_DENTRY][F2FS_SLOT_LEN];
} __attribute__((packed));

/* file types used in inode_info->flags */
enum FILE_TYPE {
	F2FS_FT_UNKNOWN,
	F2FS_FT_REG_FILE,
	F2FS_FT_DIR,
	F2FS_FT_CHRDEV,
	F2FS_FT_BLKDEV,
	F2FS_FT_FIFO,
	F2FS_FT_SOCK,
	F2FS_FT_SYMLINK,
	F2FS_FT_MAX,
	/* added for fsck */
	F2FS_FT_ORPHAN,
	F2FS_FT_XATTR,
	F2FS_FT_LAST_FILE_TYPE = F2FS_FT_XATTR,
};

/* from f2fs/segment.h */
enum {
	LFS = 0,
	SSR
};

extern int utf8_to_utf16(u_int16_t *, const char *, size_t, size_t);
extern int utf16_to_utf8(char *, const u_int16_t *, size_t, size_t);
extern int log_base_2(u_int32_t);
extern unsigned int addrs_per_inode(struct f2fs_inode *);

extern int get_bits_in_byte(unsigned char n);
extern int test_and_set_bit_le(u32, u8 *);
extern int test_and_clear_bit_le(u32, u8 *);
extern int test_bit_le(u32, const u8 *);
extern int f2fs_test_bit(unsigned int, const char *);
extern int f2fs_set_bit(unsigned int, char *);
extern int f2fs_clear_bit(unsigned int, char *);
extern u64 find_next_bit_le(const u8 *, u64, u64);
extern u64 find_next_zero_bit_le(const u8 *, u64, u64);

extern u_int32_t f2fs_cal_crc32(u_int32_t, void *, int);
extern int f2fs_crc_valid(u_int32_t blk_crc, void *buf, int len);

extern void f2fs_init_configuration(void);
extern int f2fs_devs_are_umounted(void);
extern int f2fs_dev_is_umounted(char *);
extern int f2fs_get_device_info(void);
extern int get_device_info(int);
extern void f2fs_finalize_device(void);

extern int dev_read(void *, __u64, size_t);
extern int dev_write(void *, __u64, size_t);
extern int dev_write_block(void *, __u64);
extern int dev_write_dump(void *, __u64, size_t);
/* All bytes in the buffer must be 0 use dev_fill(). */
extern int dev_fill(void *, __u64, size_t);
extern int dev_fill_block(void *, __u64);

extern int dev_read_block(void *, __u64);
extern int dev_reada_block(__u64);

extern int dev_read_version(void *, __u64, size_t);
extern void get_kernel_version(__u8 *);
f2fs_hash_t f2fs_dentry_hash(const unsigned char *, int);

#define F2FS_ZONED_NONE		0
#define F2FS_ZONED_HA		1
#define F2FS_ZONED_HM		2

#ifdef HAVE_LINUX_BLKZONED_H

#define blk_zone_type(z)        (z)->type
#define blk_zone_conv(z)	((z)->type == BLK_ZONE_TYPE_CONVENTIONAL)
#define blk_zone_seq_req(z)	((z)->type == BLK_ZONE_TYPE_SEQWRITE_REQ)
#define blk_zone_seq_pref(z)	((z)->type == BLK_ZONE_TYPE_SEQWRITE_PREF)
#define blk_zone_seq(z)		(blk_zone_seq_req(z) || blk_zone_seq_pref(z))

static inline const char *
blk_zone_type_str(struct blk_zone *blkz)
{
	switch (blk_zone_type(blkz)) {
	case BLK_ZONE_TYPE_CONVENTIONAL:
		return( "Conventional" );
	case BLK_ZONE_TYPE_SEQWRITE_REQ:
		return( "Sequential-write-required" );
	case BLK_ZONE_TYPE_SEQWRITE_PREF:
		return( "Sequential-write-preferred" );
	}
	return( "Unknown-type" );
}

#define blk_zone_cond(z)	(z)->cond

static inline const char *
blk_zone_cond_str(struct blk_zone *blkz)
{
	switch (blk_zone_cond(blkz)) {
	case BLK_ZONE_COND_NOT_WP:
		return "Not-write-pointer";
	case BLK_ZONE_COND_EMPTY:
		return "Empty";
	case BLK_ZONE_COND_IMP_OPEN:
		return "Implicit-open";
	case BLK_ZONE_COND_EXP_OPEN:
		return "Explicit-open";
	case BLK_ZONE_COND_CLOSED:
		return "Closed";
	case BLK_ZONE_COND_READONLY:
		return "Read-only";
	case BLK_ZONE_COND_FULL:
		return "Full";
	case BLK_ZONE_COND_OFFLINE:
		return "Offline";
	}
	return "Unknown-cond";
}

#define blk_zone_empty(z)	(blk_zone_cond(z) == BLK_ZONE_COND_EMPTY)

#define blk_zone_sector(z)	(z)->start
#define blk_zone_length(z)	(z)->len
#define blk_zone_wp_sector(z)	(z)->wp
#define blk_zone_need_reset(z)	(int)(z)->reset
#define blk_zone_non_seq(z)	(int)(z)->non_seq

#endif

extern void f2fs_get_zoned_model(int);
extern int f2fs_get_zone_blocks(int);
extern int f2fs_check_zones(int);
extern int f2fs_reset_zones(int);

extern struct f2fs_configuration c;

#define ALIGN(val, size)	((val) + (size) - 1) / (size)
#define SEG_ALIGN(blks)		ALIGN(blks, c.blks_per_seg)
#define ZONE_ALIGN(blks)	ALIGN(blks, c.blks_per_seg * \
					c.segs_per_zone)

static inline double get_best_overprovision(struct f2fs_super_block *sb)
{
	double reserved, ovp, candidate, end, diff, space;
	double max_ovp = 0, max_space = 0;

	if (get_sb(segment_count_main) < 256) {
		candidate = 10;
		end = 95;
		diff = 5;
	} else {
		candidate = 0.01;
		end = 10;
		diff = 0.01;
	}

	for (; candidate <= end; candidate += diff) {
		reserved = (2 * (100 / candidate + 1) + 6) *
						get_sb(segs_per_sec);
		ovp = (get_sb(segment_count_main) - reserved) * candidate / 100;
		space = get_sb(segment_count_main) - reserved - ovp;
		if (max_space < space) {
			max_space = space;
			max_ovp = candidate;
		}
	}
	return max_ovp;
}

#endif	/*__F2FS_FS_H */
