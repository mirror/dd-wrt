/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

/*
 * nss_match_cmd.c
 */

#include <linux/sysctl.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/ctype.h>
#include "nss_match_db.h"
#include "nss_match_cmd.h"
#include <nss_api_if.h>
#include "nss_match_priv.h"

unsigned char nss_match_data[100] __read_mostly;

/*
 * nss_match_cmd_instance_config_tx_sync()
 *	Sends configuration message to NSS
 */
static nss_tx_status_t nss_match_cmd_instance_config_tx_sync(struct nss_ctx_instance *nss_ctx,
						uint32_t if_num, struct nss_match_profile_configure_msg *config_msg)
{
	struct nss_match_msg matchm;

	nss_match_msg_init(&matchm, if_num,  NSS_MATCH_TABLE_CONFIGURE_MSG,
			sizeof(struct nss_match_profile_configure_msg), NULL, NULL);
	matchm.msg.configure_msg = *config_msg;

	return nss_match_msg_tx_sync(nss_ctx, &matchm);
}

static int nss_match_cmd_enable_instance(struct nss_match_profile_configure_msg *config_msg, int if_num, uint32_t table_id) {

	nss_tx_status_t nss_tx_status;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();

	nss_tx_status = nss_match_cmd_instance_config_tx_sync(nss_ctx, if_num, config_msg);
	if (nss_tx_status == NSS_TX_SUCCESS) {
		if (!nss_match_db_instance_enable(table_id)) {
			nss_match_warn("Failed to enable instance for table_id=%u\n", table_id);
			return -1;
		}
		return 0;
	}
	nss_match_warn("%px: Cannot configure/enable the new instance\n", nss_ctx);

	return -1;
}

/*
 * nss_match_cmd_parse()
 *	Returns command type.
 */
static nss_match_cmd_t nss_match_cmd_parse(char *cmd)
{
	if (cmd == NULL)
		return NSS_MATCH_UNKNOWN;
	if (!(strncasecmp(cmd, "createtable", strlen("createtable"))))
		return NSS_MATCH_CREATE_TABLE;
	if (!(strncasecmp(cmd, "addmask", strlen("addmask"))))
		return NSS_MATCH_ADD_MASK;
	if (!(strncasecmp(cmd, "enable", strlen("enable"))))
		return NSS_MATCH_ENABLE;
	if (!(strncasecmp(cmd, "addrule", strlen("addrule"))))
		return NSS_MATCH_ADD_RULE;
	if (!(strncasecmp(cmd, "delrule", strlen("delrule"))))
		return NSS_MATCH_DELETE_RULE;
	if (!(strncasecmp(cmd, "deltable", strlen("deltable"))))
		return NSS_MATCH_DESTROY_TABLE;

	return NSS_MATCH_UNKNOWN;
}

/*
 * nss_match_cmd_get_profile_type()
 *	Parse message to create an instance.
 */
static enum nss_match_profile_type nss_match_cmd_get_profile_type(char *input_msg)
{
	char *token, *param;

	token = strsep(&input_msg, " ");
	if (!token) {
		return NSS_MATCH_PROFILE_TYPE_NONE;
	}

	param = strsep(&token, "=");
	if (!param || !token) {
		return NSS_MATCH_PROFILE_TYPE_NONE;
	}

	if (!(strncasecmp(param, "profile_type", strlen("profile_type")))) {
		if (!(strncasecmp(token, "vow", strlen("vow")))) {
			return NSS_MATCH_PROFILE_TYPE_VOW;
		}

		if (!(strncasecmp(token, "l2", strlen("l2")))) {
			return NSS_MATCH_PROFILE_TYPE_L2;
		}
	}

	return NSS_MATCH_PROFILE_TYPE_NONE;
}

/*
 * nss_match_cmd_procfs_config_handler()
 * 	Handles command input by user to create and configure match instance.
 */
static int nss_match_cmd_procfs_config_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	char *command_str, *token, *param, *value;
	char *input_msg, *input_msg_orig;
	nss_match_cmd_t command;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	size_t count = *lenp;
	int ret = proc_dostring(ctl, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	input_msg = (char *)kzalloc(count + 1, GFP_KERNEL);
	if (!input_msg) {
		nss_match_warn("%px: Dynamic allocation falied while writing input message from file", ctl);
		return -ENOMEM;
	}

	input_msg_orig = input_msg;
	if (copy_from_user(input_msg, buffer, count)) {
		kfree(input_msg);
		nss_match_warn("%px: Cannot copy user's entry to kernel memory\n", ctl);
		return -EFAULT;
	}

	command_str = strsep(&input_msg, " ");
	command = nss_match_cmd_parse(command_str);

	switch (command) {
	case NSS_MATCH_CREATE_TABLE:
	{
		int table_id = -1, profile_type = 0;

		profile_type = nss_match_cmd_get_profile_type(input_msg);
		if (profile_type == NSS_MATCH_PROFILE_TYPE_NONE) {
			pr_warn("%px: Please provide a valid profile type\n", ctl);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		table_id = nss_match_instance_create();
		if (table_id <= 0) {
			pr_warn("%px: Cannot create a new match instance\n", ctl);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		nss_match_db_profile_type_add(profile_type, table_id);
		pr_warn("%px: New match instance created, table_id = %d\n", ctl, table_id);
		kfree(input_msg_orig);
		return count;
	}

	case NSS_MATCH_ADD_MASK:
	{
		uint32_t table_id = 0;
		struct nss_match_msg input_mask_param = {0};

		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		value = token;
		if (!value || !param) {
			goto fail;
		}

		if (!strncasecmp(param, "table_id", strlen("table_id"))) {
			ret = sscanf(value, "%u", &table_id);
			if (!ret) {
				pr_warn("%px: Cannot convert to integer. Wrong input!!", ctl);
				kfree(input_msg_orig);
				return -EINVAL;
			}
		}

		if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
			pr_warn("%px: Invalid table_id %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_db_table_validate(table_id)) {
			pr_warn("%px: Table is already configured, %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_db_parse_cmd(table_id, input_msg, &input_mask_param, NSS_MATCH_ADD_MASK)) {
			kfree(input_msg_orig);
			return -EINVAL;
		}

		nss_match_db_mask_add(&input_mask_param.msg.configure_msg, table_id);
		pr_warn("%px: Mask added to instance successfully. %d", ctl, table_id);

		kfree(input_msg_orig);
		return count;
	}

	case NSS_MATCH_ENABLE:
	{
		uint32_t table_id = 0;
		struct nss_match_profile_configure_msg config_msg = {0};
		int if_num = -1;

		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		value = token;
		if (!param || !value) {
			goto fail;
		}

		if (!strncasecmp(param, "table_id", strlen("table_id"))) {
			ret = sscanf(value, "%u", &table_id);
			if (!ret) {
				pr_warn("%px: Cannot convert to integer. Wrong input!!", ctl);
				kfree(input_msg_orig);
				return -EINVAL;
			}
		}

		if ((table_id == 0) || (table_id > NSS_MATCH_INSTANCE_MAX)) {
			pr_warn("%px: Invalid table_id %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_db_table_validate(table_id)) {
			pr_warn("%px: Table is already configured, %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (!nss_match_db_instance_config_get(&config_msg, &if_num, table_id)) {
			pr_warn("%px: Unable to fetch stored configuration %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (if_num < 0) {
			nss_match_warn("%px: Incorrect interface number: %d\n", ctl, if_num);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_cmd_enable_instance(&config_msg, if_num, table_id)) {
			pr_warn("%px: Failed to enable table %d\n", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		pr_warn("%px: Table %d enabled successfully\n", ctl, table_id);
		kfree(input_msg_orig);
		return count;
	}

	case NSS_MATCH_ADD_RULE:
	{
		uint32_t profile_type = 0;
		uint32_t table_id = 0;
		int rule_id = -1;
		struct nss_match_msg input_rule_param = {0};

		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		value = token;
		if (!param || !value) {
			goto fail;
		}

		if (!strncasecmp(param, "table_id", strlen("table_id"))) {
			ret = sscanf(value, "%u", &table_id);
			if (!ret) {
				pr_warn("%px: Cannot convert to integer. Wrong input!!", ctl);
				kfree(input_msg_orig);
				return -EINVAL;
			}
		}

		if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
			pr_warn("%px: Invalid table_id: %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		nss_match_db_get_profile_type(table_id, &profile_type);

		if (nss_match_db_parse_cmd(table_id, input_msg, &input_rule_param, NSS_MATCH_ADD_RULE)) {
			pr_warn("%px: Wrong input", ctl);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (profile_type == NSS_MATCH_PROFILE_TYPE_VOW) {
			rule_id = nss_match_vow_rule_add(nss_ctx, &input_rule_param.msg.vow_rule, table_id);
		} else if (profile_type == NSS_MATCH_PROFILE_TYPE_L2) {
			rule_id = nss_match_l2_rule_add(nss_ctx, &input_rule_param.msg.l2_rule, table_id);
		}

		if (rule_id < 0) {
			pr_warn("%px: Failed to add rule into table %d.\n", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		pr_warn("%px: Rule added to table %d successfully with rule_id: %d\n", ctl, table_id, rule_id);
		kfree(input_msg_orig);
		return count;
	}

	case NSS_MATCH_DELETE_RULE:
	{
		uint32_t table_id = 0;
		uint16_t rule_id = 0;

		while (input_msg != NULL) {
			token = strsep(&input_msg, " ");
			param = strsep(&token, "=");
			if (!param || !token) {
				goto fail;
			}

			/*
			 * Parsing rule_id and table_id value from the message.
			 */
			if (!(strncasecmp(param, "rule_id", strlen("rule_id")))) {
				if (!sscanf(token, "%hu", &rule_id)) {
					pr_warn("%px: Cannot convert to integer. Wrong input\n", ctl);
					kfree(input_msg_orig);
					return -EINVAL;
				}
				continue;
			}

			if (!strncasecmp(param, "table_id", strlen("table_id"))) {
				if (!sscanf(token, "%u", &table_id)) {
					pr_warn("%px: Cannot convert to integer. Wrong input!!", ctl);
					kfree(input_msg_orig);
					return -EINVAL;
				}
				continue;
			}

			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
			pr_warn("%px: Invalid table_id: %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (rule_id == 0 || rule_id > NSS_MATCH_INSTANCE_RULE_MAX) {
			pr_warn("%px: Invalid rule_id: %d", ctl, rule_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_rule_delete(nss_ctx, rule_id, table_id)) {
			pr_warn("%px: Failed to delete rule from table %d.\n", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		pr_warn("%px: Rule deleted from table %d successfully\n", ctl, table_id);
		kfree(input_msg_orig);
		return count;
	}

	case NSS_MATCH_DESTROY_TABLE:
	{
		uint32_t table_id = 0;
		char *token, *param;
		int ret = 0;

		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		if (!token || !param) {
			goto fail;
		}

		if (!(strncasecmp(param, "table_id", strlen("table_id")))) {
			ret = sscanf(token, "%u", &table_id);
			if (!ret) {
				pr_warn("%px: Cannot convert to integer. Wrong input!!", input_msg);
				kfree(input_msg_orig);
				return -EINVAL;
			}
		}

		if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
			pr_warn("%px: Invalid table_id: %d", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		if (nss_match_instance_destroy(table_id)) {
			pr_warn("%px: Failed to destroy table %d\n", ctl, table_id);
			kfree(input_msg_orig);
			return -EINVAL;
		}

		pr_warn("%px: Table %d destroyed successfully.\n", ctl, table_id);
		kfree(input_msg_orig);
		return count;
	}

	default:
	{
		pr_warn("%px: Input command is not as per syntax, Please enter a valid command", ctl);
		kfree(input_msg_orig);
		return -EINVAL;
	}
     }

fail:
	pr_warn("%px: Wrong input, check help. (cat /proc/sys/dev/nss/match/help)", ctl);
	kfree(input_msg_orig);
	return ret;

}

/*
 * nss_match_cmd_procfs_reset_nexthop
 * 	Reset to default nexthop of an interface
 */
static int nss_match_cmd_procfs_reset_nexthop(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	uint32_t if_num, type = 0;
	int ret;
	char *dev_name;
	char *cmd_buf = nss_match_data;
	nss_tx_status_t nss_tx_status;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	struct nss_ctx_instance *wifi_nss_ctx = nss_wifi_get_context();

	if (!nss_ctx || !wifi_nss_ctx) {
		pr_warn("%px: NSS Context not found. wifi_nss_ctx: %px. Reset nexthop failed", nss_ctx, wifi_nss_ctx);
		return -ENOMEM;
	}

	ret = proc_dostring(ctl, write, buffer, lenp, ppos);
	if (!write) {
		pr_warn("%px: Reset nexthop failed.\n", nss_ctx);
		return ret;
	}

	/*
	 * Parse and read the devname from command.
	 */
	dev_name = strsep(&cmd_buf, "\0");
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		pr_warn("%px: Cannot find the net device: %s. Reset nexthop failed.\n", nss_ctx, dev_name);
		return -ENODEV;
	}

	if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (if_num < 0) {
		pr_warn("%px: Invalid if_num for interface: %s. Reset nexthop failed.\n", nss_ctx, dev_name);
		dev_put(dev);
		return -ENODEV;
	}

	/*
	 * Reset Nexthop APIs.
	 * nss_phys_if_reset_nexthop: Used for physical interfaces.
	 * nss_if_reset_nexthop: used for VAP interfaces.
	 */
	type = nss_dynamic_interface_get_type(wifi_nss_ctx, if_num);
	if (type == NSS_DYNAMIC_INTERFACE_TYPE_VAP) {
		nss_tx_status = nss_if_reset_nexthop(wifi_nss_ctx, if_num);
	} else if (if_num < NSS_MAX_PHYSICAL_INTERFACES) {
		nss_tx_status = nss_phys_if_reset_nexthop(nss_ctx, if_num);
	} else {
		pr_warn("%px: Invalid interface to Reset nexthop. Failed to Reset nexthop on if_num %d.\n",
				nss_ctx, if_num);
		dev_put(dev);
		return -EFAULT;
	}

	if (nss_tx_status != NSS_TX_SUCCESS) {
		pr_warn("%px: Sending message failed, cannot reset nexthop\n", nss_ctx);
	}

	dev_put(dev);
	pr_info("%px: Reset nexthop successful.\n", nss_ctx);
	return 0;
}

/*
 * nss_match_cmd_procfs_set_if_nexthop
 * 	Set next hop of an interface to a match instance.
 * 	Only VAP and physical interfaces are supported as of now.
 */
static int nss_match_cmd_procfs_set_if_nexthop(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	uint32_t if_num, type = 0;
	uint32_t nh_if_num;
	int table_id;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	struct nss_ctx_instance *wifi_nss_ctx = nss_wifi_get_context();
	char *dev_name, *nexthop_msg;
	char *cmd_buf = NULL;
	size_t count = *lenp;
	nss_tx_status_t nss_tx_status;
	int ret = proc_dostring(ctl, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	if (!nss_ctx || !wifi_nss_ctx) {
		pr_warn("%px: NSS Context not found. wifi_nss_ctx: %px. Set nexthop failed", nss_ctx, wifi_nss_ctx);
		return -ENOMEM;
	}

	cmd_buf = (char *)kzalloc(count + 1, GFP_KERNEL);
	nexthop_msg = cmd_buf;
	if (!cmd_buf) {
		pr_warn("%px: Cannot allocate buffer to read input", nss_ctx);
		return -ENOMEM;
	}

	if (copy_from_user(cmd_buf, buffer, count)) {
		kfree(nexthop_msg);
		pr_warn("%px: Cannot copy user's entry to kernel memory\n", nss_ctx);
		return -EFAULT;
	}

	dev_name = strsep(&cmd_buf, " ");
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		pr_warn("%px: Cannot find the net device\n", nss_ctx);
		kfree(nexthop_msg);
		return -ENODEV;
	}

	if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (if_num < 0) {
		pr_warn("%px: Invalid interface number:%d\n", nss_ctx, if_num);
		kfree(nexthop_msg);
		dev_put(dev);
		return -ENODEV;
	}

	if (isdigit(cmd_buf[0])) {
		if (!sscanf(cmd_buf, "%u", &nh_if_num)) {
			pr_warn("%px, Failed to write the nexthop if_num token to integer\n", nss_ctx);
			kfree(nexthop_msg);
			dev_put(dev);
			return -EFAULT;
		}
	} else {
		pr_warn("%px: Invalid nexthop interface number.\n", nss_ctx);
		kfree(nexthop_msg);
		dev_put(dev);
		return -ENODEV;
	}

	if (nh_if_num < 0) {
		pr_warn("%px: Invalid nexthop interface number:%d\n", nss_ctx, if_num);
		kfree(nexthop_msg);
		dev_put(dev);
		return -ENODEV;
	}

	table_id = nss_match_get_table_id_by_ifnum(nh_if_num);
	if (table_id <= 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
		pr_warn("Invalid match interface. Failed to set %d as nexthop.\n", nh_if_num);
		kfree(nexthop_msg);
		dev_put(dev);
		return -EFAULT;
	}

	/*
	 * Set Nexthop APIs.
	 * nss_phys_if_set_nexthop: Used for physical interfaces.
	 * nss_if_set_nexthop: used for VAP interfaces.
	 */
	type = nss_dynamic_interface_get_type(wifi_nss_ctx, if_num);
	if (type == NSS_DYNAMIC_INTERFACE_TYPE_VAP) {
		nss_tx_status = nss_if_set_nexthop(wifi_nss_ctx, if_num, nh_if_num);
	} else if (if_num < NSS_MAX_PHYSICAL_INTERFACES) {
		nss_tx_status = nss_phys_if_set_nexthop(nss_ctx, if_num, nh_if_num);
	} else {
		pr_warn("Invalid interface to set nexthop. Failed to set nexthop on if_num %d.\n", if_num);
		kfree(nexthop_msg);
		dev_put(dev);
		return -EFAULT;
	}

	if (nss_tx_status != NSS_TX_SUCCESS) {
		pr_warn("%px: Sending message failed, cannot change nexthop\n", nss_ctx);
	}

	kfree(nexthop_msg);
	dev_put(dev);
	return ret;
}

/*
 * nss_match_cmd_procfs_read_help()
 * 	Display help for commands.
 */
static int nss_match_cmd_procfs_read_help(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec(ctl, write, buffer, lenp, ppos);

	pr_info("\nHelp: (/proc/sys/dev/nss/match/help) \n\
		1. To create match isntance:\n\
			echo createtable profile_type=<vow/l2> > config	\n\
		2. To addmask: \n\
		a. VoW profile \n\
			echo addmask table_id=<1..4> mask=<1/2> ifname=<1..ffff> dscp=<1..3f> 802.1p_outer=<1..7> 802.1p_inner=<1..7> > config\n\
		b. L2 profile \n\
			echo addmask table_id=<1..4> mask=<1/2> ifname=<1..ffff> smac=<1..ffffffffffff>\
			dmac=<1..ffffffffffff> ethertype=<1..ffff> > config\n\
		3. To enable match instance \n\
			echo enable table_id=1 > config \n\
		5. Actions:\n\
			a. action=1 priority=<pri_value>\n\
			b. action=2 nexthop=<nexthop_ifnum>\n\
			c. action=3 priority=<pri_value> nexthop=<nexthop_ifnum>\n\
			d. action=4\n\
		4. To set nexthop: \n\
			echo <phy_ifname/VAP name> <match_ifnum> > set_nexthop\n\
		5. To reset nexthop: \n\
			echo <dev_name> > reset_nexthop\n");
	*lenp = 0;
	return ret;
}

static struct ctl_table nss_match_table[] = {
	{
		.procname		= "config",
		.data			= &nss_match_data,
		.maxlen			= sizeof(nss_match_data),
		.mode			= 0644,
		.proc_handler		= &nss_match_cmd_procfs_config_handler,
	},
	{
		.procname		= "set_nexthop",
		.data			= &nss_match_data,
		.maxlen			= sizeof(nss_match_data),
		.mode			= 0644,
		.proc_handler		= &nss_match_cmd_procfs_set_if_nexthop,
	},
	{
		.procname		= "reset_nexthop",
		.data			= &nss_match_data,
		.maxlen			= IFNAMSIZ,
		.mode			= 0644,
		.proc_handler		= &nss_match_cmd_procfs_reset_nexthop,
	},
	{
		.procname		= "help",
		.data                   = &nss_match_data,
		.maxlen                 = sizeof(nss_match_data),
		.mode			= 0400,
		.proc_handler		= &nss_match_cmd_procfs_read_help,
	},
	{ }
};

static struct ctl_table nss_match_root_dir[] = {
	{
		.procname		= "match",
		.mode			= 0555,
		.child			= nss_match_table,
	},
	{ }
};

static struct ctl_table nss_match_nss_root_dir[] = {
	{
		.procname		= "nss",
		.mode			= 0555,
		.child			= nss_match_root_dir,
	},
	{ }
};

static struct ctl_table nss_match_root[] = {
	{
		.procname		= "dev",
		.mode			= 0555,
		.child			= nss_match_nss_root_dir,
	},
	{ }
};

static struct ctl_table_header *nss_match_ctl_header;

/*
 * nss_match_ctl_register
 * 	Register command line interface for match.
 */
bool nss_match_ctl_register(void) {
	nss_match_ctl_header = register_sysctl_table(nss_match_root);
	if (!nss_match_ctl_header) {
		nss_match_warn("Unable to register command line interface.\n");
		return false;
	}

	return true;
}

/*
 * nss_match_ctl_unregister
 * 	Unregister command line interface for match.
 */
void nss_match_ctl_unregister(void) {
	if (nss_match_ctl_header) {
		unregister_sysctl_table(nss_match_ctl_header);
	}
}
