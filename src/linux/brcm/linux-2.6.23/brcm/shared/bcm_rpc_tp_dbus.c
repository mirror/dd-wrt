/*
 * RPC Transport layer(for host dbus driver)
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
 * $Id: bcm_rpc_tp_dbus.c,v 1.7.4.33 2008/10/20 06:58:41 Exp $
 */

#if (!defined(WLC_HIGH) && !defined(WLC_LOW))
#error "SPLIT"
#endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>

#include <dbus.h>
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <rpc_osl.h>

/* #define DBUSDBG */
#ifdef DBUSDBG
#define	RPC_TP_DBGDBUS(args)	do {printf args;} while (0)
#else
#define	RPC_TP_DBGDBUS(args)
#endif

#define DBUS_NTXQ	50	/* queue size for TX on endpoint 1 */
#define DBUS_NRXQ_1	50	/* queue size for RX on endpoint 1 */
#define DBUS_NRXQ_2	1	/* queue size for RX on endpoint 2 */

#define RPCNUMBUF	(DBUS_NTXQ + DBUS_NRXQ_1 + DBUS_NRXQ_2) * 3
#define RPCRXLOWAT	(RPCNUMBUF - (DBUS_NRXQ_1 + DBUS_NTXQ + DBUS_NRXQ_2))

#define RPC_BUS_SEND_WAIT_TIMEOUT_MSEC 500

#ifdef NDIS
#define RPC_TP_LOCK(ri)		NdisAcquireSpinLock(&(ri)->lock)
#define RPC_TP_UNLOCK(ri)	NdisReleaseSpinLock(&(ri)->lock)
#else
#define RPC_TP_LOCK(ri)		spin_lock_irqsave(&(ri)->lock, (ri)->flags);
#define RPC_TP_UNLOCK(ri)	spin_unlock_irqrestore(&(ri)->lock, (ri)->flags);
#endif

/* RPC TRANSPORT API */

static void bcm_rpc_tp_rx(rpc_tp_info_t *rpcb, void *p);
#if defined(NDIS)
static int bcm_rpc_tp_alloc_bufpool(rpc_tp_info_t * rpcb);
#endif

struct rpc_transport_info {
	osl_t *osh;
	rpc_osl_t *rpc_osh;
	struct dbus_pub *bus;

	rpc_tx_complete_fn_t tx_complete;
	void* tx_context;

	rpc_rx_fn_t rx_pkt;
	void* rx_context;
	void* rx_rtn_pkt;

#if defined(NDIS)
	shared_info_t *sh;
	struct lbfree pktfree;
	NDIS_SPIN_LOCK	lock;
#else
	spinlock_t	lock;
	ulong flags;
#endif /* NDIS */

	uint bufalloc;
	int buf_cnt_inuse;	/* outstanding buf(alloc, not freed) */
	uint tx_cnt;		/* send successfully */
	uint txerr_cnt;		/* send failed */
	uint buf_cnt_max;
	uint rx_cnt;
	uint rxdrop_cnt;

	uint bus_mtu;		/* Max size of bus packet */
	uint bus_txdepth;	/* Max TX that can be posted */
	uint bus_txpending;	/* How many posted */
	uint tx_flowctl_cnt;	/* tx flow control transition times */
	bool rxflowctrl;	/* rx flow control active */

	uint tp_host_deagg_cnt_chain;	/* multifrag pkt */
	uint tp_host_deagg_cnt_sf;	/* total no. of frag inside multifrag */
	uint tp_host_deagg_cnt_bytes;	/* total deagg bytes */
	uint tp_host_deagg_cnt_badfmt;	/* bad format */
	uint tp_host_deagg_cnt_badsflen;	/* bad sf len */
	uint tp_host_deagg_cnt_pass;	/* passthrough, single frag */
};


/* histogram for dbus pending pkt */
static uint32 histogram_rpctp_dbus[DBUS_NTXQ];


static void
rpc_dbus_send_complete(void *handle, void *pktinfo, int status)
{
	rpc_tp_info_t *rpcb = (rpc_tp_info_t *)handle;

	ASSERT(rpcb);

	if (rpcb->tx_complete)
		(rpcb->tx_complete)(rpcb->tx_context, pktinfo, status);
	else if (pktinfo)
		bcm_rpc_tp_buf_free(rpcb, pktinfo);

	RPC_TP_LOCK(rpcb);

	rpcb->bus_txpending--;
	if (rpcb->bus_txpending == (rpcb->bus_txdepth - 1))
		RPC_OSL_WAKE(rpcb->rpc_osh);

	RPC_TP_UNLOCK(rpcb);

	if (status)
		printf("%s: tx failed=%d\n", __FUNCTION__, status);

}

static void
rpc_dbus_recv_pkt(void *handle, void *pkt)
{
	rpc_tp_info_t *rpcb = handle;

	if ((rpcb == NULL) || (pkt == NULL))
		return;

	bcm_rpc_tp_rx(rpcb, pkt);
}

static void
rpc_dbus_recv_buf(void *handle, uint8 *buf, int len)
{
	rpc_tp_info_t *rpcb = handle;
	void *pkt;
	uint32 rpc_len;
	uint frag = rpcb->tp_host_deagg_cnt_sf;
	uint agglen = len;

	if ((rpcb == NULL) || (buf == NULL))
		return;

	/* TP pkt should have more than encapsulation header */
	if (len <= BCM_RPC_TP_ENCAP_LEN) {
		printf("%s: wrong len %d\n", __FUNCTION__, len);
		goto error;
	}

	while (len > BCM_RPC_TP_ENCAP_LEN) {
		rpc_len = *(uint32*)buf;

		if (rpc_len > (uint32)(len - BCM_RPC_TP_ENCAP_LEN)) {
			rpcb->tp_host_deagg_cnt_badsflen++;
			return;
		}

		/* RPC_BUFFER_RX: allocate */
		if ((pkt = bcm_rpc_tp_buf_alloc(rpcb, rpc_len)) == NULL) {
			printf("%s: bcm_rpc_tp_buf_alloc failed (len %d)\n", __FUNCTION__, len);
			goto error;
		}

		/* RPC_BUFFER_RX: BYTE_COPY from dbus buffer */
		bcopy(buf + BCM_RPC_TP_ENCAP_LEN, bcm_rpc_buf_data(rpcb, pkt), rpc_len);

		/* !! send up */
		bcm_rpc_tp_rx(rpcb, pkt);

		len -= (BCM_RPC_TP_ENCAP_LEN + rpc_len);
		buf += (BCM_RPC_TP_ENCAP_LEN + rpc_len);

		if (len > BCM_RPC_TP_ENCAP_LEN) {	/* more frag */
			rpcb->tp_host_deagg_cnt_sf++;
			RPC_TP_DBGDBUS(("%s: deagg %d(remaining %d) bytes\n", __FUNCTION__,
				rpc_len, len));
		} else {
			if (len != 0) {
				printf("%s: deagg, remaining len %d is not 0\n", __FUNCTION__, len);
			}
			rpcb->tp_host_deagg_cnt_pass++;
		}


	}

	if (frag < rpcb->tp_host_deagg_cnt_sf) {	/* aggregated frames */
		rpcb->tp_host_deagg_cnt_sf++;	/* last one was not counted */
		rpcb->tp_host_deagg_cnt_chain++;

		rpcb->tp_host_deagg_cnt_bytes += agglen;
	}
error:
	return;
}

int
bcm_rpc_tp_recv_rtn(rpc_tp_info_t *rpcb)
{
	if (!rpcb)
		return -1;

	ASSERT(rpcb->rx_rtn_pkt == NULL);
	if ((rpcb->rx_rtn_pkt = bcm_rpc_tp_buf_alloc(rpcb, PKTBUFSZ)) == NULL)
		return -1;
	if (dbus_recv_ctl(rpcb->bus, bcm_rpc_buf_data(rpcb, rpcb->rx_rtn_pkt),
	                  PKTBUFSZ)) {
		/* May have been cleared by complete routine */
		if (rpcb->rx_rtn_pkt)
			bcm_rpc_tp_buf_free(rpcb, rpcb->rx_rtn_pkt);
		rpcb->rx_rtn_pkt = NULL;
		return -1;
	}
	return 0;
}

static void
rpc_dbus_flowctrl_tx(void *handle, bool onoff)
{
}

static void
rpc_dbus_errhandler(void *handle, int err)
{
}

static void
rpc_dbus_ctl_complete(void *handle, int type, int status)
{
	rpc_tp_info_t *rpcb = (rpc_tp_info_t *)handle;
	void *pkt = rpcb->rx_rtn_pkt;

	rpcb->rx_rtn_pkt = NULL;

	if (!status) {
		bcm_rpc_buf_pull(rpcb, pkt, BCM_RPC_TP_ENCAP_LEN);
		(rpcb->rx_pkt)(rpcb->rx_context, pkt);
	} else {
		printf("%s: no rpc rx ctl, dropping 0x%x\n", __FUNCTION__, status);
		bcm_rpc_tp_buf_free(rpcb, pkt);
	}
}

static void
rpc_dbus_state_change(void *handle, int state)
{
	rpc_tp_info_t *rpcb = handle;

	if (rpcb == NULL)
		return;

	/* FIX: DBUS is down, need to do something? */
	if (state == DBUS_STATE_DOWN) {
		printf("%s: DBUS is down\n", __FUNCTION__);
	}
}

static void *
rpc_dbus_pktget(void *handle, uint len, bool send)
{
	rpc_tp_info_t *rpcb = handle;
	void *p;

	if (rpcb == NULL)
		return NULL;

	if ((p = bcm_rpc_tp_buf_alloc(rpcb, len)) == NULL) {
		printf("%s: bcm_rpc_tp_buf_alloc failed (len %d) %d\n", __FUNCTION__, len, send);
		return NULL;
	}

	return p;
}

static void
rpc_dbus_pktfree(void *handle, void *p, bool send)
{
	rpc_tp_info_t *rpcb = handle;

	if ((rpcb == NULL) || (p == NULL))
		return;

	bcm_rpc_tp_buf_free(rpcb, p);
}

static dbus_callbacks_t rpc_dbus_cbs = {
	rpc_dbus_send_complete,
	rpc_dbus_recv_buf,
	rpc_dbus_recv_pkt,
	rpc_dbus_flowctrl_tx,
	rpc_dbus_errhandler,
	rpc_dbus_ctl_complete,
	rpc_dbus_state_change,
	rpc_dbus_pktget,
	rpc_dbus_pktfree
};

#if !defined(NDIS)
rpc_tp_info_t *
bcm_rpc_tp_attach(osl_t * osh, void *bus)
#else
rpc_tp_info_t *
bcm_rpc_tp_attach(osl_t * osh, shared_info_t *shared, void *bus)
#endif
{
	rpc_tp_info_t *rpcb;
	struct dbus_pub *dbus = NULL;
	dbus_attrib_t attrib;
	dbus_config_t config;

	rpcb = (rpc_tp_info_t*)MALLOC(osh, sizeof(rpc_tp_info_t));
	if (rpcb == NULL) {
		printf("%s: rpc_tp_info_t malloc failed\n", __FUNCTION__);
		return NULL;
	}
	memset(rpcb, 0, sizeof(rpc_tp_info_t));

	bzero(histogram_rpctp_dbus, DBUS_NTXQ * sizeof(uint32));

#if defined(NDIS)
	NdisAllocateSpinLock(&rpcb->lock);
#else
	spin_lock_init(&rpcb->lock);
#endif
	rpcb->osh = osh;

	/* FIX: Need to determing rx size and pass it here */
	dbus = (struct dbus_pub *)dbus_attach(osh, DBUS_RX_BUFFER_SIZE_RPC, DBUS_NRXQ_1,
		DBUS_NTXQ, rpcb /* info */, &rpc_dbus_cbs);
	if (dbus == NULL) {
		printf("%s: dbus_attach failed\n", __FUNCTION__);
		goto error;
	}

	rpcb->bus = (struct dbus_pub *)dbus;

	dbus_get_attrib(dbus, &attrib);
	rpcb->bus_mtu = attrib.mtu;
	rpcb->bus_txdepth = DBUS_NTXQ;

	config.rxctl_deferrespok = TRUE;
	dbus_set_config(dbus, &config);

#if defined(NDIS)
	rpcb->sh = shared;
	if (bcm_rpc_tp_alloc_bufpool(rpcb)) {
		printf("%s: bcm_rpc_tp_alloc_bufpool failed\n", __FUNCTION__);
		goto error;
	}
#endif /* NDIS */

	/* Bring DBUS up right away so RPC can start receiving */
	if (dbus_up(dbus)) {
		printf("%s: dbus_up failed\n", __FUNCTION__);
		goto error;
	}

	return rpcb;

error:
	if (rpcb)
		bcm_rpc_tp_detach(rpcb);

	return NULL;
}

#if defined(NDIS)
static int
bcm_rpc_tp_alloc_bufpool(rpc_tp_info_t * rpcb)
{
	NDIS_STATUS status;

	/* allocate and fill packet freelist */
	status = shared_lb_alloc(rpcb->sh, &rpcb->pktfree, RPCNUMBUF, FALSE, TRUE, FALSE, TRUE);
	if (NDIS_ERROR(status))
		return BCME_NOMEM;

	return BCME_OK;
}
#endif /* NDIS */

void
bcm_rpc_tp_detach(rpc_tp_info_t * rpcb)
{
	ASSERT(rpcb);

	if (rpcb->bus) {
#if defined(NDIS)
		NdisFreeSpinLock(&rpcb->lock);
#endif
		dbus_detach(rpcb->bus);
		rpcb->bus = NULL;
	}

#if defined(NDIS)
	shared_lb_free(rpcb->sh, &rpcb->pktfree, FALSE, TRUE);
#endif

	MFREE(rpcb->osh, rpcb, sizeof(rpc_tp_info_t));
}

static void
bcm_rpc_tp_rx(rpc_tp_info_t *rpcb, void *p)
{
	RPC_TP_LOCK(rpcb);
	rpcb->rx_cnt++;
	RPC_TP_UNLOCK(rpcb);

	if (rpcb->rx_pkt == NULL) {
		printf("%s: no rpc rx fn, dropping\n", __FUNCTION__);
		RPC_TP_LOCK(rpcb);
		rpcb->rxdrop_cnt++;
		RPC_TP_UNLOCK(rpcb);

		/* RPC_BUFFER_RX: free if no callback */
		bcm_rpc_tp_buf_free(rpcb, p);
		return;
	}

	/* RPC_BUFFER_RX: free inside */
	(rpcb->rx_pkt)(rpcb->rx_context, p);
}

void
bcm_rpc_tp_register_cb(rpc_tp_info_t * rpcb,
                       rpc_tx_complete_fn_t txcmplt, void* tx_context,
                       rpc_rx_fn_t rxpkt, void* rx_context,
                       rpc_osl_t *rpc_osh)
{
	rpcb->tx_complete = txcmplt;
	rpcb->tx_context = tx_context;
	rpcb->rx_pkt = rxpkt;
	rpcb->rx_context = rx_context;
	rpcb->rpc_osh = rpc_osh;
}

void
bcm_rpc_tp_deregister_cb(rpc_tp_info_t * rpcb)
{
	rpcb->tx_complete = NULL;
	rpcb->tx_context = NULL;
	rpcb->rx_pkt = NULL;
	rpcb->rx_context = NULL;
	rpcb->rpc_osh = NULL;
}

int
bcm_rpc_tp_buf_send(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
	int err;
	bool tx_flow_control;
	int len;

	/* Increment before sending to avoid race condition */
	RPC_TP_LOCK(rpcb);

	histogram_rpctp_dbus[rpcb->bus_txpending]++;

	rpcb->bus_txpending++;
	tx_flow_control = (rpcb->bus_txpending == rpcb->bus_txdepth);
	rpcb->tx_flowctl_cnt += tx_flow_control;
	RPC_TP_UNLOCK(rpcb);

	if (tx_flow_control) {
		err = RPC_OSL_WAIT(rpcb->rpc_osh, RPC_BUS_SEND_WAIT_TIMEOUT_MSEC, NULL);
		if (err) {
			printf("%s: RPC_OSL_WAIT error %d, failing send\n", __FUNCTION__, err);
			RPC_TP_LOCK(rpcb);
			rpcb->txerr_cnt++;
			rpcb->bus_txpending--;
			RPC_TP_UNLOCK(rpcb);
			return err;
		}
	}

	len = bcm_rpc_buf_len_get(rpcb, b);

	if (len % 512 == 0) {
		len += 8;
		ASSERT(len < PKTBUFSZ);
	}

	err = dbus_send_buf(rpcb->bus, bcm_rpc_buf_data(rpcb, b), len, b);

	RPC_TP_LOCK(rpcb);
	if (err != 0) {
		printf("%s: dbus_send_buf failed\n", __FUNCTION__);
		rpcb->txerr_cnt++;
		rpcb->bus_txpending--;
	} else {
		rpcb->tx_cnt++;
	}
	RPC_TP_UNLOCK(rpcb);

	return err;
}


/* Buffer manipulation */

rpc_buf_t *
bcm_rpc_tp_buf_alloc(rpc_tp_info_t * rpcb, int len)
{
	rpc_buf_t* b;
#if defined(NDIS)
	struct lbuf *lb;

	if (len > LBDATASZ)
		return (NULL);

	RPC_TP_LOCK(rpcb);
	lb = shared_lb_get(&rpcb->pktfree);
	RPC_TP_UNLOCK(rpcb);
	if (lb != NULL)
		lb->len = len;

	b = (rpc_buf_t*)lb;
#else
	struct sk_buff *skb;

	if ((skb = dev_alloc_skb(len))) {
		skb_put(skb, len);
		skb->priority = 0;
	}

	b = (rpc_buf_t*)skb;
#endif /* NDIS */

	if (b != NULL) {
		RPC_TP_LOCK(rpcb);
		rpcb->bufalloc++;

		if (!rpcb->rxflowctrl && (rpcb->buf_cnt_inuse >= RPCRXLOWAT)) {
			rpcb->rxflowctrl = TRUE;
			dbus_flowctrl_rx(rpcb->bus, TRUE);
		}

		rpcb->buf_cnt_inuse++;

		if (rpcb->buf_cnt_inuse > (int)rpcb->buf_cnt_max)
			rpcb->buf_cnt_max = rpcb->buf_cnt_inuse;

		RPC_TP_UNLOCK(rpcb);
	} else {
		printf("%s: buf alloc failed buf_cnt_inuse %d rxflowctrl:%d\n",
		       __FUNCTION__, rpcb->buf_cnt_inuse, rpcb->rxflowctrl);
		ASSERT(0);
	}

	return b;
}

void
bcm_rpc_tp_buf_free(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
#if defined(NDIS)
	struct lbuf *lb = (struct lbuf*)b;

	ASSERT(rpcb);

	ASSERT(lb != NULL);
	ASSERT(lb->next == NULL);
	ASSERT(lb->l == &rpcb->pktfree);

	RPC_TP_LOCK(rpcb);
	shared_lb_put(lb->l, lb);

#else
	struct sk_buff *skb = (struct sk_buff*)b;

	if (skb->destructor) {
		/* cannot kfree_skb() on hard IRQ (net/core/skbuff.c) if destructor exists */
		dev_kfree_skb_any(skb);
	} else {
		/* can free immediately (even in_irq()) if destructor does not exist */
		dev_kfree_skb(skb);
	}

	RPC_TP_LOCK(rpcb);

#endif /* NDIS */

	if (rpcb->rxflowctrl && (rpcb->buf_cnt_inuse < RPCRXLOWAT)) {
		rpcb->rxflowctrl = FALSE;
		dbus_flowctrl_rx(rpcb->bus, FALSE);
	}

	rpcb->buf_cnt_inuse--;
	RPC_TP_UNLOCK(rpcb);
}

void
bcm_rpc_tp_down(rpc_tp_info_t *rpcb)
{
	dbus_down(rpcb->bus);
}

int
bcm_rpc_buf_len_get(rpc_tp_info_t * rpcb, rpc_buf_t* b)
{
	return PKTLEN(rpcb->osh, b);
}

int
bcm_rpc_buf_len_set(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint len)
{
	PKTSETLEN(rpcb->osh, b, len);
	return 0;
}

unsigned char*
bcm_rpc_buf_data(rpc_tp_info_t * rpcb, rpc_buf_t* b)
{
	return PKTDATA(rpcb->osh, b);
}

unsigned char*
bcm_rpc_buf_push(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint bytes)
{
	return PKTPUSH(rpcb->osh, b, bytes);
}

unsigned char*
bcm_rpc_buf_pull(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint bytes)
{
	return PKTPULL(rpcb->osh, b, bytes);
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

#if defined(WLC_HIGH) && defined(BCMDBG)
int
bcm_rpc_tp_dump(rpc_tp_info_t *rpcb, struct bcmstrbuf *b)
{
	int i = 0, j = 0;

	RPC_TP_LOCK(rpcb);

	bcm_bprintf(b, "\nRPC_TP_DBUS:\n");
	bcm_bprintf(b, "bufalloc %d(buf_inuse %d, max %d) tx %d(txerr %d) rx %d(rxdrop %d)\n",
		rpcb->bufalloc, rpcb->buf_cnt_inuse, rpcb->buf_cnt_max, rpcb->tx_cnt,
		rpcb->txerr_cnt, rpcb->rx_cnt, rpcb->rxdrop_cnt);

	bcm_bprintf(b, "mtu %d depth %d pending %d tx_flowctrl_cnt %d, rxflowctl %d\n",
	            rpcb->bus_mtu, rpcb->bus_txdepth, rpcb->bus_txpending, rpcb->tx_flowctl_cnt,
	            rpcb->rxflowctrl);

	bcm_bprintf(b, "tp_host_deagg chain %d subframes %d bytes %d badsflen %d\n",
		rpcb->tp_host_deagg_cnt_chain, rpcb->tp_host_deagg_cnt_sf,
		rpcb->tp_host_deagg_cnt_bytes,
		rpcb->tp_host_deagg_cnt_badsflen);
	bcm_bprintf(b, "tp_host_deagg byte-per-chain %d passthrough %d\n",
		CEIL(rpcb->tp_host_deagg_cnt_bytes, (rpcb->tp_host_deagg_cnt_chain + 1)),
		rpcb->tp_host_deagg_cnt_pass);

	bcm_bprintf(b, "\nhistogram\n");
	for (i = 0; i < DBUS_NTXQ; i++) {
		if (histogram_rpctp_dbus[i]) {
			bcm_bprintf(b, "%d: %d ", i, histogram_rpctp_dbus[i]);
			j++;
			if (j % 10 == 0) {
				bcm_bprintf(b, "\n");
			}
		}
	}
	bcm_bprintf(b, "\n");

	RPC_TP_UNLOCK(rpcb);

	return 0;
}
#endif /* BCMDBG */

#ifdef NDIS
void
bcm_rpc_tp_sleep(rpc_tp_info_t *rpcb)
{
	dbus_pnp_sleep(rpcb->bus);
}

int
bcm_rpc_tp_shutdown(rpc_tp_info_t *rpcb)
{
	return dbus_shutdown(rpcb->bus);
}

void
bcm_rpc_tp_resume(rpc_tp_info_t *rpcb)
{
	int reloaded = FALSE;
	dbus_pnp_resume(rpcb->bus, &reloaded);
	printf("%s: Image reloaded %d\n", __FUNCTION__, reloaded);
}

void
bcm_rpc_tp_surp_remove(rpc_tp_info_t * rpcb)
{
	dbus_pnp_disconnect(rpcb->bus);
}
#endif /* NDIS */
