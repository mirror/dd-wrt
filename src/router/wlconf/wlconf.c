/*
 * Wireless Network Adapter Configuration Utility
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <shutils.h>
#include <wlutils.h>

/* phy types */
#define	PHY_TYPE_A		0
#define	PHY_TYPE_B		1
#define	PHY_TYPE_G		2
#define	PHY_TYPE_N		4
#define	PHY_TYPE_LP		5
#define	PHY_TYPE_NULL		0xf

/* how many times to attempt to bring up a virtual i/f when
 * we are in APSTA mode and IOVAR set of "bss" "up" returns busy
 */
#define MAX_BSS_UP_RETRIES 5

/* notify the average dma xfer rate (in kbps) to the driver */
#define AVG_DMA_XFER_RATE 120000

/* parts of an idcode: */
#define	IDCODE_MFG_MASK		0x00000fff
#define	IDCODE_MFG_SHIFT	0
#define	IDCODE_ID_MASK		0x0ffff000
#define	IDCODE_ID_SHIFT		12
#define	IDCODE_REV_MASK		0xf0000000
#define	IDCODE_REV_SHIFT	28

/*
 * Debugging Macros
 */
#define WLCONF_DBG(fmt, arg...)
#define WL_IOCTL(name, cmd, buf, len)			(ret = wl_ioctl(name, cmd, buf, len))
#define WL_SETINT(name, cmd, val)			(ret = wlconf_setint(name, cmd, val))
#define WL_GETINT(name, cmd, pval)			(ret = wlconf_getint(name, cmd, pval))
#define WL_IOVAR_SET(ifname, iovar, param, paramlen)	(ret = wl_iovar_set(ifname, iovar, \
							param, paramlen))
#define WL_IOVAR_SETINT(ifname, iovar, val)		(ret = wl_iovar_setint(ifname, iovar, val))
#define WL_IOVAR_GETINT(ifname, iovar, val)		(ret = wl_iovar_getint(ifname, iovar, val))
#define WL_BSSIOVAR_SETBUF(ifname, iovar, bssidx, param, paramlen, buf, buflen) \
		(ret = wl_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, buf, buflen))
#define WL_BSSIOVAR_SET(ifname, iovar, bssidx, param, paramlen) \
		(ret = wl_bssiovar_set(ifname, iovar, bssidx, param, paramlen))
#define WL_BSSIOVAR_GET(ifname, iovar, bssidx, param, paramlen) \
		(ret = wl_bssiovar_get(ifname, iovar, bssidx, param, paramlen))
#define WL_BSSIOVAR_SETINT(ifname, iovar, bssidx, val)	(ret = wl_bssiovar_setint(ifname, iovar, \
			bssidx, val))

#ifdef BCMWPA2
#define CHECK_PSK(mode) ((mode) & (WPA_AUTH_PSK | WPA2_AUTH_PSK))
#else
#define CHECK_PSK(mode) ((mode) & WPA_AUTH_PSK)
#endif

/* prototypes */
struct bsscfg_list *wlconf_get_bsscfgs(char* ifname, char* prefix);
int wlconf(char *name);
int wlconf_down(char *name);

static int
wlconf_getint(char* ifname, int cmd, int *pval)
{
	return wl_ioctl(ifname, cmd, pval, sizeof(int));
}

static int
wlconf_setint(char* ifname, int cmd, int val)
{
	return wl_ioctl(ifname, cmd, &val, sizeof(int));
}

/* set WEP key */
static int
wlconf_set_wep_key(char *name, char *prefix, int bsscfg_idx, int i)
{
	wl_wsec_key_t key;
	char wl_key[] = "wlXXXXXXXXXX_keyXXXXXXXXXX";
	char *keystr, hex[] = "XX";
	unsigned char *data = key.data;
	int ret = 0;

	memset(&key, 0, sizeof(key));
	key.index = i - 1;
	sprintf(wl_key, "%skey%d", prefix, i);
	keystr = nvram_safe_get(wl_key);

	switch (strlen(keystr)) {
	case WEP1_KEY_SIZE:
	case WEP128_KEY_SIZE:
		key.len = strlen(keystr);
		strcpy((char *)key.data, keystr);
		break;
	case WEP1_KEY_HEX_SIZE:
	case WEP128_KEY_HEX_SIZE:
		key.len = strlen(keystr) / 2;
		while (*keystr) {
			strncpy(hex, keystr, 2);
			*data++ = (unsigned char) strtoul(hex, NULL, 16);
			keystr += 2;
		}
		break;
	default:
		key.len = 0;
		break;
	}

	/* Set current WEP key */
	if (key.len && i == atoi(nvram_safe_get(strcat_r(prefix, "key", wl_key))))
		key.flags = WL_PRIMARY_KEY;

	WL_BSSIOVAR_SET(name, "wsec_key", bsscfg_idx, &key, sizeof(key));

	return ret;
}

extern struct nvram_tuple router_defaults[];

/* Keep this table in order */
static struct {
	int locale;
	char **names;
	char *abbr;
} countries[] = {
	{ WLC_WW,  ((char *[]) { "Worldwide", "WW", NULL }), "AU" },
	{ WLC_THA, ((char *[]) { "Thailand", "THA", NULL }), "TH" },
	{ WLC_ISR, ((char *[]) { "Israel", "ISR", NULL }), "IL" },
	{ WLC_JDN, ((char *[]) { "Jordan", "JDN", NULL }), "JO" },
	{ WLC_PRC, ((char *[]) { "China", "P.R. China", "PRC", NULL }), "CN" },
	{ WLC_JPN, ((char *[]) { "Japan", "JPN", NULL }), "JP" },
	{ WLC_FCC, ((char *[]) { "USA", "Canada", "ANZ", "New Zealand", "FCC", NULL }), "US" },
	{ WLC_EUR, ((char *[]) { "Europe", "EUR", NULL }), "DE" },
	{ WLC_USL, ((char *[]) { "USA Low", "USALow", "USL", NULL }), "US" },
	{ WLC_JPH, ((char *[]) { "Japan High", "JapanHigh", "JPH", NULL }), "JP" },
	{ WLC_ALL, ((char *[]) { "ALL", "AllTheChannels", NULL }), "ALL" },
	};

/* validate/restore all per-interface related variables */
static void
wlconf_validate_all(char *prefix, bool restore)
{
	struct nvram_tuple *t;
	char tmp[100];
	char *v;
	for (t = router_defaults; t->name; t++) {
		if (!strncmp(t->name, "wl_", 3)) {
			strcat_r(prefix, &t->name[3], tmp);
			if (!restore && nvram_get(tmp))
				continue;
			v = nvram_get(t->name);
			nvram_set(tmp, v ? v : t->value);
		}
	}
}

/* restore specific per-interface variable */
static void
wlconf_restore_var(char *prefix, char *name)
{
	struct nvram_tuple *t;
	char tmp[100];
	for (t = router_defaults; t->name; t++) {
		if (!strncmp(t->name, "wl_", 3) && !strcmp(&t->name[3], name)) {
			nvram_set(strcat_r(prefix, name, tmp), t->value);
			break;
		}
	}
}
static int
wlconf_akm_options(char *prefix)
{
	char comb[32];
	char *wl_akm;
	int akm_ret_val = 0;
	char akm[32];
	char *next;

	wl_akm = nvram_default_get(strcat_r(prefix, "akm", comb),"disabled");
	foreach(akm, wl_akm, next) {
		if (!strcmp(akm, "wpa"))
			akm_ret_val |= WPA_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk"))
			akm_ret_val |= WPA_AUTH_PSK;
#ifdef BCMWPA2
		if (!strcmp(akm, "wpa2"))
			akm_ret_val |= WPA2_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk2"))
			akm_ret_val |= WPA2_AUTH_PSK;
		if (!strcmp(akm, "brcm_psk"))
			akm_ret_val |= BRCM_AUTH_PSK;
#endif
	}
	return akm_ret_val;
}

/* Set up wsec */
static int
wlconf_set_wsec(char *ifname, char *prefix, int bsscfg_idx)
{
	char tmp[100];
	int val = 0;
	int akm_val;
	int ret;

	nvram_default_get(strcat_r(prefix, "crypto", tmp),"off");
	/* Set wsec bitvec */
	akm_val = wlconf_akm_options(prefix);
	if (akm_val != 0) {
		if (nvram_match(strcat_r(prefix, "crypto", tmp), "tkip"))
			val = TKIP_ENABLED;
		else if (nvram_match(strcat_r(prefix, "crypto", tmp), "aes"))
			val = AES_ENABLED;
		else if (nvram_match(strcat_r(prefix, "crypto", tmp), "tkip+aes"))
			val = TKIP_ENABLED | AES_ENABLED;
	}
	if (nvram_default_match(strcat_r(prefix, "wep", tmp), "enabled","disabled"))
		val |= WEP_ENABLED;
	WL_BSSIOVAR_SETINT(ifname, "wsec", bsscfg_idx, val);
	/* Set wsec restrict if WSEC_ENABLED */
	WL_BSSIOVAR_SETINT(ifname, "wsec_restrict", bsscfg_idx, val ? 1 : 0);

	return 0;
}

#ifdef BCMWPA2
static int
wlconf_set_preauth(char *name, int bsscfg_idx, int preauth)
{
	uint cap;
	int ret;

	WL_BSSIOVAR_GET(name, "wpa_cap", bsscfg_idx, &cap, sizeof(uint));
	if (ret != 0) return -1;

	if (preauth)
		cap |= WPA_CAP_WPA2_PREAUTH;
	else
		cap &= ~WPA_CAP_WPA2_PREAUTH;

	WL_BSSIOVAR_SETINT(name, "wpa_cap", bsscfg_idx, cap);

	return ret;
}
#endif /* BCMWPA2 */

/* Set up WME */
static void
wlconf_set_wme(char *name, char *prefix)
{
	int i, j, k;
	int val, ret;
	int phytype, gmode, no_ack, apsd, dp[2];
	edcf_acparam_t *acparams;
	char buf[WLC_IOCTL_MAXLEN];
	char *v, *nv_value, nv[100];
	char nv_name[] = "%swme_%s_%s";
	char *ac[] = {"be", "bk", "vi", "vo"};
	char *cwmin, *cwmax, *aifsn, *txop_b, *txop_ag, *admin_forced, *oldest_first;
	char **locals[] = { &cwmin, &cwmax, &aifsn, &txop_b, &txop_ag, &admin_forced,
	                    &oldest_first };
	struct {char *req; char *str;} mode[] = {{"wme_ac_sta", "sta"}, {"wme_ac_ap", "ap"},
	                                         {"wme_tx_params", "txp"}};

	/* query the phy type */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
	/* get gmode */
	gmode = atoi(nvram_default_get(strcat_r(prefix, "gmode", nv),"1"));

	/* WME sta setting first */
	for (i = 0; i < 2; i++) {
		/* build request block */
		memset(buf, 0, sizeof(buf));
		strcpy(buf, mode[i].req);
		/* put push wmeac params after "wme-ac" in buf */
		acparams = (edcf_acparam_t *)(buf + strlen(buf) + 1);
		dp[i] = 0;
		for (j = 0; j < AC_COUNT; j++) {
			/* get packed nvram parameter */
			snprintf(nv, sizeof(nv), nv_name, prefix, mode[i].str, ac[j]);
			nv_value = nvram_safe_get(nv);
			strcpy(nv, nv_value);
			/* unpack it */
			v = nv;
			for (k = 0; k < (sizeof(locals) / sizeof(locals[0])); k++) {
				*locals[k] = v;
				while (*v && *v != ' ')
					v++;
				if (*v) {
					*v = 0;
					v++;
				}
			}

			/* update CWmin */
			acparams->ECW &= ~EDCF_ECWMIN_MASK;
			val = atoi(cwmin);
			for (val++, k = 0; val; val >>= 1, k++);
			acparams->ECW |= (k ? k - 1 : 0) & EDCF_ECWMIN_MASK;
			/* update CWmax */
			acparams->ECW &= ~EDCF_ECWMAX_MASK;
			val = atoi(cwmax);
			for (val++, k = 0; val; val >>= 1, k++);
			acparams->ECW |= ((k ? k - 1 : 0) << EDCF_ECWMAX_SHIFT) & EDCF_ECWMAX_MASK;
			/* update AIFSN */
			acparams->ACI &= ~EDCF_AIFSN_MASK;
			acparams->ACI |= atoi(aifsn) & EDCF_AIFSN_MASK;
			/* update ac */
			acparams->ACI &= ~EDCF_ACI_MASK;
			acparams->ACI |= j << EDCF_ACI_SHIFT;
			/* update TXOP */
			if (phytype == PHY_TYPE_B || gmode == 0)
				val = atoi(txop_b);
			else
				val = atoi(txop_ag);
			acparams->TXOP = val / 32;
			/* update acm */
			acparams->ACI &= ~EDCF_ACM_MASK;
			val = strcmp(admin_forced, "on") ? 0 : 1;
			acparams->ACI |= val << 4;

			/* configure driver */
			WL_IOCTL(name, WLC_SET_VAR, buf, sizeof(buf));
		}
	}

	/* set no-ack */
	v = nvram_default_get(strcat_r(prefix, "wme_no_ack", nv),"off");
	no_ack = strcmp(v, "on") ? 0 : 1;
	WL_IOVAR_SETINT(name, "wme_noack", no_ack);

	/* set APSD */
	v = nvram_default_get(strcat_r(prefix, "wme_apsd", nv),"on");
	apsd = strcmp(v, "on") ? 0 : 1;
	WL_IOVAR_SETINT(name, "wme_apsd", apsd);

	/* set per-AC discard policy */
	strcpy(buf, "wme_dp");
	WL_IOVAR_SETINT(name, "wme_dp", dp[1]);

	/* WME Tx parameters setting */
	{
		wme_tx_params_t txparams[AC_COUNT];
		char *srl, *sfbl, *lrl, *lfbl, *maxrate;
		char **locals[] = { &srl, &sfbl, &lrl, &lfbl, &maxrate };

		/* build request block */
		memset(txparams, 0, sizeof(txparams));

		for (j = 0; j < AC_COUNT; j++) {
			/* get packed nvram parameter */
			snprintf(nv, sizeof(nv), nv_name, prefix, mode[2].str, ac[j]);
			nv_value = nvram_safe_get(nv);
			strcpy(nv, nv_value);
			/* unpack it */
			v = nv;
			for (k = 0; k < (sizeof(locals) / sizeof(locals[0])); k++) {
				*locals[k] = v;
				while (*v && *v != ' ')
					v++;
				if (*v) {
					*v = 0;
					v++;
				}
			}

			/* update short retry limit */
			txparams[j].short_retry = atoi(srl);

			/* update short fallback limit */
			txparams[j].short_fallback = atoi(sfbl);

			/* update long retry limit */
			txparams[j].long_retry = atoi(lrl);

			/* update long fallback limit */
			txparams[j].long_fallback = atoi(lfbl);

			/* update max rate */
			txparams[j].max_rate = atoi(maxrate);
		}

		/* set the WME tx parameters */
		WL_IOVAR_SET(name, mode[2].req, txparams, sizeof(txparams));
	}
}

#if defined(linux)
#include <unistd.h>
static void
sleep_ms(const unsigned int ms)
{
	usleep(1000*ms);
}
#else
#error "sleep_ms() not defined for this OS!!!"
#endif /* defined(linux) */

/*
* The following condition(s) must be met when Auto Channel Selection
* is enabled.
*  - the I/F is up (change radio channel requires it is up?)
*  - the AP must not be associated (setting SSID to empty should
*    make sure it for us)
*/
static uint8
wlconf_auto_channel(char *name)
{
	int chosen = 0;
	wl_uint32_list_t request;
	int phytype;
	int ret;
	int i;

	/* query the phy type */
	WL_GETINT(name, WLC_GET_PHYTYPE, &phytype);

	request.count = 0;	/* let the ioctl decide */
	WL_IOCTL(name, WLC_START_CHANNEL_SEL, &request, sizeof(request));
	if (!ret) {
		sleep_ms(phytype == PHY_TYPE_A ? 1000 : 750);
		for (i = 0; i < 100; i++) {
			WL_GETINT(name, WLC_GET_CHANNEL_SEL, &chosen);
			if (!ret)
				break;
			sleep_ms(100);
		}
	}
	WLCONF_DBG("interface %s: channel selected %d\n", name, chosen);
	return chosen;
}

static chanspec_t
wlconf_auto_chanspec(char *name,char *prefix)
{
	chanspec_t chosen = 0;
	wl_uint32_list_t request;
	char tmp[100];
	int bandtype;
	int ret;
	int i;
	int chanspec_asus = 0;

	/* query the band type */
	WL_GETINT(name, WLC_GET_BAND, &bandtype);

	request.count = 0;	/* let the ioctl decide */
	WL_IOCTL(name, WLC_START_CHANNEL_SEL, &request, sizeof(request));
	if (!ret) {
		sleep_ms(1000);
		for (i = 0; i < 100; i++) {
			WL_IOVAR_GETINT(name, "apcschspec", (void *)&chosen);
			if (!ret)
				break;
			sleep_ms(100);
		}
	}

	WLCONF_DBG("interface %s: chanspec selected %04x\n", name, chosen);
	return chosen;
}


/* PHY type/BAND conversion */
#define WLCONF_PHYTYPE2BAND(phy)	((phy) == PHY_TYPE_A ? WLC_BAND_5G : WLC_BAND_2G)
/* PHY type conversion */
#define WLCONF_PHYTYPE2STR(phy)	((phy) == PHY_TYPE_A ? "a" : \
				 (phy) == PHY_TYPE_B ? "b" : \
				 (phy) == PHY_TYPE_LP ? "l" : \
				 (phy) == PHY_TYPE_G ? "g" : "n")
#define WLCONF_STR2PHYTYPE(phy)	((phy) && (phy)[0] == 'a' ? PHY_TYPE_A : \
				 (phy) && (phy)[0] == 'b' ? PHY_TYPE_B : \
				 (phy) && (phy)[0] == 'l' ? PHY_TYPE_LP : \
				 (phy) && (phy)[0] == 'g' ? PHY_TYPE_G : PHY_TYPE_N)


#define PREFIX_LEN 32			/* buffer size for wlXXX_ prefix */

struct bsscfg_info {
	int idx;			/* bsscfg index */
	char ifname[PREFIX_LEN];	/* OS name of interface (debug only) */
	char prefix[PREFIX_LEN];	/* prefix for nvram params (eg. "wl0.1_") */
};

struct bsscfg_list {
	int count;
	struct bsscfg_info bsscfgs[WL_MAXBSSCFG];
};

struct bsscfg_list *
wlconf_get_bsscfgs(char* ifname, char* prefix)
{
	char var[80];
	char tmp[100];
	char *next;

	struct bsscfg_list *bclist;
	struct bsscfg_info *bsscfg;

	bclist = (struct bsscfg_list*)malloc(sizeof(struct bsscfg_list));
	if (bclist == NULL)
		return NULL;
	memset(bclist, 0, sizeof(struct bsscfg_list));

	/* Set up Primary BSS Config information */
	bsscfg = &bclist->bsscfgs[0];
	bsscfg->idx = 0;
	strncpy(bsscfg->ifname, ifname, PREFIX_LEN-1);
	strcpy(bsscfg->prefix, prefix);
	bclist->count = 1;
	
	/* additional virtual BSS Configs from wlX_vifs */
	if (nvram_default_match(strcat_r(prefix, "mode", tmp),"ap","ap") || nvram_match(strcat_r(prefix, "mode", tmp),"apsta") || nvram_match(strcat_r(prefix, "mode", tmp),"apstawet"))
	{
	foreach(var, nvram_safe_get(strcat_r(prefix, "vifs", tmp)), next) {
		if (bclist->count == WL_MAXBSSCFG) {
			WLCONF_DBG("wlconf(%s): exceeded max number of BSS Configs (%d)"
			           "in nvram %s\n"
			           "while configuring interface \"%s\"\n",
			           ifname, WL_MAXBSSCFG, strcat_r(prefix, "vifs", tmp), var);
			continue;
		}
		bsscfg = &bclist->bsscfgs[bclist->count];
		if (get_ifname_unit(var, NULL, &bsscfg->idx) != 0) {
			WLCONF_DBG("wlconfg(%s): unable to parse unit.subunit in interface "
			           "name \"%s\"\n",
			           ifname, var);
			continue;
		}
		strncpy(bsscfg->ifname, var, PREFIX_LEN-1);
		snprintf(bsscfg->prefix, PREFIX_LEN, "%s_", bsscfg->ifname);
		bclist->count++;
	}
	}

	return bclist;
}

static void
wlconf_security_options(char *name, char *prefix, int bsscfg_idx, bool wet)
{
	int i;
	int val;
	int ret;
	char tmp[100];

	/* Set WSEC */
	/*
	* Need to check errors (card may have changed) and change to
	* defaults since the new chip may not support the requested
	* encryptions after the card has been changed.
	*/
	if (wlconf_set_wsec(name, prefix, bsscfg_idx)) {
		/* change nvram only, code below will pass them on */
		wlconf_restore_var(prefix, "auth_mode");
		wlconf_restore_var(prefix, "auth");
		/* reset wep to default */
		wlconf_restore_var(prefix, "crypto");
		wlconf_restore_var(prefix, "wep");
		wlconf_set_wsec(name, prefix, bsscfg_idx);
	}

	val = wlconf_akm_options(prefix);
	/* In wet mode enable in driver wpa supplicant */
	if (wet && (CHECK_PSK(val))) {
		wsec_pmk_t psk;
		char *key;

		if (((key = nvram_get(strcat_r(prefix, "wpa_psk", tmp))) != NULL) &&
		    (strlen(key) < WSEC_MAX_PSK_LEN)) {
			psk.key_len = (ushort) strlen(key);
			psk.flags = WSEC_PASSPHRASE;
			strcpy((char *)psk.key, key);
			WL_IOCTL(name, WLC_SET_WSEC_PMK, &psk, sizeof(psk));
		}
		wl_iovar_setint(name, "sup_wpa", 1);
	}
	WL_BSSIOVAR_SETINT(name, "wpa_auth", bsscfg_idx, val);

	/* EAP Restrict if we have an AKM or radius authentication */
	val = ((val != 0) || (nvram_default_match(strcat_r(prefix, "auth_mode", tmp), "radius","disabled")));
	WL_BSSIOVAR_SETINT(name, "eap_restrict", bsscfg_idx, val);

	/* Set WEP keys */
	if (nvram_default_match(strcat_r(prefix, "wep", tmp), "enabled","disabled")) {
		for (i = 1; i <= DOT11_MAX_DEFAULT_KEYS; i++)
			wlconf_set_wep_key(name, prefix, bsscfg_idx, i);
	}

	/* Set 802.11 authentication mode - open/shared */
	val = atoi(nvram_default_get(strcat_r(prefix, "auth", tmp),"0"));
	WL_BSSIOVAR_SETINT(name, "auth", bsscfg_idx, val);
}

/* configure the specified wireless interface */
int
wlconf(char *name)
{
	int restore_defaults, val, unit, phytype, bandtype, gmode = 0, ret = 0;
	int buflen;
	uint32 *val_ptr; /* required for iovars */
	int bcmerr;
	int error_bg, error_a;
	struct bsscfg_list *bclist = NULL;
	struct bsscfg_info *bsscfg;
	char tmp[100], prefix[PREFIX_LEN];
	char var[80], *next, phy[] = "a", *str, *addr = NULL;
	char buf[WLC_IOCTL_MAXLEN];
	char *country;
	wlc_rev_info_t rev;
	channel_info_t ci;
	struct maclist *maclist;
	struct ether_addr *ea;
	wlc_ssid_t ssid;
	wl_rateset_t rs;
	unsigned int i;
	char eaddr[32];
	int ap, apsta, wds, sta = 0, wet = 0, mac_spoof = 0;
	char country_code[4];
	int nas_will_run = 0;
	char *wme, *ba;
	char vif_addr[WLC_IOCTL_SMLEN];
#ifdef BCMWPA2
	char *preauth;
	int set_preauth;
#endif
	int ii;
	int wlunit = -1;
	int wlsubunit = -1;
	int max_no_vifs = 0;
	int mbsscap = 0;
	int wl_ap_build = 0; /* wl compiled with AP capabilities */
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_SMLEN];
	int btc_mode;
	uint32 leddc;
	bool ure_enab = FALSE;
	int nmode = OFF; /* 802.11n support */

	/* wlconf doesn't work for virtual i/f, so if we are given a
	 * virtual i/f return 0 if that interface is in it's parent's "vifs"
	 * list otherwise return -1
	 */
cprintf("get ifname unit\n");
	if (get_ifname_unit(name, &wlunit, &wlsubunit) == 0)
	{
		if (wlsubunit >= 0)
		{
			/* we have been given a virtual i/f,
			 * is it in it's parent i/f's virtual i/f list?
			 */
			sprintf(tmp, "wl%d_vifs", wlunit);

			if (strstr(nvram_safe_get(tmp), name) == NULL)
				return -1; /* config error */
			else
				return 0; /* okay */
		}
	}
	else
	{
		return -1;
	}

	/* clean up tmp */
	memset(tmp, 0, sizeof(tmp));

	/* because of ifdefs in wl driver,  when we don't have AP capabilities we
	 * can't use the same iovars to configure the wl.
	 * so we use "wl_ap_build" to help us know how to configure the driver
	 */
cprintf("get caps\n");
	if (wl_iovar_get(name, "cap", (void *)caps, WLC_IOCTL_SMLEN))
		return -1;


	foreach(cap, caps, next) {
		if (!strcmp(cap, "ap")) {
			wl_ap_build = 1;
		}
		else if (!strcmp(cap, "mbss16"))
		{
			max_no_vifs = 16;
			mbsscap = 1;
		}
		else if (!strcmp(cap, "mbss4"))
		{
			max_no_vifs = 4;
			mbsscap = 1;
		}
	}
cprintf("wl probe\n");
	/* Check interface (fail silently for non-wl interfaces) */
	if ((ret = wl_probe(name)))
		return ret;

cprintf("get wl addr\n");
	/* Get MAC address */
	(void) wl_hwaddr(name, (uchar *)buf);
	memcpy(vif_addr, buf, ETHER_ADDR_LEN);
	/* Get instance */
cprintf("get instance\n");
	WL_IOCTL(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
    	sprintf(tmp, "wl%d_mbss", unit);
	if (mbsscap)
	    {
	    nvram_set(tmp,"1");
	    }else
	    {
	    nvram_set(tmp,"0");
	    }
	/* clean up tmp */
	memset(tmp, 0, sizeof(tmp));

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	/* Restore defaults if per-interface parameters do not exist */
	restore_defaults = !nvram_get(strcat_r(prefix, "ifname", tmp));
	wlconf_validate_all(prefix, restore_defaults);
	nvram_set(strcat_r(prefix, "ifname", tmp), name);
	nvram_set(strcat_r(prefix, "hwaddr", tmp), ether_etoa((uchar *)buf, eaddr));
	snprintf(buf, sizeof(buf), "%d", unit);
	nvram_set(strcat_r(prefix, "unit", tmp), buf);


cprintf("shut down %s\n",name);
	/* Bring the interface down */
	WL_IOCTL(name, WLC_DOWN, NULL, sizeof(val));

cprintf("disable bss %s\n",name);
	/* Disable all BSS Configs */
	for (i = 0; i < WL_MAXBSSCFG; i++) {
		struct {int bsscfg_idx; int enable;} setbuf;
		setbuf.bsscfg_idx = i;
		setbuf.enable = 0;

		ret = wl_iovar_set(name, "bss", &setbuf, sizeof(setbuf));
		if (ret) {
			wl_iovar_getint(name, "bcmerror", &bcmerr);
			/* fail quietly on a range error since the driver may
			 * support fewer bsscfgs than we are prepared to configure
			 */
			if (bcmerr == BCME_RANGE)
				break;
		}
		if (ret)
			WLCONF_DBG("%d:(%s): setting bsscfg #%d iovar \"bss\" to 0"
			           " (down) failed, ret = %d, bcmerr = %d\n",
			           __LINE__, name, i, ret, bcmerr);
	}

	/* Get the list of BSS Configs */
	bclist = wlconf_get_bsscfgs(name, prefix);
	if (bclist == NULL) {
		ret = -1;
		goto exit;
	}


	/* create a wlX.Y_ifname nvram setting */
	for (i = 1; i < bclist->count; i++) {
		bsscfg = &bclist->bsscfgs[i];
#if defined(linux)
		strcpy(var, bsscfg->ifname);
#endif
		nvram_set(strcat_r(bsscfg->prefix, "ifname", tmp), var);
	}

	/* wlX_mode settings: AP, STA, WET, BSS/IBSS, APSTA */
	str = nvram_default_get(strcat_r(prefix, "mode", tmp),"dd-wrt");
	ap = (!strcmp(str, "") || !strcmp(str, "ap") || !strcmp(str, "mssid"));
	apsta = (!strcmp(str, "apsta") || !strcmp(str, "apstawet") || ((!strcmp(str, "sta") || !strcmp(str, "wet")) && bclist->count > 1));
	sta = (!strcmp(str, "sta") && bclist->count == 1);
	wds = !strcmp(str, "wds");
	wet = !strcmp(str, "wet") || !strcmp(str, "apstawet");
	mac_spoof = !strcmp(str, "mac_spoof");

	if (wet && apsta) { /* URE is enabled */
		ure_enab = TRUE;
	}
cprintf("set mssid flags %s\n",name);
	if (wl_ap_build) {
		/* Enable MSSID mode if appropriate */
		WL_IOVAR_SETINT(name, "mssid", (bclist->count > 1));
		if (!ure_enab && mbsscap) {
		WL_IOVAR_SETINT(name, "mbss", (bclist->count > 1)); //compatiblitiy with newer drivers
		}else{
		WL_IOVAR_SETINT(name, "mbss", 0); //compatiblitiy with newer drivers
		
		}
		/*
		 * Set SSID for each BSS Config
		 */
		for (i = 0; i < bclist->count; i++) {
			bsscfg = &bclist->bsscfgs[i];
			strcat_r(bsscfg->prefix, "ssid", tmp);
			ssid.SSID_len = strlen(nvram_default_get(tmp,"dd-wrt"));
			if (ssid.SSID_len > sizeof(ssid.SSID))
				ssid.SSID_len = sizeof(ssid.SSID);
			strncpy((char *)ssid.SSID, nvram_safe_get(tmp), ssid.SSID_len);
			WLCONF_DBG("wlconfig(%s): configuring bsscfg #%d (%s) with SSID \"%s\"\n",
			           name, bsscfg->idx, bsscfg->ifname, nvram_safe_get(tmp));
			WL_BSSIOVAR_SET(name, "ssid", bsscfg->idx, &ssid, sizeof(ssid));
		}
	}
#define MBSS_UC_IDX_MASK		(4 - 1)

cprintf("set local addr %s\n",name);
	if (!ure_enab) {
		/* set local bit for our MBSS vif base */
		if (mbsscap)
		    ETHER_SET_LOCALADDR(vif_addr);
		char newmac[32];
		memcpy(newmac,vif_addr,sizeof(vif_addr));
		/* construct and set other wlX.Y_hwaddr */
		for (i = 1; i < 4; i++) {
			snprintf(tmp, sizeof(tmp), "wl%d.%d_hwaddr", unit, i);
			addr = nvram_safe_get(tmp);
				if (mbsscap)
				    vif_addr[5] = (newmac[5] & ~MBSS_UC_IDX_MASK) | (MBSS_UC_IDX_MASK & (newmac[5]+i));

				nvram_set(tmp, ether_etoa((uchar *)vif_addr,
				                          eaddr));
		}
		/* The addresses are available in NVRAM, so set them */
		for (i = 1; i < 4; i++) {
				snprintf(tmp, sizeof(tmp), "wl%d.%d_hwaddr",
				         unit, i);
				ether_atoe(nvram_safe_get(tmp), eaddr);
				snprintf(tmp, sizeof(tmp), "wl%d.%d",
				         unit, i);
				WL_BSSIOVAR_SET(tmp, "cur_etheraddr", i,
				                eaddr, ETHER_ADDR_LEN);
		}
	} else { /* URE is enabled */
		/* URE is on, so set wlX.1 hwaddr is same as that of primary interface */
		snprintf(tmp, sizeof(tmp), "wl%d.1_hwaddr", unit);
		WL_BSSIOVAR_SET(name, "cur_etheraddr", 1, vif_addr,
		                ETHER_ADDR_LEN);
	}


	/* Set AP mode */
	val = (ap || apsta || wds) ? 1 : 0;
cprintf("set ap flag %s\n",name);
	WL_IOCTL(name, WLC_SET_AP, &val, sizeof(val));

cprintf("set apsta flag %s\n",name);
	WL_IOVAR_SETINT(name, "apsta", apsta);

	/* Set mode: WET */
cprintf("set wet flag %s\n",name);
	if (wet)
		WL_IOCTL(name, WLC_SET_WET, &wet, sizeof(wet));

cprintf("set spoof flag %s\n",name);
	if (mac_spoof) {
		sta = 1;
		WL_IOVAR_SETINT(name, "mac_spoof", 1);
	}

	/* For STA configurations, configure association retry time.
	 * Use specified time (capped), or mode-specific defaults.
	 */
cprintf("set sta retry time %s\n",name);
	if (sta || wet || apsta) {
		char *retry_time = nvram_default_get(strcat_r(prefix, "sta_retry_time", tmp),"5");
		val = atoi(retry_time);
		WL_IOVAR_SETINT(name, "sta_retry_time", val);
	}

	/* Retain remaining WET effects only if not APSTA */
//	wet &= !apsta;

	/* Set infra: BSS/IBSS (IBSS only for WET or STA modes) */
	val = 1;
	if (wet || sta)
		val = atoi(nvram_default_get(strcat_r(prefix, "infra", tmp),"1"));
cprintf("set infra flag %s\n",name);
	WL_IOCTL(name, WLC_SET_INFRA, &val, sizeof(val));

cprintf("set maxassoc flag %s\n",name);
	/* Set The AP MAX Associations Limit */
	if (ap | apsta) {
		val = atoi(nvram_default_get(strcat_r(prefix, "maxassoc", tmp),"128"));
		if (val > 0)
			WL_IOVAR_SETINT(name, "maxassoc", val);
	}
cprintf("set bsscfg %s\n",name);

	for (i = 0; i < bclist->count; i++) {
		char *subprefix;
		bsscfg = &bclist->bsscfgs[i];

#ifdef BCMWPA2
		/* XXXMSSID: The note about setting preauth now does not seem right.
		 * NAS brings the BSS up if it runs, so setting the preauth value
		 * will make it in the bcn/prb. If that is right, we can move this
		 * chunk out of wlconf.
		 */
		/*
		 * Set The WPA2 Pre auth cap. only reason we are doing it here is the driver is down
		 * if we do it in the NAS we need to bring down the interface and up to make
		 * it affect in the  beacons
		 */
		if (ap || (apsta && bsscfg->idx != 0)) {
			set_preauth = 1;
			preauth = nvram_safe_get(strcat_r(bsscfg->prefix, "preauth", tmp));
			if (strlen (preauth) != 0) {
				set_preauth = atoi(preauth);
			}
			wlconf_set_preauth(name, bsscfg->idx, set_preauth);
		}
#endif /* BCMWPA2 */

		subprefix = apsta ? prefix : bsscfg->prefix;

		/* Set network type */
		val = atoi(nvram_default_get(strcat_r(subprefix, "closed", tmp),"0"));
		WL_BSSIOVAR_SETINT(name, "closednet", bsscfg->idx, val);

		/* Set the ap isolate mode */
		val = atoi(nvram_default_get(strcat_r(subprefix, "ap_isolate", tmp),"0"));
		WL_BSSIOVAR_SETINT(name, "ap_isolate", bsscfg->idx, val);
	}

cprintf("get phy type %s\n",name);
	/* Get current phy type */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
	snprintf(buf, sizeof(buf), "%s", WLCONF_PHYTYPE2STR(phytype));
	nvram_set(strcat_r(prefix, "phytype", tmp), buf);

	/* Set up the country code */
	(void) strcat_r(prefix, "country_code", tmp);
	
#ifdef HAVE_BUFFALO
	country = BUFFALO_COUNTRY;
#elif HAVE_VINT
	if (nvram_match("wl0_phytype","a"))
	    country="US";
	else
	    country="JP";
#else
//	if (nvram_match("wl0_phytype","a"))
//	    country="US";
//	else
	    country="ALL";
/*	if (phytype == PHY_TYPE_N)
	    {
	    if (nvram_default_match(strcat_r(prefix, "net_mode", tmp),"n-only","mixed") || nvram_match(strcat_r(prefix, "net_mode", tmp),"mixed"))
		country="DE";
	    }*/
	//country = nvram_get(tmp);
#endif
cprintf("set country %s\n",name);
	if (country) {
		strncpy(country_code, country, sizeof(country_code));
		WL_IOCTL(name, WLC_SET_COUNTRY, country_code, strlen(country_code)+1);
	}
	else {
		/* If country_code doesn't exist, check for country to be backward compatible */
		(void) strcat_r(prefix, "country", tmp);
		country = nvram_safe_get(tmp);
		for (val = 0; val < ARRAYSIZE(countries); val++) {
			char **synonym;
			for (synonym = countries[val].names; *synonym; synonym++)
				if (!strcmp(country, *synonym))
					break;
			if (*synonym)
				break;
		}

		/* Get the default country code if undefined or invalid and set the NVRAM */
		if (val >= ARRAYSIZE(countries)) {
			WL_IOCTL(name, WLC_GET_COUNTRY, country_code, sizeof(country_code));
		}
		else {
			strncpy(country_code, countries[val].abbr, sizeof(country_code));
			WL_IOCTL(name, WLC_SET_COUNTRY, country_code, strlen(country_code)+1);
		}

		/* Add the new NVRAM variable */
		nvram_set("wl_country_code", country_code);
		(void) strcat_r(prefix, "country_code", tmp);
		nvram_set(tmp, country_code);
	}

cprintf("set reg mode %s\n",name);
	/* Setup regulatory mode */
	strcat_r(prefix, "reg_mode", tmp);
	if (nvram_default_match(tmp, "off","off"))  {
		val = 0;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));
	} else if (nvram_match(tmp, "h")) {
		val = 0;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
		val = 1;
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));

		/* Set the CAC parameters */
		val = atoi(nvram_safe_get(strcat_r(prefix, "dfs_preism", tmp)));
		wl_iovar_setint(name, "dfs_preism", val);
		val = atoi(nvram_safe_get(strcat_r(prefix, "dfs_postism", tmp)));
		wl_iovar_setint(name, "dfs_postism", val);
		val = atoi(nvram_safe_get(strcat_r(prefix, "tpc_db", tmp)));
		WL_IOCTL(name, WLC_SEND_PWR_CONSTRAINT, &val, sizeof(val));

	} else if (nvram_match(tmp, "d")) {
		val = 0;
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));
		val = 1;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
	}
cprintf("set maclist %s\n",name);

	/* Set the MAC list */
	maclist = (struct maclist *) buf;
	maclist->count = 0;
	if (!nvram_default_match(strcat_r(prefix, "macmode", tmp), "disabled","disabled")) {
		ea = maclist->ea;
		foreach(var, nvram_safe_get(strcat_r(prefix, "maclist", tmp)), next) {
			if (((char *)((&ea[1])->octet)) > ((char *)(&buf[sizeof(buf)])))
				break;
			if (ether_atoe(var, ea->octet)) {
				maclist->count++;
				ea++;
			}
		}
	}
	WL_IOCTL(name, WLC_SET_MACLIST, buf, sizeof(buf));

cprintf("set macmode %s\n",name);
	/* Set the MAC list mode */
	(void) strcat_r(prefix, "macmode", tmp);
	if (nvram_default_match(tmp, "deny","disabled"))
		val = WLC_MACMODE_DENY;
	else if (nvram_match(tmp, "allow"))
		val = WLC_MACMODE_ALLOW;
	else
		val = WLC_MACMODE_DISABLED;
	WL_IOCTL(name, WLC_SET_MACMODE, &val, sizeof(val));

	/* Change LED Duty Cycle */
	leddc = (uint32)strtoul(nvram_default_get(strcat_r(prefix, "leddc", tmp),"0x640000"), NULL, 16);
	if (leddc)
		WL_IOVAR_SETINT(name, "leddc", leddc);

	/* Enable or disable the radio */
cprintf("set radio flag %s\n",name);
	val = nvram_default_match(strcat_r(prefix, "radio", tmp), "0","1");
	val += WL_RADIO_SW_DISABLE << 16;
	WL_IOCTL(name, WLC_SET_RADIO, &val, sizeof(val));

cprintf("get phy flags %s\n",name);
	/* Get supported phy types */
	WL_IOCTL(name, WLC_GET_PHYLIST, var, sizeof(var));

	nvram_set(strcat_r(prefix, "phytypes", tmp), var);

	/* Get radio IDs */
	*(next = buf) = '\0';
	for (i = 0; i < strlen(var); i++) {
		/* Switch to band */
		phy[0] = var[i];
		val = WLCONF_STR2PHYTYPE(phy);
		if (val == PHY_TYPE_N) {
			WL_GETINT(name, WLC_GET_BAND, &val);
		} else
			val = WLCONF_PHYTYPE2BAND(val);
		WL_IOCTL(name, WLC_SET_BAND, &val, sizeof(val));
		/* Get radio ID on this band */
		WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));
		next += sprintf(next, "%sBCM%X", i ? " " : "",
		                (rev.radiorev & IDCODE_ID_MASK) >> IDCODE_ID_SHIFT);
	}
cprintf("set radio ids %s\n",name);
	nvram_set(strcat_r(prefix, "radioids", tmp), buf);

	/* Set band */
cprintf("set nband %s\n",name);
	str = nvram_get(strcat_r(prefix, "phytype", tmp));
	val = WLCONF_STR2PHYTYPE(str);
	/* For NPHY use band value from NVRAM */
	if (val == PHY_TYPE_N) {
		str = nvram_get(strcat_r(prefix, "nband", tmp));
		if (str)
			val = atoi(str);
		else {
			WL_GETINT(name, WLC_GET_BAND, &val);
		}
	} else
		val = WLCONF_PHYTYPE2BAND(val);

cprintf("set band %s\n",name);
	WL_SETINT(name, WLC_SET_BAND, val);

	/* Check errors (card may have changed) */
	if (ret) {
		/* default band to the first band in band list */
		phy[0] = var[0];
		val = WLCONF_STR2PHYTYPE(phy);
		val = WLCONF_PHYTYPE2BAND(val);
		WL_SETINT(name, WLC_SET_BAND, val);
	}

	/* Store the resolved bandtype */
	bandtype = val;

	/* Get current core revision */
cprintf("get core rev %s\n",name);
	WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));
	snprintf(buf, sizeof(buf), "%d", rev.corerev);
	nvram_set(strcat_r(prefix, "corerev", tmp), buf);


	/* Set channel before setting gmode or rateset */
	/* Manual Channel Selection - when channel # is not 0 */
cprintf("set channel %s\n",name);
	val = atoi(nvram_default_get(strcat_r(prefix, "channel", tmp),"0"));
	if (val && phytype != PHY_TYPE_N) {
		WL_SETINT(name, WLC_SET_CHANNEL, val);
		if (ret) {
			/* Use current channel (card may have changed) */
			WL_IOCTL(name, WLC_GET_CHANNEL, &ci, sizeof(ci));
			snprintf(buf, sizeof(buf), "%d", ci.target_channel);
			nvram_set(strcat_r(prefix, "channel", tmp), buf);
		}
	} else if (val && phytype == PHY_TYPE_N) {
		chanspec_t chanspec = 0;
		uint channel;
		uint nbw;
		uint nctrlsb = WL_CHANSPEC_CTL_SB_NONE;
		nmode = AUTO;	/* enable by default for NPHY */
		/* Set n mode */
		strcat_r(prefix, "nmode", tmp);
		if (nvram_match(tmp, "0"))
			nmode = OFF;

		channel = val;
		/* Get BW */
		val = atoi(nvram_safe_get(strcat_r(prefix, "nbw", tmp)));
		fprintf(stderr,"nbw = %d\n",val);
		if (nvram_match(strcat_r(prefix, "net_mode", tmp),"b-only") ||  nvram_match(strcat_r(prefix, "net_mode", tmp),"g-only") || nvram_match(strcat_r(prefix, "net_mode", tmp),"a-only") ||  nvram_match(strcat_r(prefix, "net_mode", tmp),"bg-mixed"))
			val = 20;
		
		fprintf(stderr,"channel %d, val %d\n",channel,val);
		switch (val) {
		case 40:
			val = WL_CHANSPEC_BW_40;
			break;
		case 20:
			val = WL_CHANSPEC_BW_20;
			break;
		case 10:
			val = WL_CHANSPEC_BW_10;
			break;
		default:
			val = WL_CHANSPEC_BW_20;
			nvram_set(strcat_r(prefix, "nbw", tmp), "20");
		}
		nbw = val;

		/* Get Ctrl SB for 40MHz channel */
		if (nbw == WL_CHANSPEC_BW_40) {
			str = nvram_safe_get(strcat_r(prefix, "nctrlsb", tmp));

			/* Adjust the channel to be center channel */
			if (!strcmp(str, "lower")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LOWER;
				channel = channel + 2;
			} else if (!strcmp(str, "upper")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_UPPER;
				channel = channel - 2;
			}
		}

		/* band | BW | CTRL SB | Channel */
		fprintf(stderr,"%X, %X, %X\n",nbw,nctrlsb,channel);
		chanspec |= ((bandtype << WL_CHANSPEC_BAND_SHIFT) |
		             (nbw | nctrlsb | channel));
		fprintf(stderr,"spec %X\n",chanspec);

		WL_IOVAR_SETINT(name, "chanspec", (uint32)chanspec);
	}

	/* Set up number of Tx and Rx streams */
	if (phytype == PHY_TYPE_N) {
		int count;
		int streams;

		wl_iovar_getint(name, "txchain_cnt", &count);
		/* update NVRAM with capabilities */
		snprintf(var, sizeof(var), "%d", count);
		nvram_set(strcat_r(prefix, "txchain_cnt", tmp), var);

		/* Verify that there is an NVRAM param for txstreams, if not create it and
		 * set it to txchain_cnt
		 */
		streams = atoi(nvram_safe_get(strcat_r(prefix, "txstreams", tmp)));
		if (streams == 0) {
			/* invalid - NVRAM needs to be fixed/initialized */
			nvram_set(strcat_r(prefix, "txstreams", tmp), var);
			streams = count;
		}
		/* Apply user configured txstreams, use 1 if user disabled nmode */
		if (nmode == OFF)
			streams = 1;
		WL_IOVAR_SETINT(name, "txstreams", streams);

		wl_iovar_getint(name, "rxchain_cnt", &count);
		/* update NVRAM with capabilities */
		snprintf(var, sizeof(var), "%d", count);
		nvram_set(strcat_r(prefix, "rxchain_cnt", tmp), var);

		/* Verify that there is an NVRAM param for rxstreams, if not create it and
		 * set it to txchain_cnt
		 */
		streams = atoi(nvram_safe_get(strcat_r(prefix, "rxstreams", tmp)));
		if (streams == 0) {
			/* invalid - NVRAM needs to be fixed/initialized */
			nvram_set(strcat_r(prefix, "rxstreams", tmp), var);
			streams = count;
		}

		/* Apply user configured rxstreams, use 1 if user disabled nmode */
		if (nmode == OFF)
			streams = 1;
		WL_IOVAR_SETINT(name, "rxstreams", streams);
	}


cprintf("set rate set %s\n",name);
	/* Reset to hardware rateset (band may have changed) */
	WL_IOCTL(name, WLC_GET_RATESET, &rs, sizeof(wl_rateset_t));
	WL_IOCTL(name, WLC_SET_RATESET, &rs, sizeof(wl_rateset_t));

cprintf("set g mode %s\n",name);
	/* Set gmode */
	if (bandtype == WLC_BAND_2G) {
		int override = WLC_G_PROTECTION_OFF;
		int control = WLC_G_PROTECTION_CTL_OFF;

		/* Set gmode */
		gmode = atoi(nvram_default_get(strcat_r(prefix, "gmode", tmp),"1"));
		WL_IOCTL(name, WLC_SET_GMODE, &gmode, sizeof(gmode));

		/* Set gmode protection override and control algorithm */
		strcat_r(prefix, "gmode_protection", tmp);
		if (nvram_default_match(tmp, "auto","auto")) {
			override = WLC_G_PROTECTION_AUTO;
			control = WLC_G_PROTECTION_CTL_OVERLAP;
		}
		WL_IOCTL(name, WLC_SET_GMODE_PROTECTION_OVERRIDE, &override, sizeof(override));
		WL_IOCTL(name, WLC_SET_GMODE_PROTECTION_CONTROL, &control, sizeof(control));
	}

cprintf("set n prot mode %s\n",name);
	/* Set nmode_protectoin */
	if (phytype == PHY_TYPE_N) {
		int override = WLC_PROTECTION_OFF;
		int control = WLC_PROTECTION_CTL_OFF;
		int nmode = AUTO;

		/* Set n mode */


		strcat_r(prefix, "nreqd", tmp);
		if (nvram_default_match(tmp, "0","0"))
			nmode = 0;
		if (nvram_match(tmp, "1"))
			nmode = 1;
		WL_IOVAR_SETINT(name, "nreqd", (uint32)nmode);

		strcat_r(prefix, "nmode", tmp);
		if (nvram_default_match(tmp, "0","-1"))
			nmode = OFF;
		if (nvram_match(tmp, "-1"))
			nmode = AUTO;
		if (nvram_match(tmp, "2"))
			nmode = WL_NMODE_NONLY;
		WL_IOVAR_SETINT(name, "nmode", (uint32)nmode);

		/* Set n protection override and control algorithm */
		strcat_r(prefix, "nmode_protection", tmp);

		if (nvram_default_match(tmp, "auto","auto")) {
			override = WLC_PROTECTION_AUTO;
			control = WLC_PROTECTION_CTL_OVERLAP;
		}

		memset(buf, 0, WLC_IOCTL_MAXLEN);
		strcpy(buf, "nmode_protection_override");
		buflen = strlen(buf) + 1;

		val_ptr = (uint32*)(buf + buflen);
		buflen += sizeof(override);
		memcpy(val_ptr, &override, sizeof(override));
		WL_IOCTL(name, WLC_SET_VAR, buf, sizeof(buf));
		WL_IOCTL(name, WLC_SET_PROTECTION_CONTROL, &control, sizeof(control));
	}

	/* Set WME mode */
	/* This needs to be done before afterburner as wme has precedence
	 *   -disable afterburner mode to allow any wme mode be configured
	 *   -set wme mode before set afterburner mode
	 */
cprintf("set afterburner override %s\n",name);
	val = OFF;
	strcpy(var, "afterburner_override");
	wl_iovar_setint(name, var, val);
	wme = nvram_default_get(strcat_r(prefix, "wme", tmp),"on");
	val = strcmp(wme, "on") ? 0 : 1;
cprintf("set wme %s\n",name);
	wl_iovar_set(name, "wme", &val, sizeof(val));
	if (val)
		wlconf_set_wme(name, prefix);

cprintf("set btc mode %s\n",name);
	/* Get bluetootch coexistance(BTC) mode */
	btc_mode = atoi(nvram_default_get(strcat_r(prefix, "btc_mode", tmp),"0"));

	/* Set options based on capability */
cprintf("get caps %s\n",name);
	wl_iovar_get(name, "cap", (void *)tmp, 100);
	foreach(var, tmp, next) {
		bool valid_option = FALSE;
		char *nvram_str = nvram_get(strcat_r(prefix, var, buf));

		if (!nvram_str)
			continue;

		if (!strcmp(nvram_str, "on"))
			val = ON;
		else if (!strcmp(nvram_str, "off"))
			val = OFF;
		else if (!strcmp(nvram_str, "auto"))
			val = AUTO;
		else
			continue;

		if (btc_mode != WL_BTC_PREMPT && strncmp(var, "afterburner", sizeof(var)) == 0) {
			if (val == ON || val == OFF || val == AUTO)
				valid_option = TRUE;
			strcpy(var, "afterburner_override");
		}
		if ((strncmp(var, "amsdu", sizeof(var)) == 0) ||
		    (strncmp(var, "ampdu", sizeof(var)) == 0)) {
			if (val == ON || val == OFF || val == AUTO)
				valid_option = TRUE;
		}
		if (valid_option)
			wl_iovar_setint(name, var, val);
	}

	/* Get current rateset (gmode may have changed) */
	WL_IOCTL(name, WLC_GET_CURR_RATESET, &rs, sizeof(wl_rateset_t));

	strcat_r(prefix, "rateset", tmp);
	if (nvram_default_match(tmp, "all","default"))  {
		/* Make all rates basic */
		for (i = 0; i < rs.count; i++)
			rs.rates[i] |= 0x80;
	} else if (nvram_match(tmp, "12")) {
		/* Make 1 and 2 basic */
		for (i = 0; i < rs.count; i++) {
			if ((rs.rates[i] & 0x7f) == 2 || (rs.rates[i] & 0x7f) == 4)
				rs.rates[i] |= 0x80;
			else
				rs.rates[i] &= ~0x80;
		}
	}
cprintf("set btc mode %s\n",name);

	/* Set BTC mode */
	if (!wl_iovar_setint(name, "btc_mode", btc_mode)) {
		if (btc_mode == WL_BTC_PREMPT) {
			wl_rateset_t rs_tmp = rs;
			/* remove 1Mbps and 2 Mbps from rateset */
			for (i = 0, rs.count = 0; i < rs_tmp.count; i++) {
				if ((rs_tmp.rates[i] & 0x7f) == 2 || (rs_tmp.rates[i] & 0x7f) == 4)
					continue;
				rs.rates[rs.count++] = rs_tmp.rates[i];
			}
		}
	}

cprintf("set rate set %s\n",name);
	/* Set rateset */
	WL_IOCTL(name, WLC_SET_RATESET, &rs, sizeof(wl_rateset_t));

cprintf("set plcphdr %s\n",name);
	/* Allow short preamble override for b cards */
	if (phytype == PHY_TYPE_B ||
	    (phytype == PHY_TYPE_G && (gmode == GMODE_LEGACY_B || gmode == GMODE_AUTO))) {
		strcat_r(prefix, "plcphdr", tmp);
		if (nvram_default_match(tmp, "long","long"))
			val = WLC_PLCP_AUTO;
		else
			val = WLC_PLCP_SHORT;
		WL_IOCTL(name, WLC_SET_PLCPHDR, &val, sizeof(val));
	}

	/* Set rate in 500 Kbps units */
	val = atoi(nvram_default_get(strcat_r(prefix, "rate", tmp),"0")) / 500000;


	/* Convert Auto mcsidx to Auto rate */
	if (phytype == PHY_TYPE_N) {
		int mcsidx = atoi(nvram_default_get(strcat_r(prefix, "nmcsidx", tmp),"-1"));
		/* -1 mcsidx used to designate AUTO rate */
		if (mcsidx == -1)
			val = 0;
	}

	/* 1Mbps and 2 Mbps are not allowed in BTC pre-emptive mode */
	if (btc_mode == WL_BTC_PREMPT && (val == 2 || val == 4))
		/* Must b/g band.  Set to 5.5Mbps */
		val = 11;

cprintf("set get rates %s\n",name);
	/* it is band-blind. try both band */
	error_bg = wl_iovar_setint(name, "bg_rate", val);
	error_a = wl_iovar_setint(name, "a_rate", val);

	if (error_bg && error_a) {
		/* both failed. Try default rate (card may have changed) */
		val = 0;

		error_bg = wl_iovar_setint(name, "bg_rate", val);
		error_a = wl_iovar_setint(name, "a_rate", val);

		snprintf(buf, sizeof(buf), "%d", val);
		nvram_set(strcat_r(prefix, "rate", tmp), buf);
	}

	/* For N-Phy, check if nrate needs to be applied */
	if (phytype == PHY_TYPE_N) {
		uint32 nrate = 0;
		int mcsidx = atoi(nvram_safe_get(strcat_r(prefix, "nmcsidx", tmp)));
		bool ismcs = (mcsidx >= 0);
		uint nbw  = atoi(nvram_safe_get(strcat_r(prefix, "nbw", tmp)));

		/* mcsidx of 32 is valid only for 40 Mhz */
		if (mcsidx == 32 && nbw == 20) {
			mcsidx =  -1;
			ismcs = FALSE;
			nvram_set(strcat_r(prefix, "nmcsidx", tmp), "-1");
		}

		/* Use nrate iovar only for MCS rate. */
		if (ismcs) {
			nrate |= NRATE_MCS_INUSE;
			nrate |= mcsidx & NRATE_RATE_MASK;

			memset(buf, 0, WLC_IOCTL_MAXLEN);
			strcpy(buf, "nrate");
			buflen = strlen(buf) + 1;

			val_ptr = (uint32*)(buf + buflen);
			buflen += sizeof(nrate);
			memcpy(val_ptr, &nrate, sizeof(nrate));
			WL_IOCTL(name, WLC_SET_VAR, buf, sizeof(buf));
		}
	}

cprintf("set mrates %s\n",name);
	/* Set multicast rate in 500 Kbps units */
	val = atoi(nvram_default_get(strcat_r(prefix, "mrate", tmp),"0")) / 500000;
	/* 1Mbps and 2 Mbps are not allowed in BTC pre-emptive mode */
	if (btc_mode == WL_BTC_PREMPT && (val == 2 || val == 4))
		/* Must b/g band.  Set to 5.5Mbps */
		val = 11;

	/* it is band-blind. try both band */
	error_bg = wl_iovar_setint(name, "bg_mrate", val);
	error_a = wl_iovar_setint(name, "a_mrate", val);

	if (error_bg && error_a) {
		/* Try default rate (card may have changed) */
		val = 0;

		wl_iovar_setint(name, "bg_mrate", val);
		wl_iovar_setint(name, "a_mrate", val);

		snprintf(buf, sizeof(buf), "%d", val);
		nvram_set(strcat_r(prefix, "mrate", tmp), buf);
	}

cprintf("set frag tres %s\n",name);
	/* Set fragmentation threshold */
	val = atoi(nvram_default_get(strcat_r(prefix, "frag", tmp),"2346"));
	wl_iovar_setint(name, "fragthresh", val);

	/* Set RTS threshold */
	val = atoi(nvram_default_get(strcat_r(prefix, "rts", tmp),"2347"));
	wl_iovar_setint(name, "rtsthresh", val);

	/* Set DTIM period */
	val = atoi(nvram_default_get(strcat_r(prefix, "dtim", tmp),"1"));
	WL_IOCTL(name, WLC_SET_DTIMPRD, &val, sizeof(val));

	/* Set beacon period */
	val = atoi(nvram_default_get(strcat_r(prefix, "bcn", tmp),"100"));
	WL_IOCTL(name, WLC_SET_BCNPRD, &val, sizeof(val));

cprintf("set wds %s\n",name);
	/* AP only config */
	if (ap || apsta || wds) {
		/* Set lazy WDS mode */
		val = atoi(nvram_default_get(strcat_r(prefix, "lazywds", tmp),"0"));
		WL_IOCTL(name, WLC_SET_LAZYWDS, &val, sizeof(val));

		/* Set the WDS list */
		maclist = (struct maclist *) buf;
		maclist->count = 0;
		ea = maclist->ea;
		foreach(var, nvram_safe_get(strcat_r(prefix, "wds", tmp)), next) {
			if (((char *)(ea->octet)) > ((char *)(&buf[sizeof(buf)])))
				break;
			ether_atoe(var, ea->octet);
			maclist->count++;
			ea++;
		}
		WL_IOCTL(name, WLC_SET_WDSLIST, buf, sizeof(buf));

		/* Set WDS link detection timeout */
		val = atoi(nvram_safe_get(strcat_r(prefix, "wds_timeout", tmp)));
		wl_iovar_setint(name, "wdstimeout", val);
	}

cprintf("set wframeburst %s\n",name);
	/* Set framebursting mode */
	if (btc_mode == WL_BTC_PREMPT)
		val = FALSE;
	else
		val = nvram_default_match(strcat_r(prefix, "frameburst", tmp), "on","off");
	WL_IOCTL(name, WLC_SET_FAKEFRAG, &val, sizeof(val));

cprintf("set rifs mode %s\n",name);
	/* Set RIFS mode based on framebursting */
	if (phytype == PHY_TYPE_N) {
		char *nvram_str = nvram_safe_get(strcat_r(prefix, "rifs", tmp));
		if (!strcmp(nvram_str, "on"))
			wl_iovar_setint(name, "rifs", ON);
		else if (!strcmp(nvram_str, "off"))
			wl_iovar_setint(name, "rifs", OFF);
	}

cprintf("set ba mode %s\n",name);
	/* Override BA mode only if set to on/off */
	ba = nvram_safe_get(strcat_r(prefix, "ba", tmp));
	if (!strcmp(ba, "on"))
		wl_iovar_setint(name, "ba", ON);
	else if (!strcmp(ba, "off"))
		wl_iovar_setint(name, "ba", OFF);

cprintf("set up %s\n",name);
	/* Bring the interface back up */
	WL_IOCTL(name, WLC_UP, NULL, 0);

cprintf("set antdiv mode %s\n",name);
	/* Set antenna */
	val = atoi(nvram_default_get(strcat_r(prefix, "antdiv", tmp),"3"));
	WL_IOCTL(name, WLC_SET_ANTDIV, &val, sizeof(val));

	/* Auto Channel Selection - when channel # is 0 in AP mode
	 *
	 * The following condition(s) must be met in order for
	 * Auto Channel Selection to work.
	 *  - the I/F must be up for the channel scan
	 *  - the AP must not be supporting a BSS (all BSS Configs must be disabled)
	 */
	if (ap || apsta) {
		if (!(val = atoi(nvram_default_get(strcat_r(prefix, "channel", tmp),"0")))) {
			if (phytype == PHY_TYPE_N) {
				chanspec_t chanspec = wlconf_auto_chanspec(name,prefix);
				if (chanspec != 0)
					{
//					fprintf(stderr,"auto chanspec %X\n",chanspec);
					WL_IOVAR_SETINT(name, "chanspec", chanspec);
					}
			}
			else {
				/* select a channel */
				val = wlconf_auto_channel(name);
				/* switch to the selected channel */
				if (val != 0)
					WL_IOCTL(name, WLC_SET_CHANNEL, &val, sizeof(val));
			}
			/* set the auto channel scan timer in the driver when in auto mode */
			val = 15;	/* 15 minutes for now */
			WL_IOCTL(name, WLC_SET_CS_SCAN_TIMER, &val, sizeof(val));
		}
		else {
			/* reset the channel scan timer in the driver when not in auto mode */
			val = 0;
			WL_IOCTL(name, WLC_SET_CS_SCAN_TIMER, &val, sizeof(val));
		}
	}

	/* Security settings for each BSS Configuration */
	for (i = 0; i < bclist->count; i++) {
		bsscfg = &bclist->bsscfgs[i];
		wlconf_security_options(name, bsscfg->prefix, bsscfg->idx, wet);
	}

	/*
	 * Finally enable BSS Configs or Join BSS
	 *
	 * AP: Enable BSS Config to bring AP up only when nas will not run
	 * STA: Join the BSS regardless.
	 */
	for (i = 0; i < bclist->count; i++) {
		struct {int bsscfg_idx; int enable;} setbuf;

		setbuf.bsscfg_idx = bclist->bsscfgs[i].idx;
		setbuf.enable = 1;

		/* NAS runs if we have an AKM or radius authentication */
		nas_will_run = wlconf_akm_options(bclist->bsscfgs[i].prefix) ||
		        nvram_default_match(strcat_r(bclist->bsscfgs[i].prefix, "auth_mode", tmp),
		                    "radius","disabled");

		if (((ap || apsta) && !nas_will_run) || sta || wet) {
			for (ii = 0; ii < MAX_BSS_UP_RETRIES; ii++) {
				if (wl_ap_build) {
					WL_IOVAR_SET(name, "bss", &setbuf, sizeof(setbuf));
				}
				else {
					strcat_r(prefix, "ssid", tmp);
					ssid.SSID_len = strlen(nvram_safe_get(tmp));
					if (ssid.SSID_len > sizeof(ssid.SSID))
						ssid.SSID_len = sizeof(ssid.SSID);
					strncpy((char *)ssid.SSID, nvram_safe_get(tmp),
					        ssid.SSID_len);
					WL_IOCTL(name, WLC_SET_SSID, &ssid, sizeof(ssid));
				}
				if (apsta && (ret != 0))
					sleep_ms(1000);
				else
					break;
			}
		}
	}

	ret = 0;
exit:
	if (bclist != NULL)
		free(bclist);

	return ret;
}

int
wlconf_down(char *name)
{
	int val, ret = 0;
	int i;
	int wlsubunit;
	int bcmerr;
	unsigned char buf[WLC_IOCTL_MAXLEN];
	struct maclist *maclist;
	struct {int bsscfg_idx; int enable;} setbuf;
	int wl_ap_build = 0; /* 1 = wl compiled with AP capabilities */
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_SMLEN];
	char *next;
	wlc_ssid_t ssid;

	/* wlconf doesn't work for virtual i/f */
	if (get_ifname_unit(name, NULL, &wlsubunit) == 0 && wlsubunit >= 0) {
		WLCONF_DBG("wlconf: skipping virtual interface \"%s\"\n", name);
		return 0;
	}

	/* Check interface (fail silently for non-wl interfaces) */
	if ((ret = wl_probe(name)))
		return ret;

	/* because of ifdefs in wl driver,  when we don't have AP capabilities we
	 * can't use the same iovars to configure the wl.
	 * so we use "wl_ap_build" to help us know how to configure the driver
	 */
	if (wl_iovar_get(name, "cap", (void *)caps, WLC_IOCTL_SMLEN))
		return -1;

	foreach(cap, caps, next) {
		if (!strcmp(cap, "ap")) {
			wl_ap_build = 1;
		}
	}

	if (wl_ap_build) {
		/* Bring down the interface */
		WL_IOCTL(name, WLC_DOWN, NULL, sizeof(val));

		/* Disable all BSS Configs */
		for (i = 0; i < WL_MAXBSSCFG; i++) {
			setbuf.bsscfg_idx = i;
			setbuf.enable = 0;

			ret = wl_iovar_set(name, "bss", &setbuf, sizeof(setbuf));
			if (ret) {
				wl_iovar_getint(name, "bcmerror", &bcmerr);
				/* fail quietly on a range error since the driver may
				 * support fewer bsscfgs than we are prepared to configure
				 */
				if (bcmerr == BCME_RANGE)
					break;
			}
		}
	}
	else {
		WL_IOCTL(name, WLC_GET_UP, &val, sizeof(val));
		if (val) {
			/* Nuke SSID  */
			ssid.SSID_len = 0;
			ssid.SSID[0] = '\0';
			WL_IOCTL(name, WLC_SET_SSID, &ssid, sizeof(ssid));

			/* Bring down the interface */
			WL_IOCTL(name, WLC_DOWN, NULL, sizeof(val));
		}
	}

	/* Nuke the WDS list */
	maclist = (struct maclist *) buf;
	maclist->count = 0;
	WL_IOCTL(name, WLC_SET_WDSLIST, buf, sizeof(buf));

	return 0;
}

#if defined(linux)
int
main(int argc, char *argv[])
{
	/* Check parameters and branch based on action */
	if (argc == 3 && !strcmp(argv[2], "up"))
		return wlconf(argv[1]);
	else if (argc == 3 && !strcmp(argv[2], "down"))
		return wlconf_down(argv[1]);
	else {
		fprintf(stderr, "Usage: wlconf <ifname> up|down\n");
		return -1;
	}
}
#endif
