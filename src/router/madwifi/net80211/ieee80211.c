/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211.c 1520 2006-04-21 16:57:59Z dyqith $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 generic handler
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>		/* XXX for rtnl_lock */

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/if_athproto.h>

const char *ieee80211_phymode_name[] = {
	"auto",		/* IEEE80211_MODE_AUTO */
	"11a",		/* IEEE80211_MODE_11A */
	"11b",		/* IEEE80211_MODE_11B */
	"11g",		/* IEEE80211_MODE_11G */
	"FH",		/* IEEE80211_MODE_FH */
	"turboA",	/* IEEE80211_MODE_TURBO_A */
	"turboG",	/* IEEE80211_MODE_TURBO_G */
	"staticTurboA",	/* IEEE80211_MODE_TURBO_G */
};
EXPORT_SYMBOL(ieee80211_phymode_name);

static void ieee80211com_media_status(struct net_device*, struct ifmediareq *);
static int ieee80211com_media_change(struct net_device *);
static struct net_device_stats *ieee80211_getstats(struct net_device *);
static int ieee80211_change_mtu(struct net_device *, int);
static void ieee80211_set_multicast_list(struct net_device *);

MALLOC_DEFINE(M_80211_VAP, "80211vap", "802.11 vap state");

/*
 * Country Code Table for code-to-string conversion.
 */
struct country_code_to_string{
	u_int16_t iso_code;	   
	const char *iso_name;
};

/*
 * XXX: ugly, since must match define in other modules.
 */
#define CTRY_DEBUG                0x1ff   /* debug */
#define CTRY_DEFAULT              0       /* default */

static const  struct country_code_to_string country_strings[] = {
    {CTRY_DEBUG,	 	"DB" },
    {CTRY_DEFAULT,	 	"NA" },
    {CTRY_ALBANIA,		"AL" },
    {CTRY_ALGERIA,		"DZ" },
    {CTRY_ARGENTINA,		"AR" },
    {CTRY_ARMENIA,		"AM" },
    {CTRY_AUSTRALIA,		"AU" },
    {CTRY_AUSTRIA,		"AT" },
    {CTRY_AZERBAIJAN,		"AZ" },
    {CTRY_BAHRAIN,		"BH" },
    {CTRY_BELARUS,		"BY" },
    {CTRY_BELGIUM,		"BE" },
    {CTRY_BELIZE,		"BZ" },
    {CTRY_BOLIVIA,		"BO" },
    {CTRY_BRAZIL,		"BR" },
    {CTRY_BRUNEI_DARUSSALAM,	"BN" },
    {CTRY_BULGARIA,		"BG" },
    {CTRY_CANADA,		"CA" },
    {CTRY_CHILE,		"CL" },
    {CTRY_CHINA,		"CN" },
    {CTRY_COLOMBIA,		"CO" },
    {CTRY_COSTA_RICA,		"CR" },
    {CTRY_CROATIA,		"HR" },
    {CTRY_CYPRUS,		"CY" },
    {CTRY_CZECH,		"CZ" },
    {CTRY_DENMARK,		"DK" },
    {CTRY_DOMINICAN_REPUBLIC,	"DO" },
    {CTRY_ECUADOR,		"EC" },
    {CTRY_EGYPT,		"EG" },
    {CTRY_EL_SALVADOR,		"SV" },    
    {CTRY_ESTONIA,		"EE" },
    {CTRY_FINLAND,		"FI" },
    {CTRY_FRANCE,		"FR" },
    {CTRY_FRANCE2,		"F2" },
    {CTRY_GEORGIA,		"GE" },
    {CTRY_GERMANY,		"DE" },
    {CTRY_GREECE,		"GR" },
    {CTRY_GUATEMALA,		"GT" },
    {CTRY_HONDURAS,		"HN" },
    {CTRY_HONG_KONG,		"HK" },
    {CTRY_HUNGARY,		"HU" },
    {CTRY_ICELAND,		"IS" },
    {CTRY_INDIA,		"IN" },
    {CTRY_INDONESIA,		"ID" },
    {CTRY_IRAN,			"IR" },
    {CTRY_IRELAND,		"IE" },
    {CTRY_ISRAEL,		"IL" },
    {CTRY_ITALY,		"IT" },
    {CTRY_JAPAN,		"JP" },
    {CTRY_JAPAN1,		"J1" },
    {CTRY_JAPAN2,		"J2" },    
    {CTRY_JAPAN3,		"J3" },
    {CTRY_JAPAN4,		"J4" },
    {CTRY_JAPAN5,		"J5" },    
    {CTRY_JAPAN7,		"JP" },
    {CTRY_JAPAN6,		"JP" },
    {CTRY_JAPAN8,		"JP" },
    {CTRY_JAPAN9,	      	"JP" },
    {CTRY_JAPAN10,	      	"JP" }, 
    {CTRY_JAPAN11,	      	"JP" },
    {CTRY_JAPAN12,	      	"JP" },
    {CTRY_JAPAN13,	      	"JP" },
    {CTRY_JAPAN14,	      	"JP" },
    {CTRY_JAPAN15,	      	"JP" },
    {CTRY_JAPAN16,	      	"JP" }, 
    {CTRY_JAPAN17,	      	"JP" },
    {CTRY_JAPAN18,	      	"JP" },
    {CTRY_JAPAN19,	      	"JP" },
    {CTRY_JAPAN20,	      	"JP" },
    {CTRY_JAPAN21,	      	"JP" }, 
    {CTRY_JAPAN22,	      	"JP" },
    {CTRY_JAPAN23,	      	"JP" },
    {CTRY_JAPAN24,	      	"JP" },
    {CTRY_JAPAN25,	      	"JP" }, 
    {CTRY_JAPAN26,	      	"JP" },
    {CTRY_JAPAN27,	      	"JP" },
    {CTRY_JAPAN28,	      	"JP" },
    {CTRY_JAPAN29,	      	"JP" },
    {CTRY_JAPAN30,      	"JP" },
    {CTRY_JAPAN31,      	"JP" },
    {CTRY_JAPAN32,      	"JP" },
    {CTRY_JAPAN33,      	"JP" },
    {CTRY_JAPAN34,      	"JP" },
    {CTRY_JAPAN35,      	"JP" },
    {CTRY_JAPAN36,      	"JP" },
    {CTRY_JAPAN37,      	"JP" },
    {CTRY_JAPAN38,      	"JP" },
    {CTRY_JAPAN39,      	"JP" },
    {CTRY_JAPAN40,      	"JP" },
    {CTRY_JAPAN41,      	"JP" },
    {CTRY_JAPAN42,      	"JP" },
    {CTRY_JAPAN43,      	"JP" },
    {CTRY_JAPAN44,      	"JP" },
    {CTRY_JAPAN45,      	"JP" },
    {CTRY_JAPAN46,      	"JP" },
    {CTRY_JAPAN47,      	"JP" },
    {CTRY_JAPAN48,      	"JP" },
    {CTRY_JORDAN,		"JO" },
    {CTRY_KAZAKHSTAN,		"KZ" },
    {CTRY_KOREA_NORTH,		"KP" },
    {CTRY_KOREA_ROC,		"KR" },
    {CTRY_KOREA_ROC2,		"K2" },
    {CTRY_KUWAIT,		"KW" },
    {CTRY_LATVIA,		"LV" },
    {CTRY_LEBANON,		"LB" },
    {CTRY_LIECHTENSTEIN,	"LI" },
    {CTRY_LITHUANIA,		"LT" },
    {CTRY_LUXEMBOURG,		"LU" },
    {CTRY_MACAU,		"MO" },
    {CTRY_MACEDONIA,		"MK" },
    {CTRY_MALAYSIA,		"MY" },
    {CTRY_MEXICO,		"MX" },
    {CTRY_MONACO,		"MC" },
    {CTRY_MOROCCO,		"MA" },
    {CTRY_NETHERLANDS,		"NL" },
    {CTRY_NEW_ZEALAND,		"NZ" },
    {CTRY_NORWAY,		"NO" },
    {CTRY_OMAN,			"OM" },
    {CTRY_PAKISTAN,		"PK" },
    {CTRY_PANAMA,		"PA" },
    {CTRY_PERU,			"PE" },
    {CTRY_PHILIPPINES,		"PH" },
    {CTRY_POLAND,		"PL" },
    {CTRY_PORTUGAL,		"PT" },
    {CTRY_PUERTO_RICO,		"PR" },
    {CTRY_QATAR,		"QA" },
    {CTRY_ROMANIA,		"RO" },
    {CTRY_RUSSIA,		"RU" },
    {CTRY_SAUDI_ARABIA,		"SA" },
    {CTRY_SINGAPORE,		"SG" },
    {CTRY_SLOVAKIA,		"SK" },
    {CTRY_SLOVENIA,		"SI" },
    {CTRY_SOUTH_AFRICA,		"ZA" },
    {CTRY_SPAIN,		"ES" },
    {CTRY_SWEDEN,		"SE" },
    {CTRY_SWITZERLAND,		"CH" },
    {CTRY_SYRIA,		"SY" },
    {CTRY_TAIWAN,		"TW" },
    {CTRY_THAILAND,		"TH" },
    {CTRY_TRINIDAD_Y_TOBAGO,	"TT" },
    {CTRY_TUNISIA,		"TN" },
    {CTRY_TURKEY,		"TR" },
    {CTRY_UKRAINE,		"UA" },
    {CTRY_UAE,			"AE" },
    {CTRY_UNITED_KINGDOM,	"GB" },
    {CTRY_UNITED_STATES,	"US" },
    {CTRY_UNITED_STATES_FCC49,	"US" },
    {CTRY_URUGUAY,		"UY" },
    {CTRY_UZBEKISTAN,		"UZ" },    
    {CTRY_VENEZUELA,		"VE" },
    {CTRY_VIET_NAM,		"VN" },
    {CTRY_YEMEN,		"YE" },
    {CTRY_ZIMBABWE,		"ZW" }    
};

int
ieee80211_ifattach(struct ieee80211com *ic)
{
	struct net_device *dev = ic->ic_dev;
	struct ieee80211_channel *c;
	struct ifmediareq imr;
	int i;

	_MOD_INC_USE(THIS_MODULE, return -ENODEV);

	/*
	 * Pick an initial operating mode until we have a vap
	 * created to lock it down correctly.  This is only
	 * drivers have something defined for configuring the
	 * hardware at startup.
	 */
	ic->ic_opmode = IEEE80211_M_STA;	/* everyone supports this */

	/*
	 * Fill in 802.11 available channel set, mark
	 * all available channels as active, and pick
	 * a default channel if not already specified.
	 */
	KASSERT(0 < ic->ic_nchans && ic->ic_nchans < IEEE80211_CHAN_MAX,
		("invalid number of channels specified: %u", ic->ic_nchans));
	memset(ic->ic_chan_avail, 0, sizeof(ic->ic_chan_avail));
	ic->ic_modecaps |= 1<<IEEE80211_MODE_AUTO;
	for (i = 0; i < ic->ic_nchans; i++) {
		c = &ic->ic_channels[i];
		KASSERT(c->ic_flags != 0, ("channel with no flags"));
		KASSERT(c->ic_ieee < IEEE80211_CHAN_MAX,
			("channel with bogus ieee number %u", c->ic_ieee));
		setbit(ic->ic_chan_avail, c->ic_ieee);
		/*
		 * Identify mode capabilities.
		 */
		if (IEEE80211_IS_CHAN_A(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_11A;
		if (IEEE80211_IS_CHAN_B(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_11B;
		if (IEEE80211_IS_CHAN_PUREG(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_11G;
		if (IEEE80211_IS_CHAN_FHSS(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_FH;
		if (IEEE80211_IS_CHAN_108A(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_TURBO_A;
		if (IEEE80211_IS_CHAN_108G(c))
			ic->ic_modecaps |= 1<<IEEE80211_MODE_TURBO_G;
	}
	/* initialize candidate channels to all available */
	memcpy(ic->ic_chan_active, ic->ic_chan_avail,
		sizeof(ic->ic_chan_avail));
	/* validate ic->ic_curmode */
	if ((ic->ic_modecaps & (1<<ic->ic_curmode)) == 0)
		ic->ic_curmode = IEEE80211_MODE_AUTO;
	/*
	 * When 11g is supported, force the rate set to
	 * include basic rates suitable for a mixed b/g bss.
	 */
	if (ic->ic_modecaps & (1<<IEEE80211_MODE_11G))
		ieee80211_set11gbasicrates(
			&ic->ic_sup_rates[IEEE80211_MODE_11G],
			IEEE80211_MODE_11G);

	/* setup initial channel settings */
	ic->ic_bsschan = IEEE80211_CHAN_ANYC;
	/* arbitrarily pick the first channel */
	ic->ic_curchan = &ic->ic_channels[0];

	/* Enable marking of dfs by default */
	ic->ic_flags_ext |= IEEE80211_FEXT_MARKDFS;

	/*
	 * Enable WME by default if we're capable.
	 */
	if (ic->ic_caps & IEEE80211_C_WME)
		ic->ic_flags |= IEEE80211_F_WME;
	(void) ieee80211_setmode(ic, ic->ic_curmode);

	if (ic->ic_lintval == 0)
		ic->ic_lintval = IEEE80211_BINTVAL_DEFAULT;
	ic->ic_bmisstimeout = 7 * ic->ic_lintval;	/* default 7 beacons */
	IEEE80211_LOCK_INIT(ic, "ieee80211com");
	IEEE80211_VAPS_LOCK_INIT(ic, "ieee80211com_vaps");
	TAILQ_INIT(&ic->ic_vaps);

	ic->ic_txpowlimit = IEEE80211_TXPOWER_MAX;
	ic->ic_txpowlimit = IEEE80211_TXPOWER_MIN;
	ic->ic_newtxpowlimit = IEEE80211_TXPOWER_MAX;

	ieee80211_crypto_attach(ic);
	ieee80211_node_attach(ic);
	ieee80211_power_attach(ic);
	ieee80211_proto_attach(ic);
	ieee80211_scan_attach(ic);

	ieee80211_media_setup(ic, &ic->ic_media, ic->ic_caps,
		ieee80211com_media_change, ieee80211com_media_status);
	ieee80211com_media_status(dev, &imr);
	ifmedia_set(&ic->ic_media, imr.ifm_active);

	return 0;
}
EXPORT_SYMBOL(ieee80211_ifattach);

void
ieee80211_ifdetach(struct ieee80211com *ic)
{
	struct ieee80211vap *vap;

	rtnl_lock();
	while ((vap = TAILQ_FIRST(&ic->ic_vaps)) != NULL)
		ic->ic_vap_delete(vap);
	rtnl_unlock();

	ieee80211_scan_detach(ic);
	ieee80211_proto_detach(ic);
	ieee80211_crypto_detach(ic);
	ieee80211_power_detach(ic);
	ieee80211_node_detach(ic);
	ifmedia_removeall(&ic->ic_media);

	IEEE80211_VAPS_LOCK_DESTROY(ic);
	IEEE80211_LOCK_DESTROY(ic);

	_MOD_DEC_USE(THIS_MODULE);
}
EXPORT_SYMBOL(ieee80211_ifdetach);

int
ieee80211_vap_setup(struct ieee80211com *ic, struct net_device *dev,
	const char *name, int unit, int opmode, int flags)
{
#define	IEEE80211_C_OPMODE \
	(IEEE80211_C_IBSS | IEEE80211_C_HOSTAP | IEEE80211_C_AHDEMO | \
	 IEEE80211_C_MONITOR)
	struct ieee80211vap *vap = dev->priv;
	struct net_device *parent = ic->ic_dev;
	int err;
	
	if (name != NULL) {	/* XXX */
		if (strchr(name, '%')) {
			if ((err = dev_alloc_name(dev, name)) < 0) {
				printk(KERN_ERR "can't alloc name %s\n", name);
				return err;
			}
		} else
			strncpy(dev->name, name, sizeof(dev->name));
	}
	dev->get_stats = ieee80211_getstats;
	dev->open = ieee80211_open;
	dev->stop = ieee80211_stop;
	dev->hard_start_xmit = ieee80211_hardstart;
	dev->set_multicast_list = ieee80211_set_multicast_list;
#if 0
	dev->set_mac_address = ieee80211_set_mac_address;
#endif
 	dev->change_mtu = ieee80211_change_mtu;
	dev->tx_queue_len = 0;			/* NB: bypass queueing */
	dev->hard_header_len = parent->hard_header_len;
	/*
	 * The caller is assumed to allocate the device with
	 * alloc_etherdev or similar so we arrange for the
	 * space to be reclaimed accordingly.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	/* in 2.4 things are done differently... */
	dev->features |= NETIF_F_DYNALLOC;
#else
	dev->destructor = free_netdev;
#endif

	vap->iv_ic = ic;
	vap->iv_dev = dev;			/* back pointer */
#ifdef ATH_SUPERG_XR
	/*
	 * setup XR vap specific flags.
	 * link the XR vap to its normal val.
	 */
	if (flags & IEEE80211_VAP_XR) {
		struct ieee80211vap *vapparent = NULL;
		vap->iv_unit = -1;
		vap->iv_flags = ic->ic_flags | IEEE80211_F_XR;	/* propagate common flags and add XR flag */
		vap->iv_flags_ext = ic->ic_flags_ext;
		TAILQ_FOREACH(vapparent, &ic->ic_vaps, iv_next)
			if (vapparent->iv_unit == unit)
				break;
		vap->iv_xrvap = vapparent;
		vap->iv_ath_cap = vapparent->iv_ath_cap;
		/* Default multicast rate to lowest possible 256 Kbps */
		vap->iv_mcast_rate = 256;
	} else {
		vap->iv_unit = unit;
		vap->iv_flags = ic->ic_flags;		/* propagate common flags */
		vap->iv_flags_ext = ic->ic_flags_ext;
		vap->iv_xrvap = NULL;
		vap->iv_ath_cap = ic->ic_ath_cap;
		/* Default Multicast traffic to lowest rate of 1 Mbps*/
		vap->iv_mcast_rate = 1000;
	}
#else
	vap->iv_unit = unit;
	vap->iv_flags = ic->ic_flags;		/* propagate common flags */
	vap->iv_flags_ext = ic->ic_flags_ext;
	vap->iv_xrvap = NULL;
	vap->iv_ath_cap = ic->ic_ath_cap;
	/* Default Multicast traffic to lowest rate of 1000 Kbps*/
	vap->iv_mcast_rate = 1000;
#endif
	vap->iv_caps = ic->ic_caps &~ IEEE80211_C_OPMODE;
	switch (opmode) {
	case IEEE80211_M_STA:
		/* WDS/Repeater */
		if (flags & IEEE80211_NO_STABEACONS)
			vap->iv_flags_ext |= IEEE80211_FEXT_SWBMISS;
		break;
	case IEEE80211_M_IBSS:
		vap->iv_caps |= IEEE80211_C_IBSS;
		vap->iv_ath_cap &= ~IEEE80211_ATHC_XR;
		break;
	case IEEE80211_M_AHDEMO:
		vap->iv_caps |= IEEE80211_C_AHDEMO;
		vap->iv_ath_cap &= ~IEEE80211_ATHC_XR;
		break;
	case IEEE80211_M_HOSTAP:
		vap->iv_caps |= IEEE80211_C_HOSTAP;
		vap->iv_ath_cap &= ~IEEE80211_ATHC_TURBOP;
		if ((vap->iv_flags & IEEE80211_VAP_XR) == 0)
			vap->iv_ath_cap &= ~IEEE80211_ATHC_XR;
		break;
	case IEEE80211_M_MONITOR:
		vap->iv_caps |= IEEE80211_C_MONITOR;
		vap->iv_ath_cap &= ~(IEEE80211_ATHC_XR | IEEE80211_ATHC_TURBOP);
		break;
	case IEEE80211_M_WDS:
		vap->iv_caps |= IEEE80211_C_WDS;
		vap->iv_ath_cap &= ~(IEEE80211_ATHC_XR | IEEE80211_ATHC_TURBOP);
		vap->iv_flags_ext |= IEEE80211_FEXT_WDS;
		break;
	}
	vap->iv_opmode = opmode;
	IEEE80211_INIT_TQUEUE(&vap->iv_stajoin1tq, ieee80211_sta_join1_tasklet, vap);

	vap->iv_chanchange_count = 0;

	/*
	 * Enable various functionality by default if we're capable.
	 */
#ifdef notyet
	if (vap->iv_caps & IEEE80211_C_WME)
		vap->iv_flags |= IEEE80211_F_WME;
#endif
	if (vap->iv_caps & IEEE80211_C_FF)
		vap->iv_flags |= IEEE80211_F_FF;
	/* NB: bg scanning only makes sense for station mode right now */
	if (ic->ic_opmode == IEEE80211_M_STA &&
	    (vap->iv_caps & IEEE80211_C_BGSCAN))
		vap->iv_flags |= IEEE80211_F_BGSCAN;

	vap->iv_dtim_period = IEEE80211_DTIM_DEFAULT;
	vap->iv_des_chan = IEEE80211_CHAN_ANYC;	/* any channel is ok */

	IEEE80211_ADDR_COPY(vap->iv_myaddr, ic->ic_myaddr);
	/* NB: defer setting dev_addr so driver can override */

	ieee80211_crypto_vattach(vap);
	ieee80211_node_vattach(vap);
	ieee80211_power_vattach(vap);
	ieee80211_proto_vattach(vap);
	ieee80211_scan_vattach(vap);
	ieee80211_vlan_vattach(vap);
	ieee80211_ioctl_vattach(vap);
	ieee80211_sysctl_vattach(vap);

	return 1;
#undef IEEE80211_C_OPMODE
}
EXPORT_SYMBOL(ieee80211_vap_setup);

int
ieee80211_vap_attach(struct ieee80211vap *vap,
	ifm_change_cb_t media_change, ifm_stat_cb_t media_status)
{
	struct net_device *dev = vap->iv_dev;
	struct ieee80211com *ic = vap->iv_ic;
	struct ifmediareq imr;

	ieee80211_node_latevattach(vap);	/* XXX move into vattach */
	ieee80211_power_latevattach(vap);	/* XXX move into vattach */

	memset(vap->wds_mac, 0x00, IEEE80211_ADDR_LEN);

	(void) ieee80211_media_setup(ic, &vap->iv_media,
		vap->iv_caps, media_change, media_status);
	ieee80211_media_status(dev, &imr);
	ifmedia_set(&vap->iv_media, imr.ifm_active);

	IEEE80211_LOCK_IRQ(ic);
	TAILQ_INSERT_TAIL(&ic->ic_vaps, vap, iv_next);
	IEEE80211_UNLOCK_IRQ(ic);

	IEEE80211_ADDR_COPY(dev->dev_addr, vap->iv_myaddr);

#ifdef ATH_SUPERG_XR
	/* 
	 * do not register XR vap device with OS.
	 */
	if (vap->iv_flags & IEEE80211_F_XR)
		return 0; 
#endif

	ieee80211_scanner_get(vap->iv_opmode, 1);

	/* NB: rtnl is held on entry so don't use register_netdev */
	if (register_netdevice(dev)) {
		printk(KERN_ERR "%s: unable to register device\n", dev->name);
		return 0;
	} else
		return 1;
}
EXPORT_SYMBOL(ieee80211_vap_attach);

void
ieee80211_vap_detach(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct net_device *dev = vap->iv_dev;

	IEEE80211_CANCEL_TQUEUE(&vap->iv_stajoin1tq);
	IEEE80211_LOCK_IRQ(ic);
	TAILQ_REMOVE(&ic->ic_vaps, vap, iv_next);
	if (TAILQ_EMPTY(&ic->ic_vaps))		/* reset to supported mode */
		ic->ic_opmode = IEEE80211_M_STA;
	IEEE80211_UNLOCK_IRQ(ic);

	ifmedia_removeall(&vap->iv_media);

	ieee80211_sysctl_vdetach(vap);
	ieee80211_proc_cleanup(vap);
	ieee80211_ioctl_vdetach(vap);
	ieee80211_vlan_vdetach(vap);
	ieee80211_scan_vdetach(vap);
	ieee80211_proto_vdetach(vap);
	ieee80211_crypto_vdetach(vap);
	ieee80211_power_vdetach(vap);
	ieee80211_node_vdetach(vap);

#ifdef ATH_SUPERG_XR
	/*
	 *  XR vap is not registered.
	 */
	if (!(vap->iv_flags & IEEE80211_F_XR))
#endif
	/* NB: rtnl is held on entry so don't use unregister_netdev */
	unregister_netdevice(dev);
}
EXPORT_SYMBOL(ieee80211_vap_detach);

/*
 * Convert MHz frequency to IEEE channel number.
 */
u_int
ieee80211_mhz2ieee(u_int freq, u_int flags)
{
	if (flags & IEEE80211_CHAN_2GHZ) {	/* 2GHz band */
		if (freq == 2484)		/* Japan */
			return 14;
		if ((freq >= 2412) && (freq < 2484)) /* don't number non-IEEE channels */
			return (freq - 2407) / 5;
		return 0;
	} else if (flags & IEEE80211_CHAN_5GHZ)	{	/* 5Ghz band */
		if ((freq >= 5150) && (freq <= 5825))	/* don't number non-IEEE channels */
			return (freq - 5000) / 5;
		return 0;
	} else {
		/* something is fishy, don't do anything */
		return 0;
	}
}
EXPORT_SYMBOL(ieee80211_mhz2ieee);

/*
 * Convert channel to IEEE channel number.
 */
u_int
ieee80211_chan2ieee(struct ieee80211com *ic, const struct ieee80211_channel *c)
{
	if (c == NULL) {
		if_printf(ic->ic_dev, "invalid channel (NULL)\n");
		return 0;		/* XXX */
	}
	return (c == IEEE80211_CHAN_ANYC ?  IEEE80211_CHAN_ANY : c->ic_ieee);
}
EXPORT_SYMBOL(ieee80211_chan2ieee);

/*
 * Convert IEEE channel number to MHz frequency.
 */
u_int
ieee80211_ieee2mhz(u_int chan, u_int flags)
{
	if (flags & IEEE80211_CHAN_2GHZ) {	/* 2GHz band */
		if (chan == 14)
			return 2484;
		if (chan < 14)
			return 2407 + chan * 5;
		else
			return 2512 + ((chan - 15) * 20);
	} else if (flags & IEEE80211_CHAN_5GHZ) /* 5Ghz band */
		return 5000 + (chan * 5);
	else {					/* either, guess */
		if (chan == 14)
			return 2484;
		if (chan < 14)			/* 0-13 */
			return 2407 + chan * 5;
		if (chan < 27)			/* 15-26 */
			return 2512 + ((chan - 15) * 20);
		return 5000 + (chan * 5);
	}
}
EXPORT_SYMBOL(ieee80211_ieee2mhz);

/*
 * Locate a channel given a frequency+flags.  We cache
 * the previous lookup to optimize swithing between two
 * channels--as happens with dynamic turbo.
 */
struct ieee80211_channel *
ieee80211_find_channel(struct ieee80211com *ic, int freq, int flags)
{
	struct ieee80211_channel *c;
	int i;

	flags &= IEEE80211_CHAN_ALLTURBO;
	c = ic->ic_prevchan;
	if (c != NULL && c->ic_freq == freq &&
	    (c->ic_flags & IEEE80211_CHAN_ALLTURBO) == flags)
		return c;
	/* brute force search */
	for (i = 0; i < ic->ic_nchans; i++) {
		c = &ic->ic_channels[i];
		if (c->ic_freq == freq &&
		    (c->ic_flags & IEEE80211_CHAN_ALLTURBO) == flags)
			return c;
	}
	return NULL;
}
EXPORT_SYMBOL(ieee80211_find_channel);

/*
 * Setup the media data structures according to the channel and
 * rate tables.  This must be called by the driver after
 * ieee80211_attach and before most anything else.
 */
int
ieee80211_media_setup(struct ieee80211com *ic,
	struct ifmedia *media, u_int32_t caps,
	ifm_change_cb_t media_change, ifm_stat_cb_t media_stat)
{
#define	ADD(_media, _s, _o) \
	ifmedia_add(_media, IFM_MAKEWORD(IFM_IEEE80211, (_s), (_o), 0), 0, NULL)
	int i, j, mode, rate, maxrate, mword, mopt, r;
	struct ieee80211_rateset *rs;
	struct ieee80211_rateset allrates;

	/*
	 * Fill in media characteristics.
	 */
	ifmedia_init(media, 0, media_change, media_stat);
	maxrate = 0;
	memset(&allrates, 0, sizeof(allrates));
	for (mode = IEEE80211_MODE_AUTO; mode < IEEE80211_MODE_MAX; mode++) {
		static const u_int mopts[] = { 
			IFM_AUTO,
			IFM_IEEE80211_11A,
			IFM_IEEE80211_11B,
			IFM_IEEE80211_11G,
			IFM_IEEE80211_FH,
			IFM_IEEE80211_11A | IFM_IEEE80211_TURBO,
			IFM_IEEE80211_11G | IFM_IEEE80211_TURBO,
		};
		if ((ic->ic_modecaps & (1<<mode)) == 0)
			continue;
		mopt = mopts[mode];
		ADD(media, IFM_AUTO, mopt);	/* e.g. 11a auto */
		if (caps & IEEE80211_C_IBSS)
			ADD(media, IFM_AUTO, mopt | IFM_IEEE80211_ADHOC);
		if (caps & IEEE80211_C_HOSTAP)
			ADD(media, IFM_AUTO, mopt | IFM_IEEE80211_HOSTAP);
		if (caps & IEEE80211_C_AHDEMO)
			ADD(media, IFM_AUTO, mopt | IFM_IEEE80211_ADHOC | IFM_FLAG0);
		if (caps & IEEE80211_C_MONITOR)
			ADD(media, IFM_AUTO, mopt | IFM_IEEE80211_MONITOR);
		if (caps & IEEE80211_C_WDS)
			ADD(media, IFM_AUTO, mopt | IFM_IEEE80211_WDS);
		if (mode == IEEE80211_MODE_AUTO)
			continue;
		rs = &ic->ic_sup_rates[mode];
		for (i = 0; i < rs->rs_nrates; i++) {
			rate = rs->rs_rates[i];
			mword = ieee80211_rate2media(ic, rate, mode);
			if (mword == 0)
				continue;
			ADD(media, mword, mopt);
			if (caps & IEEE80211_C_IBSS)
				ADD(media, mword, mopt | IFM_IEEE80211_ADHOC);
			if (caps & IEEE80211_C_HOSTAP)
				ADD(media, mword, mopt | IFM_IEEE80211_HOSTAP);
			if (caps & IEEE80211_C_AHDEMO)
				ADD(media, mword, mopt | IFM_IEEE80211_ADHOC | IFM_FLAG0);
			if (caps & IEEE80211_C_MONITOR)
				ADD(media, mword, mopt | IFM_IEEE80211_MONITOR);
			if (caps & IEEE80211_C_WDS)
				ADD(media, mword, mopt | IFM_IEEE80211_WDS);
			/*
			 * Add rate to the collection of all rates.
			 */
			r = rate & IEEE80211_RATE_VAL;
			for (j = 0; j < allrates.rs_nrates; j++)
				if (allrates.rs_rates[j] == r)
					break;
			if (j == allrates.rs_nrates) {
				/* unique, add to the set */
				allrates.rs_rates[j] = r;
				allrates.rs_nrates++;
			}
			rate = (rate & IEEE80211_RATE_VAL) / 2;
			if (rate > maxrate)
				maxrate = rate;
		}
	}
	for (i = 0; i < allrates.rs_nrates; i++) {
		mword = ieee80211_rate2media(ic, allrates.rs_rates[i],
				IEEE80211_MODE_AUTO);
		if (mword == 0)
			continue;
		mword = IFM_SUBTYPE(mword);	/* remove media options */
		ADD(media, mword, 0);
		if (caps & IEEE80211_C_IBSS)
			ADD(media, mword, IFM_IEEE80211_ADHOC);
		if (caps & IEEE80211_C_HOSTAP)
			ADD(media, mword, IFM_IEEE80211_HOSTAP);
		if (caps & IEEE80211_C_AHDEMO)
			ADD(media, mword, IFM_IEEE80211_ADHOC | IFM_FLAG0);
		if (caps & IEEE80211_C_MONITOR)
			ADD(media, mword, IFM_IEEE80211_MONITOR);
		if (caps & IEEE80211_C_WDS)
			ADD(media, mword, IFM_IEEE80211_WDS);
	}
	return maxrate;
#undef ADD
}

void
ieee80211_mark_dfs(struct ieee80211com *ic, struct ieee80211_channel *ichan)
{
	struct ieee80211_channel *c=NULL;
	struct net_device *dev = ic->ic_dev;
	struct ieee80211vap *vap;
	int i;

	if_printf(dev, "Radar found on channel %d (%d MHz)\n",
		ichan->ic_ieee, ichan->ic_freq);
	if (ic->ic_opmode == IEEE80211_M_HOSTAP) {
		/* Mark the channel in the ic_chan list */
		if (ic->ic_flags_ext & IEEE80211_FEXT_MARKDFS) {
			for (i = 0; i < ic->ic_nchans; i++) {
				c = &ic->ic_channels[i];
				if (c->ic_freq != ichan->ic_freq)
					continue;
				c->ic_flags |= IEEE80211_CHAN_RADAR;
			}
			
			c = ieee80211_find_channel(ic, ichan->ic_freq, ichan->ic_flags);
			if (c == NULL) {
				if_printf(dev,"%s: Couldn't find matching channel for dfs mark (%d, 0x%x)\n",
					  __func__, ichan->ic_freq, ichan->ic_flags);
				return;
			}
			if  (ic->ic_curchan->ic_freq == c->ic_freq) {
				/* get an AP vap */
				vap = TAILQ_FIRST(&ic->ic_vaps);
				while ((vap->iv_state != IEEE80211_S_RUN) && (vap != NULL) &&
				       (vap->iv_ic != ic)) {
					vap = TAILQ_NEXT(vap, iv_next);
				}
				if (vap == NULL) {
					/*
					 * No running VAP was found, check
					 * any one is scanning.
					 */
					vap = TAILQ_FIRST(&ic->ic_vaps);
					while ((vap->iv_state != IEEE80211_S_SCAN) && (vap != NULL) &&
					       (vap->iv_ic != ic)) {
						vap = TAILQ_NEXT(vap, iv_next);
					}
					/*
					 * No running/scanning VAP was found, so they're all in
					 * INIT state, no channel change needed
					 */
					if (!vap)
						return;
					/* is it really Scanning */
					/* XXX race condition ?? */
					if (ic->ic_flags & IEEE80211_F_SCAN)
						return;
					/* it is not scanning , but waiting for ath driver to move the vap to RUN */
				}

				/* 
				 * Check the scan results using only cached results
				 */
				if (!(ieee80211_check_scan(vap, IEEE80211_SCAN_USECACHE | IEEE80211_SCAN_NOSSID | IEEE80211_SCAN_KEEPMODE, 0,
							   vap->iv_des_nssid, vap->iv_des_ssid,
							   ieee80211_scan_dfs_action))) {
					/* No channel was found, so call the scan action with no result */
					ieee80211_scan_dfs_action(vap, NULL);
				}
			}
		} else {
			/* Change to a radar free 11a channel for dfstesttime seconds */
			ic->ic_chanchange_chan = IEEE80211_RADAR_TEST_MUTE_CHAN;
			ic->ic_chanchange_tbtt = IEEE80211_RADAR_11HCOUNT;
			ic->ic_flags |= IEEE80211_F_CHANSWITCH;
			/* A timer is setup in the radar task if markdfs is not set and
			 * we are in hostap mode.
			 */
		}
	} else {
		/* Are we in sta mode? If so, send an action msg to ap saying we found a radar? */
	}
}
EXPORT_SYMBOL(ieee80211_mark_dfs);

void
ieee80211_dfs_test_return(struct ieee80211com *ic, u_int8_t ieeeChan)
{
	struct net_device *dev = ic->ic_dev;

	/* Return to the original channel we were on before the test mute */
	if_printf(dev, "Returning to channel %d\n", ieeeChan);
	printk("Returning to chan %d\n", ieeeChan);
	ic->ic_chanchange_chan = ieeeChan;
	ic->ic_chanchange_tbtt = IEEE80211_RADAR_11HCOUNT;
	ic->ic_flags |= IEEE80211_F_CHANSWITCH;
}
EXPORT_SYMBOL(ieee80211_dfs_test_return);

void
ieee80211_announce(struct ieee80211com *ic)
{
	struct net_device *dev = ic->ic_dev;
	int i, mode, rate, mword;
	struct ieee80211_rateset *rs;

	for (mode = IEEE80211_MODE_11A; mode < IEEE80211_MODE_MAX; mode++) {
		if ((ic->ic_modecaps & (1<<mode)) == 0)
			continue;
		if_printf(dev, "%s rates: ", ieee80211_phymode_name[mode]);
		rs = &ic->ic_sup_rates[mode];
		for (i = 0; i < rs->rs_nrates; i++) {
			rate = rs->rs_rates[i];
			mword = ieee80211_rate2media(ic, rate, mode);
			if (mword == 0)
				continue;
			printf("%s%d%sMbps", (i != 0 ? " " : ""),
			    (rate & IEEE80211_RATE_VAL) / 2,
			    ((rate & 0x1) != 0 ? ".5" : ""));
		}
		printf("\n");
	}
	if_printf(dev, "H/W encryption support:");
	if (ic->ic_caps & IEEE80211_C_WEP)
		printk(" WEP");
	if (ic->ic_caps & IEEE80211_C_AES)
		printk(" AES");
	if (ic->ic_caps & IEEE80211_C_AES_CCM)
		printk(" AES_CCM");
	if (ic->ic_caps & IEEE80211_C_CKIP)
		printk(" CKIP");
	if (ic->ic_caps & IEEE80211_C_TKIP)
		printk(" TKIP");
	printk("\n");
}
EXPORT_SYMBOL(ieee80211_announce);

void
ieee80211_announce_channels(struct ieee80211com *ic)
{
	const struct ieee80211_channel *c;
	char type;
	int i;

	printf("Chan  Freq  RegPwr  MinPwr  MaxPwr\n");
	for (i = 0; i < ic->ic_nchans; i++) {
		c = &ic->ic_channels[i];
		if (IEEE80211_IS_CHAN_ST(c))
			type = 'S';
		else if (IEEE80211_IS_CHAN_108A(c))
			type = 'T';
		else if (IEEE80211_IS_CHAN_108G(c))
			type = 'G';
		else if (IEEE80211_IS_CHAN_A(c))
			type = 'a';
		else if (IEEE80211_IS_CHAN_ANYG(c))
			type = 'g';
		else if (IEEE80211_IS_CHAN_B(c))
			type = 'b';
		else
			type = 'f';
		printf("%4d  %4d%c %6d  %6d  %6d\n"
			, c->ic_ieee, c->ic_freq, type
			, c->ic_maxregpower
			, c->ic_minpower, c->ic_maxpower
		);
	}
}
EXPORT_SYMBOL(ieee80211_announce_channels);

/*
 * Common code to calculate the media status word
 * from the operating mode and channel state.
 */
static int
media_status(enum ieee80211_opmode opmode, const struct ieee80211_channel *chan)
{
	int status;

	status = IFM_IEEE80211;
	switch (opmode) {
	case IEEE80211_M_STA:
		break;
	case IEEE80211_M_AHDEMO:
		status |= IFM_IEEE80211_ADHOC | IFM_FLAG0;
		break;
	case IEEE80211_M_IBSS:
		status |= IFM_IEEE80211_ADHOC;
		break;
	case IEEE80211_M_HOSTAP:
		status |= IFM_IEEE80211_HOSTAP;
		break;
	case IEEE80211_M_MONITOR:
		status |= IFM_IEEE80211_MONITOR;
		break;
	case IEEE80211_M_WDS:
		status |= IFM_IEEE80211_WDS;
		break;
	}
	if (IEEE80211_IS_CHAN_A(chan)) {
		status |= IFM_IEEE80211_11A;
		if (IEEE80211_IS_CHAN_TURBO(chan))
			status |= IFM_IEEE80211_TURBO;
	} else if (IEEE80211_IS_CHAN_B(chan)) {
		status |= IFM_IEEE80211_11B;
	} else if (IEEE80211_IS_CHAN_ANYG(chan)) {
		status |= IFM_IEEE80211_11G;
		if (IEEE80211_IS_CHAN_TURBO(chan))
			status |= IFM_IEEE80211_TURBO;
	} else if (IEEE80211_IS_CHAN_FHSS(chan)) {
		status |= IFM_IEEE80211_FH;
	}
	/* XXX else complain? */

	return status;
}

/*
 * Handle a media requests on the base interface.
 */
static void
ieee80211com_media_status(struct net_device *dev, struct ifmediareq *imr)
{
	struct ieee80211com *ic = dev->priv;	/*XXX*/

	imr->ifm_status = IFM_AVALID;
	if (!TAILQ_EMPTY(&ic->ic_vaps))
		imr->ifm_status |= IFM_ACTIVE;
	imr->ifm_active = media_status(ic->ic_opmode, ic->ic_curchan);
}

/*
 * Convert a media specification to an 802.11 phy mode.
 */
static int
media2mode(const struct ifmedia_entry *ime, enum ieee80211_phymode *mode)
{

	switch (IFM_MODE(ime->ifm_media)) {
	case IFM_IEEE80211_11A:
		*mode = IEEE80211_MODE_11A;
		break;
	case IFM_IEEE80211_11B:
		*mode = IEEE80211_MODE_11B;
		break;
	case IFM_IEEE80211_11G:
		*mode = IEEE80211_MODE_11G;
		break;
	case IFM_IEEE80211_FH:
		*mode = IEEE80211_MODE_FH;
		break;
	case IFM_AUTO:
		*mode = IEEE80211_MODE_AUTO;
		break;
	default:
		return 0;
	}
	/*
	 * Turbo mode is an ``option''.  
	 * XXX: Turbo currently does not apply to AUTO
	 */
	if (ime->ifm_media & IFM_IEEE80211_TURBO) {
		if (*mode == IEEE80211_MODE_11A)
			*mode = IEEE80211_MODE_TURBO_A;
		else if (*mode == IEEE80211_MODE_11G)
			*mode = IEEE80211_MODE_TURBO_G;
		else
			return 0;
	}
	return 1;
}

static int
ieee80211com_media_change(struct net_device *dev)
{
	struct ieee80211com *ic = dev->priv;	/*XXX*/
	struct ieee80211vap *vap;
	struct ifmedia_entry *ime = ic->ic_media.ifm_cur;
	enum ieee80211_phymode newphymode;
	int j, error = 0;

	/* XXX is rtnl held here? */
	/*
	 * First, identify the phy mode.
	 */
	if (!media2mode(ime, &newphymode))
		return -EINVAL;
	/* NB: mode must be supported, no need to check */
	/*
	 * Autoselect doesn't make sense when operating as an AP.
	 * If no phy mode has been selected, pick one and lock it
	 * down so rate tables can be used in forming beacon frames
	 * and the like.
	 */
	if (ic->ic_opmode == IEEE80211_M_HOSTAP &&
	    newphymode == IEEE80211_MODE_AUTO) {
		for (j = IEEE80211_MODE_11A; j < IEEE80211_MODE_MAX; j++)
			if (ic->ic_modecaps & (1 << j)) {
				newphymode = j;
				break;
			}
	}

	/*
	 * Handle phy mode change.
	 */
	IEEE80211_LOCK_IRQ(ic);
	if (ic->ic_curmode != newphymode) {		/* change phy mode */
		error = ieee80211_setmode(ic, newphymode);
		if (error != 0) {
			IEEE80211_UNLOCK_IRQ_EARLY(ic);
			return error;
		}
		TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
			/* reset WME state */
			ieee80211_wme_initparams_locked(vap);
			/*
			 * Setup an initial rate set according to the
			 * current/default channel selected above.  This
			 * will be changed when scanning but must exist
			 * now so drivers have a consistent state.
			 */
			KASSERT(vap->iv_bss != NULL, ("no bss node"));
			vap->iv_bss->ni_rates = ic->ic_sup_rates[newphymode];
		}
		error = ENETRESET;
	}
	IEEE80211_UNLOCK_IRQ(ic);

#ifdef notdef
	if (error == 0)
		ifp->if_baudrate = ifmedia_baudrate(ime->ifm_media);
#endif
	return error;
}

static int
findrate(struct ieee80211com *ic, enum ieee80211_phymode mode, int rate)
{
#define	IEEERATE(_ic,_m,_i) \
	((_ic)->ic_sup_rates[_m].rs_rates[_i] & IEEE80211_RATE_VAL)
	int i, nrates = ic->ic_sup_rates[mode].rs_nrates;
	for (i = 0; i < nrates; i++)
		if (IEEERATE(ic, mode, i) == rate)
			return i;
	return -1;
#undef IEEERATE
}

/*
 * Convert a media specification to a rate index and possibly a mode
 * (if the rate is fixed and the mode is specified as ``auto'' then
 * we need to lock down the mode so the index is meanginful).
 */
static int
checkrate(struct ieee80211com *ic, enum ieee80211_phymode mode, int rate)
{

	/*
	 * Check the rate table for the specified/current phy.
	 */
	if (mode == IEEE80211_MODE_AUTO) {
		int i;
		/*
		 * In autoselect mode search for the rate.
		 */
		for (i = IEEE80211_MODE_11A; i < IEEE80211_MODE_MAX; i++) {
			if ((ic->ic_modecaps & (1 << i)) &&
			    findrate(ic, i, rate) != -1)
				return 1;
		}
		return 0;
	} else {
		/*
		 * Mode is fixed, check for rate.
		 */
		return (findrate(ic, mode, rate) != -1);
	}
}

/*
 * Handle a media change request; the only per-vap
 * information that is meaningful is the fixed rate
 * and desired phy mode.
 */
int
ieee80211_media_change(struct net_device *dev)
{
	struct ieee80211vap *vap = dev->priv;
	struct ieee80211com *ic = vap->iv_ic;
	struct ifmedia_entry *ime = vap->iv_media.ifm_cur;
	enum ieee80211_phymode newmode;
	int newrate, error;

	/*
	 * First, identify the desired phy mode.
	 */
	if (!media2mode(ime, &newmode))
		return -EINVAL;
	/*
	 * Check for fixed/variable rate.
	 */
	if (IFM_SUBTYPE(ime->ifm_media) != IFM_AUTO) {
		/*
		 * Convert media subtype to rate and potentially
		 * lock down the mode.
		 */
		newrate = ieee80211_media2rate(ime->ifm_media);
		if (newrate == 0 || !checkrate(ic, newmode, newrate))
			return -EINVAL;
	} else
		newrate = IEEE80211_FIXED_RATE_NONE;

	/*
	 * Install the rate+mode settings.
	 */
	error = 0;
	if (vap->iv_fixed_rate != newrate) {
		vap->iv_fixed_rate = newrate;		/* fixed tx rate */
		error = ENETRESET;
	}
	if (vap->iv_des_mode != newmode) {
		vap->iv_des_mode = newmode;		/* desired phymode */
		error = ENETRESET;
	}
	return error;
}
EXPORT_SYMBOL(ieee80211_media_change);

void
ieee80211_media_status(struct net_device *dev, struct ifmediareq *imr)
{
	struct ieee80211vap *vap = dev->priv;
	struct ieee80211com *ic = vap->iv_ic;
	enum ieee80211_phymode mode;
	struct ieee80211_rateset *rs;

	imr->ifm_status = IFM_AVALID;
	/*
	 * NB: use the current channel's mode to lock down a xmit
	 * rate only when running; otherwise we may have a mismatch
	 * in which case the rate will not be convertible.
	 */
	if (vap->iv_state == IEEE80211_S_RUN) {
		imr->ifm_status |= IFM_ACTIVE;
		mode = ieee80211_chan2mode(ic->ic_curchan);
	} else
		mode = IEEE80211_MODE_AUTO;
	imr->ifm_active = media_status(vap->iv_opmode, ic->ic_curchan);
	/*
	 * Calculate a current rate if possible.
	 */
	if (vap->iv_fixed_rate != IEEE80211_FIXED_RATE_NONE) {
		/*
		 * A fixed rate is set, report that.
		 */
		imr->ifm_active |= ieee80211_rate2media(ic,
			vap->iv_fixed_rate, mode);
	} else if (vap->iv_opmode == IEEE80211_M_STA) {
		/*
		 * In station mode report the current transmit rate.
		 */
		rs = &vap->iv_bss->ni_rates;
		imr->ifm_active |= ieee80211_rate2media(ic,
			rs->rs_rates[vap->iv_bss->ni_txrate], mode);
	} else
		imr->ifm_active |= IFM_AUTO;
}
EXPORT_SYMBOL(ieee80211_media_status);

/*
 * Set the current phy mode.
 */
int
ieee80211_setmode(struct ieee80211com *ic, enum ieee80211_phymode mode)
{

#if 0
	/*
	 * Potentially invalidate the bss channel.
	 */
	/* XXX not right/too conservative */
	if (ic->ic_bsschan != IEEE80211_CHAN_ANYC &&
	    mode != ieee80211_chan2mode(ic->ic_bsschan))
		ic->ic_bsschan = IEEE80211_CHAN_ANYC;	/* invalidate */
#endif
	ieee80211_reset_erp(ic, mode);	/* reset ERP state */
	ic->ic_curmode = mode;		/* NB: must do post reset_erp */
	return 0;
}
EXPORT_SYMBOL(ieee80211_setmode);

/*
 * Return the phy mode for with the specified channel.
 */
enum ieee80211_phymode
ieee80211_chan2mode(const struct ieee80211_channel *chan)
{
	/*
	 * Callers should handle this case properly, rather than
	 * just relying that this function returns a sane value.
	 * XXX Probably needs to be revised.
	 */
	KASSERT(chan != IEEE80211_CHAN_ANYC, ("channel not setup"));
	
	if (IEEE80211_IS_CHAN_108G(chan))
		return IEEE80211_MODE_TURBO_G;
	else if (IEEE80211_IS_CHAN_TURBO(chan))
		return IEEE80211_MODE_TURBO_A;
	else if (IEEE80211_IS_CHAN_A(chan))
		return IEEE80211_MODE_11A;
	else if (IEEE80211_IS_CHAN_ANYG(chan))
		return IEEE80211_MODE_11G;
	else if (IEEE80211_IS_CHAN_B(chan))
		return IEEE80211_MODE_11B;
	else if (IEEE80211_IS_CHAN_FHSS(chan))
		return IEEE80211_MODE_FH;

	/* NB: should not get here */
	printk("%s: cannot map channel to mode; freq %u flags 0x%x\n",
		__func__, chan->ic_freq, chan->ic_flags);
	return IEEE80211_MODE_11B;
}
EXPORT_SYMBOL(ieee80211_chan2mode);

/*
 * convert IEEE80211 rate value to ifmedia subtype.
 * ieee80211 rate is in unit of 0.5Mbps.
 */
int
ieee80211_rate2media(struct ieee80211com *ic, int rate, enum ieee80211_phymode mode)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	static const struct {
		u_int	m;	/* rate + mode */
		u_int	r;	/* if_media rate */
	} rates[] = {
		{   2 | IFM_IEEE80211_FH, IFM_IEEE80211_FH1 },
		{   4 | IFM_IEEE80211_FH, IFM_IEEE80211_FH2 },
		{   2 | IFM_IEEE80211_11B, IFM_IEEE80211_DS1 },
		{   4 | IFM_IEEE80211_11B, IFM_IEEE80211_DS2 },
		{  11 | IFM_IEEE80211_11B, IFM_IEEE80211_DS5 },
		{  22 | IFM_IEEE80211_11B, IFM_IEEE80211_DS11 },
		{  44 | IFM_IEEE80211_11B, IFM_IEEE80211_DS22 },
		{   3 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM1_50 },
		{   4 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM2_25 },
		{   6 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM3 },
		{   9 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM4_50 },
		{  12 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM6 },
		{  18 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM9 },
		{  24 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM12 },
		{  27 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM13_5 },
		{  36 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM18 },
		{  48 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM24 },
		{  54 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM27 },
		{  72 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM36 },
		{  96 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM48 },
		{ 108 | IFM_IEEE80211_11A, IFM_IEEE80211_OFDM54 },
		{   2 | IFM_IEEE80211_11G, IFM_IEEE80211_DS1 },
		{   4 | IFM_IEEE80211_11G, IFM_IEEE80211_DS2 },
		{  11 | IFM_IEEE80211_11G, IFM_IEEE80211_DS5 },
		{  22 | IFM_IEEE80211_11G, IFM_IEEE80211_DS11 },
		{  12 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM6 },
		{  18 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM9 },
		{  24 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM12 },
		{  36 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM18 },
		{  48 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM24 },
		{  72 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM36 },
		{  96 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM48 },
		{ 108 | IFM_IEEE80211_11G, IFM_IEEE80211_OFDM54 },
		/* NB: OFDM72 doesn't really exist so we don't handle it */
	};
	u_int mask, i;

	mask = rate & IEEE80211_RATE_VAL;
	switch (mode) {
	case IEEE80211_MODE_11A:
	case IEEE80211_MODE_TURBO_A:
		mask |= IFM_IEEE80211_11A;
		break;
	case IEEE80211_MODE_11B:
		mask |= IFM_IEEE80211_11B;
		break;
	case IEEE80211_MODE_FH:
		mask |= IFM_IEEE80211_FH;
		break;
	case IEEE80211_MODE_AUTO:
		/* NB: ic may be NULL for some drivers */
		if (ic && ic->ic_phytype == IEEE80211_T_FH) {
			mask |= IFM_IEEE80211_FH;
			break;
		}
		/* NB: hack, 11g matches both 11b+11a rates */
		/* fall thru... */
	case IEEE80211_MODE_11G:
	case IEEE80211_MODE_TURBO_G:
		mask |= IFM_IEEE80211_11G;
		break;
	}
	for (i = 0; i < N(rates); i++)
		if (rates[i].m == mask)
			return rates[i].r;
	return IFM_AUTO;
#undef N
}
EXPORT_SYMBOL(ieee80211_rate2media);

int
ieee80211_media2rate(int mword)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	static const int ieeerates[] = {
		-1,		/* IFM_AUTO */
		0,		/* IFM_MANUAL */
		0,		/* IFM_NONE */
		2,		/* IFM_IEEE80211_FH1 */
		4,		/* IFM_IEEE80211_FH2 */
		2,		/* IFM_IEEE80211_DS1 */
		4,		/* IFM_IEEE80211_DS2 */
		11,		/* IFM_IEEE80211_DS5 */
		22,		/* IFM_IEEE80211_DS11 */
		44,		/* IFM_IEEE80211_DS22 */
		3,		/* IFM_IEEE80211_OFDM1_50 */
		4,		/* IFM_IEEE80211_OFDM2_25 */
		6,		/* IFM_IEEE80211_OFDM3 */
		9,		/* IFM_IEEE80211_OFDM4_50 */
		12,		/* IFM_IEEE80211_OFDM6 */
		18,		/* IFM_IEEE80211_OFDM9 */
		24,		/* IFM_IEEE80211_OFDM12 */
		27,		/* IFM_IEEE80211_OFDM13_5 */
		36,		/* IFM_IEEE80211_OFDM18 */
		48,		/* IFM_IEEE80211_OFDM24 */
		54,		/* IFM_IEEE80211_OFDM27 */
		72,		/* IFM_IEEE80211_OFDM36 */
		96,		/* IFM_IEEE80211_OFDM48 */
		108,		/* IFM_IEEE80211_OFDM54 */
		144,		/* IFM_IEEE80211_OFDM72 */
	};
	return IFM_SUBTYPE(mword) < N(ieeerates) ?
		ieeerates[IFM_SUBTYPE(mword)] : 0;
#undef N
}
EXPORT_SYMBOL(ieee80211_media2rate);

/*
 * Return netdevice statistics.
 */
static struct net_device_stats *
ieee80211_getstats(struct net_device *dev)
{
	struct ieee80211vap *vap = dev->priv;
	struct net_device_stats *stats = &vap->iv_devstats;

	/* XXX total guess as to what to count where */
	/* update according to private statistics */
	stats->tx_errors = vap->iv_stats.is_tx_nodefkey
			 + vap->iv_stats.is_tx_noheadroom
			 + vap->iv_stats.is_crypto_enmicfail;
	stats->tx_dropped = vap->iv_stats.is_tx_nobuf
			+ vap->iv_stats.is_tx_nonode
			+ vap->iv_stats.is_tx_unknownmgt
			+ vap->iv_stats.is_tx_badcipher
			+ vap->iv_stats.is_tx_nodefkey;
	stats->rx_errors = vap->iv_stats.is_rx_tooshort
			+ vap->iv_stats.is_rx_wepfail
			+ vap->iv_stats.is_rx_decap
			+ vap->iv_stats.is_rx_nobuf
			+ vap->iv_stats.is_rx_decryptcrc
			+ vap->iv_stats.is_rx_ccmpmic
			+ vap->iv_stats.is_rx_tkipmic
			+ vap->iv_stats.is_rx_tkipicv;
	stats->rx_crc_errors = 0;

	return stats;
}

static int
ieee80211_change_mtu(struct net_device *dev, int mtu)
{
	if (!(IEEE80211_MTU_MIN < mtu && mtu <= IEEE80211_MTU_MAX))
		return -EINVAL;
	dev->mtu = mtu;
	/* XXX coordinate with parent device */
	return 0;
}

static void
ieee80211_set_multicast_list(struct net_device *dev)
{
	struct ieee80211vap *vap = dev->priv;
	struct ieee80211com *ic = vap->iv_ic;
	struct net_device *parent = ic->ic_dev;

	IEEE80211_LOCK_IRQ(ic);
	if (dev->flags & IFF_PROMISC) {
		if ((vap->iv_flags & IEEE80211_F_PROMISC) == 0) {
			vap->iv_flags |= IEEE80211_F_PROMISC;
			ic->ic_promisc++;
			parent->flags |= IFF_PROMISC;
		}
	} else {
		if (vap->iv_flags & IEEE80211_F_PROMISC) {
			vap->iv_flags &= ~IEEE80211_F_PROMISC;
			ic->ic_promisc--;
			parent->flags &= ~IFF_PROMISC;
		}
	}
	if (dev->flags & IFF_ALLMULTI) {
		if ((vap->iv_flags & IEEE80211_F_ALLMULTI) == 0) {
			vap->iv_flags |= IEEE80211_F_ALLMULTI;
			ic->ic_allmulti++;
			parent->flags |= IFF_ALLMULTI;
		}
	} else {
		if (vap->iv_flags & IEEE80211_F_ALLMULTI) {
			vap->iv_flags &= ~IEEE80211_F_ALLMULTI;
			ic->ic_allmulti--;
			parent->flags &= ~IFF_ALLMULTI;
		}
	}
	IEEE80211_UNLOCK_IRQ(ic);

	/* XXX merge multicast list into parent device */
	parent->set_multicast_list(ic->ic_dev);
}

void
ieee80211_build_countryie(struct ieee80211com *ic)
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
	int i, j, chanflags, found;
	struct net_device *dev = ic->ic_dev;
	struct ieee80211_channel *c;
	u_int8_t chanlist[IEEE80211_CHAN_MAX + 1];
	u_int8_t chancnt = 0;
	u_int8_t *cur_runlen, *cur_chan, *cur_pow, prevchan;

	/*
	 * Fill in country IE.
	 */
	memset(&ic->ic_country_ie, 0, sizeof(ic->ic_country_ie));  
	ic->ic_country_ie.country_id = IEEE80211_ELEMID_COUNTRY;
	ic->ic_country_ie.country_len = 0; /* init needed by following code */

	/* initialize country IE */
	found = 0;
	for (i = 0; i < N(country_strings); i++) {
		if (country_strings[i].iso_code == ic->ic_country_code) {
			ic->ic_country_ie.country_str[0] = country_strings[i].iso_name[0];
			ic->ic_country_ie.country_str[1] = country_strings[i].iso_name[1];
			found = 1;
			break;
		}
	}
	if (!found) {
		if_printf(dev, "bad country string ignored: %d\n",
			ic->ic_country_code);
		ic->ic_country_ie.country_str[0] = ' ';
		ic->ic_country_ie.country_str[1] = ' ';			
	}

	/* 
	 * indoor/outdoor portion if country string.
	 * NB: this is not quite right, since we should have one of:
	 *     'I' indoor only
	 *     'O' outdoor only
	 *     ' ' all enviroments
	 *  we currently can only provide 'I' or ' '.
	 */
	ic->ic_country_ie.country_str[2] = 'I';
	if (ic->ic_country_outdoor)
		ic->ic_country_ie.country_str[2] = ' ';
	/* 
	 * runlength encoded channel max tx power info.
	 */
	cur_runlen = &ic->ic_country_ie.country_triplet[1];
	cur_chan = &ic->ic_country_ie.country_triplet[0];
	cur_pow = &ic->ic_country_ie.country_triplet[2];
	prevchan = 0;
	ic->ic_country_ie.country_len = 3; /* invalid, but just initialize */

	if ((ic->ic_flags_ext & IEEE80211_FEXT_REGCLASS) && ic->ic_nregclass) {
		/* Add regulatory triplets.
	 	* chan/no_of_chans/tx power triplet is overridden as
	 	* as follows:
	 	* cur_chan == REGULATORY EXTENSION ID.
	 	* cur_runlen = Regulatory class.
	 	* cur_pow = coverage class.
	 	*/
		for (i=0; i < ic->ic_nregclass; i++) {
			*cur_chan = IEEE80211_REG_EXT_ID;
			*cur_runlen = ic->ic_regclassids[i];
			*cur_pow = ic->ic_coverageclass;
	
			cur_runlen +=3;
			cur_chan += 3;
			cur_pow += 3;
			ic->ic_country_ie.country_len += 3;
		}
	} else {
		if ((ic->ic_curmode == IEEE80211_MODE_11A) || 
		    (ic->ic_curmode == IEEE80211_MODE_TURBO_A))
			chanflags = IEEE80211_CHAN_5GHZ;
		else 
			chanflags = IEEE80211_CHAN_2GHZ;
	
		memset(&chanlist[0], 0, sizeof(chanlist));  
		/* XXX not right due to duplicate entries */
		for (i = 0; i < ic->ic_nchans; i++) {
			c = &ic->ic_channels[i];
	
			/* Does channel belong to current operation mode */
			if (!(c->ic_flags & chanflags))
				continue;

			/* Skip previously reported channels */
			for (j = 0; j < chancnt; j++)
				if (c->ic_ieee == chanlist[j])
					break;
			
			if (j != chancnt) /* found a match */
				continue;

			chanlist[chancnt] = c->ic_ieee;
			chancnt++;
	
			/* Skip turbo channels */
			if (IEEE80211_IS_CHAN_TURBO(c))
				continue;
	
			/* Skip half/quarter rate channels */
			if (IEEE80211_IS_CHAN_HALF(c) || 
			    IEEE80211_IS_CHAN_QUARTER(c))
				continue;
	
			if (*cur_runlen == 0) {
				(*cur_runlen)++;
				*cur_pow = c->ic_maxregpower;
				*cur_chan = c->ic_ieee;
				prevchan = c->ic_ieee;
				ic->ic_country_ie.country_len += 3;
			} else if (*cur_pow == c->ic_maxregpower &&
			    c->ic_ieee == prevchan + 1) {
				(*cur_runlen)++;
				prevchan = c->ic_ieee;
			} else {
				cur_runlen +=3;
				cur_chan += 3;
				cur_pow += 3;
				(*cur_runlen)++;
				*cur_pow = c->ic_maxregpower;
				*cur_chan = c->ic_ieee;
				prevchan = c->ic_ieee;
				ic->ic_country_ie.country_len += 3;
			}
		}
	}

	/* pad */
	if (ic->ic_country_ie.country_len & 1)
		ic->ic_country_ie.country_len++;

#if 0
	ic->ic_country_ie.country_len=8;
	ic->ic_country_ie.country_triplet[0] = 10;
	ic->ic_country_ie.country_triplet[1] = 11;
	ic->ic_country_ie.country_triplet[2] = 12;
#endif
#undef N
}
