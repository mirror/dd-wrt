/*
 * Common (OS-independent) definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2005-2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#ifndef _wlc_pub_h_
#define _wlc_pub_h_

#define WLC_MAX_2G_CHANNEL	14	/* Max channe in 2G band */
#define	WLC_NUMRATES		16	/* max # of rates in a rateset */
#define	MAX_TIMERS		15	/* max # wl timers */
#define	MAXMULTILIST		32	/* max # multicast addresses */
#define	D11_PHY_HDR_LEN		6	/* Phy header length - 6 bytes */

/* phy types */
#define	PHY_TYPE_A              0	/* Phy type A */
#define	PHY_TYPE_B              1	/* Phy type B */
#define	PHY_TYPE_G              2	/* Phy type G */

#define WLC_PHYTYPE(_x) (_x) /* macro to perform WLC PHY -> D11 PHY TYPE, currently 1:1 */

#define	IS4303(pub)	((((wlc_pub_t *)pub)->sbh->chip == BCM4306_CHIP_ID) && \
			 (((wlc_pub_t *)pub)->sbh->chippkg == BCM4303_PKG_ID))


#define	WLC_RSSI_MINVAL		-200	/* Low value, e.g. for forcing roam */
#define	WLC_RSSI_NO_SIGNAL	-91	/* NDIS RSSI link quality cutoffs */
#define	WLC_RSSI_VERY_LOW	-80	/* Very low quality cutoffs */
#define	WLC_RSSI_LOW		-70	/* Low quality cutoffs */
#define	WLC_RSSI_GOOD		-68	/* Good quality cutoffs */
#define	WLC_RSSI_VERY_GOOD	-58	/* Very good quality cutoffs */
#define	WLC_RSSI_EXCELLENT	-57	/* Excellent quality cutoffs */

#define MA_WINDOW_SZ		8	/* moving average window size */

/* a large TX Power as an init value to factor out of MIN() calculations,
 * keep low enough to fit in an int8, units are .25 dBm
 */
#define WLC_TXPWR_MAX		(127)	/* ~32 dBm = 1,500 mW */

#define	INVCHANNEL		255	/* invalid channel */

/* Rx Antenna diversity control values */
#define	ANTDIV_FORCE_0		0	/* Use antenna 0 */
#define	ANTDIV_FORCE_1		1	/* Use antenna 1 */
#define	ANTDIV_START_1		2	/* Choose starting with 1 */
#define	ANTDIV_START_0		3	/* Choose starting with 0 */
#define	ANTDIV_ENABLE		3	/* APHY bbConfig Enable RX Diversity */

/* Tx Antenna control values */
#define TXANT_0			0	/* Tx on antenna 1, "Main" */
#define TXANT_1			1	/* Tx on antenna 1, "Aux" */
#define TXANT_LAST_RX		3	/* Tx on phy's last good Rx antenna */
#define TXANT_DEF		3	/* driver's default tx antenna setting */

/* 32-bit version of d11.h's macstat_t */
typedef struct macstat32 {
	uint32	txallfrm;
	uint32	txrtsfrm;
	uint32	txctsfrm;
	uint32	txackfrm;
	uint32	txdnlfrm;
	uint32	txbcnfrm;
	uint32	txfunfl[8];
	uint32	txtplunfl;
	uint32	txphyerr;
	uint32	rxfrmtoolong;
	uint32	rxfrmtooshrt;
	uint32	rxinvmachdr;
	uint32	rxbadfcs;
	uint32	rxbadplcp;
	uint32	rxcrsglitch;
	uint32	rxstrt;
	uint32	rxdfrmucastmbss;
	uint32	rxmfrmucastmbss;
	uint32	rxcfrmucast;
	uint32	rxrtsucast;
	uint32	rxctsucast;
	uint32	rxackucast;
	uint32	rxdfrmocast;
	uint32	rxmfrmocast;
	uint32	rxcfrmocast;
	uint32	rxrtsocast;
	uint32	rxctsocast;
	uint32	rxdfrmmcast;
	uint32	rxmfrmmcast;
	uint32	rxcfrmmcast;
	uint32	rxbeaconmbss;
	uint32	rxdfrmucastobss;
	uint32	rxbeaconobss;
	uint32	rxrsptmout;
	uint32	bcntxcancl;
	uint32	rxf0ovfl;
	uint32	rxf1ovfl;
	uint32	rxf2ovfl;
	uint32	txsfovfl;
	uint32	pmqovfl;
	uint32	rxcgprqfrm;
	uint32	rxcgprsqovfl;
	uint32	txcgprsfail;
	uint32	txcgprssuc;
	uint32	prs_timeout;
	uint32	rxnack;
	uint32	frmscons;
	uint32	txnack;
	uint32	txglitch_nack;
	uint32	txburst;
	uint32	rxburst;
} macstat32_t;

typedef struct rateset {
	uint	count;			/* # rates in this set */
	uint8	rates[WLC_NUMRATES];	/* rates in 500kbps units w/hi bit set if basic */
	uint8	mcs[16];	/* supported mcs index bit map */
} rateset_t;

struct rsn_parms {
	uint8 flags;		/* misc booleans (e.g., supported) */
	uint8 multicast;	/* multicast cipher */
	uint8 ucount;		/* count of unicast ciphers */
	uint8 unicast[4];	/* unicast ciphers */
	uint8 acount;		/* count of auth modes */
	uint8 auth[4];		/* Authentication modes */
};

/*
 * buffer length needed for wlc_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL.
 */
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)


#define RSN_FLAGS_SUPPORTED		0x1 /* Flag for rsn_params */
#ifdef BCMWPA2
#define RSN_FLAGS_PREAUTH		0x2 /* Flag for WPA2 rsn_params */
#endif /* BCMWPA2 */

/* wlc internal bss_info, wl external one is in wlioctl.h */
typedef struct wlc_bss_info
{
	struct ether_addr BSSID;	/* network BSSID */
	uint8		flags;		/* flags for internal attibutes */
	uint8		SSID_len;	/* the length of SSID */
	uint8		SSID[32];	/* SSID string */
	int16		RSSI;		/* receive signal strength (in dBm) */
	uint16		beacon_period;	/* units are Kusec */
	uint16		atim_window;	/* units are Kusec */
	uint8		channel;	/* Channel no. */
	int8		infra;		/* 0=IBSS, 1=infrastructure, 2=unknown */
	rateset_t	rateset;	/* supported rates */
	uint8		dtim_period;	/* DTIM period */
	int8		phy_noise;	/* noise right after tx (in dBm) */
	uint16		capability;	/* Capability information */
	struct dot11_bcn_prb *bcn_prb;	/* beacon/probe response frame (ioctl na) */
	uint16		bcn_prb_len;	/* beacon/probe response frame length (ioctl na) */
	uint8		wme_qosinfo;	/* QoS Info from WME IE; valid if WLC_BSS_WME flag set */
	struct rsn_parms wpa;
#ifdef BCMWPA2
	struct rsn_parms wpa2;
#endif /* BCMWPA2 */
	uint16		qbss_load_aac;	/* qbss load available admission capacity */
	/* qbss_load_chan_free <- (0xff - channel_utilization of qbss_load_ie_t) */
	uint8		qbss_load_chan_free;	/* indicates how free the channel is */
	uint8		mcipher;	/* multicast cipher */
	uint8		wpacfg;		/* wpa config index */
} wlc_bss_info_t;

/* forward declarations */
struct wlc_if;

/* wlc_ioctl error codes */
#define WLC_ENOIOCTL	1 /* No such Ioctl */
#define WLC_EINVAL	2 /* Invalid value */
#define WLC_ETOOSMALL	3 /* Value too small */
#define WLC_ETOOBIG	4 /* Value too big */
#define WLC_ERANGE	5 /* Out of range */
#define WLC_EDOWN	6 /* Down */
#define WLC_EUP		7 /* Up */
#define WLC_ENOMEM	8 /* No Memory */
#define WLC_EBUSY	9 /* Busy */

/* Radar detect scratchpad area */
#define RDR_NTIERS  3	   /* Number of tiers */
#define RDR_TIER_SIZE 64   /* Size per tier */
#define RDR_LIST_SIZE 256  /* Size of the list */
typedef struct {
	int length;
	int tstart_list[RDR_LIST_SIZE];
	int width_list[RDR_LIST_SIZE];
	int epoch_start[40];
	int epoch_finish[40];
	int tiern_list[RDR_NTIERS][RDR_TIER_SIZE];
	int nepochs;
} radar_work_t;

/* IOVar structure */
typedef struct wlc_iovar {
	const char *name;	/* name for lookup and display */
	uint16 varid;		/* id (wlc_varid_t) for switch */
	uint16 flags;		/* operational flag bits */
	uint16 type;		/* base type of argument */
	uint16 minlen;		/* min length for buffer vars */
} wlc_iovar_t;

/* Flags for common error checks */
#define IOVF_WHL	(1<<4)	/* value must be whole (0-max) */
#define IOVF_NTRL	(1<<5)	/* value must be natural (1-max) */

#define IOVF_SET_UP	(1<<6)	/* set requires driver be up */
#define IOVF_SET_DOWN	(1<<7)	/* set requires driver be down */
#define IOVF_SET_CLK	(1<<8)	/* set requires core clock */
#define IOVF_SET_BAND	(1<<9)	/* set requires fixed band */

#define IOVF_GET_CLK	(1<<10)	/* get requires core clock */
#define IOVF_GET_BAND	(1<<11)	/* get requires fixed band */

/* Base type definitions */
#define IOVT_VOID	0	/* no value (implictly set only) */
#define IOVT_BOOL	1	/* any value ok (zero/nonzero) */
#define IOVT_INT8	2	/* integer values are range-checked */
#define IOVT_UINT8	3	/* unsigned int 8 bits */
#define IOVT_INT16	4	/* int 16 bits */
#define IOVT_UINT16	5	/* unsigned int 16 bits */
#define IOVT_INT32	6	/* int 32 bits */
#define IOVT_UINT32	7	/* unsigned int 32 bits */
#define IOVT_BUFFER	8	/* buffer is size-checked as per minlen */

/* Actions for wlc_iovar_op() */
#define IOV_GET 0 /* Get an iovar */
#define IOV_SET 1 /* Set an iovar */

/* Varid to actionid mapping */
#define IOV_GVAL(id)		((id)*2)
#define IOV_SVAL(id)		(((id)*2)+1)
#define IOV_ISSET(actionid)	((actionid & IOV_SET) == IOV_SET)

/* watchdog and down callback function proto's */
typedef int (*watchdog_fn_t)(void *handle);
typedef int (*down_fn_t)(void *handle);

/* IOVar handler
 *
 * handle - a pointer value registered with the function
 * vi - iovar_info that was looked up
 * actionid - action ID, calculated by IOV_GVAL() and IOV_SVAL() based on varid.
 * name - the actual iovar name
 * params/plen - parameters and length for a get, input only.
 * arg/len - buffer and length for value to be set or retrieved, input or output.
 * vsize - value size, valid for integer type only.
 * wlcif - interface context (wlc_if pointer)
 *
 * All pointers may point into the same buffer.
 */
typedef int (*iovar_fn_t)(void *handle, const wlc_iovar_t *vi, uint32 actionid,
	const char *name, void *params, uint plen, void *arg, int alen,
	int vsize, struct wlc_if *wlcif);

/*
 * Public portion of "common" os-independent state structure.
 * The wlc handle points at this.
 */
typedef struct wlc_pub {
	uint		unit;			/* device instance number */
	uint		corerev;		/* core revision */
	sb_t		*sbh;			/* SB handle (cookie for sbutils calls) */
	char		*vars;			/* "environment" name=value */

	bool		promisc;		/* promiscuous destination address */
	bool		up;			/* interface up and running */
	int		_wme;			/* WME QoS mode */
	bool            _mssid;             /* true if multiple ssid configuration can be enabled */
	bool		allmulti;		/* enable all multicasts */
	bool		led_blink_run;		/* blink timer is running */
	bool		BSS;			/* infrastructure or ad hoc */
	bool		associated;		/* true:part of [I]BSS, false: not */
						/* (union of sta_associated, aps_associated) */
	bool		phytest_on;		/* whether a PHY test is running */
	bool		bf_preempt;		/* True to enable 'darwin' mode */
	bool		txqstopped;		/* tx flow control on */
	osl_t		*osh;			/* pointer to os handle */

	struct ether_addr	cur_etheraddr;	/* our local ethernet address */
	struct ether_addr	multicast[MAXMULTILIST]; /* multicast addresses */
	uint		nmulticast;		/* # enabled multicast addresses */
#ifdef BCMWPA2
	pmkid_cand_t	pmkid_cand[MAXPMKID];	/* PMKID candidate list */
	uint		npmkid_cand;	/* num PMKID candidates */
	pmkid_t		pmkid[MAXPMKID];	/* PMKID cache */
	uint		npmkid;			/* num cached PMKIDs */
#endif /* BCMWPA2 */

	uint		_nbands;		/* # bands supported */
	struct wl_timer *led_blink_timer;	/* 10ms led blink timer */
	wlc_bss_info_t	current_bss;		/* STA BSS if active, else first AP BSS */
	uint8		boardrev;		/* version # of particular board */
	uint8		sromrev;		/* version # of the srom */
	uint32		boardflags;		/* Board specific flags from srom */

	uint32		wlfeatureflag;		/* Flags to control sw features from registry */

	int		psq_pkts_total;		/* total num of ps pkts */

	uint32 		radar;			/* radar info: just on or off for now */

#ifdef WLCNT
	wl_cnt_t	_cnt;			/* monolithic counters struct */
	wl_wme_cnt_t	_wme_cnt;		/* Counters for WMM */
#endif /* WLCNT */
	uint16		txmaxpkts;		/* max number of large pkts allowed to be pending */

	uint8		txpwr_percent;		/* power output percentage */

	/* Regulatory power limits */
	uint8		txpwr_reg_max[MAXCHANNEL];	/* regulatory max txpwr in .25 dBm */
	uint8		txpwr_reg_ofdm_max[WLC_MAX_2G_CHANNEL + 1]; /* regulatory  max ofdm txpwr */
	int8		txpwr_local_max;	/*  regulatory local txpwr max */
	uint8		txpwr_local_constraint;	/* local power contraint in dB */

	/* s/w decryption counters */

	int 		bcmerror;		/* last bcm error */
	uint		now;			/* # elapsed seconds */
	mbool		radio_disabled;		/* bit vector for radio disabled reasons */
	bool		wakeforphyreg;		/* force wake for phyreg access */
	int 		antdiv_override;
} wlc_pub_t;

/* wl_monitor rx status per packet */
typedef struct	wl_rxsts{
	uint	pkterror;		/* error flags per pkt */
	uint	phytype;		/* 802.11 A/B/G ... */
	uint	channel;		/* channel */
	uint	datarate;		/* rate in 500kbps */
	uint	antenna;		/* antenna pkts recieved on */
	uint	pktlength;		/* pkt length minus bcm phy hdr */
	uint32	mactime;		/* time stamp from mac, count per 1us */
	uint	sq;			/* signal quality */
	int32	signal;			/* in dbm */
	int32	noise;			/* in dbm */
	uint	preamble;		/* Unknown, short, long */
	uint	encoding;		/* Unknown, CCK, PBCC, OFDM */
}wl_rxsts_t;

/* status per error RX pkt */
#define WL_RXS_CRC_ERROR		0x00000001 /* CRC Error in packet */
#define WL_RXS_RUNT_ERROR		0x00000002 /* Runt packet */
#define WL_RXS_ALIGN_ERROR		0x00000004 /* Misaligned packet */
#define WL_RXS_OVERSIZE_ERROR		0x00000008 /* packet bigger than RX_LENGTH (usually 1518) */
#define WL_RXS_WEP_ICV_ERROR		0x00000010 /* Integrity Check Value error */
#define WL_RXS_WEP_ENCRYPTED		0x00000020 /* Encrypted with WEP */
#define WL_RXS_PLCP_SHORT		0x00000040 /* Short PLCP error */
#define WL_RXS_DECRYPT_ERR		0x00000080 /* Decryption error */
#define WL_RXS_OTHER_ERR		0x80000000 /* Other errors */

/* phy type */
#define WL_RXS_PHY_A			0x00000000 /* A phy type */
#define WL_RXS_PHY_B			0x00000001 /* B phy type */
#define WL_RXS_PHY_G			0x00000002 /* G phy type */

/* encoding */
#define WL_RXS_ENCODING_CCK		0x00000000  /* CCK encoding */
#define WL_RXS_ENCODING_OFDM		0x00000001  /* OFDM encoding */

/* preamble */
#define WL_RXS_PREAMBLE_SHORT		0x00000000  /* Short preamble */
#define WL_RXS_PREAMBLE_LONG		0x00000001  /* Long preamble */


/* forward declare and use the struct notation so we don't have to
 * have it defined if not necessary.
 */
struct wlc_info;
struct wlc_if;

/* Structure for Pkttag area in a packet.
 * CAUTION: Pls carefully consider your design before adding any new fields to the pkttag
 * The size is limited to 32 bytes which on 64-bit machine allows only 4 fields
 * If adding a member, be sure to check if WLPKTTAG_INFO_MOVE should transfer it.
 */
typedef struct {
	uint32		flags;		/* Describe various packet properties */
	uint8		callbackidx;	/* Index into pkt_callback tables for callback function */
	uint32		exptime;	/* Time of expiry for the packet */
	struct scb*	_scb;		/* Pointer to SCB for associated ea */
} wlc_pkttag_t;

#define WLPKTTAG(p) ((wlc_pkttag_t*)PKTTAG(p))

/* Flags used in wlc_pkttag_t.
 * If adding a flag, be sure to check if WLPKTTAG_INFO_MOVE should transfer it.
 */
#define WLF_PSMARK		0x00000001	/* PKT marking for PSQ ageing */
#define WLF_PSDONTQ		0x00000002	/* PS-Poll response don't queue flag */
#define WLF_MPDU		0x00000004	/* Set if pkt is a PDU as opposed to MSDU */
#define WLF_NON8023		0x00000008	/* original pkt is not 8023 */
#define WLF_8021X		0x00000010	/* original pkt is not 8023 */
#define WLF_EXPTIME		0x00000200	/* pkttag has a valid expiration time for the pkt */

/* Move callback functions from a packet to another
 * CAUTION: This is destructive operation for pkt_from
 */
#define WLPKTTAG_INFO_MOVE(pkt_from, pkt_to) \
	do { \
		/* Make sure not moving to same packet! */ \
		ASSERT(pkt_from != pkt_to); \
		WLPKTTAG(pkt_to)->callbackidx = WLPKTTAG(pkt_from)->callbackidx; \
		WLPKTTAG(pkt_from)->callbackidx = 0; \
	} while (0)

#define WLPKTTAGSCB(p) (WLPKTTAG(p)->_scb)

#define	WLC_PREC_COUNT		16 /* Max precedence level implemented */

/* pri is PKTPRIO encoded in the packet. This maps the Packet priority to
 * enqueue precedence as defined in wlc_prec_map
 */
extern const uint8 wlc_prio2prec_map[];
#define WLC_PRIO_TO_PREC(pri)       wlc_prio2prec_map[(pri) & 7]

/* This maps priority to one precedence higher - Used by PS-Poll response packets to
 * simulate enqueue-at-head operation, but still maintain the order on the queue
 */
#define WLC_PRIO_TO_HI_PREC(pri)    MIN(WLC_PRIO_TO_PREC(pri) + 1, WLC_PREC_COUNT - 1)

extern const uint8 wme_fifo2ac[];
#define WME_PRIO2AC(prio)	wme_fifo2ac[wme_prio2fifo[(prio)]]

/* Map AC bitmap to precedence bitmap */
extern uint wlc_acbitmap2precbitmap[16];
#define WLC_ACBITMAP_TO_PRECBITMAP(ab)	wlc_acbitmap2precbitmap[(ab) & 0xf]

/* Mask to describe all precedence levels */
#define WLC_PREC_BMP_ALL		MAXBITVAL(WLC_PREC_COUNT)

#ifdef WME
#define WME_AUTO(wlc) ((wlc)->pub._wme == AUTO)
#else
#define WME_AUTO(wlc) (0)
#endif

/* common functions for every port */
extern void * wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
	osl_t *osh, void *regsva, uint bustype, void *btparam, uint *perr);
extern uint wlc_detach(struct wlc_info *wlc);
extern bool wlc_chipmatch(uint16 vendor, uint16 device);
extern void wlc_init(struct wlc_info *wlc);
extern void wlc_reset(struct wlc_info *wlc);
extern void wlc_corereset(struct wlc_info *wlc);
extern void wlc_corereset_flags(struct wlc_info *wlc, uint32 flags);
extern int  wlc_up(struct wlc_info *wlc);
extern uint wlc_down(struct wlc_info *wlc);
extern bool wlc_sendpkt(struct wlc_info *wlc, void *sdu, struct wlc_if *wlcif);
extern bool wlc_isr(struct wlc_info *wlc, bool *wantdpc);
extern bool wlc_dpc(struct wlc_info *wlc, bool bounded);
/*
 * These are not interrupts on/off entry points, which are in every port.
 * These are workbee, called by port peers, not SMP safe
 */
extern void wlc_intrson(struct wlc_info *wlc);
extern uint32 wlc_intrsoff(struct wlc_info *wlc);
extern void wlc_intrsrestore(struct wlc_info *wlc, uint32 macintmask);
extern void wlc_intrsoff_isr(struct wlc_info *wlc);
extern bool wlc_intrsupd(struct wlc_info *wlc);

extern uint32 wlc_mhf(struct wlc_info *wlc, uint32 mask, uint32 val, bool allbands);
extern void wlc_write_shm(struct wlc_info *wlc, uint offset, uint16 v);
extern uint16 wlc_read_shm(struct wlc_info *wlc, uint offset);
extern void wlc_write_template_ram(struct wlc_info *wlc, int offset, int len, void *buf);
extern void wlc_update_txpwr_shmem(wlc_pub_t *pub);
extern uint wlc_ctrupd(struct wlc_info *wlc, uint ucode_offset, uint offset);
extern void wlc_rate_lookup_init(struct wlc_info *wlc, struct rateset *rateset);
extern void wlc_default_rateset(struct wlc_info *wlc, struct rateset *rs);
/* wlc_phy.c helper functions */
extern bool wlc_scaninprog(struct wlc_info *wlc);
extern bool wlc_rminprog(struct wlc_info *wlc);
extern void *wlc_cur_phy(struct wlc_info *wlc);
extern bool wlc_ofdm_restrict(struct wlc_info *wlc);
extern int wlc_cur_bandtype(struct wlc_info *wlc);
extern void wlc_set_ps_ctrl(struct wlc_info *wlc);
extern void wlc_mctrl(struct wlc_info *wlc, uint32 mask, uint32 val);

/* ioctl */
extern int wlc_set(struct wlc_info *wlc, int cmd, int arg);
extern int wlc_get(struct wlc_info *wlc, int cmd, int *arg);
extern int wlc_iovar_getint(struct wlc_info *wlc, const char *name, int *arg);
extern int wlc_iovar_setint(struct wlc_info *wlc, const char *name, int arg);
extern int wlc_iovar_getint8(struct wlc_info *wlc, const char *name, int8 *arg);
extern int wlc_iovar_getbool(struct wlc_info *wlc, const char *name, bool *arg);
extern int wlc_iovar_op(struct wlc_info *wlc, const char *name, void *params, int p_len, void *arg,
	int len, bool set, struct wlc_if *wlcif);
extern int wlc_iovar_check(wlc_pub_t *pub, const wlc_iovar_t *vi, void *arg, int len, bool set);
extern int wlc_ioctl(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);
extern int wlc_module_register(wlc_pub_t *pub, const wlc_iovar_t *iovars,
	char *name, void *hdl, iovar_fn_t iovar_fn,
	watchdog_fn_t watchdog_fn, down_fn_t down_fn);
extern int wlc_module_unregister(wlc_pub_t *pub, const char *name, void *hdl);
extern void wlc_suspend_mac_and_wait(struct wlc_info *wlc);
extern void wlc_enable_mac(struct wlc_info *wlc);
extern uint16 wlc_rate_shm_offset(struct wlc_info *wlc, uint8 rate);

static INLINE int wlc_iovar_getuint(struct wlc_info *wlc, const char *name, uint *arg)
{
	return wlc_iovar_getint(wlc, name, (int*)arg);
}

static INLINE int wlc_iovar_getuint8(struct wlc_info *wlc, const char *name, uint8 *arg)
{
	return wlc_iovar_getint8(wlc, name, (int8*)arg);
}

static INLINE int wlc_iovar_setuint(struct wlc_info *wlc, const char *name, uint arg)
{
	return wlc_iovar_setuint(wlc, name, (uint)arg);
}

/* ioctl helper */
extern uint wlc_freq2channel(uint freq);
extern uint wlc_channel2freq(uint channel);

extern int wlc_iocbandchk(struct wlc_info *wlc, int *arg, int len, int *bands, bool clkchk);

/* helper functions */
extern void wlc_statsupd(struct wlc_info *wlc);
extern void wlc_sendup_event(struct wlc_info *wlc, int bssid, const struct ether_addr *ea,
	const wlc_event_t *e, uint8 *data, uint32 len);

extern void wlc_getrand(struct wlc_info *wlc, uint8 *buf, int len);

struct scb;
void wlc_ps_on(struct wlc_info *wlc, struct scb *scb);
void wlc_ps_off(struct wlc_info *wlc, struct scb *scb, bool discard);
void wlc_event(struct wlc_info *wlc, wlc_event_t *e);
extern void wlc_mute(struct wlc_info *wlc, bool want);


#ifdef	BCMWPA2
extern void wlc_pmkid_build_cand_list(struct wlc_info *wlc, bool check_SSID);
extern void wlc_pmkid_event(struct wlc_info *wlc);
#endif /* BCMWPA2 */

extern void wlc_tinydump(struct wlc_info *wlc, char *buf);

#endif /* _wlc_pub_h_ */
