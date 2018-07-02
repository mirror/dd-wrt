/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_cfg80211.c,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#include <linux/vmalloc.h>
#include <net/rtnetlink.h>

#include <bcmutils.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <dhd_cfg80211.h>

#ifdef PKT_FILTER_SUPPORT
#include <dngl_stats.h>
#include <dhd.h>
#endif

extern struct bcm_cfg80211 *g_bcm_cfg;

#ifdef PKT_FILTER_SUPPORT
extern uint dhd_pkt_filter_enable;
extern uint dhd_master_mode;
extern void dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode);
#endif

static int dhd_dongle_up = FALSE;

#if defined(BCMDONGLEHOST)
#include <dngl_stats.h>
#include <dhd.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <brcm_nl80211.h>
#include <dhd_cfg80211.h>
#endif /* defined(BCMDONGLEHOST) */

static s32 wl_dongle_up(struct net_device *ndev);
static s32 wl_dongle_down(struct net_device *ndev);
#ifndef OEM_ANDROID
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode);
static s32 wl_dongle_roam(struct net_device *ndev, u32 roamvar,	u32 bcn_timeout);
static s32 wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time, s32 scan_unassoc_time);
static s32 wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol);
static s32 wl_pattern_atoh(s8 *src, s8 *dst);
static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode);
#endif /* OEM_ANDROID */

/**
 * Function implementations
 */

s32 dhd_cfg80211_init(struct bcm_cfg80211 *cfg)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_deinit(struct bcm_cfg80211 *cfg)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_down(struct bcm_cfg80211 *cfg)
{
	struct net_device *ndev;
	s32 err = 0;

	WL_TRACE(("In\n"));
	if (!dhd_dongle_up) {
		WL_ERR(("Dongle is already down\n"));
		return err;
	}

	ndev = bcmcfg_to_prmry_ndev(cfg);
	wl_dongle_down(ndev);
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_set_p2p_info(struct bcm_cfg80211 *cfg, int val)
{
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
	dhd->op_mode |= val;
	WL_ERR(("Set : op_mode=0x%04x\n", dhd->op_mode));
#ifdef ARP_OFFLOAD_SUPPORT
	if (dhd->arp_version == 1) {
		/* IF P2P is enabled, disable arpoe */
		dhd_arp_offload_set(dhd, 0);
		dhd_arp_offload_enable(dhd, false);
	}
#endif /* ARP_OFFLOAD_SUPPORT */

	return 0;
}

s32 dhd_cfg80211_clean_p2p_info(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
	dhd->op_mode &= ~(DHD_FLAG_P2P_GC_MODE | DHD_FLAG_P2P_GO_MODE);
	WL_ERR(("Clean : op_mode=0x%04x\n", dhd->op_mode));

#ifdef ARP_OFFLOAD_SUPPORT
	if (dhd->arp_version == 1) {
		/* IF P2P is disabled, enable arpoe back for STA mode. */
		dhd_arp_offload_set(dhd, dhd_arp_mode);
		dhd_arp_offload_enable(dhd, true);
	}
#endif /* ARP_OFFLOAD_SUPPORT */

	return 0;
}

struct net_device* wl_cfg80211_allocate_if(struct bcm_cfg80211 *cfg, int ifidx, char *name,
	uint8 *mac, uint8 bssidx, char *dngl_name)
{
	return dhd_allocate_if(cfg->pub, ifidx, name, mac, bssidx, FALSE, dngl_name);
}

int wl_cfg80211_register_if(struct bcm_cfg80211 *cfg, int ifidx, struct net_device* ndev)
{
	return dhd_register_if(cfg->pub, ifidx, FALSE);
}

int wl_cfg80211_remove_if(struct bcm_cfg80211 *cfg, int ifidx, struct net_device* ndev)
{
	return dhd_remove_if(cfg->pub, ifidx, FALSE);
}

struct net_device * dhd_cfg80211_netdev_free(struct net_device *ndev)
{
	if (ndev) {
		if (ndev->ieee80211_ptr) {
			kfree(ndev->ieee80211_ptr);
			ndev->ieee80211_ptr = NULL;
		}
		free_netdev(ndev);
		return NULL;
	}

	return ndev;
}

void dhd_netdev_free(struct net_device *ndev)
{
#ifdef WL_CFG80211
	ndev = dhd_cfg80211_netdev_free(ndev);
#endif
	if (ndev)
		free_netdev(ndev);
}

static s32
wl_dongle_up(struct net_device *ndev)
{
	s32 err = 0;
	u32 up = 0;

	err = wldev_ioctl(ndev, WLC_UP, &up, sizeof(up), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_UP error (%d)\n", err));
	}
	return err;
}

static s32
wl_dongle_down(struct net_device *ndev)
{
	s32 err = 0;
	u32 down = 0;

	err = wldev_ioctl(ndev, WLC_DOWN, &down, sizeof(down), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_DOWN error (%d)\n", err));
	}
	return err;
}

#ifndef OEM_ANDROID
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode)
{
	s32 err = 0;

	WL_TRACE(("In\n"));
	err = wldev_ioctl(ndev, WLC_SET_PM, &power_mode, sizeof(power_mode), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_PM error (%d)\n", err));
	}
	return err;
}


static s32
wl_dongle_roam(struct net_device *ndev, u32 roamvar, u32 bcn_timeout)
{
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	s32 err = 0;

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	if (roamvar) {
		bcm_mkiovar("bcn_timeout", (char *)&bcn_timeout, 4, iovbuf,
			sizeof(iovbuf));
		err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
		if (unlikely(err)) {
			WL_ERR(("bcn_timeout error (%d)\n", err));
			goto dongle_rom_out;
		}
	}
	/* Enable/Disable built-in roaming to allow supplicant to take care of roaming */
	bcm_mkiovar("roam_off", (char *)&roamvar, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(err)) {
		WL_ERR(("roam_off error (%d)\n", err));
		goto dongle_rom_out;
	}
dongle_rom_out:
	return err;
}

static s32
wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time,
	s32 scan_unassoc_time)
{
	s32 err = 0;

	err = wldev_ioctl(ndev, WLC_SET_SCAN_CHANNEL_TIME, &scan_assoc_time,
		sizeof(scan_assoc_time), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("Scan assoc time is not supported\n"));
		} else {
			WL_ERR(("Scan assoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}
	err = wldev_ioctl(ndev, WLC_SET_SCAN_UNASSOC_TIME, &scan_unassoc_time,
		sizeof(scan_unassoc_time), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("Scan unassoc time is not supported\n"));
		} else {
			WL_ERR(("Scan unassoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}

dongle_scantime_out:
	return err;
}

static s32
wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol)
{
	/* Room for "event_msgs" + '\0' + bitvec */
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	s32 err = 0;

	/* Set ARP offload */
	bcm_mkiovar("arpoe", (char *)&arpoe, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFORM(("arpoe is not supported\n"));
		else
			WL_ERR(("arpoe error (%d)\n", err));

		goto dongle_offload_out;
	}
	bcm_mkiovar("arp_ol", (char *)&arp_ol, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFORM(("arp_ol is not supported\n"));
		else
			WL_ERR(("arp_ol error (%d)\n", err));

		goto dongle_offload_out;
	}

dongle_offload_out:
	return err;
}

static s32 wl_pattern_atoh(s8 *src, s8 *dst)
{
	int i;
	if (strncmp(src, "0x", 2) != 0 && strncmp(src, "0X", 2) != 0) {
		WL_ERR(("Mask invalid format. Needs to start with 0x\n"));
		return -1;
	}
	src = src + 2;		/* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		WL_ERR(("Mask invalid format. Needs to be of even length\n"));
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (u8) simple_strtoul(num, NULL, 16);
		src += 2;
	}
	return i;
}

static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode)
{
	/* Room for "event_msgs" + '\0' + bitvec */
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	const s8 *str;
	struct wl_pkt_filter pkt_filter;
	struct wl_pkt_filter *pkt_filterp;
	s32 buf_len;
	s32 str_len;
	u32 mask_size;
	u32 pattern_size;
	s8 buf[64] = {0};
	s32 err = 0;

	/* add a default packet filter pattern */
	str = "pkt_filter_add";
	str_len = strlen(str);
	strncpy(buf, str, sizeof(buf) - 1);
	buf[ sizeof(buf) - 1 ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (struct wl_pkt_filter *)(buf + str_len + 1);

	/* Parse packet filter id. */
	pkt_filter.id = htod32(100);

	/* Parse filter polarity. */
	pkt_filter.negate_match = htod32(0);

	/* Parse filter type. */
	pkt_filter.type = htod32(0);

	/* Parse pattern filter offset. */
	pkt_filter.u.pattern.offset = htod32(0);

	/* Parse pattern filter mask. */
	mask_size = htod32(wl_pattern_atoh("0xff",
		(char *)pkt_filterp->u.pattern.
		    mask_and_pattern));

	/* Parse pattern filter pattern. */
	pattern_size = htod32(wl_pattern_atoh("0x00",
		(char *)&pkt_filterp->u.pattern.mask_and_pattern[mask_size]));

	if (mask_size != pattern_size) {
		WL_ERR(("Mask and pattern not the same size\n"));
		err = -EINVAL;
		goto dongle_filter_out;
	}

	pkt_filter.u.pattern.size_bytes = mask_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	/* Keep-alive attributes are set in local
	 * variable (keep_alive_pkt), and
	 * then memcpy'ed into buffer (keep_alive_pktp) since there is no
	 * guarantee that the buffer is properly aligned.
	 */
	memcpy((char *)pkt_filterp, &pkt_filter,
		WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	err = wldev_ioctl(ndev, WLC_SET_VAR, buf, buf_len, true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("filter not supported\n"));
		} else {
			WL_ERR(("filter (%d)\n", err));
		}
		goto dongle_filter_out;
	}

	/* set mode to allow pattern */
	bcm_mkiovar("pkt_filter_mode", (char *)&filter_mode, 4, iovbuf,
		sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("filter_mode not supported\n"));
		} else {
			WL_ERR(("filter_mode (%d)\n", err));
		}
		goto dongle_filter_out;
	}

dongle_filter_out:
	return err;
}
#endif /* OEM_ANDROID */

s32 dhd_config_dongle(struct bcm_cfg80211 *cfg)
{
#ifndef DHD_SDALIGN
#define DHD_SDALIGN	32
#endif
	struct net_device *ndev;
	s32 err = 0;

	WL_TRACE(("In\n"));
	if (dhd_dongle_up) {
		WL_ERR(("Dongle is already up\n"));
		return err;
	}

	ndev = bcmcfg_to_prmry_ndev(cfg);

	err = wl_dongle_up(ndev);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_up failed\n"));
		goto default_conf_out;
	}
#ifndef OEM_ANDROID
	err = wl_dongle_power(ndev, PM_FAST);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_power failed\n"));
		goto default_conf_out;
	}
	err = wl_dongle_roam(ndev, (cfg->roam_on ? 0 : 1), 3);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_roam failed\n"));
		goto default_conf_out;
	}
	wl_dongle_scantime(ndev, 40, 80);
	wl_dongle_offload(ndev, 1, 0xf);
	wl_dongle_filter(ndev, 1);
#endif /* OEM_ANDROID */
	dhd_dongle_up = true;

default_conf_out:

	return err;

}

int dhd_cfgvendor_priv_string_handler(struct bcm_cfg80211 *cfg, struct wireless_dev *wdev,
	const struct bcm_nlmsg_hdr *nlioc, void *buf)
{
	struct net_device *ndev = NULL;
	dhd_pub_t *dhd;
	dhd_ioctl_t ioc = { 0 };
	int ret = 0;
	int8 index;

	WL_TRACE(("entry: cmd = %d\n", nlioc->cmd));

	dhd = cfg->pub;
	DHD_OS_WAKE_LOCK(dhd);

#if defined(OEM_ANDROID)
	/* send to dongle only if we are not waiting for reload already */
	if (dhd->hang_was_sent) {
		WL_ERR(("HANG was sent up earlier\n"));
		DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(dhd, DHD_EVENT_TIMEOUT_MS);
		DHD_OS_WAKE_UNLOCK(dhd);
		return OSL_ERROR(BCME_DONGLE_DOWN);
	}
#endif /* (OEM_ANDROID) */

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	index = dhd_net2idx(dhd->info, ndev);
	if (index == DHD_BAD_IF) {
		WL_ERR(("Bad ifidx from wdev:%p\n", wdev));
		ret = BCME_ERROR;
		goto done;
	}

	ioc.cmd = nlioc->cmd;
	ioc.len = nlioc->len;
	ioc.set = nlioc->set;
	ioc.driver = nlioc->magic;
	ret = dhd_ioctl_process(dhd, index, &ioc, buf);
	if (ret) {
		WL_TRACE(("dhd_ioctl_process return err %d\n", ret));
		ret = OSL_ERROR(ret);
		goto done;
	}

done:
	DHD_OS_WAKE_UNLOCK(dhd);
	return ret;
}
