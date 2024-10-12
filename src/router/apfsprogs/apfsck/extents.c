/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <apfs/raw.h>
#include <apfs/sha256.h>
#include "apfsck.h"
#include "btree.h"
#include "compress.h"
#include "extents.h"
#include "htable.h"
#include "inode.h"
#include "key.h"
#include "super.h"

/**
 * check_extent - Perform a final check on an extent structure
 * @extent: the extent to check
 */
static void check_extent(struct htable_entry *entry)
{
	struct extent *extent = (struct extent *)entry;

	if (extent->e_refcnt != extent->e_references)
		report("Physical extent record", "bad reference count.");

	/* Reset the real reference count, for the next snapshot */
	extent->e_references = 0;
}

/**
 * free_extent - Free an extent structure after performing a final check
 * @entry: the entry to free
 */
static void free_extent(struct htable_entry *entry)
{
	check_extent(entry);
	free(entry);
}

/**
 * free_extent_table - Free the extent hash table and all its entries
 * @table: table to free
 */
void free_extent_table(struct htable_entry **table)
{
	if (vsb->v_in_snapshot)
		report("Physical extent record", "BUG!");
	free_htable(table, free_extent);
}

/**
 * check_and_reset_extent_table - Checks the extents and resets their references
 * @table: table to check
 */
void check_and_reset_extent_table(struct htable_entry **table)
{
	if (!vsb->v_in_snapshot)
		report("Physical extent record", "BUG!");
	apply_on_htable(table, check_extent);
}

/**
 * get_extent - Find or create an extent structure in the extent hash table
 * @bno: first physical block of the extent
 *
 * Returns the extent structure, after creating it if necessary.
 */
static struct extent *get_extent(u64 bno)
{
	struct htable_entry *entry = NULL;

	entry = get_htable_entry(bno, sizeof(struct extent),
				 vsb->v_extent_table);
	return (struct extent *)entry;
}

/**
 * extent_exists - Check if an extent exists in the extent hash table
 * @bno: first physical block of the extent
 */
static bool extent_exists(u64 bno)
{
	return htable_entry_exists(bno, vsb->v_extent_table);
}

/**
 * get_subextent - Break up an extent in the hash table, and return a part
 * @whole_bno:	base of the whole extent (only a lower bound, really)
 * @part_bno:	base of the extent to extract
 * @part_bkcnt:	maximum block count for the extent to extract
 */
static struct extent *get_subextent(u64 whole_bno, u64 part_bno, u64 part_blkcnt)
{
	struct extent *whole_extent = NULL, *part_extent = NULL, *tail_extent = NULL, *head_extent = NULL;
	u64 whole_blkcnt;
	u32 whole_refcnt;

	if (whole_bno > part_bno)
		report("Physical extent record", "BUG!");

	/*
	 * The whole extent base was probably found from the physical extent
	 * b-trees, but our in-memory extent may have been split by previous
	 * reference updates.
	 */
	while (whole_bno <= part_bno) {
		if (!extent_exists(whole_bno)) {
			++whole_bno;
			continue;
		}
		whole_extent = get_extent(whole_bno);
		whole_blkcnt = whole_extent->e_blocks;
		if (whole_blkcnt == 0)
			report("Physical extent record", "has no blocks?");
		if (whole_bno + whole_blkcnt < whole_bno)
			report("Physical extent record", "overflow?");
		if (whole_bno + whole_blkcnt > part_bno)
			break;
		whole_bno += whole_blkcnt;
	}
	if (whole_bno > part_bno)
		report("Physical extent record", "update for nonexisting extent.");
	whole_refcnt = whole_extent->e_refcnt;

	/*
	 * We make the subextent as big as possible, but the caller may have
	 * to continue breaking up the next extent.
	 */
	part_blkcnt = MIN(part_blkcnt, whole_blkcnt - (part_bno - whole_bno));

	if (whole_bno == part_bno) {
		if (whole_extent->e_blocks == part_blkcnt)
			return whole_extent;

		part_extent = whole_extent;
		whole_extent = NULL;
		part_extent->e_blocks = part_blkcnt;

		tail_extent = get_extent(whole_bno + part_blkcnt);
		if (tail_extent->e_old_entry)
			report("Physical extent record", "weird overlapping extents.");
		tail_extent->e_old_entry = true;
		tail_extent->e_blocks = whole_blkcnt - part_blkcnt;
		tail_extent->e_refcnt = whole_refcnt;

		return part_extent;
	}

	head_extent = whole_extent;
	whole_extent = NULL;
	head_extent->e_blocks = part_bno - whole_bno;

	part_extent = get_extent(part_bno);
	if (part_extent->e_old_entry)
		report("Physical extent record", "weird overlapping extents.");
	part_extent->e_old_entry = true;
	part_extent->e_blocks = part_blkcnt;
	part_extent->e_refcnt = whole_refcnt;

	if (head_extent->e_blocks + part_extent->e_blocks < whole_blkcnt) {
		tail_extent = get_extent(part_bno + part_blkcnt);
		if (tail_extent->e_old_entry)
			report("Physical extent record", "weird overlapping extents.");
		tail_extent->e_old_entry = true;
		tail_extent->e_blocks = whole_blkcnt - (head_extent->e_blocks + part_extent->e_blocks);
		tail_extent->e_refcnt = whole_refcnt;
	}

	return part_extent;
}

/**
 * change_extent_refcnts - Change the refcnts for all known extents in a range
 * @bno:	base of the range
 * @bkcnt:	block count for the range
 * @refdiff:	refcnt change to apply
 */
static void change_extent_refcnts(u64 bno, u64 blkcnt, int32_t refdiff)
{
	while (blkcnt > 0) {
		struct extref_record rec = {0};
		struct extent *extent = NULL;

		extentref_update_lookup(bno, &rec);
		extent = get_subextent(rec.phys_addr, bno, blkcnt);
		extent->e_refcnt += refdiff;

		blkcnt -= extent->e_blocks;
		bno += extent->e_blocks;
	}
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
		if (dstream->d_sparse_bytes != 0) {
			/*
			 * I'm not actually sure about this, but let's leave a
			 * check and see if it happens anywhere.
			 */
			report("Data stream", "xattrs can't have holes.");
		}
	} else {
		if (!dstream->d_seen)
			report("Data stream", "missing reference count.");
		if (dstream->d_refcnt != dstream->d_references)
			report("Data stream", "bad reference count.");
	}

	/* Orphan inodes can have missing extents */
	if (dstream->d_orphan) {
		if (dstream->d_size > dstream->d_alloced_size)
			report("Orphan dstream", "reported sizes make no sense.");
		if (dstream->d_bytes != 0 && dstream->d_logic_start + dstream->d_bytes != dstream->d_alloced_size)
			report_weird("Orphan dstream");
	} else {
		if (dstream->d_logic_start != 0)
			report("Data stream", "missing leading extents.");
		if (dstream->d_bytes < dstream->d_size)
			report("Data stream", "some extents are missing.");
		if (dstream->d_bytes != dstream->d_alloced_size)
			report("Data stream", "wrong allocated space.");
	}
}

static void verify_compressed_dstream_info_hash(struct listed_hash *info, struct dstream *dstream, struct compress *compress)
{
	SHA256_CTX ctx = {0};
	u8 true_hash[APFS_HASH_CCSHA256_SIZE] = {0};
	u8 *block = NULL;
	loff_t off;
	ssize_t res;
	u16 i;

	block = malloc(sb->s_blocksize);
	if (!block)
		system_error();

	sha256_init(&ctx);

	for (i = 0; i < info->blkcnt; ++i) {
		off = info->addr + i * sb->s_blocksize;

		res = apfs_compress_read(compress, (char *)block, sb->s_blocksize, &off);
		memset(block + res, 0, sb->s_blocksize - res);

		sha256_update(&ctx, block, sb->s_blocksize);
	}

	sha256_final(&ctx, true_hash);

	free(block);
	block = NULL;

	if (memcmp(info->hash, true_hash, APFS_HASH_CCSHA256_SIZE) != 0)
		report("File info record", "incorrect hash of file data.");
}

/**
 * verify_dstream_info_hash - Verify that a a file info hash is correct
 * @info:	information about the hash to check
 * @dstream:	dstream for the file to check
 */
static void verify_dstream_info_hash(struct listed_hash *info, struct dstream *dstream)
{
	SHA256_CTX ctx = {0};
	u8 true_hash[APFS_HASH_CCSHA256_SIZE] = {0};
	u64 bno;
	u8 *block = NULL;
	u16 i;

	sha256_init(&ctx);

	for (i = 0; i < info->blkcnt; ++i) {
		if (fext_tree_lookup(dstream->d_id, info->addr + i * sb->s_blocksize, &bno))
			report("Fext tree", "query failed.");

		if (bno == 0) /* A hole */
			block = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		else
			block = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE, fd, bno * sb->s_blocksize);
		if (block == MAP_FAILED)
			system_error();

		sha256_update(&ctx, block, sb->s_blocksize);

		munmap(block, sb->s_blocksize);
		block = NULL;
	}

	sha256_final(&ctx, true_hash);

	if (memcmp(info->hash, true_hash, APFS_HASH_CCSHA256_SIZE) != 0)
		report("File info record", "incorrect hash of file data.");
}

void verify_dstream_hashes(struct dstream *dstream, struct compress *compress)
{
	struct listed_hash *hash = NULL;

	if (!apfs_volume_is_sealed())
		return;

	if (!dstream)
		report(NULL, "Bug!");

	hash = dstream->d_hashes;

	while (hash) {
		if (compress)
			verify_compressed_dstream_info_hash(hash, dstream, compress);
		else
			verify_dstream_info_hash(hash, dstream);
		dstream->d_hashes = hash->prev;
		free(hash);
		hash = dstream->d_hashes;
	}
}

static void increase_extent_references(u64 bno, u64 blkcnt, u64 owner, u8 owner_type)
{
	while (blkcnt > 0) {
		u64 base_bno = bno;
		struct extent *extent = NULL;
		int limit = 5000;

		/*
		 * Sometimes there is more than one consecutive logical extents
		 * sharing a single physical extent, and sometimes they don't
		 * even belong to the same dstream. No idea why.
		 */
		while (!extent_exists(base_bno)) {
			if (--limit == 0)
				report("Logical extent record", "doesn't seem covered by any physical extent.");
			--base_bno;
		}
		extent = get_subextent(base_bno, bno, blkcnt);

		if (extent->e_references) {
			if (extent->e_obj_type != owner_type)
				report("Physical extent record", "owners have inconsistent types.");
			/* Only count the extent once for each owner */
			if (extent->e_latest_owner != owner)
				extent->e_references++;
		} else {
			extent->e_references++;
		}
		extent->e_obj_type = owner_type;
		extent->e_latest_owner = owner;

		/* An extent may have an unused tail */
		blkcnt -= extent->e_blocks;
		bno += extent->e_blocks;
	}
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
	cnid_set_state_flag(cnid, CNID_IN_DSTREAM);

	/*
	 * Not a real dstream, just a hack for working with inline compressed
	 * files inside sealed volumes. Don't check anything.
	 */
	if(dstream->d_inline)
		return free(entry);

	/* Increase the refcount of each physical extent used by the dstream */
	while (curr_extent) {
		struct listed_extent *next_extent;

		increase_extent_references(curr_extent->paddr, curr_extent->blkcnt, dstream->d_owner, dstream->d_obj_type);

		next_extent = curr_extent->next;
		free(curr_extent);
		curr_extent = next_extent;
	}

	/* TODO: support resource forks for compressed files too */
	if (!dstream->d_xattr)
		verify_dstream_hashes(dstream, NULL);

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
	struct listed_extent **ext_p = NULL;
	struct listed_extent *ext = NULL;
	struct listed_extent *new = NULL;
	u64 paddr_end;

	paddr_end = paddr + blk_count;
	if (paddr_end < paddr) /* Overflow */
		report("Extent record", "physical address is too big.");

	ext_p = &dstream->d_extents;
	ext = *ext_p;

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
	new->blkcnt = blk_count;
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
	u64 length, flags, logical_addr;
	u64 crypid;
	bool prealloc_flagged;

	if (apfs_volume_is_sealed())
		report("Extent record", "shouldn't exist in a sealed volume.");

	if (len != sizeof(*val))
		report("Extent record", "wrong size of value.");

	crypid = le64_to_cpu(val->crypto_id);
	if (crypid && crypid != APFS_CRYPTO_SW_ID)
		++get_crypto_state(crypid)->c_references;

	length = le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;
	if (!length)
		report("Extent record", "length is zero.");
	if (length & (sb->s_blocksize - 1))
		report("Extent record", "length isn't multiple of block size.");
	logical_addr = le64_to_cpu(key->logical_addr);

	flags = (le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_FLAG_MASK) >> APFS_FILE_EXTENT_FLAG_SHIFT;
	if ((flags & APFS_VALID_FILE_EXTENT_FLAGS) != flags)
		report("Extent record", "invalid flag in use.");
	if (flags & APFS_FILE_EXTENT_CRYPTO_FLAG)
		report_unknown("Encrypted extent");
	prealloc_flagged = flags & APFS_FILE_EXTENT_PREALLOCATED;

	dstream = get_dstream(cat_cnid(&key->hdr));
	if (dstream->d_bytes == 0 && logical_addr != 0) {
		/* An orphan may have already lost its leading extents */
		dstream->d_logic_start = logical_addr;
	}
	if (dstream->d_logic_start + dstream->d_bytes != logical_addr)
		report("Data stream", "extents are not consecutive.");
	dstream->d_bytes += length;

	/* Mostly conjecture, but if I'm wrong this will trip on something */
	if (apfs_volume_has_extent_prealloc_flag()) {
		if (prealloc_flagged != (bool)(logical_addr >= dstream->d_size))
			report("Extent record", "bad preallocation flag.");
	} else {
		if (prealloc_flagged)
			report("Extent record", "uses prealloc flag without that feature.");
	}

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
	struct extent *extent = NULL;
	u8 kind;
	u32 refcnt;
	u64 length, owner, paddr;

	if (len != sizeof(*val))
		report("Physical extent record", "wrong size of value.");

	kind = le64_to_cpu(val->len_and_kind) >> APFS_PEXT_KIND_SHIFT;
	if (kind != APFS_KIND_NEW && kind != APFS_KIND_UPDATE)
		report("Physical extent record", "invalid kind");

	length = le64_to_cpu(val->len_and_kind) & APFS_PEXT_LEN_MASK;
	if (!length)
		report("Physical extent record", "has no blocks.");

	/*
	 * If the owner of a physical extent got removed, I would expect this
	 * field to be meaningless.  At least check that the number is in range.
	 */
	owner = le64_to_cpu(val->owning_obj_id);
	if (owner == APFS_OWNING_OBJ_ID_INVALID) {
		if (kind != APFS_KIND_UPDATE)
			report("Physical extent record", "invalid owner id for NEW.");
	} else {
		if (kind != APFS_KIND_NEW)
			report("Physical extent record", "valid owner id for UPDATE.");
		if (owner < APFS_MIN_USER_INO_NUM)
			report("Physical extent record", "reserved id.");
		if (owner >= vsb->v_next_obj_id)
			report("Physical extent record", "free id in use.");
	}

	refcnt = le32_to_cpu(val->refcnt);
	if (!refcnt)
		report("Physical extent record", "should have been deleted.");

	paddr = cat_cnid(&key->hdr);
	if (kind == APFS_KIND_NEW) {
		extent = get_extent(paddr);
		extent->e_blocks = length;
		extent->e_refcnt = refcnt;

		if (extent->e_old_entry)
			report("Physical extent record", "is NEW but has already used blocks.");
		extent->e_old_entry = true;

		vsb->v_block_count += extent->e_blocks;
		container_bmap_mark_as_used(extent->e_bno, extent->e_blocks);
	} else {
		change_extent_refcnts(paddr, length, refcnt);
	}

	return paddr + length - 1;
}

/**
 * free_crypto_state - Free a crypto state after performing some final checks
 * @entry: the entry to free
 */
static void free_crypto_state(struct htable_entry *entry)
{
	struct crypto_state *crypto = (struct crypto_state *)entry;

	/*
	 * It seems that the overprovisioning file may have no wrapped key,
	 * even if it does have a state record.
	 */
	if (crypto->c_keylen == 0 && !crypto->c_overprov)
		report_unknown("Encrypted metadata");

	if (crypto->c_refcnt != crypto->c_references)
		report("Crypto state record", "bad reference count.");

	free(crypto);
}

/**
 * free_crypto_table - Free the crypto state hash table and all its entries
 * @table: table to free
 */
void free_crypto_table(struct htable_entry **table)
{
	free_htable(table, free_crypto_state);
}

/**
 * get_crypto_state - Find or create a crypto state struct in their hash table
 * @id:		id of the crypto state
 *
 * Returns the crypto state structure, after creating it if necessary.
 */
struct crypto_state *get_crypto_state(u64 id)
{
	struct htable_entry *entry;

	entry = get_htable_entry(id, sizeof(struct crypto_state), vsb->v_crypto_table);
	return (struct crypto_state *)entry;
}

/**
 * parse_crypto_state_record - Parse and check a crypto state record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_crypto_state_record(struct apfs_crypto_state_key *key, struct apfs_crypto_state_val *val, int len)
{
	struct apfs_wrapped_crypto_state *wrapped = NULL;
	struct crypto_state *crypto;
	u16 key_len;

	if (!vsb->v_encrypted)
		report("Unencrypted volume", "has crypto state records.");

	if (len < sizeof(*val))
		report("Crypto state record", "value size too small.");
	wrapped = &val->state;

	key_len = le16_to_cpu(wrapped->key_len);
	if (key_len > APFS_CP_MAX_WRAPPEDKEYSIZE)
		report("Crypto state record", "wrapped key is too long.");
	if (len != sizeof(*val) + le16_to_cpu(wrapped->key_len))
		report("Crypto state record", "wrong size of value.");

	if (le16_to_cpu(wrapped->major_version) != APFS_WMCS_MAJOR_VERSION)
		report("Crypto state record", "wrong major version.");
	if (le16_to_cpu(wrapped->minor_version) != APFS_WMCS_MINOR_VERSION)
		report("Crypto state record", "wrong minor version.");
	if (wrapped->cpflags)
		report("Crypto state record", "unknown flag.");
	/* TODO: deal with the protection class */
	if (!wrapped->key_revision)
		report("Crypto state record", "key revision is not set.");

	/*
	 * I don't know how unofficial implementations are supposed to handle
	 * this field, but I'm guessing it shouldn't be zero.
	 */
	if (!wrapped->key_os_version)
		report("Crypto state record", "os version is not set.");

	crypto = get_crypto_state(cat_cnid(&key->hdr));

	switch (crypto->c_id) {
	case 0:
		report("Crypto state record", "null id.");
	case APFS_CRYPTO_SW_ID:
		report("Crypto state record", "id for software encryption.");
	case APFS_CRYPTO_RESERVED_5:
		report("Crypto state record", "reserved crypto id.");
	case APFS_UNASSIGNED_CRYPTO_ID:
		report("Crypto state record", "unassigned crypto id.");
	}

	crypto->c_refcnt = le32_to_cpu(val->refcnt);
	if (!crypto->c_refcnt)
		report("Crypto state record", "has no references.");
	crypto->c_keylen = key_len;
}

static void attach_hash_to_dstream(u64 addr, u64 blkcnt, u8 *hash, struct dstream *dstream)
{
	struct listed_hash *new = NULL;

	new = calloc(1, sizeof(*new));
	if (!new)
		system_error();

	new->addr = addr;
	new->blkcnt = blkcnt;
	memcpy(new->hash, hash, APFS_HASH_CCSHA256_SIZE);

	new->prev = dstream->d_hashes;
	dstream->d_hashes = new;
}

/**
 * parse_file_info_record - Parse an info record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_file_info_record(struct apfs_file_info_key *key, struct apfs_file_info_val *val, int len)
{
	struct apfs_file_data_hash_val *dhash = NULL;
	struct dstream *dstream = NULL;
	u64 paddr;
	u16 blkcnt;

	if (!apfs_volume_is_sealed())
		report("File info record", "volume is unsealed.");

	dhash = &val->dhash;
	if (len < sizeof(*dhash))
		report("File info record", "value is too small.");

	if (dhash->hash_size != APFS_HASH_CCSHA256_SIZE)
		report("File info record", "unusual hash length.");
	if (len != sizeof(*dhash) + dhash->hash_size)
		report("File info record", "wrong size of value.");

	blkcnt = le16_to_cpu(dhash->hashed_len);
	if (!blkcnt)
		report("File info record", "length is zero.");
	paddr = le64_to_cpu(key->info_and_lba) & APFS_FILE_INFO_LBA_MASK;

	dstream = get_dstream(cat_cnid(&key->hdr));
	attach_hash_to_dstream(paddr, blkcnt, dhash->hash, dstream);
}

/**
 * parse_fext_record - Parse a fext record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_fext_record(struct apfs_fext_tree_key *key, struct apfs_fext_tree_val *val, int len)
{
	struct dstream *dstream = NULL;
	u64 length, flags;

	/*
	 * Keys and values must be aligned to eight bytes. TODO: add this
	 * check to aligned trees of all types.
	 */
	if ((u64)key & 7 || (u64)val & 7)
		report("Omap record", "bad alignment for key or value.");

	if (len != sizeof(*val))
		report("Fext record", "wrong size of value.");

	length = le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;
	if (!length)
		report("Fext record", "length is zero.");
	if (length & (sb->s_blocksize - 1))
		report("Fext record", "length isn't multiple of block size.");

	flags = le64_to_cpu(val->len_and_flags) & APFS_FILE_EXTENT_FLAG_MASK;
	if (flags)
		report("Fext record", "no flags should be set.");

	dstream = get_dstream(le64_to_cpu(key->private_id));
	if (dstream->d_bytes != le64_to_cpu(key->logical_addr))
		report("Data stream", "fexts are not consecutive.");
	dstream->d_bytes += length;

	if (!le64_to_cpu(val->phys_block_num)) { /* This is a hole */
		dstream->d_sparse_bytes += length;
		return;
	}
	attach_extent_to_dstream(le64_to_cpu(val->phys_block_num), length >> sb->s_blocksize_bits, dstream);
}
