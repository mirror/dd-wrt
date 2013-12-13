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


float wifi_getrate(char *ifname);
int wifi_gettxpower(char *ifname);
int wifi_gettxpoweroffset(char *ifname);
int bcm_gettxpower(char *wlname);

extern double HTTxRate20_800(unsigned int index);
extern double HTTxRate20_400(unsigned int index);
extern double HTTxRate40_800(unsigned int index);
extern double HTTxRate40_400(unsigned int index); 

/*
 * Pass a wlioctl request to the specified interface.
 * @param       name    interface name
 * @param       cmd     WLC_GET_MAGIC <= cmd < WLC_LAST
 * @param       buf     buffer for passing in and/or receiving data
 * @param       len     length of buf
 * @return      >= 0 if successful or < 0 otherwise
 */
extern int wl_ioctl(char *name, int cmd, void *buf, int len);

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

/*
 * Returns the list of associated stations in the pre-existing buffer list 
 */
int getchannels(unsigned int *list, char *ifname);
int getwdslist(char *name, unsigned char *list);

int getassoclist(char *name, unsigned char *list);
int getNoise(char *ifname, unsigned char *mac);
int getUptime(char *ifname, unsigned char *mac);
int getRssi(char *ifname, unsigned char *mac);

int getassoclist_11n(char *name, unsigned char *list);
int getNoise_11n(char *ifname, unsigned char *mac);
int getUptime_11n(char *ifname, unsigned char *mac);
int getRssi_11n(char *ifname, unsigned char *mac);

extern int getassoclist_ath9k(char *name, unsigned char *list);
extern int getNoise_ath9k(char *ifname, unsigned char *mac);
extern int getUptime_ath9k(char *ifname, unsigned char *mac);
extern int getRssi_ath9k(char *ifname, unsigned char *mac);

extern int has_2ghz(char *prefix);
extern int has_5ghz(char *prefix);
extern int has_ht40(char *prefix);
#ifdef HAVE_80211AC
extern has_beamforming(char *prefix);
#endif

#define SITE_SURVEY_DB  "/tmp/site_survey"
#define SITE_SURVEY_NUM 256

struct site_survey_list {
	char SSID[33];
	unsigned char BSSID[18];
	uint16 channel;		/* Channel no. */
	uint16 frequency;		/* Frequency i.e. for superchannel */
	int16 RSSI;		/* receive signal strength (in dBm) */
	int16 phy_noise;	/* noise (in dBm) */
	uint16 beacon_period;	/* units are Kusec */
	uint16 capability;	/* Capability information */
	unsigned char ENCINFO[128];	/* encryption info */
	uint rate_count;	/* # rates in this set */
	uint8 dtim_period;	/* DTIM period */
};


#if defined(HAVE_MADWIFI) || defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
#include <stdint.h>

extern struct wifi_channels *list_channels_11n(char *devnr);
extern struct wifi_channels *list_channels_ath9k(char *devnr, char *country,int max_bandwidth_khz, unsigned char band);
extern int getdevicecount(void);
extern int mac80211_get_coverageclass(char *interface);
extern struct mac80211_info *mac80211_assoclist(char *interface); 
extern char *mac80211_get_caps(char *interface); 
extern int mac80211_check_band(char *interface,int checkband);
struct wifi_channels *mac80211_get_channels(char *interface,char *country,int max_bandwidth_khz, unsigned char checkband);
extern struct mac80211_ac *mac80211autochannel(char *interface, char *freq_range, int scans, int ammount, int enable_passive);
extern void mac80211_set_antennas(int phy,uint32_t tx_ant,uint32_t rx_ant );
extern int mac80211_get_avail_tx_antenna(int phy);
extern int mac80211_get_avail_rx_antenna(int phy);
extern int mac80211_get_configured_tx_antenna(int phy);
extern int mac80211_get_configured_rx_antenna(int phy);
extern int mac80211_check_valid_frequency(char *interface, char *country, int freq);
extern int getFrequency_mac80211(char *interface);

struct wifi_channels {
	int channel;
	int freq;
	int noise;
	unsigned char ht40minus; 
	unsigned char ht40plus; 
	unsigned char dfs; 
	int max_eirp;
	unsigned char no_outdoor; 
	unsigned char no_indoor; 
	unsigned char no_ofdm;
	unsigned char no_cck;
	unsigned char ptp_only;
	unsigned char ptmp_only;
	unsigned char passive_scan;
	unsigned char no_ibss;
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
    uint32_t noise;
    uint32_t snr;
    int8_t mcs;
    int8_t rx_mcs;
    char is_40mhz;
    char is_short_gi;
    char rx_is_40mhz;
    char rx_is_short_gi;
    uint32_t inactive_time;
    uint32_t rx_packets;
    uint32_t tx_packets;
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

#ifdef HAVE_MADWIFI
extern struct wifi_channels *list_channels(char *devnr);
int get_radiostate(char *ifname);

int isAssociated(char *ifname);

unsigned int getRegDomain(const char *country);
unsigned int getCountry(const char *country);
char *getCountryList(void);
u_int ieee80211_mhz2ieee(u_int freq);

#endif
#if defined(HAVE_RT2880) || defined(HAVE_RT61)
int wifi_getchannel(char *ifname);
int wifi_getfreq(char *ifname);
u_int ieee80211_mhz2ieee(u_int freq);
int get_radiostate(char *ifname);

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
extern int wl_iovar_setbuf(char *ifname, char *iovar, void *param,
			   int paramlen, void *bufptr, int buflen);
extern int wl_iovar_getbuf(char *ifname, char *iovar, void *param,
			   int paramlen, void *bufptr, int buflen);
extern int wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen);
extern int wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen);
extern int wl_iovar_setint(char *ifname, char *iovar, int val);

extern int wl_iovar_getint(char *ifname, char *iovar, int *val);

extern int wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx,
			      void *param, int paramlen, void *bufptr,
			      int buflen);
extern int wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx,
			      void *param, int paramlen, void *bufptr,
			      int buflen);
extern int wl_bssiovar_get(char *ifname, char *iovar, int bssidx,
			   void *outbuf, int len);
extern int wl_bssiovar_set(char *ifname, char *iovar, int bssidx,
			   void *param, int paramlen);
extern int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);

int wl_getbssid(char *wl, char *mac);

#endif				/* _wlutils_h_ */
