/*
 * Broadcom wireless network adapter utility functions
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wlutils.h,v 1.3 2005/11/11 09:26:19 seg Exp $
 */

#ifndef _wlutils_h_
#define _wlutils_h_

#include <dd_list.h>
#include <typedefs.h>
#include <stdint.h>
#include <utils.h>
#ifndef _NO_WLIOCTL_H
#include <wlioctl.h>
#endif
#ifndef WLC_IOCTL_SMLEN
#define WLC_IOCTL_SMLEN \
	256 /* "small" length ioctl buffer
					 * required */
#endif
#ifndef BCME_BUFTOOSHORT
#define BCME_BUFTOOSHORT -14 /* Buffer too short */
#endif

#ifdef HAVE_MADWIFI
#define WIFINAME "wlan"
#else
#define WIFINAME "wl"
#endif

/*
 * get wireless interface 
 */
extern char *get_wdev(void);

extern int get_wl_instance(char *ifname);
extern int get_wl_instances(void);
extern char *get_wl_instance_name(int instance);
extern int get_maxbssid(char *ifname);

long long wifi_getrate(char *ifname);
int wifi_gettxpower(char *ifname);
int wifi_gettxpoweroffset(char *ifname);
int bcm_gettxpower(char *wlname);

void set_maclist(char *iface, char *buf);

void security_disable(char *iface);

void security_deny(char *iface);

void security_allow(char *iface);

void kick_mac(char *iface, char *mac);

extern int HETxRate(unsigned int mcs, unsigned int vhtmcs, unsigned int bw);
extern int VHTTxRate(unsigned int mcs, unsigned int vhtmcs, unsigned int sgi, unsigned int bw);

/*
 * Pass a wlioctl request to the specified interface.
 * @param       name    interface name
 * @param       cmd     WLC_GET_MAGIC <= cmd < WLC_LAST
 * @param       buf     buffer for passing in and/or receiving data
 * @param       len     length of buf
 * @return      >= 0 if successful or < 0 otherwise
 */
extern int wl_ioctl(char *name, int cmd, void *buf, int len);
#ifdef HAVE_DHDAP
extern int dhd_ioctl(char *name, int cmd, void *buf, int len);
#endif

/*
 * Get the MAC (hardware) address of the specified interface.
 * @param       name    interface name
 * @param       hwaddr  6-byte buffer for receiving address
 * @return      >= 0 if successful or < 0 otherwise
 */
extern int wl_hwaddr(char *name, unsigned char *hwaddr);

/*
 * Probe the specified interface.
 * @param       name    interface name
 * @return      >= 0 if a Broadcom wireless device or < 0 otherwise
 */
extern int wl_probe(char *name);
#ifdef HAVE_DHDAP
extern int dhd_probe(char *name);
#endif
/*
 * Returns the list of associated stations in the pre-existing buffer list 
 */
int getchannels(unsigned int *list, char *ifname);
int getwdslist(char *name, unsigned char *list);

#ifdef HAVE_QTN
int rpc_qtn_ready(void);
int getassoclist_qtn(char *name, unsigned char *list);
int getNoiseIndex_qtn(char *ifname, int index);
int getRssiIndex_qtn(char *ifname, int index);
#endif
struct frequency {
	struct dd_list_head list;
	unsigned int freq;
	bool in_use;
	bool passive;
	int quality;
	unsigned long long active;
	unsigned long long busy;
	unsigned long long rx_time;
	unsigned long long tx_time;
	int rx_time_count;
	int tx_time_count;
	int busy_count;
	int active_count;
	int noise;
	int noise_count;
	int eirp;
};

struct wifi_channels {
	int channel;
	int freq;
	int noise;
	int max_eirp;
	int hw_eirp;
	int band; // band number/index
	unsigned int no_outdoor : 1, no_indoor : 1, no_ofdm : 1, no_cck : 1, ptp_only : 1, ptmp_only : 1, passive_scan : 1,
		no_ibss : 1, //
		lll : 1, llu : 1, lul : 1, luu : 1, ull : 1, ulu : 1, uul : 1, uuu : 1, //
		ht40 : 1, vht80 : 1, vht160 : 1, //
		dfs : 1;
};

struct wifi_client_info {
	char ifname[20];
	char radioname[16];
	char is_wds;
	char etheraddr[6];
	uint32_t uptime;
	uint16_t txrate;
	uint32_t rxrate;
	int8_t signal;
	int8_t signal_avg;
	uint32_t noise;
	uint32_t snr;
	int8_t chaininfo[4];
	int8_t chaininfo_avg[4];
	int8_t mcs;
	int8_t rx_mcs;
	unsigned int is_40mhz : 1, is_80mhz : 1, is_160mhz : 1, is_80p80mhz : 1, is_ht : 1, is_vht : 1, is_short_gi : 1,
		rx_is_40mhz : 1, rx_is_80mhz : 1, rx_is_160mhz : 1, rx_is_80p80mhz : 1, rx_is_ht : 1, rx_is_vht : 1,
		rx_is_short_gi : 1, ht40intol : 1, islzo : 1, ps : 1, rx_is_he : 1, is_he : 1;
	uint32_t inactive_time;
	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_compressed;
	uint32_t tx_compressed;
	uint32_t rx_bytes;
	uint32_t tx_bytes;
	struct wifi_client_info *next;
};

struct mac80211_info {
	struct wifi_client_info *wci;
	int8_t noise;
	unsigned long long channel_active_time;
	unsigned long long channel_busy_time;
	unsigned long long channel_receive_time;
	unsigned long long channel_transmit_time;
	unsigned long long extension_channel_busy_time;
	uint32_t frequency;
};

void mac80211_lock(void);
void mac80211_unlock(void);
struct mac80211_info *getcurrentsurvey_mac80211(const char *interface, struct mac80211_info *mac80211_info);
int getsurveystats(struct dd_list_head *frequencies, struct wifi_channels **channels, const char *interface, char *freq_range,
		   int scans, int bw);

int getassoclist(char *name, unsigned char *list);

#ifdef HAVE_IPQ6018
#define DEFAULT_BF 1
#else
#define DEFAULT_BF 0
#endif


#define INFO_UPTIME 0
#define INFO_RSSI 1
#define INFO_NOISE 2
#define INFO_RXRATE 3
#define INFO_TXRATE 4

int getWifiInfo_ath9k(char *ifname, unsigned char *mac, int field); // only used internal
int getWifiInfo(char *ifname, unsigned char *mac, int field);

#define getNoise(ifname, mac) getWifiInfo(ifname, mac, INFO_NOISE)
#define getRssi(ifname, mac) getWifiInfo(ifname, mac, INFO_RSSI)
#define getTxRate(ifname, mac) getWifiInfo(ifname, mac, INFO_TXRATE)
#define getRxRate(ifname, mac) getWifiInfo(ifname, mac, INFO_RXRATE)
#define getUptime(ifname, mac) getWifiInfo(ifname, mac, INFO_UPTIME)
int getValueFromPath(char *path, int dev, char *fmt, int *err); // internal

int getassoclist_11n(char *name, unsigned char *list);
int getNoise_11n(char *ifname, unsigned char *mac);
int getUptime_11n(char *ifname, unsigned char *mac);
int getRssi_11n(char *ifname, unsigned char *mac);

extern int getassoclist_ath9k(char *name, unsigned char *list);

extern int has_mimo(const char *prefix);
extern int has_ac(const char *prefix);
#ifdef HAVE_WIL6210
extern int has_ad(const char *prefix);
#else
static inline int has_ad(const char *prefix)
{
	return 0;
}
#endif

#ifdef HAVE_ATH9K
extern int has_ibss(const char *prefix);
#ifdef HAVE_MAC80211_MESH
extern int has_mesh(const char *prefix);
extern int has_tdma(const char *prefix);
#else
static inline int has_mesh(const char *prefix)
{
	return 0;
}

static inline int has_tdma(const char *prefix)
{
	return 0;
}
#endif
extern int has_gcmp(const char *prefix);
extern int has_cmac(const char *prefix);
extern int has_gcmp_128(const char *prefix);
extern int has_gcmp_256(const char *prefix);
extern int has_ccmp_256(const char *prefix);
extern int has_gmac_128(const char *prefix);
extern int has_gmac_256(const char *prefix);
extern int has_cmac_256(const char *prefix);
#else
static inline int has_mesh(const char *prefix)
{
	return 0;
}

static inline int has_ibss(const char *prefix)
{
	return 1;
}

static inline int has_tdma(const char *prefix)
{
	return 0;
}

static inline int has_gcmp(const char *prefix)
{
	return 0;
}

static inline int has_cmac(const char *prefix)
{
	return 0;
}

static inline int has_gcmp_128(const char *prefix)
{
	return 0;
}

static inline int has_gcmp_256(const char *prefix)
{
	return 0;
}

static inline int has_ccmp_256(const char *prefix)
{
	return 0;
}

static inline int has_gmac_128(const char *prefix)
{
	return 0;
}

static inline int has_gmac_256(const char *prefix)
{
	return 0;
}

static inline int has_cmac_256(const char *prefix)
{
	return 0;
}
#endif

#ifdef HAVE_QTN
extern int has_qtn(const char *prefix);
#else
static inline int has_qtn(const char *prefix)
{
	return 0;
}
#endif

extern int has_athmask(int devnum, int mask);
extern int has_2ghz(const char *prefix);
extern int is_wrt3200(void);
extern int has_5ghz(const char *prefix);
#ifdef HAVE_ATH9K
extern int can_ht40(const char *prefix);
extern int can_vht80(const char *prefix);
extern int can_vht160(const char *prefix);
#else
#define can_ht40(prefix) 0
#define can_vht80(prefix) 0
#define can_vht160(prefix) 0
#endif
#ifdef HAVE_80211AC
extern int has_beamforming(const char *prefix);
extern int has_mumimo(const char *prefix);
#endif

#define SITE_SURVEY_DB "/tmp/site_survey"
#define SITE_SURVEY_NUM 256
#define SCAN_HT20 1
#define SCAN_HT40 2
#define SCAN_VHT80 4

#define CAP_MESH 0x1
#define CAP_HT 0x2
#define CAP_VHT 0x4
#define CAP_SECCHANNEL 0x8
#define CAP_DWDS 0x10
#define CAP_MTIKWDS 0x20
#define CAP_AX 0x40

struct site_survey_list {
	char SSID[33];
	char BSSID[18];
	uint16 channel; /* Channel no. */
	uint16 frequency; /* Frequency i.e. for superchannel */
	int16 RSSI; /* receive signal strength (in dBm) */
	int16 phy_noise; /* noise (in dBm) */
	uint16 beacon_period; /* units are Kusec */
	uint16 capability; /* Capability information */
	uint16 extcap; /* anything else custom for internal use */
	char ENCINFO[128]; /* encryption info */
	uint rate_count; /* # rates in this set */
	uint8 dtim_period; /* DTIM period */
	unsigned long long active; /* channel active time */
	unsigned long long busy; /* channel busy time */
	int16 numsta;
	char radioname[16]; /* in dd-wrt typically the router name from setup page */
};

struct site_survey_list *open_site_survey(char *name);

struct wifi_interface {
	int freq;
	int width;
	int center1;
	int center2;
	int txpower;
	unsigned int he:1,vht:1,ht:1;
};

typedef struct STAINFO {
	char mac[6];
	char rssi;
	char noise;
	char ifname[32];
} STAINFO;

extern STAINFO *getRaStaInfo(char *ifname);

int wlconf_down(char *name);
int wlconf_up(char *name);
#if defined(HAVE_ATH11K)
extern int has_he160(const char *interface);
#else
static inline int has_he160(const char *prefix)
{
	return 0;
}
#endif

#if defined(HAVE_MADWIFI) || defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
const char *get_channeloffset(char *prefix, int *iht, int *channeloffset);
extern struct wifi_channels *list_channels_11n(char *devnr);
extern struct wifi_channels *list_channels_ath9k(char *devnr, char *country, int max_bandwidth_khz, unsigned char band);
extern int getdevicecount(void);

extern int mac80211_get_maxpower(char *interface);
extern int mac80211_get_coverageclass(char *interface);
extern struct mac80211_info *mac80211_assoclist(char *interface);
extern char *mac80211_get_caps(const char *interface, int shortgi, int greenfield, int ht40, int ldpc, int smps);
extern int has_greenfield(const char *interface);
#ifdef HAVE_ATH9K
extern int has_airtime_fairness(const char *prefix);
#ifdef HAVE_WPA3
extern int has_airtime_policy(const char *prefix);
#else
static inline int has_airtime_policy(const char *prefix)
{
	return 0;
}
#endif
extern int has_shortgi(const char *interface);
extern int has_uapsd(const char *interface);
extern int has_ldpc(const char *interface);
extern int has_smps(const char *interface);
extern int has_dynamic_smps(const char *interface);
extern int has_static_smps(const char *interface);
extern int has_ht(const char *interface);
#else
static inline int has_airtime_fairness(const char *prefix)
{
	return 0;
}
static inline int has_airtime_policy(const char *prefix)
{
	return 0;
}

static inline int has_shortgi(const char *prefix)
{
	return 0;
}
static inline int has_uapsd(const char *prefix)
{
	return 0;
}
static inline int has_ldpc(const char *prefix)
{
	return 0;
}
static inline int has_smps(const char *prefix)
{
	return 0;
}
static inline int has_ht(const char *prefix)
{
	return 0;
}
static inline int has_static_smps(const char *prefix)
{
	return 0;
}
static inline int has_dynamic_smps(const char *prefix)
{
	return 0;
}

#endif
#ifdef HAVE_MADWIFI
#ifdef HAVE_ATH9K
extern int has_acktiming(const char *prefix);
#else
static inline int has_acktiming(const char *prefix)
{
	return 1;
}
#endif
#elif defined(HAVE_RT2880) || defined(HAVE_RT61)
static inline int has_acktiming(const char *prefix)
{
	return 0;
}
#else
extern int has_acktiming(const char *prefix);
#endif

#if defined(HAVE_ATH10K) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
extern int has_vht160(const char *interface);
extern int has_vht80(const char *interface);
extern int has_vht80plus80(const char *interface);
int has_subeamforming(const char *interface);
int has_mubeamforming(const char *interface);
extern char *mac80211_get_vhtcaps(const char *interface, int shortgi, int vht80, int vht160, int vht8080, int subf, int mubf);
#else
static inline int has_subeamforming(const char *prefix)
{
	return 0;
}

static inline int has_mubeamforming(const char *prefix)
{
	return 0;
}

#ifdef HAVE_MADWIFI
static inline int has_vht160(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_MADWIFI
static inline int has_vht80(const char *prefix)
{
	return 0;
}
#endif
static inline int has_vht80plus80(const char *prefix)
{
	return 0;
}

#endif

struct unl;
extern int mac80211_check_band(const char *interface, int checkband);
struct wifi_channels *mac80211_get_channels(struct unl *local_unl, const char *interface, const char *country,
					    int max_bandwidth_khz, unsigned char checkband, int nocache);
struct wifi_channels *mac80211_get_channels_simple(const char *interface, const char *country, int max_bandwidth_khz,
						   unsigned char checkband);
#define AUTO_FORCEHT40 1
#define AUTO_FORCEVHT80 2
#define AUTO_FORCEVHT160 4
#define AUTO_ALL 0

extern struct mac80211_ac *mac80211autochannel(const char *interface, char *freq_range, int scans, int enable_passive, int htflags);
extern void mac80211_set_antennas(char *prefix, uint32_t tx_ant, uint32_t rx_ant);
extern int mac80211_get_avail_tx_antenna(char *prefix);
extern int mac80211_get_avail_rx_antenna(char *prefix);

extern struct wifi_interface *mac80211_get_interface(char *dev);
extern int mac80211_get_configured_tx_antenna(char *prefix);
extern int mac80211_get_configured_rx_antenna(char *prefix);
extern int mac80211_check_valid_frequency(const char *interface, char *country, int freq);
extern int getFrequency_mac80211(const char *interface);
extern int mac80211_get_phyidx_by_vifname(const char *vif);

struct mac80211_ac {
	int freq;
	int8_t noise;
	int quality;
	int clear;
	unsigned int lll : 1, llu : 1, lul : 1, luu : 1, ull : 1, ulu : 1, uul : 1, uuu : 1;
	struct mac80211_ac *next;
};

extern void free_wifi_clients(struct wifi_client_info *wci);
extern void free_mac80211_ac(struct mac80211_ac *ac);
#else
#define has_airtime_fairness(prefix) 0
#define has_shortgi(prefix) 0
#define has_subeamforming(prefix) 0
#define has_mubeamforming(prefix) 0
#define has_vht80(interface) 0
#if !defined(HAVE_MADWIFI) && (defined(HAVE_RT2880) || defined(HAVE_RT61))
static inline int has_acktiming(const char *prefix)
{
	return 0;
}
#endif
#endif

#ifdef HAVE_WPA3
static inline int has_wpa3(const char *prefix)
{
	return !is_brcmfmac(prefix);
}
#else
static inline int has_wpa3(const char *prefix)
{
	return 0;
}
#endif

char *getCountryList(char *filter);

#ifdef HAVE_MADWIFI
void invalidate_channelcache(void);
extern struct wifi_channels *list_channels(char *devnr);
int get_radiostate(char *ifname);

int isAssociated(char *ifname);

unsigned int getRegDomain(char *country);
unsigned int getCountry(char *country);
const char *getCountryByIso(char *iso);

#endif
int ieee80211_mhz2ieee(int freq);
#if defined(HAVE_RT2880) || defined(HAVE_RT61) || defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
int wifi_getchannel(char *ifname);
struct wifi_interface *wifi_getfreq(char *ifname);
int get_radiostate(char *ifname);
int get_freqoffset(char *ifname);
int get_wififreq(char *ifname, int freq);

#endif
/*
 * Set/Get named variable.
 * @param       name    interface name
 * @param       var     variable name
 * @param       val     variable value/buffer
 * @param       len     variable value/buffer length
 * @return      success == 0, failure != 0
 */
extern int wl_set_val(char *name, char *var, void *val, int len);
extern int wl_get_val(char *name, char *var, void *val, int len);
extern int wl_set_int(char *name, char *var, int val);
extern int wl_get_int(char *name, char *var, int *val);

/*
 * Get device type.
 * @param       name    interface name
 * @param       buf     buffer for passing in and/or receiving data
 * @param       len     length of buf
 * @return      >= 0 if successful or < 0 otherwise
 */
#define DEV_TYPE_LEN 3 /* Length for dev type 'et'/'wl' */
extern int wl_get_dev_type(char *name, void *buf, int len);

/*
 * Set/Get named variable.
 * @param       ifname          interface name
 * @param       iovar           variable name
 * @param       param           input param value/buffer
 * @param       paramlen        input param value/buffer length
 * @param       bufptr          io buffer
 * @param       buflen          io buffer length
 * @param       val             val or val pointer for int routines
 * @return      success == 0, failure != 0
 */
extern int wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen);
extern int wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen);
extern int wl_iovar_setint(char *ifname, char *iovar, int val);

extern int wl_iovar_getint(char *ifname, char *iovar, int *val);

extern int wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len);
extern int wl_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen);
extern int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);

int wl_getbssid(char *wl, char *mac);

/* tag_ID/length/value_buffer tuple */
#ifndef BCMWPA2
typedef struct bcm_tlv {
	uint8_t id;
	uint8_t len;
	uint8_t data[1];
} bcm_tlv_t;
#endif

#endif /* _wlutils_h_ */
