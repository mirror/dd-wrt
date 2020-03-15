/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <apfs/checksum.h>
#include <apfs/raw.h>
#include "apfsck.h"
#include "btree.h"
#include "htable.h"
#include "object.h"
#include "super.h"

int obj_verify_csum(struct apfs_obj_phys *obj)
{
	return  (le64_to_cpu(obj->o_cksum) ==
		 fletcher64((char *) obj + APFS_MAX_CKSUM_SIZE,
			    sb->s_blocksize - APFS_MAX_CKSUM_SIZE));
}

/**
 * read_object_nocheck - Read an object header from disk
 * @bno: block number for the object
 * @obj: object struct to receive the results
 *
 * Returns a pointer to the raw data of the object in memory, without running
 * any checks other than the Fletcher verification.
 */
void *read_object_nocheck(u64 bno, struct object *obj)
{
	struct apfs_obj_phys *raw;

	raw = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE,
		   fd, bno * sb->s_blocksize);
	if (raw == MAP_FAILED)
		system_error();

	/* This one check is always needed */
	if (!obj_verify_csum(raw)) {
		report("Object header", "bad checksum in block 0x%llx.",
		       (unsigned long long)bno);
	}

	obj->oid = le64_to_cpu(raw->o_oid);
	obj->xid = le64_to_cpu(raw->o_xid);
	obj->block_nr = bno;
	obj->type = le32_to_cpu(raw->o_type) & APFS_OBJECT_TYPE_MASK;
	obj->flags = le32_to_cpu(raw->o_type) & APFS_OBJECT_TYPE_FLAGS_MASK;
	obj->subtype = le32_to_cpu(raw->o_subtype);

	return raw;
}

/**
 * parse_object_flags - Check consistency of object flags
 * @flags: the flags
 *
 * Returns the storage type flags to be checked by the caller.
 */
u32 parse_object_flags(u32 flags)
{
	if ((flags & APFS_OBJECT_TYPE_FLAGS_DEFINED_MASK) != flags)
		report("Object header", "undefined flag in use.");
	if (flags & APFS_OBJ_NONPERSISTENT)
		report("Object header", "nonpersistent flag is set.");
	if (flags & APFS_OBJ_NOHEADER)
		report("Object header", "noheader flag is set.");
	if (flags & APFS_OBJ_ENCRYPTED)
		report_unknown("Encrypted object");
	return flags & APFS_OBJ_STORAGETYPE_MASK;
}

/**
 * read_object - Read an object header from disk and run some checks
 * @oid:	object id
 * @omap_table:	hash table for the object map (NULL if no translation is needed)
 * @obj:	object struct to receive the results
 *
 * Returns a pointer to the raw data of the object in memory, after checking
 * the consistency of some of its fields.
 */
void *read_object(u64 oid, struct htable_entry **omap_table, struct object *obj)
{
	struct apfs_obj_phys *raw;
	struct omap_record *omap_rec;
	u64 bno;
	u64 xid;
	u32 storage_type;

	if (omap_table) {
		omap_rec = get_omap_record(oid, omap_table);
		if (omap_rec->o_seen)
			report("Object map record", "oid was used twice.");
		omap_rec->o_seen = true;

		bno = omap_rec->o_bno;
		if (!bno)
			report("Object map", "record missing for id 0x%llx.",
			       (unsigned long long)oid);
	} else {
		bno = oid;
	}

	raw = read_object_nocheck(bno, obj);
	if (!ongoing_query) { /* Query code will revisit already parsed nodes */
		if (vsb)
			++vsb->v_block_count;
		container_bmap_mark_as_used(bno, 1 /* length */);
	}

	if (oid != obj->oid)
		report("Object header", "wrong object id in block 0x%llx.",
		       (unsigned long long)bno);
	if (oid < APFS_OID_RESERVED_COUNT)
		report("Object header", "reserved object id in block 0x%llx.",
		       (unsigned long long)bno);
	if (omap_table && oid >= sb->s_next_oid)
		report("Object header", "unassigned object id in block 0x%llx.",
		       (unsigned long long)bno);

	xid = obj->xid;
	if (!xid || sb->s_xid < xid)
		report("Object header", "bad transaction id in block 0x%llx.",
		       (unsigned long long)bno);
	if (vsb && vsb->v_first_xid > xid)
		report("Object header",
		       "transaction id in block 0x%llx is older than volume.",
		       (unsigned long long)bno);
	if (omap_table && xid != omap_rec->o_xid)
		report("Object header",
		       "transaction id in omap key doesn't match block 0x%llx.",
		       (unsigned long long)bno);

	/* Ephemeral objects are handled by read_ephemeral_object() */
	storage_type = parse_object_flags(obj->flags);
	if (omap_table && storage_type != APFS_OBJ_VIRTUAL)
		report("Object header", "wrong flag for virtual object.");
	if (!omap_table && storage_type != APFS_OBJ_PHYSICAL)
		report("Object header", "wrong flag for physical object.");

	return raw;
}

/**
 * free_cpoint_map - Free a map structure after performing some final checks
 * @entry: the entry to free
 */
static void free_cpoint_map(struct htable_entry *entry)
{
	struct cpoint_map *map = (struct cpoint_map *)entry;
	u32 blk_count = map->m_size >> sb->s_blocksize_bits;
	u64 obj_start = map->m_paddr;
	u64 obj_end = map->m_paddr + blk_count; /* Objects can't wrap, right? */
	u64 data_start = sb->s_data_base;
	u64 data_end = sb->s_data_base + sb->s_data_blocks;
	u64 valid_start;

	if (!map->m_seen)
		report("Checkpoint map", "no object for mapping.");

	if (obj_start < data_start || obj_end > data_end)
		report("Checkpoint map", "block number is out of range.");

	/* Not all blocks in the data area belong to the current checkpoint */
	valid_start = sb->s_data_base + sb->s_data_index;
	if (obj_start >= valid_start && obj_end > valid_start + sb->s_data_len)
		report("Checkpoint map", "block number outside valid range.");
	if (obj_start < valid_start &&
	    obj_end + sb->s_data_blocks > valid_start + sb->s_data_len)
		report("Checkpoint map", "block number outside valid range.");

	if (map->m_oid < APFS_OID_RESERVED_COUNT)
		report("Checkpoint map", "reserved object id.");
	if (map->m_oid >= sb->s_next_oid)
		report("Checkpoint map", "unassigned object id.");

	free(entry);
}

/**
 * free_cpoint_map_table - Free the checkpoint map table and all its entries
 * @table: table to free
 */
void free_cpoint_map_table(struct htable_entry **table)
{
	free_htable(table, free_cpoint_map);
}

/**
 * get_cpoint_map - Find or create a map structure in the checkpoint map table
 * @oid: ephemeral object id
 *
 * Returns the checkpoint mapping structure, after creating it if necessary.
 */
struct cpoint_map *get_cpoint_map(u64 oid)
{
	struct htable_entry *entry;

	entry = get_htable_entry(oid, sizeof(struct cpoint_map),
				 sb->s_cpoint_map_table);
	return (struct cpoint_map *)entry;
}

/**
 * read_ephemeral_object - Read an ephemeral object header from disk
 * @oid:	object id
 * @obj:	object struct to receive the results
 *
 * Returns a pointer to the raw data of the object in memory, after checking
 * the consistency of some of its fields.
 */
void *read_ephemeral_object(u64 oid, struct object *obj)
{
	struct apfs_obj_phys *raw;
	struct cpoint_map *map;
	u32 storage_type;

	assert(sb->s_cpoint_map_table);
	assert(sb->s_xid);

	map = get_cpoint_map(oid);
	if (!map->m_paddr)
		report("Ephemeral object", "missing checkpoint mapping.");
	if (map->m_seen)
		report("Checkpoint map", "an ephemeral object id was reused.");
	map->m_seen = true;

	/* Multiblock ephemeral objects may exist, but are not supported yet */
	raw = read_object_nocheck(map->m_paddr, obj);
	if ((obj->type | obj->flags) != map->m_type)
		report("Ephemeral object", "type field doesn't match mapping.");
	if (obj->subtype != map->m_subtype)
		report("Ephemeral object", "subtype doesn't match mapping.");
	if (obj->oid != oid)
		report("Ephemeral object", "wrong object id.");
	if (obj->xid != sb->s_xid)
		report("Ephemeral object", "not part of latest transaction.");

	storage_type = parse_object_flags(obj->flags);
	if (storage_type != APFS_OBJ_EPHEMERAL)
		report("Object header", "wrong flag for ephemeral object.");

	return raw;
}
