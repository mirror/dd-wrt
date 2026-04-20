/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/skbuff.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include <ppe_drv.h>
#include <ppe_drv_cc.h>
#include <ppe_drv_acl.h>
#include <ppe_drv_port.h>
#include "ppe_mirror.h"

struct ppe_mirror gbl_ppe_mirror = {0};

/*
 * ppe_mirror_process_skb()
 *	Process mirrored packets from PPE.
 */
bool ppe_mirror_process_skb(void *appdata, struct sk_buff *skb, void *info)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_drv_cc_metadata *cc_info = (struct ppe_drv_cc_metadata *)info;
	struct ppe_mirror_acl_map *mirror_mapping = NULL;
	struct ppe_mirror_group_info *group_info = NULL;
	struct ppe_mirror_port_group_info *port_group_info = NULL;
	struct ppe_mirror_stats *acl_stats, *acl_group_stats = NULL;
	uint16_t hw_index = cc_info->acl_hw_index;
	bool acl_valid = cc_info->acl_index_valid;
	uint16_t acl_id;
	ppe_mirror_capture_callback_t cb = NULL;

	spin_lock_bh(&mirror_g->lock);

	/*
	 * if ACL id is not valid, then check if the physical dev
	 * mirror mapping is set.
	 */
	if (!acl_valid) {
		/*
		 * Check for the group info for physical dev mappings.
		 */
		port_group_info = &mirror_g->port_group_info;
		if (!port_group_info->group_dev) {
			spin_unlock_bh(&mirror_g->lock);
			ppe_mirror_warn("%p: No group is added for port mapping %s\n", mirror_g, skb->dev->name);
			return false;
		}

		/*
		 * Get the callback registered for this group and
		 * send the mirrored packet for further processing.
		 */
		cb = port_group_info->cb;
		if (!cb) {
			spin_unlock_bh(&mirror_g->lock);
			ppe_mirror_warn("%p: Callback is not associated with for a group \n", mirror_g);
			return false;
		}

		skb->dev = port_group_info->group_dev;
		spin_unlock_bh(&mirror_g->lock);

		/*
		 * Increment the packet count for this group to which this mirrored packet belongs to.
		 * Hand over the mirrored packet to user registered callback.
		 */
		ppe_mirror_update_stats(&port_group_info->pdev_stats, skb->len);

		cb(port_group_info->app_data, skb, port_group_info->group_dev);

		return true;
	}

	mirror_mapping = &mirror_g->mirror_mapping[hw_index];

	/*
	 * Check if ACL mirror mapping is valid or not.
	 */
	if (!mirror_mapping->is_valid) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mirror_process_mapping_invalid);
		ppe_mirror_warn("%p: Mirror mapping is not valid for hw index %d\n", mirror_g, hw_index);
		return false;
	}

	/*
	 * Get the group information from the mapping.
	 */
	acl_id = mirror_mapping->acl_rule_id;
	group_info = mirror_mapping->group_info;
	if (!group_info) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mirror_process_group_invalid);
		ppe_mirror_warn("%p: ACL id is not associated with a group %d hw index %d\n", mirror_g, acl_id, hw_index);
		return false;
	}

	/*
	 * Get the callback registered for this group and
	 * send the mirrored packet for further processing.
	 */
	cb = group_info->cb;
	if (!cb) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_warn("%p: Callback is not associated with a group %d hw index %d\n", mirror_g, acl_id, hw_index);
		return false;
	}

	skb->dev = group_info->group_dev;
	acl_stats = &mirror_mapping->acl_stats;
	acl_group_stats = &group_info->acl_stats;
	spin_unlock_bh(&mirror_g->lock);

	/*
	 * Increment the packet count for this ACL ID also for the
	 * group to which this mirrored packet belongs to.
	 * Hand over the mirrored packet to user registered callback.
	 */
	ppe_mirror_update_stats(acl_stats, skb->len);
	ppe_mirror_update_stats(acl_group_stats, skb->len);

	cb(group_info->app_data, skb, group_info->group_dev);

	return true;
}

/*
 * ppe_mirror_group_free()
 *	Free memory for a mirror group.
 */
static void ppe_mirror_group_free(struct ppe_mirror_group_info *group_info)
{
	/*
	 * Free memory for a group
	 */
	kfree(group_info);
}

/*
 * ppe_mirror_group_alloc()
 *	Allocate memory for a mirror group.
 */
static inline struct ppe_mirror_group_info *ppe_mirror_group_alloc(void)
{
	/*
	 * Allocate memory for a new group
	 *
	 * kzalloc with GFP_ATOMIC is used assuming ACL groups can be created
	 * in softirq context as well while processing an SKB in the system.
	 */
	return kzalloc(sizeof(struct ppe_mirror_group_info), GFP_ATOMIC);
}

/*
 * ppe_mirror_get_group_info_for_acl()
 *	Serach the group info for the ACL index.
 *	Create one if it does not exists.
 */
ppe_mirror_ret_t ppe_mirror_get_group_info_for_acl(struct ppe_mirror *mirror_g,
						   struct ppe_mirror_acl_mapping_info *map_info,
						   struct ppe_mirror_group_info **group_info)
{
	struct ppe_mirror_group_info *cur_group = NULL;
	uint16_t acl_id = map_info->acl_id;
	/*
	 * If active group list is empty, create a new group
	 * for this ACL id.
	 */
	if (list_empty(&mirror_g->active_mirror_groups))
		goto create_group;

	/*
	 * Traverse the list of active groups to find if group info
	 * is present or not - If not present, create one.
	 */
	list_for_each_entry(cur_group, &mirror_g->active_mirror_groups, list) {
		if (cur_group->group_dev == map_info->capture_dev) {
			ppe_mirror_info("%p: Group info: %p Group dev: %p found for ACL ID: %d",
						mirror_g, cur_group, cur_group->group_dev, acl_id);

			/*
			 * Check if in the new request the callback is different
			 * for this mirror group.
			 */
			if (cur_group->cb != map_info->cb) {
				ppe_mirror_warn("%p: Group info: %p cb: %p Missmatch for ACL ID: %d new cb %p:",
						mirror_g, cur_group, cur_group->cb, acl_id, map_info->cb);
				ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_invalid_group_info);
				return PPE_MIRROR_RET_ADD_FAIL_GROUP_INFO_INVALID;
			}

			*group_info = cur_group;
			return PPE_MIRROR_RET_SUCCESS;
		}
	}

create_group:
	/*
	 * Allocate a new group information for the ACL index.
	 */
	*group_info = ppe_mirror_group_alloc();
	if (!(*group_info)) {
		ppe_mirror_warn("%p: Falied to allocate a group for ACL id %d\n", mirror_g, acl_id);
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_map_nomem);
		return PPE_MIRROR_RET_ADD_FAIL_NO_MEM;
	}

	/*
	 * Fill the group information and add the group in the
	 * active group list.
	 */
	(*group_info)->group_dev = map_info->capture_dev;
	(*group_info)->cb = map_info->cb;
	(*group_info)->app_data = map_info->app_data;
	list_add(&(*group_info)->list, &mirror_g->active_mirror_groups);
	mirror_g->no_of_active_mirror_groups++;

	return PPE_MIRROR_RET_SUCCESS;
}

/*
 * ppe_mirror_disable_mirror_for_phy_dev()
 *	Stop mirroring for a physical dev.
 */
static ppe_mirror_ret_t ppe_mirror_disable_mirror_for_phy_dev(struct net_device *dev)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_drv_iface *iface = NULL;
	ppe_drv_ret_t ret;

	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		ppe_mirror_warn("Failed to get iface for interface %s\n", dev->name);
		return PPE_MIRROR_RET_GENERIC_FAILURE;
        }

	/*
	 * Disable mirroring on the physical port.
	 */
	ret = ppe_drv_dp_set_mirror_if(iface, PPE_DRV_DP_MIRR_DI_EG, false);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_mirror_info("%p:Failed to disable mirror mapping for phy dev %s %d\n", mirror_g, dev->name, ret);
		return PPE_MIRROR_RET_GENERIC_FAILURE;
	}

	ppe_mirror_info("Mapping deleted succesfully for dev %s\n", dev->name);

	return PPE_MIRROR_RET_SUCCESS;
}

/*
 * ppe_mirror_acl_destroy_mapping_table()
 *	Destroy mapping table for an ACL index.
 */
static ppe_mirror_ret_t ppe_mirror_destroy_mapping_tbl(uint16_t hw_index)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_acl_map *mirror_map = NULL;
	struct ppe_mirror_group_info *group_info = NULL;
	ppe_mirror_ret_t ret;

	spin_lock_bh(&mirror_g->lock);

	/*
	 * Check if this ACL rule has a valid mapping or not.
	 */
	mirror_map = &mirror_g->mirror_mapping[hw_index];
	if (!mirror_map->is_valid) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_warn("%p: Mirror mapping is not valid for hw index %d\n", mirror_g, hw_index);
		ret = PPE_MIRROR_RET_DELETE_FAIL_MAPPING_NOT_FOUND;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_fail_map_not_found);
		goto fail;
	}

	/*
	 * Unmap the ACL index to group mapping.
	 * If That group does not have any ACLs mapped, delete the group.
	 */
	group_info = mirror_map->group_info;
	if (!group_info) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_warn("%p:No mirror group found for hw index %d\n", mirror_g, hw_index);
		ret = PPE_MIRROR_RET_DELETE_FAIL_GROUP_NOT_FOUND;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_fail_group_not_found);
		goto fail;
	}

	/*
	 * Decrement the number of mappings for the group.
	 * If there are no mappings, destroy the group.
	 */
	group_info->number_of_mappings -= 1;
	if (!group_info->number_of_mappings) {
		list_del(&group_info->list);
		ppe_mirror_group_free(group_info);
		mirror_g->no_of_active_mirror_groups--;
		ppe_mirror_assert(mirror_g->no_of_active_mirror_groups >= 0, "Bad number of active mappings\n");
	}

	/*
	 * At this point, release the ACL to Group mapping.
	 */
	mirror_map->group_info = NULL;
	mirror_map->is_valid = false;

	spin_unlock_bh(&mirror_g->lock);
	ppe_mirror_info("%p:Released the mirror mapping for hw_index %d\n", mirror_g, hw_index);
	return PPE_MIRROR_RET_SUCCESS;

fail:
	return ret;
}

/*
 * ppe_mirror_enable_mirror_for_phy_dev()
 *	Configure mapping table for a physical dev.
 */
static ppe_mirror_ret_t ppe_mirror_enable_mirror_for_phy_dev(struct net_device *phy_dev)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_drv_iface *iface = NULL;
	ppe_drv_ret_t ret;

	iface = ppe_drv_iface_get_by_dev(phy_dev);
	if (!iface) {
		ppe_mirror_warn("Failed to get iface for interface %s\n", phy_dev->name);
		return PPE_MIRROR_RET_GENERIC_FAILURE;
        }

	/*
	 * Enable mirroring on the physical port.
	 */
	ret = ppe_drv_dp_set_mirror_if(iface, PPE_DRV_DP_MIRR_DI_EG, true);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_mirror_warn("%p:Failed to establish the mirror mapping for phy dev %s %d\n", mirror_g, phy_dev->name, ret);
		return PPE_MIRROR_RET_GENERIC_FAILURE;
	}

	ppe_mirror_info("%p:Establish the mirror mapping for phy dev %s\n", mirror_g, phy_dev->name);
	return PPE_MIRROR_RET_SUCCESS;
}

/*
 * ppe_mirror_acl_configure_mapping_table()
 *	Configure mapping table for an ACL ID.
 */
static ppe_mirror_ret_t ppe_mirror_configure_mapping_tbl(struct ppe_mirror_acl_mapping_info *map_info, uint16_t hw_index)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_acl_map *mirror_map = NULL;
	struct ppe_mirror_group_info *group_info = NULL;
	uint16_t acl_id = map_info->acl_id;
	ppe_mirror_ret_t ret;

	spin_lock_bh(&mirror_g->lock);

	/*
	 * Check if this ACL rule has a valid mapping or not.
	 */
	mirror_map = &mirror_g->mirror_mapping[hw_index];
	if (mirror_map->is_valid) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_warn("%p: Mirror mapping is already valid for ACL index %d hw index %d\n", mirror_g, acl_id, hw_index);
		ret = PPE_MIRROR_RET_ADD_FAIL_MAPPING_EXIST;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_map_exist);
		goto fail;
	}

	/*
	 * Check if the group dev info is establish for the device linked with
	 * this ACL id.
	 * If not create a group dev and link the same with this ACL index.
	 */
	ret = ppe_mirror_get_group_info_for_acl(mirror_g, map_info, &group_info);
	if (ret != PPE_MIRROR_RET_SUCCESS) {
		spin_unlock_bh(&mirror_g->lock);
		ppe_mirror_warn("%p: Failed to get group info for ACL %d hw index %d ret %d\n", mirror_g, acl_id, hw_index, ret);
		goto fail;
	}

	/*
	 * At this point, establish the mapping and increment the count.
	 */
	mirror_map->group_info = group_info;
	mirror_map->acl_rule_id = acl_id;
	mirror_map->is_valid = true;
	group_info->number_of_mappings += 1;

	spin_unlock_bh(&mirror_g->lock);

	ppe_mirror_info("%p:Establish the mirror mapping for ACL index %d hw index %d\n", mirror_g, acl_id, hw_index);
	return PPE_MIRROR_RET_SUCCESS;

fail:
	return ret;
}

/*
 * ppe_mirror_acl_mapping_delete()
 *	PPE mirror mapping delete exported API.
 */
ppe_mirror_ret_t ppe_mirror_acl_mapping_delete(uint16_t acl_id)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	uint16_t hw_index;
	ppe_mirror_ret_t ret;

	ppe_mirror_info("%p: Mapping delete request received for ACL ID: %d", mirror_g, acl_id);
	ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_req);

	/*
	 * Check for Invalid ACL index.
	 */
	if (acl_id < 0 || acl_id >= PPE_MIRROR_ACL_RULE_MAX) {
		ppe_mirror_warn("%p: Invalid rule id received for ACL rule mapping delete %d", mirror_g, acl_id);
		ret = PPE_MIRROR_RET_DELETE_FAIL_MAPPING_INVALID_ACL_ID;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_fail_invalid_rule_id);
		goto fail;
	}

	/*
	 * Get the Hardware index and destroy the mirror mapping for the that index.
	 */
	hw_index = ppe_acl_rule_get_acl_hw_index(acl_id);
	if ((hw_index == PPE_ACL_INVALID_HW_INDEX) ||
			(hw_index >= PPE_ACL_HW_INDEX_MAX)) {
		ppe_mirror_warn("%p: Invalid hw index for ACL rule delete %d", mirror_g, acl_id);
		ret = PPE_MIRROR_RET_DELETE_FAIL_MAPPING_INVALID_ACL_RULE;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_fail_rule_not_found);
		goto fail;
	}

	ret = ppe_mirror_destroy_mapping_tbl(hw_index);
	if (ret != PPE_MIRROR_RET_SUCCESS) {
		ppe_mirror_warn("%p: Failed to destroy mapping table ACL id %d hw index %d ret %d\n", mirror_g, acl_id, hw_index, ret);
		goto fail;
	}

	/*
	 * Deref the ACL rule.
	 */
	ppe_acl_rule_deref(acl_id);

	ppe_mirror_info("%p: Mapping deleted succesfully for ACL rule %d hw index %d\n", mirror_g, acl_id, hw_index);
	ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_del_success);
	ppe_mirror_stats_dec(&mirror_g->stats.acl_mapping_count);
	return PPE_MIRROR_RET_SUCCESS;

fail:
	return ret;
}
EXPORT_SYMBOL(ppe_mirror_acl_mapping_delete);

/*
 * ppe_mirror_phy_port_mapping_delete()
 *	PPE physical port mapping delete API.
 */
ppe_mirror_ret_t ppe_mirror_phy_port_mapping_delete(struct ppe_mirror_port_mapping_info *mapping_info)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_phy_port_list *port_list = mapping_info->mirror_phy_port_list;
	ppe_mirror_ret_t ret;
	uint8_t i = 0;

	/*
	 * Iterate over the list of ports and delete the mapping
	 * to the group dev.
	 */
	for (i = 0; i < PPE_DRV_PHYSICAL_MAX; i++) {
		if (!port_list[i].is_valid)
			break;

		ret = ppe_mirror_disable_mirror_for_phy_dev(port_list[i].dev);
		if (ret != PPE_MIRROR_RET_SUCCESS) {
			ppe_mirror_warn("%p: Failed to delete mapping table for physical dev %s ret %d\n", mirror_g, port_list[i].dev->name, ret);
			return ret;
		}
	}

	return PPE_MIRROR_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_mirror_phy_port_mapping_delete);

/*
 * ppe_mirror_phy_port_mapping_add()
 *	PPE physical port mapping add API.
 */
ppe_mirror_ret_t ppe_mirror_phy_port_mapping_add(struct ppe_mirror_port_mapping_info *mapping_info)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_port_group_info *group_info = NULL;
	struct ppe_mirror_phy_port_list *port_list = mapping_info->mirror_phy_port_list;
	ppe_mirror_ret_t ret;
	uint8_t i = 0;

	/*
	 * Change the group info if new device is sent in the
	 * mapping command or the group dev is NULL. Clear the earlier
	 * stats as well in case of group dev change.
	 */
	spin_lock_bh(&mirror_g->lock);
	group_info = &mirror_g->port_group_info;
	if ((!group_info->group_dev) ||
			(group_info->group_dev != mapping_info->capture_dev)) {
		atomic64_set(&group_info->pdev_stats.packets, 0);
		atomic64_set(&group_info->pdev_stats.bytes, 0);

		group_info->group_dev = mapping_info->capture_dev;
		group_info->cb = mapping_info->cb;
		group_info->app_data = mapping_info->app_data;
	}

	spin_unlock_bh(&mirror_g->lock);

	/*
	 * Iterate over the list of physical net devices and add the mapping to
	 * the group dev.
	 */
	for (i = 0; i < PPE_DRV_PHYSICAL_MAX; i++) {
		if (!port_list[i].is_valid)
			break;

		ret = ppe_mirror_enable_mirror_for_phy_dev(port_list[i].dev);
		if (ret != PPE_MIRROR_RET_SUCCESS) {
			ppe_mirror_warn("%p: Failed to add mapping table for physical dev %s ret %d\n", mirror_g, port_list[i].dev->name, ret);
			return ret;
		}
	}

	return PPE_MIRROR_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_mirror_phy_port_mapping_add);

/*
 * ppe_mirror_acl_mapping_add()
 *	PPE mirror mapping add API.
 */
ppe_mirror_ret_t ppe_mirror_acl_mapping_add(struct ppe_mirror_acl_mapping_info *mapping_info)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	uint16_t acl_id = mapping_info->acl_id;
	uint16_t hw_index;
	ppe_mirror_ret_t ret;

	ppe_mirror_info("%p: Mapping add request: %d", mirror_g, acl_id);
	ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_req);

	/*
	 * Check for invalid ACL ID received.
	 */
	if (mapping_info->acl_id < 0 || mapping_info->acl_id >= PPE_MIRROR_ACL_RULE_MAX) {
		ppe_mirror_warn("%p: Invalid rule id received for ACL rule mapping %d", mirror_g, mapping_info->acl_id);
		ret = PPE_MIRROR_RET_ADD_FAIL_MAPPING_INVALID_ACL_ID;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_fail_invalid_rule_id);
		goto fail2;
	}

	/*
	 * Get the hardware index for the ACL id.
	 */
	hw_index = ppe_acl_rule_get_acl_hw_index(acl_id);
	if ((hw_index == PPE_ACL_INVALID_HW_INDEX) ||
			(hw_index >= PPE_ACL_HW_INDEX_MAX)) {
		ppe_mirror_warn("%p: ACL Rule not found for ACL rule mapping %d", mirror_g, mapping_info->acl_id);
		ret = PPE_MIRROR_RET_ADD_FAIL_MAPPING_INVALID_ACL_RULE;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_fail_rule_not_found);
		goto fail2;
	}

	/*
	 * ref : while mapping an ACL rule.
	 * deref : while unmapping an ACL rule.
	 */
	if (!ppe_acl_rule_ref(acl_id)) {
		ppe_mirror_warn("%p: ACL Rule not found for ACL rule mapping %d", mirror_g, mapping_info->acl_id);
		ret = PPE_MIRROR_RET_ADD_FAIL_MAPPING_INVALID_ACL_RULE;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_fail_rule_not_found);
		goto fail2;
	}

	/*
	 * Establish the mirror mapping for this ACL ID.
	 */
	ret = ppe_mirror_configure_mapping_tbl(mapping_info, hw_index);
	if (ret != PPE_MIRROR_RET_SUCCESS) {
		ppe_mirror_warn("%p: Failed to configure mapping table ACL id %d ret %d\n", mirror_g, acl_id, ret);
		goto fail1;
	}

	ppe_mirror_info("%p: Mapping added succesfully for ACL rule %d hw index %d\n", mirror_g, acl_id, hw_index);
	ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_add_success);
	ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_count);
	return PPE_MIRROR_RET_SUCCESS;

fail1:
	ppe_acl_rule_deref(acl_id);
fail2:
	ppe_mirror_warn("%p: Failed to add a mapping for an ACL rule %d Ret %d\n", mirror_g, acl_id, ret);
	return ret;
}
EXPORT_SYMBOL(ppe_mirror_acl_mapping_add);

/*
 * ppe_mirror_enable_capture_core()
 */
ppe_mirror_ret_t ppe_mirror_enable_capture_core(uint8_t core_id)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	uint8_t ret;

	if (core_id < 0 || core_id >= PPE_MIRROR_CAPTURE_CORE_MAX) {
		ppe_mirror_warn("invalid core ID received %d\n", core_id);
		ret = PPE_MIRROR_RET_INVALID_CAPTURE_CORE;
                ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_invalid_capture_core);
		return ret;
	}

	if (!ppe_drv_acl_enable_mirror_capture_core(core_id)) {
		ppe_mirror_warn("Failed to enable capture core %d\n", core_id);
		ret = PPE_MIRROR_RET_FAIL_EN_CAPTURE_CORE;
		ppe_mirror_stats_inc(&mirror_g->stats.acl_mapping_fail_en_capture_core);
		return ret;
	}

	return PPE_MIRROR_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_mirror_enable_capture_core);

/*
 * ppe_mirror_deinit()
 *	PPE mirror deinit API.
 */
void ppe_mirror_deinit(void)
{
	ppe_mirror_stats_debugfs_exit();

	/*
	 * Unregister CPU code callbacks for ingress and egress mirrored packets.
	 */
	ppe_drv_cc_unregister_cb(PPE_DRV_CC_CPU_CODE_EG_MIRROR);
	ppe_drv_cc_unregister_cb(PPE_DRV_CC_CPU_CODE_IN_MIRROR);
}

/*
 * ppe_mirror_init()
 *	PPE Mirror init API.
 */
void ppe_mirror_init(struct dentry *d_rule)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	spin_lock_init(&mirror_g->lock);

	/*
	 * Initialize active group list
	 */
	INIT_LIST_HEAD(&mirror_g->active_mirror_groups);

	/*
	 * Register CPU code callbacks for ingress and egress mirrored packets.
	 */
	ppe_drv_cc_register_cb(PPE_DRV_CC_CPU_CODE_EG_MIRROR, ppe_mirror_process_skb, NULL);
	ppe_drv_cc_register_cb(PPE_DRV_CC_CPU_CODE_IN_MIRROR, ppe_mirror_process_skb, NULL);

	/*
	 * Create debugfs directories/files.
	 */
	ppe_mirror_stats_debugfs_init(d_rule);
}
