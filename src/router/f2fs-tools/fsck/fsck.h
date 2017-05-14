/**
 * fsck.h
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _FSCK_H_
#define _FSCK_H_

#include "f2fs.h"

#define FSCK_UNMATCHED_EXTENT		0x00000001

enum {
	PREEN_MODE_0,
	PREEN_MODE_1,
	PREEN_MODE_MAX
};

enum {
	NOERROR,
	EWRONG_OPT,
	ENEED_ARG,
	EUNKNOWN_OPT,
	EUNKNOWN_ARG,
};

/* fsck.c */
struct orphan_info {
	u32 nr_inodes;
	u32 *ino_list;
};

struct extent_info {
	u32 fofs;		/* start offset in a file */
	u32 blk;		/* start block address of the extent */
	u32 len;		/* length of the extent */
};

struct child_info {
	u32 state;
	u32 links;
	u32 files;
	u32 pgofs;
	u8 dots;
	u8 dir_level;
	u32 p_ino;		/*parent ino*/
	u32 pp_ino;		/*parent parent ino*/
	struct extent_info ei;
	u32 last_blk;
};

struct f2fs_fsck {
	struct f2fs_sb_info sbi;

	struct orphan_info orphani;
	struct chk_result {
		u64 valid_blk_cnt;
		u32 valid_nat_entry_cnt;
		u32 valid_node_cnt;
		u32 valid_inode_cnt;
		u32 multi_hard_link_files;
		u64 sit_valid_blocks;
		u32 sit_free_segs;
	} chk;

	struct hard_link_node *hard_link_list_head;

	char *main_seg_usage;
	char *main_area_bitmap;
	char *nat_area_bitmap;
	char *sit_area_bitmap;

	u64 main_area_bitmap_sz;
	u32 nat_area_bitmap_sz;
	u32 sit_area_bitmap_sz;

	u64 nr_main_blks;
	u32 nr_nat_entries;

	u32 dentry_depth;
	struct f2fs_nat_entry *entries;
	u32 nat_valid_inode_cnt;
};

#define BLOCK_SZ		4096
struct block {
	unsigned char buf[BLOCK_SZ];
};

enum NODE_TYPE {
	TYPE_INODE = 37,
	TYPE_DIRECT_NODE = 43,
	TYPE_INDIRECT_NODE = 53,
	TYPE_DOUBLE_INDIRECT_NODE = 67,
	TYPE_XATTR = 77
};

struct hard_link_node {
	u32 nid;
	u32 links;
	u32 actual_links;
	struct hard_link_node *next;
};

enum seg_type {
	SEG_TYPE_DATA,
	SEG_TYPE_CUR_DATA,
	SEG_TYPE_NODE,
	SEG_TYPE_CUR_NODE,
	SEG_TYPE_MAX,
};

struct selabel_handle;

extern int fsck_chk_orphan_node(struct f2fs_sb_info *);
extern int fsck_chk_node_blk(struct f2fs_sb_info *, struct f2fs_inode *, u32,
		u8 *, enum FILE_TYPE, enum NODE_TYPE, u32 *,
		struct child_info *);
extern void fsck_chk_inode_blk(struct f2fs_sb_info *, u32, enum FILE_TYPE,
		struct f2fs_node *, u32 *, struct node_info *);
extern int fsck_chk_dnode_blk(struct f2fs_sb_info *, struct f2fs_inode *,
		u32, enum FILE_TYPE, struct f2fs_node *, u32 *,
		struct child_info *, struct node_info *);
extern int fsck_chk_idnode_blk(struct f2fs_sb_info *, struct f2fs_inode *,
		enum FILE_TYPE, struct f2fs_node *, u32 *, struct child_info *);
extern int fsck_chk_didnode_blk(struct f2fs_sb_info *, struct f2fs_inode *,
		enum FILE_TYPE, struct f2fs_node *, u32 *, struct child_info *);
extern int fsck_chk_data_blk(struct f2fs_sb_info *sbi, u32, struct child_info *,
		int, enum FILE_TYPE, u32, u16, u8, int);
extern int fsck_chk_dentry_blk(struct f2fs_sb_info *, u32, struct child_info *,
		int, int);
int fsck_chk_inline_dentries(struct f2fs_sb_info *, struct f2fs_node *,
		struct child_info *);
int fsck_chk_meta(struct f2fs_sb_info *sbi);
int convert_encrypted_name(unsigned char *, int, unsigned char *, int);

extern void update_free_segments(struct f2fs_sb_info *);
void print_cp_state(u32);
extern void print_node_info(struct f2fs_node *, int);
extern void print_inode_info(struct f2fs_inode *, int);
extern struct seg_entry *get_seg_entry(struct f2fs_sb_info *, unsigned int);
extern struct f2fs_summary_block *get_sum_block(struct f2fs_sb_info *,
				unsigned int, int *);
extern int get_sum_entry(struct f2fs_sb_info *, u32, struct f2fs_summary *);
extern void update_sum_entry(struct f2fs_sb_info *, block_t,
				struct f2fs_summary *);
extern void get_node_info(struct f2fs_sb_info *, nid_t, struct node_info *);
extern void nullify_nat_entry(struct f2fs_sb_info *, u32);
extern void rewrite_sit_area_bitmap(struct f2fs_sb_info *);
extern void build_nat_area_bitmap(struct f2fs_sb_info *);
extern void build_sit_area_bitmap(struct f2fs_sb_info *);
extern void fsck_init(struct f2fs_sb_info *);
extern int fsck_verify(struct f2fs_sb_info *);
extern void fsck_free(struct f2fs_sb_info *);
extern int f2fs_do_mount(struct f2fs_sb_info *);
extern void f2fs_do_umount(struct f2fs_sb_info *);

extern void flush_journal_entries(struct f2fs_sb_info *);
extern void zero_journal_entries(struct f2fs_sb_info *);
extern void flush_sit_entries(struct f2fs_sb_info *);
extern void move_curseg_info(struct f2fs_sb_info *, u64);
extern void write_curseg_info(struct f2fs_sb_info *);
extern int find_next_free_block(struct f2fs_sb_info *, u64 *, int, int);
extern void write_checkpoint(struct f2fs_sb_info *);
extern void update_data_blkaddr(struct f2fs_sb_info *, nid_t, u16, block_t);
extern void update_nat_blkaddr(struct f2fs_sb_info *, nid_t, nid_t, block_t);

extern void print_raw_sb_info(struct f2fs_super_block *);

extern u32 get_free_segments(struct f2fs_sb_info *);
extern struct f2fs_sit_block *get_current_sit_page(struct f2fs_sb_info *,
			unsigned int);
extern void rewrite_current_sit_page(struct f2fs_sb_info *, unsigned int,
			struct f2fs_sit_block *);

/* dump.c */
struct dump_option {
	nid_t nid;
	int start_nat;
	int end_nat;
	int start_sit;
	int end_sit;
	int start_ssa;
	int end_ssa;
	int32_t blk_addr;
};

extern void nat_dump(struct f2fs_sb_info *);
extern void sit_dump(struct f2fs_sb_info *, unsigned int, unsigned int);
extern void ssa_dump(struct f2fs_sb_info *, int, int);
extern void dump_node(struct f2fs_sb_info *, nid_t, int);
extern int dump_info_from_blkaddr(struct f2fs_sb_info *, u32);

/* defrag.c */
int f2fs_defragment(struct f2fs_sb_info *, u64, u64, u64, int);

/* resize.c */
int f2fs_resize(struct f2fs_sb_info *);

/* sload.c */
int f2fs_sload(struct f2fs_sb_info *, const char *, const char *,
		const char *, struct selabel_handle *);
void reserve_new_block(struct f2fs_sb_info *, block_t *,
					struct f2fs_summary *, int);
void new_data_block(struct f2fs_sb_info *, void *,
					struct dnode_of_data *, int);
int f2fs_build_file(struct f2fs_sb_info *, struct dentry *);
void f2fs_alloc_nid(struct f2fs_sb_info *, nid_t *, int);
void set_data_blkaddr(struct dnode_of_data *);
block_t new_node_block(struct f2fs_sb_info *,
					struct dnode_of_data *, unsigned int);
void get_dnode_of_data(struct f2fs_sb_info *, struct dnode_of_data *,
					pgoff_t, int);
int f2fs_create(struct f2fs_sb_info *, struct dentry *);
int f2fs_mkdir(struct f2fs_sb_info *, struct dentry *);
int f2fs_symlink(struct f2fs_sb_info *, struct dentry *);
int inode_set_selinux(struct f2fs_sb_info *, u32, const char *);
int f2fs_find_path(struct f2fs_sb_info *, char *, nid_t *);

#endif /* _FSCK_H_ */
