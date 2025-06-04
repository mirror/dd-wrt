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
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/slab.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/interrupt.h>

#include <net/dst.h>

#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-hwfau.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-srio.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-pko-internal-ports-range.h>
#include <asm/octeon/cvmx-app-config.h>

#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-smix-defs.h>

#ifdef CONFIG_CAVIUM_OCTEON_IPFWD_OFFLOAD
#include "ipfwd_config.h"
#endif

int rx_cpu_factor = 8;
module_param(rx_cpu_factor, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(rx_cpu_factor, "Control how many CPUs are used for packet reception.\n"
		 "\tLarger numbers result in fewer CPUs used.");

int octeon_recycle_tx = REUSE_SKBUFFS_WITHOUT_FREE;
module_param_named(recycle_tx, octeon_recycle_tx, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(recycle_tx, "Allow hardware SKB recycling.");

int num_packet_buffers = 1024;
module_param(num_packet_buffers, int, 0444);
MODULE_PARM_DESC(num_packet_buffers, "\n"
	"\tNumber of packet buffers to allocate and store in the\n"
	"\tFPA. By default, 1024 packet buffers are used.");

int pow_receive_group = 15;
module_param(pow_receive_group, int, 0444);
MODULE_PARM_DESC(pow_receive_group, "\n"
	"\tPOW group to receive packets from. All ethernet hardware\n"
	"\twill be configured to send incomming packets to this POW\n"
	"\tgroup. Also any other software can submit packets to this\n"
	"\tgroup for the kernel to process.");

static int disable_core_queueing = 1;
module_param(disable_core_queueing, int, S_IRUGO);
MODULE_PARM_DESC(disable_core_queueing, "\n"
		"\t\tWhen set the networking core's tx_queue_len is set to zero.  This\n"
		"\t\tallows packets to be sent without lock contention in the packet scheduler\n"
		"\t\tresulting in some cases in improved throughput.");

int max_rx_cpus = -1;
module_param(max_rx_cpus, int, 0444);
MODULE_PARM_DESC(max_rx_cpus, "\n"
	"\t\tThe maximum number of CPUs to use for packet reception.\n"
	"\t\tUse -1 to use all available CPUs.");

int rx_napi_weight = 32;
module_param(rx_napi_weight, int, 0444);
MODULE_PARM_DESC(rx_napi_weight, "The NAPI WEIGHT parameter.");

static int disable_lockless_pko;
module_param(disable_lockless_pko, int, S_IRUGO);
MODULE_PARM_DESC(disable_lockless_pko, "Disable lockless PKO access (use locking for queues instead).");

#if defined(CONFIG_CAVIUM_IPFWD_OFFLOAD) && defined(IPFWD_OUTPUT_QOS)
/* internal ports count for each port in a interface */
int iport_count = 1;
#include <asm/octeon/cvmx-helper.h>
CVMX_SHARED void ipfwd_pko_queue_priority(int ipd_port, uint8_t *priorities)
{
	int i;

	for (i = 0; i < 16; i++)
		priorities[i] = CVMX_PKO_QUEUE_STATIC_PRIORITY;
}

/* pko queue count for each port in a interface */
int queues_count = PKO_QUEUES_PER_PORT;
#else
/* pko queue count for each port in a interface */
int queues_count = 1;
#endif
/* packet pool */
int packet_pool = 0;
/* wqe pool */
int wqe_pool = -1;
/* output pool */
int output_pool = -1;

/**
 * cvm_oct_poll_queue - Workqueue for polling operations.
 */
struct workqueue_struct *cvm_oct_poll_queue;

/**
 * cvm_oct_poll_queue_stopping - flag to indicate polling should stop.
 *
 * Set to one right before cvm_oct_poll_queue is destroyed.
 */
atomic_t cvm_oct_poll_queue_stopping = ATOMIC_INIT(0);

/* cvm_oct_by_pkind is an array of every ethernet device owned by this
 * driver indexed by the IPD pkind/port_number.  If an entry is empty
 * (NULL) it either doesn't exist, or there was a collision.  The two
 * cases can be distinguished by trying to look up via
 * cvm_oct_dev_for_port();
 */
struct octeon_ethernet *cvm_oct_by_pkind[64] __cacheline_aligned;

/*
 * cvm_oct_by_srio_mbox is indexed by the SRIO mailbox.
 */
struct octeon_ethernet *cvm_oct_by_srio_mbox[4][4];

/* cvm_oct_list is a list of all cvm_oct_private_t created by this driver. */
LIST_HEAD(cvm_oct_list);

static void cvm_oct_rx_refill_worker(struct work_struct *work);
static DECLARE_DELAYED_WORK(cvm_oct_rx_refill_work, cvm_oct_rx_refill_worker);

static void cvm_oct_rx_refill_worker(struct work_struct *work)
{
	/* FPA 0 may have been drained, try to refill it if we need
	 * more than num_packet_buffers / 2, otherwise normal receive
	 * processing will refill it.  If it were drained, no packets
	 * could be received so cvm_oct_napi_poll would never be
	 * invoked to do the refill.
	 */
	cvm_oct_rx_refill_pool(num_packet_buffers / 2);

	if (!atomic_read(&cvm_oct_poll_queue_stopping))
		queue_delayed_work(cvm_oct_poll_queue,
				   &cvm_oct_rx_refill_work, HZ);
}

static void cvm_oct_periodic_worker(struct work_struct *work)
{
	struct octeon_ethernet *priv = container_of(work,
						    struct octeon_ethernet,
						    port_periodic_work.work);
	void (*poll_fn) (struct net_device *);

	poll_fn = READ_ONCE(priv->poll);

	if (poll_fn)
		poll_fn(priv->netdev);

	priv->netdev->netdev_ops->ndo_get_stats(priv->netdev);

	if (!atomic_read(&cvm_oct_poll_queue_stopping))
		queue_delayed_work(cvm_oct_poll_queue, &priv->port_periodic_work, HZ);
}

static int cvm_oct_num_output_buffers;

static int cvm_oct_get_total_pko_queues(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN38XX))
		return 128;
	else if (OCTEON_IS_MODEL(OCTEON_CN3XXX))
		return 32;
	else if (OCTEON_IS_MODEL(OCTEON_CN50XX))
		return 32;
	else
		return 256;
}

static bool cvm_oct_pko_lockless(void)
{
	int interface, num_interfaces;
	int queues = 0;

	if (disable_lockless_pko)
		return false;

	/* CN3XXX require workarounds in xmit.  Disable lockless for
	 * CN3XXX to optimize the lockless case with out the
	 * workarounds.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN3XXX))
		return false;

	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports, port;
		cvmx_helper_interface_mode_t imode = cvmx_helper_interface_get_mode(interface);

		num_ports = cvmx_helper_interface_enumerate(interface);
		for (port = 0; port < num_ports; port++) {
			if (!cvmx_helper_is_port_valid(interface, port))
				continue;
			switch (imode) {
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				queues += max(8u, num_possible_cpus());
				break;
			case CVMX_HELPER_INTERFACE_MODE_NPI:
			case CVMX_HELPER_INTERFACE_MODE_LOOP:
#ifdef CONFIG_RAPIDIO
			case CVMX_HELPER_INTERFACE_MODE_SRIO:
#endif
				queues += 1;
				break;
			default:
				break;
			}
		}
	}
	return queues <= cvm_oct_get_total_pko_queues();
}

#if defined(CONFIG_CAVIUM_IPFWD_OFFLOAD) && defined(IPFWD_OUTPUT_QOS)
static void cvm_oct_set_pko_multiqueue(void)
{
	int interface, num_interfaces, rv;

	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports, port;
		cvmx_helper_interface_mode_t imode =
			cvmx_helper_interface_get_mode(interface);

		num_ports = cvmx_helper_interface_enumerate(interface);
		for (port = 0; port < num_ports; port++) {
			if (!cvmx_helper_is_port_valid(interface, port))
				continue;
			switch (imode) {
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_AGL:
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				rv = cvmx_pko_alloc_iport_and_queues(interface,
						port, 1, queues_count);
				WARN(rv, "cvmx_pko_alloc_iport_and_queues failed");
				if (rv)
					return;
				break;
			default:
				break;
			}
		}
	}
}
#else
static void cvm_oct_set_pko_multiqueue(void)
{
	int interface, num_interfaces, rv;

	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports, port;
		cvmx_helper_interface_mode_t imode = cvmx_helper_interface_get_mode(interface);

		num_ports = cvmx_helper_interface_enumerate(interface);
		for (port = 0; port < num_ports; port++) {
			if (!cvmx_helper_is_port_valid(interface, port))
				continue;
			switch (imode) {
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_AGL:
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				rv = cvmx_pko_alloc_iport_and_queues(interface, port, 1,
								     max(8u, num_possible_cpus()));
				WARN(rv, "cvmx_pko_alloc_iport_and_queues failed");
				if (rv)
					return;
				break;
			default:
				break;
			}
		}
	}
}
#endif

static int cvm_oct_configure_common_hw(void)
{
#if defined(CONFIG_CAVIUM_IPFWD_OFFLOAD) && defined(IPFWD_OUTPUT_QOS)
	cvmx_override_pko_queue_priority = ipfwd_pko_queue_priority;
#endif
	/* Setup the FPA */
	cvmx_fpa1_enable();

	/* allocate packet pool */
	packet_pool = cvm_oct_alloc_fpa_pool(packet_pool, FPA_PACKET_POOL_SIZE);
	if (packet_pool < 0) {
		pr_err("cvm_oct_alloc_fpa_pool(%d, FPA_PACKET_POOL_SIZE) failed.\n", packet_pool);
		return -ENOMEM;
	}
	cvm_oct_mem_fill_fpa(packet_pool, num_packet_buffers);

	/* communicate packet pool number to ipd */
	cvmx_ipd_set_packet_pool_config(packet_pool, FPA_PACKET_POOL_SIZE,
					num_packet_buffers);

	/* allocate wqe pool */
	wqe_pool = cvm_oct_alloc_fpa_pool(-1, FPA_WQE_POOL_SIZE);
	if (wqe_pool < 0) {
		pr_err("cvm_oct_alloc_fpa_pool(-1, FPA_WQE_POOL_SIZE) failed.\n");
		return -ENOMEM;;
	}
	cvm_oct_mem_fill_fpa(wqe_pool, num_packet_buffers);

	/* communicate wqe pool to ipd */
	cvmx_ipd_set_wqe_pool_config(wqe_pool, FPA_WQE_POOL_SIZE,
				     num_packet_buffers);

	if (cvm_oct_pko_lockless()) {
		cvm_oct_set_pko_multiqueue();
		cvm_oct_num_output_buffers = 4 * cvm_oct_get_total_pko_queues();
	} else {
		cvm_oct_num_output_buffers = 128;
	}

	/* alloc fpa pool for output buffers */
	output_pool = cvm_oct_alloc_fpa_pool(-1, FPA_OUTPUT_BUFFER_POOL_SIZE);
	if (output_pool < 0) {
		pr_err("cvm_oct_alloc_fpa_pool(-1, FPA_OUTPUT_BUFFER_POOL_SIZE) failed.\n");
		return -ENOMEM;;
	}
	cvm_oct_mem_fill_fpa(output_pool, cvm_oct_num_output_buffers);

	/* communicate output pool no. to pko */
	cvmx_pko_set_cmd_que_pool_config(output_pool,
					 FPA_OUTPUT_BUFFER_POOL_SIZE,
					 cvm_oct_num_output_buffers);

	/* more configuration needs to be done, so enable ipd seperately */
	cvmx_ipd_cfg.ipd_enable = 0;

	__cvmx_export_app_config_to_named_block(CVMX_APP_CONFIG);

	cvmx_helper_initialize_packet_io_global();

#ifdef __LITTLE_ENDIAN
	{
		union cvmx_ipd_ctl_status ipd_ctl_status;
		ipd_ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
		ipd_ctl_status.s.pkt_lend = 1;
		ipd_ctl_status.s.wqe_lend = 1;
		cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_status.u64);
	}
#endif
	/* Enable red after interface is initialized */
	if (USE_RED)
		cvmx_helper_setup_red(num_packet_buffers / 4,
				      num_packet_buffers / 8);

	return 0;
}

/**
 * cvm_oct_register_callback -  Register a intercept callback for the named device.
 *
 * It returns the net_device structure for the ethernet port. Usign a
 * callback of NULL will remove the callback. Note that this callback
 * must not disturb scratch. It will be called with SYNCIOBDMAs in
 * progress and userspace may be using scratch. It also must not
 * disturb the group mask.
 *
 * @device_name: Device name to register for. (Example: "eth0")
 * @callback: Intercept callback to set.
 *
 * Returns the net_device structure for the ethernet port or NULL on failure.
 */
struct net_device *cvm_oct_register_callback(const char *device_name, cvm_oct_callback_t callback)
{
	struct octeon_ethernet *priv;

	list_for_each_entry(priv, &cvm_oct_list, list) {
		if (strcmp(device_name, priv->netdev->name) == 0) {
			priv->intercept_cb = callback;
			wmb();
			return priv->netdev;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(cvm_oct_register_callback);

#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
struct net_device *is_oct_dev(const char *device_name)
{
	struct octeon_ethernet *priv;

	list_for_each_entry(priv, &cvm_oct_list, list) {
		if (strcmp(device_name, priv->netdev->name) == 0) {
			/* wmb */
			wmb();
			return priv->netdev;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(is_oct_dev);
#endif

/**
 * cvm_oct_free_work- Free a work queue entry
 *
 * @work_queue_entry: Work queue entry to free
 *
 * Returns Zero on success, Negative on failure.
 */
int cvm_oct_free_work(void *work_queue_entry)
{
	cvmx_wqe_t *work = work_queue_entry;

	int segments = work->word2.s.bufs;
	union cvmx_buf_ptr segment_ptr = work->packet_ptr;

	while (segments--) {
		union cvmx_buf_ptr next_ptr = *(union cvmx_buf_ptr *)phys_to_virt(segment_ptr.s.addr - 8);
		if (!segment_ptr.s.i)
			cvmx_fpa1_free(cvm_oct_get_buffer_ptr(segment_ptr),
				      segment_ptr.s.pool,
				      DONT_WRITEBACK(FPA_PACKET_POOL_SIZE / 128));
		segment_ptr = next_ptr;
	}
	cvmx_fpa1_free(work, wqe_pool, DONT_WRITEBACK(1));

	return 0;
}
EXPORT_SYMBOL(cvm_oct_free_work);

/* Lock to protect racy cvmx_pko_get_port_status() */
static DEFINE_SPINLOCK(cvm_oct_tx_stat_lock);

/**
 * cvm_oct_common_get_stats - get the low level ethernet statistics
 * @dev:    Device to get the statistics from
 *
 * Returns Pointer to the statistics
 */
static struct net_device_stats *cvm_oct_common_get_stats(struct net_device *dev)
{
	unsigned long flags;
	cvmx_pip_port_status_t rx_status;
	cvmx_pko_port_status_t tx_status;
	u64 current_tx_octets;
	u32 current_tx_packets;
	struct octeon_ethernet *priv = netdev_priv(dev);

	if (octeon_is_simulation()) {
		/* The simulator doesn't support statistics */
		memset(&rx_status, 0, sizeof(rx_status));
		memset(&tx_status, 0, sizeof(tx_status));
	} else {
		cvmx_pip_get_port_status(priv->ipd_port, 1, &rx_status);

		spin_lock_irqsave(&cvm_oct_tx_stat_lock, flags);
		cvmx_pko_get_port_status(priv->ipd_port, 0, &tx_status);
		current_tx_packets = tx_status.packets;
		current_tx_octets = tx_status.octets;
		/* The tx_packets counter is 32-bits as are all these
		 * variables.  No truncation necessary.
		 */
		tx_status.packets = current_tx_packets - priv->last_tx_packets;
		/* The tx_octets counter is only 48-bits, so we need
		 * to truncate in case there was a wrap-around
		 */
		tx_status.octets = (current_tx_octets - priv->last_tx_octets) & 0xffffffffffffull;
		priv->last_tx_packets = current_tx_packets;
		priv->last_tx_octets = current_tx_octets;
		spin_unlock_irqrestore(&cvm_oct_tx_stat_lock, flags);
	}

	dev->stats.rx_packets += rx_status.inb_packets;
	dev->stats.tx_packets += tx_status.packets;
	dev->stats.rx_bytes += rx_status.inb_octets;
	dev->stats.tx_bytes += tx_status.octets;
	dev->stats.multicast += rx_status.multicast_packets;
	dev->stats.rx_crc_errors += rx_status.inb_errors;
	dev->stats.rx_frame_errors += rx_status.fcs_align_err_packets;

	/* The drop counter must be incremented atomically since the
	 * RX tasklet also increments it.
	 */
	atomic64_add(rx_status.dropped_packets,
		     (atomic64_t *)&dev->stats.rx_dropped);

	return &dev->stats;
}

/**
 * cvm_oct_change_mtu - change the link MTU
 * @dev:     Device to change
 * @new_mtu: The new MTU
 *
 * Returns Zero on success
 */
static int cvm_oct_change_mtu(struct net_device *dev, int new_mtu)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int ret;
	u64 ipd_reg;

	if (OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN58XX))
		ipd_reg = 0;
	else
		ipd_reg = CVMX_PIP_PRT_CFGX(priv->ipd_pkind);

	ret = cvm_oct_common_change_mtu(dev, new_mtu, priv->gmx_base, ipd_reg, 65392);

	if (ret)
		return ret;

	return 0;
}

/**
 * Set RX filtering
 * @dev:    Device to work on
 */
static void cvm_oct_set_rx_filter(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);

	if (priv->gmx_base)
		cvm_oct_common_set_rx_filtering(dev, priv->gmx_base,
					 &priv->poll_lock);
}

/**
 * Set the hardware MAC address for a device
 * @dev:    The device in question.
 * @addr:   Address structure to change it too.
 *
 * Returns Zero on success
 */
static int cvm_oct_set_mac_address(struct net_device *dev, void *addr)
{
	struct octeon_ethernet *priv = netdev_priv(dev);

	if (priv->gmx_base)
		cvm_oct_common_set_mac_address(dev, addr, priv->gmx_base,
					 &priv->poll_lock);

	return 0;
}

/**
 * cvm_oct_common_init - per network device initialization
 * @dev:    Device to initialize
 *
 * Returns Zero on success
 */
int cvm_oct_common_init(struct net_device *dev)
{
	unsigned long flags;
	cvmx_pko_port_status_t tx_status;
	struct octeon_ethernet *priv = netdev_priv(dev);
	struct sockaddr sa;
	int ret = 1;
	if (priv->of_node)
		ret = of_get_ethdev_address(priv->of_node, dev);
	
	if (ret)
		eth_hw_addr_random(dev);

	if (priv->num_tx_queues != -1) {
		dev->features |= NETIF_F_SG | NETIF_F_FRAGLIST;
		if (USE_HW_TCPUDP_CHECKSUM)
			dev->features |= NETIF_F_IP_CSUM;
	}

	/* We do our own locking, Linux doesn't need to */
	dev->features |= NETIF_F_LLTX;
	dev->ethtool_ops = &cvm_oct_ethtool_ops;

	memcpy(sa.sa_data, dev->dev_addr, ETH_ALEN);
	cvm_oct_common_set_mac_address(dev, &sa, priv->gmx_base, &priv->poll_lock);
	dev->netdev_ops->ndo_change_mtu(dev, dev->mtu);

	spin_lock_irqsave(&cvm_oct_tx_stat_lock, flags);
	cvmx_pko_get_port_status(priv->ipd_port, 0, &tx_status);
	priv->last_tx_packets = tx_status.packets;
	priv->last_tx_octets = tx_status.octets;
	/* Zero out stats for port so we won't mistakenly show
	 * counters from the bootloader.
	 */
	memset(&dev->stats, 0, sizeof(struct net_device_stats));
	spin_unlock_irqrestore(&cvm_oct_tx_stat_lock, flags);

	return 0;
}

static const struct net_device_ops cvm_oct_npi_netdev_ops = {
	.ndo_init		= cvm_oct_common_init,
	.ndo_start_xmit		= cvm_oct_xmit,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};

/* SGMII, AGL and XAUI handled the same so they both use this. */
static const struct net_device_ops cvm_oct_sgmii_netdev_ops = {
	.ndo_init		= cvm_oct_sgmii_init,
	.ndo_uninit		= cvm_oct_sgmii_uninit,
	.ndo_open		= cvm_oct_sgmii_open,
	.ndo_stop		= cvm_oct_sgmii_stop,
	.ndo_start_xmit		= cvm_oct_xmit,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
static const struct net_device_ops cvm_oct_sgmii_lockless_netdev_ops = {
	.ndo_init		= cvm_oct_sgmii_init,
	.ndo_uninit		= cvm_oct_sgmii_uninit,
	.ndo_open		= cvm_oct_sgmii_open,
	.ndo_stop		= cvm_oct_sgmii_stop,
	.ndo_start_xmit		= cvm_oct_xmit_lockless,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
static const struct net_device_ops cvm_oct_spi_netdev_ops = {
	.ndo_init		= cvm_oct_spi_init,
	.ndo_uninit		= cvm_oct_spi_uninit,
	.ndo_open		= cvm_oct_phy_setup_device,
	.ndo_stop		= cvm_oct_common_stop,
	.ndo_start_xmit		= cvm_oct_xmit,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
static const struct net_device_ops cvm_oct_spi_lockless_netdev_ops = {
	.ndo_init		= cvm_oct_spi_init,
	.ndo_uninit		= cvm_oct_spi_uninit,
	.ndo_open		= cvm_oct_phy_setup_device,
	.ndo_start_xmit		= cvm_oct_xmit_lockless,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
static const struct net_device_ops cvm_oct_rgmii_netdev_ops = {
	.ndo_init		= cvm_oct_rgmii_init,
	.ndo_open		= cvm_oct_rgmii_open,
	.ndo_stop		= cvm_oct_rgmii_stop,
	.ndo_start_xmit		= cvm_oct_xmit,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
static const struct net_device_ops cvm_oct_rgmii_lockless_netdev_ops = {
	.ndo_init		= cvm_oct_rgmii_init,
	.ndo_open		= cvm_oct_rgmii_open,
	.ndo_stop		= cvm_oct_rgmii_stop,
	.ndo_start_xmit		= cvm_oct_xmit_lockless,
	.ndo_set_rx_mode	= cvm_oct_set_rx_filter,
	.ndo_set_mac_address	= cvm_oct_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_change_mtu,
	.ndo_get_stats		= cvm_oct_common_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
#ifdef CONFIG_RAPIDIO
static const struct net_device_ops cvm_oct_srio_netdev_ops = {
	.ndo_init		= cvm_oct_srio_init,
	.ndo_open		= cvm_oct_srio_open,
	.ndo_stop		= cvm_oct_srio_stop,
	.ndo_start_xmit		= cvm_oct_xmit_srio,
	.ndo_set_mac_address	= cvm_oct_srio_set_mac_address,
	.ndo_do_ioctl		= cvm_oct_ioctl,
	.ndo_change_mtu		= cvm_oct_srio_change_mtu,
	.ndo_get_stats		= cvm_oct_srio_get_stats,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= cvm_oct_poll_controller,
#endif
};
#endif

static int num_devices_extra_wqe;
#define PER_DEVICE_EXTRA_WQE (MAX_OUT_QUEUE_DEPTH)

static struct rb_root cvm_oct_ipd_tree = RB_ROOT;

void cvm_oct_add_ipd_port(struct octeon_ethernet *port)
{
	struct rb_node **link = &cvm_oct_ipd_tree.rb_node;
	struct rb_node *parent = NULL;
	struct octeon_ethernet *n;
	int value = port->key;

	while (*link) {
		parent = *link;
		n = rb_entry(parent, struct octeon_ethernet, ipd_tree);

		if (value < n->key)
			link = &(*link)->rb_left;
		else if (value > n->key)
			link = &(*link)->rb_right;
		else
			BUG();
	}
	rb_link_node(&port->ipd_tree, parent, link);
	rb_insert_color(&port->ipd_tree, &cvm_oct_ipd_tree);
}

struct octeon_ethernet *cvm_oct_dev_for_port(int port_number)
{
	struct rb_node *n = cvm_oct_ipd_tree.rb_node;
	while (n) {
		struct octeon_ethernet *s = rb_entry(n, struct octeon_ethernet, ipd_tree);

		if (s->key > port_number)
			n = n->rb_left;
		else if (s->key < port_number)
			n = n->rb_right;
		else
			return s;
	}
	return NULL;
}

static struct device_node *cvm_oct_of_get_child(const struct device_node *parent,
						int reg_val)
{
	struct device_node *node = NULL;
	int size;
	const __be32 *addr;

	for (;;) {
		node = of_get_next_child(parent, node);
		if (!node)
			break;
		addr = of_get_property(node, "reg", &size);
		if (addr && (be32_to_cpu(*addr) == reg_val))
			break;
	}
	return node;
}

static struct device_node *cvm_oct_node_for_port(struct device_node *pip,
						 int interface, int port)
{
	struct device_node *ni, *np;

	ni = cvm_oct_of_get_child(pip, interface);
	if (!ni)
		return NULL;

	np = cvm_oct_of_get_child(ni, port);
	of_node_put(ni);

	return np;
}

static bool cvm_is_phy_sgmii(struct device_node *node)
{
	struct device_node	*phy_node;
	const char		*p;
	bool			rc = false;

	phy_node = of_parse_phandle(node, "phy-handle", 0);
	if (phy_node == NULL)
		return rc;

	if (!of_property_read_string(phy_node, "vitesse,phy-mode", &p)) {
		if (!strcmp(p, "sgmii"))
			rc = true;
	}
	of_node_put(phy_node);

	return rc;
}

static int cvm_oct_get_port_status(struct device_node *pip)
{
	int i, j;
	int num_interfaces = cvmx_helper_get_number_of_interfaces();

	for (i = 0; i < num_interfaces; i++) {
		int num_ports = cvmx_helper_interface_enumerate(i);
		int mode = cvmx_helper_interface_get_mode(i);
		struct device_node *port_node;

		for (j = 0; j < num_ports; j++) {
			port_node = cvm_oct_node_for_port(pip, i, j);
			switch (mode) {
			case CVMX_HELPER_INTERFACE_MODE_AGL:
				if (port_node &&
				    of_get_property(port_node,
						    "cavium,rx-clk-delay-bypass",
						    NULL))
					cvmx_helper_set_agl_rx_clock_delay_bypass(i, j, true);
					/* Continue on */
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				if (port_node)
					cvmx_helper_set_port_valid(i, j, true);
				else
					cvmx_helper_set_port_valid(i, j, false);
				cvmx_helper_set_mac_phy_mode(i, j, false);
				cvmx_helper_set_1000x_mode(i, j, false);
				break;
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			{
				if (port_node != NULL)
					cvmx_helper_set_port_valid(i, j, true);
				else
					cvmx_helper_set_port_valid(i, j, false);
				cvmx_helper_set_mac_phy_mode(i, j, false);
				cvmx_helper_set_1000x_mode(i, j, false);
				if (port_node) {
					if ((of_get_property(port_node,
					     "cavium,sgmii-mac-phy-mode", NULL) != NULL) ||
					    cvm_is_phy_sgmii(port_node))
						cvmx_helper_set_mac_phy_mode(i, j, true);
					if (of_get_property(port_node, 
					    "cavium,sgmii-mac-1000x-mode", NULL) 
					    != NULL)
						cvmx_helper_set_1000x_mode(i, j, true);
				}
				break;
			}
			default:
				cvmx_helper_set_port_valid(i, j, true);
				cvmx_helper_set_mac_phy_mode(i, j, false);
				cvmx_helper_set_1000x_mode(i, j, false);
				break;
			}
		}
	}
	return 0;
}

static int cvm_oct_probe(struct platform_device *pdev)
{
	int num_interfaces;
	int interface;
	int fau = FAU_NUM_PACKET_BUFFERS_TO_FREE;
	int qos, r;
	struct device_node *pip;

	pr_notice("octeon-ethernet %s\n", OCTEON_ETHERNET_VERSION);

	pip = pdev->dev.of_node;
	if (!pip) {
		dev_err(&pdev->dev, "No of_node.\n");
		return -EINVAL;
	}

	cvm_oct_get_port_status(pip);

	cvm_oct_poll_queue = create_singlethread_workqueue("octeon-ethernet");
	if (cvm_oct_poll_queue == NULL) {
		dev_err(&pdev->dev, "Cannot create workqueue");
		return -ENOMEM;
	}

	r = cvm_oct_configure_common_hw();
	if (r)
		return r;

	/* Change the input group for all ports before input is enabled */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports = cvmx_helper_ports_on_interface(interface);
		int index;

		for (index = 0; index < num_ports; index++) {
			union cvmx_pip_prt_tagx pip_prt_tagx;
			int port = cvmx_helper_get_ipd_port(interface, index);

			if (octeon_has_feature(OCTEON_FEATURE_PKND))
				port = cvmx_helper_get_pknd(interface, index);

			if (!cvmx_helper_is_port_valid(interface, index))
				continue;
			pip_prt_tagx.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port));
			pip_prt_tagx.s.grp = pow_receive_group;
			cvmx_write_csr(CVMX_PIP_PRT_TAGX(port), pip_prt_tagx.u64);
		}
	}

	cvmx_helper_ipd_and_packet_input_enable_node(0);

	/* Initialize the FAU used for counting packet buffers that
	 * need to be freed.
	 */
	cvmx_hwfau_atomic_write32(FAU_NUM_PACKET_BUFFERS_TO_FREE, 0);

	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		cvmx_helper_interface_mode_t imode = cvmx_helper_interface_get_mode(interface);
		int num_ports = cvmx_helper_ports_on_interface(interface);
		int interface_port;

		if (imode == CVMX_HELPER_INTERFACE_MODE_SRIO)
			num_ports = 2; /* consistent with se apps. could be 4 */

		for (interface_port = 0; interface_port < num_ports;
		     interface_port++) {
			struct octeon_ethernet *priv;
			int base_queue;
			struct net_device *dev;

			if (!cvmx_helper_is_port_valid(interface,
						      interface_port))
				continue;

			dev = alloc_etherdev(sizeof(struct octeon_ethernet));
			if (!dev) {
				dev_err(&pdev->dev,
					"Failed to allocate ethernet device for port %d:%d\n",
					interface, interface_port);
				continue;
			}

#ifdef CONFIG_CAVIUM_IPFWD_OFFLOAD
			dev->is_cvm_dev = 1;
#endif
			/* Using transmit queues degrades performance significantly */
			if (disable_core_queueing)
				dev->priv_flags |= IFF_NO_QUEUE;

			/* Initialize the device private structure. */
			SET_NETDEV_DEV(dev, &pdev->dev);
			priv = netdev_priv(dev);
			INIT_LIST_HEAD(&priv->srio_bcast);
			priv->of_node = cvm_oct_node_for_port(pip, interface, interface_port);
			dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
			priv->netdev = dev;
			priv->interface = interface;
			priv->interface_port = interface_port;
			priv->gmx_base = 0;
			spin_lock_init(&priv->poll_lock);
			INIT_DELAYED_WORK(&priv->port_periodic_work,
					  cvm_oct_periodic_worker);
			priv->imode = imode;

			if (imode == CVMX_HELPER_INTERFACE_MODE_SRIO) {
				int mbox = cvmx_helper_get_ipd_port(interface, interface_port) - cvmx_helper_get_ipd_port(interface, 0);
				union cvmx_srio_tx_message_header tx_header;
				tx_header.u64 = 0;
				tx_header.s.tt = 0;
				tx_header.s.ssize = 0xe;
				tx_header.s.mbox = mbox;
				tx_header.s.lns = 1;
				tx_header.s.intr = 1;
				priv->srio_tx_header = tx_header.u64;
				priv->ipd_port = cvmx_helper_get_ipd_port(interface, mbox);
				priv->pko_port = priv->ipd_port;
				priv->key = priv->ipd_port + (0x10000 * mbox);
				base_queue = cvmx_pko_get_base_queue(priv->ipd_port);
				priv->num_tx_queues = 1;
				cvm_oct_by_srio_mbox[interface - 4][mbox] = priv;
			} else {
				priv->ipd_port = cvmx_helper_get_ipd_port(interface, interface_port);
				priv->key = priv->ipd_port;
				priv->pko_port = cvmx_helper_get_pko_port(interface, interface_port);
				base_queue = cvmx_pko_get_base_queue(priv->ipd_port);
				priv->num_tx_queues = cvmx_pko_get_num_queues(priv->ipd_port);
			}

			if (priv->num_tx_queues == 0) {
				dev_err(&pdev->dev,
					"tx_queue count not configured for port %d:%d\n",
					interface, interface_port);
				free_netdev(dev);
				continue;
			}

			BUG_ON(priv->num_tx_queues < 1);
			BUG_ON(priv->num_tx_queues > 32);

			if (octeon_has_feature(OCTEON_FEATURE_PKND))
				priv->ipd_pkind = cvmx_helper_get_pknd(interface, interface_port);
			else
				priv->ipd_pkind = priv->ipd_port;

			for (qos = 0; qos < priv->num_tx_queues; qos++) {
				priv->tx_queue[qos].queue = base_queue + qos;
				fau = fau - sizeof(u32);
				priv->tx_queue[qos].fau = fau;
				cvmx_hwfau_atomic_write32(priv->tx_queue[qos].fau, 0);
			}

			/* Cache the fact that there may be multiple queues */
			priv->tx_multiple_queues = (priv->num_tx_queues > 1);

			switch (priv->imode) {
			/* These types don't support ports to IPD/PKO */
			case CVMX_HELPER_INTERFACE_MODE_DISABLED:
			case CVMX_HELPER_INTERFACE_MODE_PCIE:
			case CVMX_HELPER_INTERFACE_MODE_PICMG:
				break;

			case CVMX_HELPER_INTERFACE_MODE_NPI:
				dev->netdev_ops = &cvm_oct_npi_netdev_ops;
				strcpy(dev->name, "npi%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
				priv->tx_lockless = priv->tx_multiple_queues && !disable_lockless_pko;
				dev->netdev_ops = priv->tx_lockless ?
					&cvm_oct_sgmii_lockless_netdev_ops : &cvm_oct_sgmii_netdev_ops;
				dev->priv_flags |= IFF_UNICAST_FLT;
				priv->gmx_base = CVMX_GMXX_RXX_INT_REG(interface_port, interface);
				strcpy(dev->name, "xaui%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_LOOP:
				dev->netdev_ops = &cvm_oct_npi_netdev_ops;
				strcpy(dev->name, "loop%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
				priv->tx_lockless = priv->tx_multiple_queues && !disable_lockless_pko;
				dev->netdev_ops = priv->tx_lockless ?
					&cvm_oct_sgmii_lockless_netdev_ops : &cvm_oct_sgmii_netdev_ops;
				dev->priv_flags |= IFF_UNICAST_FLT;
				priv->gmx_base = CVMX_GMXX_RXX_INT_REG(interface_port, interface);
				strcpy(dev->name, "eth%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_AGL:
				priv->tx_lockless = priv->tx_multiple_queues && !disable_lockless_pko;
				dev->netdev_ops = priv->tx_lockless ?
					&cvm_oct_sgmii_lockless_netdev_ops : &cvm_oct_sgmii_netdev_ops;
				dev->priv_flags |= IFF_UNICAST_FLT;
				priv->gmx_base = CVMX_AGL_GMX_RXX_INT_REG(0);
				strcpy(dev->name, "agl%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_SPI:
				priv->tx_lockless = priv->tx_multiple_queues && !disable_lockless_pko;
				dev->netdev_ops = priv->tx_lockless ?
					&cvm_oct_spi_lockless_netdev_ops : &cvm_oct_spi_netdev_ops;
				dev->priv_flags |= IFF_UNICAST_FLT;
				priv->gmx_base = CVMX_GMXX_RXX_INT_REG(interface_port, interface);
				strcpy(dev->name, "spi%d");
				break;

			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
				priv->tx_lockless = priv->tx_multiple_queues && !disable_lockless_pko;
				dev->netdev_ops = priv->tx_lockless ?
					&cvm_oct_rgmii_lockless_netdev_ops : &cvm_oct_rgmii_netdev_ops;
				dev->priv_flags |= IFF_UNICAST_FLT;
				priv->gmx_base = CVMX_GMXX_RXX_INT_REG(interface_port, interface);
				strcpy(dev->name, "eth%d");
				break;
#ifdef CONFIG_RAPIDIO
			case CVMX_HELPER_INTERFACE_MODE_SRIO:
				dev->netdev_ops = &cvm_oct_srio_netdev_ops;
				strcpy(dev->name, "rio%d");
				break;
#endif
			}

			if (priv->of_node && of_phy_is_fixed_link(priv->of_node)) {
				if (of_phy_register_fixed_link(priv->of_node)) {
					netdev_err(dev, "Failed to register fixed link for interface %d, port %d\n",
						   interface, priv->interface_port);
					dev->netdev_ops = NULL;
				}
			}

			if (!dev->netdev_ops) {
				free_netdev(dev);
			} else if (register_netdev(dev) < 0) {
				dev_err(&pdev->dev,
					"Failed to register ethernet device for interface %d, port %d\n",
					interface, priv->ipd_port);
				free_netdev(dev);
			} else {
				list_add_tail(&priv->list, &cvm_oct_list);
				if (cvm_oct_by_pkind[priv->ipd_pkind] == NULL)
					cvm_oct_by_pkind[priv->ipd_pkind] = priv;
				else
					cvm_oct_by_pkind[priv->ipd_pkind] = (void *)-1L;

				cvm_oct_add_ipd_port(priv);
				/* Each transmit queue will need its
				 * own MAX_OUT_QUEUE_DEPTH worth of
				 * WQE to track the transmit skbs.
				 */
				cvm_oct_mem_fill_fpa(wqe_pool,
						     PER_DEVICE_EXTRA_WQE);
				num_devices_extra_wqe++;
				queue_delayed_work(cvm_oct_poll_queue,
						   &priv->port_periodic_work, HZ);
			}
		}
	}

	cvm_oct_rx_initialize(num_packet_buffers + num_devices_extra_wqe * PER_DEVICE_EXTRA_WQE);

	queue_delayed_work(cvm_oct_poll_queue, &cvm_oct_rx_refill_work, HZ);

	return 0;
}

static int cvm_oct_remove(struct platform_device *pdev)
{
	struct octeon_ethernet *priv;
	struct octeon_ethernet *tmp;

	/* Put both taking the interface down and unregistering it
	 * under the lock.  That way the devices cannot be taken back
	 * up in the middle of everything.
	 */
	rtnl_lock();

	/* Take down all the interfaces, this disables the GMX and
	 * prevents it from getting into a Bad State when IPD is
	 * disabled.
	 */
	list_for_each_entry(priv, &cvm_oct_list, list) {
		unsigned int f = dev_get_flags(priv->netdev);
		dev_change_flags(priv->netdev, f & ~IFF_UP, NULL);
	}

	mdelay(10);

	cvmx_ipd_disable();

	mdelay(10);

	atomic_inc_return(&cvm_oct_poll_queue_stopping);
	cancel_delayed_work_sync(&cvm_oct_rx_refill_work);

	cvm_oct_rx_shutdown0();

	/* unregister the ethernet devices */
	list_for_each_entry(priv, &cvm_oct_list, list) {
		cancel_delayed_work_sync(&priv->port_periodic_work);
		unregister_netdevice(priv->netdev);
	}

	rtnl_unlock();

	/* Free the ethernet devices */
	list_for_each_entry_safe_reverse(priv, tmp, &cvm_oct_list, list) {
		list_del(&priv->list);
		free_netdev(priv->netdev);
	}

	cvmx_helper_shutdown_packet_io_global();

	cvm_oct_rx_shutdown1();

	destroy_workqueue(cvm_oct_poll_queue);

	/* Free the HW pools */
	cvm_oct_mem_empty_fpa(packet_pool, num_packet_buffers);
	cvm_oct_release_fpa_pool(packet_pool);

	cvm_oct_mem_empty_fpa(wqe_pool,
			      num_packet_buffers + num_devices_extra_wqe * PER_DEVICE_EXTRA_WQE);
	cvm_oct_release_fpa_pool(wqe_pool);

	cvm_oct_mem_empty_fpa(output_pool,
				cvm_oct_num_output_buffers);
	cvm_oct_release_fpa_pool(output_pool);

	cvm_oct_mem_cleanup();

	cvmx_pko_queue_free_all();
	cvmx_pko_internal_ports_range_free_all();

	__cvmx_export_app_config_cleanup();

	return 0;
}

static void cvm_oct_shutdown(struct platform_device *pdev)
{
	cvm_oct_remove(pdev);
}

static struct of_device_id cvm_oct_match[] = {
	{
		.compatible = "cavium,octeon-3860-pip",
	},
	{},
};
MODULE_DEVICE_TABLE(of, cvm_oct_match);

static struct platform_driver cvm_oct_driver = {
	.probe		= cvm_oct_probe,
	.remove		= cvm_oct_remove,
	.shutdown       = cvm_oct_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= KBUILD_MODNAME,
		.of_match_table = cvm_oct_match,
	},
};

module_platform_driver(cvm_oct_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks Octeon ethernet driver.");
