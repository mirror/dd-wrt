/*
 **************************************************************************
 * Copyright (c) 2021 The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/in6.h>
#include <linux/list.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <net/ipv6.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "ecm_ae_classifier_public.h"

#define RULE_FIELDS 7
#define IPV4 4
#define IPV6 6

/*
 * DebugFS entry object.
 */
static struct dentry *ecm_ae_select_test_dentry;

/*
 * The rule and AE storage table.
 */
struct ecm_ae_select_test_rule {
	struct list_head list;
	unsigned int proto;
	ecm_ae_classifier_result_t accel;
	int src_port;
	int dest_port;
	unsigned int ip_version;
	union {
		__be32 v4_addr;
		struct in6_addr v6_addr;
	} src_addr;
	union {
		__be32 v4_addr;
		struct in6_addr v6_addr;
	} dest_addr;
};

LIST_HEAD(ecm_ae_select_test_rules);
DEFINE_SPINLOCK(ecm_ae_select_test_rules_lock);

/*
 * ecm_ae_select_test_rule_find()
 *	Return matching rule
 */
static struct ecm_ae_select_test_rule *ecm_ae_select_test_rule_find(unsigned int proto,
					    struct in6_addr *src_addr6,
					    struct in6_addr *dest_addr6,
					    int src_port, int dest_port,
					    __be32 *src_addr,
					    __be32 *dest_addr,
					    unsigned int ip_version)
{
	struct ecm_ae_select_test_rule *rule = NULL;

	list_for_each_entry(rule, &ecm_ae_select_test_rules, list) {

		if (ip_version == IPV6) {
			if (rule->proto == proto && rule->src_port == src_port
				&& rule->dest_port == dest_port && ip_version == rule->ip_version
				&& src_addr == NULL && dest_addr == NULL
				&& ipv6_addr_equal(&rule->src_addr.v6_addr, src_addr6)
				&& ipv6_addr_equal(&rule->dest_addr.v6_addr, dest_addr6))
				return rule;
		} else 	{
			if (rule->proto == proto && rule->src_port == src_port
				&& rule->dest_port == dest_port && ip_version == rule->ip_version
				&& src_addr6 == NULL && dest_addr6 == NULL
				&& rule->src_addr.v4_addr == *src_addr
				&& rule->dest_addr.v4_addr == *dest_addr)
				return rule;
		}
	}

	return NULL;
}

/*
 * This is the module which selects the underlying acceleration engine
 * based on the 5-tuple and type of the flow. The flow can be multicast, routed,
 * bridged, ported/non-ported.
 */

/*
 * ecm_ae_select()
 *	 Selects the acceleration engine based on the given flow information.
 */
ecm_ae_classifier_result_t ecm_ae_select(struct ecm_ae_classifier_info *info)
{
	struct ecm_ae_select_test_rule *rule = NULL;
	ecm_ae_classifier_result_t accel;
	unsigned int ip_version = info->ip_ver;
	pr_debug("%px: Acceleration engine selection\n", info);

	spin_lock_bh(&ecm_ae_select_test_rules_lock);

	if (info->ip_ver == IPV6) {
		rule = ecm_ae_select_test_rule_find(info->protocol, &info->src.v6_addr,
											 &info->dest.v6_addr,
											 info->src_port, info->dst_port, NULL, NULL, ip_version);
	} else {
		rule = ecm_ae_select_test_rule_find(info->protocol, NULL, NULL,
											  info->src_port, info->dst_port,
											  &info->src.v4_addr, &info->dest.v4_addr, ip_version);
	}

	/*
	 *We don't care the acceleration mode if there is no registered flow in our table.
	 */
	if (!rule) {
		spin_unlock_bh(&ecm_ae_select_test_rules_lock);
		return ECM_AE_CLASSIFIER_RESULT_DONT_CARE;
	}

	accel = rule->accel;
	spin_unlock_bh(&ecm_ae_select_test_rules_lock);

	/*
	 * Multicast flows don't care the acceleration mode.
	 */
	if (info->flag & ECM_AE_CLASSIFIER_FLOW_MULTICAST)
		return ECM_AE_CLASSIFIER_RESULT_DONT_CARE;

	/*
	 * Let's accelerate all routed and bridge flows by registered accelration mode.
	 */
	return accel;
}

static struct ecm_ae_classifier_ops ae_ops = {
	.ae_get = ecm_ae_select,
	.ae_flags = (ECM_AE_CLASSIFIER_FLAG_EXTERNAL_AE_REGISTERED)
};

/*
 * ecm_ae_select_test_delete_rule()
 *	Delete a rule, return 0 on failure.
 */
static unsigned int ecm_ae_select_test_delete_rule(unsigned int proto, struct in6_addr *src_addr6,
				struct in6_addr *dest_addr6, int src_port,
				int dest_port, __be32 *src_addr,
				__be32 *dest_addr, unsigned int ip_version)
{
	struct ecm_ae_select_test_rule *rule;

	spin_lock_bh(&ecm_ae_select_test_rules_lock);
	if (ip_version == IPV6) {
		rule = ecm_ae_select_test_rule_find(proto, src_addr6, dest_addr6,
				src_port, dest_port, NULL, NULL, ip_version);
	} else {
		rule = ecm_ae_select_test_rule_find(proto, NULL, NULL,
				src_port, dest_port, src_addr, dest_addr, ip_version);
	}

	if (!rule) {
		spin_unlock_bh(&ecm_ae_select_test_rules_lock);
		return 0;
	}

	list_del(&rule->list);
	spin_unlock_bh(&ecm_ae_select_test_rules_lock);
	kfree(rule);

	return 1;
}

/*
 * ecm_ae_select_test_add_rule()
 * 	Add a new rule, return 0 on failure.
 */
static unsigned int ecm_ae_select_test_add_rule(ecm_ae_classifier_result_t accel,
				unsigned int proto, struct in6_addr *src_addr6,
				struct in6_addr *dest_addr6, int src_port,
				int dest_port, __be32 *src_addr,
				__be32 *dest_addr, unsigned int ip_version)
{
	struct ecm_ae_select_test_rule *new_rule = NULL;

	spin_lock_bh(&ecm_ae_select_test_rules_lock);

	if (ip_version == IPV6) {
		new_rule = ecm_ae_select_test_rule_find(proto, src_addr6, dest_addr6,
				src_port, dest_port, NULL, NULL, ip_version);
	} else {
		new_rule = ecm_ae_select_test_rule_find(proto, NULL, NULL,
				src_port, dest_port, src_addr, dest_addr, ip_version);
	}

	if (new_rule) {
		pr_info("Rule already registered\n");
		spin_unlock_bh(&ecm_ae_select_test_rules_lock);
		return 0;
	}

	spin_unlock_bh(&ecm_ae_select_test_rules_lock);

	new_rule = kzalloc(sizeof(struct ecm_ae_select_test_rule), GFP_ATOMIC);
	if (!new_rule)
		return 0;

	new_rule->accel = accel;
	new_rule->proto = proto;
	new_rule->src_port = src_port;
	new_rule->dest_port = dest_port;
	new_rule->ip_version = ip_version;
	if (ip_version == IPV6) {
		new_rule->src_addr.v6_addr = *src_addr6;
		new_rule->dest_addr.v6_addr = *dest_addr6;
	} else {
		new_rule->src_addr.v4_addr = *src_addr;
		new_rule->dest_addr.v4_addr = *dest_addr;
	}

	INIT_LIST_HEAD(&new_rule->list);

	spin_lock_bh(&ecm_ae_select_test_rules_lock);
	list_add(&new_rule->list, &ecm_ae_select_test_rules);
	spin_unlock_bh(&ecm_ae_select_test_rules_lock);
	pr_info("New Rule registered\n");

	return 1;
}

/*
 * ecm_ae_select_test_str_to_ip()
 *	Convert string IP to in6_addr.  Return IPv4 or IPv6.
	Return something else in error.
 *
 * NOTE: When string is IPv4 the lower 32 bit word of the in6 address
 * contains the address. Network order.
 */
static unsigned int ecm_ae_select_test_str_to_ip(char *ip_str, struct in6_addr *addr, __be32 *ipv4_addr)
{
	uint8_t *ptr = (uint8_t *)(addr->s6_addr);

	/*
	 * IPv4 address in addr->s6_addr32[0]
	 */
	if (in4_pton(ip_str, -1, ptr, '\0', NULL) > 0) {
		*ipv4_addr = addr->s6_addr32[0];
		return IPV4;
	}

	/*
	 * IPv6
	 */
	if (in6_pton(ip_str, -1, ptr, '\0', NULL) > 0)
		return IPV6;

	return 0;
}

/*
 * ecm_ae_select_test_rule_write()
 *	Write a rule
 */
static ssize_t ecm_ae_select_test_rule_write(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char *rule_buf;
	int field_count;
	char *field_ptr;
	char *fields[RULE_FIELDS];
	unsigned int oper;
	ecm_ae_classifier_result_t accel;
	unsigned int proto;
	int src_port;
	int dest_port;
	union {
		__be32 v4_addr;
		struct in6_addr v6_addr;
	} src_addr;
	union {
		__be32 v4_addr;
		struct in6_addr v6_addr;
	} dest_addr;
	unsigned int ip_version;
	__be32 src;
	__be32 dest;

	/*
	 * buf is formed as:
	 * [0]           [1]     [2]        [3]        [4]         [5]         [6]
	 * <0=del,1=add>/<proto>/<src_addr>/<src_port>/<dest_addr>/<dest_port>/<2= ECM_AE_CLASSIFIER_RESULT_SFE, 3= ECM_AE_CLASSIFIER_RESULT_PPE_VP, 4= ECM_AE_CLASSIFIER_RESULT_PPE_DS, 5= ECM_AE_CLASSIFIER_RESULT_NONE 7= ECM_AE_CLASSIFIER_RESULT_DONT_CARE>
	 * e.g.:
	 * echo "1/17/192.168.1.254/1235/192.168.1.251/1234/2" > /sys/kernel/debug/ecm_ae_select_test/rule
	 * cat /sys/kernel/debug/ecm_ae_select_test/rule (shows all rules)
	 */

	rule_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!rule_buf)
		return -EINVAL;

	if (copy_from_user(rule_buf, user_buf, count)) {
		kfree(rule_buf);
		return -EFAULT;
	}

	/*
	 * Split the buffer into its fields
	 */
	field_count = 0;
	field_ptr = rule_buf;
	fields[field_count] = strsep(&field_ptr, "/");
	while (fields[field_count] != NULL) {
		pr_info("Field %d: %s\n", field_count, fields[field_count]);
		field_count++;
		if (field_count == RULE_FIELDS)
			break;
		fields[field_count] = strsep(&field_ptr, "/ \n");
	}

	if (field_count != RULE_FIELDS) {
		pr_info("Invalid field count %d\n", field_count);
		kfree(rule_buf);
		return -EINVAL;
	}

	/*
	 * Convert fields
	 */
	if (sscanf(fields[0], "%u", &oper) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[6], "%d", (int *)&accel) != 1)
		goto sscanf_read_error;

	switch (accel) {
	case ECM_AE_CLASSIFIER_RESULT_SFE:
	case ECM_AE_CLASSIFIER_RESULT_PPE_VP:
	case ECM_AE_CLASSIFIER_RESULT_PPE_DS:
	case ECM_AE_CLASSIFIER_RESULT_NONE:
	case ECM_AE_CLASSIFIER_RESULT_DONT_CARE:
		break;
	default:
		pr_info("Unsupported selection: %u\n", accel);
		kfree(rule_buf);
		return -EINVAL;
	}

	if (sscanf(fields[1], "%u", &proto) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[3], "%d", &src_port) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[5], "%d", &dest_port) != 1)
		goto sscanf_read_error;

	ip_version = ecm_ae_select_test_str_to_ip(fields[2], &src_addr.v6_addr, &src);
	if (ip_version != ecm_ae_select_test_str_to_ip(fields[4], &dest_addr.v6_addr, &dest)) {
		pr_info("Conflicting IP address types\n");
		kfree(rule_buf);
		return -EINVAL;
	}

	if (ip_version == IPV4) {
		src_addr.v4_addr = src;
		dest_addr.v4_addr = dest;
	}

	kfree(rule_buf);

	pr_info("oper: %u\n"
			"accel: %d\n"
			"proto: %u\n"
			"src_port: %d\n"
			"dest_port: %d\n"
			"ip_version: %u\n",
			oper,
			(int)accel,
			proto,
			src_port,
			dest_port,
			ip_version
			);
	if (ip_version == IPV4) {
		pr_info("src_addr: %pI4\n", &src_addr.v4_addr);
		pr_info("dest_addr: %pI4\n", &dest_addr.v4_addr);
	} else {
		pr_info("src_addr: %pI6\n", &src_addr.v6_addr);
		pr_info("dest_addr: %pI6\n", &dest_addr.v6_addr);
	}

	if (oper == 0) {
		pr_info("Delete\n");
		if (ip_version == IPV6) {
			if (!ecm_ae_select_test_delete_rule(proto, &src_addr.v6_addr,
								&dest_addr.v6_addr, src_port, dest_port,
								NULL, NULL, ip_version))
				return -EINVAL;
		} else {
			if (!ecm_ae_select_test_delete_rule(proto, NULL, NULL,
								src_port, dest_port, &src_addr.v4_addr,
								&dest_addr.v4_addr, ip_version))
			return -EINVAL;
		}
	} else if (oper == 1) {
		pr_info("Add\n");
		if (ip_version == IPV6) {
			if (!ecm_ae_select_test_add_rule(accel, proto, &src_addr.v6_addr,
								&dest_addr.v6_addr, src_port, dest_port,
								NULL, NULL, ip_version))
				return -EINVAL;
		} else {
			if (!ecm_ae_select_test_add_rule(accel, proto, NULL, NULL,
								src_port, dest_port, &src_addr.v4_addr,
								&dest_addr.v4_addr, ip_version))
			    return -EINVAL;
		}
	} else {
		pr_info("Unknown operation: %u\n", oper);
		return -EINVAL;
	}

	return count;

sscanf_read_error:
	pr_info("sscanf read error\n");
	kfree(rule_buf);
	return -EINVAL;
}

/*
 * ecm_ae_select_test_rule_seq_show()
 * 	Shows the rules registered, return 0 on success.
 */
static int ecm_ae_select_test_rule_seq_show(struct seq_file *m, void *v)
{
	struct ecm_ae_select_test_rule *rule;
	rule = list_entry(v, struct ecm_ae_select_test_rule, list);

	seq_printf(m, "RULE:\n"
			"\taccel: %d\n"
			"\tproto: %u\n"
			"\tsrc_port: %d\n"
			"\tdest_port: %d\n"
			"\tipv: %u\n",
			(int)(rule->accel),
			rule->proto,
			rule->src_port,
			rule->dest_port,
			rule->ip_version
			);
	if (rule->ip_version == IPV4) {
		seq_printf(m, "\tsrc_addr: %pI4\n"
			      "\tdest_addr: %pI4\n",
				&rule->src_addr.v4_addr,
				&rule->dest_addr.v4_addr);
	} else {
		seq_printf(m, "\tsrc_addr: %pI6\n"
			      "\tdest_addr: %pI6\n",
				&rule->src_addr.v6_addr,
				&rule->dest_addr.v6_addr);
	}

	return 0;
}

/*
 * ecm_ae_select_test_rule_seq_stop()
 *	We no longer require the file to read or the file is closed.
 */
static void ecm_ae_select_test_rule_seq_stop(struct seq_file *p, void *v)
{
	spin_unlock_bh(&ecm_ae_select_test_rules_lock);
}

/*
 * ecm_ae_select_test_rule_seq_start()
 *	Aquires the lock and initialize the iterator pointing to the table.
 */
static void *ecm_ae_select_test_rule_seq_start(struct seq_file *m, loff_t *_pos)
{
	spin_lock_bh(&ecm_ae_select_test_rules_lock);
	return seq_list_start(&ecm_ae_select_test_rules, *_pos);
}

/*
 * ecm_ae_select_test_rule_seq_next()
 *	points the iterator to the next entry.
 */
static void *ecm_ae_select_test_rule_seq_next(struct seq_file *p, void *v,
					loff_t *pos)
{
	return seq_list_next(v, &ecm_ae_select_test_rules, pos);
}

static const struct seq_operations ecm_ae_select_test_rule_seq_ops = {
	.start = ecm_ae_select_test_rule_seq_start,
	.next  = ecm_ae_select_test_rule_seq_next,
	.stop  = ecm_ae_select_test_rule_seq_stop,
	.show  = ecm_ae_select_test_rule_seq_show,
};

/*
 * ecm_ae_select_test_rule_open()
 * 	The operations required to open the file
 */
static int ecm_ae_select_test_rule_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ecm_ae_select_test_rule_seq_ops);
}

/*
 * ecm_ae_select_test_rule_release()
 * 	resoure cleanup and deallocation
 */
static int ecm_ae_select_test_rule_release(struct inode *inode, struct file *file)
{
	return seq_release(inode, file);
}

static const struct file_operations ecm_ae_select_test_rule_fops = {
	.owner		= THIS_MODULE,
	.open		= ecm_ae_select_test_rule_open,
	.write		= ecm_ae_select_test_rule_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= ecm_ae_select_test_rule_release,
};

/*
 * ecm_ae_select_init()
 * 	create a directory, file to store the external input and register the callback for acceleration engine selection.
 */
static int __init ecm_ae_select_init(void)
{
	pr_info("ECM AE Select INIT\n");

	/*
	 * Create entries in DebugFS for control functions
	 */
	ecm_ae_select_test_dentry = debugfs_create_dir("ecm_ae_select_test", NULL);
	if (!ecm_ae_select_test_dentry) {
		pr_info("Failed to create ecm-ae-select-test directory entry\n");
		return -EPERM;
	}

	if (!debugfs_create_file("rule",
			S_IRUGO | S_IWUSR, ecm_ae_select_test_dentry,
			NULL, &ecm_ae_select_test_rule_fops)) {
		pr_info("Failed to create ecm_ae_select_test_rule_fops\n");
		debugfs_remove_recursive(ecm_ae_select_test_dentry);
		return -ENOENT;
	}

	/*
	 * Register the callbacks.
	 */
	ecm_ae_classifier_ops_register(&ae_ops);
	return 0;
}

/*
 * ecm_ae_select_exit()
 *	unregisters the registered callbacks and remove the created directory.
 */
static void __exit ecm_ae_select_exit(void)
{
	pr_info("ECM AE Select EXIT\n");

	/*
	 * Unregister the callbacks.
	 */
	ecm_ae_classifier_ops_unregister();
	debugfs_remove_recursive(ecm_ae_select_test_dentry);
}

module_init(ecm_ae_select_init)
module_exit(ecm_ae_select_exit)

MODULE_DESCRIPTION("ECM Acceleration Engine Selection Module");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
