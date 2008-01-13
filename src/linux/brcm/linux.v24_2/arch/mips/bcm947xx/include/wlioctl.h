/*
 * Custom OID/ioctl definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#ifndef _wlioctl_h_
#define	_wlioctl_h_

#include <typedefs.h>
#include <proto/ethernet.h>
#include <proto/bcmeth.h>
#include <proto/bcmevent.h>
#include <proto/802.11.h>
#include <bcmwifi.h>

#ifdef __NetBSD__
/* NetBSD 2.0 does not have SIOCDEVPRIVATE. This is NetBSD 2.0 specific */
#define SIOCDEVPRIVATE	_IOWR('i', 139, struct ifreq)
#endif

/* require default structure packing */
#if !defined(__GNUC__)
#pragma pack(push, 8)
#endif

/* Legacy structure to help keep backward compatible wl tool and tray app */

#define	LEGACY_WL_BSS_INFO_VERSION	107	/* older version of wl_bss_info struct */

typedef struct wl_bss_info_107 {
	uint32		version;		/* version field */
	uint32		length;			/* byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/* units are Kusec */
	uint16		capability;		/* Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	struct {
		uint	count;			/* # rates in this set */
		uint8	rates[16];		/* rates in 500kbps units w/hi bit set if basic */
	} rateset;				/* supported rates */
	uint8		channel;		/* Channel no. */
	uint16		atim_window;		/* units are Kusec */
	uint8		dtim_period;		/* DTIM period */
	int16		RSSI;			/* receive signal strength (in dBm) */
	int8		phy_noise;		/* noise (in dBm) */
	uint32		ie_length;		/* byte length of Information Elements */
	/* variable length Information Elements */
} wl_bss_info_107_t;

/*
 * Per-bss information structure.
 */

#define	WL_BSS_INFO_VERSION	108		/* current version of wl_bss_info struct */

/* BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info {
	uint32		version;		/* version field */
	uint32		length;			/* byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/* units are Kusec */
	uint16		capability;		/* Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	struct {
		uint	count;			/* # rates in this set */
		uint8	rates[16];		/* rates in 500kbps units w/hi bit set if basic */
	} rateset;				/* supported rates */
	chanspec_t	chanspec;		/* chanspec for bss */
	uint16		atim_window;		/* units are Kusec */
	uint8		dtim_period;		/* DTIM period */
	int16		RSSI;			/* receive signal strength (in dBm) */
	int8		phy_noise;		/* noise (in dBm) */

	bool		n_cap;			/* BSS is 802.11N Capable */
	uint32		nbss_cap;		/* 802.11N BSS Capabilities (based on EWC_CAP_*) */
	uint8		ctl_ch;			/* 802.11N BSS control channel number */
	uint32		reserved[2];		/* Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/* 802.11N BSS required MCS set */

	uint16		ie_offset;		/* offset at which IEs start, from beginning */
	uint32		ie_length;		/* byte length of Information Elements */
	/* Add new fields here */
	/* variable length Information Elements */
} wl_bss_info_t;

typedef struct wlc_ssid {
	uint32		SSID_len;
	uchar		SSID[32];
} wlc_ssid_t;

typedef struct chan_scandata {
	uint8		txpower;
	uint8		pad;
	chanspec_t	channel;	/* Channel num, bw, ctrl_sb and band */
	uint32		channel_mintime;
	uint32		channel_maxtime;
} chan_scandata_t;

typedef enum wl_scan_type {
	EXTDSCAN_BACKGROUND_SCAN,
	EXTDSCAN_FOREGROUND_SCAN,
	EXTDSCAN_FORCEDBACKGROUND_SCAN
}wl_scan_type_t;

#define WLC_EXTDSCAN_MAX_SSID		5
#define WL_EXTDSCAN_PARAMS_FIXED_SIZE 	200

typedef struct wl_extdscan_params {
	int8 		nprobes;		/* 0, passive, otherwise active */
	wl_scan_type_t	scan_type;		/* enum */
	int8    	split_scan;		/* split scan */
	int8		band;			/* band */
	wlc_ssid_t 	ssid[WLC_EXTDSCAN_MAX_SSID]; /* ssid list */
	int32 		channel_num;
	chan_scandata_t channel_list[1];	/* list of chandata structs */
} wl_extdscan_params_t;

typedef struct wl_scan_params {
	wlc_ssid_t ssid;		/* default: {0, ""} */
	struct ether_addr bssid;	/* default: bcast */
	int8 bss_type;			/* default: any,
					 * DOT11_BSSTYPE_ANY/INFRASTRUCTURE/INDEPENDENT
					 */
	int8 scan_type;			/* -1 use default, DOT11_SCANTYPE_ACTIVE/PASSIVE */
	int32 nprobes;			/* -1 use default, number of probes per channel */
	int32 active_time;		/* -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/* -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/* -1 use default, dwell time for the home channel
					 * between channel scans
					 */
	int32 channel_num;		/* 0 use default (all available channels), count of
					 * channels in channel_list
					 */
	uint16 channel_list[1];		/* list of chanspecs */
} wl_scan_params_t;
/* size of wl_scan_params not including variable length array */
#define WL_SCAN_PARAMS_FIXED_SIZE 64

#define WL_SCAN_ACTION_START      1
#define WL_SCAN_ACTION_CONTINUE   2

#define ISCAN_REQ_VERSION 1

/* incremental scan struct */
typedef struct wl_iscan_params {
	uint32 version;
	uint16 action;
	uint16 scan_duration;
	wl_scan_params_t params;
} wl_iscan_params_t;
/* 3 fields + size of wl_scan_params, not including variable length array */
#define WL_ISCAN_PARAMS_FIXED_SIZE (OFFSETOF(wl_iscan_params_t, params) + sizeof(wlc_ssid_t))

typedef struct wl_scan_results {
	uint32 buflen;
	uint32 version;
	uint32 count;
	wl_bss_info_t bss_info[1];
} wl_scan_results_t;
/* size of wl_scan_results not including variable length array */
#define WL_SCAN_RESULTS_FIXED_SIZE 12

/* wl_iscan_results status values */
#define WL_SCAN_RESULTS_SUCCESS	0
#define WL_SCAN_RESULTS_PARTIAL	1
#define WL_SCAN_RESULTS_PENDING	2
#define WL_SCAN_RESULTS_ABORTED	3

/* incremental scan results struct */
typedef struct wl_iscan_results {
	uint32 status;
	wl_scan_results_t results;
} wl_iscan_results_t;
/* size of wl_iscan_results not including variable length array */
#define WL_ISCAN_RESULTS_FIXED_SIZE \
	(WL_SCAN_RESULTS_FIXED_SIZE + OFFSETOF(wl_iscan_results_t, results))

#define WL_NUMRATES		255	/* max # of rates in a rateset */
typedef struct wl_rateset {
	uint32	count;			/* # rates in this set */
	uint8	rates[WL_NUMRATES];	/* rates in 500kbps units w/hi bit set if basic */
} wl_rateset_t;

/* uint32 list */
typedef struct wl_uint32_list {
	/* in - # of elements, out - # of entries */
	uint32 count;
	/* variable length uint32 list */
	uint32 element[1];
} wl_uint32_list_t;

#define WLC_CNTRY_BUF_SZ	4		/* Country string is 3 bytes + NULL */

/* defines used by the nrate iovar */
#define NRATE_MCS_INUSE	0x00000080	/* MSC in use,indicates b0-6 holds an mcs */
#define NRATE_RATE_MASK 0x0000007f	/* rate/mcs value */
#define NRATE_STF_MASK	0x0000ff00	/* stf mode mask: siso, cdd, stbc, sdm */
#define NRATE_STF_SHIFT	8		/* stf mode shift */
#define NRATE_OVERRIDE	0x80000000	/* bit indicate override both rate & mode */

#define NRATE_STF_SISO	0		/* stf mode SISO */
#define NRATE_STF_CDD	1		/* stf mode CDD */
#define NRATE_STF_STBC	2		/* stf mode STBC */
#define NRATE_STF_SDM	3		/* stf mode SDM */

#define ANTENNA_NUM_1	1		/* total number of antennas to be used */
#define ANTENNA_NUM_2	2
#define ANTENNA_NUM_3	3
#define ANTENNA_NUM_4	4

#define WL_ANTCFG_AUTO		0x80	/* bit indicate antenna sel AUTO */
#define ANTSEL_CONFIG_MASK	0x33	/* antenna configuration mask */
#define MAX_NUM_ANT_CFG		4	/* max number of antenna configuration */
#define UNICAST_TXANT_CFG	0	/* unicast tx antenna configuration */
#define UNICAST_RXANT_CFG	1	/* unicast rx antenna configuration */
#define DEFAULT_TXANT_CFG	2	/* default tx antenna configuration */
#define DEFAULT_RXANT_CFG	3	/* default rx antenna configuration */

typedef struct {
	uint8 ant_config[MAX_NUM_ANT_CFG];	/* antenna configuration */
	uint8 num_antcfg;	/* number of available antenna configurations */
} wlc_antselcfg_t;

#define HIGHEST_SINGLE_STREAM_MCS	7 /* MCS values greater than this enable multiple streams */

typedef struct wl_channels_in_country {
	uint32 buflen;
	uint32 band;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	uint32 count;
	uint32 channel[1];
} wl_channels_in_country_t;

typedef struct wl_country_list {
	uint32 buflen;
	uint32 band_set;
	uint32 band;
	uint32 count;
	char country_abbrev[1];
} wl_country_list_t;

#define WL_NUM_RPI_BINS		8
#define WL_RM_TYPE_BASIC	1
#define WL_RM_TYPE_CCA		2
#define WL_RM_TYPE_RPI		3

#define WL_RM_FLAG_PARALLEL	(1<<0)

#define WL_RM_FLAG_LATE		(1<<1)
#define WL_RM_FLAG_INCAPABLE	(1<<2)
#define WL_RM_FLAG_REFUSED	(1<<3)

typedef struct wl_rm_req_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/* token for this measurement */
	uint32	tsf_h;		/* TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/* TSF low 32-bits */
	uint32	dur;		/* TUs */
} wl_rm_req_elt_t;

typedef struct wl_rm_req {
	uint32	token;		/* overall measurement set token */
	uint32	count;		/* number of measurement requests */
	void * cb;		/* completion callback function: may be NULL */
	void *	cb_arg;	/* arg to completion callback function */
	wl_rm_req_elt_t	req[1];	/* variable length block of requests */
} wl_rm_req_t;
#define WL_RM_REQ_FIXED_LEN	OFFSETOF(wl_rm_req_t, req)

typedef struct wl_rm_rep_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/* token for this measurement */
	uint32	tsf_h;		/* TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/* TSF low 32-bits */
	uint32	dur;		/* TUs */
	uint32	len;		/* byte length of data block */
	uint8	data[1];	/* variable length data block */
} wl_rm_rep_elt_t;
#define WL_RM_REP_ELT_FIXED_LEN	24	/* length excluding data block */

#define WL_RPI_REP_BIN_NUM 8
typedef struct wl_rm_rpi_rep {
	uint8	rpi[WL_RPI_REP_BIN_NUM];
	int8	rpi_max[WL_RPI_REP_BIN_NUM];
} wl_rm_rpi_rep_t;

typedef struct wl_rm_rep {
	uint32	token;		/* overall measurement set token */
	uint32	len;		/* length of measurement report block */
	wl_rm_rep_elt_t	rep[1];	/* variable length block of reports */
} wl_rm_rep_t;
#define WL_RM_REP_FIXED_LEN	8


#if defined(BCMSUP_PSK)
typedef enum sup_auth_status {
	WLC_SUP_DISCONNECTED = 0,
	WLC_SUP_CONNECTING,
	WLC_SUP_IDREQUIRED,
	WLC_SUP_AUTHENTICATING,
	WLC_SUP_AUTHENTICATED,
	WLC_SUP_KEYXCHANGE,
	WLC_SUP_KEYED,
	WLC_SUP_TIMEOUT
} sup_auth_status_t;
#endif	

/* Enumerate crypto algorithms */
#define	CRYPTO_ALGO_OFF			0
#define	CRYPTO_ALGO_WEP1		1
#define	CRYPTO_ALGO_TKIP		2
#define	CRYPTO_ALGO_WEP128		3
#define CRYPTO_ALGO_AES_CCM		4
#define CRYPTO_ALGO_AES_OCB_MSDU	5
#define CRYPTO_ALGO_AES_OCB_MPDU	6
#define CRYPTO_ALGO_NALG		7

#define WSEC_GEN_MIC_ERROR	0x0001
#define WSEC_GEN_REPLAY		0x0002

#define WL_SOFT_KEY	(1 << 0)	/* Indicates this key is using soft encrypt */
#define WL_PRIMARY_KEY	(1 << 1)	/* Indicates this key is the primary (ie tx) key */
#define WL_KF_RES_4	(1 << 4)	/* Reserved for backward compat */
#define WL_KF_RES_5	(1 << 5)	/* Reserved for backward compat */
#define WL_IBSS_PEER_GROUP_KEY	(1 << 6)	/* Indicates a group key for a IBSS PEER */

typedef struct wl_wsec_key {
	uint32		index;		/* key index */
	uint32		len;		/* key length */
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	uint32		pad_1[18];
	uint32		algo;		/* CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint32		flags;		/* misc flags */
	uint32		pad_2[2];
	int		pad_3;
	int		iv_initialized;	/* has IV been initialized already? */
	int		pad_4;
	/* Rx IV */
	struct {
		uint32	hi;		/* upper 32 bits of IV */
		uint16	lo;		/* lower 16 bits of IV */
	} rxiv;
	uint32		pad_5[2];
	struct ether_addr ea;		/* per station */
} wl_wsec_key_t;

#define WSEC_MIN_PSK_LEN	8
#define WSEC_MAX_PSK_LEN	64

/* Flag for key material needing passhash'ing */
#define WSEC_PASSPHRASE		(1<<0)

/* receptacle for WLC_SET_WSEC_PMK parameter */
typedef struct {
	ushort	key_len;		/* octets in key material */
	ushort	flags;			/* key handling qualification */
	uint8	key[WSEC_MAX_PSK_LEN];	/* PMK material */
} wsec_pmk_t;

/* wireless security bitvec */
#define WEP_ENABLED		0x0001
#define TKIP_ENABLED		0x0002
#define AES_ENABLED		0x0004
#define WSEC_SWFLAG		0x0008
#define SES_OW_ENABLED		0x0040	/* to go into transition mode without setting wep */
#define FIPS_ENABLED	0x0080

/* WPA authentication mode bitvec */
#define WPA_AUTH_DISABLED	0x0000	/* Legacy (i.e., non-WPA) */
#define WPA_AUTH_NONE		0x0001	/* none (IBSS) */
#define WPA_AUTH_UNSPECIFIED	0x0002	/* over 802.1x */
#define WPA_AUTH_PSK		0x0004	/* Pre-shared key */
/* #define WPA_AUTH_8021X 0x0020 */	/* 802.1x, reserved */
#ifdef BCMWPA2
#define WPA2_AUTH_UNSPECIFIED	0x0040	/* over 802.1x */
#define WPA2_AUTH_PSK		0x0080	/* Pre-shared key */
#endif	/* BCMWPA2 */

#ifdef BCMWPA2
/* pmkid */
#define	MAXPMKID		16	

typedef struct _pmkid
{
	struct ether_addr	BSSID;
	uint8			PMKID[WPA2_PMKID_LEN];
} pmkid_t;

typedef struct _pmkid_list
{
	uint32	npmkid;
	pmkid_t	pmkid[1];
} pmkid_list_t;

typedef struct _pmkid_cand {
	struct ether_addr	BSSID;
	uint8			preauth;
} pmkid_cand_t;

typedef struct _pmkid_cand_list {
	uint32	npmkid_cand;
	pmkid_cand_t	pmkid_cand[1];
} pmkid_cand_list_t;
#endif	/* BCMWPA2 */

typedef struct wl_led_info {
	uint32		index;		/* led index */
	uint32		behavior;
	bool		activehi;
} wl_led_info_t;

typedef struct wlc_assoc_info {
	uint32		req_len;
	uint32		resp_len;
	uint32		flags;
	struct dot11_assoc_req req;
	struct ether_addr reassoc_bssid; /* used in reassoc's */
	struct dot11_assoc_resp resp;
} wl_assoc_info_t;
/* flags */
#define WLC_ASSOC_REQ_IS_REASSOC 0x01 /* assoc req was actually a reassoc */
/* srom read/write struct passed through ioctl */
typedef struct {
	uint	byteoff;		/* byte offset */
	uint	nbytes;		/* number of bytes */
	uint16 buf[1];
} srom_rw_t;

/* R_REG and W_REG struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/* byte offset of the field in d11regs_t */
	uint32	val;		/* read/write value of the field */
	uint32	size;		/* sizeof the field */
	uint	band;		/* band (optional) */
} rw_reg_t;

/* Structure used by GET/SET_ATTEN ioctls - it controls power in b/g-band */
/* PCL - Power Control Loop */
/* current gain setting is replaced by user input */
#define WL_ATTEN_APP_INPUT_PCL_OFF	0	/* turn off PCL, apply supplied input */
#define WL_ATTEN_PCL_ON			1	/* turn on PCL */
/* current gain setting is maintained */
#define WL_ATTEN_PCL_OFF		2	/* turn off PCL. */
typedef struct {
	uint16	auto_ctrl;	/* WL_ATTEN_XX */
	uint16	bb;		/* Baseband attenuation */
	uint16	radio;		/* Radio attenuation */
	uint16	txctl1;		/* Radio TX_CTL1 value */
} atten_t;

/* Per-AC retry parameters */
struct wme_tx_params_s {
	uint8  short_retry;
	uint8  short_fallback;
	uint8  long_retry;
	uint8  long_fallback;
	uint16 max_rate;  /* In units of 512 Kbps */
};
typedef struct wme_tx_params_s wme_tx_params_t;
#define WL_WME_TX_PARAMS_IO_BYTES (sizeof(wme_tx_params_t) * AC_COUNT)

/* defines used by poweridx iovar - it controls power in a-band */
/* current gain setting is maintained */
#define WL_PWRIDX_PCL_OFF	-2	/* turn off PCL.  */
#define WL_PWRIDX_PCL_ON	-1	/* turn on PCL */
#define WL_PWRIDX_LOWER_LIMIT	-2	/* lower limit */
#define WL_PWRIDX_UPPER_LIMIT	63	/* upper limit */
/* value >= 0 causes
 *	- input to be set to that value
 *	- PCL to be off
 */

/* Used to get specific STA parameters */
typedef struct {
	uint32	val;
	struct ether_addr ea;
} scb_val_t;

#define BCM_MAC_STATUS_INDICATION	(0x40010200L)

/* Please update the following when modifying this structure:
 *   StaInfo Twiki page flags section - description of the sta_info_t struct
 *    src/wl/exe/wlu.c - print of sta_info_t
 * Pay attention to version if structure changes.
 */
typedef struct {
	uint16			ver;		/* version of this struct */
	uint16			len;		/* length in bytes of this structure */
	uint16			cap;		/* sta's advertised capabilities */
	uint32			flags;		/* flags defined below */
	uint32			idle;		/* time since data pkt rx'd from sta */
	struct ether_addr	ea;		/* Station address */
	wl_rateset_t		rateset;	/* rateset in use */
	uint32			in;		/* seconds elapsed since associated */
	uint32			listen_interval_inms; /* Min Listen interval in ms for this STA */
} sta_info_t;

#define WL_STA_VER		2

/* Flags for sta_info_t indicating properties of STA */
#define WL_STA_BRCM		0x1		/* Running a Broadcom driver */
#define WL_STA_WME		0x2		/* WMM association */
#define WL_STA_ABCAP		0x4		/* Afterburner-capable */
#define WL_STA_AUTHE		0x8		/* Authenticated */
#define WL_STA_ASSOC		0x10		/* Associated */
#define WL_STA_AUTHO		0x20		/* Authorized */
#define WL_STA_WDS		0x40		/* Wireless Distribution System */
#define WL_STA_WDS_LINKUP	0x80		/* WDS traffic/probes flowing properly */
#define WL_STA_PS		0x100		/* STA is in power save mode from AP's viewpoint */
#define WL_STA_APSD_BE		0x200		/* APSD delv/trigger for AC_BE is default enabled */
#define WL_STA_APSD_BK		0x400		/* APSD delv/trigger for AC_BK is default enabled */
#define WL_STA_APSD_VI		0x800		/* APSD delv/trigger for AC_VI is default enabled */
#define WL_STA_APSD_VO		0x1000		/* APSD delv/trigger for AC_VO is default enabled */
#define WL_STA_N_CAP		0x2000		/* STA 802.11n capable */

#define WL_WDS_LINKUP		WL_STA_WDS_LINKUP	/* deprecated */

/* channel encoding */
typedef struct channel_info {
	int hw_channel;
	int target_channel;
	int scan_channel;
} channel_info_t;

/* For ioctls that take a list of MAC addresses */
struct maclist {
	uint count;			/* number of MAC addresses */
	struct ether_addr ea[1];	/* variable length array of MAC addresses */
};

/* get pkt count struct passed through ioctl */
typedef struct get_pktcnt {
	uint rx_good_pkt;
	uint rx_bad_pkt;
	uint tx_good_pkt;
	uint tx_bad_pkt;
	uint rx_ocast_good_pkt; /* unicast packets destined for others */
} get_pktcnt_t;

/* Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	uint cmd;	/* common ioctl definition */
	void *buf;	/* pointer to user buffer */
	uint len;	/* length of user buffer */
	bool set;	/* get or set request (optional) */
	uint used;	/* bytes read or written (optional) */
	uint needed;	/* bytes needed (optional) */
} wl_ioctl_t;

/*
 * Structure for passing hardware and software
 * revision info up from the driver.
 */
typedef struct wlc_rev_info {
	uint		vendorid;	/* PCI vendor id */
	uint		deviceid;	/* device id of chip */
	uint		radiorev;	/* radio revision */
	uint		chiprev;	/* chip revision */
	uint		corerev;	/* core revision */
	uint		boardid;	/* board identifier (usu. PCI sub-device id) */
	uint		boardvendor;	/* board vendor (usu. PCI sub-vendor id) */
	uint		boardrev;	/* board revision */
	uint		driverrev;	/* driver version */
	uint		ucoderev;	/* microcode version */
	uint		bus;		/* bus type */
	uint		chipnum;	/* chip number */
} wlc_rev_info_t;

#define WL_BRAND_MAX 10
typedef struct wl_instance_info {
	uint instance;
	char brand[WL_BRAND_MAX];
} wl_instance_info_t;

/* structure to change size of tx fifo */
typedef struct wl_txfifo_sz {
	uint8	fifo;
	uint8	size;
} wl_txfifo_sz_t;

/* Transfer info about an IOVar from the driver */
/* Max supported IOV name size in bytes, + 1 for null termination */
#define WLC_IOV_NAME_LEN 30
typedef struct wlc_iov_trx_s {
	uint8 module;
	uint8 type;
	char name[WLC_IOV_NAME_LEN];
} wlc_iov_trx_t;

/* check this magic number */
#define WLC_IOCTL_MAGIC		0x14e46c77

/* bump this number if you change the ioctl interface */
#define WLC_IOCTL_VERSION	1

#define	WLC_IOCTL_MAXLEN	8192		/* max length ioctl buffer required */
#define	WLC_IOCTL_SMLEN		256		/* "small" length ioctl buffer required */

/* common ioctl definitions */
#define WLC_GET_MAGIC				0
#define WLC_GET_VERSION				1
#define WLC_UP					2
#define WLC_DOWN				3
#define WLC_GET_LOOP				4
#define WLC_SET_LOOP				5
#define WLC_DUMP				6
#define WLC_GET_MSGLEVEL			7
#define WLC_SET_MSGLEVEL			8
#define WLC_GET_PROMISC				9
#define WLC_SET_PROMISC				10
/* #define WLC_DUMP_RSSI			11 */ /* no longer supported */
#define WLC_GET_RATE				12
/* #define WLC_SET_RATE				13 */ /* no longer supported */
#define WLC_GET_INSTANCE			14
/* #define WLC_GET_FRAG				15 */ /* no longer supported */
/* #define WLC_SET_FRAG				16 */ /* no longer supported */
/* #define WLC_GET_RTS				17 */ /* no longer supported */
/* #define WLC_SET_RTS				18 */ /* no longer supported */
#define WLC_GET_INFRA				19
#define WLC_SET_INFRA				20
#define WLC_GET_AUTH				21
#define WLC_SET_AUTH				22
#define WLC_GET_BSSID				23
#define WLC_SET_BSSID				24
#define WLC_GET_SSID				25
#define WLC_SET_SSID				26
#define WLC_RESTART				27
/* #define WLC_DUMP_SCB				28 */ /* no longer supported */
#define WLC_GET_CHANNEL				29
#define WLC_SET_CHANNEL				30
#define WLC_GET_SRL				31
#define WLC_SET_SRL				32
#define WLC_GET_LRL				33
#define WLC_SET_LRL				34
#define WLC_GET_PLCPHDR				35
#define WLC_SET_PLCPHDR				36
#define WLC_GET_RADIO				37
#define WLC_SET_RADIO				38
#define WLC_GET_PHYTYPE				39
#define WLC_DUMP_RATE				40
#define WLC_SET_RATE_PARAMS			41
/* #define WLC_GET_WEP				42 */ /* no longer supported */
/* #define WLC_SET_WEP				43 */ /* no longer supported */
#define WLC_GET_KEY				44
#define WLC_SET_KEY				45
#define WLC_GET_REGULATORY			46
#define WLC_SET_REGULATORY			47
#define WLC_GET_PASSIVE_SCAN			48
#define WLC_SET_PASSIVE_SCAN			49
#define WLC_SCAN				50
#define WLC_SCAN_RESULTS			51
#define WLC_DISASSOC				52
#define WLC_REASSOC				53
#define WLC_GET_ROAM_TRIGGER			54
#define WLC_SET_ROAM_TRIGGER			55
#define WLC_GET_ROAM_DELTA			56
#define WLC_SET_ROAM_DELTA			57
#define WLC_GET_ROAM_SCAN_PERIOD		58
#define WLC_SET_ROAM_SCAN_PERIOD		59
#define WLC_EVM					60	/* diag */
#define WLC_GET_TXANT				61
#define WLC_SET_TXANT				62
#define WLC_GET_ANTDIV				63
#define WLC_SET_ANTDIV				64
/* #define WLC_GET_TXPWR			65 */ /* no longer supported */
/* #define WLC_SET_TXPWR			66 */ /* no longer supported */
#define WLC_GET_CLOSED				67
#define WLC_SET_CLOSED				68
#define WLC_GET_MACLIST				69
#define WLC_SET_MACLIST				70
#define WLC_GET_RATESET				71
#define WLC_SET_RATESET				72
/* #define WLC_GET_LOCALE			73 */ /* no longer supported */
#define WLC_LONGTRAIN				74
#define WLC_GET_BCNPRD				75
#define WLC_SET_BCNPRD				76
#define WLC_GET_DTIMPRD				77
#define WLC_SET_DTIMPRD				78
#define WLC_GET_SROM				79
#define WLC_SET_SROM				80
#define WLC_GET_WEP_RESTRICT			81
#define WLC_SET_WEP_RESTRICT			82
#define WLC_GET_COUNTRY				83
#define WLC_SET_COUNTRY				84
#define WLC_GET_PM				85
#define WLC_SET_PM				86
#define WLC_GET_WAKE				87
#define WLC_SET_WAKE				88
/* #define WLC_GET_D11CNTS			89 */ /* -> "counters" iovar */
#define WLC_GET_FORCELINK			90	/* ndis only */
#define WLC_SET_FORCELINK			91	/* ndis only */
#define WLC_FREQ_ACCURACY			92	/* diag */
#define WLC_CARRIER_SUPPRESS			93	/* diag */
#define WLC_GET_PHYREG				94
#define WLC_SET_PHYREG				95
#define WLC_GET_RADIOREG			96
#define WLC_SET_RADIOREG			97
#define WLC_GET_REVINFO				98
#define WLC_GET_UCANTDIV			99
#define WLC_SET_UCANTDIV			100
#define WLC_R_REG				101
#define WLC_W_REG				102
/* #define WLC_DIAG_LOOPBACK			103	old tray diag */
/* #define WLC_RESET_D11CNTS			104 */ /* -> "reset_d11cnts" iovar */
#define WLC_GET_MACMODE				105
#define WLC_SET_MACMODE				106
#define WLC_GET_MONITOR				107
#define WLC_SET_MONITOR				108
#define WLC_GET_GMODE				109
#define WLC_SET_GMODE				110
#define WLC_GET_LEGACY_ERP			111
#define WLC_SET_LEGACY_ERP			112
#define WLC_GET_RX_ANT				113
#define WLC_GET_CURR_RATESET			114	/* current rateset */
#define WLC_GET_SCANSUPPRESS			115
#define WLC_SET_SCANSUPPRESS			116
#define WLC_GET_AP				117
#define WLC_SET_AP				118
#define WLC_GET_EAP_RESTRICT			119
#define WLC_SET_EAP_RESTRICT			120
#define WLC_SCB_AUTHORIZE			121
#define WLC_SCB_DEAUTHORIZE			122
#define WLC_GET_WDSLIST				123
#define WLC_SET_WDSLIST				124
#define WLC_GET_ATIM				125
#define WLC_SET_ATIM				126
#define WLC_GET_RSSI				127
#define WLC_GET_PHYANTDIV			128
#define WLC_SET_PHYANTDIV			129
#define WLC_AP_RX_ONLY				130
#define WLC_GET_TX_PATH_PWR			131
#define WLC_SET_TX_PATH_PWR			132
#define WLC_GET_WSEC				133
#define WLC_SET_WSEC				134
#define WLC_GET_PHY_NOISE			135
#define WLC_GET_BSS_INFO			136
#define WLC_GET_PKTCNTS				137
#define WLC_GET_LAZYWDS				138
#define WLC_SET_LAZYWDS				139
#define WLC_GET_BANDLIST			140
#define WLC_GET_BAND				141
#define WLC_SET_BAND				142
#define WLC_SCB_DEAUTHENTICATE			143
#define WLC_GET_SHORTSLOT			144
#define WLC_GET_SHORTSLOT_OVERRIDE		145
#define WLC_SET_SHORTSLOT_OVERRIDE		146
#define WLC_GET_SHORTSLOT_RESTRICT		147
#define WLC_SET_SHORTSLOT_RESTRICT		148
#define WLC_GET_GMODE_PROTECTION		149
#define WLC_GET_GMODE_PROTECTION_OVERRIDE	150
#define WLC_SET_GMODE_PROTECTION_OVERRIDE	151
#define WLC_UPGRADE				152
/* #define WLC_GET_MRATE			153 */ /* no longer supported */
/* #define WLC_SET_MRATE			154 */ /* no longer supported */
#define WLC_GET_IGNORE_BCNS			155
#define WLC_SET_IGNORE_BCNS			156
#define WLC_GET_SCB_TIMEOUT			157
#define WLC_SET_SCB_TIMEOUT			158
#define WLC_GET_ASSOCLIST			159
#define WLC_GET_CLK				160
#define WLC_SET_CLK				161
#define WLC_GET_UP				162
#define WLC_OUT					163
#define WLC_GET_WPA_AUTH			164
#define WLC_SET_WPA_AUTH			165
#define WLC_GET_UCFLAGS				166
#define WLC_SET_UCFLAGS				167
#define WLC_GET_PWRIDX				168
#define WLC_SET_PWRIDX				169
#define WLC_GET_TSSI				170
#define WLC_GET_SUP_RATESET_OVERRIDE		171
#define WLC_SET_SUP_RATESET_OVERRIDE		172
/* #define WLC_SET_FAST_TIMER			173 */ /* no longer supported */
/* #define WLC_GET_FAST_TIMER			174 */ /* no longer supported */
/* #define WLC_SET_SLOW_TIMER			175 */ /* no longer supported */
/* #define WLC_GET_SLOW_TIMER			176 */ /* no longer supported */
/* #define WLC_DUMP_PHYREGS			177 */ /* no longer supported */
#define WLC_GET_PROTECTION_CONTROL		178
#define WLC_SET_PROTECTION_CONTROL		179
#define WLC_GET_PHYLIST				180
#define WLC_ENCRYPT_STRENGTH			181	/* ndis only */
#define WLC_DECRYPT_STATUS			182	/* ndis only */
#define WLC_GET_KEY_SEQ				183
#define WLC_GET_SCAN_CHANNEL_TIME		184
#define WLC_SET_SCAN_CHANNEL_TIME		185
#define WLC_GET_SCAN_UNASSOC_TIME		186
#define WLC_SET_SCAN_UNASSOC_TIME		187
#define WLC_GET_SCAN_HOME_TIME			188
#define WLC_SET_SCAN_HOME_TIME			189
#define WLC_GET_SCAN_NPROBES			190
#define WLC_SET_SCAN_NPROBES			191
#define WLC_GET_PRB_RESP_TIMEOUT		192
#define WLC_SET_PRB_RESP_TIMEOUT		193
#define WLC_GET_ATTEN				194
#define WLC_SET_ATTEN				195
#define WLC_GET_SHMEM				196	/* diag */
#define WLC_SET_SHMEM				197	/* diag */
/* #define WLC_GET_GMODE_PROTECTION_CTS		198 */ /* no longer supported */
/* #define WLC_SET_GMODE_PROTECTION_CTS		199 */ /* no longer supported */
#define WLC_SET_WSEC_TEST			200
#define WLC_SCB_DEAUTHENTICATE_FOR_REASON	201
#define WLC_TKIP_COUNTERMEASURES		202
#define WLC_GET_PIOMODE				203
#define WLC_SET_PIOMODE				204
#define WLC_SET_ASSOC_PREFER			205
#define WLC_GET_ASSOC_PREFER			206
#define WLC_SET_ROAM_PREFER			207
#define WLC_GET_ROAM_PREFER			208
#define WLC_SET_LED				209
#define WLC_GET_LED				210
#define WLC_GET_INTERFERENCE_MODE		211
#define WLC_SET_INTERFERENCE_MODE		212
#define WLC_GET_CHANNEL_QA			213
#define WLC_START_CHANNEL_QA			214
#define WLC_GET_CHANNEL_SEL			215
#define WLC_START_CHANNEL_SEL			216
#define WLC_GET_VALID_CHANNELS			217
#define WLC_GET_FAKEFRAG			218
#define WLC_SET_FAKEFRAG			219
#define WLC_GET_PWROUT_PERCENTAGE		220
#define WLC_SET_PWROUT_PERCENTAGE		221
#define WLC_SET_BAD_FRAME_PREEMPT		222
#define WLC_GET_BAD_FRAME_PREEMPT		223
#define WLC_SET_LEAP_LIST			224
#define WLC_GET_LEAP_LIST			225
#define WLC_GET_CWMIN				226
#define WLC_SET_CWMIN				227
#define WLC_GET_CWMAX				228
#define WLC_SET_CWMAX				229
#define WLC_GET_WET				230
#define WLC_SET_WET				231
#define WLC_GET_PUB				232
/* #define WLC_SET_GLACIAL_TIMER		233 */ /* no longer supported */
/* #define WLC_GET_GLACIAL_TIMER		234 */ /* no longer supported */
#define WLC_GET_KEY_PRIMARY			235
#define WLC_SET_KEY_PRIMARY			236
/* #define WLC_DUMP_RADIOREGS			237 */ /* no longer supported */
#define WLC_GET_ACI_ARGS			238
#define WLC_SET_ACI_ARGS			239
#define WLC_UNSET_CALLBACK			240
#define WLC_SET_CALLBACK			241
#define WLC_GET_RADAR				242
#define WLC_SET_RADAR				243
#define WLC_SET_SPECT_MANAGMENT			244
#define WLC_GET_SPECT_MANAGMENT			245
#define WLC_WDS_GET_REMOTE_HWADDR		246	/* handled in wl_linux.c/wl_vx.c */
#define WLC_WDS_GET_WPA_SUP			247
#define WLC_SET_CS_SCAN_TIMER			248
#define WLC_GET_CS_SCAN_TIMER			249
#define WLC_MEASURE_REQUEST			250
#define WLC_INIT				251
#define WLC_SEND_QUIET				252
/* #define WLC_CHANNEL_SWITCH			253 */ /* no longer supported */
#define WLC_SEND_PWR_CONSTRAINT			254
#define WLC_UPGRADE_STATUS			255
#define WLC_CURRENT_PWR				256
#define WLC_GET_SCAN_PASSIVE_TIME		257
#define WLC_SET_SCAN_PASSIVE_TIME		258
#define WLC_LEGACY_LINK_BEHAVIOR		259
#define WLC_GET_CHANNELS_IN_COUNTRY		260
#define WLC_GET_COUNTRY_LIST			261
#define WLC_GET_VAR				262	/* get value of named variable */
#define WLC_SET_VAR				263	/* set named variable to value */
#define WLC_NVRAM_GET				264	/* deprecated */
#define WLC_NVRAM_SET				265
#define WLC_NVRAM_DUMP				266
#define WLC_REBOOT				267
#define WLC_SET_WSEC_PMK			268
#define WLC_GET_AUTH_MODE			269
#define WLC_SET_AUTH_MODE			270
#define WLC_GET_WAKEENTRY			271
#define WLC_SET_WAKEENTRY			272
#define WLC_NDCONFIG_ITEM			273	/* currently handled in wl_oid.c */
#define WLC_NVOTPW				274
#define WLC_OTPW				275
#define WLC_IOV_BLOCK_GET			276
#define WLC_IOV_MODULES_GET			277
#define WLC_SOFT_RESET				278
#define WLC_GET_ALLOW_MODE			279
#define WLC_SET_ALLOW_MODE			280
#define WLC_GET_DESIRED_BSSID			281
#define WLC_SET_DESIRED_BSSID			282
#define	WLC_DISASSOC_MYAP			283
#define WLC_LAST				284	/* do not change - use get_var/set_var */

/*
 * Minor kludge alert:
 * Duplicate a few definitions that irelay requires from epiioctl.h here
 * so caller doesn't have to include this file and epiioctl.h .
 * If this grows any more, it would be time to move these irelay-specific
 * definitions out of the epiioctl.h and into a separate driver common file.
 */
#ifndef EPICTRL_COOKIE
#define EPICTRL_COOKIE		0xABADCEDE
#endif

/* vx wlc ioctl's offset */
#define CMN_IOCTL_OFF 0x180

/*
 * custom OID support
 *
 * 0xFF - implementation specific OID
 * 0xE4 - first byte of Broadcom PCI vendor ID
 * 0x14 - second byte of Broadcom PCI vendor ID
 * 0xXX - the custom OID number
 */

/* begin 0x1f values beyond the start of the ET driver range. */
#define WL_OID_BASE		0xFFE41420

/* NDIS overrides */
#define OID_WL_GETINSTANCE	(WL_OID_BASE + WLC_GET_INSTANCE)
#define OID_WL_GET_FORCELINK	(WL_OID_BASE + WLC_GET_FORCELINK)
#define OID_WL_SET_FORCELINK	(WL_OID_BASE + WLC_SET_FORCELINK)
#define	OID_WL_ENCRYPT_STRENGTH	(WL_OID_BASE + WLC_ENCRYPT_STRENGTH)
#define OID_WL_DECRYPT_STATUS	(WL_OID_BASE + WLC_DECRYPT_STATUS)
#define OID_LEGACY_LINK_BEHAVIOR (WL_OID_BASE + WLC_LEGACY_LINK_BEHAVIOR)
#define OID_WL_NDCONFIG_ITEM (WL_OID_BASE + WLC_NDCONFIG_ITEM)

#define WL_DECRYPT_STATUS_SUCCESS	1
#define WL_DECRYPT_STATUS_FAILURE	2
#define WL_DECRYPT_STATUS_UNKNOWN	3

/* allows user-mode app to poll the status of USB image upgrade */
#define WLC_UPGRADE_SUCCESS			0
#define WLC_UPGRADE_PENDING			1

#ifdef CONFIG_USBRNDIS_RETAIL
/* struct passed in for WLC_NDCONFIG_ITEM */
typedef struct {
	char *name;
	void *param;
} ndconfig_item_t;
#endif

/* Bit masks for radio disabled status - returned by WL_GET_RADIO */
#define WL_RADIO_SW_DISABLE		(1<<0)
#define WL_RADIO_HW_DISABLE		(1<<1)
#define WL_RADIO_MPC_DISABLE		(1<<2)
#define WL_RADIO_COUNTRY_DISABLE	(1<<3)	/* some countries don't support any channel */

/* Override bit for WLC_SET_TXPWR.  if set, ignore other level limits */
#define WL_TXPWR_OVERRIDE	(1<<31)

/* "diag" iovar argument and error code */
#define WL_DIAG_INTERRUPT			1	/* d11 loopback interrupt test */
#define WL_DIAG_LOOPBACK			2	/* d11 loopback data test */
#define WL_DIAG_MEMORY				3	/* d11 memory test */
#define WL_DIAG_LED				4	/* LED test */
#define WL_DIAG_REG				5	/* d11/phy register test */
#define WL_DIAG_SROM				6	/* srom read/crc test */
#define WL_DIAG_DMA				7	/* DMA test */

#define WL_DIAGERR_SUCCESS			0
#define WL_DIAGERR_FAIL_TO_RUN			1	/* unable to run requested diag */
#define WL_DIAGERR_NOT_SUPPORTED		2	/* diag requested is not supported */
#define WL_DIAGERR_INTERRUPT_FAIL		3	/* loopback interrupt test failed */
#define WL_DIAGERR_LOOPBACK_FAIL		4	/* loopback data test failed */
#define WL_DIAGERR_SROM_FAIL			5	/* srom read failed */
#define WL_DIAGERR_SROM_BADCRC			6	/* srom crc failed */
#define WL_DIAGERR_REG_FAIL			7	/* d11/phy register test failed */
#define WL_DIAGERR_MEMORY_FAIL			8	/* d11 memory test failed */
#define WL_DIAGERR_NOMEM			9	/* diag test failed due to no memory */
#define WL_DIAGERR_DMA_FAIL			10	/* DMA test failed */

#define WL_DIAGERR_MEMORY_TIMEOUT		11	/* d11 memory test didn't finish in time */
#define WL_DIAGERR_MEMORY_BADPATTERN		12	/* d11 memory test result in bad pattern */

/* band types */
#define	WLC_BAND_AUTO		0	/* auto-select */
#define	WLC_BAND_5G		1	/* 5 Ghz */
#define	WLC_BAND_2G		2	/* 2.4 Ghz */
#define	WLC_BAND_ALL		3	/* all bands */

/* phy types (returned by WLC_GET_PHYTPE) */
#define	WLC_PHY_TYPE_A		0
#define	WLC_PHY_TYPE_B		1
#define	WLC_PHY_TYPE_G		2
#define	WLC_PHY_TYPE_N		4
#define	WLC_PHY_TYPE_LP		5
#define	WLC_PHY_TYPE_NULL	0xf

/* MAC list modes */
#define WLC_MACMODE_DISABLED	0	/* MAC list disabled */
#define WLC_MACMODE_DENY	1	/* Deny specified (i.e. allow unspecified) */
#define WLC_MACMODE_ALLOW	2	/* Allow specified (i.e. deny unspecified) */

/*
 * 54g modes (basic bits may still be overridden)
 *
 * GMODE_LEGACY_B			Rateset: 1b, 2b, 5.5, 11
 *					Preamble: Long
 *					Shortslot: Off
 * GMODE_AUTO				Rateset: 1b, 2b, 5.5b, 11b, 18, 24, 36, 54
 *					Extended Rateset: 6, 9, 12, 48
 *					Preamble: Long
 *					Shortslot: Auto
 * GMODE_ONLY				Rateset: 1b, 2b, 5.5b, 11b, 18, 24b, 36, 54
 *					Extended Rateset: 6b, 9, 12b, 48
 *					Preamble: Short required
 *					Shortslot: Auto
 * GMODE_B_DEFERRED			Rateset: 1b, 2b, 5.5b, 11b, 18, 24, 36, 54
 *					Extended Rateset: 6, 9, 12, 48
 *					Preamble: Long
 *					Shortslot: On
 * GMODE_PERFORMANCE			Rateset: 1b, 2b, 5.5b, 6b, 9, 11b, 12b, 18, 24b, 36, 48, 54
 *					Preamble: Short required
 *					Shortslot: On and required
 * GMODE_LRS				Rateset: 1b, 2b, 5.5b, 11b
 *					Extended Rateset: 6, 9, 12, 18, 24, 36, 48, 54
 *					Preamble: Long
 *					Shortslot: Auto
 */
#define GMODE_LEGACY_B		0
#define GMODE_AUTO		1
#define GMODE_ONLY		2
#define GMODE_B_DEFERRED	3
#define GMODE_PERFORMANCE	4
#define GMODE_LRS		5
#define GMODE_MAX		6

/* values for PLCPHdr_override */
#define WLC_PLCP_AUTO	-1
#define WLC_PLCP_SHORT	0
#define WLC_PLCP_LONG	1

/* values for g_protection_override and n_protection_override */
#define WLC_PROTECTION_AUTO		-1
#define WLC_PROTECTION_OFF		0
#define WLC_PROTECTION_ON		1
#define WLC_PROTECTION_MMHDR_ONLY	2
#define WLC_PROTECTION_CTS_ONLY		3

/* values for g_protection_control and n_protection_control */
#define WLC_PROTECTION_CTL_OFF		0
#define WLC_PROTECTION_CTL_LOCAL	1
#define WLC_PROTECTION_CTL_OVERLAP	2

/* deprecated const names for g_protection_override */
#define WLC_G_PROTECTION_AUTO		WLC_PROTECTION_AUTO
#define WLC_G_PROTECTION_OFF		WLC_PROTECTION_OFF
#define WLC_G_PROTECTION_ON		WLC_PROTECTION_ON

/* deprecated const names for g_protection_control */
#define WLC_G_PROTECTION_CTL_OFF	WLC_PROTECTION_CTL_OFF
#define WLC_G_PROTECTION_CTL_LOCAL	WLC_PROTECTION_CTL_LOCAL
#define WLC_G_PROTECTION_CTL_OVERLAP	WLC_PROTECTION_CTL_OVERLAP

/* deprecated const names for get/set g_protection_control */
#define WLC_GET_GMODE_PROTECTION_CONTROL	WLC_GET_PROTECTION_CONTROL
#define WLC_SET_GMODE_PROTECTION_CONTROL	WLC_SET_PROTECTION_CONTROL

/* values for n_protection */
#define WLC_N_PROTECTION_OFF		0
#define WLC_N_PROTECTION_OPTIONAL	1
#define WLC_N_PROTECTION_20IN40		2
#define WLC_N_PROTECTION_MIXEDMODE	3

/* values for n_preamble_type */
#define WLC_N_PREAMBLE_MIXEDMODE	0
#define WLC_N_PREAMBLE_GF		1

/* values for band specific 40MHz capabilities */
#define WLC_N_BW_20ALL			0
#define WLC_N_BW_40ALL			1
#define WLC_N_BW_20IN2G_40IN5G		2

/* values to force tx/rx chain */
#define WLC_N_TXRX_CHAIN0		0
#define WLC_N_TXRX_CHAIN1		1

/* Values for PM */
#define PM_OFF	0
#define PM_MAX	1
#define PM_FAST 2

/* interference mitigation options */
#define	INTERFERE_NONE	0	/* off */
#define	NON_WLAN	1	/* foreign/non 802.11 interference, no auto detect */
#define	WLAN_MANUAL	2	/* ACI: no auto detection */
#define	WLAN_AUTO	3	/* ACI: auto - detact */
#define AUTO_ACTIVE	(1 << 7) /* Auto is currently active */

/* Begin: wl_aci_args_t */
typedef struct wl_aci_args {
	int enter_aci_thresh; /* Trigger level to start detecting ACI */
	int exit_aci_thresh; /* Trigger level to exit ACI mode */
	int usec_spin; /* microsecs to delay between rssi samples */
	int glitch_delay; /* interval between ACI scans when glitch count is consistently high */
	uint16 nphy_adcpwr_enter_thresh;	/* ADC power to enter ACI mitigation mode */
	uint16 nphy_adcpwr_exit_thresh;	/* ADC power to exit ACI mitigation mode */
	uint16 nphy_repeat_ctr;		/* Number of tries per channel to compute power */
	uint16 nphy_num_samples;	/* Number of samples to compute power on one channel */
	uint16 nphy_undetect_window_sz;	/* num of undetects to exit ACI Mitigation mode */
	uint16 nphy_b_energy_lo_aci;	/* low ACI power energy threshold for bphy */
	uint16 nphy_b_energy_md_aci;	/* mid ACI power energy threshold for bphy */
	uint16 nphy_b_energy_hi_aci;	/* high ACI power energy threshold for bphy */
} wl_aci_args_t;
/* End: wl_aci_args_t */

#define WL_ACI_ARGS_LEGACY_LENGTH	16	/* bytes of pre NPHY aci args */

/* Begin: wl_radar_args_t */
typedef struct {
	int npulses; 	/* required number of pulses at n * t_int */
	int ncontig; 	/* required number of pulses at t_int */
	int min_pw; 	/* minimum pulse width (20 MHz clocks) */
	int max_pw; 	/* maximum pulse width (20 MHz clocks) */
	uint16 thresh0;	/* Radar detection, thresh 0 */
	uint16 thresh1;	/* Radar detection, thresh 1 */
	uint16 blank;	/* Radar detection, blank control */
	uint16 fmdemodcfg;	/* Radar detection, fmdemod config */
	int npulses_lp;  /* Radar detection, minimum long pulses */
	int min_pw_lp; /* Minimum pulsewidth for long pulses */
	int max_pw_lp; /* Maximum pulsewidth for long pulses */
	int min_fm_lp; /* Minimum fm for long pulses */
	int max_deltat_lp;  /* Maximum deltat for long pulses */
	int min_deltat; /* Minimum spacing between pulses */
	int max_deltat; /* Maximum spacing between pulses */
} wl_radar_args_t;
/* End: wl_radar_args_t */

/* radar iovar SET defines */
#define WL_RADAR_DETECTOR_OFF		0	/* radar detector off */
#define WL_RADAR_DETECTOR_ON		1	/* radar detector on */
#define WL_RADAR_SIMULATED		2	/* force radar detector to declare
						 * detection once
						 */

#define WL_RSSI_ANT_VERSION	1	/* current version of wl_rssi_ant_t */
#define WL_RSSI_ANT_MAX		4	/* max possible rx antennas */

/* RSSI per antenna */
typedef struct {
	uint32	version;		/* version field */
	uint32	count;			/* number of valid antenna rssi */
	int8 rssi_ant[WL_RSSI_ANT_MAX];	/* rssi per antenna */
} wl_rssi_ant_t;


/* dfs_status iovar-related defines */

/* cac - channel availability check,
 * ism - in-service monitoring
 * csa - channel switching announcement
 */

/* cac state values */
#define WL_DFS_CACSTATE_IDLE		0	/* state for operating in non-radar channel */
#define	WL_DFS_CACSTATE_PREISM_CAC	1	/* CAC in progress */
#define WL_DFS_CACSTATE_ISM		2	/* ISM in progress */
#define WL_DFS_CACSTATE_CSA		3	/* csa */
#define WL_DFS_CACSTATE_POSTISM_CAC	4	/* ISM CAC */
#define WL_DFS_CACSTATE_PREISM_OOC	5	/* PREISM OOC */
#define WL_DFS_CACSTATE_POSTISM_OOC	6	/* POSTISM OOC */
#define WL_DFS_CACSTATES		7	/* this many states exist */

/* data structure used in 'dfs_status' wl interface, which is used to query dfs status */
typedef struct {
	uint state;		/* noted by WL_DFS_CACSTATE_XX. */
	uint duration;		/* time spent in ms in state. */
	/* as dfs enters ISM state, it removes the operational channel from quiet channel
	 * list and notes the channel in channel_cleared. set to 0 if no channel is cleared
	 */
	chanspec_t chanspec_cleared;
	/* chanspec cleared used to be a uint, add another to uint16 to maintain size */
	uint16 pad;
} wl_dfs_status_t;

/* Begin: tx_power_legacy_t */
#define NUM_PWRCTRL_RATES 12
typedef struct {
	uint8 txpwr_band_max[NUM_PWRCTRL_RATES];	/* User set target */
	uint8 txpwr_limit[NUM_PWRCTRL_RATES];		/* reg and local power limit */
	uint8 txpwr_local_max;				/* local max according to the AP */
	uint8 txpwr_local_constraint;			/* local constraint according to the AP */
	uint8 txpwr_chan_reg_max;			/* Regulatory max for this channel */
	uint8 txpwr_target[2][NUM_PWRCTRL_RATES];	/* Latest target for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout[2];			/* Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_opo[NUM_PWRCTRL_RATES];		/* On G phy, OFDM power offset */
	uint8 txpwr_bphy_cck_max[NUM_PWRCTRL_RATES];	/* Max CCK power for this band (SROM) */
	uint8 txpwr_bphy_ofdm_max;			/* Max OFDM power for this band (SROM) */
	uint8 txpwr_aphy_max[NUM_PWRCTRL_RATES];	/* Max power for A band (SROM) */
	int8  txpwr_antgain[2];				/* Ant gain for each band - from SROM */
	uint8 txpwr_est_Pout_gofdm;			/* Pwr estimate for 2.4 OFDM */
} tx_power_legacy_t;
/* End: tx_power_legacy_t */

/* Begin: tx_power_t */
#define WL_TX_POWER_RATES	45
#define WL_TX_POWER_CCK_FIRST	0
#define WL_TX_POWER_CCK_NUM	4
#define WL_TX_POWER_OFDM_FIRST	4
#define WL_TX_POWER_OFDM_NUM	8
#define WL_TX_POWER_MCS20_FIRST	12
#define WL_TX_POWER_MCS20_NUM	16
#define WL_TX_POWER_MCS40_FIRST	28
#define WL_TX_POWER_MCS40_NUM	17
typedef struct {
	uint32 flags;
	chanspec_t chanspec;			/* txpwr report for this channel */
	chanspec_t local_chanspec;		/* channel on which we are associated */
	uint8 local_max;			/* local max according to the AP */
	uint8 local_constraint;			/* local constraint according to the AP */
	int8  antgain[2];			/* Ant gain for each band - from SROM */
	uint8 rf_cores;				/* count of RF Cores being reported */
	uint8 est_Pout[4];			/* Latest tx power out estimate per RF chain */
	uint8 est_Pout_cck;			/* Latest CCK tx power out estimate */
	uint8 user_limit[WL_TX_POWER_RATES];	/* User limit */
	uint8 reg_limit[WL_TX_POWER_RATES];	/* Regulatory power limit */
	uint8 board_limit[WL_TX_POWER_RATES];	/* Max power board can support (SROM) */
	uint8 target[WL_TX_POWER_RATES];	/* Latest target power */
} tx_power_t;

/* tx_power_t.flags bits */
#define WL_TX_POWER_F_ENABLED	1
#define WL_TX_POWER_F_HW	2
#define WL_TX_POWER_F_MIMO	4

/* End: tx_power_t */

typedef struct tx_inst_power {
	uint8 txpwr_est_Pout[2];			/* Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout_gofdm;			/* Pwr estimate for 2.4 OFDM */
} tx_inst_power_t;

/* 802.11h measurement types */
#define WLC_MEASURE_TPC			1
#define WLC_MEASURE_CHANNEL_BASIC	2
#define WLC_MEASURE_CHANNEL_CCA		3
#define WLC_MEASURE_CHANNEL_RPI		4

/* regulatory enforcement levels */
#define SPECT_MNGMT_OFF			0		/* both 11h and 11d disabled */
#define SPECT_MNGMT_LOOSE_11H		1		/* allow non-11h APs in scan lists */
#define SPECT_MNGMT_STRICT_11H		2		/* prune out non-11h APs from scan list */
#define SPECT_MNGMT_STRICT_11D		3		/* switch to 802.11D mode */
/* SPECT_MNGMT_LOOSE_11H_D - same as SPECT_MNGMT_LOOSE with the exception that Country IE
 * adoption is done irregardless of capability-spectrum_management
 */
#define SPECT_MNGMT_LOOSE_11H_D		4		/* operation defined above */

#define WL_CHAN_VALID_HW	(1 << 0)	/* valid with current HW */
#define WL_CHAN_VALID_SW	(1 << 1)	/* valid with current country setting */
#define WL_CHAN_BAND_5G		(1 << 2)	/* 5GHz-band channel */
#define WL_CHAN_RADAR		(1 << 3)	/* radar sensitive  channel */
#define WL_CHAN_INACTIVE	(1 << 4)	/* temporarily inactive due to radar */
#define WL_CHAN_PASSIVE		(1 << 5)	/* channel is in passive mode */
#define WL_CHAN_RESTRICTED	(1 << 6)	/* restricted use channel */

/* BTC mode used by "btc_mode" iovar */
#define	WL_BTC_DISABLE		0	/* disable BT coexistence */
#define WL_BTC_ENABLE		1	/* enable BT coexistence */
#define WL_BTC_PREMPT		2	/* enable BT coexistence and BT preemption */

/* Message levels */
#define WL_ERROR_VAL		0x00000001
#define WL_TRACE_VAL		0x00000002
#define WL_PRHDRS_VAL		0x00000004
#define WL_PRPKT_VAL		0x00000008
#define WL_INFORM_VAL		0x00000010
#define WL_TMP_VAL		0x00000020
#define WL_OID_VAL		0x00000040
#define WL_RATE_VAL		0x00000080
#define WL_ASSOC_VAL		0x00000100
#define WL_PRUSR_VAL		0x00000200
#define WL_PS_VAL		0x00000400
#define WL_TXPWR_VAL		0x00000800
#define WL_GMODE_VAL		0x00001000
#define WL_DUAL_VAL		0x00002000
#define WL_WSEC_VAL		0x00004000
#define WL_WSEC_DUMP_VAL	0x00008000
#define WL_LOG_VAL		0x00010000
#define WL_NRSSI_VAL		0x00020000
#define WL_LOFT_VAL		0x00040000
#define WL_REGULATORY_VAL	0x00080000
#define WL_PHYCAL_VAL		0x00100000
#define WL_RADAR_VAL		0x00200000
#define WL_MPC_VAL		0x00400000
#define WL_APSTA_VAL		0x00800000
#define WL_DFS_VAL		0x01000000
#define WL_BA_VAL		0x02000000
#define WL_MBSS_VAL		0x04000000
#define WL_CAC_VAL		0x08000000
#define WL_AMSDU_VAL		0x10000000
#define WL_AMPDU_VAL		0x20000000
#define WL_FFPLD_VAL		0x40000000

/* max # of leds supported by GPIO (gpio pin# == led index#) */
#define	WL_LED_NUMGPIO		16	/* gpio 0-15 */

/* led per-pin behaviors */
#define	WL_LED_OFF		0		/* always off */
#define	WL_LED_ON		1		/* always on */
#define	WL_LED_ACTIVITY		2		/* activity */
#define	WL_LED_RADIO		3		/* radio enabled */
#define	WL_LED_ARADIO		4		/* 5  Ghz radio enabled */
#define	WL_LED_BRADIO		5		/* 2.4Ghz radio enabled */
#define	WL_LED_BGMODE		6		/* on if gmode, off if bmode */
#define	WL_LED_WI1		7		
#define	WL_LED_WI2		8		
#define	WL_LED_WI3		9		
#define	WL_LED_ASSOC		10		/* associated state indicator */
#define	WL_LED_INACTIVE		11		/* null behavior (clears default behavior) */
#define	WL_LED_ASSOCACT		12		/* on when associated; blink fast for activity */
#define	WL_LED_NUMBEHAVIOR	13

/* led behavior numeric value format */
#define	WL_LED_BEH_MASK		0x7f		/* behavior mask */
#define	WL_LED_AL_MASK		0x80		/* activelow (polarity) bit */

/* maximum channels returned by the get valid channels iovar */
#define WL_NUMCHANNELS		64
#define WL_NUMCHANSPECS		100

/* WDS link local endpoint WPA role */
#define WL_WDS_WPA_ROLE_AUTH	0	/* authenticator */
#define WL_WDS_WPA_ROLE_SUP	1	/* supplicant */
#define WL_WDS_WPA_ROLE_AUTO	255	/* auto, based on mac addr value */

/* number of bytes needed to define a 128-bit mask for MAC event reporting */
#define WL_EVENTING_MASK_LEN	16

/* Structures and constants used for "vndr_ie" IOVar interface */
#define VNDR_IE_CMD_LEN		4	/* length of the set command string:
					 * "add", "del" (+ NULL)
					 */

/* 802.11 Mgmt Packet flags */
#define VNDR_IE_BEACON_FLAG	0x1
#define VNDR_IE_PRBRSP_FLAG	0x2
#define VNDR_IE_ASSOCRSP_FLAG	0x4
#define VNDR_IE_AUTHRSP_FLAG	0x8
#define VNDR_IE_PRBREQ_FLAG	0x10
#define VNDR_IE_ASSOCREQ_FLAG	0x20

#define VNDR_IE_INFO_HDR_LEN	(sizeof(uint32))

typedef struct {
	uint32 pktflag;			/* bitmask indicating which packet(s) contain this IE */
	vndr_ie_t vndr_ie_data;		/* vendor IE data */
} vndr_ie_info_t;

typedef struct {
	int iecount;			/* number of entries in the vndr_ie_list[] array */
	vndr_ie_info_t vndr_ie_list[1];	/* variable size list of vndr_ie_info_t structs */
} vndr_ie_buf_t;

typedef struct {
	char cmd[VNDR_IE_CMD_LEN];	/* vndr_ie IOVar set command : "add", "del" + NULL */
	vndr_ie_buf_t vndr_ie_buffer;	/* buffer containing Vendor IE list information */
} vndr_ie_setbuf_t;

/* join preference iovar value */
/*
 * Join preference iovar value is an array of tuples. Each tuple has a one-byte type,
 * a one-byte length, and a variable length value.  RSSI type tuple must be present
 * in the array.
 *
 * Types are defined in "join preference types" section.
 *
 * Length is the value size in octets. It is reserved for WL_JOIN_PREF_WPA type tuple
 * and must be set to zero.
 *
 * Values are defined below.
 *
 * 1. RSSI - 2 octets
 * offset 0: reserved
 * offset 1: reserved
 *
 * 2. WPA - 2 + 12 * n octets (n is # tuples defined below)
 * offset 0: reserved
 * offset 1: # of tuples
 * offset 2: tuple 1
 * offset 14: tuple 2
 * ...
 * offset 2 + 12 * (n - 1) octets: tuple n
 *
 * struct wpa_cfg_tuple {
 *   uint8 akm[DOT11_OUI_LEN+1];     akm suite
 *   uint8 ucipher[DOT11_OUI_LEN+1]; unicast cipher suite
 *   uint8 mcipher[DOT11_OUI_LEN+1]; multicast cipher suite
 * };
 *
 * multicast cipher suite can be specified as a specific cipher suite or WL_WPA_ACP_MCS_ANY.
 *
 * 3. BAND - 2 octets
 * offset 0: reserved
 * offset 1: see "band preference" and "band types"
 */
/* join preference types */
#define WL_JOIN_PREF_RSSI	1	/* by RSSI */
#define WL_JOIN_PREF_WPA	2	/* by akm and ciphers */
#define WL_JOIN_PREF_BAND	3	/* by 802.11 band */

/* band preference */
#define WLJP_BAND_ASSOC_PREF	255	/* use what WLC_SET_ASSOC_PREFER ioctl specifies */

/* any multicast cipher suite */
#define WL_WPA_ACP_MCS_ANY	"\x00\x00\x00\x00"

struct tsinfo_arg {
	uint8 octets[3];
};

#if !defined(__GNUC__)
#pragma pack(pop)
#endif

#define	NFIFO			6	/* # tx/rx fifopairs */

#define	WL_CNT_T_VERSION	3	/* current version of wl_cnt_t struct */

typedef struct {
	uint16	version;	/* see definition of WL_CNT_T_VERSION */
	uint16	length;		/* length of entire structure */

	/* transmit stat counters */
	uint32	txframe;	/* tx data frames */
	uint32	txbyte;		/* tx data bytes */
	uint32	txretrans;	/* tx mac retransmits */
	uint32	txerror;	/* tx data errors (derived: sum of others) */
	uint32	txctl;		/* tx management frames */
	uint32	txprshort;	/* tx short preamble frames */
	uint32	txserr;		/* tx status errors */
	uint32	txnobuf;	/* tx out of buffers errors */
	uint32	txnoassoc;	/* tx discard because we're not associated */
	uint32	txrunt;		/* tx runt frames */
	uint32	txchit;		/* tx header cache hit (fastpath) */
	uint32	txcmiss;	/* tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/* tx fifo underflows */
	uint32	txphyerr;	/* tx phy errors (indicated in tx status) */
	uint32	txphycrs;	

	/* receive stat counters */
	uint32	rxframe;	/* rx data frames */
	uint32	rxbyte;		/* rx data bytes */
	uint32	rxerror;	/* rx data errors (derived: sum of others) */
	uint32	rxctl;		/* rx management frames */
	uint32	rxnobuf;	/* rx out of buffers errors */
	uint32	rxnondata;	/* rx non data frames in the data channel errors */
	uint32	rxbadds;	/* rx bad DS errors */
	uint32	rxbadcm;	/* rx bad control or management frames */
	uint32	rxfragerr;	/* rx fragmentation errors */
	uint32	rxrunt;		/* rx runt frames */
	uint32	rxgiant;	/* rx giant frames */
	uint32	rxnoscb;	/* rx no scb error */
	uint32	rxbadproto;	/* rx invalid frames */
	uint32	rxbadsrcmac;	/* rx frames with Invalid Src Mac */
	uint32	rxbadda;	/* rx frames tossed for invalid da */
	uint32	rxfilter;	/* rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/* rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/* rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/* d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/* d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/* d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/* tx/rx dma descriptor errors */
	uint32	dmada;		/* tx/rx dma data errors */
	uint32	dmape;		/* tx/rx dma descriptor protocol errors */
	uint32	reset;		/* reset count */
	uint32	tbtt;		/* cnts the TBTT int's */
	uint32	txdmawar;	
	uint32	pkt_callback_reg_fail;	/* callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/* total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/* number of RTS sent out by the MAC */
	uint32	txctsfrm;	/* number of CTS sent out by the MAC */
	uint32	txackfrm;	/* number of ACK frames sent out */
	uint32	txdnlfrm;	/* Not used */
	uint32	txbcnfrm;	/* beacons transmitted */
	uint32	txfunfl[8];	/* per-fifo tx underflows */
	uint32	txtplunfl;	/* Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/* Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32	rxfrmtoolong;	/* Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt;	/* Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/* Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/* number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/* parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/* PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/* Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast;	/* number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/* number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/* number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/* number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast;	/* number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast;	/* number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast;	/* number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/* number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/* number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/* number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/* number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/* number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/* beacons received from member of BSS */
	uint32	rxdfrmucastobss; /* number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/* beacons received from other BSS */
	uint32	rxrsptmout;	/* Number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/* transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxf0ovfl;	/* Number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/* Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/* Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/* Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/* Number of PMQ overflows */
	uint32	rxcgprqfrm;	/* Number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/* Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/* Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/* Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/* Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/* Number of NACKS received (Afterburner) */
	uint32	frmscons;	/* Number of frames completed without transmission because of an
				 * Afterburner re-queue
				 */
	uint32	txnack;		/* Number of NACKs transmitted (Afterburner) */
	uint32	txglitch_nack;	/* obsolete */
	uint32	txburst;	/* obsolete */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/* dot11TransmittedFragmentCount */
	uint32	txmulti;	/* dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/* dot11FailedCount */
	uint32	txretry;	/* dot11RetryCount */
	uint32	txretrie;	/* dot11MultipleRetryCount */
	uint32	rxdup;		/* dot11FrameduplicateCount */
	uint32	txrts;		/* dot11RTSSuccessCount */
	uint32	txnocts;	/* dot11RTSFailureCount */
	uint32	txnoack;	/* dot11ACKFailureCount */
	uint32	rxfrag;		/* dot11ReceivedFragmentCount */
	uint32	rxmulti;	/* dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/* dot11FCSErrorCount */
	uint32	txfrmsnt;	/* dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/* dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/* TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/* TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/* TKIPReplays */
	uint32	ccmpfmterr;	/* CCMPFormatErrors */
	uint32	ccmpreplay;	/* CCMPReplays */
	uint32	ccmpundec;	/* CCMPDecryptErrors */
	uint32	fourwayfail;	/* FourWayHandshakeFailures */
	uint32	wepundec;	/* dot11WEPUndecryptableCount */
	uint32	wepicverr;	/* dot11WEPICVErrorCount */
	uint32	decsuccess;	/* DecryptSuccessCount */
	uint32	tkipicverr;	/* TKIPICVErrorCount */
	uint32	wepexcluded;	/* dot11WEPExcludedCount */

	uint32	txchanrej;	/* Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/* Count PSM watchdogs */
	uint32	phywatchdog;	/* Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/* PRQ entries read in */
	uint32	prq_undirected_entries;	/*    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/*    which could not be translated to info */
	uint32	atim_suppress_count;	/* TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/* Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /* ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/* TBTT DPC did not happen in time */
} wl_cnt_t;

#define WL_WME_CNT_VERSION	1	/* current version of wl_wme_cnt_t */

typedef struct {
	uint32 packets;
	uint32 bytes;
} wl_traffic_stats_t;

typedef struct {
	uint16	version;	/* see definition of WL_WME_CNT_VERSION */
	uint16	length;		/* length of entire structure */

	wl_traffic_stats_t tx[AC_COUNT];	/* Packets transmitted */
	wl_traffic_stats_t tx_failed[AC_COUNT];	/* Packets dropped or failed to transmit */
	wl_traffic_stats_t rx[AC_COUNT];	/* Packets received */
	wl_traffic_stats_t rx_failed[AC_COUNT];	/* Packets failed to receive */

	wl_traffic_stats_t forward[AC_COUNT];	/* Packets forwarded by AP */

	wl_traffic_stats_t tx_expired[AC_COUNT];	/* packets dropped due to lifetime expiry */

} wl_wme_cnt_t;

#ifdef WLBA

#define	WLC_BA_CNT_VERSION	1	/* current version of wlc_ba_cnt_t */

/* block ack related stats */
typedef struct wlc_ba_cnt {
	uint16	version;	/* WLC_BA_CNT_VERSION */
	uint16	length;		/* length of entire structure */

	/* transmit stat counters */
	uint32 txpdu;		/* pdus sent */
	uint32 txsdu;		/* sdus sent */
	uint32 txfc;		/* tx side flow controlled packets */
	uint32 txfci;		/* tx side flow control initiated */
	uint32 txretrans;	/* retransmitted pdus */
	uint32 txbatimer;	/* ba resend due to timer */
	uint32 txdrop;		/* dropped packets */
	uint32 txaddbareq;	/* addba req sent */
	uint32 txaddbaresp;	/* addba resp sent */
	uint32 txdelba;		/* delba sent */
	uint32 txba;		/* ba sent */
	uint32 txbar;		/* bar sent */
	uint32 txpad[4];	/* future */

	/* receive side counters */
	uint32 rxpdu;		/* pdus recd */
	uint32 rxqed;		/* pdus buffered before sending up */
	uint32 rxdup;		/* duplicate pdus */
	uint32 rxnobuf;		/* pdus discarded due to no buf */
	uint32 rxaddbareq;	/* addba req recd */
	uint32 rxaddbaresp;	/* addba resp recd */
	uint32 rxdelba;		/* delba recd */
	uint32 rxba;		/* ba recd */
	uint32 rxbar;		/* bar recd */
	uint32 rxinvba;		/* invalid ba recd */
	uint32 rxbaholes;	/* ba recd with holes */
	uint32 rxunexp;		/* unexpected packets */
	uint32 rxpad[4];	/* future */
} wlc_ba_cnt_t;
#endif /* WLBA */

/* structure for per-tid ampdu control */
struct ampdu_tid_control {
	uint8 tid;			/* tid */
	uint8 enable;			/* enable/disable */
};

/* structure for identifying ea/tid for sending addba/delba */
struct ampdu_ea_tid {
	struct ether_addr ea;		/* Station address */
	uint8 tid;			/* tid */
};

/* Different discovery modes for dpt */
#define	DPT_DISCOVERY_MANUAL	0x01	/* manual discovery mode */
#define	DPT_DISCOVERY_AUTO	0x02	/* auto discovery mode */
#define	DPT_DISCOVERY_SCAN	0x04	/* scan-based discovery mode */

/* different path selection values */
#define DPT_PATHSEL_AUTO	0	/* auto mode for path selection */
#define DPT_PATHSEL_DIRECT	1	/* always use direct DPT path */
#define DPT_PATHSEL_APPATH	2	/* always use AP path */

/* different ops for deny list */
#define DPT_DENY_LIST_ADD 	1	/* add to dpt deny list */
#define DPT_DENY_LIST_REMOVE 	2	/* remove from dpt deny list */

/* different ops for manual end point */
#define DPT_MANUAL_EP_CREATE	1	/* create manual dpt endpoint */
#define DPT_MANUAL_EP_MODIFY	2	/* modify manual dpt endpoint */
#define DPT_MANUAL_EP_DELETE	3	/* delete manual dpt endpoint */

/* structure for dpt iovars */
typedef struct dpt_iovar {
	struct ether_addr ea;		/* Station address */
	uint8 mode;			/* mode: depends on iovar */
	uint32 pad;			/* future */
} dpt_iovar_t;

/* structure for addts arguments */
/* For ioctls that take a list of TSPEC */
struct tslist {
	int count;			/* number of tspecs */
	struct tsinfo_arg tsinfo[1];	/* variable length array of tsinfo */
};

/* structure for addts/delts arguments */
typedef struct tspec_arg {
	uint16 version;			/* see definition of TSPEC_ARG_VERSION */
	uint16 length;			/* length of entire structure */
	uint flag;			/* bit field */
	/* TSPEC Arguments */
	struct tsinfo_arg tsinfo;	/* TS Info bit field */
	uint16 nom_msdu_size;		/* (Nominal or fixed) MSDU Size (bytes) */
	uint16 max_msdu_size;		/* Maximum MSDU Size (bytes) */
	uint min_srv_interval;		/* Minimum Service Interval (us) */
	uint max_srv_interval;		/* Maximum Service Interval (us) */
	uint inactivity_interval;	/* Inactivity Interval (us) */
	uint suspension_interval;	/* Suspension Interval (us) */
	uint srv_start_time;		/* Service Start Time (us) */
	uint min_data_rate;		/* Minimum Data Rate (bps) */
	uint mean_data_rate;		/* Mean Data Rate (bps) */
	uint peak_data_rate;		/* Peak Data Rate (bps) */
	uint max_burst_size;		/* Maximum Burst Size (bytes) */
	uint delay_bound;		/* Delay Bound (us) */
	uint min_phy_rate;		/* Minimum PHY Rate (bps) */
	uint16 surplus_bw;		/* Surplus Bandwidth Allowance Factor */
	uint16 medium_time;		/* Medium Time (32 us/s periods) */
} tspec_arg_t;

/* current version of wl_tspec_arg_t struct */
#define	TSPEC_ARG_VERSION	1	/* current version of wl_tspec_arg_t struct */
#define TSPEC_ARG_LENGTH	55	/* argment length from tsinfo to medium_time */

/* define for flag */
#define TSPEC_PENDING		0	/* TSPEC pending */
#define TSPEC_ACCEPTED		1	/* TSPEC accepted */
#define TSPEC_REJECTED		2	/* TSPEC rejected */
#define TSPEC_UNKNOWN		3	/* TSPEC unknown */
#define TSPEC_STATUS_MASK	7	/* TSPEC status mask */


/* Software feature flag defines used by wlfeatureflag */
#define WL_SWFL_ABBFL		0x0001 /* Allow Afterburner on systems w/o hardware BFL */
#define WL_SWFL_ABENCORE	0x0002 /* Allow AB on non-4318E chips */
#define WL_SWFL_NOHWRADIO	0x0004 /* Disable HW Radio monitor (e.g., Apple) */

#define WL_LIFETIME_MAX 0xFFFF /* Max value in ms */

/* Packet lifetime configuration per ac */
typedef struct wl_lifetime {
	uint32 ac;	        /* access class */
	uint32 lifetime;    /* Packet lifetime value in ms */
} wl_lifetime_t;


/* Channel Switch Announcement param */
typedef struct wl_chan_switch {
	uint8 mode;		/* value 0 or 1 */
	uint8 count;		/* count # of beacons before switching */
	chanspec_t chspec;	/* chanspec */
	uint8 reg;		/* regulatory class */
} wl_chan_switch_t;

/* Roaming trigger definitions for WLC_SET_ROAM_TRIGGER.
 *
 * (-100 < value < 0)   value is used directly as a roaming trigger in dBm
 * (0 <= value) value specifies a logical roaming trigger level from
 *                      the list below
 *
 * WLC_GET_ROAM_TRIGGER always returns roaming trigger value in dBm, never
 * the logical roam trigger value.
 */
#define WLC_ROAM_TRIGGER_DEFAULT	0 /* default roaming trigger */
#define WLC_ROAM_TRIGGER_BANDWIDTH	1 /* optimize for bandwidth roaming trigger */
#define WLC_ROAM_TRIGGER_DISTANCE	2 /* optimize for distance roaming trigger */
#define WLC_ROAM_TRIGGER_MAX_VALUE	2 /* max. valid value */

/* Preferred Network Offload (PNO, formerly PFN) defines */
enum {
	PFN_LIST_ORDER,
	PFN_RSSI
};

enum {
	DISABLE,
	ENABLE
};

#define SORT_CRITERIA_BIT		0
#define AUTO_NET_SWITCH_BIT		1
#define ENABLE_BKGRD_SCAN_BIT		2

#define SORT_CRITERIA_MASK		0x01
#define AUTO_NET_SWITCH_MASK		0x02
#define ENABLE_BKGRD_SCAN_MASK		0x04

#define PFN_VERSION			1

/* PFN data structure */
typedef struct wl_pfn_param {
	int32 version;					/* version field */
	int32 scan_freq;				/* Scan frequency */
	int32 lost_network_timeout;			/* Timeout in sec. to declare
							 * discovered network as lost
							 */
	int16 flags;					/* Bit field to control feature
							 * of PFN such as sort criteria auto
							 * enable switch and back ground scan
							 */
#if defined(WLPFN_AUTO_CONNECT) || !defined(BCMDRIVER)
	int16 rssi_margin;				/* Margin to avoid jitter for choosing a
							 * PFN based on RSSI sort criteria
							 */
#else
	int16 pad;
#endif
} wl_pfn_param_t;

typedef struct wl_pfn {
	wlc_ssid_t		ssid;			/* ssid name and its length */
	int32			bss_type;		/* IBSS or infrastructure */
#if defined(WLPFN_AUTO_CONNECT) || !defined(BCMDRIVER)
	int32			infra;			/* BSS Vs IBSS */
	int32			auth;			/* Open Vs Closed */
	int32			wpa_auth;		/* WPA type */
	int32			wsec;			/* wsec value */
	union {
		wl_wsec_key_t	sec_key;		/* Security Settings for WEP */
		wsec_pmk_t	wpa_sec_key;		/* Security setting for WPA */
	} pfn_security;
#endif
} wl_pfn_t;

/* TCP Checksum Offload defines */
#define TOE_TX_CSUM_OL		0x00000001
#define TOE_RX_CSUM_OL		0x00000002

/* TCP Checksum Offload error injection for testing (BCMINTERNAL) */
#define TOE_ERRTEST_TX_CSUM	0x00000001
#define TOE_ERRTEST_RX_CSUM	0x00000002
#define TOE_ERRTEST_RX_CSUM2	0x00000004

struct toe_ol_stats_t {
	/* Num of tx packets that don't need to be checksummed */
	uint32 tx_summed;

	/* Num of tx packets where checksum is filled by offload engine */
	uint32 tx_iph_fill;
	uint32 tx_tcp_fill;
	uint32 tx_udp_fill;
	uint32 tx_icmp_fill;

	/*  Num of rx packets where toe finds out if checksum is good or bad */
	uint32 rx_iph_good;
	uint32 rx_iph_bad;
	uint32 rx_tcp_good;
	uint32 rx_tcp_bad;
	uint32 rx_udp_good;
	uint32 rx_udp_bad;
	uint32 rx_icmp_good;
	uint32 rx_icmp_bad;

	/* Num of tx packets in which csum error is injected */
	uint32 tx_tcp_errinj;
	uint32 tx_udp_errinj;
	uint32 tx_icmp_errinj;

	/* Num of rx packets in which csum error is injected */
	uint32 rx_tcp_errinj;
	uint32 rx_udp_errinj;
	uint32 rx_icmp_errinj;
};

/* ARP Offload feature flags for arp_ol iovar */
#define ARP_OL_AGENT		0x00000001
#define ARP_OL_SNOOP		0x00000002
#define ARP_OL_HOST_AUTO_REPLY	0x00000004
#define ARP_OL_PEER_AUTO_REPLY	0x00000008

/* ARP Offload error injection (BCMINTERNAL) */
#define ARP_ERRTEST_REPLY_PEER	0x1
#define ARP_ERRTEST_REPLY_HOST	0x2

#define ARP_MULTIHOMING_MAX	8	/* Maximum local host IP addresses */

/* Arp offload statistic counts */
struct arp_ol_stats_t {
	uint32  host_ip_entries;	/* Host IP table addresses (more than one if multihomed) */
	uint32  host_ip_overflow;	/* Host IP table additions skipped due to overflow */

	uint32  arp_table_entries;	/* ARP table entries */
	uint32  arp_table_overflow;	/* ARP table additions skipped due to overflow */

	uint32  host_request;		/* ARP requests from host */
	uint32  host_reply;		/* ARP replies from host */
	uint32  host_service;		/* ARP requests from host serviced by ARP Agent */

	uint32  peer_request;		/* ARP requests received from network */
	uint32  peer_request_drop;	/* ARP requests from network that were dropped */
	uint32  peer_reply;		/* ARP replies received from network */
	uint32  peer_reply_drop;	/* ARP replies from network that were dropped */
	uint32  peer_service;		/* ARP request from host serviced by ARP Agent */
};

/* Packet engine interface */
#define WL_PKTENG_PER_TX_START	0x01
#define WL_PKTENG_PER_TX_STOP	0x02
#define WL_PKTENG_PER_RX_START	0x04
#define WL_PKTENG_PER_RX_STOP	0x08
#define WL_PKTENG_CONT_TX_START	0x10
#define WL_PKTENG_CONT_TX_STOP	0x20

typedef struct wl_pkteng {
	uint32 flags;
	uint32 delay;		/* Inter packet delay */
	uint32 nframes;		/* number of frames */
	uint32 length;		/* packet length */
	bool seqno;			/* enable/disable sequence no. */
	struct ether_addr dest;	/* destination address */
} wl_pkteng_t;

#define WL_WOWL_MAGIC	(1 << 0)	/* Wakeup on Magic packet */
#define WL_WOWL_NET	(1 << 1)	/* Wakeup on Netpattern */
#define WL_WOWL_DIS	(1 << 2)	/* Wakeup on loss-of-link due to Disassoc/Deauth */
#define WL_WOWL_RETR	(1 << 3)	/* Wakeup on retrograde TSF */
#define WL_WOWL_BCN	(1 << 4)	/* Wakeup on loss of beacon */

typedef struct {
	uint masksize;		/* Size of the mask in #of bytes */
	uint offset;		/* Offset to start looking for the packet in # of bytes */
	uint patternoffset;	/* Offset of start of pattern in the structure */
	uint patternsize;	/* Size of the pattern itself in #of bytes */
	/* Mask follows the structure above */
	/* Pattern follows the mask is at 'patternoffset' from the start */
} wl_wowl_pattern_t;

typedef struct {
	uint			count;
	wl_wowl_pattern_t	pattern[1];
} wl_wowl_pattern_list_t;

#endif /* _wlioctl_h_ */
