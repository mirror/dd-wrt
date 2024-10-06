/*
 * Copyright (C) 2022 Ernesto A. Fernández <ernesto@corellium.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "apfsck.h"
#include "htable.h"
#include "key.h"
#include "snapshot.h"
#include "super.h"

/**
 * free_snap - Free a snapshot structure after some final checks
 * @entry: the entry to free
 */
static void free_snap(struct htable_entry *entry)
{
	struct snapshot *snap = (struct snapshot *)entry;

	if (!snap->sn_name_seen || !snap->sn_meta_seen)
		report("Snapshot", "missing metadata entry.");
	if (!snap->sn_omap_seen)
		report("Snapshot", "missing omap entry.");
	free(entry);
}

/**
 * free_snap_table - Free the snapshot hash table and all its entries
 * @table: table to free
 */
void free_snap_table(struct htable_entry **table)
{
	free_htable(table, free_snap);
}

/**
 * get_snapshot - Find or create a snapshot structure in the snapshot hash table
 * @xid: transaction id for the snapshot
 *
 * Returns the snapshot structure, after creating it if necessary.
 */
struct snapshot *get_snapshot(u64 xid)
{
	struct htable_entry *entry;

	entry = get_htable_entry(xid, sizeof(struct snapshot), vsb->v_snap_table);
	return (struct snapshot *)entry;
}

/**
 * parse_snap_name_record - Parse and check a snapshot name record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_snap_name_record(struct apfs_snap_name_key *key, struct apfs_snap_name_val *val, int len)
{
	struct snapshot *snap = NULL;

	if (len != sizeof(*val))
		report("Snapshot name record", "wrong length for value.");

	snap = get_snapshot(le64_to_cpu(val->snap_xid));
	if (snap->sn_name_seen)
		report("Snapshot tree", "snap with two name records.");
	snap->sn_name_seen = true;

	if (!snap->sn_meta_seen || !snap->sn_meta_name)
		report("Snapshot tree", "missing a metadata record.");
	if (strcmp((char *)key->name, snap->sn_meta_name) != 0)
		report("Snapshot tree", "inconsistent names for snapshot.");
}

/**
 * check_snapshot - Check a whole snapshot
 * @xid:		transaction for the snapshot
 * @vol_bno:		block number for the snapshot volume superblock
 * @extentref_bno:	block number for the snapshot's extentref tree
 * @inum:		inode number for the snapshot
 */
static void check_snapshot(u64 xid, u64 vol_bno, u64 extentref_bno, u64 inum)
{
	struct volume_superblock *latest_vsb = vsb;
	u64 latest_xid = sb->s_xid;
	struct listed_cnid *cnid = NULL;

	vsb = NULL;

	sb->s_xid = xid;
	vsb = alloc_volume_super(true);
	/* The list of extref trees is shared by all transactions */
	vsb->v_snap_extrefs = latest_vsb->v_snap_extrefs;
	vsb->v_snap_count = latest_vsb->v_snap_count;
	vsb->v_raw = read_object(vol_bno, NULL, &vsb->v_obj);
	read_volume_super(latest_vsb->v_index, vsb, &vsb->v_obj);

	/*
	 * Each snapshot is given an "inode number" and, unlike dstream ids,
	 * they don't seem to overlap with real inode numbers. This lines are
	 * to check that the number is not reused inside the snapshot.
	 */
	cnid = get_listed_cnid(inum);
	cnid_set_state_flag(cnid, CNID_IN_INODE);

	if (vsb->v_extref_oid != 0)
		report("Snapshot volume superblock", "has extentref tree.");
	vsb->v_extref_oid = extentref_bno;

	if (vsb->v_omap_oid != 0)
		report("Snapshot volume superblock", "has object map.");
	vsb->v_omap = latest_vsb->v_omap;
	vsb->v_omap_table = latest_vsb->v_omap_table;
	omap_htable_clear_seen_for_snap(latest_vsb->v_omap_table);
	vsb->v_snap_max_xid = latest_vsb->v_snap_max_xid;

	vsb->v_extent_table = latest_vsb->v_extent_table;

	if (vsb->v_snap_meta_oid != 0)
		report("Snapshot volume superblock", "has snapshot tree.");

	check_volume_super();

	/* Go back to the latest transaction */
	sb->s_xid = latest_xid;
	latest_vsb->v_snap_extrefs = vsb->v_snap_extrefs;
	latest_vsb->v_block_count += vsb->v_block_count;
	vsb = latest_vsb; /* TODO: don't leak */

	/*
	 * Repeat this now to check that the number is not reused inside the
	 * next snapshot either, or for the current transaction.
	 */
	cnid = get_listed_cnid(inum);
	cnid_set_state_flag(cnid, CNID_IN_INODE);
}

/**
 * parse_snap_metadata_record - Parse and check a snapshot metadata record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_snap_metadata_record(struct apfs_snap_metadata_key *key, struct apfs_snap_metadata_val *val, int len)
{
	struct snapshot *snap = NULL;
	u64 snap_xid;
	int namelen;

	if (len < sizeof(*val) + 1)
		report("Snapshot metadata record", "value is too small.");
	if (*((char *)val + len - 1) != 0)
		report("Snapshot metadata record", "name lacks NULL-termination.");

	namelen = le16_to_cpu(val->name_len);
	if (strlen((char *)val->name) + 1 != namelen)
		report("Snapshot metadata record", "wrong name length.");
	if (len != sizeof(*val) + namelen)
		report("Snapshot metadata record", "size of value doesn't match name length.");

	snap_xid = cat_cnid(&key->hdr);
	snap = get_snapshot(snap_xid);
	if (snap->sn_meta_seen)
		report("Snapshot tree", "snap with two metadata records.");
	snap->sn_meta_seen = true;

	snap->sn_meta_name = calloc(1, namelen);
	if (!snap->sn_meta_name)
		system_error();
	strcpy(snap->sn_meta_name, (char *)val->name);

	if (le32_to_cpu(val->extentref_tree_type) != (APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_BTREE))
		report("Snapshot metadata", "wrong type for extentref tree.");
	if (val->flags)
		report_unknown("Snapshot flags");
	if (!val->inum)
		report("Snapshot metadata", "no inode number.");

	check_snapshot(snap_xid, le64_to_cpu(val->sblock_oid), le64_to_cpu(val->extentref_tree_oid), le64_to_cpu(val->inum));
	++vsb->v_snap_count;
}

/**
 * parse_snap_record - Parse and check a snapshot tree record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_snap_record(void *key, void *val, int len)
{
	switch (cat_type(key)) {
	case APFS_TYPE_SNAP_METADATA:
		return parse_snap_metadata_record(key, val, len);
	case APFS_TYPE_SNAP_NAME:
		return parse_snap_name_record(key, val, len);
	default:
		report(NULL, "Bug!");
	}
}

/**
 * parse_omap_snap_record - Parse and check an omap snapshot tree record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_omap_snap_record(__le64 *key, struct apfs_omap_snapshot *val, int len)
{
	struct snapshot *snapshot = NULL;
	u64 snap_xid;

	/*
	 * These keys and values must be aligned to eight bytes.
	 * TODO: add the same check to the free queue?
	 */
	if ((u64)key & 7 || (u64)val & 7)
		report("Omap snapshot record", "bad alignment for key or value.");

	if (len != sizeof(*val))
		report("Omap snapshot record", "value is too small.");

	snap_xid = le64_to_cpu(*key);
	if (snap_xid == 0)
		report("Omap snapshot record", "xid is zero.");
	if (snap_xid > sb->s_xid)
		report("Omap snapshot record", "xid is in the future.");
	if (snap_xid >= vsb->v_snap_max_xid)
		vsb->v_snap_max_xid = snap_xid;
	snapshot = get_snapshot(snap_xid);
	snapshot->sn_omap_seen = true;

	if (val->oms_flags)
		report_unknown("Deleted or reverted snapshot");
	if (val->oms_pad)
		report("Omap snapshot record", "padding should be zeroes.");
	if (val->oms_oid)
		report("Omap snapshot record", "oid should be zero.");
}
