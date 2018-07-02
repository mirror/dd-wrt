/*
 * drivers/net/fec_1588.c
 *
 * Copyright (C) 2011-2012 Freescale Semiconductor, Inc.
 * Copyright (C) 2009 IXXAT Automation, GmbH
 *
 * FEC Ethernet Driver -- IEEE 1588 interface functionality
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <linux/io.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "fec.h"
#include "fec_1588.h"

#if defined(CONFIG_ARCH_MX28)
static struct fec_ptp_private *ptp_private[2];
#elif defined(CONFIG_ARCH_MX6)
static struct fec_ptp_private *ptp_private[1];
#endif

/* Alloc the ring resource */
static int fec_ptp_init_circ(struct fec_ptp_circular *buf, int size)
{
	buf->data_buf = (struct fec_ptp_ts_data *)
		vmalloc(size * sizeof(struct fec_ptp_ts_data));

	if (!buf->data_buf)
		return 1;
	buf->front = 0;
	buf->end = 0;
	buf->size = size;
	return 0;
}

static inline int fec_ptp_calc_index(int size, int curr_index, int offset)
{
	return (curr_index + offset) % size;
}

static int fec_ptp_is_empty(struct fec_ptp_circular *buf)
{
	return (buf->front == buf->end);
}

static int fec_ptp_nelems(struct fec_ptp_circular *buf)
{
	const int front = buf->front;
	const int end = buf->end;
	const int size = buf->size;
	int n_items;

	if (end > front)
		n_items = end - front;
	else if (end < front)
		n_items = size - (front - end);
	else
		n_items = 0;

	return n_items;
}

static int fec_ptp_is_full(struct fec_ptp_circular *buf)
{
	if (fec_ptp_nelems(buf) == (buf->size - 1))
		return 1;
	else
		return 0;
}

static int fec_ptp_insert(struct fec_ptp_circular *ptp_buf,
			  struct fec_ptp_ts_data *data)

{
	struct fec_ptp_ts_data *tmp;

	if (fec_ptp_is_full(ptp_buf))
		ptp_buf->end = fec_ptp_calc_index(ptp_buf->size,
						ptp_buf->end, 1);

	tmp = (ptp_buf->data_buf + ptp_buf->end);
	memcpy(tmp, data, sizeof(struct fec_ptp_ts_data));
	ptp_buf->end = fec_ptp_calc_index(ptp_buf->size, ptp_buf->end, 1);

	return 0;
}

static int fec_ptp_find_and_remove(struct fec_ptp_circular *ptp_buf,
			struct fec_ptp_ident *ident, struct ptp_time *ts)
{
	int i;
	int size = ptp_buf->size, end = ptp_buf->end;
	struct fec_ptp_ident *tmp_ident;

	if (fec_ptp_is_empty(ptp_buf))
		return 1;

	i = ptp_buf->front;
	while (i != end) {
		tmp_ident = &(ptp_buf->data_buf + i)->ident;
		if (tmp_ident->version == ident->version) {
			if (tmp_ident->message_type == ident->message_type) {
				if ((tmp_ident->netw_prot == ident->netw_prot)
				|| (ident->netw_prot ==
					FEC_PTP_PROT_DONTCARE)) {
					if (tmp_ident->seq_id ==
							ident->seq_id) {
						int ret =
						memcmp(tmp_ident->spid,
							ident->spid,
							PTP_SOURCE_PORT_LENGTH);
						if (0 == ret)
							break;
					}
				}
			}
		}
		/* get next */
		i = fec_ptp_calc_index(size, i, 1);
	}

	/* not found ? */
	if (i == end) {
		/* buffer full ? */
		if (fec_ptp_is_full(ptp_buf))
			/* drop one in front */
			ptp_buf->front =
			fec_ptp_calc_index(size, ptp_buf->front, 1);

		return 1;
	}
	*ts = (ptp_buf->data_buf + i)->ts;

	return 0;
}

/* 1588 Module intialization */
int fec_ptp_start(struct fec_ptp_private *priv)
{
	struct fec_ptp_private *fpp = priv;

	/* Select 1588 Timer source and enable module for starting Tmr Clock *
	 * When enable both FEC0 and FEC1 1588 Timer in the same time,       *
	 * enable FEC1 timer's slave mode. */
	if ((fpp == ptp_private[0]) || !(ptp_private[0]->ptp_active)) {
		writel(FEC_T_CTRL_RESTART, fpp->hwp + FEC_ATIME_CTRL);
		writel(FEC_T_INC_CLK << FEC_T_INC_OFFSET,
				fpp->hwp + FEC_ATIME_INC);
		writel(FEC_T_PERIOD_ONE_SEC, fpp->hwp + FEC_ATIME_EVT_PERIOD);
		/* start counter */
		writel(FEC_T_CTRL_PERIOD_RST | FEC_T_CTRL_ENABLE,
				fpp->hwp + FEC_ATIME_CTRL);
		fpp->ptp_slave = 0;
		fpp->ptp_active = 1;

		#if defined(CONFIG_ARCH_MX28)
		/* if the FEC1 timer was enabled, set it to slave mode */
		if ((fpp == ptp_private[0]) && (ptp_private[1]->ptp_active)) {
			writel(0, ptp_private[1]->hwp + FEC_ATIME_CTRL);
			fpp->prtc = ptp_private[1]->prtc;
			writel(FEC_T_CTRL_RESTART,
					ptp_private[1]->hwp + FEC_ATIME_CTRL);
			writel(FEC_T_INC_CLK << FEC_T_INC_OFFSET,
					ptp_private[1]->hwp + FEC_ATIME_INC);
			/* Set the timer as slave mode */
			writel(FEC_T_CTRL_SLAVE,
					ptp_private[1]->hwp + FEC_ATIME_CTRL);
			ptp_private[1]->ptp_slave = 1;
			ptp_private[1]->ptp_active = 1;
		}
		#endif
	} else {
		writel(FEC_T_INC_CLK << FEC_T_INC_OFFSET,
				fpp->hwp + FEC_ATIME_INC);
		/* Set the timer as slave mode */
		writel(FEC_T_CTRL_SLAVE, fpp->hwp + FEC_ATIME_CTRL);
		fpp->ptp_slave = 1;
		fpp->ptp_active = 1;
	}

	return 0;
}

/* Cleanup routine for 1588 module.
 * When PTP is disabled this routing is called */
void fec_ptp_stop(struct fec_ptp_private *priv)
{
	struct fec_ptp_private *fpp = priv;

	writel(0, fpp->hwp + FEC_ATIME_CTRL);
	writel(FEC_T_CTRL_RESTART, fpp->hwp + FEC_ATIME_CTRL);
	priv->ptp_active = 0;
	priv->ptp_slave = 0;
}

static void fec_get_curr_cnt(struct fec_ptp_private *priv,
			struct ptp_rtc_time *curr_time)
{
	u32 tempval;
	struct fec_ptp_private *tmp_priv;

	if (!priv->ptp_slave)
		tmp_priv = priv;
	else
		tmp_priv = ptp_private[0];

	tempval = readl(priv->hwp + FEC_ATIME_CTRL);
	tempval |= FEC_T_CTRL_CAPTURE;

	writel(tempval, priv->hwp + FEC_ATIME_CTRL);
	writel(tempval, priv->hwp + FEC_ATIME_CTRL);
	curr_time->rtc_time.nsec = readl(priv->hwp + FEC_ATIME);
	curr_time->rtc_time.sec = tmp_priv->prtc;

	writel(tempval, priv->hwp + FEC_ATIME_CTRL);
	tempval = readl(priv->hwp + FEC_ATIME);

	if (tempval < curr_time->rtc_time.nsec) {
		curr_time->rtc_time.nsec = tempval;
		curr_time->rtc_time.sec = tmp_priv->prtc;
	}
}

/* Set the 1588 timer counter registers */
static void fec_set_1588cnt(struct fec_ptp_private *priv,
			struct ptp_rtc_time *fec_time)
{
	u32 tempval;
	unsigned long flags;
	struct fec_ptp_private *tmp_priv;

	if (!priv->ptp_slave)
		tmp_priv = priv;
	else
		tmp_priv = ptp_private[0];

	spin_lock_irqsave(&priv->cnt_lock, flags);
	tmp_priv->prtc = fec_time->rtc_time.sec;

	tempval = fec_time->rtc_time.nsec;
	writel(tempval, tmp_priv->hwp + FEC_ATIME);
	spin_unlock_irqrestore(&priv->cnt_lock, flags);
}

/**
 * Parse packets if they are PTP.
 * The PTP header can be found in an IPv4, IPv6 or in an IEEE802.3
 * ethernet frame. The function returns the position of the PTP packet
 * or NULL, if no PTP found
 */
u8 *fec_ptp_parse_packet(struct sk_buff *skb, u16 *eth_type)
{
	u8 *position = skb->data + ETH_ALEN + ETH_ALEN;
	u8 *ptp_loc = NULL;

	*eth_type = *((u16 *)position);
	/* Check if outer vlan tag is here */
	if (*eth_type == ETH_P_8021Q) {
		position += FEC_VLAN_TAG_LEN;
		*eth_type = *((u16 *)position);
	}

	/* set position after ethertype */
	position += FEC_ETHTYPE_LEN;
	if (ETH_P_1588 == *eth_type) {
		ptp_loc = position;
		/* IEEE1588 event message which needs timestamping */
		if ((ptp_loc[0] & 0xF) <= 3) {
			if (skb->len >=
			((ptp_loc - skb->data) + PTP_HEADER_SZE))
				return ptp_loc;
		}
	} else if (ETH_P_IP == ntohs(*eth_type)) {
		u8 *ip_header, *prot, *udp_header;
		u8 ip_version, ip_hlen;
		ip_header = position;
		ip_version = ip_header[0] >> 4; /* correct IP version? */
		if (0x04 == ip_version) { /* IPv4 */
			prot = ip_header + 9; /* protocol */
			if (FEC_PACKET_TYPE_UDP == *prot) {
				u16 udp_dstPort;
				/* retrieve the size of the ip-header
				 * with the first byte of the ip-header:
				 * version ( 4 bits) + Internet header
				 * length (4 bits)
				 */
				ip_hlen   = (*ip_header & 0xf) * 4;
				udp_header = ip_header + ip_hlen;
				udp_dstPort = *((u16 *)(udp_header + 2));
				/* check the destination port address
				 * ( 319 (0x013F) = PTP event port )
				 */
				if (ntohs(udp_dstPort) == PTP_EVENT_PORT) {
					ptp_loc = udp_header + 8;
					/* long enough ? */
					if (skb->len >= ((ptp_loc - skb->data)
							+ PTP_HEADER_SZE))
						return ptp_loc;
				}
			}
		}
	} else if (ETH_P_IPV6 == ntohs(*eth_type)) {
		u8 *ip_header, *udp_header, *prot;
		u8 ip_version;
		ip_header = position;
		ip_version = ip_header[0] >> 4;
		if (0x06 == ip_version) {
			prot = ip_header + 6;
			if (FEC_PACKET_TYPE_UDP == *prot) {
				u16 udp_dstPort;
				udp_header = ip_header + 40;
				udp_dstPort = *((u16 *)(udp_header + 2));
				/* check the destination port address
				 * ( 319 (0x013F) = PTP event port )
				 */
				if (ntohs(udp_dstPort) == PTP_EVENT_PORT) {
					ptp_loc = udp_header + 8;
					/* long enough ? */
					if (skb->len >= ((ptp_loc - skb->data)
							+ PTP_HEADER_SZE))
						return ptp_loc;
				}
			}
		}
	}

	return NULL; /* no PTP frame */
}

/* Set the BD to ptp */
int fec_ptp_do_txstamp(struct sk_buff *skb)
{
	u8 *ptp_loc;
	u16 eth_type;

	ptp_loc = fec_ptp_parse_packet(skb, &eth_type);
	if (ptp_loc != NULL)
		return 1;

	return 0;
}

void fec_ptp_store_txstamp(struct fec_ptp_private *priv,
			   struct sk_buff *skb,
			   struct bufdesc *bdp)
{
	struct fec_ptp_ts_data tmp_tx_time;
	struct fec_ptp_private *fpp;
	u8 *ptp_loc;
	u16 eth_type;

	if (!priv->ptp_slave)
		fpp = priv;
	else
		fpp = ptp_private[0];

	ptp_loc = fec_ptp_parse_packet(skb, &eth_type);
	if (ptp_loc != NULL) {
		/* store identification data */
		switch (ntohs(eth_type)) {
		case ETH_P_IP:
			tmp_tx_time.ident.netw_prot = FEC_PTP_PROT_IPV4;
			break;
		case ETH_P_IPV6:
			tmp_tx_time.ident.netw_prot = FEC_PTP_PROT_IPV6;
			break;
		case ETH_P_1588:
			tmp_tx_time.ident.netw_prot = FEC_PTP_PROT_802_3;
			break;
		default:
			return;
		}
		tmp_tx_time.ident.version = (*(ptp_loc + 1)) & 0X0F;
		tmp_tx_time.ident.message_type = (*(ptp_loc)) & 0x0F;
		tmp_tx_time.ident.seq_id =
			ntohs(*((u16 *)(ptp_loc + PTP_HEADER_SEQ_OFFS)));
		memcpy(tmp_tx_time.ident.spid, &ptp_loc[PTP_SPID_OFFS],
						PTP_SOURCE_PORT_LENGTH);
		/* store tx timestamp */
		tmp_tx_time.ts.sec = fpp->prtc;
		tmp_tx_time.ts.nsec = bdp->ts;
		/* insert timestamp in circular buffer */
		fec_ptp_insert(&(priv->tx_timestamps), &tmp_tx_time);
	}
}

void fec_ptp_store_rxstamp(struct fec_ptp_private *priv,
			   struct sk_buff *skb,
			   struct bufdesc *bdp)
{
	struct fec_ptp_ts_data tmp_rx_time;
	struct fec_ptp_private *fpp;
	u8 *ptp_loc;
	u16 eth_type;

	if (!priv->ptp_slave)
		fpp = priv;
	else
		fpp = ptp_private[0];

	ptp_loc = fec_ptp_parse_packet(skb, &eth_type);
	if (ptp_loc != NULL) {
		/* store identification data */
		tmp_rx_time.ident.version = (*(ptp_loc + 1)) & 0X0F;
		tmp_rx_time.ident.message_type = (*(ptp_loc)) & 0x0F;
		switch (ntohs(eth_type)) {
		case ETH_P_IP:
			tmp_rx_time.ident.netw_prot = FEC_PTP_PROT_IPV4;
			break;
		case ETH_P_IPV6:
			tmp_rx_time.ident.netw_prot = FEC_PTP_PROT_IPV6;
			break;
		case ETH_P_1588:
			tmp_rx_time.ident.netw_prot = FEC_PTP_PROT_802_3;
			break;
		default:
			return;
		}
		tmp_rx_time.ident.seq_id =
			ntohs(*((u16 *)(ptp_loc + PTP_HEADER_SEQ_OFFS)));
		memcpy(tmp_rx_time.ident.spid, &ptp_loc[PTP_SPID_OFFS],
						PTP_SOURCE_PORT_LENGTH);
		/* store rx timestamp */
		tmp_rx_time.ts.sec = fpp->prtc;
		tmp_rx_time.ts.nsec = bdp->ts;

		/* insert timestamp in circular buffer */
		fec_ptp_insert(&(fpp->rx_timestamps), &tmp_rx_time);
	}
}

static uint8_t fec_get_tx_timestamp(struct fec_ptp_private *priv,
				    struct fec_ptp_ts_data *pts,
				    struct ptp_time *tx_time)
{
	int ret = 0;

	ret = fec_ptp_find_and_remove(&(priv->tx_timestamps),
					&pts->ident, tx_time);

	return ret;
}

static uint8_t fec_get_rx_timestamp(struct fec_ptp_private *priv,
				    struct fec_ptp_ts_data *pts,
				    struct ptp_time *rx_time)
{
	int ret = 0;

	ret = fec_ptp_find_and_remove(&(priv->rx_timestamps),
					&pts->ident, rx_time);

	return ret;
}

static void fec_handle_ptpdrift(struct ptp_set_comp *comp,
				struct ptp_time_correct *ptc)
{
	u32 ndrift;
	u32 i, adj_inc, adj_period;
	u32 tmp_current, tmp_winner;

	ndrift = comp->drift;

	if (ndrift == 0) {
		ptc->corr_inc = 0;
		ptc->corr_period = 0;
		return;
	} else if (ndrift >= FEC_ATIME_CLK) {
		ptc->corr_inc = (u32)(ndrift / FEC_ATIME_CLK);
		ptc->corr_period = 1;
		return;
	} else {
		tmp_winner = 0xFFFFFFFF;
		adj_inc = 1;

		if (ndrift > (FEC_ATIME_CLK / FEC_T_INC_CLK)) {
			adj_inc = FEC_T_INC_CLK / FEC_PTP_SPINNER_2;
		} else if (ndrift > (FEC_ATIME_CLK /
			(FEC_T_INC_CLK * FEC_PTP_SPINNER_4))) {
			adj_inc = FEC_T_INC_CLK / FEC_PTP_SPINNER_4;
			adj_period = FEC_PTP_SPINNER_2;
		} else {
			adj_inc = FEC_PTP_SPINNER_4;
			adj_period = FEC_PTP_SPINNER_4;
		}

		for (i = 1; i < adj_inc; i++) {
			tmp_current = (FEC_ATIME_CLK * i) % ndrift;
			if (tmp_current == 0) {
				ptc->corr_inc = i;
				ptc->corr_period = (u32)((FEC_ATIME_CLK *
						adj_period * i)	/ ndrift);
				break;
			} else if (tmp_current < tmp_winner) {
				ptc->corr_inc = i;
				ptc->corr_period = (u32)((FEC_ATIME_CLK *
						adj_period * i)	/ ndrift);
				tmp_winner = tmp_current;
			}
		}
	}
}

static void fec_set_drift(struct fec_ptp_private *priv,
			  struct ptp_set_comp *comp)
{
	struct ptp_time_correct	tc;
	struct fec_ptp_private *fpp;
	u32 tmp, corr_ns;

	memset(&tc, 0, sizeof(struct ptp_time_correct));
	fec_handle_ptpdrift(comp, &tc);
	if (tc.corr_inc == 0)
		return;

	if (comp->o_ops == TRUE)
		corr_ns = FEC_T_INC_CLK + tc.corr_inc;
	else
		corr_ns = FEC_T_INC_CLK - tc.corr_inc;

	if (!priv->ptp_slave)
		fpp = priv;
	else
		fpp = ptp_private[0];

	tmp = readl(fpp->hwp + FEC_ATIME_INC) & FEC_T_INC_MASK;
	tmp |= corr_ns << FEC_T_INC_CORR_OFFSET;
	writel(tmp, fpp->hwp + FEC_ATIME_INC);
	writel(tc.corr_period, fpp->hwp + FEC_ATIME_CORR);
}

int fec_ptp_ioctl(struct fec_ptp_private *priv, struct ifreq *ifr, int cmd)
{
	struct ptp_rtc_time curr_time;
	struct ptp_time rx_time, tx_time;
	struct fec_ptp_ts_data p_ts;
	struct fec_ptp_ts_data *p_ts_user;
	struct ptp_set_comp p_comp;
	u32 freq_compensation;
	int retval = 0;

	switch (cmd) {
	case PTP_ENBL_TXTS_IOCTL:
	case PTP_DSBL_TXTS_IOCTL:
	case PTP_ENBL_RXTS_IOCTL:
	case PTP_DSBL_RXTS_IOCTL:
		break;
	case PTP_GET_RX_TIMESTAMP:
		p_ts_user = (struct fec_ptp_ts_data *)ifr->ifr_data;
		if (0 != copy_from_user(&p_ts.ident,
			&p_ts_user->ident, sizeof(p_ts.ident)))
			return -EINVAL;
		if (fec_get_rx_timestamp(priv, &p_ts, &rx_time) != 0)
			return -EAGAIN;
		if (copy_to_user((void __user *)(&p_ts_user->ts),
			&rx_time, sizeof(rx_time)))
			return -EFAULT;
		break;
	case PTP_GET_TX_TIMESTAMP:
		p_ts_user = (struct fec_ptp_ts_data *)ifr->ifr_data;
		if (0 != copy_from_user(&p_ts.ident,
			&p_ts_user->ident, sizeof(p_ts.ident)))
			return -EINVAL;
		retval = fec_get_tx_timestamp(priv, &p_ts, &tx_time);
		if (retval == 0 &&
			copy_to_user((void __user *)(&p_ts_user->ts),
				&tx_time, sizeof(tx_time)))
			retval = -EFAULT;
		break;
	case PTP_GET_CURRENT_TIME:
		fec_get_curr_cnt(priv, &curr_time);
		if (0 != copy_to_user(ifr->ifr_data,
					&(curr_time.rtc_time),
					sizeof(struct ptp_time)))
			return -EFAULT;
		break;
	case PTP_SET_RTC_TIME:
		if (0 != copy_from_user(&(curr_time.rtc_time),
					ifr->ifr_data,
					sizeof(struct ptp_time)))
			return -EINVAL;
		fec_set_1588cnt(priv, &curr_time);
		break;
	case PTP_FLUSH_TIMESTAMP:
		/* reset tx-timestamping buffer */
		priv->tx_timestamps.front = 0;
		priv->tx_timestamps.end = 0;
		priv->tx_timestamps.size = (DEFAULT_PTP_TX_BUF_SZ + 1);
		/* reset rx-timestamping buffer */
		priv->rx_timestamps.front = 0;
		priv->rx_timestamps.end = 0;
		priv->rx_timestamps.size = (DEFAULT_PTP_RX_BUF_SZ + 1);
		break;
	case PTP_SET_COMPENSATION:
		if (0 != copy_from_user(&p_comp, ifr->ifr_data,
			sizeof(struct ptp_set_comp)))
			return -EINVAL;
		fec_set_drift(priv, &p_comp);
		break;
	case PTP_GET_ORIG_COMP:
		freq_compensation = FEC_PTP_ORIG_COMP;
		if (copy_to_user(ifr->ifr_data, &freq_compensation,
					sizeof(freq_compensation)) > 0)
			return -EFAULT;
		break;
	default:
		return -EINVAL;
	}
	return retval;
}

/*
 * Resource required for accessing 1588 Timer Registers.
 */
int fec_ptp_init(struct fec_ptp_private *priv, int id)
{
	/* initialize circular buffer for tx timestamps */
	if (fec_ptp_init_circ(&(priv->tx_timestamps),
			(DEFAULT_PTP_TX_BUF_SZ+1)))
		return 1;
	/* initialize circular buffer for rx timestamps */
	if (fec_ptp_init_circ(&(priv->rx_timestamps),
			(DEFAULT_PTP_RX_BUF_SZ+1)))
		return 1;

	spin_lock_init(&priv->cnt_lock);
	ptp_private[id] = priv;
	priv->dev_id = id;
	return 0;
}
EXPORT_SYMBOL(fec_ptp_init);

void fec_ptp_cleanup(struct fec_ptp_private *priv)
{
	if (priv->tx_timestamps.data_buf)
		vfree(priv->tx_timestamps.data_buf);
	if (priv->rx_timestamps.data_buf)
		vfree(priv->rx_timestamps.data_buf);
}
EXPORT_SYMBOL(fec_ptp_cleanup);
