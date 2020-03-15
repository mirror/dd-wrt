/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <apfs/raw.h>
#include "apfsck.h"
#include "btree.h"
#include "extents.h"
#include "htable.h"
#include "inode.h"
#include "key.h"
#include "super.h"

/**
 * free_extent - Free an extent structure after performing a final check
 * @entry: the entry to free
 */
static void free_extent(struct htable_entry *entry)
{
	struct extent *extent = (struct extent *)entry;

	if (extent->e_refcnt != extent->e_references)
		report("Physical extent record", "bad reference count.");

	free(entry);
}

/**
 * free_extent_table - Free the extent hash table and all its entries
 * @table: table to free
 */
void free_extent_table(struct htable_entry **table)
{
	free_htable(table, free_extent);
}

/**
 * get_extent - Find or create an extent structure in the extent hash table
 * @bno: first physical block of the extent
 *
 * Returns the extent structure, after creating it if necessary.
 */
static struct extent *get_extent(u64 bno)
{
	struct htable_entry *entry;

	entry = get_htable_entry(bno, sizeof(struct extent),
				 vsb->v_extent_table);
	return (struct extent *)entry;
}

/**
 * check_dstream_stats - Verify the stats gathered by the fsck vs the metadata
 * @dstream: dstream structure to check
 */
static void check_dstream_stats(struct dstream *dstream)
{
	if (!dstream->d_references)
		report("Data stream", "has no references.");
	if (dstream->d_id < APFS_MIN_USER_INO_NUM)
		report("Data stream", "invalid or reserved id.");
	if (dstream->d_id >= vsb->v_next_obj_id)
		report("Data stream", "free id in use.");

	if (dstream->d_obj_type == APFS_TYPE_XATTR) {
		if (dstream->d_seen || dstream->d_references != 1)
			report("Data stream", "xattrs can't be cloned.");
	} else {
		if (!dstream->d_seen)
			report("Data stream", "missing reference count.");
		if (dstream->d_refcnt != dstream->d_references)
			report("Data stream", "bad reference count.");
	}

	if (dstream->d_bytes < dstream->d_size)
		report("Data stream", "some extents are missing.");
	if (dstream->d_bytes != dstream->d_alloced_size)
		report("Data stream", "wrong allocated space.");
}

/**
 * free_dstream - Free a dstream structure after performing some final checks
 * @entry: the entry to free
 */
static void free_dstream(struct htable_entry *entry)
{
	struct dstream *dstream = (struct dstream *)entry;
	struct listed_cnid *cnid;
	struct listed_extent *curr_extent = dstream->d_extents;

	/* The dstreams must be freed before the cnids */
	assert(vsb->v_cnid_table);

	/* To check for reuse, put all filesystem object ids in a list */
	cnid = get_listed_cnid(dstream->d_id);
	if (cnid->c_state == CNID_USED)
		report("Catalog", "a filesystem object id was used twice.");
	cnid->c_state = CNID_USED;

	/* Increase the refcount of each physical extent used by the dstream */
	while (curr_extent) {
		struct listed_extent *next_extent;
		struct extent *extent;

		extent = get_extent(curr_extent->paddr);
		if (extent->e_references) {
			if (extent->e_obj_type != dstream->d_obj_type)
				report("Physical extent record",
				       "owners have inconsistent types.");
			/* Only count the extent once for each owner */
			if (extent->e_latest_owner != dstream->d_owner)
				extent->e_references++;
		} else {
			extent->e_references++;
		}
		extent->e_obj_type = dstream->d_obj_type;
		extent->e_latest_owner = dstream->d_owner;

		next_extent = curr_extent->next;
		free(curr_extent);
		curr_extent = next_extent;
	}

	check_dstream_stats(dstream);
	free(entry);
}

/**
 * free_dstream_table - Free the dstream hash table and all its entries
 * @table: table to free
 */
void free_dstream_table(struct htable_entry **table)
{
	free_htable(table, free_dstream);
}

/**
 * get_dstream - Find or create a dstream structure in the dstream hash table
 * @id:		id of the dstream
 *
 * Returns the dstream structure, after creating it if necessary.
 */
struct dstream *get_dstream(u64 id)
{
	struct htable_entry *entry;

	entry = get_htable_entry(id, sizeof(struct dstream),
				 vsb->v_dstream_table);
	return (struct dstream *)entry;
}

/**
 * attach_prange_to_dstream - Attach a physical range to a dstream structure
 * @paddr:	physical address of the range
 * @blk_count:	number of blocks in the range
 * @dstream:	dstream structure
 */
static void attach_extent_to_dstream(u64 paddr, u64 blk_count,
				     struct dstream *dstream)
{
	struct listed_extent **ext_p = &dstream->d_extents;
	struct listed_extent *ext = *ext_p;
	struct listed_extent *new;
	struct extref_record extref;

	if (paddr + blk_count < paddr) /* Overflow */
		report("Extent record", "physical address is too big.");

	/* Find out which physical extent contains this address range */
	extentref_lookup(vsb->v_extent_ref->root, paddr, &extref);
	if (extref.phys_addr + extref.blocks < paddr + blk_count)
		report("Extent record", "no entry in extent reference tree.");
	paddr = extref.phys_addr;

	/* Entries are ordered by their physical address */
	while (ext) {
		/* Count physical extents only once for each owner */
		if (paddr == ext->paddr)
			return;

		if (paddr < ext->paddr)
			break;
		ext_p = &ext->next;
		ext = *ext_p;
	}

	new = malloc(sizeof(*new));
	if (!new)
		system_error();

	new->paddr = paddr;
	new->next = ext;
	*ext_p = new;
}

/**
 * parse_extent_record - Parse an extent record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_extent_record(struct apfs_file_extent_key *key,
			 struct apfs_file_extent_val *val, int len)
{
	struct dstream *dstream;
	u64 length, flags;

	if (len != sizeof(*val))
		report("Extent record", "wrong size of value.");

	if (val->crypto_id)
		report_unknown("Extent encryption");

	length = le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;
	if (!length)
		report("Extent record", "length is zero.");
	if (length & (sb->s_blocksize - 1))
		report("Extent record", "length isn't multiple of block size.");

	flags = le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_FLAG_MASK;
	if (flags)
		report("Extent record", "no flags should be set.");

	dstream = get_dstream(cat_cnid(&key->hdr));
	if (dstream->d_bytes != le64_to_cpu(key->logical_addr))
		report("Data stream", "extents are not consecutive.");
	dstream->d_bytes += length;

	if (!le64_to_cpu(val->phys_block_num)) { /* This is a hole */
		dstream->d_sparse_bytes += length;
		return;
	}
	attach_extent_to_dstream(le64_to_cpu(val->phys_block_num),
				 length >> sb->s_blocksize_bits, dstream);
}

/**
 * parse_dstream_id_record - Parse a dstream id record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_dstream_id_record(struct apfs_dstream_id_key *key,
			     struct apfs_dstream_id_val *val, int len)
{
	struct dstream *dstream;

	if (len != sizeof(*val))
		report("Dstream id record", "wrong size of value.");

	dstream = get_dstream(cat_cnid(&key->hdr));
	dstream->d_seen = true;
	dstream->d_refcnt = le32_to_cpu(val->refcnt);
}

/**
 * parse_phys_ext_record - Parse and check a physical extent record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Returns the physical address of the last block in the extent.
 *
 * Internal consistency of @key must be checked before calling this function.
 */
u64 parse_phys_ext_record(struct apfs_phys_ext_key *key,
			  struct apfs_phys_ext_val *val, int len)
{
	struct extent *extent;
	u8 kind;
	u32 refcnt;
	u64 length, owner;

	if (len != sizeof(*val))
		report("Physical extent record", "wrong size of value.");

	kind = le64_to_cpu(val->len_and_kind) >> APFS_PEXT_KIND_SHIFT;
	if (kind != APFS_KIND_NEW && kind != APFS_KIND_UPDATE)
		report("Physical extent record", "invalid kind");
	if (kind == APFS_KIND_UPDATE)
		report_unknown("Snapshots");

	length = le64_to_cpu(val->len_and_kind) & APFS_PEXT_LEN_MASK;
	if (!length)
		report("Physical extent record", "has no blocks.");

	/*
	 * If the owner of a physical extent got removed, I would expect this
	 * field to be meaningless.  At least check that the number is in range.
	 */
	owner = le64_to_cpu(val->owning_obj_id);
	if (owner < APFS_MIN_USER_INO_NUM)
		report("Physical extent record", "invalid or reserved id.");
	if (owner >= vsb->v_next_obj_id)
		report("Physical extent record", "free id in use.");

	refcnt = le32_to_cpu(val->refcnt);
	if (!refcnt)
		report("Physical extent record", "should have been deleted.");

	extent = get_extent(cat_cnid(&key->hdr));
	extent->e_refcnt = refcnt;

	vsb->v_block_count += length;
	container_bmap_mark_as_used(extent->e_bno, length);

	return extent->e_bno + length - 1;
}
