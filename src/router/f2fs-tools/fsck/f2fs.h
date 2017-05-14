/**
 * f2fs.h
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _F2FS_H_
#define _F2FS_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <assert.h>

#include <f2fs_fs.h>

#define EXIT_ERR_CODE		(-1)
#define ver_after(a, b) (typecheck(unsigned long long, a) &&            \
		typecheck(unsigned long long, b) &&                     \
		((long long)((a) - (b)) > 0))

struct list_head {
	struct list_head *next, *prev;
};

enum {
	NAT_BITMAP,
	SIT_BITMAP
};

struct node_info {
	nid_t nid;
	nid_t ino;
	u32 blk_addr;
	unsigned char version;
};

struct f2fs_nm_info {
	block_t nat_blkaddr;
	nid_t max_nid;
	nid_t init_scan_nid;
	nid_t next_scan_nid;

	unsigned int nat_cnt;
	unsigned int fcnt;

	char *nat_bitmap;
	int bitmap_size;
	char *nid_bitmap;
};

struct seg_entry {
	unsigned short valid_blocks;    /* # of valid blocks */
	unsigned char *cur_valid_map;   /* validity bitmap of blocks */
	/*
	 * # of valid blocks and the validity bitmap stored in the the last
	 * checkpoint pack. This information is used by the SSR mode.
	 */
	unsigned short ckpt_valid_blocks;
	unsigned char *ckpt_valid_map;
	unsigned char type;             /* segment type like CURSEG_XXX_TYPE */
	unsigned char orig_type;        /* segment type like CURSEG_XXX_TYPE */
	unsigned long long mtime;       /* modification time of the segment */
	int dirty;
};

struct sec_entry {
	unsigned int valid_blocks;      /* # of valid blocks in a section */
};

struct sit_info {

	block_t sit_base_addr;          /* start block address of SIT area */
	block_t sit_blocks;             /* # of blocks used by SIT area */
	block_t written_valid_blocks;   /* # of valid blocks in main area */
	char *sit_bitmap;               /* SIT bitmap pointer */
	unsigned int bitmap_size;       /* SIT bitmap size */

	unsigned long *dirty_sentries_bitmap;   /* bitmap for dirty sentries */
	unsigned int dirty_sentries;            /* # of dirty sentries */
	unsigned int sents_per_block;           /* # of SIT entries per block */
	struct seg_entry *sentries;             /* SIT segment-level cache */
	struct sec_entry *sec_entries;          /* SIT section-level cache */

	unsigned long long elapsed_time;        /* elapsed time after mount */
	unsigned long long mounted_time;        /* mount time */
	unsigned long long min_mtime;           /* min. modification time */
	unsigned long long max_mtime;           /* max. modification time */
};

struct curseg_info {
	struct f2fs_summary_block *sum_blk;     /* cached summary block */
	unsigned char alloc_type;               /* current allocation type */
	unsigned int segno;                     /* current segment number */
	unsigned short next_blkoff;             /* next block offset to write */
	unsigned int zone;                      /* current zone number */
	unsigned int next_segno;                /* preallocated segment */
};

struct f2fs_sm_info {
	struct sit_info *sit_info;
	struct curseg_info *curseg_array;

	block_t seg0_blkaddr;
	block_t main_blkaddr;
	block_t ssa_blkaddr;

	unsigned int segment_count;
	unsigned int main_segments;
	unsigned int reserved_segments;
	unsigned int ovp_segments;
};

struct f2fs_dentry_ptr {
	struct inode *inode;
	u8 *bitmap;
	struct f2fs_dir_entry *dentry;
	__u8 (*filename)[F2FS_SLOT_LEN];
	int max;
};

struct dentry {
	char *path;
	char *full_path;
	const u8 *name;
	int len;
	char *link;
	unsigned long size;
	u8 file_type;
	u16 mode;
	u16 uid;
	u16 gid;
	u32 *inode;
	u32 mtime;
	char *secon;
	uint64_t capabilities;
	nid_t ino;
	nid_t pino;
};

/* different from dnode_of_data in kernel */
struct dnode_of_data {
	struct f2fs_node *inode_blk;	/* inode page */
	struct f2fs_node *node_blk;	/* cached direct node page */
	nid_t nid;
	unsigned int ofs_in_node;
	block_t data_blkaddr;
	block_t node_blkaddr;
	int idirty, ndirty;
};

struct f2fs_sb_info {
	struct f2fs_fsck *fsck;

	struct f2fs_super_block *raw_super;
	struct f2fs_nm_info *nm_info;
	struct f2fs_sm_info *sm_info;
	struct f2fs_checkpoint *ckpt;
	int cur_cp;

	struct list_head orphan_inode_list;
	unsigned int n_orphans;

	/* basic file system units */
	unsigned int log_sectors_per_block;     /* log2 sectors per block */
	unsigned int log_blocksize;             /* log2 block size */
	unsigned int blocksize;                 /* block size */
	unsigned int root_ino_num;              /* root inode number*/
	unsigned int node_ino_num;              /* node inode number*/
	unsigned int meta_ino_num;              /* meta inode number*/
	unsigned int log_blocks_per_seg;        /* log2 blocks per segment */
	unsigned int blocks_per_seg;            /* blocks per segment */
	unsigned int segs_per_sec;              /* segments per section */
	unsigned int secs_per_zone;             /* sections per zone */
	unsigned int total_sections;            /* total section count */
	unsigned int total_node_count;          /* total node block count */
	unsigned int total_valid_node_count;    /* valid node block count */
	unsigned int total_valid_inode_count;   /* valid inode count */
	int active_logs;                        /* # of active logs */

	block_t user_block_count;               /* # of user blocks */
	block_t total_valid_block_count;        /* # of valid blocks */
	block_t alloc_valid_block_count;        /* # of allocated blocks */
	block_t last_valid_block_count;         /* for recovery */
	u32 s_next_generation;                  /* for NFS support */

	unsigned int cur_victim_sec;            /* current victim section num */
	u32 free_segments;
};

static inline struct f2fs_super_block *F2FS_RAW_SUPER(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_super_block *)(sbi->raw_super);
}

static inline struct f2fs_checkpoint *F2FS_CKPT(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_checkpoint *)(sbi->ckpt);
}

static inline struct f2fs_fsck *F2FS_FSCK(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_fsck *)(sbi->fsck);
}

static inline struct f2fs_nm_info *NM_I(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_nm_info *)(sbi->nm_info);
}

static inline struct f2fs_sm_info *SM_I(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_sm_info *)(sbi->sm_info);
}

static inline struct sit_info *SIT_I(struct f2fs_sb_info *sbi)
{
	return (struct sit_info *)(SM_I(sbi)->sit_info);
}

static inline void *inline_data_addr(struct f2fs_node *node_blk)
{
	return (void *)&(node_blk->i.i_addr[1]);
}

static inline unsigned int ofs_of_node(struct f2fs_node *node_blk)
{
	unsigned flag = le32_to_cpu(node_blk->footer.flag);
	return flag >> OFFSET_BIT_SHIFT;
}

static inline unsigned long __bitmap_size(struct f2fs_sb_info *sbi, int flag)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);

	/* return NAT or SIT bitmap */
	if (flag == NAT_BITMAP)
		return le32_to_cpu(ckpt->nat_ver_bitmap_bytesize);
	else if (flag == SIT_BITMAP)
		return le32_to_cpu(ckpt->sit_ver_bitmap_bytesize);

	return 0;
}

static inline void *__bitmap_ptr(struct f2fs_sb_info *sbi, int flag)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	int offset;
	if (le32_to_cpu(F2FS_RAW_SUPER(sbi)->cp_payload) > 0) {
		if (flag == NAT_BITMAP)
			return &ckpt->sit_nat_version_bitmap;
		else
			return ((char *)ckpt + F2FS_BLKSIZE);
	} else {
		offset = (flag == NAT_BITMAP) ?
			le32_to_cpu(ckpt->sit_ver_bitmap_bytesize) : 0;
		return &ckpt->sit_nat_version_bitmap + offset;
	}
}

static inline bool is_set_ckpt_flags(struct f2fs_checkpoint *cp, unsigned int f)
{
	unsigned int ckpt_flags = le32_to_cpu(cp->ckpt_flags);
	return ckpt_flags & f;
}

static inline block_t __start_cp_addr(struct f2fs_sb_info *sbi)
{
	block_t start_addr = le32_to_cpu(F2FS_RAW_SUPER(sbi)->cp_blkaddr);

	if (sbi->cur_cp == 2)
		start_addr += sbi->blocks_per_seg;
	return start_addr;
}

static inline block_t __start_sum_addr(struct f2fs_sb_info *sbi)
{
	return le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_start_sum);
}

static inline block_t __end_block_addr(struct f2fs_sb_info *sbi)
{
	block_t end = SM_I(sbi)->main_blkaddr;
	return end + le64_to_cpu(F2FS_RAW_SUPER(sbi)->block_count);
}

#define GET_ZONENO_FROM_SEGNO(sbi, segno)                               \
	((segno / sbi->segs_per_sec) / sbi->secs_per_zone)

#define IS_DATASEG(t)                                                   \
	((t == CURSEG_HOT_DATA) || (t == CURSEG_COLD_DATA) ||           \
	 (t == CURSEG_WARM_DATA))

#define IS_NODESEG(t)                                                   \
	((t == CURSEG_HOT_NODE) || (t == CURSEG_COLD_NODE) ||           \
	 (t == CURSEG_WARM_NODE))

#define GET_SUM_BLKADDR(sbi, segno)					\
	((sbi->sm_info->ssa_blkaddr) + segno)

#define GET_SEGOFF_FROM_SEG0(sbi, blk_addr)				\
	((blk_addr) - SM_I(sbi)->seg0_blkaddr)

#define GET_SEGNO_FROM_SEG0(sbi, blk_addr)				\
	(GET_SEGOFF_FROM_SEG0(sbi, blk_addr) >> sbi->log_blocks_per_seg)

#define GET_BLKOFF_FROM_SEG0(sbi, blk_addr)				\
	(GET_SEGOFF_FROM_SEG0(sbi, blk_addr) & (sbi->blocks_per_seg - 1))

#define FREE_I_START_SEGNO(sbi)						\
	GET_SEGNO_FROM_SEG0(sbi, SM_I(sbi)->main_blkaddr)
#define GET_R2L_SEGNO(sbi, segno)	(segno + FREE_I_START_SEGNO(sbi))

#define START_BLOCK(sbi, segno)	(SM_I(sbi)->main_blkaddr +		\
	((segno) << sbi->log_blocks_per_seg))

static inline struct curseg_info *CURSEG_I(struct f2fs_sb_info *sbi, int type)
{
	return (struct curseg_info *)(SM_I(sbi)->curseg_array + type);
}

static inline block_t start_sum_block(struct f2fs_sb_info *sbi)
{
	return __start_cp_addr(sbi) + le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_start_sum);
}

static inline block_t sum_blk_addr(struct f2fs_sb_info *sbi, int base, int type)
{
	return __start_cp_addr(sbi) + le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count)
		- (base + 1) + type;
}

#define nats_in_cursum(jnl)             (le16_to_cpu(jnl->n_nats))
#define sits_in_cursum(jnl)             (le16_to_cpu(jnl->n_sits))

#define nat_in_journal(jnl, i)          (jnl->nat_j.entries[i].ne)
#define nid_in_journal(jnl, i)          (jnl->nat_j.entries[i].nid)
#define sit_in_journal(jnl, i)          (jnl->sit_j.entries[i].se)
#define segno_in_journal(jnl, i)        (jnl->sit_j.entries[i].segno)

#define SIT_ENTRY_OFFSET(sit_i, segno)                                  \
	((segno) % sit_i->sents_per_block)
#define SIT_BLOCK_OFFSET(sit_i, segno)                                  \
	((segno) / SIT_ENTRY_PER_BLOCK)
#define TOTAL_SEGS(sbi) (SM_I(sbi)->main_segments)

static inline bool IS_VALID_NID(struct f2fs_sb_info *sbi, u32 nid)
{
	return (nid <= (NAT_ENTRY_PER_BLOCK *
			le32_to_cpu(F2FS_RAW_SUPER(sbi)->segment_count_nat)
			<< (sbi->log_blocks_per_seg - 1)));
}

static inline bool IS_VALID_BLK_ADDR(struct f2fs_sb_info *sbi, u32 addr)
{
	int i;

	if (addr >= le64_to_cpu(F2FS_RAW_SUPER(sbi)->block_count) ||
				addr < SM_I(sbi)->main_blkaddr) {
		DBG(1, "block addr [0x%x]\n", addr);
		return 0;
	}

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		if (START_BLOCK(sbi, curseg->segno) +
					curseg->next_blkoff == addr)
			return 0;
	}
	return 1;
}

static inline int IS_CUR_SEGNO(struct f2fs_sb_info *sbi, u32 segno, int type)
{
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		if (type == i)
			continue;

		if (segno == curseg->segno)
			return 1;
	}
	return 0;
}

static inline u64 BLKOFF_FROM_MAIN(struct f2fs_sb_info *sbi, u64 blk_addr)
{
	ASSERT(blk_addr >= SM_I(sbi)->main_blkaddr);
	return blk_addr - SM_I(sbi)->main_blkaddr;
}

static inline u32 GET_SEGNO(struct f2fs_sb_info *sbi, u64 blk_addr)
{
	return (u32)(BLKOFF_FROM_MAIN(sbi, blk_addr)
			>> sbi->log_blocks_per_seg);
}

static inline u32 OFFSET_IN_SEG(struct f2fs_sb_info *sbi, u64 blk_addr)
{
	return (u32)(BLKOFF_FROM_MAIN(sbi, blk_addr)
			% (1 << sbi->log_blocks_per_seg));
}

static inline void node_info_from_raw_nat(struct node_info *ni,
		struct f2fs_nat_entry *raw_nat)
{
	ni->ino = le32_to_cpu(raw_nat->ino);
	ni->blk_addr = le32_to_cpu(raw_nat->block_addr);
	ni->version = raw_nat->version;
}

static inline void set_summary(struct f2fs_summary *sum, nid_t nid,
			unsigned int ofs_in_node, unsigned char version)
{
	sum->nid = cpu_to_le32(nid);
	sum->ofs_in_node = cpu_to_le16(ofs_in_node);
	sum->version = version;
}

#define S_SHIFT 12
static unsigned char f2fs_type_by_mode[S_IFMT >> S_SHIFT] = {
	[S_IFREG >> S_SHIFT]    = F2FS_FT_REG_FILE,
	[S_IFDIR >> S_SHIFT]    = F2FS_FT_DIR,
	[S_IFCHR >> S_SHIFT]    = F2FS_FT_CHRDEV,
	[S_IFBLK >> S_SHIFT]    = F2FS_FT_BLKDEV,
	[S_IFIFO >> S_SHIFT]    = F2FS_FT_FIFO,
	[S_IFSOCK >> S_SHIFT]   = F2FS_FT_SOCK,
	[S_IFLNK >> S_SHIFT]    = F2FS_FT_SYMLINK,
};

static inline void set_de_type(struct f2fs_dir_entry *de, umode_t mode)
{
	de->file_type = f2fs_type_by_mode[(mode & S_IFMT) >> S_SHIFT];
}

static inline void *inline_xattr_addr(struct f2fs_inode *inode)
{
	return (void *)&(inode->i_addr[DEF_ADDRS_PER_INODE_INLINE_XATTR]);
}

static inline int inline_xattr_size(struct f2fs_inode *inode)
{
	if (inode->i_inline & F2FS_INLINE_XATTR)
		return F2FS_INLINE_XATTR_ADDRS << 2;
	return 0;
}

extern int lookup_nat_in_journal(struct f2fs_sb_info *sbi, u32 nid, struct f2fs_nat_entry *ne);
#define IS_SUM_NODE_SEG(footer)		(footer.entry_type == SUM_TYPE_NODE)
#define IS_SUM_DATA_SEG(footer)		(footer.entry_type == SUM_TYPE_DATA)

#endif /* _F2FS_H_ */
