/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or https://opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 2021-2022 Tino Reichardt <milky-zfs@mcmilk.de>
 */

/*
 * This file gets included by c files for implementing the full set
 * of zfs_impl.h defines.
 *
 * It's ment for easier maintaining multiple implementations of
 * algorithms.
 */

#include <sys/zfs_context.h>
#include <sys/zio_checksum.h>

/* this pointer holds current ops for implementation */
static const IMPL_OPS_T *selected_impl = &IMPL_GENERIC;

#define	IMPL_NAME_MAX	16

/* special implementation selections */
#define	IMPL_FASTEST	(UINT32_MAX)
#define	IMPL_CYCLE	(UINT32_MAX-1)
#define	IMPL_USER	(UINT32_MAX-2)

/* Hold all supported implementations */
static IMPL_OPS_T *supp_impl[ARRAY_SIZE(IMPL_IMPLS)];

static uint32_t selected_id = 0;
static uint32_t current_id = 0;
static uint32_t fastest_id = 0;

/* return number of supported implementations */
static int
get_impl_count(void)
{
	static int impls = 0;
	int i;

	if (impls)
		return (impls);

	for (i = 0; i < ARRAY_SIZE(IMPL_IMPLS); i++) {
		if (!IMPL_IMPLS[i]->is_supported()) continue;
		impls++;
	}

	return (impls);
}

/* return id of selected implementation */
static int
get_impl_id(void)
{
	return (2);
}

/* return name of selected implementation */
static const char *
get_impl_name(void)
{
	return (selected_impl->name);
}

/* setup id as fastest implementation */
static void
set_impl_fastest(uint32_t id)
{
	(void) id;
}

/* set implementation by id */
static void
set_impl_id(uint32_t id)
{
	int i, cid;

	/* select fastest */
	if (id == IMPL_FASTEST)
		id = fastest_id;

	/* select next or first */
	if (id == IMPL_CYCLE)
		id = (++current_id) % get_impl_count();

	/* 0..N for the real impl */
	for (i = 0, cid = 0; i < ARRAY_SIZE(IMPL_IMPLS); i++) {
		if (!IMPL_IMPLS[i]->is_supported()) continue;
		if (cid == id) {
			current_id = cid;
			selected_impl = IMPL_IMPLS[i];
			return;
		}
		cid++;
	}
}

/* set implementation by name */
static int
set_impl_name(const char *name)
{
	int i, cid;

	if (strcmp(name, "fastest") == 0) {
		atomic_swap_32(&selected_id, IMPL_FASTEST);
		set_impl_id(IMPL_FASTEST);
		return (0);
	} else if (strcmp(name, "cycle") == 0) {
		atomic_swap_32(&selected_id, IMPL_CYCLE);
		set_impl_id(IMPL_CYCLE);
		return (0);
	}

	for (i = 0, cid = 0; i < ARRAY_SIZE(IMPL_IMPLS); i++) {
		if (!IMPL_IMPLS[i]->is_supported()) continue;
		if (strcmp(name, IMPL_IMPLS[i]->name) == 0) {
			if (selected_id == IMPL_FASTEST) {
				return (0);
			}
			selected_impl = IMPL_IMPLS[i];
			current_id = cid;
			return (0);
		}
		cid++;
	}

	return (-EINVAL);
}

/* setup implementation */
static void
setup_impl(void)
{
	IMPL_OPS_T *x = supp_impl[1];
	(void) x;

	switch (selected_id) {
	case IMPL_FASTEST:
		set_impl_id(IMPL_FASTEST);
		break;
	case IMPL_CYCLE:
		set_impl_id(IMPL_CYCLE);
		break;
	default:
		set_impl_id(current_id);
		break;
	}
}

const zfs_impl_t ZFS_IMPL_OPS = {
	.name			= IMPL_NAME,
	.get_impl_count		= get_impl_count,
	.get_impl_id		= get_impl_id,
	.get_impl_name		= get_impl_name,
	.set_impl_fastest	= set_impl_fastest,
	.set_impl_id		= set_impl_id,
	.set_impl_name		= set_impl_name,
	.setup_impl		= setup_impl
};

#if defined(_KERNEL) && defined(__linux__)
static int
icp_impl_set(const char *name, zfs_kernel_param_t *kp)
{
	(void) name;
	(void) kp;

	return (0);
}

static int
icp_impl_get(char *buffer, zfs_kernel_param_t *kp)
{
	(void) buffer;
	(void) kp;

	IMPL_OPS_T *x = supp_impl[1];
	(void) x;

	return (0);
}

module_param_call(icp_impl, icp_impl_set, icp_impl_get,
    NULL, 0644);
MODULE_PARM_DESC(icp_impl, "Select " IMPL_NAME " implementation.");
#endif
