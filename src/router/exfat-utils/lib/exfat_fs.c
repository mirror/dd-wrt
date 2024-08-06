// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2021 LG Electronics.
 *
 *   Author(s): Hyunchul Lee <hyc.lee@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exfat_ondisk.h"
#include "libexfat.h"

#include "exfat_fs.h"
#include "exfat_dir.h"

struct exfat_inode *exfat_alloc_inode(__u16 attr)
{
	struct exfat_inode *node;
	int size;

	size = offsetof(struct exfat_inode, name) + NAME_BUFFER_SIZE;
	node = calloc(1, size);
	if (!node) {
		exfat_err("failed to allocate exfat_node\n");
		return NULL;
	}

	node->parent = NULL;
	INIT_LIST_HEAD(&node->children);
	INIT_LIST_HEAD(&node->sibling);
	INIT_LIST_HEAD(&node->list);

	node->attr = attr;
	return node;
}

void exfat_free_inode(struct exfat_inode *node)
{
	if (node) {
		if (node->dentry_set)
			free(node->dentry_set);
		free(node);
	}
}

void exfat_free_children(struct exfat_inode *dir, bool file_only)
{
	struct exfat_inode *node, *i;

	list_for_each_entry_safe(node, i, &dir->children, sibling) {
		if (file_only) {
			if (!(node->attr & ATTR_SUBDIR)) {
				list_del(&node->sibling);
				exfat_free_inode(node);
			}
		} else {
			list_del(&node->sibling);
			list_del(&node->list);
			exfat_free_inode(node);
		}
	}
}

void exfat_free_file_children(struct exfat_inode *dir)
{
	exfat_free_children(dir, true);
}

/* delete @child and all ancestors that does not have
 * children
 */
void exfat_free_ancestors(struct exfat_inode *child)
{
	struct exfat_inode *parent;

	while (child && list_empty(&child->children)) {
		if (!child->parent || !(child->attr & ATTR_SUBDIR))
			return;

		parent = child->parent;
		list_del(&child->sibling);
		exfat_free_inode(child);

		child = parent;
	}
	return;
}

void exfat_free_dir_list(struct exfat *exfat)
{
	struct exfat_inode *dir, *i;

	list_for_each_entry_safe(dir, i, &exfat->dir_list, list) {
		if (!dir->parent)
			continue;
		exfat_free_file_children(dir);
		list_del(&dir->list);
		exfat_free_inode(dir);
	}
}

void exfat_free_exfat(struct exfat *exfat)
{
	if (exfat) {
		if (exfat->bs)
			free(exfat->bs);
		if (exfat->alloc_bitmap)
			free(exfat->alloc_bitmap);
		if (exfat->disk_bitmap)
			free(exfat->disk_bitmap);
		if (exfat->ohead_bitmap)
			free(exfat->ohead_bitmap);
		if (exfat->upcase_table)
			free(exfat->upcase_table);
		if (exfat->root)
			exfat_free_inode(exfat->root);
		if (exfat->lookup_buffer)
			free(exfat->lookup_buffer);
		free(exfat);
	}
}

struct exfat *exfat_alloc_exfat(struct exfat_blk_dev *blk_dev, struct pbr *bs)
{
	struct exfat *exfat;

	exfat = calloc(1, sizeof(*exfat));
	if (!exfat) {
		free(bs);
		return NULL;
	}

	INIT_LIST_HEAD(&exfat->dir_list);
	exfat->blk_dev = blk_dev;
	exfat->bs = bs;
	exfat->clus_count = le32_to_cpu(bs->bsx.clu_count);
	exfat->clus_size = EXFAT_CLUSTER_SIZE(bs);
	exfat->sect_size = EXFAT_SECTOR_SIZE(bs);

	/* TODO: bitmap could be very large. */
	exfat->alloc_bitmap = calloc(1, EXFAT_BITMAP_SIZE(exfat->clus_count));
	if (!exfat->alloc_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		goto err;
	}

	exfat->ohead_bitmap = calloc(1, EXFAT_BITMAP_SIZE(exfat->clus_count));
	if (!exfat->ohead_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		goto err;
	}

	exfat->disk_bitmap = calloc(1, EXFAT_BITMAP_SIZE(exfat->clus_count));
	if (!exfat->disk_bitmap) {
		exfat_err("failed to allocate bitmap\n");
		goto err;
	}

	exfat->buffer_count = ((MAX_EXT_DENTRIES + 1) * DENTRY_SIZE) /
		exfat_get_read_size(exfat) + 1;

	exfat->start_clu = EXFAT_FIRST_CLUSTER;
	return exfat;
err:
	exfat_free_exfat(exfat);
	return NULL;
}

struct buffer_desc *exfat_alloc_buffer(struct exfat *exfat)
{
	struct buffer_desc *bd;
	unsigned int i;
	unsigned int read_size = exfat_get_read_size(exfat);

	bd = calloc(exfat->buffer_count, sizeof(*bd));
	if (!bd)
		return NULL;

	for (i = 0; i < exfat->buffer_count; i++) {
		bd[i].buffer = malloc(read_size);
		if (!bd[i].buffer)
			goto err;

		memset(&bd[i].dirty, 0, sizeof(bd[i].dirty));
	}
	return bd;
err:
	exfat_free_buffer(exfat, bd);
	return NULL;
}

void exfat_free_buffer(const struct exfat *exfat, struct buffer_desc *bd)
{
	unsigned int i;

	for (i = 0; i < exfat->buffer_count; i++) {
		if (bd[i].buffer)
			free(bd[i].buffer);
	}
	free(bd);
}

/*
 * get references of ancestors that include @child until the count of
 * ancesters is not larger than @count and the count of characters of
 * their names is not larger than @max_char_len.
 * return true if root is reached.
 */
static bool get_ancestors(struct exfat_inode *child,
			  struct exfat_inode **ancestors, int count,
			  int max_char_len,
			  int *ancestor_count)
{
	struct exfat_inode *dir;
	int name_len, char_len;
	int root_depth, depth, i;

	root_depth = 0;
	char_len = 0;
	max_char_len += 1;

	dir = child;
	while (dir) {
		name_len = exfat_utf16_len(dir->name, NAME_BUFFER_SIZE);
		if (char_len + name_len > max_char_len)
			break;

		/* include '/' */
		char_len += name_len + 1;
		root_depth++;

		dir = dir->parent;
	}

	depth = MIN(root_depth, count);

	for (dir = child, i = depth - 1; i >= 0; dir = dir->parent, i--)
		ancestors[i] = dir;

	*ancestor_count = depth;
	return !dir;
}

int exfat_resolve_path(struct path_resolve_ctx *ctx, struct exfat_inode *child)
{
	int depth, i;
	int name_len;
	__le16 *utf16_path;
	static const __le16 utf16_slash = cpu_to_le16(0x002F);
	static const __le16 utf16_null = cpu_to_le16(0x0000);
	size_t in_size;

	ctx->local_path[0] = '\0';

	get_ancestors(child,
		      ctx->ancestors,
		      sizeof(ctx->ancestors) / sizeof(ctx->ancestors[0]),
		      PATH_MAX,
		      &depth);

	utf16_path = ctx->utf16_path;
	for (i = 0; i < depth; i++) {
		name_len = exfat_utf16_len(ctx->ancestors[i]->name,
					   NAME_BUFFER_SIZE);
		memcpy((char *)utf16_path, (char *)ctx->ancestors[i]->name,
		       name_len * 2);
		utf16_path += name_len;
		memcpy((char *)utf16_path, &utf16_slash, sizeof(utf16_slash));
		utf16_path++;
	}

	if (depth > 1)
		utf16_path--;
	memcpy((char *)utf16_path, &utf16_null, sizeof(utf16_null));
	utf16_path++;

	in_size = (utf16_path - ctx->utf16_path) * sizeof(__le16);
	return exfat_utf16_dec(ctx->utf16_path, in_size,
				ctx->local_path, sizeof(ctx->local_path));
}

int exfat_resolve_path_parent(struct path_resolve_ctx *ctx,
			      struct exfat_inode *parent, struct exfat_inode *child)
{
	int ret;
	struct exfat_inode *old;

	old = child->parent;
	child->parent = parent;

	ret = exfat_resolve_path(ctx, child);
	child->parent = old;
	return ret;
}
