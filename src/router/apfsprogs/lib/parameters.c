/*
 * Copyright (C) 2021 Ernesto A. Fernández <ernesto@corellium.com>
 *
 * Calculation of filesystem parameters needed by both mkfs and fsck.
 */

#include <apfs/parameters.h>
#include <apfs/types.h>

/**
 * ip_fq_node_limit - Calculate the node limit for the internal pool free queue
 * @chunks: chunk count for the container
 */
u16 ip_fq_node_limit(u64 chunks)
{
	u16 ret = 3 * (chunks + 751) / 1127 - 1;

	if (ret == 2)
		ret = 3; /* Leave room for a new root node */
	return ret;
}

/**
 * main_fq_node_limit - Calculate the node limit for the main free queue
 * @blocks: block count for the container
 */
u16 main_fq_node_limit(u64 blocks)
{
	u64 blks_1gb = 0x40000;
	u64 blks_4gb = 0x100000;
	u16 ret;

	/*
	 * The node limit is required to be a function of the block count.
	 * Inside each of the (0, 1G), [1G, 4G) and [4G, +inf) intervals, the
	 * function is linear.
	 */
	if (blocks < blks_1gb)
		ret = 1 + (blocks - 1) / 4544;
	else if (blocks < blks_4gb)
		ret = 116 + (blocks - 261281) / 2272;
	else
		ret = 512;

	if (ret == 2)
		ret = 3; /* Leave room for a new root node */
	return ret;
}
