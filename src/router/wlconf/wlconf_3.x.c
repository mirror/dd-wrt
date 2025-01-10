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
#include <bcmwifi_channels.h>
#include <proto/802.1d.h>
#include <sys/utsname.h>

#define WLCONF_DBG(fmt, arg...)

//#define WLCONF_DBG(fmt, arg...)       fprintf(stderr, "%s: "fmt, __FUNCTION__ , ## arg)

#define	BCM4306_CHIP_ID		0x4306	/* 4306 chipcommon chipid */
#define	BCM4311_CHIP_ID		0x4311	/* 4311 PCIe 802.11a/b/g */
#define	BCM43111_CHIP_ID	43111	/* 43111 chipcommon chipid (OTP chipid) */
#define	BCM43112_CHIP_ID	43112	/* 43112 chipcommon chipid (OTP chipid) */
#define	BCM4312_CHIP_ID		0x4312	/* 4312 chipcommon chipid */
#define BCM4313_CHIP_ID		0x4313	/* 4313 chip id */
#define	BCM4315_CHIP_ID		0x4315	/* 4315 chip id */
#define	BCM4318_CHIP_ID		0x4318	/* 4318 chipcommon chipid */
#define	BCM4319_CHIP_ID		0x4319	/* 4319 chip id */
#define	BCM4320_CHIP_ID		0x4320	/* 4320 chipcommon chipid */
#define	BCM4321_CHIP_ID		0x4321	/* 4321 chipcommon chipid */
#define	BCM4322_CHIP_ID		0x4322	/* 4322 chipcommon chipid */
#define	BCM43221_CHIP_ID	43221	/* 43221 chipcommon chipid (OTP chipid) */
#define	BCM43222_CHIP_ID	43222	/* 43222 chipcommon chipid */
#define	BCM43224_CHIP_ID	43224	/* 43224 chipcommon chipid */
#define	BCM43225_CHIP_ID	43225	/* 43225 chipcommon chipid */
#define	BCM43227_CHIP_ID	43227	/* 43227 chipcommon chipid */
#define	BCM43228_CHIP_ID	43228	/* 43228 chipcommon chipid */
#define	BCM43421_CHIP_ID	43421	/* 43421 chipcommon chipid */
#define	BCM43226_CHIP_ID	43226	/* 43226 chipcommon chipid */
#define	BCM43231_CHIP_ID	43231	/* 43231 chipcommon chipid (OTP chipid) */
#define	BCM43234_CHIP_ID	43234	/* 43234 chipcommon chipid */
#define	BCM43235_CHIP_ID	43235	/* 43235 chipcommon chipid */
#define	BCM43236_CHIP_ID	43236	/* 43236 chipcommon chipid */
#define	BCM43237_CHIP_ID	43237	/* 43237 chipcommon chipid */
#define	BCM43238_CHIP_ID	43238	/* 43238 chipcommon chipid */
#define	BCM4325_CHIP_ID		0x4325	/* 4325 chip id */
#define	BCM4328_CHIP_ID		0x4328	/* 4328 chip id */
#define	BCM4329_CHIP_ID		0x4329	/* 4329 chipcommon chipid */
#define	BCM4331_CHIP_ID		0x4331	/* 4331 chipcommon chipid */
#define BCM4336_CHIP_ID		0x4336	/* 4336 chipcommon chipid */
#define BCM43362_CHIP_ID	43362	/* 43362 chipcommon chipid */
#define BCM4330_CHIP_ID		0x4330	/* 4330 chipcommon chipid */
#define BCM6362_CHIP_ID		0x6362	/* 6362 chipcommon chipid */
#define	BCM43431_CHIP_ID	43431	/* 4331  chipcommon chipid (OTP, RBBU) */
#define	BCM43217_CHIP_ID	43217	/* 43217 chip id (OTP chipid) */

#define	BCM4342_CHIP_ID		4342	/* 4342 chipcommon chipid (OTP, RBBU) */
#define	BCM4402_CHIP_ID		0x4402	/* 4402 chipid */
#define	BCM4704_CHIP_ID		0x4704	/* 4704 chipcommon chipid */
#define	BCM4710_CHIP_ID		0x4710	/* 4710 chipid */
#define	BCM4712_CHIP_ID		0x4712	/* 4712 chipcommon chipid */
#define	BCM4716_CHIP_ID		0x4716	/* 4716 chipcommon chipid */
#define	BCM47162_CHIP_ID	47162	/* 47162 chipcommon chipid */
#define	BCM4748_CHIP_ID		0x4748	/* 4716 chipcommon chipid (OTP, RBBU) */
#define BCM4785_CHIP_ID		0x4785	/* 4785 chipcommon chipid */
#define	BCM5350_CHIP_ID		0x5350	/* 5350 chipcommon chipid */
#define	BCM5352_CHIP_ID		0x5352	/* 5352 chipcommon chipid */
#define	BCM5354_CHIP_ID		0x5354	/* 5354 chipcommon chipid */
#define BCM5365_CHIP_ID		0x5365	/* 5365 chipcommon chipid */
#define	BCM5356_CHIP_ID		0x5356	/* 5356 chipcommon chipid */
#define	BCM5357_CHIP_ID		0x5357	/* 5357 chipcommon chipid */
#define	BCM53572_CHIP_ID	53572	/* 53572 chipcommon chipid */

#define BCM4365_CHIP_ID		0x4365	/* 4365 chipcommon chipid */
#define BCM4366_CHIP_ID		0x4366	/* 4366 chipcommon chipid */
#define BCM4365_CHIP(chipid)	((CHIPID(chipid) == BCM4365_CHIP_ID) || \
				(CHIPID(chipid) == BCM4366_CHIP_ID))

#define	BCM53573_CHIP_ID	53573	/* 53573 chipcommon chipid */
#define	BCM53574_CHIP_ID	53574	/* 53574 chipcommon chipid */
#define BCM53573_CHIP(chipid)	((CHIPID(chipid) == BCM53573_CHIP_ID) || \
				(CHIPID(chipid) == BCM53574_CHIP_ID))

#define BCM43602_CHIP_ID	0xaa52	/* 43602 chipcommon chipid */

/* phy types */
#define	PHY_TYPE_A		0
#define	PHY_TYPE_B		1
#define	PHY_TYPE_G		2
#define	PHY_TYPE_N		4
#define	PHY_TYPE_LP		5
#define PHY_TYPE_SSN		6
#define	PHY_TYPE_HT		7
#define	PHY_TYPE_LCN		8
#define PHY_TYPE_AC		11
#define	PHY_TYPE_NULL		0xf

/* how many times to attempt to bring up a virtual i/f when
 * we are in APSTA mode and IOVAR set of "bss" "up" returns busy
 */
#define MAX_BSS_UP_RETRIES 5

/* notify the average dma xfer rate (in kbps) to the driver */
#define AVG_DMA_XFER_RATE 120000

/* Length of the wl capabilities string */
#define CAP_STR_LEN 250

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
#ifdef BCMDBG
#define WLCONF_DBG(fmt, arg...)	fprintf(stderr, "%s: "fmt, __FUNCTION__ , ## arg)
#define WL_IOCTL(ifname, cmd, buf, len)					\
	if ((ret = wl_ioctl(ifname, cmd, buf, len)))			\
		fprintf(stderr, "%s:%d:(%s): %s failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, #cmd, ret);
#define WL_SETINT(ifname, cmd, val)								\
	if ((ret = wlconf_setint(ifname, cmd, val)))						\
		fprintf(stderr, "%s:%d:(%s): setting %s to %d (0x%x) failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, #cmd, (int)val, (unsigned int)val, ret);
#define WL_GETINT(ifname, cmd, pval)								\
	if ((ret = wlconf_getint(ifname, cmd, pval)))						\
		fprintf(stderr, "%s:%d:(%s): getting %s failed, err = %d\n",			\
		        __FUNCTION__, __LINE__, ifname, #cmd, ret);
#define WL_IOVAR_SET(ifname, iovar, param, paramlen)					\
	if ((ret = wl_iovar_set(ifname, iovar, param, paramlen)))			\
		fprintf(stderr, "%s:%d:(%s): setting iovar \"%s\" failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, iovar, ret);
#define WL_IOVAR_SETINT(ifname, iovar, val)							\
	if ((ret = wl_iovar_setint(ifname, iovar, val)))					\
		fprintf(stderr, "%s:%d:(%s): setting iovar \"%s\" to 0x%x failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, iovar, (unsigned int)val, ret);
#define WL_IOVAR_GET(ifname, iovar, param, paramlen)					\
	if ((ret = wl_iovar_get(ifname, iovar, param, paramlen)))			\
		fprintf(stderr, "%s:%d:(%s): getting iovar \"%s\" failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, iovar, ret);
#define WL_IOVAR_GETINT(ifname, iovar, val)							\
	if ((ret = wl_iovar_getint(ifname, iovar, val)))					\
		fprintf(stderr, "%s:%d:(%s): getting iovar \"%s\" failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, iovar, ret);
#define WL_BSSIOVAR_SETBUF(ifname, iovar, bssidx, param, paramlen, buf, buflen)			\
	if ((ret = wl_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, buf, buflen)))	\
		fprintf(stderr, "%s:%d:(%s): setting bsscfg #%d iovar \"%s\" failed, err = %d\n", \
		        __FUNCTION__, __LINE__, ifname, bssidx, iovar, ret);
#define WL_BSSIOVAR_SET(ifname, iovar, bssidx, param, paramlen)					\
	if ((ret = wl_bssiovar_set(ifname, iovar, bssidx, param, paramlen)))			\
		fprintf(stderr, "%s:%d:(%s): setting bsscfg #%d iovar \"%s\" failed, err = %d\n", \
		        __FUNCTION__, __LINE__, ifname, bssidx, iovar, ret);
#define WL_BSSIOVAR_GET(ifname, iovar, bssidx, param, paramlen)					\
	if ((ret = wl_bssiovar_get(ifname, iovar, bssidx, param, paramlen)))			\
		fprintf(stderr, "%s:%d:(%s): getting bsscfg #%d iovar \"%s\" failed, err = %d\n", \
		        __FUNCTION__, __LINE__, ifname, bssidx, iovar, ret);
#define WL_BSSIOVAR_SETINT(ifname, iovar, bssidx, val)						\
	if ((ret = wl_bssiovar_setint(ifname, iovar, bssidx, val)))				\
		fprintf(stderr, "%s:%d:(%s): setting bsscfg #%d iovar \"%s\" " \
				"to val 0x%x failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, bssidx, iovar, (unsigned int)val, ret);
#ifdef __CONFIG_DHDAP__
#define DHD_BSSIOVAR_SETINT(ifname, iovar, bssidx, val)						\
	if ((ret = dhd_bssiovar_setint(ifname, iovar, bssidx, val)))				\
		fprintf(stderr, "%s:%d:(%s): setting bsscfg #%d iovar \"%s\" " \
				"to val 0x%x failed, err = %d\n",	\
		        __FUNCTION__, __LINE__, ifname, bssidx, iovar, (unsigned int)val, ret);
#endif
#else
#define WL_IOCTL(name, cmd, buf, len)			(ret = wl_ioctl(name, cmd, buf, len))
#define WL_SETINT(name, cmd, val)			(ret = wlconf_setint(name, cmd, val))
#define WL_GETINT(name, cmd, pval)			(ret = wlconf_getint(name, cmd, pval))
#define WL_IOVAR_SET(ifname, iovar, param, paramlen)	(ret = wl_iovar_set(ifname, iovar, \
							param, paramlen))
#define WL_IOVAR_GET(ifname, iovar, param, paramlen)	(ret = wl_iovar_get(ifname, iovar, \
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
#ifdef __CONFIG_DHDAP__
#define DHD_BSSIOVAR_SETINT(ifname, iovar, bssidx, val)	(ret = dhd_bssiovar_setint(ifname, iovar, \
			bssidx, val))
#endif
#endif

#define CHECK_PSK(mode) ((mode) & (WPA_AUTH_PSK | WPA2_AUTH_PSK))

#ifdef __CONFIG_DHDAP__
#define DHD_MAX_ASSOC_VAL		32
#define DHD_BSS_MAXASSOC_VAL		32
#endif

#define PSPRETEND_DEFAULT_THRESHOLD 5

#ifdef __CONFIG_DHDAP__
extern int dhd_probe(char *name);
extern int dhd_iovar_setint(char *ifname, char *iovar, int val);
extern int dhd_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);
#endif

/* prototypes */
struct bsscfg_list *wlconf_get_bsscfgs(char *ifname, char *prefix);
int wlconf(char *name);
int wlconf_down(char *name);

static int wlconf_getint(char *ifname, int cmd, int *pval)
{
	return wl_ioctl(ifname, cmd, pval, sizeof(int));
}

static int wlconf_setint(char *ifname, int cmd, int val)
{
	return wl_ioctl(ifname, cmd, &val, sizeof(int));
}

char *nvram_ifexists_get(char *var, char *def)
{
	char *v = nvram_get(var);
	if (v == NULL || strlen(v) == 0) {
		return def;
	}
	return nvram_safe_get(var);
}

/* set WEP key */
static int wlconf_set_wep_key(char *name, char *prefix, int bsscfg_idx, int i)
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
			*data++ = (unsigned char)strtoul(hex, NULL, 16);
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

/* validate/restore all per-interface related variables */
/*static void
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
}*/

/* restore specific per-interface variable */
/*static void
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
}*/
static int wlconf_akm_options(char *prefix)
{
	char comb[32];
	char *wl_akm;
	int akm_ret_val = 0;
	char akm[32];
	char *next;

	wl_akm = nvram_default_get(strcat_r(prefix, "akm", comb), "disabled");
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
static int wlconf_set_wsec(char *ifname, char *prefix, int bsscfg_idx)
{
	char tmp[100];
	int val = 0;
	int akm_val;
	int ret;

	nvram_default_get(strcat_r(prefix, "crypto", tmp), "off");
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
	if (nvram_default_match(strcat_r(prefix, "wep", tmp), "enabled", "disabled"))
		val |= WEP_ENABLED;
	WL_BSSIOVAR_SETINT(ifname, "wsec", bsscfg_idx, val);
	/* Set wsec restrict if WSEC_ENABLED */
	WL_BSSIOVAR_SETINT(ifname, "wsec_restrict", bsscfg_idx, val ? 1 : 0);

	return 0;
}

#ifdef BCMWPA2
static int wlconf_set_preauth(char *name, int bsscfg_idx, int preauth)
{
	uint cap;
	int ret;

	WL_BSSIOVAR_GET(name, "wpa_cap", bsscfg_idx, &cap, sizeof(uint));
	if (ret != 0)
		return -1;

	if (preauth)
		cap |= WPA_CAP_WPA2_PREAUTH;
	else
		cap &= ~WPA_CAP_WPA2_PREAUTH;

	WL_BSSIOVAR_SETINT(name, "wpa_cap", bsscfg_idx, cap);

	return ret;
}
#endif				/* BCMWPA2 */

static void wlconf_set_radarthrs(char *name, char *prefix)
{
	wl_radar_thr_t radar_thr;
	int i, ret, len;
	char nv_buf[NVRAM_MAX_VALUE_LEN], *rargs, *v, *endptr;
	char buf[WLC_IOCTL_SMLEN];

	char *version = NULL;
	char *thr0_20_lo = NULL, *thr1_20_lo = NULL;
	char *thr0_40_lo = NULL, *thr1_40_lo = NULL;
	char *thr0_80_lo = NULL, *thr1_80_lo = NULL;
	char *thr0_20_hi = NULL, *thr1_20_hi = NULL;
	char *thr0_40_hi = NULL, *thr1_40_hi = NULL;
	char *thr0_80_hi = NULL, *thr1_80_hi = NULL;

	char **locals[] = { &version, &thr0_20_lo, &thr1_20_lo, &thr0_40_lo, &thr1_40_lo,
		&thr0_80_lo, &thr1_80_lo, &thr0_20_hi, &thr1_20_hi,
		&thr0_40_hi, &thr1_40_hi, &thr0_80_hi, &thr1_80_hi
	};

	rargs = nvram_safe_get(strcat_r(prefix, "radarthrs", nv_buf));
	if (!rargs)
		goto err;

	len = strlen(rargs);
	if ((len > NVRAM_MAX_VALUE_LEN) || (len == 0))
		goto err;

	memset(nv_buf, 0, sizeof(nv_buf));
	strncpy(nv_buf, rargs, len);
	v = nv_buf;
	for (i = 0; i < (sizeof(locals) / sizeof(locals[0])); i++) {
		*locals[i] = v;
		while (*v && *v != ' ') {
			v++;
		}
		if (*v) {
			*v = 0;
			v++;
		}
		if (v >= (nv_buf + len))	/* Check for complete list, if not caught later */
			break;
	}

	/* Start building request */
	memset(buf, 0, sizeof(buf));
	strcpy(buf, "radarthrs");
	/* Retrieve radar thrs parameters */
	if (!version)
		goto err;
	radar_thr.version = atoi(version);
	if (radar_thr.version > WL_RADAR_THR_VERSION)
		goto err;

	/* Retrieve ver 0 params */
	if (!thr0_20_lo)
		goto err;
	radar_thr.thresh0_20_lo = (uint16) strtol(thr0_20_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (!thr1_20_lo)
		goto err;
	radar_thr.thresh1_20_lo = (uint16) strtol(thr1_20_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (!thr0_40_lo)
		goto err;
	radar_thr.thresh0_40_lo = (uint16) strtol(thr0_40_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (!thr1_40_lo)
		goto err;
	radar_thr.thresh1_40_lo = (uint16) strtol(thr1_40_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (!thr0_80_lo)
		goto err;
	radar_thr.thresh0_80_lo = (uint16) strtol(thr0_80_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (!thr1_80_lo)
		goto err;
	radar_thr.thresh1_80_lo = (uint16) strtol(thr1_80_lo, &endptr, 0);
	if (*endptr != '\0')
		goto err;

	if (radar_thr.version == 0) {
		/*
		 * Attempt a best effort update of ver 0 to ver 1 by updating
		 * the appropriate values with the specified defaults.  The defaults
		 * are from the reference design.
		 */
		radar_thr.version = WL_RADAR_THR_VERSION;	/* avoid driver rejecting it */
		radar_thr.thresh0_20_hi = 0x6ac;
		radar_thr.thresh1_20_hi = 0x6cc;
		radar_thr.thresh0_40_hi = 0x6bc;
		radar_thr.thresh1_40_hi = 0x6e0;
		radar_thr.thresh0_80_hi = 0x6b0;
		radar_thr.thresh1_80_hi = 0x30;
	} else {
		/* Retrieve ver 1 params */
		if (!thr0_20_hi)
			goto err;
		radar_thr.thresh0_20_hi = (uint16) strtol(thr0_20_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

		if (!thr1_20_hi)
			goto err;
		radar_thr.thresh1_20_hi = (uint16) strtol(thr1_20_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

		if (!thr0_40_hi)
			goto err;
		radar_thr.thresh0_40_hi = (uint16) strtol(thr0_40_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

		if (!thr1_40_hi)
			goto err;
		radar_thr.thresh1_40_hi = (uint16) strtol(thr1_40_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

		if (!thr0_80_hi)
			goto err;
		radar_thr.thresh0_80_hi = (uint16) strtol(thr0_80_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

		if (!thr1_80_hi)
			goto err;
		radar_thr.thresh1_80_hi = (uint16) strtol(thr1_80_hi, &endptr, 0);
		if (*endptr != '\0')
			goto err;

	}

	/* Copy radar parameters into buffer and plug them to the driver */
	memcpy((char *)(buf + strlen(buf) + 1), (char *)&radar_thr, sizeof(wl_radar_thr_t));
	WL_IOCTL(name, WLC_SET_VAR, buf, sizeof(buf));

	return;

err:
	WLCONF_DBG("Did not parse radar thrs params, using driver defaults\n");
	return;
}

/*
 * This allows phy antenna selection to be retrieved from NVRAM
 */
static void wlconf_set_antsel(char *name, char *prefix)
{
	int i, j, len, argc, ret;
	char buf[WLC_IOCTL_SMLEN];
	wlc_antselcfg_t val = { { 0 }, 0 };
	char *argv[ANT_SELCFG_MAX] = { };
	char nv_buf[NVRAM_MAX_VALUE_LEN], *argstr, *v, *endptr;

	argstr = nvram_safe_get(strcat_r(prefix, "phy_antsel", nv_buf));
	if (!argstr) {
		return;
	}
	len = strlen(argstr);
	if ((len == 0) || (len > NVRAM_MAX_VALUE_LEN)) {
		return;
	}

	memset(nv_buf, 0, sizeof(nv_buf));
	strncpy(nv_buf, argstr, len);
	v = nv_buf;
	for (argc = 0; argc < ANT_SELCFG_MAX;) {
		argv[argc++] = v;
		while (*v && *v != ' ') {
			v++;
		}
		if (*v) {
			*v = 0;
			v++;
		}
		if (v >= (nv_buf + len)) {
			break;
		}
	}
	if ((argc != 1) && (argc != ANT_SELCFG_MAX)) {
		WLCONF_DBG("phy_antsel requires 1 or %d arguments\n", ANT_SELCFG_MAX);
		return;
	}

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "phy_antsel");
	for (i = 0, j = 0; i < ANT_SELCFG_MAX; i++) {
		val.ant_config[i] = (uint8) strtol(argv[j], &endptr, 0);
		if (*endptr != '\0') {
			WLCONF_DBG("Invalid antsel argument\n");
			return;
		}
		if (argc > 1) {
			/* ANT_SELCFG_MAX argument format */
			j++;
		}
	}

	/* Copy antsel parameters into buffer and plug them to the driver */
	memcpy((char *)(buf + strlen(buf) + 1), (char *)&val, sizeof(wlc_antselcfg_t));
	WL_IOCTL(name, WLC_SET_VAR, buf, sizeof(buf));

	return;
}

/* Set up WME */
static void wlconf_set_wme(char *name, char *prefix)
{
	int i, j, k;
	int val, ret;
	int phytype, gmode, no_ack, apsd, dp[2];
	edcf_acparam_t *acparams;
	char buf[WLC_IOCTL_MAXLEN];
	char *v, *nv_value, nv[100];
	char nv_name[] = "%swme_%s_%s";
	char *ac[] = { "be", "bk", "vi", "vo" };
	char *cwmin, *cwmax, *aifsn, *txop_b, *txop_ag, *admin_forced, *oldest_first;
	char **locals[] = { &cwmin, &cwmax, &aifsn, &txop_b, &txop_ag, &admin_forced,
		&oldest_first
	};
	struct {
		char *req;
		char *str;
	} mode[] = { { "wme_ac_sta", "sta" }, { "wme_ac_ap", "ap" },
	{ "wme_tx_params", "txp" }
	};

	/* query the phy type */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
	/* get gmode */
	gmode = atoi(nvram_default_get(strcat_r(prefix, "gmode", nv), "1"));

	/* WME sta setting first */
	for (i = 0; i < 2; i++) {
		/* build request block */
		memset(buf, 0, sizeof(buf));
		strcpy(buf, mode[i].req);
		/* put push wmeac params after "wme-ac" in buf */
		acparams = (edcf_acparam_t *) (buf + strlen(buf) + 1);
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
			for (val++, k = 0; val; val >>= 1, k++) ;
			acparams->ECW |= (k ? k - 1 : 0) & EDCF_ECWMIN_MASK;
			/* update CWmax */
			acparams->ECW &= ~EDCF_ECWMAX_MASK;
			val = atoi(cwmax);
			for (val++, k = 0; val; val >>= 1, k++) ;
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
	v = nvram_default_get(strcat_r(prefix, "wme_no_ack", nv), "off");
	no_ack = strcmp(v, "on") ? 0 : 1;
	WL_IOVAR_SETINT(name, "wme_noack", no_ack);

	/* set APSD */
	v = nvram_default_get(strcat_r(prefix, "wme_apsd", nv), "on");
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
static void sleep_ms(const unsigned int ms)
{
	usleep(1000 * ms);
}
#else
#error "sleep_ms() not defined for this OS!!!"
#endif				/* defined(linux) */

/*
* The following condition(s) must be met when Auto Channel Selection
* is enabled.
*  - the I/F is up (change radio channel requires it is up?)
*  - the AP must not be associated (setting SSID to empty should
*    make sure it for us)
*/
static uint8 wlconf_auto_channel(char *name)
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

static chanspec_t wlconf_auto_chanspec(char *name)
{
	chanspec_t chosen = 0;
	int temp = 0;
	wl_uint32_list_t request;
	int ret;
	int i;

	request.count = 0;	/* let the ioctl decide */
	WL_IOCTL(name, WLC_START_CHANNEL_SEL, &request, sizeof(request));
	if (!ret) {
		/* this time needs to be < 1000 to prevent mpc kicking in for 2nd radio */
		sleep_ms(500);
		for (i = 0; i < 100; i++) {
			WL_IOVAR_GETINT(name, "apcschspec", &temp);
			if (!ret)
				break;
			sleep_ms(100);
		}
	}

	chosen = (chanspec_t) temp;
	WLCONF_DBG("interface %s: chanspec selected %04x\n", name, chosen);
	return chosen;
}

/* PHY type/BAND conversion */
#define WLCONF_PHYTYPE2BAND(phy)	((phy) == PHY_TYPE_A ? WLC_BAND_5G : WLC_BAND_2G)
/* PHY type conversion */
#define WLCONF_PHYTYPE2STR(phy)	((phy) == PHY_TYPE_A ? "a" : \
				 (phy) == PHY_TYPE_B ? "b" : \
				 (phy) == PHY_TYPE_LP ? "l" : \
				 (phy) == PHY_TYPE_G ? "g" : \
				 (phy) == PHY_TYPE_SSN ? "s" : \
				 (phy) == PHY_TYPE_HT ? "h" : \
				 (phy) == PHY_TYPE_AC ? "v" : \
				 (phy) == PHY_TYPE_LCN ? "c" : "n")
#define WLCONF_STR2PHYTYPE(ch)	((ch) == 'a' ? PHY_TYPE_A : \
				 (ch) == 'b' ? PHY_TYPE_B : \
				 (ch) == 'l' ? PHY_TYPE_LP : \
				 (ch) == 'g' ? PHY_TYPE_G : \
				 (ch) == 's' ? PHY_TYPE_SSN : \
				 (ch) == 'h' ? PHY_TYPE_HT : \
				 (ch) == 'v' ? PHY_TYPE_AC : \
				 (ch) == 'c' ? PHY_TYPE_LCN : PHY_TYPE_N)

#define WLCONF_PHYTYPE_11N(phy) ((phy) == PHY_TYPE_N 	|| (phy) == PHY_TYPE_SSN || \
				 (phy) == PHY_TYPE_LCN 	|| (phy) == PHY_TYPE_HT || \
				 (phy) == PHY_TYPE_AC)

#define PREFIX_LEN 32		/* buffer size for wlXXX_ prefix */

struct bsscfg_info {
	int idx;		/* bsscfg index */
	char ifname[PREFIX_LEN];	/* OS name of interface (debug only) */
	char prefix[PREFIX_LEN];	/* prefix for nvram params (eg. "wl0.1_") */
};

struct bsscfg_list {
	int count;
	struct bsscfg_info bsscfgs[WL_MAXBSSCFG];
};

struct bsscfg_list *wlconf_get_bsscfgs(char *ifname, char *prefix)
{
	char var[80];
	char tmp[100];
	char *next;

	struct bsscfg_list *bclist;
	struct bsscfg_info *bsscfg;

	bclist = (struct bsscfg_list *)malloc(sizeof(struct bsscfg_list));
	if (bclist == NULL)
		return NULL;
	memset(bclist, 0, sizeof(struct bsscfg_list));

	/* Set up Primary BSS Config information */
	bsscfg = &bclist->bsscfgs[0];
	bsscfg->idx = 0;
	strncpy(bsscfg->ifname, ifname, PREFIX_LEN - 1);
	strcpy(bsscfg->prefix, prefix);
	bclist->count = 1;

	/* additional virtual BSS Configs from wlX_vifs */
	if (nvram_default_match(strcat_r(prefix, "mode", tmp), "ap", "ap") || nvram_match(strcat_r(prefix, "mode", tmp), "apsta") || nvram_match(strcat_r(prefix, "mode", tmp), "apstawet")) {
		foreach(var, nvram_safe_get(strcat_r(prefix, "vifs", tmp)), next) {
			if (bclist->count == WL_MAXBSSCFG) {
				fprintf(stderr, "wlconf(%s): exceeded max number of BSS Configs (%d)" "in nvram %s\n" "while configuring interface \"%s\"\n", ifname, WL_MAXBSSCFG, strcat_r(prefix, "vifs", tmp), var);
				continue;
			}
			bsscfg = &bclist->bsscfgs[bclist->count];
			if (get_ifname_unit(var, NULL, &bsscfg->idx) != 0) {
				fprintf(stderr, "wlconfg(%s): unable to parse unit.subunit in interface " "name \"%s\"\n", ifname, var);
				continue;
			}
			strncpy(bsscfg->ifname, var, PREFIX_LEN - 1);
			snprintf(bsscfg->prefix, PREFIX_LEN, "%s_", bsscfg->ifname);
			bclist->count++;
		}
	}
	fprintf(stderr, "total vif count %d\n", bclist->count);

	return bclist;
}

static void wlconf_security_options(char *name, char *prefix, int bsscfg_idx, bool wet, bool supplicant)
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
//              wlconf_restore_var(prefix, "auth_mode");
//              wlconf_restore_var(prefix, "auth");
		/* reset wep to default */
//              wlconf_restore_var(prefix, "crypto");
//              wlconf_restore_var(prefix, "wep");
		wlconf_set_wsec(name, prefix, bsscfg_idx);
	}
	val = wlconf_akm_options(prefix);
	/* In wet mode enable in driver wpa supplicant */

	if (supplicant && wet && (CHECK_PSK(val))) {
		wsec_pmk_t psk;
		char *key;

		if (((key = nvram_get(strcat_r(prefix, "wpa_psk", tmp))) != NULL) && (strlen(key) < WSEC_MAX_PSK_LEN)) {
			psk.key_len = (ushort) strlen(key);
			psk.flags = WSEC_PASSPHRASE;
			strcpy((char *)psk.key, key);
			WL_IOCTL(name, WLC_SET_WSEC_PMK, &psk, sizeof(psk));
		}
		wl_iovar_setint(name, "sup_wpa", 1);
	}
	WL_BSSIOVAR_SETINT(name, "wpa_auth", bsscfg_idx, val);

	/* EAP Restrict if we have an AKM or radius authentication */
	val = ((val != 0) || (nvram_default_match(strcat_r(prefix, "auth_mode", tmp), "radius", "disabled")));
	WL_BSSIOVAR_SETINT(name, "eap_restrict", bsscfg_idx, val);

	/* Set WEP keys */
	if (nvram_default_match(strcat_r(prefix, "wep", tmp), "enabled", "disabled")) {
		for (i = 1; i <= DOT11_MAX_DEFAULT_KEYS; i++)
			wlconf_set_wep_key(name, prefix, bsscfg_idx, i);
	}

	/* Set 802.11 authentication mode - open/shared */
	val = atoi(nvram_default_get(strcat_r(prefix, "auth", tmp), "0"));
	WL_BSSIOVAR_SETINT(name, "auth", bsscfg_idx, val);
}

static void wlconf_set_ampdu_retry_limit(char *name, char *prefix)
{
	int i, j, ret, nv_len;
	struct ampdu_retry_tid retry_limit;
	char *nv_name[2] = { "ampdu_rtylimit_tid", "ampdu_rr_rtylimit_tid" };
	char *iov_name[2] = { "ampdu_retry_limit_tid", "ampdu_rr_retry_limit_tid" };
	char *retry, *v, *nv_value, nv[100], tmp[100];

	/* Get packed AMPDU (rr) retry limit per-tid from NVRAM if present */
	for (i = 0; i < 2; i++) {
		nv_value = nvram_safe_get(strcat_r(prefix, nv_name[i], tmp));
		nv_len = strlen(nv_value);
		strcpy(nv, nv_value);

		/* unpack it */
		v = nv;
		for (j = 0; nv_len >= 0 && j < NUMPRIO; j++) {
			retry = v;
			while (*v && *v != ' ') {
				v++;
				nv_len--;
			}
			if (*v) {
				*v = 0;
				v++;
				nv_len--;
			}
			/* set the AMPDU retry limit per-tid */
			retry_limit.tid = j;
			retry_limit.retry = atoi(retry);
			WL_IOVAR_SET(name, iov_name[i], &retry_limit, sizeof(retry_limit));
		}
	}

	return;
}

/* Function to set the Short and the Long retry limits
 */
static void wlconf_set_retry_limit(char *name, char *prefix)
{
	char *srl, *lrl;
	int srl_val, lrl_val, ret;
	char tmp[100];

	srl = nvram_safe_get(strcat_r(prefix, "srl", tmp));

	if (srl && *srl != '\0') {
		srl_val = atoi(srl);
		if (srl_val)
			WL_IOCTL(name, WLC_SET_SRL, &srl_val, sizeof(srl_val));
	}

	lrl = nvram_safe_get(strcat_r(prefix, "lrl", tmp));

	if (lrl && *lrl != '\0') {
		lrl_val = atoi(lrl);
		if (lrl_val)
			WL_IOCTL(name, WLC_SET_LRL, &lrl_val, sizeof(lrl_val));
	}
	return;
}

static int wlconf_aburn_ampdu_amsdu_set(char *name, char prefix[PREFIX_LEN], int nmode, int btc_mode)
{
	bool ampdu_valid_option = FALSE;
	bool amsdu_valid_option = FALSE;
	bool aburn_valid_option = FALSE;
	int val, aburn_option_val = OFF, ampdu_option_val = OFF, amsdu_option_val = OFF;
	int wme_option_val = ON;	/* On by default */
	char tmp[CAP_STR_LEN], var[80], *next, *wme_val;
	char buf[WLC_IOCTL_SMLEN];
	int len = strlen("amsdu");
	int ret;
	int phytype;
	wlc_rev_info_t rev;
#ifdef __CONFIG_DHDAP__
	int is_dhd = 0;
#endif
	struct utsname unamebuf;

	uname(&unamebuf);
#ifdef __CONFIG_DHDAP__
	is_dhd = !dhd_probe(name);
#endif
	WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));

	/* First, clear WMM and afterburner settings to avoid conflicts */
	WL_IOVAR_SETINT(name, "wme", OFF);
	WL_IOVAR_SETINT(name, "afterburner_override", OFF);
	/* Get WME setting from NVRAM if present */
	wme_val = nvram_get(strcat_r(prefix, "wme", tmp));
	if (wme_val && !strcmp(wme_val, "off")) {
		wme_option_val = OFF;
	}

	/* Set options based on capability */
	wl_iovar_get(name, "cap", (void *)tmp, CAP_STR_LEN);
	foreach(var, tmp, next) {
		char *nvram_str;
		bool amsdu = 0;

		/* Check for the capabilitiy 'amsdutx' */
		if (strncmp(var, "amsdutx", sizeof(var)) == 0) {
			var[len] = '\0';
			amsdu = 1;
		}
		nvram_str = nvram_get(strcat_r(prefix, var, buf));
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
			aburn_valid_option = TRUE;
			aburn_option_val = val;
		}

		if (strncmp(var, "ampdu", sizeof(var)) == 0) {
			ampdu_valid_option = TRUE;
			ampdu_option_val = val;
		}

		if (amsdu) {
			amsdu_valid_option = TRUE;
			amsdu_option_val = val;
		}
	}

	if (nmode != OFF) {	/* N-mode is ON/AUTO */
		if (ampdu_valid_option) {
			if (ampdu_option_val != OFF) {
//                              if (phytype != PHY_TYPE_AC) {
				WL_IOVAR_SETINT(name, "amsdu", OFF);
//                              }
				WL_IOVAR_SETINT(name, "ampdu", ampdu_option_val);
			} else {
				WL_IOVAR_SETINT(name, "ampdu", OFF);
			}
			wlconf_set_ampdu_retry_limit(name, prefix);

			/* For MIPS routers set the num mpdu per ampdu limit to 32. We observed
			 * that having this value at 32 helps with bi-di thru'put as well.
			 */
			if ((phytype == PHY_TYPE_AC) && (strncmp(unamebuf.machine, "mips", 4) == 0)) {
				WL_IOVAR_SETINT(name, "ampdu_mpdu", 32);
			}
		}

		if (amsdu_valid_option) {
			if (amsdu_option_val != OFF) {	/* AMPDU (above) has priority over AMSDU */
				if (rev.corerev >= 40) {
					WL_IOVAR_SETINT(name, "amsdu", amsdu_option_val);
#ifdef __CONFIG_DHDAP__
					/* For dhdap, ampdu_mpdu=32 gives good tput in
					 * multiple radio with amsdu.
					 */
					if (is_dhd)
						WL_IOVAR_SETINT(name, "ampdu_mpdu", 32);
#endif
				} else if (ampdu_option_val == OFF) {
					WL_IOVAR_SETINT(name, "ampdu", OFF);
					WL_IOVAR_SETINT(name, "amsdu", amsdu_option_val);
				} else if (rev.chipnum == BCM43217_CHIP_ID) {
					/* In 43217, ampdu_mpdu=32 gives good tput with amsdu */
					WL_IOVAR_SETINT(name, "ampdu_mpdu", 32);
				}
			} else {
				WL_IOVAR_SETINT(name, "amsdu", OFF);
#ifdef __CONFIG_DHDAP__
				/* Use default ampdu_mpdu=-1 when amsdu is disabled */
				if (is_dhd)
					WL_IOVAR_SETINT(name, "ampdu_mpdu", -1);
#endif
			}

		} else {
#ifdef __CONFIG_DHDAP__
			/* Use default ampdu_mpdu=-1 when amsdu is disabled */
			if (is_dhd)
				WL_IOVAR_SETINT(name, "ampdu_mpdu", -1);
#endif
		}

//              WL_IOVAR_SETINT(name, "rx_amsdu_in_ampdu", OFF);
		/* allow ab in N mode. Do this last: may defeat ampdu et al */
		if (aburn_valid_option) {
			WL_IOVAR_SETINT(name, "afterburner_override", aburn_option_val);

			/* Also turn off N reqd setting if ab is not OFF */
			if (aburn_option_val != 0)
				WL_IOVAR_SETINT(name, "nreqd", 0);
		}

	} else {
		/* When N-mode is off or for non N-phy device, turn off AMPDU, AMSDU;
		 * if WME is off, set the afterburner based on the configured nvram setting.
		 */
		wl_iovar_setint(name, "amsdu", OFF);
		wl_iovar_setint(name, "ampdu", OFF);
		if (wme_option_val != OFF) {	/* Can't have afterburner with WMM */
			if (aburn_valid_option) {
				WL_IOVAR_SETINT(name, "afterburner_override", OFF);
			}
		} else if (aburn_valid_option) {	/* Okay, use NVRam setting for afterburner */
			WL_IOVAR_SETINT(name, "afterburner_override", aburn_option_val);
		}
	}

	if (wme_option_val && aburn_option_val == 0) {
		WL_IOVAR_SETINT(name, "wme", wme_option_val);
		wlconf_set_wme(name, prefix);
	}

	/* Override SRL & LRL if nvram configuration is defined */
	wlconf_set_retry_limit(name, prefix);
	return wme_option_val;
}

/* Get configured bandwidth cap. */
static int wlconf_bw_cap(char *prefix, int bandtype)
{
	char *str, tmp[100];
	int bw_cap = WLC_BW_CAP_20MHZ;
	char *q = NULL;

	if ((str = nvram_get(strcat_r(prefix, "bw_cap", tmp))) != NULL) {
		if (!strcmp(str, "0xff"))
			str = "255";
		bw_cap = strtol(str, &q, 0);
	} else {
		/* Backward compatibility. Map to bandwidth cap bitmap values. */
		int val = strtol(nvram_safe_get(strcat_r(prefix, "nbw_cap", tmp)), &q, 0);

		if (((bandtype == WLC_BAND_2G) && (val == WLC_N_BW_40ALL)) || ((bandtype == WLC_BAND_5G) && (val == WLC_N_BW_40ALL || val == WLC_N_BW_20IN2G_40IN5G)))
			bw_cap = WLC_BW_CAP_40MHZ;
		else
			bw_cap = WLC_BW_CAP_20MHZ;
	}

	return bw_cap;
}

/* Unset TXBF. Called when i/f is down. */
static void wlconf_unset_txbf(char *name, char *prefix)
{
	char tmp[100];
	int ret = 0;

	/* unset nvram TxBF off */
	nvram_set(strcat_r(prefix, "txbf_bfr_cap", tmp), "0");
	nvram_set(strcat_r(prefix, "txbf_bfe_cap", tmp), "0");
	nvram_set(strcat_r(prefix, "txbf_imp", tmp), "0");

	/* turning TXBF off */
	WL_IOVAR_SETINT(name, "txbf", 0);
	WL_IOVAR_SETINT(name, "txbf_bfr_cap", 0);
	WL_IOVAR_SETINT(name, "txbf_bfe_cap", 0);
	WL_IOVAR_SETINT(name, "txbf_imp", 0);
}

/* Set up TxBF. Called when i/f is down. */
static void wlconf_set_txbf(char *name, char *prefix)
{
	char *str, tmp[100];
	wlc_rev_info_t rev;
	uint32 txbf_bfe_cap = 0;
	uint32 txbf_bfr_cap = 0;
	uint32 txbf_imp = 0;
	uint32 mu_features = 0;
	wl_txbf_rateset_t rs;
	int ret = 0;

	WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));

	if (rev.corerev < 40)
		return;		/* TxBF unsupported */

	if ((str = nvram_get(strcat_r(prefix, "txbf_bfr_cap", tmp))) != NULL) {
		txbf_bfr_cap = atoi(str);

		if (txbf_bfr_cap) {
			/* Turning TxBF on (order matters) */
			WL_IOVAR_SETINT(name, "txbf_bfr_cap", txbf_bfr_cap);
			WL_IOVAR_SETINT(name, "txbf", 1);
		} else {
			/* Similarly, turning TxBF off in reverse order */
			WL_IOVAR_SETINT(name, "txbf", 0);
			WL_IOVAR_SETINT(name, "txbf_bfr_cap", 0);
		}
	}

	if ((str = nvram_get(strcat_r(prefix, "txbf_bfe_cap", tmp))) != NULL) {
		txbf_bfe_cap = atoi(str);

		WL_IOVAR_SETINT(name, "txbf_bfe_cap", txbf_bfe_cap);
	}

	if ((str = nvram_get(strcat_r(prefix, "txbf_imp", tmp))) != NULL) {
		txbf_imp = atoi(str);

		WL_IOVAR_SETINT(name, "txbf_imp", txbf_imp);
	}

	if ((str = nvram_get(strcat_r(prefix, "mu_features", tmp))) != NULL) {
		mu_features = strtoul(str, NULL, 0);
		WL_IOVAR_SETINT(name, "mu_features", mu_features);
	}
}

/* Set up TxBF timer. Called when i/f is up. */
static void wlconf_set_txbf_timer(char *name, char *prefix)
{
	char *str, tmp[100];
	wlc_rev_info_t rev;
	uint32 txbf_timer = 0;
	int ret = 0;

	WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));

	if (rev.corerev < 40)
		return;		/* TxBF unsupported */

	if ((str = nvram_get(strcat_r(prefix, "txbf_timer", tmp))) != NULL) {
		txbf_timer = (uint32) atoi(str);
		WL_IOVAR_SETINT(name, "txbf_timer", txbf_timer);
	}
}

static int wlconf_del_brcm_syscap_ie(char *name, int bsscfg_idx, char *oui)
{
	int iebuf_len = 0;
	vndr_ie_setbuf_t *ie_setbuf = NULL;
	int iecount, i;

	char getbuf[2048] = { 0 };
	vndr_ie_buf_t *iebuf;
	vndr_ie_info_t *ieinfo;
	char *bufaddr;
	int buflen = 0;
	int found = 0;
	uint32 pktflag;
	uint32 frametype;
	int ret = 0;

	frametype = VNDR_IE_BEACON_FLAG;

	WL_BSSIOVAR_GET(name, "vndr_ie", bsscfg_idx, getbuf, 2048);
	iebuf = (vndr_ie_buf_t *) getbuf;

	bufaddr = (char *)iebuf->vndr_ie_list;

	for (i = 0; i < iebuf->iecount; i++) {
		ieinfo = (vndr_ie_info_t *) bufaddr;
		bcopy((char *)&ieinfo->pktflag, (char *)&pktflag, (int)sizeof(uint32));
		if (pktflag == frametype) {
			if (!memcmp(ieinfo->vndr_ie_data.oui, oui, DOT11_OUI_LEN)) {
				found = 1;
				bufaddr = (char *)&ieinfo->vndr_ie_data;
				buflen = (int)ieinfo->vndr_ie_data.len + VNDR_IE_HDR_LEN;
				break;
			}
		}
		bufaddr = (char *)(ieinfo->vndr_ie_data.oui + ieinfo->vndr_ie_data.len);
	}

	if (!found)
		goto err;

	iebuf_len = buflen + sizeof(vndr_ie_setbuf_t) - sizeof(vndr_ie_t);
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(iebuf_len);
	if (!ie_setbuf) {
		WLCONF_DBG("memory alloc failure\n");
		ret = -1;
		goto err;
	}

	memset(ie_setbuf, 0, iebuf_len);

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "del");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy(&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));

	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &frametype, sizeof(uint32));

	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data, bufaddr, buflen);

	WL_BSSIOVAR_SET(name, "vndr_ie", bsscfg_idx, ie_setbuf, iebuf_len);

err:
	if (ie_setbuf)
		free(ie_setbuf);

	return ret;
}

static int wlconf_set_brcm_syscap_ie(char *name, int bsscfg_idx, char *oui, uchar * data, int datalen)
{
	vndr_ie_setbuf_t *ie_setbuf = NULL;
	unsigned int pktflag;
	int buflen, iecount;
	int ret = 0;

	pktflag = VNDR_IE_BEACON_FLAG;

	buflen = sizeof(vndr_ie_setbuf_t) + datalen - 1;
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(buflen);
	if (!ie_setbuf) {
		WLCONF_DBG("memory alloc failure\n");
		ret = -1;
		goto err;
	}

	memset(ie_setbuf, 0, buflen);

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "add");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy(&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));

	/* The packet flag bit field indicates the packets that will contain this IE */
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &pktflag, sizeof(uint32));

	/* Now, add the IE to the buffer, +1: one byte OUI_TYPE */
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len = DOT11_OUI_LEN + datalen;

	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[0], oui, DOT11_OUI_LEN);
	if (datalen > 0)
		memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[0], data, datalen);

	ret = wlconf_del_brcm_syscap_ie(name, bsscfg_idx, oui);
	if (ret)
		goto err;

	WL_BSSIOVAR_SET(name, "vndr_ie", (int)bsscfg_idx, ie_setbuf, buflen);

err:
	if (ie_setbuf)
		free(ie_setbuf);

	return (ret);
}

#define VIFNAME_LEN 16

static void wlconf_set_taf(char *name, bool enable)
{
#if defined(__CONFIG_TOAD__)
	static const char toad_ifnames[] = "toad_ifnames";
#endif
	char iobuf[sizeof("taf")	/* Need some space for the iovar itself, + nul */
		   +sizeof(wl_taf_define_t) - 1	/* struct size -1 byte for text[1] declaration */
		   + sizeof("enable")	/* "enable" string + terminating nul byte */
		   +sizeof("0")	/* "0" or "1" string + terminating nul byte */
		   +sizeof("")];	/* Final terminating nul byte */

	wl_taf_define_t *taf_def;
	char *ptr;
	int ret;		/* Variable name "ret" is required by WL_IOCTL() */

	memset(iobuf, 0, sizeof(iobuf));
	strcpy(iobuf, "taf");

	taf_def = (wl_taf_define_t *) & iobuf[sizeof("taf")];
	memset(&taf_def->ea, 0xff, sizeof(taf_def->ea));

	taf_def->version = 1;

	ptr = taf_def->text;
	ptr += sprintf(ptr, "enable");	/* Set up the command name argument */
	*ptr++ = '\0';		/* add the NUL separator */
	*ptr++ = (enable) ? '1' : '0';	/* enable or disable taf */
	*ptr++ = '\0';		/* add the NUL separator */
	*ptr = '\0';		/* and another NUL to terminate the argument list */

	WL_IOCTL(name, WLC_SET_VAR, iobuf, sizeof(iobuf));

#if defined(__CONFIG_TOAD__)

	/* Update the "toad_ifnames" nvram variable by adding or removing the interface name */

	ptr = nvram_get(toad_ifnames);

	if (!ptr && enable) {	/* Creating nvram with the first ifname */

		nvram_set(toad_ifnames, name);

	} else if (ptr && enable) {	/* Possibly need to add a new ifname */

		if (!strstr(ptr, name)) {
			char *tmp;

			tmp = malloc(strlen(ptr) + sizeof(" ") + strlen(name));

			if (tmp) {
				sprintf(tmp, "%s %s", ptr, name);
				nvram_set(toad_ifnames, tmp);
				free(tmp);
			}
		}

	} else if (ptr && !enable) {	/* Possibly need to remove an existing ifname */

		if (strstr(ptr, name)) {
			int tlen;

			tlen = strlen(ptr) - strlen(name);
			if (tlen == 0) {
				nvram_unset(toad_ifnames);
			} else {
				char *tmp;
				tmp = malloc(tlen + sizeof(""));
				if (tmp) {
					char ifname[VIFNAME_LEN];
					char *next;
					char *cp;

					memset(tmp, 0, tlen + sizeof(""));
					cp = tmp;

					foreach(ifname, ptr, next) {
						if (strcmp(ifname, name) != 0) {
							cp += sprintf(cp, "%s%s", (cp == tmp) ? "" : " ", name);
						}
					}
					nvram_set(toad_ifnames, tmp);
					free(tmp);
				}
			}
		}
	}			/* otherwise this must be (!ptr && !enable), no action needed here. */

#endif				/* __CONFIG_TOAD__ */

}

/* configure the specified wireless interface */
int wlconf(char *name)
{

	int restore_defaults, val, unit, phytype, bandtype, gmode = 0, ret = 0;
	int buflen;
	uint32 *val_ptr;	/* required for iovars */
	int bcmerr;
	int error_bg, error_a;
	struct bsscfg_list *bclist = NULL;
	struct bsscfg_info *bsscfg;
	char tmp[100], tmp2[100], prefix[PREFIX_LEN];
	char var[80], *next, phy[] = "a", *str, *addr = NULL;
	int bandlist[3];
	char buf[WLC_IOCTL_MAXLEN];
	char *country;
	char *country_rev;
	wlc_rev_info_t rev;
	channel_info_t ci;
	struct maclist *maclist;
	struct ether_addr *ea;
	wlc_ssid_t ssid;
	wl_rateset_t rs;
	unsigned int i;
	char eaddr[32];
	int ap, apsta, wds, sta = 0, wet = 0, mac_spoof = 0, wmf = 0;
	int radio_pwrsave = 0, rxchain_pwrsave = 0;
	wl_country_t country_spec = { { 0 }, 0, { 0 } };
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
	int wl_ap_build = 0;	/* wl compiled with AP capabilities */
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	int btc_mode;
	uint32 leddc;
	uint nbw = WL_CHANSPEC_BW_20;
	int nmode = OFF;	/* 802.11n support */
	int wme_global;
	int max_assoc = -1;
	bool ure_enab = FALSE;
	bool radar_enab = FALSE;
	chanspec_t chanspec = 0, prev_chspec = 0;
	bool obss_coex = FALSE;
	int wet_tunnel_cap = 0, wet_tunnel_enable = 0;
	bool cap_160 = FALSE, cap_dyn160 = FALSE;
	brcm_prop_ie_t brcm_syscap_ie;
	int is_dhd = 0;
	int cfg_max_assoc = -1;

	/* wlconf doesn't work for virtual i/f, so if we are given a
	 * virtual i/f return 0 if that interface is in it's parent's "vifs"
	 * list otherwise return -1
	 */
	if (get_ifname_unit(name, &wlunit, &wlsubunit) == 0) {
		if (wlsubunit >= 0) {
			/* we have been given a virtual i/f,
			 * is it in it's parent i/f's virtual i/f list?
			 */
			sprintf(tmp, "wl%d_vifs", wlunit);

			if (strstr(nvram_safe_get(tmp), name) == NULL)
				return -1;	/* config error */
			else
				return 0;	/* okay */
		}
	} else {
		return -1;
	}

	/* clean up tmp */
	memset(tmp, 0, sizeof(tmp));

	/* Check interface (fail silently for non-wl interfaces) */
	if ((ret = wl_probe(name)))
		return ret;

#ifdef __CONFIG_DHDAP__
	/* Check if interface uses dhd adapter */
	is_dhd = !dhd_probe(name);
#endif				/* __CONFIG_DHDAP__ */

	/* Bring up the interface temporarily before issuing iovars. */
	/* This will ensure all the cores are fully initialized */
	if (is_dhd)
		WL_IOCTL(name, WLC_UP, NULL, 0);

	/* because of ifdefs in wl driver,  when we don't have AP capabilities we
	 * can't use the same iovars to configure the wl.
	 * so we use "wl_ap_build" to help us know how to configure the driver
	 */
	if (wl_iovar_get(name, "cap", (void *)caps, sizeof(caps)))
		return -1;

	foreach(cap, caps, next) {
		if (!strcmp(cap, "ap")) {
			wl_ap_build = 1;
		} else if (!strcmp(cap, "mbss16")) {
			max_no_vifs = 16;
			mbsscap = 1;
		} else if (!strcmp(cap, "mbss8")) {
			max_no_vifs = 8;
			mbsscap = 1;
		} else if (!strcmp(cap, "mbss4")) {
			max_no_vifs = 4;
			mbsscap = 1;
		} else if (!strcmp(cap, "wmf"))
			wmf = 1;
		else if (!strcmp(cap, "rxchain_pwrsave"))
			rxchain_pwrsave = 1;
		else if (!strcmp(cap, "radio_pwrsave"))
			radio_pwrsave = 1;
		else if (!strcmp(cap, "wet_tunnel"))
			wet_tunnel_cap = 1;
		else if (!strcmp(cap, "160"))
			cap_160 = TRUE;
		else if (!strcmp(cap, "dyn160"))
			cap_dyn160 = TRUE;

	}
	/* Get MAC address */
	(void)wl_hwaddr(name, (uchar *) buf);
	memcpy(vif_addr, buf, ETHER_ADDR_LEN);
	/* Get instance */
	int offset = 0;
#ifdef __CONFIG_DHDAP__
	if (getRouterBrand() != ROUTER_NETGEAR_R7000P && !strcmp(name, "eth2")) {
		if (!dhd_probe("eth1") && dhd_probe("eth2") && !wl_probe("eth2"))
			offset = 1;
		else if (!dhd_probe("eth2") && dhd_probe("eth1") && !wl_probe("eth1"))
			offset = 1;
	}
#endif
	WL_IOCTL(name, WLC_GET_INSTANCE, &unit, sizeof(unit));

	unit += offset;
#ifdef __CONFIG_DHDAP__
	fprintf(stderr, "dhd probe eth1 %d\n", dhd_probe("eth1"));
	fprintf(stderr, "dhd probe eth2 %d\n", dhd_probe("eth2"));
	fprintf(stderr, "wl probe eth1 %d\n", wl_probe("eth1"));
	fprintf(stderr, "wl probe eth2 %d\n", wl_probe("eth2"));
	fprintf(stderr, "unit %d, offset %d\n", unit, offset);
#endif
	sprintf(tmp, "wl%d_mbss", unit);
	if (mbsscap) {
		nvram_set(tmp, "1");
	} else {
		nvram_set(tmp, "0");
	}
	/* clean up tmp */
	memset(tmp, 0, sizeof(tmp));

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	/* Restore defaults if per-interface parameters do not exist */
	restore_defaults = !nvram_get(strcat_r(prefix, "ifname", tmp));
//      wlconf_validate_all(prefix, restore_defaults);
	nvram_set(strcat_r(prefix, "ifname", tmp), name);
	nvram_set(strcat_r(prefix, "hwaddr", tmp), ether_etoa((uchar *) buf, eaddr));
	snprintf(buf, sizeof(buf), "%d", unit);
	nvram_set(strcat_r(prefix, "unit", tmp), buf);


	int wds_num = 1;
	int wds_enabled = 0;
	char wds_var[32];
	for (wds_num = 1; wds_num <= 10; wds_num++) {
		snprintf(wds_var, sizeof(wds_var), "wl%d_wds%d_enable", unit, wds_num);
		if (atoi(nvram_default_get(wds_var, "0")) != 0)
			wds_enabled = 1;
	}

	/* Bring the interface down */
	if (!wds_enabled)	//workaround for driver hang in SDK7 when wds is enabled ticket #4073
		WL_IOCTL(name, WLC_DOWN, NULL, 0);

	/* Disable all BSS Configs */
	for (i = 0; i < WL_MAXBSSCFG; i++) {
		struct {
			int bsscfg_idx;
			int enable;
		} setbuf;
		setbuf.bsscfg_idx = i;
		setbuf.enable = 0;
#ifdef __CONFIG_DHDAP__
		if (is_dhd) {
			ret = dhd_bssiovar_setint(name, "bss", setbuf.bsscfg_idx, setbuf.enable);
		} else {
#endif
			ret = wl_iovar_set(name, "bss", &setbuf, sizeof(setbuf));
#ifdef __CONFIG_DHDAP__
		}
#endif
		if (ret) {
			wl_iovar_getint(name, "bcmerror", &bcmerr);
			/* fail quietly on a range error since the driver may
			 * support fewer bsscfgs than we are prepared to configure
			 */
			if (bcmerr == BCME_RANGE)
				break;
		}
		if (ret)
			WLCONF_DBG("%d:(%s): setting bsscfg #%d iovar \"bss\" to 0" " (down) failed, ret = %d, bcmerr = %d\n", __LINE__, name, i, ret, bcmerr);
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
	str = nvram_default_get(strcat_r(prefix, "mode", tmp), "dd-wrt");
	ap = (!strcmp(str, "") || !strcmp(str, "ap") || !strcmp(str, "mssid"));
	apsta = (!strcmp(str, "apsta") || !strcmp(str, "apstawet") || ((!strcmp(str, "sta") || !strcmp(str, "wet")) && bclist->count > 1));
	sta = (!strcmp(str, "sta") && bclist->count == 1);
	wds = !strcmp(str, "wds");
	wet = !strcmp(str, "wet") || !strcmp(str, "apstawet");
	mac_spoof = !strcmp(str, "mac_spoof");

	if (wet && apsta) {	/* URE is enabled */
		ure_enab = TRUE;
	}

	if (wl_ap_build) {
		/* Enable MSSID mode if appropriate */
		WL_IOVAR_SETINT(name, "mssid", (bclist->count > 1));
		if (!ure_enab && mbsscap) {
			WL_IOVAR_SETINT(name, "mbss", (bclist->count > 1));	//compatiblitiy with newer drivers
		} else {
			WL_IOVAR_SETINT(name, "mbss", 0);	//compatiblitiy with newer drivers

		}
		/*
		 * Set SSID for each BSS Config
		 */
		for (i = 0; i < bclist->count; i++) {
			bsscfg = &bclist->bsscfgs[i];
			strcat_r(bsscfg->prefix, "ssid", tmp);
			ssid.SSID_len = strlen(nvram_default_get(tmp, "dd-wrt"));
			if (ssid.SSID_len > sizeof(ssid.SSID))
				ssid.SSID_len = sizeof(ssid.SSID);
			strncpy((char *)ssid.SSID, nvram_safe_get(tmp), ssid.SSID_len);
			WLCONF_DBG("wlconfig(%s): configuring bsscfg #%d (%s) with SSID \"%s\"\n", name, bsscfg->idx, bsscfg->ifname, nvram_safe_get(tmp));
			WL_BSSIOVAR_SET(name, "ssid", bsscfg->idx, &ssid, sizeof(ssid));
		}
	}
#define MBSS_UC_IDX_MASK		(51 - 1)

	fprintf(stderr, "base addr %s\n", ether_etoa((uchar *) vif_addr, eaddr));
	if (!ure_enab) {
		/* set local bit for our MBSS vif base */
		ETHER_SET_LOCALADDR(vif_addr);
		fprintf(stderr, "local bit addr %s\n", ether_etoa((uchar *) vif_addr, eaddr));

		/* construct and set other wlX.Y_hwaddr */
/*		for (i = 1; i < bclist->count; i++) {
			snprintf(tmp, sizeof(tmp), "wl%d.%d_hwaddr", unit, i);
			addr = nvram_safe_get(tmp);
				vif_addr[5] = (vif_addr[5] & ~MBSS_UC_IDX_MASK) | (MBSS_UC_IDX_MASK & (vif_addr[5]+i));
				fprintf(stderr,"set new addr %s mask %d\n",ether_etoa((uchar *)vif_addr, eaddr), MBSS_UC_IDX_MASK);
				nvram_set(tmp, ether_etoa((uchar *)vif_addr, eaddr));
		}
*/
		for (i = 1; i < bclist->count; i++) {
			snprintf(tmp, sizeof(tmp), "wl%d.%d_hwaddr", unit, i);
			addr = nvram_safe_get(tmp);
			vif_addr[5] = (vif_addr[5] & ~(max_no_vifs - 1))
			    | ((max_no_vifs - 1) & (vif_addr[5] + 1));

			nvram_set(tmp, ether_etoa((uchar *) vif_addr, eaddr));
		}

		for (i = 0; i < bclist->count; i++) {
			bsscfg = &bclist->bsscfgs[i];
			/* Ignore primary */
			if (bsscfg->idx == 0)
				continue;

			snprintf(tmp, sizeof(tmp), "wl%d.%d_hwaddr", unit, bsscfg->idx);
			ether_atoe(nvram_safe_get(tmp), eaddr);
			WL_BSSIOVAR_SET(name, "cur_etheraddr", bsscfg->idx, eaddr, ETHER_ADDR_LEN);
		}
	} else {		/* One of URE or Proxy STA Repeater is enabled */
		/* URE/PSR is on, so set wlX.1 hwaddr is same as that of primary interface */
		snprintf(tmp, sizeof(tmp), "wl%d.1_hwaddr", unit);
		WL_BSSIOVAR_SET(name, "cur_etheraddr", 1, vif_addr, ETHER_ADDR_LEN);
		nvram_set(tmp, ether_etoa((uchar *) vif_addr, eaddr));
	}

	/* Set AP mode */
	val = (ap || apsta || wds) ? 1 : 0;
//fprintf(stderr, "set ap flag %s\n",name);
	WL_IOCTL(name, WLC_SET_AP, &val, sizeof(val));

//fprintf(stderr, "set apsta flag %s\n",name);
	WL_IOVAR_SETINT(name, "apsta", apsta);

	/* Set mode: WET */
//fprintf(stderr, "set wet flag %s\n",name);
#ifdef __CONFIG_DHDAP__
	if (is_dhd) {
		dhd_iovar_setint(name, "wet", wet);
	} else
#endif
		WL_IOCTL(name, WLC_SET_WET, &wet, sizeof(wet));

//fprintf(stderr, "set spoof flag %s\n",name);
	if (mac_spoof) {
		sta = 1;
		WL_IOVAR_SETINT(name, "mac_spoof", 1);
	} else
		WL_IOVAR_SETINT(name, "mac_spoof", 0);

	WL_IOVAR_SETINT(name, "wet_enab", wet);

	/* For STA configurations, configure association retry time.
	 * Use specified time (capped), or mode-specific defaults.
	 */
//fprintf(stderr, "set sta retry time %s\n",name);
	if (sta || wet || apsta) {
		char *sta_retry_time_name = "sta_retry_time";
		char *assoc_retry_max_name = "assoc_retry_max";

		str = nvram_safe_get(strcat_r(prefix, sta_retry_time_name, tmp));
		WL_IOVAR_SETINT(name, sta_retry_time_name, atoi(str));

		/* Set the wlX_assoc_retry_max, but only if one was specified. */
		if ((str = nvram_get(strcat_r(prefix, assoc_retry_max_name, tmp)))) {
			WL_IOVAR_SETINT(name, assoc_retry_max_name, atoi(str));
		}
	}

	/* Retain remaining WET effects only if not APSTA */
	//      wet &= !apsta;

	/* Set infra: BSS/IBSS (IBSS only for WET or STA modes) */
	val = 1;
	if (wet || sta)
		val = atoi(nvram_default_get(strcat_r(prefix, "infra", tmp), "1"));
//fprintf(stderr, "set infra flag %s\n",name);
	WL_IOCTL(name, WLC_SET_INFRA, &val, sizeof(val));


	/* Set DWDS only for AP or STA modes */
	for (i = 0; i < bclist->count; i++) {
		val = 0;
		bsscfg = &bclist->bsscfgs[i];

		if (ap || sta || (apsta && !wet)) {
			strcat_r(bsscfg->prefix, "dwds", tmp);
			val = atoi(nvram_safe_get(tmp));
		}
		WL_BSSIOVAR_SETINT(name, "dwds", bsscfg->idx, val);
	}

//fprintf(stderr, "set maxassoc flag %s\n",name);
	/* Set The AP MAX Associations Limit */
	if (ap | apsta) {
#ifdef __CONFIG_DHDAP__
		/* check if we have driver maxassoc tuneable value */
		cfg_max_assoc = atoi(nvram_safe_get(strcat_r(prefix, "cfg_maxassoc", tmp)));
		if (cfg_max_assoc <= 0) {
			WL_IOVAR_GETINT(name, "maxassoc", &cfg_max_assoc);
			/* save to nvram */
			snprintf(var, sizeof(var), "%d", cfg_max_assoc);
			nvram_set(tmp, var);
		}
#endif

		max_assoc = val = atoi(nvram_safe_get(strcat_r(prefix, "maxassoc", tmp)));
		if (val > 0) {
#ifdef __CONFIG_DHDAP__
			/* fix for max_assoc value greater than 32 for DHD */
//                      if ((val > DHD_MAX_ASSOC_VAL) && is_dhd) {
//                          val = DHD_MAX_ASSOC_VAL;
			if ((val > cfg_max_assoc) && is_dhd) {
				val = cfg_max_assoc;
				snprintf(var, sizeof(var), "%d", val);
				nvram_set(tmp, var);
			}
#endif
			WL_IOVAR_SETINT(name, "maxassoc", val);
		} else {	/* Get value from driver if not in nvram */
			WL_IOVAR_GETINT(name, "maxassoc", &max_assoc);
		}
	}
//fprintf(stderr, "set bsscfg %s\n",name);
	/* Turn WET tunnel mode ON or OFF */
	if ((ap || apsta) && (wet_tunnel_cap)) {
		if (atoi(nvram_safe_get(strcat_r(prefix, "wet_tunnel", tmp))) == 1) {
			WL_IOVAR_SETINT(name, "wet_tunnel", 1);
			wet_tunnel_enable = 1;
		} else {
			WL_IOVAR_SETINT(name, "wet_tunnel", 0);
		}
	}

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
			if (strlen(preauth) != 0) {
				set_preauth = atoi(preauth);
			}
			wlconf_set_preauth(name, bsscfg->idx, set_preauth);
		}
#endif				/* BCMWPA2 */
		/* Clear BRCM System level Capability IE */
		memset(&brcm_syscap_ie, 0, sizeof(brcm_prop_ie_t));
		brcm_syscap_ie.type = BRCM_SYSCAP_IE_TYPE;

		/* Add WET TUNNEL to IE */
		if (wet_tunnel_enable) {
			brcm_syscap_ie.cap |= BRCM_SYSCAP_WET_TUNNEL;
			fprintf(stderr, "enable wet tunnel\n");
		}

		subprefix = apsta ? prefix : bsscfg->prefix;

		if (ap || (apsta && bsscfg->idx != 0)) {
			val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "bss_maxassoc", tmp)));
			if (val > 0) {
#ifdef __CONFIG_DHDAP__
				/* fix for val greater than 32 for DHD */
//                              if (is_dhd && (val > DHD_BSS_MAXASSOC_VAL)) {
//                                      val = DHD_BSS_MAXASSOC_VAL;
				if (is_dhd && (val > cfg_max_assoc)) {
					val = cfg_max_assoc;
					/* rewrite the nvram contents */
					snprintf(var, sizeof(var), "%d", val);
					nvram_set(tmp, var);
				}
#endif
				WL_BSSIOVAR_SETINT(name, "bss_maxassoc", bsscfg->idx, val);
			} else if (max_assoc > 0) {	/* Set maxassoc same as global if not set */
				snprintf(var, sizeof(var), "%d", max_assoc);
				nvram_set(tmp, var);
			}
		}

		/* Set network type */
		val = atoi(nvram_default_get(strcat_r(subprefix, "closed", tmp), "0"));
		WL_BSSIOVAR_SETINT(name, "closednet", bsscfg->idx, val);

		/* Set the ap isolate mode */
		val = atoi(nvram_default_get(strcat_r(subprefix, "ap_isolate", tmp), "0"));
#ifdef __CONFIG_DHDAP__
		if (is_dhd) {
			DHD_BSSIOVAR_SETINT(name, "ap_isolate", bsscfg->idx, val);
		} else
#endif				/* __CONFIG_DHDAP__ */
			WL_BSSIOVAR_SETINT(name, "ap_isolate", bsscfg->idx, val);

		/* Set the WMF enable mode */
		if (wmf ||
#ifdef __CONFIG_DHDAP__
		    is_dhd ||
#endif
		    0) {
#ifdef __CONFIG_DHDAP__
			if (is_dhd) {
				val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "wmf_bss_enable", tmp)));
				DHD_BSSIOVAR_SETINT(name, "wmf_bss_enable", bsscfg->idx, val);

				val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "wmf_psta_disable", tmp)));
				DHD_BSSIOVAR_SETINT(name, "wmf_psta_disable", bsscfg->idx, val);

			} else {
#endif				/* __CONFIG_DHDAP__ */
				val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "wmf_bss_enable", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_bss_enable", bsscfg->idx, val);

				val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "wmf_psta_disable", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_psta_disable", bsscfg->idx, val);
#ifdef __CONFIG_DHDAP__
			}
#endif				/* __CONFIG_DHDAP__ */
		}

		if (wet) {
			val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "mcast_regen_bss_enable", tmp)));
#ifdef __CONFIG_DHDAP__
			if (is_dhd) {
				DHD_BSSIOVAR_SETINT(name, "mcast_regen_bss_enable", bsscfg->idx, val);
			} else {
#endif				/* __CONFIG_DHDAP__ */
				WL_BSSIOVAR_SETINT(name, "mcast_regen_bss_enable", bsscfg->idx, val);
#ifdef __CONFIG_DHDAP__
			}
#endif				/* __CONFIG_DHDAP__ */
		}

//fprintf(stderr, "set rxchain pwrsave %s\n",name);

		if (rxchain_pwrsave) {
			val = atoi(nvram_ifexists_get(strcat_r(prefix, "rxchain_pwrsave_enable", tmp), "1"));
			WL_BSSIOVAR_SETINT(name, "rxchain_pwrsave_enable", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(prefix, "rxchain_pwrsave_quiet_time", tmp), "1800"));
			WL_BSSIOVAR_SETINT(name, "rxchain_pwrsave_quiet_time", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(prefix, "rxchain_pwrsave_pps", tmp), "10"));
			WL_BSSIOVAR_SETINT(name, "rxchain_pwrsave_pps", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(bsscfg->prefix, "rxchain_pwrsave_stas_assoc_check", tmp), "0"));
			WL_BSSIOVAR_SETINT(name, "rxchain_pwrsave_stas_assoc_check", bsscfg->idx, val);
		}

		if (radio_pwrsave) {
			val = atoi(nvram_ifexists_get(strcat_r(prefix, "radio_pwrsave_enable", tmp), "0"));
			WL_BSSIOVAR_SETINT(name, "radio_pwrsave_enable", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(prefix, "radio_pwrsave_quiet_time", tmp), "1800"));
			WL_BSSIOVAR_SETINT(name, "radio_pwrsave_quiet_time", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(prefix, "radio_pwrsave_pps", tmp), "10"));
			WL_BSSIOVAR_SETINT(name, "radio_pwrsave_pps", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(prefix, "radio_pwrsave_level", tmp), "0"));
			WL_BSSIOVAR_SETINT(name, "radio_pwrsave_level", bsscfg->idx, val);

			val = atoi(nvram_ifexists_get(strcat_r(bsscfg->prefix, "radio_pwrsave_stas_assoc_check", tmp), "0"));
			WL_BSSIOVAR_SETINT(name, "radio_pwrsave_stas_assoc_check", bsscfg->idx, val);
		}

		val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "aspm", tmp)));
		WL_BSSIOVAR_SETINT(name, "aspm", bsscfg->idx, val);

		/* Configure SYSCAP IE to driver */
		if (brcm_syscap_ie.cap) {
			wlconf_set_brcm_syscap_ie(name, bsscfg->idx, BRCM_PROP_OUI, (uchar *) & (brcm_syscap_ie.type), sizeof(brcm_syscap_ie.type) + sizeof(brcm_syscap_ie.cap));
		}

	}
//fprintf(stderr, "get phy type %s\n",name);
	/* Get current phy type */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
	snprintf(buf, sizeof(buf), "%s", WLCONF_PHYTYPE2STR(phytype));
	nvram_set(strcat_r(prefix, "phytype", tmp), buf);

	/* Set up the country code */
	(void)strcat_r(prefix, "country_code", tmp);
	country = nvram_get(tmp);
	(void)strcat_r(prefix, "country_rev", tmp2);
	country_rev = nvram_get(tmp2);
	fprintf(stderr, "country %s\n",country);
	fprintf(stderr, "country_rev %s\n",country_rev);
	if ((country && country[0] != '\0') && (country_rev && country_rev[0] != '\0')) {
		/* Initialize the wl country parameter */
		strncpy(country_spec.country_abbrev, country, WLC_CNTRY_BUF_SZ - 1);
		country_spec.country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
		strncpy(country_spec.ccode, country, WLC_CNTRY_BUF_SZ - 1);
		country_spec.ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';
		country_spec.rev = atoi(country_rev);

		WL_IOVAR_SET(name, "country", &country_spec, sizeof(country_spec));
	} else {
		/* Get the default country code if undefined */
		wl_iovar_get(name, "country", &country_spec, sizeof(country_spec));

		/* Add the new NVRAM variable */
		nvram_set("wl_country_code", country_spec.ccode);
		(void)strcat_r(prefix, "country_code", tmp);
		nvram_set(tmp, country_spec.ccode);
		snprintf(buf, sizeof(buf), "%d", country_spec.rev);
		nvram_set("wl_country_rev", buf);
		(void)strcat_r(prefix, "country_rev", tmp);
		nvram_set(tmp, buf);
	}

//fprintf(stderr, "set reg mode %s\n",name);
	/* Setup regulatory mode */
	strcpy(tmp, "wl_reg_mode");
//      strcat_r(prefix, "reg_mode", tmp);
	if (nvram_default_match(tmp, "off", "off")) {
		val = 0;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));
	} else if (nvram_match(tmp, "h") || nvram_match(tmp, "strict_h")) {
		val = 0;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
		val = 1;
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		radar_enab = TRUE;
		if (nvram_match(tmp, "h"))
			val = 1;
		else
			val = 2;
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));

		/* Set the CAC parameters */
		val = atoi(nvram_safe_get(strcat_r(prefix, "dfs_preism", tmp)));
		wl_iovar_setint(name, "dfs_preism", val);
		val = atoi(nvram_safe_get(strcat_r(prefix, "dfs_postism", tmp)));
		wl_iovar_setint(name, "dfs_postism", val);

//              val = atoi(nvram_safe_get(strcat_r(prefix, "tpc_db", tmp)));
		val = atoi(nvram_safe_get("wl_tpc_db"));
		WL_IOCTL(name, WLC_SEND_PWR_CONSTRAINT, &val, sizeof(val));

	} else if (nvram_match(tmp, "d")) {
		val = 0;
		WL_IOCTL(name, WLC_SET_RADAR, &val, sizeof(val));
		WL_IOCTL(name, WLC_SET_SPECT_MANAGMENT, &val, sizeof(val));
		val = 1;
		WL_IOCTL(name, WLC_SET_REGULATORY, &val, sizeof(val));
	}

	/* Change LED Duty Cycle */
	leddc = (uint32) strtoul(nvram_default_get(strcat_r(prefix, "leddc", tmp), "0x640000"), NULL, 16);
	if (leddc)
		WL_IOVAR_SETINT(name, "leddc", leddc);

	/* Enable or disable the radio */
	val = nvram_default_match(strcat_r(prefix, "radio", tmp), "0", "1");
	val += WL_RADIO_SW_DISABLE << 16;
	WL_IOCTL(name, WLC_SET_RADIO, &val, sizeof(val));

	WL_IOCTL(name, WLC_GET_BANDLIST, bandlist, sizeof(bandlist));
	if (bandlist[0] > 2)
		bandlist[0] = 2;

	*buf = '\0';
	for (i = 1; i <= bandlist[0]; i++)
		if (bandlist[i] == WLC_BAND_5G)
			strcat(buf, "a");
		else if (bandlist[i] == WLC_BAND_2G)
			strcat(buf, "b");

	nvram_set(strcat_r(prefix, "bandlist", tmp), buf);

	/* Get supported phy types */
	WL_IOCTL(name, WLC_GET_PHYLIST, var, sizeof(var));

	nvram_set(strcat_r(prefix, "phytypes", tmp), var);

	/* Get radio IDs */
	*(next = buf) = '\0';
	for (i = 0; i < strlen(var); i++) {
		/* Switch to band */
		val = WLCONF_STR2PHYTYPE(var[i]);
		if (WLCONF_PHYTYPE_11N(val)) {
			WL_GETINT(name, WLC_GET_BAND, &val);
		} else
			val = WLCONF_PHYTYPE2BAND(val);
		WL_IOCTL(name, WLC_SET_BAND, &val, sizeof(val));
		/* Get radio ID on this band */
		WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));
		next += sprintf(next, "%sBCM%X", i ? " " : "", (rev.radiorev & IDCODE_ID_MASK) >> IDCODE_ID_SHIFT);
	}
	nvram_set(strcat_r(prefix, "radioids", tmp), buf);

	/* Set band */
//fprintf(stderr, "set nband %s\n",name);
	str = nvram_get(strcat_r(prefix, "phytype", tmp));
	val = str ? WLCONF_STR2PHYTYPE(str[0]) : PHY_TYPE_G;
	/* For NPHY use band value from NVRAM */
	if (WLCONF_PHYTYPE_11N(val)) {
		str = nvram_get(strcat_r(prefix, "nband", tmp));
		if (str && !strcmp(str, "0") && strstr(nvram_safe_get(strcat_r(prefix, "bandlist", tmp)), "b"))
			str = "2";
		if (str && !strcmp(str, "0") && strstr(nvram_safe_get(strcat_r(prefix, "bandlist", tmp)), "a"))
			str = "1";
		if (str) {
			val = atoi(str);
			nvram_set(strcat_r(prefix, "nband", tmp), str);
		} else {
			WL_GETINT(name, WLC_GET_BAND, &val);
		}
	} else
		val = WLCONF_PHYTYPE2BAND(val);

	WL_SETINT(name, WLC_SET_BAND, val);

	/* Check errors (card may have changed) */
	if (ret) {
		/* default band to the first band in band list */
		val = WLCONF_STR2PHYTYPE(var[0]);
		val = WLCONF_PHYTYPE2BAND(val);
		WL_SETINT(name, WLC_SET_BAND, val);
	}

	/* Store the resolved bandtype */
	bandtype = val;

	/* Get current core revision */
	WL_IOCTL(name, WLC_GET_REVINFO, &rev, sizeof(rev));
	snprintf(buf, sizeof(buf), "%d", rev.corerev);
	nvram_set(strcat_r(prefix, "corerev", tmp), buf);

	/* this does not work for newer AC radios if in nmode (ac does not support GF, but nmode does). blame broadcom for this */
	if ((rev.chipnum == BCM4716_CHIP_ID) || (rev.chipnum == BCM47162_CHIP_ID) ||
	    (rev.chipnum == BCM4748_CHIP_ID) || (rev.chipnum == BCM4331_CHIP_ID) ||
	    (rev.chipnum == BCM43431_CHIP_ID) || (rev.chipnum == BCM5357_CHIP_ID) || (rev.chipnum == BCM53572_CHIP_ID) || (rev.chipnum == BCM43236_CHIP_ID)) {
		int pam_mode = WLC_N_PREAMBLE_GF_BRCM;	/* default GF-BRCM */

		strcat_r(prefix, "mimo_preamble", tmp);
		if (nvram_default_match(tmp, "mm", "mm"))
			pam_mode = WLC_N_PREAMBLE_MIXEDMODE;
		else if (nvram_match(tmp, "gf"))
			pam_mode = WLC_N_PREAMBLE_GF;
		else if (nvram_match(tmp, "gfbrcm"))
			pam_mode = WLC_N_PREAMBLE_GF_BRCM;
		else if (nvram_match(tmp, "auto"))
			pam_mode = -1;
		WL_IOVAR_SETINT(name, "mimo_preamble", pam_mode);
	}
#define AMPDU_DENSITY_8USEC 6

	/* Making default ampdu_density to 8usec in order to improve throughput
	 * of very small packet sizes (64, 88, 128,..).
	 */
	if (rev.chipnum == BCM43217_CHIP_ID)
		WL_IOVAR_SETINT(name, "ampdu_rx_density", AMPDU_DENSITY_8USEC);

	if ((rev.chipnum == BCM5357_CHIP_ID) || (rev.chipnum == BCM53572_CHIP_ID)) {
		val = atoi(nvram_safe_get("coma_sleep"));
		if (val > 0) {
			struct {
				int sleep;
				int delay;
			} setbuf;
			nvram_unset("coma_sleep");
			nvram_commit();
			setbuf.sleep = val;
			setbuf.delay = 1;
			WL_IOVAR_SET(name, "coma", &setbuf, sizeof(setbuf));
		}
	}

	if (WLCONF_PHYTYPE_11N(phytype)) {
		struct {
			int bandtype;
			uint8 bw_cap;
		} param;

		/* Get the user nmode setting now */
		nmode = AUTO;	/* enable by default for NPHY */
		/* Set n mode */
		strcat_r(prefix, "nmode", tmp);
		if (nvram_match(tmp, "0"))
			nmode = OFF;

		WL_IOVAR_SETINT(name, "nmode", (uint32) nmode);

		if (nmode == OFF)
			val = WLC_BW_CAP_20MHZ;
		else
			val = wlconf_bw_cap(prefix, bandtype);

		val &= WLC_BW_CAP_UNRESTRICTED;

		/* record the bw here */
		if (val == WLC_BW_CAP_UNRESTRICTED) {
			if (phytype == PHY_TYPE_AC) {
				nbw = cap_160 ? WL_CHANSPEC_BW_160 : WL_CHANSPEC_BW_80;
			} else {
				nbw = WL_CHANSPEC_BW_40;
			}
		} else if (val == WLC_BW_CAP_160MHZ)
			nbw = WL_CHANSPEC_BW_160;
		else if (val == WLC_BW_CAP_80MHZ)
			nbw = WL_CHANSPEC_BW_80;
		else if (val == WLC_BW_CAP_40MHZ)
			nbw = WL_CHANSPEC_BW_40;

		param.bandtype = bandtype;
		param.bw_cap = (uint8) val;

		WL_IOVAR_SET(name, "bw_cap", &param, sizeof(param));
	} else {
		/* Save n mode to OFF */
		nvram_set(strcat_r(prefix, "nmode", tmp), "0");
	}

	str = nvram_safe_get(strcat_r(prefix, "dyn160", tmp));
	/* validate before setting */
	if (cap_dyn160 && str != NULL && str[0] != '\0' && str[0] >= '0' && str[0] <= '1') {
		val = atoi(str);
		WL_IOVAR_SETINT(name, "dyn160", (uint32) val);
	}

	WL_IOVAR_GETINT(name, "chanspec", &prev_chspec);

	/* Set channel before setting gmode or rateset */
	/* Manual Channel Selection - when channel # is not 0 */
//fprintf(stderr, "set channel %s\n",name);
	val = atoi(nvram_default_get(strcat_r(prefix, "channel", tmp), "0"));
	if (val && !WLCONF_PHYTYPE_11N(phytype)) {
		WL_SETINT(name, WLC_SET_CHANNEL, val);
		if (ret) {
			/* Use current channel (card may have changed) */
			WL_IOCTL(name, WLC_GET_CHANNEL, &ci, sizeof(ci));
			snprintf(buf, sizeof(buf), "%d", ci.target_channel);
			nvram_set(strcat_r(prefix, "channel", tmp), buf);
		}
	} else if (val && WLCONF_PHYTYPE_11N(phytype)) {
		uint channel;
		uint nctrlsb = 0;
		nmode = AUTO;	/* enable by default for NPHY */
		/* Set n mode */
		strcat_r(prefix, "nmode", tmp);
		if (nvram_match(tmp, "0"))
			nmode = OFF;

		channel = val;
		/* Get BW */
		val = atoi(nvram_safe_get(strcat_r(prefix, "nbw", tmp)));
		fprintf(stderr, "nbw = %d\n", val);
		if (nvram_match(strcat_r(prefix, "net_mode", tmp), "b-only") || nvram_match(strcat_r(prefix, "net_mode", tmp), "g-only") || nvram_match(strcat_r(prefix, "net_mode", tmp), "a-only")
		    || nvram_match(strcat_r(prefix, "net_mode", tmp), "bg-mixed"))
			val = 20;

		fprintf(stderr, "channel %d, val %d\n", channel, val);
		switch (val) {
		case 160:
			val = WL_CHANSPEC_BW_160;
			break;
		case 80:
			val = WL_CHANSPEC_BW_80;
			break;
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

		//fprintf(stderr,"nbw:%X, nctrlsb:%X, channel:%X\n",nbw,nctrlsb,channel);
		/* Get Ctrl SB for 40MHz channel */
		if (nbw == WL_CHANSPEC_BW_160) {
			/* Get Ctrl SB for 160MHz channel */
			str = nvram_safe_get(strcat_r(prefix, "nctrlsb", tmp));

			/* Adjust the channel to be center channel */
			channel = channel + CH_80MHZ_APART - CH_10MHZ_APART;

			if (!strcmp(str, "lll")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LLL;
			} else if (!strcmp(str, "llu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LLU;
				channel -= CH_20MHZ_APART;
			} else if (!strcmp(str, "lul")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LUL;
				channel -= 2 * CH_20MHZ_APART;
			} else if (!strcmp(str, "luu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LUU;
				channel -= 3 * CH_20MHZ_APART;
			} else if (!strcmp(str, "ull")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_ULL;
				channel -= 4 * CH_20MHZ_APART;
			} else if (!strcmp(str, "ulu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_ULU;
				channel -= 5 * CH_20MHZ_APART;
			} else if (!strcmp(str, "uul")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_UUL;
				channel -= 6 * CH_20MHZ_APART;
			} else if (!strcmp(str, "uuu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_UUU;
				channel -= 7 * CH_20MHZ_APART;
			}

		} else if (nbw == WL_CHANSPEC_BW_80) {
			/* Get Ctrl SB for 80MHz channel */
			str = nvram_safe_get(strcat_r(prefix, "nctrlsb", tmp));

			/* Adjust the channel to be center channel */
			channel = channel + CH_40MHZ_APART - CH_10MHZ_APART;

			if (!strcmp(str, "ll")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LL;
			} else if (!strcmp(str, "lu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_LU;
				channel -= CH_20MHZ_APART;
			} else if (!strcmp(str, "ul")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_UL;
				channel -= 2 * CH_20MHZ_APART;
			} else if (!strcmp(str, "uu")) {
				nctrlsb = WL_CHANSPEC_CTL_SB_UU;
				channel -= 3 * CH_20MHZ_APART;
			}

		}
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

		uint cspecbw = (bandtype == WLC_BAND_2G) ? WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G;
		/* band | BW | CTRL SB | Channel */
		fprintf(stderr, "%X, %X, %X, %X\n", cspecbw, nbw, nctrlsb, channel);
		chanspec |= (cspecbw | (nbw | nctrlsb | channel));
		fprintf(stderr, "spec %X\n", chanspec);

		WL_IOVAR_SETINT(name, "chanspec", (uint32) chanspec);
	}

	/* Set up number of Tx and Rx streams */
	if (WLCONF_PHYTYPE_11N(phytype)) {
		int count;
		int streams;
		int policy;

		/* Get the number of tx chains supported by the hardware */
		wl_iovar_getint(name, "hw_txchain", &count);
		/* update NVRAM with capabilities */
		snprintf(var, sizeof(var), "%d", count);

		nvram_set(strcat_r(prefix, "hw_txchain", tmp), var);

		/* Verify that there is an NVRAM param for txstreams, if not create it and
		 * set it to hw_txchain
		 */
		streams = atoi(nvram_safe_get(strcat_r(prefix, "txchain", tmp)));
		if (streams == 0) {
			/* invalid - NVRAM needs to be fixed/initialized */
			nvram_set(strcat_r(prefix, "txchain", tmp), var);
			streams = count;
		}
		/* Apply user configured txstreams, use 1 if user disabled nmode */
		WL_IOVAR_SETINT(name, "txchain", streams);

		wl_iovar_getint(name, "hw_rxchain", &count);
		/* update NVRAM with capabilities */
		snprintf(var, sizeof(var), "%d", count);
		nvram_set(strcat_r(prefix, "hw_rxchain", tmp), var);

		/* Verify that there is an NVRAM param for rxstreams, if not create it and
		 * set it to hw_txchain
		 */
		streams = atoi(nvram_safe_get(strcat_r(prefix, "rxchain", tmp)));
		if (streams == 0) {
			/* invalid - NVRAM needs to be fixed/initialized */
			nvram_set(strcat_r(prefix, "rxchain", tmp), var);
			streams = count;
		}

		/* Apply user configured rxstreams, use 1 if user disabled nmode */
		WL_IOVAR_SETINT(name, "rxchain", streams);

		/* update the spatial policy to make chain changes effect */
		if (phytype == PHY_TYPE_HT) {
			WL_IOVAR_GETINT(name, "spatial_policy", &policy);
			WL_IOVAR_SETINT(name, "spatial_policy", policy);
		}
	}

	/* Reset to hardware rateset (band may have changed) */
	WL_IOCTL(name, WLC_GET_RATESET, &rs, sizeof(wl_rateset_t));
	WL_IOCTL(name, WLC_SET_RATESET, &rs, sizeof(wl_rateset_t));

	/* Set gmode */
	if (bandtype == WLC_BAND_2G) {
		int override = WLC_PROTECTION_OFF;
		int control = WLC_PROTECTION_CTL_OFF;

		/* Set gmode */
		gmode = atoi(nvram_default_get(strcat_r(prefix, "gmode", tmp), "1"));
		WL_IOCTL(name, WLC_SET_GMODE, &gmode, sizeof(gmode));

		/* Set gmode protection override and control algorithm */
		strcat_r(prefix, "gmode_protection", tmp);
		if (nvram_default_match(tmp, "auto", "auto")) {
			override = WLC_PROTECTION_AUTO;
			control = WLC_PROTECTION_CTL_OVERLAP;
		}
		WL_IOCTL(name, WLC_SET_GMODE_PROTECTION_OVERRIDE, &override, sizeof(override));
		WL_IOCTL(name, WLC_SET_PROTECTION_CONTROL, &control, sizeof(control));
	}

	/* Set nmode_protectoin */
	if (WLCONF_PHYTYPE_11N(phytype)) {
		int override = WLC_PROTECTION_OFF;
		int control = WLC_PROTECTION_CTL_OFF;
		/* Set n protection override and control algorithm */
		str = nvram_get(strcat_r(prefix, "nmode_protection", tmp));
		if (!str || !strcmp(str, "auto")) {
			override = WLC_PROTECTION_AUTO;
			control = WLC_PROTECTION_CTL_OVERLAP;
		} else if (!strcmp(tmp, "on")) {
			override = WLC_PROTECTION_ON;
			control = WLC_PROTECTION_CTL_OVERLAP;
		} else if (!strcmp(tmp, "off")) {
			override = WLC_PROTECTION_OFF;
			control = WLC_PROTECTION_CTL_OVERLAP;
		} else if (!strcmp(tmp, "cts")) {
			override = WLC_PROTECTION_CTS_ONLY;
			control = WLC_PROTECTION_CTL_OVERLAP;
		} else if (!strcmp(tmp, "mmhdr")) {
			override = WLC_PROTECTION_MMHDR_ONLY;
			control = WLC_PROTECTION_CTL_OVERLAP;
		}

		WL_IOVAR_SETINT(name, "nmode_protection_override", (uint32) override);
		WL_IOCTL(name, WLC_SET_PROTECTION_CONTROL, &control, sizeof(control));

		str = nvram_get(strcat_r(prefix, "gf_protection", tmp));
		if (!str || !strcmp(str, "auto")) {
			override = WLC_PROTECTION_AUTO;
		} else if (!strcmp(tmp, "on")) {
			override = WLC_PROTECTION_ON;
		} else if (!strcmp(tmp, "off")) {
			override = WLC_PROTECTION_OFF;
		}
		WL_IOVAR_SETINT(name, "gf_protection_override", (uint32) override);

	}

	/* Set WME mode */
	/* This needs to be done before afterburner as wme has precedence
	 *   -disable afterburner mode to allow any wme mode be configured
	 *   -set wme mode before set afterburner mode
	 */
	val = OFF;
	strcpy(var, "afterburner_override");
	wl_iovar_setint(name, var, val);
	wme = nvram_default_get(strcat_r(prefix, "wme", tmp), "on");
	val = strcmp(wme, "on") ? 0 : 1;
	wl_iovar_set(name, "wme", &val, sizeof(val));
	if (val)
		wlconf_set_wme(name, prefix);

	/* Set 802.11n required */
	if (nmode != OFF) {
		uint32 nreqd = OFF;	/* default */

		strcat_r(prefix, "nreqd", tmp);

		if (nvram_match(tmp, "1"))
			nreqd = ON;

		WL_IOVAR_SETINT(name, "nreqd", nreqd);
	}

	/*
	 * Set operational capabilities required for stations
	 * to associate to the BSS. Per-BSS setting.
	 */
	for (i = 0; i < bclist->count; i++) {
		bsscfg = &bclist->bsscfgs[i];
		str = nvram_safe_get(strcat_r(prefix, "bss_opmode_cap_reqd", tmp));	//include sub interfaces
//              str = nvram_safe_get(strcat_r(bsscfg->prefix, "bss_opmode_cap_reqd", tmp));
		if (strlen(str) == 0) {
			strcat_r(prefix, "nreqd", tmp);
			if (nvram_match(tmp, "1")) {
				val = 2;
			} else {
				val = 0;
			}
		} else {
			val = atoi(str);
		}
		WL_BSSIOVAR_SETINT(name, "mode_reqd", bsscfg->idx, val);
		val = atoi(nvram_safe_get(strcat_r(bsscfg->prefix, "bsscfg_class", tmp)));
		if (val > 0) {
			WL_BSSIOVAR_SETINT(name, "bsscfg_class", bsscfg->idx, val);
		}
	}

	/* Set vlan_prio_mode */
	{
		uint32 mode = OFF;	/* default */

		strcat_r(prefix, "vlan_prio_mode", tmp);

		if (nvram_match(tmp, "on"))
			mode = ON;

		WL_IOVAR_SETINT(name, "vlan_mode", mode);
	}

	/* Get bluetootch coexistance(BTC) mode */
	btc_mode = atoi(nvram_default_get(strcat_r(prefix, "btc_mode", tmp), "0"));

	/* Set the afterburner, AMPDU and AMSDU options based on the N-mode */
	wme_global = wlconf_aburn_ampdu_amsdu_set(name, prefix, nmode, btc_mode);

	/* Now that wme_global is known, check per-BSS disable settings */
	for (i = 0; i < bclist->count; i++) {
		char *subprefix;
		bsscfg = &bclist->bsscfgs[i];

		subprefix = apsta ? prefix : bsscfg->prefix;

		/* For each BSS, check WME; make sure wme is set properly for this interface */
		strcat_r(subprefix, "wme", tmp);
		nvram_set(tmp, wme_global ? "on" : "off");

		str = nvram_safe_get(strcat_r(bsscfg->prefix, "wme_bss_disable", tmp));
		val = (str[0] == '1') ? 1 : 0;
		WL_BSSIOVAR_SETINT(name, "wme_bss_disable", bsscfg->idx, val);
	}

	/* Set options based on capability */
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
		if ((strncmp(var, "amsdu", sizeof(var)) == 0) || (strncmp(var, "ampdu", sizeof(var)) == 0)) {
			if (val == ON || val == OFF || val == AUTO)
				valid_option = TRUE;
		}
		if (valid_option)
			wl_iovar_setint(name, var, val);
	}
	/* Get current rateset (gmode may have changed) */
	WL_IOCTL(name, WLC_GET_CURR_RATESET, &rs, sizeof(wl_rateset_t));

	strcat_r(prefix, "rateset", tmp);
	if (nvram_default_match(tmp, "all", "default")) {
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

	if (phytype != PHY_TYPE_SSN && phytype != PHY_TYPE_LCN) {
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

	}
	/* Set rateset */
	if (!CHSPEC_IS160(prev_chspec) || bandtype == WLC_BAND_2G || phytype != PHY_TYPE_AC) {
		WL_IOCTL(name, WLC_SET_RATESET, &rs, sizeof(wl_rateset_t));
	}

	/* Allow short preamble override for b cards */
	if (phytype == PHY_TYPE_B || (WLCONF_PHYTYPE_11N(phytype) && (bandtype == WLC_BAND_2G)) || ((phytype == PHY_TYPE_G || phytype == PHY_TYPE_LP) && (gmode == GMODE_LEGACY_B || gmode == GMODE_AUTO))) {
		strcat_r(prefix, "plcphdr", tmp);
		if (nvram_match(tmp, "long"))
			val = WLC_PLCP_AUTO;
		else
			val = WLC_PLCP_SHORT;
		WL_IOCTL(name, WLC_SET_PLCPHDR, &val, sizeof(val));
	}

	/* Set rate in 500 Kbps units */
	val = atoi(nvram_default_get(strcat_r(prefix, "rate", tmp), "0")) / 500000;

	/* Convert Auto mcsidx to Auto rate */
	if (WLCONF_PHYTYPE_11N(phytype)) {
		int mcsidx = atoi(nvram_default_get(strcat_r(prefix, "nmcsidx", tmp), "-1"));
		/* -1 mcsidx used to designate AUTO rate */
		if (mcsidx == -1)
			val = 0;
	}

	/* 1Mbps and 2 Mbps are not allowed in BTC pre-emptive mode */
	if (btc_mode == WL_BTC_PREMPT && (val == 2 || val == 4))
		/* Must b/g band.  Set to 5.5Mbps */
		val = 11;

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
	if (nmode != OFF) {
		uint32 nrate = 0;
		int mcsidx = atoi(nvram_safe_get(strcat_r(prefix, "nmcsidx", tmp)));
		bool ismcs = (mcsidx >= 0);

		/* mcsidx of 32 is valid only for 40 Mhz */
		if (mcsidx == 32 && nbw == WL_CHANSPEC_BW_20) {
			mcsidx = -1;
			ismcs = FALSE;
			nvram_set(strcat_r(prefix, "nmcsidx", tmp), "-1");
		}

		/* Use nrate iovar only for MCS rate. */
		if (ismcs) {
			nrate |= WL_RSPEC_ENCODE_HT;
			nrate |= mcsidx & WL_RSPEC_RATE_MASK;

			WL_IOVAR_SETINT(name, "nrate", nrate);
		}
	}

	/* Set multicast rate in 500 Kbps units */
	val = atoi(nvram_default_get(strcat_r(prefix, "mrate", tmp), "0")) / 500000;
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

	/* Set fragmentation threshold */
	val = atoi(nvram_default_get(strcat_r(prefix, "frag", tmp), "2346"));
	wl_iovar_setint(name, "fragthresh", val);

	/* Set RTS threshold */
	val = atoi(nvram_default_get(strcat_r(prefix, "rts", tmp), "2347"));
	wl_iovar_setint(name, "rtsthresh", val);

	/* Set DTIM period */
	val = atoi(nvram_default_get(strcat_r(prefix, "dtim", tmp), "1"));
	WL_IOCTL(name, WLC_SET_DTIMPRD, &val, sizeof(val));

	/* Set beacon period */
	val = atoi(nvram_default_get(strcat_r(prefix, "bcn", tmp), "100"));
	WL_IOCTL(name, WLC_SET_BCNPRD, &val, sizeof(val));

	/* Set obss dyn bw switch only for 5g */
	val = atoi(nvram_safe_get(strcat_r(prefix, "obss_dyn_bw", tmp)));
	if ((bandtype == WLC_BAND_5G) && (val == 1 || val == 2)) {
		/*  val = 1 means dynamic bwsw is based on rxcrs stats
		 *    val = 2 means dynamic bwsw is based on txop stats
		 */
		WL_IOVAR_SETINT(name, "obss_dyn_bw", val);
	} else {
		WL_IOVAR_SETINT(name, "obss_dyn_bw", 0);
	}

	/* Set beacon rotation */
	str = nvram_get(strcat_r(prefix, "bcn_rotate", tmp));
	if (!str) {
		/* No nvram variable found, use the default */
		str = nvram_default_get(strcat_r(prefix, "bcn_rotate", tmp), "0");
	}
	val = atoi(str);
	wl_iovar_setint(name, "bcn_rotate", val);

	/* Set framebursting mode */
	if (btc_mode == WL_BTC_PREMPT)
		val = FALSE;
	else
		val = nvram_match(strcat_r(prefix, "frameburst", tmp), "on");
	WL_IOCTL(name, WLC_SET_FAKEFRAG, &val, sizeof(val));

	/* Set dynamic frameburst max station limit */
	str = nvram_get(strcat_r(prefix, "frameburst_dyn_max_stations", tmp));
	if (!str) {		/* Fall back to previous name, frameburst_dyn */
		str = nvram_default_get("frameburst_dyn", "0");
	}
	wl_iovar_setint(name, "frameburst_dyn_max_stations", atoi(str));

	/* Enable or disable PLC failover */
	val = atoi(nvram_safe_get(strcat_r(prefix, "plc", tmp)));
	WL_IOVAR_SETINT(name, "plc", val);

	/* AP only config */
	if (ap || apsta || wds) {
		/* Set lazy WDS mode */
		val = atoi(nvram_default_get(strcat_r(prefix, "lazywds", tmp), "0"));
		WL_IOCTL(name, WLC_SET_LAZYWDS, &val, sizeof(val));

		/* Set the WDS list */
		maclist = (struct maclist *)buf;
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

	/* Set the STBC tx mode */
	if (phytype == PHY_TYPE_N || phytype == PHY_TYPE_HT || phytype == PHY_TYPE_AC) {
		char *nvram_str = nvram_safe_get(strcat_r(prefix, "stbc_tx", tmp));

		if (!strcmp(nvram_str, "auto")) {
			WL_IOVAR_SETINT(name, "stbc_tx", AUTO);
		} else if (!strcmp(nvram_str, "on")) {
			WL_IOVAR_SETINT(name, "stbc_tx", ON);
		} else if (!strcmp(nvram_str, "off")) {
			WL_IOVAR_SETINT(name, "stbc_tx", OFF);
		}
		val = atoi(nvram_safe_get(strcat_r(prefix, "stbc_rx", tmp)));
		WL_IOVAR_SETINT(name, "stbc_rx", val);
	}

	/* Set RIFS mode based on framebursting */
	if (WLCONF_PHYTYPE_11N(phytype)) {
		char *nvram_str = nvram_safe_get(strcat_r(prefix, "rifs", tmp));
		if (!strcmp(nvram_str, "on"))
			wl_iovar_setint(name, "rifs", ON);
		else if (!strcmp(nvram_str, "off"))
			wl_iovar_setint(name, "rifs", OFF);

		/* RIFS mode advertisement */
		nvram_str = nvram_safe_get(strcat_r(prefix, "rifs_advert", tmp));
		if (!strcmp(nvram_str, "auto"))
			wl_iovar_setint(name, "rifs_advert", AUTO);
		else if (!strcmp(nvram_str, "off"))
			wl_iovar_setint(name, "rifs_advert", OFF);
	}

	/* Override BA mode only if set to on/off */
	ba = nvram_safe_get(strcat_r(prefix, "ba", tmp));
	if (!strcmp(ba, "on"))
		wl_iovar_setint(name, "ba", ON);
	else if (!strcmp(ba, "off"))
		wl_iovar_setint(name, "ba", OFF);

	if (WLCONF_PHYTYPE_11N(phytype)) {
		val = AVG_DMA_XFER_RATE;
		wl_iovar_set(name, "avg_dma_xfer_rate", &val, sizeof(val));
	}

	if ((BCM53573_CHIP(rev.chipnum)) && (rev.chiprev == 0x0)) {
		/* Temporary : Unset TXBF cap */
		wlconf_unset_txbf(name, prefix);
	} else {
		/* Set up TxBF */
		wlconf_set_txbf(name, prefix);
	}

	/* 4365b1: Set ctdma completion busy dma war */
	if (rev.corerev == 64) {
		val = 0;
		str = nvram_get(strcat_r(prefix, "cpbusy_war", tmp));
		if (str) {
			val = atoi(str);
			WL_IOVAR_SETINT(name, "cpbusy_war", val);
		}
	}

	/* Set maximum MU clients number if needed. */
	if (rev.corerev >= 64) {
		val = 0;
		str = nvram_get(strcat_r(prefix, "max_muclients", tmp));
		if (str) {
			val = atoi(str);
			WL_IOVAR_SETINT(name, "max_muclients", val);
		}
	}


	/* set airtime fairness */
	val = 0;
	str = nvram_get(strcat_r(prefix, "atf", tmp));
	if (str) {
		val = atoi(str);
	}
	WL_IOVAR_SETINT(name, "atf", val);

	str = nvram_get(strcat_r(prefix, "ampdu_atf_us", tmp));
	if (str) {
		val = atoi(str);
		if (val) {
			WL_IOVAR_SETINT(name, "ampdu_atf_us", val);
			WL_IOVAR_SETINT(name, "nar_atf_us", val);
		}
	}

	/* set TAF */
	val = 0;
	str = nvram_get(strcat_r(prefix, "taf_enable", tmp));
	if (str && (bandtype == WLC_BAND_5G)) {
		val = atoi(str);
	}
	wlconf_set_taf(name, val);

	val = 0;
	str = nvram_get(strcat_r(prefix, "taf_rule", tmp));
	if (str) {
		val = strtoul(str, NULL, 0);
		WL_IOVAR_SETINT(name, "taf_rule", val);
	}

#ifdef __CONFIG_DHDAP__
	if (is_dhd) {
		val = atoi(nvram_safe_get(strcat_r(prefix, "scb_alloc", tmp)));
		if (val > 0) {
			WL_IOVAR_SETINT(name, "scb_alloc", val);
		}

		val = atoi(nvram_safe_get(strcat_r(prefix, "scb_alloc_class", tmp)));
		if (val > 0) {
			WL_IOVAR_SETINT(name, "scb_alloc_class", val);
		}

		val = atoi(nvram_safe_get(strcat_r(prefix, "scb_alloc_min_mem", tmp)));
		if (val > 0) {
			WL_IOVAR_SETINT(name, "scb_alloc_min_mem", val);
		}
	}
#endif				/* __CONFIG_DHDAP__ */

	/* Bring the interface back up */
	WL_IOCTL(name, WLC_UP, NULL, 0);

	/* Set phy periodic cal if nvram present. Otherwise, use driver defaults. */
	str = nvram_get(strcat_r(prefix, "cal_period", tmp));
	if (str) {
		/*
		 *  If cal_period is "-1 / Auto"
		 *     - For corerev >= 40, set cal_period to 0
		 *     - For corerev < 40, use driver defaults.
		 *  Else
		 *     - Use the value specified in the nvram.
		 */
		val = atoi(str);
		if (val == -1) {
			if (rev.corerev >= 40) {
				val = 0;
				WL_IOVAR_SET(name, "cal_period", &val, sizeof(val));
			}
		} else {
			WL_IOVAR_SET(name, "cal_period", &val, sizeof(val));
		}
	}

	/* Set antenna */
	val = atoi(nvram_default_get(strcat_r(prefix, "antdiv", tmp), "3"));
	WL_IOCTL(name, WLC_SET_ANTDIV, &val, sizeof(val));

	/* Set antenna selection */
	wlconf_set_antsel(name, prefix);

	/* Set radar parameters if it is enabled */
	if (radar_enab) {
		wlconf_set_radarthrs(name, prefix);
	}

	/* set pspretend */
	val = 0;
	if (ap) {
		bool set_psp;
		/* Set pspretend for multi-ssid bss */
		for (i = 0; i < bclist->count; i++) {
			set_psp = FALSE;
			bsscfg = &bclist->bsscfgs[i];
			str = nvram_safe_get(strcat_r(bsscfg->prefix, "pspretend_retry_limit", tmp));
			if (str && *str != '\0') {
				val = atoi(str);
				set_psp = TRUE;
			} else if (BCM4365_CHIP(rev.chipnum)) {
				/* For 4365/4366 Enable PSP by default if
				 *  not explicitly present in nvram
				 */
				val = PSPRETEND_DEFAULT_THRESHOLD;
				set_psp = TRUE;
			}

			if (set_psp) {
				WL_BSSIOVAR_SETINT(name, "pspretend_retry_limit", bsscfg->idx, val);
			}
		}

		/* now set it for primary bss */
		val = 0;
		str = nvram_get(strcat_r(prefix, "pspretend_retry_limit", tmp));
		if (str && *str != '\0') {
			val = atoi(str);
		} else if (BCM4365_CHIP(rev.chipnum)) {
			/* For 4365/4366 Enable PSP by default if
			 * not explicitly present in nvram
			 */
			val = PSPRETEND_DEFAULT_THRESHOLD;
		}
	}
	WL_IOVAR_SETINT(name, "pspretend_retry_limit", val);

	val = 0;
	if (ap) {
		str = nvram_get(strcat_r(prefix, "pspretend_threshold", tmp));
		if (str) {
			val = atoi(str);
		}
	}
	WL_IOVAR_SETINT(name, "pspretend_threshold", val);

	/* Set channel interference threshold value if it is enabled */
	str = nvram_get(strcat_r(prefix, "glitchthres", tmp));

	if (str) {
		int glitch_thres = atoi(str);
		if (glitch_thres > 0)
			WL_IOVAR_SETINT(name, "chanim_glitchthres", glitch_thres);
	}


	str = nvram_get(strcat_r(prefix, "ccathres", tmp));

	if (str) {
		int cca_thres = atoi(str);
		if (cca_thres > 0)
			WL_IOVAR_SETINT(name, "chanim_ccathres", cca_thres);
	}

	str = nvram_get(strcat_r(prefix, "chanimmode", tmp));

	if (str) {
		int chanim_mode = atoi(str);
		if (chanim_mode >= 0)
			WL_IOVAR_SETINT(name, "chanim_mode", chanim_mode);
	}

	/* Overlapping BSS Coexistence aka 20/40 Coex. aka OBSS Coex.
	 * For an AP - Only use if 2G band AND user wants a 40Mhz chanspec.
	 * For a STA - Always
	 */
	if (WLCONF_PHYTYPE_11N(phytype)) {
		if (sta || ((ap || apsta) && (nbw == WL_CHANSPEC_BW_40) && (bandtype == WLC_BAND_2G))) {
			str = nvram_safe_get(strcat_r(prefix, "obss_coex", tmp));
			if (!str) {
				/* No nvram variable found, use the default */
				str = nvram_default_get(strcat_r(prefix, "obss_coex", tmp), "0");
			}
			obss_coex = atoi(str);
		} else {
			/* Need to disable obss coex in case of 20MHz and/or
			 * in case of 5G.
			 */
			obss_coex = 0;
		}
		WL_IOVAR_SETINT(name, "obss_coex", obss_coex);
	}

	/* Set up TxBF timer */
	wlconf_set_txbf_timer(name, prefix);

	/* Auto Channel Selection - when channel # is 0 in AP mode
	 *
	 * The following condition(s) must be met in order for
	 * Auto Channel Selection to work.
	 *  - the I/F must be up for the channel scan
	 *  - the AP must not be supporting a BSS (all BSS Configs must be disabled)
	 */
	if (ap || apsta) {
		int channel = chanspec ? wf_chspec_ctlchan(chanspec) : 0;
		if (obss_coex || channel == 0) {
			if (WLCONF_PHYTYPE_11N(phytype)) {
				chanspec_t chanspec;
				int pref_chspec;

				if (channel != 0) {
					/* assumes that initial chanspec has been set earlier */
					/* Maybe we expand scope of chanspec from above so
					 * that we don't have to do the iovar_get here?
					 */

					/* We're not doing auto-channel, give the driver
					 * the preferred chanspec.
					 */
					WL_IOVAR_GETINT(name, "chanspec", &pref_chspec);
					WL_IOVAR_SETINT(name, "pref_chanspec", pref_chspec);
				} else {
					WL_IOVAR_SETINT(name, "pref_chanspec", 0);
				}

				chanspec = wlconf_auto_chanspec(name);
				if (chanspec != 0)
					WL_IOVAR_SETINT(name, "chanspec", chanspec);
			} else {
				/* select a channel */
				val = wlconf_auto_channel(name);
				/* switch to the selected channel */
				if (val != 0)
					WL_IOCTL(name, WLC_SET_CHANNEL, &val, sizeof(val));
			}
			/* set the auto channel scan timer in the driver when in auto mode */
			if (channel == 0) {
				val = 15;	/* 15 minutes for now */
			} else {
				val = 0;
			}
		} else {
			/* reset the channel scan timer in the driver when not in auto mode */
			val = 0;
		}

		WL_IOCTL(name, WLC_SET_CS_SCAN_TIMER, &val, sizeof(val));
		WL_IOVAR_SETINT(name, "chanim_mode", CHANIM_ACT);

	}

	/* Security settings for each BSS Configuration */
	for (i = 0; i < bclist->count; i++) {
		bsscfg = &bclist->bsscfgs[i];
		wlconf_security_options(name, bsscfg->prefix, bsscfg->idx, wet, rev.chipnum != BCM43602_CHIP_ID);
	}

	/*
	 * Finally enable BSS Configs or Join BSS
	 *
	 * AP: Enable BSS Config to bring AP up only when nas will not run
	 * STA: Join the BSS regardless.
	 */
	for (i = 0; i < bclist->count; i++) {

		bsscfg = &bclist->bsscfgs[i];
		struct {
			int bsscfg_idx;
			int enable;
		} setbuf;
		char vifname[VIFNAME_LEN];
		char *name_ptr = name;

		setbuf.bsscfg_idx = bsscfg->idx;
		setbuf.enable = 1;

		/* NAS runs if we have an AKM or radius authentication */
//              nas_will_run = wlconf_akm_options(bclist->bsscfgs[i].prefix) ||
//                      nvram_default_match(strcat_r(bclist->bsscfgs[i].prefix, "auth_mode", tmp),
//                                  "radius","disabled");
		fprintf(stderr, "enable bss %s %d (%d)\n", name, i, nas_will_run);

		/* Set the MAC list */
		maclist = (struct maclist *)buf;
		maclist->count = 0;
		if (nvram_match(strcat_r(bsscfg->prefix, "macmode", tmp), "allow") || nvram_match(strcat_r(bsscfg->prefix, "macmode", tmp), "deny")) {
			ea = maclist->ea;
			foreach(var, nvram_safe_get(strcat_r(bsscfg->prefix, "maclist", tmp)), next) {
				if (((char *)((&ea[1])->octet)) > ((char *)(&buf[sizeof(buf)])))
					break;
				if (ether_atoe(var, ea->octet)) {
					maclist->count++;
					ea++;
				}
			}
		}

		if (setbuf.bsscfg_idx == 0) {
			name_ptr = name;
		} else {	/* Non-primary BSS; changes name syntax */
			char tmp[VIFNAME_LEN];
			int len;

			/* Remove trailing _ if present */
			memset(tmp, 0, sizeof(tmp));
			strncpy(tmp, bsscfg->prefix, VIFNAME_LEN - 1);
			if (((len = strlen(tmp)) > 0) && (tmp[len - 1] == '_')) {
				tmp[len - 1] = 0;
			}
			nvifname_to_osifname(tmp, vifname, VIFNAME_LEN);
			name_ptr = vifname;
		}

		WL_IOCTL(name_ptr, WLC_SET_MACLIST, buf, sizeof(buf));

		/* Set macmode for each VIF */
		(void)strcat_r(bsscfg->prefix, "macmode", tmp);

		if (nvram_match(tmp, "deny"))
			val = WLC_MACMODE_DENY;
		else if (nvram_match(tmp, "allow"))
			val = WLC_MACMODE_ALLOW;
		else
			val = WLC_MACMODE_DISABLED;

		WL_IOCTL(name_ptr, WLC_SET_MACMODE, &val, sizeof(val));

		if (((ap || apsta) && !nas_will_run) || sta || (wet && !apsta)) {

			for (ii = 0; ii < MAX_BSS_UP_RETRIES; ii++) {
				if (wl_ap_build) {
#ifdef __CONFIG_DHDAP__
					if (is_dhd) {
						DHD_BSSIOVAR_SETINT(name, "bss", setbuf.bsscfg_idx, setbuf.enable);
					} else {
#endif
						WL_IOVAR_SET(name, "bss", &setbuf, sizeof(setbuf));
#ifdef __CONFIG_DHDAP__
					}
#endif
				}
				{
					strcat_r(prefix, "ssid", tmp);
					ssid.SSID_len = strlen(nvram_safe_get(tmp));
					if (ssid.SSID_len > sizeof(ssid.SSID))
						ssid.SSID_len = sizeof(ssid.SSID);
					strncpy((char *)ssid.SSID, nvram_safe_get(tmp), ssid.SSID_len);
					WL_IOCTL(name, WLC_SET_SSID, &ssid, sizeof(ssid));
				}
				if (apsta && (ret != 0))
					sleep_ms(1000);
				else
					break;
			}
		}
	}


#ifdef __CONFIG_DHDAP__
	if (is_dhd) {
		if (nvram_match(strcat_r(prefix, "wmf_bss_enable", tmp), "1")) {
			val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_ucigmp_query", tmp)));
			(void)dhd_iovar_setint(name, "wmf_ucast_igmp_query", val);
			val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_mdata_sendup", tmp)));
			(void)dhd_iovar_setint(name, "wmf_mcast_data_sendup", val);
			val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_ucast_upnp", tmp)));
			(void)dhd_iovar_setint(name, "wmf_ucast_upnp", val);
			val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_igmpq_filter", tmp)));
			(void)dhd_iovar_setint(name, "wmf_igmpq_filter", val);
		}
	} else {
#endif				/* __CONFIG_DHDAP__ */
		for (i = 0; i < bclist->count; i++) {
			bsscfg = &bclist->bsscfgs[i];
			if (nvram_match(strcat_r(bsscfg->prefix, "wmf_bss_enable", tmp), "1")) {
				strncpy(prefix, bsscfg->prefix, PREFIX_LEN - 1);
				val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_ucigmp_query", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_ucast_igmp_query", bsscfg->idx, val);
				val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_mdata_sendup", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_mcast_data_sendup", bsscfg->idx, val);
				val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_ucast_upnp", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_ucast_upnp", bsscfg->idx, val);
				val = atoi(nvram_safe_get(strcat_r(prefix, "wmf_igmpq_filter", tmp)));
				WL_BSSIOVAR_SETINT(name, "wmf_igmpq_filter", bsscfg->idx, val);
			}
		}
#ifdef __CONFIG_DHDAP__
	}
#endif

#ifdef __CONFIG_DHDAP__
	if (is_dhd) {
		val = nvram_geti(strcat_r(prefix, "lbr_aggr_en_mask", tmp));
		(void)dhd_iovar_setint(name, "lbr_aggr_en_mask", val);
		val = nvram_geti(strcat_r(prefix, "lbr_aggr_release_timeout", tmp));
		if (val) {
			(void)dhd_iovar_setint(name, "lbr_aggr_release_timeout", val);
		}
		val = atoi(nvram_safe_get(strcat_r(prefix, "lbr_aggr_len", tmp)));
		if (val) {
			(void)dhd_iovar_setint(name, "lbr_aggr_len", val);
		}
	}
#endif				/* __CONFIG_DHDAP__ */

	ret = 0;
exit:
	if (bclist != NULL)
		free(bclist);

	return ret;
}

int wlconf_down(char *name)
{
	int val, ret = 0;
	int i;
	int wlsubunit;
	int bcmerr;
	unsigned char buf[WLC_IOCTL_MAXLEN];
	struct maclist *maclist;
	struct {
		int bsscfg_idx;
		int enable;
	} setbuf;
	int wl_ap_build = 0;	/* 1 = wl compiled with AP capabilities */
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next;
	wlc_ssid_t ssid;
	int is_dhd = 0;

	/* wlconf doesn't work for virtual i/f */
	if (get_ifname_unit(name, NULL, &wlsubunit) == 0 && wlsubunit >= 0) {
		WLCONF_DBG("wlconf: skipping virtual interface \"%s\"\n", name);
		return 0;
	}

	/* Check interface (fail silently for non-wl interfaces) */
	if ((ret = wl_probe(name)))
		return ret;

#ifdef __CONFIG_DHDAP__
	/* Check if interface uses dhd adapter */
	is_dhd = !dhd_probe(name);
#endif				/* __CONFIG_DHDAP__ */

	/* because of ifdefs in wl driver,  when we don't have AP capabilities we
	 * can't use the same iovars to configure the wl.
	 * so we use "wl_ap_build" to help us know how to configure the driver
	 */
	if (wl_iovar_get(name, "cap", (void *)caps, sizeof(caps)))
		return -1;

	foreach(cap, caps, next) {
		if (!strcmp(cap, "ap")) {
			wl_ap_build = 1;
		}
	}

	if (wl_ap_build) {
		/* Bring down the interface */
		WL_IOCTL(name, WLC_DOWN, NULL, 0);

		/* Disable all BSS Configs */
		for (i = 0; i < WL_MAXBSSCFG; i++) {
			setbuf.bsscfg_idx = i;
			setbuf.enable = 0;
#ifdef __CONFIG_DHDAP__
			if (is_dhd) {
				ret = dhd_bssiovar_setint(name, "bss", setbuf.bsscfg_idx, setbuf.enable);
			} else {
#endif
				ret = wl_iovar_set(name, "bss", &setbuf, sizeof(setbuf));
#ifdef __CONFIG_DHDAP__
			}
#endif
			if (ret) {
				wl_iovar_getint(name, "bcmerror", &bcmerr);
				/* fail quietly on a range error since the driver may
				 * support fewer bsscfgs than we are prepared to configure
				 */
				if (bcmerr == BCME_RANGE)
					break;
			}
		}
	} else {
		WL_IOCTL(name, WLC_GET_UP, &val, sizeof(val));
		if (val) {
			/* Nuke SSID  */
			ssid.SSID_len = 0;
			ssid.SSID[0] = '\0';
			WL_IOCTL(name, WLC_SET_SSID, &ssid, sizeof(ssid));

			/* Bring down the interface */
			WL_IOCTL(name, WLC_DOWN, NULL, 0);
		}
	}

	/* Nuke the WDS list */
	maclist = (struct maclist *)buf;
	maclist->count = 0;
	WL_IOCTL(name, WLC_SET_WDSLIST, buf, sizeof(buf));

	return 0;
}

#if defined(linux)
int main(int argc, char *argv[])
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
