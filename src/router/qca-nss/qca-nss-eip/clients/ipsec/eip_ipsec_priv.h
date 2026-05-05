/*
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __EIP_IPSEC_PRIV_H
#define __EIP_IPSEC_PRIV_H

#include <linux/kref.h>
#include <eip.h>
#if defined(EIP_IPSEC_VP_SUPP)
	#include <ppe_drv.h>
	#include <ppe_drv_vp.h>
	#include <ppe_vp_public.h>
	#include <ppe_drv_sc.h>
	#include <ppe_drv_eip.h>
	#include <ppe_drv_iface.h>
	#include <ppe_vp_tx.h>
#endif

#include "../exports/eip_ipsec.h"
#include "eip_ipsec_sa.h"
#include "eip_ipsec_dev.h"
#include "eip_ipsec_proto.h"
#include "eip_ipsec_xfrm.h"

#define EIP_IPSEC_MAX_STR_LEN 64U	/* Maximum print lenght */
#define EIP_IPSEC_TAG_MAX 128U		/* Maximum tag length for debug prints */
#define EIP_IPSEC_HASH_SHIFT 6U
#define EIP_IPSEC_HASH_SIZE (1U << EIP_IPSEC_HASH_SHIFT)
#define EIP_IPSEC_HASH_MASK (EIP_IPSEC_HASH_SIZE - 1)

#define EIP_IPSEC_RX_STEER_BUDGET 128U		/* RPS napi poll budget */
#define EIP_IPSEC_TX_STEER_BUDGET 128U		/* TPS napi poll budget */
#define EIP_IPSEC_PKT_STEER_FIFO_LEN 8192U	/* Packet steer FIFO length */


#define EIP_IPSEC_NATT_PORT 4500U	/* IPsec NAT-T Port number */

/*
 * IPsec driver object.
 */
struct eip_ipsec_drv {
	struct kref ref;		/* Driver reference */
	struct completion completion;	/* Completion to wait for all deref */

	spinlock_t lock;		/* Common lock for list manipulation */
	struct hlist_head __rcu sa_hlist[EIP_IPSEC_HASH_SIZE];
					/* SA hash list based on IP & SPI */

	struct list_head dev_head;	/* Device database */
	struct kmem_cache *sa_cache;	/* Kmem cache for SA memory */
	struct eip_ctx *ctx;		/* DMA context to use for transformation request */
	struct dentry *dentry;		/* Driver debugfs dentry */
	struct platform_device *pdev;	/* Device associated with the driver */
	bool tx_steer_en;		/* Tx packet steer is enabled */
	bool rx_steer_en;		/* Rx packet steer is enabled */
#if defined(EIP_IPSEC_VP_SUPP)
	struct ppe_drv_iface *iface;	/* PPE interface for inline port */
#endif
};

/*
 * EIP-PPE bypass data
 */
struct eip_ipsec_ppe_mdata {
	uint16_t ppe_src_port;                   /* Source port */
	uint16_t ppe_dest_port;                  /* Destination port */
	uint8_t tstamp_high;                    /* Timestamp high */
	uint8_t tstamp_flag:1;                  /* Timestamp flag */
	uint8_t ptp_type:4;			/* PTP Type */
	uint8_t ts_dir:1;			/* Direct 0 = From MAC, 1 = To MAC */
	uint8_t rsv:1;				/* Reserved */
	uint8_t fake_mac:1;			/* Mac header is fake */
	uint8_t service_code;                   /* Service code */
	uint8_t rsv2:7;				/* Reserved */
	uint8_t fake_l2_prot:1;                   /* fake_l2_prot */
	uint32_t tstamp_low;                    /* Timestamp low */
	uint32_t hash;                          /* Hashing information */
}__attribute__((packed));

extern struct eip_ipsec_drv eip_ipsec_drv_g;	/* Global Driver object */
extern uint8_t eip_ipsec_core_id;		/* Global ipsec core id */
extern int eip_pkt_steer_qlen;			/* Global queue length for packet steer */
extern ulong eip_rps_core_mask;			/* Global core mask for IPsec RPS */
extern void eip_ipsec_drv_final(struct kref *kref);
extern bool disable_v4_offload;

/*
 * Increment driver object reference
 */
static inline struct eip_ipsec_drv *eip_ipsec_drv_ref(struct eip_ipsec_drv *drv)
{
	kref_get(&drv->ref);
	return drv;
}

/*
 * Decrement driver object reference
 */
static inline void eip_ipsec_drv_deref(struct eip_ipsec_drv *drv)
{
	kref_put(&drv->ref, eip_ipsec_drv_final);
}

/*
 * Flow functions
 */
int eip_ipsec_flow_init(void);
void eip_ipsec_flow_deinit(void);
void eip_ipsec_flow_del(struct net_device *ndev, struct eip_ipsec_tuple *flow_tuple, bool is_encap);
int eip_ipsec_flow_add(struct net_device *ndev, struct eip_ipsec_tuple *flow_tuple, struct eip_ipsec_sa *sa);
s16 eip_ipsec_flow_match_spi(__le32 spi, struct net_device *dev, u32 sa_flags);

#endif /* !__EIP_IPSEC_PRIV_H */
