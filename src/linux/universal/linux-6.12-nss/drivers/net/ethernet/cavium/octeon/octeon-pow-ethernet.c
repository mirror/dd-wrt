/*
 *   Octeon POW Ethernet Driver
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005-2012 Cavium, Inc.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/if_vlan.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-fpa1.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-wqe.h>
#include <asm/octeon/cvmx-pow-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>
#include <asm/octeon/cvmx-ipd.h>

#define VIRTUAL_PORT    63	/* Value to put in work->ipprt */
#define CN78XX_SSO_INTSN_EXE 0x61

#define DEBUGPRINT(format, ...) do {					\
		if (printk_ratelimit())					\
			printk(format, ##__VA_ARGS__);			\
	} while (0)

#define DEV_NAME "octeon-pow-ethernet"

/* Packet pointers come in two flavors depending on whether pki or pid/ipd are
 * used.
 */
union octeon_packet_ptr {
	uint64_t		u64;
	union cvmx_buf_ptr	pip_packet_ptr;
	union cvmx_buf_ptr_pki	pki_packet_ptr;
};

static int receive_group = -1;
module_param(receive_group, int, 0444);
MODULE_PARM_DESC(receive_group,
		 " 0-16 POW group to receive packets from. This must be unique in\n"
		 "\t\tthe system. If you don't specify a value, the core ID will\n"
		 "\t\tbe used.");

static int broadcast_groups;
module_param(broadcast_groups, int, 0644);
MODULE_PARM_DESC(broadcast_groups,
		 " Bitmask of groups to send broadcasts to. This MUST be specified.\n"
		 "\t\tWARNING: Be careful to not send broadcasts to groups that aren't\n"
		 "\t\tread otherwise you may fill the POW and stop receiving packets.\n");



static int ptp_rx_group = -1;
module_param(ptp_rx_group, int, 0444);
MODULE_PARM_DESC(ptp_rx_group,
		 "For the PTP POW device, 0-64 POW group to receive packets from.\n"
		 "\t\tIf you don't specify a value, the 'pow0' device will not be created\n.");

static int ptp_tx_group = -1;
module_param(ptp_tx_group, int, 0444);
MODULE_PARM_DESC(ptp_tx_group,
		 "For the PTP POW device, 0-64 POW group to transmit packets to.\n"
		 "\t\tIf you don't specify a value, the 'pow0' device will not be created\n.");

static int pki_packet_pool = 0;
module_param(pki_packet_pool, int, 0644);
MODULE_PARM_DESC(pki_packet_pool,
		 "Pool to use for transmit/receive buffer alloc/frees.\n");

static int reverse_endian;
module_param(reverse_endian, int, 0444);
MODULE_PARM_DESC(reverse_endian,
		 "Link partner is running with different endianness (set on only one end of the link).\n");

static void *memcpy_re_to(void *d, const void *s, size_t n)
{
	u8 *dst = d;
	const u8 *src = s;
	while (n) {
		u8 *pd = (u8 *)((unsigned long)dst ^ 7);
		*pd = *src;
		n--;
		dst++;
		src++;
	}
	return d;
}
static void *memcpy_re_from(void *d, const void *s, size_t n)
{
	u8 *dst = d;
	const u8 *src = s;
	while (n) {
		u8 *ps = (u8 *)((unsigned long)src ^ 7);
		*dst = *ps;
		n--;
		dst++;
		src++;
	}
	return d;
}
static void * (*octeon_pow_copy_to)(void *d, const void *s, size_t n);
static void * (*octeon_pow_copy_from)(void *d, const void *s, size_t n);

/*
 * This is the definition of the Ethernet driver's private
 * driver state.
 */
struct octeon_pow {
	u64 tx_mask;
	int rx_group;
	bool is_ptp;
	int rx_irq;
	int numa_node;
	struct net_device *netdev;
	struct napi_struct napi;
};

static int fpa_wqe_pool = 1;	/* HW FPA pool to use for work queue entries */
static int fpa_packet_pool;	/* HW FPA pool to use for packet buffers */
static int fpa_packet_pool_size = 2048;	/* Size of the packet buffers */
static struct net_device *octeon_pow_oct_dev;
static struct net_device *octeon_pow_ptp_dev;
static int octeon_pow_num_groups;

void *work_to_skb(void *work)
{
	return work - (octeon_has_feature(OCTEON_FEATURE_PKI) ? 0x80 : 0);
}

/*
 * Given a packet data address, return a pointer to the
 * beginning of the packet buffer.
 */
static void *get_buffer_ptr(union octeon_packet_ptr packet_ptr)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		ulong	buf_ptr;

		buf_ptr = round_down((ulong)packet_ptr.pki_packet_ptr.addr,
				     128ul);
		buf_ptr -= 8 * 16;
		return (void *)buf_ptr;
	} else {
		return phys_to_virt(((packet_ptr.pip_packet_ptr.s.addr >> 7) -
				     packet_ptr.pip_packet_ptr.s.back) << 7);
	}
}

static union octeon_packet_ptr octeon_get_first_packet_ptr(cvmx_wqe_t *work)
{
	union octeon_packet_ptr	packet_ptr;

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		packet_ptr.u64 = cvmx_wqe_get_pki_pkt_ptr(work).u64;
	} else {
		if ((work->word2.s.bufs > 0) || (work->word2.s.software))
			packet_ptr.u64 = work->packet_ptr.u64;
		else
			packet_ptr.u64 = cvmx_ptr_to_phys(work) + 32;
	}

	return packet_ptr;
}

static union octeon_packet_ptr
octeon_get_next_packet_ptr(union octeon_packet_ptr packet_ptr)
{
	uint64_t	addr;

	if (octeon_has_feature(OCTEON_FEATURE_PKI))
		addr = packet_ptr.pki_packet_ptr.addr - 8;
	else
		addr = packet_ptr.pip_packet_ptr.s.addr - 8;

	return *(union octeon_packet_ptr *)phys_to_virt(addr);
}

static int octeon_pow_free_work(cvmx_wqe_t *work)
{
	union octeon_packet_ptr	packet_ptr;
	int			segments;
	void			*buffer_ptr;
	int			aura;

	segments = cvmx_wqe_get_bufs(work);

	packet_ptr = octeon_get_first_packet_ptr(work);
	if (octeon_has_feature(OCTEON_FEATURE_PKI))
		buffer_ptr = (void *)((ulong)work - 128);
	else
		buffer_ptr = get_buffer_ptr(packet_ptr);

	aura = cvmx_wqe_get_aura(work);
	while (segments--) {
		packet_ptr = octeon_get_next_packet_ptr(packet_ptr);
		cvmx_fpa_free(buffer_ptr, aura, 0);
		buffer_ptr = get_buffer_ptr(packet_ptr);
	}

	if (!octeon_has_feature(OCTEON_FEATURE_PKI))
		cvmx_fpa_free(work, fpa_wqe_pool, 0);

	return 0;
}

static int octeon_packet_ptr_init(void *packet_ptr, void *data, int len)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		union cvmx_buf_ptr_pki	*pki_packet_ptr;

		pki_packet_ptr = packet_ptr;
		pki_packet_ptr->u64 = 0;
		pki_packet_ptr->addr = virt_to_phys(data);
		pki_packet_ptr->packet_outside_wqe = 0;
		pki_packet_ptr->size = len;
	} else {
		union cvmx_buf_ptr	*pip_packet_ptr;

		pip_packet_ptr = packet_ptr;
		pip_packet_ptr->u64 = 0;
		pip_packet_ptr->s.addr = virt_to_phys(data);
		pip_packet_ptr->s.pool = fpa_packet_pool;
		pip_packet_ptr->s.size = fpa_packet_pool_size -
			sizeof(pip_packet_ptr);
		pip_packet_ptr->s.back = (data - packet_ptr) >> 7;
	}

	return 0;
}

static cvmx_wqe_t *octeon_skb_to_fpa_buf(struct net_device *dev,
					 struct sk_buff *skb)
{
	cvmx_wqe_t	*work;
	int		len = skb->len;
	int		first_segment;
	int		num_segments;
	int		segment_len;
	void		*packet_ptr;
	void		*prev_packet_ptr = NULL;
	void		*data_ptr = skb->data;

	/* Get a work queue entry */
	work = cvmx_fpa_alloc(fpa_wqe_pool);
	if (unlikely(work == NULL)) {
		DEBUGPRINT("%s: Failed to allocate a work queue entry\n",
			   dev->name);
		return NULL;
	}

	/* The first 128 bytes of each fpa buffer store information used by the
	 * octeon3-ethernet driver that must be preserved.
	 */
	if (octeon_has_feature(OCTEON_FEATURE_FPA3))
		work = (void *)work + 128;

	memset(work, 0, sizeof(*work));

	/* Copy all the skb data to fpa buffers */
	first_segment = 1;
	num_segments = 0;
	cvmx_wqe_set_bufs(work, 0);
	while (len > 0) {
		void	*copy_location;

		/* Get a packet buffer. On octeon III the wqe and packet data
		 * share the same fpa buffer.
		 */
		if (octeon_has_feature(OCTEON_FEATURE_PKI) && first_segment) {
			packet_ptr = (void *)work + sizeof(*work);
			segment_len = fpa_packet_pool_size - 128 -
				sizeof(*work);
		} else {
			packet_ptr = cvmx_fpa_alloc(fpa_packet_pool);
			if (unlikely(packet_ptr == NULL)) {
				DEBUGPRINT("%s: Failed to allocate a fpa buf\n",
					   dev->name);
				goto error;
			}

			segment_len = fpa_packet_pool_size;
			if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
				packet_ptr = (void *)packet_ptr + 128;
				segment_len = fpa_packet_pool_size - 128;
			}
		}

		/* We need to leave 8 bytes for the next pointer */
		copy_location = packet_ptr + sizeof(uint64_t);
		segment_len -= sizeof(uint64_t);
		octeon_packet_ptr_init(packet_ptr, copy_location,
				       segment_len);
		if (segment_len > len)
			segment_len = len;
		octeon_pow_copy_to(copy_location, data_ptr, segment_len);

		if (first_segment) {
			if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
				cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

				wqe->packet_ptr.u64 = *(uint64_t *)packet_ptr;
				wqe->pki_errata20776 = 1;
			} else {
				work->packet_ptr.u64 = *(uint64_t *)packet_ptr;
			}

			first_segment = 0;
		} else
			*(uint64_t *)prev_packet_ptr = *(uint64_t *)packet_ptr;

		num_segments++;
		len -= segment_len;
		data_ptr += segment_len;
		prev_packet_ptr = packet_ptr;
	}

	cvmx_wqe_set_bufs(work, num_segments);

	return work;

 error:
	octeon_pow_free_work(work);
	return NULL;
}

static int octeon_pow_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct octeon_pow *priv;
	cvmx_wqe_t *work = NULL;
	void *packet_buffer = NULL;
	u64 send_group_mask;
	int send_group;

	priv = netdev_priv(dev);

	/* Any unknown MAC address goes to all groups in the module
	 * param broadcast_groups. Known MAC addresses use the low
	 * order dest mac byte as the group number.
	 */
	if (!priv->is_ptp && ((*(uint64_t *) (skb->data) >> 16) < 0x01ff))
		send_group_mask = 1ull << (skb->data[5] & (octeon_pow_num_groups - 1));
	else
		send_group_mask = priv->tx_mask;

	/* Remove self from group mask */
	send_group_mask &= ~(1 << priv->rx_group);

	/* It is ugly, but we need to send multiple times for
	 * broadcast packets. The hardware doesn't support submitting
	 * work to multiple groups
	 */
	for (send_group = 0; send_group < octeon_pow_num_groups; send_group++) {
		/* Don't transmit to groups not in our send_group_mask */
		if (likely((send_group_mask & (1ULL << send_group)) == 0))
			continue;

		work = octeon_skb_to_fpa_buf(dev, skb);
		if (work == NULL)
			goto fail;

		/* Fill in some of the work queue fields. We may need
		 * to add more if the software at the other end needs
		 * them.
		 */
#if 0
		work->hw_chksum = skb->csum;
#endif

		cvmx_wqe_set_len(work, skb->len);
		cvmx_wqe_set_port(work, VIRTUAL_PORT);
		cvmx_wqe_set_qos(work, 0);
		cvmx_wqe_set_grp(work, send_group);
		cvmx_wqe_set_tt(work, 2);
		cvmx_wqe_set_tag(work, 0);
		cvmx_wqe_set_aura(work, fpa_packet_pool);

		if (skb->protocol == htons(ETH_P_8021Q)) {
			struct vlan_hdr *vhdr;

			cvmx_wqe_set_vlan(work, true);
#if 0
			work->word2.s.vlan_cfi = 0;	/* FIXME */
			work->word2.s.vlan_id = 0;	/* FIXME */
			work->word2.s.dec_ipcomp = 0;	/* FIXME */
#endif
			vhdr = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			vlan_set_encap_proto(skb, vhdr);
		}

		if (skb->protocol == htons(ETH_P_IP)) {
			cvmx_wqe_set_l3_ipv4(work, true);
			cvmx_wqe_set_l3_offset(work,
				(uint)(skb_network_header(skb) - skb->data));
			if (ip_hdr(skb)->protocol == IPPROTO_UDP)
				cvmx_wqe_set_l4_udp(work, true);
			else if (ip_hdr(skb)->protocol == IPPROTO_TCP)
				cvmx_wqe_set_l4_tcp(work, true);
			cvmx_wqe_set_l3_frag(work, !((ip_hdr(skb)->frag_off == 0)
						     || (ip_hdr(skb)->frag_off ==
							 1 << 14)));
			cvmx_wqe_set_l2_bcast(work, skb->pkt_type == PACKET_BROADCAST);
			cvmx_wqe_set_l2_mcast(work, skb->pkt_type == PACKET_MULTICAST);

			/* When copying the data, include 4 bytes of the
			   ethernet header to align the same way hardware does */
			octeon_pow_copy_to(work->packet_data, skb->data + 10,
			       sizeof(work->packet_data));
		} else if (skb->protocol == htons(ETH_P_IPV6)) {
			cvmx_wqe_set_l3_ipv6(work, 1);
			cvmx_wqe_set_l3_offset(work,
				(uint)(skb_network_header(skb) - skb->data));
			if (ipv6_hdr(skb)->nexthdr == IPPROTO_UDP)
				cvmx_wqe_set_l4_udp(work, true);
			else if (ipv6_hdr(skb)->nexthdr == IPPROTO_TCP)
				cvmx_wqe_set_l4_tcp(work, true);
		} else if (skb->protocol == htons(ETH_P_ARP))
			cvmx_wqe_set_l3_arp(work, true);
		else {
			if (skb->pkt_type == PACKET_BROADCAST)
				cvmx_wqe_set_l2_bcast(work, true);
			if (skb->pkt_type == PACKET_MULTICAST)
				cvmx_wqe_set_l2_mcast(work, true);
			if (skb->protocol == htons(ETH_P_RARP))
				cvmx_wqe_set_l3_rarp(work, work);

			cvmx_wqe_set_l3_ipv4(work, 0);
			octeon_pow_copy_to(work->packet_data, skb->data,
			       sizeof(work->packet_data));
		}

		/*
		 * Submit the packet to the POW
		 * tag: 0
		 * tag_type: 2
		 * qos: 0
		 * grp: send_group
		 */
		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			cvmx_pow_work_submit_node(work, 0, 2, send_group, priv->numa_node);
		else
			cvmx_pow_work_submit(work, 0, 2, 0, send_group);
		work = NULL;
		packet_buffer = NULL;
	}

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;

fail:
	dev->stats.tx_dropped++;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

/**
 * Process packets received via the pki. It copies the packet data out
 * of the fpa buffers and into the skb.
 *
 * @param: work
 * @skb:
 * @return
 */
static int octeon_pow_pki_rx(cvmx_wqe_t *work, struct sk_buff *skb)
{
	cvmx_buf_ptr_pki_t	pkt_ptr;
	int			segments = cvmx_wqe_get_bufs(work);
	int			len = cvmx_wqe_get_len(work);

	pkt_ptr = cvmx_wqe_get_pki_pkt_ptr(work);
	while (segments--) {
		int segment_size = pkt_ptr.size;

		if (segment_size > len)
			segment_size = len;

		/* Copy the data into the packet */
		octeon_pow_copy_from(skb_put(skb, segment_size),
				     phys_to_virt(pkt_ptr.addr), segment_size);
		/* Reduce the amount of bytes left to copy */
		len -= segment_size;

		pkt_ptr =
			*((cvmx_buf_ptr_pki_t *)phys_to_virt(pkt_ptr.addr - 8));
	}

	return 0;
}

/**
 * Process packets received via the pip/ipd. It copies the packet data out
 * of the fpa buffers and into the skb.
 *
 * @param: work
 * @skb:
 * @return
 */
static int octeon_pow_pip_ipd_rx(cvmx_wqe_t *work, struct sk_buff *skb)
{
	union cvmx_buf_ptr	pkt_ptr;
	int			segments = cvmx_wqe_get_bufs(work);
	int			len = cvmx_wqe_get_len(work);

	pkt_ptr = work->packet_ptr;
	while (segments--) {
		int segment_size;
		/* Octeon PKI-100: The segment size is wrong. Until it is fixed,
		 * calculate the segment size based on the packet pool buffer
		 * size. When it is fixed, the following line should be replaced
		 * with this one: int segment_size = segment_ptr.s.size;
		 */
		segment_size = fpa_packet_pool_size -
			(pkt_ptr.s.addr - (((pkt_ptr.s.addr >> 7) -
					    pkt_ptr.s.back) << 7));
		if (segment_size > len)
			segment_size = len;

		/* Copy the data into the packet */
		octeon_pow_copy_from(skb_put(skb, segment_size),
				     phys_to_virt(pkt_ptr.s.addr),
				     segment_size);

		/* Reduce the amount of bytes left to copy */
		len -= segment_size;

		pkt_ptr =
			*(union cvmx_buf_ptr *)phys_to_virt(pkt_ptr.s.addr - 8);
	}

	return 0;
}

static void octeon_pow_arm_interrupt(struct octeon_pow *priv, bool en)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		union cvmx_sso_grpx_int_thr thr;
		union cvmx_sso_grpx_int grp_int;

		thr.u64 = 0;
		thr.cn78xx.iaq_thr = en ? 1 : 0;
		cvmx_write_csr_node(priv->numa_node,
				    CVMX_SSO_GRPX_INT_THR(priv->rx_group),
				    thr.u64);

		grp_int.u64 = 0;
		grp_int.s.exe_int = 1;
		cvmx_write_csr_node(priv->numa_node,
				    CVMX_SSO_GRPX_INT(priv->rx_group),
				    grp_int.u64);
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_sso_wq_int_thrx thr;

		thr.u64 = 0;
		thr.s.iq_thr = en ? 1 : 0;
		thr.s.ds_thr = en ? 1 : 0;
		cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(priv->rx_group), thr.u64);
		cvmx_write_csr(CVMX_SSO_WQ_INT, 1ull << priv->rx_group);
	} else {
		union cvmx_pow_wq_int_thrx thr;

		thr.u64 = 0;
		thr.s.iq_thr = en ? 1 : 0;
		thr.s.ds_thr = en ? 1 : 0;
		cvmx_write_csr(CVMX_POW_WQ_INT_THRX(priv->rx_group), thr.u64);
		cvmx_write_csr(CVMX_POW_WQ_INT, 1ull << priv->rx_group);
	}
}

/**
 * Interrupt handler. The interrupt occurs whenever the POW
 * transitions from 0->1 packets in our group.
 *
 * @param cpl
 * @param dev_id
 * @param regs
 * @return
 */
static irqreturn_t octeon_pow_interrupt(int cpl, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct octeon_pow *priv;

	priv = netdev_priv(dev);

	/* Disable the rx interrupt and start napi*/
	octeon_pow_arm_interrupt(priv, false);
	napi_schedule(&priv->napi);

	return IRQ_HANDLED;
}

static int octeon_pow_napi_poll(struct napi_struct *napi, int budget)
{
	const uint64_t coreid = cvmx_get_core_num();
	struct net_device *dev;
	struct octeon_pow *priv;
	uint64_t old_group_mask = 0;
	cvmx_wqe_t *work;
	struct sk_buff *skb;
	unsigned long flags = 0;
	int rx_count = 0;

	priv = container_of(napi, struct octeon_pow, napi);
	dev = priv->netdev;

	while (rx_count < budget) {
		/* Non sso3 architectures need to save/restore the sso core
		 * group mask atomically. If not, it is possible the wrong sso
		 * core group mask is restored preventing the core from
		 * receiving work from other groups any longer.
		 */
		if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
			local_irq_save(flags);

			/* Make sure any iobdma operations in progress
			 * complete.
			 */
			asm volatile ("synciobdma" : : : "memory");
		}

		/* Save the sso core group mask and set it for our group */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			old_group_mask =
				cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
			cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid),
				       1ull << priv->rx_group);
			/* Read it back so it takes effect */
			cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
		} else if (!octeon_has_feature(OCTEON_FEATURE_PKI)) {
			old_group_mask =
				cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(coreid));
			cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid),
				       1 << priv->rx_group);
		}

		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			work = cvmx_sso_work_request_grp_sync_nocheck(priv->rx_group,
				CVMX_POW_NO_WAIT);
		else
			work = cvmx_pow_work_request_sync(0);

		/* Restore the original sso core group mask */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid),
				       old_group_mask);
			/* Must read the original pow group mask back so it
			 * takes effect before ??
			 */
			cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
		} else if (!octeon_has_feature(OCTEON_FEATURE_PKI))
			cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid),
				       old_group_mask);

		if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
			local_irq_restore(flags);

		if (work == NULL)
			break;

		/* Silently drop packets if we aren't up */
		if ((dev->flags & IFF_UP) == 0) {
			octeon_pow_free_work(work);
			continue;
		}

		/* Throw away all packets with receive errors */
		if (unlikely(cvmx_wqe_get_rcv_err(work))) {
			DEBUGPRINT("%s: Receive error code %d, packet dropped\n",
				   dev->name, cvmx_wqe_get_rcv_err(work));
			octeon_pow_free_work(work);
			dev->stats.rx_errors++;
			continue;
		}

		/* We have to copy the packet. First allocate an skbuff for it */
		skb = dev_alloc_skb(cvmx_wqe_get_len(work));
		if (!skb) {
			DEBUGPRINT("%s: Failed to allocate skbuff, packet dropped\n",
				   dev->name);
			octeon_pow_free_work(work);
			dev->stats.rx_dropped++;
			continue;
		}

		/* Check if we've received a packet that was entirely
		 * stored the work entry. This is untested
		 */
		if (unlikely(cvmx_wqe_get_bufs(work) == 0)) {
			union octeon_packet_ptr	packet_ptr;
			uint64_t		addr;
			int			len = cvmx_wqe_get_len(work);

			DEBUGPRINT("%s: Received a work with work->word2.s.bufs=0, untested\n",
				   dev->name);
			packet_ptr = octeon_get_first_packet_ptr(work);
			addr = packet_ptr.pip_packet_ptr.s.addr;
			octeon_pow_copy_from(skb_put(skb, len),
					     phys_to_virt(addr), len);
		} else {
			if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
				octeon_pow_pki_rx(work, skb);
			} else {
				octeon_pow_pip_ipd_rx(work, skb);
			}
		}

		octeon_pow_free_work(work);
		skb->protocol = eth_type_trans(skb, dev);
		skb->dev = dev;
		skb->ip_summed = CHECKSUM_NONE;
		dev->stats.rx_bytes += skb->len;
		dev->stats.rx_packets++;
		netif_receive_skb(skb);
		rx_count++;
	}

	/* Stop napi and enable the interrupt when no work is pending */
	if (rx_count < budget) {
		napi_complete(napi);
		octeon_pow_arm_interrupt(priv, true);
	}

	return rx_count;
}

#ifdef CONFIG_NET_POLL_CONTROLLER

static void octeon_pow_poll(struct net_device *dev)
{
	octeon_pow_interrupt(0, dev);
}
#endif

static int octeon_pow_open(struct net_device *dev)
{
	int r;
	struct octeon_pow *priv = netdev_priv(dev);

	/* Clear the statistics whenever the interface is brought up */
	memset(&dev->stats, 0, sizeof(dev->stats));

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		int sso_intsn = (CN78XX_SSO_INTSN_EXE << 12) | priv->rx_group;
		struct irq_domain *d = octeon_irq_get_block_domain(priv->numa_node,
								   CN78XX_SSO_INTSN_EXE);
		priv->rx_irq = irq_create_mapping(d, sso_intsn);
		irqd_set_trigger_type(irq_get_irq_data(priv->rx_irq),
                                      IRQ_TYPE_EDGE_RISING);
		if (!priv->rx_irq) {
			netdev_err(dev, "ERROR: Couldn't map hwirq: %x\n",
				   sso_intsn);
			return -EINVAL;
		}
	} else
		priv->rx_irq = OCTEON_IRQ_WORKQ0 + priv->rx_group;
	/* Register an IRQ hander for to receive POW interrupts */
	r = request_irq(priv->rx_irq, octeon_pow_interrupt, 0, dev->name, dev);
	if (r)
		return r;

	/* Enable POW interrupt when our port has at least one packet */
	octeon_pow_arm_interrupt(priv, true);

	return 0;
}

static int octeon_pow_stop(struct net_device *dev)
{
	struct octeon_pow *priv = netdev_priv(dev);

	/* Disable POW interrupt */
	octeon_pow_arm_interrupt(priv, false);

	/* Free the interrupt handler */
	free_irq(priv->rx_irq, dev);
	return 0;
}

/**
 * Per network device initialization
 *
 * @param dev    Device to initialize
 * @return Zero on success
 */
static int octeon_pow_init(struct net_device *dev)
{
	struct octeon_pow *priv = netdev_priv(dev);

	dev->features |= NETIF_F_LLTX;	/* We do our own locking, Linux doesn't
					   need to */
	dev->dev_addr[0] = 0;
	dev->dev_addr[1] = 0;
	dev->dev_addr[2] = 0;
	dev->dev_addr[3] = 0;
	dev->dev_addr[4] = priv->is_ptp ? 3 : 1;
	dev->dev_addr[5] = priv->rx_group;
	priv->numa_node = cvmx_get_node_num();

	/* Initialize and enable napi */
	netif_napi_add_weight(dev, &priv->napi, octeon_pow_napi_poll, 32);
	napi_enable(&priv->napi);

	return 0;
}

static const struct net_device_ops octeon_pow_netdev_ops = {
	.ndo_init = octeon_pow_init,
	.ndo_open = octeon_pow_open,
	.ndo_stop = octeon_pow_stop,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_start_xmit = octeon_pow_xmit,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller =  octeon_pow_poll,
#endif
};


/*
 * Module/ driver initialization. Creates the linux network
 * devices.
 *
 * @return Zero on success
 */
static int __init octeon_pow_mod_init(void)
{
	struct octeon_pow *priv;
	u64 allowed_group_mask;

	if (reverse_endian) {
		octeon_pow_copy_to = memcpy_re_to;
		octeon_pow_copy_from = memcpy_re_from;
	} else {
		octeon_pow_copy_to = memcpy;
		octeon_pow_copy_from = memcpy;
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Actually 256 groups total, only 64 currently supported */
		octeon_pow_num_groups = 64;
		allowed_group_mask = 0xffffffffffffffffull;
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		octeon_pow_num_groups = 64;
		allowed_group_mask = 0xffffffffffffffffull;
	} else {
		octeon_pow_num_groups = 16;
		allowed_group_mask = 0xffffull;
	}

	/* If a receive group isn't specified, default to the core id */
	if (receive_group < 0)
		receive_group = cvmx_get_core_num();


	if ((receive_group > octeon_pow_num_groups)) {
		pr_err(DEV_NAME " ERROR: Invalid receive group. Must be 0-%d\n",
		       octeon_pow_num_groups - 1);
		return -1;
	}

	if (!broadcast_groups) {
		pr_err(DEV_NAME " ERROR: You must specify a broadcast group mask.\n");
		return -1;
	}

	if ((broadcast_groups & allowed_group_mask) != broadcast_groups) {
		pr_err(DEV_NAME " ERROR: Invalid broadcast group mask.\n");
		return -1;
	}

	if ((ptp_rx_group >= 0 && ptp_tx_group < 0) || (ptp_rx_group < 0 && ptp_tx_group >= 0)) {
		pr_err(DEV_NAME " ERROR: Both ptp_rx_group AND ptp_tx_group must be set.\n");
		return -1;
	}

	if (ptp_rx_group >= 0 && ptp_tx_group == ptp_rx_group) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group and ptp_tx_group must differ.\n");
		return -1;
	}

	if (ptp_rx_group >= octeon_pow_num_groups || ptp_tx_group >= octeon_pow_num_groups) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group and ptp_tx_group. Must be 0-%d\n",
		       octeon_pow_num_groups - 1);
		return -1;
	}

	if (receive_group == ptp_rx_group) {
		pr_err(DEV_NAME " ERROR: ptp_rx_group(%d) and  receive_group(%d) must differ.\n",
			ptp_rx_group, receive_group);
		return -1;
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI) && (pki_packet_pool == 0)) {
		pr_err(DEV_NAME " ERROR: pki_packet_pool must be specified for CN78XX.\n");
	}

	pr_info("Octeon POW only ethernet driver\n");

	/* Setup is complete, create the virtual ethernet devices */
	octeon_pow_oct_dev = alloc_etherdev(sizeof(struct octeon_pow));
	if (octeon_pow_oct_dev == NULL) {
		pr_err(DEV_NAME " ERROR: Failed to allocate ethernet device\n");
		return -1;
	}

	octeon_pow_oct_dev->netdev_ops = &octeon_pow_netdev_ops;
	strcpy(octeon_pow_oct_dev->name, "oct%d");

	/* Initialize the device private structure. */
	priv = netdev_priv(octeon_pow_oct_dev);
	priv->netdev = octeon_pow_oct_dev;
	priv->rx_group = receive_group;
	priv->tx_mask = broadcast_groups;
	priv->numa_node = cvmx_get_node_num();


	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* Spin waiting for another core to setup all the hardware */
		printk("Waiting for another core to setup the PKI hardware...");
		while ((cvmx_read_csr_node(priv->numa_node, CVMX_PKI_BUF_CTL) & 1) == 0)
			mdelay(100);

		printk("Done\n");
		fpa_packet_pool_size = 2048;
		fpa_packet_pool = fpa_wqe_pool = pki_packet_pool;

		if (fpa_packet_pool < 0) {
			netdev_err(octeon_pow_oct_dev, "ERROR: Failed to initialize fpa pool\n");
			goto err1;
		}
	} else {
		/* Spin waiting for another core to setup all the hardware */
		printk("Waiting for another core to setup the IPD hardware...");
		while ((cvmx_read_csr(CVMX_IPD_CTL_STATUS) & 1) == 0)
			mdelay(100);

		printk("Done\n");
		/* Read the configured size of the FPA packet buffers. This
		 * way we don't need changes if someone chooses to use a
		 * different buffer size
		 */
		fpa_packet_pool = cvmx_fpa_get_packet_pool();
		fpa_packet_pool_size = (cvmx_read_csr(CVMX_IPD_PACKET_MBUFF_SIZE) & 0xfff) * 8;

		/* Read the work queue pool */
		fpa_wqe_pool = cvmx_read_csr(CVMX_IPD_WQE_FPA_QUEUE) & 7;
	}

	if (register_netdev(octeon_pow_oct_dev) < 0) {
		netdev_err(octeon_pow_oct_dev, "ERROR: Failed to register ethernet device\n");
		goto err1;
	}

	if (ptp_rx_group < 0)
		return 0;

	/* Else create a ptp device. */
	octeon_pow_ptp_dev = alloc_etherdev(sizeof(struct octeon_pow));
	if (octeon_pow_ptp_dev == NULL) {
		pr_err(DEV_NAME " ERROR: Failed to allocate ethernet device\n");
		goto err1;
	}

	octeon_pow_ptp_dev->netdev_ops = &octeon_pow_netdev_ops;
	strcpy(octeon_pow_ptp_dev->name, "pow%d");

	/* Initialize the device private structure. */
	priv = netdev_priv(octeon_pow_ptp_dev);
	priv->netdev = octeon_pow_ptp_dev;
	priv->rx_group = ptp_rx_group;
	priv->tx_mask = 1ull << ptp_tx_group;
	priv->is_ptp = true;

	if (register_netdev(octeon_pow_ptp_dev) < 0) {
		netdev_err(octeon_pow_ptp_dev, "ERROR: Failed to register ethernet device\n");
		goto err;
	}

	return 0;

 err:
	free_netdev(octeon_pow_ptp_dev);
 err1:
	free_netdev(octeon_pow_oct_dev);
	return -1;
}

/**
 * Module / driver shutdown
 *
 * @return Zero on success
 */
static void __exit octeon_pow_mod_exit(void)
{
	/* Free the ethernet devices */
	unregister_netdev(octeon_pow_oct_dev);
	free_netdev(octeon_pow_oct_dev);
	if (octeon_pow_ptp_dev) {
		unregister_netdev(octeon_pow_ptp_dev);
		free_netdev(octeon_pow_ptp_dev);
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Inc. <support@cavium.com>");
MODULE_DESCRIPTION("Cavium Inc. OCTEON internal only POW ethernet driver.");
module_init(octeon_pow_mod_init);
module_exit(octeon_pow_mod_exit);
