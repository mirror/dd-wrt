/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "btree.h"
#include "mkapfs.h"
#include "object.h"
#include "spaceman.h"

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
 * count_used_blocks - Calculate the number of blocks used by the mkfs
 */
static inline u32 count_used_blocks(void)
{
	u64 chunk_count = DIV_ROUND_UP(param->block_count, blocks_per_chunk());
	u32 cib_count = DIV_ROUND_UP(chunk_count, chunks_per_cib());
	u32 blocks = 0;

	blocks += 1;			/* Block zero */
	blocks += CPOINT_DESC_BLOCKS;	/* Checkpoint descriptor blocks */
	blocks += CPOINT_DATA_BLOCKS;	/* Checkpoint data blocks */
	blocks += cib_count;		/* Chunk-info blocks */
	blocks += 1;			/* Allocation bitmap block */
	blocks += 2;			/* Container object map and its root */
	blocks += 6;			/* Volume superblock and its trees */
	blocks += IP_BMAP_BLOCKS;	/* Internal pool bitmap blocks */
	blocks += IP_BLOCKS;		/* Internal pool blocks */
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
 * @bno: block number to use
 */
static void make_alloc_bitmap(u64 bno)
{
	u64 chunk_count = DIV_ROUND_UP(param->block_count, blocks_per_chunk());
	u32 cib_count = DIV_ROUND_UP(chunk_count, chunks_per_cib());
	void *bmap = get_zeroed_block(bno);

	/* Block zero */
	bmap_mark_as_used(bmap, 0, 1);
	/* Checkpoint descriptor blocks */
	bmap_mark_as_used(bmap, CPOINT_DESC_BASE, CPOINT_DESC_BLOCKS);
	/* Checkpoint data blocks */
	bmap_mark_as_used(bmap, CPOINT_DATA_BASE, CPOINT_DATA_BLOCKS);
	/* Chunk-info blocks */
	bmap_mark_as_used(bmap, FIRST_CIB_BNO, cib_count);
	/* Allocation bitmap block */
	bmap_mark_as_used(bmap, bno, 1);
	/* Container object map and its root */
	bmap_mark_as_used(bmap, MAIN_OMAP_BNO, 2);
	/* Volume superblock and its trees */
	bmap_mark_as_used(bmap, FIRST_VOL_BNO, 6);
	/* Internal pool bitmap blocks */
	bmap_mark_as_used(bmap, IP_BMAP_BASE, IP_BMAP_BLOCKS);
	/* Internal pool blocks */
	bmap_mark_as_used(bmap, IP_BASE, IP_BLOCKS);

	munmap(bmap, param->blocksize);
}

/*
 * Offsets into the spaceman block for a non-versioned container; the values
 * have been borrowed from a test image.
 */
#define BITMAP_XID_OFF		0x150	/* Transaction id for the ip bitmap */
#define BITMAP_OFF		0x158	/* Address of the ip bitmap */
#define BITMAP_FREE_NEXT_OFF	0x160	/* No idea */
#define CIB_ADDR_BASE_OFF	0x180	/* First cib address for main device */

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
	u32 block_count, free_count;

	chunk->ci_xid = cpu_to_le64(MKFS_XID);
	chunk->ci_addr = cpu_to_le64(start);

	/* The first chunk is the only one that's not a hole */
	if (!start) {
		chunk->ci_bitmap_addr = cpu_to_le64(FIRST_CHUNK_BITMAP_BNO);
		make_alloc_bitmap(FIRST_CHUNK_BITMAP_BNO);
	}

	block_count = blocks_per_chunk();
	if (remaining_blocks < block_count) /* This is the final chunk */
		block_count = remaining_blocks;
	chunk->ci_block_count = cpu_to_le32(block_count);

	free_count = block_count;
	if (!start) /* The mkfs puts all of its blocks in the first chunk */
		free_count -= count_used_blocks();
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

	set_object_header(&cib->cib_o, bno,
			  APFS_OBJ_PHYSICAL | APFS_OBJECT_TYPE_SPACEMAN_CIB,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(cib, param->blocksize);

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
	u64 start = 0;
	__le64 *cib_addr;
	int i;

	/*
	 * We must have room for the addresses of all main device cibs, plus
	 * an extra offset for tier 2.
	 */
	if (cib_count + 1 >
	    (param->blocksize - CIB_ADDR_BASE_OFF) / sizeof(__le64)) {
		printf("Large containers are not yet supported.\n");
		exit(1);
	}

	dev->sm_block_count = cpu_to_le64(param->block_count);
	dev->sm_chunk_count = cpu_to_le64(chunk_count);
	dev->sm_cib_count = cpu_to_le32(cib_count);
	dev->sm_cab_count = 0; /* Not supported, hence the block count limit */
	dev->sm_free_count = cpu_to_le64(param->block_count -
					 count_used_blocks());

	dev->sm_addr_offset = cpu_to_le32(CIB_ADDR_BASE_OFF);
	cib_addr = (void *)sm + CIB_ADDR_BASE_OFF;
	for (i = 0; i < cib_count; ++i) {
		cib_addr[i] = cpu_to_le64(FIRST_CIB_BNO + i);
		start = make_chunk_info_block(FIRST_CIB_BNO + i, i, start);
	}

	/* For the tier2 device, just set the offset; the address is null */
	dev = &sm->sm_dev[APFS_SD_TIER2];
	dev->sm_addr_offset = cpu_to_le32(CIB_ADDR_BASE_OFF +
					  cib_count * sizeof(__le64));
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
	fq->sfq_oldest_xid = 0;	/* Is this correct? */
	fq->sfq_tree_node_limit = cpu_to_le16(1);
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
	fq->sfq_oldest_xid = 0;	/* Is this correct? */
	fq->sfq_tree_node_limit = cpu_to_le16(200); /* From test images */
}

/**
 * make_internal_pool - Make the internal pool of the space manager
 * @sm: pointer to the on-disk spaceman structure
 */
static void make_internal_pool(struct apfs_spaceman_phys *sm)
{
	int i;
	__le64 *addr;

	sm->sm_ip_bm_tx_multiplier =
				cpu_to_le32(APFS_SPACEMAN_IP_BM_TX_MULTIPLIER);
	sm->sm_ip_block_count = cpu_to_le64(IP_BLOCKS);
	sm->sm_ip_base = cpu_to_le64(IP_BASE);
	/* No support for multiblock bitmaps */
	sm->sm_ip_bm_size_in_blocks = cpu_to_le32(1);

	sm->sm_ip_bm_block_count = cpu_to_le32(IP_BMAP_BLOCKS);
	sm->sm_ip_bm_base = cpu_to_le64(IP_BMAP_BASE);
	for (i = 0; i < IP_BMAP_BLOCKS; ++i) /* We use no blocks from the ip */
		munmap(get_zeroed_block(IP_BMAP_BASE + i), param->blocksize);

	/* Current bitmap is the first, so the offset is left at zero */
	sm->sm_ip_bitmap_offset = cpu_to_le32(BITMAP_OFF);
	sm->sm_ip_bm_free_head = cpu_to_le16(1);
	sm->sm_ip_bm_free_tail = cpu_to_le16(IP_BMAP_BLOCKS - 1);

	sm->sm_ip_bm_xid_offset = cpu_to_le32(BITMAP_XID_OFF);
	addr = (void *)sm + BITMAP_XID_OFF;
	*addr = cpu_to_le64(MKFS_XID);

	/* I have no idea what this means */
	sm->sm_ip_bm_free_next_offset = cpu_to_le32(BITMAP_FREE_NEXT_OFF);
	addr = (void *)sm + BITMAP_FREE_NEXT_OFF;
	*addr = cpu_to_le64(0x0004000300020001);
}

/**
 * make_spaceman - Make the space manager for the container
 * @bno: block number to use
 * @oid: object id
 */
void make_spaceman(u64 bno, u64 oid)
{
	struct apfs_spaceman_phys *sm = get_zeroed_block(bno);

	sm->sm_block_size = cpu_to_le32(param->blocksize);
	sm->sm_blocks_per_chunk = cpu_to_le32(blocks_per_chunk());
	sm->sm_chunks_per_cib = cpu_to_le32(chunks_per_cib());
	sm->sm_cibs_per_cab = cpu_to_le32(cibs_per_cab());

	make_devices(sm);
	make_ip_free_queue(&sm->sm_fq[APFS_SFQ_IP]);
	make_main_free_queue(&sm->sm_fq[APFS_SFQ_MAIN]);
	make_internal_pool(sm);

	set_object_header(&sm->sm_o, oid,
			  APFS_OBJ_EPHEMERAL | APFS_OBJECT_TYPE_SPACEMAN,
			  APFS_OBJECT_TYPE_INVALID);
	munmap(sm, param->blocksize);
}
