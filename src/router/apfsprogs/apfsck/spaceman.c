/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <apfs/raw.h>
#include "apfsck.h"
#include "btree.h"
#include "key.h"
#include "object.h"
#include "spaceman.h"
#include "super.h"

/**
 * parse_spaceman_chunk_counts - Parse spaceman fields for chunk-related counts
 * @raw: pointer to the raw spaceman structure
 *
 * Checks the counts of blocks per chunk, chunks per cib, and cibs per cab, and
 * reads them into the in-memory container superblock.  Also calculates the
 * total number of chunks and cibs in the container.
 */
static void parse_spaceman_chunk_counts(struct apfs_spaceman_phys *raw)
{
	struct spaceman *sm = &sb->s_spaceman;
	int chunk_info_size = sizeof(struct apfs_chunk_info);
	int cib_size = sizeof(struct apfs_chunk_info_block);
	int cab_size = sizeof(struct apfs_cib_addr_block);

	sm->sm_blocks_per_chunk = le32_to_cpu(raw->sm_blocks_per_chunk);
	if (sm->sm_blocks_per_chunk != 8 * sb->s_blocksize)
		/* One bitmap block for each chunk */
		report("Space manager", "wrong count of blocks per chunk.");

	sm->sm_chunks_per_cib = (sb->s_blocksize - cib_size) / chunk_info_size;
	if (le32_to_cpu(raw->sm_chunks_per_cib) != sm->sm_chunks_per_cib)
		report("Space manager", "wrong count of chunks per cib.");

	sm->sm_cibs_per_cab = (sb->s_blocksize - cab_size) / sizeof(__le64);
	if (le32_to_cpu(raw->sm_cibs_per_cab) != sm->sm_cibs_per_cab)
		report("Space manager", "wrong count of cibs per cab.");

	sm->sm_chunk_count = DIV_ROUND_UP(sb->s_block_count,
					  sm->sm_blocks_per_chunk);
	sm->sm_cib_count = DIV_ROUND_UP(sm->sm_chunk_count,
					sm->sm_chunks_per_cib);
}

/**
 * read_chunk_bitmap - Read a chunk's bitmap into memory
 * @addr: first block number for the chunk
 * @bmap: block number for the chunk's bitmap, or zero if the chunk is all free
 *
 * Returns a pointer to the chunk's bitmap, read into its proper position
 * within the in-memory bitmap for the container.
 */
static void *read_chunk_bitmap(u64 addr, u64 bmap)
{
	struct spaceman *sm = &sb->s_spaceman;
	ssize_t read_bytes;
	void *buf, *ret;
	size_t count;
	off_t offset;
	u32 chunk_number;

	assert(sm->sm_bitmap);

	/* Prevent out-of-bounds writes to sm->sm_bitmap */
	if (addr & (sm->sm_blocks_per_chunk - 1))
		report("Chunk-info", "chunk address isn't multiple of size.");
	chunk_number = addr / sm->sm_blocks_per_chunk;
	if (addr >= sb->s_block_count)
		report("Chunk-info", "chunk address is out of bounds.");

	ret = buf = sm->sm_bitmap + chunk_number * sb->s_blocksize;
	if (!bmap) /* The whole chunk is free, so leave this block as zero */
		return ret;

	count = sb->s_blocksize;
	offset = bmap * sb->s_blocksize;
	do {
		read_bytes = pread(fd, buf, count, offset);
		if (read_bytes < 0)
			system_error();
		buf += read_bytes;
		count -= read_bytes;
		offset += read_bytes;
	} while (read_bytes > 0);

	/* Mark the bitmap block as used in the actual allocation bitmap */
	container_bmap_mark_as_used(bmap, 1 /* length */);
	return ret;
}

/**
 * count_chunk_free - Count the free blocks in a chunk
 * @bmap: pointer to the chunk's bitmap
 * @blks: number of blocks in the chunk
 */
static int count_chunk_free(void *bmap, u32 blks)
{
	unsigned long long *curr, *end;
	int free = blks;

	end = bmap + sb->s_blocksize;
	for (curr = bmap; curr < end; ++curr)
		free -= __builtin_popcountll(*curr);
	return free;
}

/**
 * parse_chunk_info - Parse and check a chunk info structure
 * @chunk:	pointer to the raw chunk info structure
 * @is_last:	is this the last chunk of the device?
 * @start:	expected first block number for the chunk
 * @xid:	on return, the transaction id of the chunk
 *
 * Returns the first block number for the next chunk.
 */
static u64 parse_chunk_info(struct apfs_chunk_info *chunk, bool is_last,
			    u64 start, u64 *xid)
{
	struct spaceman *sm = &sb->s_spaceman;
	u32 block_count;
	void *bitmap;
	u32 free_count;

	block_count = le32_to_cpu(chunk->ci_block_count);
	if (!block_count)
		report("Chunk-info", "has no blocks.");
	if (block_count > sm->sm_blocks_per_chunk)
		report("Chunk-info", "too many blocks.");
	if (!is_last && block_count != sm->sm_blocks_per_chunk)
		report("Chunk-info", "too few blocks.");
	sm->sm_blocks += block_count;

	if (le64_to_cpu(chunk->ci_addr) != start)
		report("Chunk-info block", "chunks are not consecutive.");
	bitmap = read_chunk_bitmap(start, le64_to_cpu(chunk->ci_bitmap_addr));

	free_count = le32_to_cpu(chunk->ci_free_count);
	if (free_count != count_chunk_free(bitmap, block_count))
		report("Chunk-info", "wrong count of free blocks.");
	sm->sm_free += free_count;

	*xid = le64_to_cpu(chunk->ci_xid);
	if (!*xid)
		report("Chunk-info", "bad transaction id.");
	return start + block_count;
}

/**
 * parse_chunk_info_block - Parse and check a chunk-info block
 * @bno:	block number of the chunk-info block
 * @index:	index of the chunk-info block
 * @start:	expected first block number for the first chunk
 *
 * Returns the first block number for the first chunk of the next cib.
 */
static u64 parse_chunk_info_block(u64 bno, int index, u64 start)
{
	struct spaceman *sm = &sb->s_spaceman;
	struct object obj;
	struct apfs_chunk_info_block *cib;
	u32 chunk_count;
	bool last_cib = index == sm->sm_cib_count - 1;
	u64 max_chunk_xid = 0;
	int i;

	cib = read_object(bno, NULL, &obj);
	if (obj.type != APFS_OBJECT_TYPE_SPACEMAN_CIB)
		report("Chunk-info block", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Chunk-info block", "wrong object subtype.");
	if (obj.xid > sm->sm_xid) /* Cib address is stored in the spaceman */
		report("Chunk-info block", "xid is more recent than spaceman.");

	if (le32_to_cpu(cib->cib_index) != index)
		report("Chunk-info block", "wrong index.");

	chunk_count = le32_to_cpu(cib->cib_chunk_info_count);
	if (!chunk_count)
		report("Chunk-info block", "has no chunks.");
	if (chunk_count > sm->sm_chunks_per_cib)
		report("Chunk-info block", "too many chunks.");
	if (!last_cib && chunk_count != sm->sm_chunks_per_cib)
		report("Chunk-info block", "too few chunks.");
	sm->sm_chunks += chunk_count;

	for (i = 0; i < chunk_count; ++i) {
		bool last_block = false;
		u64 chunk_xid;

		if (last_cib && i == chunk_count - 1)
			last_block = true;
		start = parse_chunk_info(&cib->cib_chunk_info[i], last_block,
					 start, &chunk_xid);

		if (chunk_xid > obj.xid)
			report("Chunk-info", "xid is too recent.");
		if (chunk_xid > max_chunk_xid)
			max_chunk_xid = chunk_xid;
	}
	if (obj.xid != max_chunk_xid) /* Cib only changes if a chunk changes */
		report("Chunk-info block", "xid is too recent.");

	munmap(cib, sb->s_blocksize);
	return start;
}

/**
 * spaceman_val_from_off - Get the value stored on a given spaceman offset
 * @raw:	pointer to the raw space manager
 * @offset:	offset of the value in @raw
 *
 * This is not in the official documentation and I didn't figure it out myself.
 * Credit should go to Joachim Metz: <https://github.com/libyal/libfsapfs>.
 *
 * TODO: check that no values found by this function overlap with each other.
 */
static u64 spaceman_val_from_off(struct apfs_spaceman_phys *raw, u32 offset)
{
	struct spaceman *sm = &sb->s_spaceman;
	char *value_p = (char *)raw + offset;

	assert(sm->sm_struct_size);

	if (offset & 0x7)
		report("Spaceman", "offset is not aligned to 8 bytes.");
	if (offset < sm->sm_struct_size)
		report("Spaceman", "offset overlaps with structure.");
	if (offset >= sb->s_blocksize || offset + sizeof(u64) > sb->s_blocksize)
		report("Spaceman", "offset is out of bounds.");
	return *((u64 *)value_p);
}

/**
 * parse_spaceman_main_device - Parse and check the spaceman main device struct
 * @raw: pointer to the raw space manager
 */
static void parse_spaceman_main_device(struct apfs_spaceman_phys *raw)
{
	struct spaceman *sm = &sb->s_spaceman;
	struct apfs_spaceman_device *dev = &raw->sm_dev[APFS_SD_MAIN];
	u32 addr_off;
	u64 start = 0;
	int i;

	if (dev->sm_cab_count)
		report_unknown("Chunk-info address block");
	if (le32_to_cpu(dev->sm_cib_count) != sm->sm_cib_count)
		report("Spaceman device", "wrong count of chunk-info blocks.");
	if (le64_to_cpu(dev->sm_chunk_count) != sm->sm_chunk_count)
		report("Spaceman device", "wrong count of chunks.");
	if (le64_to_cpu(dev->sm_block_count) != sb->s_block_count)
		report("Spaceman device", "wrong block count.");

	addr_off = le32_to_cpu(dev->sm_addr_offset);
	for (i = 0; i < sm->sm_cib_count; ++i) {
		u64 bno = spaceman_val_from_off(raw,
						addr_off + i * sizeof(u64));

		start = parse_chunk_info_block(bno, i, start);
	}

	if (sm->sm_chunk_count != sm->sm_chunks)
		report("Spaceman device", "bad total number of chunks.");
	if (sb->s_block_count != sm->sm_blocks)
		report("Spaceman device", "bad total number of blocks.");
	if (le64_to_cpu(dev->sm_free_count) != sm->sm_free)
		report("Spaceman device", "bad total number of free blocks.");

	if (dev->sm_reserved || dev->sm_reserved2)
		report("Spaceman device", "non-zero padding.");
}

/**
 * check_spaceman_tier2_device - Check that the second-tier device is empty
 * @raw: pointer to the raw space manager
 */
static void check_spaceman_tier2_device(struct apfs_spaceman_phys *raw)
{
	struct spaceman *sm = &sb->s_spaceman;
	struct apfs_spaceman_device *main_dev = &raw->sm_dev[APFS_SD_MAIN];
	struct apfs_spaceman_device *dev = &raw->sm_dev[APFS_SD_TIER2];
	u32 addr_off, main_addr_off;

	addr_off = le32_to_cpu(dev->sm_addr_offset);
	main_addr_off = le32_to_cpu(main_dev->sm_addr_offset);
	if (addr_off != main_addr_off + sm->sm_cib_count * sizeof(u64))
		report("Spaceman device", "not consecutive address offsets.");
	if (spaceman_val_from_off(raw, addr_off)) /* Empty device has no cib */
		report_unknown("Fusion drive");

	if (dev->sm_block_count || dev->sm_chunk_count || dev->sm_cib_count ||
	    dev->sm_cab_count || dev->sm_free_count)
		report_unknown("Fusion drive");
	if (dev->sm_reserved || dev->sm_reserved2)
		report("Spaceman device", "non-zero padding.");
}

/**
 * check_allocation_boundaries - Check the boundaries of an allocation zone
 * @azb: pointer to the raw boundaries structure
 * @dev: index for the device
 *
 * Allocation zones are undocumented, so we can't do much more than report them
 * as unsupported if they are in use.
 */
static void check_allocation_boundaries(
		struct apfs_spaceman_allocation_zone_boundaries *azb, int dev)
{
	if (!azb->saz_zone_start && !azb->saz_zone_end)
		return;

	if (dev == APFS_SD_MAIN)
		report_unknown("Allocation zones");
	else
		report_unknown("Fusion drive");
}

/**
 * check_spaceman_datazone - Check the spaceman allocation zones
 * @dz: pointer to the raw datazone structure
 *
 * Allocation zones are undocumented, so we can't do much more than report them
 * as unsupported if they are in use.
 */
static void check_spaceman_datazone(struct apfs_spaceman_datazone_info_phys *dz)
{
	int i, dev;

	for (dev = 0; dev < APFS_SD_COUNT; ++dev) {
		for (i = 0; i < APFS_SM_DATAZONE_ALLOCZONE_COUNT; ++i) {
			struct apfs_spaceman_allocation_zone_info_phys *az;
			struct apfs_spaceman_allocation_zone_boundaries *azb;
			int j;

			az = &dz->sdz_allocation_zones[dev][i];
			if (az->saz_zone_id || az->saz_previous_boundary_index)
				report_unknown("Allocation zones");
			if (az->saz_reserved)
				report("Datazone", "reserved field in use.");

			azb = &az->saz_current_boundaries;
			check_allocation_boundaries(azb, dev);
			for (j = 0;
			     j < APFS_SM_ALLOCZONE_NUM_PREVIOUS_BOUNDARIES;
			     ++j) {
				azb = &az->saz_previous_boundaries[j];
				check_allocation_boundaries(azb, dev);
			}
		}
	}
}

/**
 * check_spaceman_free_queues - Check the spaceman free queues
 * @sfq: pointer to the raw free queue array
 */
static void check_spaceman_free_queues(struct apfs_spaceman_free_queue *sfq)
{
	struct spaceman *sm = &sb->s_spaceman;
	int i;

	if (sfq[APFS_SFQ_TIER2].sfq_count || sfq[APFS_SFQ_TIER2].sfq_tree_oid ||
	    sfq[APFS_SFQ_TIER2].sfq_oldest_xid ||
	    sfq[APFS_SFQ_TIER2].sfq_tree_node_limit)
		report_unknown("Fusion drive");

	for (i = 0; i < APFS_SFQ_COUNT; ++i) {
		if (sfq[i].sfq_pad16 || sfq[i].sfq_pad32)
			report("Spaceman free queue", "non-zero padding.");
		if (sfq[i].sfq_reserved)
			report("Spaceman free queue", "reserved field in use.");
	}

	sm->sm_ip_fq = parse_free_queue_btree(
				le64_to_cpu(sfq[APFS_SFQ_IP].sfq_tree_oid));
	if (le64_to_cpu(sfq[APFS_SFQ_IP].sfq_count) != sm->sm_ip_fq->sfq_count)
		report("Spaceman free queue", "wrong block count.");
	if (le64_to_cpu(sfq[APFS_SFQ_IP].sfq_oldest_xid) !=
					sm->sm_ip_fq->sfq_oldest_xid)
		report("Spaceman free queue", "oldest xid is wrong.");
	if (le16_to_cpu(sfq[APFS_SFQ_IP].sfq_tree_node_limit) <
					sm->sm_ip_fq->sfq_btree.node_count)
		report("Spaceman free queue", "node count above limit.");

	sm->sm_main_fq = parse_free_queue_btree(
				le64_to_cpu(sfq[APFS_SFQ_MAIN].sfq_tree_oid));
	if (le64_to_cpu(sfq[APFS_SFQ_MAIN].sfq_count) !=
					sm->sm_main_fq->sfq_count)
		report("Spaceman free queue", "wrong block count.");
	if (le64_to_cpu(sfq[APFS_SFQ_MAIN].sfq_oldest_xid) !=
					sm->sm_main_fq->sfq_oldest_xid)
		report("Spaceman free queue", "oldest xid is wrong.");
	if (le16_to_cpu(sfq[APFS_SFQ_MAIN].sfq_tree_node_limit) <
					sm->sm_main_fq->sfq_btree.node_count)
		report("Spaceman free queue", "node count above limit.");
}

/**
 * compare_container_bitmaps - Verify the container's allocation bitmap
 * @sm_bmap:	allocation bitmap reported by the space manager
 * @real_bmap:	allocation bitmap assembled by the fsck
 * @chunks:	container chunk count, i.e., block count for the bitmaps
 */
static void compare_container_bitmaps(u64 *sm_bmap, u64 *real_bmap, u64 chunks)
{
	unsigned long long bmap_size = sb->s_blocksize * chunks;
	unsigned long long count64;
	u64 i;

	/*
	 * TODO: sometimes the bitmaps don't match; maybe this has something to
	 * do with the file count issue mentioned at check_container()?
	 */
	if (!memcmp(sm_bmap, real_bmap, bmap_size))
		return;
	report_weird("Container allocation bitmap");

	/* At least verify that all used blocks are marked as such */
	count64 = bmap_size / sizeof(count64);
	for (i = 0; i < count64; ++i)
		if ((sm_bmap[i] | real_bmap[i]) != sm_bmap[i])
			report("Space manager", "bad allocation bitmap.");
}

/**
 * container_bmap_mark_as_used - Mark a range as used in the allocation bitmap
 * @paddr:	first block number
 * @length:	block count
 *
 * Checks that the given address range is still marked as free in the
 * container's allocation bitmap, and then switches those bits.
 */
void container_bmap_mark_as_used(u64 paddr, u64 length)
{
	u64 *bitmap = sb->s_bitmap;
	u64 *byte;
	u64 flag;
	u64 i;

	/* Avoid out-of-bounds writes to the allocation bitmap */
	if (paddr + length >= sb->s_block_count || paddr + length < paddr)
		report(NULL /* context */, "Out-of-range block number.");

	for (i = paddr; i < paddr + length; ++i) {
		byte = bitmap + i / 64;
		flag = 1ULL << i % 64;
		if (*byte & flag)
			report(NULL /* context */, "A block is used twice.");
		*byte |= flag;
	}
}

/**
 * parse_ip_bitmap_list - Check consistency of the internal pool bitmap list
 * @raw: pointer to the raw space manager
 *
 * Returns the block number for the current bitmap.
 */
static u64 parse_ip_bitmap_list(struct apfs_spaceman_phys *raw)
{
	u64 bmap_base = le64_to_cpu(raw->sm_ip_bm_base);
	u64 bmap_off;
	u32 bmap_length = le32_to_cpu(raw->sm_ip_bm_block_count);
	u16 free_head, free_tail, free_length;

	/*
	 * So far all internal pool bitmaps encountered had only one block; the
	 * bitmap area is larger than that because it keeps some old versions.
	 */
	bmap_off = spaceman_val_from_off(raw,
					 le32_to_cpu(raw->sm_ip_bitmap_offset));
	if (bmap_off >= bmap_length)
		report("Internal pool", "bitmap block is out-of-bounds.");
	if (le32_to_cpu(raw->sm_ip_bm_size_in_blocks) != 1)
		report_unknown("Multiblock bitmap in internal pool");

	/* The head and tail fit in 16-bit fields, so the length also should */
	if (bmap_length > (u16)(~0U))
		report("Internal pool", "bitmap list is too long.");

	free_head = le16_to_cpu(raw->sm_ip_bm_free_head);
	free_tail = le16_to_cpu(raw->sm_ip_bm_free_tail);
	free_length = (bmap_length + free_tail + 1 - free_head) % bmap_length;

	if (free_head >= bmap_length || free_tail >= bmap_length)
		report("Internal pool", "free bitmaps are out-of-bounds.");
	if ((bmap_length + bmap_off - free_head) % bmap_length < free_length)
		report("Internal pool", "current bitmap listed as free.");
	if (free_length != bmap_length - 1)
		report_unknown("Multiple internal pool bitmaps in use");

	container_bmap_mark_as_used(bmap_base, bmap_length);
	return bmap_base + bmap_off;
}

/**
 * check_ip_bitmap_blocks - Check that the bitmap blocks are properly zeroed
 * @raw: pointer to the raw space manager
 *
 * For most internal pool bitmap blocks this is the only check needed; the
 * current one also needs to be compared against the actual allocation bitmap.
 */
static void check_ip_bitmap_blocks(struct apfs_spaceman_phys *raw)
{
	u64 bmap_base = le64_to_cpu(raw->sm_ip_bm_base);
	u32 bmap_length = le32_to_cpu(raw->sm_ip_bm_block_count);
	u64 pool_blocks = le64_to_cpu(raw->sm_ip_block_count);
	int i;

	for (i = 0; i < bmap_length; ++i) {
		char *bmap;
		int edge, j;

		bmap = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE,
			    fd, (bmap_base + i) * sb->s_blocksize);
		if (bmap == MAP_FAILED)
			system_error();

		/*
		 * The edge is the last byte inside the allocation bitmap;
		 * everything that comes afterwards must be zeroed.
		 */
		edge = pool_blocks / 8;
		for (j = pool_blocks % 8; j < 8; ++j) {
			u8 flag = 1 << j;

			if (bmap[edge] & flag)
				report("Internal pool", "non-zeroed bitmap.");
		}
		for (j = edge + 1; j < sb->s_blocksize; ++j) {
			if (bmap[j])
				report("Internal pool", "non-zeroed bitmap.");
		}

		munmap(bmap, sb->s_blocksize);
	}
}

/**
 * check_internal_pool - Check the internal pool of blocks
 * @raw:	pointer to the raw space manager
 * @real_bmap:	container allocation bitmap assembled by the fsck
 */
static void check_internal_pool(struct apfs_spaceman_phys *raw, u64 *real_bmap)
{
	u64 *pool_bmap;
	u64 pool_base = le64_to_cpu(raw->sm_ip_base);
	u64 pool_blocks = le64_to_cpu(raw->sm_ip_block_count);
	u64 xid, free_next;
	u64 i;

	pool_bmap = mmap(NULL, sb->s_blocksize, PROT_READ, MAP_PRIVATE,
			 fd, parse_ip_bitmap_list(raw) * sb->s_blocksize);
	if (pool_bmap == MAP_FAILED)
		system_error();

	for (i = 0; i < pool_blocks; ++i) {
		u64 bno = pool_base + i;
		u64 real_index = bno / 64;
		u64 real_flag = 1ULL << bno % 64;
		u64 pool_index = i / 64;
		u64 pool_flag = 1ULL << i % 64;

		if ((bool)(pool_bmap[pool_index] & pool_flag) !=
		    (bool)(real_bmap[real_index] & real_flag))
			report("Internal pool", "bad allocation bitmap.");

		/* In the container bitmap, the whole pool is marked as used */
		real_bmap[real_index] |= real_flag;
	}

	munmap(pool_bmap, sb->s_blocksize);

	if (le32_to_cpu(raw->sm_ip_bm_tx_multiplier) !=
					APFS_SPACEMAN_IP_BM_TX_MULTIPLIER)
		report("Space manager", "bad tx multiplier for internal pool.");

	xid = spaceman_val_from_off(raw, le32_to_cpu(raw->sm_ip_bm_xid_offset));
	if (xid > sb->s_xid)
		report("Internal pool", "bad transaction id.");

	/* TODO: actually figure out the sm_ip_bm_free_next_offset field */
	free_next = spaceman_val_from_off(raw,
				le32_to_cpu(raw->sm_ip_bm_free_next_offset));
	if (free_next != 0x0004000300020001)
		report_weird("Free next field of the space manager");

	check_ip_bitmap_blocks(raw);
}

/**
 * check_spaceman - Check the space manager structures for a container
 * @oid: ephemeral object id for the spaceman structure
 */
void check_spaceman(u64 oid)
{
	struct spaceman *sm = &sb->s_spaceman;
	struct object obj;
	struct apfs_spaceman_phys *raw;
	u32 flags;

	raw = read_ephemeral_object(oid, &obj);
	if (obj.type != APFS_OBJECT_TYPE_SPACEMAN)
		report("Space manager", "wrong object type.");
	if (obj.subtype != APFS_OBJECT_TYPE_INVALID)
		report("Space manager", "wrong object subtype.");
	sm->sm_xid = obj.xid;

	flags = le32_to_cpu(raw->sm_flags);
	if ((flags & APFS_SM_FLAGS_VALID_MASK) != flags)
		report("Space manager", "invalid flag in use.");
	if (flags & APFS_SM_FLAG_VERSIONED) {
		sm->sm_struct_size = le32_to_cpu(raw->sm_struct_size);
		if (sm->sm_struct_size != sizeof(*raw))
			report("Space manager", "wrong reported struct size.");
		check_spaceman_datazone(&raw->sm_datazone);
	} else {
		/* Some fields are missing in the non-versioned structure */
		sm->sm_struct_size = sizeof(*raw) - sizeof(raw->sm_datazone) -
				     sizeof(raw->sm_struct_size) -
				     sizeof(raw->sm_version);
	}

	if (le32_to_cpu(raw->sm_block_size) != sb->s_blocksize)
		report("Space manager", "wrong block size.");
	parse_spaceman_chunk_counts(raw);

	/* All bitmaps will need to be read into memory */
	sm->sm_bitmap = calloc(sm->sm_chunk_count, sb->s_blocksize);
	if (!sm->sm_bitmap)
		system_error();

	parse_spaceman_main_device(raw);
	check_spaceman_tier2_device(raw);
	check_spaceman_free_queues(raw->sm_fq);
	check_internal_pool(raw, sb->s_bitmap);

	if (raw->sm_fs_reserve_block_count || raw->sm_fs_reserve_alloc_count)
		report_unknown("Reserved allocation blocks");

	compare_container_bitmaps(sm->sm_bitmap, sb->s_bitmap,
				  sm->sm_chunk_count);
	munmap(raw, sb->s_blocksize);
}

/**
 * parse_free_queue_record - Parse a free queue record and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 * @btree:	the free queue btree structure
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_free_queue_record(struct apfs_spaceman_free_queue_key *key,
			     void *val, int len, struct btree *btree)
{
	struct free_queue *sfq = (struct free_queue *)btree;
	u64 length, xid;

	if (!len) {
		length = 1; /* Ghost records are for one block long extents */
	} else if (len == 8) {
		__le64 *val64 = (__le64 *)val;

		length = le64_to_cpu(*val64);
		if (!length)
			report("Free queue record", "length is zero.");
		if (length == 1)
			report("Free queue record", "value is unnecessary.");
	} else {
		report("Free queue record", "wrong size of value.");
	}
	sfq->sfq_count += length;

	xid = le64_to_cpu(key->sfqk_xid);
	if (xid > sb->s_xid)
		report("Free queue record", "bad transaction id.");
	if (!sfq->sfq_oldest_xid || xid < sfq->sfq_oldest_xid)
		sfq->sfq_oldest_xid = xid;

	/*
	 * These blocks are free, but still not marked as such.  The point
	 * seems to be the preservation of recent checkpoints.
	 */
	container_bmap_mark_as_used(le64_to_cpu(key->sfqk_paddr), length);
}
