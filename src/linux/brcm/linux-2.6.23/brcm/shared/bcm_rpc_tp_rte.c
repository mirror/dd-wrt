/*
 * RPC Transport layer(for HNDRTE bus driver)
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcm_rpc_tp_rte.c,v 1.1.2.17 2008/10/21 06:24:07 Exp $
 */

#ifndef WLC_LOW
#error "SPLIT"
#endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>

#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>

/* #define RTEDBG	1 */

#ifdef RTEDBG
#define	RPC_TP_DBGRTE(args)	do {printf args;} while (0)
#else
#define	RPC_TP_DBGRTE(args)
#endif

/* CLIENT dongle drvier RPC Transport implementation
 * HOST dongle driver uses DBUS, so it's in bcm_rpc_tp_dbus.c.
 *   This can be moved to bcm_rpc_th_dngl.c
 */

struct rpc_transport_info {
	osl_t *osh;
	hndrte_dev_t	*ctx;

	rpc_tx_complete_fn_t tx_complete;
	void* tx_context;
	bool tx_flowctl;		/* Global RX (WL->RPC->BUS->Host) flowcontrol state */
	struct spktq *tx_flowctlq;	/* Queue to store pkts when in global RX flowcontrol */
	uint8 tx_q_flowctl_hiwm;	/* Queue high watermask */
	uint8 tx_q_flowctl_lowm;	/* Queue low watermask */

	rpc_rx_fn_t rx_pkt;
	void* rx_context;

	uint bufalloc;
	int buf_cnt_inuse;		/* outstanding buf(alloc, not freed) */
	uint tx_cnt;			/* send successfully */
	uint txerr_cnt;			/* send failed */
	uint rx_cnt;
	uint rxdrop_cnt;

	uint tx_flowctl_cnt;		/* tx flow control transition times */
	bool tx_flowcontrolled;		/* tx flow control active */
#ifdef WLC_LOW
	rpc_txflowctl_cb_t txflowctl_cb; /* rpc tx flow control to control wlc_dpc() */
	void *txflowctl_ctx;

	mbool tp_dngl_aggregation;	/* aggregate into transport buffers */
	rpc_buf_t *tp_dngl_agg_p;	/* current aggregate chain header */
	rpc_buf_t *tp_dngl_agg_ptail;	/* current aggregate chain tail */
	uint tp_dngl_agg_sframes;	/* current aggregate packet subframes */
	uint8 tp_dngl_agg_sframes_limit;	/* agg sframe limit */
	uint tp_dngl_agg_bytes;		/* current aggregate packet total length */
	uint16 tp_dngl_agg_bytes_max;	/* agg byte max */
	uint tp_dngl_agg_txpending;	/* TBD, for agg watermark flow control */
	uint tp_dngl_agg_cnt_chain;	/* total aggregated pkt */
	uint tp_dngl_agg_cnt_sf;	/* total aggregated subframes */
	uint tp_dngl_agg_cnt_bytes;	/* total aggregated bytes */
	uint tp_dngl_agg_cnt_pass;	/* no. pkts not aggregated */
#endif	/* WLC_LOW */
};

#define	BCM_RPC_TP_Q_MAX	1024	/* Rx flow control queue size - Set it big and we don't
					 * expect it to get full. If the memory gets low, we
					 * just stop processing wlc_dpc
					 */
#ifdef WLC_LOW
#define BCM_RPC_TP_FLOWCTL_QWM_HIGH	16	/* high watermark for tp queue */
#define BCM_RPC_TP_FLOWCTL_QWM_LOW	4	/* low watermark for tp queue */
#endif

#define BCM_RPC_TP_AGG_MAX_SFRAME	6	/* agg limit on max subframes */
#define	BCM_RPC_TP_AGG_MAX_BYTE		18000	/* agg limit on max bytes */

static void bcm_rpc_tp_tx_encap(rpc_tp_info_t * rpcb, rpc_buf_t *b);
static int  bcm_rpc_tp_buf_send_internal(rpc_tp_info_t * rpc_th, rpc_buf_t *b);
static void bcm_rpc_tp_buf_send_enq(rpc_tp_info_t * rpc_th, rpc_buf_t *b);

static void bcm_rpc_tp_dngl_agg_initstate(rpc_tp_info_t * rpcb);
static bool bcm_rpc_tp_dngl_agg(rpc_tp_info_t *rpcb, rpc_buf_t *b);
static void bcm_rpc_tp_dngl_agg_append(rpc_tp_info_t * rpcb, rpc_buf_t *b);
static void bcm_rpc_tp_dngl_agg_close(rpc_tp_info_t * rpcb);
static void bcm_rpc_tp_dngl_agg_flush(rpc_tp_info_t * rpcb);

rpc_tp_info_t *
BCMATTACHFN(bcm_rpc_tp_attach)(osl_t * osh, void *bus)
{
	rpc_tp_info_t *rpc_th;
	hndrte_dev_t	*ctx = (hndrte_dev_t *)bus;

	rpc_th = (rpc_tp_info_t *)MALLOC(osh, sizeof(rpc_tp_info_t));
	if (rpc_th == NULL) {
		printf("%s: rpc_tp_info_t malloc failed\n", __FUNCTION__);
		return NULL;
	}

	memset(rpc_th, 0, sizeof(rpc_tp_info_t));

	rpc_th->osh = osh;
	rpc_th->ctx = ctx;

	/* Init for flow control */
	rpc_th->tx_flowctl = FALSE;
	rpc_th->tx_flowctlq = (struct spktq *)MALLOC(osh, sizeof(struct spktq));
	if (rpc_th->tx_flowctlq == NULL) {
		printf("%s: txflowctlq malloc failed\n", __FUNCTION__);
		MFREE(rpc_th->osh, rpc_th, sizeof(rpc_tp_info_t));
		return NULL;
	}
	pktqinit(rpc_th->tx_flowctlq, BCM_RPC_TP_Q_MAX);
	rpc_th->tx_q_flowctl_hiwm = BCM_RPC_TP_FLOWCTL_QWM_HIGH;
	rpc_th->tx_q_flowctl_lowm = BCM_RPC_TP_FLOWCTL_QWM_LOW;

	rpc_th->tp_dngl_agg_sframes_limit = BCM_RPC_TP_AGG_MAX_SFRAME;
	rpc_th->tp_dngl_agg_bytes_max = BCM_RPC_TP_AGG_MAX_BYTE;

	return rpc_th;
}

void
BCMATTACHFN(bcm_rpc_tp_detach)(rpc_tp_info_t * rpc_th)
{
	ASSERT(rpc_th);
	if (rpc_th->tx_flowctlq)
		MFREE(rpc_th->osh, rpc_th->tx_flowctlq, sizeof(struct spktq));

	MFREE(rpc_th->osh, rpc_th, sizeof(rpc_tp_info_t));
}


void
bcm_rpc_tp_rx_from_dnglbus(rpc_tp_info_t *rpc_th, struct lbuf *lb)
{
	void *p;

	rpc_th->rx_cnt++;

	if (rpc_th->rx_pkt == NULL) {
		printf("%s: no rpc rx fn, dropping\n", __FUNCTION__);
		rpc_th->rxdrop_cnt++;
		lb_free(lb);
		return;
	}

	p = PKTFRMNATIVE(rpc_th->osh, lb);

	(rpc_th->rx_pkt)(rpc_th->rx_context, p);
}
void
bcm_rpc_tp_register_cb(rpc_tp_info_t * rpc_th,
                               rpc_tx_complete_fn_t txcmplt, void* tx_context,
                               rpc_rx_fn_t rxpkt, void* rx_context, rpc_osl_t *rpc_osh)
{
	rpc_th->tx_complete = txcmplt;
	rpc_th->tx_context = tx_context;
	rpc_th->rx_pkt = rxpkt;
	rpc_th->rx_context = rx_context;
}

void
bcm_rpc_tp_deregister_cb(rpc_tp_info_t * rpcb)
{
	rpcb->tx_complete = NULL;
	rpcb->tx_context = NULL;
	rpcb->rx_pkt = NULL;
	rpcb->rx_context = NULL;
}


/* This is called by dngl_txstop as txflowcontrol (stopping tx from dongle to host) of bcmwl,
 * but is called rxflowcontrol in wl driver (pausing rx of wl driver). This is for low driver only.
 */
void
bcm_rpc_tp_txflowctl(rpc_tp_info_t *rpc_th, bool state, int prio)
{
	rpc_buf_t *b;

	ASSERT(rpc_th);

	if (rpc_th->tx_flowctl == state)
		return;

	printf("tp_txflowctl %d\n", state);

	rpc_th->tx_flowctl = state;
	rpc_th->tx_flowctl_cnt++;
	rpc_th->tx_flowcontrolled = state;

	/* when get out of flowcontrol, send all queued packets in a loop
	 *  but need to check tx_flowctl every iteration and stop if we got flowcontrolled again
	 */
	while (!rpc_th->tx_flowctl && !pktq_empty(rpc_th->tx_flowctlq)) {

		b = pktdeq(rpc_th->tx_flowctlq);

		bcm_rpc_tp_buf_send_internal(rpc_th, b);
	}

	/* if lowm is reached, release wldriver
	 *   TODO, count more(average 3?) if agg is ON
	 */
	if (pktq_len(rpc_th->tx_flowctlq) < rpc_th->tx_q_flowctl_lowm) {
		printf("bcm_rpc_tp_txflowctl, wm hit low!\n");
		rpc_th->txflowctl_cb(rpc_th->txflowctl_ctx, OFF);
	}

	return;
}

void
bcm_rpc_tp_down(rpc_tp_info_t * rpc_th)
{
	bcm_rpc_tp_dngl_agg_flush(rpc_th);
}

static void
bcm_rpc_tp_tx_encap(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
	uint32 *tp_lenp;
	uint32 rpc_len;

	rpc_len = PKTLEN(rpcb->osh, b);
	tp_lenp = (uint32*)PKTPUSH(rpcb->osh, b, BCM_RPC_TP_ENCAP_LEN);
	*tp_lenp = rpc_len;
}

int
bcm_rpc_tp_send_callreturn(rpc_tp_info_t * rpc_th, rpc_buf_t *b)
{
	int err;
	struct lbuf *lb;
	hndrte_dev_t *chained = rpc_th->ctx->dev_chained;

	ASSERT(chained);

	/* Add the TP encapsulation */
	bcm_rpc_tp_tx_encap(rpc_th, b);

	lb = PKTTONATIVE(rpc_th->osh, b);
	/* send through control endpoint */
	if ((err = chained->dev_funcs->xmit_ctl(rpc_th->ctx, chained, lb)) != 0) {
		printf("%s: xmit failed; free pkt 0x%p\n", __FUNCTION__, lb);
		rpc_th->txerr_cnt++;
		lb_free(lb);
	} else {
		rpc_th->tx_cnt++;
	}

	return err;

}

static void
bcm_rpc_tp_buf_send_enq(rpc_tp_info_t * rpc_th, rpc_buf_t *b)
{
	pktenq(rpc_th->tx_flowctlq, (void*)b);

	/* if hiwm is reached, throttle wldriver
	 *   TODO, count more(average 3?) if agg is ON
	 */
	if (pktq_len(rpc_th->tx_flowctlq) > rpc_th->tx_q_flowctl_hiwm) {
		printf("bcm_rpc_tp_buf_send_enq, wm hit high!\n");
		rpc_th->txflowctl_cb(rpc_th->txflowctl_ctx, ON);
	}

	/* If tx_flowctlq  gets full, set a bigger BCM_RPC_TP_Q_MAX */
	ASSERT(!pktq_full(rpc_th->tx_flowctlq));
}

int
bcm_rpc_tp_buf_send(rpc_tp_info_t * rpc_th, rpc_buf_t *b)
{
	int err;

	/* Add the TP encapsulation */
	bcm_rpc_tp_tx_encap(rpc_th, b);

	/* if agg successful, done; otherwise, send it */
	if (rpc_th->tp_dngl_aggregation) {
		if (bcm_rpc_tp_dngl_agg(rpc_th, b))
			return 0;
	}

	if (rpc_th->tx_flowctl) {
		bcm_rpc_tp_buf_send_enq(rpc_th, b);
		err = 0;
	} else {
		err = bcm_rpc_tp_buf_send_internal(rpc_th, b);
	}

	return err;
}

static int
bcm_rpc_tp_buf_send_internal(rpc_tp_info_t * rpc_th, rpc_buf_t *b)
{
	int err;
	struct lbuf *lb = (struct lbuf *)b;
	hndrte_dev_t *chained = rpc_th->ctx->dev_chained;

	ASSERT(chained);

	lb = PKTTONATIVE(rpc_th->osh, b);
	/* send through data endpoint */
	if ((err = chained->dev_funcs->xmit(rpc_th->ctx, chained, lb)) != 0) {
		printf("%s: xmit failed; free pkt 0x%p\n", __FUNCTION__, lb);
		rpc_th->txerr_cnt++;
		lb_free(lb);
	} else {
		rpc_th->tx_cnt++;
	}

	return err;
}

void
bcm_rpc_tp_dump(rpc_tp_info_t *rpcb)
{
	printf("\nRPC_TP_RTE:\n");
	printf("bufalloc %d(buf_cnt_inuse %d) tx %d(txerr %d) rx %d(rxdrop %d)\n",
		rpcb->bufalloc, rpcb->buf_cnt_inuse, rpcb->tx_cnt, rpcb->txerr_cnt,
		rpcb->rx_cnt, rpcb->rxdrop_cnt);

#if WLC_LOW
	printf("tx_flowctrl_cnt %d tx_flowctrl_status %d hwm %d lwm %d\n",
		rpcb->tx_flowctl_cnt, rpcb->tx_flowcontrolled,
		rpcb->tx_q_flowctl_hiwm, rpcb->tx_q_flowctl_lowm);

	printf("tp_dngl_agg sf_limit %d bytes_limit %d aggregation 0x%x\n",
		rpcb->tp_dngl_agg_sframes_limit, rpcb->tp_dngl_agg_bytes_max,
		rpcb->tp_dngl_aggregation);
	printf("agg counter: chain %d, sf %d, bytes %d byte-per-chain %d, bypass %d\n",
		rpcb->tp_dngl_agg_cnt_chain, rpcb->tp_dngl_agg_cnt_sf,
		rpcb->tp_dngl_agg_cnt_bytes,
		CEIL(rpcb->tp_dngl_agg_cnt_bytes, (rpcb->tp_dngl_agg_cnt_chain + 1)),
		rpcb->tp_dngl_agg_cnt_pass);
#endif	/* WLC_LOW */
}

/* Buffer manipulation, LEN + RPC_header + body */

rpc_buf_t *
bcm_rpc_tp_buf_alloc(rpc_tp_info_t * rpc_th, int len)
{
	rpc_buf_t * b;
	size_t tp_len = len + BCM_RPC_TP_ENCAP_LEN;

	b = (rpc_buf_t*)PKTGET(rpc_th->osh, tp_len, FALSE);

	if (b != NULL) {
		rpc_th->bufalloc++;
		rpc_th->buf_cnt_inuse++;
		PKTPULL(rpc_th->osh, b, BCM_RPC_TP_ENCAP_LEN);
	}

	return b;
}

void
bcm_rpc_tp_buf_free(rpc_tp_info_t * rpc_th, rpc_buf_t *b)
{
	ASSERT(b);

	rpc_th->buf_cnt_inuse -= pktsegcnt(rpc_th->osh, b);
	PKTFREE(rpc_th->osh, b, FALSE);
}

int
bcm_rpc_buf_len_get(rpc_tp_info_t * rpc_th, rpc_buf_t* b)
{
	return PKTLEN(rpc_th->osh, b);
}

int
bcm_rpc_buf_len_set(rpc_tp_info_t * rpc_th, rpc_buf_t* b, uint len)
{
	PKTSETLEN(rpc_th->osh, b, len);
	return 0;
}

unsigned char*
bcm_rpc_buf_data(rpc_tp_info_t * rpc_th, rpc_buf_t* b)
{
	return PKTDATA(rpc_th->osh, b);
}

unsigned char*
bcm_rpc_buf_push(rpc_tp_info_t * rpc_th, rpc_buf_t* b, uint bytes)
{
	return PKTPUSH(rpc_th->osh, b, bytes);
}

unsigned char*
bcm_rpc_buf_pull(rpc_tp_info_t * rpc_th, rpc_buf_t* b, uint bytes)
{
	return PKTPULL(rpc_th->osh, b, bytes);
}

rpc_buf_t *
bcm_rpc_buf_next_get(rpc_tp_info_t * rpcb, rpc_buf_t* b)
{
	return (rpc_buf_t *)PKTLINK(b);
}

void
bcm_rpc_buf_next_set(rpc_tp_info_t * rpcb, rpc_buf_t* b, rpc_buf_t *nextb)
{
	PKTSETLINK(b, nextb);
}

#ifdef WLC_LOW
void
bcm_rpc_tp_txflowctlcb_init(rpc_tp_info_t *rpc_th, void *ctx, rpc_txflowctl_cb_t cb)
{
	rpc_th->txflowctl_cb = cb;
	rpc_th->txflowctl_ctx = ctx;
}

void
bcm_rpc_tp_txflowctlcb_deinit(rpc_tp_info_t *rpc_th)
{
	rpc_th->txflowctl_cb = NULL;
	rpc_th->txflowctl_ctx = NULL;
}

void
bcm_rpc_tp_txq_wm_set(rpc_tp_info_t *rpc_th, uint8 hiwm, uint8 lowm)
{
	rpc_th->tx_q_flowctl_hiwm = hiwm;
	rpc_th->tx_q_flowctl_lowm = lowm;
}

void
bcm_rpc_tp_txq_wm_get(rpc_tp_info_t *rpc_th, uint8 *hiwm, uint8 *lowm)
{
	*hiwm = rpc_th->tx_q_flowctl_hiwm;
	*lowm = rpc_th->tx_q_flowctl_lowm;
}

void
bcm_rpc_tp_agg_limit_set(rpc_tp_info_t *rpc_th, uint8 sf, uint16 bytes)
{
	rpc_th->tp_dngl_agg_sframes_limit = sf;
	rpc_th->tp_dngl_agg_bytes_max = bytes;
}

void
bcm_rpc_tp_agg_limit_get(rpc_tp_info_t *rpc_th, uint8 *sf, uint16 *bytes)
{
	*sf = rpc_th->tp_dngl_agg_sframes_limit;
	*bytes = rpc_th->tp_dngl_agg_bytes_max;
}

#endif	/* WLC_LOW */

/* TP aggregation: set, init, agg, append, close, flush */
static void
bcm_rpc_tp_dngl_agg_initstate(rpc_tp_info_t * rpcb)
{
	rpcb->tp_dngl_agg_p = NULL;
	rpcb->tp_dngl_agg_ptail = NULL;
	rpcb->tp_dngl_agg_sframes = 0;
	rpcb->tp_dngl_agg_bytes = 0;
	rpcb->tp_dngl_agg_txpending = 0;
}

static bool
bcm_rpc_tp_dngl_agg(rpc_tp_info_t *rpcb, rpc_buf_t *b)
{
	uint totlen;
	uint pktlen;

	ASSERT(rpcb->tp_dngl_aggregation);

	pktlen = bcm_rpc_buf_len_get(rpcb, b);

	totlen = pktlen + rpcb->tp_dngl_agg_bytes;

	if ((totlen > rpcb->tp_dngl_agg_bytes_max) ||
		(rpcb->tp_dngl_agg_sframes + 1 > rpcb->tp_dngl_agg_sframes_limit)) {

		RPC_TP_DBGRTE(("bcm_rpc_tp_dngl_agg: terminte TP agg for tpbyte %d or txframe %d\n",
			rpcb->tp_dngl_agg_bytes_max,	rpcb->tp_dngl_agg_sframes_limit));

		bcm_rpc_tp_dngl_agg_close(rpcb);
		rpcb->tp_dngl_agg_cnt_pass++;

		return FALSE;
	}

	bcm_rpc_tp_dngl_agg_append(rpcb, b);
	return TRUE;
}

/*
 *  tp_dngl_agg_p points to the header lbuf, tp_dngl_agg_ptail points to the tail lbuf
 *
 * The TP agg format typically will be below
 *   | TP header(len) | subframe1 rpc_header | subframe1 data |
 *     | TP header(len) | subframe2 rpc_header | subframe2 data |
 *          ...
 *           | TP header(len) | subframeN rpc_header | subframeN data |
 * no padding
*/
static void
bcm_rpc_tp_dngl_agg_append(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
	uint tp_len = bcm_rpc_buf_len_get(rpcb, b);

	if (rpcb->tp_dngl_agg_p == NULL) {

		rpcb->tp_dngl_agg_p = rpcb->tp_dngl_agg_ptail = b;

	} else {
		/* chain the pkts at the end of current one */
		ASSERT(rpcb->tp_dngl_agg_ptail != NULL);

		PKTSETNEXT(rpcb->osh, rpcb->tp_dngl_agg_ptail, b);
		rpcb->tp_dngl_agg_ptail = b;
	}

	rpcb->tp_dngl_agg_sframes++;
	rpcb->tp_dngl_agg_bytes += tp_len;

	RPC_TP_DBGRTE(("bcm_rpc_tp_dngl_agg_append, tp_len %d tot %d, sframe %d\n", tp_len,
		rpcb->tp_dngl_agg_bytes, rpcb->tp_dngl_agg_sframes));
}

#define BCM_RPC_TP_USB_TOTAL_LEN_BOUNDARY	516

static void
bcm_rpc_tp_dngl_agg_close(rpc_tp_info_t * rpcb)
{
	int err;
	rpc_buf_t *b;

	if (rpcb->tp_dngl_agg_p == NULL) {	/* no aggregation formed */
		return;
	}

	RPC_TP_DBGRTE(("bcm_rpc_tp_dngl_agg_close, send %d, sframe %d\n", rpcb->tp_dngl_agg_bytes,
		rpcb->tp_dngl_agg_sframes));

	b = rpcb->tp_dngl_agg_p;
	rpcb->tp_dngl_agg_cnt_chain++;
	rpcb->tp_dngl_agg_cnt_sf += rpcb->tp_dngl_agg_sframes;
	rpcb->tp_dngl_agg_cnt_bytes += rpcb->tp_dngl_agg_bytes;

	if (rpcb->tp_dngl_agg_bytes == BCM_RPC_TP_USB_TOTAL_LEN_BOUNDARY) {
		RPC_TP_DBGRTE(("%s: risk totlen pkt\n", __FUNCTION__));
	}

	bcm_rpc_tp_dngl_agg_initstate(rpcb);

	rpcb->tp_dngl_agg_txpending++;

	if (rpcb->tx_flowctl) {
		bcm_rpc_tp_buf_send_enq(rpcb, b);
		err = 0;
	} else {
		err = bcm_rpc_tp_buf_send_internal(rpcb, b);
	}

	if (err != 0) {
		printf("bcm_rpc_tp_dngl_agg_close: send err!!!\n");
		/* ASSERT(0) */
	}
}

static void
bcm_rpc_tp_dngl_agg_flush(rpc_tp_info_t * rpcb)
{
	/* toss the chained buffer */
	if (rpcb->tp_dngl_agg_p)
		bcm_rpc_tp_buf_free(rpcb, rpcb->tp_dngl_agg_p);

	bcm_rpc_tp_dngl_agg_initstate(rpcb);
}

void
bcm_rpc_tp_dngl_agg_set(rpc_tp_info_t *rpcb, uint32 reason, bool set)
{
	if (set) {
		RPC_TP_DBGRTE(("bcm_rpc_tp_dngl_agg_set: agg start\n"));

		mboolset(rpcb->tp_dngl_aggregation, reason);

	} else if (rpcb->tp_dngl_aggregation) {
		RPC_TP_DBGRTE(("bcm_rpc_tp_dngl_agg_set: agg end\n"));

		mboolclr(rpcb->tp_dngl_aggregation, reason);
		if (!rpcb->tp_dngl_aggregation)
			bcm_rpc_tp_dngl_agg_close(rpcb);
	}
}
