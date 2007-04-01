/*
 * Linux Wireless Extensions support
 *
 * Copyright 2005-2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#include <linux/if_arp.h>
#include <asm/uaccess.h>

#include <wlc_cfg.h>

#include <typedefs.h>

#include <linux/module.h>
#include <osl.h>

#include <bcmutils.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>

typedef void wlc_info_t;
typedef void wl_info_t;
typedef const struct sb_pub  sb_t;
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wl_iw.h>


uint
bcm_mkiovar(char *name, char *data, uint datalen, char *buf, uint buflen)
{
	uint len;

	len = strlen(name) + 1;

	if ((len + datalen) > buflen)
		return 0;

	strcpy(buf, name);

	/* append data onto the end of the name string */
	memcpy(&buf[len], data, datalen);
	len += datalen;

	return len;
}
static int wl_ioctl(struct net_device *dev, int cmd, void *buf, int len)
{
	mm_segment_t old_fs = get_fs();
	struct ifreq ifr;
	int ret;
	wl_ioctl_t ioc;
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	strncpy(ifr.ifr_name, dev->name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;
	set_fs(KERNEL_DS);
	ret = dev->do_ioctl(dev,&ifr,SIOCDEVPRIVATE);
	set_fs (old_fs);
	return ret;
}

#define dev_wlc_ioctl(dev,cmd,arg,len) wl_ioctl(dev,cmd,arg,len)
/*static int
dev_wlc_ioctl(
	struct net_device *dev,
	int cmd,
	void *arg,
	int len
)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	mm_segment_t fs;
	int ret;

	ioc.cmd = cmd;
	ioc.buf = arg;
	ioc.len = len;

	strcpy(ifr.ifr_name, dev->name);
	ifr.ifr_data = (caddr_t) &ioc;

	dev_open(dev);

	fs = get_fs();
	set_fs(get_ds());
	ret = dev->do_ioctl(dev, &ifr, SIOCDEVPRIVATE);
	set_fs(fs);

	return ret;
}*/

/*
set named driver variable to int value and return error indication
calling example: dev_wlc_intvar_set(dev, "arate", rate)
*/

static int
dev_wlc_intvar_set(
	struct net_device *dev,
	char *name,
	int val)
{
	char buf[WLC_IOCTL_SMLEN];
	uint len;

	len = bcm_mkiovar(name, (char *)(&val), sizeof(val), buf, sizeof(buf));
	ASSERT(len);

	return (dev_wlc_ioctl(dev, WLC_SET_VAR, buf, len));
}

#if WIRELESS_EXT > 17
static int
dev_wlc_bufvar_set(
	struct net_device *dev,
	char *name,
	char *buf, int len)
{
	char ioctlbuf[WLC_IOCTL_SMLEN];
	uint buflen;

	buflen = bcm_mkiovar(name, buf, len, ioctlbuf, sizeof(ioctlbuf));
	ASSERT(buflen);

	return (dev_wlc_ioctl(dev, WLC_SET_VAR, ioctlbuf, buflen));
}

/*
get named driver variable to int value and return error indication
calling example: dev_wlc_intvar_get(dev, "arate", &rate)
*/

static int
dev_wlc_bufvar_get(
	struct net_device *dev,
	char *name,
	char *buf, int buflen)
{
	char ioctlbuf[WLC_IOCTL_SMLEN];
	int error;

	uint len;

	len = bcm_mkiovar(name, NULL, 0, ioctlbuf, sizeof(ioctlbuf));
	ASSERT(len);
	error = dev_wlc_ioctl(dev, WLC_GET_VAR, (void *)ioctlbuf, len);
	bcopy(ioctlbuf, buf, buflen);

	return (error);
}
#endif

/*
get named driver variable to int value and return error indication
calling example: dev_wlc_intvar_get(dev, "arate", &rate)
*/

static int
dev_wlc_intvar_get(
	struct net_device *dev,
	char *name,
	int *retval)
{
	union {
		char buf[WLC_IOCTL_SMLEN];
		int val;
	} var;
	int error;

	uint len;
	uint data_null;

	len = bcm_mkiovar(name, (char *)(&data_null), 0, (char *)(&var), sizeof(var.buf));
	ASSERT(len);
	error = dev_wlc_ioctl(dev, WLC_GET_VAR, (void *)&var, len);

	*retval = var.val;

	return (error);
}

/* Maintain backward compatibility */
#if WIRELESS_EXT < 13
struct iw_request_info
{
	__u16		cmd;		/* Wireless Extension command */
	__u16		flags;		/* More to come ;-) */
};

typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info,
	void *wrqu, char *extra);
#endif /* WIRELESS_EXT < 13 */

static int
wl_iw_config_commit(
	struct net_device *dev,
	struct iw_request_info *info,
	void *zwrq,
	char *extra
)
{
	wlc_ssid_t ssid;
	int error;
	struct sockaddr bssid;

	WL_TRACE(("%s: SIOCSIWCOMMIT\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_SSID, &ssid, sizeof(ssid))))
		return error;

	if (!ssid.SSID_len)
		return 0;

	bzero(&bssid, sizeof(struct sockaddr));
	if ((error = dev_wlc_ioctl(dev, WLC_REASSOC, &bssid, ETHER_ADDR_LEN))) {
		WL_ERROR(("Invalid ioctl data.\n"));
		return error;
	}

	return 0;
}

static int
wl_iw_get_name(
	struct net_device *dev,
	struct iw_request_info *info,
	char *cwrq,
	char *extra
)
{
	WL_TRACE(("%s: SIOCGIWNAME\n", dev->name));

	strcpy(cwrq, "IEEE 802.11-DS");

	return 0;
}

static int
wl_iw_set_freq(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_freq *fwrq,
	char *extra
)
{
	int error, chan;

	WL_TRACE(("%s: SIOCSIWFREQ\n", dev->name));

	/* Setting by channel number */
	if (fwrq->e == 0 && fwrq->m < MAXCHANNEL) {
		chan = fwrq->m;
	}

	/* Setting by frequency */
	else {
		/* Convert to MHz as best we can */
		if (fwrq->e >= 6) {
			fwrq->e -= 6;
			while (fwrq->e--)
				fwrq->m *= 10;
		} else if (fwrq->e < 6) {
			while (fwrq->e++ < 6)
				fwrq->m /= 10;
		}
		chan = wlc_freq2channel(fwrq->m);
	}
	if ((error = dev_wlc_ioctl(dev, WLC_SET_CHANNEL, &chan, sizeof(chan))))
		return error;

	/* -EINPROGRESS: Call commit handler */
	return -EINPROGRESS;
}

static int
wl_iw_get_freq(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_freq *fwrq,
	char *extra
)
{
	channel_info_t ci;
	int error;

	WL_TRACE(("%s: SIOCGIWFREQ\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_CHANNEL, &ci, sizeof(ci))))
		return error;

	/* Return radio channel in channel form */
	fwrq->m = ci.hw_channel;
	fwrq->e = 0;

	return 0;
}

static int
wl_iw_set_mode(
	struct net_device *dev,
	struct iw_request_info *info,
	__u32 *uwrq,
	char *extra
)
{
	int infra = 0, ap = 0, error = 0;

	WL_TRACE(("%s: SIOCSIWMODE\n", dev->name));

	switch (*uwrq) {
	case IW_MODE_MASTER:
		infra = ap = 1;
		break;
	case IW_MODE_ADHOC:
	case IW_MODE_AUTO:
		break;
	case IW_MODE_INFRA:
		infra = 1;
		break;
	default:
		return -EINVAL;
	}
	dev_wlc_ioctl(dev, WLC_SET_AP, &ap, sizeof(ap));

	dev_wlc_ioctl(dev, WLC_SET_INFRA, &infra, sizeof(infra));


	/* -EINPROGRESS: Call commit handler */
	return -EINPROGRESS;
}

static int
wl_iw_get_mode(
	struct net_device *dev,
	struct iw_request_info *info,
	__u32 *uwrq,
	char *extra
)
{
	int error, infra = 0, ap = 0;

	WL_TRACE(("%s: SIOCGIWMODE\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_INFRA, &infra, sizeof(infra))) ||
	    (error = dev_wlc_ioctl(dev, WLC_GET_AP, &ap, sizeof(ap))))
		return error;

	*uwrq = infra ? ap ? IW_MODE_MASTER : IW_MODE_INFRA : IW_MODE_ADHOC;

	return 0;
}
const long channel_frequency[] = {
	2412, 2417, 2422, 2427, 2432, 2437, 2442,
	2447, 2452, 2457, 2462, 2467, 2472, 2484
};

static int
wl_iw_get_range(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	struct iw_range *range = (struct iw_range *) extra;
	int channels[MAXCHANNEL];
	wl_uint32_list_t *list = (wl_uint32_list_t *) channels;
	wl_rateset_t rateset;
	int error, i;
	int k = 0;


	if (!extra)
		{
		return -EINVAL;
		}

	dwrq->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(range));

	/* We don't use nwids */
	range->min_nwid = range->max_nwid = 0;

	/* Set available channels/frequencies */

/*	range->num_channels = NUM_CHANNELS;
	for (i = 0; i < NUM_CHANNELS; i++) {
		range->freq[k].i = i + 1;
		range->freq[k].m = channel_frequency[i] * 100000;
		range->freq[k].e = 1;
		k++;
		if (k >= IW_MAX_FREQUENCIES)
			break;
	}

	range->num_frequency = range->num_channels;*/
	
	list->count = MAXCHANNEL;
	if ((error = dev_wlc_ioctl(dev, WLC_GET_VALID_CHANNELS, channels, sizeof(channels))))
		return error;
	for (i = 0; i < list->count && i < IW_MAX_FREQUENCIES; i++) {
		range->freq[i].i = list->element[i];
		range->freq[i].m = wlc_channel2freq(list->element[i]);
		range->freq[i].e = 6;
	}
	range->num_frequency = range->num_channels = i;


	/* Link quality (use NDIS cutoffs) */
	range->max_qual.qual = 5;
	/* Signal level (use RSSI) */
	range->max_qual.level = 0x100 - 200;	/* -200 dBm */
	/* Noise level (use noise) */
	range->max_qual.noise = 0x100 - 200;	/* -200 dBm */
	/* Signal level threshold range (?) */
	range->sensitivity = 65535;

#if WIRELESS_EXT > 11
	/* Link quality (use NDIS cutoffs) */
	range->avg_qual.qual = 3;
	/* Signal level (use RSSI) */
	range->avg_qual.level = 0x100 + WLC_RSSI_GOOD;
	/* Noise level (use noise) */
	range->avg_qual.noise = 0x100 - 75;	/* -75 dBm */
#endif /* WIRELESS_EXT > 11 */
	/* Set available bitrates */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_CURR_RATESET, &rateset, sizeof(rateset))))
		return error;
	range->num_bitrates = rateset.count;
	for (i = 0; i < rateset.count && i < IW_MAX_BITRATES; i++)
		range->bitrate[i] = (rateset.rates[i] & 0x7f) * 500000; /* convert to bps */

	/* Set an indication of the max TCP throughput
	 * in bit/s that we can expect using this interface.
	 * May be use for QoS stuff... Jean II
	 */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_PHYTYPE, &i, sizeof(i))))
		return error;
	if (i == PHY_TYPE_A)
		range->throughput = 24000000;	/* 24 Mbits/s */
	else
		range->throughput = 1500000;	/* 1.5 Mbits/s */

	/* RTS and fragmentation thresholds */
	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;

	range->max_encoding_tokens = DOT11_MAX_DEFAULT_KEYS;
	range->num_encoding_sizes = 4;
	range->encoding_size[0] = WEP1_KEY_SIZE;
	range->encoding_size[1] = WEP128_KEY_SIZE;
#if WIRELESS_EXT > 17
	range->encoding_size[2] = TKIP_KEY_SIZE;
#else
	range->encoding_size[2] = 0;
#endif
	range->encoding_size[3] = AES_KEY_SIZE;

	/* Do not support power micro-management */
	range->min_pmp = 0;
	range->max_pmp = 0;
	range->min_pmt = 0;
	range->max_pmt = 0;
	range->pmp_flags = 0;
	range->pm_capa = 0;

	/* Transmit Power - values are in mW */
	range->num_txpower = 2;
	range->txpower[0] = 1;
	range->txpower[1] = 255;
	range->txpower_capa = IW_TXPOW_MWATT;

#if WIRELESS_EXT > 10
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = WIRELESS_EXT;

	/* Only support retry limits */
	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
	range->r_time_flags = 0;
	/* SRL and LRL limits */
	range->min_retry = 1;
	range->max_retry = 255;
	/* Retry lifetime limits unsupported */
	range->min_r_time = 0;
	range->max_r_time = 0;
#endif /* WIRELESS_EXT > 10 */

#if WIRELESS_EXT > 17

	range->enc_capa = IW_ENC_CAPA_WPA;
	range->enc_capa |= IW_ENC_CAPA_CIPHER_TKIP;
	range->enc_capa |= IW_ENC_CAPA_CIPHER_CCMP;
#ifdef BCMWPA2
	range->enc_capa |= IW_ENC_CAPA_WPA2;
#endif
#endif /* WIRELESS_EXT > 17 */

	return 0;
}

static int
rssi_to_qual(int rssi)
{
	if (rssi <= WLC_RSSI_NO_SIGNAL)
		return 0;
	else if (rssi <= WLC_RSSI_VERY_LOW)
		return 1;
	else if (rssi <= WLC_RSSI_LOW)
		return 2;
	else if (rssi <= WLC_RSSI_GOOD)
		return 3;
	else if (rssi <= WLC_RSSI_VERY_GOOD)
		return 4;
	else
		return 5;
}

static int
wl_iw_set_spy(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_iw_t *iw = dev->priv;
	struct sockaddr *addr = (struct sockaddr *) extra;
	int i;

	WL_TRACE(("%s: SIOCSIWSPY\n", dev->name));

	if (!extra)
		return -EINVAL;

	iw->spy_num = MIN(ARRAYSIZE(iw->spy_addr), dwrq->length);
	for (i = 0; i < iw->spy_num; i++)
		memcpy(&iw->spy_addr[i], addr[i].sa_data, ETHER_ADDR_LEN);
	memset(iw->spy_qual, 0, sizeof(iw->spy_qual));

	return 0;
}

static int
wl_iw_get_spy(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_iw_t *iw = dev->priv;
	struct sockaddr *addr = (struct sockaddr *) extra;
	struct iw_quality *qual = (struct iw_quality *) &addr[iw->spy_num];
	int i;

	WL_TRACE(("%s: SIOCGIWSPY\n", dev->name));

	if (!extra)
		return -EINVAL;

	dwrq->length = iw->spy_num;
	for (i = 0; i < iw->spy_num; i++) {
		memcpy(addr[i].sa_data, &iw->spy_addr[i], ETHER_ADDR_LEN);
		addr[i].sa_family = AF_UNIX;
		memcpy(&qual[i], &iw->spy_qual[i], sizeof(struct iw_quality));
		iw->spy_qual[i].updated = 0;
	}

	return 0;
}

static int
wl_iw_set_wap(
	struct net_device *dev,
	struct iw_request_info *info,
	struct sockaddr *awrq,
	char *extra
)
{
	int error = -EINVAL;

	WL_TRACE(("%s: SIOCSIWAP\n", dev->name));

	if (awrq->sa_family != ARPHRD_ETHER) {
		WL_ERROR(("Invalid Header...sa_family\n"));
		return -EINVAL;
	}

	/* Ignore "auto" or "off" */
	if (ETHER_ISBCAST(awrq->sa_data) || ETHER_ISNULLADDR(awrq->sa_data)) {
		scb_val_t scbval;
		WL_ASSOC(("disassociating \n"));
		bzero(&scbval, sizeof (scb_val_t));
		dev_wlc_ioctl(dev, WLC_DISASSOC, &scbval, sizeof(scb_val_t));
		return 0;
	}
	WL_ASSOC(("Assoc to %s\n", bcm_ether_ntoa((struct ether_addr *)&(awrq->sa_data), eabuf)));
	/* Reassociate to the specified AP */
	if ((error = dev_wlc_ioctl(dev, WLC_REASSOC, awrq->sa_data, ETHER_ADDR_LEN))) {
		WL_ERROR(("Invalid ioctl data.\n"));
		return error;
	}

	return 0;
}

static int
wl_iw_get_wap(
	struct net_device *dev,
	struct iw_request_info *info,
	struct sockaddr *awrq,
	char *extra
)
{
	WL_TRACE(("%s: SIOCGIWAP\n", dev->name));

	awrq->sa_family = ARPHRD_ETHER;
	memset(awrq->sa_data, 0, ETHER_ADDR_LEN);

	/* Ignore error (may be down or disassociated) */
	(void) dev_wlc_ioctl(dev, WLC_GET_BSSID, awrq->sa_data, ETHER_ADDR_LEN);

	return 0;
}

#if WIRELESS_EXT > 17
static int
wl_iw_mlme(
	struct net_device *dev,
	struct iw_request_info *info,
	struct sockaddr *awrq,
	char *extra
)
{
	struct iw_mlme *mlme;
	scb_val_t scbval;
	int error  = -EINVAL;

	WL_TRACE(("%s: SIOCSIWMLME\n", dev->name));

	mlme = (struct iw_mlme *)extra;
	if (mlme == NULL) {
		WL_ERROR(("Invalid ioctl data.\n"));
		return error;
	}

	scbval.val = mlme->reason_code;
	bcopy(&mlme->addr.sa_data, &scbval.ea, ETHER_ADDR_LEN);

	if (mlme->cmd == IW_MLME_DISASSOC)
		error = dev_wlc_ioctl(dev, WLC_DISASSOC, &scbval, sizeof(scb_val_t));
	else if (mlme->cmd == IW_MLME_DEAUTH)
		error = dev_wlc_ioctl(dev, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scbval, sizeof(scb_val_t));
	else {
		WL_ERROR(("Invalid ioctl data.\n"));
		return error;
	}

	return error;
}
#endif

static int
wl_iw_get_aplist(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_scan_results_t *list;
	struct sockaddr *addr = (struct sockaddr *) extra;
	struct iw_quality qual[IW_MAX_AP];
	wl_bss_info_t *bi = NULL;
	int error, i;

	WL_TRACE(("%s: SIOCGIWAPLIST\n", dev->name));

	if (!extra)
		return -EINVAL;

	/* Get scan results (too large to put on the stack) */
	list = kmalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!list)
		return -ENOMEM;
	memset(list, 0, WLC_IOCTL_MAXLEN);
	list->buflen = WLC_IOCTL_MAXLEN;
	if ((error = dev_wlc_ioctl(dev, WLC_SCAN_RESULTS, list, WLC_IOCTL_MAXLEN))) {
		WL_ERROR(("%d: Scan results error %d\n", __LINE__, error));
		kfree(list);
		return error;
	}
	ASSERT(list->version == WL_BSS_INFO_VERSION);

	for (i = 0, dwrq->length = 0; i < list->count && dwrq->length < IW_MAX_AP; i++) {
		bi = bi ? (wl_bss_info_t *)((unsigned) bi + bi->length) : list->bss_info;
		ASSERT(((unsigned) bi + bi->length) <= ((unsigned) list + WLC_IOCTL_MAXLEN));

		/* Infrastructure only */
		if (!(bi->capability & DOT11_CAP_ESS))
			continue;

		/* BSSID */
		memcpy(addr[dwrq->length].sa_data, &bi->BSSID, ETHER_ADDR_LEN);
		addr[dwrq->length].sa_family = ARPHRD_ETHER;
		qual[dwrq->length].qual = rssi_to_qual(bi->RSSI);
		qual[dwrq->length].level = 0x100 + bi->RSSI;
		qual[dwrq->length].noise = 0x100 + bi->phy_noise;

		/* Updated qual, level, and noise */
		qual[dwrq->length].updated = 7;
		dwrq->length++;
	}

	kfree(list);

	if (dwrq->length) {
		memcpy(&addr[dwrq->length], qual, sizeof(struct iw_quality) * dwrq->length);
		/* Provided qual */
		dwrq->flags = 1;
	}

	return 0;
}

#if WIRELESS_EXT > 13
static int
wl_iw_set_scan(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	wlc_ssid_t ssid;

	WL_TRACE(("%s: SIOCSIWSCAN\n", dev->name));

	/* Broadcast scan */
	ssid.SSID_len = 0;

	/* Ignore error (most likely scan in progress) */
	(void) dev_wlc_ioctl(dev, WLC_SCAN, &ssid, sizeof(ssid));

	return 0;
}

#if WIRELESS_EXT > 17
static bool
ie_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, int *tlvs_len)
{
/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && 
		!bcmp((const void *)&ie[2], (const void *)(WPA_OUI "\x01"), 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)ie - (int)*tlvs;
	/* update the pointer to the start of the buffer */
	*tlvs = ie;
	return FALSE;
}
#endif

static int
wl_iw_get_scan(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_scan_results_t *list;
	struct iw_event	iwe;
	wl_bss_info_t *bi = NULL;
	int error, i, j;
	char *event = extra, *end = extra + IW_SCAN_MAX_DATA, *value;

	WL_TRACE(("%s: SIOCGIWSCAN\n", dev->name));

	if (!extra)
		return -EINVAL;

	/* Get scan results (too large to put on the stack) */
	list = kmalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!list)
		return -ENOMEM;
	memset(list, 0, WLC_IOCTL_MAXLEN);
	list->buflen = WLC_IOCTL_MAXLEN;
	if ((error = dev_wlc_ioctl(dev, WLC_SCAN_RESULTS, list, WLC_IOCTL_MAXLEN))) {
		kfree(list);
		return error;
	}
	ASSERT(list->version == WL_BSS_INFO_VERSION);

	for (i = 0; i < list->count && i < IW_MAX_AP; i++) {
		bi = bi ? (wl_bss_info_t *)((unsigned) bi + bi->length) : list->bss_info;
		ASSERT(((unsigned) bi + bi->length) <= ((unsigned) list + WLC_IOCTL_MAXLEN));

		/* First entry must be the BSSID */
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, &bi->BSSID, ETHER_ADDR_LEN);
		event = iwe_stream_add_event(event, end, &iwe, IW_EV_ADDR_LEN);

		/* SSID */
		iwe.u.data.length = bi->SSID_len;
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.flags = 1;
		event = iwe_stream_add_point(event, end, &iwe, bi->SSID);

		/* Mode */
		if (bi->capability & (DOT11_CAP_ESS | DOT11_CAP_IBSS)) {
			iwe.cmd = SIOCGIWMODE;
			if (bi->capability & DOT11_CAP_ESS)
				iwe.u.mode = IW_MODE_MASTER;
			else
				iwe.u.mode = IW_MODE_ADHOC;
			event = iwe_stream_add_event(event, end, &iwe, IW_EV_UINT_LEN);
		}

		/* Channel */
		iwe.cmd = SIOCGIWFREQ;
		iwe.u.freq.m = bi->chanspec & 0xff;
		iwe.u.freq.e = 0;
		event = iwe_stream_add_event(event, end, &iwe, IW_EV_FREQ_LEN);

		/* Channel quality */
		iwe.cmd = IWEVQUAL;
		iwe.u.qual.qual = rssi_to_qual(bi->RSSI);
		iwe.u.qual.level = 0x100 + bi->RSSI;
		iwe.u.qual.noise = 0x100 + bi->phy_noise;
		event = iwe_stream_add_event(event, end, &iwe, IW_EV_QUAL_LEN);

#if WIRELESS_EXT > 17
		/* wpa_ie */
		if (bi->ie_length) {
			/* look for wpa/rsn ies in the ie list... */
			bcm_tlv_t *ie;
			uint8*ptr= ((uint8 *)bi) + sizeof(wl_bss_info_t);
			int ptr_len = bi->ie_length;
#ifdef BCMWPA2
			if ((ie = bcm_parse_tlvs(ptr, ptr_len, DOT11_MNG_RSN_ID))) {
				iwe.cmd = IWEVGENIE;
				iwe.u.data.length = ie->len + 2;
				event = iwe_stream_add_point(event, end, &iwe, (char *)ie);
			}
			ptr = ((uint8 *)bi) + sizeof(wl_bss_info_t);
#endif /* BCMWPA2 */
			while ((ie = bcm_parse_tlvs(ptr, ptr_len, DOT11_MNG_WPA_ID))) {
				if (ie_is_wpa_ie(((uint8 **)&ie), &ptr, &ptr_len)) {
					iwe.cmd = IWEVGENIE;
					iwe.u.data.length = ie->len + 2;
					event = iwe_stream_add_point(event, end, &iwe, (char *)ie);
					goto done;
				}
			}
		}
done:
#endif
		/* Encryption */
		iwe.cmd = SIOCGIWENCODE;
		if (bi->capability & DOT11_CAP_PRIVACY)
			iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;
		iwe.u.data.length = 0;
		event = iwe_stream_add_point(event, end, &iwe, (char *)event);



		/* Rates */
		if (bi->rateset.count) {
			value = event + IW_EV_LCP_LEN;
			iwe.cmd = SIOCGIWRATE;
			/* Those two flags are ignored... */
			iwe.u.bitrate.fixed = iwe.u.bitrate.disabled = 0;
			for (j = 0; j < bi->rateset.count && j < IW_MAX_BITRATES; j++) {
				iwe.u.bitrate.value = (bi->rateset.rates[j] & 0x7f) * 500000;
				value = iwe_stream_add_value(event, value, end, &iwe,
					IW_EV_PARAM_LEN);
			}
			event = value;
		}
	}

	kfree(list);

	dwrq->length = event - extra;
	dwrq->flags = 0;	/* todo */

	return 0;
}
#endif /* WIRELESS_EXT > 13 */

static int
wl_iw_set_essid(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wlc_ssid_t ssid;
	int error;

	WL_TRACE(("%s: SIOCSIWESSID\n", dev->name));

	if (dwrq->length && extra) {
		ssid.SSID_len = MIN(sizeof(ssid.SSID), dwrq->length - 1);
		memcpy(ssid.SSID, extra, ssid.SSID_len);
	} else {
		/* Broadcast SSID */
		ssid.SSID_len = 0;
	}

	if ((error = dev_wlc_ioctl(dev, WLC_SET_SSID, &ssid, sizeof(ssid))))
		return error;

	return 0;
}

static int
wl_iw_get_essid(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wlc_ssid_t ssid;
	int error;

	WL_TRACE(("%s: SIOCGIWESSID\n", dev->name));

	if (!extra)
		return -EINVAL;

	if ((error = dev_wlc_ioctl(dev, WLC_GET_SSID, &ssid, sizeof(ssid)))) {
		WL_ERROR(("Error getting the SSID\n"));
		return error;
	}

	/* Get the current SSID */
	memcpy(extra, ssid.SSID, ssid.SSID_len);
	extra[ssid.SSID_len] = '\0';

	dwrq->length = ssid.SSID_len ;

	dwrq->flags = 1; /* active */

	return 0;
}

static int
wl_iw_set_nick(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_iw_t *iw = dev->priv;

	WL_TRACE(("%s: SIOCSIWNICKN\n", dev->name));

	if (!extra)
		return -EINVAL;

	/* Check the size of the string */
	if (dwrq->length > sizeof(iw->nickname))
		return -E2BIG;

	memcpy(iw->nickname, extra, dwrq->length);
	iw->nickname[dwrq->length - 1] = '\0';

	return 0;
}

static int
wl_iw_get_nick(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_iw_t *iw = dev->priv;

	WL_TRACE(("%s: SIOCGIWNICKN\n", dev->name));

	if (!extra)
		return -EINVAL;

	strcpy(extra, iw->nickname);
	dwrq->length = strlen(extra) + 1;

	return 0;
}

static int wl_iw_set_rate(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	wl_rateset_t rateset;
	int error, rate, i, error_bg, error_a;

	WL_TRACE(("%s: SIOCSIWRITE\n", dev->name));

	/* Get current rateset */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_CURR_RATESET, &rateset, sizeof(rateset))))
		return error;

	if (vwrq->value < 0) {
		/* Select maximum rate */
		rate = rateset.rates[rateset.count - 1] & 0x7f;
	} else if (vwrq->value < rateset.count) {
		/* Select rate by rateset index */
		rate = rateset.rates[vwrq->value] & 0x7f;
	} else {
		/* Specified rate in bps */
		rate = vwrq->value / 500000;
	}

	if (vwrq->fixed) {
		/*
			Set rate override,
			Since the is a/b/g-blind, both a/bg_rate are enforced.
		*/
		error_bg = dev_wlc_intvar_set(dev, "bg_rate", rate);
		error_a = dev_wlc_intvar_set(dev, "a_rate", rate);

		if (error_bg && error_a)
			return (error_bg | error_a);
	} else {
		/*
			clear rate override
			Since the is a/b/g-blind, both a/bg_rate are enforced.
		*/
		/* 0 is for clearing rate override */
		error_bg = dev_wlc_intvar_set(dev, "bg_rate", 0);
		/* 0 is for clearing rate override */
		error_a = dev_wlc_intvar_set(dev, "a_rate", 0);

		if (error_bg && error_a)
			return (error_bg | error_a);

		/* Remove rates above selected rate */
		for (i = 0; i < rateset.count; i++)
			if ((rateset.rates[i] & 0x7f) > rate)
				break;
		rateset.count = i;

		/* Set current rateset */
		if ((error = dev_wlc_ioctl(dev, WLC_SET_RATESET, &rateset, sizeof(rateset))))
			return error;
	}

	return 0;
}

static int wl_iw_get_rate(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, rate;

	WL_TRACE(("%s: SIOCGIWRATE\n", dev->name));

	/* Report the current tx rate */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_RATE, &rate, sizeof(rate))))
		return error;
	vwrq->value = (rate & 0x7f) * 500000;

	return 0;
}

static int
wl_iw_set_rts(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, rts;

	WL_TRACE(("%s: SIOCSIWRTS\n", dev->name));

	if (vwrq->disabled)
		rts = DOT11_DEFAULT_RTS_LEN;
	else if (vwrq->value < 0 || vwrq->value > DOT11_DEFAULT_RTS_LEN)
		return -EINVAL;
	else
		rts = vwrq->value;

	if ((error = dev_wlc_intvar_set(dev, "rtsthresh", rts)))
		return error;

	return 0;
}

static int
wl_iw_get_rts(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, rts;

	WL_TRACE(("%s: SIOCGIWRTS\n", dev->name));

	if ((error = dev_wlc_intvar_get(dev, "rtsthresh", &rts)))
		return error;

	vwrq->value = rts;
	vwrq->disabled = (rts >= DOT11_DEFAULT_RTS_LEN);
	vwrq->fixed = 1;

	return 0;
}

static int
wl_iw_set_frag(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, frag;

	WL_TRACE(("%s: SIOCSIWFRAG\n", dev->name));

	if (vwrq->disabled)
		frag = DOT11_DEFAULT_FRAG_LEN;
	else if (vwrq->value < 0 || vwrq->value > DOT11_DEFAULT_FRAG_LEN)
		return -EINVAL;
	else
		frag = vwrq->value;

	if ((error = dev_wlc_intvar_set(dev, "fragthresh", frag)))
		return error;

	return 0;
}

static int
wl_iw_get_frag(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, fragthreshold;

	WL_TRACE(("%s: SIOCGIWFRAG\n", dev->name));

	if ((error = dev_wlc_intvar_get(dev, "fragthresh", &fragthreshold)))
		return error;

	vwrq->value = fragthreshold;
	vwrq->disabled = (fragthreshold >= DOT11_DEFAULT_FRAG_LEN);
	vwrq->fixed = 1;

	return 0;
}

static int
wl_iw_set_txpow(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, disable;
	uint16 txpwrmw;
	WL_TRACE(("%s: SIOCSIWTXPOW\n", dev->name));

	/* Make sure radio is off or on as far as software is concerned */
	disable = vwrq->disabled ? WL_RADIO_SW_DISABLE : 0;
	disable += WL_RADIO_SW_DISABLE << 16;

	if ((error = dev_wlc_ioctl(dev, WLC_SET_RADIO, &disable, sizeof(disable))))
		return error;

	/* Only handle mW */
	if (!(vwrq->flags & IW_TXPOW_MWATT))
		return -EINVAL;

	/* Value < 0 means just "on" or "off" */
	if (vwrq->value < 0)
		return 0;

	if (vwrq->value > 0xffff) txpwrmw = 0xffff;
	else txpwrmw = (uint16)vwrq->value;


	error = dev_wlc_intvar_set(dev, "qtxpower", (int)(bcm_mw_to_qdbm(txpwrmw)));
	return error;
}

static int
wl_iw_get_txpow(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, disable, txpwrdbm;
	uint8 result;

	WL_TRACE(("%s: SIOCGIWTXPOW\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_RADIO, &disable, sizeof(disable))) ||
	    (error = dev_wlc_intvar_get(dev, "qtxpower", &txpwrdbm)))
		return error;

	result = (uint8)(txpwrdbm & ~WL_TXPWR_OVERRIDE);
	vwrq->value = (int32)bcm_qdbm_to_mw(result);
	vwrq->fixed = 0;
	vwrq->disabled = (disable & (WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE)) ? 1 : 0;
	vwrq->flags = IW_TXPOW_MWATT;

	return 0;
}

#if WIRELESS_EXT > 10
static int
wl_iw_set_retry(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, lrl, srl;

	WL_TRACE(("%s: SIOCSIWRETRY\n", dev->name));

	/* Do not handle "off" or "lifetime" */
	if (vwrq->disabled || (vwrq->flags & IW_RETRY_LIFETIME))
		return -EINVAL;

	/* Handle "[min|max] limit" */
	if (vwrq->flags & IW_RETRY_LIMIT) {
		/* "max limit" or just "limit" */
		if ((vwrq->flags & IW_RETRY_MAX) || !(vwrq->flags & IW_RETRY_MIN)) {
			lrl = vwrq->value;
			if ((error = dev_wlc_ioctl(dev, WLC_SET_LRL, &lrl, sizeof(lrl))))
				return error;
		}
		/* "min limit" or just "limit" */
		if ((vwrq->flags & IW_RETRY_MIN) || !(vwrq->flags & IW_RETRY_MAX)) {
			srl = vwrq->value;
			if ((error = dev_wlc_ioctl(dev, WLC_SET_SRL, &srl, sizeof(srl))))
				return error;
		}
	}

	return 0;
}

static int
wl_iw_get_retry(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, lrl, srl;

	WL_TRACE(("%s: SIOCGIWRETRY\n", dev->name));

	vwrq->disabled = 0;      /* Can't be disabled */

	/* Do not handle lifetime queries */
	if ((vwrq->flags & IW_RETRY_TYPE) == IW_RETRY_LIFETIME)
		return -EINVAL;

	/* Get retry limits */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_LRL, &lrl, sizeof(lrl))) ||
	    (error = dev_wlc_ioctl(dev, WLC_GET_SRL, &srl, sizeof(srl))))
		return error;

	/* Note : by default, display the min retry number */
	if (vwrq->flags & IW_RETRY_MAX) {
		vwrq->flags = IW_RETRY_LIMIT | IW_RETRY_MAX;
		vwrq->value = lrl;
	} else {
		vwrq->flags = IW_RETRY_LIMIT;
		vwrq->value = srl;
		if (srl != lrl)
			vwrq->flags |= IW_RETRY_MIN;
	}

	return 0;
}
#endif /* WIRELESS_EXT > 10 */

static int
wl_iw_set_encode(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_wsec_key_t key;
	int error, val, wsec;

	WL_TRACE(("%s: SIOCSIWENCODE\n", dev->name));

	memset(&key, 0, sizeof(key));

	if ((dwrq->flags & IW_ENCODE_INDEX) == 0) {
		/* Find the current key */
		for (key.index = 0; key.index < DOT11_MAX_DEFAULT_KEYS; key.index++) {
			val = key.index;
			if ((error = dev_wlc_ioctl(dev, WLC_GET_KEY_PRIMARY, &val, sizeof(val))))
				return error;
			if (val)
				break;
		}
		/* Default to 0 */
		if (key.index == DOT11_MAX_DEFAULT_KEYS)
			key.index = 0;
	} else {
		key.index = (dwrq->flags & IW_ENCODE_INDEX) - 1;
		if (key.index >= DOT11_MAX_DEFAULT_KEYS)
			return -EINVAL;
	}

	/* Old API used to pass a NULL pointer instead of IW_ENCODE_NOKEY */
	if (!extra || !dwrq->length || (dwrq->flags & IW_ENCODE_NOKEY)) {
		/* Just select a new current key */
		val = key.index;
		if ((error = dev_wlc_ioctl(dev, WLC_SET_KEY_PRIMARY, &val, sizeof(val))))
			return error;
	} else {
		key.len = dwrq->length;

		if (dwrq->length > sizeof(key.data))
			return -EINVAL;

		memcpy(key.data, extra, dwrq->length);

		key.flags = WL_PRIMARY_KEY;
		switch (key.len) {
		case WEP1_KEY_SIZE:
			key.algo = CRYPTO_ALGO_WEP1;
			break;
		case WEP128_KEY_SIZE:
			key.algo = CRYPTO_ALGO_WEP128;
			break;
		case TKIP_KEY_SIZE:
			key.algo = CRYPTO_ALGO_TKIP;
			break;
		case AES_KEY_SIZE:
			key.algo = CRYPTO_ALGO_AES_CCM;
			break;
		default:
			return -EINVAL;
		}

		/* Set the new key/index */
		if ((error = dev_wlc_ioctl(dev, WLC_SET_KEY, &key, sizeof(key))))
			return error;
	}

	/* Interpret "off" to mean no encryption */
	val = (dwrq->flags & IW_ENCODE_DISABLED) ? 0 : WEP_ENABLED;

	if ((error = dev_wlc_intvar_get(dev, "wsec", &wsec)))
		return error;

	wsec  &= ~(WEP_ENABLED);
	wsec |= val;

	if ((error = dev_wlc_intvar_set(dev, "wsec", wsec)))
		return error;

	/* Interpret "restricted" to mean shared key authentication */
	val = (dwrq->flags & IW_ENCODE_RESTRICTED) ? 1 : 0;
	if ((error = dev_wlc_ioctl(dev, WLC_SET_AUTH, &val, sizeof(val))))
		return error;

	return 0;
}

static int
wl_iw_get_encode(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_wsec_key_t key;
	int error, val, wsec, auth;

	WL_TRACE(("%s: SIOCGIWENCODE\n", dev->name));

	if ((dwrq->flags & IW_ENCODE_INDEX) == 0) {
		/* Find the current key */
		for (key.index = 0; key.index < DOT11_MAX_DEFAULT_KEYS; key.index++) {
			val = key.index;
			if ((error = dev_wlc_ioctl(dev, WLC_GET_KEY_PRIMARY, &val, sizeof(val))))
				return error;
			if (val)
				break;
		}
	} else
		key.index = (dwrq->flags & IW_ENCODE_INDEX) - 1;

	if (key.index < 0 || key.index >= DOT11_MAX_DEFAULT_KEYS)
		key.index = 0;

	/* Get info */
	if ((error = dev_wlc_ioctl(dev, WLC_GET_KEY, &key, sizeof(key))) ||
	    (error = dev_wlc_ioctl(dev, WLC_GET_WSEC, &wsec, sizeof(wsec))) ||
	    (error = dev_wlc_ioctl(dev, WLC_GET_AUTH, &auth, sizeof(auth))))
		return error;

	/* Get key length */
	dwrq->length = MIN(IW_ENCODING_TOKEN_MAX, key.len);

	/* Get flags */
	dwrq->flags = key.index + 1;
	if (!(wsec & (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED))) {
		/* Interpret "off" to mean no encryption */
		dwrq->flags |= IW_ENCODE_DISABLED;
	}
	if (auth) {
		/* Interpret "restricted" to mean shared key authentication */
		dwrq->flags |= IW_ENCODE_RESTRICTED;
	}

	/* Get key */
	if (dwrq->length && extra)
		memcpy(extra, key.data, dwrq->length);

	return 0;
}

static int
wl_iw_set_power(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, pm;

	WL_TRACE(("%s: SIOCSIWPOWER\n", dev->name));

	pm = vwrq->disabled ? PM_OFF : PM_MAX;

	if ((error = dev_wlc_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm))))
		return error;

	return 0;
}

static int
wl_iw_get_power(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error, pm;

	WL_TRACE(("%s: SIOCGIWPOWER\n", dev->name));

	if ((error = dev_wlc_ioctl(dev, WLC_GET_PM, &pm, sizeof(pm))))
		return error;

	vwrq->disabled = pm ? 0 : 1;
	vwrq->flags = IW_POWER_ALL_R;

	return 0;
}

#if WIRELESS_EXT > 17
static int
wl_iw_set_wpaie (
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *iwp,
	char *extra
)
{
	WL_TRACE(("%s: SIOCSIWGENIE\n", dev->name));
	dev_wlc_bufvar_set(dev, "wpaie", extra, iwp->length);
	return 0;
}
static int
wl_iw_get_wpaie (
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *iwp,
	char *extra
)
{
	WL_TRACE(("%s: SIOCGIWGENIE\n", dev->name));
	iwp->length = 64;
	dev_wlc_bufvar_get(dev, "wpaie", extra, iwp->length);
	return 0;
}

static int
wl_iw_set_encodeext (
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_point *dwrq,
	char *extra
)
{
	wl_wsec_key_t key;
	int error;
	struct iw_encode_ext *iwe;

	WL_TRACE(("%s: SIOCSIWENCODEEXT\n", dev->name));

	memset(&key, 0, sizeof(key));
	iwe = (struct iw_encode_ext *)extra;

	/* disable encryption completely  */
	if (dwrq->flags & IW_ENCODE_DISABLED) {
		
	}

	/* get the key index */
	key.index = 0;
	if (dwrq->flags & IW_ENCODE_INDEX)
		key.index = (dwrq->flags & IW_ENCODE_INDEX) - 1;

	key.len = iwe->key_len;
	/* addr */
	bcopy((void *)&iwe->addr.sa_data, (char *)&key.ea, ETHER_ADDR_LEN);

	/* check for key index change */
	if (key.len == 0) {
		if (iwe->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
			WL_WSEC(("Changing the the primary Key to %d\n", key.index));
			/* change the key index .... */
			error = dev_wlc_ioctl(dev, WLC_SET_KEY_PRIMARY,
				&key.index, sizeof(key.index));
			if (error)
				return error;
		}
		/* key delete */
		else
			dev_wlc_ioctl(dev, WLC_SET_KEY, &key, sizeof(key));
	}
	else {
		if (iwe->key_len > sizeof(key.data))
			return -EINVAL;

		WL_WSEC(("Setting the key index %d\n", key.index));
		if (iwe->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
			WL_WSEC(("key is a Primary Key\n"));
			key.flags = WL_PRIMARY_KEY;
		}
		bcopy((void *)iwe->key, key.data, iwe->key_len);

		/* rx iv */
		if (iwe->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID) {
			uchar *ivptr;
			ivptr = (uchar *)iwe->rx_seq;
			key.rxiv.hi = (ivptr[5] << 24) | (ivptr[4] << 16) |
					(ivptr[3] << 8) | ivptr[2];
			key.rxiv.lo = (ivptr[1] << 8) | ivptr[0];
			key.iv_initialized = TRUE;
		}

		switch(iwe->alg) {
			case IW_ENCODE_ALG_NONE:
				key.algo = CRYPTO_ALGO_OFF;
				break;
			case IW_ENCODE_ALG_WEP:
				if (iwe->key_len == WEP1_KEY_SIZE)
					key.algo = CRYPTO_ALGO_WEP1;
				else 
					key.algo = CRYPTO_ALGO_WEP128;
				break;
			case IW_ENCODE_ALG_TKIP:
				key.algo = CRYPTO_ALGO_TKIP;
				break;
			case IW_ENCODE_ALG_CCMP:
				key.algo = CRYPTO_ALGO_AES_CCM;
				break;
			default:
				break;
		}
		error = dev_wlc_ioctl(dev, WLC_SET_KEY, &key, sizeof(key));
		if (error)
			return error;
	}
	return 0;
}

static int
wl_iw_get_encodeext (
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	WL_TRACE(("%s: SIOCGIWENCODEEXT\n", dev->name));
	return 0;
}

static int
wl_iw_set_wpaauth(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error = 0;
	int paramid;
	int paramval;
	int val = 0;


	paramid = vwrq->flags & IW_AUTH_INDEX;
	paramval = vwrq->value;

	switch(paramid) {
	case IW_AUTH_WPA_VERSION:
		/* supported wpa version disabled or wpa or wpa2 */
		if (paramval & IW_AUTH_WPA_VERSION_DISABLED)
			val = WPA_AUTH_DISABLED;
		else if (paramval & (IW_AUTH_WPA_VERSION_WPA))
			val = WPA_AUTH_PSK | WPA_AUTH_UNSPECIFIED;
#ifdef BCMWPA2
		else if (paramval & IW_AUTH_WPA_VERSION_WPA2)
			val = WPA2_AUTH_PSK | WPA2_AUTH_UNSPECIFIED;
#endif /* BCMWPA2 */
		if ((error = dev_wlc_intvar_set(dev, "wpa_auth", val)))
			return error;
		break;
	case IW_AUTH_CIPHER_PAIRWISE:
	case IW_AUTH_CIPHER_GROUP:
		/* none, wep40, tkip, ccmp, wep104 */
		/* wsec: none, wep40, tkip, aes, wep104 */
		if (paramval & (IW_AUTH_CIPHER_WEP40 | IW_AUTH_CIPHER_WEP104))
			val |= 	WEP_ENABLED;
		if (paramval & IW_AUTH_CIPHER_TKIP)
			val |= TKIP_ENABLED;
		if (paramval & IW_AUTH_CIPHER_CCMP)
			val |= AES_ENABLED;

		if (paramid == IW_AUTH_CIPHER_PAIRWISE) {
			if ((error = dev_wlc_intvar_set(dev, "wsec", val)))
				{
				return error;
				}
		}		
		else {
			if ((error = dev_wlc_intvar_set(dev, "wsec", val)))
				{
				return error;
				}

		}
		break;

	case IW_AUTH_KEY_MGMT:
		if ((error = dev_wlc_intvar_get(dev, "wpa_auth", &val)))
				{
				return error;
				}


		if (val & (WPA_AUTH_PSK | WPA_AUTH_UNSPECIFIED)) {	
			if (paramval & IW_AUTH_KEY_MGMT_PSK)
				val = WPA_AUTH_PSK ;
			else 
				val = WPA_AUTH_UNSPECIFIED;
		}
#ifdef BCMWPA2
		else if (val & (WPA2_AUTH_PSK | WPA2_AUTH_UNSPECIFIED)) {
			if (paramval & IW_AUTH_KEY_MGMT_PSK)
				val = WPA2_AUTH_PSK;
			else 
				val = WPA2_AUTH_UNSPECIFIED;
		}
#endif /* BCMWPA2 */
		if ((error = dev_wlc_intvar_set(dev, "wpa_auth", val)))
				{
				return error;
				}

		break;
	case IW_AUTH_TKIP_COUNTERMEASURES:
		dev_wlc_bufvar_set(dev, "tkip_countermeasures", (char *)&paramval, 1);
		break;
	case IW_AUTH_80211_AUTH_ALG:
		/* open shared */
		if (paramval & IW_AUTH_ALG_OPEN_SYSTEM)
			val = 0;
		else if (paramval & IW_AUTH_ALG_SHARED_KEY)
			val = 1;
		else 
			error = 1;
		if (!error && (error = dev_wlc_intvar_set(dev, "auth", val)))
				{
				return error;
				}

		break;
	case IW_AUTH_WPA_ENABLED:
		
		if (paramval == 0) {
			if ((error = dev_wlc_intvar_get(dev, "wsec", &val)))
				{
				return error;
				}

			if (val & (TKIP_ENABLED | AES_ENABLED)) {
				val &= ~(TKIP_ENABLED | AES_ENABLED);
		 		dev_wlc_intvar_set(dev, "wsec", val);
			}
			val = 0;
			if ((dev_wlc_intvar_set(dev, "wpa_auth", 0)))
				{
				return error;
				}
		}

		/* nothing really needs to be done if wpa_auth enabled */
		break;

	case IW_AUTH_DROP_UNENCRYPTED:
		dev_wlc_bufvar_set(dev, "wsec_restrict", (char *)&paramval, 1);
		break;

	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
		dev_wlc_bufvar_set(dev, "rx_unencrypted_eapol", (char *)&paramval, 1);
		break;

#if WIRELESS_EXT > 18
	case IW_AUTH_ROAMING_CONTROL:
		WL_ERROR(("%s: IW_AUTH_ROAMING_CONTROL\n", __FUNCTION__));
		/* driver control or user space app control */
		break;
	case IW_AUTH_PRIVACY_INVOKED:
		WL_ERROR(("%s: IW_AUTH_PRIVACY_INVOKED\n", __FUNCTION__));
		break;
#endif /* WIRELESS_EXT > 18 */
	default:
		break;
	}
	return 0;
}

static int
wl_iw_get_wpaauth(
	struct net_device *dev,
	struct iw_request_info *info,
	struct iw_param *vwrq,
	char *extra
)
{
	int error;
	int paramid;
	int paramval = 0;
	int val;

	WL_TRACE(("%s: SIOCGIWAUTH\n", dev->name));

	paramid = vwrq->flags & IW_AUTH_INDEX;

	switch(paramid) {
	case IW_AUTH_WPA_VERSION:
		/* supported wpa version disabled or wpa or wpa2 */
		if ((error = dev_wlc_intvar_get(dev, "wpa_auth", &val)))
			return error;
		if (val & (WPA_AUTH_NONE | WPA_AUTH_DISABLED))
			paramval = IW_AUTH_WPA_VERSION_DISABLED;
		else if (val & (WPA_AUTH_PSK | WPA_AUTH_UNSPECIFIED))
			paramval = IW_AUTH_WPA_VERSION_WPA;
#ifdef BCMWPA2
		else if (val & (WPA2_AUTH_PSK | WPA2_AUTH_UNSPECIFIED))
			paramval = IW_AUTH_WPA_VERSION_WPA2;
#endif /* BCMWPA2 */
		break;
	case IW_AUTH_CIPHER_PAIRWISE:
	case IW_AUTH_CIPHER_GROUP:
		if (paramid == IW_AUTH_CIPHER_PAIRWISE) {
			if ((error = dev_wlc_intvar_get(dev, "pwsec", &val)))
				return error;
		}
		else {
			if ((error = dev_wlc_intvar_get(dev, "gwsec", &val)))
				return error;
		}
		paramval = 0;
		if (val) {
			if (val & WEP_ENABLED)
				paramval |= (IW_AUTH_CIPHER_WEP40 | IW_AUTH_CIPHER_WEP104) ;
			if (val & TKIP_ENABLED)
				paramval |= (IW_AUTH_CIPHER_TKIP) ;
			if (val & AES_ENABLED)
				paramval |= (IW_AUTH_CIPHER_CCMP) ;
		}
		else 
			paramval = IW_AUTH_CIPHER_NONE;
		break;
	case IW_AUTH_KEY_MGMT:
		/* psk, 1x */
		if ((error = dev_wlc_intvar_get(dev, "wpa_auth", &val)))
			return error;
		if ( (val & WPA_AUTH_PSK)
#ifdef BCMWPA2
		|| (val & WPA2_AUTH_PSK)
#endif /* BCMWPA2 */
		)
			paramval = IW_AUTH_KEY_MGMT_PSK;
		else 
			paramval = IW_AUTH_KEY_MGMT_802_1X;
		
		break;
	case IW_AUTH_TKIP_COUNTERMEASURES:
		dev_wlc_bufvar_get(dev, "tkip_countermeasures", (char *)&paramval, 1);
		break;

	case IW_AUTH_DROP_UNENCRYPTED:
		dev_wlc_bufvar_get(dev, "wsec_restrict", (char *)&paramval, 1);
		break;

	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
		dev_wlc_bufvar_get(dev, "rx_unencrypted_eapol", (char *)&paramval, 1);
		break;

	case IW_AUTH_80211_AUTH_ALG:
		/* open, shared, leap */
		if ((error = dev_wlc_intvar_get(dev, "auth", &val)))
			return error;
		if (!val)
			paramval = IW_AUTH_ALG_OPEN_SYSTEM;
		else 
			paramval = IW_AUTH_ALG_SHARED_KEY;
		break;
	case IW_AUTH_WPA_ENABLED:
		if ((error = dev_wlc_intvar_get(dev, "wpa_auth", &val)))
			return error;
		if (val)
			paramval = TRUE;
		else 
			paramval = FALSE;
		break;
#if WIRELESS_EXT > 18
	case IW_AUTH_ROAMING_CONTROL:
		WL_ERROR(("%s: IW_AUTH_ROAMING_CONTROL\n", __FUNCTION__));
		/* driver control or user space app control */
		break;
	case IW_AUTH_PRIVACY_INVOKED:
		WL_ERROR(("%s: IW_AUTH_PRIVACY_INVOKED\n", __FUNCTION__));
		break;
#endif /* WIRELESS_EXT > 18 */
	}
	vwrq->value = paramval;
	return 0;
}
#endif

static const iw_handler wl_iw_handler[] =
{
	(iw_handler) wl_iw_config_commit,	/* SIOCSIWCOMMIT */
	(iw_handler) wl_iw_get_name,		/* SIOCGIWNAME */
	(iw_handler) NULL,			/* SIOCSIWNWID */
	(iw_handler) NULL,			/* SIOCGIWNWID */
	(iw_handler) wl_iw_set_freq,		/* SIOCSIWFREQ */
	(iw_handler) wl_iw_get_freq,		/* SIOCGIWFREQ */
	(iw_handler) wl_iw_set_mode,		/* SIOCSIWMODE */
	(iw_handler) wl_iw_get_mode,		/* SIOCGIWMODE */
	(iw_handler) NULL,			/* SIOCSIWSENS */
	(iw_handler) NULL,			/* SIOCGIWSENS */
	(iw_handler) NULL,			/* SIOCSIWRANGE */
	(iw_handler) wl_iw_get_range,		/* SIOCGIWRANGE */
	(iw_handler) NULL,			/* SIOCSIWPRIV */
	(iw_handler) NULL,			/* SIOCGIWPRIV */
	(iw_handler) NULL,			/* SIOCSIWSTATS */
	(iw_handler) NULL,			/* SIOCGIWSTATS */
	(iw_handler) wl_iw_set_spy,		/* SIOCSIWSPY */
	(iw_handler) wl_iw_get_spy,		/* SIOCGIWSPY */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) wl_iw_set_wap,		/* SIOCSIWAP */
	(iw_handler) wl_iw_get_wap,		/* SIOCGIWAP */
#if WIRELESS_EXT > 17
	(iw_handler) wl_iw_mlme,		/* SIOCSIWMLME */
#else
	(iw_handler) NULL,			/* -- hole -- */
#endif
	(iw_handler) wl_iw_get_aplist,		/* SIOCGIWAPLIST */
#if WIRELESS_EXT > 13
	(iw_handler) wl_iw_set_scan,		/* SIOCSIWSCAN */
	(iw_handler) wl_iw_get_scan,		/* SIOCGIWSCAN */
#else	/* WIRELESS_EXT > 13 */
	(iw_handler) NULL,			/* SIOCSIWSCAN */
	(iw_handler) NULL,			/* SIOCGIWSCAN */
#endif	/* WIRELESS_EXT > 13 */
	(iw_handler) wl_iw_set_essid,		/* SIOCSIWESSID */
	(iw_handler) wl_iw_get_essid,		/* SIOCGIWESSID */
	(iw_handler) wl_iw_set_nick,		/* SIOCSIWNICKN */
	(iw_handler) wl_iw_get_nick,		/* SIOCGIWNICKN */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) wl_iw_set_rate,		/* SIOCSIWRATE */
	(iw_handler) wl_iw_get_rate,		/* SIOCGIWRATE */
	(iw_handler) wl_iw_set_rts,		/* SIOCSIWRTS */
	(iw_handler) wl_iw_get_rts,		/* SIOCGIWRTS */
	(iw_handler) wl_iw_set_frag,		/* SIOCSIWFRAG */
	(iw_handler) wl_iw_get_frag,		/* SIOCGIWFRAG */
	(iw_handler) wl_iw_set_txpow,		/* SIOCSIWTXPOW */
	(iw_handler) wl_iw_get_txpow,		/* SIOCGIWTXPOW */
#if WIRELESS_EXT > 10
	(iw_handler) wl_iw_set_retry,		/* SIOCSIWRETRY */
	(iw_handler) wl_iw_get_retry,		/* SIOCGIWRETRY */
#endif /* WIRELESS_EXT > 10 */
	(iw_handler) wl_iw_set_encode,		/* SIOCSIWENCODE */
	(iw_handler) wl_iw_get_encode,		/* SIOCGIWENCODE */
	(iw_handler) wl_iw_set_power,		/* SIOCSIWPOWER */
	(iw_handler) wl_iw_get_power,		/* SIOCGIWPOWER */
#if WIRELESS_EXT > 17
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) NULL,			/* -- hole -- */
	(iw_handler) wl_iw_set_wpaie,		/* SIOCSIWGENIE */
	(iw_handler) wl_iw_get_wpaie,		/* SIOCGIWGENIE */
	(iw_handler) wl_iw_set_wpaauth,		/* SIOCSIWAUTH */
	(iw_handler) wl_iw_get_wpaauth,		/* SIOCGIWAUTH */
	(iw_handler) wl_iw_set_encodeext,	/* SIOCSIWENCODEEXT */
	(iw_handler) wl_iw_get_encodeext,	/* SIOCGIWENCODEEXT */
#ifdef BCMWPA2
	(iw_handler) NULL,			/* SIOCSIWPMKSA */
#endif
#endif
};

#if WIRELESS_EXT > 12
const struct iw_handler_def wl_iw_handler_def =
{
	num_standard: ARRAYSIZE(wl_iw_handler),
	num_private: 0,
	num_private_args: 0,
	standard: (iw_handler *) wl_iw_handler,
	private: NULL,
	private_args: NULL,
	};
#endif /* WIRELESS_EXT > 12 */

static int (*old_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);

int
wl_iw_ioctl(
	struct net_device *dev,
	struct ifreq *rq,
	int cmd
)
{
	struct iwreq *wrq = (struct iwreq *) rq;
	struct iw_request_info info;
	iw_handler handler;
	char *extra = NULL;
	int token_size = 1, max_tokens = 0, ret = 0;


	if (cmd < SIOCIWFIRST ||
	    (cmd - SIOCIWFIRST) >= ARRAYSIZE(wl_iw_handler) ||
	    !(handler = wl_iw_handler[cmd - SIOCIWFIRST]))
		{
		return old_ioctl(dev,rq,cmd);
		}

	switch (cmd) {

	case SIOCSIWESSID:
	case SIOCGIWESSID:
	case SIOCSIWNICKN:
	case SIOCGIWNICKN:
		max_tokens = IW_ESSID_MAX_SIZE + 1;
		break;

	case SIOCSIWENCODE:
	case SIOCGIWENCODE:
#if WIRELESS_EXT > 17
	case SIOCSIWENCODEEXT:
	case SIOCGIWENCODEEXT:
#endif
		max_tokens = IW_ENCODING_TOKEN_MAX;
		break;

	case SIOCGIWRANGE:
		max_tokens = sizeof(struct iw_range);
		break;

	case SIOCGIWAPLIST:
		token_size = sizeof(struct sockaddr) + sizeof(struct iw_quality);
		max_tokens = IW_MAX_AP;
		break;

#if WIRELESS_EXT > 13
	case SIOCGIWSCAN:
		max_tokens = IW_SCAN_MAX_DATA;
		break;
#endif /* WIRELESS_EXT > 13 */

	case SIOCSIWSPY:
		token_size = sizeof(struct sockaddr);
		max_tokens = IW_MAX_SPY;
		break;

	case SIOCGIWSPY:
		token_size = sizeof(struct sockaddr) + sizeof(struct iw_quality);
		max_tokens = IW_MAX_SPY;
		break;

	}

	if (max_tokens && wrq->u.data.pointer) {
		if (wrq->u.data.length > max_tokens)
			return -E2BIG;

		if (!(extra = kmalloc(max_tokens * token_size, GFP_KERNEL)))
			return -ENOMEM;

		if (copy_from_user(extra, wrq->u.data.pointer, wrq->u.data.length * token_size)) {
			kfree(extra);
			return -EFAULT;
		}
	}

	info.cmd = cmd;
	info.flags = 0;

	ret = handler(dev, &info, &wrq->u, extra);

	if (extra) {
		if (copy_to_user(wrq->u.data.pointer, extra, wrq->u.data.length * token_size)) {
			kfree(extra);
			return -EFAULT;
		}

		kfree(extra);
	}

	return ret;
}




#ifndef IW_CeSTOM_MAX
#define IW_CUSTOM_MAX 256 /* size of extra buffer used for translation of events */
#endif /* IW_CUSTOM_MAX */

void
wl_iw_event(struct net_device *dev, wlc_event_t *e)
{
#if WIRELESS_EXT > 13
	union iwreq_data wrqu;
	char extra[IW_CUSTOM_MAX + 1];
	int cmd;

	memset(&wrqu, 0, sizeof(wrqu));
	memset(extra, 0, sizeof(extra));

	if (e->addr) {
		memcpy(wrqu.addr.sa_data, e->addr, ETHER_ADDR_LEN);
		wrqu.addr.sa_family = ARPHRD_ETHER;
	}

	switch (e->event.event_type) {
	case WLC_E_TXFAIL:
		cmd = IWEVTXDROP;
		break;
#if WIRELESS_EXT > 14
	case WLC_E_JOIN:
	case WLC_E_ASSOC_IND:
	case WLC_E_REASSOC_IND:
		cmd = IWEVREGISTERED;
		break;
	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:
		cmd = SIOCGIWAP;
		wrqu.data.length = strlen(extra);
		bzero(wrqu.addr.sa_data, ETHER_ADDR_LEN);
		bzero(&extra, ETHER_ADDR_LEN);
		break;
	case WLC_E_LINK:
	case WLC_E_NDIS_LINK:
		cmd = SIOCGIWAP;
		wrqu.data.length = strlen(extra);
		if (!e->event.flags & WLC_EVENT_MSG_LINK) {
			bzero(wrqu.addr.sa_data, ETHER_ADDR_LEN);
			bzero(&extra, ETHER_ADDR_LEN);
		}
		break;
#endif /* WIRELESS_EXT > 14 */
#if WIRELESS_EXT > 17
	case WLC_E_MIC_ERROR: {
		struct	iw_michaelmicfailure  *micerrevt = (struct  iw_michaelmicfailure  *)&extra;
		cmd = IWEVMICHAELMICFAILURE;
		wrqu.data.length = sizeof(struct iw_michaelmicfailure);
		if (e->event.flags & WLC_EVENT_MSG_GROUP)
			micerrevt->flags |= IW_MICFAILURE_GROUP;
		else 
			micerrevt->flags |= IW_MICFAILURE_PAIRWISE;
		if (e->addr) {
			memcpy(micerrevt->src_addr.sa_data, e->addr, ETHER_ADDR_LEN);
			micerrevt->src_addr.sa_family = ARPHRD_ETHER;
		}
			
		break;
	}
#ifdef BCMWPA2
	case WLC_E_PMKID_CACHE: {
		struct iw_pmkid_cand *iwpmkidcand = (struct iw_pmkid_cand *)&extra;
		pmkid_cand_list_t *pmkcandlist;
		pmkid_cand_t	*pmkidcand;
		int count;

		cmd = IWEVPMKIDCAND;
		pmkcandlist = e->data; 
		count = pmkcandlist->npmkid_cand;
		wrqu.data.length = sizeof(struct iw_pmkid_cand);
		pmkidcand = pmkcandlist->pmkid_cand;
		while (count) {
			bzero(iwpmkidcand, sizeof (struct iw_pmkid_cand));
			if (pmkidcand->preauth)
				iwpmkidcand->flags |= IW_PMKID_CAND_PREAUTH;
			bcopy(&pmkidcand->BSSID, &iwpmkidcand->bssid.sa_data, ETHER_ADDR_LEN);
			wireless_send_event(dev, cmd, &wrqu, extra);
			pmkidcand++;
			count--;
		}
		return;
	}
#endif /* BCMWPA2 */
#endif

	default:
		/* Cannot translate event */
		return;
	}
		wireless_send_event(dev, cmd, &wrqu, extra);
#endif /* WIRELESS_EXT > 13 */
}



/* Quarter dBm units to mW
 * Table starts at QDBM_OFFSET, so the first entry is mW for qdBm=153
 * Table is offset so the last entry is largest mW value that fits in
 * a uint16.
 */

#define QDBM_OFFSET 153		/* Offset for first entry */
#define QDBM_TABLE_LEN 40	/* Table size */

/* Smallest mW value that will round up to the first table entry, QDBM_OFFSET.
 * Value is ( mW(QDBM_OFFSET - 1) + mW(QDBM_OFFSET) ) / 2
 */
#define QDBM_TABLE_LOW_BOUND 6493 /* Low bound */

/* Largest mW value that will round down to the last table entry,
 * QDBM_OFFSET + QDBM_TABLE_LEN-1.
 * Value is ( mW(QDBM_OFFSET + QDBM_TABLE_LEN - 1) + mW(QDBM_OFFSET + QDBM_TABLE_LEN) ) / 2.
 */
#define QDBM_TABLE_HIGH_BOUND 64938 /* High bound */

static const uint16 nqdBm_to_mW_map[QDBM_TABLE_LEN] = {
/* qdBm: 	+0 	+1 	+2 	+3 	+4 	+5 	+6 	+7 */
/* 153: */      6683,	7079,	7499,	7943,	8414,	8913,	9441,	10000,
/* 161: */      10593,	11220,	11885,	12589,	13335,	14125,	14962,	15849,
/* 169: */      16788,	17783,	18836,	19953,	21135,	22387,	23714,	25119,
/* 177: */      26607,	28184,	29854,	31623,	33497,	35481,	37584,	39811,
/* 185: */      42170,	44668,	47315,	50119,	53088,	56234,	59566,	63096
};

uint16
bcm_qdbm_to_mw(uint8 qdbm)
{
	uint factor = 1;
	int idx = qdbm - QDBM_OFFSET;

	if (idx > QDBM_TABLE_LEN) {
		/* clamp to max uint16 mW value */
		return 0xFFFF;
	}

	/* scale the qdBm index up to the range of the table 0-40
	 * where an offset of 40 qdBm equals a factor of 10 mW.
	 */
	while (idx < 0) {
		idx += 40;
		factor *= 10;
	}

	/* return the mW value scaled down to the correct factor of 10,
	 * adding in factor/2 to get proper rounding.
	 */
	return ((nqdBm_to_mW_map[idx] + factor/2) / factor);
}

uint8
bcm_mw_to_qdbm(uint16 mw)
{
	uint8 qdbm;
	int offset;
	uint mw_uint = mw;
	uint boundary;

	/* handle boundary case */
	if (mw_uint <= 1)
		return 0;

	offset = QDBM_OFFSET;

	/* move mw into the range of the table */
	while (mw_uint < QDBM_TABLE_LOW_BOUND) {
		mw_uint *= 10;
		offset -= 40;
	}

	for (qdbm = 0; qdbm < QDBM_TABLE_LEN-1; qdbm++) {
		boundary = nqdBm_to_mW_map[qdbm] + (nqdBm_to_mW_map[qdbm+1] -
		                                    nqdBm_to_mW_map[qdbm])/2;
		if (mw_uint < boundary) break;
	}

	qdbm += (uint8)offset;

	return (qdbm);
}



static struct iw_statistics wstats;

char buf[WLC_IOCTL_MAXLEN];

struct iw_statistics *wlcompat_get_wireless_stats(struct net_device *dev)
{
	wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	get_pktcnt_t pkt;
	unsigned int rssi, noise, ap;
	
	memset(&wstats, 0, sizeof(wstats));
	memset(&pkt, 0, sizeof(pkt));
	memset(buf, 0, sizeof(buf));
	bss_info->version = 0x2000;
	wl_ioctl(dev, WLC_GET_BSS_INFO, bss_info, WLC_IOCTL_MAXLEN);
	wl_ioctl(dev, WLC_GET_PKTCNTS, &pkt, sizeof(pkt));

	rssi = 0;
	if ((wl_ioctl(dev, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) {
		if (wl_ioctl(dev, WLC_GET_PHY_NOISE, &noise, sizeof(noise)) < 0)
			noise = 0;
	} else {
		// somehow the structure doesn't fit here
		rssi = buf[82];
		noise = buf[84];
	}
	rssi = (rssi == 0 ? 1 : rssi);
	wstats.qual.updated = 0x10;
	if (rssi <= 1) 
		wstats.qual.updated |= 0x20;
	if (noise <= 1)
		wstats.qual.updated |= 0x40;

	if ((wstats.qual.updated & 0x60) == 0x60)
		return NULL;

	wstats.qual.level = rssi;
	wstats.qual.noise = noise;
	wstats.discard.misc = pkt.rx_bad_pkt;
	wstats.discard.retries = pkt.tx_bad_pkt;

	return &wstats;
}
static struct net_device *dev;

static int __init wlcompat_init()
{
	int found = 0, i;
	char devname[4] = "eth1";

	while (!found && (dev = dev_get_by_name(devname))) {
		if ((dev->wireless_handlers == NULL) && ((wl_ioctl(dev, WLC_GET_MAGIC, &i, sizeof(i)) == 0) && i == WLC_IOCTL_MAGIC))
			found = 1;
		devname[3]++;
	}
	
	if (!found) {
		printk(KERN_EMERG "No Broadcom Wireless Device found\n");
		return -ENODEV;
	}
		

	old_ioctl = dev->do_ioctl;
//	dev->do_ioctl = new_ioctl;
	dev->do_ioctl = wl_iw_ioctl;
	dev->wireless_handlers = (struct iw_handler_def *)&wl_iw_handler_def;
	dev->get_wireless_stats = wlcompat_get_wireless_stats;

        printk(KERN_EMERG "Broadcom Wireless Extensions Driver for WEXT%d installed\n", WIRELESS_EXT);	
#ifdef DEBUG
	printk("broadcom driver private data: 0x%08x\n", dev->priv);
#endif
	return 0;
}

static void __exit wlcompat_exit()
{
	dev->get_wireless_stats = NULL;
	dev->wireless_handlers = NULL;
	dev->do_ioctl = old_ioctl;
	return;
}

EXPORT_NO_SYMBOLS;
MODULE_AUTHOR("dd-wrt.com");
MODULE_LICENSE("GPL");

module_param(random, int, 0);
module_init(wlcompat_init);
module_exit(wlcompat_exit);
