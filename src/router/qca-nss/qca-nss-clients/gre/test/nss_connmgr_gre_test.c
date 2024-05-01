/*
 **************************************************************************
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * nss_connnmgr_gre_test.c
 *	test module implementation
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/inet.h>
#include <net/ipv6.h>
#include <net/ip6_tunnel.h>
#include <net/ip_tunnels.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <nss_api_if.h>

#include "nss_connmgr_gre_public.h"
#include "nss_connmgr_gre.h"

#define MAX_PROC_SIZE 1024
static struct proc_dir_entry *proc_entry;

/*
 * nss_connmgr_gre_test()
 *	Function calls API (exported by client code) to create GRE tap interface
 *	and destroy interface based on command writter to /proc/gre entry.
 */
static int nss_connmgr_gre_test(char *src_ip, char *dest_ip, char *next_dev_name)
{
	int ret = -1;
	uint32_t sip, dip;
	struct nss_connmgr_gre_cfg cfg;
	struct net_device *dev, *next_dev;
	enum nss_connmgr_gre_err_codes err_code;

	ret = in4_pton(src_ip, -1, (uint8_t *)&sip, -1, NULL);
	if (ret != 1) {
		pr_err("Not a valid src ip\n");
		return -1;
	}

	ret = in4_pton(dest_ip, -1, (uint8_t *)&dip, -1, NULL);
	if (ret != 1) {
		pr_err("Not a valid dest ip\n");
		return -1;
	}

	sip = ntohl(sip);
	dip = ntohl(dip);

	if (!next_dev_name) {
		pr_err("next_dev=%s is not a valid param", next_dev_name);
		return -1;
	}

	next_dev = dev_get_by_name(&init_net, next_dev_name);
	if (!next_dev) {
		pr_err("next_dev=%s is not a valid param", next_dev_name);
		return -1;
	}

	memset(&cfg, 0, sizeof(struct nss_connmgr_gre_cfg));

	cfg.mode = GRE_MODE_TAP;
	cfg.ip_type = GRE_OVER_IPV4;
	cfg.ttl_inherit = true;
	cfg.tos_inherit = true;
	cfg.add_padding = false;
	cfg.copy_metadata = true;
	cfg.next_dev = next_dev;

	cfg.src_ip[0] = sip;
	cfg.dest_ip[0] = dip;

	dev = nss_connmgr_gre_create_interface(&cfg, &err_code);
	dev_put(next_dev);
	if (!dev) {
		pr_err("Could not create gre interface, err=%d\n", err_code);
		return -1;
	}

	pr_info("Gre interface %s created\n", dev->name);
	return 0;
}

/*
 * nss_connmgr_gre_test_write_proc()
 *	Write call back for proc entry.
 */
static ssize_t nss_connmgr_gre_test_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	char *token;
	struct net_device *dev;
	char saddr[20] = {0}, daddr[20] = {0}, dev_name[IFNAMSIZ] = {0};
	bool saddr_valid = false, daddr_valid = false, dev_name_valid = false;
	char *buffer = kzalloc(MAX_PROC_SIZE, GFP_KERNEL);
	char *pfree;
	int ret;

	if (!buffer) {
		return -ENOMEM;
	}
	pfree = buffer;

	if (count > MAX_PROC_SIZE) {
		count = MAX_PROC_SIZE;
	}

	if (copy_from_user(buffer, buf, count)) {
		kfree(pfree);
		return -EFAULT;
	}

	while (buffer) {
		char *param, *value;
		token = strsep(&buffer, " \n");
		if (token[0] == 0) {
			continue;
		}

		param = strsep(&token, "=");
		value = token;

		/*
		 * parameter parsing for delete command
		 */
		if (!strcmp(param, "dev")) {
			strlcpy(dev_name, value, IFNAMSIZ);
			dev_name_valid = true;
			break;
		}

		/*
		 * Parse next_dev, src ip and dest ip for GRE
		 * tap create command
		 */
		if (!strcmp(param, "next_dev")) {
			strlcpy(dev_name, value, IFNAMSIZ);
			dev_name_valid = true;
			continue;
		}

		if (!strcmp(param, "saddr")) {
			strlcpy(saddr, value, 20);
			saddr_valid = true;
			continue;
		}

		if (!strcmp(param, "daddr")) {
			strlcpy(daddr, value, 20);
			daddr_valid = true;
			continue;
		}

	}

	kfree(pfree);

	if (dev_name_valid && !saddr_valid && !daddr_valid) {
		dev = dev_get_by_name(&init_net, dev_name);
		if (!dev) {
			pr_err("Invalid arguments\n");
			return -EINVAL;
		}
		dev_put(dev);

		ret = nss_connmgr_gre_destroy_interface(dev);
		if (ret) {
			pr_err("Could not delete interface, failed with errcode=%d\n", ret);
			return -EINVAL;
		}

		pr_info("Sucessfully deleted %s interface\n", dev_name);
		return count;
	}

	if (!(dev_name_valid && saddr_valid && daddr_valid)) {
		pr_err("Invalid arguments\n");
		return -EINVAL;
	}

	if (nss_connmgr_gre_test(saddr, daddr, dev_name)) {
		return -EFAULT;
	}

	return count;
}

/*
 * nss_connmgr_gre_test_show_proc()
 *	Displays help text on how to use this module.
 */
static int nss_connmgr_gre_test_show_proc(struct seq_file *filp, void *data)
{
	seq_printf(filp, "Help:\n** Create interface **\n");
	seq_printf(filp, "echo \"saddr=192.168.10.1 daddr=192.168.10.100 next_dev=ath0\" > /proc/gre\n\n");
	seq_printf(filp, "\n** Delete interface **\n");
	seq_printf(filp, "echo \"dev=tap-0\" > /proc/gre\n\n");
	return 0;
}

/*
 * nss_connmgr_gre_test_open_proc()
 *	Open call back for proc entry.
 */
static int nss_connmgr_gre_test_open_proc(struct inode *inode, struct file *filp)
{
	return single_open(filp, nss_connmgr_gre_test_show_proc, pde_data(inode));
}

/*
 * Proc ops
 */
static const struct proc_ops nss_connmgr_gre_test_proc_ops =  {
	.proc_open	= nss_connmgr_gre_test_open_proc,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= nss_connmgr_gre_test_write_proc,
};

/*
 * nss_connmgr_gre_test_init_module()
 *	Test module init function.
 */
static int __init nss_connmgr_gre_test_init_module(void)
{
	proc_entry = proc_create_data("gre", S_IWUGO, NULL,
					    &nss_connmgr_gre_test_proc_ops,
					    NULL);

	if (!proc_entry) {
		pr_err("Creation of proc entry faild\n");
		return -1;
	}

	pr_info("Read Help: cat /proc/gre\n");
	return 0;
}

/*
 * nss_connmgr_gre_test_exit_module
 *	Test module exit function
 */
static void __exit nss_connmgr_gre_test_exit_module(void)
{
	proc_remove(proc_entry);
}

module_init(nss_connmgr_gre_test_init_module);
module_exit(nss_connmgr_gre_test_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS gre test module");
