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

#include <typedefs.h>
#ifndef _NO_WLIOCTL_H
#include <wlioctl.h>
#endif
#ifndef WLC_IOCTL_SMLEN
#define	WLC_IOCTL_SMLEN		256	/* "small" length ioctl buffer
					 * required */
#endif
#ifndef BCME_BUFTOOSHORT
#define BCME_BUFTOOSHORT		-14	/* Buffer too short */
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

int getassoclist(char *name, unsigned char *list);

#define INFO_UPTIME 0
#define INFO_RSSI 1
#define INFO_NOISE 2
#define INFO_RXRATE 3
#define INFO_TXRATE 4

int getWifiInfo_ath9k(char *ifname, unsigned char *mac, int field);	// only used internal
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

extern int has_mimo(char *prefix);
extern int has_ac(char *prefix);
#ifdef HAVE_WIL6210
extern int has_ad(char *prefix);
#else
#define has_ad(prefix) (0)
#endif
extern int has_gcmp(char *prefix);
extern int has_qtn(char *prefix);
extern int has_athmask(int devnum, int mask);
extern int has_2ghz(char *prefix);
extern int is_wrt3200(void);
extern int has_5ghz(char *prefix);
extern int has_ht40(char *prefix);
#ifdef HAVE_80211AC
extern int has_beamforming(char *prefix);
#endif

#define SITE_SURVEY_DB  "/tmp/site_survey"
#define SITE_SURVEY_NUM 256
#define SCAN_HT20 1
#define SCAN_HT40 2
#define SCAN_VHT80 4

struct site_survey_list {
	char SSID[33];
	char BSSID[18];
	uint16 channel;		/* Channel no. */
	uint16 frequency;	/* Frequency i.e. for superchannel */
	int16 RSSI;		/* receive signal strength (in dBm) */
	int16 phy_noise;	/* noise (in dBm) */
	uint16 beacon_period;	/* units are Kusec */
	uint16 capability;	/* Capability information */
	char ENCINFO[128];	/* encryption info */
	uint rate_count;	/* # rates in this set */
	uint8 dtim_period;	/* DTIM period */
};

struct wifi_interface {
	int freq;
	int width;
	int center1;
	int center2;
};

typedef struct STAINFO {
	char mac[6];
	char rssi;
	char noise;
	char ifname[32];
} STAINFO;

extern STAINFO *getRaStaInfo(char *ifname);

#if defined(HAVE_MADWIFI) || defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
#include <stdint.h>

extern struct wifi_channels *list_channels_11n(char *devnr);
extern struct wifi_channels *list_channels_ath9k(char *devnr, char *country, int max_bandwidth_khz, unsigned char band);
extern int getdevicecount(void);

extern int mac80211_get_coverageclass(char *interface);
extern struct mac80211_info *mac80211_assoclist(char *interface);
extern char *mac80211_get_caps(char *interface, int shortgi, int greenfield);
extern int has_shortgi(char *interface);
extern int has_greenfield(char *interface);

#ifdef HAVE_ATH10K
extern int has_vht160(char *interface);
extern int has_vht80(char *interface);
extern int has_vht80plus80(char *interface);
int has_subeamforming(char *interface);
int has_mubeamforming(char *interface);
extern char *mac80211_get_vhtcaps(char *interface, int shortgi, int vht80, int vht160, int vht8080, int subf, int mubf);
extern unsigned int get_ath10kreg(char *ifname, unsigned int reg);
extern void set_ath10kreg(char *ifname, unsigned int reg, unsigned int value);
extern void set_ath10kdistance(char *ifname, unsigned int distance);
extern unsigned int get_ath10kack(char *ifname);
extern unsigned int get_ath10kdistance(char *ifname);
#endif
struct unl;
extern int has_airtime_fairness(char *prefix);
extern int mac80211_check_band(char *interface, int checkband);
struct wifi_channels *mac80211_get_channels(struct unl *unl, char *interface, const char *country, int max_bandwidth_khz, unsigned char checkband);
struct wifi_channels *mac80211_get_channels_simple(char *interface, const char *country, int max_bandwidth_khz, unsigned char checkband);
#define AUTO_FORCEHT40 1
#define AUTO_FORCEVHT80 2
#define AUTO_FORCEVHT160 4
#define AUTO_ALL 0

extern struct mac80211_ac *mac80211autochannel(char *interface, char *freq_range, int scans, int ammount, int enable_passive, int htflags);
extern void mac80211_set_antennas(int phy, uint32_t tx_ant, uint32_t rx_ant);
extern int mac80211_get_avail_tx_antenna(int phy);
extern int mac80211_get_avail_rx_antenna(int phy);

extern struct wifi_interface *mac80211_get_interface(char *dev);
extern int mac80211_get_configured_tx_antenna(int phy);
extern int mac80211_get_configured_rx_antenna(int phy);
extern int mac80211_check_valid_frequency(char *interface, char *country, int freq);
extern int getFrequency_mac80211(char *interface);
extern int mac80211_get_phyidx_by_vifname(char *vif);

struct wifi_channels {
	int channel;
	int freq;
	int noise;
	int max_eirp;
	int hw_eirp;
	unsigned int no_outdoor:1, no_indoor:1, no_ofdm:1, no_cck:1, ptp_only:1, ptmp_only:1, passive_scan:1, no_ibss:1, lll:1, llu:1, lul:1, luu:1, ull:1, ulu:1, uul:1, uuu:1, ht40:1, vht80:1, vht160:1, dfs:1;
};

struct mac80211_info {
	struct wifi_client_info *wci;
	int8_t noise;
	uint64_t channel_active_time;
	uint64_t channel_busy_time;
	uint64_t channel_receive_time;
	uint64_t channel_transmit_time;
	uint64_t extension_channel_busy_time;
	uint32_t frequency;
};

struct wifi_client_info {
	char ifname[20];
	char is_wds;
	char mac[18];
	uint32_t uptime;
	uint16_t txrate;
	uint32_t rxrate;
	int8_t signal;
	int8_t signal_avg;
	uint32_t noise;
	uint32_t snr;
	int8_t mcs;
	int8_t rx_mcs;
	unsigned int is_40mhz:1, is_80mhz:1, is_160mhz:1, is_80p80mhz:1, is_ht:1, is_vht:1, is_short_gi:1, rx_is_40mhz:1, rx_is_80mhz:1, rx_is_160mhz:1, rx_is_80p80mhz:1, rx_is_ht:1, rx_is_vht:1, rx_is_short_gi:1,
	    ht40intol:1, islzo:1;
	uint32_t inactive_time;
	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_compressed;
	uint32_t tx_compressed;
	uint32_t rx_bytes;
	uint32_t tx_bytes;
	struct wifi_client_info *next;
};

struct mac80211_ac {
	int freq;
	int8_t noise;
	int quality;
	int clear;
	struct mac80211_ac *next;
};

extern void free_wifi_clients(struct wifi_client_info *wci);
extern void free_mac80211_ac(struct mac80211_ac *ac);
#endif

char *getCountryList(void);

#ifdef HAVE_MADWIFI
extern struct wifi_channels *list_channels(char *devnr);
int get_radiostate(char *ifname);

int isAssociated(char *ifname);

unsigned int getRegDomain(char *country);
unsigned int getCountry(char *country);
const char *getCountryByIso(char *iso);

#endif
int ieee80211_mhz2ieee(int freq);
#if defined(HAVE_RT2880) || defined(HAVE_RT61) || defined(HAVE_MADWIFI)
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
#define DEV_TYPE_LEN 3		/* Length for dev type 'et'/'wl' */
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

#endif				/* _wlutils_h_ */
