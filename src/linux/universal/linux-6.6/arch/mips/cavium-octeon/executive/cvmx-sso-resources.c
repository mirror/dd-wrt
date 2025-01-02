/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-global-resources.h>
#else
#include "cvmx.h"
#include "cvmx-pow.h"
#include "cvmx-global-resources.h"
#endif

static struct global_resource_tag get_sso_resource_tag(int node)
{
	switch(node) {
	case 0:
	case 1:
	case 2:
	case 3:
		return CVMX_GR_TAG_SSO_GRP(node);
	default:
		/* Add a panic?? */
		return CVMX_GR_TAG_INVALID;
	}
}

static int cvmx_sso_get_num_groups(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 256;
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 64;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 64;
	return 16;
}

static struct global_resource_tag cvmx_sso_get_resource(int node)
{
	struct global_resource_tag tag = get_sso_resource_tag(node);

	if (cvmx_create_global_resource_range(tag, cvmx_sso_get_num_groups()) != 0) {
		cvmx_dprintf("ERROR: failed to create sso global resource for node=%d\n", node);
		return CVMX_GR_TAG_INVALID;
	}

	return tag;
}

int cvmx_sso_reserve_group_range(int node, int *base_group, int count)
{
	int start, i;
	uint64_t owner = 0;
	struct global_resource_tag tag = cvmx_sso_get_resource(node);

	if (cvmx_gr_same_tag(tag, CVMX_GR_TAG_INVALID))
		return CVMX_RESOURCE_ALLOC_FAILED;

	if (*base_group >= 0) {
		start = cvmx_reserve_global_resource_range(tag, owner, *base_group, count);
		if (start != *base_group) {
			cvmx_printf("Warning: SSO groups %i-%i already reserved.\n",
				    *base_group, *base_group + count);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		} else
			return 0;
	} else {
		start = cvmx_allocate_global_resource_range(tag, owner, count, 1);
		if (start < 0) {
			return CVMX_RESOURCE_ALREADY_RESERVED;
		} else {
			for (i = 0; i < count; i++)
				base_group[i] = start + i;
			return 0;
		}
	}
}
EXPORT_SYMBOL(cvmx_sso_reserve_group_range);

int cvmx_sso_reserve_group(int node)
{
	int r;
	int grp = -1;

	r = cvmx_sso_reserve_group_range(node, &grp, 1);

	return r == 0 ? grp : -1;
}
EXPORT_SYMBOL(cvmx_sso_reserve_group);

int cvmx_sso_release_group_range(int node, int base_group, int count)
{
	struct global_resource_tag tag = cvmx_sso_get_resource(node);
	
	return cvmx_free_global_resource_range_with_base(tag, base_group, count);
}
EXPORT_SYMBOL(cvmx_sso_release_group_range);

int cvmx_sso_release_group(int node, int group)
{
	return cvmx_sso_release_group_range(node, group, 1);
}
EXPORT_SYMBOL(cvmx_sso_release_group);
