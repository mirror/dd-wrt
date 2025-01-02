/***********************license start***************
 * Copyright (c) 2012  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include "linux/export.h"
#include "asm/octeon/cvmx.h"
#include "asm/octeon/cvmx-fpa.h"
#include "asm/octeon/cvmx-global-resources.h"
#else
#include "cvmx.h"
#include "cvmx-fpa.h"
#include "cvmx-global-resources.h"
#include "cvmx-sysinfo.h"
#endif

int cvmx_fpa1_init_pool = 0; /* ACKSYS: for FPA first init, see cvmx-fpa1.h */

static struct global_resource_tag
get_fpa1_resource_tag(void)
{
		return CVMX_GR_TAG_FPA;
}

static struct global_resource_tag
get_fpa3_aura_resource_tag(int node)
{
	return cvmx_get_gr_tag('c', 'v', 'm', '_', 'a', 'u', 'r', 'a', '_',
		node+'0', '.', '.', '.', '.', '.', '.');
}


static struct global_resource_tag
get_fpa3_pool_resource_tag(int node)
{
	return cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'o', 'o', 'l', '_',
		node+'0', '.', '.', '.', '.', '.', '.');
}

int cvmx_fpa_get_max_pools(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3))
		return cvmx_fpa3_num_auras();
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		/* 68xx pool 8 is not available via API */
		return  CVMX_FPA1_NUM_POOLS;
	else
		return  CVMX_FPA1_NUM_POOLS;
}

uint64_t cvmx_fpa3_get_aura_owner(cvmx_fpa3_gaura_t aura)
{
	return cvmx_get_global_resource_owner(
		get_fpa3_aura_resource_tag(aura.node),
		aura.laura);
}

uint64_t cvmx_fpa1_get_pool_owner(cvmx_fpa1_pool_t pool)
{
	return cvmx_get_global_resource_owner(
		get_fpa1_resource_tag(), pool);
}

uint64_t cvmx_fpa_get_pool_owner(int pool_num)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3))
		return cvmx_fpa3_get_aura_owner(
			cvmx_fpa1_pool_to_fpa3_aura(pool_num));
	else
		return cvmx_fpa1_get_pool_owner(pool_num);
}

/**
 */
cvmx_fpa3_gaura_t
cvmx_fpa3_reserve_aura(int node, int desired_aura_num)
{
	uint64_t owner = cvmx_get_app_id();
	int rv = 0;
	struct global_resource_tag tag;
	cvmx_fpa3_gaura_t aura;

	if (node == -1)
		node = cvmx_get_node_num();

	tag = get_fpa3_aura_resource_tag(node);

	if (cvmx_create_global_resource_range(tag,
			cvmx_fpa3_num_auras()) != 0) {
		cvmx_printf("ERROR: %s: global resource create node=%u\n",
			__func__, node);
		return CVMX_FPA3_INVALID_GAURA;
	}

	if (desired_aura_num >= 0)
		rv = cvmx_reserve_global_resource_range(
			tag, owner, desired_aura_num, 1);
	else
		rv = cvmx_resource_alloc_reverse(tag, owner);

	if (rv < 0) {
		cvmx_printf("ERROR: %s: node=%u desired aura=%d\n",
			__func__, node, desired_aura_num);
		return CVMX_FPA3_INVALID_GAURA;
	}

	aura = __cvmx_fpa3_gaura(node, rv);

	return aura;
}

int cvmx_fpa3_release_aura(cvmx_fpa3_gaura_t aura)
{
	struct global_resource_tag tag = get_fpa3_aura_resource_tag(aura.node);
	int laura = aura.laura;

	if (!__cvmx_fpa3_aura_valid(aura))
		return -1;

	return
		cvmx_free_global_resource_range_multiple(tag, &laura, 1);
}

/**
 */
cvmx_fpa3_pool_t
cvmx_fpa3_reserve_pool(int node, int desired_pool_num)
{
	uint64_t owner = cvmx_get_app_id();
	int rv = 0;
	struct global_resource_tag tag;
	cvmx_fpa3_pool_t pool;

	if (node == -1) node = cvmx_get_node_num();

	tag = get_fpa3_pool_resource_tag(node);

	if (cvmx_create_global_resource_range(tag,
			cvmx_fpa3_num_pools()) != 0) {
		cvmx_printf("ERROR: %s: global resource create node=%u\n",
			__func__, node);
		return CVMX_FPA3_INVALID_POOL;
	}

	if (desired_pool_num >= 0)
		rv = cvmx_reserve_global_resource_range(
			tag, owner, desired_pool_num, 1);
	else
		rv = cvmx_resource_alloc_reverse(tag, owner);

	if (rv < 0) {
		/* Desired pool is already in use */
		return CVMX_FPA3_INVALID_POOL;
	}

	pool = __cvmx_fpa3_pool(node, rv);

	return pool;
}

int cvmx_fpa3_release_pool(cvmx_fpa3_pool_t pool)
{
	struct global_resource_tag tag = get_fpa3_pool_resource_tag(pool.node);
	int lpool = pool.lpool;

	if (!__cvmx_fpa3_pool_valid(pool))
		return -1;

	if (cvmx_create_global_resource_range(tag,
			cvmx_fpa3_num_pools()) != 0) {
		cvmx_printf("ERROR: %s: global resource create node=%u\n",
			__func__, pool.node);
		return -1;
	}

	return
		cvmx_free_global_resource_range_multiple(tag, &lpool, 1);
}

cvmx_fpa1_pool_t
cvmx_fpa1_reserve_pool(int desired_pool_num)
{
	uint64_t owner = cvmx_get_app_id();
	struct global_resource_tag tag;
	int rv;

	tag = get_fpa1_resource_tag();

	if (cvmx_create_global_resource_range(tag, CVMX_FPA1_NUM_POOLS) != 0) {
		cvmx_printf("ERROR: %s: global resource not created\n",
			__func__);
		return -1;
	}

	if (desired_pool_num >= 0) {
		rv = cvmx_reserve_global_resource_range(
			tag, owner, desired_pool_num, 1);
	} else {
		rv = cvmx_resource_alloc_reverse(tag, owner);
	}

	if (rv <  0) {
		cvmx_printf("ERROR: %s: FPA_POOL %d unavailable\n",
			__func__, desired_pool_num);
		return CVMX_RESOURCE_ALREADY_RESERVED;
	}
	return (cvmx_fpa1_pool_t) rv;
}

int cvmx_fpa1_release_pool(cvmx_fpa1_pool_t pool)
{
	struct global_resource_tag tag;

	tag = get_fpa1_resource_tag();

	return
		cvmx_free_global_resource_range_multiple(tag, &pool, 1);
}

/**
 * Query if an FPA pool is available for reservation
 * using global resources
 * @note This function is no longer in use, and will be removed in a future release
 */
int cvmx_fpa1_is_pool_available(cvmx_fpa1_pool_t pool)
{
	if (cvmx_fpa1_reserve_pool(pool) == -1)
		return 0;
	cvmx_fpa1_release_pool(pool);
	return 1;
}

/**
 * @INTERNAL
 *
 * This function is no longer in use, and will be removed in a future release
 */
int cvmx_fpa3_is_pool_available(int node, int lpool)
{
	cvmx_fpa3_pool_t pool;

	pool = cvmx_fpa3_reserve_pool(node, lpool);

	if (!__cvmx_fpa3_pool_valid(pool))
		return 0;

	cvmx_fpa3_release_pool(pool);
	return 1;
}

/**
 * @INTERNAL
 *
 * This function is no longer in use, and will be removed in a future release
 */
int cvmx_fpa3_is_aura_available(int node, int laura)
{
	cvmx_fpa3_gaura_t aura;

	aura = cvmx_fpa3_reserve_aura(node, laura);

	if (!__cvmx_fpa3_aura_valid(aura))
		return 0;

	cvmx_fpa3_release_aura(aura);
	return 1;
}

/**
 * Return if aura/pool is already reserved
 * @param pool_num - pool to check (aura for o78+)
 * @return 0 if reserved, 1 if available
 *
 * @note This function is no longer in use, and will be removed in a future release
 */
int cvmx_fpa_is_pool_available(int pool_num)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3))
		return cvmx_fpa3_is_aura_available(0, pool_num);
	else
		return cvmx_fpa1_is_pool_available(pool_num);
}

EXPORT_SYMBOL(cvmx_fpa3_reserve_aura);
EXPORT_SYMBOL(cvmx_fpa3_release_aura);
EXPORT_SYMBOL(cvmx_fpa3_reserve_pool);
EXPORT_SYMBOL(cvmx_fpa3_release_pool);
EXPORT_SYMBOL(cvmx_fpa1_reserve_pool);
EXPORT_SYMBOL(cvmx_fpa1_release_pool);
