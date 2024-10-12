/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <apfs/raw.h>
#include <apfs/sha256.h>
#include <apfs/types.h>
#include "apfsck.h"
#include "btree.h"
#include "dir.h"
#include "extents.h"
#include "htable.h"
#include "inode.h"
#include "key.h"
#include "object.h"
#include "snapshot.h"
#include "spaceman.h"
#include "super.h"
#include "xattr.h"

bool ongoing_query;

/**
 * node_min_table_size - Return the minimum size for a node's table of contents
 * @node: the node
 */
static int node_min_table_size(struct node *node)
{
	u32 type = node->object.subtype;
	bool leaf = (node->flags) & APFS_BTNODE_LEAF;
	int key_size, val_size, toc_size;
	int space, count;

	/* Trees with fixed key/value sizes preallocate the whole table */
	switch (type) {
	case APFS_OBJECT_TYPE_OMAP:
		key_size = sizeof(struct apfs_omap_key);
		val_size = leaf ? sizeof(struct apfs_omap_val) : sizeof(__le64);
		toc_size = sizeof(struct apfs_kvoff);
		break;
	case APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE:
		key_size = sizeof(struct apfs_spaceman_free_queue_key);
		val_size = sizeof(__le64); /* We assume no ghosts here */
		toc_size = sizeof(struct apfs_kvoff);
		break;
	case APFS_OBJECT_TYPE_OMAP_SNAPSHOT:
		key_size = sizeof(__le64);
		val_size = leaf ? sizeof(struct apfs_omap_snapshot) : sizeof(__le64);
		toc_size = sizeof(struct apfs_kvoff);
		break;
	case APFS_OBJECT_TYPE_FEXT_TREE:
		key_size = sizeof(struct apfs_fext_tree_key);
		val_size = leaf ? sizeof(struct apfs_fext_tree_val) : sizeof(__le64);
		toc_size = sizeof(struct apfs_kvoff);
		break;
	default:
		/* It should at least have room for one record */
		return sizeof(struct apfs_kvloc);
	}

	/* The footer of root nodes is ignored for some reason */
	space = sb->s_blocksize - sizeof(struct apfs_btree_node_phys);
	count = space / (key_size + val_size + toc_size);
	return count * toc_size;
}

/**
 * node_is_valid - Check basic sanity of the node index
 * @node:	node to check
 */
static bool node_is_valid(struct node *node)
{
	u16 flags = node->flags;
	int records = node->records;
	int index_size = node->key - node->toc;
	int entry_size, min_index_size;

	if ((flags & APFS_BTNODE_MASK) != flags)
		return false;

	if (!node_is_root(node) && !records)
		return false; /* Empty children should just be deleted */

	if (node->toc != sizeof(struct apfs_btree_node_phys))
		return false; /* The table of contents follows the header */

	if (node->data > sb->s_blocksize -
		(node_is_root(node) ? sizeof(struct apfs_btree_info) : 0))
		return false; /* The value area must start before it ends... */

	entry_size = (node_has_fixed_kv_size(node)) ?
		sizeof(struct apfs_kvoff) : sizeof(struct apfs_kvloc);

	min_index_size = node_min_table_size(node);
	if (index_size < min_index_size)
		return false;
	if (node_has_fixed_kv_size(node) && index_size != min_index_size) {
		/*
		 * Free queue nodes have ghost records, which makes no sense if
		 * their index is not allowed to grow bigger.
		 */
		if (node->object.subtype != APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE)
			return false;
	}

	/* All records must have an entry in the table of contents */
	return records * entry_size <= index_size;
}

/**
 * node_parse_key_free_list - Parse a node's free key list into a bitmap
 * @node: the node to parse
 *
 * The bitmap will be set in @node->free_key_bmap.
 */
static void node_parse_key_free_list(struct node *node)
{
	struct apfs_nloc *free = &node->raw->btn_key_free_list;
	void *area_raw = (void *)node->raw + node->key;
	int area_len = node->free - node->key;
	int total = le16_to_cpu(free->len);
	int off;

	/* Each bit represents a byte in the key area */
	node->free_key_bmap = malloc((area_len + 7) / 8);
	if (!node->free_key_bmap)
		system_error();
	memset(node->free_key_bmap, 0xFF, (area_len + 7) / 8);

	off = le16_to_cpu(free->off);
	while (total > 0) {
		int len, i;

		/* Tiny free areas may not be in the list */
		if (off == APFS_BTOFF_INVALID)
			break;

		if (off + sizeof(*free) > area_len)
			report("B-tree node",
			       "no room for free list entry in key area.");

		free = area_raw + off;
		len = le16_to_cpu(free->len);
		if (len < sizeof(*free))
			report("B-tree node", "free key is too small.");

		if (off + len > area_len)
			report("B-tree node", "free key is out-of-bounds.");

		for (i = off; i < off + len; ++i) {
			u8 *byte = node->free_key_bmap + i / 8;
			u8 flag = 1 << i % 8;

			if (!(*byte & flag))
				report("B-tree node",
				       "byte listed twice in free key list.");
			*byte ^= flag;
		}
		total -= len;

		off = le16_to_cpu(free->off);
	}

	if (off != APFS_BTOFF_INVALID)
		report("B-tree node", "bad last key in free list.");
}

/**
 * node_parse_val_free_list - Parse a node's free value list into a bitmap
 * @node: the node to parse
 *
 * The bitmap will be set in @node->free_val_bmap.
 */
static void node_parse_val_free_list(struct node *node)
{
	struct apfs_nloc *free = &node->raw->btn_val_free_list;
	void *end_raw = node->raw;
	int area_len;
	int total = le16_to_cpu(free->len);
	int off;

	/* Only the root has a footer */
	area_len = sb->s_blocksize - node->data -
		   (node_is_root(node) ? sizeof(struct apfs_btree_info) : 0);
	end_raw = (void *)node->raw + node->data + area_len;

	/* Each bit represents a byte in the value area */
	node->free_val_bmap = malloc((area_len + 7) / 8);
	if (!node->free_val_bmap)
		system_error();
	memset(node->free_val_bmap, 0xFF, (area_len + 7) / 8);

	off = le16_to_cpu(free->off);
	while (total > 0) {
		int len, i;

		/* Tiny free areas may not be in the list */
		if (off == APFS_BTOFF_INVALID)
			break;

		if (off < sizeof(*free))
			report("B-tree node",
			       "no room for free list entry in value area.");

		free = end_raw - off;
		len = le16_to_cpu(free->len);
		if (len < sizeof(*free))
			report("B-tree node", "free value is too small.");

		if (area_len < off || len > off)
			report("B-tree node", "free value is out-of-bounds.");

		for (i = area_len - off; i < area_len - off + len; ++i) {
			u8 *byte = node->free_val_bmap + i / 8;
			u8 flag = 1 << i % 8;

			if (!(*byte & flag))
				report("B-tree node",
				       "byte listed twice in free value list.");
			*byte ^= flag;
		}
		total -= len;

		off = le16_to_cpu(free->off);
	}

	if (off != APFS_BTOFF_INVALID)
		report("B-tree node", "bad last value in free list.");
}

/**
 * node_prepare_bitmaps - Do the basic setup of the nodes allocation bitmaps
 * @node: the node to parse
 *
 * The @node->free_key_bmap and @free_val_bmap bitmaps will be set entirely
 * from the information in the linked lists.  The @node->used_key_bmap and
 * @node->used_val_bmap will only be allocated, to be set later when parsing
 * the keys.
 *
 * TODO: should we check that the free space in the node is zeroed?
 */
static void node_prepare_bitmaps(struct node *node)
{
	int keys_len;
	int values_len;

	assert(node->raw);
	assert(!node->free_key_bmap);
	assert(!node->free_val_bmap);

	keys_len = node->free - node->key;

	/* Each bit represents a byte in the key area */
	node->used_key_bmap = calloc(1, (keys_len + 7) / 8);
	if (!node->used_key_bmap)
		system_error();

	/* Only the root has a footer */
	values_len = sb->s_blocksize - node->data -
		     (node_is_root(node) ? sizeof(struct apfs_btree_info) : 0);

	/* Each bit represents a byte in the value area */
	node->used_val_bmap = calloc(1, (values_len + 7) / 8);
	if (!node->used_val_bmap)
		system_error();

	node_parse_key_free_list(node);
	node_parse_val_free_list(node);
}

/**
 * verify_block_hash - Verify that a block has the given hash
 * @raw:	pointer to the block contents
 * @hash:	pointer to the SHA-256 hash
 */
static void verify_block_hash(u8 *raw, u8 *hash)
{
	SHA256_CTX ctx = {0};
	u8 true_hash[APFS_HASH_CCSHA256_SIZE] = {0};

	if (!hash)
		return;

	sha256_init(&ctx);
	sha256_update(&ctx, raw, sb->s_blocksize);
	sha256_final(&ctx, true_hash);

	if (memcmp(hash, true_hash, APFS_HASH_CCSHA256_SIZE) != 0)
		report("Sealed volume", "incorrect hash for node.");
}

/**
 * read_node - Read a node header from disk
 * @oid:	object id for the node
 * @btree:	tree structure, with the omap_table already set
 * @hash:	SHA-256 hash to check against (NULL if none)
 *
 * Returns a pointer to the resulting node structure.
 */
static struct node *read_node(u64 oid, struct btree *btree, u8 *hash)
{
	struct apfs_btree_node_phys *raw;
	struct node *node;
	u32 obj_type, obj_subtype;
	bool noheader;

	node = calloc(1, sizeof(*node));
	if (!node)
		system_error();
	node->btree = btree;

	noheader = btree_is_catalog(btree) && apfs_volume_is_sealed();

	/* The free-space queue is the only tree with ephemeral nodes so far */
	if (btree_is_free_queue(btree))
		raw = read_ephemeral_object(oid, &node->object);
	else if (noheader)
		raw = read_object_noheader(oid, btree->omap_table, &node->object);
	else
		raw = read_object(oid, btree->omap_table, &node->object);
	node->raw = raw;

	if (hash)
		verify_block_hash((u8 *)raw, hash);

	node->level = le16_to_cpu(raw->btn_level);
	node->flags = le16_to_cpu(raw->btn_flags);
	node->records = le32_to_cpu(raw->btn_nkeys);
	node->toc = sizeof(*raw) + le16_to_cpu(raw->btn_table_space.off);
	node->key = node->toc + le16_to_cpu(raw->btn_table_space.len);
	node->free = node->key + le16_to_cpu(raw->btn_free_space.off);
	node->data = node->free + le16_to_cpu(raw->btn_free_space.len);

	if (!node_is_valid(node)) {
		report("B-tree node", "block 0x%llx is not sane.",
		       (unsigned long long)node->object.block_nr);
	}

	if ((bool)(node->flags & APFS_BTNODE_NOHEADER) != noheader)
		report("B-tree node", "wrong setting of hashed flag.");
	if ((bool)(node->flags & APFS_BTNODE_HASHED) != noheader)
		report("B-tree node", "wrong setting of hashed flag.");

	obj_type = node->object.type;
	if (!noheader && node_is_root(node) && obj_type != APFS_OBJECT_TYPE_BTREE)
		report("B-tree node", "wrong object type for root.");
	if (!noheader && !node_is_root(node) && obj_type != APFS_OBJECT_TYPE_BTREE_NODE)
		report("B-tree node", "wrong object type for nonroot.");

	obj_subtype = node->object.subtype;
	if (btree_is_omap(btree) && obj_subtype != APFS_OBJECT_TYPE_OMAP)
		report("Object map node", "wrong object subtype.");
	if (!noheader && btree_is_catalog(btree) && obj_subtype != APFS_OBJECT_TYPE_FSTREE)
		report("Catalog node", "wrong object subtype.");
	if (btree_is_extentref(btree) && obj_subtype !=
						APFS_OBJECT_TYPE_BLOCKREFTREE)
		report("Extent reference tree node", "wrong object subtype.");
	if (btree_is_snap_meta(btree) && obj_subtype !=
						APFS_OBJECT_TYPE_SNAPMETATREE)
		report("Snapshot metadata node", "wrong object subtype.");
	if (btree_is_free_queue(btree) && obj_subtype !=
					 APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE)
		report("Free queue node", "wrong object subtype.");
	if (btree_is_snapshots(btree) && obj_subtype != APFS_OBJECT_TYPE_OMAP_SNAPSHOT)
		report("Omap snapshot tree node", "wrong object subtype.");
	if (btree_is_fext(btree) && obj_subtype != APFS_OBJECT_TYPE_FEXT_TREE)
		report("File extents tree node", "wrong object subtype.");

	node_prepare_bitmaps(node);

	return node;
}

/**
 * node_free - Free a node structure
 * @node: node to free
 *
 * This function works under the assumption that the node flags are not
 * corrupted, but we are not yet checking that (TODO).
 */
static void node_free(struct node *node)
{
	if (node_is_root(node))
		return;	/* The root nodes are needed by the sb until the end */
	munmap(node->raw, node->object.size);
	free(node->free_key_bmap);
	free(node->free_val_bmap);
	free(node->used_key_bmap);
	free(node->used_val_bmap);
	free(node);
}

/**
 * node_locate_key - Locate the key of a node record
 * @node:	node to be searched
 * @index:	number of the entry to locate
 * @off:	on return will hold the offset in the block
 *
 * Returns the length of the key. The function checks that this length fits
 * within the key area; callers must use the returned value to make sure they
 * never operate outside its bounds.
 */
static int node_locate_key(struct node *node, int index, int *off)
{
	struct apfs_btree_node_phys *raw;
	int len, off_in_area;

	if (index >= node->records)
		report("B-tree node", "requested index out-of-bounds.");

	raw = node->raw;
	if (node_has_fixed_kv_size(node)) {
		struct apfs_kvoff *entry;

		entry = (struct apfs_kvoff *)raw->btn_data + index;
		len = btree_is_snapshots(node->btree) ? 8 : 16;
		off_in_area = le16_to_cpu(entry->k);
	} else {
		/* These node types have variable length keys and data */
		struct apfs_kvloc *entry;

		entry = (struct apfs_kvloc *)raw->btn_data + index;
		len = le16_to_cpu(entry->k.len);
		off_in_area = le16_to_cpu(entry->k.off);
	}

	/* Translate offset in key area to offset in block */
	*off = node->key + off_in_area;
	if (*off + len > node->free)
		report("B-tree", "key is out-of-bounds.");

	return len;
}

/**
 * node_locate_data - Locate the data of a node record
 * @node:	node to be searched
 * @index:	number of the entry to locate
 * @off:	on return will hold the offset in the block
 *
 * Returns the length of the data. The function checks that this length fits
 * within the value area; callers must use the returned value to make sure they
 * never operate outside its bounds.
 */
static int node_locate_data(struct node *node, int index, int *off)
{
	struct apfs_btree_node_phys *raw;
	int len, off_in_area, area_len;

	if (index >= node->records)
		report("B-tree", "requested index out-of-bounds.");

	/* Only the root has a footer */
	area_len = sb->s_blocksize - node->data -
		   (node_is_root(node) ? sizeof(struct apfs_btree_info) : 0);

	raw = node->raw;
	if (node_has_fixed_kv_size(node)) {
		/* These node types have fixed length keys and data */
		struct apfs_kvoff *entry;
		struct btree *btree = node->btree;

		entry = (struct apfs_kvoff *)raw->btn_data + index;

		if (btree_is_free_queue(btree)) {
			/* A free-space queue record may have no value */
			if (le16_to_cpu(entry->v) == APFS_BTOFF_INVALID)
				return 0;
			len = 8;
		}
		if (btree_is_omap(btree))
			len = node_is_leaf(node) ? 16 : 8;
		if (btree_is_snapshots(btree))
			len = node_is_leaf(node) ? sizeof(struct apfs_omap_snapshot) : 8;
		if (btree_is_fext(btree))
			len = node_is_leaf(node) ? sizeof(struct apfs_fext_tree_val) : 8;

		/* Value offsets are backwards from the end of the value area */
		off_in_area = area_len - le16_to_cpu(entry->v);
	} else {
		/* These node types have variable length keys and data */
		struct apfs_kvloc *entry;

		entry = (struct apfs_kvloc *)raw->btn_data + index;
		len = le16_to_cpu(entry->v.len);

		/* Value offsets are backwards from the end of the value area */
		off_in_area = area_len - le16_to_cpu(entry->v.off);
	}

	*off = node->data + off_in_area;
	if (*off < node->data || off_in_area >= area_len)
		report("B-tree", "value is out-of-bounds.");

	return len;
}

/**
 * bmap_mark_as_used - Mark a region of a node as used in the allocation bitmap
 * @bitmap:	bitmap to update
 * @off:	offset of the region, relative to its area in the node
 * @len:	length of the region in the node
 */
static void bmap_mark_as_used(u8 *bitmap, int off, int len)
{
	u8 *byte;
	u8 flag;
	int i;

	for (i = off; i < off + len; ++i) {
		byte = bitmap + i / 8;
		flag = 1 << i % 8;

		if (*byte & flag)
			report("B-tree node", "overlapping record data.");
		*byte |= flag;
	}
}

/**
 * compare_bmaps - Compare record allocation bitmaps for free and used space
 * @free_bmap:	bitmap compiled from the free space linked list
 * @used_bmap:	bitmap compiled from the table of contents
 * @area_len:	length of the area to check in the node
 *
 * Verifies that @free_bmap and @used_bmap are consistent, and returns the
 * total number of free bytes (including those not counted in @free_bmap due
 * to fragmentation).
 */
static int compare_bmaps(u8 *free_bmap, u8 *used_bmap, int area_len)
{
	int unused = 0;
	int i, j;

	for (i = 0; i < area_len / 8; ++i) {
		for (j = 0; j < 8; ++j) {
			u8 mask = 1 << j;

			if (!(used_bmap[i] & mask))
				++unused;
		}

		if ((free_bmap[i] | used_bmap[i]) != free_bmap[i])
			report("B-tree node",
			       "used record space listed as free.");
	}

	/* Last byte has some undefined bits by the end, so be careful */
	for (j = 0; j < area_len % 8; ++j) {
		u8 mask = 1 << j;

		if (!(used_bmap[area_len / 8] & mask))
			++unused;
		if (used_bmap[area_len / 8] & mask &&
		    !(free_bmap[area_len / 8] & mask))
			report("B-tree node",
			       "used record space listed as free.");
	}

	return unused;
}

/**
 * node_compare_bmaps - Check consistency of the allocation bitmaps for a node
 * @node: node to check
 *
 * Verifies that the bitmaps built from the free lists match the actual unused
 * space bitmaps, built from the table of contents.  This function shouldn't be
 * called, of course, until all the bitmaps are assembled.
 */
static void node_compare_bmaps(struct node *node)
{
	struct apfs_nloc *free_head;
	int area_len;
	int free_count;

	/*
	 * First check the key area.
	 */
	free_head = &node->raw->btn_key_free_list;

	area_len = node->free - node->key;
	free_count = compare_bmaps(node->free_key_bmap, node->used_key_bmap,
				   area_len);

	if (free_count != le16_to_cpu(free_head->len))
		report("B-tree", "wrong free space total for key area.");

	/*
	 * Now check the value area.
	 */
	free_head = &node->raw->btn_val_free_list;

	/* Only the root has a footer */
	area_len = sb->s_blocksize - node->data -
		   (node_is_root(node) ? sizeof(struct apfs_btree_info) : 0);
	free_count = compare_bmaps(node->free_val_bmap, node->used_val_bmap,
				   area_len);

	if (free_count != le16_to_cpu(free_head->len))
		report("B-tree", "wrong free space total for value area.");
}

/**
 * parse_cat_record - Parse a catalog record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_cat_record(void *key, void *val, int len)
{
	switch (cat_type(key)) {
	case APFS_TYPE_INODE:
		parse_inode_record(key, val, len);
		break;
	case APFS_TYPE_DIR_REC:
		parse_dentry_record(key, val, len);
		break;
	case APFS_TYPE_FILE_EXTENT:
		parse_extent_record(key, val, len);
		break;
	case APFS_TYPE_SIBLING_LINK:
		parse_sibling_record(key, val, len);
		break;
	case APFS_TYPE_XATTR:
		parse_xattr_record(key, val, len);
		break;
	case APFS_TYPE_SIBLING_MAP:
		parse_sibling_map_record(key, val, len);
		break;
	case APFS_TYPE_DSTREAM_ID:
		parse_dstream_id_record(key, val, len);
		break;
	case APFS_TYPE_DIR_STATS:
		parse_dir_stats_record(key, val, len);
		break;
	case APFS_TYPE_CRYPTO_STATE:
		parse_crypto_state_record(key, val, len);
		break;
	case APFS_TYPE_FILE_INFO:
		parse_file_info_record(key, val, len);
		break;
	default:
		report(NULL, "Bug!");
	}
}

/**
 * free_omap_record - Free an object map record list after a final check
 * @entry: the entry to free
 */
static void free_omap_record_list(struct htable_entry *entry)
{
	struct omap_record_list *list = (struct omap_record_list *)entry;
	struct omap_record *curr_rec = list->o_records;

	while (curr_rec) {
		struct omap_record *next_rec = NULL;

		if (curr_rec->flags & APFS_OMAP_VAL_DELETED) {
			if (curr_rec->seen)
				report("Omap record", "deleted but still in use.");
		} else if (!curr_rec->seen) {
			/*
			 * Old implementations that are unaware of extended
			 * snapshot metadata are allowed to leak those blocks
			 * when a snapshot gets deleted.
			 */
			if (vsb && curr_rec->xid < sb->s_xid) {
				struct apfs_obj_phys *raw = NULL;
				struct object obj = {0};

				raw = read_object_nocheck(curr_rec->bno, sb->s_blocksize, &obj);
				if (obj.type != APFS_OBJECT_TYPE_SNAP_META_EXT || obj.subtype != APFS_OBJECT_TYPE_INVALID)
					report("Leaked omap record", "unexpected object type.");
				container_bmap_mark_as_used(curr_rec->bno, 1);
				++vsb->v_block_count;
				munmap(raw, obj.size);
			} else {
				report("Omap record", "oid-xid combination is never used.");
			}
		}

		next_rec = curr_rec->next;
		free(curr_rec);
		curr_rec = next_rec;
	}

	free(entry);
}

/**
 * free_omap_table - Free a hash table for omap records, and all its entries
 * @table: table to free
 */
void free_omap_table(struct htable_entry **table)
{
	free_htable(table, free_omap_record_list);
}

/**
 * omap_list_clear_seen_for_snap - Clear seen_for_snap for all omap recs in list
 * @entry: a list of omap records
 */
static void omap_list_clear_seen_for_snap(struct htable_entry *entry)
{
	struct omap_record_list *list = (struct omap_record_list *)entry;
	struct omap_record *rec = NULL;

	for (rec = list->o_records; rec; rec = rec->next)
		rec->seen_for_snap = false;
}

/**
 * omap_htable_clear_seen_for_snap - Clear seen_for_snap on all omap recs
 * @table: the hash table of omap records to be cleared
 */
void omap_htable_clear_seen_for_snap(struct htable_entry **table)
{
	return apply_on_htable(table, omap_list_clear_seen_for_snap);
}

/**
 * get_omap_record - Find or create an omap record structure in a hash table
 * @oid:	object id to be mapped
 * @xid:	transaction id
 * @table:	the hash table of omap records to be searched
 *
 * Returns the omap record structure, after creating it if necessary.
 */
static struct omap_record *get_omap_record(u64 oid, u64 xid, struct htable_entry **table)
{
	struct omap_record_list *list = NULL;
	struct omap_record **omap_p = NULL;
	struct omap_record *omap = NULL;
	struct omap_record *new = NULL;

	list = (struct omap_record_list *)get_htable_entry(oid, sizeof(struct omap_record), table);

	omap_p = &list->o_records;
	omap = *omap_p;
	while (omap) {
		if (xid == omap->xid)
			return omap;
		if (xid < omap->xid)
			break;
		omap_p = &omap->next;
		omap = *omap_p;
	}
	new = calloc(1, sizeof(*new));
	if (!new)
		system_error();
	new->xid = xid;
	new->oid = oid;
	new->next = omap;
	*omap_p = new;
	return new;
}

/**
 * get_latest_omap_record - Find the most recent omap record before a given xid
 * @oid:	object id to be mapped
 * @xid:	transaction id
 * @table:	the hash table of omap records to be searched
 *
 * Returns the omap record structure, or NULL if there is none matches.
 */
struct omap_record *get_latest_omap_record(u64 oid, u64 xid, struct htable_entry **table)
{
	struct omap_record_list *list = NULL;
	struct omap_record *omap = NULL, *prev_omap = NULL;

	list = (struct omap_record_list *)get_htable_entry(oid, sizeof(struct omap_record), table);
	omap = list->o_records;
	while (omap) {
		if (xid < omap->xid)
			break;
		prev_omap = omap;
		omap = prev_omap->next;
	}
	return prev_omap;
}

/**
 * parse_omap_record - Parse an object map record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_omap_record(struct apfs_omap_key *key,
			      struct apfs_omap_val *val, int len)
{
	struct omap_record *omap_rec;
	u32 flags;
	u32 size;

	/* In the omap, keys and values must be aligned to eight bytes */
	if ((u64)key & 7 || (u64)val & 7)
		report("Omap record", "bad alignment for key or value.");

	if (len != sizeof(*val))
		report("Omap record", "wrong size of value.");

	if (vsb) {
		/* We are parsing a volume's object map */
		omap_rec = get_omap_record(le64_to_cpu(key->ok_oid), le64_to_cpu(key->ok_xid), vsb->v_omap_table);
	} else {
		/* We are parsing the container's object map */
		omap_rec = get_omap_record(le64_to_cpu(key->ok_oid), le64_to_cpu(key->ok_xid), sb->s_omap_table);
	}

	if (omap_rec->bno)
		report("Object map", "two entries with the same oid-xid.");
	omap_rec->bno = le64_to_cpu(val->ov_paddr);

	omap_rec->flags = flags = le32_to_cpu(val->ov_flags);
	if ((flags & APFS_OMAP_VAL_FLAGS_VALID_MASK) != flags)
		report("Omap record", "invalid flag in use.");
	if (flags & APFS_OMAP_VAL_SAVED)
		report("Omap record", "saved flag is set.");
	if ((bool)(flags & APFS_OMAP_VAL_ENCRYPTED) != (vsb && vsb->v_encrypted))
		report("Omap record", "wrong encryption flag.");
	if (flags & APFS_OMAP_VAL_CRYPTO_GENERATION)
		report_unknown("Crypto generation flag");

	size = le32_to_cpu(val->ov_size);
	if (size & (sb->s_blocksize - 1))
		report("Omap record", "size isn't multiple of block size.");
	if (size != sb->s_blocksize)
		report_unknown("Objects with more than one block");
}

/**
 * parse_subtree - Parse a subtree and check for corruption
 * @root:	root node of the subtree
 * @last_key:	parent key, that must come before all the keys in this subtree;
 *		on return, this will hold the last key of this subtree, that
 *		must come before the next key of the parent node
 * @name_buf:	buffer to store the name of @last_key when its node is freed
 *		(can be NULL if the keys have no name)
 */
static void parse_subtree(struct node *root,
			  struct key *last_key, char *name_buf)
{
	struct btree *btree = root->btree;
	struct key curr_key;
	bool is_sealed_cat;
	int i;

	if (node_is_leaf(root)) {
		if (root->level != 0)
			report("B-tree", "nonleaf node flagged as leaf.");
		btree->key_count += root->records;
	}
	++btree->node_count;

	if (btree_is_omap(btree) && !node_has_fixed_kv_size(root))
		report("Object map", "key size should be fixed.");
	if (btree_is_catalog(btree) && node_has_fixed_kv_size(root))
		report("Catalog", "key size should not be fixed.");
	if (btree_is_free_queue(btree) && !node_has_fixed_kv_size(root))
		report("Free-space queue", "key size should be fixed.");
	if (btree_is_snap_meta(btree) && node_has_fixed_kv_size(root))
		report("Snap meta tree", "key size shouldn't be fixed.");
	if (btree_is_snapshots(btree) && !node_has_fixed_kv_size(root))
		report("Omap snapshot tree", "key size should be fixed.");
	if (btree_is_fext(btree) && !node_has_fixed_kv_size(root))
		report("File extents tree", "key size should be fixed.");

	/* This makes little sense, but it appears to be true */
	if (btree_is_extentref(btree) && node_has_fixed_kv_size(root))
		report("Extent reference tree", "key size shouldn't be fixed.");

	is_sealed_cat = btree_is_catalog(btree) && apfs_volume_is_sealed();

	for (i = 0; i < root->records; ++i) {
		struct node *child;
		void *raw = root->raw;
		void *raw_key, *raw_val;
		int off, len;
		u64 child_id;
		u8 *child_hash = NULL;

		len = node_locate_key(root, i, &off);
		if (len > btree->longest_key)
			btree->longest_key = len;
		bmap_mark_as_used(root->used_key_bmap, off - root->key, len);
		raw_key = raw + off;

		if (btree_is_omap(btree)) {
			read_omap_key(raw_key, len, &curr_key);

			/* When a key is added, the node is updated */
			if (curr_key.number > root->object.xid)
				report("Object map",
				       "node xid is older than key xid.");
		}
		if (btree_is_catalog(btree))
			read_cat_key(raw_key, len, &curr_key);
		if (btree_is_extentref(btree))
			read_extentref_key(raw_key, len, &curr_key);
		if (btree_is_free_queue(btree))
			read_free_queue_key(raw_key, len, &curr_key);
		if (btree_is_snap_meta(btree))
			read_snap_key(raw_key, len, &curr_key);
		if (btree_is_snapshots(btree))
			read_omap_snap_key(raw_key, len, &curr_key);
		if (btree_is_fext(btree))
			read_fext_key(raw_key, len, &curr_key);

		if (keycmp(last_key, &curr_key) > 0)
			report("B-tree", "keys are out of order.");
		if (!i && !node_is_root(root) && keycmp(last_key, &curr_key))
			report("B-tree", "index key absent from child node.");

		if (i != 0 && node_is_leaf(root) &&
		    !keycmp(last_key, &curr_key))
			report("B-tree", "leaf keys are repeated.");
		*last_key = curr_key;

		len = node_locate_data(root, i, &off);
		bmap_mark_as_used(root->used_val_bmap, off - root->data, len);
		raw_val = raw + off;

		if (node_is_leaf(root)) {
			if (len > btree->longest_val)
				btree->longest_val = len;
			if (btree_is_catalog(btree))
				parse_cat_record(raw_key, raw_val, len);
			if (btree_is_omap(btree))
				parse_omap_record(raw_key, raw_val, len);
			if (btree_is_free_queue(btree))
				parse_free_queue_record(raw_key, raw_val, len,
							btree);
			if (btree_is_extentref(btree))
				/* Physical extents must not overlap */
				last_key->id = parse_phys_ext_record(raw_key,
								raw_val, len);
			if (btree_is_snap_meta(btree))
				parse_snap_record(raw_key, raw_val, len);
			if (btree_is_snapshots(btree))
				parse_omap_snap_record(raw_key, raw_val, len);
			if (btree_is_fext(btree))
				parse_fext_record(raw_key, raw_val, len);
			continue;
		}

		if (is_sealed_cat) {
			struct apfs_btn_index_node_val *index_val = NULL;
			u64 base_oid = btree->root->object.oid;

			if (len != sizeof(*index_val))
				report("B-tree", "wrong size of hashed nonleaf record value.");
			index_val = (struct apfs_btn_index_node_val *)raw_val;
			/*
			 * The reference is wrong here, binv_child_oid is not
			 * the id for the child node, it's an offset from the
			 * id of the root node. As usual, I have no idea what
			 * this is about.
			 */
			child_id = le64_to_cpu(index_val->binv_child_oid) + base_oid;
			child_hash = index_val->binv_child_hash;
		} else {
			if (len != 8)
				report("B-tree", "wrong size of nonleaf record value.");
			child_id = le64_to_cpu(*(__le64 *)(raw_val));
		}
		child = read_node(child_id, btree, child_hash);

		if (child->level != root->level - 1)
			report("B-tree", "node levels are corrupted.");
		if (node_is_root(child))
			report("B-tree", "nonroot node is flagged as root.");

		/* If a physical node changes, the parent must update the bno */
		if ((btree_is_omap(btree) || btree_is_extentref(btree) || btree_is_snap_meta(btree) || btree_is_snapshots(btree) || btree_is_fext(btree)) && root->object.xid < child->object.xid)
			report("Physical tree",
			       "xid of node is older than xid of its child.");

		parse_subtree(child, last_key, name_buf);
		node_free(child);
	}

	/* All records of @root are processed, so it's a good time for this */
	node_compare_bmaps(root);

	/*
	 * last_key->name is just a pointer to the memory-mapped on-disk name
	 * of the key.  Since the caller will free the node, make a copy.
	 */
	if (node_is_leaf(root) && last_key->name) {
		assert(name_buf);
		strcpy(name_buf, last_key->name);
		last_key->name = name_buf;
	}
}

/**
 * check_btree_footer_flags - Check consistency of b-tree footer flags
 * @flags:	the flags
 * @btree:	the b-tree being checked
 * @ctx:	context string for corruption reports
 */
static void check_btree_footer_flags(u32 flags, struct btree *btree, char *ctx)
{
	bool aligned, is_free_queue, is_physical, is_sealed_cat;

	if ((flags & APFS_BTREE_FLAGS_VALID_MASK) != flags)
		report(ctx, "invalid flag in use.");
	if (flags & (APFS_BTREE_NONPERSISTENT))
		report(ctx, "nonpersistent flag is set.");

	/* TODO: are these really the only allowed settings for the flag? */
	aligned = btree_is_omap(btree) || btree_is_free_queue(btree) || btree_is_snapshots(btree) || btree_is_fext(btree);
	if (aligned != !(flags & APFS_BTREE_KV_NONALIGNED))
		report(ctx, "wrong alignment flag.");

	is_free_queue = btree_is_free_queue(btree);
	if (is_free_queue != (bool)(flags & (APFS_BTREE_ALLOW_GHOSTS)))
		report(ctx, "wrong setting of ghosts flag.");
	if (is_free_queue != (bool)(flags & (APFS_BTREE_EPHEMERAL)))
		report(ctx, "wrong setting of ephemeral flag.");

	is_physical = !(btree_is_catalog(btree) || btree_is_free_queue(btree));
	if (is_physical != (bool)(flags & APFS_BTREE_PHYSICAL))
		report(ctx, "wrong setting of physical flag.");

	is_sealed_cat = btree_is_catalog(btree) && apfs_volume_is_sealed();
	if (is_sealed_cat != (bool)(flags & APFS_BTREE_HASHED))
		report(ctx, "wrong setting of hashed flag.");
	if (is_sealed_cat != (bool)(flags & APFS_BTREE_NOHEADER))
		report(ctx, "wrong setting of no-header flag.");
}

/**
 * check_btree_footer - Check that btree_info matches the collected stats
 * @btree: b-tree to check
 */
static void check_btree_footer(struct btree *btree)
{
	struct node *root = btree->root;
	struct apfs_btree_info *info;
	char *ctx;

	switch (btree->type) {
	case BTREE_TYPE_OMAP:
		ctx = "Object map";
		break;
	case BTREE_TYPE_CATALOG:
		ctx = "Catalog";
		break;
	case BTREE_TYPE_EXTENTREF:
		ctx = "Extent reference tree";
		break;
	case BTREE_TYPE_SNAP_META:
		ctx = "Snapshot metadata tree";
		break;
	case BTREE_TYPE_FREE_QUEUE:
		ctx = "Free-space queue";
		break;
	case BTREE_TYPE_SNAPSHOTS:
		ctx = "Omap snapshot tree";
		break;
	case BTREE_TYPE_FEXT:
		ctx = "File extents tree";
		break;
	default:
		report(NULL, "Bug!");
	}

	/* This flag is not part of the footer, but its check fits best here */
	if (!node_is_root(root))
		report(ctx, "wrong flag in root node.");

	info = (void *)root->raw + sb->s_blocksize - sizeof(*info);
	if (le32_to_cpu(info->bt_fixed.bt_node_size) != sb->s_blocksize)
		report_unknown("Objects with more than one block");

	check_btree_footer_flags(le32_to_cpu(info->bt_fixed.bt_flags),
				 btree, ctx);

	if (le64_to_cpu(info->bt_key_count) != btree->key_count)
		report(ctx, "wrong key count in info footer.");
	if (le64_to_cpu(info->bt_node_count) != btree->node_count)
		report(ctx, "wrong node count in info footer.");

	if (btree_is_omap(btree)) {
		u32 longest_key = le32_to_cpu(info->bt_longest_key);
		u32 longest_val = le32_to_cpu(info->bt_longest_val);

		if (le32_to_cpu(info->bt_fixed.bt_key_size) !=
					sizeof(struct apfs_omap_key))
			report(ctx, "wrong key size in info footer.");

		if (le32_to_cpu(info->bt_fixed.bt_val_size) !=
					sizeof(struct apfs_omap_val))
			report(ctx, "wrong value size in info footer.");

		/* Containers with no volumes do exist */
		if ((longest_key || btree->key_count) && longest_key != sizeof(struct apfs_omap_key))
			report(ctx, "wrong maximum key size in info footer.");
		if ((longest_val || btree->key_count) && longest_val != sizeof(struct apfs_omap_val))
			report(ctx, "wrong maximum value size in info footer.");
		return;
	}

	if (btree_is_free_queue(btree)) {
		if (le32_to_cpu(info->bt_fixed.bt_key_size) !=
				sizeof(struct apfs_spaceman_free_queue_key))
			report(ctx, "wrong key size in info footer.");

		/* Ghost records may also exist, but they don't count here */
		if (le32_to_cpu(info->bt_fixed.bt_val_size) != 8)
			report(ctx, "wrong value size in info footer.");

		if (le32_to_cpu(info->bt_longest_key) !=
				sizeof(struct apfs_spaceman_free_queue_key))
			report(ctx, "wrong maximum key size in info footer.");

		if (le32_to_cpu(info->bt_longest_val) < btree->longest_val)
			report(ctx, "wrong maximum value size in info footer.");

		return;
	}

	if (btree_is_snapshots(btree)) {
		if (le32_to_cpu(info->bt_fixed.bt_key_size) != 8)
			report(ctx, "wrong key size in info footer.");
		if (le32_to_cpu(info->bt_fixed.bt_val_size) != sizeof(struct apfs_omap_snapshot))
			report(ctx, "wrong value size in info footer.");
		if (le32_to_cpu(info->bt_longest_key) != 8)
			report(ctx, "wrong maximum key size in info footer.");
		if (le32_to_cpu(info->bt_longest_val) != sizeof(struct apfs_omap_snapshot))
			report(ctx, "wrong maximum value size in info footer.");
		return;
	}

	if (btree_is_fext(btree)) {
		if (le32_to_cpu(info->bt_fixed.bt_key_size) != sizeof(struct apfs_fext_tree_key))
			report(ctx, "wrong key size in info footer.");
		if (le32_to_cpu(info->bt_fixed.bt_val_size) != sizeof(struct apfs_fext_tree_val))
			report(ctx, "wrong value size in info footer.");
		if (le32_to_cpu(info->bt_longest_key) != sizeof(struct apfs_fext_tree_key))
			report(ctx, "wrong maximum key size in info footer.");
		if (le32_to_cpu(info->bt_longest_val) != sizeof(struct apfs_fext_tree_val))
			report(ctx, "wrong maximum value size in info footer.");
		return;
	}

	/* The remaining trees don't report fixed key/value sizes */
	if (le32_to_cpu(info->bt_fixed.bt_key_size) != 0)
		report(ctx, "key size should not be set.");
	if (le32_to_cpu(info->bt_fixed.bt_val_size) != 0)
		report(ctx, "value size should not be set.");

	if (btree_is_catalog(btree) || btree_is_snap_meta(btree)) {
		if (le32_to_cpu(info->bt_longest_key) < btree->longest_key)
			report(ctx, "wrong maximum key size in info footer.");
		if (le32_to_cpu(info->bt_longest_val) < btree->longest_val)
			report(ctx, "wrong maximum value size in info footer.");
		return;
	}

	/*
	 * The extentref only seems to have records of this one type.
	 * No idea why it reports keys/values of variable size...
	 */
	if (btree_is_extentref(btree)) {
		u32 longest_key = le32_to_cpu(info->bt_longest_key);
		u32 longest_val = le32_to_cpu(info->bt_longest_val);

		if ((longest_key || btree->key_count) &&
		    longest_key != sizeof(struct apfs_phys_ext_key))
			report(ctx, "wrong maximum key size in info footer.");
		if ((longest_val || btree->key_count) &&
		    longest_val != sizeof(struct apfs_phys_ext_val))
			report(ctx, "wrong maximum val size in info footer.");
	}
}

/**
 * parse_free_queue_btree - Parse and check a free-space queue tree
 * @oid:	object id for the b-tree root
 * @index:	position of this free queue in the free queue array
 *
 * Returns a pointer to the free queue structure.
 */
struct free_queue *parse_free_queue_btree(u64 oid, int index)
{
	struct free_queue *sfq;
	struct btree *btree;
	struct key last_key = {0};

	sfq = calloc(1, sizeof(*sfq));
	if (!sfq)
		system_error();
	btree = &sfq->sfq_btree;
	sfq->sfq_index = index;

	if (oid == 0) {
		/* I've seen null fq's in fresh containers with no volumes */
		return sfq;
	}

	btree->type = BTREE_TYPE_FREE_QUEUE;
	btree->omap_table = NULL; /* These are ephemeral objects */
	btree->root = read_node(oid, btree, NULL /* hash */);
	parse_subtree(btree->root, &last_key, NULL /* name_buf */);

	check_btree_footer(btree);
	return sfq;
}

/**
 * parse_snap_meta_btree - Parse and check a snapshot metadata tree
 * @oid:	object id for the b-tree root
 *
 * Returns a pointer to the btree struct for the snapshot metadata tree.
 */
struct btree *parse_snap_meta_btree(u64 oid)
{
	struct btree *snap;
	struct key last_key = {0};
	char name_buf[256];

	snap = calloc(1, sizeof(*snap));
	if (!snap)
		system_error();
	snap->type = BTREE_TYPE_SNAP_META;
	snap->omap_table = NULL; /* These are physical objects */
	snap->root = read_node(oid, snap, NULL /* hash */);

	parse_subtree(snap->root, &last_key, name_buf);

	check_btree_footer(snap);
	return snap;
}

/**
 * parse_cat_btree - Parse a catalog tree and check for corruption
 * @oid:	object id for the catalog root
 * @omap_table:	hash table of object map records for the b-tree
 *
 * Returns a pointer to the btree struct for the catalog.
 */
struct btree *parse_cat_btree(u64 oid, struct htable_entry **omap_table)
{
	struct btree *cat;
	struct key last_key = {0};
	char name_buf[256];

	cat = calloc(1, sizeof(*cat));
	if (!cat)
		system_error();

	/*
	 * We need to set this here so that parse_xattr_record() can read
	 * dstream contents. Maybe all parse_*_btree() functions should do
	 * this, for consistency...
	 */
	vsb->v_cat = cat;

	cat->type = BTREE_TYPE_CATALOG;
	cat->omap_table = omap_table;
	cat->root = read_node(oid, cat, apfs_volume_is_sealed() ? vsb->v_hash : NULL);

	parse_subtree(cat->root, &last_key, name_buf);

	check_btree_footer(cat);
	return cat;
}

/**
 * parse_fext_btree - Parse a fext tree and check for corruption
 * @oid:	object id for the fext root
 *
 * Returns a pointer to the btree struct for the catalog.
 */
struct btree *parse_fext_btree(u64 oid)
{
	struct btree *fext = NULL;
	struct key last_key = {0};

	fext = calloc(1, sizeof(*fext));
	if (!fext)
		system_error();

	fext->type = BTREE_TYPE_FEXT;
	fext->omap_table = NULL;
	fext->root = read_node(oid, fext, NULL /* hash */);

	parse_subtree(fext->root, &last_key, NULL);

	check_btree_footer(fext);
	return fext;
}

/**
 * check_omap_flags - Check consistency of object map flags
 * @flags: the flags
 */
static void check_omap_flags(u32 flags)
{
	if ((flags & APFS_OMAP_FLAGS_VALID_MASK) != flags)
		report("Object map", "invalid flag in use.");

	if (flags & (APFS_OMAP_ENCRYPTING | APFS_OMAP_DECRYPTING |
		     APFS_OMAP_KEYROLLING | APFS_OMAP_CRYPTO_GENERATION))
		report_unknown("Omap encryption");

	if (vsb && (flags & APFS_OMAP_MANUALLY_MANAGED))
		report("Volume object map", "is manually managed.");
	if (!vsb && !(flags & APFS_OMAP_MANUALLY_MANAGED))
		report("Container object map", "isn't manually managed.");
}

static struct btree *parse_snapshot_tree(u64 oid)
{
	struct btree *snaps = NULL;
	struct key last_key = {0};

	snaps = calloc(1, sizeof(*snaps));
	if (!snaps)
		system_error();

	snaps->type = BTREE_TYPE_SNAPSHOTS;
	snaps->omap_table = NULL;
	snaps->root = read_node(oid, snaps, NULL /* hash */);

	parse_subtree(snaps->root, &last_key, NULL);

	check_btree_footer(snaps);
	return snaps;
}

/**
 * parse_omap_btree - Parse an object map and check for corruption
 * @oid:	object id for the omap
 *
 * Returns a pointer to the btree struct for the omap.
 */
struct btree *parse_omap_btree(u64 oid)
{
	struct apfs_omap_phys *raw;
	struct btree *omap;
	struct key last_key = {0};
	struct object obj;

	/* Many checks are missing, of course */
	raw = read_object(oid, NULL /* omap_table */, &obj);
	if (obj.type != APFS_OBJECT_TYPE_OMAP)
		report("Object map", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Object map", "wrong object subtype.");

	check_omap_flags(le32_to_cpu(raw->om_flags));

	if (raw->om_snapshot_tree_oid) {
		if (!vsb)
			report("Container omap", "has snapshot tree.");
		vsb->v_snapshots = parse_snapshot_tree(le64_to_cpu(raw->om_snapshot_tree_oid));
		if (vsb->v_snapshots->key_count != le32_to_cpu(raw->om_snap_count))
			report("Omap snapshot tree", "snap count doesn't match keys.");
		if (vsb->v_snap_max_xid != le64_to_cpu(raw->om_most_recent_snap))
			report("Omap snapshot tree", "latest xid doesn't match keys.");
	} else if (raw->om_snap_count || raw->om_most_recent_snap) {
		report("Object map", "has snapshots but no snapshot tree.");
	}

	/* Oddly, the type is still reported even when the tree is not set */
	if (le32_to_cpu(raw->om_snapshot_tree_type) !=
				(APFS_OBJECT_TYPE_BTREE | APFS_OBJ_PHYSICAL))
		report("Object map", "wrong type for snapshot tree.");

	if (raw->om_pending_revert_min || raw->om_pending_revert_max)
		report_unknown("Revert in progress");

	omap = calloc(1, sizeof(*omap));
	if (!omap)
		system_error();
	omap->type = BTREE_TYPE_OMAP;
	omap->omap_table = NULL; /* The omap doesn't have an omap of its own */
	omap->root = read_node(le64_to_cpu(raw->om_tree_oid), omap, NULL /* hash */);

	/* The tree type reported by the omap must match the root node */
	if (raw->om_tree_type != omap->root->raw->btn_o.o_type)
		report("Object map", "wrong type for tree.");

	parse_subtree(omap->root, &last_key, NULL /* name_buf */);

	check_btree_footer(omap);
	munmap(raw, obj.size);
	return omap;
}

/**
 * parse_extentref_btree - Parse and check an extent reference tree
 * @oid:	object id for the b-tree root
 *
 * Returns a pointer to the btree struct for the extent reference tree.
 */
struct btree *parse_extentref_btree(u64 oid)
{
	struct btree *extref;
	struct key last_key = {0};

	extref = calloc(1, sizeof(*extref));
	if (!extref)
		system_error();
	extref->type = BTREE_TYPE_EXTENTREF;
	extref->omap_table = NULL; /* These are physical objects */
	extref->root = read_node(oid, extref, NULL /* hash */);

	parse_subtree(extref->root, &last_key, NULL /* name_buf */);

	check_btree_footer(extref);
	return extref;
}

/**
 * child_from_query - Read the child id found by a successful nonleaf query
 * @query:	the query that found the record
 *
 * Returns the child id in the nonleaf node record.
 */
static u64 child_from_query(struct query *query)
{
	void *raw = query->node->raw;

	/* This check is actually redundant, at least for now */
	if (query->len != 8) /* The data on a nonleaf node is the child id */
		report("B-tree", "wrong size of nonleaf record value.");

	return le64_to_cpu(*(__le64 *)(raw + query->off));
}

/**
 * extref_rec_from_query - Read the info found by a successful extentref query
 * @query:	the query for the extent reference record
 * @extref:	extent reference record struct to receive the result
 */
static void extref_rec_from_query(struct query *query,
				  struct extref_record *extref)
{
	struct apfs_phys_ext_val *extref_val;
	struct apfs_phys_ext_key *extref_key;
	void *raw = query->node->raw;
	int kind;

	if (query->len != sizeof(*extref_val))
		report("Extent reference record", "wrong size of value.");

	extref_val = (struct apfs_phys_ext_val *)(raw + query->off);
	extref_key = (struct apfs_phys_ext_key *)(raw + query->key_off);

	/* The physical address is used as the id in the extentref tree */
	extref->phys_addr = cat_cnid(&extref_key->hdr);
	extref->blocks = le64_to_cpu(extref_val->len_and_kind) &
							APFS_PEXT_LEN_MASK;
	extref->owner = le64_to_cpu(extref_val->owning_obj_id);
	extref->refcnt = le32_to_cpu(extref_val->refcnt);

	kind = le64_to_cpu(extref_val->len_and_kind) >> APFS_PEXT_KIND_SHIFT;
	extref->update = kind == APFS_KIND_UPDATE;
}

/*
 * In-memory fext record
 */
struct fext_record {
	u64 log_addr;	/* Logical address */
	u64 phys_bno;	/* Physical block number */
	u64 length;	/* Length (in bytes) */
};

/**
 * fext_rec_from_query - Read the info found by a successful fext query
 * @query:	the query for the fext record
 * @fext:	fext record struct to receive the result
 */
static void fext_rec_from_query(struct query *query, struct fext_record *fext)
{
	struct apfs_fext_tree_val *fext_val = NULL;
	struct apfs_fext_tree_key *fext_key = NULL;
	void *raw = query->node->raw;

	if (query->len != sizeof(*fext_val))
		report("Fext record", "wrong size of value.");

	fext_val = (struct apfs_fext_tree_val *)(raw + query->off);
	fext_key = (struct apfs_fext_tree_key *)(raw + query->key_off);

	fext->log_addr = le64_to_cpu(fext_key->logical_addr);
	fext->phys_bno = le64_to_cpu(fext_val->phys_block_num);
	fext->length = le64_to_cpu(fext_val->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;
}

/**
 * fext_tree_lookup - Map a logical address to a physical one in a sealed volume
 * @oid:	dstream id
 * @logaddr:	logical address inside the dstream
 * @bno:	on return, the physical block number
 *
 * Returns 0 on success, or -1 if nothing was found.
 */
int fext_tree_lookup(u64 oid, u64 logaddr, u64 *bno)
{
	struct query *query = NULL;
	struct key key = {0};
	struct fext_record fext = {0};
	u64 off;
	int ret = -1;

	query = alloc_query(vsb->v_fext->root, NULL /* parent */);

	init_fext_key(oid, logaddr, &key);
	query->key = &key;
	query->flags |= QUERY_FEXT;

	/*
	 * The fext nodes have already been parsed, and the allocation
	 * bitmap has been updated accordingly.  This global variable tells
	 * read_object() to ignore the bitmap this time.
	 */
	ongoing_query = true;
	if (btree_query(&query))
		goto fail;
	ongoing_query = false;

	fext_rec_from_query(query, &fext);
	if (fext.phys_bno == 0) {
		*bno = 0;
	} else {
		off = (logaddr - fext.log_addr) >> sb->s_blocksize_bits;
		*bno = fext.phys_bno + off;
	}

	ret = 0;
fail:
	free_query(query);
	return ret;
}

/**
 * extent_from_query - Read the info found by a successful file extent query
 * @query:	the query for the extent record
 * @fext:	fext record struct to receive the result
 */
static void extent_from_query(struct query *query, struct fext_record *fext)
{
	struct apfs_file_extent_val *ext;
	struct apfs_file_extent_key *ext_key;
	void *raw = query->node->raw;

	if (query->len != sizeof(*ext))
		report("File extent record", "wrong size of value.");

	ext = (struct apfs_file_extent_val *)(raw + query->off);
	ext_key = (struct apfs_file_extent_key *)(raw + query->key_off);

	fext->log_addr = le64_to_cpu(ext_key->logical_addr);
	fext->phys_bno = le64_to_cpu(ext->phys_block_num);
	fext->length = le64_to_cpu(ext->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;
}

/**
 * file_extent_lookup - Map a logical address to a physical one in an unsealed volume
 * @oid:	dstream id
 * @logaddr:	logical address inside the dstream
 * @bno:	on return, the physical block number
 *
 * Returns 0 on success, or -1 if nothing was found.
 */
int file_extent_lookup(u64 oid, u64 logaddr, u64 *bno)
{
	struct query *query = NULL;
	struct key key = {0};
	struct fext_record fext = {0};
	u64 off;
	int ret = -1;

	query = alloc_query(vsb->v_cat->root, NULL /* parent */);

	init_file_extent_key(oid, logaddr, &key);
	query->key = &key;
	query->flags |= QUERY_CAT;

	/*
	 * The catalog nodes have already been parsed, and the allocation
	 * bitmap has been updated accordingly.  This global variable tells
	 * read_object() to ignore the bitmap this time.
	 */
	ongoing_query = true;
	if (btree_query(&query))
		goto fail;
	ongoing_query = false;

	extent_from_query(query, &fext);
	if (fext.phys_bno == 0) {
		*bno = 0;
	} else {
		off = (logaddr - fext.log_addr) >> sb->s_blocksize_bits;
		*bno = fext.phys_bno + off;
	}

	ret = 0;
fail:
	free_query(query);
	return ret;
}

/**
 * extentref_tree_lookup - Find best match for an extent in an extentref tree
 * @tbl:	root of the extent reference tree to be searched
 * @bno:	first block number for the extent
 * @extref:	extentref record struct to receive the result
 *
 * Returns 0 on success, or -1 if nothing was found.
 */
static int extentref_tree_lookup(struct node *tbl, u64 bno, struct extref_record *extref)
{
	struct query *query;
	struct key key;
	int ret = -1;

	query = alloc_query(tbl, NULL /* parent */);

	init_extref_key(bno, &key);
	query->key = &key;
	query->flags |= QUERY_EXTENTREF;

	/*
	 * The extentref nodes have already been parsed, and the allocation
	 * bitmap has been updated accordingly.  This global variable tells
	 * read_object() to ignore the bitmap this time.
	 */
	ongoing_query = true;
	if (btree_query(&query))
		goto fail;
	ongoing_query = false;

	extref_rec_from_query(query, extref);
	ret = 0;
fail:
	free_query(query);
	return ret;
}

/**
 * extentref_update_lookup - Find latest snap phys extent for an updated block
 * @bno:	block number
 * @extref:	extentref record struct to receive the result
 */
void extentref_update_lookup(u64 bno, struct extref_record *extref)
{
	struct listed_btree *ext_tree = NULL;
	int ret;

	/* We look at the most recent snapshots first */
	for (ext_tree = vsb->v_snap_extrefs; ext_tree; ext_tree = ext_tree->next) {
		ret = extentref_tree_lookup(ext_tree->btree->root, bno, extref);
		if (ret == 0 && extref->phys_addr <= bno && extref->phys_addr + extref->blocks > bno)
			return;
	}

	report("Physical extent record", "update of nonexistent record.");
}

/**
 * alloc_query - Allocates a query structure
 * @node:	node to be searched
 * @parent:	query for the parent node
 *
 * Callers other than btree_query() should set @parent to NULL, and @node
 * to the root of the b-tree. They should also initialize most of the query
 * fields themselves; when @parent is not NULL the query will inherit them.
 *
 * Returns the allocated query.
 */
struct query *alloc_query(struct node *node, struct query *parent)
{
	struct query *query;

	query = malloc(sizeof(*query));
	if (!query)
		system_error();

	query->node = node;
	query->key = parent ? parent->key : NULL;
	query->flags = parent ? parent->flags & ~(QUERY_DONE | QUERY_NEXT) : 0;
	query->parent = parent;
	/* Start the search with the last record and go backwards */
	query->index = node->records;
	query->depth = parent ? parent->depth + 1 : 0;

	return query;
}

/**
 * free_query - Free a query structure
 * @query:	query to free
 *
 * Also frees the ancestor queries, if they are kept.
 */
void free_query(struct query *query)
{
	while (query) {
		struct query *parent = query->parent;

		node_free(query->node);
		free(query);
		query = parent;
	}
}

/**
 * key_from_query - Read the current key from a query structure
 * @query:	the query, with @query->key_off and @query->key_len already set
 * @key:	return parameter for the key
 *
 * Reads the key into @key after some basic sanity checks.
 */
static void key_from_query(struct query *query, struct key *key)
{
	void *raw = query->node->raw;
	void *raw_key = (void *)(raw + query->key_off);

	switch (query->flags & QUERY_TREE_MASK) {
	case QUERY_CAT:
		read_cat_key(raw_key, query->key_len, key);
		break;
	case QUERY_OMAP:
		read_omap_key(raw_key, query->key_len, key);
		break;
	case QUERY_EXTENTREF:
		read_extentref_key(raw_key, query->key_len, key);
		break;
	case QUERY_FEXT:
		read_fext_key(raw_key, query->key_len, key);
		break;
	default:
		report(NULL, "Bug!");
	}

	if (query->flags & QUERY_MULTIPLE) {
		/* A multiple query must ignore these fields */
		key->number = 0;
		key->name = NULL;
	}
}

/**
 * node_next - Find the next matching record in the current node
 * @query:	multiple query in execution
 *
 * Returns 0 on success, -EAGAIN if the next record is in another node, and
 * -ENODATA if no more matching records exist.
 */
static int node_next(struct query *query)
{
	struct node *node = query->node;
	struct key curr_key;
	int cmp;
	u64 bno = node->object.block_nr;

	if (query->flags & QUERY_DONE)
		/* Nothing left to search; the query failed */
		return -ENODATA;

	if (!query->index) /* The next record may be in another node */
		return -EAGAIN;
	--query->index;

	query->key_len = node_locate_key(node, query->index, &query->key_off);
	key_from_query(query, &curr_key);

	cmp = keycmp(&curr_key, query->key);

	if (cmp > 0)
		report("B-tree", "records are out of order.");

	if (cmp != 0 && node_is_leaf(node) && query->flags & QUERY_EXACT)
		return -ENODATA;

	query->len = node_locate_data(node, query->index, &query->off);
	if (query->len == 0) {
		report("B-tree", "corrupted record value in node 0x%llx.",
		       (unsigned long long)bno);
	}

	if (cmp != 0) {
		/*
		 * This is the last entry that can be relevant in this node.
		 * Keep searching the children, but don't return to this level.
		 */
		query->flags |= QUERY_DONE;
	}

	return 0;
}

/**
 * node_query - Execute a query on a single node
 * @query:	the query to execute
 *
 * The search will start at index @query->index, looking for the key that comes
 * right before @query->key, according to the order given by keycmp().
 *
 * The @query->index will be updated to the last index checked. This is
 * important when searching for multiple entries, since the query may need
 * to remember where it was on this level. If we are done with this node, the
 * query will be flagged as QUERY_DONE, and the search will end in failure
 * as soon as we return to this level. The function may also return -EAGAIN,
 * to signal that the search should go on in a different branch.
 *
 * On success returns 0; the offset of the data within the block will be saved
 * in @query->off, and its length in @query->len. The function checks that this
 * length fits within the block; callers must use the returned value to make
 * sure they never operate outside its bounds.
 *
 * -ENODATA will be returned if no appropriate entry was found.
 */
static int node_query(struct query *query)
{
	struct node *node = query->node;
	int left, right;
	int cmp;
	u64 bno = node->object.block_nr;

	if (query->flags & QUERY_NEXT)
		return node_next(query);

	/* Search by bisection */
	cmp = 1;
	left = 0;
	do {
		struct key curr_key;
		if (cmp > 0) {
			right = query->index - 1;
			if (right < left)
				return -ENODATA;
			query->index = (left + right) / 2;
		} else {
			left = query->index;
			query->index = DIV_ROUND_UP(left + right, 2);
		}

		query->key_len = node_locate_key(node, query->index,
						 &query->key_off);
		key_from_query(query, &curr_key);

		cmp = keycmp(&curr_key, query->key);
		if (cmp == 0 && !(query->flags & QUERY_MULTIPLE))
			break;
	} while (left != right);

	if (cmp > 0)
		return -ENODATA;

	if (cmp != 0 && node_is_leaf(query->node) && query->flags & QUERY_EXACT)
		return -ENODATA;

	if (query->flags & QUERY_MULTIPLE) {
		if (cmp != 0) /* Last relevant entry in level */
			query->flags |= QUERY_DONE;
		query->flags |= QUERY_NEXT;
	}

	query->len = node_locate_data(node, query->index, &query->off);
	if (query->len == 0) {
		report("B-tree", "corrupted record value in node 0x%llx.",
		       (unsigned long long)bno);
	}
	return 0;
}

/**
 * btree_query - Execute a query on a b-tree
 * @query:	the query to execute
 *
 * Searches the b-tree starting at @query->index in @query->node, looking for
 * the record corresponding to @query->key.
 *
 * Returns 0 in case of success and sets the @query->len, @query->off and
 * @query->index fields to the results of the query. @query->node will now
 * point to the leaf node holding the record.
 *
 * In case of failure returns -ENODATA.
 */
int btree_query(struct query **query)
{
	struct node *node = (*query)->node;
	struct query *parent;
	struct btree *btree = node->btree;
	u64 child_id;
	int err;

	assert(ongoing_query);

next_node:
	if ((*query)->depth >= 12) {
		/* This is the maximum depth allowed by the module */
		report("B-tree", "is too deep.");
	}

	err = node_query(*query);
	if (err == -EAGAIN) {
		if (!(*query)->parent) /* We are at the root of the tree */
			return -ENODATA;

		/* Move back up one level and continue the query */
		parent = (*query)->parent;
		(*query)->parent = NULL; /* Don't free the parent */
		free_query(*query);
		*query = parent;
		goto next_node;
	}
	if (err)
		return err;
	if (node_is_leaf((*query)->node)) /* All done */
		return 0;

	/* Now go a level deeper and search the child */
	child_id = child_from_query(*query);
	node = read_node(child_id, btree, NULL /* hash */);

	if ((*query)->flags & QUERY_MULTIPLE) {
		/*
		 * We are looking for multiple entries, so we must remember
		 * the parent node and index to continue the search later.
		 */
		*query = alloc_query(node, *query);
	} else {
		/* Reuse the same query structure to search the child */
		node_free((*query)->node);
		(*query)->node = node;
		(*query)->index = node->records;
		(*query)->depth++;
	}
	goto next_node;
}
