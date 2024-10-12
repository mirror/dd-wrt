/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdlib.h>
#include <apfs/raw.h>
#include "btree.h"
#include "dir.h"
#include "mkapfs.h"
#include "object.h"

/* Constants used in managing the size of a node's table of contents */
#define BTREE_TOC_ENTRY_INCREMENT	8
#define BTREE_TOC_ENTRY_MAX_UNUSED	(2 * BTREE_TOC_ENTRY_INCREMENT)

/**
 * set_empty_btree_info - Set the info footer for an empty b-tree node
 * @info:	pointer to the on-disk info footer
 * @subtype:	subtype of the root node, i.e., tree type
 *
 * Should only be called for the free queues, the snapshot metadata tree, and
 * the extent reference tree.
 */
static void set_empty_btree_info(struct apfs_btree_info *info, u32 subtype)
{
	u16 flags;

	if (subtype == APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE)
		flags = APFS_BTREE_EPHEMERAL | APFS_BTREE_ALLOW_GHOSTS;
	else
		flags = APFS_BTREE_PHYSICAL | APFS_BTREE_KV_NONALIGNED;

	info->bt_fixed.bt_flags = cpu_to_le32(flags);
	info->bt_fixed.bt_node_size = cpu_to_le32(param->blocksize);

	if (subtype == APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE) {
		/* The other two trees don't have fixed key/value sizes */
		info->bt_fixed.bt_key_size =
		       cpu_to_le32(sizeof(struct apfs_spaceman_free_queue_key));
		info->bt_fixed.bt_val_size = cpu_to_le32(8);
		info->bt_longest_key =
		       cpu_to_le32(sizeof(struct apfs_spaceman_free_queue_key));
		info->bt_longest_val = cpu_to_le32(8);
	}
	info->bt_node_count = cpu_to_le64(1); /* Only one node: the root */
}

/**
 * min_table_size - Get the minimum size of the table of contents for a leaf
 * @type: btree type for the leaf node
 */
static int min_table_size(u32 type)
{
	int key_size, val_size, toc_size;
	int space, count;

	/* Trees with fixed key/value sizes preallocate the whole table */
	switch (type) {
	case APFS_OBJECT_TYPE_OMAP:
		key_size = sizeof(struct apfs_omap_key);
		val_size = sizeof(struct apfs_omap_val);
		toc_size = sizeof(struct apfs_kvoff);
		break;
	case APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE:
		key_size = sizeof(struct apfs_spaceman_free_queue_key);
		val_size = sizeof(__le64); /* We assume no ghosts here */
		toc_size = sizeof(struct apfs_kvoff);
		break;
	default:
		/* It should at least have room for one record */
		return sizeof(struct apfs_kvloc) * BTREE_TOC_ENTRY_MAX_UNUSED;
	}

	/* The footer of root nodes is ignored for some reason */
	space = param->blocksize - sizeof(struct apfs_btree_node_phys);
	count = space / (key_size + val_size + toc_size);
	return count * toc_size;
}

/**
 * make_empty_btree_root - Make an empty root for a b-tree
 * @bno:	block number to use
 * @oid:	object id to use
 * @subtype:	subtype of the root node, i.e., tree type
 *
 * Should only be called for the free queues, the snapshot metadata tree, and
 * the extent reference tree.
 */
void make_empty_btree_root(u64 bno, u64 oid, u32 subtype)
{
	struct apfs_btree_node_phys *root = get_zeroed_block(bno);
	u32 type;
	u16 flags;
	int toc_len, free_len;
	int head_len = sizeof(*root);
	int info_len = sizeof(struct apfs_btree_info);

	flags = APFS_BTNODE_ROOT | APFS_BTNODE_LEAF;
	if (subtype == APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE)
		flags |= APFS_BTNODE_FIXED_KV_SIZE;
	root->btn_flags = cpu_to_le16(flags);

	toc_len = min_table_size(subtype);

	/* No keys and no values, so this is straightforward */
	root->btn_nkeys = 0;
	free_len = param->blocksize - head_len - toc_len - info_len;
	root->btn_table_space.off = 0;
	root->btn_table_space.len = cpu_to_le16(toc_len);
	root->btn_free_space.off = 0;
	root->btn_free_space.len = cpu_to_le16(free_len);

	/* No fragmentation */
	root->btn_key_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_key_free_list.len = 0;
	root->btn_val_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_val_free_list.len = 0;

	set_empty_btree_info((void *)root + param->blocksize - info_len,
			     subtype);

	type = APFS_OBJECT_TYPE_BTREE;
	if (subtype == APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE)
		type |= APFS_OBJ_EPHEMERAL;
	else
		type |= APFS_OBJ_PHYSICAL;
	set_object_header(&root->btn_o, param->blocksize, oid, type, subtype);

	munmap(root, param->blocksize);
}

/**
 * set_omap_info - Set the info footer for an object map node
 * @info:	pointer to the on-disk info footer
 * @nkeys:	number of records in the omap
 */
static void set_omap_info(struct apfs_btree_info *info, int nkeys)
{
	info->bt_fixed.bt_flags = cpu_to_le32(APFS_BTREE_PHYSICAL);
	info->bt_fixed.bt_node_size = cpu_to_le32(param->blocksize);
	info->bt_fixed.bt_key_size = cpu_to_le32(sizeof(struct apfs_omap_key));
	info->bt_fixed.bt_val_size = cpu_to_le32(sizeof(struct apfs_omap_val));
	info->bt_longest_key = cpu_to_le32(sizeof(struct apfs_omap_key));
	info->bt_longest_val = cpu_to_le32(sizeof(struct apfs_omap_val));
	info->bt_key_count = cpu_to_le64(nkeys);
	info->bt_node_count = cpu_to_le64(1); /* Only one node: the root */
}

/**
 * make_omap_root - Make the root node of an object map
 * @bno:	block number to use
 * @is_vol:	is this the object map for a volume?
 */
static void make_omap_root(u64 bno, bool is_vol)
{
	struct apfs_btree_node_phys *root = get_zeroed_block(bno);
	struct apfs_omap_key *key;
	struct apfs_omap_val *val;
	struct apfs_kvoff *kvoff;
	int toc_len, key_len, val_len, free_len;
	int head_len = sizeof(*root);
	int info_len = sizeof(struct apfs_btree_info);

	root->btn_flags = cpu_to_le16(APFS_BTNODE_ROOT | APFS_BTNODE_LEAF |
				      APFS_BTNODE_FIXED_KV_SIZE);

	/* The mkfs will need only one record on each omap */
	root->btn_nkeys = cpu_to_le32(1);
	toc_len = min_table_size(APFS_OBJECT_TYPE_OMAP);
	key_len = 1 * sizeof(*key);
	val_len = 1 * sizeof(*val);

	/* Location of the one record */
	key = (void *)root + head_len + toc_len;
	val = (void *)root + param->blocksize - info_len - val_len;
	kvoff = (void *)root + head_len;
	kvoff->k = 0;
	kvoff->v = cpu_to_le16(val_len);

	/* Set the key and value for the one record */
	if (is_vol) {
		/* Map for the catalog root */
		key->ok_oid = cpu_to_le64(FIRST_VOL_CAT_ROOT_OID);
		val->ov_paddr = cpu_to_le64(FIRST_VOL_CAT_ROOT_BNO);
	} else {
		/* Map for the one volume superblock */
		key->ok_oid = cpu_to_le64(FIRST_VOL_OID);
		val->ov_paddr = cpu_to_le64(FIRST_VOL_BNO);
	}
	key->ok_xid = cpu_to_le64(MKFS_XID);
	val->ov_size = cpu_to_le32(param->blocksize); /* Only size supported */

	root->btn_table_space.off = 0;
	root->btn_table_space.len = cpu_to_le16(toc_len);

	free_len = param->blocksize -
		   head_len - toc_len - key_len - val_len - info_len;
	root->btn_free_space.off = cpu_to_le16(key_len);
	root->btn_free_space.len = cpu_to_le16(free_len);

	/* No fragmentation */
	root->btn_key_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_key_free_list.len = 0;
	root->btn_val_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_val_free_list.len = 0;

	set_omap_info((void *)root + param->blocksize - info_len, 1);
	set_object_header(&root->btn_o, param->blocksize, bno,
			  APFS_OBJECT_TYPE_BTREE | APFS_OBJ_PHYSICAL,
			  APFS_OBJECT_TYPE_OMAP);
	munmap(root, param->blocksize);
}

/**
 * make_omap_btree - Make an object map
 * @bno:	block number to use
 * @is_vol:	is this the object map for a volume?
 */
void make_omap_btree(u64 bno, bool is_vol)
{
	struct apfs_omap_phys *omap = get_zeroed_block(bno);

	if (!is_vol)
		omap->om_flags = cpu_to_le32(APFS_OMAP_MANUALLY_MANAGED);
	omap->om_tree_type = cpu_to_le32(APFS_OBJECT_TYPE_BTREE |
					 APFS_OBJ_PHYSICAL);
	omap->om_snapshot_tree_type = cpu_to_le32(APFS_OBJECT_TYPE_BTREE |
						  APFS_OBJ_PHYSICAL);
	if (is_vol) {
		omap->om_tree_oid = cpu_to_le64(FIRST_VOL_OMAP_ROOT_BNO);
		make_omap_root(FIRST_VOL_OMAP_ROOT_BNO, is_vol);
	} else {
		omap->om_tree_oid = cpu_to_le64(MAIN_OMAP_ROOT_BNO);
		make_omap_root(MAIN_OMAP_ROOT_BNO, is_vol);
	}

	set_object_header(&omap->om_o, param->blocksize, bno,
			  APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_OMAP,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(omap, param->blocksize);
}

/**
 * set_cat_info - Set the info footer for a catalog root node
 * @info:	pointer to the on-disk info footer
 */
static void set_cat_info(struct apfs_btree_info *info)
{
	int drec_keysz = param->norm_sensitive ? sizeof(struct apfs_drec_key) :
						 sizeof(struct apfs_drec_hashed_key);
	int maxkey = drec_keysz + strlen("private-dir") + 1;
	int maxval = sizeof(struct apfs_inode_val) +
		     sizeof(struct apfs_xf_blob) + sizeof(struct apfs_x_field) +
		     ROUND_UP(strlen("private-dir") + 1, 8);

	info->bt_fixed.bt_flags = cpu_to_le32(APFS_BTREE_KV_NONALIGNED);
	info->bt_fixed.bt_node_size = cpu_to_le32(param->blocksize);

	info->bt_longest_key = cpu_to_le32(maxkey);
	info->bt_longest_val = cpu_to_le32(maxval);
	info->bt_key_count = cpu_to_le64(4); /* Two records for each dir */
	info->bt_node_count = cpu_to_le64(1); /* Only one node: the root */
}

/**
 * make_cat_root - Make the root node of a catalog tree
 * @bno: block number to use
 * @oid: object id
 *
 * Creates a root node with two records: the root and private directories.
 */
void make_cat_root(u64 bno, u64 oid)
{
	struct apfs_btree_node_phys *root = get_zeroed_block(bno);
	struct apfs_kvloc *kvloc;
	void *key, *key_area, *val_end, *val_area_end;
	int toc_len, key_len, free_len, val_len;
	int head_len = sizeof(*root);
	int info_len = sizeof(struct apfs_btree_info);

	root->btn_flags = cpu_to_le16(APFS_BTNODE_ROOT | APFS_BTNODE_LEAF);

	/* The two dentry records and their inodes */
	root->btn_nkeys = cpu_to_le32(4);
	toc_len = min_table_size(APFS_OBJECT_TYPE_FSTREE);
	root->btn_table_space.off = 0;
	root->btn_table_space.len = cpu_to_le16(toc_len);

	kvloc = (void *)root + head_len;
	key = key_area = (void *)root + head_len + toc_len;
	val_end = val_area_end = (void *)root + param->blocksize - info_len;

	make_special_dir_dentry(APFS_PRIV_DIR_INO_NUM, "private-dir", &kvloc,
				key_area, &key, val_area_end, &val_end);
	make_special_dir_dentry(APFS_ROOT_DIR_INO_NUM, "root", &kvloc,
				key_area, &key, val_area_end, &val_end);
	make_special_dir_inode(APFS_ROOT_DIR_INO_NUM, "root", &kvloc,
			       key_area, &key, val_area_end, &val_end);
	make_special_dir_inode(APFS_PRIV_DIR_INO_NUM, "private-dir", &kvloc,
			       key_area, &key, val_area_end, &val_end);

	key_len = key - key_area;
	val_len = val_area_end - val_end;
	free_len = param->blocksize -
		   head_len - toc_len - key_len - val_len - info_len;
	root->btn_free_space.off = cpu_to_le16(key_len);
	root->btn_free_space.len = cpu_to_le16(free_len);

	/* No fragmentation */
	root->btn_key_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_key_free_list.len = 0;
	root->btn_val_free_list.off = cpu_to_le16(APFS_BTOFF_INVALID);
	root->btn_val_free_list.len = 0;

	set_cat_info((void *)root + param->blocksize - info_len);
	set_object_header(&root->btn_o, param->blocksize, oid,
			  APFS_OBJECT_TYPE_BTREE | APFS_OBJ_VIRTUAL,
			  APFS_OBJECT_TYPE_FSTREE);
	munmap(root, param->blocksize);
}
