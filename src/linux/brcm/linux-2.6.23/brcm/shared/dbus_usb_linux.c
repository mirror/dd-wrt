/*
 * Dongle BUS interface
 * USB Linux Implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dbus_usb_linux.c,v 1.8.4.9 2008/10/05 06:33:37 Exp $
 */

#include <typedefs.h>
#include <osl.h>

#include <usbrdl.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
#include <dbus.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define KERNEL26
#define USB_ALLOC_URB()		usb_alloc_urb(0, GFP_ATOMIC)
#define USB_SUBMIT_URB(urb)	usb_submit_urb(urb, GFP_ATOMIC)
#define USB_UNLINK_URB(urb)	usb_kill_urb(urb)
#define USB_BUFFER_ALLOC(dev, size, mem, dma) \
				usb_buffer_alloc(dev, size, mem, dma)
#define USB_BUFFER_FREE(dev, size, data, dma) \
				usb_buffer_free(dev, size, data, dma)
#define URB_QUEUE_BULK		URB_ZERO_PACKET
#define CALLBACK_ARGS		struct urb *urb, struct pt_regs *regs
#define CONFIGDESC(usb)		(&((usb)->actconfig)->desc)
#define IFPTR(usb, idx)		((usb)->actconfig->interface[idx])
#define IFALTS(usb, idx)	(IFPTR((usb), (idx))->altsetting[0])
#define IFDESC(usb, idx)	IFALTS((usb), (idx)).desc
#define IFEPDESC(usb, idx, ep)	(IFALTS((usb), (idx)).endpoint[ep]).desc
#else /* 2.4 */
#define URB_QUEUE_BULK		USB_QUEUE_BULK|USB_ZERO_PACKET
#define USB_ALLOC_URB()		usb_alloc_urb(0)
#define USB_SUBMIT_URB(urb)	usb_submit_urb(urb)
#define USB_UNLINK_URB(urb)	usb_unlink_urb(urb)
#define USB_BUFFER_ALLOC(dev, size, mem, dma) \
				kmalloc(size, mem)
#define USB_BUFFER_FREE(dev, size, data, dma) \
				kfree(data)
#define CALLBACK_ARGS		struct urb *urb
#define CONFIGDESC(usb)		((usb)->actconfig)
#define IFPTR(usb, idx)		(&(usb)->actconfig->interface[idx])
#define IFALTS(usb, idx)	((usb)->actconfig->interface[idx].altsetting[0])
#define IFDESC(usb, idx)	IFALTS((usb), (idx))
#define IFEPDESC(usb, idx, ep)	(IFALTS((usb), (idx)).endpoint[ep])
#endif /* 2.4 */

#define RX_QLEN			25	/* rx bulk queue length */
#define TX_QLEN			25	/* tx bulk queue length */

#define CONTROL_IF		0
#define BULK_IF			0

/* Private data kept in skb */
#define SKB_PRIV(skb, idx)	(&((void **)skb->cb)[idx])
#define SKB_PRIV_URB(skb)	(*(struct urb **)SKB_PRIV(skb, 0))

typedef struct {
	uint32 notification;
	uint32 reserved;
} intr_t;

typedef struct {
	dbus_pub_t *pub;

	void *cbarg;
	dbus_intf_callbacks_t *cbs;

	/* Imported */
	struct usb_device *usb;	/* USB device pointer from OS */
	struct urb *intr_urb; /* URB for interrupt endpoint */
	struct urb_req *req_freeq;
	struct urb_req *req_rxpendq;
	struct urb_req *req_txpendq;
	spinlock_t free_lock; /* Lock for free list */
	spinlock_t rxpend_lock; /* Lock for free list */
	spinlock_t txpend_lock; /* Lock for free list */
	uint rx_pipe, tx_pipe, intr_pipe; /* Pipe numbers for USB I/O */

	struct urb *ctl_urb;
	int ctl_in_pipe, ctl_out_pipe;
	struct usb_ctrlrequest ctl_write;
	struct usb_ctrlrequest ctl_read;

	spinlock_t rxlock;      /* Lock for rxq management */
	spinlock_t txlock;      /* Lock for txq management */

	int intr_size;          /* Size of interrupt message */
	int interval;           /* Interrupt polling interval */
	intr_t intr;            /* Data buffer for interrupt endpoint */

	int maxps;
	int txpending;
	int rxpending;
	bool rxctl_deferrespok;	/* Get a response for setup from dongle */

} usbos_info_t;

typedef struct urb_req {
	void *buf;
	int buf_len;
	struct urb *urb;
	void *arg;
	usbos_info_t *usbinfo;
	struct urb_req *next;
} urb_req_t;

/* Local function prototypes */
static void dbus_usbos_send_complete(CALLBACK_ARGS);
static void dbus_usbos_recv_complete(CALLBACK_ARGS);
static int  dbus_usbos_errhandler(void *bus, int err);
static int  dbus_usbos_state_change(void *bus, int state);
static void dbusos_stop(usbos_info_t *usbos_info);

#ifdef KERNEL26
static int dbus_usbos_probe(struct usb_interface *intf, const struct usb_device_id *id);
static void dbus_usbos_disconnect(struct usb_interface *intf);
#else
static void *dbus_usbos_probe(struct usb_device *usb, unsigned int ifnum,
	const struct usb_device_id *id);
static void dbus_usbos_disconnect(struct usb_device *usb, void *ptr);
#endif

static struct usb_device_id devid_table[] = {
	{ USB_DEVICE(0x0a5c, 0x0000) }, /* Configurable via register() */
	{ USB_DEVICE(0x0a5c, 0x0bdc) }, /* Default BDC */
	{ }
};

MODULE_DEVICE_TABLE(usb, devid_table);

static struct usb_driver dbus_usbdev = {
	name:           "dbus_usbdev",
	probe:          dbus_usbos_probe,
	disconnect:     dbus_usbos_disconnect,
	id_table:       devid_table
};

/* This stores USB info during Linux probe callback
 * since attach() is not called yet at this point
 */
typedef struct {
	void *usbos_info;
	struct usb_device *usb; /* USB device pointer from OS */
	uint rx_pipe, tx_pipe, intr_pipe; /* Pipe numbers for USB I/O */
	int intr_size; /* Size of interrupt message */
	int interval;  /* Interrupt polling interval */
	bool dldone;
	int vid;
	int pid;
} probe_info_t;

static probe_info_t g_probe_info;

/*
 * USB Linux dbus_intf_t
 */
static void *dbus_usbos_intf_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_usbos_intf_detach(dbus_pub_t *pub, void *info);
static int  dbus_usbos_intf_send_irb(void *bus, dbus_irb_tx_t *txirb);
static int  dbus_usbos_intf_recv_irb(void *bus, dbus_irb_rx_t *rxirb);
static int  dbus_usbos_intf_cancel_irb(void *bus, dbus_irb_tx_t *txirb);
static int  dbus_usbos_intf_send_ctl(void *bus, uint8 *buf, int len);
static int  dbus_usbos_intf_recv_ctl(void *bus, uint8 *buf, int len);
static int  dbus_usbos_intf_get_attrib(void *bus, dbus_attrib_t *attrib);
static int  dbus_usbos_intf_up(void *bus);
static int  dbus_usbos_intf_down(void *bus);
static int  dbus_usbos_intf_stop(void *bus);
static int  dbus_usbos_intf_set_config(void *bus, dbus_config_t *config);
static bool dbus_usbos_intf_dlneeded(void *bus);
static int  dbus_usbos_intf_dlstart(void *bus, uint8 *fw, int len);
static int  dbus_usbos_intf_dlrun(void *bus);
static bool dbus_usbos_intf_recv_needed(void *bus);
static void *dbus_usbos_intf_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static void *dbus_usbos_intf_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args);

static dbus_intf_t dbus_usbos_intf = {
	dbus_usbos_intf_attach,
	dbus_usbos_intf_detach,
	dbus_usbos_intf_up,
	dbus_usbos_intf_down,
	dbus_usbos_intf_send_irb,
	dbus_usbos_intf_recv_irb,
	dbus_usbos_intf_cancel_irb,
	dbus_usbos_intf_send_ctl,
	dbus_usbos_intf_recv_ctl,
	NULL, /* get_stats */
	dbus_usbos_intf_get_attrib,
	NULL, /* pnp */
	NULL, /* remove */
	NULL, /* resume */
	NULL, /* suspend */
	dbus_usbos_intf_stop,
	NULL, /* reset */
	NULL, /* pktget */
	NULL, /* pktfree */
	NULL, /* iovar_op */
	NULL, /* dump */
	dbus_usbos_intf_set_config, /* set_config */
	NULL, /* get_config */
	dbus_usbos_intf_dlneeded,
	dbus_usbos_intf_dlstart,
	dbus_usbos_intf_dlrun,
	dbus_usbos_intf_recv_needed,
	dbus_usbos_intf_exec_rxlock,
	dbus_usbos_intf_exec_txlock

	/* tx_timer_init */
	/* tx_timer_start */
	/* tx_timer_stop */

	/* sched_dpc */
	/* lock */
	/* unlock */
	/* sched_probe_cb */

	/* shutdown */

	/* recv_stop */
	/* recv_resume */
};

static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

static urb_req_t *
dbus_usbos_qdeq(urb_req_t **urbreq_q, spinlock_t *lock)
{
	unsigned long flags;
	urb_req_t *req;

	spin_lock_irqsave(lock, flags);

	if ((req = *urbreq_q) != NULL) {
		*urbreq_q = req->next;
		req->next = NULL;
	}
	spin_unlock_irqrestore(lock, flags);

	return req;
}

static void
dbus_usbos_qenq(urb_req_t **urbreq_q, urb_req_t *req, spinlock_t *lock)
{
	unsigned long flags;

	spin_lock_irqsave(lock, flags);

	req->next = *urbreq_q;
	*urbreq_q = req;

	spin_unlock_irqrestore(lock, flags);
}

/*
 * FIX: Change linear search to something more efficient
 */
static void
dbus_usbos_req_del(urb_req_t **urbreq_q, urb_req_t *req, spinlock_t *lock)
{
	unsigned long flags;
	urb_req_t *curr_req;
	urb_req_t *prev_req;
	urb_req_t *head;

	spin_lock_irqsave(lock, flags);

	head = prev_req = curr_req = *urbreq_q;
	for (; curr_req != NULL; curr_req = curr_req->next) {
		if (curr_req == req) {
			if (prev_req != NULL) {
				prev_req->next = curr_req->next;
				req->next = NULL;

				/* If first entry, clear it queue head */
				if (head->next == NULL)
					*urbreq_q = NULL;
			} else {
				/* Ooops: Should not happen */
				DBUSERR(("%s ERROR: prev_req == NULL!!!\n", __FUNCTION__));
			}
			goto done;
		}
		prev_req = curr_req;
	}
done:
	spin_unlock_irqrestore(lock, flags);
}

static int
dbus_usbos_urbreqs_alloc(usbos_info_t *usbos_info)
{
	int i;

	/* FIX: should probably have independent queues for tx and rx */
	for (i = 0; i < (usbos_info->pub->nrxq + usbos_info->pub->ntxq); i++) {
		urb_req_t *req;

		req = MALLOC(usbos_info->pub->osh, sizeof(urb_req_t));
		if (req == NULL) {
			/* dbus_usbos_urbreqs_free() takes care of partial
			 * allocation
			 */
			DBUSERR(("%s: usb_alloc_urb failed\n", __FUNCTION__));
			return DBUS_ERR_NOMEM;
		}
		bzero(req, sizeof(urb_req_t));

		req->urb = USB_ALLOC_URB();
		if (req->urb == NULL) {
			DBUSERR(("%s: usb_alloc_urb failed\n", __FUNCTION__));
			return DBUS_ERR_NOMEM;
		}

		req->buf = MALLOC(usbos_info->pub->osh, usbos_info->pub->rxsize);
		if (req->buf == NULL) {
			DBUSERR(("%s: usb_alloc_urb failed\n", __FUNCTION__));
			return DBUS_ERR_NOMEM;
		}
		req->buf_len = usbos_info->pub->rxsize;
		dbus_usbos_qenq(&usbos_info->req_freeq, req, &usbos_info->free_lock);
	}

	return DBUS_OK;
}

/* Don't call until all URBs unlinked */
static void
dbus_usbos_urbreqs_free(usbos_info_t *usbos_info)
{
	urb_req_t *req;

	while ((req = dbus_usbos_qdeq(&usbos_info->req_freeq,
		&usbos_info->free_lock)) != NULL) {

		if (req->buf) {
			MFREE(usbos_info->pub->osh, req->buf, req->buf_len);
			req->buf = NULL;
			req->buf_len = 0;
		}

		if (req->urb) {
			usb_free_urb(req->urb);
			req->urb = NULL;
		}
		MFREE(usbos_info->pub->osh, req, sizeof(urb_req_t));
	}
}

void
dbus_usbos_send_complete(CALLBACK_ARGS)
{
	urb_req_t *req = urb->context;
	dbus_irb_tx_t *txirb = req->arg;
	usbos_info_t *usbos_info = req->usbinfo;
	unsigned long flags;
	int status = DBUS_OK;

	spin_lock_irqsave(&usbos_info->txlock, flags);
	dbus_usbos_req_del(&usbos_info->req_txpendq, req, &usbos_info->txpend_lock);
	usbos_info->txpending--;
	if (usbos_info->txpending < 0) {
		DBUSERR(("%s ERROR: txpending is negative!!\n", __FUNCTION__));
	}
	spin_unlock_irqrestore(&usbos_info->txlock, flags);

	if (urb->status)
		status = DBUS_ERR_TXFAIL;

	dbus_usbos_qenq(&usbos_info->req_freeq, req, &usbos_info->free_lock);
	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->send_irb_complete)
			usbos_info->cbs->send_irb_complete(usbos_info->cbarg, txirb, status);
	}
}

static int
dbus_usbos_recv_urb_submit(usbos_info_t *usbos_info, dbus_irb_rx_t *rxirb)
{
	urb_req_t *req;
	int ret = DBUS_OK;
	unsigned long flags;

	if (!(req = dbus_usbos_qdeq(&usbos_info->req_freeq, &usbos_info->free_lock))) {
		return DBUS_ERR_NOMEM;
	}

	spin_lock_irqsave(&usbos_info->rxlock, flags);
	rxirb->buf = req->buf;
	rxirb->buf_len = req->buf_len;
	req->usbinfo = usbos_info;
	req->arg = rxirb;

	/* Prepare the URB */
	usb_fill_bulk_urb(req->urb, usbos_info->usb, usbos_info->rx_pipe,
		rxirb->buf,
		rxirb->buf_len,
		(usb_complete_t)dbus_usbos_recv_complete, req);
		req->urb->transfer_flags |= URB_QUEUE_BULK;

	if ((ret = USB_SUBMIT_URB(req->urb))) {
		DBUSERR(("%s USB_SUBMIT_URB failed with status %d\n", __FUNCTION__, ret));
		dbus_usbos_qenq(&usbos_info->req_freeq, req, &usbos_info->free_lock);
		ret = DBUS_ERR_RXFAIL;
		goto fail;
	}
	usbos_info->rxpending++;

	dbus_usbos_qenq(&usbos_info->req_rxpendq, req, &usbos_info->rxpend_lock);
fail:
	spin_unlock_irqrestore(&usbos_info->rxlock, flags);
	return ret;
}

void
dbus_usbos_recv_complete(CALLBACK_ARGS)
{
	urb_req_t *req = urb->context;
	dbus_irb_rx_t *rxirb = req->arg;
	usbos_info_t *usbos_info = req->usbinfo;
	unsigned long flags;
	int dbus_status = DBUS_OK;

	spin_lock_irqsave(&usbos_info->rxlock, flags);
	dbus_usbos_req_del(&usbos_info->req_rxpendq, req, &usbos_info->rxpend_lock);
	usbos_info->rxpending--;
	spin_unlock_irqrestore(&usbos_info->rxlock, flags);

	/* Handle errors */
	if (urb->status) {
		/*
		 * Linux 2.4 disconnect: -ENOENT or -EILSEQ for CRC error; rmmod: -ENOENT
		 * Linux 2.6 disconnect: -EPROTO, rmmod: -ESHUTDOWN
		 */
		if (urb->status == -ENOENT || urb->status == -ESHUTDOWN) {
	 			/* NOTE: unlink() can not be called from URB callback().
				 * Do not call dbusos_stop() here.
				 */
				dbus_usbos_state_change(usbos_info, DBUS_STATE_DOWN);
		} else if (urb->status == -EPROTO) {
		} else {
			DBUSERR(("%s rx error %d\n", __FUNCTION__, urb->status));
			dbus_usbos_errhandler(usbos_info, DBUS_ERR_RXFAIL);
		}

		/* On error, don't submit more URBs yet */
		rxirb->buf = NULL;
		rxirb->actual_len = 0;
		dbus_status = DBUS_ERR_RXFAIL;
		goto fail;
	}

	/* Make the skb represent the received urb */
	rxirb->actual_len = urb->actual_length;
fail:
	dbus_usbos_qenq(&usbos_info->req_freeq, req, &usbos_info->free_lock);
	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->recv_irb_complete) {
			usbos_info->cbs->recv_irb_complete(usbos_info->cbarg, rxirb, dbus_status);
		}
	}
}

static void
dbus_usbos_intr_complete(CALLBACK_ARGS)
{
	usbos_info_t *usbos_info = (usbos_info_t *)urb->context;
#ifdef KERNEL26
	int ret;
#endif

	if (usbos_info == NULL)
		return;

#ifdef KERNEL26
	/* Resubmit every time (2.6 only) */
	if (((usbos_info->pub->busstate == DBUS_STATE_UP) &&
	     (ret = USB_SUBMIT_URB(usbos_info->intr_urb))))
		DBUSERR(("%s intr USB_SUBMIT_URB failed with status %d\n", __FUNCTION__, ret));
#endif
}

static void
dbus_usbos_ctl_complete(usbos_info_t *usbos_info, int type, int urbstatus)
{
	int status = DBUS_ERR;

	if (usbos_info == NULL)
		return;

	switch (urbstatus) {
		case 0:
			status = DBUS_OK;
		break;
		case -EINPROGRESS:
		case -ENOENT:
		default:
			DBUSERR(("%s: failed with status %d\n", __FUNCTION__, urbstatus));
			status = DBUS_ERR;
		break;
	}

	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->ctl_complete)
			usbos_info->cbs->ctl_complete(usbos_info->cbarg, type, status);
	}
}

static void
dbus_usbos_ctlread_complete(CALLBACK_ARGS)
{
	usbos_info_t *usbos_info = (usbos_info_t *)urb->context;

	ASSERT(urb);
	usbos_info = (usbos_info_t *)urb->context;

	dbus_usbos_ctl_complete(usbos_info, DBUS_CBCTL_READ, urb->status);

	if (usbos_info->rxctl_deferrespok) {
		usbos_info->ctl_read.bRequestType = USB_DIR_IN | USB_TYPE_CLASS |
			USB_RECIP_INTERFACE;
		usbos_info->ctl_read.bRequest = 1;
	}
}

static void
dbus_usbos_ctlwrite_complete(CALLBACK_ARGS)
{
	usbos_info_t *usbos_info = (usbos_info_t *)urb->context;

	ASSERT(urb);
	usbos_info = (usbos_info_t *)urb->context;

	dbus_usbos_ctl_complete(usbos_info, DBUS_CBCTL_WRITE, urb->status);
}

static void
dbus_usbos_unlink(urb_req_t **urbreq_q, spinlock_t *lock)
{
	urb_req_t *req;

	/* dbus_usbos_recv_complete() adds req back to req_freeq */
	while ((req = dbus_usbos_qdeq(urbreq_q, lock)) != NULL) {
		ASSERT(req->urb != NULL);
		USB_UNLINK_URB(req->urb);
	}
}

static void
dbusos_stop(usbos_info_t *usbos_info)
{
	ASSERT(usbos_info);

	dbus_usbos_state_change(usbos_info, DBUS_STATE_DOWN);
	DBUSTRACE(("%s: unlink all URBs\n", __FUNCTION__));
	if (usbos_info->intr_urb)
		USB_UNLINK_URB(usbos_info->intr_urb);

	if (usbos_info->ctl_urb)
		USB_UNLINK_URB(usbos_info->ctl_urb);

	dbus_usbos_unlink(&usbos_info->req_txpendq, &usbos_info->txpend_lock);
	if (usbos_info->txpending > 0) {
		DBUSERR(("%s ERROR: tx REQs pending=%d in stop!\n", __FUNCTION__,
			usbos_info->txpending));
	}
	dbus_usbos_unlink(&usbos_info->req_rxpendq, &usbos_info->rxpend_lock);
	if (usbos_info->rxpending > 0) {
		DBUSERR(("%s ERROR: rx REQs pending=%d in stop!\n", __FUNCTION__,
			usbos_info->rxpending));
	}
}

#ifdef KERNEL26
static int
dbus_usbos_probe(struct usb_interface *intf,
             const struct usb_device_id *id)
#else
static void *
dbus_usbos_probe(struct usb_device *usb, unsigned int ifnum,
             const struct usb_device_id *id)
#endif
{
	int ep;
	struct usb_endpoint_descriptor *endpoint;
	int ret = 0;
#ifdef KERNEL26
	struct usb_device *usb = interface_to_usbdev(intf);
#else
	int claimed = 0;
#endif
	g_probe_info.usb = usb;
	g_probe_info.dldone = TRUE;

	if (id != NULL) {
		g_probe_info.vid = id->idVendor;
		g_probe_info.pid = id->idProduct;
	}

#ifdef KERNEL26
	usb_set_intfdata(intf, &g_probe_info);
#endif

	/* Check that the device supports only one configuration */
	if (usb->descriptor.bNumConfigurations != 1) {
		ret = -1;
		goto fail;
	}

	if (usb->descriptor.bDeviceClass != USB_CLASS_VENDOR_SPEC) {
		ret = -1;
		goto fail;
	}

	/*
	 * Only the BDC interface configuration is supported:
	 *	Device class: USB_CLASS_VENDOR_SPEC
	 *	if0 class: USB_CLASS_VENDOR_SPEC
	 *	if0/ep0: control
	 *	if0/ep1: bulk in
	 *	if0/ep2: bulk out (ok if swapped with bulk in)
	 */
	if (CONFIGDESC(usb)->bNumInterfaces != 1) {
		ret = -1;
		goto fail;
	}

	/* Check interface */
#ifndef KERNEL26
	if (usb_interface_claimed(IFPTR(usb, CONTROL_IF))) {
		ret = -1;
		goto fail;
	}
#endif

	if (IFDESC(usb, CONTROL_IF).bInterfaceClass != USB_CLASS_VENDOR_SPEC ||
	    IFDESC(usb, CONTROL_IF).bInterfaceSubClass != 2 ||
	    IFDESC(usb, CONTROL_IF).bInterfaceProtocol != 0xff) {
		DBUSERR(("%s: invalid control interface: class %d, subclass %d, proto %d\n",
		           __FUNCTION__,
		           IFDESC(usb, CONTROL_IF).bInterfaceClass,
		           IFDESC(usb, CONTROL_IF).bInterfaceSubClass,
		           IFDESC(usb, CONTROL_IF).bInterfaceProtocol));
		ret = -1;
		goto fail;
	}

	/* Check control endpoint */
	endpoint = &IFEPDESC(usb, CONTROL_IF, 0);
	if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT) {
		DBUSERR(("%s: invalid control endpoint %d\n",
		           __FUNCTION__, endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK));
		ret = -1;
		goto fail;
	}

	g_probe_info.intr_pipe =
		usb_rcvintpipe(usb, endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

#ifndef KERNEL26
	/* Claim interface */
	usb_driver_claim_interface(&dbus_usbdev, IFPTR(usb, CONTROL_IF), &g_probe_info);
	claimed = 1;
#endif

	/* Check data endpoints and get pipes */
	for (ep = 1; ep <= 2; ep++) {
		endpoint = &IFEPDESC(usb, BULK_IF, ep);
		if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
		    USB_ENDPOINT_XFER_BULK) {
			DBUSERR(("%s: invalid data endpoint %d\n",
			           __FUNCTION__, ep));
			ret = -1;
			goto fail;
		}

		if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
			g_probe_info.rx_pipe = usb_rcvbulkpipe(usb, (endpoint->bEndpointAddress &
			                                     USB_ENDPOINT_NUMBER_MASK));
		else
			g_probe_info.tx_pipe = usb_sndbulkpipe(usb, (endpoint->bEndpointAddress &
			                                     USB_ENDPOINT_NUMBER_MASK));
	}

	/* Allocate interrupt URB and data buffer */
	/* RNDIS says 8-byte intr, our old drivers used 4-byte */
	g_probe_info.intr_size = (IFEPDESC(usb, CONTROL_IF, 0).wMaxPacketSize == 16) ? 8 : 4;

	g_probe_info.interval = IFEPDESC(usb, CONTROL_IF, 0).bInterval;

#ifndef KERNEL26
	/* usb_fill_int_urb does the interval decoding in 2.6 */
	if (usb->speed == USB_SPEED_HIGH)
		g_probe_info.interval = 1 << (g_probe_info.interval - 1);
#endif
	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", USB_BUS, 0);
	}

	/* Success */
#ifdef KERNEL26
	return DBUS_OK;
#else
	usb_inc_dev_use(usb);
	return &g_probe_info;
#endif

fail:
	DBUSERR(("%s: failed with errno %d\n", __FUNCTION__, ret));
#ifndef KERNEL26
	if (claimed)
		usb_driver_release_interface(&dbus_usbdev, IFPTR(usb, CONTROL_IF));
#endif
#ifdef KERNEL26
	usb_set_intfdata(intf, NULL);
#endif

#ifdef KERNEL26
	return ret;
#else
	return NULL;
#endif
}

#ifdef KERNEL26
static void
dbus_usbos_disconnect(struct usb_interface *intf)
#else
static void
dbus_usbos_disconnect(struct usb_device *usb, void *ptr)
#endif
{
#ifdef KERNEL26
	struct usb_device *usb = interface_to_usbdev(intf);
	probe_info_t *probe_usb_init_data = usb_get_intfdata(intf);
#else
	probe_info_t *probe_usb_init_data = (probe_info_t *) ptr;
#endif
	usbos_info_t *usbos_info;

	if ((probe_usb_init_data == NULL) || (usb == NULL)) {
		/* Should never happen */
		ASSERT(0);
		return;
	}

	usbos_info = (usbos_info_t *) probe_usb_init_data->usbos_info;

	if (usbos_info) {
		if (disconnect_cb)
			disconnect_cb(disc_arg);
	}

#ifndef KERNEL26
	usb_driver_release_interface(&dbus_usbdev, IFPTR(usb, CONTROL_IF));
	usb_dec_dev_use(usb);
#endif
}

static int
dbus_usbos_intf_send_irb(void *bus, dbus_irb_tx_t *txirb)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	urb_req_t *req;
	int ret = DBUS_OK;
	unsigned long flags;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (!(req = dbus_usbos_qdeq(&usbos_info->req_freeq, &usbos_info->free_lock))) {
		DBUSERR(("%s No free URB!\n", __FUNCTION__));
		return DBUS_ERR_NOMEM;
	}

	spin_lock_irqsave(&usbos_info->txlock, flags);
	req->arg = txirb;
	req->usbinfo = usbos_info;

	/* Prepare the URB */
	usb_fill_bulk_urb(req->urb, usbos_info->usb, usbos_info->tx_pipe, txirb->buf,
		txirb->len, (usb_complete_t)dbus_usbos_send_complete, req);
	req->urb->transfer_flags |= URB_QUEUE_BULK;

	if ((ret = USB_SUBMIT_URB(req->urb))) {
		dbus_usbos_qenq(&usbos_info->req_freeq, req, &usbos_info->free_lock);
		ret = DBUS_ERR_TXFAIL;
		goto fail;
	}

	usbos_info->txpending++;

	dbus_usbos_qenq(&usbos_info->req_txpendq, req, &usbos_info->txpend_lock);
fail:
	spin_unlock_irqrestore(&usbos_info->txlock, flags);
	return ret;
}

static int
dbus_usbos_intf_recv_irb(void *bus, dbus_irb_rx_t *rxirb)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret = DBUS_OK;

	if (usbos_info == NULL)
		return DBUS_ERR;

	ret = dbus_usbos_recv_urb_submit(usbos_info, rxirb);
	return ret;
}

static int
dbus_usbos_intf_cancel_irb(void *bus, dbus_irb_tx_t *txirb)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	/* FIX: Need to implement */
	return DBUS_ERR;
}

static int
dbus_usbos_intf_send_ctl(void *bus, uint8 *buf, int len)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret = DBUS_OK;
	uint16 size;

	if ((usbos_info == NULL) || (buf == NULL) || (len == 0))
		return DBUS_ERR;

	if (usbos_info->ctl_urb == NULL)
		return DBUS_ERR;

	size = len;
	usbos_info->ctl_write.wLength = cpu_to_le16p(&size);
	usbos_info->ctl_urb->transfer_buffer_length = size;

	usb_fill_control_urb(usbos_info->ctl_urb,
		usbos_info->usb,
		usbos_info->ctl_out_pipe,
		(unsigned char *) &usbos_info->ctl_write,
		buf, size, (usb_complete_t)dbus_usbos_ctlwrite_complete, usbos_info);

	ret = USB_SUBMIT_URB(usbos_info->ctl_urb);
	if (ret < 0) {
		DBUSERR(("%s: usb_submit_urb failed %d\n", __FUNCTION__, ret));
		return DBUS_ERR_TXCTLFAIL;
	}

	return DBUS_OK;
}

static int
dbus_usbos_intf_recv_ctl(void *bus, uint8 *buf, int len)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret = DBUS_OK;
	uint16 size;

	if ((usbos_info == NULL) || (buf == NULL) || (len == 0))
		return DBUS_ERR;

	if (usbos_info->ctl_urb == NULL)
		return DBUS_ERR;

	size = len;
	usbos_info->ctl_read.wLength = cpu_to_le16p(&size);
	usbos_info->ctl_urb->transfer_buffer_length = size;

	if (usbos_info->rxctl_deferrespok) {
		usbos_info->ctl_read.bRequestType = USB_DIR_IN | USB_TYPE_VENDOR |
			USB_RECIP_INTERFACE;
		usbos_info->ctl_read.bRequest = DL_DEFER_RESP_OK;
	}

	usb_fill_control_urb(usbos_info->ctl_urb,
		usbos_info->usb,
		usbos_info->ctl_in_pipe,
		(unsigned char *) &usbos_info->ctl_read,
		buf, size, (usb_complete_t)dbus_usbos_ctlread_complete, usbos_info);

	ret = USB_SUBMIT_URB(usbos_info->ctl_urb);
	if (ret < 0) {
		DBUSERR(("%s: usb_submit_urb failed %d\n", __FUNCTION__, ret));
		return DBUS_ERR_RXCTLFAIL;
	}

	return ret;
}

static int
dbus_usbos_intf_get_attrib(void *bus, dbus_attrib_t *attrib)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if ((usbos_info == NULL) || (attrib == NULL))
		return DBUS_ERR;

	attrib->bustype = DBUS_USB;
	attrib->vid = g_probe_info.vid;
	attrib->pid = g_probe_info.pid;
	attrib->devid = 0x4322;

	/* FIX: Need nchan for both TX and RX?;
	 * BDC uses one RX pipe and one TX pipe
	 * RPC may use two RX pipes and one TX pipe?
	 */
	attrib->nchan = 1;

	/* MaxPacketSize for USB hi-speed bulk out is 512 pipes
	 * and 64-bytes for full-speed.
	 * When sending pkt > MaxPacketSize, Host SW breaks it
	 * up into multiple packets.
	 */
	attrib->mtu = usbos_info->maxps;

	return DBUS_OK;
}

static int
dbus_usbos_intf_up(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret = DBUS_OK;
	uint16 ifnum;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->usb == NULL)
		return DBUS_ERR;

	if (usbos_info->intr_urb) {
		usb_fill_int_urb(usbos_info->intr_urb, usbos_info->usb,
			usbos_info->intr_pipe, &usbos_info->intr,
			usbos_info->intr_size, (usb_complete_t)dbus_usbos_intr_complete,
			usbos_info, usbos_info->interval);

		if ((ret = USB_SUBMIT_URB(usbos_info->intr_urb))) {
			DBUSERR(("%s USB_SUBMIT_URB failed with status %d\n", __FUNCTION__, ret));
			return DBUS_ERR;
		}
	}

	if (usbos_info->ctl_urb) {
		usbos_info->ctl_in_pipe = usb_rcvctrlpipe(usbos_info->usb, 0);
		usbos_info->ctl_out_pipe = usb_sndctrlpipe(usbos_info->usb, 0);

		ifnum = cpu_to_le16(IFDESC(usbos_info->usb, CONTROL_IF).bInterfaceNumber);
		/* CTL Write */
		usbos_info->ctl_write.bRequestType =
			USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
		usbos_info->ctl_write.bRequest = 0;
		usbos_info->ctl_write.wValue = cpu_to_le16(0);
		usbos_info->ctl_write.wIndex = cpu_to_le16p(&ifnum);

		/* CTL Read */
		usbos_info->ctl_read.bRequestType =
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
		usbos_info->ctl_read.bRequest = 1;
		usbos_info->ctl_read.wValue = cpu_to_le16(0);
		usbos_info->ctl_read.wIndex = cpu_to_le16p(&ifnum);
	}

	/* Success, indicate usbos_info is fully up */
	dbus_usbos_state_change(usbos_info, DBUS_STATE_UP);

	return DBUS_OK;
}

static int
dbus_usbos_intf_down(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	dbusos_stop(usbos_info);
	return DBUS_OK;
}

static int
dbus_usbos_intf_stop(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	dbusos_stop(usbos_info);
	return DBUS_OK;
}

static int
dbus_usbos_intf_set_config(void *bus, dbus_config_t *config)
{
	usbos_info_t* usbos_info = bus;

	usbos_info->rxctl_deferrespok = config->rxctl_deferrespok;
	return DBUS_OK;
}

static bool
dbus_usbos_intf_dlneeded(void *bus)
{
	return FALSE;
}

static int
dbus_usbos_intf_dlstart(void *bus, uint8 *fw, int len)
{
	return DBUS_ERR;
}

static int
dbus_usbos_intf_dlrun(void *bus)
{
	return DBUS_ERR;
}

static bool
dbus_usbos_intf_recv_needed(void *bus)
{
	return FALSE;
}

static void*
dbus_usbos_intf_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	void *ret;
	unsigned long flags;

	if (usbos_info == NULL)
		return NULL;

	spin_lock_irqsave(&usbos_info->rxlock, flags);
	ret = cb(args);
	spin_unlock_irqrestore(&usbos_info->rxlock, flags);

	return ret;
}

static void*
dbus_usbos_intf_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	void *ret;
	unsigned long flags;

	if (usbos_info == NULL)
		return NULL;

	spin_lock_irqsave(&usbos_info->txlock, flags);
	ret = cb(args);
	spin_unlock_irqrestore(&usbos_info->txlock, flags);

	return ret;
}

int
dbus_usbos_errhandler(void *bus, int err)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->errhandler)
			usbos_info->cbs->errhandler(usbos_info->cbarg, err);
	}

	return DBUS_OK;
}

int
dbus_usbos_state_change(void *bus, int state)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->state_change)
			usbos_info->cbs->state_change(usbos_info->cbarg, state);
	}

	return DBUS_OK;
}

int
dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	bzero(&g_probe_info, sizeof(probe_info_t));

	probe_cb = prcb;
	disconnect_cb = discb;
	probe_arg = prarg;

	devid_table[0].idVendor = vid;
	devid_table[0].idProduct = pid;

	*intf = &dbus_usbos_intf;

	usb_register(&dbus_usbdev);
	return DBUS_ERR_NODEVICE;
}

int
dbus_bus_osl_deregister()
{
	usb_deregister(&dbus_usbdev);

	return DBUS_OK;
}

void *
dbus_usbos_intf_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	usbos_info_t *usbos_info;

	if (g_probe_info.dldone == FALSE) {
		DBUSERR(("%s: err device not downloaded!\n", __FUNCTION__));
		return NULL;
	}

	usbos_info = MALLOC(pub->osh, sizeof(usbos_info_t));
	if (usbos_info == NULL)
		return NULL;

	bzero(usbos_info, sizeof(usbos_info_t));

	usbos_info->pub = pub;
	usbos_info->cbarg = cbarg;
	usbos_info->cbs = cbs;

	/* Needed for disconnect() */
	g_probe_info.usbos_info = usbos_info;

	/* Update USB Info */
	usbos_info->usb = g_probe_info.usb;
	usbos_info->rx_pipe = g_probe_info.rx_pipe;
	usbos_info->tx_pipe = g_probe_info.tx_pipe;
	usbos_info->intr_pipe = g_probe_info.intr_pipe;
	usbos_info->intr_size = g_probe_info.intr_size;
	usbos_info->interval = g_probe_info.interval;
	usbos_info->pub->busstate = DBUS_STATE_DL_DONE;

	if (usbos_info->tx_pipe)
		usbos_info->maxps = usb_maxpacket(usbos_info->usb,
			usbos_info->tx_pipe, usb_pipeout(usbos_info->tx_pipe));

	spin_lock_init(&usbos_info->free_lock);
	spin_lock_init(&usbos_info->rxpend_lock);
	spin_lock_init(&usbos_info->txpend_lock);
	spin_lock_init(&usbos_info->rxlock);
	spin_lock_init(&usbos_info->txlock);

	if (!(usbos_info->intr_urb = USB_ALLOC_URB())) {
		DBUSERR(("%s: usb_alloc_urb (tx) failed\n", __FUNCTION__));
		goto fail;
	}

	if (!(usbos_info->ctl_urb = USB_ALLOC_URB())) {
		DBUSERR(("%s: usb_alloc_urb (tx) failed\n", __FUNCTION__));
		goto fail;
	}

	if (dbus_usbos_urbreqs_alloc(usbos_info) != DBUS_OK) {
		dbus_usbos_urbreqs_free(usbos_info);
		goto fail;
	}

	return (void *) usbos_info;
fail:
	MFREE(pub->osh, usbos_info, sizeof(usbos_info_t));
	return NULL;

}

void
dbus_usbos_intf_detach(dbus_pub_t *pub, void *info)
{
	usbos_info_t *usbos_info = (usbos_info_t *) info;
	osl_t *osh = pub->osh;

	if (usbos_info == NULL) {
		return;
	}

	/* Must unlink all URBs prior to driver unload;
	 * otherwise an URB callback can occur after driver
	 * has been de-allocated and rmmod'd
	 */
	dbusos_stop(usbos_info);

	if (usbos_info->intr_urb) {
		usb_free_urb(usbos_info->intr_urb);
		usbos_info->intr_urb = NULL;
	}

	if (usbos_info->ctl_urb) {
		usb_free_urb(usbos_info->ctl_urb);
		usbos_info->ctl_urb = NULL;
	}

	dbus_usbos_urbreqs_free(usbos_info);

	g_probe_info.usbos_info = NULL;
	MFREE(osh, usbos_info, sizeof(usbos_info_t));
}
