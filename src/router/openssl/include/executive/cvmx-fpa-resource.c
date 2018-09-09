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
#endif

/** Allocates the pool from global resources and reserves them
  * @param pool	    FPA pool to allocate/reserve. If -1 it
  *                 finds the empty pool to allocate.
  * @return         Allocated pool number OR -1 if fails to allocate
                    the pool
  */
int cvmx_fpa_alloc_pool(int pool)
{
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_FPA, CVMX_FPA_NUM_POOLS)) {
		cvmx_dprintf("\nFailed to create FPA global resource");
		return -1;
	}

	if (pool >= 0)
		pool = cvmx_reserve_global_resource_range(CVMX_GR_TAG_FPA, pool, pool, 1);
	else
		/* Find an empty pool */
		pool = cvmx_allocate_global_resource_range(CVMX_GR_TAG_FPA,(uint64_t)pool, 1, 1);
	if (pool == -1) {
		cvmx_dprintf("Error: FPA pool is not available to use\n");
		return -1;
	}
	//cvmx_dprintf("Error: FPA pool %d is free to use\n", pool);
	return pool;
}
EXPORT_SYMBOL(cvmx_fpa_alloc_pool);

static inline struct global_resource_tag get_fpa_resourse_tag(int node)
{
	switch(node) {
	case 0:
		return CVMX_GR_TAG_FPA;
	case 1:
		return cvmx_get_gr_tag('c','v','m','_','f','p','a','_','0','1','.','.','.','.','.','.');
	case 2:
		return cvmx_get_gr_tag('c','v','m','_','f','p','a','_','0','2','.','.','.','.','.','.');
	case 3:
		return cvmx_get_gr_tag('c','v','m','_','f','p','a','_','0','3','.','.','.','.','.','.');
	default:
		/* Add a panic?? */
		return cvmx_get_gr_tag('i','n','v','a','l','i','d','.','.','.','.','.','.','.','.','.');
	}
}


static inline struct global_resource_tag get_aura_resourse_tag(int node)
{
	switch(node) {
	case 0:
		return cvmx_get_gr_tag('c','v','m','_','a','u','r','a','_','0','_','0','.','.','.','.');
	case 1:
		return cvmx_get_gr_tag('c','v','m','_','a','u','r','a','_','0','_','1','.','.','.','.');
	case 2:
		return cvmx_get_gr_tag('c','v','m','_','a','u','r','a','_','0','_','2','.','.','.','.');
	case 3:
		return cvmx_get_gr_tag('c','v','m','_','a','u','r','a','_','0','_','3','.','.','.','.');
	default:
		/* Add a panic?? */
		return cvmx_get_gr_tag('i','n','v','a','l','i','d','.','.','.','.','.','.','.','.','.');
	}
}


/**
 * This will allocate/reserve count number of FPA pools on the specified node to the
 * calling application. These pools will be for exclusive use of the application
 * until they are freed.
 * @param pools_allocated is an array of length count allocated by the application
 * before invoking the cvmx_allocate_fpa_pool call. On return it will contain the
 * index numbers of the pools allocated.
 * If -1 it finds the empty pool to allocate otherwise it reserves the specified pool.
 * @return 0 on success and -1 on failure.
 */
int cvmx_fpa_allocate_fpa_pools(int node, int pools_allocated[], int count)
{
	int num_pools = CVMX_FPA_NUM_POOLS;
	uint64_t owner = 0;
	int rv = 0;
	struct global_resource_tag tag = get_fpa_resourse_tag(node);

	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) 
		num_pools = CVMX_FPA3_NUM_POOLS;

	if (cvmx_create_global_resource_range(tag, num_pools) != 0) {
		cvmx_dprintf("ERROR: failed to create FPA global resource for"
			     " node=%d\n", node);
		return -1;
	}

	if (pools_allocated[0] >= 0) {
		while (count--)
			rv = cvmx_reserve_global_resource_range(tag, owner, pools_allocated[count], 1);
	} else {
		rv = cvmx_resource_alloc_many(tag, owner,
						       count,
						       pools_allocated);
	}
	return rv;
}


/** Release/Frees the specified pool
  * @param pool	    Pool to free
  * @return         0 for success -1 failure
  */
int cvmx_fpa_release_pool(int pool)
{
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_FPA, pool, 1) == -1) {
		cvmx_dprintf("\nERROR Failed to release FPA pool %d", (int)pool);
		return -1;
	}
	return 0;
}

int cvmx_fpa_allocate_auras(int node, int auras_allocated[], int count)
{
	int num_aura = CVMX_FPA_AURA_NUM;
	uint64_t owner = 0;
	int rv = 0;
	struct global_resource_tag tag = get_aura_resourse_tag(node);

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_dprintf("ERROR :  Aura allocation not supported "
			     "on this model\n");
	}

	if (cvmx_create_global_resource_range(tag, num_aura) != 0) {
		cvmx_dprintf("ERROR: failed to create aura global resource for"
			     " node=%d\n", node);
		return -1;
	}
	if (auras_allocated[0] >= 0) {
		while (count--)
			rv = cvmx_reserve_global_resource_range(tag, owner, auras_allocated[count], 1);
	} else
		rv = cvmx_resource_alloc_many(tag, owner, count,
					auras_allocated);
	return rv;

}

int cvmx_fpa_free_auras(int node, int *pools_allocated, int count)
{
	int rv;
	struct global_resource_tag tag = get_aura_resourse_tag(node);

	rv = cvmx_free_global_resource_range_multiple(tag, pools_allocated,
						      count);
	return rv;
}

EXPORT_SYMBOL(cvmx_fpa_release_pool);
