/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/sysctl.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "ppe_mirror.h"

/*
 * Maxmum number of capture groups supported by test module.
 */
#define PPE_MIRROR_TEST_GROUP_MAX	8

/*
 * Invalid group ID.
 */
#define PPE_MIRROR_TEST_INVALID_GROUP_ID	0xFFFF

/*
 * Number of parameters in a input command.
 */
#define PPE_MIRROR_TEST_NO_OF_PARAMS	2

/*
 * Maximum number of ACL rule ids.
 */
#define PPE_MIRROR_TEST_ACL_MAX		2048

/*
 * Maxmimum buffer for input command.
 */
#define PPE_MIRROR_TEST_CMD_STR		200

/*
 * ppe_mirror_test_cmd_type
 *	PPE mirror test command types.
 */
enum ppe_mirror_test_cmd_type {
	PPE_MIRROR_TEST_CMD_UNKNOWN,		/* Unknown command */
	PPE_MIRROR_TEST_CMD_CREATE_DEV,		/* Create dev command */
	PPE_MIRROR_TEST_CMD_DESTROY_DEV,	/* Destroy dev command */
	PPE_MIRROR_TEST_CMD_MAP_ACL,		/* MAP ACL to group command */
	PPE_MIRROR_TEST_CMD_UNMAP_ACL,		/* UNMAP ACL to group command */
	PPE_MIRROR_TEST_CMD_ENABLE_CORE,	/* Command to enable capture core */
	PPE_MIRROR_TEST_CMD_DROP,		/* Command to drop the mirrored packets in test module */
};

/*
 * ppe_mirror_test_group_dev
 *	Mirror test group information.
 */
struct ppe_mirror_test_group_dev {
	struct net_device *dev;		/* Group net device */
	bool is_valid;			/* Group is valid */
};
