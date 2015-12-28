/*
 * drivers/net/fec_1588.h
 *
 * Copyright (C) 2011-2012 Freescale Semiconductor, Inc.
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

#ifndef FEC_1588_H
#define FEC_1588_H

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/circ_buf.h>

#define FALSE			0
#define TRUE			1

/* FEC 1588 register bits */
#define FEC_T_CTRL_SLAVE		0x00002000
#define FEC_T_CTRL_CAPTURE		0x00000800
#define FEC_T_CTRL_RESTART		0x00000200
#define FEC_T_CTRL_PERIOD_RST		0x00000030
#define FEC_T_CTRL_ENABLE		0x00000001

#define FEC_T_INC_MASK			0x0000007f
#define FEC_T_INC_OFFSET		0
#define FEC_T_INC_CORR_MASK		0x00007f00
#define FEC_T_INC_CORR_OFFSET		8

#define FEC_T_INC_50MHZ			20
#define FEC_ATIME_50MHZ			50000000
#define FEC_T_INC_CLK			FEC_T_INC_50MHZ
#define FEC_ATIME_CLK			FEC_ATIME_50MHZ

#define FEC_T_PERIOD_ONE_SEC		0x3B9ACA00

/* IEEE 1588 definition */
#define FEC_ECNTRL_TS_EN	0x10

#define DEFAULT_PTP_RX_BUF_SZ		64
#define DEFAULT_PTP_TX_BUF_SZ		64

/* 1588stack API defines */
#define PTP_ENBL_TXTS_IOCTL	SIOCDEVPRIVATE
#define PTP_DSBL_TXTS_IOCTL	(SIOCDEVPRIVATE + 1)
#define PTP_ENBL_RXTS_IOCTL	(SIOCDEVPRIVATE + 2)
#define PTP_DSBL_RXTS_IOCTL	(SIOCDEVPRIVATE + 3)
#define PTP_GET_TX_TIMESTAMP	(SIOCDEVPRIVATE + 4)
#define PTP_GET_RX_TIMESTAMP	(SIOCDEVPRIVATE + 5)
#define PTP_SET_RTC_TIME	(SIOCDEVPRIVATE + 6)
#define PTP_GET_CURRENT_TIME	(SIOCDEVPRIVATE + 7)
#define PTP_SET_COMPENSATION	(SIOCDEVPRIVATE + 9)
#define PTP_GET_ORIG_COMP	(SIOCDEVPRIVATE + 10)
#define PTP_FLUSH_TIMESTAMP	(SIOCDEVPRIVATE + 11)

/* IEEE1588 ptp head format */
#define PTP_CTRL_OFFS		0x52
#define PTP_SOURCE_PORT_LENGTH	10
#define	PTP_HEADER_SEQ_OFFS	30
#define PTP_HEADER_CTL_OFFS	32
#define PTP_SPID_OFFS		20
#define PTP_HEADER_SZE		34
#define PTP_EVENT_PORT		0x013F

#define FEC_VLAN_TAG_LEN	0x04
#define FEC_ETHTYPE_LEN		0x02

/* 1588-2008 network protocol enumeration values */
#define FEC_PTP_PROT_IPV4		1
#define FEC_PTP_PROT_IPV6		2
#define FEC_PTP_PROT_802_3		3
#define FEC_PTP_PROT_DONTCARE		0xFFFF
#define FEC_PACKET_TYPE_UDP		0x11

#define FEC_PTP_ORIG_COMP		0x15555555
#define FEC_PTP_SPINNER_2		2
#define FEC_PTP_SPINNER_4		4

/* PTP standard time representation structure */
struct ptp_time{
	u64 sec;	/* seconds */
	u32 nsec;	/* nanoseconds */
};

/* struct needed to identify a timestamp */
struct fec_ptp_ident {
	u8	version;
	u8	message_type;
	u16	netw_prot;
	u16	seq_id;
	u8	spid[10];
};

/* interface for PTP driver command GET_TX_TIME */
struct fec_ptp_ts_data {
	struct fec_ptp_ident ident;
	/* PTP timestamp */
	struct ptp_time ts;
};

/* circular buffer for ptp timestamps over ioctl */
struct fec_ptp_circular {
	int	front;
	int	end;
	int	size;
	struct	fec_ptp_ts_data	*data_buf;
};

/* interface for PTP driver command SET_RTC_TIME/GET_CURRENT_TIME */
struct ptp_rtc_time {
	struct ptp_time rtc_time;
};

/* interface for PTP driver command SET_COMPENSATION */
struct ptp_set_comp {
	u32 drift;
	bool o_ops;
	u32 freq_compensation;
};

/* interface for PTP driver command GET_ORIG_COMP */
struct ptp_get_comp {
	/* the initial compensation value */
	u32 dw_origcomp;
	/* the minimum compensation value */
	u32 dw_mincomp;
	/*the max compensation value*/
	u32 dw_maxcomp;
	/*the min drift applying min compensation value in ppm*/
	u32 dw_mindrift;
	/*the max drift applying max compensation value in ppm*/
	u32 dw_maxdrift;
};

struct ptp_time_correct {
	u32 corr_period;
	u32 corr_inc;
};

/* PTP message version */
#define PTP_1588_MSG_VER_1	1
#define PTP_1588_MSG_VER_2	2

#define BD_ENET_TX_TS		0x20000000
#define BD_ENET_TX_BDU		0x80000000

struct fec_ptp_private {
	void __iomem *hwp;
	int	dev_id;

	struct fec_ptp_circular tx_timestamps;
	struct fec_ptp_circular rx_timestamps;
	spinlock_t cnt_lock;

	u64	prtc;
	u8	ptp_active;
	u8	ptp_slave;
	struct circ_buf	txstamp;
};

#ifdef CONFIG_FEC_1588
static inline int fec_ptp_malloc_priv(struct fec_ptp_private **priv)
{
	*priv = kzalloc(sizeof(struct fec_ptp_private), GFP_KERNEL);
	return 1;
}
extern int fec_ptp_init(struct fec_ptp_private *priv, int id);
extern void fec_ptp_cleanup(struct fec_ptp_private *priv);
extern int fec_ptp_start(struct fec_ptp_private *priv);
extern void fec_ptp_stop(struct fec_ptp_private *priv);
extern int fec_ptp_do_txstamp(struct sk_buff *skb);
extern void fec_ptp_store_txstamp(struct fec_ptp_private *priv,
				  struct sk_buff *skb,
				  struct bufdesc *bdp);
extern void fec_ptp_store_rxstamp(struct fec_ptp_private *priv,
				  struct sk_buff *skb,
				  struct bufdesc *bdp);
extern int fec_ptp_ioctl(struct fec_ptp_private *priv,
				struct ifreq *ifr, int cmd);
#else
static inline int fec_ptp_malloc_priv(struct fec_ptp_private **priv)
{
	return 0;
}
static inline int fec_ptp_init(struct fec_ptp_private *priv, int id)
{
	return 1;
}
static inline void fec_ptp_cleanup(struct fec_ptp_private *priv) { }
static inline int fec_ptp_start(struct fec_ptp_private *priv)
{
	return 1;
}
static inline void fec_ptp_stop(struct fec_ptp_private *priv) {}
static inline int fec_ptp_do_txstamp(struct sk_buff *skb)
{
	return 0;
}
static inline void fec_ptp_store_txstamp(struct fec_ptp_private *priv,
					 struct sk_buff *skb,
					 struct bufdesc *bdp) {}
static inline void fec_ptp_store_rxstamp(struct fec_ptp_private *priv,
					 struct sk_buff *skb,
					 struct bufdesc *bdp) {}
static inline int fec_ptp_ioctl(struct fec_ptp_private *priv,
				struct ifreq *ifr, int cmd)
{
	return 0;
}

#endif /* 1588 */

#endif
