/*
 * Header file describing the internal (inter-module) DHD interfaces.
 *
 * Provides type definitions and function prototypes used to link the
 * DHD OS, bus, and protocol modules.
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_proto.h 634379 2016-04-27 20:21:50Z $
 */

#ifndef _dhd_proto_h_
#define _dhd_proto_h_

#include <dhdioctl.h>
#include <wlioctl.h>
#ifdef BCMPCIE
#include <dhd_flowring.h>
#endif

#define DEFAULT_IOCTL_RESP_TIMEOUT	2000
#ifndef IOCTL_RESP_TIMEOUT
#ifdef BCMQT
#define IOCTL_RESP_TIMEOUT  30000 /* In milli second */
#else
/* In milli second default value for Production FW */
#define IOCTL_RESP_TIMEOUT  DEFAULT_IOCTL_RESP_TIMEOUT
#endif /* BCMQT */
#endif /* IOCTL_RESP_TIMEOUT */

#ifndef MFG_IOCTL_RESP_TIMEOUT
#define MFG_IOCTL_RESP_TIMEOUT  20000  /* In milli second default value for MFG FW */
#endif /* MFG_IOCTL_RESP_TIMEOUT */

#define IOCTL_DISABLE_TIMEOUT 0
/*
 * Exported from the dhd protocol module (dhd_cdc, dhd_rndis)
 */

/* Linkage, sets prot link and updates hdrlen in pub */
extern int dhd_prot_attach(dhd_pub_t *dhdp);

/* Initilizes the index block for dma'ing indices */
extern int dhd_prot_dma_indx_init(dhd_pub_t *dhdp, uint32 rw_index_sz,
	uint8 type, uint32 length);

/* Unlink, frees allocated protocol memory (including dhd_prot) */
extern void dhd_prot_detach(dhd_pub_t *dhdp);

/* Initialize protocol: sync w/dongle state.
 * Sets dongle media info (iswl, drv_version, mac address).
 */
extern int dhd_sync_with_dongle(dhd_pub_t *dhdp);

/* Protocol initialization needed for IOCTL/IOVAR path */
extern int dhd_prot_init(dhd_pub_t *dhd);

/* Stop protocol: sync w/dongle state. */
extern void dhd_prot_stop(dhd_pub_t *dhdp);

/* Add any protocol-specific data header.
 * Caller must reserve prot_hdrlen prepend space.
 */
#ifdef __FreeBSD__
extern void dhd_prot_hdrpush(dhd_pub_t *, int ifidx, void **txp);
#else
extern void dhd_prot_hdrpush(dhd_pub_t *, int ifidx, void *txp);
#endif
extern uint dhd_prot_hdrlen(dhd_pub_t *, void *txp);

/* Remove any protocol-specific data header. */
extern int dhd_prot_hdrpull(dhd_pub_t *, int *ifidx, void *rxp, uchar *buf, uint *len);

/* Use protocol to issue ioctl to dongle */
extern int dhd_prot_ioctl(dhd_pub_t *dhd, int ifidx, wl_ioctl_t * ioc, void * buf, int len);

/* Handles a protocol control response asynchronously */
extern int dhd_prot_ctl_complete(dhd_pub_t *dhd);

/* Check for and handle local prot-specific iovar commands */
extern int dhd_prot_iovar_op(dhd_pub_t *dhdp, const char *name,
                             void *params, int plen, void *arg, int len, bool set);

/* Add prot dump output to a buffer */
extern void dhd_prot_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);

/* Update local copy of dongle statistics */
extern void dhd_prot_dstats(dhd_pub_t *dhdp);

extern int dhd_ioctl(dhd_pub_t * dhd_pub, dhd_ioctl_t *ioc, void * buf, uint buflen);

extern int dhd_preinit_ioctls(dhd_pub_t *dhd);

extern int dhd_process_pkt_reorder_info(dhd_pub_t *dhd, uchar *reorder_info_buf,
	uint reorder_info_len, void **pkt, uint32 *free_buf_count);

#ifdef BCMPCIE
extern bool dhd_prot_process_msgbuf_txcpl(dhd_pub_t *dhd, uint bound);
extern bool dhd_prot_process_msgbuf_rxcpl(dhd_pub_t *dhd, uint bound);
extern int dhd_prot_process_ctrlbuf(dhd_pub_t * dhd);
extern bool dhd_prot_dtohsplit(dhd_pub_t * dhd);
extern int dhd_post_dummy_msg(dhd_pub_t *dhd);
extern int dhdmsgbuf_lpbk_req(dhd_pub_t *dhd, uint len);
extern void dhd_prot_rx_dataoffset(dhd_pub_t *dhd, uint32 offset);
extern int dhd_prot_txdata(dhd_pub_t *dhd, void *p, uint8 ifidx);
extern int dhdmsgbuf_dmaxfer_req(dhd_pub_t *dhd, uint len, uint srcdelay, uint destdelay);

extern void dhd_dma_buf_init(dhd_pub_t *dhd, void *dma_buf,
	void *va, uint32 len, dmaaddr_t pa, void *dmah, void *secdma);
extern void dhd_prot_flowrings_pool_release(dhd_pub_t *dhd,
	uint16 flowid, void *msgbuf_ring);
extern int dhd_prot_flow_ring_create(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node);
extern int dhd_post_tx_ring_item(dhd_pub_t *dhd, void *PKTBUF, uint8 ifindex);
extern int dhd_prot_flow_ring_delete(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node);
extern int dhd_prot_flow_ring_flush(dhd_pub_t *dhd, flow_ring_node_t *flow_ring_node);
extern int dhd_prot_ringupd_dump(dhd_pub_t *dhd, struct bcmstrbuf *b);
extern uint32 dhd_prot_metadata_dbg_set(dhd_pub_t *dhd, bool val);
extern uint32 dhd_prot_metadata_dbg_get(dhd_pub_t *dhd);
extern uint32 dhd_prot_metadatalen_set(dhd_pub_t *dhd, uint32 val, bool rx);
extern uint32 dhd_prot_metadatalen_get(dhd_pub_t *dhd, bool rx);
extern void dhd_prot_print_flow_ring(dhd_pub_t *dhd, void *msgbuf_flow_info,
	struct bcmstrbuf *strbuf, const char * fmt);
extern void dhd_prot_print_info(dhd_pub_t *dhd, struct bcmstrbuf *strbuf);
extern void dhd_prot_update_txflowring(dhd_pub_t *dhdp, uint16 flow_id, void *msgring_info);
extern void dhd_prot_txdata_write_flush(dhd_pub_t *dhd, uint16 flow_id, bool in_lock);
extern uint32 dhd_prot_txp_threshold(dhd_pub_t *dhd, bool set, uint32 val);
extern void dhd_prot_reset(dhd_pub_t *dhd);
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
extern int dhd_prot_txqueue_flush(dhd_pub_t *dhd, void *flow_ring);
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
#ifdef DHD_LB
extern void dhd_lb_tx_compl_handler(unsigned long data);
extern void dhd_lb_rx_compl_handler(unsigned long data);
extern void dhd_lb_rx_process_handler(unsigned long data);
#endif /* DHD_LB */
#endif /* BCMPCIE */
/********************************
 * For version-string expansion *
 */
#if defined(BDC)
#define DHD_PROTOCOL "bdc"
#elif defined(CDC)
#define DHD_PROTOCOL "cdc"
#else
#define DHD_PROTOCOL "unknown"
#endif /* proto */

#if defined(BCM_DHD_RUNNER)
extern void dhd_prot_schedule_runner(dhd_pub_t *dhd);
#endif /* BCM_DHD_RUNNER */
#endif /* _dhd_proto_h_ */
