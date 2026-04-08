/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/

/*
 * External interface for the Cavium Octeon ethernet driver.
 */
#ifndef OCTEON_ETHERNET_H
#define OCTEON_ETHERNET_H

#include <linux/of.h>

#include "octeon_common.h"

#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-hwfau.h>
#include <asm/octeon/octeon-ethernet-user.h>

/**
 * This is the definition of the Ethernet driver's private
 * driver state stored in netdev_priv(dev).
 */
struct octeon_ethernet {
	struct rb_node ipd_tree;
	int key;
	int ipd_port;
	int pko_port;
	int ipd_pkind;
	int interface;
	int interface_port;

	/* My netdev. */
	struct net_device *netdev;
	/* My location in the cvm_oct_list */
	struct list_head list;

	/* Type of port. This is one of the enums in
	 * cvmx_helper_interface_mode_t
	 */
	int imode;

	unsigned int rx_strip_fcs:1;
	unsigned int tx_timestamp_hw:1;
	unsigned int rx_timestamp_hw:1;
	unsigned int tx_multiple_queues:1;
	unsigned int tx_lockless:1;

	/* Base address for accessing GMX registers */
	u64 gmx_base;

	/* Optional intecept callback defined above */
	cvm_oct_callback_t      intercept_cb;

	/* Number of elements in tx_queue below */
	int                     num_tx_queues;

	/* SRIO ports add this header for the SRIO block */
	u64 srio_tx_header;

	struct {
		/* PKO hardware queue for the port */
		int	queue;
		/* Hardware fetch and add to count outstanding tx buffers */
		int	fau;
	} tx_queue[32];

	struct phy_device *phydev;
	int last_link;
	/* Last negotiated link state */
	u64 link_info;
	/* Called periodically to check link status */
	spinlock_t poll_lock;
	void (*poll) (struct net_device *dev);
	void (*link_change) (struct octeon_ethernet *, cvmx_helper_link_info_t);
	struct delayed_work	port_periodic_work;
	struct work_struct	port_work;	/* may be unused. */
	struct device_node	*of_node;
	u64 last_tx_octets;
	u32 last_tx_packets;
	struct list_head srio_bcast;
	struct notifier_block	hw_status_notifier;
};

struct octeon_ethernet_srio_bcast_target {
	struct list_head list;
	u16 destid;
};

struct octeon_ethernet *cvm_oct_dev_for_port(int);

int cvm_oct_free_work(void *work_queue_entry);

int cvm_oct_rgmii_init(struct net_device *dev);
int cvm_oct_rgmii_open(struct net_device *dev);
int cvm_oct_rgmii_stop(struct net_device *dev);

int cvm_oct_sgmii_init(struct net_device *dev);
void cvm_oct_sgmii_uninit(struct net_device *dev);
int cvm_oct_sgmii_open(struct net_device *dev);
int cvm_oct_sgmii_stop(struct net_device *dev);

int cvm_oct_spi_init(struct net_device *dev);
void cvm_oct_spi_uninit(struct net_device *dev);

int cvm_oct_xaui_init(struct net_device *dev);
int cvm_oct_xaui_open(struct net_device *dev);
int cvm_oct_xaui_stop(struct net_device *dev);

int cvm_oct_srio_init(struct net_device *dev);
int cvm_oct_srio_open(struct net_device *dev);
int cvm_oct_srio_stop(struct net_device *dev);
int cvm_oct_xmit_srio(struct sk_buff *skb, struct net_device *dev);
int cvm_oct_srio_set_mac_address(struct net_device *dev, void *addr);
int cvm_oct_srio_change_mtu(struct net_device *dev, int new_mtu);
struct net_device_stats *cvm_oct_srio_get_stats(struct net_device *dev);

int cvm_oct_common_init(struct net_device *dev);
int cvm_oct_common_stop(struct net_device *dev);

void cvm_oct_set_carrier(struct octeon_ethernet *priv,
				cvmx_helper_link_info_t link_info);
void cvm_oct_adjust_link(struct net_device *dev);

int cvm_oct_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int cvm_oct_phy_setup_device(struct net_device *dev);

int cvm_oct_xmit(struct sk_buff *skb, struct net_device *dev);
int cvm_oct_xmit_lockless(struct sk_buff *skb, struct net_device *dev);

void cvm_oct_poll_controller(struct net_device *dev);
void cvm_oct_rx_initialize(int);
void cvm_oct_rx_shutdown0(void);
void cvm_oct_rx_shutdown1(void);

int cvm_oct_mem_fill_fpa(int pool, int elements);
int cvm_oct_mem_empty_fpa(int pool, int elements);
int cvm_oct_alloc_fpa_pool(int pool, int size);
int cvm_oct_release_fpa_pool(int pool);
void cvm_oct_mem_cleanup(void);

extern const struct ethtool_ops cvm_oct_ethtool_ops;

extern int rx_cpu_factor;
extern int octeon_recycle_tx;
extern int packet_pool;
extern int wqe_pool;
extern int output_pool;
extern int always_use_pow;
extern int pow_send_group;
extern int pow_receive_group;
extern char pow_send_list[];
extern struct list_head cvm_oct_list;
extern struct octeon_ethernet *cvm_oct_by_pkind[];
extern struct octeon_ethernet *cvm_oct_by_srio_mbox[4][4];

extern struct workqueue_struct *cvm_oct_poll_queue;
extern atomic_t cvm_oct_poll_queue_stopping;

extern int max_rx_cpus;
extern int rx_napi_weight;

static inline void cvm_oct_rx_refill_pool(int fill_threshold)
{
	int number_to_free;
	int num_freed;
	/* Refill the packet buffer pool */
	number_to_free =
		cvmx_hwfau_fetch_and_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE, 0);

	if (number_to_free > fill_threshold) {
		cvmx_hwfau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE,
				      -number_to_free);
		num_freed = cvm_oct_mem_fill_fpa(packet_pool, number_to_free);
		if (num_freed != number_to_free) {
			cvmx_hwfau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE,
					number_to_free - num_freed);
		}
	}
}

/**
 * cvm_oct_get_buffer_ptr - convert packet data address to pointer
 * @pd: Packet data hardware address
 *
 * Returns Packet buffer pointer
 */
static inline void *cvm_oct_get_buffer_ptr(union cvmx_buf_ptr pd)
{
	return phys_to_virt(((pd.s.addr >> 7) - pd.s.back) << 7);
}

static inline struct sk_buff **cvm_oct_packet_to_skb(void *packet)
{
	char *p = packet;
	return (struct sk_buff **)(p - sizeof(void *));
}

#define CVM_OCT_SKB_TO_FPA_PADDING (128 + sizeof(void *) - 1)

static inline u8 *cvm_oct_get_fpa_head(struct sk_buff *skb)
{
	return (u8 *)((unsigned long)(skb->head + CVM_OCT_SKB_TO_FPA_PADDING) & ~0x7ful);
}

#endif
