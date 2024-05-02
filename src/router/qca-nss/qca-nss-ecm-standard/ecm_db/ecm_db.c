/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
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
#include "ecm_notifier_pvt.h"
#include "ecm_interface.h"

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
 * Global listener instance for DB events.
 */
static struct ecm_db_listener_instance *ecm_db_li;

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
 * ecm_db_ipv4_route_table_update_event()
 *	This is a call back for "routing table update event for IPv4".
 */
static int ecm_db_ipv4_route_table_update_event(struct notifier_block *nb,
					       unsigned long event,
					       void *ptr)
{
	DEBUG_TRACE("route table update event v4\n");

	/*
	 * Disable IPv4 frontend processing until defunct function call is completed.
	 */
	ecm_front_end_ipv4_stop(1);

	ecm_db_connection_defunct_ip_version(4);

	/*
	 * Re-enable IPv4 frontend processing.
	 */
	ecm_front_end_ipv4_stop(0);

	return NOTIFY_DONE;
}

static struct notifier_block ecm_db_iproute_table_update_nb = {
	.notifier_call = ecm_db_ipv4_route_table_update_event,
};

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_db_ipv6_route_table_update_event()
 *	This is a call back for "routing table update event for IPv6".
 */
static int ecm_db_ipv6_route_table_update_event(struct notifier_block *nb,
					       unsigned long event,
					       void *ptr)
{
	struct fib6_config *cfg = (struct fib6_config *)ptr;
	struct ecm_db_connection_instance *ci;

	DEBUG_TRACE("route table update event v6\n");

	if ((event != RTM_DELROUTE) && (event != RTM_NEWROUTE)) {
		DEBUG_WARN("%px: Unhandled route table event: %lu\n", cfg, event);
		return NOTIFY_DONE;
	}

	/*
	 * If a default route is changed, fc_dst address is set to all zeros.
	 * In this case, we should defunct all the IPv6 flows.
	 */
	if (ipv6_addr_any(&cfg->fc_dst)) {
		DEBUG_TRACE("%px fc_dst (%pI6), default route is changed, defunct all IPv6 connections\n",
			    cfg, &cfg->fc_dst);
		ecm_db_connection_defunct_ip_version(6);
		return NOTIFY_DONE;
	}

	/*
	 * Disable IPv6 frontend processing until defunct function call is completed.
	 */
	ecm_front_end_ipv6_stop(1);

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;
		struct in6_addr prefix_addr;
		struct in6_addr ecm_in6;
		ip_addr_t ecm_addr;
		struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t if_first;
		struct net_device *ecm_dev;
		struct net_device *fc_dev;
		bool is_dest_ip_match = true;
		ecm_db_obj_dir_t obj_dir = ECM_DB_OBJ_DIR_TO;

		if (ci->ip_version != 6) {
			goto next;
		}

		/*
		 * Get the ECM connection's destination IPv6 address.
		 */
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, ecm_addr);
		ECM_IP_ADDR_TO_NIN6_ADDR(ecm_in6, ecm_addr);

		/*
		 * Compute ECM connection's prefix destination address by masking it with the
		 * route config's destination address prefix length.
		 */
		ipv6_addr_prefix(&prefix_addr, &ecm_in6, cfg->fc_dst_len);

		DEBUG_TRACE("dest addr prefix: %pI6 prefix_len: %d ecm_in6: %pI6\n", &prefix_addr, cfg->fc_dst_len, &ecm_in6);

		/*
		 * Compare the ECM connection's destination address prefix with the route config's
		 * destination address. If they are not equal, try with the ECM's source address prefix.
		 * Because ECM can create the connection in the reply direction, and in this case, ECM
		 * connection's source prefix IP address will match with the route config's dst IP address.
		 *
		 * If none of them match with the route config's destination address, this means that
		 * this connection is not related to this route change event.
		 * We should check with the next connection.
		 */
		if (ipv6_addr_cmp(&prefix_addr, &cfg->fc_dst)) {
			DEBUG_TRACE("dest addr prefix: %pI6 not equal to cfg->fc_dst: %pI6, go to next connection\n", &prefix_addr, &cfg->fc_dst);

			/*
			 * ECM's destination address didn't match.
			 * Get the ECM connection's source IPv6 address.
			 */
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, ecm_addr);
			ECM_IP_ADDR_TO_NIN6_ADDR(ecm_in6, ecm_addr);

			/*
			 * Compute ECM connection's prefix source address by masking it with the
			 * route config's destination address prefix length.
			 */
			ipv6_addr_prefix(&prefix_addr, &ecm_in6, cfg->fc_dst_len);

			DEBUG_TRACE("src addr prefix: %pI6 prefix_len: %d ecm_in6: %pI6\n", &prefix_addr, cfg->fc_dst_len, &ecm_in6);

			if (ipv6_addr_cmp(&prefix_addr, &cfg->fc_dst)) {
				DEBUG_TRACE("src addr prefix: %pI6 not equal to cfg->fc_dst: %pI6, go to next connection\n", &prefix_addr, &cfg->fc_dst);
				goto next;
			}

			is_dest_ip_match = false;
			obj_dir = ECM_DB_OBJ_DIR_FROM;
		}

		DEBUG_TRACE("%px: ECM connection's %s address prefix: %pI6 equals to cfg->fc_dst: %pI6\n",
			    ci, is_dest_ip_match?"dest":"src", &prefix_addr, &cfg->fc_dst);

		/*
		 * If the event is a route delete event, comparing only the IP address is enough
		 * to defunct the connection.
		 */
		if (event == RTM_DELROUTE) {
			DEBUG_TRACE("%px: Route DELETE event, defunct the connection\n", ci);
			ecm_db_connection_make_defunct(ci);
			goto next;
		}

		DEBUG_TRACE("%px: Route ADD event\n", ci);

		/*
		 * If there is a route for this connection's source or destination IP address, we should
		 * compare the devices as well, because the IP address could remain the same, but
		 * the output device could be changed. So, the flows should take the new out device
		 * for their route.
		 */
		if_first = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, obj_dir);
		if (if_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("%px: Failed to get %s interfaces list\n",
					ci, ecm_db_obj_dir_strings[obj_dir]);
			goto next;
		}

		/*
		 * Inner most interface has the IP address, so we should get that interface.
		 */
		ecm_dev = dev_get_by_index(&init_net,
				  ecm_db_iface_interface_identifier_get(interfaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1]));
		if (!ecm_dev) {
			DEBUG_WARN("%px: unable to find ecm netdevice\n", ci);
			ecm_db_connection_interfaces_deref(interfaces, if_first);
			goto next;
		}
		ecm_db_connection_interfaces_deref(interfaces, if_first);

		fc_dev = dev_get_by_index(&init_net, cfg->fc_ifindex);
		if (!fc_dev) {
			DEBUG_WARN("%px: unable to find fib6_config netdevice\n", ci);
			dev_put(ecm_dev);
			goto next;
		}

		/*
		 * Compare the ECM connection's netdevice with the route change config's netdevice.
		 * If they are different, this means the new route effected the connection. So, defunct it.
		 */
		if (ecm_dev != fc_dev) {
			DEBUG_TRACE("%px: fib6_config dev: %s is different from ecm dev: %s, defunct the connection\n",
				    ci, fc_dev->name, ecm_dev->name);
			ecm_db_connection_make_defunct(ci);
		}
		dev_put(fc_dev);
		dev_put(ecm_dev);
next:
		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}

	/*
	 * Re-enable IPv6 frontend processing.
	 */
	ecm_front_end_ipv6_stop(0);
	return NOTIFY_DONE;
}

static struct notifier_block ecm_db_ip6route_table_update_nb = {
	.notifier_call = ecm_db_ipv6_route_table_update_event,
};
#endif

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

	ecm_db_li = ecm_db_listener_alloc();
	if (!ecm_db_li) {
		DEBUG_ERROR("%px: Failed to allocate a listener instance\n", dentry);
		goto init_cleanup_4;
	}
	ecm_db_listener_add(ecm_db_li,
			NULL, /* ecm_notifier_iface_added */
			NULL, /* ecm_notifier_iface_removed */
			NULL, /* ecm_notifier_node_added */
			NULL, /* ecm_notifier_node_removed */
			NULL, /* ecm_notifier_host_added */
			NULL, /* ecm_notifier_host_removed */
			NULL, /* ecm_notifier_mapping_added */
			NULL, /* ecm_notifier_mapping_removed */
			ecm_notifier_connection_added,
			ecm_notifier_connection_removed,
			NULL, /* ecm_notifier_connection_final */
			NULL);

	/*
	 * Initialize the timer resources.
	 */
	ecm_db_timer_init();

	/*
	 * register for route table modification events
	 */
	ip_rt_register_notifier(&ecm_db_iproute_table_update_nb);
#ifdef ECM_IPV6_ENABLE
	rt6_register_notifier(&ecm_db_ip6route_table_update_nb);
#endif
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
#ifdef ECM_IPV6_ENABLE
	rt6_unregister_notifier(&ecm_db_ip6route_table_update_nb);
#endif
	ecm_db_connection_defunct_all();

	/*
	 * Clean-up the timer resources.
	 */
	ecm_db_timer_exit();

	if (ecm_db_li) {
		ecm_db_listener_deref(ecm_db_li);
		ecm_db_li = NULL;
	}

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
