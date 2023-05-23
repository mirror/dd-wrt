/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _BTREE_H
#define _BTREE_H

#include <apfs/raw.h>
#include <apfs/types.h>
#include "htable.h"
#include "object.h"

struct super_block;
struct extref_record;
struct free_queue;

/*
 * Omap record data in memory
 */
struct omap_record {
	struct omap_record *next; /* Next entry in linked list */

	u64	xid;	/* Transaction id */
	u64	oid;	/* Virtual object id */
	u64	bno;	/* Block number */
	u32	flags;	/* Omap record flags */

	/* Was this oid-xid pair ever seen in use? */
	bool	seen;
	/* Was it ever seen in use for the latest checkpoint? */
	bool	seen_for_latest;
	/* If we are currently checking a snapshot, was it seen in use for it? */
	bool	seen_for_snap;
};

/*
 * List of all omap records for a given oid
 */
struct omap_record_list {
	struct htable_entry	o_htable;	/* Hash table entry header */
	struct omap_record 	*o_records;	/* Linked list of records */
};

/*
 * In-memory representation of an APFS node
 */
struct node {
	u16 flags;		/* Node flags */
	u32 records;		/* Number of records in the node */
	int level;		/* Number of child levels below this node */

	int toc;		/* Offset of the TOC in the block */
	int key;		/* Offset of the key area in the block */
	int free;		/* Offset of the free area in the block */
	int data;		/* Offset of the data area in the block */

	u8 *free_key_bmap;	/* Free space bitmap for the key area */
	u8 *free_val_bmap;	/* Free space bitmap for the value area */
	u8 *used_key_bmap;	/* Used space bitmap for the key area */
	u8 *used_val_bmap;	/* Used space bitmap for the value area */

	struct btree *btree;			/* Btree the node belongs to */
	struct apfs_btree_node_phys *raw;	/* Raw node in memory */
	struct object object;			/* Object holding the node */
};

/**
 * apfs_node_is_leaf - Check if a b-tree node is a leaf
 * @node: the node to check
 */
static inline bool node_is_leaf(struct node *node)
{
	return (node->flags & APFS_BTNODE_LEAF) != 0;
}

/**
 * apfs_node_is_root - Check if a b-tree node is the root
 * @node: the node to check
 */
static inline bool node_is_root(struct node *node)
{
	return (node->flags & APFS_BTNODE_ROOT) != 0;
}

/**
 * apfs_node_has_fixed_kv_size - Check if a b-tree node has fixed key/value
 * sizes
 * @node: the node to check
 */
static inline bool node_has_fixed_kv_size(struct node *node)
{
	return (node->flags & APFS_BTNODE_FIXED_KV_SIZE) != 0;
}

/* Flags for the query structure */
#define QUERY_TREE_MASK		0017	/* Which b-tree we query */
#define QUERY_OMAP		0001	/* This is a b-tree object map query */
#define QUERY_CAT		0002	/* This is a catalog tree query */
#define QUERY_EXTENTREF		0004	/* This is an extentref tree query */
#define QUERY_FEXT		0010	/* This is an extentref tree query */
#define QUERY_MULTIPLE		0020	/* Search for multiple matches */
#define QUERY_NEXT		0040	/* Find next of multiple matches */
#define QUERY_EXACT		0100	/* Search for an exact match */
#define QUERY_DONE		0200	/* The search at this level is over */

/*
 * Structure used to retrieve data from an APFS B-Tree. For now only used
 * on the calalog and the object map.
 */
struct query {
	struct node *node;		/* Node being searched */
	struct key *key;		/* What the query is looking for */

	struct query *parent;		/* Query for parent node */
	unsigned int flags;

	/* Set by the query on success */
	int index;			/* Index of the entry in the node */
	int key_off;			/* Offset of the key in the node */
	int key_len;			/* Length of the key */
	int off;			/* Offset of the data in the node */
	int len;			/* Length of the data */

	int depth;			/* Put a limit on recursion */
};

/* In-memory tree types */
#define BTREE_TYPE_OMAP		1 /* The tree is an object map */
#define BTREE_TYPE_CATALOG	2 /* The tree is a catalog */
#define BTREE_TYPE_EXTENTREF	3 /* The tree is for extent references */
#define BTREE_TYPE_SNAP_META	4 /* The tree is for snapshot metadata */
#define BTREE_TYPE_FREE_QUEUE	5 /* The tree is for a free-space queue */
#define BTREE_TYPE_SNAPSHOTS	6 /* The tree is for omap snapshots */
#define BTREE_TYPE_FEXT		7 /* The tree is for file extents */

/* In-memory structure representing a b-tree */
struct btree {
	u8 type;		/* Type of the tree */
	struct node *root;	/* Root of this b-tree */

	/* Hash table for the tree's object map (can be NULL) */
	struct htable_entry **omap_table;

	/* B-tree stats as measured by the fsck */
	u64 key_count;		/* Number of keys */
	u64 node_count;		/* Number of nodes */
	int longest_key;	/* Length of longest key */
	int longest_val;	/* Length of longest value */
};

/**
 * btree_is_free_queue - Check if a b-tree is a free-space queue
 * @btree: the b-tree to check
 */
static inline bool btree_is_free_queue(struct btree *btree)
{
	return btree->type == BTREE_TYPE_FREE_QUEUE;
}

static inline bool btree_is_snapshots(struct btree *btree)
{
	return btree->type == BTREE_TYPE_SNAPSHOTS;
}

/**
 * btree_is_omap - Check if a b-tree is an object map
 * @btree: the b-tree to check
 */
static inline bool btree_is_omap(struct btree *btree)
{
	return btree->type == BTREE_TYPE_OMAP;
}

/**
 * btree_is_snap_meta - Check if a b-tree is for snapshot metadata
 * @btree: the b-tree to check
 */
static inline bool btree_is_snap_meta(struct btree *btree)
{
	return btree->type == BTREE_TYPE_SNAP_META;
}

/**
 * btree_is_catalog - Check if a b-tree is a catalog
 * @btree: the b-tree to check
 */
static inline bool btree_is_catalog(struct btree *btree)
{
	return btree->type == BTREE_TYPE_CATALOG;
}

/**
 * btree_is_extentref - Check if a b-tree is for extent references
 * @btree: the b-tree to check
 */
static inline bool btree_is_extentref(struct btree *btree)
{
	return btree->type == BTREE_TYPE_EXTENTREF;
}

/**
 * btree_is_fext - Check if a b-tree is for file extents
 * @btree: the b-tree to check
 */
static inline bool btree_is_fext(struct btree *btree)
{
	return btree->type == BTREE_TYPE_FEXT;
}

extern struct free_queue *parse_free_queue_btree(u64 oid, int index);
extern struct btree *parse_snap_meta_btree(u64 oid);
extern struct btree *parse_extentref_btree(u64 oid);
extern struct btree *parse_omap_btree(u64 oid);
extern struct btree *parse_cat_btree(u64 oid, struct htable_entry **omap_table);
extern struct btree *parse_fext_btree(u64 oid);
extern struct query *alloc_query(struct node *node, struct query *parent);
extern void free_query(struct query *query);
extern int btree_query(struct query **query);
extern struct node *omap_read_node(u64 id);
extern void free_omap_table(struct htable_entry **table);
extern struct omap_record *get_latest_omap_record(u64 oid, u64 xid, struct htable_entry **table);
extern void extentref_update_lookup(u64 bno, struct extref_record *extref);
extern int fext_tree_lookup(u64 oid, u64 logaddr, u64 *bno);
extern int file_extent_lookup(u64 oid, u64 logaddr, u64 *bno);
extern void omap_htable_clear_seen_for_snap(struct htable_entry **table);

#endif	/* _BTREE_H */
