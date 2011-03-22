/*
 * Dongle BUS interface for USB, SDIO, SPI, etc.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dbus.c,v 1.44.6.34 2010/07/14 09:38:49 Exp $
 */

#include "osl.h"
#include "dbus.h"
#ifdef BCMEMBEDIMAGE
#include BCMEMBEDIMAGE
#elif defined(BCM_DNGL_EMBEDIMAGE)
#ifdef EMBED_IMAGE_4322
#include "rtecdc_4322.h"
#endif /* EMBED_IMAGE_4322 */
#ifdef EMBED_IMAGE_43236a0
#include "rtecdc_43236a0.h"
#endif /* EMBED_IMAGE_43236a0 */
#ifdef EMBED_IMAGE_43236a1
#include "rtecdc_43236a1.h"
#endif /* EMBED_IMAGE_43236a1 */
#ifdef EMBED_IMAGE_43236b0
#include "rtecdc_43236b0.h"
#endif /* EMBED_IMAGE_43236b0 */
#ifdef EMBED_IMAGE_4319usb
#include "rtecdc_4319usb.h"
#endif /* EMBED_IMAGE_4319usb */
#ifdef EMBED_IMAGE_GENERIC
#include "rtecdc.h"
#endif
#endif /* BCMEMBEDIMAGE */

#include <bcmutils.h>
#if (defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)) && (defined(NDIS) || \
	defined(USBAP))
#include <bcmsrom_fmt.h>
#include <trxhdr.h>
#include <usbrdl.h>
#include <bcmendian.h>
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE) && defined(NDIS) */

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
	bool txoverride;
	bool rxoff;
	bool tx_timer_ticking;

	dbus_irbq_t *rx_q;
	dbus_irbq_t *tx_q;

#ifdef BCMDBG
	int *txpend_q_hist;
	int *rxpend_q_hist;
#endif /* BCMDBG */
	uint8 *nvram;
	int	nvram_len;
	uint8 *image;	/* buffer for combine fw and nvram */
	int image_len;
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
static void dbus_if_rxerr_indicate(void *handle, bool on);

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
	dbus_if_getirb,
	dbus_if_rxerr_indicate
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

#if (defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)) && (defined(NDIS) || \
	defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW))
char BCMATTACHDATA(mfgsromvars)[VARS_MAX];
int defvarslen = 0;
#endif /* (BCM_DNGL_EMBEDIMAGE || BCMEMBEDIMAGE) && (NDIS || USBAP) &&
	* (WLC_HIGH && !WLC_LOW)
	*/

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

#if (defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)) && (defined(NDIS) || \
	defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW))
static int dbus_get_nvram(dbus_info_t *dbus_info);
#endif /* (BCM_DNGL_EMBEDIMAGE || BCMEMBEDIMAGE) && (NDIS || USBAP) &&
	* (WLC_HIGH && !WLC_LOW)
	*/

/* function */
static void
dbus_flowctrl_tx(dbus_info_t *dbus_info, bool on)
{
	if (dbus_info == NULL)
		return;

	DBUSTRACE(("%s on %d\n", __FUNCTION__, on));

	if (dbus_info->txoff == on)
		return;

	dbus_info->txoff = on;

	if (dbus_info->cbs && dbus_info->cbs->txflowcontrol)
		dbus_info->cbs->txflowcontrol(dbus_info->cbarg, on);
}

static void
dbus_if_rxerr_indicate(void *handle, bool on)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;

	DBUSTRACE(("%s, on %d\n", __FUNCTION__, on));

	if (dbus_info == NULL)
		return;

	if (dbus_info->txoverride == on)
		return;

	dbus_info->txoverride = on;

	if (!on)
		dbus_rxirbs_fill(dbus_info);

}

/*
 * q_enq()/q_deq() are executed with protection
 * via exec_rxlock()/exec_txlock()
 */
static void*
q_enq(dbus_irbq_t *q, dbus_irb_t *b)
{
	ASSERT(q->tail != b);
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
		if (err == DBUS_ERR_RXDROP) {
			/* Add the the free rxirb back to the queue
			 * and wait till later
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
			txirb->buf = NULL;
			txirb->len = 0;
		} else if (buf != NULL) {
			txirb->pkt = NULL;
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
			if (err == DBUS_ERR_TXDROP) {
				/* tx fail and no completion routine to clean up, reclaim irb NOW */
				DBUSERR(("%s: send_irb failed, status = %d\n", __FUNCTION__, err));
				bzero(txirb, sizeof(dbus_irb_tx_t));
				args.qenq.q = dbus_info->tx_q;
				args.qenq.b = (dbus_irb_t *) txirb;
				EXEC_TXLOCK(dbus_info, q_enq_exec, &args);
			} else {
				dbus_tx_timer_start(dbus_info, DBUS_TX_TIMEOUT_INTERVAL);
				txirb_pending = dbus_info->pub.ntxq - dbus_info->tx_q->cnt;
#ifdef BCMDBG
				dbus_info->txpend_q_hist[txirb_pending]++;
#endif /* BCMDBG */
				if (txirb_pending > (dbus_info->tx_low_watermark * 3)) {
					dbus_flowctrl_tx(dbus_info, TRUE);
				}
			}
		}
	} else {
		err = DBUS_ERR_TXFAIL;
		DBUSTRACE(("%s: bus down, send_irb failed\n", __FUNCTION__));
	}

	return err;
}

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)
#if (defined(NDIS) || defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW))
static int check_file(osl_t *osh, unsigned char *headers);
static int
dbus_get_nvram(dbus_info_t *dbus_info)
{
	int len, i;
	struct trx_header *hdr;
	int	actual_fwlen;

	dbus_info->nvram_len = 0;
	if (defvarslen) {
		dbus_info->nvram = mfgsromvars;
		dbus_info->nvram_len = defvarslen;
	}
	if (dbus_info->nvram) {
		/* Validate the format/length etc of the file */
		if ((actual_fwlen = check_file(dbus_info->pub.osh, dbus_info->fw)) <= 0) {
			DBUSERR(("%s: bad firmware format!\n", __FUNCTION__));
			return DBUS_ERR_NVRAM;
		}
		len = actual_fwlen + dbus_info->nvram_len;
#ifdef USBAP
		/* Allocate virtual memory otherwise it might fail on embedded systems */
		dbus_info->image = VMALLOC(dbus_info->pub.osh, len);
#else
		dbus_info->image = MALLOC(dbus_info->pub.osh, len);
#endif /* USBAP */
		dbus_info->image_len = len;
		if (dbus_info->image == NULL) {
			DBUSERR(("%s: malloc failed!\n", __FUNCTION__));
			return DBUS_ERR_NVRAM;
		}
		bcopy(dbus_info->fw, dbus_info->image, actual_fwlen);
		bcopy(dbus_info->nvram, (uint8 *)(dbus_info->image + actual_fwlen),
			dbus_info->nvram_len);
		/* update TRX header for nvram size */
		hdr = (struct trx_header *)dbus_info->image;
		hdr->len = htol32(len);
		/* Pass the actual fw len */
		hdr->offsets[TRX_OFFSETS_NVM_LEN_IDX] = htol32(dbus_info->nvram_len);
		/* Calculate CRC over header */
		hdr->crc32 = hndcrc32((uint8 *)&hdr->flag_version,
			sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
			CRC32_INIT_VALUE);

		/* Calculate CRC over data */
		for (i = sizeof(struct trx_header); i < len; ++i)
				hdr->crc32 = hndcrc32((uint8 *)&dbus_info->image[i], 1, hdr->crc32);
		hdr->crc32 = htol32(hdr->crc32);
	} else {
		dbus_info->image = dbus_info->fw;
		dbus_info->image_len = (uint32)dbus_info->fwlen;
	}
	return DBUS_OK;
}
#endif /* (defined(NDIS) || defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW)) */

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
			dbus_info->image, dbus_info->image_len);

		if (err == DBUS_OK)
			err = dbus_info->drvintf->dlrun(dbus_info->bus_info);
	} else
		err = DBUS_ERR;
#if defined(NDIS) || defined(USBAP)
	if (dbus_info->nvram) {
#ifdef USBAP
		VFREE(dbus_info->pub.osh, dbus_info->image, dbus_info->image_len);
#else
		MFREE(dbus_info->pub.osh, dbus_info->image, dbus_info->image_len);
#endif /* USBAP */
	}
#endif /* NDIS || USBAP */
	return err;
}
#endif /* BCM_DNGL_EMBEDIMAGE */

static void
dbus_disconnect(void *handle)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (disconnect_cb)
		disconnect_cb(disc_arg);
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

static void BCMFASTPATH
dbus_if_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;
	int txirb_pending;
	struct exec_parms args;
	void *pktinfo;

	if ((dbus_info == NULL) || (txirb == NULL)) {
		return;
	}

	DBUSTRACE(("%s: status = %d\n", __FUNCTION__, status));

	dbus_tx_timer_stop(dbus_info);

	/* re-queue BEFORE calling send_complete which will assume that this irb 
	   is now available.
	 */
	pktinfo = txirb->info;
	bzero(txirb, sizeof(dbus_irb_tx_t));
	args.qenq.q = dbus_info->tx_q;
	args.qenq.b = (dbus_irb_t *) txirb;
	EXEC_TXLOCK(dbus_info, q_enq_exec, &args);

	if (dbus_info->pub.busstate != DBUS_STATE_DOWN) {
		if ((status == DBUS_OK) || (status == DBUS_ERR_NODEVICE)) {
			if (dbus_info->cbs && dbus_info->cbs->send_complete)
				dbus_info->cbs->send_complete(dbus_info->cbarg, pktinfo,
					status);

			if (status == DBUS_OK) {
				txirb_pending = dbus_info->pub.ntxq - dbus_info->tx_q->cnt;
				if (txirb_pending)
					dbus_tx_timer_start(dbus_info, DBUS_TX_TIMEOUT_INTERVAL);
				if ((txirb_pending < dbus_info->tx_low_watermark) &&
					dbus_info->txoff && !dbus_info->txoverride) {
					dbus_flowctrl_tx(dbus_info, OFF);
				}
			}
		} else {
			DBUSERR(("%s: %d WARNING freeing orphan pkt %p\n", __FUNCTION__, __LINE__,
				pktinfo));
#if defined(BCM_RPC_NOCOPY) || defined(BCM_RPC_TXNOCOPY) || defined(BCM_RPC_TOC)
			if (pktinfo)
				if (dbus_info->cbs && dbus_info->cbs->send_complete)
					dbus_info->cbs->send_complete(dbus_info->cbarg, pktinfo,
						status);
#else
			dbus_if_pktfree(dbus_info, (void*)pktinfo, TRUE);
#endif /* BCM_RPC_NOCOPY || BCM_RPC_TXNOCOPY || BCM_RPC_TOC */
		}
	} else {
		DBUSERR(("%s: %d WARNING freeing orphan pkt %p\n", __FUNCTION__, __LINE__,
			pktinfo));
#if defined(BCM_RPC_NOCOPY) || defined(BCM_RPC_TXNOCOPY) || defined(BCM_RPC_TOC)
		if (pktinfo)
			if (dbus_info->cbs && dbus_info->cbs->send_complete)
				dbus_info->cbs->send_complete(dbus_info->cbarg, pktinfo, status);
#else
		dbus_if_pktfree(dbus_info, (void*)pktinfo, TRUE);
#endif /* BCM_RPC_NOCOPY || BCM_RPC_TXNOCOPY || BCM_RPC_TOC */
	}
}

static void BCMFASTPATH
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
#ifdef BCMDBG
			dbus_info->rxpend_q_hist[rxirb_pending]++;
#endif /* BCMDBG */
			if ((rxirb_pending <= dbus_info->rx_low_watermark) &&
				!dbus_info->rxoff) {
				DBUSTRACE(("Low watermark so submit more %d <= %d \n",
					dbus_info->rx_low_watermark, rxirb_pending));

				dbus_rxirbs_fill(dbus_info);
			} else if (dbus_info->rxoff)
				DBUSTRACE(("rx flow controlled. not filling more. cut_rxq=%d\n",
					dbus_info->rx_q->cnt));
		} else if (status == DBUS_ERR_NODEVICE) {
			DBUSERR(("%s: %d status = %d, buf %p\n", __FUNCTION__, __LINE__, status,
				rxirb->buf));
#if defined(BCM_RPC_NOCOPY) || defined(BCM_RPC_TXNOCOPY) || defined(BCM_RPC_TOC)
			PKTFRMNATIVE(dbus_info->pub.osh, rxirb->buf);
			PKTFREE(dbus_info->pub.osh, rxirb->buf, FALSE);
#endif /* BCM_RPC_NOCOPY || BCM_RPC_TXNOCOPY || BCM_RPC_TOC */
		} else {
			DBUSERR(("%s: %d status = %d, buf %p\n", __FUNCTION__, __LINE__, status,
				rxirb->buf));
#if defined(BCM_RPC_NOCOPY) || defined(BCM_RPC_TXNOCOPY) || defined(BCM_RPC_TOC)
			if (rxirb->buf) {
				PKTFRMNATIVE(dbus_info->pub.osh, rxirb->buf);
				PKTFREE(dbus_info->pub.osh, rxirb->buf, FALSE);
			}
#endif /* BCM_RPC_NOCOPY || BCM_RPC_TXNOCOPY || BCM_RPC_TOC */
		}
	} else {
		DBUSTRACE(("%s: DBUS down, ignoring recv callback. buf %p\n", __FUNCTION__,
			rxirb->buf));
#if defined(BCM_RPC_NOCOPY) || defined(BCM_RPC_TXNOCOPY) || defined(BCM_RPC_TOC)
		if (rxirb->buf) {
			PKTFRMNATIVE(dbus_info->pub.osh, rxirb->buf);
			PKTFREE(dbus_info->pub.osh, rxirb->buf, FALSE);
		}
#endif /* BCM_RPC_NOCOPY || BCM_RPC_TXNOCOPY || BCM_RPC_TOC */
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

	if (dbus_info->pub.busstate != DBUS_STATE_DOWN) {
		if (dbus_info->cbs && dbus_info->cbs->ctl_complete)
			dbus_info->cbs->ctl_complete(dbus_info->cbarg, type, status);
	}
}

static void
dbus_if_state_change(void *handle, int state)
{
	dbus_info_t *dbus_info = (dbus_info_t *) handle;
	int old_state;

	if (dbus_info == NULL)
		return;

	if (dbus_info->pub.busstate == state)
		return;
	old_state = dbus_info->pub.busstate;
	if (state == DBUS_STATE_DISCONNECT) {
		DBUSERR(("DBUS disconnected\n"));
	}

	DBUSTRACE(("dbus state change from %d to to %d\n", old_state, state));

	/* Don't update state if it's PnP firmware re-download */
	if (state != DBUS_STATE_PNP_FWDL)
		dbus_info->pub.busstate = state;
	if (state == DBUS_STATE_SLEEP)
		dbus_flowctrl_rx(handle, TRUE);
	if ((old_state  == DBUS_STATE_SLEEP) && (state == DBUS_STATE_UP)) {
		dbus_rxirbs_fill(dbus_info);
		dbus_flowctrl_rx(handle, FALSE);
	}

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
	int ret;

	DBUSTRACE(("%s\n", __FUNCTION__));

	ret = dbus_bus_deregister();
	probe_cb = NULL;
	disconnect_cb = NULL;
	probe_arg = NULL;

	return ret;

}

const dbus_pub_t *
dbus_attach(osl_t *osh, int rxsize, int nrxq, int ntxq, void *cbarg,
	dbus_callbacks_t *cbs, struct shared_info *sh)
{
	dbus_info_t *dbus_info;
	unsigned int devid = 0;
	int err;

	devid = devid; /* avoid unused variable warning */

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

	dbus_info->pub.sh = sh;
	dbus_info->pub.osh = osh;
	dbus_info->pub.rxsize = rxsize;

	if (nrxq <= 0)
		nrxq = DBUS_NRXQ;
	if (ntxq <= 0)
		ntxq = DBUS_NTXQ;

	dbus_info->pub.nrxq = nrxq;
	dbus_info->rx_low_watermark = nrxq / 2;	/* keep enough posted rx urbs */
	dbus_info->pub.ntxq = ntxq;
	dbus_info->tx_low_watermark = ntxq / 4;	/* flow control when too many tx urbs posted */

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

#ifdef BCMDBG
	dbus_info->txpend_q_hist = MALLOC(osh, dbus_info->pub.ntxq * sizeof(int));
	bzero(dbus_info->txpend_q_hist, dbus_info->pub.ntxq * sizeof(int));
	dbus_info->rxpend_q_hist = MALLOC(osh, dbus_info->pub.nrxq * sizeof(int));
	bzero(dbus_info->rxpend_q_hist, dbus_info->pub.nrxq * sizeof(int));
#endif /* BCMDBG */

	dbus_info->bus_info = (void *)g_busintf->attach(&dbus_info->pub,
		dbus_info, &dbus_intf_cbs);
	if (dbus_info->bus_info == NULL)
		goto error;

	dbus_tx_timer_init(dbus_info);

	/* Use default firmware */
#if defined(BCMEMBEDIMAGE)
	dbus_info->fw = (uint8 *) dlarray;
	dbus_info->fwlen = sizeof(dlarray);
#endif /* BCMEMBEDIMAGE */	

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)
	if (dbus_info->drvintf->dlneeded) {
		if (dbus_info->drvintf->dlneeded(dbus_info->bus_info)) {
#if defined(BCM_DNGL_EMBEDIMAGE)
			dbus_info->fw = NULL;
			dbus_info->fwlen = 0;
			devid = dbus_info->pub.attrib.devid;
			if ((devid == 0x4323) ||(devid == 0x43231) || (devid == 0x4322)) {
#ifdef EMBED_IMAGE_4322
				dbus_info->fw = (uint8 *)dlarray_4322;
				dbus_info->fwlen = sizeof(dlarray_4322);
#endif
			} else if ((devid == 43236) || (devid == 43235) || (devid == 43238)) {
				ASSERT(dbus_info->pub.attrib.chiprev <= 2);
				if (dbus_info->pub.attrib.chiprev == 2) {
#ifdef EMBED_IMAGE_43236b0
					dbus_info->fw = (uint8 *)dlarray_43236b0;
					dbus_info->fwlen = sizeof(dlarray_43236b0);
#endif
				} else if (dbus_info->pub.attrib.chiprev == 1) {
#ifdef EMBED_IMAGE_43236a1
					dbus_info->fw = (uint8 *)dlarray_43236a1;
					dbus_info->fwlen = sizeof(dlarray_43236a1);
#endif
					} else {
#ifdef EMBED_IMAGE_43236a0
					dbus_info->fw = (uint8 *)dlarray_43236a0;
					dbus_info->fwlen = sizeof(dlarray_43236a0);
#endif
					}
			} else if (devid == 0x4319) {
#ifdef EMBED_IMAGE_4319usb
				dbus_info->fw = (uint8 *)dlarray_4319usb;
				dbus_info->fwlen = sizeof(dlarray_4319usb);
#endif
			} else {
#ifdef EMBED_IMAGE_GENERIC
				dbus_info->fw = (uint8 *)dlarray;
				dbus_info->fwlen = sizeof(dlarray);
#endif
			}
			if (!dbus_info->fw) {
				DBUSERR(("dbus_attach: devid 0x%x / %d not supported\n",
					devid, devid));
				goto error;
			}
			dbus_info->image = dbus_info->fw;
			dbus_info->image_len = (uint32)dbus_info->fwlen;
#if (defined(NDIS) || defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW))
			err = dbus_get_nvram(dbus_info);
			if (err) {
				DBUSERR(("dbus_attach: fail to get nvram %d\n", err));
				goto error;
			}
#endif /* (defined(NDIS) || defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW)) */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) */
			err = dbus_do_download(dbus_info);
			if (err == DBUS_ERR) {
				DBUSERR(("attach: download failed=%d\n", err));
				goto error;
			}
		}
	}
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE) */

	return (dbus_pub_t *)dbus_info;

error:
	dbus_detach((dbus_pub_t *)dbus_info);
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

#ifdef BCMDBG
	MFREE(osh, dbus_info->txpend_q_hist, dbus_info->pub.ntxq * sizeof(int));
	MFREE(osh, dbus_info->rxpend_q_hist, dbus_info->pub.nrxq * sizeof(int));
#endif /* BCMDBG */

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
		(dbus_info->pub.busstate == DBUS_STATE_DOWN) ||
		(dbus_info->pub.busstate == DBUS_STATE_SLEEP)) {
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

	dbus_tx_timer_stop(dbus_info);

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

int
dbus_recv_bulk(const dbus_pub_t *pub, uint32 ep_idx)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	dbus_irb_rx_t *rxirb;
	struct exec_parms args;
	int status;


	if (dbus_info == NULL)
		return DBUS_ERR;

	args.qdeq.q = dbus_info->rx_q;
	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		if (dbus_info->drvintf && dbus_info->drvintf->recv_irb_from_ep) {
			if ((rxirb = (EXEC_RXLOCK(dbus_info, q_deq_exec, &args))) != NULL) {
				status = dbus_info->drvintf->recv_irb_from_ep(dbus_info->bus_info,
					rxirb, ep_idx);
				if (status == DBUS_ERR_RXDROP) {
					bzero(rxirb, sizeof(dbus_irb_rx_t));
					args.qenq.q = dbus_info->rx_q;
					args.qenq.b = (dbus_irb_t *) rxirb;
					EXEC_RXLOCK(dbus_info, q_enq_exec, &args);
				}
			}
		}
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
dbus_get_device_speed(const dbus_pub_t *pub)
{
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	if (dbus_info == NULL)
		return INVALID_SPEED;

	return (dbus_info->pub.device_speed);
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

	if (dbus_info->pub.busstate == DBUS_STATE_UP) {
		return DBUS_OK;
	}
#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)
	if (dbus_info->drvintf->device_exists &&
		dbus_info->drvintf->device_exists(dbus_info->bus_info)) {
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
	} else {
		return DBUS_ERR;
	}
#endif /* BCM_DNGL_EMBEDIMAGE */

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

#ifdef BCMDBG
void
dbus_hist_dump(const dbus_pub_t *pub, struct bcmstrbuf *b)
{
	int i = 0, j = 0;
	dbus_info_t *dbus_info = (dbus_info_t *) pub;

	bcm_bprintf(b, "\nDBUS histogram\n");
	bcm_bprintf(b, "txq\n");
	for (i = 0; i < dbus_info->pub.ntxq; i++) {
		if (dbus_info->txpend_q_hist[i]) {
			bcm_bprintf(b, "%d: %d ", i, dbus_info->txpend_q_hist[i]);
			j++;
			if (j % 10 == 0) {
				bcm_bprintf(b, "\n");
			}
		}
	}

	j = 0;
	bcm_bprintf(b, "\nrxq\n");
	for (i = 0; i < dbus_info->pub.nrxq; i++) {
		if (dbus_info->rxpend_q_hist[i]) {
			bcm_bprintf(b, "%d: %d ", i, dbus_info->rxpend_q_hist[i]);
			j++;
			if (j % 10 == 0) {
				bcm_bprintf(b, "\n");
			}
		}
	}
	bcm_bprintf(b, "\n");

	if (dbus_info->drvintf && dbus_info->drvintf->dump)
		dbus_info->drvintf->dump(dbus_info->bus_info, b);
}
#endif /* BCMDBG */

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE)
#if defined(NDIS) || defined(USBAP)
static int
check_file(osl_t *osh, unsigned char *headers)
{
	struct trx_header *trx;
	int actual_len = -1;

	/* Extract trx header */
	trx = (struct trx_header *)headers;
	if (ltoh32(trx->magic) != TRX_MAGIC) {
		printf("Error: trx bad hdr %x\n", ltoh32(trx->magic));
		return -1;
	}

	headers += sizeof(struct trx_header);

	if (ltoh32(trx->flag_version) & TRX_UNCOMP_IMAGE) {
		actual_len = ltoh32(trx->offsets[TRX_OFFSETS_DLFWLEN_IDX]) +
		                     sizeof(struct trx_header);
		return actual_len;
	}  else {
		printf("compressed image\n");
	}
	return -1;

}
#endif /* (defined(NDIS) || defined(USBAP)) && (defined(WLC_HIGH) && !defined(WLC_LOW)) */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCMEMBEDIMAGE) */
