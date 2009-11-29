/*
 * hostapd / Driver interaction with MADWIFI 802.11 driver
 * Copyright (c) 2004, Sam Leffler <sam@errno.com>
 * Copyright (c) 2004, Video54 Technologies
 * Copyright (c) 2005-2007, Jouni Malinen <j@w1.fi>
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
#include <net/if.h>
#include <sys/ioctl.h>

#include "wireless_copy.h"

#include "hostapd.h"
#include "driver.h"
#include "ieee802_1x.h"
#include "eloop.h"
#include "priv_netlink.h"
#include "sta_info.h"
#include "l2_packet/l2_packet.h"

#include "wpa.h"
#include "radius/radius.h"
#include "ieee802_11.h"
#include "accounting.h"
#include "common.h"

struct wext_driver_data {
	struct hostapd_data *hapd;		/* back pointer */

	char	iface[IFNAMSIZ + 1];
	int     ifindex;
	struct l2_packet_data *sock_xmit;	/* raw packet xmit socket */
	struct l2_packet_data *sock_recv;	/* raw packet recv socket */
	int	ioctl_sock;			/* socket for ioctl() use */
	int	wext_sock;			/* socket for wireless events */
	int	we_version;
	u8	acct_mac[ETH_ALEN];
	struct hostap_sta_driver_data acct_data;
};


static int
wext_set_auth_generic(struct wext_driver_data *drv,
                      int idx, u32 value)
{
    struct iwreq iwr;
    int ret = 0;

    memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
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


static const char *
ether_sprintf(const u8 *addr)
{
	static char buf[sizeof(MACSTR)];

	if (addr != NULL)
		snprintf(buf, sizeof(buf), MACSTR, MAC2STR(addr));
	else
		snprintf(buf, sizeof(buf), MACSTR, 0,0,0,0,0,0);
	return buf;
}

/*
 * Configure WPA parameters.
 */
static int
wext_configure_wpa(struct wext_driver_data *drv)
{
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	int v;

	switch (conf->wpa_group) {
	case WPA_CIPHER_CCMP:
		v = IW_AUTH_CIPHER_CCMP;
		break;
	case WPA_CIPHER_TKIP:
		v = IW_AUTH_CIPHER_TKIP;
		break;
	case WPA_CIPHER_WEP104:
		v = IW_AUTH_CIPHER_WEP104;
		break;
	case WPA_CIPHER_WEP40:
		v = IW_AUTH_CIPHER_WEP40;
		break;
	case WPA_CIPHER_NONE:
		v = IW_AUTH_CIPHER_NONE;
		break;
	default:
		wpa_printf(MSG_ERROR, "Unknown group key cipher %u\n",
			conf->wpa_group);
		return -1;
	}
	wpa_printf(MSG_DEBUG, "%s: group key cipher=%d\n", __func__, v);
	if (wext_set_auth_generic(drv, IW_AUTH_CIPHER_GROUP, v)) {
		printf("Unable to set group key cipher to %u\n", v);
		return -1;
	}

	v = 0;
	if (conf->wpa_pairwise & WPA_CIPHER_CCMP)
		v |= IW_AUTH_CIPHER_CCMP;
	if (conf->wpa_pairwise & WPA_CIPHER_TKIP)
		v |= IW_AUTH_CIPHER_TKIP;
	if (conf->wpa_pairwise & WPA_CIPHER_NONE)
		v |= IW_AUTH_CIPHER_NONE;
	wpa_printf(MSG_DEBUG, "%s: pairwise key ciphers=0x%x\n", __func__, v);
	if (wext_set_auth_generic(drv, IW_AUTH_CIPHER_PAIRWISE, v)) {
		printf("Unable to set pairwise key ciphers to 0x%x\n", v);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: key management algorithms=0x%x\n",
			__func__, conf->wpa_key_mgmt);
	if (wext_set_auth_generic(drv, IW_AUTH_KEY_MGMT, conf->wpa_key_mgmt)) {
		printf("Unable to set key management algorithms to 0x%x\n",
			conf->wpa_key_mgmt);
		return -1;
	}

#ifdef	SOME_TIME_LATER
	v = 0;
	if (conf->rsn_preauth)
		v |= BIT(0);
	wpa_printf(MSG_DEBUG, "%s: rsn capabilities=0x%x\n", __func__, conf->rsn_preauth);
	if (set80211param(drv, IEEE80211_PARAM_RSNCAPS, v)) {
		printf("Unable to set RSN capabilities to 0x%x\n", v);
		return -1;
	}
#endif	//	SOME_TIME_LATER
	return 0;
}


static int
wext_set_iface_flags(void *priv, int dev_up)
{
	struct wext_driver_data *drv = priv;
	struct ifreq ifr;

	wpa_printf(MSG_DEBUG, "%s: dev_up=%d", __func__, dev_up);

	if (drv->ioctl_sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, drv->iface, IFNAMSIZ);

	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}

#ifdef CONFIG_UPDOWN_CONTROL
	if (dev_up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	if (ioctl(drv->ioctl_sock, SIOCSIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCSIFFLAGS]");
		return -1;
	}
#endif

#ifdef CONFIG_HOSTAPD_SETMTU
	if (dev_up) {
		memset(&ifr, 0, sizeof(ifr));
		os_strlcpy(ifr.ifr_name, drv->iface, IFNAMSIZ);
		ifr.ifr_mtu = HOSTAPD_MTU;
		if (ioctl(drv->ioctl_sock, SIOCSIFMTU, &ifr) != 0) {
			perror("ioctl[SIOCSIFMTU]");
			printf("Setting MTU failed - trying to survive with "
			       "current value\n");
		}
	}
#endif

	return 0;
}

static int
wext_set_ieee8021x(const char *ifname, void *priv, int enabled)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	int res = 0;

	wpa_printf(MSG_DEBUG, "%s: enabled=%d\n", __func__, enabled);

	if (!enabled) {
		/* XXX restore state */
		return wext_set_auth_generic(drv, IW_AUTH_80211_AUTH_ALG,
				(IW_AUTH_ALG_SHARED_KEY | IW_AUTH_ALG_OPEN_SYSTEM) );
	}
	if (!conf->wpa && !conf->ieee802_1x) {
		hostapd_logger(hapd, NULL, HOSTAPD_MODULE_DRIVER,
			HOSTAPD_LEVEL_WARNING, "No 802.1X or WPA enabled!");
		return -1;
	}
	if (conf->wpa && wext_configure_wpa(drv) != 0) {
		hostapd_logger(hapd, NULL, HOSTAPD_MODULE_DRIVER,
			HOSTAPD_LEVEL_WARNING, "Error configuring WPA state!");
		return -1;
	}
	if (conf->wpa) {
		int v = 0;
		res = wext_set_auth_generic(drv, IW_AUTH_80211_AUTH_ALG,
				(IW_AUTH_ALG_SHARED_KEY | IW_AUTH_ALG_OPEN_SYSTEM));
//		if (res == 0)
//			res = wext_set_auth_generic(drv, IW_AUTH_WPA_ENABLED, 1);

		/* have to set WPA version after wpa enabling */
		wpa_printf(MSG_DEBUG, "%s: enable WPA=0x%x\n", __func__, conf->wpa);
		switch (conf->wpa) {
		case 0:
			v = IW_AUTH_WPA_VERSION_DISABLED;
			break;
		case 1:
			v = IW_AUTH_WPA_VERSION_WPA;
			break;
		case 2:
			v = IW_AUTH_WPA_VERSION_WPA2;
			break;
		case 3:
			v = IW_AUTH_WPA_VERSION_WPA | IW_AUTH_WPA_VERSION_WPA2;
			break;
		}
		res = wext_set_auth_generic(drv, IW_AUTH_WPA_VERSION, v);
	} else
		res = wext_set_auth_generic(drv, IW_AUTH_80211_AUTH_ALG, IW_AUTH_ALG_LEAP);
	if (res)
		hostapd_logger(hapd, NULL, HOSTAPD_MODULE_DRIVER,
			HOSTAPD_LEVEL_WARNING, "Error enabling WPA/802.1X!");
	return res;
}

static int
wext_set_privacy(const char *ifname, void *priv, int enabled)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG, "%s: enabled=%d\n", __func__, enabled);

	return wext_set_auth_generic(drv, IW_AUTH_PRIVACY_INVOKED, enabled);
}

static int
wext_set_key_ext(const char *ifname, void *priv, const char *alg,
		const u8 *addr, int key_idx,
		const u8 *key, size_t key_len, int txkey)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct iwreq iwr;
	struct iw_encode_ext *ext;

	wpa_printf(MSG_DEBUG, "%s: alg=%s addr=%s key_idx=%d\n",
		__func__, alg, ether_sprintf(addr), key_idx);

	ext = malloc(sizeof(*ext) + key_len);
    if (ext == NULL)
        return -1;
    memset(&iwr, 0, sizeof(iwr));
    memset(ext,  0, sizeof(*ext));
    strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
    iwr.u.encoding.flags = key_idx + 1;
    iwr.u.encoding.pointer = (caddr_t) ext;
    iwr.u.encoding.length = sizeof(*ext) + key_len;

    if (!addr || !memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) )
        ext->ext_flags |= IW_ENCODE_EXT_GROUP_KEY;
    if (txkey)
        ext->ext_flags |= IW_ENCODE_EXT_SET_TX_KEY;
//    ext->addr.sa_family = ARPHRD_ETHER;

	if (addr)
        memcpy(ext->addr.sa_data, addr, ETH_ALEN);
    else
		memset(ext->addr.sa_data, 0xff, ETH_ALEN);
    if (key && key_len) {
        memcpy(ext + 1, key, key_len);
        ext->key_len = key_len;
    }

	if (strcmp(alg, "none") == 0) {
		ext->alg = IW_ENCODE_ALG_NONE;
        iwr.u.encoding.flags |= IW_ENCODE_DISABLED;
	} else if (strcmp(alg, "WEP") == 0)
		ext->alg = IW_ENCODE_ALG_WEP;
	else if (strcmp(alg, "TKIP") == 0)
		ext->alg = IW_ENCODE_ALG_TKIP;
	else if (strcmp(alg, "CCMP") == 0)
		ext->alg = IW_ENCODE_ALG_CCMP;
	else {
		printf("%s: unknown/unsupported algorithm %s\n",
			__func__, alg);
		free(ext);
		return -1;
	}

	if (ioctl(drv->ioctl_sock, SIOCSIWENCODEEXT, &iwr) < 0) {
		free(ext);
        perror("ioctl[SIOCSIWENCODEEXT]");
		return -1;
    }

	free(ext);
    return 0;
}



static int
wext_get_seqnum(const char *ifname, void *priv, const u8 *addr, int idx,
		   u8 *seq)
{
#define	WEXT_KEY_LEN	32
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct iwreq iwr;
	struct iw_encode_ext *ext;

	wpa_printf(MSG_DEBUG, "%s: addr=%s key_idx=%d\n", __func__, ether_sprintf(addr), idx);

	ext = malloc(sizeof(*ext) + WEXT_KEY_LEN);
    if (ext == NULL)
        return -1;
    memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
    iwr.u.encoding.flags = idx + 1;
    iwr.u.encoding.pointer = (caddr_t) ext;
    iwr.u.encoding.length = sizeof(*ext) + WEXT_KEY_LEN;

	if (addr)
        memcpy(ext->addr.sa_data, addr, ETH_ALEN);
    else
		memset(ext->addr.sa_data, 0xff, ETH_ALEN);
    if (!addr || !memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) )
        ext->ext_flags |= IW_ENCODE_EXT_GROUP_KEY;

	if (ioctl(drv->ioctl_sock, SIOCGIWENCODEEXT, &iwr) < 0) {
        perror("ioctl[SIOCGIWENCODEEXT]");
		free(ext);
		return -1;
    }

	memcpy(seq, ext->tx_seq, WPA_KEY_RSC_LEN);
	free(ext);
#undef	WEX_KEY_LEN
	return 0;
}


static int
wext_read_sta_driver_data(void *priv, struct hostap_sta_driver_data *data,
			     const u8 *addr)
{
	return 0;
}


static int
wext_sta_clear_stats(void *priv, const u8 *addr)
{
	return 0;
}


static int
wext_set_opt_ie(const char *ifname, void *priv, const u8 *ie, size_t ie_len)
{
	/*
	 * Do nothing; we setup parameters at startup that define the
	 * contents of the beacon information element.
	 */
	return 0;
}

static int wext_mlme_generic(struct wext_driver_data *drv,
                const u8 *addr, int cmd, int reason_code)
{
    struct iwreq iwr;
    struct iw_mlme mlme;
    int ret = 0;

    os_memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
    memset(&mlme, 0, sizeof(mlme));
    mlme.cmd = cmd;
    mlme.reason_code = reason_code;
//    mlme.addr.sa_family = ARPHRD_ETHER;
    memcpy(mlme.addr.sa_data, addr, ETH_ALEN);
    iwr.u.data.pointer = (caddr_t) &mlme;
    iwr.u.data.length = sizeof(mlme);

    if (ioctl(drv->ioctl_sock, SIOCSIWMLME, &iwr) < 0) {
        perror("ioctl[SIOCSIWMLME]");
        ret = -1;
    }
    return ret;
}


static int
wext_sta_deauth(void *priv, const u8 *addr, int reason_code)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG, "%s: addr=%s reason_code=%d\n",
		__func__, ether_sprintf(addr), reason_code);


	return wext_mlme_generic(drv, addr, IW_MLME_DEAUTH, reason_code);
}

static int
wext_sta_disassoc(void *priv, const u8 *addr, int reason_code)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG,
		"%s: addr=%s reason_code=%d\n",
		__func__, ether_sprintf(addr), reason_code);

	return wext_mlme_generic(drv, addr, IW_MLME_DISASSOC, reason_code);
}

static int
wext_flush(void *priv)
{
	u8 allsta[ETH_ALEN];
	memset(allsta, 0xff, ETH_ALEN);
	return wext_sta_deauth(priv, allsta, 1);
}


/**
 * Some kind of UBNT hack to support (un)autorize station
 * via standart siwmlme ioctl
 **/
#define	IW_MLME_AUTHORIZE	63
#define	IW_MLME_UNAUTHORIZE	64
static int
wext_sta_set_flags(void *priv, const u8 *addr, int total_flags,
		      int flags_or, int flags_and)
{
	struct wext_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;


	/* For now, only support setting Authorized flag */
	if (flags_or & WLAN_STA_AUTHORIZED) {
		wpa_printf(MSG_DEBUG,
				"%s: addr=%s autorized\n",	__func__, ether_sprintf(addr));
		return wext_mlme_generic(drv, addr,	IW_MLME_AUTHORIZE, 0);
	}
	if (!(flags_and & WLAN_STA_AUTHORIZED)) {
		wpa_printf(MSG_DEBUG,
				"%s: addr=%s unautorized\n",	__func__, ether_sprintf(addr));
		return wext_mlme_generic(drv, addr,	IW_MLME_UNAUTHORIZE, 0);
	}
	return 0;
}
#undef	IW_MLME_AUTHORIZE
#undef	IW_MLME_UNAUTHORIZE



static int
wext_del_sta(struct wext_driver_data *drv, u8* addr)
{
	struct hostapd_data *hapd = drv->hapd;
	struct sta_info *sta;

	hostapd_logger(hapd, addr, HOSTAPD_MODULE_IEEE80211,
		HOSTAPD_LEVEL_INFO, "disassociated");

	sta = ap_get_sta(hapd, addr);
	if (sta != NULL) {
		sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
		wpa_auth_sm_event(sta->wpa_sm, WPA_DISASSOC);
		sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		ap_free_sta(hapd, sta);
	}
	return 0;
}

static int
wext_process_wpa_ie(struct wext_driver_data *drv, struct sta_info *sta)
{
#define	WEXT_WPA_IE_LEN	6 + 256
	struct hostapd_data *hapd = drv->hapd;
	struct iwreq iwr;
	int ielen, res;
	u8 iebuf[WEXT_WPA_IE_LEN];

	wpa_printf(MSG_DEBUG,
		"%s: addr=%s\n", __func__, ether_sprintf(sta->addr));

    memset(&iwr, 0, sizeof(iwr));
    memset(&iebuf, 0, sizeof(iebuf));
    strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
    iwr.u.encoding.pointer = (caddr_t)iebuf;
    iwr.u.encoding.length = WEXT_WPA_IE_LEN;

	/**
	 * Some king of hack here:
	 * I set WPA flag, so driver will check iebuf for sta mac address
	 * and will return this station WPA IE
	 * Will work with UBNT atheros driver
	 **/
    iwr.u.encoding.flags = IW_AUTH_WPA_ENABLED;
	memcpy(iebuf, sta->addr, ETH_ALEN);

	if (ioctl(drv->ioctl_sock, SIOCGIWGENIE, &iwr) < 0) {
        perror("ioctl[SIOCGIWGENIE]");
		return -1;
    }

	ielen = iebuf[1];
	if (ielen == 0) {
		printf("No WPA/RSN information element for station!?\n");
		return -1;		/* XXX not right */
	}
	ielen += 2;
	if (sta->wpa_sm == NULL)
		sta->wpa_sm = wpa_auth_sta_init(hapd->wpa_auth, sta->addr);
	if (sta->wpa_sm == NULL) {
		printf("Failed to initialize WPA state machine\n");
		return -1;
	}
	res = wpa_validate_wpa_ie(hapd->wpa_auth, sta->wpa_sm,
				  iebuf, ielen, NULL, 0);
	if (res != WPA_IE_OK) {
		printf("WPA/RSN information element rejected? (res %u)\n", res);
		return -1;
	}
#undef	WEXT_WPA_IE_LEN
	return 0;
}

static int
wext_new_sta(struct wext_driver_data *drv, u8* addr)
{
	struct hostapd_data *hapd = drv->hapd;
	struct sta_info *sta;
	int new_assoc;

	hostapd_logger(hapd, addr, HOSTAPD_MODULE_IEEE80211,
		HOSTAPD_LEVEL_INFO, "associated");

	sta = ap_get_sta(hapd, addr);
	if (sta) {
		accounting_sta_stop(hapd, sta);
	} else {
		sta = ap_sta_add(hapd, addr);
		if (sta == NULL)
			return -1;
	}

	if (memcmp(addr, drv->acct_mac, ETH_ALEN) == 0) {
		/* Cached accounting data is not valid anymore. */
		memset(drv->acct_mac, 0, ETH_ALEN);
		memset(&drv->acct_data, 0, sizeof(drv->acct_data));
	}

	if (hapd->conf->wpa) {
		if (wext_process_wpa_ie(drv, sta))
			return -1;
	}

	/*
	 * Now that the internal station state is setup
	 * kick the authenticator into action.
	 */
	new_assoc = (sta->flags & WLAN_STA_ASSOC) == 0;
	sta->flags |= WLAN_STA_AUTH | WLAN_STA_ASSOC;
	wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);
	hostapd_new_assoc_sta(hapd, sta, !new_assoc);
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);
	return 0;
}

static void
wext_wireless_event_wireless_custom(struct wext_driver_data *drv,
				       char *custom)
{
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG, "Custom wireless event: '%s'\n",
		      custom);

	if (strncmp(custom, "MLME-MICHAELMICFAILURE.indication", 33) == 0) {
		char *pos;
		u8 addr[ETH_ALEN];
		pos = strstr(custom, "addr=");
		if (pos == NULL) {
			wpa_printf(MSG_DEBUG,
				      "MLME-MICHAELMICFAILURE.indication "
				      "without sender address ignored\n");
			return;
		}
		pos += 5;
		if (hwaddr_aton(pos, addr) == 0) {
			ieee80211_michael_mic_failure(drv->hapd, addr, 1);
		} else {
			wpa_printf(MSG_DEBUG,
				      "MLME-MICHAELMICFAILURE.indication "
				      "with invalid MAC address");
		}
	} else if (strncmp(custom, "STA-TRAFFIC-STAT", 16) == 0) {
		char *key, *value;
		u32 val;
		key = custom;
		while ((key = strchr(key, '\n')) != NULL) {
			key++;
			value = strchr(key, '=');
			if (value == NULL)
				continue;
			*value++ = '\0';
			val = strtoul(value, NULL, 10);
			if (strcmp(key, "mac") == 0)
				hwaddr_aton(value, drv->acct_mac);
			else if (strcmp(key, "rx_packets") == 0)
				drv->acct_data.rx_packets = val;
			else if (strcmp(key, "tx_packets") == 0)
				drv->acct_data.tx_packets = val;
			else if (strcmp(key, "rx_bytes") == 0)
				drv->acct_data.rx_bytes = val;
			else if (strcmp(key, "tx_bytes") == 0)
				drv->acct_data.tx_bytes = val;
			key = value;
		}
	}
}

static void
wext_wireless_event_wireless(struct wext_driver_data *drv,
					    char *data, int len)
{
	struct hostapd_data *hapd = drv->hapd;
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom, *buf;

	pos = data;
	end = data + len;

	while (pos + IW_EV_LCP_LEN <= end) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		wpa_printf(MSG_DEBUG, "Wireless event: "
			      "cmd=0x%x len=%d\n", iwe->cmd, iwe->len);
		if (iwe->len <= IW_EV_LCP_LEN)
			return;

		custom = pos + IW_EV_POINT_LEN;
		if (drv->we_version > 18 &&
		    (iwe->cmd == IWEVMICHAELMICFAILURE ||
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
		case IWEVEXPIRED:
			wext_del_sta(drv, (u8 *) iwe->u.addr.sa_data);
			break;
		case IWEVREGISTERED:
			wext_new_sta(drv, (u8 *) iwe->u.addr.sa_data);
			break;
		case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
				return;
			buf = malloc(iwe->u.data.length + 1);
			if (buf == NULL)
				return;		/* XXX */
			memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';
			wext_wireless_event_wireless_custom(drv, buf);
			free(buf);
			break;
		}

		pos += iwe->len;
	}
}


static void
wext_wireless_event_rtm_newlink(struct wext_driver_data *drv,
					       struct nlmsghdr *h, int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;

	if (len < (int) sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);

	if (ifi->ifi_index != drv->ifindex)
		return;

	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			wext_wireless_event_wireless(
				drv, ((char *) attr) + rta_len,
				attr->rta_len - rta_len);
		}
		attr = RTA_NEXT(attr, attrlen);
	}
}


static void
wext_wireless_event_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	char buf[256];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	struct wext_driver_data *drv = eloop_ctx;

	fromlen = sizeof(from);
	left = recvfrom(sock, buf, sizeof(buf), MSG_DONTWAIT,
			(struct sockaddr *) &from, &fromlen);
	if (left < 0) {
		if (errno != EINTR && errno != EAGAIN)
			perror("recvfrom(netlink)");
		return;
	}

	h = (struct nlmsghdr *) buf;
	while (left >= (int) sizeof(*h)) {
		int len, plen;

		len = h->nlmsg_len;
		plen = len - sizeof(*h);
		if (len > left || plen < 0) {
			printf("Malformed netlink message: "
			       "len=%d left=%d plen=%d\n",
			       len, left, plen);
			break;
		}

		switch (h->nlmsg_type) {
		case RTM_NEWLINK:
			wext_wireless_event_rtm_newlink(drv, h, plen);
			break;
		}

		len = NLMSG_ALIGN(len);
		left -= len;
		h = (struct nlmsghdr *) ((char *) h + len);
	}

	if (left > 0) {
		printf("%d extra bytes in the end of netlink message\n", left);
	}
}


static int
wext_get_we_version(struct wext_driver_data *drv)
{
	struct iw_range *range;
	struct iwreq iwr;
	int minlen;
	size_t buflen;

	drv->we_version = 0;

	/*
	 * Use larger buffer than struct iw_range in order to allow the
	 * structure to grow in the future.
	 */
	buflen = sizeof(struct iw_range) + 500;
	range = os_zalloc(buflen);
	if (range == NULL)
		return -1;

	memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
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
		drv->we_version = range->we_version_compiled;
	}

	free(range);
	return 0;
}


static int
wext_wireless_event_init(void *priv)
{
	struct wext_driver_data *drv = priv;
	int s;
	struct sockaddr_nl local;

	wext_get_we_version(drv);

	drv->wext_sock = -1;

	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s < 0) {
		perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind(netlink)");
		close(s);
		return -1;
	}

	eloop_register_read_sock(s, wext_wireless_event_receive, drv, NULL);
	drv->wext_sock = s;

	return 0;
}


static void
wext_wireless_event_deinit(void *priv)
{
	struct wext_driver_data *drv = priv;

	if (drv != NULL) {
		if (drv->wext_sock < 0)
			return;
		eloop_unregister_read_sock(drv->wext_sock);
		close(drv->wext_sock);
	}
}


static int
wext_send_eapol(void *priv, const u8 *addr, const u8 *data, size_t data_len,
		   int encrypt, const u8 *own_addr)
{
	struct wext_driver_data *drv = priv;
	unsigned char buf[3000];
	unsigned char *bp = buf;
	struct l2_ethhdr *eth;
	size_t len;
	int status;

	/*
	 * Prepend the Ethernet header.  If the caller left us
	 * space at the front we could just insert it but since
	 * we don't know we copy to a local buffer.  Given the frequency
	 * and size of frames this probably doesn't matter.
	 */
	len = data_len + sizeof(struct l2_ethhdr);
	if (len > sizeof(buf)) {
		bp = malloc(len);
		if (bp == NULL) {
			printf("EAPOL frame discarded, cannot malloc temp "
			       "buffer of size %lu!\n", (unsigned long) len);
			return -1;
		}
	}
	eth = (struct l2_ethhdr *) bp;
	memcpy(eth->h_dest, addr, ETH_ALEN);
	memcpy(eth->h_source, own_addr, ETH_ALEN);
	eth->h_proto = htons(ETH_P_EAPOL);
	memcpy(eth+1, data, data_len);

	wpa_hexdump(MSG_MSGDUMP, "TX EAPOL", bp, len);

	status = l2_packet_send(drv->sock_xmit, addr, ETH_P_EAPOL, bp, len);

	if (bp != buf)
		free(bp);
	return status;
}

static void
handle_read(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
	struct wext_driver_data *drv = ctx;
	struct hostapd_data *hapd = drv->hapd;
	struct sta_info *sta;

	sta = ap_get_sta(hapd, src_addr);
	if (!sta || !(sta->flags & WLAN_STA_ASSOC)) {
		printf("Data frame from not associated STA %s\n",
		       ether_sprintf(src_addr));
		/* XXX cannot happen */
		return;
	}
	ieee802_1x_receive(hapd, src_addr, buf + sizeof(struct l2_ethhdr),
			   len - sizeof(struct l2_ethhdr));
}

static void *
wext_init(struct hostapd_data *hapd)
{
	struct wext_driver_data *drv;
	struct ifreq ifr;
	struct iwreq iwr;

	drv = os_zalloc(sizeof(struct wext_driver_data));
	if (drv == NULL) {
		printf("Could not allocate memory for wext driver data\n");
		goto bad;
	}

	drv->hapd = hapd;
	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		goto bad;
	}
	memcpy(drv->iface, hapd->conf->iface, sizeof(drv->iface));

	memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, drv->iface, sizeof(ifr.ifr_name));
	if (ioctl(drv->ioctl_sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		goto bad;
	}
	drv->ifindex = ifr.ifr_ifindex;

	drv->sock_xmit = l2_packet_init(drv->iface, NULL, ETH_P_EAPOL,
					handle_read, drv, 1);
	if (drv->sock_xmit == NULL)
		goto bad;
	if (l2_packet_get_own_addr(drv->sock_xmit, hapd->own_addr))
		goto bad;
	if (hapd->conf->bridge[0] != '\0') {
		wpa_printf(MSG_DEBUG,
			"Configure bridge %s for EAPOL traffic.\n",
			hapd->conf->bridge);
		drv->sock_recv = l2_packet_init(hapd->conf->bridge, NULL,
						ETH_P_EAPOL, handle_read, drv,
						1);
		if (drv->sock_recv == NULL)
			goto bad;
	} else
		drv->sock_recv = drv->sock_xmit;

	memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->iface, IFNAMSIZ);

	iwr.u.mode = IW_MODE_MASTER;

	if (ioctl(drv->ioctl_sock, SIOCSIWMODE, &iwr) < 0) {
		perror("ioctl[SIOCSIWMODE]");
		printf("Could not set interface to master mode!\n");
		goto bad;
	}

	wext_set_iface_flags(drv, 0);	/* mark down during setup */
	wext_set_privacy(drv->iface, drv, 0); /* default to no privacy */

	return drv;
bad:
	if (drv->sock_xmit != NULL)
		l2_packet_deinit(drv->sock_xmit);
	if (drv->ioctl_sock >= 0)
		close(drv->ioctl_sock);
	if (drv != NULL)
		free(drv);
	return NULL;
}


static void
wext_deinit(void *priv)
{
	struct wext_driver_data *drv = priv;

	(void) wext_set_iface_flags(drv, 0);
	if (drv->ioctl_sock >= 0)
		close(drv->ioctl_sock);
	if (drv->sock_recv != NULL && drv->sock_recv != drv->sock_xmit)
		l2_packet_deinit(drv->sock_recv);
	if (drv->sock_xmit != NULL)
		l2_packet_deinit(drv->sock_xmit);
	free(drv);
}

static int
wext_set_ssid(const char *ifname, void *priv, const u8 *buf, int len)
{
	struct wext_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
	iwr.u.essid.flags = 1; /* SSID active */
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len + 1;

	if (ioctl(drv->ioctl_sock, SIOCSIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCSIWESSID]");
		printf("len=%d\n", len);
		return -1;
	}
	return 0;
}

static int
wext_get_ssid(const char *ifname, void *priv, u8 *buf, int len)
{
	struct wext_driver_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len;

	if (ioctl(drv->ioctl_sock, SIOCGIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCGIWESSID]");
		ret = -1;
	} else {
		ret = iwr.u.essid.length;
		if (ret > 32)
			ret = 32;
		if (ret > 0 && buf[ret - 1] == '\0')
			ret--;
	}

	return ret;
}


static int
wext_set_countermeasures(void *priv, int enabled)
{
	struct wext_driver_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return wext_set_auth_generic(drv, IW_AUTH_TKIP_COUNTERMEASURES, enabled);
}

static int
wext_commit(void *priv)
{
	return wext_set_iface_flags(priv, 1);
}

const struct wpa_driver_ops wpa_driver_wext_ops = {
	.name					= "wext",
	.init					= wext_init,
	.deinit					= wext_deinit,
	.set_ieee8021x			= wext_set_ieee8021x,
	.set_privacy			= wext_set_privacy,
	.set_encryption			= wext_set_key_ext,
	.get_seqnum				= wext_get_seqnum,
	.flush					= wext_flush,
	.set_generic_elem		= wext_set_opt_ie,
	.wireless_event_init	= wext_wireless_event_init,
	.wireless_event_deinit	= wext_wireless_event_deinit,
	.sta_set_flags			= wext_sta_set_flags,
	.read_sta_data			= wext_read_sta_driver_data,
	.send_eapol				= wext_send_eapol,
	.sta_disassoc			= wext_sta_disassoc,
	.sta_deauth				= wext_sta_deauth,
	.set_ssid				= wext_set_ssid,
	.get_ssid				= wext_get_ssid,
	.set_countermeasures	= wext_set_countermeasures,
	.sta_clear_stats        = wext_sta_clear_stats,
	.commit					= wext_commit,
};
