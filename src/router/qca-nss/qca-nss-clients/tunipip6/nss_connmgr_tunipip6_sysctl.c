 /*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#include <net/ip_tunnels.h>
#include <net/ip6_tunnel.h>
#include <linux/if_arp.h>
#include <nss_api_if.h>
#include <linux/sysctl.h>
#include <linux/printk.h>
#include <linux/inet.h>
#include "nss_connmgr_tunipip6.h"

#define MAX_PROC_SIZE 1024
#define MAX_DATA_LEN 500
#define MAX_IPV4_PREFIX_LEN 32
#define MAX_IPV6_PREFIX_LEN 128
#define MAX_PSID_OFFSET_LEN 15
#define NETDEV_STR_LEN 30
#define PREFIX_STR_LEN 100

extern bool frag_id_update;

unsigned char nss_tunipip6_data[MAX_DATA_LEN] __read_mostly;
enum nss_tunipip6_sysctl_mode {
	NSS_TUNIPIP6_SYSCTL_ADD_MAPRULE,
	NSS_TUNIPIP6_SYSCTL_DEL_MAPRULE,
	NSS_TUNIPIP6_SYSCTL_FLUSH_FMR_RULE,
	NSS_TUNIPIP6_SYSCTL_FRAG_ID,
};


static int nss_tunipip6_data_parser(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos, enum nss_tunipip6_sysctl_mode mode)
{
	char dev_name[NETDEV_STR_LEN] = {0}, ipv6_prefix_str[PREFIX_STR_LEN] = {0}, ipv6_suffix_str[PREFIX_STR_LEN] = {0}, ipv4_prefix_str[PREFIX_STR_LEN] = {0};
	uint32_t ipv6_prefix[4], ipv6_prefix_len, ipv6_suffix[4], ipv6_suffix_len, ipv4_prefix, ipv4_prefix_len, ea_len, psid_offset;
	int rule_type = 0, frag_id = 0;
	bool ipv6_prefix_valid = false, ipv6_prefix_len_valid = false, ipv6_suffix_valid = false;
	bool ipv4_prefix_valid = false, ipv4_prefix_len_valid = false, ipv6_suffix_len_valid = false;
	bool rule_type_valid = false, ea_len_valid = false, psid_offset_valid = false, netdev_valid = false;
	struct nss_connmgr_tunipip6_maprule_cfg mrcfg = {0};
	char *buf;
	enum nss_connmgr_tunipip6_err_codes status;
	struct net_device *dev = NULL;
	char *pfree;
	char *token;
	int ret;
	int count;

        if (!write) {
                return -EINVAL;
        }

	buf = kzalloc(MAX_PROC_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}
	pfree = buf;
	count = *lenp;
	if (count > MAX_PROC_SIZE) {
		count = MAX_PROC_SIZE;
	}

	if (copy_from_user(buf, buffer, count)) {
		kfree(pfree);
		return -EFAULT;
	}

	while (buf) {
		char *param, *value;
		token = strsep(&buf, " \n");
		if (token[0] == 0) {
			continue;
		}

		param = strsep(&token, "=");
		value = token;

		if (!value || !param) {
			kfree(pfree);
			goto fail;
		}

		/*
		 * Parse netdev and FMR parameters.
		 */

		if (!strcmp(param, "netdev")) {
			strlcpy(dev_name, value, 30);
			dev = dev_get_by_name(&init_net, dev_name);
			if (!dev) {
				kfree(pfree);
				goto fail;
			}
			netdev_valid = true;
			continue;
		}

		if (!strcmp(param, "rule_type")) {
			if (!sscanf(value, "%u", &rule_type)) {
				kfree(pfree);
				goto fail;
			}

			if ((rule_type !=NSS_CONNMGR_TUNIPIP6_RULE_BMR) &&
				       (rule_type != NSS_CONNMGR_TUNIPIP6_RULE_FMR)) {
				kfree(pfree);
				goto fail;
			}
			rule_type_valid = true;
			continue;
		}

		if (!strcmp(param, "frag_id_update")) {
			if (!sscanf(value, "%u", &frag_id)) {
				kfree(pfree);
				goto fail;
			}

			if (frag_id != 0 && frag_id != 1) {
				kfree(pfree);
				goto fail;
			}
			continue;
		}

		if (!strcmp(param, "ipv4_prefix")) {
			strlcpy(ipv4_prefix_str, value, 30);
			ret = in4_pton(ipv4_prefix_str, -1, (uint8_t *)&ipv4_prefix, -1, NULL);
			if (ret != 1) {
				kfree(pfree);
				goto fail;
			}
			ipv4_prefix_valid = true;
			continue;
		}

		if (!strcmp(param, "ipv4_prefix_len")) {
			if (!sscanf(value, "%u", &ipv4_prefix_len)) {
				kfree(pfree);
				goto fail;
			}

			if (ipv4_prefix_len > MAX_IPV4_PREFIX_LEN) {
				kfree(pfree);
				goto fail;
			}

			ipv4_prefix_len_valid = true;
			continue;
		}

		if (!strcmp(param, "ipv6_prefix")) {
			strlcpy(ipv6_prefix_str, value, 100);
			ret = in6_pton(ipv6_prefix_str, -1, (uint8_t *)&ipv6_prefix, -1, NULL);
			if (ret != 1) {
				kfree(pfree);
				goto fail;
			}
			ipv6_prefix_valid = true;
			continue;
		}

		if (!strcmp(param, "ipv6_prefix_len")) {
			if (!sscanf(value, "%u", &ipv6_prefix_len)) {
				kfree(pfree);
				goto fail;
			}

			if (ipv6_prefix_len > MAX_IPV6_PREFIX_LEN) {
				kfree(pfree);
				goto fail;
			}

			ipv6_prefix_len_valid = true;
			continue;
		}

		if (!strcmp(param, "ipv6_suffix")) {
			strlcpy(ipv6_suffix_str, value, 100);
			ret = in6_pton(ipv6_suffix_str, -1, (uint8_t *)&ipv6_suffix, -1, NULL);
			if (ret != 1) {
				kfree(pfree);
				goto fail;
			}
			ipv6_suffix_valid = true;
			continue;
		}

		if (!strcmp(param, "ipv6_suffix_len")) {
			if (!sscanf(value, "%u", &ipv6_suffix_len)) {
				kfree(pfree);
				goto fail;
			}

			if (ipv6_suffix_len > MAX_IPV6_PREFIX_LEN) {
				kfree(pfree);
				goto fail;
			}

			ipv6_suffix_len_valid = true;
			continue;
		}

		if (!strcmp(param, "ea_len")) {
			if (!sscanf(value, "%u", &ea_len)) {
				kfree(pfree);
				goto fail;
			}

			ea_len_valid = true;
			continue;
		}

		if (!strcmp(param, "psid_offset")) {
			if (!sscanf(value, "%u", &psid_offset)) {
				kfree(pfree);
				goto fail;
			}

			if (psid_offset> MAX_PSID_OFFSET_LEN) {
				kfree(pfree);
				goto fail;
			}

			psid_offset_valid = true;
			continue;
		}
	}

	kfree(pfree);

	/*
	 * Netdev param is not needed only for frag_id command
	 */
	if (!netdev_valid && mode != NSS_TUNIPIP6_SYSCTL_FRAG_ID) {
		goto fail;
	}

	switch(mode) {
	case NSS_TUNIPIP6_SYSCTL_DEL_MAPRULE:
		if (!rule_type_valid ) {
			goto fail;
		}

		/*
		 * BMR delete will follow this and FMR delete will fall through.
		 */
		if (rule_type == NSS_CONNMGR_TUNIPIP6_RULE_BMR) {
			if ((ipv6_prefix_valid || ipv6_prefix_len_valid || ipv6_suffix_valid ||
				ipv4_prefix_valid || ipv4_prefix_len_valid || ea_len_valid ||
				psid_offset_valid)) {
				goto fail;
			}

			mrcfg.rule_type = rule_type;
			status = nss_connmgr_tunipip6_del_maprule(dev, &mrcfg);
			if (status == NSS_CONNMGR_TUNIPIP6_SUCCESS) {
				pr_info("Map Rule delete success for netdev: %s\n", dev->name);
			} else {
				pr_info("Map Rule delete failure for netdev: %s\n", dev->name);
			}
			break;
		}
#if __has_attribute(__fallthrough__)
	__attribute__((__fallthrough__));
#endif
	case NSS_TUNIPIP6_SYSCTL_ADD_MAPRULE:
		if (!(rule_type_valid && ipv6_prefix_valid && ipv6_prefix_len_valid && ipv6_suffix_valid &&
			ipv4_prefix_valid && ipv4_prefix_len_valid && ea_len_valid &&
			psid_offset_valid)) {
			goto fail;
		}

		mrcfg.rule_type = rule_type;
		mrcfg.ipv6_prefix[0] = ntohl(ipv6_prefix[0]);
		mrcfg.ipv6_prefix[1] = ntohl(ipv6_prefix[1]);
		mrcfg.ipv6_prefix[2] = ntohl(ipv6_prefix[2]);
		mrcfg.ipv6_prefix[3] = ntohl(ipv6_prefix[3]);
		mrcfg.ipv6_prefix_len = ipv6_prefix_len;

		mrcfg.ipv4_prefix = ntohl(ipv4_prefix);
		mrcfg.ipv4_prefix_len = ipv4_prefix_len;

		mrcfg.ipv6_suffix[0] = ntohl(ipv6_suffix[0]);
		mrcfg.ipv6_suffix[1] = ntohl(ipv6_suffix[1]);
		mrcfg.ipv6_suffix[2] = ntohl(ipv6_suffix[2]);
		mrcfg.ipv6_suffix[3] = ntohl(ipv6_suffix[3]);
		mrcfg.ipv6_suffix_len = ipv6_suffix_len;

		mrcfg.ea_len = ea_len;
		mrcfg.psid_offset = psid_offset;

		if (mode == NSS_TUNIPIP6_SYSCTL_ADD_MAPRULE) {
			status = nss_connmgr_tunipip6_add_maprule(dev, &mrcfg);
			if (status == NSS_CONNMGR_TUNIPIP6_SUCCESS) {
				pr_info("Map Rule create success for netdev: %s\n", dev->name);
			} else {
				pr_info("Map Rule create failure for netdev: %s\n", dev->name);
			}
		} else {
			status = nss_connmgr_tunipip6_del_maprule(dev, &mrcfg);
			if (status == NSS_CONNMGR_TUNIPIP6_SUCCESS) {
				pr_info("Map Rule delete success for netdev: %s\n", dev->name);
			} else {
				pr_info("Map Rule delete failure for netdev: %s\n", dev->name);
			}
		}
		break;

	case NSS_TUNIPIP6_SYSCTL_FLUSH_FMR_RULE:
		status = nss_connmgr_tunipip6_flush_fmr_rule(dev);
		if (status == NSS_CONNMGR_TUNIPIP6_SUCCESS) {
			pr_info("Map Rule flush success for netdev: %s\n", dev->name);
		} else {
			pr_info("Map Rule flush failed for netdev: %s\n", dev->name);
		}
		break;

	case NSS_TUNIPIP6_SYSCTL_FRAG_ID:
		if ((netdev_valid || rule_type_valid || ipv6_prefix_valid || ipv6_prefix_len_valid || ipv6_suffix_valid ||
				ipv4_prefix_valid || ipv4_prefix_len_valid || ea_len_valid ||
				psid_offset_valid)) {
				goto fail;
		}

		if (frag_id) {
			frag_id_update = true;
			pr_info("Frag Id enabled for all tunnels.\n");
		} else {
			frag_id_update = false;
			pr_info("Frag Id disabled for all tunnels.\n");
		}
		return 0;
	}

	dev_put(dev);
	return 0;

fail:
	if (dev) {
		dev_put(dev);
	}

	pr_info("Wrong input, check help. (cat /proc/sys/dev/nss/ipip6/help)\n");
	return 0;
}

static int nss_tunipip6_cmd_procfs_add_maprule(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return nss_tunipip6_data_parser(ctl, write, buffer, lenp, ppos, NSS_TUNIPIP6_SYSCTL_ADD_MAPRULE);
}

static int nss_tunipip6_cmd_procfs_del_maprule(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return nss_tunipip6_data_parser(ctl, write, buffer, lenp, ppos, NSS_TUNIPIP6_SYSCTL_DEL_MAPRULE);
}

static int nss_tunipip6_cmd_procfs_flush_fmr_rule(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return nss_tunipip6_data_parser(ctl, write, buffer, lenp, ppos, NSS_TUNIPIP6_SYSCTL_FLUSH_FMR_RULE);
}

static int nss_tunipip6_cmd_procfs_enable_frag_id(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	return nss_tunipip6_data_parser(ctl, write, buffer, lenp, ppos, NSS_TUNIPIP6_SYSCTL_FRAG_ID);
}

static int nss_tunipip6_cmd_procfs_read_help(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec(ctl, write, buffer, lenp, ppos);

	pr_info("\nHelp: (/proc/sys/dev/nss/ipip6/help) \n\
			1. To add maprule(rule_type=1(BMR)/2(FMR)):\n\
			echo dev=<map-mape/MAP-E netdevice> rule_type=<1/2> ipv6_prefix=<XXXX::XXXX> ipv6_prefix_len=<XX> ipv4_prefix=<X.X.X.X> ipv4_prefix_len=<XX> \n\
			ipv6_suffix=<XXXX::XXXX> ipv6_sufffix_len=<XX> ea_len=<XX> psid_offset=<XX> > add_map_rule \n\
			2. a. To delete maprule(FMR):\n\
			echo dev=<map-mape/MAP-E netdevice> rule_type=<2> ipv6_prefix=<XXXX::XXXX> ipv6_prefix_len=<XX> ipv4_prefix=<X.X.X.X> ipv4_prefix_len=<XX> \n\
			ipv6_suffix=<XXXX::XXXX> ipv6_sufffix_len=<XX> ea_len=<XX> psid_offset=<XX> > remove_map_rule \n\
			b. To delete maprule(BMR):\n\
			echo dev=<map-mape/MAP-E netdevice> rule_type=<1> > remove_map_rule\n\
			3. To flush FMR entries:\n\
			echo dev=<map-mape/MAP-E netdevice> > flush_fmr_rule\n");
	pr_info("\t\t\t4. To enable/disable frag id: \n\
			echo frag_id_update=<0/1> > frag_id \n\
			=====end of help=====\n");
	*lenp = 0;
	return ret;
}

static struct ctl_table nss_tunipip6_table[] = {
	{
		.procname		= "add_map_rule",
		.data			= &nss_tunipip6_data,
		.maxlen			= sizeof(nss_tunipip6_data),
		.mode			= 0644,
		.proc_handler		= &nss_tunipip6_cmd_procfs_add_maprule,
	},
	{
		.procname		= "remove_map_rule",
		.data			= &nss_tunipip6_data,
		.maxlen			= sizeof(nss_tunipip6_data),
		.mode			= 0644,
		.proc_handler		= &nss_tunipip6_cmd_procfs_del_maprule,
	},
	{
		.procname		= "flush_fmr_rule",
		.data			= &nss_tunipip6_data,
		.maxlen			= sizeof(nss_tunipip6_data),
		.mode			= 0644,
		.proc_handler		= &nss_tunipip6_cmd_procfs_flush_fmr_rule,
	},
	{
		.procname               = "frag_id",
		.data                   = &nss_tunipip6_data,
		.maxlen                 = sizeof(nss_tunipip6_data),
		.mode                   = 0644,
		.proc_handler           = &nss_tunipip6_cmd_procfs_enable_frag_id,
	},
	{
		.procname		= "help",
		.data			= &nss_tunipip6_data,
		.maxlen			= sizeof(nss_tunipip6_data),
		.mode			= 0400,
		.proc_handler		= &nss_tunipip6_cmd_procfs_read_help,
	},
	{ }
};

static struct ctl_table nss_tunipip6_root_dir[] = {
	{
		.procname		= "ipip6",
		.mode			= 0555,
		.child			= nss_tunipip6_table,
	},
	{ }
};

static struct ctl_table nss_tunipip6_nss_root_dir[] = {
	{
		.procname		= "nss",
		.mode			= 0555,
		.child			= nss_tunipip6_root_dir,
	},
	{ }
};

static struct ctl_table nss_tunipip6_root[] = {
	{
		.procname		= "dev",
		.mode			= 0555,
		.child			= nss_tunipip6_nss_root_dir,
	},
	{ }
};

static struct ctl_table_header *nss_tunipip6_ctl_header;

/*
 * nss_tunipip6_sysctl_register()
 * 	Register command line interface for tunipip6.
 */
bool nss_tunipip6_sysctl_register(void) {
	nss_tunipip6_ctl_header = register_sysctl_table(nss_tunipip6_root);
	if (!nss_tunipip6_ctl_header) {
		return false;
	}

	return true;
}

/*
 * nss_tunipip6_sysctl_unregister()
 * 	Unregister command line interface for tunipip6.
 */
void nss_tunipip6_sysctl_unregister(void) {
	if (nss_tunipip6_ctl_header) {
		unregister_sysctl_table(nss_tunipip6_ctl_header);
	}
}
