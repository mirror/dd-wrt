/*
 * Dongle BUS interface for USB, SDIO, SPI, etc.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dbus.c,v 1.8.4.18 2008/10/14 21:54:31 Exp $
 */

#include "osl.h"
#include "dbus.h"
#ifdef BCM_DNGL_EMBEDIMAGE
#include "rtecdc.h"
#endif

/* General info for all BUS */
typedef struct dbus_irbq {
	dbus_irb_t *head;
	dbus_irb_t *tail;
	int cnt;
} dbus_irbq_t;

typedef struct {
	dbus_pub_t pub; /* MUST BE FIRST */

	void *cbarg;
	dbus_callbacks_t *cbs;
	void *bus_info;
	dbus_intf_t *drvintf;
	uint8 *fw;
	int fwlen;
	uint32 errmask;
	int rx_low_watermark;
	int tx_low_watermark;
	bool txoff;
	bool rxoff;
	bool tx_timer_ticking;

	dbus_irbq_t *rx_q;
	dbus_irbq_t *tx_q;
} dbus_info_t;

struct exec_parms {
union {
	/* Can consolidate same params, if need be, but this shows
	 * group of parameters per function
	 */
	struct {
		dbus_irbq_t *q;
		dbus_irb_t *b;
	} qenq;

	struct {
		dbus_irbq_t *q;
	} qdeq;
};
};

#define DBUS_NTXQ	256
#define DBUS_NRXQ	256
#define EXEC_RXLOCK(info, fn, a) \
	info->drvintf->exec_rxlock(dbus_info->bus_info, ((exec_cb_t)fn), ((struct exec_parms *) a))

#define EXEC_TXLOCK(info, fn, a) \
	info->drvintf->exec_txlock(dbus_info->bus_info, ((exec_cb_t)fn), ((struct exec_parms *) a))

/*
 * Callbacks common for all BUS
 */
static void dbus_if_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb);
static void dbus_if_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status);
static void dbus_if_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status);
static void dbus_if_errhandler(void *handle, int err);
static void dbus_if_ctl_complete(void *handle, int type, int status);
static void dbus_if_state_change(void *handle, int state);
static void *dbus_if_pktget(void *handle, uint len, bool send);
static void dbus_if_pktfree(void *handle, void *p, bool send);
static struct dbus_irb *dbus_if_getirb(void *cbarg, bool send);

static dbus_intf_callbacks_t dbus_intf_cbs = {
	dbus_if_send_irb_timeout,
	dbus_if_send_irb_complete,
	dbus_if_recv_irb_complete,
	dbus_if_errhandler,
	dbus_if_ctl_complete,
	dbus_if_state_change,
	NULL,			/* isr */
	NULL,			/* dpc */
	NULL,			/* watchdog */
	dbus_if_pktget,
	dbus_if_pktfree,
	dbus_if_getirb
};

/*
 * Need global for probe() and disconnect() since
 * attach() is not called at probe and detach()
 * can be called inside disconnect()
 */
static dbus_intf_t *g_busintf = NULL;
static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

static void  dbus_flowctrl_tx(dbus_info_t *dbus_info, bool onoff);
static void* q_enq(dbus_irbq_t *q, dbus_irb_t *b);
static void* q_enq_exec(struct exec_parms *args);
static dbus_irb_t*q_deq(dbus_irbq_t *q);
static void* q_deq_exec(struct exec_parms *args);
static int   dbus_tx_timer_init(dbus_info_t *dbus_info);
static int   dbus_tx_timer_start(dbus_info_t *dbus_info, uint timeout);
static int   dbus_tx_timer_stop(dbus_info_t *dbus_info);
static int   dbus_irbq_init(dbus_info_t *dbus_info, dbus_irbq_t *q, int nq, int size_irb);
static int   dbus_irbq_deinit(dbus_info_t *dbus_info, dbus_irbq_t *q, int size_irb);
static int   dbus_rxirbs_fill(dbus_info_t *dbus_info);
static int   dbus_send_irb(const dbus_pub_t *pub, uint8 *buf, int len, void *pkt, void *info);
static void  dbus_disconnect(void *handle);
static void *dbus_probe(void *arg, const char *desc, uint32 bustype, uint32 hdrlen);

/* function */
static void
dbus_flowctrl_tx(dbus_info_t *dbus_info, bool onoff)
{
	if (dbus_info == NULL)
		return;

	DBUSTRACE(("%s\n", __FUNCTION__));

	dbus_info->txoff = onoff;
	if (dbus_info->cbs && dbus_info->cbs->txflowcontrol)
		dbus_info->cbs->txflowcontrol(dbus_info->cbarg, onoff);
}

/*
 * q_enq()/q_deq() are executed with protection
 * via exec_rxlock()/exec_txlock()
 */
static void*
q_enq(dbus_irbq_t *q, dbus_irb_t *b)
{
	ASSERT(b->next == NULL);
	b->next = NULL;
	if (q->tail) {
		q->tail->next = b;
		q->tail = b;
	} else
		q->head = q->tail = b;

	q->cnt++;

	return b;
}

static void*
q_enq_exec(struct exec_parms *args)
{
	return q_enq(args->qenq.q, args->qenq.b);
}

static dbus_irb_t*
q_deq(dbus_irbq_t *q)
{
	dbus_irb_t *b;

	b = q->head;
	if (b) {
		q->head = q->head->next;
		b->next = NULL;

		if (q->head == NULL)
			q->tail = q->head;

		q->cnt--;
	}
	return b;
}

static void*
q_deq_exec(struct exec_parms *args)
{
	return q_deq(args->qdeq.q);
}

static int
dbus_tx_timer_init(dbus_info_t *dbus_info)
{
	if (dbus_info && dbus_info->drvintf && dbus_info->drvintf->tx_timer_init)
		return dbus_info->drvintf->tx_timer_init(dbus_info->bus_info);
	else
		return DBUS_ERR;
}

static int
dbus_tx_timer_start(dbus_info_t *dbus_info, uint timeout)
{
	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->tx_timer_ticking)
		return DBUS_OK;

	if (dbus_info->drvintf && dbus_info->drvintf->tx_timer_start) {
		if (dbus_info->drvintf->tx_timer_start(dbus_info->bus_info, timeout) == DBUS_OK) {
			dbus_info->tx_timer_ticking = TRUE;
			return DBUS_OK;
		}
	}

	return DBUS_ERR;
}

static int
dbus_tx_timer_stop(dbus_info_t *dbus_info)
{
	if (dbus_info == NULL)
		return DBUS_ERR;

	if (!dbus_info->tx_timer_ticking)
		return DBUS_OK;

	if (dbus_info->drvintf && dbus_info->drvintf->tx_timer_stop) {
		if (dbus_info->drvintf->tx_timer_stop(dbus_info->bus_info) == DBUS_OK) {
			dbus_info->tx_timer_ticking = FALSE;
			return DBUS_OK;
		}
	}

	return DBUS_ERR;
}

static int
dbus_irbq_init(dbus_info_t *dbus_info, dbus_irbq_t *q, int nq, int size_irb)
{
	int i;
	dbus_irb_t *irb;

	ASSERT(q);
	ASSERT(dbus_info);

	for (i = 0; i < nq; i++) {
		/* MALLOC dbus_irb_tx or dbus_irb_rx, but cast to simple dbus_irb_t linkedlist */
		irb = (dbus_irb_t *) MALLOC(dbus_info->pub.osh, size_irb);
		if (irb == NULL) {
			ASSERT(irb);
			return DBUS_ERR;
		}
		bzero(irb, size_irb);

		/* q_enq() does not need to go through EXEC_xxLOCK() during init() */
		q_enq(q, irb);
	}

	return DBUS_OK;
}

static int
dbus_irbq_deinit(dbus_info_t *dbus_info, dbus_irbq_t *q, int size_irb)
{
	dbus_irb_t *irb;

	ASSERT(q);
	ASSERT(dbus_info);

	/* q_deq() does not need to go through EXEC_xxLOCK()
	 * during deinit(); all callbacks are stopped by this time
	 */
	while ((irb = q_deq(q)) != NULL) {
		MFREE(dbus_info->pub.osh, irb, size_irb);
	}

	if (q->cnt)
		DBUSERR(("deinit: q->cnt=%d > 0\n", q->cnt));
	return DBUS_OK;
}

static int
dbus_rxirbs_fill(dbus_info_t *dbus_info)
{
	int err = DBUS_OK;
	dbus_irb_rx_t *rxirb;
	struct exec_parms args;

	ASSERT(dbus_info);
	if (dbus_info->pub.busstate != DBUS_STATE_UP) {
		DBUSERR(("dbus_rxirbs_fill: DBUS not up \n"));
		return DBUS_ERR;
	} else if (dbus_info->drvintf && (dbus_info->drvintf->recv_irb == NULL)) {
		/* Lower edge bus interface does not support recv_irb().
		 * No need to pre-submit IRBs in this case.
		 */
		return DBUS_ERR;
	}

	/* The dongle recv callback is freerunning without lock. So multiple callbacks(and this
	 *  refill) can run in parallel. While the rxoff condition is triggered outside,
	 *  below while loop has to check and abort posting more to avoid RPC rxq overflow.
	 */
	args.qdeq.q = dbus_info->rx_q;
	while ((!dbus_info->rxoff) &&
	       (rxirb = (EXEC_RXLOCK(dbus_info, q_deq_exec, &args))) != NULL) {
		err = dbus_info->drvintf->recv_irb(dbus_info->bus_info, rxirb);
		if (err != DBUS_OK) {
			/* Add the the free rxirb back to the queue
			 * and wait til later
			 */
			bzero(rxirb, sizeof(dbus_irb_rx_t));
			args.qenq.q = dbus_info->rx_q;
			args.qenq.b = (dbus_irb_t *) rxirb;
			EXEC_RXLOCK(dbus_info, q_enq_exec, &args);
			break;
		}
	}

	return err;
}

void
dbus_flowctrl_rx(const dbus_pub_t *pub, bool on)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if (dbus_info == NULL)
		return;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info->rxoff == on)
		return;

	dbus_info->rxoff = on;

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (!on) {
			/* post more irbs, resume rx if necessary */
			dbus_rxirbs_fill(dbus_info);
			if (dbus_info && dbus_info->drvintf->recv_resume) {
				dbus_info->drvintf->recv_resume(dbus_info->bus_info);
			}
		} else {
			/* ??? cancell posted irbs first */

			if (dbus_info && dbus_info->drvintf->recv_stop) {
				dbus_info->drvintf->recv_stop(dbus_info->bus_info);
			}
		}
	}
}

/* Handles both sending of a buffer or a pkt */
static int
dbus_send_irb(const dbus_pub_t *pub, uint8 *buf, int len, void *pkt, void *info)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	dbus_irb_tx_t *txirb;
	int txirb_pending;
	int err = DBUS_OK;
	struct exec_parms args;

	if (dbus_info == NULL)
		return DBUS_ERR;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		args.qdeq.q = dbus_info->tx_q;
		txirb = EXEC_TXLOCK(dbus_info, q_deq_exec, &args);

		if (txirb == NULL) {
			DBUSERR(("Out of tx dbus_bufs\n"));
			return DBUS_ERR;
		}

		if (pkt != NULL) {
			txirb->pkt = pkt;
		} else if (buf != NULL) {
			txirb->buf = buf;
			txirb->len = len;
		} else {
			ASSERT(0); /* Should not happen */
		}
		txirb->info = info;
		txirb->arg = NULL;
		txirb->retry_count = 0;

		if (dbus_info->drvintf && dbus_info->drvintf->send_irb) {
			err = dbus_info->drvintf->send_irb(dbus_info->bus_info, txirb);
			if (err != DBUS_OK) {
				DBUSERR(("ERROR: send_irb failed\n"));
				bzero(txirb, sizeof(dbus_irb_tx_t));
				args.qenq.q = dbus_info->tx_q;
				args.qenq.b = (dbus_irb_t *) txirb;
				EXEC_TXLOCK(dbus_info, q_enq_exec, &args);
			} else {
				dbus_tx_timer_start(dbus_info, DBUS_TX_TIMEOUT_INTERVAL);
				txirb_pending = dbus_info->pub.ntxq - dbus_info->tx_q->cnt;
				if (txirb_pending > (dbus_info->tx_low_watermark * 3)) {
					dbus_flowctrl_tx(dbus_info, ON);
				}
			}
		}
	}
	return err;
}

#ifdef BCM_DNGL_EMBEDIMAGE
static int
dbus_do_download(dbus_info_t *dbus_info)
{
	int err = DBUS_OK;

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->fw == NULL)
		return DBUS_ERR;

	if (dbus_info->drvintf->dlstart && dbus_info->drvintf->dlrun) {
		err = dbus_info->drvintf->dlstart(dbus_info->bus_info,
			dbus_info->fw, dbus_info->fwlen);

		if (err == DBUS_OK)
			err = dbus_info->drvintf->dlrun(dbus_info->bus_info);
	} else
		err = DBUS_ERR;

	return err;
}
#endif /* BCM_DNGL_EMBEDIMAGE */

static void
dbus_disconnect(void *handle)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (disconnect_cb)
		disconnect_cb(disc_arg);
	disc_arg = NULL;
	disconnect_cb = NULL;
}

/*
 * This function is called when the sent irb timesout without a tx response status.
 * DBUS adds reliability by resending timedout irbs DBUS_TX_RETRY_LIMIT times.
 */
static void
dbus_if_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;

	if ((dbus_info == NULL) || (dbus_info->drvintf == NULL) || (txirb == NULL)) {
		return;
	}

	DBUSTRACE(("%s\n", __FUNCTION__));

	return;


	if (dbus_info->drvintf->cancel_irb) {
		if (dbus_info->pub.busstate != DBUS_STATE_DOWN) {
			DBUSTRACE(("%s: cancelling timed out irb\n", __FUNCTION__));
			dbus_info->drvintf->cancel_irb(dbus_info->bus_info, txirb);
		}
	}

	if (txirb->retry_count < DBUS_TX_RETRY_LIMIT) {
		txirb->retry_count++;
		if (dbus_info->drvintf->send_irb) {
			if (dbus_info->drvintf->send_irb(dbus_info->bus_info, txirb) == DBUS_OK) {
				DBUSTRACE(("%s: resending cancelled irb\n", __FUNCTION__));
				dbus_tx_timer_start(dbus_info, DBUS_TX_TIMEOUT_INTERVAL);
			}
		}
	} else {
		if (dbus_info->cbs && dbus_info->cbs->send_complete) {
			DBUSTRACE(("%s: retry irb limit reached, quitting!\n", __FUNCTION__));
			dbus_info->cbs->send_complete(dbus_info->cbarg, txirb->info,
				DBUS_ERR_TXTIMEOUT);
		}
	}
}

static void
dbus_if_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;
	int txirb_pending;
	struct exec_parms args;

	if ((dbus_info == NULL) || (txirb == NULL)) {
		return;
	}

	DBUSTRACE(("%s: status = %d\n", __FUNCTION__, status));

	dbus_tx_timer_stop(dbus_info);

	if (dbus_info->pub.busstate != DBUS_STATE_DOWN) {
		if ((status == DBUS_OK) || (status == DBUS_ERR_NODEVICE)) {
			if (dbus_info->cbs && dbus_info->cbs->send_complete)
				dbus_info->cbs->send_complete(dbus_info->cbarg, txirb->info,
					status);

			if (status == DBUS_OK) {
				txirb_pending = dbus_info->pub.ntxq - dbus_info->tx_q->cnt;
				if (txirb_pending)
					dbus_tx_timer_start(dbus_info, DBUS_TX_TIMEOUT_INTERVAL);
				if ((txirb_pending < dbus_info->tx_low_watermark) &&
					dbus_info->txoff) {
					dbus_flowctrl_tx(dbus_info, OFF);
				}
			}
		}
	}

	bzero(txirb, sizeof(dbus_irb_tx_t));
	args.qenq.q = dbus_info->tx_q;
	args.qenq.b = (dbus_irb_t *) txirb;
	EXEC_TXLOCK(dbus_info, q_enq_exec, &args);
}

static void
dbus_if_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;
	int rxirb_pending;
	struct exec_parms args;

	if ((dbus_info == NULL) || (rxirb == NULL)) {
		return;
	}

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info->pub.busstate != DBUS_STATE_DOWN) {
		if (status == DBUS_OK) {
			if ((rxirb->buf != NULL) && (rxirb->actual_len > 0)) {
				if (dbus_info->cbs && dbus_info->cbs->recv_buf)
					dbus_info->cbs->recv_buf(dbus_info->cbarg, rxirb->buf,
					rxirb->actual_len);
			} else if (rxirb->pkt != NULL) {
				if (dbus_info->cbs && dbus_info->cbs->recv_pkt)
					dbus_info->cbs->recv_pkt(dbus_info->cbarg, rxirb->pkt);
			} else {
				ASSERT(0); /* Should not happen */
			}

			rxirb_pending = dbus_info->pub.nrxq - dbus_info->rx_q->cnt - 1;
			if ((rxirb_pending <= dbus_info->rx_low_watermark) &&
				!dbus_info->rxoff) {
				DBUSTRACE(("Low watermark so submit more %d <= %d \n",
					dbus_info->rx_low_watermark, rxirb_pending));

				dbus_rxirbs_fill(dbus_info);
			} else if (dbus_info->rxoff)
				DBUSTRACE(("rx flow controlled. not filling more. cut_rxq=%d\n",
					dbus_info->rx_q->cnt));
		} else if (status == DBUS_ERR_NODEVICE) {
			DBUSTRACE(("%s: status = %d\n", __FUNCTION__, status));
		} else {
			DBUSERR(("%s: status = %d\n", __FUNCTION__, status));
		}
	} else {
		DBUSTRACE(("%s: DBUS down, ignoring recv callback\n", __FUNCTION__));
	}

	bzero(rxirb, sizeof(dbus_irb_rx_t));
	args.qenq.q = dbus_info->rx_q;
	args.qenq.b = (dbus_irb_t *) rxirb;
	EXEC_RXLOCK(dbus_info, q_enq_exec, &args);
}

static void
dbus_if_errhandler(void *handle, int err)
{
	dbus_info_t *dbus_info = handle;
	uint32 mask = 0;

	if (dbus_info == NULL)
		return;

	switch (err) {
		case DBUS_ERR_TXFAIL:
			dbus_info->pub.stats.tx_errors++;
			mask |= ERR_CBMASK_TXFAIL;
			break;
		case DBUS_ERR_TXDROP:
			dbus_info->pub.stats.tx_dropped++;
			mask |= ERR_CBMASK_TXFAIL;
			break;
		case DBUS_ERR_RXFAIL:
			dbus_info->pub.stats.rx_errors++;
			mask |= ERR_CBMASK_RXFAIL;
			break;
		case DBUS_ERR_RXDROP:
			dbus_info->pub.stats.rx_dropped++;
			mask |= ERR_CBMASK_RXFAIL;
			break;
		default:
			break;
	}

	if (dbus_info->cbs && dbus_info->cbs->errhandler && (dbus_info->errmask & mask))
		dbus_info->cbs->errhandler(dbus_info->cbarg, err);
}

static void
dbus_if_ctl_complete(void *handle, int type, int status)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;

	if (dbus_info == NULL)
		return;

	if (dbus_info->cbs && dbus_info->cbs->ctl_complete)
		dbus_info->cbs->ctl_complete(dbus_info->cbarg, type, status);
}

static void
dbus_if_state_change(void *handle, int state)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;

	if (dbus_info == NULL)
		return;

	if (dbus_info->pub.busstate == state)
		return;

	if (state == DBUS_STATE_DISCONNECT) {
		DBUSERR(("DBUS disconnected\n"));
	}

	/* Don't update state if it's PnP firmware re-download */
	if (state != DBUS_STATE_PNP_FWDL)
		dbus_info->pub.busstate = state;

	if (dbus_info->cbs && dbus_info->cbs->state_change)
		dbus_info->cbs->state_change(dbus_info->cbarg, state);
}

static void *
dbus_if_pktget(void *handle, uint len, bool send)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;
	void *p = NULL;

	if (dbus_info == NULL)
		return NULL;

	if (dbus_info->cbs && dbus_info->cbs->pktget)
		p = dbus_info->cbs->pktget(dbus_info->cbarg, len, send);
	else
		ASSERT(0);

	return p;
}

static void
dbus_if_pktfree(void *handle, void *p, bool send)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;

	if (dbus_info == NULL)
		return;

	if (dbus_info->cbs && dbus_info->cbs->pktfree)
		dbus_info->cbs->pktfree(dbus_info->cbarg, p, send);
	else
		ASSERT(0);
}

static struct dbus_irb*
dbus_if_getirb(void *cbarg, bool send)
{
	dbus_info_t *dbus_info = (dbus_info_t *) cbarg;
	struct exec_parms args;
	struct dbus_irb *irb;

	if ((dbus_info == NULL) || (dbus_info->pub.busstate != DBUS_STATE_UP))
		return NULL;

	if (send == TRUE) {
		args.qdeq.q = dbus_info->tx_q;
		irb = EXEC_TXLOCK(dbus_info, q_deq_exec, &args);
	} else {
		args.qdeq.q = dbus_info->rx_q;
		irb = EXEC_RXLOCK(dbus_info, q_deq_exec, &args);
	}

	return irb;
}

static void *
dbus_probe(void *arg, const char *desc, uint32 bustype, uint32 hdrlen)
{
	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, desc, bustype, hdrlen);
		return disc_arg;
	}

	return (void *)DBUS_ERR;
}

int
dbus_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, void *param1, void *param2)
{
	int err;

	DBUSTRACE(("%s\n", __FUNCTION__));

	probe_cb = prcb;
	disconnect_cb = discb;
	probe_arg = prarg;

	err = dbus_bus_register(vid, pid, dbus_probe,
		dbus_disconnect, NULL, &g_busintf, param1, param2);

	return err;
}

int
dbus_deregister()
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	return dbus_bus_deregister();
}

const dbus_pub_t *
dbus_attach(osl_t *osh, int rxsize, int nrxq, int ntxq, void *cbarg, dbus_callbacks_t *cbs)
{
	dbus_info_t *dbus_info;
	int err;

	if ((g_busintf == NULL) || (g_busintf->attach == NULL) || (cbs == NULL))
		return NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	dbus_info = MALLOC(osh, sizeof(dbus_info_t));
	if (dbus_info == NULL)
		return NULL;

	bzero(dbus_info, sizeof(dbus_info_t));

	/* BUS-specific driver interface */
	dbus_info->drvintf = g_busintf;
	dbus_info->cbarg = cbarg;
	dbus_info->cbs = cbs;

	dbus_info->pub.osh = osh;
	dbus_info->pub.rxsize = rxsize;

	if (nrxq <= 0)
		nrxq = DBUS_NRXQ;
	if (ntxq <= 0)
		ntxq = DBUS_NTXQ;

	dbus_info->pub.nrxq = nrxq;
	dbus_info->rx_low_watermark = nrxq / 4;
	dbus_info->pub.ntxq = ntxq;
	dbus_info->tx_low_watermark = ntxq / 4;

	dbus_info->tx_q = MALLOC(osh, sizeof(dbus_irbq_t));
	if (dbus_info->tx_q == NULL)
		goto error;
	else {
		bzero(dbus_info->tx_q, sizeof(dbus_irbq_t));
		err = dbus_irbq_init(dbus_info, dbus_info->tx_q, ntxq, sizeof(dbus_irb_tx_t));
		if (err != DBUS_OK)
			goto error;
	}

	dbus_info->rx_q = MALLOC(osh, sizeof(dbus_irbq_t));
	if (dbus_info->rx_q == NULL)
		goto error;
	else {
		bzero(dbus_info->rx_q, sizeof(dbus_irbq_t));
		err = dbus_irbq_init(dbus_info, dbus_info->rx_q, nrxq, sizeof(dbus_irb_rx_t));
		if (err != DBUS_OK)
			goto error;
	}

	dbus_info->bus_info = (void *)g_busintf->attach(&dbus_info->pub,
		dbus_info, &dbus_intf_cbs);
	if (dbus_info->bus_info == NULL)
		goto error;

	dbus_tx_timer_init(dbus_info);

	/* Use default firmware */
#ifdef BCM_DNGL_EMBEDIMAGE
	dbus_info->fw = (uint8 *) dlarray;
	dbus_info->fwlen = sizeof(dlarray);

	if (dbus_info->drvintf->dlneeded) {
		if (dbus_info->drvintf->dlneeded(dbus_info->bus_info)) {
			err = dbus_do_download(dbus_info);
			if (err == DBUS_ERR) {
				DBUSERR(("attach: download failed=%d\n", err));
				goto error;
			}
		}
	}
#endif

	return (dbus_pub_t *)dbus_info;
error:
	if (dbus_info) {
		if (dbus_info->tx_q) {
			dbus_irbq_deinit(dbus_info, dbus_info->tx_q, sizeof(dbus_irb_tx_t));
			MFREE(osh, dbus_info->tx_q, sizeof(dbus_irbq_t));
			dbus_info->tx_q = NULL;
		}

		if (dbus_info->rx_q) {
			dbus_irbq_deinit(dbus_info, dbus_info->rx_q, sizeof(dbus_irb_rx_t));
			MFREE(osh, dbus_info->rx_q, sizeof(dbus_irbq_t));
			dbus_info->rx_q = NULL;
		}

		MFREE(osh, dbus_info, sizeof(dbus_info_t));
	}

	return NULL;
}

void
dbus_detach(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	osl_t *osh;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return;

	dbus_tx_timer_stop(dbus_info);

	osh = pub->osh;

	if (dbus_info->drvintf && dbus_info->drvintf->detach)
		 dbus_info->drvintf->detach((dbus_pub_t *)dbus_info, dbus_info->bus_info);

	if (dbus_info->tx_q) {
		dbus_irbq_deinit(dbus_info, dbus_info->tx_q, sizeof(dbus_irb_tx_t));
		MFREE(osh, dbus_info->tx_q, sizeof(dbus_irbq_t));
		dbus_info->tx_q = NULL;
	}

	if (dbus_info->rx_q) {
		dbus_irbq_deinit(dbus_info, dbus_info->rx_q, sizeof(dbus_irb_rx_t));
		MFREE(osh, dbus_info->rx_q, sizeof(dbus_irbq_t));
		dbus_info->rx_q = NULL;
	}

	MFREE(osh, dbus_info, sizeof(dbus_info_t));
}

int
dbus_up(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	if ((dbus_info->pub.busstate == DBUS_STATE_DL_DONE) ||
		(dbus_info->pub.busstate == DBUS_STATE_DOWN)) {
		if (dbus_info->drvintf && dbus_info->drvintf->up) {
			err = dbus_info->drvintf->up(dbus_info->bus_info);

			if (err == DBUS_OK) {
				dbus_rxirbs_fill(dbus_info);
			}
		}
	} else
		err = DBUS_ERR;

	return err;
}

int
dbus_down(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (dbus_info->drvintf && dbus_info->drvintf->down)
			return dbus_info->drvintf->down(dbus_info->bus_info);
	}

	return DBUS_ERR;
}

int
dbus_shutdown(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->drvintf && dbus_info->drvintf->shutdown)
		return dbus_info->drvintf->shutdown(dbus_info->bus_info);

	return DBUS_ERR;
}

int
dbus_stop(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (dbus_info->drvintf && dbus_info->drvintf->stop)
			return dbus_info->drvintf->stop(dbus_info->bus_info);
	}

	return DBUS_ERR;
}

int
dbus_send_buf(const dbus_pub_t *pub, uint8 *buf, int len, void *info)
{
	return dbus_send_irb(pub, buf, len, NULL, info);
}

int
dbus_send_pkt(const dbus_pub_t *pub, void *pkt, void *info)
{
	return dbus_send_irb(pub, NULL, 0, pkt, info);
}


int
dbus_send_ctl(const dbus_pub_t *pub, uint8 *buf, int len)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (dbus_info->drvintf && dbus_info->drvintf->send_ctl)
			return dbus_info->drvintf->send_ctl(dbus_info->bus_info, buf, len);
	}

	return DBUS_ERR;
}

int
dbus_recv_ctl(const dbus_pub_t *pub, uint8 *buf, int len)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if ((dbus_info == NULL) || (buf == NULL))
		return DBUS_ERR;

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (dbus_info->drvintf && dbus_info->drvintf->recv_ctl)
			return dbus_info->drvintf->recv_ctl(dbus_info->bus_info, buf, len);
	}

	return DBUS_ERR;
}

void *
dbus_pktget(const dbus_pub_t *pub, int len)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if ((dbus_info == NULL) || (len < 0))
		return NULL;

	return PKTGET(dbus_info->pub.osh, len, TRUE);
}

void
dbus_pktfree(const dbus_pub_t *pub, void* pkt)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if ((dbus_info == NULL) || (pkt == NULL))
		return;

	PKTFREE(dbus_info->pub.osh, pkt, TRUE);
}

int
dbus_get_stats(const dbus_pub_t *pub, dbus_stats_t *stats)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if ((dbus_info == NULL) || (stats == NULL))
		return DBUS_ERR;

	bcopy(&dbus_info->pub.stats, stats, sizeof(dbus_stats_t));

	return DBUS_OK;
}

int
dbus_get_attrib(const dbus_pub_t *pub, dbus_attrib_t *attrib)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	if ((dbus_info == NULL) || (attrib == NULL))
		return DBUS_ERR;

	if (dbus_info->drvintf && dbus_info->drvintf->get_attrib) {
		err = dbus_info->drvintf->get_attrib(dbus_info->bus_info,
		&dbus_info->pub.attrib);
	}

	bcopy(&dbus_info->pub.attrib, attrib, sizeof(dbus_attrib_t));
	return err;
}

int
dbus_set_config(const dbus_pub_t *pub, dbus_config_t *config)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	if ((dbus_info == NULL) || (config == NULL))
		return DBUS_ERR;

	if (dbus_info->drvintf && dbus_info->drvintf->set_config) {
		err = dbus_info->drvintf->set_config(dbus_info->bus_info,
		config);
	}

	return err;
}

int
dbus_get_config(const dbus_pub_t *pub, dbus_config_t *config)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	if ((dbus_info == NULL) || (config == NULL))
		return DBUS_ERR;

	if (dbus_info->drvintf && dbus_info->drvintf->get_config) {
		err = dbus_info->drvintf->get_config(dbus_info->bus_info,
		config);
	}

	return err;
}

int
dbus_set_errmask(const dbus_pub_t *pub, uint32 mask)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_OK;

	if (dbus_info == NULL)
		return DBUS_ERR;

	dbus_info->errmask = mask;
	return err;
}

int
dbus_pnp_resume(const dbus_pub_t *pub, int *fw_reload)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;
	bool fwdl = FALSE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

#ifdef BCM_DNGL_EMBEDIMAGE
	if (dbus_info->drvintf->dlneeded) {
		if (dbus_info->drvintf->dlneeded(dbus_info->bus_info)) {
			err = dbus_do_download(dbus_info);
			if (err == DBUS_OK) {
				fwdl = TRUE;
			}
			if (dbus_info->pub.busstate == DBUS_STATE_DL_DONE)
				dbus_up(&dbus_info->pub);

		}
	}
#endif

	if (dbus_info->drvintf->pnp) {
		err = dbus_info->drvintf->pnp(dbus_info->bus_info,
			DBUS_PNP_RESUME);
	}

	if (dbus_info->drvintf->recv_needed) {
		if (dbus_info->drvintf->recv_needed(dbus_info->bus_info)) {
			/* Refill after sleep/hibernate */
			dbus_rxirbs_fill(dbus_info);
		}
	}

	if (fwdl == TRUE) {
		dbus_if_state_change(dbus_info, DBUS_STATE_PNP_FWDL);
	}

	if (fw_reload)
		*fw_reload = fwdl;

	return err;
}

int
dbus_pnp_sleep(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	dbus_tx_timer_stop(dbus_info);

	if (dbus_info->drvintf && dbus_info->drvintf->pnp) {
		err = dbus_info->drvintf->pnp(dbus_info->bus_info,
			DBUS_PNP_SLEEP);
	}

	return err;
}

int
dbus_pnp_disconnect(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	dbus_tx_timer_stop(dbus_info);

	if (dbus_info->drvintf && dbus_info->drvintf->pnp) {
		err = dbus_info->drvintf->pnp(dbus_info->bus_info,
			DBUS_PNP_DISCONNECT);
	}

	return err;
}

int
dbus_iovar_op(const dbus_pub_t *pub, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;
	int err = DBUS_ERR;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (dbus_info == NULL)
		return DBUS_ERR;

	if (dbus_info->drvintf && dbus_info->drvintf->iovar_op) {
		err = dbus_info->drvintf->iovar_op(dbus_info->bus_info,
			name, params, plen, arg, len, set);
	}

	return err;
}
