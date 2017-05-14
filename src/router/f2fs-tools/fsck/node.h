/**
 * node.h
 *
 * Many parts of codes are copied from Linux kernel/fs/f2fs.
 *
 * Copyright (C) 2015 Huawei Ltd.
 * Witten by:
 *   Hou Pengyang <houpengyang@huawei.com>
 *   Liu Shuoran <liushuoran@huawei.com>
 *   Jaegeuk Kim <jaegeuk@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _NODE_H_
#define _NODE_H_

#include "fsck.h"

#define ADDRS_PER_PAGE(page) \
	(IS_INODE(page) ? ADDRS_PER_INODE(&page->i) : ADDRS_PER_BLOCK)

static inline int IS_INODE(struct f2fs_node *node)
{
	return ((node)->footer.nid == (node)->footer.ino);
}

static inline __le32 *blkaddr_in_node(struct f2fs_node *node)
{
	return IS_INODE(node) ? node->i.i_addr : node->dn.addr;
}

static inline block_t datablock_addr(struct f2fs_node *node_page,
					unsigned int offset)
{
	__le32 *addr_array;

	ASSERT(node_page);
	addr_array = blkaddr_in_node(node_page);
	return le32_to_cpu(addr_array[offset]);
}

static inline void set_nid(struct f2fs_node * rn, int off, nid_t nid, int i)
{
	if (i)
		rn->i.i_nid[off - NODE_DIR1_BLOCK] = cpu_to_le32(nid);
	else
		rn->in.nid[off] = cpu_to_le32(nid);
}

static inline nid_t get_nid(struct f2fs_node * rn, int off, int i)
{
	if (i)
		return le32_to_cpu(rn->i.i_nid[off - NODE_DIR1_BLOCK]);
	else
		return le32_to_cpu(rn->in.nid[off]);
}

enum {
	ALLOC_NODE,	/* allocate a new node page if needed */
	LOOKUP_NODE,	/* lookup up a node without readahead */
	LOOKUP_NODE_RA,
};

static inline void set_new_dnode(struct dnode_of_data *dn,
		struct f2fs_node *iblk, struct f2fs_node *nblk, nid_t nid)
{
	memset(dn, 0, sizeof(*dn));
	dn->inode_blk = iblk;
	dn->node_blk = nblk;
	dn->nid = nid;
	dn->idirty = 0;
	dn->ndirty = 0;
}

static inline void inc_inode_blocks(struct dnode_of_data *dn)
{
	u64 blocks = le64_to_cpu(dn->inode_blk->i.i_blocks);

	dn->inode_blk->i.i_blocks = cpu_to_le64(blocks + 1);
	dn->idirty = 1;
}

static inline int IS_DNODE(struct f2fs_node *node_page)
{
	unsigned int ofs = ofs_of_node(node_page);

	if (ofs == 3 || ofs == 4 + NIDS_PER_BLOCK ||
			ofs == 5 + 2 * NIDS_PER_BLOCK)
		return 0;

	if (ofs >= 6 + 2 * NIDS_PER_BLOCK) {
		ofs -= 6 + 2 * NIDS_PER_BLOCK;
		if (!((long int)ofs % (NIDS_PER_BLOCK + 1)))
			return 0;
	}
	return 1;
}

#endif
