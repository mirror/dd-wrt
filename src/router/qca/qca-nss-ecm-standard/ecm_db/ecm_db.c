/*
 **************************************************************************
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <linux/random.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_DB_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_classifier_default.h"
#include "ecm_db.h"
#include "ecm_front_end_ipv4.h"
#ifdef ECM_IPV6_ENABLE
#include "ecm_front_end_ipv6.h"
#endif

/*
 * Locking of the database - concurrency control
 */
DEFINE_SPINLOCK(ecm_db_lock);					/* Protect the table from SMP access. */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_db_dentry;

/*
 * Management thread control
 */
bool ecm_db_terminate_pending = false;			/* When true the user has requested termination */

/*
 * Random seed used during hash calculations
 */
uint32_t ecm_db_jhash_rnd __read_mostly;

/*
 * ecm_db_obj_dir_strings[]
 *      Common array that maps the object direction to a string
 */
char *ecm_db_obj_dir_strings[ECM_DB_OBJ_DIR_MAX] = {
        "FROM",
        "TO",
        "FROM_NAT",
        "TO_NAT"
};

/*
 * ecm_db_adv_stats_state_write()
 *	Write out advanced stats state
 */
int ecm_db_adv_stats_state_write(struct ecm_state_file_instance *sfi,uint64_t from_data_total, uint64_t to_data_total,
				uint64_t from_packet_total, uint64_t to_packet_total, uint64_t from_data_total_dropped,
				uint64_t to_data_total_dropped, uint64_t from_packet_total_dropped, uint64_t to_packet_total_dropped)
{
	int result;

	if ((result = ecm_state_prefix_add(sfi, "adv_stats"))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "from_data_total", "%llu", from_data_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "to_data_total", "%llu", to_data_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "from_packet_total", "%llu", from_packet_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "to_packet_total", "%llu", to_packet_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "from_data_total_dropped", "%llu", from_data_total_dropped))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "to_data_total_dropped", "%llu", to_data_total_dropped))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "from_packet_total_dropped", "%llu", from_packet_total_dropped))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "to_packet_total_dropped", "%llu", to_packet_total_dropped))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

/*
 * ecm_db_get_defunct_all()
 *	Reading this file returns the accumulated total of all objects
 */
static ssize_t ecm_db_get_defunct_all(struct file *file,
					char __user *user_buf,
					size_t sz, loff_t *ppos)
{
	int ret;
	int num;
	char *buf;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Operate under our locks
	 */
	spin_lock_bh(&ecm_db_lock);
	num = _ecm_db_connection_count_get() + _ecm_db_mapping_count_get() + _ecm_db_host_count_get()
			+ _ecm_db_node_count_get() + _ecm_db_iface_count_get();
	spin_unlock_bh(&ecm_db_lock);

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
	if (ret < 0) {
		kfree(buf);
		return ret;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * ecm_db_set_defunct_all()
 */
static ssize_t ecm_db_set_defunct_all(struct file *file,
					const char __user *user_buf,
					size_t sz, loff_t *ppos)
{
	ecm_db_connection_defunct_all();
	return sz;
}

/*
 * File operations for defunct_all.
 */
static struct file_operations ecm_db_defunct_all_fops = {
	.read = ecm_db_get_defunct_all,
	.write = ecm_db_set_defunct_all,
};

/*
 * ecm_db_route_table_update_event()
 *	This is a call back for "routing table update event for IPv4 and IPv6".
 */
static int ecm_db_route_table_update_event(struct notifier_block *nb,
					       unsigned long event,
					       void *ptr)
{
	DEBUG_TRACE("route table update event\n");

	/*
	 * Disable frontend processing until defunct function call is completed.
	 */
	ecm_front_end_ipv4_stop(1);
#ifdef ECM_IPV6_ENABLE
	ecm_front_end_ipv6_stop(1);
#endif
	ecm_db_connection_defunct_all();

	/*
	 * Re-enable frontend processing.
	 */
	ecm_front_end_ipv4_stop(0);
#ifdef ECM_IPV6_ENABLE
	ecm_front_end_ipv6_stop(0);
#endif
	return NOTIFY_DONE;
}

static struct notifier_block ecm_db_iproute_table_update_nb = {
	.notifier_call = ecm_db_route_table_update_event,
};

static struct notifier_block ecm_db_ip6route_table_update_nb = {
	.notifier_call = ecm_db_route_table_update_event,
};

/*
 * ecm_db_init()
 */
int ecm_db_init(struct dentry *dentry)
{
	DEBUG_INFO("ECM Module init\n");

	ecm_db_dentry = debugfs_create_dir("ecm_db", dentry);
	if (!ecm_db_dentry) {
		DEBUG_ERROR("Failed to create ecm db directory in debugfs\n");
		return -1;
	}

	/*
	 * Get a random seed for jhash()
	 */
	get_random_bytes(&ecm_db_jhash_rnd, sizeof(ecm_db_jhash_rnd));
	printk(KERN_INFO "ECM database jhash random seed: 0x%x\n", ecm_db_jhash_rnd);

	if (!ecm_db_connection_init(ecm_db_dentry)) {
		goto init_cleanup;
	}

	if (!ecm_db_host_init(ecm_db_dentry)) {
		goto init_cleanup_1;
	}

	if (!ecm_db_mapping_init(ecm_db_dentry)) {
		goto init_cleanup_2;
	}

	if (!ecm_db_node_init(ecm_db_dentry)) {
		goto init_cleanup_3;
	}

	if (!ecm_db_iface_init(ecm_db_dentry)) {
		goto init_cleanup_4;
	}

	if (!debugfs_create_file("defunct_all", S_IRUGO | S_IWUSR, ecm_db_dentry,
					NULL, &ecm_db_defunct_all_fops)) {
		DEBUG_ERROR("Failed to create ecm db defunct_all file in debugfs\n");
		goto init_cleanup_4;
	}

	/*
	 * Initialize the timer resources.
	 */
	ecm_db_timer_init();

	/*
	 * register for route table modification events
	 */
	ip_rt_register_notifier(&ecm_db_iproute_table_update_nb);
	rt6_register_notifier(&ecm_db_ip6route_table_update_nb);

	return 0;

init_cleanup_4:
	ecm_db_node_exit();
init_cleanup_3:
	ecm_db_mapping_exit();
init_cleanup_2:
	ecm_db_host_exit();
init_cleanup_1:
	ecm_db_connection_exit();
init_cleanup:
	debugfs_remove_recursive(ecm_db_dentry);
	return -1;
}
EXPORT_SYMBOL(ecm_db_init);

/*
 * ecm_db_exit()
 */
void ecm_db_exit(void)
{
	DEBUG_INFO("ECM DB Module exit\n");

	spin_lock_bh(&ecm_db_lock);
	ecm_db_terminate_pending = true;
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * unregister for route table update events
	 */
	ip_rt_unregister_notifier(&ecm_db_iproute_table_update_nb);
	rt6_unregister_notifier(&ecm_db_ip6route_table_update_nb);

	ecm_db_connection_defunct_all();

	/*
	 * Clean-up the timer resources.
	 */
	ecm_db_timer_exit();

	/*
	 * Free the database.
	 */
	ecm_db_node_exit();
	ecm_db_mapping_exit();
	ecm_db_host_exit();
	ecm_db_connection_exit();

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_db_dentry) {
		debugfs_remove_recursive(ecm_db_dentry);
	}
}
EXPORT_SYMBOL(ecm_db_exit);
