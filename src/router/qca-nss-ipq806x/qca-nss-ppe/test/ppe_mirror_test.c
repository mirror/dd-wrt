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


#include <linux/debugfs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/version.h>
#include "ppe_mirror_test.h"

/*
 * Procfs entry for test module.
 */
static struct ctl_table_header *ppe_mirror_test_procfs_header;

/*
 * Disable packet drop by default.
 */
bool ppe_mirror_test_drop_packets = false;

static DEFINE_SPINLOCK(ppe_mirror_test_group_lock);

/*
 * ppe_mirror_test_group
 *	test group information.
 *	Maximum 8 such groups are supported in this module.
 */
static struct ppe_mirror_test_group_dev ppe_mirror_test_group[PPE_MIRROR_TEST_GROUP_MAX] = {0};

/*
 * ppe_mirror_test_acl_arr
 *	ACL id to group id mapping.
 */
static uint16_t ppe_mirror_test_acl_arr[PPE_MIRROR_TEST_ACL_MAX];

/*
 * ppe_mirror_test_data
 *	Buffer to get the user command.
 */
static unsigned char ppe_mirror_test_data[PPE_MIRROR_TEST_CMD_STR] __read_mostly;

/*
 * ppe_mirror_test_netdev_up()
 *	API to make the net device up.
 */
static int ppe_mirror_test_netdev_up(struct net_device *dev)
{
	dev->flags |= IFF_RUNNING;
	netif_start_queue(dev);
	return 0;
}

/*
 * ppe_mirror_test_netdev_down()
 *	API to make the net device down.
 */
static int ppe_mirror_test_netdev_down(struct net_device *dev)
{
	dev->flags &= ~IFF_RUNNING;
	netif_stop_queue(dev);
	return 0;
}

/*
 * ppe_mirror_test_xmit()
 *	API for netdev xmit.
 */
static int ppe_mirror_test_xmit(struct sk_buff  *skb, struct net_device  *netdev)
{
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * ppe_mirror_test_netdev_ops
 *	Net device ops.
 */
static const struct net_device_ops ppe_mirror_test_netdev_ops = {
	.ndo_open		= ppe_mirror_test_netdev_up,
	.ndo_stop		= ppe_mirror_test_netdev_down,
	.ndo_start_xmit		= ppe_mirror_test_xmit,
	.ndo_get_stats64	= NULL,
};

/*
 * ppe_mirror_test_get_netdev_list()
 *	API to get the netdev list input.
 *
 * Note: Caller is expected to release the hold on the dev.
 */
static bool ppe_mirror_test_get_netdev_list(char *name, struct ppe_mirror_phy_port_list *port_list, bool *valid)
{
	char *dev = NULL;
	struct net_device *netdev = NULL;
	uint8_t i = 0;

	do {
		dev = strsep(&name, ",");
		if (!dev)
			break;

		if (dev[strlen(dev) - 1] == '\n')
			dev[strlen(dev) - 1] = '\0';

		netdev = dev_get_by_name(&init_net, dev);
		if (!netdev) {
			printk("Error in fetching dev %s\n", dev);
			return false;
		}

		port_list[i].dev = netdev;
		port_list[i].is_valid = true;

		dev_put(netdev);
		i++;

	} while (dev && i < 6);

	*valid = true;
	return true;
}

/*
 * ppe_mirror_test_get_netdev_by_name()
 *	API to get netdev from name.
 *
 * Note: Caller is expected to release the hold on the dev.
 */
static bool ppe_mirror_test_get_netdev_by_name(char *name, struct net_device **dev)
{
	char dev_name[IFNAMSIZ] = {0};

	strlcpy(dev_name, name, IFNAMSIZ);
	if (dev_name[strlen(dev_name) - 1] == '\n') {
		dev_name[strlen(dev_name) - 1] = '\0';
	}

	*dev = dev_get_by_name(&init_net, dev_name);
	if (!*dev) {
		return false;
	}

	return true;
}

/*
 * ppe_mirror_test_dev_destroy()
 *      API to un-register and free group net device.
 */
static void ppe_mirror_test_dev_destroy(struct net_device *dev)
{
	if (!dev) {
		printk("Invalid mirror device pointer\n");
		return;
	}

	unregister_netdev(dev);
	free_netdev(dev);
}

/*
 * ppe_mirror_test_read_nextarg()
 *	API to read the next argument in the command.
 */
static char *ppe_mirror_test_read_nextarg(char **buf_ptr)
{
	if (!buf_ptr || !(*buf_ptr)) {
		printk("Read Buf is NULL\n");
		return NULL;
	}

	return strsep(buf_ptr, " ");
}

/*
 * ppe_mirror_test_read_value()
 *	API to read a value of the param in the command.
 */
static char *ppe_mirror_test_read_value(char **buf_ptr, char **value_ptr, char *delim)
{
	*value_ptr = ppe_mirror_test_read_nextarg(buf_ptr);

	if (!(*value_ptr)) {
		return NULL;
	}

	return strsep(value_ptr, delim);
}

/*
 * ppe_mirror_test_convert_char_to_u16()
 *	API to convert character to u16.
 */
static int ppe_mirror_test_convert_char_to_u16(char *buf, uint16_t *arg)
{
	int ret;

	/*
	 * Write the tokens to unsigned short integer.
	 */
	ret = sscanf(buf, "%hu", arg);
	if (ret != 1) {
		printk("Failed to write the %s token to u16\n", buf);
		return -1;
	}
	return 0;
}

/*
 * ppe_mirror_test_group_cb_process_skb()
 *	Function to be registered with mirror module for
 *	receiving packets.
 */
void ppe_mirror_test_group_cb_process_skb(void *app_data, struct sk_buff *skb, struct net_device *dev)
{
	/*
	 * Drop the packet if ppe_mirror_test_drop_packets is set.
	 */
	if (ppe_mirror_test_drop_packets) {
		if (net_ratelimit())
			printk("Dropping mirrored skb %p \n", skb);

		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * Process the packet and send it to stack.
	 * NOTE : This is dummy API for test PPE mirror functionality,
	 * ideally the packet has to be dropped by the registerent and not
	 * reinject to stack.
	 */
	skb->dev = dev;
	skb->skb_iif = dev->ifindex;
	skb->protocol = eth_type_trans(skb, dev);

	netif_receive_skb(skb);
	return;
}

/*
 * ppe_mirror_test_group_get_group_id()
 *      Get the group id for the group device.
 */
static int ppe_mirror_test_group_get_group_id(struct net_device *dev)
{
	uint8_t i;

	spin_lock(&ppe_mirror_test_group_lock);
	for (i = 0; i < 8; i++) {
		if (!ppe_mirror_test_group[i].is_valid)
			continue;

		/*
		 * return the index for the group.
		 */
		if (ppe_mirror_test_group[i].dev == dev) {
			spin_unlock(&ppe_mirror_test_group_lock);
			return i;
		}

	}

	spin_unlock(&ppe_mirror_test_group_lock);
	printk("No group id found for the dev %p \n", dev);
	return PPE_MIRROR_TEST_INVALID_GROUP_ID;
}

/*
 * ppe_mirror_test_group_delete()
 *	Delelte the group from global array.
 */
static bool ppe_mirror_test_group_delete(struct net_device *dev)
{
	uint8_t i;

	spin_lock(&ppe_mirror_test_group_lock);
	for (i = 0; i < 8; i++) {
		if (!ppe_mirror_test_group[i].is_valid)
			continue;

		/*
		 * Delete the test group.
		 */
                if (ppe_mirror_test_group[i].dev == dev) {
			ppe_mirror_test_group[i].is_valid = false;
			ppe_mirror_test_group[i].dev = NULL;
		}

		spin_unlock(&ppe_mirror_test_group_lock);
		return true;
	}

	spin_unlock(&ppe_mirror_test_group_lock);
	printk("No mapping found for the dev %p \n", dev);
	return false;
}

/*
 * ppe_mirror_test_group_add()
 *	API to add group netdev in global array.
 */
static bool ppe_mirror_test_group_add(struct net_device *dev)
{
	uint8_t i;

	spin_lock(&ppe_mirror_test_group_lock);
	for (i = 0; i < 8; i++) {
		if (ppe_mirror_test_group[i].is_valid)
                        continue;

		/*
                 * Add interface in global array.
		 */
		ppe_mirror_test_group[i].dev = dev;
		ppe_mirror_test_group[i].is_valid = true;

		spin_unlock(&ppe_mirror_test_group_lock);
		return true;
        }

	spin_unlock(&ppe_mirror_test_group_lock);
	printk("Maximum number of groups are already created \n");

	return false;
}

/*
 * ppe_mirror_test_parse_config_param()
 *	API to parse the params of the command.
 */
static bool ppe_mirror_test_parse_config_param(char *buffer, uint16_t *acl_id,
					      struct net_device **dev,
					      struct ppe_mirror_phy_port_list *dev_list,
					      bool *valid)
{
	char *param, *value;
	uint8_t param_num = PPE_MIRROR_TEST_NO_OF_PARAMS;

	/*
	 * Parse the number of params one by one and
	 * pass the information back to caller.
	 */
	do {
		param = ppe_mirror_test_read_value(&buffer, &value, "=");

		/*
		 * No configure parameter is given. Use the default configure values.
		 */
		if (!param || !value) {
			return false;
		}

		if (!strcmp(param, "acl_id")) {
			if (ppe_mirror_test_convert_char_to_u16(value, acl_id)) {
				printk("Not able to parse %s to u16\n", value);
				return false;
                        }
		} else if (!strcmp(param, "group")) {

			/*
			 * Check if the dev is present or not.
			 */
			if (!ppe_mirror_test_get_netdev_by_name(value, dev)) {
				printk("Netdev group is not present %s.\n", value);
				return false;
			}

			dev_put(*dev);
		} else if (!strcmp(param, "port")){

			/*
			 * Get the netdev list from the command.
			 */
			if (!ppe_mirror_test_get_netdev_list(value, dev_list, valid)) {
				printk("Error in parsing netdev list!\n");
				return false;
			}
		} else {
			printk("invalid parametes for mapping add !!\n");
			return false;
		}
	} while (--param_num);

	return true;
}

/*
 * ppe_mirror_test_parse_map_cmd()
 *	Parse the map command.
 */
static bool ppe_mirror_test_parse_map_cmd(char *buffer)
{
	struct net_device *dev;
	struct ppe_mirror_acl_mapping_info acl_info = {0};
	struct ppe_mirror_port_mapping_info port_info = {0};
	uint16_t acl_id = 0;
	uint16_t group_id = PPE_MIRROR_TEST_INVALID_GROUP_ID;
	ppe_mirror_ret_t ret;
	bool port_based_valid = false;

	/*
	 * Parse the params from the command : acl id and dev.
	 */
	if (!ppe_mirror_test_parse_config_param(buffer, &acl_id, &dev, port_info.mirror_phy_port_list, &port_based_valid)) {
		printk("Error in parsing mirror configure command\n");
		return false;
        }

	/*
	 * get the group id for the group dev.
	 */
	group_id = ppe_mirror_test_group_get_group_id(dev);
	if (group_id == PPE_MIRROR_TEST_INVALID_GROUP_ID) {
		printk("Invalid group id %p\n", dev);
		return false;
	}

	/*
	 * Port based mapping is valid.
	 */
	if (port_based_valid) {
		port_info.capture_dev = dev;
		port_info.cb = ppe_mirror_test_group_cb_process_skb;

		ret = ppe_mirror_phy_port_mapping_add(&port_info);
		if (ret != PPE_MIRROR_RET_SUCCESS) {
			printk("Failed to add pdev mapping for mirror, ret %d\n", ret);
			return false;
		}

		return true;
	}

	/*
	 * Call into PPE mirror to add the mapping.
	 */
	acl_info.acl_id = acl_id;
	acl_info.capture_dev = dev;
	acl_info.cb = ppe_mirror_test_group_cb_process_skb;

	ret = ppe_mirror_acl_mapping_add(&acl_info);
	if (ret != PPE_MIRROR_RET_SUCCESS) {
		printk("Failed to add ACL mapping for ACL id %d ret %d\n", acl_id, ret);
		return false;
	}

	/*
	 * After mapping is done successfully, add the same into
	 * the test mirror array.
	 */
	ppe_mirror_test_acl_arr[acl_id] = group_id;
	printk(" Mapping added ACL ID: %d, group dev: %s \n", acl_id, dev->name);

        return true;
}

/*
 * ppe_mirror_test_parse_cmd()
 *	API to parse config commands.
 */
static int32_t ppe_mirror_test_parse_cmd(char *cmd)
{
	if (cmd == NULL) {
		return PPE_MIRROR_TEST_CMD_UNKNOWN;
	}

	if (!strcmp(cmd, "create")) {
		return PPE_MIRROR_TEST_CMD_CREATE_DEV;
	}

	if (!strcmp(cmd, "destroy")) {
		return PPE_MIRROR_TEST_CMD_DESTROY_DEV;
	}

	if (!strcmp(cmd, "map")) {
		return PPE_MIRROR_TEST_CMD_MAP_ACL;
	}

	if (!strcmp(cmd, "unmap")) {
		return PPE_MIRROR_TEST_CMD_UNMAP_ACL;
	}

	if (!strcmp(cmd, "capture_core")) {
		return PPE_MIRROR_TEST_CMD_ENABLE_CORE;
	}

	if (!strcmp(cmd, "drop")) {
		return PPE_MIRROR_TEST_CMD_DROP;
	}

	printk("Invalid string:%s in command\n", cmd);
	return PPE_MIRROR_TEST_CMD_UNKNOWN;
}

/*
 * ppe_mirror_test_parse_unmap_cmd()
 *	Parse the unmapping command.
 */
bool ppe_mirror_test_parse_unmap_cmd(char *buffer)
{
	char *param, *value;
	uint16_t acl_id;
	ppe_mirror_ret_t ret;
	struct ppe_mirror_port_mapping_info port_info = {0};
	bool valid_flag = false;

	param = ppe_mirror_test_read_value(&buffer, &value, "=");
	if (!param || !value)
		return false;

	/*
	 * In case of physical dev unmapping request, send the dev list to
	 * mirror module.
	 */
	if (!strcmp(param, "port")) {

		/*
		 * Get the netdev list from the command.
		 */
		if (!ppe_mirror_test_get_netdev_list(value, port_info.mirror_phy_port_list, &valid_flag)) {
			printk("Error in parsing netdev list!\n");
			return false;
		}

		/*
		 * Here call mirror module unmapping API
		 */
		ret = ppe_mirror_phy_port_mapping_delete(&port_info);
		if (ret != PPE_MIRROR_RET_SUCCESS) {
			printk("Failed to delete port mappings, ret %d\n", ret);
			return false;
		}

		return true;
	}

	if (strcmp(param, "acl_id")) {
		printk("Invalid param %s in unmap command, valid param is: acl_id\n", param);
		return false;
	}

	ppe_mirror_test_convert_char_to_u16(value, &acl_id);

	/*
	 * Here call mirror module unmapping API
	 */
	ret = ppe_mirror_acl_mapping_delete(acl_id);

	if (ret != PPE_MIRROR_RET_SUCCESS) {
		printk("Failed to delete ACL mapping for ACL id %d ret %d\n", acl_id, ret);
		return false;
	}

	/*
	 * After successfull unmapping, remove it from local test array.
	 */
	ppe_mirror_test_acl_arr[acl_id] = PPE_MIRROR_TEST_INVALID_GROUP_ID;

	printk("unmapping done for ACL id %d! \n", acl_id);
	return true;
}

/*
 * ppe_mirror_test_parse_drop_cmd()
 *	Parse drop packet command.
 */
bool ppe_mirror_test_parse_drop_cmd(char *buffer)
{
	char *param, *value;
	uint16_t en;

	param = ppe_mirror_test_read_value(&buffer, &value, "=");
	if (!param || !value)
		return false;

	if (strcmp(param, "en")) {
		printk("Invalid param %s in drop en command, valid param is: en\n", param);
		return false;
	}

	ppe_mirror_test_convert_char_to_u16(value, &en);

	if (en)
		ppe_mirror_test_drop_packets = true;
	else
		ppe_mirror_test_drop_packets = false;

	return true;
}
/*
 * ppe_mirror_test_parse_core_select_cmd()
 *	Parse core select command.
 */
bool ppe_mirror_test_parse_core_select_cmd(char *buffer)
{
	char *param, *value;
	uint16_t core_id;
	ppe_mirror_ret_t ret;

	param = ppe_mirror_test_read_value(&buffer, &value, "=");
	if (!param || !value)
		return false;

	if (strcmp(param, "core_id")) {
		printk("Invalid param %s in core select command, valid param is: core_id\n", param);
		return false;
	}

	ppe_mirror_test_convert_char_to_u16(value, &core_id);

	/*
	 * Here call the mirror module core selection API.
	 */
	ret = ppe_mirror_enable_capture_core((uint8_t)core_id);

	if (ret != PPE_MIRROR_RET_SUCCESS) {
		printk("Failed to select the core ret %d\n", ret);
		return false;
	}

	return true;
}

/*
 * ppe_mirror_test_destroy_dev()
 *	Destroy the test group.
 */
bool ppe_mirror_test_destroy_dev(char *buffer)
{
	struct net_device *group_dev;
	char *param, *value;
	uint16_t group_id = PPE_MIRROR_TEST_INVALID_GROUP_ID;
	int i;

	/*
	 * parse the command and get the dev name.
	 */
	param = ppe_mirror_test_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return false;
        }

	if (strcmp(param, "group")) {
		printk("Invalid param %s in mirror configure command\n", param);
		return false;
	}

	/*
	 * Check if the group dev is present.
	 */
	if (!ppe_mirror_test_get_netdev_by_name(value, &group_dev)) {
		printk("Netdev group is not present %s.\n", value);
		return false;
	}

	if (!group_dev)
		return false;

	dev_put(group_dev);

	/*
	 * Check if this dev has mappings valid on it.
	 * if yes, do not allow to delete them.
	 */
	group_id = ppe_mirror_test_group_get_group_id(group_dev);

	if (group_id == PPE_MIRROR_TEST_INVALID_GROUP_ID)
		return false;

	for (i = 0; i < PPE_MIRROR_TEST_ACL_MAX; i++) {
		if (ppe_mirror_test_acl_arr[i] == group_id) {
			printk("Mapping is there for the dev - please delete the mappings before group.\n");
			return false;
		}
	}

	/*
	 * Delete the group from the global array.
	 */
	if (!ppe_mirror_test_group_delete(group_dev))
                return false;

	unregister_netdev(group_dev);
	free_netdev(group_dev);

	printk("Sucessfully deleted the group dev %s\n", value);
	return true;
}

/*
 * ppe_mirror_test_create_dev()
 *	Create the group dev.
 */
struct net_device *ppe_mirror_test_create_dev(char * buffer)
{
	struct net_device *group_dev;
	char *param, *value;
	int ret;
	char dev_name[IFNAMSIZ] = {0};

	param = ppe_mirror_test_read_value(&buffer, &value, "=");
	if (!param || !value)
		return NULL;

	if (strcmp(param, "group")) {
		printk("Invalid param %s in mirror configure command\n", param);
		return NULL;
	}

	/*
	 * Check if the device is already present.
	 */
	if (ppe_mirror_test_get_netdev_by_name(value, &group_dev)) {
		printk("Netdev is already present %s.\n", group_dev->name);
		dev_put(group_dev);
		return NULL;
	}

	strlcpy(dev_name, value, IFNAMSIZ);
	if (dev_name[strlen(dev_name) - 1] == '\n')
		dev_name[strlen(dev_name) - 1] = '\0';

	/*
	 * Allocate and register group device.
	 */
	group_dev = alloc_netdev(16, dev_name, NET_NAME_UNKNOWN, ether_setup);
	if (!group_dev) {
		printk("netdev allocation failed\n");
		return NULL;
	}

	/*
	 * Setup net device ops and hw address.
	 */
	group_dev->netdev_ops = &ppe_mirror_test_netdev_ops;
	eth_hw_addr_random(group_dev);

	ret = register_netdev(group_dev);
	if (ret) {
		printk("netdev registration failed %d\n", ret);
		free_netdev(group_dev);
		return NULL;
        }

	/*
	 * Add the group netdev to global array.
         */
	if (!ppe_mirror_test_group_add(group_dev)) {
		printk("Error in adding %s mirror device in mirror array\n",group_dev->name);
		ppe_mirror_test_dev_destroy(group_dev);
		return NULL;
	}

	return group_dev;
}

/*
 * ppe_mirror_test_config_params()
 *	Get the buffer and process the command.
 */
static int ppe_mirror_test_config_params(struct ctl_table *ctl, int write, void __user *buf, size_t *lenp, loff_t *ppos)
{
	char *buffer, *pfree;
	char * nextarg;
	int command, ret;
	size_t count = *lenp;
	struct net_device *group_dev;

	/*
	 * Find the input command, return if not found.
	 */
	ret = proc_dostring(ctl, write, buf, lenp, ppos);
	if (ret || !write)
		return ret;

	/*
	 * Check length of the config command.
	 */
	if (count >= PPE_MIRROR_TEST_CMD_STR) {
		printk("Input string too big \n");
		return -E2BIG;
	}

	buffer = vzalloc(count + 1);
	if (!buffer) {
		printk("Dynamic allocation failed for input buffer\n");
		return -ENOMEM;
	}

	pfree = buffer;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (copy_from_user(buffer, buf, count)) {
		vfree(pfree);
		return -EFAULT;
	}
#else
	memcpy((void *)buffer, (void *)buf, count);
#endif

	/*
	 * Get the first argument of the command which is
	 * command type.
	 */
	nextarg = ppe_mirror_test_read_nextarg(&buffer);
	if (!nextarg)
		goto err;

	if (nextarg[strlen(nextarg) - 1] == '\n')
		nextarg[strlen(nextarg) - 1] = '\0';

	/*
	 * Parse the command from the first argument.
	 */
	command = ppe_mirror_test_parse_cmd(nextarg);

	switch(command) {
	case PPE_MIRROR_TEST_CMD_CREATE_DEV:
	{
		if (!(group_dev = ppe_mirror_test_create_dev(buffer))) {
			printk("%s Error in create cmd\n", __func__);
			goto err;
		}

		printk("successfully create the netdev %p\n", group_dev);
		break;
	}

	case PPE_MIRROR_TEST_CMD_DESTROY_DEV:
	{
		if (!ppe_mirror_test_destroy_dev(buffer)) {
			printk("%s Error in destroy dev command\n", __func__);
			goto err;
		}

		printk("successfully destroyed the netdev \n");
		break;
	}

	case PPE_MIRROR_TEST_CMD_MAP_ACL:
	{
		if (!ppe_mirror_test_parse_map_cmd(buffer)) {
			printk("Error in parsing map cmd\n");
			goto err;
		}

		printk("Mapping added successfully\n");
		break;
	}

	case PPE_MIRROR_TEST_CMD_UNMAP_ACL:
	{
		if (!ppe_mirror_test_parse_unmap_cmd(buffer)) {
			printk("Error in parsing unmap cmd\n");
			goto err;
		}

		printk("Mapping deleted successfully\n");
		break;
	}

	case PPE_MIRROR_TEST_CMD_ENABLE_CORE:
	{
		if (!ppe_mirror_test_parse_core_select_cmd(buffer)) {
			printk("Error in parsing core select cmd\n");
			goto err;
		}

		printk("Core selected successfully\n");
		break;
	}

	case PPE_MIRROR_TEST_CMD_DROP:
	{
		if (!ppe_mirror_test_parse_drop_cmd(buffer)) {
			printk("Error in parsing drop command \n");
			goto err;
		}

		printk("Setting dropped packets en/disable successfully \n");
		break;
	}

	default:
		printk("Invalid input in command, Valid are : create, destroy, map, unmap, capture_core\n");
	}

err:
	vfree(pfree);
	return ret;
}

/*
 * ppe_mirror_test_tbl
 *	procfs table to configure test module.
 */
static struct ctl_table ppe_mirror_test_tbl[] = {
	{
		.procname       = "config",
		.data           = &ppe_mirror_test_data,
		.maxlen         = sizeof(ppe_mirror_test_data),
		.mode           = 0644,
		.proc_handler   = &ppe_mirror_test_config_params,
	},
	{ }
};

/*
 * ppe_mirror_test_procfs_register()
 *	Register the procfs entry.
 */
struct ctl_table_header *ppe_mirror_test_procfs_register(void)
{
	return register_sysctl("ppe/ppe_test", ppe_mirror_test_tbl);
}

/*
 * ppe_mirror_test_cleanup()
 *	Unmap the exsisting mappings and destroy
 *	net devices created by the test module.
 */
void ppe_mirror_test_cleanup(void)
{
	uint16_t i, j;
	struct net_device *dev;

	for (i = 0; i < 8; i++) {
		if (!ppe_mirror_test_group[i].is_valid)
			continue;

		dev = ppe_mirror_test_group[i].dev;

		/*
		 * Delete all the mappings on this group dev.
		 */
		for (j = 0; j < PPE_MIRROR_TEST_ACL_MAX; j++) {
			if (ppe_mirror_test_acl_arr[j] == i) {
				printk("Deleting the mapping for dev %s ACL %d\n", dev->name, j);
				ppe_mirror_acl_mapping_delete(j);
				ppe_mirror_test_acl_arr[j] = PPE_MIRROR_TEST_INVALID_GROUP_ID;
			}
		}

		ppe_mirror_test_dev_destroy(dev);
	}
}

/*
 * ppe_mirror_test_procfs_unregister()
 *	Unregister the procfs entry.
 */
void ppe_mirror_test_procfs_unregister(struct ctl_table_header *ctl)
{
	if (ctl)
		unregister_sysctl_table(ctl);
}

/*
 * ppe_mirror_test_module_init()
 *      module init for ppe test.
 */
static int __init ppe_mirror_test_module_init(void)
{
	int i;

	ppe_mirror_test_procfs_header = ppe_mirror_test_procfs_register();
	if (!ppe_mirror_test_procfs_header) {
		printk("Unable to register procfs directory\n");
		return -1;
	}

	/*
	 * Initialize the local ACL mapping array.
	 */
	for (i = 0; i < PPE_MIRROR_TEST_ACL_MAX; i++)
		ppe_mirror_test_acl_arr[i] = PPE_MIRROR_TEST_INVALID_GROUP_ID;

	printk("PPE-TEST module loaded successfully\n");
	return 0;
}
module_init(ppe_mirror_test_module_init);

/*
 * ppe_mirror_test_module_exit()
 *      module exit for ppe test.
 */
static void __exit ppe_mirror_test_module_exit(void)
{
	/*
	 * Cleanup the mirror mappings and destroy the netdevices
	 * before unloading the module.
	 */
	ppe_mirror_test_cleanup();
	ppe_mirror_test_procfs_unregister(ppe_mirror_test_procfs_header);
	printk("PPE-TEST module unloaded");
}
module_exit(ppe_mirror_test_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("PPE Test");
