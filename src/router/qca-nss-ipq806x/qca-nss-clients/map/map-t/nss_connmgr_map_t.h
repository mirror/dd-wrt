/*
 **************************************************************************
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_connnmgr_map_t.h
 *
 * Header file implementation of nss_connnmgr_map_t.c
 */
#ifndef _NSS_CONNMGR_MAP_T_H_
#define _NSS_CONNMGR_MAP_T_H_

/*
 * Restrict max number of rules that can be configured for
 * a MAP-T device
 */
#define MAP_T_MAX_NUM_RULES_PER_MAP_T_INSTANCE 64
#define MAP_T_MIN_NUM_RULES_PER_MAP_T_INSTANCE 1

/*
 * MAP-T Flag.
 */
#define MAPT_FLAG_ADD_DUMMY_HDR 0x01

struct list_dev_to_map_t_rules_entry_t {
	struct list_head list;					/* list head */
	struct net_device *dev;					/* net device */
	struct nat46_xlate_rulepair *rule_set;			/* rule set */
	uint64_t *rule_status;					/* rule validation status */
	uint64_t rule_pair_count;				/* number of rules */
};

struct list_lookup_entry_t {
	struct list_head list;				/* ipv6 address list */
	struct nat46_xlate_rulepair *ptr_rule_set;	/* rule set */
};

struct list_ipv6_address_entry_t {
	struct in6_addr addr;	/* ipv6 address */
	u32 prefix_len;		/* prefix length */
	struct list_head list;	/* list head */
};

/*
 * Below enum indicates bits position in <DEBUG-FS>/map-t file. Each bit
 * reperesent an error of map-t rule. If all bits zero represents goodness
 * of a map-t rule
 */
enum {
	MAP_T_LOCAL_EA_BITS_LEN_IS_INVALID = 0,
	MAP_T_LOCAL_PSID_LEN_PLUS_PSID_OFFSET_IS_GREATER_THAN_16,
	MAP_T_LOCAL_IPV6_PREFIX_LEN_IS_NOT_32_40_48_56_64_OR_96,
	MAP_T_LOCAL_STYLE_IS_NOT_MAP_T_OR_RFC6052,
	MAP_T_REMOTE_EA_BITS_LEN_IS_INVALID,
	MAP_T_REMOTE_PSID_LEN_PLUS_REMOTE_PSID_OFFSET_IS_GREATER_THAN_16,
	MAP_T_REMOTE_IPV6_PREFIX_LEN_IS_NOT_32_40_48_56_64_OR_96,
	MAP_T_NO_IPV6_END_USER_PREFIX,
	MAP_T_REMOTE_STYLE_IS_NOT_MAP_T_OR_RFC6052,
	MAP_T_INVALID_RULE,
	MAP_T_RULE_ERROR_MAX,
};

/*
 *  AE configure/deconfigure errors
 */
enum {
	MAPT_AE_ERR_CONFIGURE = 1,		/* Error while configuring */
	MAPT_AE_ERR_DECONFIGURE,		/* Error while deconfiguring */
	MAPT_AE_ERR_MAX				/* Max error */
};

#endif
