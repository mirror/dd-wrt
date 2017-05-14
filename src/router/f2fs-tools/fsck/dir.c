/**
 * dir.c
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
#include "fsck.h"
#include "node.h"

static unsigned int dir_buckets(unsigned int level)
{
	if (level < MAX_DIR_HASH_DEPTH / 2)
		return 1 << level;
	else
		return MAX_DIR_BUCKETS;
}

static unsigned int bucket_blocks(unsigned int level)
{
	if (level < MAX_DIR_HASH_DEPTH / 2)
		return 2;
	else
		return 4;
}

static unsigned long dir_block_index(unsigned int level,
		int dir_level, unsigned int idx)
{
	unsigned long i;
	unsigned long bidx = 0;

	for (i = 0; i < level; i++)
		bidx += dir_buckets(i + dir_level) * bucket_blocks(i);
	bidx += idx * bucket_blocks(level);
	return bidx;
}

static int room_for_filename(const u8 *bitmap, int slots, int max_slots)
{
	int bit_start = 0;
	int zero_start, zero_end;
next:
	zero_start = find_next_zero_bit_le(bitmap, max_slots, bit_start);
	if (zero_start >= max_slots)
		return max_slots;

	zero_end = find_next_bit_le(bitmap, max_slots, zero_start + 1);

	if (zero_end - zero_start >= slots)
		return zero_start;
	bit_start = zero_end;
	goto next;

}

static void make_dentry_ptr(struct f2fs_dentry_ptr *d, void *src, int type)
{
	if (type == 1) {
		struct f2fs_dentry_block *t = (struct f2fs_dentry_block *)src;
		d->max = NR_DENTRY_IN_BLOCK;
		d->bitmap = t->dentry_bitmap;
		d->dentry = t->dentry;
		d->filename = t->filename;
	} else {
		struct f2fs_inline_dentry *t = (struct f2fs_inline_dentry *)src;
		d->max = NR_INLINE_DENTRY;
		d->bitmap = t->dentry_bitmap;
		d->dentry = t->dentry;
		d->filename = t->filename;
	}
}

static struct f2fs_dir_entry *find_target_dentry(const u8 *name,
		unsigned int len, f2fs_hash_t namehash, int *max_slots,
		struct f2fs_dentry_ptr *d)
{
	struct f2fs_dir_entry *de;
	unsigned long bit_pos = 0;
	int max_len = 0;

	if (max_slots)
		*max_slots = 0;
	while (bit_pos < d->max) {
		if (!test_bit_le(bit_pos, d->bitmap)) {
			bit_pos++;
			max_len++;
			continue;
		}

		de = &d->dentry[bit_pos];
		if (le16_to_cpu(de->name_len) == len &&
			de->hash_code == namehash &&
			!memcmp(d->filename[bit_pos], name, len)) {
			goto found;
		}

		if (max_slots && max_len > *max_slots)
			*max_slots = max_len;
		max_len = 0;
		bit_pos += GET_DENTRY_SLOTS(le16_to_cpu(de->name_len));
	}
	de = NULL;
found:
	if (max_slots && max_len > *max_slots)
		*max_slots = max_len;
	return de;
}

static struct f2fs_dir_entry *find_in_block(void *block,
		const u8 *name, int len, f2fs_hash_t namehash,
		int *max_slots)
{
	struct f2fs_dentry_ptr d;

	make_dentry_ptr(&d, block, 1);
	return find_target_dentry(name, len, namehash, max_slots, &d);
}

static int find_in_level(struct f2fs_sb_info *sbi,struct f2fs_node *dir,
		unsigned int level, struct dentry *de)
{
	unsigned int nbucket, nblock;
	unsigned int bidx, end_block;
	struct f2fs_dir_entry *dentry = NULL;
	struct dnode_of_data dn = {0};
	void *dentry_blk;
	int max_slots = 214;
	nid_t ino = le32_to_cpu(dir->footer.ino);
	f2fs_hash_t namehash;
	int ret = 0;

	namehash = f2fs_dentry_hash(de->name, de->len);

	nbucket = dir_buckets(level);
	nblock = bucket_blocks(level);

	bidx = dir_block_index(level, 0, le32_to_cpu(namehash) % nbucket);
	end_block = bidx + nblock;

	dentry_blk = calloc(BLOCK_SZ, 1);
	ASSERT(dentry_blk);

	for (; bidx < end_block; bidx++) {

		/* Firstly, we should know direct node of target data blk */
		if (dn.node_blk && dn.node_blk != dn.inode_blk)
			free(dn.node_blk);

		set_new_dnode(&dn, dir, NULL, ino);
		get_dnode_of_data(sbi, &dn, bidx, LOOKUP_NODE);
		if (dn.data_blkaddr == NULL_ADDR)
			continue;

		ret = dev_read_block(dentry_blk, dn.data_blkaddr);
		ASSERT(ret >= 0);

		dentry = find_in_block(dentry_blk, de->name, de->len,
						namehash, &max_slots);
		if (dentry) {
			ret = 1;
			de->ino = le32_to_cpu(dentry->ino);
			break;
		}
	}

	if (dn.node_blk && dn.node_blk != dn.inode_blk)
		free(dn.node_blk);
	free(dentry_blk);

	return ret;
}

static int f2fs_find_entry(struct f2fs_sb_info *sbi,
				struct f2fs_node *dir, struct dentry *de)
{
	unsigned int max_depth;
	unsigned int level;

	if (dir->i.i_inline & F2FS_INLINE_DENTRY) {
		ERR_MSG("Not support to find \"%s\" in inline_dir pino=%x\n",
				de->name, de->pino);
		return 0;
	}

	max_depth = le32_to_cpu(dir->i.i_current_depth);
	for (level = 0; level < max_depth; level ++) {
		if (find_in_level(sbi, dir, level, de))
			return 1;
	}
	return 0;
}

static void f2fs_update_dentry(nid_t ino, umode_t mode,
		struct f2fs_dentry_ptr *d,
		const unsigned char *name, int len, f2fs_hash_t name_hash,
		unsigned int bit_pos)
{
	struct f2fs_dir_entry *de;
	int slots = GET_DENTRY_SLOTS(len);
	int i;

	de = &d->dentry[bit_pos];
	de->name_len = cpu_to_le16(len);
	de->hash_code = name_hash;
	memcpy(d->filename[bit_pos], name, len);
	d->filename[bit_pos][len] = 0;
	de->ino = cpu_to_le32(ino);
	set_de_type(de, mode);
	for (i = 0; i < slots; i++)
		test_and_set_bit_le(bit_pos + i, d->bitmap);
}

/*
 * f2fs_add_link - Add a new file(dir) to parent dir.
 */
static int f2fs_add_link(struct f2fs_sb_info *sbi, struct f2fs_node *parent,
			struct f2fs_node *child, block_t p_blkaddr)
{
	int level = 0, current_depth, bit_pos;
	int nbucket, nblock, bidx, block;
	const unsigned char *name = child->i.i_name;
	int name_len = le32_to_cpu(child->i.i_namelen);
	int slots = GET_DENTRY_SLOTS(name_len);
	f2fs_hash_t dentry_hash = f2fs_dentry_hash(name, name_len);
	struct f2fs_dentry_block *dentry_blk;
	struct f2fs_dentry_ptr d;
	struct dnode_of_data dn = {0};
	nid_t pino = le32_to_cpu(parent->footer.ino);
	nid_t ino = le32_to_cpu(child->footer.ino);
	umode_t mode = le16_to_cpu(child->i.i_mode);
	int ret;

	if (parent == NULL || child == NULL)
		return -EINVAL;

	if (!pino) {
		ERR_MSG("Wrong parent ino:%d \n", pino);
		return -EINVAL;
	}

	dentry_blk = calloc(BLOCK_SZ, 1);
	ASSERT(dentry_blk);

	current_depth = le32_to_cpu(parent->i.i_current_depth);
start:
	if (current_depth == MAX_DIR_HASH_DEPTH) {
		free(dentry_blk);
		ERR_MSG("\tError: MAX_DIR_HASH\n");
		return -ENOSPC;
	}

	/* Need a new dentry block */
	if (level == current_depth)
		++current_depth;

	nbucket = dir_buckets(level);
	nblock = bucket_blocks(level);
	bidx = dir_block_index(level, 0, le32_to_cpu(dentry_hash) % nbucket);

	for (block = bidx; block <= (bidx + nblock - 1); block++) {

		/* Firstly, we should know the direct node of target data blk */
		if (dn.node_blk && dn.node_blk != dn.inode_blk)
			free(dn.node_blk);

		set_new_dnode(&dn, parent, NULL, pino);
		get_dnode_of_data(sbi, &dn, block, ALLOC_NODE);

		if (dn.data_blkaddr == NULL_ADDR) {
			new_data_block(sbi, dentry_blk, &dn, CURSEG_HOT_DATA);
		} else {
			ret = dev_read_block(dentry_blk, dn.data_blkaddr);
			ASSERT(ret >= 0);
		}
		bit_pos = room_for_filename(dentry_blk->dentry_bitmap,
				slots, NR_DENTRY_IN_BLOCK);

		if (bit_pos < NR_DENTRY_IN_BLOCK)
			goto add_dentry;
	}
	level ++;
	goto start;

add_dentry:
	make_dentry_ptr(&d, (void *)dentry_blk, 1);
	f2fs_update_dentry(ino, mode, &d, name, name_len, dentry_hash, bit_pos);

	ret = dev_write_block(dentry_blk, dn.data_blkaddr);
	ASSERT(ret >= 0);

	/*
	 * Parent inode needs updating, because its inode info may be changed.
	 * such as i_current_depth and i_blocks.
	 */
	if (parent->i.i_current_depth != cpu_to_le32(current_depth)) {
		parent->i.i_current_depth = cpu_to_le32(current_depth);
		dn.idirty = 1;
	}

	/* Update parent's i_links info*/
	if (S_ISDIR(mode)) {
		u32 links = le32_to_cpu(parent->i.i_links);
		parent->i.i_links = cpu_to_le32(links + 1);
		dn.idirty = 1;
	}

	if ((block + 1) * F2FS_BLKSIZE > le64_to_cpu(parent->i.i_size)) {
		parent->i.i_size = cpu_to_le64((block + 1) * F2FS_BLKSIZE);
		dn.idirty = 1;
	}

	if (dn.ndirty) {
		ret = dev_write_block(dn.node_blk, dn.node_blkaddr);
		ASSERT(ret >= 0);
	}

	if (dn.idirty) {
		ASSERT(parent == dn.inode_blk);
		ret = dev_write_block(dn.inode_blk, p_blkaddr);
		ASSERT(ret >= 0);
	}

	if (dn.node_blk != dn.inode_blk)
		free(dn.node_blk);
	free(dentry_blk);
	return 0;
}

static void make_empty_dir(struct f2fs_sb_info *sbi, struct f2fs_node *inode)
{
	struct f2fs_dentry_block *dent_blk;
	nid_t ino = le32_to_cpu(inode->footer.ino);
	nid_t pino = le32_to_cpu(inode->i.i_pino);
	struct f2fs_summary sum;
	struct node_info ni;
	block_t blkaddr;
	int ret;

	get_node_info(sbi, ino, &ni);

	dent_blk = calloc(BLOCK_SZ, 1);
	ASSERT(dent_blk);

	dent_blk->dentry[0].hash_code = 0;
	dent_blk->dentry[0].ino = cpu_to_le32(ino);
	dent_blk->dentry[0].name_len = cpu_to_le16(1);
	dent_blk->dentry[0].file_type = F2FS_FT_DIR;
	memcpy(dent_blk->filename[0], ".", 1);

	dent_blk->dentry[1].hash_code = 0;
	dent_blk->dentry[1].ino = cpu_to_le32(pino);
	dent_blk->dentry[1].name_len = cpu_to_le16(2);
	dent_blk->dentry[1].file_type = F2FS_FT_DIR;
	memcpy(dent_blk->filename[1], "..", 2);

	test_and_set_bit_le(0, dent_blk->dentry_bitmap);
	test_and_set_bit_le(1, dent_blk->dentry_bitmap);

	set_summary(&sum, ino, 0, ni.version);
	reserve_new_block(sbi, &blkaddr, &sum, CURSEG_HOT_DATA);

	ret = dev_write_block(dent_blk, blkaddr);
	ASSERT(ret >= 0);

	inode->i.i_addr[0] = cpu_to_le32(blkaddr);
	free(dent_blk);
}

static void page_symlink(struct f2fs_sb_info *sbi, struct f2fs_node *inode,
					const char *symname, int symlen)
{
	nid_t ino = le32_to_cpu(inode->footer.ino);
	struct f2fs_summary sum;
	struct node_info ni;
	char *data_blk;
	block_t blkaddr;
	int ret;

	get_node_info(sbi, ino, &ni);

	/* store into inline_data */
	if (symlen + 1 <= MAX_INLINE_DATA) {
		inode->i.i_inline |= F2FS_INLINE_DATA;
		inode->i.i_inline |= F2FS_DATA_EXIST;
		memcpy(&inode->i.i_addr[1], symname, symlen);
		return;
	}

	data_blk = calloc(BLOCK_SZ, 1);
	ASSERT(data_blk);

	memcpy(data_blk, symname, symlen);

	set_summary(&sum, ino, 0, ni.version);
	reserve_new_block(sbi, &blkaddr, &sum, CURSEG_WARM_DATA);

	ret = dev_write_block(data_blk, blkaddr);
	ASSERT(ret >= 0);

	inode->i.i_addr[0] = cpu_to_le32(blkaddr);
	free(data_blk);
}

static void init_inode_block(struct f2fs_sb_info *sbi,
		struct f2fs_node *node_blk, struct dentry *de)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	mode_t mode = de->mode;
	int links = 1;
	unsigned int size;
	int blocks = 1;

	if (de->file_type == F2FS_FT_DIR) {
		mode |= S_IFDIR;
		size = 4096;
		links++;
		blocks++;
	} else if (de->file_type == F2FS_FT_REG_FILE) {
		mode |= S_IFREG;
		size = 0;
	} else if (de->file_type == F2FS_FT_SYMLINK) {
		ASSERT(de->link);
		mode |= S_IFLNK;
		size = strlen(de->link);
		if (size + 1 > MAX_INLINE_DATA)
			blocks++;
	} else {
		ASSERT(0);
	}

	node_blk->i.i_mode = cpu_to_le16(mode);
	node_blk->i.i_advise = 0;
	node_blk->i.i_uid = cpu_to_le32(de->uid);
	node_blk->i.i_gid = cpu_to_le32(de->gid);
	node_blk->i.i_links = cpu_to_le32(links);
	node_blk->i.i_size = cpu_to_le32(size);
	node_blk->i.i_blocks = cpu_to_le32(blocks);
	node_blk->i.i_atime = cpu_to_le64(de->mtime);
	node_blk->i.i_ctime = cpu_to_le64(de->mtime);
	node_blk->i.i_mtime = cpu_to_le64(de->mtime);
	node_blk->i.i_atime_nsec = 0;
	node_blk->i.i_ctime_nsec = 0;
	node_blk->i.i_mtime_nsec = 0;
	node_blk->i.i_generation = 0;
	node_blk->i.i_current_depth = cpu_to_le32(1);
	node_blk->i.i_xattr_nid = 0;
	node_blk->i.i_flags = 0;
	node_blk->i.i_inline = F2FS_INLINE_XATTR;
	node_blk->i.i_pino = cpu_to_le32(de->pino);
	node_blk->i.i_namelen = cpu_to_le32(de->len);
	memcpy(node_blk->i.i_name, de->name, de->len);
	node_blk->i.i_name[de->len] = 0;

	node_blk->footer.ino = cpu_to_le32(de->ino);
	node_blk->footer.nid = cpu_to_le32(de->ino);
	node_blk->footer.flag = 0;
	node_blk->footer.cp_ver = ckpt->checkpoint_ver;

	if (S_ISDIR(mode))
		make_empty_dir(sbi, node_blk);
	else if (S_ISLNK(mode))
		page_symlink(sbi, node_blk, de->link, size);
}

int f2fs_create(struct f2fs_sb_info *sbi, struct dentry *de)
{
	struct f2fs_node *parent, *child;
	struct node_info ni;
	struct f2fs_summary sum;
	block_t blkaddr;
	int ret;

	/* Find if there is a */
	get_node_info(sbi, de->pino, &ni);
	if (ni.blk_addr == NULL_ADDR) {
		MSG(0, "No parent directory pino=%x\n", de->pino);
		return -1;
	}

	parent = calloc(BLOCK_SZ, 1);
	ASSERT(parent);

	ret = dev_read_block(parent, ni.blk_addr);
	ASSERT(ret >= 0);

	ret = f2fs_find_entry(sbi, parent, de);
	if (ret) {
		MSG(0, "Skip the existing \"%s\" pino=%x ERR=%d\n",
					de->name, de->pino, ret);
		if (de->file_type == F2FS_FT_REG_FILE)
			de->ino = 0;
		goto free_parent_dir;
	}
	if (parent->i.i_inline & F2FS_INLINE_DENTRY) {
		ERR_MSG("Not support adding \"%s\" in inline_dir pino=%x\n",
					de->name, de->pino);
		if (de->file_type == F2FS_FT_REG_FILE)
			de->ino = 0;
		goto free_parent_dir;
	}

	child = calloc(BLOCK_SZ, 1);
	ASSERT(child);

	f2fs_alloc_nid(sbi, &de->ino, 1);

	init_inode_block(sbi, child, de);

	ret = f2fs_add_link(sbi, parent, child, ni.blk_addr);
	if (ret) {
		MSG(0, "Skip the existing \"%s\" pino=%x ERR=%d\n",
					de->name, de->pino, ret);
		goto free_child_dir;
	}

	/* write child */
	set_summary(&sum, de->ino, 0, ni.version);
	reserve_new_block(sbi, &blkaddr, &sum, CURSEG_HOT_NODE);

	/* update nat info */
	update_nat_blkaddr(sbi, de->ino, de->ino, blkaddr);

	ret = dev_write_block(child, blkaddr);
	ASSERT(ret >= 0);

	update_free_segments(sbi);
	MSG(1, "Info: Create \"%s\" type=%x, ino=%x / %x into \"%s\"\n",
			de->full_path, de->file_type,
			de->ino, de->pino, de->path);
free_child_dir:
	free(child);
free_parent_dir:
	free(parent);
	return 0;
}

int f2fs_mkdir(struct f2fs_sb_info *sbi, struct dentry *de)
{
	return f2fs_create(sbi, de);
}

int f2fs_symlink(struct f2fs_sb_info *sbi, struct dentry *de)
{
	return f2fs_create(sbi, de);
}

int f2fs_find_path(struct f2fs_sb_info *sbi, char *path, nid_t *ino)
{
	struct f2fs_node *parent;
	struct node_info ni;
	struct dentry de;
	int err = 0;
	int ret;
	char *p;

	if (path[0] != '/')
		return -ENOENT;

	*ino = F2FS_ROOT_INO(sbi);
	parent = calloc(BLOCK_SZ, 1);
	ASSERT(parent);

	p = strtok(path, "/");
	while (p) {
		de.name = (const u8 *)p;
		de.len = strlen(p);

		get_node_info(sbi, *ino, &ni);
		if (ni.blk_addr == NULL_ADDR) {
			err = -ENOENT;
			goto err;
		}
		ret = dev_read_block(parent, ni.blk_addr);
		ASSERT(ret >= 0);

		ret = f2fs_find_entry(sbi, parent, &de);
		if (!ret) {
			err = -ENOENT;
			goto err;
		}

		*ino = de.ino;
		p = strtok(NULL, "/");
	}
err:
	free(parent);
	return err;
}
