/*
 **************************************************************************
 * Copyright (c) 2015-2018, The Linux Foundation.  All rights reserved.
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
#include <linux/of.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <asm/unaligned.h>
#include <linux/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/etherdevice.h>

#include "ecm_classifier_pcc_public.h"

#define MAC_FMT "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx"

#define RULE_FIELDS 10

/*
 * DebugFS entry object.
 */
static struct dentry *ecm_pcc_test_dentry;

/*
 * Registration
 */
struct ecm_classifier_pcc_registrant *ecm_pcc_test_registrant = NULL;

/*
 * Rule table
 */
struct ecm_pcc_test_rule {
	struct list_head list;
	char name[50];
	ecm_classifier_pcc_result_t accel;
	unsigned int proto;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dest_mac[ETH_ALEN];
	int src_port;
	int dest_port;
	struct in6_addr src_addr;
	struct in6_addr dest_addr;
	unsigned int ipv;
};
LIST_HEAD(ecm_pcc_test_rules);
DEFINE_SPINLOCK(ecm_pcc_test_rules_lock);

/*
 * ecm_pcc_test_registrant_ref()
 *	Invoked when an additional hold is kept upon the registrant
 */
static void ecm_pcc_test_registrant_ref(struct ecm_classifier_pcc_registrant *r)
{
	int remain;

	/*
	 * Increment the ref count by 1.
	 * This causes the registrant structure to remain in existance until
	 * released (deref).
	 * By definition the caller of this method has a hold on the registrant
	 * already so it cannot 'go away'.
	 * This is because either:
	 * 1. The caller itself has been passed it in a function parameter;
	 * 2. It has its own explicit hold.
	 */
	remain = atomic_inc_return(&r->ref_count);
	if (remain <= 0)
		pr_info("REFERENCE COUNT WRAP!\n");
	else
		pr_info("ECM PCC Registrant ref: %d\n", remain);
}

/*
 * ecm_pcc_test_registrant_deref()
 *	Caller is releasing its hold upon the registrant.
 */
static void
ecm_pcc_test_registrant_deref(struct ecm_classifier_pcc_registrant *r)
{
	int remain;
	struct ecm_pcc_test_rule *tmp = NULL;
	struct list_head *pos, *q;

	/*
	 * Decrement the reference count
	 */
	remain = atomic_dec_return(&r->ref_count);
	if (remain > 0) {
		/*
		 * Something still holds a reference
		 */
		pr_info("ECM PCC Registrant deref: %d\n", remain);
		return;
	}

	/*
	 * Last hold upon the registrant is released and so we can now
	 * destroy it.
	 */
	if (remain < 0)
		pr_info("REFERENCE COUNT WRAP!\n");

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	list_for_each_safe(pos, q , &ecm_pcc_test_rules) {
		tmp = list_entry(pos, struct ecm_pcc_test_rule, list);
		list_del(pos);
		kfree(tmp);
	}
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	pr_info("ECM PCC Registrant DESTROYED\n");
	kfree(r);
}

/*
 * __ecm_pcc_test_rule_find()
 *	Return matching rule
 */
static struct
ecm_pcc_test_rule *__ecm_pcc_test_rule_find(unsigned int proto,
					    uint8_t *src_mac,
					    uint8_t *dest_mac,
					    struct in6_addr *src_addr,
					    struct in6_addr *dest_addr,
					    int src_port, int dest_port)
{
	struct ecm_pcc_test_rule *rule = NULL;

	list_for_each_entry(rule , &ecm_pcc_test_rules, list) {
		if (rule->proto != proto)
			continue;

		if (rule->src_port != src_port)
			goto try_reverse;
		if (rule->dest_port != dest_port)
			goto try_reverse;
		if (!ipv6_addr_equal(&rule->src_addr, src_addr))
			goto try_reverse;
		if (!ipv6_addr_equal(&rule->dest_addr, dest_addr))
			goto try_reverse;

		return rule;

try_reverse:

		if (rule->src_port != dest_port)
			continue;
		if (rule->dest_port != src_port)
			continue;
		if (!ipv6_addr_equal(&rule->src_addr, dest_addr))
			continue;
		if (!ipv6_addr_equal(&rule->dest_addr, src_addr))
			continue;

		return rule;
	}
	return NULL;
}


/*
 * ecm_pcc_test_okay_to_accel_v4()
 *	Invoked by the ECM to query if the given connection may be accelerated
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_okay_to_accel_v4(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, __be32 src_ip, int src_port,
			      uint8_t *dest_mac, __be32 dest_ip, int dest_port,
			      int protocol)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;
	struct in6_addr src_addr = IN6ADDR_ANY_INIT;
	struct in6_addr dest_addr = IN6ADDR_ANY_INIT;

	src_addr.s6_addr32[0] = src_ip;
	dest_addr.s6_addr32[0] = dest_ip;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			&src_addr, &dest_addr,
			src_port, dest_port);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	return accel;
}

/*
 * ecm_pcc_test_okay_to_accel_v6()
 *	Invoked by the ECM to query if the given connection may be accelerated
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_okay_to_accel_v6(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, struct in6_addr *src_addr,
			      int src_port, uint8_t *dest_mac,
			      struct in6_addr *dest_addr, int dest_port,
			      int protocol)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			src_addr, dest_addr,
			src_port, dest_port);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
	return accel;
}

/*
 * ecm_pcc_test_unregister_get_unregister()
 */
static int ecm_pcc_test_unregister_get_unregister(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

/*
 * ecm_pcc_test_unregister_set_unregister()
 */
static int ecm_pcc_test_unregister_set_unregister(void *data, u64 val)
{
	if (ecm_pcc_test_registrant) {
		pr_info("ECM PCC Test unregister\n");
		ecm_classifier_pcc_unregister_begin(ecm_pcc_test_registrant);
		ecm_pcc_test_registrant = NULL;
	}
	return 0;
}

/*
 * ecm_pcc_test_str_to_ip()
 *	Convert string IP to in6_addr.  Return 4 for IPv4 or 6 for IPv6.
	Return something else in error.
 *
 * NOTE: When string is IPv4 the lower 32 bit word of the in6 address
 * contains the address. Network order.
 */
static unsigned int ecm_pcc_test_str_to_ip(char *ip_str, struct in6_addr *addr)
{
	uint8_t *ptr = (uint8_t *)(addr->s6_addr);

	/*
	 * IPv4 address in addr->s6_addr32[0]
	 */
	if (in4_pton(ip_str, -1, ptr, '\0', NULL) > 0)
		return 4;

	/*
	 * IPv6
	 */
	if (in6_pton(ip_str, -1, ptr, '\0', NULL) > 0)
		return 6;

	return 0;
}

/*
 * ecm_pcc_test_deny_accel()
 *	Deny acceleration
 */
static void ecm_pcc_test_deny_accel(unsigned int proto,
		uint8_t *src_mac, uint8_t *dest_mac,
		struct in6_addr *src_addr, struct in6_addr *dest_addr,
		int src_port, int dest_port,
		unsigned int ipv)
{
	if (ipv == 4)
		ecm_classifier_pcc_deny_accel_v4(src_mac,
						src_addr->s6_addr32[0],
						src_port, dest_mac,
						dest_addr->s6_addr32[0],
						dest_port, proto);
	else
		ecm_classifier_pcc_deny_accel_v6(src_mac, src_addr, src_port,
						dest_mac, dest_addr, dest_port,
						proto);
}

/*
 * ecm_pcc_test_permit_accel()
 *	Permit acceleration
 */
static void ecm_pcc_test_permit_accel(unsigned int proto,
		uint8_t *src_mac, uint8_t *dest_mac,
		struct in6_addr *src_addr, struct in6_addr *dest_addr,
		int src_port, int dest_port,
		unsigned int ipv)
{
	if (ipv == 4)
		ecm_classifier_pcc_permit_accel_v4(src_mac,
						src_addr->s6_addr32[0],
						src_port, dest_mac,
						dest_addr->s6_addr32[0],
						dest_port, proto);
	else
		ecm_classifier_pcc_permit_accel_v6(src_mac, src_addr, src_port,
						dest_mac, dest_addr, dest_port,
						proto);

}

/*
 * ecm_pcc_test_update_rule()
 *	Update a rule, return 0 on failure. If change in accel this will
 *	inform ECM.
 */
static unsigned int ecm_pcc_test_update_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port)
{
	unsigned int ipv;
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t oaccel;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return 0;
	}
	ipv = rule->ipv;
	oaccel = rule->accel;
	rule->accel = accel;
	strcpy(rule->name, name);
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	if (oaccel == accel)
		return 1;

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_deny_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);
	else
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_delete_rule()
 *	Delete a rule, return 0 on failure.  If deleted connection rule denied
 *	accel this will permit accel.
 */
static unsigned int ecm_pcc_test_delete_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port)
{
	unsigned int ipv;
	struct ecm_pcc_test_rule *rule;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return 0;
	}
	ipv = rule->ipv;
	accel = rule->accel;
	list_del(&rule->list);
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
	kfree(rule);

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_add_rule()
 *	Add a new rule, return 0 on failure.  Given accel the ecm is informed
 *	of permit/deny accel status.
 */
static unsigned int ecm_pcc_test_add_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port,
				unsigned int ipv)
{
	struct ecm_pcc_test_rule *new_rule;
	new_rule = kzalloc(sizeof(struct ecm_pcc_test_rule), GFP_ATOMIC);
	if (!new_rule)
		return 0;

	strcpy(new_rule->name, name);
	new_rule->accel = accel;
	new_rule->proto = proto;
	new_rule->src_port = src_port;
	new_rule->dest_port = dest_port;
	ether_addr_copy(new_rule->src_mac, src_mac);
	ether_addr_copy(new_rule->dest_mac, dest_mac);
	new_rule->src_addr = *src_addr;
	new_rule->dest_addr = *dest_addr;
	new_rule->ipv = ipv;
	INIT_LIST_HEAD(&new_rule->list);

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	list_add(&new_rule->list, &ecm_pcc_test_rules);
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_deny_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);
	else
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_rule_write()
 *	Write a rule
 */
static ssize_t ecm_pcc_test_rule_write(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char *rule_buf;
	int field_count;
	char *field_ptr;
	char *fields[RULE_FIELDS];
	char name[50];
	uint8_t src_mac[ETH_ALEN];
	uint8_t dest_mac[ETH_ALEN];
	unsigned int oper;
	ecm_classifier_pcc_result_t accel;
	unsigned int proto;
	int src_port;
	int dest_port;
	struct in6_addr src_addr = IN6ADDR_ANY_INIT;
	struct in6_addr dest_addr = IN6ADDR_ANY_INIT;
	unsigned int ipv;

	/*
	 * buf is formed as:
	 * [0]    [1]                 [2]                           [3]     [4]       [5]        [6]        [7]        [8]         [9]
	 * <name>/<0=del,1=add,2=upd>/<1=denied, 2=accel_permitted>/<proto>/<src_mac>/<src_addr>/<src_port>/<dest mac>/<dest_addr>/<dest_port>/
	 * e.g.:
	 * echo "my_rule/1/2/6/00:12:12:34:56:78/192.168.1.33/1234/00:12:12:34:56:22/10.10.10.10/80" > /sys/kernel/debug/ecm_pcc_test/rule
	 * cat /sys/kernel/debug/ecm_pcc_test/rule (shows all rules)
	 */
	rule_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!rule_buf)
		return -EINVAL;

	memcpy(rule_buf, user_buf, count);

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

		fields[field_count] = strsep(&field_ptr, "/");
	}

	if (field_count != RULE_FIELDS) {
		pr_info("Invalid field count %d\n", field_count);
		kfree(rule_buf);
		return -EINVAL;
	}

	/*
	 * Convert fields
	 */
	strncpy(name, fields[0], sizeof(name));
	name[sizeof(name) - 1] = 0;
	if (sscanf(fields[1], "%u", &oper) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[2], "%d", (int *)&accel) != 1)
		goto sscanf_read_error;

	switch (accel) {
	case ECM_CLASSIFIER_PCC_RESULT_DENIED:
	case ECM_CLASSIFIER_PCC_RESULT_PERMITTED:
		break;
	default:
		pr_info("Bad accel: %u\n", accel);
		kfree(rule_buf);
		return -EINVAL;
	}

	if (sscanf(fields[3], "%u", &proto) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[6], "%d", &src_port) != 1)
		goto sscanf_read_error;

	src_port = htons(src_port);

	if (sscanf(fields[9], "%d", &dest_port) != 1)
		goto sscanf_read_error;

	dest_port = htons(dest_port);

	if (sscanf(fields[4], MAC_FMT, src_mac, src_mac + 1, src_mac + 2,
		src_mac + 3, src_mac + 4, src_mac + 5) != 6)
		goto sscanf_read_error;

	if (sscanf(fields[7], MAC_FMT, dest_mac, dest_mac + 1, dest_mac + 2,
			dest_mac + 3, dest_mac + 4, dest_mac + 5) != 6)
		goto sscanf_read_error;

	ipv = ecm_pcc_test_str_to_ip(fields[5], &src_addr);
	if (ipv != ecm_pcc_test_str_to_ip(fields[8], &dest_addr)) {
		pr_info("Conflicting IP address types\n");
		kfree(rule_buf);
		return -EINVAL;
	}
	kfree(rule_buf);

	pr_info("name: %s\n"
			"oper: %u\n"
			"accel: %d\n"
			"proto: %u\n"
			"%pM\n"
			"%pI6:%d\n"
			"%pM\n"
			"%pI6:%d\n"
			"ipv: %u\n",
			name,
			oper,
			(int)accel,
			proto,
			src_mac,
			&src_addr,
			ntohs(src_port),
			dest_mac,
			&dest_addr,
			ntohs(dest_port),
			ipv
			);

	if (oper == 0) {
		pr_info("Delete\n");
		if (!ecm_pcc_test_delete_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port))
			return -EINVAL;

	} else if (oper == 1) {
		pr_info("Add\n");
		if (!ecm_pcc_test_add_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port, ipv))
			return -EINVAL;

	} else if (oper == 2) {
		pr_info("Update\n");
		if (!ecm_pcc_test_update_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port))
			return -EINVAL;

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
 * ecm_pcc_test_rule_seq_show()
 */
static int ecm_pcc_test_rule_seq_show(struct seq_file *m, void *v)
{
	struct ecm_pcc_test_rule *rule;

	rule = list_entry(v, struct ecm_pcc_test_rule, list);

	seq_printf(m, "RULE:\n"
			"\tname: %s\n"
			"\taccel: %d\n"
			"\tproto: %u\n"
			"\t"
			"%pM\n"
			"\t%pI6:%u\n"
			"\t"
			"%pM\n"
			"\t%pI6:%d\n"
			"\tipv: %u\n",
			rule->name,
			(int)(rule->accel),
			rule->proto,
			rule->src_mac,
			&rule->src_addr,
			ntohs(rule->src_port),
			rule->dest_mac,
			&rule->dest_addr,
			ntohs(rule->dest_port),
			rule->ipv
			);

	return 0;
}

/*
 * ecm_pcc_test_rule_seq_stop()
 */
static void ecm_pcc_test_rule_seq_stop(struct seq_file *p, void *v)
{
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
}

/*
 * ecm_pcc_test_rule_seq_start()
 */
static void *ecm_pcc_test_rule_seq_start(struct seq_file *m, loff_t *_pos)
{
	spin_lock_bh(&ecm_pcc_test_rules_lock);
	return seq_list_start(&ecm_pcc_test_rules, *_pos);
}

/*
 * ecm_pcc_test_rule_seq_next()
 */
static void *ecm_pcc_test_rule_seq_next(struct seq_file *p, void *v,
					loff_t *pos)
{
	return seq_list_next(v, &ecm_pcc_test_rules, pos);
}

/*
 * DebugFS attributes.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_pcc_test_unregister_fops,
			ecm_pcc_test_unregister_get_unregister,
			ecm_pcc_test_unregister_set_unregister, "%llu\n");

static const struct seq_operations ecm_pcc_test_rule_seq_ops = {
	.start = ecm_pcc_test_rule_seq_start,
	.next  = ecm_pcc_test_rule_seq_next,
	.stop  = ecm_pcc_test_rule_seq_stop,
	.show  = ecm_pcc_test_rule_seq_show,
};

/*
 * ecm_pcc_test_rule_open()
 */
static int ecm_pcc_test_rule_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ecm_pcc_test_rule_seq_ops);
}

/*
 * ecm_pcc_test_rule_release()
 */
static int ecm_pcc_test_rule_release(struct inode *inode, struct file *file)
{
	return seq_release(inode, file);
}

static const struct file_operations ecm_pcc_test_rule_fops = {
	.owner		= THIS_MODULE,
	.open		= ecm_pcc_test_rule_open,
	.write		= ecm_pcc_test_rule_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= ecm_pcc_test_rule_release,
};

/*
 * ecm_pcc_test_init()
 */
static int __init ecm_pcc_test_init(void)
{
	int result;

	pr_info("ECM PCC Test INIT\n");

	/*
	 * Create entries in DebugFS for control functions
	 */
	ecm_pcc_test_dentry = debugfs_create_dir("ecm_pcc_test", NULL);
	if (!ecm_pcc_test_dentry) {
		pr_info("Failed to create PCC directory entry\n");
		return -1;
	}
	if (!debugfs_create_file("unregister",
			S_IRUGO | S_IWUSR, ecm_pcc_test_dentry,
			NULL, &ecm_pcc_test_unregister_fops)) {
		pr_info("Failed to create ecm_pcc_test_unregister_fops\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -2;
	}
	if (!debugfs_create_file("rule",
			S_IRUGO | S_IWUSR, ecm_pcc_test_dentry,
			NULL, &ecm_pcc_test_rule_fops)) {
		pr_info("Failed to create ecm_pcc_test_rule_fops\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -3;
	}

	/*
	 * Create our registrant structure
	 */
	ecm_pcc_test_registrant = (struct ecm_classifier_pcc_registrant *)
			kzalloc(sizeof(struct ecm_classifier_pcc_registrant),
				GFP_ATOMIC | __GFP_NOWARN);
	if (!ecm_pcc_test_registrant) {
		pr_info("ECM PCC Failed to alloc registrant\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -4;
	}
	ecm_pcc_test_registrant->version = 1;
	ecm_pcc_test_registrant->this_module = THIS_MODULE;
	ecm_pcc_test_registrant->ref = ecm_pcc_test_registrant_ref;
	ecm_pcc_test_registrant->deref = ecm_pcc_test_registrant_deref;
	ecm_pcc_test_registrant->okay_to_accel_v4 =
						ecm_pcc_test_okay_to_accel_v4;
	ecm_pcc_test_registrant->okay_to_accel_v6 =
						ecm_pcc_test_okay_to_accel_v6;

	/*
	 * Register with the PCC Classifier. ECM classifier will take a ref for
	 * registrant.
	 */
	result = ecm_classifier_pcc_register(ecm_pcc_test_registrant);
	if (result != 0) {
		pr_info("ECM PCC registrant failed to register: %d\n", result);
		kfree(ecm_pcc_test_registrant);
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -5;
	}

	pr_info("ECM PCC registrant REGISTERED\n");

	return 0;
}

/*
 * ecm_pcc_test_exit()
 */
static void __exit ecm_pcc_test_exit(void)
{
	pr_info("ECM PCC Test EXIT\n");
	debugfs_remove_recursive(ecm_pcc_test_dentry);
}

module_init(ecm_pcc_test_init)
module_exit(ecm_pcc_test_exit)

MODULE_DESCRIPTION("ECM PCC Test");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

