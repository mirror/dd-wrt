/*
 * hostapd / Kernel driver communication with Devicescape IEEE 802.11 stack
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
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
#include <sys/ioctl.h>

#ifdef USE_KERNEL_HEADERS
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>
#include <linux/wireless.h>
#else /* USE_KERNEL_HEADERS */
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include "wireless_copy.h"
#endif /* USE_KERNEL_HEADERS */

#include "hostapd.h"
#include "driver.h"
#include "ieee802_1x.h"
#include "eloop.h"
#include "priv_netlink.h"
#include "ieee802_11.h"
#include "sta_info.h"
#include "hw_features.h"
#include <hostapd_ioctl.h>
#include <ieee80211_common.h>
/* from net/mac80211.h */
enum {
	MODE_IEEE80211A = 0 /* IEEE 802.11a */,
	MODE_IEEE80211B = 1 /* IEEE 802.11b only */,
	MODE_ATHEROS_TURBO = 2 /* Atheros Turbo mode (2x.11a at 5 GHz) */,
	MODE_IEEE80211G = 3 /* IEEE 802.11g (and 802.11b compatibility) */,
	MODE_ATHEROS_TURBOG = 4 /* Atheros Turbo mode (2x.11g at 2.4 GHz) */,
	NUM_IEEE80211_MODES = 5
};
#include "mlme.h"


struct i802_driver_data {
	struct driver_ops ops;
	struct hostapd_data *hapd;

	char iface[IFNAMSIZ + 1];
	char mgmt_iface[IFNAMSIZ + 1];
	int mgmt_ifindex;
	int sock; /* raw packet socket for driver access */
	int ioctl_sock; /* socket for ioctl() use */
	int wext_sock; /* socket for wireless events */

	int we_version;
};

static const struct driver_ops devicescape_driver_ops;


#define HAPD_DECL	struct hostapd_data *hapd = iface->bss[0]

static int i802_sta_set_flags(void *priv, const u8 *addr,
			      int flags_or, int flags_and);


static int hostapd_set_iface_flags(struct i802_driver_data *drv, int dev_up)
{
	struct ifreq ifr;

	if (drv->ioctl_sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", drv->iface);

	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		wpa_printf(MSG_DEBUG, "Could not read interface flags (%s)",
			   drv->mgmt_iface);
		return -1;
	}

	if (dev_up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	if (ioctl(drv->ioctl_sock, SIOCSIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCSIFFLAGS]");
		return -1;
	}

	if (dev_up) {
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, IFNAMSIZ, "%s", drv->mgmt_iface);
		ifr.ifr_mtu = HOSTAPD_MTU;
		if (ioctl(drv->ioctl_sock, SIOCSIFMTU, &ifr) != 0) {
			perror("ioctl[SIOCSIFMTU]");
			printf("Setting MTU failed - trying to survive with "
			       "current value\n");
		}
	}

	return 0;
}


static int hostapd_ioctl_iface(const char *iface, struct i802_driver_data *drv,
			       struct prism2_hostapd_param *param, int len)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, iface, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) param;
	iwr.u.data.length = len;

	if (ioctl(drv->ioctl_sock, PRISM2_IOCTL_HOSTAPD, &iwr) < 0) {
		/* perror("ioctl[PRISM2_IOCTL_HOSTAPD]"); */
		return -1;
	}

	return 0;
}


static int hostapd_ioctl(struct i802_driver_data *drv,
			 struct prism2_hostapd_param *param, int len)
{
	return hostapd_ioctl_iface(drv->iface, drv, param, len);
}


static int i802_set_encryption(const char *iface, void *priv, const char *alg,
			       const u8 *addr, int idx, const u8 *key,
			       size_t key_len, int txkey)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	u8 *buf;
	size_t blen;
	int ret = 0;

	blen = sizeof(*param) + key_len;
	buf = wpa_zalloc(blen);
	if (buf == NULL)
		return -1;

	param = (struct prism2_hostapd_param *) buf;
	param->cmd = PRISM2_SET_ENCRYPTION;
	if (addr == NULL)
		memset(param->sta_addr, 0xff, ETH_ALEN);
	else
		memcpy(param->sta_addr, addr, ETH_ALEN);
	strncpy((char *) param->u.crypt.alg, alg, HOSTAP_CRYPT_ALG_NAME_LEN);
	param->u.crypt.flags = txkey ? HOSTAP_CRYPT_FLAG_SET_TX_KEY : 0;
	param->u.crypt.idx = idx;
	param->u.crypt.key_len = key_len;
	memcpy(param->u.crypt.key, key, key_len);

	if (hostapd_ioctl_iface(iface, drv, param, blen) && errno != ENOENT) {
		printf("%s: Failed to set encryption to alg '%s' addr " MACSTR
		       " errno=%d\n",
		       iface, alg, MAC2STR(param->sta_addr), errno);
		ret = -1;
	}

	free(buf);

	return ret;
}


static int i802_get_seqnum(const char *iface, void *priv, const u8 *addr,
			   int idx, u8 *seq)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	size_t param_len;
	int ret;

	param_len = sizeof(struct prism2_hostapd_param) + 32;
	param = wpa_zalloc(param_len);
	if (param == NULL)
		return -1;

	param->cmd = PRISM2_GET_ENCRYPTION;
	if (addr == NULL)
		memset(param->sta_addr, 0xff, ETH_ALEN);
	else
		memcpy(param->sta_addr, addr, ETH_ALEN);
	param->u.crypt.idx = idx;

	ret = hostapd_ioctl_iface(iface, drv, param, param_len);
	if (ret == 0) {
		memcpy(seq, param->u.crypt.seq_counter,
		       HOSTAP_SEQ_COUNTER_SIZE);
	}
	free(param);
	return ret;
}


static int i802_set_rate_sets(void *priv, int *supp_rates, int *basic_rates,
			      int mode)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	u8 *buf;
	u16 *pos;
	size_t blen;
	int ret = 0;
	int i, num_supp = 0, num_basic = 0;

	if (supp_rates)
		for (i = 0; supp_rates[i] >= 0; i++)
			num_supp++;
	if (basic_rates)
		for (i = 0; basic_rates[i] >= 0; i++)
			num_basic++;

	blen = sizeof(*param) + (num_supp + num_basic) * 2;
	buf = wpa_zalloc(blen);
	if (buf == NULL)
		return -1;

	param = (struct prism2_hostapd_param *) buf;
	param->cmd = PRISM2_HOSTAPD_SET_RATE_SETS;
	switch (mode) {
	case HOSTAPD_MODE_IEEE80211A:
		param->u.set_rate_sets.mode = MODE_IEEE80211A;
		break;
	case HOSTAPD_MODE_IEEE80211B:
		param->u.set_rate_sets.mode = MODE_IEEE80211B;
		break;
	case HOSTAPD_MODE_IEEE80211G:
		param->u.set_rate_sets.mode = MODE_IEEE80211G;
		break;
	}
	param->u.set_rate_sets.num_supported_rates = num_supp;
	param->u.set_rate_sets.num_basic_rates = num_basic;
	pos = (u16 *) param->u.set_rate_sets.data;

	for (i = 0; i < num_supp; i++)
		*pos++ = supp_rates[i];
	for (i = 0; i < num_basic; i++)
		*pos++ = basic_rates[i];

	if (hostapd_ioctl(drv, param, blen)) {
		printf("Failed to set rate sets.\n");
		ret = -1;
	}
	free(buf);

	return ret;
}


static int hostap_ioctl_prism2param_iface(const char *iface,
					  struct i802_driver_data *drv,
					  int param, int value)
{
	struct iwreq iwr;
	int *i;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, iface, IFNAMSIZ);
	i = (int *) iwr.u.name;
	*i++ = param;
	*i++ = value;

	if (ioctl(drv->ioctl_sock, PRISM2_IOCTL_PRISM2_PARAM, &iwr) < 0) {
		char buf[128];
		snprintf(buf, sizeof(buf),
			 "%s: ioctl[PRISM2_IOCTL_PRISM2_PARAM]", iface);
		perror(buf);
		return -1;
	}

	return 0;
}


static int hostap_ioctl_prism2param(struct i802_driver_data *drv, int param,
				    int value)
{
	return hostap_ioctl_prism2param_iface(drv->iface, drv, param, value);
}


static int hostap_ioctl_get_prism2param_iface(const char *iface,
					      struct i802_driver_data *drv,
					      int param)
{
	struct iwreq iwr;
	int *i;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, iface, IFNAMSIZ);
	i = (int *) iwr.u.name;
	*i = param;

	if (ioctl(drv->ioctl_sock, PRISM2_IOCTL_GET_PRISM2_PARAM, &iwr) < 0) {
		char buf[128];
		snprintf(buf, sizeof(buf),
			 "%s: ioctl[PRISM2_IOCTL_GET_PRISM2_PARAM]", iface);
		perror(buf);
		return -1;
	}

	return *i;
}


static int hostap_ioctl_get_prism2param(struct i802_driver_data *drv,
					int param)
{
	return hostap_ioctl_get_prism2param_iface(drv->iface, drv, param);
}


static int i802_set_ssid(const char *ifname, void *priv, const u8 *buf,
			 int len)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.essid.flags = 1; /* SSID active */
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len;

	if (ioctl(drv->ioctl_sock, SIOCSIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCSIWESSID]");
		printf("len=%d\n", len);
		return -1;
	}

	return 0;
}


static int i802_send_mgmt_frame(void *priv, const void *msg, size_t len,
				int flags)
{
	struct i802_driver_data *drv = priv;

	return send(drv->sock, msg, len, flags);
}


/* Set kernel driver on given frequency (MHz) */
static int i802_set_freq(void *priv, int mode, int freq)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	switch (mode) {
	case HOSTAPD_MODE_IEEE80211A:
		mode = MODE_IEEE80211A;
		break;
	case HOSTAPD_MODE_IEEE80211B:
		mode = MODE_IEEE80211B;
		break;
	case HOSTAPD_MODE_IEEE80211G:
		mode = MODE_IEEE80211G;
		break;
	}

	hostap_ioctl_prism2param(drv, PRISM2_PARAM_NEXT_MODE, mode);

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);
	iwr.u.freq.m = freq;
	iwr.u.freq.e = 6;

	if (ioctl(drv->ioctl_sock, SIOCSIWFREQ, &iwr) < 0) {
		perror("ioctl[SIOCSIWFREQ]");
		return -1;
	}

	return 0;
}


static int i802_set_rts(void *priv, int rts)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);
	iwr.u.rts.value = rts;
	iwr.u.rts.fixed = 1;

	if (ioctl(drv->ioctl_sock, SIOCSIWRTS, &iwr) < 0) {
		perror("ioctl[SIOCSIWRTS]");
		return -1;
	}

	return 0;
}


static int i802_get_rts(void *priv, int *rts)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);

	if (ioctl(drv->ioctl_sock, SIOCGIWRTS, &iwr) < 0) {
		perror("ioctl[SIOCGIWRTS]");
		return -1;
	}

	*rts = iwr.u.rts.value;

	return 0;
}


static int i802_set_frag(void *priv, int frag)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);
	iwr.u.frag.value = frag;
	iwr.u.frag.fixed = 1;

	if (ioctl(drv->ioctl_sock, SIOCSIWFRAG, &iwr) < 0) {
		perror("ioctl[SIOCSIWFRAG]");
		return -1;
	}

	return 0;
}


static int i802_get_frag(void *priv, int *frag)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);

	if (ioctl(drv->ioctl_sock, SIOCGIWFRAG, &iwr) < 0) {
		perror("ioctl[SIOCGIWFRAG]");
		return -1;
	}

	*frag = iwr.u.frag.value;

	return 0;
}


static int i802_set_retry(void *priv, int short_retry, int long_retry)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);

	iwr.u.retry.value = short_retry;
	iwr.u.retry.flags = IW_RETRY_LIMIT | IW_RETRY_MIN;
	if (ioctl(drv->ioctl_sock, SIOCSIWFRAG, &iwr) < 0) {
		perror("ioctl[SIOCSIWRETRY(short)]");
		return -1;
	}

	iwr.u.retry.value = long_retry;
	iwr.u.retry.flags = IW_RETRY_LIMIT | IW_RETRY_MAX;
	if (ioctl(drv->ioctl_sock, SIOCSIWFRAG, &iwr) < 0) {
		perror("ioctl[SIOCSIWRETRY(long)]");
		return -1;
	}

	return 0;
}


static int i802_get_retry(void *priv, int *short_retry, int *long_retry)
{
	struct i802_driver_data *drv = priv;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->hapd->conf->iface, IFNAMSIZ);

	iwr.u.retry.flags = IW_RETRY_LIMIT | IW_RETRY_MIN;
	if (ioctl(drv->ioctl_sock, SIOCGIWRETRY, &iwr) < 0) {
		perror("ioctl[SIOCGIWFRAG(short)]");
		return -1;
	}
	*short_retry = iwr.u.retry.value;

	iwr.u.retry.flags = IW_RETRY_LIMIT | IW_RETRY_MAX;
	if (ioctl(drv->ioctl_sock, SIOCGIWRETRY, &iwr) < 0) {
		perror("ioctl[SIOCGIWFRAG(long)]");
		return -1;
	}
	*long_retry = iwr.u.retry.value;

	return 0;
}


static int i802_flush(void *priv)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_FLUSH;
	return hostapd_ioctl(drv, &param, sizeof(param));
}


static int i802_read_sta_data(void *priv, struct hostap_sta_driver_data *data,
			      const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(data, 0, sizeof(*data));

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_GET_INFO_STA;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	if (hostapd_ioctl(drv, &param, sizeof(param))) {
		printf("  Could not get station info from kernel driver.\n");
		return -1;
	}

	data->inactive_msec = param.u.get_info_sta.inactive_msec;
	data->rx_packets = param.u.get_info_sta.rx_packets;
	data->tx_packets = param.u.get_info_sta.tx_packets;
	data->rx_bytes = param.u.get_info_sta.rx_bytes;
	data->tx_bytes = param.u.get_info_sta.tx_bytes;
	data->current_tx_rate = param.u.get_info_sta.current_tx_rate;
	data->flags = param.u.get_info_sta.flags;
	data->num_ps_buf_frames = param.u.get_info_sta.num_ps_buf_frames;
	data->tx_retry_failed = param.u.get_info_sta.tx_retry_failed;
	data->tx_retry_count = param.u.get_info_sta.tx_retry_count;
	data->last_rssi = param.u.get_info_sta.last_rssi;
	data->last_ack_rssi = param.u.get_info_sta.last_ack_rssi;
	return 0;
}


static int i802_send_eapol(void *priv, const u8 *addr, const u8 *data,
			   size_t data_len, int encrypt, const u8 *own_addr)
{
	struct i802_driver_data *drv = priv;
	struct ieee80211_hdr *hdr;
	size_t len;
	u8 *pos;
	int res;
#if 0 /* FIX */
	int qos = sta->flags & WLAN_STA_WME;
#else
	int qos = 0;
#endif

	len = sizeof(*hdr) + (qos ? 2 : 0) + sizeof(rfc1042_header) + 2 +
		data_len;
	hdr = wpa_zalloc(len);
	if (hdr == NULL) {
		printf("malloc() failed for i802_send_data(len=%lu)\n",
		       (unsigned long) len);
		return -1;
	}

	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	hdr->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	/* Request TX callback */
	hdr->frame_control |= host_to_le16(BIT(1));
	if (encrypt)
		hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	if (qos) {
		hdr->frame_control |=
			host_to_le16(WLAN_FC_STYPE_QOS_DATA << 4);
	}

	memcpy(hdr->IEEE80211_DA_FROMDS, addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_BSSID_FROMDS, own_addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_SA_FROMDS, own_addr, ETH_ALEN);
	pos = (u8 *) (hdr + 1);

	if (qos) {
		/* add an empty QoS header if needed */
		pos[0] = 0;
		pos[1] = 0;
		pos += 2;
	}

	memcpy(pos, rfc1042_header, sizeof(rfc1042_header));
	pos += sizeof(rfc1042_header);
	/* Network order, i.e. big endian, or MSB first */
	pos[0] = (ETH_P_PAE >> 8) & 0xff;
	pos[1] = ETH_P_PAE & 0xff;
	pos += 2;
	memcpy(pos, data, data_len);

	res = i802_send_mgmt_frame(drv, (u8 *) hdr, len, 0);
	free(hdr);

	if (res < 0) {
		perror("i802_send_eapol: send");
		printf("i802_send_eapol - packet len: %lu - failed\n",
		       (unsigned long) len);
	}

	return res;
}


static int i802_sta_add(const char *ifname, void *priv, const u8 *addr,
			u16 aid, u16 capability, u8 *supp_rates,
			size_t supp_rates_len, int flags)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;
	size_t len;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_ADD_STA;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	param.u.add_sta.aid = aid;
	param.u.add_sta.capability = capability;
	len = supp_rates_len;
	if (len > sizeof(param.u.add_sta.supp_rates))
		len = sizeof(param.u.add_sta.supp_rates);
	memcpy(param.u.add_sta.supp_rates, supp_rates, len);
	return hostapd_ioctl_iface(ifname, drv, &param, sizeof(param));
}


static int i802_sta_remove(void *priv, const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	i802_sta_set_flags(drv, addr, 0, ~WLAN_STA_AUTHORIZED);

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_REMOVE_STA;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	if (hostapd_ioctl(drv, &param, sizeof(param)))
		return -1;
	return 0;
}


static int i802_sta_set_flags(void *priv, const u8 *addr,
			      int flags_or, int flags_and)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_SET_FLAGS_STA;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	param.u.set_flags_sta.flags_or = flags_or;
	param.u.set_flags_sta.flags_and = flags_and;
	return hostapd_ioctl(drv, &param, sizeof(param));
}


static int i802_set_generic_elem(const char *ifname, void *priv,
				 const u8 *elem, size_t elem_len)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	u8 *buf;
	size_t blen;
	int ret = 0;

	blen = sizeof(*param) + elem_len;
	buf = wpa_zalloc(blen);
	if (buf == NULL)
		return -1;

	param = (struct prism2_hostapd_param *) buf;
	param->cmd = PRISM2_HOSTAPD_SET_GENERIC_INFO_ELEM;
	param->u.set_generic_info_elem.len = elem_len;
	memcpy(param->u.set_generic_info_elem.data, elem, elem_len);

	if (hostapd_ioctl_iface(ifname, drv, param, blen)) {
		printf("%s: Failed to set generic info element\n", drv->iface);
		ret = -1;
	}
	free(buf);

	return ret;
}


static int i802_set_channel_flag(void *priv, int mode, int chan, int flag,
				 unsigned char power_level,
				 unsigned char antenna_max)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_SET_CHANNEL_FLAG;
	switch (mode) {
	case HOSTAPD_MODE_IEEE80211A:
		param.u.set_channel_flag.mode = MODE_IEEE80211A;
		break;
	case HOSTAPD_MODE_IEEE80211B:
		param.u.set_channel_flag.mode = MODE_IEEE80211B;
		break;
	case HOSTAPD_MODE_IEEE80211G:
		param.u.set_channel_flag.mode = MODE_IEEE80211G;
		break;
	}

	param.u.set_channel_flag.chan = chan;
	param.u.set_channel_flag.flag = flag;
	param.u.set_channel_flag.power_level = power_level;
	param.u.set_channel_flag.antenna_max = antenna_max;

	if (hostapd_ioctl(drv, &param, sizeof(param))) {
		printf("Failed to set channel flag (mode=%d chan=%d flag=0x%x)"
		       "\n", mode, chan, flag);
		return -1;
	}

	return 0;
}


static int i802_set_regulatory_domain(void *priv, unsigned int rd)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_SET_REGULATORY_DOMAIN;
	param.u.set_regulatory_domain.rd = rd;

	if (hostapd_ioctl(drv, &param, sizeof(param))) {
		printf("Failed to set regulatory domain (%x)\n", rd);
		return -1;
	}

	return 0;
}


static int i802_set_tx_queue_params(void *priv, int queue, int aifs,
				    int cw_min, int cw_max, int burst_time)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_SET_TX_QUEUE_PARAMS;
	param.u.tx_queue_params.queue = queue;
	param.u.tx_queue_params.aifs = aifs;
	param.u.tx_queue_params.cw_min = cw_min;
	param.u.tx_queue_params.cw_max = cw_max;
	param.u.tx_queue_params.burst_time = burst_time;

	if (hostapd_ioctl(drv, &param, sizeof(param)))
		return -1;

	return 0;
}


static int i802_bss_add(void *priv, const char *ifname, const u8 *bssid)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;

	param = wpa_zalloc(sizeof(struct prism2_hostapd_param) + ETH_ALEN);
	if (param == NULL)
		return -1;

	param->cmd = PRISM2_HOSTAPD_ADD_IF;
	param->u.if_info.type = HOSTAP_IF_BSS;
	memcpy(param->u.if_info.data, bssid, ETH_ALEN);
	snprintf((char *) param->u.if_info.name, IFNAMSIZ, "%s", ifname);

	if (hostapd_ioctl(drv, param,
			  sizeof(struct prism2_hostapd_param) + ETH_ALEN)) {
		printf("Could not add bss iface: %s.\n", ifname);
		free(param);
		return -1;
	}

	free(param);

	return 0;
}


static int i802_bss_remove(void *priv, const char *ifname)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	int ret = 0;

	param = wpa_zalloc(sizeof(struct prism2_hostapd_param) + ETH_ALEN);
	if (param == NULL)
		return -1;

	param->cmd = PRISM2_HOSTAPD_REMOVE_IF;
	param->u.if_info.type = HOSTAP_IF_BSS;
	snprintf((char *) param->u.if_info.name, IFNAMSIZ, "%s", ifname);

	if (hostapd_ioctl(drv, param,
			  sizeof(struct prism2_hostapd_param) + ETH_ALEN)) {
		printf("Could not remove iface: %s.\n", ifname);
		ret = -1;
	}

	free(param);

	return ret;
}


static int i802_set_beacon(const char *ifname, void *priv,
			   u8 *head, size_t head_len,
			   u8 *tail, size_t tail_len)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	int len, ret = 0;

	param = wpa_zalloc(sizeof(*param) + head_len + tail_len);
	if (param == NULL) {
		printf("Failed to alloc memory for beacon ioctl\n");
		return -1;
	}
	len = (&param->u.beacon.data[0] - (u8 *) param) + head_len + tail_len;
	param->cmd = PRISM2_HOSTAPD_SET_BEACON;
	param->u.beacon.head_len = head_len;
	param->u.beacon.tail_len = tail_len;
	memcpy(&param->u.beacon.data[0], head, head_len);
	memcpy(&param->u.beacon.data[0] + head_len, tail, tail_len);

	if (len < (int) sizeof(*param))
		len = sizeof(*param);
	if (hostapd_ioctl_iface(ifname, drv, param, len)) {
		printf("Could not set beacon data to kernel driver.\n");
		printf("ifname='%s' head=%p head_len=%d tail=%p tail_len=%d "
		       "cmd=%d\n",
		       ifname, head, head_len, tail, tail_len, param->cmd);
		ret = -1;
	}

	free(param);
	return ret;
}


static int i802_set_ieee8021x(const char *ifname, void *priv, int enabled)
{
	struct i802_driver_data *drv = priv;

	if (hostap_ioctl_prism2param_iface(ifname, drv,
					   PRISM2_PARAM_IEEE_802_1X, enabled))
	{
		printf("%s: Could not %s IEEE 802.1X PAE support in kernel "
		       "driver.\n", ifname, enabled ? "enable" : "disable");
		return -1;
	}
	if (hostap_ioctl_prism2param_iface(ifname, drv,
					   PRISM2_PARAM_EAPOL, enabled)) {
		printf("%s: Could not %s EAPOL support in kernel "
		       "driver.\n", ifname, enabled ? "enable" : "disable");
		return -1;
	}
	return 0;
}


static int i802_set_privacy(const char *ifname, void *priv, int enabled)
{
	struct i802_driver_data *drv = priv;

	return hostap_ioctl_prism2param_iface(ifname, drv,
					      PRISM2_PARAM_PRIVACY_INVOKED,
					      enabled);
}


static int i802_set_internal_bridge(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_AP_BRIDGE_PACKETS,
					value);
}


static int i802_set_beacon_int(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_BEACON_INT, value);
}


static int i802_set_dtim_period(const char *ifname, void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param_iface(ifname, drv,
					      PRISM2_PARAM_DTIM_PERIOD, value);
}


static int i802_set_broadcast_ssid(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_BROADCAST_SSID,
					value);
}


static int i802_set_cts_protect(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv,
					PRISM2_PARAM_CTS_PROTECT_ERP_FRAMES,
					value);
}


static int i802_set_key_tx_rx_threshold(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_KEY_TX_RX_THRESHOLD,
					value);
}


static int i802_set_preamble(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_PREAMBLE, value);
}


static int i802_set_short_slot_time(void *priv, int value)
{
	struct i802_driver_data *drv = priv;
	return hostap_ioctl_prism2param(drv, PRISM2_PARAM_SHORT_SLOT_TIME,
					value);
}


static int i802_if_type(enum hostapd_driver_if_type type)
{
	switch (type) {
	case HOSTAPD_IF_VLAN:
		return HOSTAP_IF_VLAN;
	case HOSTAPD_IF_WDS:
		return HOSTAP_IF_WDS;
	}
	return -1;
}


static int i802_if_add(const char *iface, void *priv,
		       enum hostapd_driver_if_type type, char *ifname,
		       const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;

	param = malloc(sizeof(struct prism2_hostapd_param) + ETH_ALEN);
	if (!param)
		return -1;
	memset(param, 0, sizeof(param));

	param->cmd = PRISM2_HOSTAPD_ADD_IF;
	param->u.if_info.type = i802_if_type(type);
	if (addr)
		memcpy(param->u.if_info.data, addr, ETH_ALEN);
	else
		memset(param->u.if_info.data, 0, ETH_ALEN);
	snprintf((char *) param->u.if_info.name, IFNAMSIZ, "%s", ifname);

	/* FIX: should the size have + ETH_ALEN ? */
	if (hostapd_ioctl_iface(iface, drv, param,
				sizeof(struct prism2_hostapd_param))) {
		printf("Could not add iface: %s.\n", ifname);
		free(param);
		return -1;
	}

	snprintf(ifname, IFNAMSIZ, "%s", param->u.if_info.name);
	free(param);
	return 0;
}


static int i802_if_update(void *priv, enum hostapd_driver_if_type type,
			  char *ifname, const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;

	param = malloc(sizeof(struct prism2_hostapd_param) + ETH_ALEN);
	if (!param)
		return -1;
	memset(param, 0, sizeof(param));

	param->cmd = PRISM2_HOSTAPD_UPDATE_IF;
	param->u.if_info.type = i802_if_type(type);
	if (addr)
		memcpy(param->u.if_info.data, addr, ETH_ALEN);
	else
		memset(param->u.if_info.data, 0, ETH_ALEN);
	snprintf((char *) param->u.if_info.name, IFNAMSIZ, "%s", ifname);

	/* FIX: should the size have + ETH_ALEN ? */
	if (hostapd_ioctl(drv, param, sizeof(struct prism2_hostapd_param))) {
		printf("Could not update iface: %s.\n", ifname);
		free(param);
		return -1;
	}

	snprintf(ifname, IFNAMSIZ, "%s", param->u.if_info.name);
	free(param);
	return 0;
}


static int i802_if_remove(void *priv, enum hostapd_driver_if_type type,
			  const char *ifname, const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;

	param = malloc(sizeof(struct prism2_hostapd_param) + ETH_ALEN);
	if (!param)
		return -1;
	memset(param, 0, sizeof(param));

	param->cmd = PRISM2_HOSTAPD_REMOVE_IF;
	param->u.if_info.type = i802_if_type(type);
	if (addr)
		memcpy(param->u.if_info.data, addr, ETH_ALEN);
	else
		memset(param->u.if_info.data, 0, ETH_ALEN);
	snprintf((char *) param->u.if_info.name, IFNAMSIZ, "%s", ifname);
	if (hostapd_ioctl(drv, param, sizeof(struct prism2_hostapd_param))) {
		printf("Could not remove iface: %s.\n", ifname);
		free(param);
		return -1;
	}

	free(param);
	return 0;
}


static struct hostapd_hw_modes * i802_get_hw_feature_data(void *priv,
							  u16 *num_modes,
							  u16 *flags)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param *param;
	u8 *pos, *end;
	struct hostapd_hw_modes *modes;
	int i;

	param = wpa_zalloc(PRISM2_HOSTAPD_MAX_BUF_SIZE);
	if (param == NULL)
		return NULL;
	param->cmd = PRISM2_HOSTAPD_GET_HW_FEATURES;
	if (hostapd_ioctl(drv, param, PRISM2_HOSTAPD_MAX_BUF_SIZE)) {
		free(param);
		return NULL;
	}

	*num_modes = param->u.hw_features.num_modes;
	*flags = param->u.hw_features.flags;

	pos = param->u.hw_features.data;
	end = pos + PRISM2_HOSTAPD_MAX_BUF_SIZE -
		(param->u.hw_features.data - (u8 *) param);

	modes = wpa_zalloc(*num_modes * sizeof(struct hostapd_hw_modes));
	if (modes == NULL) {
		free(param);
		return NULL;
	}

	for (i = 0; i < *num_modes; i++) {
		struct hostapd_ioctl_hw_modes_hdr *hdr;
		struct hostapd_hw_modes *feature;
		int clen, rlen;

		hdr = (struct hostapd_ioctl_hw_modes_hdr *) pos;
		feature = &modes[i];
		switch (hdr->mode) {
		case MODE_IEEE80211A:
			feature->mode = HOSTAPD_MODE_IEEE80211A;
			break;
		case MODE_IEEE80211B:
			feature->mode = HOSTAPD_MODE_IEEE80211B;
			break;
		case MODE_IEEE80211G:
			feature->mode = HOSTAPD_MODE_IEEE80211G;
			break;
		default:
			wpa_printf(MSG_ERROR, "Unknown hw_mode=%d in "
				   "get_hw_features data", hdr->mode);
			hostapd_free_hw_features(modes, *num_modes);
			modes = NULL;
			break;
		}
		feature->num_channels = hdr->num_channels;
		feature->num_rates = hdr->num_rates;

		pos = (u8 *) (hdr + 1);

		clen = hdr->num_channels *
			sizeof(struct hostapd_channel_data);
		rlen = hdr->num_rates *
			sizeof(struct hostapd_rate_data);

		feature->channels = malloc(clen);
		feature->rates = malloc(rlen);
		if (!feature->channels || !feature->rates ||
		    pos + clen + rlen > end) {
			wpa_printf(MSG_ERROR, "%s: Could not parse data",
				   __func__);
			hostapd_free_hw_features(modes, *num_modes);
			modes = NULL;
			break;
		}

		memcpy(feature->channels, pos, clen);
		pos += clen;
		memcpy(feature->rates, pos, rlen);
		pos += rlen;
	}

	free(param);

	return modes;
}


static int i802_set_sta_vlan(void *priv, const u8 *addr, const char *ifname,
			     int vlan_id)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_SET_STA_VLAN;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	strncpy(param.u.set_sta_vlan.vlan_name, ifname, IFNAMSIZ);
	param.u.set_sta_vlan.vlan_id = vlan_id;
	return hostapd_ioctl(drv, &param, sizeof(param));
}


static void handle_data(struct hostapd_data *hapd, u8 *buf, size_t len,
			u16 stype, struct ieee80211_frame_info *fi)
{
	struct ieee80211_hdr *hdr;
	u16 fc, ethertype;
	u8 *pos, *sa;
	size_t left;
	struct sta_info *sta;

	if (len < sizeof(struct ieee80211_hdr))
		return;

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);

	if ((fc & (WLAN_FC_FROMDS | WLAN_FC_TODS)) != WLAN_FC_TODS) {
		printf("Not ToDS data frame (fc=0x%04x)\n", fc);
		return;
	}

	sa = hdr->addr2;
	sta = ap_get_sta(hapd, sa);
	if (!sta || !(sta->flags & WLAN_STA_ASSOC)) {
		printf("Data frame from not associated STA " MACSTR "\n",
		       MAC2STR(sa));
		if (sta && (sta->flags & WLAN_STA_AUTH))
			hostapd_sta_disassoc(
				hapd, sa,
				WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
		else
			hostapd_sta_deauth(
				hapd, sa,
				WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
		return;
	}

	pos = (u8 *) (hdr + 1);
	left = len - sizeof(*hdr);

	if (left < sizeof(rfc1042_header)) {
		printf("Too short data frame\n");
		return;
	}

	if (memcmp(pos, rfc1042_header, sizeof(rfc1042_header)) != 0) {
		printf("Data frame with no RFC1042 header\n");
		return;
	}
	pos += sizeof(rfc1042_header);
	left -= sizeof(rfc1042_header);

	if (left < 2) {
		printf("No ethertype in data frame\n");
		return;
	}

	ethertype = *pos++ << 8;
	ethertype |= *pos++;
	left -= 2;
	switch (ethertype) {
	case ETH_P_PAE:
		ieee802_1x_receive(hapd, sa, pos, left);
		break;

	default:
		printf("Unknown ethertype 0x%04x in data frame\n", ethertype);
		break;
	}
}


static void handle_tx_callback(struct hostapd_data *hapd, u8 *buf, size_t len,
			       int ok)
{
	struct ieee80211_hdr *hdr;
	u16 fc, type, stype;
	struct sta_info *sta;

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);

	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	switch (type) {
	case WLAN_FC_TYPE_MGMT:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "MGMT (TX callback) %s\n", ok ? "ACK" : "fail");
		ieee802_11_mgmt_cb(hapd, buf, len, stype, ok);
		break;
	case WLAN_FC_TYPE_CTRL:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "CTRL (TX callback) %s\n", ok ? "ACK" : "fail");
		break;
	case WLAN_FC_TYPE_DATA:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "DATA (TX callback) %s\n", ok ? "ACK" : "fail");
		sta = ap_get_sta(hapd, hdr->addr1);
		if (sta && sta->flags & WLAN_STA_PENDING_POLL) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR
				      " %s pending activity poll\n",
				      MAC2STR(sta->addr),
				      ok ? "ACKed" : "did not ACK");
			if (ok)
				sta->flags &= ~WLAN_STA_PENDING_POLL;
		}
		if (sta)
			ieee802_1x_tx_status(hapd, sta, buf, len, ok);
		break;
	default:
		printf("unknown TX callback frame type %d\n", type);
		break;
	}
}


static void dump_frame_info(struct ieee80211_frame_info *fi, size_t len)
{
	u64 ts, tus;

	tus = ts = be_to_host64(fi->hosttime);
	ts /= 1000000;
	tus -= ts * 1000000;
	wpa_hexdump(MSG_DEBUG, "Frame info dump", (u8 *) fi, len);
        printf("version:\t0x%08x\n", ntohl(fi->version));
        printf("length:\t%d\n", ntohl(fi->length));
        printf("mactime:\t%lld\n", be_to_host64(fi->mactime));
        printf("hosttime:\t%lld.%06lld\n", ts, tus);
        printf("phytype:\t%d\n", ntohl(fi->phytype));
        printf("channel:\t%d\n", ntohl(fi->channel));
        printf("datarate:\t%d\n", ntohl(fi->datarate));
        printf("antenna:\t%d\n", ntohl(fi->antenna));
        printf("priority\t%d\n", ntohl(fi->priority));
        printf("ssi_type:\t%d\n", ntohl(fi->ssi_type));
        printf("ssi_signal:\t%d\n", ntohl(fi->ssi_signal));
        printf("ssi_noise:\t%d\n", ntohl(fi->ssi_noise));
        printf("preamble:\t%d\n", ntohl(fi->preamble));
        printf("encoding:\t%d\n", ntohl(fi->encoding));
        printf("msg_type:\t%d\n", ntohl(fi->msg_type));
}


static void hostapd_michael_mic_failure(struct hostapd_data *hapd, u8 *buf,
					size_t len)
{
	struct ieee80211_hdr *hdr;

	if (len < 24) {
		printf("Too short frame (%d) with Michael MIC failure\n", len);
		return;
	}

	hdr = (struct ieee80211_hdr *) buf;

	mlme_michaelmicfailure_indication(hapd, hdr->addr2);
}


static void handle_frame(struct hostapd_iface *iface, u8 *buf, size_t len,
			 struct ieee80211_frame_info *fi)
{
	struct ieee80211_hdr *hdr;
	u16 fc, type, stype;
	size_t data_len = len;
	struct hostapd_data *hapd = NULL;
	int broadcast_bssid = 0;
	size_t i;
	u8 *bssid;
	int msg_type = ntohl(fi->msg_type);
	struct hostapd_frame_info hfi;

	/* special handling for message types without IEEE 802.11 header */
	if (msg_type == ieee80211_msg_set_aid_for_sta) {
#if 0 /* TODO */
		ieee802_11_set_aid_for_sta(iface->bss[0], buf, data_len);
#endif
		return;
	}
	if (msg_type == ieee80211_msg_key_threshold_notification) {
#if 0 /* TODO */
		ieee802_11_key_threshold_notification(iface->bss[0], buf,
						      data_len);
#endif
		return;
	}

	/* PS-Poll frame from not associated is 16 bytes. All other frames
	 * passed to hostapd are 24 bytes or longer. */
	if (len < 24 &&
	    (msg_type != ieee80211_msg_sta_not_assoc || len < 16)) {
		printf("handle_frame: too short (%lu), type %d\n",
		       (unsigned long) len, msg_type);
		return;
	}

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);
	bssid = hdr->addr3;

	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	if (type == WLAN_FC_TYPE_DATA) {
		switch (fc & (WLAN_FC_FROMDS | WLAN_FC_TODS)) {
		case WLAN_FC_TODS:
			bssid = hdr->addr1;
			break;
		case WLAN_FC_FROMDS:
			bssid = hdr->addr2;
			break;
		}
	}

	for (i = 0; i < iface->num_bss; i++) {
		if (memcmp(bssid, iface->bss[i]->own_addr, ETH_ALEN) == 0) {
			hapd = iface->bss[i];
			break;
		}
	}
	if (hapd == NULL) {
		hapd = iface->bss[0];

		if (bssid[0] != 0xff || bssid[1] != 0xff ||
		    bssid[2] != 0xff || bssid[3] != 0xff ||
		    bssid[4] != 0xff || bssid[5] != 0xff) {
			/* Unknown BSSID - drop frame if this is not from
			 * passive scanning or a beacon
			 * (at least ProbeReq frames to other APs may be
			 * allowed through RX filtering in the wlan hw/driver)
			 */
			if (msg_type != ieee80211_msg_passive_scan &&
			    (type != WLAN_FC_TYPE_MGMT ||
			     stype != WLAN_FC_STYPE_BEACON))
				return;
		} else
			broadcast_bssid = 1;
	}

	switch (msg_type) {
	case ieee80211_msg_normal:
	case ieee80211_msg_passive_scan:
		/* continue processing */
		break;
	case ieee80211_msg_tx_callback_ack:
		handle_tx_callback(hapd, buf, data_len, 1);
		return;
	case ieee80211_msg_tx_callback_fail:
		handle_tx_callback(hapd, buf, data_len, 0);
		return;
	case ieee80211_msg_wep_frame_unknown_key:
		/* TODO: ieee802_11_rx_unknown_key(hapd, buf, data_len); */
		return;
	case ieee80211_msg_michael_mic_failure:
		hostapd_michael_mic_failure(hapd, buf, data_len);
		return;
	case ieee80211_msg_sta_not_assoc:
		/* TODO: ieee802_11_rx_sta_not_assoc(hapd, buf, data_len); */
		return;
	default:
		printf("handle_frame: unknown msg_type %d\n", msg_type);
		return;
	}

	switch (type) {
	case WLAN_FC_TYPE_MGMT:
		HOSTAPD_DEBUG((stype == WLAN_FC_STYPE_BEACON ||
			       stype == WLAN_FC_STYPE_PROBE_REQ) ?
			      HOSTAPD_DEBUG_EXCESSIVE : HOSTAPD_DEBUG_VERBOSE,
			      "MGMT\n");
		memset(&hfi, 0, sizeof(hfi));
		hfi.phytype = ntohl(fi->phytype);
		hfi.channel = ntohl(fi->channel);
		hfi.datarate = ntohl(fi->datarate);
		hfi.ssi_signal = ntohl(fi->ssi_signal);
		hfi.passive_scan = ntohl(fi->msg_type) ==
			ieee80211_msg_passive_scan;
		if (broadcast_bssid) {
			for (i = 0; i < iface->num_bss; i++)
				ieee802_11_mgmt(iface->bss[i], buf, data_len,
						stype, &hfi);
		} else
			ieee802_11_mgmt(hapd, buf, data_len, stype, &hfi);
		break;
	case WLAN_FC_TYPE_CTRL:
		/* TODO: send deauth/disassoc if not associated STA sends
		 * PS-Poll */
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "CTRL\n");
		break;
	case WLAN_FC_TYPE_DATA:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "DATA\n");
		handle_data(hapd, buf, data_len, stype, fi);
		break;
	default:
		printf("unknown frame type %d\n", type);
		break;
	}
}


static void handle_read(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct hostapd_iface *iface = eloop_ctx;
	int len;
	unsigned char buf[3000];
	struct ieee80211_frame_info *fi;
	HAPD_DECL;

	len = recv(sock, buf, sizeof(buf), 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS,
		      "Received %d bytes management frame\n", len);
	if (HOSTAPD_DEBUG_COND(HOSTAPD_DEBUG_MSGDUMPS)) {
		int i;
		printf("  dump:");
		for (i = 0; i < len; i++)
			printf(" %02x", buf[i]);
		printf("\n");
        }

	if (len < (int) sizeof(struct ieee80211_frame_info)) {
		printf("handle_read: too short (%d)\n", len);
		return;
	}

	fi = (struct ieee80211_frame_info *) buf;

	if (ntohl(fi->version) != IEEE80211_FI_VERSION) {
		printf("Invalid frame info version!\n");
		dump_frame_info(fi, len);
		return;
	}

	handle_frame(iface,
		     buf + sizeof(struct ieee80211_frame_info),
		     len - sizeof(struct ieee80211_frame_info),
		     fi);
}


static int i802_init_sockets(struct i802_driver_data *drv)
{
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_iface *iface = hapd->iface;
	struct ifreq ifr;
	struct sockaddr_ll addr;
	struct iwreq iwr;

	drv->sock = drv->ioctl_sock = -1;

	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	/* Enable management interface */
	if (hostap_ioctl_prism2param(drv, PRISM2_PARAM_MGMT_IF, 1) < 0) {
		printf("Failed to enable management interface.\n");
		return -1;
	}
	drv->mgmt_ifindex =
		hostap_ioctl_get_prism2param(drv, PRISM2_PARAM_MGMT_IF);
	if (drv->mgmt_ifindex < 0) {
		printf("Failed to get ifindex for the management "
		       "interface.\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_ifindex = drv->mgmt_ifindex;
	if (ioctl(drv->ioctl_sock, SIOCGIFNAME, &ifr) != 0) {
		perror("ioctl(SIOCGIFNAME)");
		return -1;
	}
	snprintf(drv->mgmt_iface, sizeof(drv->mgmt_iface), "%s", ifr.ifr_name);

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, hapd->conf->iface, IFNAMSIZ);
	iwr.u.mode = IW_MODE_MASTER;

	if (ioctl(drv->ioctl_sock, SIOCSIWMODE, &iwr) < 0) {
		perror("ioctl[SIOCSIWMODE]");
	}

	if (hostapd_set_iface_flags(drv, 1))
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "Opening raw packet socket for ifindex %d\n",
		      addr.sll_ifindex);

	drv->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (drv->sock < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}

	if (bind(drv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror(__FILE__ ":bind");
		return -1;
	}

	if (eloop_register_read_sock(drv->sock, handle_read, iface, NULL)) {
		printf("Could not register read socket\n");
		return -1;
	}

        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", drv->iface);
        if (ioctl(drv->sock, SIOCGIFHWADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		return -1;
        }

	if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
		printf("Invalid HW-addr family 0x%04x\n",
		       ifr.ifr_hwaddr.sa_family);
		return -1;
	}
	memcpy(drv->hapd->own_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	return 0;
}


static int i802_get_inact_sec(void *priv, const u8 *addr)
{
	struct i802_driver_data *drv = priv;
	struct prism2_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = PRISM2_HOSTAPD_GET_INFO_STA;
	memcpy(param.sta_addr, addr, ETH_ALEN);
	if (hostapd_ioctl(drv, &param, sizeof(param))) {
		return -1;
	}

	return param.u.get_info_sta.inactive_msec / 1000;
}


static int i802_sta_clear_stats(void *priv, const u8 *addr)
{
#if 0
	/* TODO */
#endif
	return 0;
}


static void
hostapd_wireless_event_wireless_custom(struct i802_driver_data *drv,
				       char *custom)
{
	struct hostapd_data *hapd = drv->hapd;

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Custom wireless event: '%s'\n",
		      custom);

	if (strncmp(custom, "MLME-MICHAELMICFAILURE.indication", 33) == 0) {
		char *pos;
		u8 addr[ETH_ALEN];
		pos = strstr(custom, "addr=");
		if (pos == NULL) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				      "MLME-MICHAELMICFAILURE.indication "
				      "without sender address ignored\n");
			return;
		}
		pos += 5;
		if (hwaddr_aton(pos, addr) == 0) {
			ieee80211_michael_mic_failure(drv->hapd, addr, 1);
		} else {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				      "MLME-MICHAELMICFAILURE.indication "
				      "with invalid MAC address");
		}
	}
}


static void hostapd_wireless_event_wireless(struct i802_driver_data *drv,
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
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "Wireless event: "
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
		case IWEVCUSTOM:
			if (custom + iwe->u.data.length > end)
				return;
			buf = malloc(iwe->u.data.length + 1);
			if (buf == NULL)
				return;
			memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';
			hostapd_wireless_event_wireless_custom(drv, buf);
			free(buf);
			break;
		}

		pos += iwe->len;
	}
}


static void hostapd_wireless_event_rtm_newlink(struct i802_driver_data *drv,
					       struct nlmsghdr *h, int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr *attr;

	if (len < (int) sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);

	/* TODO: use ifi->ifi_index to filter out wireless events from other
	 * interfaces */

	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			hostapd_wireless_event_wireless(
				drv, ((char *) attr) + rta_len,
				attr->rta_len - rta_len);
		}
		attr = RTA_NEXT(attr, attrlen);
	}
}


static void hostapd_wireless_event_receive(int sock, void *eloop_ctx,
					   void *sock_ctx)
{
	char buf[256];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	struct i802_driver_data *drv = eloop_ctx;

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
			hostapd_wireless_event_rtm_newlink(drv, h, plen);
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


static int hostap_get_we_version(struct i802_driver_data *drv)
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
	range = wpa_zalloc(buflen);
	if (range == NULL)
		return -1;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
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


static int i802_wireless_event_init(void *priv)
{
	struct i802_driver_data *drv = priv;
	int s;
	struct sockaddr_nl local;

	hostap_get_we_version(drv);

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

	eloop_register_read_sock(s, hostapd_wireless_event_receive, drv,
				 NULL);
	drv->wext_sock = s;

	return 0;
}


static void i802_wireless_event_deinit(void *priv)
{
	struct i802_driver_data *drv = priv;
	if (drv->wext_sock < 0)
		return;
	eloop_unregister_read_sock(drv->wext_sock);
	close(drv->wext_sock);
}


static int i802_sta_deauth(void *priv, const u8 *addr, int reason)
{
	struct i802_driver_data *drv = priv;
	struct ieee80211_mgmt mgmt;

	memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DEAUTH);
	memcpy(mgmt.da, addr, ETH_ALEN);
	memcpy(mgmt.sa, drv->hapd->own_addr, ETH_ALEN);
	memcpy(mgmt.bssid, drv->hapd->own_addr, ETH_ALEN);
	mgmt.u.deauth.reason_code = host_to_le16(reason);
	return i802_send_mgmt_frame(drv, &mgmt, IEEE80211_HDRLEN +
				      sizeof(mgmt.u.deauth), 0);
}


static int i802_sta_disassoc(void *priv, const u8 *addr, int reason)
{
	struct i802_driver_data *drv = priv;
	struct ieee80211_mgmt mgmt;

	memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DISASSOC);
	memcpy(mgmt.da, addr, ETH_ALEN);
	memcpy(mgmt.sa, drv->hapd->own_addr, ETH_ALEN);
	memcpy(mgmt.bssid, drv->hapd->own_addr, ETH_ALEN);
	mgmt.u.disassoc.reason_code = host_to_le16(reason);
	return  i802_send_mgmt_frame(drv, &mgmt, IEEE80211_HDRLEN +
				       sizeof(mgmt.u.disassoc), 0);
}


static int i802_init(struct hostapd_data *hapd)
{
	struct i802_driver_data *drv;

	drv = wpa_zalloc(sizeof(struct i802_driver_data));
	if (drv == NULL) {
		printf("Could not allocate memory for i802 driver data\n");
		return -1;
	}

	drv->ops = devicescape_driver_ops;
	drv->hapd = hapd;
	memcpy(drv->iface, hapd->conf->iface, sizeof(drv->iface));

	if (i802_init_sockets(drv))
		goto failed;

	hapd->driver = &drv->ops;

	/* Enable the radio by default. */
	(void) hostap_ioctl_prism2param(drv, PRISM2_PARAM_RADIO_ENABLED, 1);

	return 0;

failed:
	free(drv);
	return -1;
}


static void i802_deinit(void *priv)
{
	struct i802_driver_data *drv = priv;

	/* Disable the radio. */
	(void) hostap_ioctl_prism2param(drv, PRISM2_PARAM_RADIO_ENABLED, 0);

	/* Disable management interface */
	(void) hostap_ioctl_prism2param(drv, PRISM2_PARAM_MGMT_IF, 0);

	drv->hapd->driver = NULL;

	(void) hostapd_set_iface_flags(drv, 0);

	if (drv->sock >= 0)
		close(drv->sock);
	if (drv->ioctl_sock >= 0)
		close(drv->ioctl_sock);

	free(drv);
}


static const struct driver_ops devicescape_driver_ops = {
	.name = "devicescape",
	.init = i802_init,
	.deinit = i802_deinit,
	.wireless_event_init = i802_wireless_event_init,
	.wireless_event_deinit = i802_wireless_event_deinit,
	.set_ieee8021x = i802_set_ieee8021x,
	.set_privacy = i802_set_privacy,
	.set_encryption = i802_set_encryption,
	.get_seqnum = i802_get_seqnum,
	.flush = i802_flush,
	.set_generic_elem = i802_set_generic_elem,
	.read_sta_data = i802_read_sta_data,
	.send_eapol = i802_send_eapol,
	.sta_set_flags = i802_sta_set_flags,
	.sta_deauth = i802_sta_deauth,
	.sta_disassoc = i802_sta_disassoc,
	.sta_remove = i802_sta_remove,
	.set_ssid = i802_set_ssid,
	.send_mgmt_frame = i802_send_mgmt_frame,
	.sta_add = i802_sta_add,
	.get_inact_sec = i802_get_inact_sec,
	.sta_clear_stats = i802_sta_clear_stats,
	.set_freq = i802_set_freq,
	.set_rts = i802_set_rts,
	.get_rts = i802_get_rts,
	.set_frag = i802_set_frag,
	.get_frag = i802_get_frag,
	.set_retry = i802_set_retry,
	.get_retry = i802_get_retry,
	.set_rate_sets = i802_set_rate_sets,
	.set_channel_flag = i802_set_channel_flag,
	.set_regulatory_domain = i802_set_regulatory_domain,
	.set_beacon = i802_set_beacon,
	.set_internal_bridge = i802_set_internal_bridge,
	.set_beacon_int = i802_set_beacon_int,
	.set_dtim_period = i802_set_dtim_period,
	.set_broadcast_ssid = i802_set_broadcast_ssid,
	.set_cts_protect = i802_set_cts_protect,
	.set_key_tx_rx_threshold = i802_set_key_tx_rx_threshold,
	.set_preamble = i802_set_preamble,
	.set_short_slot_time = i802_set_short_slot_time,
	.set_tx_queue_params = i802_set_tx_queue_params,
	.bss_add = i802_bss_add,
	.bss_remove = i802_bss_remove,
	.if_add = i802_if_add,
	.if_update = i802_if_update,
	.if_remove = i802_if_remove,
	.get_hw_feature_data = i802_get_hw_feature_data,
	.set_sta_vlan = i802_set_sta_vlan,
};


void devicescape_driver_register(void)
{
	driver_register(devicescape_driver_ops.name, &devicescape_driver_ops);
}
