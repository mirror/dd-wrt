/*
 * WPA Supplicant - Layer2 packet handling with Microsoft NDISUIO
 * Copyright (c) 2003-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This implementation requires Windows specific event loop implementation,
 * i.e., eloop_win.c. In addition, the NDISUIO connection is shared with
 * driver_ndis.c, so only that driver interface can be used and
 * CONFIG_USE_NDISUIO must be defined.
 */

#include "includes.h"
#include <winsock2.h>
#include <ntddndis.h>

#include "common.h"
#include "eloop.h"
#include "l2_packet.h"

/* from nuiouser.h */
#define FSCTL_NDISUIO_BASE      FILE_DEVICE_NETWORK
#define _NDISUIO_CTL_CODE(_Function, _Method, _Access) \
	CTL_CODE(FSCTL_NDISUIO_BASE, _Function, _Method, _Access)
#define IOCTL_NDISUIO_SET_ETHER_TYPE \
	_NDISUIO_CTL_CODE(0x202, METHOD_BUFFERED, \
			  FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/* From driver_ndis.c to shared the handle to NDISUIO */
HANDLE driver_ndis_get_ndisuio_handle(void);

/*
 * NDISUIO supports filtering of only one ethertype at the time, so we must
 * fake support for two (EAPOL and RSN pre-auth) by switching to pre-auth
 * whenever wpa_supplicant is trying to pre-authenticate and then switching
 * back to EAPOL when pre-authentication has been completed.
 */

struct l2_packet_data;

struct l2_packet_ndisuio_global {
	int refcount;
	unsigned short first_proto;
	struct l2_packet_data *l2[2];
};

static struct l2_packet_ndisuio_global *l2_ndisuio_global = NULL;

struct l2_packet_data {
	char ifname[100];
	u8 own_addr[ETH_ALEN];
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len);
	void *rx_callback_ctx;
	int l2_hdr; /* whether to include layer 2 (Ethernet) header in calls to
		     * rx_callback and l2_packet_send() */
	HANDLE rx_avail;
	OVERLAPPED rx_overlapped;
	u8 rx_buf[1514];
	DWORD rx_written;
};


int l2_packet_get_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	memcpy(addr, l2->own_addr, ETH_ALEN);
	return 0;
}


int l2_packet_send(struct l2_packet_data *l2, const u8 *dst_addr, u16 proto,
		   const u8 *buf, size_t len)
{
	BOOL res;
	DWORD written;
	struct l2_ethhdr *eth;
	OVERLAPPED overlapped;

	if (l2 == NULL)
		return -1;

	memset(&overlapped, 0, sizeof(overlapped));

	if (l2->l2_hdr) {
		res = WriteFile(driver_ndis_get_ndisuio_handle(), buf, len,
			&written, &overlapped);
	} else {
		size_t mlen = sizeof(*eth) + len;
		eth = malloc(mlen);
		if (eth == NULL)
			return -1;

		memcpy(eth->h_dest, dst_addr, ETH_ALEN);
		memcpy(eth->h_source, l2->own_addr, ETH_ALEN);
		eth->h_proto = htons(proto);
		memcpy(eth + 1, buf, len);
		res = WriteFile(driver_ndis_get_ndisuio_handle(), eth, mlen,
			&written, &overlapped);
		free(eth);
	}

	if (!res) {
		DWORD err = GetLastError();
		if (err == ERROR_IO_PENDING) {
			/* For now, just assume that the packet will be sent in
			 * time before the next write happens. This could be
			 * cleaned up at some point to actually wait for
			 * completion before starting new writes.
			 */
			return 0;
		}
		wpa_printf(MSG_DEBUG, "L2(NDISUIO): WriteFile failed: %d",
			   (int) GetLastError());
		return -1;
	}

	return 0;
}


static void l2_packet_callback(struct l2_packet_data *l2);

static int l2_ndisuio_start_read(struct l2_packet_data *l2, int recursive)
{
	memset(&l2->rx_overlapped, 0, sizeof(l2->rx_overlapped));
	l2->rx_overlapped.hEvent = l2->rx_avail;
	if (!ReadFile(driver_ndis_get_ndisuio_handle(), l2->rx_buf,
		      sizeof(l2->rx_buf), &l2->rx_written, &l2->rx_overlapped))
	{
		DWORD err = GetLastError();
		if (err != ERROR_IO_PENDING) {
			wpa_printf(MSG_DEBUG, "L2(NDISUIO): ReadFile failed: "
				   "%d", (int) err);
			return -1;
		}
		/*
		 * Once read is completed, l2_packet_rx_event() will be
		 * called.
		 */
	} else {
		wpa_printf(MSG_DEBUG, "L2(NDISUIO): ReadFile returned data "
			   "without wait for completion");
		if (!recursive)
			l2_packet_callback(l2);
	}

	return 0;
}


static void l2_packet_callback(struct l2_packet_data *l2)
{
	const u8 *rx_buf, *rx_src;
	size_t rx_len;
	struct l2_ethhdr *ethhdr = (struct l2_ethhdr *) l2->rx_buf;

	wpa_printf(MSG_DEBUG, "L2(NDISUIO): Read %d bytes",
		   (int) l2->rx_written);

	if (l2->l2_hdr || l2->rx_written < sizeof(*ethhdr)) {
		rx_buf = (u8 *) ethhdr;
		rx_len = l2->rx_written;
	} else {
		rx_buf = (u8 *) (ethhdr + 1);
		rx_len = l2->rx_written - sizeof(*ethhdr);
	}
	rx_src = ethhdr->h_source;

	l2->rx_callback(l2->rx_callback_ctx, rx_src, rx_buf, rx_len);
	l2_ndisuio_start_read(l2, 1);
}


static void l2_packet_rx_event(void *eloop_data, void *user_data)
{
	struct l2_packet_data *l2 = eloop_data;

	if (l2_ndisuio_global)
		l2 = l2_ndisuio_global->l2[l2_ndisuio_global->refcount - 1];

	ResetEvent(l2->rx_avail);

	if (!GetOverlappedResult(driver_ndis_get_ndisuio_handle(),
				 &l2->rx_overlapped, &l2->rx_written, FALSE)) {
		wpa_printf(MSG_DEBUG, "L2(NDISUIO): GetOverlappedResult "
			   "failed: %d", (int) GetLastError());
		return;
	}

	l2_packet_callback(l2);
}


static int l2_ndisuio_set_ether_type(unsigned short protocol)
{
	USHORT proto = ntohs(protocol);
	DWORD written;

	if (!DeviceIoControl(driver_ndis_get_ndisuio_handle(),
			     IOCTL_NDISUIO_SET_ETHER_TYPE, &proto,
			     sizeof(proto), NULL, 0, &written, NULL)) {
		wpa_printf(MSG_ERROR, "L2(NDISUIO): "
			   "IOCTL_NDISUIO_SET_ETHER_TYPE failed: %d",
			   (int) GetLastError());
		return -1;
	}

	return 0;
}


struct l2_packet_data * l2_packet_init(
	const char *ifname, const u8 *own_addr, unsigned short protocol,
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len),
	void *rx_callback_ctx, int l2_hdr)
{
	struct l2_packet_data *l2;

	if (l2_ndisuio_global == NULL) {
		l2_ndisuio_global = wpa_zalloc(sizeof(*l2_ndisuio_global));
		if (l2_ndisuio_global == NULL)
			return NULL;
		l2_ndisuio_global->first_proto = protocol;
	}
	if (l2_ndisuio_global->refcount >= 2) {
		wpa_printf(MSG_ERROR, "L2(NDISUIO): Not more than two "
			   "simultaneous connections allowed");
		return NULL;
	}
	l2_ndisuio_global->refcount++;

	l2 = wpa_zalloc(sizeof(struct l2_packet_data));
	if (l2 == NULL)
		return NULL;
	l2_ndisuio_global->l2[l2_ndisuio_global->refcount - 1] = l2;

	strncpy(l2->ifname, ifname, sizeof(l2->ifname));
	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;
	l2->l2_hdr = l2_hdr;

	if (own_addr)
		memcpy(l2->own_addr, own_addr, ETH_ALEN);

	if (l2_ndisuio_set_ether_type(protocol) < 0) {
		free(l2);
		return NULL;
	}

	if (l2_ndisuio_global->refcount > 1) {
		wpa_printf(MSG_DEBUG, "L2(NDISUIO): Temporarily setting "
			   "filtering ethertype to %04x", protocol);
		if (l2_ndisuio_global->l2[0])
			l2->rx_avail = l2_ndisuio_global->l2[0]->rx_avail;
		return l2;
	}

	l2->rx_avail = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (l2->rx_avail == NULL) {
		free(l2);
		return NULL;
	}

	eloop_register_event(l2->rx_avail, sizeof(l2->rx_avail),
			     l2_packet_rx_event, l2, NULL);

	l2_ndisuio_start_read(l2, 0);

	return l2;
}


void l2_packet_deinit(struct l2_packet_data *l2)
{
	if (l2 == NULL)
		return;

	if (l2_ndisuio_global) {
		l2_ndisuio_global->refcount--;
		l2_ndisuio_global->l2[l2_ndisuio_global->refcount] = NULL;
		if (l2_ndisuio_global->refcount) {
			wpa_printf(MSG_DEBUG, "L2(NDISUIO): restore filtering "
				   "ethertype to %04x",
				   l2_ndisuio_global->first_proto);
			l2_ndisuio_set_ether_type(
				l2_ndisuio_global->first_proto);
			return;
		}
	}

	CancelIo(driver_ndis_get_ndisuio_handle());

	eloop_unregister_event(l2->rx_avail, sizeof(l2->rx_avail));
	CloseHandle(l2->rx_avail);
	free(l2);
}


int l2_packet_get_ip_addr(struct l2_packet_data *l2, char *buf, size_t len)
{
	return -1;
}


void l2_packet_notify_auth_start(struct l2_packet_data *l2)
{
}
