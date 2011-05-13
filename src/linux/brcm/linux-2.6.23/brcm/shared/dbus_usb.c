/*
 * Dongle BUS interface for USB, OS independent
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dbus_usb.c,v 1.34.8.4 2010-08-10 20:39:22 Exp $
 */

#include <osl.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <dbus.h>
#include <usbrdl.h>
#include <bcmendian.h>

typedef struct {
	dbus_pub_t *pub;

	void *cbarg;
	dbus_intf_callbacks_t *cbs;
	dbus_intf_t *drvintf;
	void *usbosl_info;
} usb_info_t;

#define USB_DLIMAGE_SPINWAIT		10	/* in unit of ms */
#define USB_DLIMAGE_LIMIT		500	/* spinwait limit (ms) */
#define USB_SFLASH_DLIMAGE_SPINWAIT	200	/* in unit of ms */
#define USB_SFLASH_DLIMAGE_LIMIT	1000	/* spinwait limit (ms) */
#define POSTBOOT_ID			0xA123  /* ID to detect if dongle has boot up */
#define USB_RESETCFG_SPINWAIT		1	/* wait after resetcfg (ms) */
#define USB_DEV_ISBAD(u)		(u->pub->attrib.devid == 0xDEAD)

#define USB_DLGO_SPINWAIT		100	/* wait after DL_GO (ms) */
#define TEST_CHIP			0x4328

/*
 * Callbacks common to all USB
 */
static void dbus_usb_disconnect(void *handle);
static void dbus_usb_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb);
static void dbus_usb_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status);
static void dbus_usb_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status);
static void dbus_usb_errhandler(void *handle, int err);
static void dbus_usb_ctl_complete(void *handle, int type, int status);
static void dbus_usb_state_change(void *handle, int state);
struct dbus_irb* dbus_usb_getirb(void *handle, bool send);
static void dbus_usb_rxerr_indicate(void *handle, bool on);
static int dbus_usb_resetcfg(usb_info_t *usbinfo);

#ifdef BCM_DNGL_EMBEDIMAGE
static int dbus_usb_dlstart(void *bus, uint8 *fw, int len);
static bool dbus_usb_dlneeded(void *bus);
static int dbus_usb_dlrun(void *bus);
static bool dbus_usb_device_exists(void *bus);
#endif

/* OS specific */
extern bool dbus_usbos_dl_cmd(void *info, uint8 cmd, void *buffer, int buflen);
extern int dbus_usbos_wait(void *info, uint16 ms);
#ifdef BCM_DNGL_EMBEDIMAGE
extern bool dbus_usbos_dl_send_bulk(void *info, void *buffer, int len);
#endif

static dbus_intf_callbacks_t dbus_usb_intf_cbs = {
	dbus_usb_send_irb_timeout,
	dbus_usb_send_irb_complete,
	dbus_usb_recv_irb_complete,
	dbus_usb_errhandler,
	dbus_usb_ctl_complete,
	dbus_usb_state_change,
	NULL,			/* isr */
	NULL,			/* dpc */
	NULL,			/* watchdog */
	NULL,			/* dbus_if_pktget */
	NULL, 			/* dbus_if_pktfree */
	dbus_usb_getirb,
	dbus_usb_rxerr_indicate
};

/*
 * Need global for probe() and disconnect() since
 * attach() is not called at probe and detach()
 * can be called inside disconnect()
 */
static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

/* 
 * dbus_intf_t common to all USB
 * These functions override dbus_usb_<os>.c.
 */
static void *dbus_usb_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_usb_detach(dbus_pub_t *pub, void *info);

/* FIX: g_usb_info needed for over-ridden functions
 * since the bus argument is actually from dbus_usb_<os>.c.
 */
static dbus_intf_t *g_dbusintf = NULL;
static dbus_intf_t dbus_usb_intf;

static void * dbus_usb_probe(void *arg, const char *desc, uint32 bustype, uint32 hdrlen);

/* functions */
static void *
dbus_usb_probe(void *arg, const char *desc, uint32 bustype, uint32 hdrlen)
{
	if (probe_cb) {

		if (g_dbusintf != NULL) {
			/* First, initialize all lower-level functions as default
			 * so that dbus.c simply calls directly to dbus_usb_os.c.
			 */
			bcopy(g_dbusintf, &dbus_usb_intf, sizeof(dbus_intf_t));

			/* Second, selectively override functions we need, if any. */
			dbus_usb_intf.attach = dbus_usb_attach;
			dbus_usb_intf.detach = dbus_usb_detach;

#ifdef BCM_DNGL_EMBEDIMAGE
			dbus_usb_intf.dlstart = dbus_usb_dlstart;
			dbus_usb_intf.dlneeded = dbus_usb_dlneeded;
			dbus_usb_intf.dlrun = dbus_usb_dlrun;
			dbus_usb_intf.device_exists = dbus_usb_device_exists;
#endif
		}

		disc_arg = probe_cb(probe_arg, "DBUS USB", USB_BUS, hdrlen);
		return disc_arg;
	}

	return NULL;
}

int
dbus_bus_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	int err;

	probe_cb = prcb;
	disconnect_cb = discb;
	probe_arg = prarg;

	*intf = &dbus_usb_intf;

	err = dbus_bus_osl_register(vid, pid, dbus_usb_probe,
		dbus_usb_disconnect, NULL, &g_dbusintf, param1, param2);

	ASSERT(g_dbusintf);
	return err;
}

int
dbus_bus_deregister()
{
	return dbus_bus_osl_deregister();
}

void *
dbus_usb_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	usb_info_t *usb_info;

	if ((g_dbusintf == NULL) || (g_dbusintf->attach == NULL))
		return NULL;

	/* Sanity check for BUS_INFO() */
	ASSERT(OFFSETOF(usb_info_t, pub) == 0);

	usb_info = MALLOC(pub->osh, sizeof(usb_info_t));
	if (usb_info == NULL)
		return NULL;

	bzero(usb_info, sizeof(usb_info_t));

	usb_info->pub = pub;
	usb_info->cbarg = cbarg;
	usb_info->cbs = cbs;

	usb_info->usbosl_info = (dbus_pub_t *)g_dbusintf->attach(pub,
		usb_info, &dbus_usb_intf_cbs);
	if (usb_info->usbosl_info == NULL) {
		MFREE(pub->osh, usb_info, sizeof(usb_info_t));
		return NULL;
	}

	/* Save USB OS-specific driver entry points */
	usb_info->drvintf = g_dbusintf;

	pub->bus = usb_info;

#ifndef BCM_DNGL_EMBEDIMAGE
	usb_info->pub->busstate = DBUS_STATE_DL_DONE;
	if (dbus_usb_resetcfg(usb_info)) {
		return NULL;
	}
#endif
	/* Return Lower layer info */
	return (void *) usb_info->usbosl_info;
}

void
dbus_usb_detach(dbus_pub_t *pub, void *info)
{
	usb_info_t *usb_info = (usb_info_t *) pub->bus;
	osl_t *osh = pub->osh;

	if (usb_info == NULL)
		return;

	if (usb_info->drvintf && usb_info->drvintf->detach)
		usb_info->drvintf->detach(pub, usb_info->usbosl_info);

	MFREE(osh, usb_info, sizeof(usb_info_t));
}

void
dbus_usb_disconnect(void *handle)
{
	if (disconnect_cb)
		disconnect_cb(disc_arg);
}

static void
dbus_usb_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->send_irb_timeout)
		usb_info->cbs->send_irb_timeout(usb_info->cbarg, txirb);
}

static void
dbus_usb_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->send_irb_complete)
		usb_info->cbs->send_irb_complete(usb_info->cbarg, txirb, status);
}

static void
dbus_usb_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->recv_irb_complete)
		usb_info->cbs->recv_irb_complete(usb_info->cbarg, rxirb, status);
}

struct dbus_irb*
dbus_usb_getirb(void *handle, bool send)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return NULL;

	if (usb_info->cbs && usb_info->cbs->getirb)
		return usb_info->cbs->getirb(usb_info->cbarg, send);

	return NULL;
}

static void
dbus_usb_rxerr_indicate(void *handle, bool on)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->rxerr_indicate)
		usb_info->cbs->rxerr_indicate(usb_info->cbarg, on);
}

static void
dbus_usb_errhandler(void *handle, int err)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->errhandler)
		usb_info->cbs->errhandler(usb_info->cbarg, err);
}

static void
dbus_usb_ctl_complete(void *handle, int type, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->ctl_complete)
		usb_info->cbs->ctl_complete(usb_info->cbarg, type, status);
}

static void
dbus_usb_state_change(void *handle, int state)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->state_change)
		usb_info->cbs->state_change(usb_info->cbarg, state);
}

static int dbus_usb_resetcfg(usb_info_t *usbinfo)
{
	void *osinfo;
	bootrom_id_t id;
	uint16 wait = 0, wait_time;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Give dongle chance to boot */
	wait_time = USB_SFLASH_DLIMAGE_SPINWAIT;
	while (wait < USB_SFLASH_DLIMAGE_LIMIT) {
		dbus_usbos_wait(osinfo, wait_time);

		wait += wait_time;

		id.chip = 0xDEAD;       /* Get the ID */
		dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));
		id.chip = ltoh32(id.chip);

		if (id.chip == POSTBOOT_ID)
			break;
	}

	if (id.chip == POSTBOOT_ID) {
		DBUSERR(("%s: download done %d ms chip 0x%x rev 0x%x\n",
			__FUNCTION__, wait, id.chip, id.chiprev));

		dbus_usbos_dl_cmd(osinfo, DL_RESETCFG, &id, sizeof(bootrom_id_t));

		dbus_usbos_wait(osinfo, USB_RESETCFG_SPINWAIT);
		return DBUS_OK;
	} else {
		DBUSERR(("%s: ***** Error! Cannot talk to Dongle after download, %d ms \n",
			__FUNCTION__, wait));
		return DBUS_ERR;
	}

	return DBUS_OK;
}

#ifdef BCM_DNGL_EMBEDIMAGE
static int
dbus_usb_dl_writeimage(usb_info_t *usbinfo, uint8 *fw, int fwlen)
{
	osl_t *osh = usbinfo->pub->osh;
	void *osinfo = usbinfo->usbosl_info;
	unsigned int sendlen, sent, dllen;
	char *bulkchunk = NULL, *dlpos;
	rdl_state_t state;
	int err = DBUS_OK;
	bootrom_id_t id;
	uint16 wait, wait_time;

	bulkchunk = MALLOC(osh, RDL_CHUNK);
	if (bulkchunk == NULL) {
		err = DBUS_ERR;
		goto fail;
	}

	/* 1) Prepare USB boot loader for runtime image */
	dbus_usbos_dl_cmd(osinfo, DL_START, &state, sizeof(rdl_state_t));

	state.state = ltoh32(state.state);
	state.bytes = ltoh32(state.bytes);

	/* 2) Check we are in the Waiting state */
	if (state.state != DL_WAITING) {
		DBUSERR(("%s: Failed to DL_START\n", __FUNCTION__));
		err = DBUS_ERR;
		goto fail;
	}

	sent = 0;
	dlpos = fw;
	dllen = fwlen;

	id.chip = usbinfo->pub->attrib.devid;
	id.chiprev = usbinfo->pub->attrib.chiprev;

	/* 3) Load the image */
	while ((state.bytes != dllen)) {
		/* Wait until the usb device reports it received all the bytes we sent */

		if ((state.bytes == sent) && (state.bytes != dllen)) {
			if ((dllen-sent) < RDL_CHUNK)
				sendlen = dllen-sent;
			else
				sendlen = RDL_CHUNK;

			/* simply avoid having to send a ZLP by ensuring we never have an even
			 * multiple of 64
			 */
			if (!(sendlen % 64))
				sendlen -= 4;

			/* send data */
			memcpy(bulkchunk, dlpos, sendlen);
			if (!dbus_usbos_dl_send_bulk(osinfo, bulkchunk, sendlen)) {
				err = DBUS_ERR;
				goto fail;
			}

			dlpos += sendlen;
			sent += sendlen;
			DBUSTRACE(("%s: sendlen %d\n", __FUNCTION__, sendlen));
		}

		/* 43236a0 bootloader runs from sflash, which is slower than rom
		 * Wait for downloaded image crc check to complete in the dongle
		 */
		wait = 0;
		wait_time = USB_SFLASH_DLIMAGE_SPINWAIT;
		while (!dbus_usbos_dl_cmd(osinfo, DL_GETSTATE, &state,
			sizeof(rdl_state_t))) {
			if ((id.chip == 43236) && (id.chiprev == 0)) {
				DBUSERR(("%s: 43236a0 SFlash delay, waiting for dongle crc check "
					 "completion!!!\n", __FUNCTION__));
				dbus_usbos_wait(osinfo, wait_time);
				wait += wait_time;
				if (wait >= USB_SFLASH_DLIMAGE_LIMIT) {
					DBUSERR(("%s: DL_GETSTATE Failed xxxx\n", __FUNCTION__));
					err = DBUS_ERR;
					goto fail;
					break;
				}
			} else {
				DBUSERR(("%s: DL_GETSTATE Failed xxxx\n", __FUNCTION__));
				err = DBUS_ERR;
				goto fail;
			}
		}

		state.state = ltoh32(state.state);
		state.bytes = ltoh32(state.bytes);

		/* restart if an error is reported */
		if ((state.state == DL_BAD_HDR) || (state.state == DL_BAD_CRC)) {
			DBUSERR(("%s: Bad Hdr or Bad CRC\n", __FUNCTION__));
			err = DBUS_ERR;
			goto fail;
		}

	}
fail:
	if (bulkchunk)
		MFREE(osh, bulkchunk, RDL_CHUNK);

	return err;
}

static int
dbus_usb_dlstart(void *bus, uint8 *fw, int len)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	int err;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	if (USB_DEV_ISBAD(usbinfo))
		return DBUS_ERR;

	err = dbus_usb_dl_writeimage(usbinfo, fw, len);
	if (err == DBUS_OK)
		usbinfo->pub->busstate = DBUS_STATE_DL_DONE;
	else
		usbinfo->pub->busstate = DBUS_STATE_DL_PENDING;

	return err;
}

static bool
dbus_usb_dlneeded(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	bootrom_id_t id;
	bool needed = TRUE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return FALSE;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Check if firmware downloaded already by querying runtime ID */
	id.chip = 0xDEAD;
	dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));

	id.chip = ltoh32(id.chip);
	id.chiprev = ltoh32(id.chiprev);

	DBUSERR(("%s: chip 0x%x rev 0x%x\n", __FUNCTION__, id.chip, id.chiprev));
	if (id.chip == POSTBOOT_ID) {
		/* This code is  needed to support two enumerations on USB1.1 scenario */
		DBUSERR(("%s: Firmware already downloaded\n", __FUNCTION__));

		dbus_usbos_dl_cmd(osinfo, DL_RESETCFG, &id, sizeof(bootrom_id_t));
		needed = FALSE;
		if (usbinfo->pub->busstate == DBUS_STATE_DL_PENDING)
			usbinfo->pub->busstate = DBUS_STATE_DL_DONE;
	} else {
		usbinfo->pub->attrib.devid = id.chip;
		usbinfo->pub->attrib.chiprev = id.chiprev;
	}

	return needed;
}

static int
dbus_usb_dlrun(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	rdl_state_t state;
	int err = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	if (USB_DEV_ISBAD(usbinfo))
		return DBUS_ERR;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Check we are runnable */
	dbus_usbos_dl_cmd(osinfo, DL_GETSTATE, &state, sizeof(rdl_state_t));

	state.state = ltoh32(state.state);
	state.bytes = ltoh32(state.bytes);

	/* Start the image */
	if (state.state == DL_RUNNABLE) {
		DBUSERR(("%s: Issue DL_GO\n", __FUNCTION__));
		dbus_usbos_dl_cmd(osinfo, DL_GO, &state, sizeof(rdl_state_t));

		/* FIX: Need this for 4326 for some reason
		 * Same issue under both Linux/Windows
		 */
		if (usbinfo->pub->attrib.devid == TEST_CHIP)
			dbus_usbos_wait(osinfo, USB_DLGO_SPINWAIT);

		err = dbus_usb_resetcfg(usbinfo);
	} else {
		DBUSERR(("%s: Dongle not runnable\n", __FUNCTION__));
		err = DBUS_ERR;
	}

	return err;
}

static bool
dbus_usb_device_exists(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	bootrom_id_t id;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return FALSE;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	id.chip = 0xDEAD;
	/* Query device to see if we get a response */
	dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));

	usbinfo->pub->attrib.devid = id.chip;
	if (id.chip == 0xDEAD)
		return FALSE;
	else
		return TRUE;
}
#endif /* BCM_DNGL_EMBEDIMAGE */
