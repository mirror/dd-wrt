/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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

/**
 * @file
 *
 * PKI Support.
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-pki.h>
#include "asm/octeon/cvmx-global-resources.h"
#include "asm/octeon/cvmx-range.h"
#include "asm/octeon/cvmx-atomic.h"
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki.h"
#include "cvmx-global-resources.h"
#include "cvmx-range.h"
#include "cvmx-atomic.h"
#endif

static CVMX_SHARED int32_t cvmx_pki_style_refcnt
	[CVMX_MAX_NODES][CVMX_PKI_NUM_INTERNAL_STYLE];

/**
 * This function allocates/reserves a style from pool of global styles per node.
 * @param node	node to allocate style from.
 * @param style	style to allocate, if -1 it will be allocated
 *		first available style from style resource. If index is positive
 *		number and in range, it will try to allocate specified style.
 * @return 	style number on success,
 *		-1 on alloc failure.
 *		-2 on resource already reserved.
 */
int cvmx_pki_style_alloc(int node, int style)
{
	int rs;

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_STYLE(node), CVMX_PKI_NUM_INTERNAL_STYLE)) {
		cvmx_printf("ERROR: Failed to create styles global resource\n");
		return -1;
	}
	if (style >= 0) {
		/* Reserving specific style, use refcnt for sharing */
		rs = cvmx_atomic_fetch_and_add32(
			&cvmx_pki_style_refcnt[node][style], 1);
		if (rs > 0)
			return CVMX_RESOURCE_ALREADY_RESERVED;

		rs = cvmx_reserve_global_resource_range(CVMX_GR_TAG_STYLE(node), style, style, 1);
		if (rs == -1) {
			/* This means the style is taken by another app */
			cvmx_printf("ERROR: style %d is reserved by another app\n",
				style);
			cvmx_atomic_fetch_and_add32(
				&cvmx_pki_style_refcnt[node][style], -1);
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	} else {
		/* Allocate first available style */
		rs = cvmx_allocate_global_resource_range(CVMX_GR_TAG_STYLE(node), style, 1, 1);
		if (rs < 0) {
			cvmx_printf("ERROR: Failed to allocate style, "
				"none available\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
		style = rs;
		/* Increment refcnt for newly created style */
		cvmx_atomic_fetch_and_add32(
			&cvmx_pki_style_refcnt[node][style], 1);
	}
	return style;
}

/**
 * This function frees a style from pool of global styles per node.
 * @param node	 node to free style from.
 * @param style	 style to free
 * @return 	 0 on success, -1 on failure or
 * if the style is shared a positive count of remaining users for this style.
 */
int cvmx_pki_style_free(int node, int style)
{
	int rs;

	rs = cvmx_atomic_fetch_and_add32(
		&cvmx_pki_style_refcnt[node][style], -1);
	if (rs > 1)
		return rs-1;

	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_STYLE(node), style, 1) == -1) {
		cvmx_printf("ERROR Failed to release style %d\n", (int)style);
		return -1;
	}
	return 0;
}


/**
 * This function allocates/reserves a cluster group from per node
   cluster group resources.
 * @param node	 	node to allocate cluster group from.
   @param cl_grp	cluster group to allocate/reserve, if -1 ,
 *			allocate any available cluster group.
 * @return 	 	cluster group number
 *			-1 on alloc failure.
 *			-2 on resource already reserved.
 */
int cvmx_pki_cluster_grp_alloc(int node, int cl_grp)
{
	int rs;

	if (node >= CVMX_MAX_NODES) {
		cvmx_printf("ERROR: Invalid node number %d\n", node);
		return -1;
	}
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_CLUSTER_GRP(node), CVMX_PKI_NUM_CLUSTER_GROUP)) {
		cvmx_printf("ERROR: Failed to create Cluster group global resource\n");
		return -1;
	}
	if (cl_grp >= 0) {
		rs = cvmx_reserve_global_resource_range(CVMX_GR_TAG_CLUSTER_GRP(node), 0, cl_grp, 1);
		if (rs == -1) {
			cvmx_dprintf("INFO: cl_grp %d is already reserved\n", (int)cl_grp);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		rs = cvmx_allocate_global_resource_range(CVMX_GR_TAG_CLUSTER_GRP(node), 0, 1, 1);
		if (rs == -1) {
			cvmx_dprintf("Warning: Failed to alloc cluster grp\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	cl_grp = rs;
	return cl_grp;
}

/**
 * This function frees a cluster group from per node
   cluster group resources.
 * @param node	 	node to free cluster group from.
   @param cl_grp	cluster group to free
 * @return 	 	0 on success
 *			-1 on alloc failure.
 *			-2 on resource already reserved.
 */
int cvmx_pki_cluster_grp_free(int node, int cl_grp)
{
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_CLUSTER_GRP(node), cl_grp, 1) == -1) {
		cvmx_printf("ERROR Failed to release cluster group %d\n", (int)cl_grp);
		return -1;
	}
	return 0;
}

/**
 * This function allocates/reserves a cluster from per node
 * cluster resources.
 * @param node	 	node to allocate cluster group from.
 * @param cluster_mask	mask of clusters  to allocate/reserve, if -1 ,
 *			allocate any available clusters.
 * @param num_clusters	number of clusters that will be allocated
 */
int cvmx_pki_cluster_alloc(int node, int num_clusters, uint64_t *cluster_mask)
{
	unsigned cluster = 0;
	int clusters[CVMX_PKI_NUM_CLUSTER];

	if (node >= CVMX_MAX_NODES) {
		cvmx_printf("ERROR: Invalid node number %d\n", node);
		return -1;
	}
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_CLUSTERS(node), CVMX_PKI_NUM_CLUSTER)) {
		cvmx_printf("ERROR: Failed to create Clusters global resource\n");
		return -1;
	}
	if (*cluster_mask > 0) {
		while (cluster < CVMX_PKI_NUM_CLUSTER) {
			if (*cluster_mask & (0x01L << cluster)) {
				if (cvmx_reserve_global_resource_range(CVMX_GR_TAG_CLUSTERS(node), 0, cluster, 1) == -1) {
					cvmx_printf("ERROR: allocating cluster %d\n", cluster);
					return -1;
				}
			}
			cluster++;
		}
	} else {
		if (cvmx_resource_alloc_many(CVMX_GR_TAG_CLUSTERS(node), 0, num_clusters, clusters) == -1) {
			   cvmx_printf("ERROR: allocating clusters\n");
			   return -1;
		}
		*cluster_mask = 0;
		while (num_clusters--)
			*cluster_mask |= (0x1ul << clusters[num_clusters]);
	}
	return 0;
}

/**
 * This function frees  clusters  from per node
   clusters resources.
 * @param node	 	node to free clusters from.
 * @param cluster_mask  mask of clusters need freeing
 * @return 	 	0 on success or -1 on failure
 */
int cvmx_pki_cluster_free(int node, uint64_t cluster_mask)
{
	unsigned cluster;
	if (cluster_mask == 0)
		return 0;

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		if ((cluster_mask & (0x01L << cluster)) == 0)
			continue;
		if (cvmx_free_global_resource_range_with_base(
			CVMX_GR_TAG_CLUSTERS(node), cluster, 1) == -1) {
			cvmx_printf("ERROR: freeing cluster %d\n", cluster);
			return -1;
		}
	} /* for */
	return 0;
}

/**
 * This function allocates/reserves a pcam entry from node
 * @param node	 	node to allocate pcam entry from.
 * @param index  	index of pacm entry (0-191), if -1 ,
 *			allocate any available pcam entry.
 * @param bank		pcam bank where to allocate/reserve pcan entry from
 * @param cluster_mask  mask of clusters from which pcam entry is needed.
 * @return 	 	pcam entry of -1 on failure
 */
int cvmx_pki_pcam_entry_alloc(int node, int index, int bank, uint64_t cluster_mask)
{
	int rs = 0;
	unsigned cluster;

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		if ((cluster_mask & (1 << cluster)) == 0) 
			continue;
		rs =cvmx_create_global_resource_range(
			CVMX_GR_TAG_PCAM(node, cluster, bank),
			CVMX_PKI_TOTAL_PCAM_ENTRY);
		if (rs != 0) {
			cvmx_printf("ERROR: Failed to create pki pcam global resource\n");
			return -1;
		}
		if (index >= 0)
			rs = cvmx_reserve_global_resource_range(
				CVMX_GR_TAG_PCAM(node, cluster, bank),
				cluster, index, 1);
		else
			rs = cvmx_allocate_global_resource_range(
				CVMX_GR_TAG_PCAM(node, cluster, bank),
				cluster, 1, 1);
		if (rs == -1) {
			cvmx_printf("ERROR: PCAM :index %d not available in cluster %d bank %d",
				(int)index, (int)cluster, bank);
			return -1;
		}
	} /* for cluster */
	index = rs;
	/* implement cluster handle for pass2, for now assume
	all clusters will have same base index*/
	return index;
}

/**
 * This function frees a pcam entry from node
 * @param node	 	node to allocate pcam entry from.
   @param index  	index of pacm entry (0-191) needs to be freed.
 * @param bank		pcam bank where to free pcam entry from
 * @param cluster_mask  mask of clusters from which pcam entry is freed.
 * @return 	 	0 on success OR -1 on failure
 */
int cvmx_pki_pcam_entry_free(int node, int index, int bank, uint64_t cluster_mask)
{
	unsigned cluster;

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		if ((cluster_mask & (0x01L << cluster)) == 0)
			continue;
		if (cvmx_free_global_resource_range_with_base (
			CVMX_GR_TAG_PCAM(node, cluster, bank), index, 1) == -1){
			cvmx_printf("ERROR: freeing cluster %d\n", (int)cluster);
			return -1;
		}
	}
	return 0;
}


/**
 * This function allocates/reserves QPG table entries per node.
 * @param node	 	node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
 *			qpg base_offset will be allocated. If base_offset is positive
 *		 	number and in range, it will try to allocate specified base_offset.
 * @param count		number of consecutive qpg entries to allocate. They will be consecutive
 *                       from base offset.
 * @return 	 	qpg table base offset number on success
 *			-1 on alloc failure.
 *			-2 on resource already reserved.
 */
int cvmx_pki_qpg_entry_alloc(int node, int base_offset, int count)
{
	int rs;

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_QPG_ENTRY(node), CVMX_PKI_NUM_QPG_ENTRY)) {
		cvmx_printf("ERROR: Failed to create qpg_entry global resource\n");
		return -1;
	}
	if (base_offset >= 0) {
		rs = cvmx_reserve_global_resource_range(CVMX_GR_TAG_QPG_ENTRY(node),
				base_offset, base_offset, count);
		if (rs == -1) {
			cvmx_dprintf("INFO: qpg entry %d is already reserved\n", (int)base_offset);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		rs = cvmx_allocate_global_resource_range(CVMX_GR_TAG_QPG_ENTRY(node), base_offset, count, 1);
		if (rs == -1) {
			cvmx_printf("ERROR: Failed to allocate qpg entry\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	base_offset = rs;
	return base_offset;
}

/**
 * This function frees QPG table entries per node.
 * @param node	 	node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
 *			qpg base_offset will be allocated. If base_offset is positive
 *		 	number and in range, it will try to allocate specified base_offset.
 * @param count		number of consecutive qpg entries to allocate. They will be consecutive
 *			from base offset.
 * @return 	 	qpg table base offset number on success, -1 on failure.
 */
int cvmx_pki_qpg_entry_free(int node, int base_offset, int count)
{
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_QPG_ENTRY(node), base_offset, count) == -1) {
		cvmx_printf("ERROR Failed to release qpg offset %d", (int)base_offset);
		return -1;
	}
	return 0;
}

/**
 * This function frees all the PKI software resources
 * (clusters, styles, qpg_entry, pcam_entry etc) for the specified node
 */
void __cvmx_pki_global_rsrc_free(int node)
{
	int cnt;
	unsigned  cluster;
	int  bank;

	cnt = CVMX_PKI_NUM_CLUSTER_GROUP;
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_CLUSTER_GRP(node), 0, cnt) == -1) {
		cvmx_printf("ERROR pki-rsrc: Failed to release all styles\n");
	}

#if 0
	cnt = CVMX_PKI_NUM_CLUSTER;
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_CLUSTERS(node), 0, cnt) == -1) {
		cvmx_printf("ERROR pki-rsrc:Failed to release all clusters\n");
	}
#endif

	cnt = CVMX_PKI_NUM_FINAL_STYLE;
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_STYLE(node), 0, cnt) == -1) {
		cvmx_printf("ERROR pki-rsrc:Failed to release all styles\n");
	}

	cnt = CVMX_PKI_NUM_QPG_ENTRY;
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_QPG_ENTRY(node), 0, cnt) == -1) {
		cvmx_printf("ERROR pki-rsrc:Failed to release all qpg entries\n");
	}

	cnt = CVMX_PKI_NUM_BPID;
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_BPID(node), 0, cnt) == -1) {
		cvmx_printf("ERROR pki-rsrc:Failed to release all bpids\n");
	}

	cnt = CVMX_PKI_NUM_PCAM_ENTRY;
	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		for (bank = 0; bank < CVMX_PKI_NUM_PCAM_BANK; bank++) {
			if (cvmx_free_global_resource_range_with_base (
				CVMX_GR_TAG_PCAM(node, cluster, bank), 0, cnt) == -1) {
				cvmx_printf("ERROR pki-rsrc:Failed to release all pcan entries\n");
			}
		}
	}

}

/**
 * This function allocates/reserves a bpid from pool of global bpid per node.
 * @param node	node to allocate bpid from.
 * @param bpid	bpid  to allocate, if -1 it will be allocated
 *		first available boid from bpid resource. If index is positive
 *		number and in range, it will try to allocate specified bpid.
 * @return 	bpid number on success,
 *		-1 on alloc failure.
 *		-2 on resource already reserved.
 */
int cvmx_pki_bpid_alloc(int node, int bpid)
{
	int rs;

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_BPID(node), CVMX_PKI_NUM_BPID)) {
		cvmx_printf("ERROR: Failed to create bpid global resource\n");
		return -1;
	}
	if (bpid >= 0) {
		rs = cvmx_reserve_global_resource_range(CVMX_GR_TAG_BPID(node),
				bpid, bpid, 1);
		if (rs == -1) {
			cvmx_dprintf("INFO: bpid %d is already reserved\n", (int)bpid);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		rs = cvmx_allocate_global_resource_range(CVMX_GR_TAG_BPID(node), bpid, 1, 1);
		if (rs == -1) {
			cvmx_printf("ERROR: Failed to allocate bpid\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
		if (rs == 0) { /* don't use bpid 0 */
			rs = cvmx_allocate_global_resource_range(CVMX_GR_TAG_BPID(node), bpid, 1, 1);
			if (rs == -1) {
				cvmx_printf("ERROR: Failed to allocate bpid\n");
				return CVMX_RESOURCE_ALLOC_FAILED;
			}
		}
	}
	bpid = rs;
	return bpid;
}

/**
 * This function frees a bpid from pool of global bpid per node.
 * @param node	 node to free bpid from.
 * @param bpid	 bpid to free
 * @return 	 0 on success, -1 on failure or
 */
int cvmx_pki_bpid_free(int node, int bpid)
{
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_BPID(node), bpid, 1) == -1) {
		cvmx_printf("ERROR Failed to release bpid %d\n", (int)bpid);
		return -1;
	}
	return 0;
}

int cvmx_pki_mtag_idx_alloc(int node, int idx)
{
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_MTAG_IDX(node), CVMX_PKI_NUM_MTAG_IDX)) {
		cvmx_printf("ERROR: Failed to create MTAG-IDX global resource\n");
		return -1;
	}
	if (idx >= 0) {
		idx = cvmx_reserve_global_resource_range(CVMX_GR_TAG_MTAG_IDX(node), idx, idx, 1);
		if (idx == -1) {
			cvmx_dprintf("INFO: MTAG index %d is already reserved\n", (int)idx);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		idx = cvmx_allocate_global_resource_range(CVMX_GR_TAG_MTAG_IDX(node), idx, 1, 1);
		if (idx == -1) {
			cvmx_printf("ERROR: Failed to allocate MTAG index\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	return idx;
}

void cvmx_pki_mtag_idx_free(int node, int idx)
{
	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_MTAG_IDX(node), idx, 1) == -1)
		cvmx_printf("ERROR Failed to release MTAG index %d\n", (int)idx);
}

