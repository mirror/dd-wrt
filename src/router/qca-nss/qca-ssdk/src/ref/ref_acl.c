/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "ref_acl.h"
#include "ssdk_init.h"
#include "ssdk_plat.h"
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>

/* entry 0-1 is for global deny all and accept eapol rule
   entry 2-9 is for phy port1 specific mac accept rule
   ...
   entry 42-49 is for phy port6 specific mac accept rule
*/
#define PPE_ACL_MAC_ENTRY_MAX 50

/* list 64 is for global deny all and accept eapol rule
   list 65 is for phy port1 specific mac accept rule
   ...
   list 70 is for phy port6 specific mac accept rule
*/
#define PPE_ACL_MAC_ENTRY_LIST_START 64

#define PPE_ACL_MAC_ENTRY_LIST_PRI 60

/* permit eapol rule pri */
#define PPE_ACL_MAC_ENTRY_RULE_EAPOL_PRI 2

/* deny all mac rule pri */
#define PPE_ACL_MAC_ENTRY_RULE_DENY_PRI 0

/* accept per mac rule pri */
#define PPE_ACL_MAC_ENTRY_RULE_ACCEPT_PRI 1

#define PORT_TO_START_ENTRY_ID(port) (((port - 1) << 3) + 2)
#define PORT_TO_END_ENTRY_ID(port) ((port << 3) + 2)
#define ENTRY_ID_TO_RULE_ID(id) ((id - 2)%8)
#define is_deny_all_mac(a) is_broadcast_ether_addr(a)

static ref_acl_mac_entry_t
ref_acl_mac_entry[SW_MAX_NR_DEV][PPE_ACL_MAC_ENTRY_MAX] = {0};

static a_bool_t
_ref_acl_mac_entry_valid(a_uint32_t dev_id, a_uint32_t entry_idx)
{
	return !SW_IS_PBMP_EQ(ref_acl_mac_entry[dev_id][entry_idx].port_map, 0);
}

static sw_error_t
_ref_acl_mac_entry_find(a_uint32_t dev_id,
		fal_acl_mac_entry_t * entry, a_uint32_t * entry_idx)
{
	a_uint32_t find_idx_start = 0, find_idx_end = 0;
	a_uint32_t index, empty_index = PPE_ACL_MAC_ENTRY_MAX;
	a_uint32_t port_id = ssdk_ifname_to_port(dev_id, entry->ifname);

	if (port_id < SSDK_PHYSICAL_PORT1 || port_id > SSDK_PHYSICAL_PORT6)
	{
		SSDK_ERROR("port_id %d\n", port_id);
		return SW_OUT_OF_RANGE;
	}
	if (is_deny_all_mac(entry->src_mac.uc))
	{
		find_idx_start = 0;
		find_idx_end = 1;
	}
	else
	{
		find_idx_start = PORT_TO_START_ENTRY_ID(port_id);
		find_idx_end = PORT_TO_END_ENTRY_ID(port_id);
	}

	SSDK_DEBUG("ifname %s port_id %d find_idx_start %d find_idx_end %d\n",
			entry->ifname, port_id, find_idx_start, find_idx_end);
	for (index = find_idx_start; index < find_idx_end; index++)
	{
		if (!_ref_acl_mac_entry_valid(dev_id, index))
		{
			if (empty_index == PPE_ACL_MAC_ENTRY_MAX)
			{
				empty_index = index;
			}
		}
		if (_ref_acl_mac_entry_valid(dev_id, index) &&
				ether_addr_equal(entry->src_mac.uc,
				ref_acl_mac_entry[dev_id][index].src_mac.uc))
		{
			/* find */
			*entry_idx = index;
			return SW_OK;
		}
	}
	if (empty_index == PPE_ACL_MAC_ENTRY_MAX)
	{
		/* not find and table if full */
		*entry_idx = empty_index;
		return SW_NO_RESOURCE;
	}
	else
	{
		/* not find, return the first empty entry */
		*entry_idx = empty_index;
		return SW_NOT_FOUND;
	}
}

static sw_error_t
_ref_acl_mac_entry_create_rule(a_uint32_t dev_id,
		fal_acl_mac_entry_t * entry, a_uint32_t entry_idx)
{
	sw_error_t rv = SW_OK;
	fal_acl_rule_t rule = {0};
	struct net_device *eth_dev = NULL;
	a_uint32_t port_id = ssdk_ifname_to_port(dev_id, entry->ifname);
	SSDK_DEBUG("port_id %d entry_idx %d\n", port_id, entry_idx);

	if (port_id < SSDK_PHYSICAL_PORT1 || port_id > SSDK_PHYSICAL_PORT6)
	{
		return SW_OUT_OF_RANGE;
	}
	if (entry_idx >= PPE_ACL_MAC_ENTRY_MAX)
	{
		SSDK_ERROR("no resource to create new rule\n");
		return SW_NO_RESOURCE;
	}

	if (is_deny_all_mac(entry->src_mac.uc) && !entry->acl_policy)
	{
		if (entry_idx != 0)
		{
			SSDK_ERROR("entry index 0 should reserved for deny all\n");
			return SW_FAIL;
		}
		/* create deny all mac and accept eapol rule */
		rv = fal_acl_list_creat(dev_id, PPE_ACL_MAC_ENTRY_LIST_START,
				PPE_ACL_MAC_ENTRY_LIST_PRI);
		if (rv != SW_OK && rv != SW_ALREADY_EXIST)
		{
			return rv;
		}

		aos_mem_zero(&rule, sizeof(fal_acl_rule_t));
		rule.rule_type = FAL_ACL_RULE_MAC;
		rule.pri = PPE_ACL_MAC_ENTRY_RULE_DENY_PRI;
		FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_DENY);
		rv = fal_acl_rule_add(dev_id, PPE_ACL_MAC_ENTRY_LIST_START, 0, 1, &rule);
		SW_RTN_ON_ERROR(rv);

		FAL_ACTION_FLG_CLR(rule.action_flg, FAL_ACL_ACTION_DENY);
		rule.ethtype_val = ETH_P_PAE;
		rule.ethtype_mask = 0xffff;
		rule.pri = PPE_ACL_MAC_ENTRY_RULE_EAPOL_PRI;
		FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
		FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_PERMIT);
		rv = fal_acl_rule_add(dev_id, PPE_ACL_MAC_ENTRY_LIST_START, 1, 1, &rule);
		SW_RTN_ON_ERROR(rv);

		rv = fal_acl_list_bind(dev_id, PPE_ACL_MAC_ENTRY_LIST_START,
				FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, BIT(port_id));
		SW_RTN_ON_ERROR(rv);

		/* update the ref_acl_mac_entry */
		/* deny all mac rule */
		ether_addr_copy(ref_acl_mac_entry[dev_id][0].src_mac.uc, entry->src_mac.uc);
		ref_acl_mac_entry[dev_id][0].port_map = BIT(port_id);
		ref_acl_mac_entry[dev_id][0].acl_policy = 0;
		/* eapol rule */
		ref_acl_mac_entry[dev_id][1].port_map = BIT(port_id);
		ref_acl_mac_entry[dev_id][1].acl_policy = 1;

		SSDK_DEBUG("ref_acl idx 0: port_map %x, acl_policy %d\n",
			ref_acl_mac_entry[dev_id][0].port_map,
			ref_acl_mac_entry[dev_id][0].acl_policy);
		SSDK_DEBUG("ref_acl idx 1: port_map %x, acl_policy %d\n",
			ref_acl_mac_entry[dev_id][1].port_map,
			ref_acl_mac_entry[dev_id][1].acl_policy);
	}
	else if (!is_deny_all_mac(entry->src_mac.uc) && entry->acl_policy)
	{
		rv = fal_acl_list_creat(dev_id, PPE_ACL_MAC_ENTRY_LIST_START + port_id,
					PPE_ACL_MAC_ENTRY_LIST_PRI);
		if (rv != SW_OK && rv != SW_ALREADY_EXIST)
		{
			return rv;
		}

		aos_mem_zero(&rule, sizeof(fal_acl_rule_t));
		rule.rule_type = FAL_ACL_RULE_MAC;
		rule.pri = PPE_ACL_MAC_ENTRY_RULE_ACCEPT_PRI;
		ether_addr_copy(rule.src_mac_val.uc, entry->src_mac.uc);
		eth_broadcast_addr(rule.src_mac_mask.uc);

		FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_MAC_SA);
		FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_PERMIT);
		rv = fal_acl_rule_add(dev_id, PPE_ACL_MAC_ENTRY_LIST_START + port_id,
				ENTRY_ID_TO_RULE_ID(entry_idx), 1, &rule);
		SW_RTN_ON_ERROR(rv);

		rv = fal_acl_list_bind(dev_id, PPE_ACL_MAC_ENTRY_LIST_START + port_id,
				FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, BIT(port_id));
		SW_RTN_ON_ERROR(rv);

		/* update the ref_acl_mac_entry */
		ether_addr_copy(ref_acl_mac_entry[dev_id][entry_idx].src_mac.uc,
				entry->src_mac.uc);
		ref_acl_mac_entry[dev_id][entry_idx].port_map = BIT(port_id);
		ref_acl_mac_entry[dev_id][entry_idx].acl_policy = 1;
	}
	else if (!is_deny_all_mac(entry->src_mac.uc) && !entry->acl_policy)
	{
		eth_dev = dev_get_by_name(&init_net, entry->ifname);
		if (eth_dev)
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
			// TODO: replace with corresponding ver
#else
			br_fdb_delete_by_netdev(eth_dev, entry->src_mac.uc, 0);
#endif
			dev_put(eth_dev);
		}
	}
	return rv;
}

static sw_error_t
_ref_acl_mac_entry_update_rule(a_uint32_t dev_id,
		fal_acl_mac_entry_t * entry, a_uint32_t entry_idx)
{
	sw_error_t rv = SW_OK;
	a_uint32_t index = 0;
	a_uint32_t port_id = ssdk_ifname_to_port(dev_id, entry->ifname);
	SSDK_DEBUG("port_id %d entry_idx %d\n", port_id, entry_idx);

	if (port_id < SSDK_PHYSICAL_PORT1 || port_id > SSDK_PHYSICAL_PORT6)
	{
		return SW_OUT_OF_RANGE;
	}
	if (is_deny_all_mac(entry->src_mac.uc))
	{
		if (entry_idx != 0)
		{
			SSDK_ERROR("entry index 0 should reserved for deny all\n");
			return SW_FAIL;
		}
		if (!entry->acl_policy)
		{
			/* update deny all on port_id */
			SW_PBMP_ADD_PORT(ref_acl_mac_entry[dev_id][0].port_map, port_id);
			SW_PBMP_ADD_PORT(ref_acl_mac_entry[dev_id][1].port_map, port_id);
			rv = fal_acl_list_bind(dev_id, PPE_ACL_MAC_ENTRY_LIST_START,
					FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP,
					ref_acl_mac_entry[dev_id][0].port_map);
		}
		else
		{
			/* accept all on port_id */
			SW_PBMP_DEL_PORT(ref_acl_mac_entry[dev_id][0].port_map, port_id);
			SW_PBMP_DEL_PORT(ref_acl_mac_entry[dev_id][1].port_map, port_id);
			rv = fal_acl_list_unbind(dev_id, PPE_ACL_MAC_ENTRY_LIST_START,
					FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, BIT(port_id));
			SW_RTN_ON_ERROR(rv);
			rv = fal_acl_list_destroy(dev_id,
					PPE_ACL_MAC_ENTRY_LIST_START + port_id);
			SW_RTN_ON_ERROR(rv);
			for (index = PORT_TO_START_ENTRY_ID(port_id);
					index < PORT_TO_END_ENTRY_ID(port_id); index ++)
			{
				/* set invalid */
				SW_PBMP_CLEAR(ref_acl_mac_entry[dev_id][index].port_map);
			}
			if (SW_IS_PBMP_EQ(ref_acl_mac_entry[dev_id][0].port_map, 0))
			{
				rv = fal_acl_list_destroy(dev_id, PPE_ACL_MAC_ENTRY_LIST_START);
			}
		}
	}
	else
	{
		if (!entry->acl_policy)
		{
			/* deny specific mac on port_id */
			SSDK_DEBUG("Delete rule: port_id %d  list_id %d rule_id %d\n",
				port_id, PPE_ACL_MAC_ENTRY_LIST_START + port_id,
				ENTRY_ID_TO_RULE_ID(entry_idx));
			rv = fal_acl_rule_delete(dev_id, PPE_ACL_MAC_ENTRY_LIST_START + port_id,
						ENTRY_ID_TO_RULE_ID(entry_idx), 1);
			SW_PBMP_CLEAR(ref_acl_mac_entry[dev_id][entry_idx].port_map);
		}
	}
	return rv;
}

sw_error_t
ref_acl_mac_entry_set(a_uint32_t dev_id, fal_acl_mac_entry_t * entry)
{
	a_uint32_t entry_idx = 0;
	sw_error_t rv = SW_OK;
	SSDK_DEBUG("setting deny unless accept policy rules...\n");

	rv = _ref_acl_mac_entry_find(dev_id, entry, &entry_idx);
	switch(rv)
	{
		case SW_NOT_FOUND:
			rv = _ref_acl_mac_entry_create_rule(dev_id, entry, entry_idx);
			break;
		case SW_OK:
			rv = _ref_acl_mac_entry_update_rule(dev_id, entry, entry_idx);
			break;
		case SW_NO_RESOURCE:
			SSDK_ERROR("table is full, each port only support 8 mac entries!\n");
			break;
		case SW_OUT_OF_RANGE:
			SSDK_ERROR("port id is out of range, only support port1-port6!\n");
			break;
		default:
			;
	}
	return rv;
}

sw_error_t
ref_acl_mac_entry_dump(a_uint32_t dev_id)
{
	a_uint32_t index = 0, port_map = 0, port_id = 0;

	if (_ref_acl_mac_entry_valid(dev_id, 0))
	{
		port_map = ref_acl_mac_entry[dev_id][0].port_map;
		aos_printk("acl deny unless accept policy are applied on portbitmap 0x%x\n",
			port_map);
	}
	else
	{
		aos_printk("acl deny unless accept mac entries are empty\n");
	}

	for (port_id = SSDK_PHYSICAL_PORT1; port_id <= SSDK_PHYSICAL_PORT6; port_id ++)
	{
		if (SW_IS_PBMP_MEMBER(port_map, port_id))
		{
			aos_printk("port %d accept mac entries:\n", port_id);
			for (index = PORT_TO_START_ENTRY_ID(port_id);
					index < PORT_TO_END_ENTRY_ID(port_id); index ++)
			{
				if (_ref_acl_mac_entry_valid(dev_id, index))
				{
					aos_printk("		index %d", index);
					aos_printk(KERN_CONT " macaddr " SW_MACSTR "\n",
					SW_MAC2STR(ref_acl_mac_entry[dev_id][index].src_mac.uc));
				}
			}
		}
	}
	return SW_OK;
}

EXPORT_SYMBOL(ref_acl_mac_entry_set);
/**
 * @}
 */
