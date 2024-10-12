/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <apfs/parameters.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "btree.h"
#include "mkapfs.h"
#include "object.h"
#include "spaceman.h"

/* Extra information about the space manager */
static struct spaceman_info {
	u64 chunk_count;
	u32 cib_count;
	u32 cab_count;
	u64 ip_blocks;
	u32 ip_bm_size;
	u32 ip_bmap_blocks;
	u64 ip_base;
	u32 bm_addr_off;	/* Offset of bitmap address in the spaceman */
	u32 bm_free_next_off;	/* Offset of free_next in the spaceman */
	u32 cib_addr_base_off;	/* Offset of the cib/cab address in spaceman */

	u64 used_blocks_end;	/* Block right after the last one we allocate */
	u64 used_chunks_end;	/* Chunk right after the last one we allocate */

	u64 first_chunk_bmap;	/* Block number for the first chunk's bitmap */
	u64 first_cib;		/* Block number for first chunk-info block */
	u64 first_cab;		/* Block number for first cib address block */
} sm_info = {0};

/**
 * blocks_per_chunk - Calculate the number of blocks per chunk
 */
static inline u32 blocks_per_chunk(void)
{
	return 8 * param->blocksize; /* One bitmap block for each chunk */
}

/**
 * chunks_per_cib - Calculate the number of chunks per chunk-info block
 */
static inline u32 chunks_per_cib(void)
{
	int chunk_info_size = sizeof(struct apfs_chunk_info);
	int cib_size = sizeof(struct apfs_chunk_info_block);

	return (param->blocksize - cib_size) / chunk_info_size;
}

/**
 * cibs_per_cab - Calculate the count of chunk-info blocks per cib address block
 */
static inline u32 cibs_per_cab(void)
{
	int cab_size = sizeof(struct apfs_cib_addr_block);

	return (param->blocksize - cab_size) / sizeof(__le64);
}

/**
 * spaceman_size - Calculate the size of the spaceman object in bytes
 */
u32 spaceman_size(void)
{
	u64 chunk_count = DIV_ROUND_UP(param->block_count, blocks_per_chunk());
	u32 cib_count = DIV_ROUND_UP(chunk_count, chunks_per_cib());
	u32 cab_count = DIV_ROUND_UP(cib_count, cibs_per_cab());

	/*
	 * I don't currently expect to deal with filesystems big enough to need
	 * more than one block for cab addresses and ip bitmap metadata: that
	 * would be like 113 TB.
	 */
	if (cab_count > 1)
		return param->blocksize;

	/*
	 * The spaceman must have room for the addresses of all main device
	 * cibs, plus an extra offset for tier 2. Some containers require an
	 * extra block to store this stuff.
	 */
	if ((cib_count + 1) * sizeof(__le64) + sm_info.cib_addr_base_off > param->blocksize)
		return 2 * param->blocksize;
	return param->blocksize;
}

#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

/**
 * count_used_blocks_in_chunk - Calculate number of allocated blocks in a chunk
 * @chunkno:	chunk number to check
 */
static u32 count_used_blocks_in_chunk(u64 chunkno)
{
	u32 first_chunk_ip_blocks;

	if (chunkno >= sm_info.used_chunks_end)
		return 0;

	/* The internal pool may not fit whole in the chunk */
	first_chunk_ip_blocks = MIN(sm_info.ip_blocks, blocks_per_chunk() - sm_info.ip_base);

	if (chunkno == 0) {
		u32 blocks = 0;

		/* This stuff always goes in the first chunk */
		blocks += 1;			/* Block zero */
		blocks += CPOINT_DESC_BLOCKS;	/* Checkpoint descriptor blocks */
		blocks += CPOINT_DATA_BLOCKS;	/* Checkpoint data blocks */
		blocks += 2;			/* Container object map and its root */
		blocks += 6;			/* Volume superblock and its trees */
		blocks += sm_info.ip_bmap_blocks; /* Internal pool bitmap blocks */

		blocks += first_chunk_ip_blocks;
		return blocks;
	}

	/* Later chunks are only needed for the rest of the internal pool */
	if (chunkno != sm_info.used_chunks_end - 1)
		return blocks_per_chunk();

	/* Last chunk */
	return (sm_info.ip_blocks - first_chunk_ip_blocks) % blocks_per_chunk();
}

/**
 * count_used_blocks - Calculate the number of blocks used by the mkfs
 */
static u32 count_used_blocks(void)
{
	u32 blocks = 0;
	u64 chunkno;

	for (chunkno = 0; chunkno < sm_info.used_chunks_end; ++chunkno)
		blocks += count_used_blocks_in_chunk(chunkno);
	return blocks;
}

/**
 * bmap_mark_as_used - Mark a range as used in the allocation bitmap
 * @bitmap:	allocation bitmap for the first chunk
 * @paddr:	first block number
 * @length:	block count
 */
static void bmap_mark_as_used(u64 *bitmap, u64 paddr, u64 length)
{
	u64 *byte;
	u64 flag;
	u64 i;

	for (i = paddr; i < paddr + length; ++i) {
		byte = bitmap + i / 64;
		flag = 1ULL << i % 64;
		*byte |= flag;
	}
}

/**
 * make_alloc_bitmap - Make the allocation bitmap for the first chunk
 */
static void make_alloc_bitmap(void)
{
	void *bmap = get_zeroed_blocks(sm_info.first_chunk_bmap, sm_info.used_chunks_end);

	/* Block zero */
	bmap_mark_as_used(bmap, 0, 1);
	/* Checkpoint descriptor blocks */
	bmap_mark_as_used(bmap, CPOINT_DESC_BASE, CPOINT_DESC_BLOCKS);
	/* Checkpoint data blocks */
	bmap_mark_as_used(bmap, CPOINT_DATA_BASE, CPOINT_DATA_BLOCKS);
	/* Container object map and its root */
	bmap_mark_as_used(bmap, MAIN_OMAP_BNO, 2);
	/* Volume superblock and its trees */
	bmap_mark_as_used(bmap, FIRST_VOL_BNO, 6);
	/* Internal pool bitmap blocks */
	bmap_mark_as_used(bmap, IP_BMAP_BASE, sm_info.ip_bmap_blocks);
	/* Internal pool blocks */
	bmap_mark_as_used(bmap, sm_info.ip_base, sm_info.ip_blocks);

	munmap(bmap, sm_info.used_chunks_end * param->blocksize);
}

/*
 * Offsets into the spaceman block for a non-versioned container; the values
 * have been borrowed from a test image.
 */
#define BITMAP_XID_OFF		0x150	/* Transaction id for the ip bitmap */

/**
 * make_chunk_info - Write a chunk info structure
 * @chunk:	pointer to the raw chunk info structure
 * @start:	first block number for the chunk
 *
 * Returns the first block number for the next chunk.
 */
static u64 make_chunk_info(struct apfs_chunk_info *chunk, u64 start)
{
	u64 remaining_blocks = param->block_count - start;
	u64 chunkno = start / blocks_per_chunk();
	u32 block_count, free_count;

	chunk->ci_xid = cpu_to_le64(MKFS_XID);
	chunk->ci_addr = cpu_to_le64(start);

	/* Later chunks are just holes */
	if (start < sm_info.used_blocks_end)
		chunk->ci_bitmap_addr = cpu_to_le64(sm_info.first_chunk_bmap + chunkno);

	block_count = blocks_per_chunk();
	if (remaining_blocks < block_count) /* This is the final chunk */
		block_count = remaining_blocks;
	chunk->ci_block_count = cpu_to_le32(block_count);

	free_count = block_count - count_used_blocks_in_chunk(chunkno);
	chunk->ci_free_count = cpu_to_le32(free_count);

	start += block_count;
	return start;
}

/**
 * make_chunk_info_block - Make a chunk-info block
 * @bno:	block number for the chunk-info block
 * @index:	index of the chunk-info block
 * @start:	first block number for the first chunk
 *
 * Returns the first block number for the first chunk of the next cib.
 */
static u64 make_chunk_info_block(u64 bno, int index, u64 start)
{
	struct apfs_chunk_info_block *cib = get_zeroed_block(bno);
	int i;

	cib->cib_index = cpu_to_le32(index);
	for (i = 0; i < chunks_per_cib(); ++i) {
		if (start == param->block_count) /* No more chunks in device */
			break;
		start = make_chunk_info(&cib->cib_chunk_info[i], start);
	}
	cib->cib_chunk_info_count = cpu_to_le32(i);

	set_object_header(&cib->cib_o, param->blocksize, bno,
			  APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_SPACEMAN_CIB,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(cib, param->blocksize);

	return start;
}

/**
 * make_cib_addr_block - Make a cib address block
 * @bno:	block number for the chunk-info block
 * @index:	index of the chunk-info block
 * @start:	first block number for the first chunk
 *
 * Returns the first block number for the first chunk of the next cib.
 */
static u64 make_cib_addr_block(u64 bno, int index, u64 start)
{
	struct apfs_cib_addr_block *cab = get_zeroed_block(bno);
	int i;

	cab->cab_index = cpu_to_le32(index);
	for (i = 0; i < cibs_per_cab(); ++i) {
		int cib_index;
		u64 cib_bno;

		if (start == param->block_count) /* No more chunks in device */
			break;

		cib_index = cibs_per_cab() * index + i;
		cib_bno = sm_info.first_cib + cib_index;
		cab->cab_cib_addr[i] = cpu_to_le64(cib_bno);
		start = make_chunk_info_block(cib_bno, cib_index, start);
	}
	cab->cab_cib_count = cpu_to_le32(i);

	set_object_header(&cab->cab_o, param->blocksize, bno,
			  APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_SPACEMAN_CAB,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(cab, param->blocksize);

	return start;
}

/**
 * make_devices - Make the spaceman device structures
 * @sm: pointer to the on-disk spaceman structure
 */
static void make_devices(struct apfs_spaceman_phys *sm)
{
	struct apfs_spaceman_device *dev = &sm->sm_dev[APFS_SD_MAIN];
	u64 chunk_count = DIV_ROUND_UP(param->block_count, blocks_per_chunk());
	u32 cib_count = DIV_ROUND_UP(chunk_count, chunks_per_cib());
	u32 cab_count;
	u64 start = 0;
	int i;

	cab_count = DIV_ROUND_UP(cib_count, cibs_per_cab());
	if (cab_count == 1)
		cab_count = 0;

	/*
	 * We must have room for the addresses of all main device cabs, plus
	 * an extra offset for tier 2.
	 */
	if ((cab_count + 1) * sizeof(__le64) + sm_info.cib_addr_base_off > param->blocksize) {
		printf("Huge containers (> ~113TB) not yet supported.\n");
		exit(1);
	}

	dev->sm_block_count = cpu_to_le64(param->block_count);
	dev->sm_chunk_count = cpu_to_le64(chunk_count);
	dev->sm_cib_count = cpu_to_le32(cib_count);
	dev->sm_cab_count = cpu_to_le32(cab_count);
	dev->sm_free_count = cpu_to_le64(param->block_count -
					 count_used_blocks());

	dev->sm_addr_offset = cpu_to_le32(sm_info.cib_addr_base_off);
	if (!cab_count) {
		__le64 *cib_addr = (void *)sm + sm_info.cib_addr_base_off;
		for (i = 0; i < cib_count; ++i) {
			cib_addr[i] = cpu_to_le64(sm_info.first_cib + i);
			start = make_chunk_info_block(sm_info.first_cib + i, i, start);
		}
	} else {
		__le64 *cab_addr = (void *)sm + sm_info.cib_addr_base_off;
		for (i = 0; i < cab_count; ++i) {
			cab_addr[i] = cpu_to_le64(sm_info.first_cab + i);
			start = make_cib_addr_block(sm_info.first_cab + i, i, start);
		}
	}

	/* For the tier2 device, just set the offset; the address is null */
	dev = &sm->sm_dev[APFS_SD_TIER2];
	if (cab_count)
		dev->sm_addr_offset = cpu_to_le32(sm_info.cib_addr_base_off + cab_count * sizeof(__le64));
	else
		dev->sm_addr_offset = cpu_to_le32(sm_info.cib_addr_base_off + cib_count * sizeof(__le64));
}

/**
 * make_ip_free_queue - Make an empty free queue for the internal pool
 * @fq:	free queue structure
 */
static void make_ip_free_queue(struct apfs_spaceman_free_queue *fq)
{
	fq->sfq_tree_oid = cpu_to_le64(IP_FREE_QUEUE_OID);
	make_empty_btree_root(IP_FREE_QUEUE_BNO, IP_FREE_QUEUE_OID,
			      APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE);
	fq->sfq_oldest_xid = 0;
	fq->sfq_tree_node_limit = cpu_to_le16(ip_fq_node_limit(sm_info.chunk_count));
}

/**
 * make_main_free_queue - Make an empty free queue for the main device
 * @fq:	free queue structure
 */
static void make_main_free_queue(struct apfs_spaceman_free_queue *fq)
{
	fq->sfq_tree_oid = cpu_to_le64(MAIN_FREE_QUEUE_OID);
	make_empty_btree_root(MAIN_FREE_QUEUE_BNO, MAIN_FREE_QUEUE_OID,
			      APFS_OBJECT_TYPE_SPACEMAN_FREE_QUEUE);
	fq->sfq_oldest_xid = 0;
	fq->sfq_tree_node_limit = cpu_to_le16(main_fq_node_limit(param->block_count));
}

/**
 * make_ip_bitmap - Make the allocation bitmap for the internal pool
 */
static void make_ip_bitmap(void)
{
	void *bmap = get_zeroed_blocks(IP_BMAP_BASE, sm_info.ip_bm_size);

	/* Cib address blocks */
	bmap_mark_as_used(bmap, sm_info.first_cab - sm_info.ip_base, sm_info.cab_count);
	/* Chunk-info blocks */
	bmap_mark_as_used(bmap, sm_info.first_cib - sm_info.ip_base, sm_info.cib_count);
	/* Allocation bitmap block */
	bmap_mark_as_used(bmap, sm_info.first_chunk_bmap - sm_info.ip_base, sm_info.used_chunks_end);

	munmap(bmap, param->blocksize);
}

/**
 * make_ip_bm_free_next - Fill the free_next field for the internal pool
 * @addr: pointer to the beginning of the field
 */
static void make_ip_bm_free_next(__le16 *addr)
{
	int i;

	/*
	 * Ip bitmap blocks are marked with numbers 1,2,3,...,ip_bmap_blocks,0
	 * in free_next, except when they are in use: those get overwritten with
	 * 0xFFFF.
	 */
	for (i = 0; i < sm_info.ip_bm_size; ++i)
		addr[i] = cpu_to_le16(0xFFFF);
	for (i = sm_info.ip_bm_size; i < sm_info.ip_bmap_blocks - 1; i++)
		addr[i] = cpu_to_le16(i + 1);
	addr[sm_info.ip_bmap_blocks - 1] = cpu_to_le16(0xFFFF);
}

/**
 * make_internal_pool - Make the internal pool of the space manager
 * @sm: pointer to the on-disk spaceman structure
 */
static void make_internal_pool(struct apfs_spaceman_phys *sm)
{
	int i;
	__le64 *addr;
	__le16 *bm_off_addr;

	sm->sm_ip_bm_tx_multiplier =
				cpu_to_le32(APFS_SPACEMAN_IP_BM_TX_MULTIPLIER);
	sm->sm_ip_block_count = cpu_to_le64(sm_info.ip_blocks);
	sm->sm_ip_base = cpu_to_le64(sm_info.ip_base);
	/* No support for multiblock bitmaps */
	sm->sm_ip_bm_size_in_blocks = cpu_to_le32(sm_info.ip_bm_size);

	sm->sm_ip_bm_block_count = cpu_to_le32(sm_info.ip_bmap_blocks);
	sm->sm_ip_bm_base = cpu_to_le64(IP_BMAP_BASE);
	for (i = 0; i < sm_info.ip_bmap_blocks; ++i) /* We use no blocks from the ip */
		munmap(get_zeroed_block(IP_BMAP_BASE + i), param->blocksize);

	/* The current bitmaps are the first in the ring */
	sm->sm_ip_bitmap_offset = cpu_to_le32(sm_info.bm_addr_off);
	bm_off_addr = (void *)sm + sm_info.bm_addr_off;
	for (i = 0; i < sm_info.ip_bm_size; ++i)
		bm_off_addr[i] = cpu_to_le16(i);
	sm->sm_ip_bm_free_head = cpu_to_le16(sm_info.ip_bm_size);
	sm->sm_ip_bm_free_tail = cpu_to_le16(sm_info.ip_bmap_blocks - 1);

	sm->sm_ip_bm_xid_offset = cpu_to_le32(BITMAP_XID_OFF);
	addr = (void *)sm + BITMAP_XID_OFF;
	for (i = 0; i < sm_info.ip_bm_size; ++i)
		addr[i] = cpu_to_le64(MKFS_XID);

	sm->sm_ip_bm_free_next_offset = cpu_to_le32(sm_info.bm_free_next_off);
	make_ip_bm_free_next((void *)sm + sm_info.bm_free_next_off);

	make_ip_bitmap();
}

/**
 * make_spaceman - Make the space manager for the container
 * @bno: block number to use
 * @oid: object id
 */
void make_spaceman(u64 bno, u64 oid)
{
	struct apfs_spaceman_phys *sm = NULL;

	sm_info.chunk_count = DIV_ROUND_UP(param->block_count, blocks_per_chunk());
	sm_info.cib_count = DIV_ROUND_UP(sm_info.chunk_count, chunks_per_cib());
	sm_info.cab_count = DIV_ROUND_UP(sm_info.cib_count, cibs_per_cab());
	if (sm_info.cab_count == 1)
		sm_info.cab_count = 0;
	sm_info.ip_blocks = (sm_info.chunk_count + sm_info.cib_count + sm_info.cab_count) * 3;

	/*
	 * We have 16 ip bitmaps; each of them maps the whole ip and may span
	 * multiple blocks.
	 */
	sm_info.ip_bm_size = DIV_ROUND_UP(sm_info.ip_blocks, blocks_per_chunk());
	sm_info.ip_bmap_blocks = 16 * sm_info.ip_bm_size;
	sm_info.ip_base = IP_BMAP_BASE + sm_info.ip_bmap_blocks;

	/* We have one xid for each of the ip bitmaps */
	sm_info.bm_addr_off = BITMAP_XID_OFF + sizeof(__le64) * sm_info.ip_bm_size;
	sm_info.bm_free_next_off = sm_info.bm_addr_off + ROUND_UP(sizeof(__le16) * sm_info.ip_bm_size, sizeof(__le64));
	sm_info.cib_addr_base_off = sm_info.bm_free_next_off + sm_info.ip_bmap_blocks * sizeof(__le16);

	/* Only the ip size matters, all other used blocks come before it */
	sm_info.used_blocks_end = sm_info.ip_base + sm_info.ip_blocks;
	sm_info.used_chunks_end = DIV_ROUND_UP(sm_info.used_blocks_end, blocks_per_chunk());

	/*
	 * Put the chunk bitmaps at the beginning of the internal pool, and
	 * the cibs right after them, followed by the cabs if any.
	 */
	sm_info.first_chunk_bmap = sm_info.ip_base;
	sm_info.first_cib = sm_info.first_chunk_bmap + sm_info.used_chunks_end;
	sm_info.first_cab = sm_info.first_cib + sm_info.cib_count;

	sm = get_zeroed_blocks(bno, spaceman_size() / param->blocksize);

	sm->sm_block_size = cpu_to_le32(param->blocksize);
	sm->sm_blocks_per_chunk = cpu_to_le32(blocks_per_chunk());
	sm->sm_chunks_per_cib = cpu_to_le32(chunks_per_cib());
	sm->sm_cibs_per_cab = cpu_to_le32(cibs_per_cab());

	make_devices(sm);
	make_ip_free_queue(&sm->sm_fq[APFS_SFQ_IP]);
	make_main_free_queue(&sm->sm_fq[APFS_SFQ_MAIN]);
	make_internal_pool(sm);
	make_alloc_bitmap();

	set_object_header(&sm->sm_o, spaceman_size(), oid,
			  APFS_OBJ_EPHEMERAL | APFS_OBJECT_TYPE_SPACEMAN,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(sm, spaceman_size());
}
