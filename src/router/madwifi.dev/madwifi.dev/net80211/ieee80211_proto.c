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
 * $Id: ieee80211_proto.c 3314 2008-01-30 23:50:16Z mtaylor $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 protocol support.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <ath/if_athvar.h>

/* XXX tunables */
#define	AGGRESSIVE_MODE_SWITCH_HYSTERESIS	3	/* pkts / 100ms */
#define	HIGH_PRI_SWITCH_THRESH			10	/* pkts / 100ms */

#define	IEEE80211_RATE2MBS(r)	(((r) & IEEE80211_RATE_VAL) / 2)

const char *ieee80211_mgt_subtype_name[] = {
	"assoc_req", "assoc_resp", "reassoc_req", "reassoc_resp",
	"probe_req", "probe_resp", "reserved#6", "reserved#7",
	"beacon", "atim", "disassoc", "auth",
	"deauth", "reserved#13", "reserved#14", "reserved#15"
};

EXPORT_SYMBOL(ieee80211_mgt_subtype_name);
const char *ieee80211_ctl_subtype_name[] = {
	"reserved#0", "reserved#1", "reserved#2", "reserved#3",
	"reserved#3", "reserved#5", "reserved#6", "reserved#7",
	"reserved#8", "reserved#9", "ps_poll", "rts",
	"cts", "ack", "cf_end", "cf_end_ack"
};

EXPORT_SYMBOL(ieee80211_ctl_subtype_name);
const char *ieee80211_state_name[IEEE80211_S_MAX] = {
	"INIT",			/* IEEE80211_S_INIT */
	"SCAN",			/* IEEE80211_S_SCAN */
	"AUTH",			/* IEEE80211_S_AUTH */
	"ASSOC",		/* IEEE80211_S_ASSOC */
	"RUN"			/* IEEE80211_S_RUN */
};

EXPORT_SYMBOL(ieee80211_state_name);
const char *ieee80211_wme_acnames[] = {
	"WME_AC_BE",
	"WME_AC_BK",
	"WME_AC_VI",
	"WME_AC_VO",
	"WME_UPSD",
};

EXPORT_SYMBOL(ieee80211_wme_acnames);

static int ieee80211_newstate(struct ieee80211vap *, enum ieee80211_state, int);
static void ieee80211_tx_timeout(unsigned long);
#ifdef ATH_SUPERG_XR
static void ieee80211_start_xrvap(unsigned long);
#endif
void ieee80211_auth_setup(void);

void ieee80211_proto_attach(struct ieee80211com *ic)
{

	ic->ic_protmode = IEEE80211_PROT_CTSONLY;

	ic->ic_wme.wme_hipri_switch_hysteresis = AGGRESSIVE_MODE_SWITCH_HYSTERESIS;

	/* initialize management frame handlers */
	ic->ic_recv_mgmt = ieee80211_recv_mgmt;
	ic->ic_send_mgmt = ieee80211_send_mgmt;

	ieee80211_auth_setup();
}

void ieee80211_proto_detach(struct ieee80211com *ic)
{
}

#ifdef LED2_PIN
/* UBNT */
static void ieee80211_sta_probe_ap(unsigned long arg)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)arg;
	struct ieee80211_node *ni;
	struct net_device *dev = vap->iv_dev;

	if (vap->iv_opmode == IEEE80211_M_STA && (dev->flags & IFF_RUNNING)) {
		ni = vap->iv_bss;
		if (ni != NULL && vap->iv_state == IEEE80211_S_RUN) {
			if (vap->iv_last_rssiupdate < jiffies + (2 * HZ)) {
				/* So send a directed probe request here. */
				ieee80211_send_probereq(ni, vap->iv_myaddr, ni->ni_bssid, ni->ni_bssid, ni->ni_essid, ni->ni_esslen, vap->iv_opt_ie, vap->iv_opt_ie_len);
			}
		}
		if (vap->iv_state == IEEE80211_S_RUN)
			mod_timer(&vap->iv_rssiupdate, jiffies + HZ);
	} else
		del_timer(&vap->iv_rssiupdate);
}

/* END UBNT */
#endif

void ieee80211_proto_vattach(struct ieee80211vap *vap)
{
#ifdef notdef
	vap->iv_rtsthreshold = IEEE80211_RTS_DEFAULT;
#else
	vap->iv_rtsthreshold = IEEE80211_RTS_MAX;
#endif
	vap->iv_fragthreshold = 2346;	/* XXX not used yet */
	vap->iv_fixed_rate = IEEE80211_FIXED_RATE_NONE;
	init_timer(&vap->iv_mgtsend);
	init_timer(&vap->iv_xrvapstart);
	init_timer(&vap->iv_swbmiss);
	init_timer(&vap->iv_csa_timer);
#ifdef LED2_PIN
	/* UBNT */
	if (vap->iv_opmode == IEEE80211_M_STA) {
		init_timer(&vap->iv_rssiupdate);
	}
#endif
	/* END UBNT */
	vap->iv_mgtsend.function = ieee80211_tx_timeout;
	vap->iv_mgtsend.data = (unsigned long)vap;

	/* protocol state change handler */
	vap->iv_newstate = ieee80211_newstate;
}

void ieee80211_proto_vdetach(struct ieee80211vap *vap)
{

#ifdef LED2_PIN
	/* UBNT */
	if (vap->iv_opmode == IEEE80211_M_STA) {
		del_timer(&vap->iv_rssiupdate);
	}
	/* END UBNT */
#endif
	/*
	 * This should not be needed as we detach when reseting
	 * the state but be conservative here since the
	 * authenticator may do things like spawn kernel threads.
	 */
	if (vap->iv_auth->ia_detach)
		vap->iv_auth->ia_detach(vap);

	/*
	 * Detach any ACL'ator.
	 */
	if (vap->iv_acl != NULL)
		vap->iv_acl->iac_detach(vap);
}

/*
 * Simple-minded authenticator module support.
 */

#define	IEEE80211_AUTH_MAX	(IEEE80211_AUTH_WPA+1)
/* XXX well-known names */
static const char *auth_modnames[IEEE80211_AUTH_MAX] = {
	"wlan_internal",	/* IEEE80211_AUTH_NONE */
	"wlan_internal",	/* IEEE80211_AUTH_OPEN */
	"wlan_internal",	/* IEEE80211_AUTH_SHARED */
	"wlan_xauth",		/* IEEE80211_AUTH_8021X  */
	"wlan_internal",	/* IEEE80211_AUTH_AUTO */
	"wlan_xauth",		/* IEEE80211_AUTH_WPA */
};

static const struct ieee80211_authenticator *authenticators[IEEE80211_AUTH_MAX];

static const struct ieee80211_authenticator auth_internal = {
	.ia_name = "wlan_internal",
	.ia_attach = NULL,
	.ia_detach = NULL,
	.ia_node_join = NULL,
	.ia_node_leave = NULL,
};

/*
 * Setup internal authenticators once; they are never unregistered.
 */
void ieee80211_auth_setup(void)
{
	ieee80211_authenticator_register(IEEE80211_AUTH_OPEN, &auth_internal);
	ieee80211_authenticator_register(IEEE80211_AUTH_SHARED, &auth_internal);
	ieee80211_authenticator_register(IEEE80211_AUTH_AUTO, &auth_internal);
}

const struct ieee80211_authenticator *ieee80211_authenticator_get(int auth)
{
	if (auth >= IEEE80211_AUTH_MAX)
		return NULL;
	if (authenticators[auth] == NULL)
		ieee80211_load_module(auth_modnames[auth]);
	return authenticators[auth];
}

void ieee80211_authenticator_register(int type, const struct ieee80211_authenticator *auth)
{
	if (type >= IEEE80211_AUTH_MAX)
		return;
	authenticators[type] = auth;
}

EXPORT_SYMBOL(ieee80211_authenticator_register);

void ieee80211_authenticator_unregister(int type)
{
	if (type >= IEEE80211_AUTH_MAX)
		return;
	authenticators[type] = NULL;
}

EXPORT_SYMBOL(ieee80211_authenticator_unregister);

/*
 * Very simple-minded authenticator backend module support.
 */
/* XXX just one for now */
static const struct ieee80211_authenticator_backend *backend = NULL;

void ieee80211_authenticator_backend_register(const struct ieee80211_authenticator_backend *be)
{
	printk(KERN_INFO "wlan: %s backend registered\n", be->iab_name);
	backend = be;
}

EXPORT_SYMBOL(ieee80211_authenticator_backend_register);

void ieee80211_authenticator_backend_unregister(const struct ieee80211_authenticator_backend *be)
{
	if (backend == be)
		backend = NULL;
	printk(KERN_INFO "wlan: %s backend unregistered\n", be->iab_name);
}

EXPORT_SYMBOL(ieee80211_authenticator_backend_unregister);

const struct ieee80211_authenticator_backend *ieee80211_authenticator_backend_get(const char *name)
{
	return backend && strcmp(backend->iab_name, name) == 0 ? backend : NULL;
}

EXPORT_SYMBOL(ieee80211_authenticator_backend_get);

/*
 * Very simple-minded ACL module support.
 */
/* XXX just one for now */
static const struct ieee80211_aclator *acl = NULL;

void ieee80211_aclator_register(const struct ieee80211_aclator *iac)
{
	printk(KERN_INFO "wlan: %s acl policy registered\n", iac->iac_name);
	acl = iac;
}

EXPORT_SYMBOL(ieee80211_aclator_register);

void ieee80211_aclator_unregister(const struct ieee80211_aclator *iac)
{
	if (acl == iac)
		acl = NULL;
	printk(KERN_INFO "wlan: %s acl policy unregistered\n", iac->iac_name);
}

EXPORT_SYMBOL(ieee80211_aclator_unregister);

const struct ieee80211_aclator *ieee80211_aclator_get(const char *name)
{
	if (acl == NULL)
		ieee80211_load_module("wlan_acl");
	return acl && strcmp(acl->iac_name, name) == 0 ? acl : NULL;
}

EXPORT_SYMBOL(ieee80211_aclator_get);

#ifdef IEEE80211_DEBUG
void ieee80211_print_essid(const u_int8_t *essid, int len)
{
	int i;
	const u_int8_t *p;

	if (len > IEEE80211_NWID_LEN)
		len = IEEE80211_NWID_LEN;
	/* determine printable or not */
	for (i = 0, p = essid; i < len; i++, p++) {
		if (*p < ' ' || *p > 0x7e)
			break;
	}
	if (i == len) {
		printk("\"");
		for (i = 0, p = essid; i < len; i++, p++)
			printk("%c", *p);
		printk("\"");
	} else {
		printk("0x");
		for (i = 0, p = essid; i < len; i++, p++)
			printk("%02x", *p);
	}
}

EXPORT_SYMBOL(ieee80211_print_essid);

void ieee80211_dump_pkt(struct ieee80211com *ic, const u_int8_t *buf, int len, int rate, int rssi)
{
	const struct ieee80211_frame *wh;
	int i;

	wh = (const struct ieee80211_frame *)buf;
	switch (wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		printk("NODS " MAC_FMT, MAC_ADDR(wh->i_addr2));
		printk("->" MAC_FMT, MAC_ADDR(wh->i_addr1));
		printk("(" MAC_FMT ")", MAC_ADDR(wh->i_addr3));
		break;
	case IEEE80211_FC1_DIR_TODS:
		printk("TODS " MAC_FMT, MAC_ADDR(wh->i_addr2));
		printk("->" MAC_FMT, MAC_ADDR(wh->i_addr3));
		printk("(" MAC_FMT ")", MAC_ADDR(wh->i_addr1));
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		printk("FRDS " MAC_FMT, MAC_ADDR(wh->i_addr3));
		printk("->" MAC_FMT, MAC_ADDR(wh->i_addr1));
		printk("(" MAC_FMT ")", MAC_ADDR(wh->i_addr2));
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		printk("DSDS " MAC_FMT, MAC_ADDR((u_int8_t *)&wh[1]));
		printk("->" MAC_FMT, MAC_ADDR(wh->i_addr3));
		printk("(" MAC_FMT, MAC_ADDR(wh->i_addr2));
		printk("->" MAC_FMT ")", MAC_ADDR(wh->i_addr1));
		break;
	}
	switch (wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) {
	case IEEE80211_FC0_TYPE_DATA:
		printk(" data");
		break;
	case IEEE80211_FC0_TYPE_MGT:
		printk(" %s", ieee80211_mgt_subtype_name[(wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK)
							 >> IEEE80211_FC0_SUBTYPE_SHIFT]);
		break;
	default:
		printk(" type#%d", wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK);
		break;
	}
	if (IEEE80211_QOS_HAS_SEQ(wh)) {
		const struct ieee80211_qosframe *qwh = (const struct ieee80211_qosframe *)buf;
		printk(" QoS [TID %u%s]", qwh->i_qos[0] & IEEE80211_QOS_TID, qwh->i_qos[0] & IEEE80211_QOS_ACKPOLICY ? " ACM" : "");
	}
	if (wh->i_fc[1] & IEEE80211_FC1_PROT) {
		int off;

		off = ieee80211_anyhdrspace(ic, wh);
		printk(" WEP [IV %.02x %.02x %.02x", buf[off + 0], buf[off + 1], buf[off + 2]);
		if (buf[off + IEEE80211_WEP_IVLEN] & IEEE80211_WEP_EXTIV)
			printk(" %.02x %.02x %.02x", buf[off + 4], buf[off + 5], buf[off + 6]);
		printk(" KID %u]", buf[off + IEEE80211_WEP_IVLEN] >> 6);
	}
	if (rate >= 0)
		printk(" %dM", rate / 2);
	if (rssi >= 0)
		printk(" +%d", rssi);
	printk("\n");
	if (len > 0) {
		for (i = 0; i < len; i++) {
			if ((i % 8) == 0)
				printk(" ");
			if ((i % 16) == 0)
				printk("\n");
			printk("%02x ", buf[i]);
		}
		printk("\n\n");
	}
}

EXPORT_SYMBOL(ieee80211_dump_pkt);
#endif

/*
 * CN Policy module support.
 */
static const struct cn_policy *policy = NULL;

void cn_policy_register(const struct cn_policy *my_policy)
{
	printk(KERN_INFO "CN: Policy Module registered\n");
	policy = my_policy;
}

EXPORT_SYMBOL(cn_policy_register);

void cn_policy_unregister(const struct cn_policy *my_policy)
{
	if (policy == my_policy)
		policy = NULL;
	printk(KERN_INFO "CN: Policy Module unregistered\n");
}

EXPORT_SYMBOL(cn_policy_unregister);

const struct cn_policy *cn_policy_get(const char *name)
{
	if (policy == NULL)
		ieee80211_load_module("wlan_cn_policy");
	return policy && strcmp("policy", name) == 0 ? policy : NULL;
}

EXPORT_SYMBOL(cn_policy_get);

/* End CN Policy Module support. */

/*
 * CN Idle module support.
 */
static const struct cn_idle *idle = NULL;

void cn_idle_register(const struct cn_idle *my_idle)
{
	printk(KERN_INFO "CN: Idle Module registered\n");
	idle = my_idle;
}

EXPORT_SYMBOL(cn_idle_register);

void cn_idle_unregister(const struct cn_idle *my_idle)
{
	if (idle == idle)
		idle = NULL;
	printk(KERN_INFO "CN: Idle Module unregistered\n");
}

EXPORT_SYMBOL(cn_idle_unregister);

const struct cn_idle *cn_idle_get(const char *name)
{
	if (idle == NULL)
		ieee80211_load_module("wlan_cn_idle");
	return idle && strcmp("idle", name) == 0 ? idle : NULL;
}

EXPORT_SYMBOL(cn_idle_get);

/* End CN Idle Module support. */

/*
 * CN Polling module support.
 */
static const struct cn_polling *polling = NULL;

void cn_polling_register(const struct cn_polling *my_polling)
{
	printk(KERN_INFO "CN: Polling Queue registered\n");
	polling = my_polling;
}

EXPORT_SYMBOL(cn_polling_register);

void cn_polling_unregister(const struct cn_polling *my_polling)
{
	if (polling == my_polling)
		polling = NULL;
	printk(KERN_INFO "CN: Polling Queue unregistered\n");
}

EXPORT_SYMBOL(cn_polling_unregister);

const struct cn_polling *cn_polling_get(const char *name)
{
	if (polling == NULL)
		ieee80211_load_module("wlan_cn_polling");
	return polling && strcmp("polling", name) == 0 ? polling : NULL;
}

EXPORT_SYMBOL(cn_polling_get);

/* End CN Polling Module support. */

int ieee80211_fix_rate(struct ieee80211_node *ni, int flags)
{
#define	RV(v)	((v) & IEEE80211_RATE_VAL)
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	int i, j, ignore, error;
	int okrate, badrate, fixedrate;
	struct ieee80211_rateset *srs, *nrs;
	u_int8_t r;
	flags |= IEEE80211_F_DONEGO;
	flags |= IEEE80211_F_DODEL;
	error = 0;
	okrate = badrate = fixedrate = 0;
	srs = &ic->ic_sup_rates[ieee80211_chan2ratemode(ic->ic_curchan, -1)];
	nrs = &ni->ni_rates;
	fixedrate = IEEE80211_FIXED_RATE_NONE;
	for (i = 0; i < nrs->rs_nrates;) {
		ignore = 0;
		if (flags & IEEE80211_F_DOSORT) {
			/*
			 * Sort rates.
			 */
			for (j = i + 1; j < nrs->rs_nrates; j++) {
				if (RV(nrs->rs_rates[i]) > RV(nrs->rs_rates[j])) {
					r = nrs->rs_rates[i];
					nrs->rs_rates[i] = nrs->rs_rates[j];
					nrs->rs_rates[j] = r;
				}
			}
		}
		r = nrs->rs_rates[i] & IEEE80211_RATE_VAL;
		badrate = r;

		/*
		 * remove 0 rates
		 * they don't make sense and can lead to trouble later
		 */
		if (r == 0) {
			nrs->rs_nrates--;
			for (j = i; j < nrs->rs_nrates; j++)
				nrs->rs_rates[j] = nrs->rs_rates[j + 1];
			nrs->rs_rates[j] = 0;
			continue;
		}

		/*
		 * Check for fixed rate. 
		 */
		if (r == vap->iv_fixed_rate)
			fixedrate = r;
		if (flags & IEEE80211_F_DONEGO) {
			/*
			 * Check against supported rates.
			 */
			for (j = 0; j < srs->rs_nrates; j++) {
				if (r == RV(srs->rs_rates[j])) {
					/*
					 * Overwrite with the supported rate
					 * value so any basic rate bit is set.
					 * This ensures that response we send
					 * to stations have the necessary basic
					 * rate bit set.
					 */
					nrs->rs_rates[i] = srs->rs_rates[j];
					break;
				}
			}
			if (j == srs->rs_nrates) {
				/*
				 * A rate in the node's rate set is not
				 * supported.  If this is a basic rate and we
				 * are operating as an AP then this is an error.
				 * Otherwise we just discard/ignore the rate.
				 * Note that this is important for 11b stations
				 * when they want to associate with an 11g AP.
				 */
				if (vap->iv_opmode == IEEE80211_M_HOSTAP && (nrs->rs_rates[i] & IEEE80211_RATE_BASIC))
					error++;
				ignore++;
			}
		}
		if (flags & IEEE80211_F_DODEL || flags & IEEE80211_F_DONEGO) {
			/*
			 * Delete unacceptable rates.
			 */
			if (ignore) {
				nrs->rs_nrates--;
				for (j = i; j < nrs->rs_nrates; j++)
					nrs->rs_rates[j] = nrs->rs_rates[j + 1];
				nrs->rs_rates[j] = 0;
				continue;
			}
		}
		if (!ignore)
			okrate = nrs->rs_rates[i];
		i++;
	}
	if (okrate == 0 || error != 0 || ((flags & IEEE80211_F_DOFRATE) && fixedrate != vap->iv_fixed_rate))
		return badrate | IEEE80211_RATE_BASIC;
	else
		return RV(okrate);
#undef RV
}

/*
 * Reset 11g-related state.
 */
void ieee80211_reset_erp(struct ieee80211com *ic, enum ieee80211_phymode mode)
{
#define	IS_11G(m) \
	((m) == IEEE80211_MODE_11G || (m) == IEEE80211_MODE_TURBO_G)

	ic->ic_flags &= ~IEEE80211_F_USEPROT;
	/*
	 * Preserve the long slot and nonerp station count if
	 * switching between 11g and turboG. Otherwise, inactivity
	 * will cause the turbo station to disassociate and possibly
	 * try to leave the network.
	 * XXX not right if really trying to reset state
	 */
	if (IS_11G(mode) ^ IS_11G(ic->ic_curmode)) {
		ic->ic_nonerpsta = 0;
		ic->ic_longslotsta = 0;
	}

	/*
	 * Short slot time is enabled only when operating in 11g
	 * and not in an IBSS.  We must also honor whether or not
	 * the driver is capable of doing it.
	 */
	ieee80211_set_shortslottime(ic, IEEE80211_IS_CHAN_A(ic->ic_curchan) || (IEEE80211_IS_CHAN_ANYG(ic->ic_curchan) && ic->ic_opmode == IEEE80211_M_HOSTAP && (ic->ic_caps & IEEE80211_C_SHSLOT)));
	/*
	 * Set short preamble and ERP barker-preamble flags.
	 */
	if (IEEE80211_IS_CHAN_A(ic->ic_curchan) || (ic->ic_caps & IEEE80211_C_SHPREAMBLE)) {
		ic->ic_flags |= IEEE80211_F_SHPREAMBLE;
		ic->ic_flags &= ~IEEE80211_F_USEBARKER;
	} else {
		ic->ic_flags &= ~IEEE80211_F_SHPREAMBLE;
		ic->ic_flags |= IEEE80211_F_USEBARKER;
	}
#undef IS_11G
}

/*
 * Set the short slot time state and notify the driver.
 */
void ieee80211_set_shortslottime(struct ieee80211com *ic, int onoff)
{
	if (onoff)
		ic->ic_flags |= IEEE80211_F_SHSLOT;
	else
		ic->ic_flags &= ~IEEE80211_F_SHSLOT;
	/* notify driver */
	if (ic->ic_updateslot != NULL)
		ic->ic_updateslot(ic->ic_dev);
}

/*
 * Check if the specified rate set supports ERP.
 * NB: the rate set is assumed to be sorted.
 */
int ieee80211_iserp_rateset(struct ieee80211com *ic, struct ieee80211_rateset *rs)
{
	static const int rates[] = { 2, 4, 11, 22, 12, 24, 48 };
	int i, j;

	if (rs->rs_nrates < ARRAY_SIZE(rates))
		return 0;
	for (i = 0; i < ARRAY_SIZE(rates); i++) {
		for (j = 0; j < rs->rs_nrates; j++) {
			int r = rs->rs_rates[j] & IEEE80211_RATE_VAL;
			if (rates[i] == r)
				goto next;
			if (r > rates[i])
				return 0;
		}
		return 0;
	      next:
		;
	}
	return 1;
}

static const struct ieee80211_rateset basic11g[IEEE80211_MODE_MAX] = {
	{ 0 },			/* IEEE80211_MODE_AUTO */
	{ 3, { 12, 24, 48} },	/* IEEE80211_MODE_11A */
	{ 2, { 2, 4} },		/* IEEE80211_MODE_11B */
	{ 4, { 2, 4, 11, 22} },	/* IEEE80211_MODE_11G (mixed b/g) */
	{ 0 },			/* IEEE80211_MODE_FH */
	{ 3, { 12, 24, 48} },	/* IEEE80211_MODE_TURBO_A */
	{ 4, { 2, 4, 11, 22} },	/* IEEE80211_MODE_TURBO_G (mixed b/g) */
};

static const struct ieee80211_rateset basicpureg[] = {
	{ 7, { 2, 4, 11, 22, 12, 24, 48} },
};

/*
 * Mark basic rates for the 11g rate table based on the pureg setting
 */
void ieee80211_setpuregbasicrates(struct ieee80211_rateset *rs)
{
	int i, j;

	for (i = 0; i < rs->rs_nrates; i++) {
		rs->rs_rates[i] &= IEEE80211_RATE_VAL;
		for (j = 0; j < basicpureg[0].rs_nrates; j++)
			if (basicpureg[0].rs_rates[j] == rs->rs_rates[i]) {
				rs->rs_rates[i] |= IEEE80211_RATE_BASIC;
				break;
			}
	}
}

/*
 * Mark the basic rates for the 11g rate table based on the
 * specified mode.  For 11b compatibility we mark only 11b
 * rates.  There's also a pseudo 11a-mode used to mark only
 * the basic OFDM rates; this is used to exclude 11b stations
 * from an 11g bss.
 */
void ieee80211_set11gbasicrates(struct ieee80211_rateset *rs, enum ieee80211_phymode mode)
{
	int i, j;

	KASSERT(mode < IEEE80211_MODE_MAX, ("invalid mode %u", mode));
	for (i = 0; i < rs->rs_nrates; i++) {
		rs->rs_rates[i] &= IEEE80211_RATE_VAL;
		for (j = 0; j < basic11g[mode].rs_nrates; j++)
			if (basic11g[mode].rs_rates[j] == rs->rs_rates[i]) {
				rs->rs_rates[i] |= IEEE80211_RATE_BASIC;
				break;
			}
	}
}

/*
 * Deduce the 11g setup by examining the rates
 * that are marked basic.
 */
enum ieee80211_phymode ieee80211_get11gbasicrates(struct ieee80211_rateset *rs)
{
	struct ieee80211_rateset basic;
	int i;

	memset(&basic, 0, sizeof(basic));
	for (i = 0; i < rs->rs_nrates; i++)
		if (rs->rs_rates[i] & IEEE80211_RATE_BASIC)
			basic.rs_rates[basic.rs_nrates++] = rs->rs_rates[i] & IEEE80211_RATE_VAL;
	for (i = 0; i < IEEE80211_MODE_MAX; i++)
		if (memcmp(&basic, &basic11g[i], sizeof(basic)) == 0)
			return i;
	return IEEE80211_MODE_AUTO;
}

void ieee80211_wme_initparams(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	IEEE80211_LOCK_IRQ(ic);
	ieee80211_wme_initparams_locked(vap);
	IEEE80211_UNLOCK_IRQ(ic);
}

void ieee80211_wme_initparams_locked(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_wme_state *wme = &ic->ic_wme;
	struct phyParamType {
		u_int8_t aifsn;
		u_int8_t logcwmin;
		u_int8_t logcwmax;
		u_int16_t txopLimit;
		u_int8_t acm;
	};
	enum ieee80211_phymode mode;

	struct phyParamType *pPhyParam, *pBssPhyParam;

	static struct phyParamType phyParamForAC_BE[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 3, 4, 6, 0, 0 },
		/* IEEE80211_MODE_11A   */ { 3, 4, 6, 0, 0 },
		/* IEEE80211_MODE_11B   */ { 3, 5, 7, 0, 0 },
		/* IEEE80211_MODE_11G   */ { 3, 4, 6, 0, 0 },
		/* IEEE80211_MODE_FH    */ { 3, 5, 7, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 3, 5, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 3, 5, 0, 0 }
	};
	static struct phyParamType phyParamForAC_BK[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 7, 4, 10, 0, 0 },
		/* IEEE80211_MODE_11A   */ { 7, 4, 10, 0, 0 },
		/* IEEE80211_MODE_11B   */ { 7, 5, 10, 0, 0 },
		/* IEEE80211_MODE_11G   */ { 7, 4, 10, 0, 0 },
		/* IEEE80211_MODE_FH    */ { 7, 5, 10, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 7, 3, 10, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 7, 3, 10, 0, 0 }
	};
	static struct phyParamType phyParamForAC_VI[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 1, 3, 4, 94, 0 },
		/* IEEE80211_MODE_11A   */ { 1, 3, 4, 94, 0 },
		/* IEEE80211_MODE_11B   */ { 1, 4, 5, 188, 0 },
		/* IEEE80211_MODE_11G   */ { 1, 3, 4, 94, 0 },
		/* IEEE80211_MODE_FH    */ { 1, 4, 5, 188, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 3, 94, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 3, 94, 0 }
	};
	static struct phyParamType phyParamForAC_VO[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 1, 2, 3, 47, 0 },
		/* IEEE80211_MODE_11A   */ { 1, 2, 3, 47, 0 },
		/* IEEE80211_MODE_11B   */ { 1, 3, 4, 102, 0 },
		/* IEEE80211_MODE_11G   */ { 1, 2, 3, 47, 0 },
		/* IEEE80211_MODE_FH    */ { 1, 3, 4, 102, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 2, 47, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 2, 47, 0 }
	};

	static struct phyParamType bssPhyParamForAC_BE[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 3, 4, 10, 0, 0 },
		/* IEEE80211_MODE_11A   */ { 3, 4, 10, 0, 0 },
		/* IEEE80211_MODE_11B   */ { 3, 5, 10, 0, 0 },
		/* IEEE80211_MODE_11G   */ { 3, 4, 10, 0, 0 },
		/* IEEE80211_MODE_FH    */ { 3, 5, 10, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 3, 10, 0, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 3, 10, 0, 0 }
	};
	static struct phyParamType bssPhyParamForAC_VI[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 2, 3, 4, 94, 0 },
		/* IEEE80211_MODE_11A   */ { 2, 3, 4, 94, 0 },
		/* IEEE80211_MODE_11B   */ { 2, 4, 5, 188, 0 },
		/* IEEE80211_MODE_11G   */ { 2, 3, 4, 94, 0 },
		/* IEEE80211_MODE_FH    */ { 2, 4, 5, 188, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 2, 3, 94, 0 },
		/* IEEE80211_MODE_TURBO */ { 2, 2, 3, 94, 0 }
	};
	static struct phyParamType bssPhyParamForAC_VO[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 2, 2, 3, 47, 0 },
		/* IEEE80211_MODE_11A   */ { 2, 2, 3, 47, 0 },
		/* IEEE80211_MODE_11B   */ { 2, 3, 4, 102, 0 },
		/* IEEE80211_MODE_11G   */ { 2, 2, 3, 47, 0 },
		/* IEEE80211_MODE_FH    */ { 2, 3, 4, 102, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 2, 47, 0 },
		/* IEEE80211_MODE_TURBO */ { 1, 2, 2, 47, 0 }
	};

	int i;

	IEEE80211_LOCK_ASSERT(ic);

	if ((ic->ic_caps & IEEE80211_C_WME) == 0)
		return;
	/*
	 * Select mode; we can be called early in which case we
	 * always use auto mode.  We know we'll be called when
	 * entering the RUN state with bsschan setup properly
	 * so state will eventually get set correctly
	 */
	if (ic->ic_bsschan != IEEE80211_CHAN_ANYC)
		mode = ieee80211_chan2mode(ic->ic_bsschan);
	else
		mode = IEEE80211_MODE_AUTO;
	for (i = 0; i < WME_NUM_AC; i++) {
		switch (i) {
		case WME_AC_BK:
			pPhyParam = &phyParamForAC_BK[mode];
			pBssPhyParam = &phyParamForAC_BK[mode];
			break;
		case WME_AC_VI:
			pPhyParam = &phyParamForAC_VI[mode];
			pBssPhyParam = &bssPhyParamForAC_VI[mode];
			break;
		case WME_AC_VO:
			pPhyParam = &phyParamForAC_VO[mode];
			pBssPhyParam = &bssPhyParamForAC_VO[mode];
			break;
		case WME_AC_BE:
		default:
			pPhyParam = &phyParamForAC_BE[mode];
			pBssPhyParam = &bssPhyParamForAC_BE[mode];
			break;
		}

		if (ic->ic_opmode == IEEE80211_M_HOSTAP) {
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_acm = pPhyParam->acm;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_aifsn = pPhyParam->aifsn;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmin = pPhyParam->logcwmin;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmax = pPhyParam->logcwmax;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_txopLimit = pPhyParam->txopLimit;
		} else {
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_acm = pBssPhyParam->acm;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_aifsn = pBssPhyParam->aifsn;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmin = pBssPhyParam->logcwmin;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmax = pBssPhyParam->logcwmax;
			wme->wme_wmeChanParams.cap_wmeParams[i].wmep_txopLimit = pBssPhyParam->txopLimit;
		}
		wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_acm = pBssPhyParam->acm;
		wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_aifsn = pBssPhyParam->aifsn;
		wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_logcwmin = pBssPhyParam->logcwmin;
		wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_logcwmax = pBssPhyParam->logcwmax;
		wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_txopLimit = pBssPhyParam->txopLimit;
	}
	/* NB: check ic_bss to avoid NULL deref on initial attach */
	if (vap->iv_bss != NULL) {
		/*
		 * Calculate aggressive mode switching threshold based
		 * on beacon interval.
		 */
		wme->wme_hipri_switch_thresh = (HIGH_PRI_SWITCH_THRESH * vap->iv_bss->ni_intval) / 100;
		ieee80211_wme_updateparams_locked(vap);
	}
}

/*
 * Update WME parameters for ourself and the BSS.
 */
void ieee80211_wme_updateparams_locked(struct ieee80211vap *vap)
{
	static const struct {
		u_int8_t aifsn;
		u_int8_t logcwmin;
		u_int8_t logcwmax;
		u_int16_t txopLimit;
	} phyParam[IEEE80211_MODE_MAX] = {
		/* IEEE80211_MODE_AUTO  */ { 2, 4, 10, 64 },
		/* IEEE80211_MODE_11A   */ { 2, 4, 10, 64 },
		/* IEEE80211_MODE_11B   */ { 2, 5, 10, 64 },
		/* IEEE80211_MODE_11G   */ { 2, 4, 10, 64 },
		/* IEEE80211_MODE_FH    */ { 2, 5, 10, 64 },
		/* IEEE80211_MODE_TURBO */ { 1, 3, 10, 64 }
	};
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_wme_state *wme = &ic->ic_wme;
	enum ieee80211_phymode mode;
	int i;

	IEEE80211_LOCK_ASSERT(vap->iv_ic);

	/* set up the channel access parameters for the physical device */

	for (i = 0; i < WME_NUM_AC; i++) {
		wme->wme_chanParams.cap_wmeParams[i].wmep_aifsn = wme->wme_wmeChanParams.cap_wmeParams[i].wmep_aifsn;
		wme->wme_chanParams.cap_wmeParams[i].wmep_logcwmin = wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmin;
		wme->wme_chanParams.cap_wmeParams[i].wmep_logcwmax = wme->wme_wmeChanParams.cap_wmeParams[i].wmep_logcwmax;
		wme->wme_chanParams.cap_wmeParams[i].wmep_txopLimit = wme->wme_wmeChanParams.cap_wmeParams[i].wmep_txopLimit;
		wme->wme_bssChanParams.cap_wmeParams[i].wmep_aifsn = wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_aifsn;
		wme->wme_bssChanParams.cap_wmeParams[i].wmep_logcwmin = wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_logcwmin;
		wme->wme_bssChanParams.cap_wmeParams[i].wmep_logcwmax = wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_logcwmax;
		wme->wme_bssChanParams.cap_wmeParams[i].wmep_txopLimit = wme->wme_wmeBssChanParams.cap_wmeParams[i].wmep_txopLimit;
	}

	/*
	 * Select mode; we can be called early in which case we
	 * always use auto mode.  We know we'll be called when
	 * entering the RUN state with bsschan setup properly
	 * so state will eventually get set correctly
	 */
	if (ic->ic_bsschan != IEEE80211_CHAN_ANYC)
		mode = ieee80211_chan2mode(ic->ic_bsschan);
	else
		mode = IEEE80211_MODE_AUTO;
	if ((vap->iv_opmode == IEEE80211_M_HOSTAP &&
	     (wme->wme_flags & WME_F_AGGRMODE) != 0) || (vap->iv_opmode != IEEE80211_M_HOSTAP && (vap->iv_bss->ni_flags & IEEE80211_NODE_QOS) == 0) || (vap->iv_flags & IEEE80211_F_WME) == 0) {
		struct ieee80211vap *tmpvap;
		u_int8_t burstEnabled = 0;
		/* check if bursting  enabled on at least one vap */
		TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
			if (tmpvap->iv_ath_cap & IEEE80211_ATHC_BURST) {
				burstEnabled = 1;
				break;
			}
		}
		wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_aifsn = phyParam[mode].aifsn;
		wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin = phyParam[mode].logcwmin;
		wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmax = phyParam[mode].logcwmax;
		wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_txopLimit = burstEnabled ? phyParam[mode].txopLimit : 0;
		wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_aifsn = phyParam[mode].aifsn;
		wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin = phyParam[mode].logcwmin;
		wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmax = phyParam[mode].logcwmax;
		wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_txopLimit = burstEnabled ? phyParam[mode].txopLimit : 0;
	}

	if (ic->ic_opmode == IEEE80211_M_HOSTAP && ic->ic_sta_assoc < 2 && (wme->wme_flags & WME_F_AGGRMODE) != 0) {
		static const u_int8_t logCwMin[IEEE80211_MODE_MAX] = {
			/* IEEE80211_MODE_AUTO  */ 3,
			/* IEEE80211_MODE_11A   */ 3,
			/* IEEE80211_MODE_11B   */ 4,
			/* IEEE80211_MODE_11G   */ 3,
			/* IEEE80211_MODE_FH    */ 4,
			/* IEEE80211_MODE_TURBO_A */ 3,
			/* IEEE80211_MODE_TURBO_G */ 3
		};

		wme->wme_chanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin = wme->wme_bssChanParams.cap_wmeParams[WME_AC_BE].wmep_logcwmin = logCwMin[mode];
	}
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {	/* XXX ibss? */
		/*
		 * Arrange for a beacon update and bump the parameter
		 * set number so associated stations load the new values.
		 */
		wme->wme_bssChanParams.cap_info_count = (wme->wme_bssChanParams.cap_info_count + 1) & WME_QOSINFO_COUNT;
		vap->iv_flags |= IEEE80211_F_WMEUPDATE;
	}

	wme->wme_update(ic);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_WME, "%s: WME params updated, cap_info 0x%x\n", __func__, vap->iv_opmode == IEEE80211_M_STA ? wme->wme_wmeChanParams.cap_info_count : wme->wme_bssChanParams.cap_info_count);
}

EXPORT_SYMBOL(ieee80211_wme_updateparams);

void ieee80211_wme_updateparams(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	if (ic->ic_caps & IEEE80211_C_WME) {
		IEEE80211_LOCK_IRQ(ic);
		ieee80211_wme_updateparams_locked(vap);
		IEEE80211_UNLOCK_IRQ(ic);
	}
}

/*
 * Start a vap.  If this is the first vap running on the
 * underlying device then we first bring it up.
 */
int ieee80211_init(struct net_device *dev, int forcescan)
{
#define	IS_RUNNING(_dev) \
	((_dev->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))
	struct ieee80211vap *vap = netdev_priv(dev);
	struct ieee80211com *ic = vap->iv_ic;
	struct net_device *parent = ic->ic_dev;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "start running (state=%d)\n", vap->iv_state);

	if (vap->iv_master && vap->iv_master->iv_state == IEEE80211_S_INIT) {
		int ret = ieee80211_init(vap->iv_master->iv_dev, forcescan);
		if (ret < 0)
			return ret;
	}

	if ((dev->flags & IFF_RUNNING) == 0) {
		if (ic->ic_nopened++ == 0 && (parent->flags & IFF_RUNNING) == 0)
			dev_open(parent);
		/*
		 * Mark us running.  Note that we do this after
		 * opening the parent device to avoid recursion.
		 */
		dev->flags |= IFF_RUNNING;	/* mark us running */
	}
	/*
	 * If the parent is up and running, then kick the
	 * 802.11 state machine as appropriate.
	 * XXX parent should always be up+running
	 */
	if (IS_RUNNING(ic->ic_dev)) {
		if (vap->iv_opmode == IEEE80211_M_STA) {
			if (ic->ic_roaming != IEEE80211_ROAMING_MANUAL) {
				/* Try to be intelligent about clocking the 
				 * state machine.  If we're currently in RUN 
				 * state then we should be able to apply any 
				 * new state/parameters simply by 
				 * re-associating.  Otherwise we need to 
				 * re-scan to select an appropriate ap. */
				if (vap->iv_state != IEEE80211_S_RUN || forcescan) {
					IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "Bringing vap %p[%s] " "to %s\n", vap, vap->iv_nickname, ieee80211_state_name[IEEE80211_S_SCAN]);
					ieee80211_new_state(vap, IEEE80211_S_SCAN, 0);
				} else {
					IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "Bringing vap %p[%s] " "to %s\n", vap, vap->iv_nickname, ieee80211_state_name[IEEE80211_S_ASSOC]);
					ieee80211_new_state(vap, IEEE80211_S_ASSOC, 1);
				}
			}
		} else {
			/*
			 * When the old state is running the vap must 
			 * be brought to init.
			 */
			if (vap->iv_state == IEEE80211_S_RUN) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "Bringing vap %p[%s] to %s\n", vap, vap->iv_nickname, ieee80211_state_name[IEEE80211_S_INIT]);
				ieee80211_new_state(vap, IEEE80211_S_INIT, -1);
			}
			/*
			 * For monitor+wds modes there's nothing to do but
			 * start running.  Otherwise, if this is the first
			 * vap to be brought up, start a scan which may be
			 * preempted if the station is locked to a particular
			 * channel.
			 */
			if (vap->iv_opmode == IEEE80211_M_MONITOR || vap->iv_opmode == IEEE80211_M_WDS) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "Bringing vap %p[%s] to %s\n", vap, vap->iv_nickname, ieee80211_state_name[IEEE80211_S_RUN]);
				ieee80211_new_state(vap, IEEE80211_S_RUN, -1);
			} else {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "Bringing vap %p[%s] to %s\n", vap, vap->iv_nickname, ieee80211_state_name[IEEE80211_S_SCAN]);
				ieee80211_new_state(vap, IEEE80211_S_SCAN, 0);
			}
		}
	}

	return 0;
#undef IS_RUNNING
}

int ieee80211_open(struct net_device *dev)
{
	struct ieee80211vap *vap = netdev_priv(dev);

	return ieee80211_init(dev, 0);
}

/*
 * Start all runnable VAPs on a device.
 */
void ieee80211_start_running(struct ieee80211com *ic)
{
	struct ieee80211vap *vap, *avp;
	struct net_device *dev;

	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		dev = vap->iv_dev;
		/* NB: avoid recursion */
		if ((dev->flags & IFF_UP) && !(dev->flags & IFF_RUNNING))
			ieee80211_open(dev);

		TAILQ_FOREACH(avp, &vap->iv_wdslinks, iv_wdsnext) {
			if (avp->iv_wdsnode && avp->iv_wdsnode->ni_subif == avp)
				continue;

			dev = avp->iv_dev;
			/* NB: avoid recursion */
			if ((dev->flags & IFF_UP) && !(dev->flags & IFF_RUNNING))
				ieee80211_open(dev);
		}
	}
}

EXPORT_SYMBOL(ieee80211_start_running);

/*
 * Stop a vap.  We force it down using the state machine
 * then mark its device not running.  If this is the last
 * vap running on the underlying device then we close it
 * too to ensure it will be properly initialized when the
 * next vap is brought up.
 */
int ieee80211_stop(struct net_device *dev)
{
	struct ieee80211vap *vap = netdev_priv(dev);
	struct ieee80211com *ic = vap->iv_ic;
	struct net_device *parent = ic->ic_dev;
	struct ieee80211_node *tni, *ni;
	struct ieee80211vap *avp;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "%s\n", "stop running");

	if (vap->iv_wdsnode && !vap->iv_wdsnode->ni_subif)
		ieee80211_unref_node(&vap->iv_wdsnode);

	/* stop wds interfaces */
	TAILQ_FOREACH(avp, &vap->iv_wdslinks, iv_next) {
		if (avp->iv_state != IEEE80211_S_INIT)
			ieee80211_stop(avp->iv_dev);
	}

	/* get rid of all wds nodes while we're still locked */
	do {
		ni = NULL;

		IEEE80211_NODE_TABLE_LOCK_IRQ(&ic->ic_sta);
		TAILQ_FOREACH(tni, &ic->ic_sta.nt_node, ni_list) {
			if (tni->ni_vap != vap)
				continue;
			if (!tni->ni_subif)
				continue;
			ni = tni;
			break;
		}
		IEEE80211_NODE_TABLE_UNLOCK_IRQ(&ic->ic_sta);

		if (!ni)
			break;

		ieee80211_node_leave(ni);
	} while (1);

	ieee80211_new_state(vap, IEEE80211_S_INIT, -1);
#ifdef LED2_PIN
	/* UBNT */
	if (vap->iv_opmode == IEEE80211_M_STA) {
		del_timer(&vap->iv_rssiupdate);
	}
	/* END UBNT */
#endif
	if (dev->flags & IFF_RUNNING) {
		dev->flags &= ~IFF_RUNNING;	/* mark us stopped */
		del_timer(&vap->iv_mgtsend);
		if (--ic->ic_nopened == 0 && (parent->flags & IFF_RUNNING))
			dev_close(parent);
	}
#ifdef ATH_SUPERG_XR
	/*
	 * also stop the XR vap. 
	 */
	if (vap->iv_xrvap && !(vap->iv_flags & IEEE80211_F_XR)) {
		ieee80211_stop(vap->iv_xrvap->iv_dev);
		del_timer(&vap->iv_xrvapstart);
		vap->iv_xrvap->iv_dev->flags = dev->flags;
	}
#endif
	return 0;
}

EXPORT_SYMBOL(ieee80211_stop);

/*
 * Stop all VAPs running on a device.
 */
void ieee80211_stop_running(struct ieee80211com *ic)
{
	struct ieee80211vap *vap, *avp;
	struct net_device *dev;

	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		dev = vap->iv_dev;
		if (dev->flags & IFF_RUNNING)	/* NB: avoid recursion */
			ieee80211_stop(dev);

		TAILQ_FOREACH(avp, &vap->iv_wdslinks, iv_wdsnext) {
			dev = avp->iv_dev;
			if (dev->flags & IFF_RUNNING)	/* NB: avoid recursion */
				ieee80211_stop(dev);
		}
	}
}

EXPORT_SYMBOL(ieee80211_stop_running);

#ifdef ATH_SUPERG_DYNTURBO
/*
 * Switch between turbo and non-turbo operating modes.
 * Use the specified channel flags to locate the new
 * channel, update 802.11 state, and then call back into
 * the driver to effect the change.
 */
void ieee80211_dturbo_switch(struct ieee80211com *ic, int newflags)
{
#ifdef IEEE80211_DEBUG
	/* XXX use first vap for debug flags */
	struct ieee80211vap *vap = TAILQ_FIRST(&ic->ic_vaps);
#endif
	struct ieee80211_channel *chan;
	unsigned long flags;

	chan = ieee80211_find_channel(ic, ic->ic_bsschan->ic_freq, newflags);
	if (chan == NULL) {	/* XXX should not happen */
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: no channel with freq %u flags 0x%x\n", __func__, ic->ic_bsschan->ic_freq, newflags);
		return;
	}

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG,
			  "%s: %s -> %s (freq %u flags 0x%x)\n", __func__, ieee80211_phymode_name[ieee80211_chan2mode(ic->ic_bsschan)], ieee80211_phymode_name[ieee80211_chan2mode(chan)], chan->ic_freq, chan->ic_flags);

	ic->ic_bsschan = chan;
	ic->ic_curchan = chan;
	ic->ic_set_channel(ic);
	spin_lock_irqsave(&channel_lock, flags);
	ieee80211_scan_set_bss_channel(ic, ic->ic_bsschan);
	spin_unlock_irqrestore(&channel_lock, flags);
	/* NB: do not need to reset ERP state because in sta mode */
}

EXPORT_SYMBOL(ieee80211_dturbo_switch);
#endif				/* ATH_SUPERG_DYNTURBO */

void ieee80211_beacon_miss(struct ieee80211com *ic)
{
	struct ieee80211vap *vap;

	if (ic->ic_flags & IEEE80211_F_SCAN) {
		/* XXX check ic_curchan != ic_bsschan? */
		return;
	}
	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		int count;

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE | IEEE80211_MSG_DEBUG, "%s\n", "beacon miss");

		/*
		 * Our handling is only meaningful for stations that are
		 * associated; any other conditions else will be handled
		 * through different means (e.g. the tx timeout on mgt frames).
		 */
		if (vap->iv_opmode != IEEE80211_M_STA || vap->iv_state != IEEE80211_S_RUN)
			continue;

		IEEE80211_LOCK_IRQ(ic);
		count = vap->iv_bmiss_count++;
		if (count) {
			/* if the counter was already above zero, reset it
			 * here, since we're going to do the bmiss handling
			 * in any case */
			vap->iv_bmiss_count = 0;
		} else {
			/* schedule the software beacon miss timer, it will be
			 * cancelled, if the probe request is acked */
			mod_timer(&vap->iv_swbmiss, jiffies + vap->iv_swbmiss_period);
		}
		IEEE80211_UNLOCK_IRQ(ic);

		if (!count) {
			ieee80211_send_probereq(vap->iv_bss, vap->iv_myaddr, vap->iv_bss->ni_bssid, vap->iv_bss->ni_bssid, vap->iv_bss->ni_essid, vap->iv_bss->ni_esslen, NULL, 0);
			continue;
		}

		if (ic->ic_roaming == IEEE80211_ROAMING_AUTO) {
#ifdef ATH_SUPERG_DYNTURBO
			/* 
			 * If we receive a beacon miss interrupt when using
			 * dynamic turbo, attempt to switch modes before
			 * reassociating.
			 */
			if (IEEE80211_ATH_CAP(vap, vap->iv_bss, IEEE80211_ATHC_TURBOP))
				ieee80211_dturbo_switch(ic, ic->ic_bsschan->ic_flags ^ IEEE80211_CHAN_TURBO);
#endif				/* ATH_SUPERG_DYNTURBO */
			/*
			 * Try to reassociate before scanning for a new ap.
			 */
			ieee80211_new_state(vap, IEEE80211_S_ASSOC, 1);
		} else {
			/*
			 * Somebody else is controlling state changes (e.g.
			 * a user-mode app) don't do anything that would
			 * confuse them; just drop into scan mode so they'll
			 * notified of the state change and given control.
			 */
			ieee80211_new_state(vap, IEEE80211_S_SCAN, 0);
		}
	}
}

EXPORT_SYMBOL(ieee80211_beacon_miss);

/*
 * STA software beacon timer callback. This is called
 * only when we have a series beacon misses.
 */
static void ieee80211_sta_swbmiss(unsigned long arg)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)arg;
	ieee80211_beacon_miss(vap->iv_ic);
}

/*
 * Per-ieee80211vap watchdog timer callback.  This
 * is used only to timeout the xmit of management frames.
 */
static void ieee80211_tx_timeout(unsigned long arg)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)arg;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: state %s%s\n", __func__, ieee80211_state_name[vap->iv_state], vap->iv_ic->ic_flags & IEEE80211_F_SCAN ? ", scan active" : "");

	if (vap->iv_state != IEEE80211_S_INIT && (vap->iv_ic->ic_flags & IEEE80211_F_SCAN) == 0) {
		/*
		 * NB: it's safe to specify a timeout as the reason here;
		 *     it'll only be used in the right state.
		 */
		ieee80211_new_state(vap, IEEE80211_S_SCAN, IEEE80211_SCAN_FAIL_TIMEOUT);
	}
}

static void sta_disassoc(void *arg, struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = arg;

	if (ni->ni_vap == vap && ni->ni_associd != 0) {
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DISASSOC, IEEE80211_REASON_ASSOC_LEAVE);
		ieee80211_node_leave(ni);
	}
}

static void sta_deauth(void *arg, struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = arg;

	if (ni->ni_vap == vap)
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_ASSOC_LEAVE);
}

/*
 * Context: softIRQ (tasklet) and process
 */
int ieee80211_new_state(struct ieee80211vap *vap, enum ieee80211_state nstate, int arg)
{
	struct ieee80211com *ic = vap->iv_ic;
	int rc;

	IEEE80211_VAPS_LOCK_IRQ(ic);
	rc = vap->iv_newstate(vap, nstate, arg);
	IEEE80211_VAPS_UNLOCK_IRQ(ic);
	return rc;
}

EXPORT_SYMBOL(ieee80211_new_state);

static int __ieee80211_newstate(struct ieee80211vap *vap, enum ieee80211_state nstate, int arg)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;
	enum ieee80211_state ostate;

	ostate = vap->iv_state;
	vap->iv_state = nstate;	/* state transition */
	del_timer(&vap->iv_mgtsend);
	if ((vap->iv_opmode != IEEE80211_M_HOSTAP) && (ostate != IEEE80211_S_SCAN) && !(vap->iv_flags_ext & IEEE80211_FEXT_SCAN_PENDING))
		ieee80211_cancel_scan(vap);	/* background scan */
	ni = vap->iv_bss;	/* NB: no reference held */
	switch (nstate) {
	case IEEE80211_S_INIT:
		switch (ostate) {
		case IEEE80211_S_INIT:
			break;
		case IEEE80211_S_RUN:
			switch (vap->iv_opmode) {
			case IEEE80211_M_STA:
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DISASSOC, IEEE80211_REASON_ASSOC_LEAVE);
				ieee80211_node_leave(ni);
				break;
			case IEEE80211_M_HOSTAP:
				ieee80211_iterate_nodes(&ic->ic_sta, sta_disassoc, vap);
				break;
			default:
				break;
			}
			goto reset;
		case IEEE80211_S_AUTH:
		case IEEE80211_S_ASSOC:
			switch (vap->iv_opmode) {
			case IEEE80211_M_STA:
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_AUTH_LEAVE);
				ieee80211_node_leave(ni);
				break;
			case IEEE80211_M_HOSTAP:
				ieee80211_iterate_nodes(&ic->ic_sta, sta_deauth, vap);
				break;
			default:
				break;
			}
			goto reset;
		case IEEE80211_S_SCAN:
			if (!(vap->iv_flags_ext & IEEE80211_FEXT_SCAN_PENDING))
				ieee80211_cancel_scan(vap);
			goto reset;
		      reset:
			ieee80211_reset_bss(vap);
			break;
		}
		if (vap->iv_auth->ia_detach != NULL)
			vap->iv_auth->ia_detach(vap);
		break;
	case IEEE80211_S_SCAN:
		switch (ostate) {
		case IEEE80211_S_INIT:
		      createibss:
			if ((vap->iv_opmode == IEEE80211_M_HOSTAP || vap->iv_opmode == IEEE80211_M_IBSS || vap->iv_opmode == IEEE80211_M_AHDEMO) && vap->iv_des_chan != IEEE80211_CHAN_ANYC) {
				/*
				 * AP operation and we already have a channel;
				 * bypass the scan and startup immediately.
				 */
				ieee80211_create_ibss(vap, vap->iv_des_chan);
			} else {
				ieee80211_check_scan(vap, IEEE80211_SCAN_ACTIVE | IEEE80211_SCAN_FLUSH, IEEE80211_SCAN_FOREVER, vap->iv_des_nssid, vap->iv_des_ssid, NULL);
			}
			break;
		case IEEE80211_S_SCAN:
		case IEEE80211_S_AUTH:
		case IEEE80211_S_ASSOC:
			/*
			 * These can happen either because of a timeout
			 * on an assoc/auth response or because of a
			 * change in state that requires a reset.  For
			 * the former we're called with a non-zero arg
			 * that is the cause for the failure; pass this
			 * to the scan code so it can update state.
			 * Otherwise trigger a new scan unless we're in
			 * manual roaming mode in which case an application
			 * must issue an explicit scan request.
			 */
			if (arg != 0)
				ieee80211_scan_assoc_fail(ic, vap->iv_bss->ni_macaddr, arg);
			if (ic->ic_roaming == IEEE80211_ROAMING_AUTO)
				ieee80211_check_scan(vap, IEEE80211_SCAN_ACTIVE, IEEE80211_SCAN_FOREVER, vap->iv_des_nssid, vap->iv_des_ssid, NULL);
			else
				ieee80211_node_leave(vap->iv_bss);
			break;
		case IEEE80211_S_RUN:	/* beacon miss */
			if (vap->iv_opmode == IEEE80211_M_STA) {
				ieee80211_node_leave(ni);
				vap->iv_flags &= ~IEEE80211_F_SIBSS;	/* XXX */
				if (ic->ic_roaming == IEEE80211_ROAMING_AUTO)
					ieee80211_check_scan(vap, IEEE80211_SCAN_ACTIVE, IEEE80211_SCAN_FOREVER, vap->iv_des_nssid, vap->iv_des_ssid, NULL);
			} else {
				ieee80211_iterate_nodes(&ic->ic_sta, sta_disassoc, vap);
				goto createibss;
			}
			break;
		}
		break;
	case IEEE80211_S_AUTH:
		vap->iv_lastconnect = jiffies;
		/* auth frames are possible between IBSS nodes, 
		 * see 802.11-1999, chapter 5.7.6 */
		KASSERT(vap->iv_opmode == IEEE80211_M_STA || vap->iv_opmode == IEEE80211_M_IBSS, ("switch to %s state when operating in mode %u", ieee80211_state_name[nstate], vap->iv_opmode));
		switch (ostate) {
		case IEEE80211_S_INIT:
		case IEEE80211_S_SCAN:
			IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, 1);
			break;
		case IEEE80211_S_AUTH:
		case IEEE80211_S_ASSOC:
			switch (arg) {
			case IEEE80211_FC0_SUBTYPE_AUTH:
				/* ??? */
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, 2);
				break;
			case IEEE80211_FC0_SUBTYPE_DEAUTH:
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, 1);
				break;
			}
			break;
		case IEEE80211_S_RUN:
			switch (arg) {
			case IEEE80211_FC0_SUBTYPE_AUTH:
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, 2);
				vap->iv_state = ostate;	/* stay RUN */
				break;
			case IEEE80211_FC0_SUBTYPE_DEAUTH:
				ieee80211_sta_leave(ni);
				if (ic->ic_roaming == IEEE80211_ROAMING_AUTO) {
					/* try to reauth */
					IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, 1);
				}
				break;
			}
			break;
		}
		break;
	case IEEE80211_S_ASSOC:
		KASSERT(vap->iv_opmode == IEEE80211_M_STA, ("switch to %s state when operating in mode %u", ieee80211_state_name[nstate], vap->iv_opmode));
		switch (ostate) {
		case IEEE80211_S_INIT:
		case IEEE80211_S_SCAN:
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_ANY, "%s: invalid transition\n", __func__);
			break;
		case IEEE80211_S_AUTH:
		case IEEE80211_S_ASSOC:
			IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_ASSOC_REQ, 0);
			break;
		case IEEE80211_S_RUN:
			if (ic->ic_roaming == IEEE80211_ROAMING_AUTO) {
				/* NB: caller specifies ASSOC/REASSOC by arg */
				IEEE80211_SEND_MGMT(ni, arg ? IEEE80211_FC0_SUBTYPE_REASSOC_REQ : IEEE80211_FC0_SUBTYPE_ASSOC_REQ, 0);
			}
			break;
		}
		break;
	case IEEE80211_S_RUN:
		if (vap->iv_flags & IEEE80211_F_WPA) {
			/* XXX validate prerequisites */
		}

		switch (ostate) {
		case IEEE80211_S_INIT:
			if (vap->iv_opmode == IEEE80211_M_MONITOR || vap->iv_opmode == IEEE80211_M_HOSTAP) {
				/*
				 * Already have a channel; bypass the
				 * scan and startup immediately.
				 */
				ieee80211_create_ibss(vap, ic->ic_curchan);
				break;
			}
			/* fall thru... */
		case IEEE80211_S_AUTH:
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_ANY, "%s: invalid transition\n", __func__);
			break;
		case IEEE80211_S_RUN:
			break;
		case IEEE80211_S_SCAN:	/* adhoc/hostap mode */
		case IEEE80211_S_ASSOC:	/* infra mode */
			KASSERT(ni->ni_txrate < ni->ni_rates.rs_nrates, ("%s: bogus xmit rate %u setup\n", __func__, ni->ni_txrate));
#ifdef IEEE80211_DEBUG
			if (ieee80211_msg_debug(vap)) {
				ieee80211_note(vap, "%s with " MAC_FMT " ssid ", (vap->iv_opmode == IEEE80211_M_STA ? "associated" : "synchronized "), MAC_ADDR(vap->iv_bssid));
				ieee80211_print_essid(vap->iv_bss->ni_essid, ni->ni_esslen);
				printk(" channel %d start %uMb\n", ieee80211_chan2ieee(ic, ic->ic_curchan), IEEE80211_RATE2MBS(ni->ni_rates.rs_rates[ni->ni_txrate]));
			}
#endif
			if (vap->iv_opmode == IEEE80211_M_STA) {
				//UBNT: Set RX_RATE to the MAX negotiated
				//keba: it would be nice to do through sc function like sc->set_rxrate(ni->ni_rxrate)
				struct ath_softc *sc = netdev_priv(ic->ic_dev);
				u_int8_t i;

				ni->ni_rxrate = (ni->ni_rates.rs_rates[ni->ni_rates.rs_nrates - 1] & IEEE80211_RATE_VAL);
				for (i = 0; i < (sizeof(sc->sc_hwmap) / sizeof(sc->sc_hwmap[0])); i++) {
					if (ni->ni_rxrate == (sc->sc_hwmap[i].ieeerate & IEEE80211_RATE_VAL)) {
						sc->sc_rxrate = i;
						break;
					}
				}
				ieee80211_scan_assoc_success(ic, ni->ni_macaddr);
				ieee80211_notify_node_join(ni, (arg == IEEE80211_FC0_SUBTYPE_ASSOC_RESP) | (arg == IEEE80211_FC0_SUBTYPE_REASSOC_RESP));
			}
			break;
		}

		/* WDS/Repeater: Start software beacon timer for STA */
		vap->iv_swbmiss.function = ieee80211_sta_swbmiss;
		vap->iv_swbmiss.data = (unsigned long)vap;
		vap->iv_swbmiss_period = IEEE80211_TU_TO_JIFFIES(vap->iv_ic->ic_bmissthreshold * ni->ni_intval);

		if (ostate != IEEE80211_S_RUN && (vap->iv_opmode == IEEE80211_M_STA && vap->iv_flags_ext & IEEE80211_FEXT_SWBMISS)) {
			mod_timer(&vap->iv_swbmiss, jiffies + vap->iv_swbmiss_period);
		}
#ifdef LED2_PIN
		/* UBNT */
		if ((vap->iv_dev->flags & IFF_RUNNING) && vap->iv_opmode == IEEE80211_M_STA && ostate != IEEE80211_S_RUN && nstate == IEEE80211_S_RUN) {
			vap->iv_rssiupdate.function = ieee80211_sta_probe_ap;
			vap->iv_rssiupdate.data = (unsigned long)vap;
			mod_timer(&vap->iv_rssiupdate, jiffies + HZ);
		}
		/* END UBNT */
#endif
		/*
		 * Start/stop the authenticator when operating as an
		 * AP.  We delay until here to allow configuration to
		 * happen out of order.
		 */
		/* XXX WDS? */
		if (vap->iv_opmode == IEEE80211_M_HOSTAP &&	/* XXX IBSS/AHDEMO */
		    vap->iv_auth->ia_attach != NULL) {
			/* XXX check failure */
			vap->iv_auth->ia_attach(vap);
		} else if (vap->iv_auth->ia_detach != NULL)
			vap->iv_auth->ia_detach(vap);
		/*
		 * When 802.1x is not in use mark the port authorized
		 * at this point so traffic can flow.
		 */
		if (ni->ni_authmode != IEEE80211_AUTH_8021X)
			ieee80211_node_authorize(ni);

#ifdef ATH_SUPERG_XR
		/*
		 * fire a timer to bring up XR vap if configured.
		 */
		if (ostate != IEEE80211_S_RUN && vap->iv_xrvap && !(vap->iv_flags & IEEE80211_F_XR)) {
			vap->iv_xrvapstart.function = ieee80211_start_xrvap;
			vap->iv_xrvapstart.data = (unsigned long)vap->iv_xrvap;
			mod_timer(&vap->iv_xrvapstart, jiffies + HZ);	/* start xr vap on next second */
			/* 
			 * do not let the normal vap automatically bring up XR vap.
			 * let the timer handler start the XR vap. if you let the
			 * normal vap automatically start the XR vap  normal vap will not
			 * have the bssid initialized and the XR vap will use the
			 * invalid bssid in XRIE of its beacon.
			 */
			if (vap->iv_xrvap->iv_flags_ext & IEEE80211_FEXT_SCAN_PENDING)
				vap->iv_xrvap->iv_flags_ext &= ~IEEE80211_FEXT_SCAN_PENDING;
		}
		/*
		 * when an XR vap transitions to RUN state,
		 * normal vap needs to update the XR IE
		 * with the xr vaps MAC address.
		 */
		if (vap->iv_flags & IEEE80211_F_XR)
			vap->iv_xrvap->iv_flags |= IEEE80211_F_XRUPDATE;
#endif
		break;
	}
	return 0;
}

/* Get the dominant state of the device (init, running, or scanning 
 * (and/or associating)) */
static int get_dominant_state(struct ieee80211com *ic)
{
	int nscanning = 0;
	int nrunning = 0;
	struct ieee80211vap *tmpvap;

	TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
		if (tmpvap->iv_opmode == IEEE80211_M_MONITOR)
			/* skip monitor vaps as their
			 * S_RUN shouldn't have any
			 * influence on modifying state
			 * transition */
			continue;
		if (tmpvap->iv_state == IEEE80211_S_RUN)
			nrunning++;
		else if (tmpvap->iv_state == IEEE80211_S_SCAN || tmpvap->iv_state == IEEE80211_S_AUTH || tmpvap->iv_state == IEEE80211_S_ASSOC) {
			KASSERT((nscanning <= 1), ("Two VAPs cannot scan at " "the same time\n"));
			nscanning++;
		}
	}
	KASSERT(!(nscanning && nrunning), ("SCAN and RUN can't happen at the " "same time\n"));
	KASSERT((nscanning <= 1), ("Two VAPs must not SCAN at the " "same time\n"));

	if (nrunning > 0)
		return IEEE80211_S_RUN;
	else if (nscanning > 0)
		return IEEE80211_S_SCAN;
	else
		return IEEE80211_S_INIT;
}

static void dump_vap_states(struct ieee80211com *ic, struct ieee80211vap *highlighed)
{
	/* RE-count the number of VAPs in RUN, SCAN states */
	int nrunning = 0;
	int nscanning = 0;
	struct ieee80211vap *tmpvap;
	TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
		IEEE80211_DPRINTF(tmpvap, IEEE80211_MSG_STATE,
				  "%s: VAP %s%p[%24s]%s = %s%s%s.\n", __func__,
				  (highlighed == tmpvap ? "*" : " "),
				  tmpvap, tmpvap->iv_nickname,
				  (highlighed == tmpvap ? "*" : " "),
				  ieee80211_state_name[tmpvap->iv_state],
				  (tmpvap->iv_state == IEEE80211_S_RUN) ?
				  "[RUNNING]" : "", (tmpvap->iv_state == IEEE80211_S_SCAN || tmpvap->iv_state == IEEE80211_S_AUTH || tmpvap->iv_state == IEEE80211_S_ASSOC) ? "[SCANNING]" : "");
		/* Ignore monitors they are passive */
		if (tmpvap->iv_opmode == IEEE80211_M_MONITOR) {
			continue;
		}
		if (tmpvap->iv_state == IEEE80211_S_RUN) {
			KASSERT((nscanning == 0), ("SCAN and RUN can't happen " "at the same time\n"));
			nrunning++;
		}
		if (tmpvap->iv_state == IEEE80211_S_SCAN ||
		    /* STA in WDS/Repeater */
		    tmpvap->iv_state == IEEE80211_S_AUTH || tmpvap->iv_state == IEEE80211_S_ASSOC) {
			KASSERT((nscanning == 0), ("Two VAPs cannot scan at " "the same time\n"));
			KASSERT((nrunning == 0), ("SCAN and RUN can't happen " "at the same time\n"));
			nscanning++;
		}
	}
}

static int ieee80211_newstate(struct ieee80211vap *vap, enum ieee80211_state nstate, int arg)
{
	struct ieee80211com *ic = vap->iv_ic;
	enum ieee80211_state ostate;
	enum ieee80211_state dstate;
	int blocked = 0;
	struct ieee80211vap *tmpvap;

	ostate = vap->iv_state;
	dstate = get_dominant_state(ic);
#ifdef LED2_PIN
	/* UBNT turn off rssi leds, whe device is not in running state */
	if ((nstate != IEEE80211_S_RUN) && (ic->ic_update_rssi_leds))
		ic->ic_update_rssi_leds(ic, 0);
#endif
	IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: %p[%s] %s -> %s (dominant %s)\n", __func__, vap, vap->iv_nickname, ieee80211_state_name[ostate], ieee80211_state_name[nstate], ieee80211_state_name[dstate]);

	ieee80211_update_link_status(vap, nstate, ostate);

	if ((nstate != IEEE80211_S_RUN) && vap->iv_wdsnode && !vap->iv_wdsnode->ni_subif)
		ieee80211_unref_node(&vap->iv_wdsnode);

	if (ostate == IEEE80211_S_RUN && nstate != IEEE80211_S_RUN && vap->iv_opmode == IEEE80211_M_STA && (vap->iv_flags_ext & IEEE80211_FEXT_WDS))
		del_timer_sync(&ic->ic_wdsprobe);

	switch (nstate) {
	case IEEE80211_S_AUTH:
	case IEEE80211_S_ASSOC:
	case IEEE80211_S_SCAN:
		switch (dstate) {
		case IEEE80211_S_RUN:
			if (vap->iv_opmode == IEEE80211_M_MONITOR || vap->iv_opmode == IEEE80211_M_WDS || vap->iv_opmode == IEEE80211_M_HOSTAP) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: Jumping directly to RUN " "on VAP %p [%s].\n", __func__, vap, vap->iv_nickname);
				/* One or more VAPs are running, so 
				 * non-station VAPs can skip SCAN/AUTH/ASSOC 
				 * states and just run. */
				__ieee80211_newstate(vap, IEEE80211_S_RUN, arg);
			} else {
				/* We'll use this flag briefly to mark 
				 * transition in progress */
				ic->ic_flags_ext |= IEEE80211_FEXT_SCAN_PENDING;
				/* IEEE80211_M_IBSS or IEEE80211_M_STA VAP
				 * is forced to scan, we need to change 
				 * all other VAPs state to S_INIT and pend for 
				 * the scan completion */
				TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
					if ((vap != tmpvap) && (tmpvap->iv_opmode != IEEE80211_M_MONITOR)) {
						IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: Setting " "SCAN_PENDING " "flag on " "VAP %p " "[%s].\n", __func__, tmpvap, tmpvap->iv_nickname);
						tmpvap->iv_flags_ext |= IEEE80211_FEXT_SCAN_PENDING;
						if (tmpvap->iv_state != IEEE80211_S_INIT) {
							IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: " "Forcing " "INIT " "state " "on " "VAP " "%p " "[%s].\n", __func__, tmpvap, tmpvap->iv_nickname);
							tmpvap->iv_newstate(tmpvap, IEEE80211_S_INIT, 0);
						} else {
							IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE, "%s: " "NOT " "forcing " "INIT " "state " "on " "VAP " "%p " "[%s].\n", __func__, tmpvap, tmpvap->iv_nickname);
						}
					}
				}
				/* We used this flag briefly to mark transition in progress */
				ic->ic_flags_ext &= ~IEEE80211_FEXT_SCAN_PENDING;
				/* Transition S_INIT -> S_SCAN */
				__ieee80211_newstate(vap, nstate, arg);
				break;
			}
			break;
		case IEEE80211_S_SCAN:
		case IEEE80211_S_AUTH:
		case IEEE80211_S_ASSOC:
			/* this VAP was scanning */
			/* STA in WDS/Repeater needs to bring up other VAPs */
			if (ostate == IEEE80211_S_SCAN || ostate == IEEE80211_S_AUTH || ostate == IEEE80211_S_ASSOC) {
				/* Transition (S_SCAN|S_AUTH|S_ASSOC) -> 
				 * S_SCAN */
				__ieee80211_newstate(vap, nstate, arg);
			} else {
				/* Someone else is scanning, so block the 
				 * transition */
				vap->iv_flags_ext |= IEEE80211_FEXT_SCAN_PENDING;
				__ieee80211_newstate(vap, IEEE80211_S_INIT, arg);
				blocked = 1;
			}
			break;
		case IEEE80211_S_INIT:
			/* Transition S_INIT -> S_SCAN */
			__ieee80211_newstate(vap, nstate, arg);
			break;
		}
		break;

	case IEEE80211_S_RUN:
		/* this VAP was scanning */
		/* STA in WDS/Repeater needs to bring up other VAPs  */
		if (ostate == IEEE80211_S_SCAN || ostate == IEEE80211_S_AUTH || ostate == IEEE80211_S_ASSOC) {

			/* Transition (S_SCAN|S_AUTH|S_ASSOC) -> S_RUN */
			__ieee80211_newstate(vap, nstate, arg);

			/* if we're in wds, let the ap know that we're doing this */
			if ((vap->iv_opmode == IEEE80211_M_STA) && (vap->iv_flags_ext & IEEE80211_FEXT_WDS))
				mod_timer(&ic->ic_wdsprobe, jiffies + HZ / 10);

			/* Then bring up all other vaps pending on the scan */
			dstate = get_dominant_state(ic);
			if (dstate == IEEE80211_S_RUN) {
				TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
					if ((vap != tmpvap) && (tmpvap->iv_opmode != IEEE80211_M_MONITOR) && (tmpvap->iv_flags_ext & IEEE80211_FEXT_SCAN_PENDING)) {
						IEEE80211_DPRINTF(vap,
								  IEEE80211_MSG_STATE,
								  "%s: Clearing " "SCAN_PENDING " "flag from VAP " "%p [%s] and " "transitioning " "to RUN state.\n", __func__, tmpvap, tmpvap->iv_nickname);
						tmpvap->iv_flags_ext &= ~IEEE80211_FEXT_SCAN_PENDING;
						if (tmpvap->iv_state != IEEE80211_S_RUN) {
							tmpvap->iv_newstate(tmpvap, IEEE80211_S_RUN, 0);
						} else if (tmpvap->iv_opmode == IEEE80211_M_HOSTAP) {
							/* Force other AP through 
							 * -> INIT -> RUN to make
							 * sure beacons are 
							 * reallocated */
							tmpvap->iv_newstate(tmpvap, IEEE80211_S_INIT, 0);
							tmpvap->iv_newstate(tmpvap, IEEE80211_S_RUN, 0);
						}
					}
				}
			}
		} else if ((dstate == IEEE80211_S_SCAN) || (dstate == IEEE80211_S_AUTH) || (dstate == IEEE80211_S_ASSOC)) {
			/* Force to scan pending... someone is scanning */
			vap->iv_flags_ext |= IEEE80211_FEXT_SCAN_PENDING;
			__ieee80211_newstate(vap, IEEE80211_S_INIT, arg);
			blocked = 1;
		} else {
			__ieee80211_newstate(vap, nstate, arg);
		}
		break;
	default:
		__ieee80211_newstate(vap, nstate, arg);
	}

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_STATE,
			  "%s: %s requested transition %s -> %s on VAP %p [%s].  "
			  "Dominant state is %s.\n", __func__, (blocked ? "BLOCKED" : "ALLOWED"), ieee80211_state_name[ostate], ieee80211_state_name[nstate], vap, vap->iv_nickname, ieee80211_state_name[dstate]);

	dump_vap_states(ic, vap);
	return 0;
}

#ifdef ATH_SUPERG_XR
/*
 * start the XR vap .
 * called from a timer when normal vap enters RUN state .
 */
static void ieee80211_start_xrvap(unsigned long data)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)data;
	/* make sure that the normal vap is still in RUN state */
	if (vap->iv_xrvap->iv_state == IEEE80211_S_RUN)
		ieee80211_init(vap->iv_dev, 0);
}

#endif
