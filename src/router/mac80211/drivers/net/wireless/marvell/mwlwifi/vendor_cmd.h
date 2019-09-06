/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file defines vendor constants and register function. */

#ifndef _VENDOR_CMD_H_
#define _VENDOR_CMD_H_

#ifdef __KERNEL__
void vendor_cmd_register(struct wiphy *wiphy);
void vendor_cmd_basic_event(struct wiphy *wiphy, int event_idx);
#endif

#define MRVL_OUI        0x005043

enum mwl_vendor_commands {
	MWL_VENDOR_CMD_SET_BF_TYPE,
	MWL_VENDOR_CMD_GET_BF_TYPE,

	/* add commands here, update the command in vendor_cmd.c */

	__MWL_VENDOR_CMD_AFTER_LAST,
	NUM_MWL_VENDOR_CMD = __MWL_VENDOR_CMD_AFTER_LAST,
	MWL_VENDOR_CMD_MAX = __MWL_VENDOR_CMD_AFTER_LAST - 1
};

enum mwl_vendor_attributes {
	MWL_VENDOR_ATTR_NOT_USE,
	MWL_VENDOR_ATTR_BF_TYPE,

	/* add attributes here, update the policy in vendor_cmd.c */

	__MWL_VENDOR_ATTR_AFTER_LAST,
	NUM_MWL_VENDOR_ATTR = __MWL_VENDOR_ATTR_AFTER_LAST,
	MWL_VENDOR_ATTR_MAX = __MWL_VENDOR_ATTR_AFTER_LAST - 1
};

enum mwl_vendor_events {
	MWL_VENDOR_EVENT_DRIVER_READY,
	MWL_VENDOR_EVENT_DRIVER_START_REMOVE,
	MWL_VENDOR_EVENT_CMD_TIMEOUT,

	__MWL_VENDOR_EVENT_AFTER_LAST,
	NUM_MWL_VENDOR_EVENT = __MWL_VENDOR_EVENT_AFTER_LAST,
	MWL_VENDOR_EVENT_MAX = __MWL_VENDOR_EVENT_AFTER_LAST - 1
};

#endif /* _VENDOR_CMD_H_ */
