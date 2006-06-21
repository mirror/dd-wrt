/*
 * hostapd / IEEE 802.11 Management: Beacon and Probe Request/Response
 * Copyright (c) 2002-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#ifndef CONFIG_NATIVE_WINDOWS

#include "hostapd.h"
#include "ieee802_11.h"
#include "wpa.h"
#include "hostap_common.h"
#include "beacon.h"
#include "driver.h"
#include "sta_info.h"


static u8 * hostapd_eid_ds_params(struct hostapd_data *hapd, u8 *eid)
{
	*eid++ = WLAN_EID_DS_PARAMS;
	*eid++ = 1;
	*eid++ = hapd->iconf->channel;
	return eid;
}


static u8 * hostapd_eid_wpa(struct hostapd_data *hapd, u8 *eid, size_t len,
			    struct sta_info *sta)
{
	const u8 *ie;
	size_t ielen;

	ie = wpa_auth_get_wpa_ie(hapd->wpa_auth, &ielen);
	if (ie == NULL || ielen > len)
		return eid;

	memcpy(eid, ie, ielen);
	return eid + ielen;
}


void handle_probe_req(struct hostapd_data *hapd, struct ieee80211_mgmt *mgmt,
		      size_t len)
{
	struct ieee80211_mgmt *resp;
	struct ieee802_11_elems elems;
	char *ssid;
	u8 *pos, *epos;
	size_t ssid_len;
	struct sta_info *sta = NULL;

	if (!hapd->iconf->send_probe_response)
		return;

	if (ieee802_11_parse_elems(hapd, mgmt->u.probe_req.variable,
				   len - (IEEE80211_HDRLEN +
					  sizeof(mgmt->u.probe_req)), &elems,
				   0)
	    == ParseFailed) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "Could not parse ProbeReq from " MACSTR "\n",
			      MAC2STR(mgmt->sa));
		return;
	}

	ssid = NULL;
	ssid_len = 0;

	if ((!elems.ssid || !elems.supp_rates)) {
		printf("STA " MACSTR " sent probe request without SSID or "
		       "supported rates element\n", MAC2STR(mgmt->sa));
		return;
	}

	if (hapd->conf->ignore_broadcast_ssid && elems.ssid_len == 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS,
			      "Probe Request from " MACSTR " for broadcast "
			      "SSID ignored\n", MAC2STR(mgmt->sa));
		return;
	}

	sta = ap_get_sta(hapd, mgmt->sa);

	if (elems.ssid_len == 0 ||
	    (elems.ssid_len == hapd->conf->ssid.ssid_len &&
	     memcmp(elems.ssid, hapd->conf->ssid.ssid, elems.ssid_len) == 0)) {
		ssid = hapd->conf->ssid.ssid;
		ssid_len = hapd->conf->ssid.ssid_len;
		if (sta)
			sta->ssid_probe = &hapd->conf->ssid;
	}

	if (!ssid) {
		if (HOSTAPD_DEBUG_COND(HOSTAPD_DEBUG_MSGDUMPS)) {
			printf("Probe Request from " MACSTR " for foreign "
			       "SSID '", MAC2STR(mgmt->sa));
			ieee802_11_print_ssid(elems.ssid, elems.ssid_len);
			printf("'\n");
		}
		return;
	}

	/* TODO: verify that supp_rates contains at least one matching rate
	 * with AP configuration */
#define MAX_PROBERESP_LEN 512
	resp = wpa_zalloc(MAX_PROBERESP_LEN);
	if (resp == NULL)
		return;
	epos = ((u8 *) resp) + MAX_PROBERESP_LEN;

	resp->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_PROBE_RESP);
	memcpy(resp->da, mgmt->sa, ETH_ALEN);
	memcpy(resp->sa, hapd->own_addr, ETH_ALEN);

	memcpy(resp->bssid, hapd->own_addr, ETH_ALEN);
	resp->u.probe_resp.beacon_int =
		host_to_le16(hapd->iconf->beacon_int);

	/* hardware or low-level driver will setup seq_ctrl and timestamp */
	resp->u.probe_resp.capab_info =
		host_to_le16(hostapd_own_capab_info(hapd, sta, 1));

	pos = resp->u.probe_resp.variable;
	*pos++ = WLAN_EID_SSID;
	*pos++ = ssid_len;
	memcpy(pos, ssid, ssid_len);
	pos += ssid_len;

	/* Supported rates */
	pos = hostapd_eid_supp_rates(hapd, pos);

	/* DS Params */
	pos = hostapd_eid_ds_params(hapd, pos);

#if 0 /* TODO */
	/* ERP Information element */
	pos = hostapd_eid_erp_info(hapd, pos);
#endif /* TODO */

	/* Extended supported rates */
	pos = hostapd_eid_ext_supp_rates(hapd, pos);

	pos = hostapd_eid_wpa(hapd, pos, epos - pos, sta);

	if (hostapd_send_mgmt_frame(hapd, resp, pos - (u8 *) resp, 0) < 0)
		perror("handle_probe_req: send");

	free(resp);

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "STA " MACSTR
		      " sent probe request for %s SSID\n",
		      MAC2STR(mgmt->sa), elems.ssid_len == 0 ? "broadcast" :
		      "our");
}


void ieee802_11_set_beacon(struct hostapd_data *hapd)
{
	struct ieee80211_mgmt *head;
	u8 *pos, *tail, *tailpos;
	u16 capab_info;
	size_t head_len, tail_len;

#define BEACON_HEAD_BUF_SIZE 256
#define BEACON_TAIL_BUF_SIZE 256
	head = wpa_zalloc(BEACON_HEAD_BUF_SIZE);
	tailpos = tail = malloc(BEACON_TAIL_BUF_SIZE);
	if (head == NULL || tail == NULL) {
		printf("Failed to set beacon data\n");
		free(head);
		free(tail);
		return;
	}

	head->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_BEACON);
	head->duration = host_to_le16(0);
	memset(head->da, 0xff, ETH_ALEN);

	memcpy(head->sa, hapd->own_addr, ETH_ALEN);
	memcpy(head->bssid, hapd->own_addr, ETH_ALEN);
	head->u.beacon.beacon_int =
		host_to_le16(hapd->iconf->beacon_int);

	/* hardware or low-level driver will setup seq_ctrl and timestamp */
	capab_info = hostapd_own_capab_info(hapd, NULL, 0);
	head->u.beacon.capab_info = host_to_le16(capab_info);
	pos = &head->u.beacon.variable[0];

	/* SSID */
	*pos++ = WLAN_EID_SSID;
	if (hapd->conf->ignore_broadcast_ssid == 2) {
		/* clear the data, but keep the correct length of the SSID */
		*pos++ = hapd->conf->ssid.ssid_len;
		memset(pos, 0, hapd->conf->ssid.ssid_len);
		pos += hapd->conf->ssid.ssid_len;
	} else if (hapd->conf->ignore_broadcast_ssid) {
		*pos++ = 0; /* empty SSID */
	} else {
		*pos++ = hapd->conf->ssid.ssid_len;
		memcpy(pos, hapd->conf->ssid.ssid, hapd->conf->ssid.ssid_len);
		pos += hapd->conf->ssid.ssid_len;
	}

	/* Supported rates */
	pos = hostapd_eid_supp_rates(hapd, pos);

	/* DS Params */
	pos = hostapd_eid_ds_params(hapd, pos);

	head_len = pos - (u8 *) head;

#if 0 /* TODO */
	/* ERP Information element */
	tailpos = hostapd_eid_erp_info(hapd, tailpos);
#endif /* TODO */

	/* Extended supported rates */
	tailpos = hostapd_eid_ext_supp_rates(hapd, tailpos);

	tailpos = hostapd_eid_wpa(hapd, tailpos, tail + BEACON_TAIL_BUF_SIZE -
				  tailpos, NULL);

	tail_len = tailpos > tail ? tailpos - tail : 0;

	if (hostapd_set_beacon(hapd->conf->iface, hapd, (u8 *) head, head_len,
			       tail, tail_len))
		printf("Failed to set beacon head/tail\n");

	free(tail);
	free(head);
}


void ieee802_11_set_beacons(struct hostapd_iface *iface)
{
	int i;
	for (i = 0; i < iface->num_bss; i++)
		ieee802_11_set_beacon(iface->bss[i]);
}

#endif /* CONFIG_NATIVE_WINDOWS */
