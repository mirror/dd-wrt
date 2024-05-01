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

#include <linux/sysctl.h>
#include <linux/string.h>

#include "nss_mirror_public.h"
#include "nss_mirror.h"

#define NSS_MIRROR_CONFIG_PARAM_NUM 3
#define NSS_MIRROR_SET_NEXTHOP_PARAM_NUM 2
#define NSS_MIRROR_ENABLE_INGRESS_PMC_NUM 2

/*
 * nss_mirror_ctl_cmd_type
 *	Mirror command types.
 */
enum nss_mirror_ctl_cmd_type {
	NSS_MIRROR_CMD_UNKNOWN,			/* Unknown command. */
	NSS_MIRROR_CMD_CREATE,			/* Create command. */
	NSS_MIRROR_CMD_CONFIGURE,		/* Configure command. */
	NSS_MIRROR_CMD_SET_NEXTHOP,		/* Set nexthop command. */
	NSS_MIRROR_CMD_RESET_NEXTHOP,		/* Reset nexthop command. */
	NSS_MIRROR_CMD_INGRESS_PMC_ENABLE,	/* Ingress promiscuous enable command. */
	NSS_MIRROR_CMD_INGRESS_PMC_DISABLE,	/* Ingress promiscuous disable command. */
	NSS_MIRROR_CMD_ENABLE,			/* Enable command. */
	NSS_MIRROR_CMD_DISABLE,			/* Disable command. */
	NSS_MIRROR_CMD_DISPLAY,			/* Display command. */
	NSS_MIRROR_CMD_DESTROY,			/* Destroy command. */
	NSS_MIRROR_CMD_HELP,			/* Help command. */
};

/*
 * Sysctl table header for mirror.
 */
static struct ctl_table_header *nss_mirror_ctl_header;

/*
 * Mirror config data.
 */
static unsigned char nss_mirror_config_data[128] __read_mostly;

/*
 * nss_mirror_ctl_read_nextarg()
 *	API to read the next argument in the command.
 */
static char *nss_mirror_ctl_read_nextarg(char **buf_ptr)
{
	if (!buf_ptr || !(*buf_ptr)) {
		nss_mirror_warn("Read Buf is NULL\n");
		return NULL;
	}

	return strsep(buf_ptr, " ");
}

/*
 * nss_mirror_ctl_read_value()
 *	API to read a value of the param in the command.
 */
static char *nss_mirror_ctl_read_value(char **buf_ptr, char **value_ptr, char *delim)
{
	*value_ptr = nss_mirror_ctl_read_nextarg(buf_ptr);

	if (!(*value_ptr)) {
		return NULL;
	}

	return strsep(value_ptr, delim);
}

/*
 * nss_mirror_ctl_convert_char_to_u32()
 *	API to convert character to u32.
 */
static int nss_mirror_ctl_convert_char_to_u32(char *buf, uint32_t *arg)
{
	int ret;

	/*
	 * Write the tokens to unsigned integer.
	 */
	ret = sscanf(buf, "%u", arg);
	if (ret != 1) {
		nss_mirror_warn("Failed to write the %s token to u32\n", buf);
		return -1;
	}
	return 0;
}

/*
 * nss_mirror_ctl_convert_char_to_u16()
 *	API to convert character to u16.
 */
static int nss_mirror_ctl_convert_char_to_u16(char *buf, uint16_t *arg)
{
	int ret;

	/*
	 * Write the tokens to unsigned short integer.
	 */
	ret = sscanf(buf, "%hu", arg);
	if (ret != 1) {
		nss_mirror_warn("Failed to write the %s token to u16\n", buf);
		return -1;
	}
	return 0;
}

/*
 * nss_mirror_ctl_get_netdev_by_name()
 *	API to get netdev from name.
 *
 * Note: Caller is expected to release the hold on the dev.
 */
static int nss_mirror_ctl_get_netdev_by_name(char *name, struct net_device **dev)
{
	char dev_name[IFNAMSIZ] = {0};

	strlcpy(dev_name, name, IFNAMSIZ);
	if (dev_name[strlen(dev_name) - 1] == '\n') {
		dev_name[strlen(dev_name) - 1] = '\0';
	}
	nss_mirror_info("Device name: %s\n", dev_name);
	*dev = dev_get_by_name(&init_net, dev_name);
	if (!*dev) {
		nss_mirror_warn("Cannot find %s netdevice\n", dev_name);
		return -1;
	}
	return 0;
}

/*
 * nss_mirror_ctl_display_help()
 *	API to display help.
 */
static void nss_mirror_ctl_display_help(void)
{
	pr_info("Usage: \n"
		"echo create > <REDIRECT PATH>\n"
		"echo configure mirrordev=<mirror dev name> [mirror_size=<MIRROR SIZE>]"
		" [mirror_point=<MIRROR POINT>] [mirror_offset=<MIRROR OFFSET>] > <REDIRECT PATH>\n"
		"echo enable mirrordev=<mirror dev name> > <REDIRECT PATH>\n"
		"echo disable mirrordev=<mirror dev name> > <REDIRECT PATH>\n"
		"echo set_nexthop mirrordev=<mirror dev name> mirror_next_hop=n2h/eth_rx\n > <REDIRECT PATH>"
		"echo reset_nexthop mirrordev=<mirror dev name> > <REDIRECT PATH>\n"
		"echo ingress_pmc_enable devname=<eth/ath dev name> mirrordev=<mirror dev name> > <REDIRECT PATH>\n"
		"echo ingress_pmc_disable devname=<eth/ath dev name> > <REDIRECT PATH>\n"
		"echo display [mirrordev=<dev name> / all] > <REDIRECT PATH>\n"
		"echo destroy mirrordev=<mirror dev name> > <REDIRECT PATH>\n"
		"Where,\n"
		"\t<REDIRECT PATH> = /proc/sys/dev/nss/mirror/config\n"
	      );
}

/*
 * nss_mirror_ctl_parse_reset_nexthop_cmd()
 *	API to parse reset nexthop command.
 */
static int nss_mirror_ctl_parse_reset_nexthop_cmd(char *buffer)
{
	struct net_device *mirror_dev;
	char *param, *value;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid param: %s in promiscuous enable cmd\n", value);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
		nss_mirror_warn("Error in getting mirror net device.\n");
		return -1;
	}

	if (nss_mirror_reset_nexthop(mirror_dev)) {
		nss_mirror_warn("Error in sending reset nexthop config to mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_set_nexthop_cmd()
 *	API to parse set nexthop command.
 */
static int nss_mirror_ctl_parse_set_nexthop_cmd(char *buffer)
{
	struct net_device *mirror_dev = NULL;
	char *param, *value;
	int32_t nexthop_if_num = -1;
	uint8_t param_num = NSS_MIRROR_SET_NEXTHOP_PARAM_NUM;

	do {
		param = nss_mirror_ctl_read_value(&buffer, &value, "=");
		if (!param || !value) {
			return -1;
		}

		if (!strcmp(param, "mirrordev")) {
			if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
				nss_mirror_warn("Error in getting mirror net device.\n");
				return -1;
			}
		} else if (!strcmp(param, "mirror_next_hop")) {
			if (value[strlen(value) - 1] == '\n') {
				value[strlen(value) - 1] = '\0';
			}
			if (!strcmp(value, "eth_rx")) {
				nexthop_if_num = NSS_ETH_RX_INTERFACE;
			} else if (!strcmp(value, "n2h")) {
				nexthop_if_num = NSS_N2H_INTERFACE;
			} else {
				if (mirror_dev) {
					dev_put(mirror_dev);
				}
				nss_mirror_warn("Invalid mirror next hop value %s\n", value);
				return -1;
			}
		} else {
			if (mirror_dev) {
				dev_put(mirror_dev);
			}
			nss_mirror_warn("Invalid param: %s in promiscuous enable cmd\n", value);
			return -1;
		}
	} while (--param_num);

	if (!mirror_dev) {
		nss_mirror_warn("Mirror device is NULL\n");
		return -1;
	}

	if (nexthop_if_num < 0) {
		nss_mirror_warn("Invalid nexthop interface number\n");
		dev_put(mirror_dev);
		return -1;
	}

	if (nss_mirror_set_nexthop(mirror_dev, nexthop_if_num)) {
		nss_mirror_warn("Error in sending set nexthop config to mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_disable_cmd()
 *	API to parse disable command.
 */
static int nss_mirror_ctl_parse_disable_cmd(char *buffer)
{
	struct net_device *mirror_dev;
	char *param, *value;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid parameter %s in disable cmd\n", param);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
		nss_mirror_warn("Error in getting mirror net device.\n");
		return -1;
	}

	if (nss_mirror_disable(mirror_dev)) {
		nss_mirror_warn("Error in sending disable config to mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_enable_cmd()
 *	API to parse enable command.
 */
static int nss_mirror_ctl_parse_enable_cmd(char *buffer)
{
	struct net_device *mirror_dev;
	char *param, *value;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid parameter %s in enable cmd\n", param);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
		nss_mirror_warn("Error in getting mirror net device.\n");
		return -1;
	}

	if (nss_mirror_enable(mirror_dev)) {
		nss_mirror_warn("Error in sending enable config to mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_display_cmd()
 *	API to parse display command.
 */
static int nss_mirror_ctl_parse_display_cmd(char *buffer)
{
	char *param, *value;
	char dev_name[IFNAMSIZ] = {0};
	struct net_device *mirror_dev;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid param %s in mirror display cmd\n", param);
		return -1;
	}

	strlcpy(dev_name, value, IFNAMSIZ);
	if (dev_name[strlen(dev_name) - 1] == '\n') {
		dev_name[strlen(dev_name) - 1] = '\0';
	}
	nss_mirror_info("dev name: %s\n", dev_name);

	if (!strcmp(dev_name, "all")) {
		nss_mirror_display_all_info();
		return 0;
	}

	mirror_dev = dev_get_by_name(&init_net, dev_name);
	if (!mirror_dev) {
		nss_mirror_warn("Cannot find the %s netdevice\n", dev_name);
		return -1;
	}

	/*
	 * Verify the mirror netdevice.
	 */
	if (nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR) < 0) {
		dev_put(mirror_dev);
		nss_mirror_warn("No valid NSS FW interface for %s device\n", dev_name);
		return -1;
	}

	nss_mirror_display_info(mirror_dev);

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_enable_ingress_pmc_cmd()
 *	API to parse ingress promiscuous enable command.
 */
static int nss_mirror_ctl_parse_enable_ingress_pmc_cmd(char *buffer)
{
	char *param, *value;
	struct nss_ctx_instance *nss_ctx;
	struct net_device *dev, *mirror_dev = NULL;
	nss_tx_status_t status;
	int32_t if_num = -1, type = 0, mirror_if_num = -1;
	uint8_t param_num = NSS_MIRROR_ENABLE_INGRESS_PMC_NUM;

	do {
		param = nss_mirror_ctl_read_value(&buffer, &value, "=");
		if (!param || !value) {
			return -1;
		}

		if (!strcmp(param, "devname")) {
			if (nss_mirror_ctl_get_netdev_by_name(value, &dev) < 0) {
				nss_mirror_warn("Error in getting devname net device.\n");
				if (mirror_dev) {
					dev_put(mirror_dev);
				}
				return -1;
			}

			if ((if_num = nss_cmn_get_interface_number_by_dev(dev)) < 0) {
				nss_mirror_warn("No valid NSS FW interface for %s device\n", dev->name);
				if (mirror_dev) {
					dev_put(mirror_dev);
				}
				dev_put(dev);
				return -1;
			}
			dev_put(dev);
		} else if (!strcmp(param, "mirrordev")) {
			if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
				nss_mirror_warn("Error in getting mirror net device.\n");
				return -1;
			}

			/*
			 * Verify the mirror netdevice.
			 */
			if ((mirror_if_num = nss_cmn_get_interface_number_by_dev_and_type(mirror_dev, NSS_DYNAMIC_INTERFACE_TYPE_MIRROR)) < 0) {
				nss_mirror_warn("No valid NSS FW interface for %s device\n", mirror_dev->name);
				dev_put(mirror_dev);
				return -1;
			}
		} else {
			if (mirror_dev) {
				dev_put(mirror_dev);
			}
			nss_mirror_warn("Invalid param: %s in promiscuous enable cmd\n", value);
			return -1;
		}
	} while (--param_num);

	if (!mirror_dev) {
		nss_mirror_warn("Mirror device is NULL\n");
		return -1;
	}

	if ((if_num < 0) || (mirror_if_num < 0)) {
		nss_mirror_warn("Invalid interface number\n");
		dev_put(mirror_dev);
		return -1;
	}

	/*
	 * Return if the mirror interface is in open state.
	 */
	if (mirror_dev->flags & IFF_UP) {
		nss_mirror_warn("%s mirror interface is in open state\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	type = nss_dynamic_interface_get_type(nss_ctx, if_num);

	if (type == NSS_DYNAMIC_INTERFACE_TYPE_VAP) {
		status = nss_wifi_vdev_set_next_hop(nss_wifi_get_context(), if_num, mirror_if_num);
	} else if (if_num < NSS_MAX_PHYSICAL_INTERFACES) {
		status = nss_phys_if_set_nexthop(nss_ctx, if_num, mirror_if_num);
	} else {
		nss_mirror_warn("Invalid nexthop interface:%d type:%d\n", if_num, type);
		return -1;
	}

	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Setting nexthop failed. if_num: %u, nexthop: %u\n", if_num, mirror_if_num);
		return -1;
	}

	return 0;
}

/*
 * nss_mirror_ctl_parse_disable_ingress_pmc_cmd()
 *	API to parse ingress promiscuous disable command.
 */
static int nss_mirror_ctl_parse_disable_ingress_pmc_cmd(char *buffer)
{
	char *param, *value;
	struct nss_ctx_instance *nss_ctx;
	struct net_device *dev;
	nss_tx_status_t status;
	int32_t if_num, type = 0;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "devname")) {
		nss_mirror_warn("Invalid param: %s in promiscuous enable cmd\n", value);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &dev) < 0) {
		nss_mirror_warn("Error in getting devname net device.\n");
		return -1;
	}

	if ((if_num = nss_cmn_get_interface_number_by_dev(dev)) < 0) {
		nss_mirror_warn("No valid NSS FW interface for %s device\n", dev->name);
		dev_put(dev);
		return -1;
	}

	dev_put(dev);

	nss_ctx = nss_mirror_get_context();
	if (!nss_ctx) {
		nss_mirror_warn("Invalid NSS context\n");
		return -1;
	}

	type = nss_dynamic_interface_get_type(nss_ctx, if_num);

	if (type == NSS_DYNAMIC_INTERFACE_TYPE_VAP) {
		status = nss_mirror_reset_if_nexthop(if_num);
	} else if (if_num < NSS_MAX_PHYSICAL_INTERFACES) {
		status = nss_phys_if_reset_nexthop(nss_ctx, if_num);
	} else {
		nss_mirror_warn("Reset nexthop failed for %d interface, type:%d\n", if_num, type);
		return -1;
	}

	if (status != NSS_TX_SUCCESS) {
		nss_mirror_warn("Re-setting nexthop failed for if_num: %u\n", if_num);
		return -1;
	}

	return 0;
}

/*
 * nss_mirror_ctl_parse_destroy_cmd()
 *	API to parse destroy command.
 */
static int nss_mirror_ctl_parse_destroy_cmd(char *buffer)
{
	struct net_device *mirror_dev;
	char *param, *value;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid param %s in mirror delete command\n", param);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
		nss_mirror_warn("Error in getting mirror net device.\n");
		return -1;
	}

	/*
	 * Deconfigure mirror interface.
	 */
	if (nss_mirror_deconfigure_mirror(mirror_dev) < 0) {
		nss_mirror_warn("Error in deconfiguring mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);

	if (nss_mirror_destroy(mirror_dev)) {
		nss_mirror_warn("Error in sending delete config to mirror interface: %s\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	return 0;
}

/*
 * nss_mirror_ctl_parse_config_param()
 *	API to parse configure parameters.
 */
static int nss_mirror_ctl_parse_config_param(char *buffer, uint16_t *size_ptr,
		enum nss_mirror_pkt_clone_point *pt_ptr, uint16_t *offset_ptr)
{
	uint8_t param_num = NSS_MIRROR_CONFIG_PARAM_NUM;
	char *param, *value;

	do {
		param = nss_mirror_ctl_read_value(&buffer, &value, "=");

		/*
		 * No configure parameter is given. Use the default configure values.
		 */
		if (!param || !value) {
			return 0;
		}

		if (!strcmp(param, "mirror_size")) {
			if (nss_mirror_ctl_convert_char_to_u16(value, size_ptr)) {
				nss_mirror_warn("Not able to parse %s to u16\n", value);
				return -1;
			}
		} else if (!strcmp(param, "mirror_point")) {
			if (nss_mirror_ctl_convert_char_to_u32(value, pt_ptr)) {
				nss_mirror_warn("Not able to parse %s to u32\n", value);
				return -1;
			}
		} else if (!strcmp(param, "mirror_offset")) {
			if (nss_mirror_ctl_convert_char_to_u16(value, offset_ptr)) {
				nss_mirror_warn("Not able to parse %s to u16\n", value);
				return -1;
			}
		} else {
			nss_mirror_warn("Invalid param:%s in create cmd\n", param);
			return -1;
		}
	} while (--param_num);

	return 0;
}

/*
 * nss_mirror_ctl_parse_configure_cmd()
 *	API to parse configure command.
 */
static int nss_mirror_ctl_parse_configure_cmd(char *buffer)
{
	struct net_device *mirror_dev;
	char *param, *value;
	enum nss_mirror_pkt_clone_point mirror_point = NSS_MIRROR_PKT_CLONE_POINT_DEFAULT;
	uint16_t mirror_size = 0;
	uint16_t mirror_offset = 0;

	param = nss_mirror_ctl_read_value(&buffer, &value, "=");
	if (!param || !value) {
		return -1;
	}

	if (strcmp(param, "mirrordev")) {
		nss_mirror_warn("Invalid param %s in mirror configure command\n", param);
		return -1;
	}

	if (nss_mirror_ctl_get_netdev_by_name(value, &mirror_dev) < 0) {
		nss_mirror_warn("Error in getting mirror net device.\n");
		return -1;
	}

	if (nss_mirror_ctl_parse_config_param(buffer, &mirror_size,
				&mirror_point, &mirror_offset)) {
		dev_put(mirror_dev);
		nss_mirror_warn("Error in parsing mirror configure command\n");
		return -1;
	}

	nss_mirror_info("Mirror size: %d, mirror point: %d, mirror offset: %d\n",
			mirror_size, mirror_point, mirror_offset);

	if (nss_mirror_configure(mirror_dev, mirror_point,
			mirror_size, mirror_offset)) {
		nss_mirror_warn("Error in sending config message to %s mirror interface\n", mirror_dev->name);
		dev_put(mirror_dev);
		return -1;
	}

	dev_put(mirror_dev);
	return 0;
}

/*
 * nss_mirror_ctl_parse_cmd()
 *	API to parse mirror commands.
 */
static int32_t nss_mirror_ctl_parse_cmd(char *cmd)
{
	if (cmd == NULL) {
		return NSS_MIRROR_CMD_UNKNOWN;
	}

	if (!strcmp(cmd, "create")) {
		return NSS_MIRROR_CMD_CREATE;
	}

	if (!strcmp(cmd, "configure")) {
		return NSS_MIRROR_CMD_CONFIGURE;
	}

	if (!strcmp(cmd, "enable")) {
		return NSS_MIRROR_CMD_ENABLE;
	}

	if (!strcmp(cmd, "disable")) {
		return NSS_MIRROR_CMD_DISABLE;
	}

	if (!strcmp(cmd, "display")) {
		return NSS_MIRROR_CMD_DISPLAY;
	}

	if (!strcmp(cmd, "destroy")) {
		return NSS_MIRROR_CMD_DESTROY;
	}

	if (!strcmp(cmd, "set_nexthop")) {
		return NSS_MIRROR_CMD_SET_NEXTHOP;
	}

	if (!strcmp(cmd, "reset_nexthop")) {
		return NSS_MIRROR_CMD_RESET_NEXTHOP;
	}

	if (!strcmp(cmd, "ingress_pmc_enable")) {
		return NSS_MIRROR_CMD_INGRESS_PMC_ENABLE;
	}

	if (!strcmp(cmd, "ingress_pmc_disable")) {
		return NSS_MIRROR_CMD_INGRESS_PMC_DISABLE;
	}

	if (!strcmp(cmd, "help")) {
		return NSS_MIRROR_CMD_HELP;
	}

	nss_mirror_warn("Invalid string:%s in command\n", cmd);
	return NSS_MIRROR_CMD_UNKNOWN;
}

/*
 * nss_mirror_ctl_config_handler()
 *	Mirror sysctl config handler.
 */
static int nss_mirror_ctl_config_handler(struct ctl_table *ctl, int write,
		 void __user *buf, size_t *lenp, loff_t *ppos)
{
	char *buffer, *pfree;
	char * nextarg;
	int command, ret;
	size_t count = *lenp;

	/*
	 * Find the string, return an error if not found
	 */
	ret = proc_dostring(ctl, write, buf, lenp, ppos);
	if (ret || !write) {
		return ret;
	}

	buffer = vzalloc(count + 1);
	if (!buffer) {
		nss_mirror_warn("%px: Dynamic allocation failed for input buffer\n", ctl);
		return -ENOMEM;
	}

	pfree = buffer;

	if (copy_from_user(buffer, buf, count)) {
		vfree(pfree);
		return -EFAULT;
	}

	nextarg = nss_mirror_ctl_read_nextarg(&buffer);
	if (!nextarg) {
		goto err;
	}

	if (nextarg[strlen(nextarg) - 1] == '\n') {
		nextarg[strlen(nextarg) - 1] = '\0';
	}

	command = nss_mirror_ctl_parse_cmd(nextarg);
	switch (command) {

	case NSS_MIRROR_CMD_CREATE:
	{
		struct net_device *mirror_dev;
		if (!(mirror_dev = nss_mirror_create())) {
			nss_mirror_warn("%s Error in create cmd\n", __func__);
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_CONFIGURE:
	{
		if (nss_mirror_ctl_parse_configure_cmd(buffer)) {
			nss_mirror_warn("Error in parsing configure cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_SET_NEXTHOP:
	{
		if (nss_mirror_ctl_parse_set_nexthop_cmd(buffer)) {
			nss_mirror_warn("Error in parsing promiscuous rx enable cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_RESET_NEXTHOP:
	{
		if (nss_mirror_ctl_parse_reset_nexthop_cmd(buffer)) {
			nss_mirror_warn("Error in parsing promiscuous rx disable cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_INGRESS_PMC_ENABLE:
	{
		if (nss_mirror_ctl_parse_enable_ingress_pmc_cmd(buffer)) {
			nss_mirror_warn("Error in parsing set nexthop cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_INGRESS_PMC_DISABLE:
	{
		if (nss_mirror_ctl_parse_disable_ingress_pmc_cmd(buffer)) {
			nss_mirror_warn("Error in parsing reset nexthop cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_DISABLE:
	{
		if (nss_mirror_ctl_parse_disable_cmd(buffer)) {
			nss_mirror_warn("Error in parsing disable cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_ENABLE:
	{
		if (nss_mirror_ctl_parse_enable_cmd(buffer)) {
			nss_mirror_warn("Error in parsing enable cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_DISPLAY:
	{
		if (nss_mirror_ctl_parse_display_cmd(buffer)) {
			nss_mirror_warn("Error in parsing display cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_DESTROY:
	{
		if (nss_mirror_ctl_parse_destroy_cmd(buffer)) {
			nss_mirror_warn("Error in parsing destroy cmd\n");
			goto err;
		}
		break;
	}

	case NSS_MIRROR_CMD_HELP:
	{
		nss_mirror_ctl_display_help();
		break;
	}

	default:
		nss_mirror_warn("Invalid input in command.\n");
		goto err;
	}

	vfree(pfree);
	return ret;
err:
	vfree(pfree);
	return -EINVAL;
}

/*
 * nss mirror sysctl table
 */
static struct ctl_table nss_mirror_table[] = {
	{
		.procname	= "config",
		.data		= &nss_mirror_config_data,
		.maxlen		= sizeof(nss_mirror_config_data),
		.mode		= 0644,
		.proc_handler	= &nss_mirror_ctl_config_handler,
	},
	{ }
};

/*
 * nss mirror dir
 */
static struct ctl_table nss_mirror_root_dir[] = {
	{
		.procname		= "mirror",
		.mode			= 0555,
		.child			= nss_mirror_table,
	},
	{ }
};

/*
 * nss mirror sysctl nss root dir
 */
static struct ctl_table nss_mirror_nss_root_dir[] = {
	{
		.procname		= "nss",
		.mode			= 0555,
		.child			= nss_mirror_root_dir,
	},
	{ }
};

/*
 * nss mirror sysctl root dir
 */
static struct ctl_table nss_mirror_root[] = {
	{
		.procname		= "dev",
		.mode			= 0555,
		.child			= nss_mirror_nss_root_dir,
	},
	{ }
};

/*
 * nss_mirror_ctl_register()
 *	Register command line interface for mirror.
 */
int nss_mirror_ctl_register(void)
{
	nss_mirror_ctl_header = register_sysctl_table(nss_mirror_root);
	if (!nss_mirror_ctl_header) {
		nss_mirror_warn("Creating sysctl directory table header for mirror failed\n");
		return -1;
	}
	return 0;
}

/*
 * nss_mirror_ctl_unregister()
 *	Un-register command line interface for mirror.
 */
void nss_mirror_ctl_unregister(void)
{
	if (nss_mirror_ctl_header) {
		unregister_sysctl_table(nss_mirror_ctl_header);
	}
}
