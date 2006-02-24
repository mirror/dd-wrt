/*
 * WPA Supplicant - driver interaction with generic Linux Wireless Extensions
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
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
 * This file implements a driver interface for the Linux Wireless Extensions.
 * When used with WE-18 or newer, this interface can be used as-is with number
 * of drivers. In addition to this, some of the common functions in this file
 * can be used by other driver interface implementations that use generic WE
 * ioctls, but require private ioctls for some of the functionality.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if_arp.h>

#include "wireless_copy.h"
#include "common.h"
#include "driver.h"
#include "l2_packet.h"
#include "eloop.h"
#include "wpa_supplicant.h"
#include "priv_netlink.h"
#include "driver_wext.h"
#include "wpa.h"


struct wpa_driver_wext_data {
	void *ctx;
	int event_sock;
	int ioctl_sock;
	char ifname[IFNAMSIZ + 1];
	int ifindex;
	int ifindex2;
	u8 *assoc_req_ies;
	size_t assoc_req_ies_len;
	u8 *assoc_resp_ies;
	size_t assoc_resp_ies_len;
	struct wpa_driver_capa capa;
	int has_capability;
	int we_version_compiled;
};


static int wpa_driver_wext_flush_pmkid(void *priv);
static int wpa_driver_wext_get_range(void *priv);


static int wpa_driver_wext_set_auth_param(struct wpa_driver_wext_data *drv,
					  int idx, u32 value)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.param.flags = idx & IW_AUTH_INDEX;
	iwr.u.param.value = value;

	if (ioctl(drv->ioctl_sock, SIOCSIWAUTH, &iwr) < 0) {
		perror("ioctl[SIOCSIWAUTH]");
		fprintf(stderr, "WEXT auth param %d value 0x%x - ",
			idx, value);
		ret = -1;
	}

	return ret;
}


/**
 * wpa_driver_wext_get_bssid - Get BSSID, SIOCGIWAP
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @bssid: Buffer for BSSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

	if (ioctl(drv->ioctl_sock, SIOCGIWAP, &iwr) < 0) {
		perror("ioctl[SIOCGIWAP]");
		ret = -1;
	}
	memcpy(bssid, iwr.u.ap_addr.sa_data, ETH_ALEN);

	return ret;
}


/**
 * wpa_driver_wext_set_bssid - Set BSSID, SIOCSIWAP
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @bssid: BSSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_bssid(void *priv, const u8 *bssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.ap_addr.sa_family = ARPHRD_ETHER;
	if (bssid)
		memcpy(iwr.u.ap_addr.sa_data, bssid, ETH_ALEN);
	else
		memset(iwr.u.ap_addr.sa_data, 0, ETH_ALEN);

	if (ioctl(drv->ioctl_sock, SIOCSIWAP, &iwr) < 0) {
		perror("ioctl[SIOCSIWAP]");
		ret = -1;
	}

	return ret;
}


/**
 * wpa_driver_wext_get_ssid - Get SSID, SIOCGIWESSID
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: Buffer for the SSID; must be at least 32 bytes long
 * Returns: SSID length on success, -1 on failure
 */
int wpa_driver_wext_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.essid.pointer = (caddr_t) ssid;
	iwr.u.essid.length = 32;

	if (ioctl(drv->ioctl_sock, SIOCGIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCGIWESSID]");
		ret = -1;
	} else
		ret = iwr.u.essid.length;

	return ret;
}


/**
 * wpa_driver_wext_set_ssid - Set SSID, SIOCSIWESSID
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: SSID
 * @ssid_len: Length of SSID (0..32)
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_ssid(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;
	char buf[33];

	if (ssid_len > 32)
		return -1;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	/* flags: 1 = ESSID is active, 0 = not (promiscuous) */
	iwr.u.essid.flags = (ssid_len != 0);
	memset(buf, 0, sizeof(buf));
	memcpy(buf, ssid, ssid_len);
	iwr.u.essid.pointer = (caddr_t) buf;
	/* For historic reasons, set SSID length to include one extra
	 * character, C string nul termination, even though SSID is really an
	 * octet string that should not be presented as a C string. Some Linux
	 * drivers decrement the length by one and can thus end up missing the
	 * last octet of the SSID if the length is not incremented here. */
	iwr.u.essid.length = ssid_len ? ssid_len + 1 : 0;

	if (ioctl(drv->ioctl_sock, SIOCSIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCSIWESSID]");
		ret = -1;
	}

	return ret;
}


/**
 * wpa_driver_wext_set_freq - Set frequency/channel, SIOCSIWFREQ
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @freq: Frequency in MHz
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_freq(void *priv, int freq)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.freq.m = freq * 100000;
	iwr.u.freq.e = 1;

	if (ioctl(drv->ioctl_sock, SIOCSIWFREQ, &iwr) < 0) {
		perror("ioctl[SIOCSIWFREQ]");
		ret = -1;
	}

	return ret;
}


static void
wpa_driver_wext_event_wireless_custom(struct wpa_driver_wext_data *drv,
				      void *ctx, char *custom)
{
	union wpa_event_data data;

	wpa_printf(MSG_DEBUG, "Custom wireless event: '%s'", custom);

	memset(&data, 0, sizeof(data));
	/* Host AP driver */
	if (strncmp(custom, "MLME-MICHAELMICFAILURE.indication", 33) == 0) {
		data.michael_mic_failure.unicast =
			strstr(custom, " unicast ") != NULL;
		/* TODO: parse parameters(?) */
		wpa_supplicant_event(ctx, EVENT_MICHAEL_MIC_FAILURE, &data);
	} else if (strncmp(custom, "ASSOCINFO(ReqIEs=", 17) == 0) {
		char *spos;
		int bytes;

		spos = custom + 17;

		bytes = strspn(spos, "0123456789abcdefABCDEF");
		if (!bytes || (bytes & 1))
			return;
		bytes /= 2;

		data.assoc_info.req_ies = malloc(bytes);
		if (data.assoc_info.req_ies == NULL)
			return;

		data.assoc_info.req_ies_len = bytes;
		hexstr2bin(spos, data.assoc_info.req_ies, bytes);

		spos += bytes * 2;

		data.assoc_info.resp_ies = NULL;
		data.assoc_info.resp_ies_len = 0;

		if (strncmp(spos, " RespIEs=", 9) == 0) {
			spos += 9;

			bytes = strspn(spos, "0123456789abcdefABCDEF");
			if (!bytes || (bytes & 1))
				goto done;
			bytes /= 2;

			data.assoc_info.resp_ies = malloc(bytes);
			if (data.assoc_info.resp_ies == NULL)
				goto done;

			data.assoc_info.resp_ies_len = bytes;
			hexstr2bin(spos, data.assoc_info.resp_ies, bytes);
		}

		wpa_supplicant_event(ctx, EVENT_ASSOCINFO, &data);

	done:
		free(data.assoc_info.resp_ies);
		free(data.assoc_info.req_ies);
	}
}


static int wpa_driver_wext_event_wireless_michaelmicfailure(
	struct wpa_driver_wext_data *drv, void *ctx, const char *ev, int len)
{
	const struct iw_michaelmicfailure *mic;
	union wpa_event_data data;

	if (len < sizeof(*mic))
		return -1;

	mic = (const struct iw_michaelmicfailure *) ev;

	wpa_printf(MSG_DEBUG, "Michael MIC failure wireless event: "
		   "flags=0x%x src_addr=" MACSTR, mic->flags,
		   MAC2STR(mic->src_addr.sa_data));

	memset(&data, 0, sizeof(data));
	data.michael_mic_failure.unicast = !(mic->flags & IW_MICFAILURE_GROUP);
	wpa_supplicant_event(ctx, EVENT_MICHAEL_MIC_FAILURE, &data);

	return 0;
}


static int wpa_driver_wext_event_wireless_pmkidcand(
	struct wpa_driver_wext_data *drv, void *ctx, const char *ev, int len)
{
	const struct iw_pmkid_cand *cand;
	union wpa_event_data data;
	const u8 *addr;

	if (len < sizeof(*cand))
		return -1;

	cand = (const struct iw_pmkid_cand *) ev;
	addr = (const u8 *) cand->bssid.sa_data;

	wpa_printf(MSG_DEBUG, "PMKID candidate wireless event: "
		   "flags=0x%x index=%d bssid=" MACSTR, cand->flags,
		   cand->index, MAC2STR(addr));

	memset(&data, 0, sizeof(data));
	memcpy(data.pmkid_candidate.bssid, addr, ETH_ALEN);
	data.pmkid_candidate.index = cand->index;
	data.pmkid_candidate.preauth = cand->flags & IW_PMKID_CAND_PREAUTH;
	wpa_supplicant_event(drv->ctx, EVENT_PMKID_CANDIDATE, &data);

	return 0;
}


static int wpa_driver_wext_event_wireless_assocreqie(
	struct wpa_driver_wext_data *drv, void *ctx, const char *ev, int len)
{
	if (len < 0)
		return -1;

	wpa_hexdump(MSG_DEBUG, "AssocReq IE wireless event", (const u8 *) ev,
		    len);
	free(drv->assoc_req_ies);
	drv->assoc_req_ies = malloc(len);
	if (drv->assoc_req_ies == NULL) {
		drv->assoc_req_ies_len = 0;
		return -1;
	}
	memcpy(drv->assoc_req_ies, ev, len);
	drv->assoc_req_ies_len = len;

	return 0;
}


static int wpa_driver_wext_event_wireless_assocrespie(
	struct wpa_driver_wext_data *drv, void *ctx, const char *ev, int len)
{
	if (len < 0)
		return -1;

	wpa_hexdump(MSG_DEBUG, "AssocResp IE wireless event", (const u8 *) ev,
		    len);
	free(drv->assoc_resp_ies);
	drv->assoc_resp_ies = malloc(len);
	if (drv->assoc_resp_ies == NULL) {
		drv->assoc_resp_ies_len = 0;
		return -1;
	}
	memcpy(drv->assoc_resp_ies, ev, len);
	drv->assoc_resp_ies_len = len;

	return 0;
}


static void wpa_driver_wext_event_assoc_ies(struct wpa_driver_wext_data *drv)
{
	union wpa_event_data data;

	if (drv->assoc_req_ies == NULL && drv->assoc_resp_ies == NULL)
		return;

	memset(&data, 0, sizeof(data));
	if (drv->assoc_req_ies) {
		data.assoc_info.req_ies = drv->assoc_req_ies;
		drv->assoc_req_ies = NULL;
		data.assoc_info.req_ies_len = drv->assoc_req_ies_len;
	}
	if (drv->assoc_resp_ies) {
		data.assoc_info.resp_ies = drv->assoc_resp_ies;
		drv->assoc_resp_ies = NULL;
		data.assoc_info.resp_ies_len = drv->assoc_resp_ies_len;
	}

	wpa_supplicant_event(drv->ctx, EVENT_ASSOCINFO, &data);

	free(data.assoc_info.req_ies);
	free(data.assoc_info.resp_ies);
}


static void wpa_driver_wext_event_wireless(struct wpa_driver_wext_data *drv,
					   void *ctx, char *data, int len)
{
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom, *buf;

	pos = data;
	end = data + len;

	while (pos + IW_EV_LCP_LEN <= end) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		wpa_printf(MSG_DEBUG, "Wireless event: cmd=0x%x len=%d",
			   iwe->cmd, iwe->len);
		if (iwe->len <= IW_EV_LCP_LEN)
			return;

		custom = pos + IW_EV_POINT_LEN;
		if (drv->we_version_compiled > 18 &&
		    (iwe->cmd == IWEVMICHAELMICFAILURE ||
		     iwe->cmd == IWEVCUSTOM ||
		     iwe->cmd == IWEVASSOCREQIE ||
		     iwe->cmd == IWEVASSOCRESPIE ||
		     iwe->cmd == IWEVPMKIDCAND)) {
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			memcpy(dpos, pos + IW_EV_LCP_LEN,
			       sizeof(struct iw_event) - dlen);
		} else {
			memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;
		}

		switch (iwe->cmd) {
		case SIOCGIWAP:
			wpa_printf(MSG_DEBUG, "Wireless event: new AP: "
				   MACSTR,
				   MAC2STR((u8 *) iwe->u.ap_addr.sa_data));
			if (memcmp(iwe->u.ap_addr.sa_data,
				   "\x00\x00\x00\x00\x00\x00", ETH_ALEN) == 0
			    ||
			    memcmp(iwe->u.ap_addr.sa_data,
				   "\x44\x44\x44\x44\x44\x44", ETH_ALEN) == 0)
			{
				free(drv->assoc_req_ies);
				drv->assoc_req_ies = NULL;
				free(drv->assoc_resp_ies);
				drv->assoc_resp_ies = NULL;
				wpa_supplicant_event(ctx, EVENT_DISASSOC,
						     NULL);
			
			} else {
				wpa_driver_wext_event_assoc_ies(drv);
				wpa_supplicant_event(ctx, EVENT_ASSOC, NULL);
			}
			break;
		case IWEVMICHAELMICFAILURE:
			wpa_driver_wext_event_wireless_michaelmicfailure(
				drv, ctx, custom, iwe->u.data.length);
			break;
		case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
				return;
			buf = malloc(iwe->u.data.length + 1);
			if (buf == NULL)
				return;
			memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';
			wpa_driver_wext_event_wireless_custom(drv, ctx, buf);
			free(buf);
			break;
		case SIOCGIWSCAN:
			eloop_cancel_timeout(wpa_driver_wext_scan_timeout,
					     drv, ctx);
			wpa_supplicant_event(ctx, EVENT_SCAN_RESULTS, NULL);
			break;
		case IWEVASSOCREQIE:
			wpa_driver_wext_event_wireless_assocreqie(
				drv, ctx, custom, iwe->u.data.length);
			break;
		case IWEVASSOCRESPIE:
			wpa_driver_wext_event_wireless_assocrespie(
				drv, ctx, custom, iwe->u.data.length);
			break;
		case IWEVPMKIDCAND:
			wpa_driver_wext_event_wireless_pmkidcand(
				drv, ctx, custom, iwe->u.data.length);
			break;
		}

		pos += iwe->len;
	}
}


static void wpa_driver_wext_event_link(struct wpa_driver_wext_data *drv,
				       void *ctx, char *buf, size_t len,
				       int del)
{
	union wpa_event_data event;

	memset(&event, 0, sizeof(event));
	if (len > sizeof(event.interface_status.ifname))
		len = sizeof(event.interface_status.ifname) - 1;
	memcpy(event.interface_status.ifname, buf, len);
	event.interface_status.ievent = del ? EVENT_INTERFACE_REMOVED :
		EVENT_INTERFACE_ADDED;

	wpa_printf(MSG_DEBUG, "RTM_%sLINK, IFLA_IFNAME: Interface '%s' %s",
		   del ? "DEL" : "NEW",
		   event.interface_status.ifname,
		   del ? "removed" : "added");

	wpa_supplicant_event(ctx, EVENT_INTERFACE_STATUS, &event);
}


static void wpa_driver_wext_event_rtm_newlink(struct wpa_driver_wext_data *drv,
					      void *ctx, struct nlmsghdr *h,
					      int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;

	if (len < sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);

	if (drv->ifindex != ifi->ifi_index && drv->ifindex2 != ifi->ifi_index)
	{
		wpa_printf(MSG_DEBUG, "Ignore event for foreign ifindex %d",
			   ifi->ifi_index);
		return;
	}

	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			wpa_driver_wext_event_wireless(
				drv, ctx, ((char *) attr) + rta_len,
				attr->rta_len - rta_len);
		} else if (attr->rta_type == IFLA_IFNAME) {
			wpa_driver_wext_event_link(drv, ctx,
						   ((char *) attr) + rta_len,
						   attr->rta_len - rta_len, 0);
		}
		attr = RTA_NEXT(attr, attrlen);
	}
}


static void wpa_driver_wext_event_rtm_dellink(struct wpa_driver_wext_data *drv,
					      void *ctx, struct nlmsghdr *h,
					      int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;

	if (len < sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);

	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_IFNAME) {
			wpa_driver_wext_event_link(drv, ctx,
						   ((char *) attr) + rta_len,
						   attr->rta_len - rta_len, 1);
		}
		attr = RTA_NEXT(attr, attrlen);
	}
}


static void wpa_driver_wext_event_receive(int sock, void *eloop_ctx,
					  void *sock_ctx)
{
	char buf[8192];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;

	fromlen = sizeof(from);
	left = recvfrom(sock, buf, sizeof(buf), MSG_DONTWAIT,
			(struct sockaddr *) &from, &fromlen);
	if (left < 0) {
		if (errno != EINTR && errno != EAGAIN)
			perror("recvfrom(netlink)");
		return;
	}

	h = (struct nlmsghdr *) buf;
	while (left >= sizeof(*h)) {
		int len, plen;

		len = h->nlmsg_len;
		plen = len - sizeof(*h);
		if (len > left || plen < 0) {
			wpa_printf(MSG_DEBUG, "Malformed netlink message: "
				   "len=%d left=%d plen=%d",
				   len, left, plen);
			break;
		}

		switch (h->nlmsg_type) {
		case RTM_NEWLINK:
			wpa_driver_wext_event_rtm_newlink(eloop_ctx, sock_ctx,
							  h, plen);
			break;
		case RTM_DELLINK:
			wpa_driver_wext_event_rtm_dellink(eloop_ctx, sock_ctx,
							  h, plen);
			break;
		}

		len = NLMSG_ALIGN(len);
		left -= len;
		h = (struct nlmsghdr *) ((char *) h + len);
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%d extra bytes in the end of netlink "
			   "message", left);
	}
}


/**
 * wpa_driver_wext_get_ifflags - Get interface flags (SIOCGIFFLAGS)
 * @drv: driver_wext private data
 * @flags: Pointer to returned flags value
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_get_ifflags(struct wpa_driver_wext_data *drv, int *flags)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, drv->ifname, IFNAMSIZ);
	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}
	*flags = ifr.ifr_flags & 0xffff;
	return 0;
}


/**
 * wpa_driver_wext_set_ifflags - Set interface flags (SIOCSIFFLAGS)
 * @drv: driver_wext private data
 * @flags: New value for flags
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_ifflags(struct wpa_driver_wext_data *drv, int flags)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, drv->ifname, IFNAMSIZ);
	ifr.ifr_flags = flags & 0xffff;
	if (ioctl(drv->ioctl_sock, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
		perror("SIOCSIFFLAGS");
		return -1;
	}
	return 0;
}


/**
 * wpa_driver_wext_init - Initialize WE driver interface
 * @ctx: context to be used when calling wpa_supplicant functions,
 * e.g., wpa_supplicant_event()
 * @ifname: interface name, e.g., wlan0
 * Returns: Pointer to private data, %NULL on failure
 */
void * wpa_driver_wext_init(void *ctx, const char *ifname)
{
	int s, flags;
	struct sockaddr_nl local;
	struct wpa_driver_wext_data *drv;

	drv = malloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	memset(drv, 0, sizeof(*drv));
	drv->ctx = ctx;
	strncpy(drv->ifname, ifname, sizeof(drv->ifname));

	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket(PF_INET,SOCK_DGRAM)");
		free(drv);
		return NULL;
	}

	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s < 0) {
		perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		close(drv->ioctl_sock);
		free(drv);
		return NULL;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind(netlink)");
		close(s);
		close(drv->ioctl_sock);
		free(drv);
		return NULL;
	}

	eloop_register_read_sock(s, wpa_driver_wext_event_receive, drv, ctx);
	drv->event_sock = s;

	/*
	 * Make sure that the driver does not have any obsolete PMKID entries.
	 */
	wpa_driver_wext_flush_pmkid(drv);

	if (wpa_driver_wext_set_mode(drv, 0) < 0) {
		printf("Could not configure driver to use managed mode\n");
	}

	if (wpa_driver_wext_get_ifflags(drv, &flags) != 0 ||
	    wpa_driver_wext_set_ifflags(drv, flags | IFF_UP) != 0) {
		printf("Could not set interface '%s' UP\n", drv->ifname);
	}

	wpa_driver_wext_get_range(drv);

	drv->ifindex = if_nametoindex(drv->ifname);

	if (strncmp(ifname, "wlan", 4) == 0) {
		/*
		 * Host AP driver may use both wlan# and wifi# interface in
		 * wireless events. Since some of the versions included WE-18
		 * support, let's add the alternative ifindex also from
		 * driver_wext.c for the time being. This may be removed at
		 * some point once it is believed that old versions of the
		 * driver are not in use anymore.
		 */
		char ifname2[IFNAMSIZ + 1];
		strncpy(ifname2, ifname, sizeof(ifname2));
		memcpy(ifname2, "wifi", 4);
		wpa_driver_wext_alternative_ifindex(drv, ifname2);
	}

	return drv;
}


/**
 * wpa_driver_wext_deinit - Deinitialize WE driver interface
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 *
 * Shut down driver interface and processing of driver events. Free
 * private data buffer if one was allocated in wpa_driver_wext_init().
 */
void wpa_driver_wext_deinit(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	int flags;

	/*
	 * Clear possibly configured driver parameters in order to make it
	 * easier to use the driver after wpa_supplicant has been terminated.
	 */
	wpa_driver_wext_set_bssid(drv, (u8 *) "\x00\x00\x00\x00\x00\x00");

	eloop_unregister_read_sock(drv->event_sock);

	if (wpa_driver_wext_get_ifflags(drv, &flags) == 0)
		(void) wpa_driver_wext_set_ifflags(drv, flags & ~IFF_UP);

	close(drv->event_sock);
	close(drv->ioctl_sock);
	free(drv->assoc_req_ies);
	free(drv->assoc_resp_ies);
	free(drv);
}


/**
 * wpa_driver_wext_scan_timeout - Scan timeout to report scan completion
 * @eloop_ctx: Unused
 * @timeout_ctx: ctx argument given to wpa_driver_wext_init()
 *
 * This function can be used as registered timeout when starting a scan to
 * generate a scan completed event if the driver does not report this.
 */
void wpa_driver_wext_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	wpa_printf(MSG_DEBUG, "Scan timeout - try to get results");
	wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, NULL);
}


/**
 * wpa_driver_wext_scan - Request the driver to initiate scan
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: Specific SSID to scan for (ProbeReq) or %NULL to scan for
 *	all SSIDs (either active scan with broadcast SSID or passive
 *	scan
 * @ssid_len: Length of the SSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;
	struct iw_scan_req req;

	if (ssid_len > IW_ESSID_MAX_SIZE) {
		wpa_printf(MSG_DEBUG, "%s: too long SSID (%lu)",
			   __FUNCTION__, (unsigned long) ssid_len);
		return -1;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

	if (ssid && ssid_len) {
		memset(&req, 0, sizeof(req));
		req.essid_len = ssid_len;
		req.bssid.sa_family = ARPHRD_ETHER;
		memset(req.bssid.sa_data, 0xff, ETH_ALEN);
		memcpy(req.essid, ssid, ssid_len);
		iwr.u.data.pointer = (caddr_t) &req;
		iwr.u.data.length = sizeof(req);
		iwr.u.data.flags = IW_SCAN_THIS_ESSID;
	}

	if (ioctl(drv->ioctl_sock, SIOCSIWSCAN, &iwr) < 0) {
		perror("ioctl[SIOCSIWSCAN]");
		ret = -1;
	}

	/* Not all drivers generate "scan completed" wireless event, so try to
	 * read results after a timeout. */
	eloop_register_timeout(3, 0, wpa_driver_wext_scan_timeout, drv,
			       drv->ctx);

	return ret;
}


/* Compare function for sorting scan results. Return >0 if @b is considered
 * better. */
static int wpa_scan_result_compar(const void *a, const void *b)
{
	const struct wpa_scan_result *wa = a;
	const struct wpa_scan_result *wb = b;

	/* WPA/WPA2 support preferred */
	if ((wb->wpa_ie_len || wb->rsn_ie_len) &&
	    !(wa->wpa_ie_len || wa->rsn_ie_len))
		return 1;
	if (!(wb->wpa_ie_len || wb->rsn_ie_len) &&
	    (wa->wpa_ie_len || wa->rsn_ie_len))
		return -1;

	/* privacy support preferred */
	if ((wa->caps & IEEE80211_CAP_PRIVACY) == 0 &&
	    (wb->caps & IEEE80211_CAP_PRIVACY))
		return 1;
	if ((wa->caps & IEEE80211_CAP_PRIVACY) &&
	    (wb->caps & IEEE80211_CAP_PRIVACY) == 0)
		return -1;

	/* best/max rate preferred if signal level close enough XXX */
	if (wa->maxrate != wb->maxrate && abs(wb->level - wa->level) < 5)
		return wb->maxrate - wa->maxrate;

	/* use freq for channel preference */

	/* all things being equal, use signal level */
	return wb->level - wa->level;
}


/**
 * wpa_driver_wext_get_scan_results - Fetch the latest scan results
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @results: Pointer to buffer for scan results
 * @max_size: Maximum number of entries (buffer size)
 * Returns: Number of scan result entries used on success, -1 on
 * failure
 *
 * If scan results include more than max_size BSSes, max_size will be
 * returned and the remaining entries will not be included in the
 * buffer.
 */
int wpa_driver_wext_get_scan_results(void *priv,
				     struct wpa_scan_result *results,
				     size_t max_size)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ap_num = 0, first, maxrate;
	u8 *res_buf;
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom, *genie, *gpos, *gend;
	struct iw_param p;
	size_t len, clen, res_buf_len;

	memset(results, 0, max_size * sizeof(struct wpa_scan_result));

	res_buf_len = IW_SCAN_MAX_DATA;
	for (;;) {
		res_buf = malloc(res_buf_len);
		if (res_buf == NULL)
			return -1;
		memset(&iwr, 0, sizeof(iwr));
		strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
		iwr.u.data.pointer = res_buf;
		iwr.u.data.length = res_buf_len;

		if (ioctl(drv->ioctl_sock, SIOCGIWSCAN, &iwr) == 0)
			break;

		if (errno == E2BIG && res_buf_len < 100000) {
			free(res_buf);
			res_buf = NULL;
			res_buf_len *= 2;
			wpa_printf(MSG_DEBUG, "Scan results did not fit - "
				   "trying larger buffer (%lu bytes)",
				   (unsigned long) res_buf_len);
		} else {
			perror("ioctl[SIOCGIWSCAN]");
			free(res_buf);
			return -1;
		}
	}

	len = iwr.u.data.length;
	ap_num = 0;
	first = 1;

	pos = (char *) res_buf;
	end = (char *) res_buf + len;

	while (pos + IW_EV_LCP_LEN <= end) {
		int ssid_len;
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		if (iwe->len <= IW_EV_LCP_LEN)
			break;

		custom = pos + IW_EV_POINT_LEN;
		if (drv->we_version_compiled > 18 &&
		    (iwe->cmd == SIOCGIWESSID ||
		     iwe->cmd == SIOCGIWENCODE ||
		     iwe->cmd == IWEVGENIE ||
		     iwe->cmd == IWEVCUSTOM)) {
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			memcpy(dpos, pos + IW_EV_LCP_LEN,
			       sizeof(struct iw_event) - dlen);
		} else {
			memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;
		}

		switch (iwe->cmd) {
		case SIOCGIWAP:
			if (!first)
				ap_num++;
			first = 0;
			if (ap_num < max_size) {
				memcpy(results[ap_num].bssid,
				       iwe->u.ap_addr.sa_data, ETH_ALEN);
			}
			break;
		case SIOCGIWMODE:
			if (ap_num >= max_size)
				break;
			if (iwe->u.mode == IW_MODE_ADHOC)
				results[ap_num].caps |= IEEE80211_CAP_IBSS;
			else if (iwe->u.mode == IW_MODE_MASTER ||
				 iwe->u.mode == IW_MODE_INFRA)
				results[ap_num].caps |= IEEE80211_CAP_ESS;
			break;
		case SIOCGIWESSID:
			ssid_len = iwe->u.essid.length;
			if (custom + ssid_len > end)
				break;
			if (iwe->u.essid.flags &&
			    ssid_len > 0 &&
			    ssid_len <= IW_ESSID_MAX_SIZE) {
				if (ap_num < max_size) {
					memcpy(results[ap_num].ssid, custom,
					       ssid_len);
					results[ap_num].ssid_len = ssid_len;
				}
			}
			break;
		case SIOCGIWFREQ:
			if (ap_num < max_size) {
				int divi = 1000000, i;
				if (iwe->u.freq.e > 6) {
					wpa_printf(
						MSG_DEBUG, "Invalid freq "
						"in scan results (BSSID="
						MACSTR ": m=%d e=%d\n",
						MAC2STR(results[ap_num].bssid),
						iwe->u.freq.m, iwe->u.freq.e);
					break;
				}
				for (i = 0; i < iwe->u.freq.e; i++)
					divi /= 10;
				results[ap_num].freq = iwe->u.freq.m / divi;
			}
			break;
		case IWEVQUAL:
			if (ap_num < max_size) {
				results[ap_num].qual = iwe->u.qual.qual;
				results[ap_num].noise = iwe->u.qual.noise;
				results[ap_num].level = iwe->u.qual.level;
			}
			break;
		case SIOCGIWENCODE:
			if (ap_num < max_size &&
			    !(iwe->u.data.flags & IW_ENCODE_DISABLED))
				results[ap_num].caps |= IEEE80211_CAP_PRIVACY;
			break;
		case SIOCGIWRATE:
			custom = pos + IW_EV_LCP_LEN;
			clen = iwe->len;
			if (custom + clen > end)
				break;
			maxrate = 0;
			while (((ssize_t) clen) >= sizeof(struct iw_param)) {
				/* Note: may be misaligned, make a local,
				 * aligned copy */
				memcpy(&p, custom, sizeof(struct iw_param));
				if (p.value > maxrate)
					maxrate = p.value;
				clen -= sizeof(struct iw_param);
				custom += sizeof(struct iw_param);
			}
			if (ap_num < max_size)
				results[ap_num].maxrate = maxrate;
			break;
		case IWEVGENIE:
			if (ap_num >= max_size)
				break;
			gpos = genie = custom;
			gend = genie + iwe->u.data.length;
			if (gend > end) {
				wpa_printf(MSG_INFO, "IWEVGENIE overflow");
				break;
			}
			while (gpos + 1 < gend &&
			       gpos + 2 + (u8) gpos[1] <= gend) {
				u8 ie = gpos[0], ielen = gpos[1] + 2;
				if (ielen > SSID_MAX_WPA_IE_LEN) {
					gpos += ielen;
					continue;
				}
				switch (ie) {
				case GENERIC_INFO_ELEM:
					if (ielen < 2 + 4 ||
					    memcmp(&gpos[2],
						   "\x00\x50\xf2\x01", 4) != 0)
						break;
					memcpy(results[ap_num].wpa_ie, gpos,
					       ielen);
					results[ap_num].wpa_ie_len = ielen;
					break;
				case RSN_INFO_ELEM:
					memcpy(results[ap_num].rsn_ie, gpos,
					       ielen);
					results[ap_num].rsn_ie_len = ielen;
					break;
				}
				gpos += ielen;
			}
			break;
		case IWEVCUSTOM:
			clen = iwe->u.data.length;
			if (custom + clen > end)
				break;
			if (clen > 7 && strncmp(custom, "wpa_ie=", 7) == 0 &&
			    ap_num < max_size) {
				char *spos;
				int bytes;
				spos = custom + 7;
				bytes = custom + clen - spos;
				if (bytes & 1)
					break;
				bytes /= 2;
				if (bytes > SSID_MAX_WPA_IE_LEN) {
					wpa_printf(MSG_INFO, "Too long WPA IE "
						   "(%d)", bytes);
					break;
				}
				hexstr2bin(spos, results[ap_num].wpa_ie,
					   bytes);
				results[ap_num].wpa_ie_len = bytes;
			} else if (clen > 7 &&
				   strncmp(custom, "rsn_ie=", 7) == 0 &&
				   ap_num < max_size) {
				char *spos;
				int bytes;
				spos = custom + 7;
				bytes = custom + clen - spos;
				if (bytes & 1)
					break;
				bytes /= 2;
				if (bytes > SSID_MAX_WPA_IE_LEN) {
					wpa_printf(MSG_INFO, "Too long RSN IE "
						   "(%d)", bytes);
					break;
				}
				hexstr2bin(spos, results[ap_num].rsn_ie,
					   bytes);
				results[ap_num].rsn_ie_len = bytes;
			}
			break;
		}

		pos += iwe->len;
	}
	free(res_buf);
	res_buf = NULL;
	if (!first)
		ap_num++;
	if (ap_num > max_size) {
		wpa_printf(MSG_DEBUG, "Too small scan result buffer - %d BSSes"
			   " but room only for %lu", ap_num,
			   (unsigned long) max_size);
		ap_num = max_size;
	}
	qsort(results, ap_num, sizeof(struct wpa_scan_result),
	      wpa_scan_result_compar);

	wpa_printf(MSG_DEBUG, "Received %lu bytes of scan results (%d BSSes)",
		   (unsigned long) len, ap_num);

	return ap_num;
}


static int wpa_driver_wext_get_range(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iw_range *range;
	struct iwreq iwr;
	int minlen;
	size_t buflen;

	/*
	 * Use larger buffer than struct iw_range in order to allow the
	 * structure to grow in the future.
	 */
	buflen = sizeof(struct iw_range) + 500;
	range = malloc(buflen);
	if (range == NULL)
		return -1;
	memset(range, 0, buflen);

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) range;
	iwr.u.data.length = buflen;

	minlen = ((char *) &range->enc_capa) - (char *) range +
		sizeof(range->enc_capa);

	if (ioctl(drv->ioctl_sock, SIOCGIWRANGE, &iwr) < 0) {
		perror("ioctl[SIOCGIWRANGE]");
		free(range);
		return -1;
	} else if (iwr.u.data.length >= minlen &&
		   range->we_version_compiled >= 18) {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: WE(compiled)=%d "
			   "WE(source)=%d enc_capa=0x%x",
			   range->we_version_compiled,
			   range->we_version_source,
			   range->enc_capa);
		drv->has_capability = 1;
		drv->we_version_compiled = range->we_version_compiled;
		if (range->enc_capa & IW_ENC_CAPA_WPA) {
			drv->capa.key_mgmt |= WPA_DRIVER_CAPA_KEY_MGMT_WPA |
				WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK;
		}
		if (range->enc_capa & IW_ENC_CAPA_WPA2) {
			drv->capa.key_mgmt |= WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
				WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK;
		}
		drv->capa.enc |= WPA_DRIVER_CAPA_ENC_WEP40 |
			WPA_DRIVER_CAPA_ENC_WEP104;
		if (range->enc_capa & IW_ENC_CAPA_CIPHER_TKIP)
			drv->capa.enc |= WPA_DRIVER_CAPA_ENC_TKIP;
		if (range->enc_capa & IW_ENC_CAPA_CIPHER_CCMP)
			drv->capa.enc |= WPA_DRIVER_CAPA_ENC_CCMP;
		wpa_printf(MSG_DEBUG, "  capabilities: key_mgmt 0x%x enc 0x%x",
			   drv->capa.key_mgmt, drv->capa.enc);
	} else {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: too old (short) data - "
			   "assuming WPA is not supported");
	}

	free(range);
	return 0;
}


static int wpa_driver_wext_set_wpa(void *priv, int enabled)
{
	struct wpa_driver_wext_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);

	return wpa_driver_wext_set_auth_param(drv, IW_AUTH_WPA_ENABLED,
					      enabled);
}


static int wpa_driver_wext_set_key_ext(void *priv, wpa_alg alg,
				       const u8 *addr, int key_idx,
				       int set_tx, const u8 *seq,
				       size_t seq_len,
				       const u8 *key, size_t key_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;
	struct iw_encode_ext *ext;

	if (seq_len > IW_ENCODE_SEQ_MAX_SIZE) {
		wpa_printf(MSG_DEBUG, "%s: Invalid seq_len %lu",
			   __FUNCTION__, (unsigned long) seq_len);
		return -1;
	}

	ext = malloc(sizeof(*ext) + key_len);
	if (ext == NULL)
		return -1;
	memset(ext, 0, sizeof(*ext) + key_len);
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.encoding.flags = key_idx + 1;
	if (alg == WPA_ALG_NONE)
		iwr.u.encoding.flags |= IW_ENCODE_DISABLED;
	iwr.u.encoding.pointer = (caddr_t) ext;
	iwr.u.encoding.length = sizeof(*ext) + key_len;

	if (addr == NULL ||
	    memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0)
		ext->ext_flags |= IW_ENCODE_EXT_GROUP_KEY;
	if (set_tx)
		ext->ext_flags |= IW_ENCODE_EXT_SET_TX_KEY;

	ext->addr.sa_family = ARPHRD_ETHER;
	if (addr)
		memcpy(ext->addr.sa_data, addr, ETH_ALEN);
	else
		memset(ext->addr.sa_data, 0xff, ETH_ALEN);
	if (key && key_len) {
		memcpy(ext + 1, key, key_len);
		ext->key_len = key_len;
	}
	switch (alg) {
	case WPA_ALG_NONE:
		ext->alg = IW_ENCODE_ALG_NONE;
		break;
	case WPA_ALG_WEP:
		ext->alg = IW_ENCODE_ALG_WEP;
		break;
	case WPA_ALG_TKIP:
		ext->alg = IW_ENCODE_ALG_TKIP;
		break;
	case WPA_ALG_CCMP:
		ext->alg = IW_ENCODE_ALG_CCMP;
		break;
	default:
		wpa_printf(MSG_DEBUG, "%s: Unknown algorithm %d",
			   __FUNCTION__, alg);
		free(ext);
		return -1;
	}

	if (seq && seq_len) {
		ext->ext_flags |= IW_ENCODE_EXT_RX_SEQ_VALID;
		memcpy(ext->rx_seq, seq, seq_len);
	}

	if (ioctl(drv->ioctl_sock, SIOCSIWENCODEEXT, &iwr) < 0) {
		ret = errno == EOPNOTSUPP ? -2 : -1;
		if (errno == ENODEV) {
			/*
			 * ndiswrapper seems to be returning incorrect error
			 * code.. */
			ret = -2;
		}

		perror("ioctl[SIOCSIWENCODEEXT]");
	}

	free(ext);
	return ret;
}


/**
 * wpa_driver_wext_set_key - Configure encryption key
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @priv: Private driver interface data
 * @alg: Encryption algorithm (%WPA_ALG_NONE, %WPA_ALG_WEP,
 *	%WPA_ALG_TKIP, %WPA_ALG_CCMP); %WPA_ALG_NONE clears the key.
 * @addr: Address of the peer STA or ff:ff:ff:ff:ff:ff for
 *	broadcast/default keys
 * @key_idx: key index (0..3), usually 0 for unicast keys
 * @set_tx: Configure this key as the default Tx key (only used when
 *	driver does not support separate unicast/individual key
 * @seq: Sequence number/packet number, seq_len octets, the next
 *	packet number to be used for in replay protection; configured
 *	for Rx keys (in most cases, this is only used with broadcast
 *	keys and set to zero for unicast keys)
 * @seq_len: Length of the seq, depends on the algorithm:
 *	TKIP: 6 octets, CCMP: 6 octets
 * @key: Key buffer; TKIP: 16-byte temporal key, 8-byte Tx Mic key,
 *	8-byte Rx Mic Key
 * @key_len: Length of the key buffer in octets (WEP: 5 or 13,
 *	TKIP: 32, CCMP: 16)
 * Returns: 0 on success, -1 on failure
 *
 * This function uses SIOCSIWENCODEEXT by default, but tries to use
 * SIOCSIWENCODE if the extended ioctl fails when configuring a WEP key.
 */
int wpa_driver_wext_set_key(void *priv, wpa_alg alg,
			    const u8 *addr, int key_idx,
			    int set_tx, const u8 *seq, size_t seq_len,
			    const u8 *key, size_t key_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	wpa_printf(MSG_DEBUG, "%s: alg=%d key_idx=%d set_tx=%d seq_len=%lu "
		   "key_len=%lu",
		   __FUNCTION__, alg, key_idx, set_tx,
		   (unsigned long) seq_len, (unsigned long) key_len);

	ret = wpa_driver_wext_set_key_ext(drv, alg, addr, key_idx, set_tx,
					  seq, seq_len, key, key_len);
	if (ret == 0)
		return 0;

	if (ret == -2 &&
	    (alg == WPA_ALG_NONE || alg == WPA_ALG_WEP)) {
		wpa_printf(MSG_DEBUG, "Driver did not support "
			   "SIOCSIWENCODEEXT, trying SIOCSIWENCODE");
		ret = 0;
	} else {
		wpa_printf(MSG_DEBUG, "Driver did not support "
			   "SIOCSIWENCODEEXT");
		return ret;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.encoding.flags = key_idx + 1;
	if (alg == WPA_ALG_NONE)
		iwr.u.encoding.flags |= IW_ENCODE_DISABLED;
	iwr.u.encoding.pointer = (caddr_t) key;
	iwr.u.encoding.length = key_len;

	if (ioctl(drv->ioctl_sock, SIOCSIWENCODE, &iwr) < 0) {
		perror("ioctl[SIOCSIWENCODE]");
		ret = -1;
	}

	if (set_tx && alg != WPA_ALG_NONE) {
		memset(&iwr, 0, sizeof(iwr));
		strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
		iwr.u.encoding.flags = key_idx + 1;
		iwr.u.encoding.pointer = (caddr_t) key;
		iwr.u.encoding.length = 0;
		if (ioctl(drv->ioctl_sock, SIOCSIWENCODE, &iwr) < 0) {
			perror("ioctl[SIOCSIWENCODE] (set_tx)");
			ret = -1;
		}
	}

	return ret;
}


static int wpa_driver_wext_set_countermeasures(void *priv,
					       int enabled)
{
	struct wpa_driver_wext_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	return wpa_driver_wext_set_auth_param(drv,
					      IW_AUTH_TKIP_COUNTERMEASURES,
					      enabled);
}


static int wpa_driver_wext_set_drop_unencrypted(void *priv,
						int enabled)
{
	struct wpa_driver_wext_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	return wpa_driver_wext_set_auth_param(drv, IW_AUTH_DROP_UNENCRYPTED,
					      enabled);
}


static int wpa_driver_wext_mlme(struct wpa_driver_wext_data *drv,
				const u8 *addr, int cmd, int reason_code)
{
	struct iwreq iwr;
	struct iw_mlme mlme;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	memset(&mlme, 0, sizeof(mlme));
	mlme.cmd = cmd;
	mlme.reason_code = reason_code;
	mlme.addr.sa_family = ARPHRD_ETHER;
	memcpy(mlme.addr.sa_data, addr, ETH_ALEN);
	iwr.u.data.pointer = (caddr_t) &mlme;
	iwr.u.data.length = sizeof(mlme);

	if (ioctl(drv->ioctl_sock, SIOCSIWMLME, &iwr) < 0) {
		perror("ioctl[SIOCSIWMLME]");
		ret = -1;
	}

	return ret;
}


static int wpa_driver_wext_deauthenticate(void *priv, const u8 *addr,
					  int reason_code)
{
	struct wpa_driver_wext_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	return wpa_driver_wext_mlme(drv, addr, IW_MLME_DEAUTH, reason_code);
}


static int wpa_driver_wext_disassociate(void *priv, const u8 *addr,
					int reason_code)
{
	struct wpa_driver_wext_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	return wpa_driver_wext_mlme(drv, addr, IW_MLME_DISASSOC,
				    reason_code);
}


static int wpa_driver_wext_set_gen_ie(void *priv, const u8 *ie,
				      size_t ie_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) ie;
	iwr.u.data.length = ie_len;

	if (ioctl(drv->ioctl_sock, SIOCSIWGENIE, &iwr) < 0) {
		perror("ioctl[SIOCSIWGENIE]");
		ret = -1;
	}

	return ret;
}


static int wpa_driver_wext_cipher2wext(int cipher)
{
	switch (cipher) {
	case CIPHER_NONE:
		return IW_AUTH_CIPHER_NONE;
	case CIPHER_WEP40:
		return IW_AUTH_CIPHER_WEP40;
	case CIPHER_TKIP:
		return IW_AUTH_CIPHER_TKIP;
	case CIPHER_CCMP:
		return IW_AUTH_CIPHER_CCMP;
	case CIPHER_WEP104:
		return IW_AUTH_CIPHER_WEP104;
	default:
		return 0;
	}
}


static int wpa_driver_wext_keymgmt2wext(int keymgmt)
{
	switch (keymgmt) {
	case KEY_MGMT_802_1X:
		return IW_AUTH_KEY_MGMT_802_1X;
	case KEY_MGMT_PSK:
		return IW_AUTH_KEY_MGMT_PSK;
	default:
		return 0;
	}
}


static int
wpa_driver_wext_associate(void *priv,
			  struct wpa_driver_associate_params *params)
{
	struct wpa_driver_wext_data *drv = priv;
	int ret = 0;
	int allow_unencrypted_eapol;
	int value;

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);

	if (!params->bssid &&
	    wpa_driver_wext_set_bssid(drv, NULL) < 0)
		ret = -1;

	if (wpa_driver_wext_set_mode(drv, params->mode) < 0)
		ret = -1;
	/* TODO: should consider getting wpa version and cipher/key_mgmt suites
	 * from configuration, not from here, where only the selected suite is
	 * available */
	if (wpa_driver_wext_set_gen_ie(drv, params->wpa_ie, params->wpa_ie_len)
	    < 0)
		ret = -1;
	if (params->wpa_ie == NULL || params->wpa_ie_len == 0)
		value = IW_AUTH_WPA_VERSION_DISABLED;
	else if (params->wpa_ie[0] == RSN_INFO_ELEM)
		value = IW_AUTH_WPA_VERSION_WPA2;
	else
		value = IW_AUTH_WPA_VERSION_WPA;
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_WPA_VERSION, value) < 0)
		ret = -1;
	value = wpa_driver_wext_cipher2wext(params->pairwise_suite);
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_CIPHER_PAIRWISE, value) < 0)
		ret = -1;
	value = wpa_driver_wext_cipher2wext(params->group_suite);
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_CIPHER_GROUP, value) < 0)
		ret = -1;
	value = wpa_driver_wext_keymgmt2wext(params->key_mgmt_suite);
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_KEY_MGMT, value) < 0)
		ret = -1;
	value = params->key_mgmt_suite != KEY_MGMT_NONE;
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_PRIVACY_INVOKED, value) < 0)
		ret = -1;

	/* Allow unencrypted EAPOL messages even if pairwise keys are set when
	 * not using WPA. IEEE 802.1X specifies that these frames are not
	 * encrypted, but WPA encrypts them when pairwise keys are in use. */
	if (params->key_mgmt_suite == KEY_MGMT_802_1X ||
	    params->key_mgmt_suite == KEY_MGMT_PSK)
		allow_unencrypted_eapol = 0;
	else
		allow_unencrypted_eapol = 1;
	
	if (wpa_driver_wext_set_auth_param(drv,
					   IW_AUTH_RX_UNENCRYPTED_EAPOL,
					   allow_unencrypted_eapol) < 0)
		ret = -1;
	if (params->freq && wpa_driver_wext_set_freq(drv, params->freq) < 0)
		ret = -1;
	if (wpa_driver_wext_set_ssid(drv, params->ssid, params->ssid_len) < 0)
		ret = -1;
	if (params->bssid &&
	    wpa_driver_wext_set_bssid(drv, params->bssid) < 0)
		ret = -1;

	return ret;
}


static int wpa_driver_wext_set_auth_alg(void *priv, int auth_alg)
{
	struct wpa_driver_wext_data *drv = priv;
	int algs = 0;

	if (auth_alg & AUTH_ALG_OPEN_SYSTEM)
		algs |= IW_AUTH_ALG_OPEN_SYSTEM;
	if (auth_alg & AUTH_ALG_SHARED_KEY)
		algs |= IW_AUTH_ALG_SHARED_KEY;
	if (auth_alg & AUTH_ALG_LEAP)
		algs |= IW_AUTH_ALG_LEAP;
	if (algs == 0) {
		/* at least one algorithm should be set */
		algs = IW_AUTH_ALG_OPEN_SYSTEM;
	}

	return wpa_driver_wext_set_auth_param(drv,
					      IW_AUTH_80211_AUTH_ALG, algs);
}


/**
 * wpa_driver_wext_set_mode - Set wireless mode (infra/adhoc), SIOCSIWMODE
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @mode: 0 = infra/BSS (associate with an AP), 1 = adhoc/IBSS
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_mode(void *priv, int mode)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.mode = mode ? IW_MODE_ADHOC : IW_MODE_INFRA;

	if (ioctl(drv->ioctl_sock, SIOCSIWMODE, &iwr) < 0) {
		perror("ioctl[SIOCSIWMODE]");
		ret = -1;
	}

	return ret;
}


static int wpa_driver_wext_pmksa(struct wpa_driver_wext_data *drv,
				 u32 cmd, const u8 *bssid, const u8 *pmkid)
{
	struct iwreq iwr;
	struct iw_pmksa pmksa;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	memset(&pmksa, 0, sizeof(pmksa));
	pmksa.cmd = cmd;
	pmksa.bssid.sa_family = ARPHRD_ETHER;
	if (bssid)
		memcpy(pmksa.bssid.sa_data, bssid, ETH_ALEN);
	if (pmkid)
		memcpy(pmksa.pmkid, pmkid, IW_PMKID_LEN);
	iwr.u.data.pointer = (caddr_t) &pmksa;
	iwr.u.data.length = sizeof(pmksa);

	if (ioctl(drv->ioctl_sock, SIOCSIWPMKSA, &iwr) < 0) {
		if (errno != EOPNOTSUPP)
			perror("ioctl[SIOCSIWPMKSA]");
		ret = -1;
	}

	return ret;
}


static int wpa_driver_wext_add_pmkid(void *priv, const u8 *bssid,
				     const u8 *pmkid)
{
	struct wpa_driver_wext_data *drv = priv;
	return wpa_driver_wext_pmksa(drv, IW_PMKSA_ADD, bssid, pmkid);
}


static int wpa_driver_wext_remove_pmkid(void *priv, const u8 *bssid,
		 			const u8 *pmkid)
{
	struct wpa_driver_wext_data *drv = priv;
	return wpa_driver_wext_pmksa(drv, IW_PMKSA_REMOVE, bssid, pmkid);
}


static int wpa_driver_wext_flush_pmkid(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	return wpa_driver_wext_pmksa(drv, IW_PMKSA_FLUSH, NULL, NULL);
}


static int wpa_driver_wext_get_capa(void *priv, struct wpa_driver_capa *capa)
{
	struct wpa_driver_wext_data *drv = priv;
	if (!drv->has_capability)
		return -1;
	memcpy(capa, &drv->capa, sizeof(*capa));
	return 0;
}


int wpa_driver_wext_alternative_ifindex(struct wpa_driver_wext_data *drv,
					const char *ifname)
{
	if (ifname == NULL) {
		drv->ifindex2 = -1;
		return 0;
	}

	drv->ifindex2 = if_nametoindex(ifname);
	if (drv->ifindex2 <= 0)
		return -1;

	wpa_printf(MSG_DEBUG, "Added alternative ifindex %d (%s) for "
		   "wireless events", drv->ifindex2, ifname);

	return 0;
}


const struct wpa_driver_ops wpa_driver_wext_ops = {
	.name = "wext",
	.desc = "Linux wireless extensions (generic)",
	.get_bssid = wpa_driver_wext_get_bssid,
	.get_ssid = wpa_driver_wext_get_ssid,
	.set_wpa = wpa_driver_wext_set_wpa,
	.set_key = wpa_driver_wext_set_key,
	.set_countermeasures = wpa_driver_wext_set_countermeasures,
	.set_drop_unencrypted = wpa_driver_wext_set_drop_unencrypted,
	.scan = wpa_driver_wext_scan,
	.get_scan_results = wpa_driver_wext_get_scan_results,
	.deauthenticate = wpa_driver_wext_deauthenticate,
	.disassociate = wpa_driver_wext_disassociate,
	.associate = wpa_driver_wext_associate,
	.set_auth_alg = wpa_driver_wext_set_auth_alg,
	.init = wpa_driver_wext_init,
	.deinit = wpa_driver_wext_deinit,
	.add_pmkid = wpa_driver_wext_add_pmkid,
	.remove_pmkid = wpa_driver_wext_remove_pmkid,
	.flush_pmkid = wpa_driver_wext_flush_pmkid,
	.get_capa = wpa_driver_wext_get_capa,
};
