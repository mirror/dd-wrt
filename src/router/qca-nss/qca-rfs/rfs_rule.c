/*
 * Copyright (c) 2014 - 2018, The Linux Foundation. All rights reserved.
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
 */

/*
 * rfs_rule.c
 *	Receiving Flow Streering - Rules
 */

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/jhash.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/proc_fs.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>

#include "rfs.h"
#include "rfs_rule.h"
#include "rfs_ess.h"
#include "rfs_nbr.h"


#define RFS_RULE_HASH_SHIFT 8
#define RFS_RULE_HASH_SIZE (1 << RFS_RULE_HASH_SHIFT)
#define RFS_RULE_HASH_MASK (RFS_RULE_HASH_SIZE - 1)


/*
 * Per-module structure.
 */
struct rfs_rule {
	spinlock_t hash_lock;
	struct hlist_head hash[RFS_RULE_HASH_SIZE];
	struct proc_dir_entry *proc_rule;
};

static struct rfs_rule __rr;


/*
 * rfs_rule_hash
 */
unsigned int rfs_rule_hash(uint32_t type, uint8_t *data)
{
	switch (type) {
	case RFS_RULE_TYPE_MAC_RULE:
		return jhash(data, ETH_ALEN, 0) & RFS_RULE_HASH_MASK;
	case RFS_RULE_TYPE_IP4_RULE:
		return jhash(data, 4, 0) & RFS_RULE_HASH_MASK;
	case RFS_RULE_TYPE_IP6_RULE:
		return jhash(data, sizeof(struct in6_addr), 0) & RFS_RULE_HASH_MASK;
	default:
		return 0;
	}
}


/*
 * rfs_rule_rcu_free
 */
static void rfs_rule_rcu_free(struct rcu_head *head)
{
	struct rfs_rule_entry *re
		= container_of(head, struct rfs_rule_entry, rcu);
	kfree(re);
}


/*
 * rfs_rule_ip_equal
 */
static inline int rfs_rule_ip_equal(int family, uint8_t *ipaddr1, uint8_t *ipaddr2)
{
	if (family ==AF_INET)
		return (*(__be32*)ipaddr1 == *(__be32*)ipaddr2);
	else
		return !memcmp(ipaddr1, ipaddr2, sizeof(struct in6_addr));
}


/*
 * rfs_rule_get_cpu
 *	The caller should hold rcu_read_lock
 */
static uint16_t __rfs_rule_get_cpu(uint32_t type, uint8_t *addr)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint16_t cpu = RPS_NO_CPU;

	head = &rr->hash[rfs_rule_hash(type, addr)];
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (type == RFS_RULE_TYPE_MAC_RULE) {
			if (memcmp(re->mac, addr, ETH_ALEN) == 0) {
				break;
			}
		}
		else if (type == RFS_RULE_TYPE_IP4_RULE) {
			if (re->u.ip4addr ==  *(__be32*)addr) {
				break;
			}
		}
		else if (type == RFS_RULE_TYPE_IP6_RULE) {
			if (!memcmp(&re->u.ip6addr, addr, sizeof(struct in6_addr))) {
				break;
			}
		}
	}

	if (re)
		cpu = re->cpu;

	return cpu;
}


/*
 * rfs_rule_get_cpu_by_ipaddr
 */
uint16_t rfs_rule_get_cpu_by_ipaddr(__be32 ipaddr)
{
	uint16_t cpu;
	uint32_t type = RFS_RULE_TYPE_IP4_RULE;

	rcu_read_lock();
	cpu = __rfs_rule_get_cpu(type, (uint8_t *)&ipaddr);
	rcu_read_unlock();

	return cpu;
}


/*
 * rfs_rule_update_iprule_by_mac
 *	Caller should hold rcu_read_lock
 */
static void __rfs_rule_update_iprule_by_mac(uint8_t *addr, uint16_t cpu)
{
	int index;
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;

	for ( index = 0; index < RFS_RULE_HASH_SIZE; index++) {
		struct hlist_node *n;
		head = &rr->hash[index];
		hlist_for_each_entry_safe(re, n, head, hlist) {
			if (re->type != RFS_RULE_TYPE_IP4_RULE
				&& re->type != RFS_RULE_TYPE_IP6_RULE)
				continue;

			if (re->is_static)
				continue;

			if (memcmp(re->mac, addr, ETH_ALEN))
				continue;

			if (re->cpu == cpu)
				continue;

			rfs_ess_update_ip_rule(re, cpu);
			re->cpu = cpu;

		}
	}
}



/*
 * rfs_rule_create_mac_rule
 */
int rfs_rule_create_mac_rule(uint8_t *addr, uint16_t cpu, struct net_device *to, uint32_t is_static)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint32_t type = RFS_RULE_TYPE_MAC_RULE;
	struct net_device *brdev;
	int brindex = 0;

	rcu_read_lock();
	brdev = netdev_master_upper_dev_get_rcu(to);
	if (brdev) {
		brindex = brdev->ifindex;
	}
	rcu_read_unlock();

	head = &rr->hash[rfs_rule_hash(type, addr)];

	spin_lock_bh(&rr->hash_lock);
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (memcmp(re->mac, addr, ETH_ALEN) == 0 && re->brindex == brindex) {
			break;
		}
	}

	if (re) {
		/*
		 * don't overwrite any existing rule
		 */
		if (!is_static) {
			spin_unlock_bh(&rr->hash_lock);
			return 0;
		}
	}

	/*
	 * Create a rule entry if it doesn't exist
	 */
	if (!re ) {
		re = kzalloc(sizeof(struct rfs_rule_entry), GFP_ATOMIC);
		if (!re ) {
			spin_unlock_bh(&rr->hash_lock);
			return -1;
		}

		memcpy(re->mac, addr, ETH_ALEN);
		re->type = type;
		re->cpu = RPS_NO_CPU;
		re->to = to;
		re->brindex = brindex;
		hlist_add_head_rcu(&re->hlist, head);
		RFS_DEBUG("New MAC rule %pM, cpu %d to:%s\n", addr, cpu, to->name);
	}


	re->is_static = is_static;
	re->to = to;
	re->brindex = brindex;
	if (re->cpu != cpu ) {
		rfs_ess_update_mac_rule(re, cpu);
	}
	re->cpu = cpu;

	RFS_DEBUG("update Mac: %pM, cpu %d to:%s\n", addr, re->cpu, re->to->name);
	__rfs_rule_update_iprule_by_mac(addr, cpu);
	spin_unlock_bh(&rr->hash_lock);
	return 0;
}


/*
 * rfs_rule_destroy_mac_rule
 */
int rfs_rule_destroy_mac_rule(uint8_t *addr, struct net_device *to, uint32_t is_static)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint16_t cpu;
	uint32_t type = RFS_RULE_TYPE_MAC_RULE;
	struct net_device *brdev;
	int brindex = 0;

	head = &rr->hash[rfs_rule_hash(type, addr)];
	rcu_read_lock();
	brdev = netdev_master_upper_dev_get_rcu(to);
	if (brdev) {
		brindex = brdev->ifindex;
	}
	rcu_read_unlock();

	spin_lock_bh(&rr->hash_lock);
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (memcmp(re->mac, addr, ETH_ALEN) == 0 && re->brindex == brindex) {
			break;
		}
	}

	if (!re || (re->is_static && !is_static)) {
		spin_unlock_bh(&rr->hash_lock);
		return 0;
	}

	hlist_del_rcu(&re->hlist);
	cpu = re->cpu;

	RFS_DEBUG("Remove rules: %pM, cpu %d\n", addr, cpu);
	rfs_ess_update_mac_rule(re, RPS_NO_CPU);

	re->cpu = RPS_NO_CPU;
	call_rcu(&re->rcu, rfs_rule_rcu_free);

	__rfs_rule_update_iprule_by_mac(addr, RPS_NO_CPU);
	spin_unlock_bh(&rr->hash_lock);

	return 0;
}


/*
 * rfs_rule_find_mac_rule
 */
int rfs_rule_find_mac_rule(uint8_t *addr, struct net_device *to, uint32_t is_static)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint32_t type = RFS_RULE_TYPE_MAC_RULE;
	struct net_device *brdev;
	int brindex = 0;

	rcu_read_lock();
	brdev = netdev_master_upper_dev_get_rcu(to);
	if (brdev) {
		brindex = brdev->ifindex;
	}
	rcu_read_unlock();

	head = &rr->hash[rfs_rule_hash(type, addr)];

	spin_lock_bh(&rr->hash_lock);
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (memcmp(re->mac, addr, ETH_ALEN) == 0 && re->brindex == brindex) {
			break;
		}
	}

	if (!re || (re->is_static && !is_static)) {
		spin_unlock_bh(&rr->hash_lock);
		return 0;
	}

	spin_unlock_bh(&rr->hash_lock);
	return 1;
}


/*
 * rfs_rule_create_ip_rule
 */
int rfs_rule_create_ip_rule(int family, uint8_t *ipaddr, uint8_t *maddr,
			    uint16_t cpu, uint32_t is_static)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint32_t type;


	if (family == AF_INET)
		type = RFS_RULE_TYPE_IP4_RULE;
	else
		type = RFS_RULE_TYPE_IP6_RULE;

	head = &rr->hash[rfs_rule_hash(type, ipaddr)];

	spin_lock_bh(&rr->hash_lock);
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (rfs_rule_ip_equal(family, (uint8_t *)&re->u.ip4addr, ipaddr))
			break;
	}

	if (re) {
		/*
		 * don't overwrite any existing rule
		 */
		if (!is_static) {
			spin_unlock_bh(&rr->hash_lock);
			return 0;
		}
	}

	/*
	 * Create a rule entry if it doesn't exist
	 */
	if (!re) {
		re = kzalloc(sizeof(struct rfs_rule_entry), GFP_ATOMIC);
		if (!re ) {
			spin_unlock_bh(&rr->hash_lock);
			return -1;
		}

		if (family ==AF_INET)
			re->u.ip4addr =  *(__be32*)ipaddr;
		else
			memcpy(&re->u.ip6addr, ipaddr, sizeof(struct in6_addr));
		re->type = type;
		re->cpu  = RPS_NO_CPU;
		memcpy(re->mac, maddr, ETH_ALEN);
		hlist_add_head_rcu(&re->hlist, head);
	}

	/*
	 * Look up MAC rules
	 */
	if (cpu == RPS_NO_CPU)
		cpu = __rfs_rule_get_cpu(RFS_RULE_TYPE_MAC_RULE, maddr);


	if (family ==AF_INET)
		RFS_DEBUG("New IP rule %pI4, cpu %d\n", ipaddr, cpu);
	else
		RFS_DEBUG("New IP rule %pI6, cpu %d\n", ipaddr, cpu);

	if (re->cpu != cpu) {
		rfs_ess_update_ip_rule(re, cpu);
	}

	re->is_static = is_static;
	re->cpu = cpu;

	spin_unlock_bh(&rr->hash_lock);


	return 0;
}


/*
 * rfs_rule_destroy_ip_rule
 */
int rfs_rule_destroy_ip_rule(int family, uint8_t *ipaddr, uint32_t is_static)
{
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;
	uint32_t type = RFS_RULE_TYPE_IP4_RULE;
	uint16_t cpu;

	head = &rr->hash[rfs_rule_hash(type, ipaddr)];

	spin_lock_bh(&rr->hash_lock);
	hlist_for_each_entry_rcu(re, head, hlist) {
		if (type != re->type)
			continue;

		if (rfs_rule_ip_equal(family, (uint8_t *)&re->u.ip4addr, ipaddr))
			break;
	}

	if (!re || (re->is_static && !is_static)) {
		spin_unlock_bh(&rr->hash_lock);
		return 0;
	}

	hlist_del_rcu(&re->hlist);
	cpu = re->cpu;

	if (family ==AF_INET)
		RFS_DEBUG("Remove IP rule %pI4, cpu %d\n", ipaddr, cpu);
	else
		RFS_DEBUG("Remove IP rule %pI6, cpu %d\n", ipaddr, cpu);

	if (cpu != RPS_NO_CPU) {
		rfs_ess_update_ip_rule(re, RPS_NO_CPU);
	}

	re->cpu = RPS_NO_CPU;
	call_rcu(&re->rcu, rfs_rule_rcu_free);
	spin_unlock_bh(&rr->hash_lock);

	return 0;
}


/*
 * rfs_rule_reset_all
 *	This function will be called when VLAN changed
 */
void rfs_rule_reset_all(void)
{
	int index;
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;

	spin_lock_bh(&rr->hash_lock);
	for ( index = 0; index < RFS_RULE_HASH_SIZE; index++) {
		struct hlist_node *n;
		head = &rr->hash[index];
		hlist_for_each_entry_safe(re, n, head, hlist) {
			if (re->cpu == RPS_NO_CPU)
				continue;
			if (re->type == RFS_RULE_TYPE_MAC_RULE)
				rfs_ess_update_mac_rule(re, RPS_NO_CPU);
			else
				rfs_ess_update_ip_rule(re, RPS_NO_CPU);
			re->cpu = RPS_NO_CPU;

		}
	}
	spin_unlock_bh(&rr->hash_lock);
}



/*
 * rfs_rule_destroy_all
 *	Clear the rules and free the rules' entry
 */
void rfs_rule_destroy_all(void)
{
	int index;
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;

	spin_lock_bh(&rr->hash_lock);
	for ( index = 0; index < RFS_RULE_HASH_SIZE; index++) {
		struct hlist_node *n;
		head = &rr->hash[index];
		hlist_for_each_entry_safe(re, n, head, hlist) {
			if (re->cpu != RPS_NO_CPU) {
				if (re->type == RFS_RULE_TYPE_MAC_RULE)
					rfs_ess_update_mac_rule(re, RPS_NO_CPU);
				else
					rfs_ess_update_ip_rule(re, RPS_NO_CPU);
			}
			hlist_del_rcu(&re->hlist);
			re->cpu = RPS_NO_CPU;
			call_rcu(&re->rcu, rfs_rule_rcu_free);
		}
	}
	spin_unlock_bh(&rr->hash_lock);
}


/*
 * rfs_rule_proc_show
 *	show RFS rules in proc
 */
static int rfs_rule_proc_show(struct seq_file *m, void *v)
{
	int index;
	int count = 0;
	struct hlist_head *head;
	struct rfs_rule_entry *re;
	struct rfs_rule *rr = &__rr;

	seq_printf(m, "RFS rule table:\n");

	rcu_read_lock();
	for ( index = 0; index < RFS_RULE_HASH_SIZE; index++) {
		head = &rr->hash[index];
		hlist_for_each_entry_rcu(re, head, hlist) {
			seq_printf(m, "%03d %04x", ++count, index);
			if (re->type == RFS_RULE_TYPE_MAC_RULE)
				seq_printf(m, " MAC: %pM cpu %d", re->mac, re->cpu);
			else if (re->type == RFS_RULE_TYPE_IP4_RULE)
				seq_printf(m, "  IP: %pI4 cpu %d", &re->u.ip4addr, re->cpu);
			else
				seq_printf(m, "  IP: %pI6 cpu %d", &re->u.ip4addr, re->cpu);

			if (re->is_static)
				seq_printf(m, " static");
			else
				seq_printf(m, " dynamic");

			if (re->type == RFS_RULE_TYPE_MAC_RULE && re->cpu != RPS_NO_CPU)
				seq_printf(m, " to %s", re->to->name);

			if (re->nvif) {
				int i;
				seq_printf(m, " ifvid");
				for (i = 0; i < re->nvif; i++) {
					seq_printf(m, " %d", re->ifvid[i]);
					if (re->type != RFS_RULE_TYPE_MAC_RULE) {
						seq_printf(m, "@%pM", &re->ifmac[ETH_ALEN * i]);
					}
				}
			}

			seq_putc(m, '\n');

		}
	}
	seq_putc(m, '\n');
	rcu_read_unlock();
	return 0;
}


/*
 * rfs_rule_proc_write
 *	get user configuration from proc
 */
static ssize_t rfs_rule_proc_write(struct file *file, const char __user *buffer,
			size_t count, loff_t *ppos)
{
	char buf[64];
	unsigned int addr[6];
	int nvar;
	int cpu;
	char devname[IFNAMSIZ];
	struct net_device *dev;

	count = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;
	buf[count] = '\0';

	nvar = sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x %d %s",
			&addr[0], &addr[1], &addr[2], &addr[3],
			&addr[4], &addr[5], &cpu, devname);

	if (nvar >= 7) {
		uint8_t mac[6];
		mac[0] = addr[0];
		mac[1] = addr[1];
		mac[2] = addr[2];
		mac[3] = addr[3];
		mac[4] = addr[4];
		mac[5] = addr[5];
		dev =  dev_get_by_name(&init_net, devname);
		if(!dev) {
			RFS_DEBUG("Not found device %s\n", devname);
			return 0;
		}
		if ((uint16_t)cpu != RPS_NO_CPU)
			rfs_rule_create_mac_rule(mac, (uint16_t)cpu, dev, 1);
		else
			rfs_rule_destroy_mac_rule(mac, dev, 1);
		dev_put(dev);
		return count;
	}

	nvar = sscanf(buf, "%u.%u.%u.%u %d", &addr[0], &addr[1],
			&addr[2], &addr[3], &cpu);
	if (nvar == 5) {
		uint8_t  ip[4];
		uint8_t  mac[6] = {0};
		ip[0] = addr[0];
		ip[1] = addr[1];
		ip[2] = addr[2];
		ip[3] = addr[3];
		if ((uint16_t)cpu != RPS_NO_CPU)
			rfs_rule_create_ip_rule(AF_INET, ip, mac, (uint16_t)cpu, 1);
		else
			rfs_rule_destroy_ip_rule(AF_INET, ip, 1);
		return count;
	}

	return -EFAULT;

}


/*
 * rfs_rule_proc_open
 */
static int rfs_rule_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rfs_rule_proc_show, NULL);
}


/*
 * struct file_operations rule_proc_fops
 */
static const struct file_operations rule_proc_fops = {
	.owner = THIS_MODULE,
	.open  = rfs_rule_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.write  = rfs_rule_proc_write,
	.release = single_release,
};


/*
 * rfs_rule_init()
 */
int rfs_rule_init(void)
{
	struct rfs_rule *rr = &__rr;

	RFS_DEBUG("RFS Rule init\n");
	spin_lock_init(&rr->hash_lock);
	memset(&rr->hash, 0, RFS_RULE_HASH_SIZE);

	rr->proc_rule = proc_create("rule", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
				    rfs_proc_entry, &rule_proc_fops);
	return 0;
}

/*
 * rfs_rule_exit()
 */
void rfs_rule_exit(void)
{
	struct rfs_rule *rr = &__rr;

	RFS_DEBUG("RFS Rule exit\n");
	if (rr->proc_rule);
		remove_proc_entry("rule", rfs_proc_entry);
	rfs_rule_destroy_all();
}


