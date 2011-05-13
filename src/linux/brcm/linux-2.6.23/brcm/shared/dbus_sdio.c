/*
 * Dongle BUS interface
 * Common to all SDIO interface
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dbus_sdio.c,v 1.42.2.51 2011-02-03 23:47:01 Exp $
 */

#include <typedefs.h>
#include <osl.h>

#include <bcmsdh.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <siutils.h>
#include <hndpmu.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbhnddma.h>
#include <bcmsrom.h>

#include <sdio.h>
#include <spid.h>
#include <sbsdio.h>
#include <sbsdpcmdev.h>
#include <bcmsdpcm.h>
#include <sbpcmcia.h>

#include <proto/ethernet.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <sdiovar.h>
#include "dbus.h"

#define IDLE_IMMEDIATE	(-1)	/* Enter idle immediately (no timeout) */
/* Values for idleclock iovar: other values are the sd_divisor to use when idle */
#define IDLE_ACTIVE	0	/* Do not request any SD clock change when idle */
#define IDLE_STOP	(-1)	/* Request SD clock be stopped (and use SD1 mode) */

#define PRIOMASK	7
#define DBUS_SDIO_RX_MAX	20
#define DBUS_SDIO_TX_MAX	20

#define TXRETRIES	2	/* # of retries for tx frames */
#define REGRETRIES	2	/* # of retries for tx frames */

#define MEMBLOCK    2048 /* Block size used for downloading of dongle image */
#define MAX_DATA_BUF (32 * 1024)	/* which should be more than
						* and to hold biggest glom possible
						*/
#if !ISPOWEROF2(SDALIGN)
#error "SDALIGN is not a power of 2!"
#endif

/* Total length of frame header for dongle protocol */
#define SDPCM_HDRLEN	(SDPCM_FRAMETAG_LEN + SDPCM_SWHEADER_LEN)
#define SDPCM_RESERVE	(SDPCM_HDRLEN + SDALIGN)

/* Space for header read, limit for data packets */
#ifndef MAX_HDR_READ
#define MAX_HDR_READ	32
#endif
#define MAX_RX_DATASZ	2048

/* Maximum milliseconds to wait for F2 to come up */
#define DHD_WAIT_F2RDY	4000

/* Value for ChipClockCSR during initial setup */
#define DHD_INIT_CLKCTL1	(SBSDIO_FORCE_HW_CLKREQ_OFF | SBSDIO_ALP_AVAIL_REQ)
#define DHD_INIT_CLKCTL2	(SBSDIO_FORCE_HW_CLKREQ_OFF | SBSDIO_FORCE_ALP)

/* Flags for SDH calls */
#define F2SYNC	(SDIO_REQ_4BYTE | SDIO_REQ_FIXED)

typedef struct {
	bool pending;
	uint8 *buf;
	int len;
} sdctl_info_t;

typedef struct {
	dbus_pub_t *pub; /* Must be first */

	void *cbarg;
	dbus_intf_callbacks_t *cbs;
	dbus_intf_t *drvintf;
	void *sdos_info;
	int devid;
	int devready;
	bool halting;

	long rxtx_flag;

	/* FIX: Ported from dhd_info_t */
	uint maxctl;            /* Max size rxctl request from proto to bus */
	ulong rx_readahead_cnt; /* Number of packets where header read-ahead was used. */
	ulong tx_realloc;       /* Number of tx packets we had to realloc for headroom */
	uint32 tx_ctlerrs;
	uint32 tx_ctlpkts;
	uint32 rx_ctlerrs;
	uint32 rx_ctlpkts;
	bool up;                /* Driver up/down (to OS) */
	bool dongle_reset;  /* TRUE = DEVRESET put dongle into reset */
	uint8 wme_dp;   /* wme discard priority */

	sdctl_info_t rxctl_req;
	sdctl_info_t txctl_req;

	si_t		*sih;	/* Handle for SI calls */
	char		*vars;	/* Variables (from CIS and/or other) */
	uint		varsz;	/* Size of variables buffer */

	sdpcmd_regs_t	*regs;    /* Registers for SDIO core */
	uint		sdpcmrev; /* SDIO core revision */
	uint		armrev;	  /* CPU core revision */
	uint		ramrev;	  /* SOCRAM core revision */
	uint32		ramsize;  /* Size of RAM in SOCRAM (bytes) */
	uint32		orig_ramsize;  /* Size of RAM in SOCRAM (bytes) */

	uint32		bus;      /* gSPI or SDIO bus */
	uint32		hostintmask;  /* Copy of Host Interrupt Mask */

	uint		blocksize; /* Block size of SDIO transfers */
	uint		roundup; /* Max roundup limit */

	struct pktq	rxq;	/* Queue used for rx frames */

	struct pktq	txq;	/* Queue length used for flow-control */
	uint8		flowcontrol;	/* per prio flow control bitmask */
	uint8		tx_seq;	/* Transmit sequence number (next) */
	uint16		tx_max;	/* Maximum transmit sequence allowed */

	uint8		hdrbuf[MAX_HDR_READ + SDALIGN];
	uint8		*rxhdr; /* Header of current rx frame (in hdrbuf) */
	uint16		nextlen; /* Next Read Len from last header */
	uint8		rx_seq;	/* Receive sequence number (expected) */
	bool		rxskip;	/* Skip receive (awaiting NAK ACK) */

	void		*glomd;	/* Packet containing glomming descriptor */
	void		*glom;	/* Packet chain for glommed superframe */
	uint		glomerr; /* Glom packet read errors */

	uint8		*rxbuf; /* Buffer for receiving control packets */
	uint		rxblen;	/* Allocated length of rxbuf */
	uint8		*rxctl;	/* Aligned pointer into rxbuf */
	uint8		*databuf; /* Buffer for receiving big glom packet */
	uint8		*dataptr; /* Aligned pointer into databuf */
	uint		rxlen;	/* Length of valid data in buffer */

	uint8		sdpcm_ver; /* Bus protocol reported by dongle */

	bool		alp_only; /* Don't use HT clock (ALP only) */

	bool		poll;	/* Use polling */
	uint 		intrcount; /* Count of device interrupt callbacks */
	uint		lastintrs; /* Count as of last watchdog timer */
	uint		spurious; /* Count of spurious interrupts */
	uint		pollrate; /* Ticks between device polls */
	uint		polltick; /* Tick counter */
	uint		pollcnt; /* Count of active polls */

	uint		clkstate; /* State of sd and backplane clock(s) */
	bool		activity; /* Activity flag for clock down */
	int32		idletime; /* Control for activity timeout */
	uint32		idlecount; /* Activity timeout counter */
	int32		idleclock; /* How to set bus driver when idle */
	uint32		sd_divisor; /* Speed control to bus driver */
	uint32		sd_mode; /* Mode control to bus driver */
	uint32		sd_rxchain; /* If bcmsdh api accepts PKT chains */
	bool		use_rxchain; /* If dhd should use PKT chains */
	bool		sleeping; /* Is SDIO bus sleeping? */
	/* Field to decide if rx of control frames happen in rxbuf or lb-pool */
	bool		usebufpool;

	/* Some additional counters */
	uint	tx_sderrs;	/* Count of tx attempts with sd errors */
	uint	fcqueued;	/* Tx packets that got queued */
	uint	rxrtx;		/* Count of rtx requests (NAK to dongle) */
	uint	rx_toolong;	/* Receive frames too long to receive */
	uint	rxc_errors;	/* SDIO errors when reading control frames */
	uint	rx_hdrfail;	/* SDIO errors on header reads */
	uint	rx_badhdr;	/* Bad received headers (roosync?) */
	uint	rx_badseq;	/* Mismatched rx sequence number */
	uint	fc_rcvd;	/* Number of flow-control events received */
	uint	fc_xoff;	/* Number which turned on flow-control */
	uint	fc_xon;		/* Number which turned off flow-control */
	uint	rxglomfail;	/* Failed deglom attempts */
	uint	rxglomframes;	/* Number of glom frames (superframes) */
	uint	rxglompkts;	/* Number of packets from glom frames */
	uint	f2rxhdrs;	/* Number of header reads */
	uint	f2rxdata;	/* Number of frame data reads */
	uint	f2txdata;	/* Number of f2 frame writes */
	uint	f1regdata;	/* Number of f1 register accesses */

} sdio_info_t;

typedef struct {
	sdio_info_t *sdio_info;
	dbus_irb_tx_t *txirb;
} pkttag_t;

struct exec_parms {
union {
	struct {
		sdio_info_t *sdio_info;
		struct pktq *q;
		int prec_map;
		int *prec_out;
	} pdeq;

	struct {
		sdio_info_t *sdio_info;
		struct pktq *q;
		void *pkt;
		int prec;
	} penq;
};
};

#define INIT_FLAG(flag)		*flag = 0
#define SET_FLAG(flag)		InterlockedExchange((flag), 1)
#define CLEAR_FLAG(flag)	InterlockedExchange((flag), 0)
#define	CHECK_FLAG(flag)	InterlockedAnd((flag), 1)

/* clkstate */
#define CLK_NONE	0
#define CLK_SDONLY	1
#define CLK_PENDING	2	/* Not used yet */
#define CLK_AVAIL	3

#define DHD_NOPMU(dhd)	(FALSE)

#ifdef BCMDBG
static int qcount[NUMPRIO];
#endif /* BCMDBG */

/* overrride the RAM size if possible */
#define DONGLE_MIN_MEMSIZE (128 *1024)
int dhd_dongle_memsize = 0;


static bool dhd_alignctl = TRUE;

static bool sd1idle = TRUE;

static bool retrydata = FALSE;
#define RETRYCHAN(chan) (((chan) == SDPCM_EVENT_CHANNEL) || retrydata)

#ifdef BCMSPI
/* At a watermark around 8 the spid hits underflow error. */
static const uint watermark = 32;
#else
static const uint watermark = 8;
#endif /* BCMSPI */
#ifndef SD_FIRSTREAD
static const uint firstread = 32;
#else
static const uint firstread = SD_FIRSTREAD;
#endif

/* Initial idletime behavior (immediate, never, or ticks) */
#define DHD_IDLETIME_TICKS 1;
extern void *def_sdos_info;

/* SDIO Drive Strength */
extern uint dhd_sdiod_drive_strength;

/* Force even SD lengths (some host controllers mess up on odd bytes) */
#ifdef BCMSPI
static bool forcealign = FALSE;
#else
static bool forcealign = TRUE;
#endif /* !BCMSPI */

extern char mfgsromvars[VARS_MAX];
extern int defvarslen;

/*
 * Default is to bring up eth1 immediately.
 */
uint delay_eth = 0;

#define ALIGNMENT  4

#define PKTALIGN(osh, p, len, align) \
	do {                                                        \
		uintptr datalign;                                   \
								    \
		datalign = (uintptr)PKTDATA((osh), (p));            \
		datalign = ROUNDUP(datalign, (align)) - datalign;   \
		ASSERT(datalign < (align));                         \
		ASSERT(PKTLEN((osh), (p)) >= ((len) + datalign));   \
		if (datalign)                                       \
			PKTPULL((osh), (p), (uint)datalign);        \
		PKTSETLEN((osh), (p), (len));                       \
	} while (0)

/* Limit on rounding up frames */
static uint max_roundup = 512;

/* Try doing readahead */
static bool dhd_readahead = TRUE;

/* To check if there's window offered */
#define DATAOK(bus)	\
	(((uint8)((sdio_info->tx_max - sdio_info->tx_seq) % SDPCM_SEQUENCE_WRAP) != 0) && \
	(((uint8)((sdio_info->tx_max - sdio_info->tx_seq) & 0x80) % SDPCM_SEQUENCE_WRAP) == 0))

#ifdef BCMSPI

#define SD_BUSTYPE			SPI_BUS

/* check packet-available-interrupt in piggybacked dstatus */
#define PKT_AVAILABLE()	(bcmsdh_get_dstatus(sdio_info->sdos_info) & STATUS_F2_PKT_AVAILABLE)

#define HOSTINTMASK		(I_HMB_FC_CHANGE | I_HMB_HOST_INT)

#define GSPI_PR55150_BAILOUT									\
do {												\
	uint32 dstatussw = bcmsdh_get_dstatus((void *)sdio_info->sdos_info);	\
	uint32 dstatushw = bcmsdh_cfg_read_word(sdio_info->sdos_info,		\
			SDIO_FUNC_0, SPID_STATUS_REG, NULL);			\
	uint32 intstatuserr = 0;						\
	uint retries = 0;							\
										\
	dbus_sdio_reg_read(sdio_info, (uint32)&sdio_info->regs->intstatus,	\
		sizeof(sdio_info->regs->intstatus), &intstatuserr, REGRETRIES); \
	printf("dstatussw = 0x%x, dstatushw = 0x%x, intstatus = 0x%x\n",	\
	        dstatussw, dstatushw, intstatuserr); 				\
										\
	sdio_info->nextlen = 0;							\
	*finished = TRUE;							\
} while (0)

#else /* BCMSDIO */

#define SD_BUSTYPE			SDIO_BUS

#define HOSTINTMASK		(I_TOHOSTMAIL | I_CHIPACTIVE)

#define GSPI_PR55150_BAILOUT

#endif /* BCMSPI */

/* Debug */
#define DBUSINTR DBUSTRACE
#define DBUSTIMER DBUSTRACE
#define DBUSGLOM DBUSTRACE
#define DBUSDATA DBUSTRACE
#define DBUSCTL DBUSTRACE
#define DBUSGLOM_ON() 0

typedef struct {
	chipcregs_t	*ccregs;
	sdpcmd_regs_t	*sdregs;
	uint32		socram_size;
} chipinfo_t;

/*
 * FIX: Basic information needed to prep dongle for download.
 * The goal is to simplify probe setup before a valid
 * image has been downloaded.  Also, can we avoid si_attach() during
 * probe setup since it brings in a lot of unnecessary dependencies?
 */

/* 4325 and 4315 have the same address map */
static const chipinfo_t chipinfo_4325_15 = {
	(chipcregs_t *) 0x18000000,
	(sdpcmd_regs_t *) 0x18002000,
	(384 * 1024)
};

static const chipinfo_t chipinfo_4329 = {
	(chipcregs_t *) 0x18000000,
	(sdpcmd_regs_t *) 0x18011000,
	(288 * 1024)
};

static const chipinfo_t chipinfo_4336 = {
	(chipcregs_t *) 0x18000000,
	(sdpcmd_regs_t *) 0x18011000,
	(240 * 1024)
};

static const chipinfo_t chipinfo_43237 = {
	(chipcregs_t *) 0x18000000,
	(sdpcmd_regs_t *) 0x18004000,
	(320 * 1024)
};

static const chipinfo_t chipinfo_4319 = {
	(chipcregs_t *) 0x18000000,
	(sdpcmd_regs_t *) 0x18002000,
	(288 * 1024)
};

/*
 * Local function prototypes
 */
static void *dbus_sdio_probe_cb(void *handle, const char *desc, uint32 bustype, uint32 hdrlen);
static int dbus_sdio_rxctl(sdio_info_t *sdio_info, uchar *msg, uint msglen);
static uint dbus_sdio_sendfromq(sdio_info_t *sdio_info);
static int dbus_sdio_txctl(sdio_info_t *sdio_info, uchar *msg, uint msglen);
static void dbus_sdio_txq_flush(sdio_info_t *sdio_info);

/*
 * NOTE: These functions can also be called before attach() occurs
 * so do not access sdio_info from them.  This is to support DBUS
 * async probe callback to upper layer such as DHD/BMAC/etc.  Another
 * alternative was to modify SDH to do async probe callback only
 * when a valid image is downloaded to the dongle.
 */
static int dbus_sdio_download_state(sdio_info_t *sdio_info, bool enter);
static int dbus_sdio_membytes(sdio_info_t *sdio_info, bool write,
	uint32 address, uint8 *data, uint size);
static int dbus_sdio_write_vars(sdio_info_t *sdio_info);
static int dbus_sdio_downloadvars(sdio_info_t *sdio_info, void *arg, int len);
#ifdef BCM_DNGL_EMBEDIMAGE
static int dbus_sdio_download_nvram_file(sdio_info_t *sdio_info);
static int dbus_sdio_download_image_array(sdio_info_t *sdio_info,
	uint8 *fw, int len);
#endif

/*
 * Wrappers to interface functions in dbus_sdio_os.c
 */
static void * dbus_sdio_exec_txlock(sdio_info_t *sdio_info, exec_cb_t cb, struct exec_parms *args);
static void * dbus_sdio_exec_rxlock(sdio_info_t *sdio_info, exec_cb_t cb, struct exec_parms *args);
extern int dbus_sdos_send_buf(void *sdos_info, uint32 addr, uint fn, uint flags,
	uint8 *buf, uint len);
extern int dbus_sdos_recv_buf(void *sdos_info, uint32 addr, uint fn, uint flags,
	uint8 *buf, uint len);
extern uint32 dbus_sdos_reg_write(void *sdos_info, uint32 addr, uint size, uint32 data);
extern uint32 dbus_sdos_reg_read(void *sdos_info, uint32 addr, uint size, uint32 *data);
extern int dbus_sdos_rwdata(void *sdos_info, uint rw, uint32 addr, uint8 *buf, uint len);
extern int dbus_sdos_get_sbaddr_window(void *sdos_info);
extern int dbus_sdos_set_sbaddr_window(void *sdos_info, uint32 address, bool force_set);
extern void dbus_sdos_cfg_write(void *sdos_info, uint func_num, uint32 addr, uint8 data, int *err);
extern uint8 dbus_sdos_cfg_read(void *sdos_info, uint func_num, uint32 addr, int *err);
extern int dbus_sdos_intr_enable(void *sdos_info);
extern int dbus_sdos_intr_disable(void *sdos_info);
extern int dbus_sdos_iovar_op(void *sdio_info, const char *name,
	void *params, int plen, void *arg, int len, bool set);
extern int dbus_sdos_cis_read(void *sdos_info, uint func_num, uint8 *cis, uint32 length);
extern int dbus_sdos_abort(void *sdos_info, uint fn);

/*
 * Wrappers to callback functions in dbus.c
 */
static void *dbus_sdio_pktget(sdio_info_t *sdio_info, uint len, bool send);
static void dbus_sdio_pktfree(sdio_info_t *sdio_info, void *p, bool send);
static dbus_irb_t *dbus_sdio_getirb(sdio_info_t *sdio_info, bool send);

/*
 * Callbacks common to all SDIO
 */
static void dbus_sdio_disconnect_cb(void *handle);
static void dbus_sdio_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb);
static void dbus_sdio_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status);
static void dbus_sdio_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status);
static void dbus_sdio_errhandler(void *handle, int err);
static void dbus_sdio_ctl_complete(void *handle, int type, int status);
static void dbus_sdio_state_change(void *handle, int state);
static bool dbus_sdio_dpc(void *handle, bool bounded);
static void dbus_sdio_watchdog(void *handle);

static dbus_intf_callbacks_t dbus_sdio_intf_cbs = {
	dbus_sdio_send_irb_timeout,
	dbus_sdio_send_irb_complete,
	dbus_sdio_recv_irb_complete,
	dbus_sdio_errhandler,
	dbus_sdio_ctl_complete,
	dbus_sdio_state_change,
	NULL,
	dbus_sdio_dpc,
	dbus_sdio_watchdog
};

/*
 * Need global for probe() and disconnect() since
 * attach() is not called at probe and detach()
 * can be called inside disconnect()
 */
static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

/* 
 * dbus_intf_t common to all SDIO
 * These functions override dbus_sdio_os.c.
 */
static void *dbus_sdif_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_sdif_detach(dbus_pub_t *pub, void *info);
static int dbus_sdif_send_irb(void *bus, dbus_irb_tx_t *txirb);
static int dbus_sdif_send_ctl(void *bus, uint8 *buf, int len);
static int dbus_sdif_recv_ctl(void *bus, uint8 *buf, int len);
static int dbus_sdif_up(void *bus);
static bool dbus_sdif_device_exists(void *bus);
static bool dbus_sdif_dlneeded(void *bus);
static int dbus_sdif_dlstart(void *bus, uint8 *fw, int len);
static int dbus_sdif_dlrun(void *bus);
static int dbus_sdif_down(void *bus);
static int dbus_sdif_get_attrib(void *bus, dbus_attrib_t *attrib);
static void dbus_bus_stop(sdio_info_t *sdio_info);

static dbus_intf_t dbus_sdio_intf;
static dbus_intf_t *g_dbusintf = NULL;

/* Functions shared between dbus_sdio.c/dbus_sdio_os.c */
extern int dbus_sdio_txq_process(void *bus);

static __inline
uint32 dbus_sdio_reg_read(sdio_info_t *sdio_info, uint32 addr, uint size,
	uint32 *data, uint32 retrycount)
{
	while (retrycount-- > 0) {
		if (dbus_sdos_reg_read(sdio_info->sdos_info, addr, size, data) == BCME_OK)
			return BCME_OK;
	}

	return BCME_ERROR;
}

static __inline
uint32 dbus_sdio_reg_write(sdio_info_t *sdio_info, uint32 addr, uint size,
	uint32 data, uint32 retrycount)
{
	while (retrycount-- > 0) {
		if (dbus_sdos_reg_write(sdio_info->sdos_info, addr, size, data) == BCME_OK)
			return BCME_OK;
	}

	return BCME_ERROR;
}

unsigned int
dbus_sdio_osl_rreg(void *ctx, void *reg, unsigned int size)
{
	unsigned int val;

	if (dbus_sdos_reg_read(ctx, (uint32)reg, size, &val) != BCME_OK)
		val = (uint32)(size == 2 ? (uint16)BCME_ERROR : BCME_ERROR);

	return val;
}

void
dbus_sdio_osl_wreg(void *ctx, void *reg, unsigned int val, unsigned int size)
{
	dbus_sdos_reg_write(ctx, (uint32)reg, size, val);
}

/* This callback is overloaded to also handle pre-attach() requests
 * such as downloading an image to the dongle.
 * Before attach(), we're limited to what can be done since
 * sdio_info handle is not available yet:
 * 	- Reading/writing registers
 * 	- Querying cores using si handle
 */
static void *
dbus_sdio_probe_cb(void *handle, const char *desc, uint32 bustype, uint32 hdrlen)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (probe_cb) {

		if (g_dbusintf != NULL) {

			/* First, initialize all lower-level functions as default
			 * so that dbus.c simply calls directly to dbus_sdio_os.c.
			 */
			bcopy(g_dbusintf, &dbus_sdio_intf, sizeof(dbus_intf_t));

			/* Second, selectively override functions we need.
			 */
			dbus_sdio_intf.attach = dbus_sdif_attach;
			dbus_sdio_intf.detach = dbus_sdif_detach;
			dbus_sdio_intf.send_irb = dbus_sdif_send_irb;
			/* SDIO does not need pre-submitted IRBs like USB
			 * so set recv_irb() to NULL so dbus.c would not call
			 * this function.
			 */
			dbus_sdio_intf.send_ctl = dbus_sdif_send_ctl;
			dbus_sdio_intf.recv_ctl = dbus_sdif_recv_ctl;
			dbus_sdio_intf.up = dbus_sdif_up;
			dbus_sdio_intf.device_exists = dbus_sdif_device_exists;
			dbus_sdio_intf.dlneeded = dbus_sdif_dlneeded;
			dbus_sdio_intf.dlstart = dbus_sdif_dlstart;
			dbus_sdio_intf.dlrun = dbus_sdif_dlrun;
			dbus_sdio_intf.down = dbus_sdif_down;
			dbus_sdio_intf.get_attrib = dbus_sdif_get_attrib;
		}

		/* Assume a valid image has been downloaded when
		 * the handle matches ours so propagate probe callback to upper
		 * layer
		 */
		if (probe_cb) {
			disc_arg = probe_cb(probe_arg, "DBUS SDIO", SD_BUSTYPE,
				SDPCM_RESERVE);
			return disc_arg;
		}
	}

	return NULL;
}

static void
dbus_sdio_disconnect_cb(void *handle)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (disconnect_cb)
		disconnect_cb(disc_arg);
}

int
dbus_bus_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	int err;

	probe_cb = prcb;
	disconnect_cb = discb;
	probe_arg = prarg;

	DBUSTRACE(("%s\n", __FUNCTION__));

	*intf = &dbus_sdio_intf;

	err = dbus_bus_osl_register(vid, pid, dbus_sdio_probe_cb,
		dbus_sdio_disconnect_cb, NULL, &g_dbusintf, param1, param2);

	ASSERT(g_dbusintf);

	return err;
}

int
dbus_bus_deregister()
{
	dbus_bus_osl_deregister();
	return DBUS_OK;
}

bool
dbus_sdio_probe_device(sdio_info_t *sdio_info)
{
	dbus_pub_t *pub = sdio_info->pub;
	const chipinfo_t *chipinfo = NULL;
	int err, i = 0;
	uint8 *cis, tup, tlen;

	sdio_info->devid = 0;

	if (!(cis = MALLOC(pub->osh, SBSDIO_CIS_SIZE_LIMIT))) {
		DBUSERR(("%s: CIS malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	bzero(cis, SBSDIO_CIS_SIZE_LIMIT);

	if ((err = dbus_sdos_cis_read(sdio_info->sdos_info,
		0, cis, SBSDIO_CIS_SIZE_LIMIT))) {
		DBUSERR(("%s: CIS read err %d\n", __FUNCTION__, err));
		MFREE(pub->osh, cis, SBSDIO_CIS_SIZE_LIMIT);
		return FALSE;
	}

	sdio_info->devid = 0xffff;

	i = 0;

	do {
		tup = cis[i++];
		if (tup == CISTPL_NULL || tup == CISTPL_END)
			tlen = 0;
		else
			tlen = cis[i++];
		if ((i + tlen) >= CIS_SIZE)
			break;

		if (tup == CISTPL_MANFID) {
			sdio_info->devid = (cis[i + 3] << 8) + cis[i + 2];
		} else if (tup == HNBU_HNBUCIS) {
			break;
		} else {
			i += tlen;
		}

	} while (tup != CISTPL_END);

	MFREE(pub->osh, cis, SBSDIO_CIS_SIZE_LIMIT);

	if (sdio_info->devid == 0) {
		if (dbus_sdos_reg_read(sdio_info->sdos_info,
			SI_ENUM_BASE, 4, &sdio_info->devid) == BCME_OK)
			sdio_info->devid &= CID_ID_MASK;
	}

	/* Check the Device ID and make sure it's one that we support */
	switch (sdio_info->devid) {
		case BCM4325_CHIP_ID:
		case BCM4325_D11DUAL_ID:		/* 4325 802.11a/g id */
		case BCM4325_D11G_ID:			/* 4325 802.11g 2.4Ghz band id */
		case BCM4325_D11A_ID:			/* 4325 802.11a 5Ghz band id */
			DBUSERR(("%s: found 4325 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_4325_15;
			break;
		case BCM4329_D11N_ID:		/* 4329 802.11n dualband device */
		case BCM4329_D11N2G_ID:		/* 4329 802.11n 2.4G device */
		case BCM4329_D11N5G_ID:		/* 4329 802.11n 5G device */
		case BCM4321_D11N2G_ID:
			DBUSERR(("%s: found 4329 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_4329;
			break;
		case BCM4315_CHIP_ID:
		case BCM4315_D11DUAL_ID:		/* 4315 802.11a/g id */
		case BCM4315_D11G_ID:			/* 4315 802.11g id */
		case BCM4315_D11A_ID:			/* 4315 802.11a id */
			DBUSINFO(("%s: found 4315 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_4325_15;
			break;
		case BCM4336_D11N_ID:
			DBUSINFO(("%s: found 4336 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_4336;
			break;
		case BCM4319_CHIP_ID:			/* BCM4319_D11DUAL_ID */
			DBUSINFO(("%s: found 4319 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_4319;
			break;
		case BCM43237_CHIP_ID:
			DBUSINFO(("%s: found 43237 Dongle\n", __FUNCTION__));
			chipinfo =  &chipinfo_43237;
			break;

		case 0:
			DBUSINFO(("%s: allow device id 0, will check chip internals\n",
			          __FUNCTION__));
			/* FIX: Need to query chip */
			chipinfo =  &chipinfo_4325_15;
			break;

		default:
			DBUSERR(("%s: skipping 0x%04x/0x%04x, not a dongle\n",
			           __FUNCTION__, pub->attrib.vid, pub->attrib.devid));
			return FALSE;
			break;
	}

	sdio_info->regs = chipinfo->sdregs;
	sdio_info->ramsize = chipinfo->socram_size;
	sdio_info->orig_ramsize = chipinfo->socram_size;

	return TRUE;
}

static int
dbus_sdio_device_init(sdio_info_t *sdio_info)
{
	int err;
	void *prarg;
	uint32 val;
#ifndef BCMSPI
	uint8 clkctl = 0;
#endif /* !BCMSPI */

	DBUSTRACE(("%s\n", __FUNCTION__));

	prarg = NULL;

#ifndef BCMSPI      /* wake-wlan in gSPI will bring up the htavail/alpavail clocks. */
	/* Force PLL off until si_attach() programs PLL control regs */
	dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
		SBSDIO_FUNC1_CHIPCLKCSR, DHD_INIT_CLKCTL1, &err);
	if (!err)
		clkctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
			SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, &err);

	if (err || ((clkctl & ~SBSDIO_AVBITS) != DHD_INIT_CLKCTL1)) {
		DBUSERR(("dbus_sdio_probe: ChipClkCSR access: err %d wrote 0x%02x read 0x%02x\n",
			err, DHD_INIT_CLKCTL1, clkctl));
		return DBUS_ERR;
	}
#endif /* !BCMSPI */

	/* The si_attach() will provide an SI handle, scan the 
	 * backplane, and initialize the PLL.
	 */
	if (!(sdio_info->sih = si_attach(sdio_info->devid, sdio_info->pub->osh,
		(void *)SI_ENUM_BASE, SD_BUSTYPE, sdio_info->sdos_info,
	                           &sdio_info->vars, &sdio_info->varsz))) {
		DBUSERR(("%s: si_attach failed!\n", __FUNCTION__));
		return DBUS_ERR;
	}

	ASSERT(sdio_info->orig_ramsize == si_socram_size(sdio_info->sih));

	/* FIX: this is needed on some boards for download.  If not, it can
	 * cause data errors if drive strength is not correct.
	 * Default is 10mA, but 6mA is optimal.
	 */
	si_sdiod_drive_strength_init(sdio_info->sih, sdio_info->pub->osh, dhd_sdiod_drive_strength);

	sdio_info->alp_only = TRUE;
	sdio_info->devready = FALSE;

	/* Set core control so an SDIO reset does a backplane reset */
	OR_REG(sdio_info->pub->osh, &sdio_info->regs->corecontrol, CC_BPRESEN);

	/* Set up the interrupt mask */
	W_REG(sdio_info->pub->osh, &sdio_info->regs->hostintmask, HOSTINTMASK);

	return DBUS_OK;
}

void
dbus_sdio_device_terminate(sdio_info_t *sdio_info)
{
	int err;
	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	if ((sdio_info == NULL) || (sdio_info->sih == NULL)) {
		DBUSERR(("%s: pinfo is NULL\n", __FUNCTION__));
		return;
	}

	if (sdio_info->sih) {
		si_detach(sdio_info->sih);
		sdio_info->sih = NULL;
	}
	if (sdio_info->vars && sdio_info->varsz) {
		MFREE(sdio_info->pub->osh, sdio_info->vars, sdio_info->varsz);
		sdio_info->vars = NULL;
		sdio_info->varsz = 0;
	}
}

void
dbus_sdif_detach(dbus_pub_t *pub, void *info)
{
	sdio_info_t *sdio_info = pub->bus;
	osl_t *osh = pub->osh;

	if (sdio_info == NULL) {
		DBUSERR(("%s: sdio_info is NULL\n", __FUNCTION__));
		return;
	}

	dbus_sdio_device_terminate(sdio_info);

	if (sdio_info->sdos_info) {
		if (sdio_info->drvintf && sdio_info->drvintf->detach)
			sdio_info->drvintf->detach(pub, sdio_info->sdos_info);
	}

	if (sdio_info->rxbuf) {
		MFREE(osh, sdio_info->rxbuf, sdio_info->rxblen);
		sdio_info->rxctl = sdio_info->rxbuf = NULL;
		sdio_info->rxlen = 0;
	}

	if (sdio_info->databuf) {
		MFREE(osh, sdio_info->databuf, MAX_DATA_BUF);
		sdio_info->databuf = NULL;
	}

	MFREE(osh, sdio_info, sizeof(sdio_info_t));
	pub->bus = NULL;
}

void *
dbus_sdif_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	int32 fnum;
	sdio_info_t *sdio_info;

	DBUSTRACE(("%s\n", __FUNCTION__));
	if ((g_dbusintf == NULL) || (g_dbusintf->attach == NULL))
		return NULL;

	/* Sanity check for BUS_INFO() */
	ASSERT(OFFSETOF(sdio_info_t, pub) == 0);

	sdio_info = MALLOC(pub->osh, sizeof(sdio_info_t));
	if (sdio_info == NULL)
		return NULL;

	pub->bus = sdio_info;

	bzero(sdio_info, sizeof(sdio_info_t));

	sdio_info->pub = pub;
	sdio_info->cbarg = cbarg;
	sdio_info->cbs = cbs;
	sdio_info->bus = SD_BUSTYPE;
	/* Use bufpool if allocated, else use locally malloced rxbuf */
	sdio_info->usebufpool = FALSE;
	sdio_info->hostintmask = HOSTINTMASK;

	sdio_info->halting = FALSE;
	INIT_FLAG(&sdio_info->rxtx_flag);

	sdio_info->sdos_info = g_dbusintf->attach(pub,
		sdio_info, &dbus_sdio_intf_cbs);
	if (sdio_info->sdos_info == NULL)
		goto err;

	def_sdos_info = sdio_info->sdos_info;

	/* Query the SD clock speed */
	if (dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_divisor", NULL, 0,
	                    &sdio_info->sd_divisor, sizeof(int32), FALSE) != BCME_OK) {
		DBUSERR(("%s: fail on %s get\n", __FUNCTION__, "sd_divisor"));
		sdio_info->sd_divisor = -1;
	} else {
		DBUSINFO(("%s: Initial value for %s is %d\n",
		          __FUNCTION__, "sd_divisor", sdio_info->sd_divisor));
	}

	/* Query the SD bus mode */
	if (dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_mode", NULL, 0,
	                    &sdio_info->sd_mode, sizeof(int32), FALSE) != BCME_OK) {
		DBUSERR(("%s: fail on %s get\n", __FUNCTION__, "sd_mode"));
		sdio_info->sd_mode = -1;
	} else {
		DBUSINFO(("%s: Initial value for %s is %d\n",
		          __FUNCTION__, "sd_mode", sdio_info->sd_mode));
	}

	/* Query the F2 block size, set roundup accordingly */
	fnum = 2;
	if (dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_blocksize", &fnum, sizeof(int32),
	                    &sdio_info->blocksize, sizeof(int32), FALSE) != BCME_OK) {
		sdio_info->blocksize = 0;
		DBUSERR(("%s: fail on %s get\n", __FUNCTION__, "sd_blocksize"));
	} else {
		DBUSINFO(("%s: Initial value for %s is %d\n",
		          __FUNCTION__, "sd_blocksize", sdio_info->blocksize));
	}
	sdio_info->roundup = MIN(max_roundup, sdio_info->blocksize);

	/* Query if bus module supports packet chaining, default to use if supported */
	if (dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_rxchain", NULL, 0,
	                    &sdio_info->sd_rxchain, sizeof(int32), FALSE) != BCME_OK) {
		sdio_info->sd_rxchain = FALSE;
	} else {
		DBUSINFO(("%s: bus module (through bcmsdh API) %s chaining\n",
		          __FUNCTION__, (sdio_info->sd_rxchain ? "supports" : "does not support")));
	}
	sdio_info->use_rxchain = (bool)sdio_info->sd_rxchain;

	/* FIX: Need to redo this maxctl stuff since we don't want cdc and IOCTL
	 * info in DBUS.  maxctl is used by rxbuf for static allocation.
	 *
	 * sdio_info->maxctl = WLC_IOCTL_MAXLEN + sizeof(cdc_ioctl_t) + ROUND_UP_MARGIN;
	 */
	sdio_info->maxctl = 8192 + 16 + 2048;
	if (sdio_info->maxctl) {
		sdio_info->rxblen =
			ROUNDUP((sdio_info->maxctl + SDPCM_HDRLEN), ALIGNMENT) + SDALIGN;
		if (!(sdio_info->rxbuf = MALLOC(pub->osh, sdio_info->rxblen))) {
			DBUSERR(("%s: MALLOC of %d-byte rxbuf failed\n",
			           __FUNCTION__, sdio_info->rxblen));
			goto err;
		}
	}

	/* Allocate buffer to receive glomed packet */
	if (!(sdio_info->databuf = MALLOC(pub->osh, MAX_DATA_BUF))) {
		DBUSERR(("%s: MALLOC of %d-byte databuf failed\n",
			__FUNCTION__, MAX_DATA_BUF));
		goto err;
	}

	/* Align the buffer */
	if ((uintptr)sdio_info->databuf % SDALIGN)
		sdio_info->dataptr =
			sdio_info->databuf + (SDALIGN - ((uintptr)sdio_info->databuf % SDALIGN));
	else
		sdio_info->dataptr = sdio_info->databuf;

	/* ...and initialize clock/power states */
	sdio_info->sleeping = FALSE;
	sdio_info->idletime = (int32)DHD_IDLETIME_TICKS;
	sdio_info->idleclock = IDLE_ACTIVE;

	/* read the vendor/device ID from the CIS */
	if (!dbus_sdio_probe_device(sdio_info)) {
		DBUSERR(("%s: dbus_sdio_probe_device failed\n", __FUNCTION__));
		goto err;
	}

	/* try to attach to the target device */
	if (dbus_sdio_device_init(sdio_info) != DBUS_OK) {
		DBUSERR(("%s: dbus_sdio_device_init\n", __FUNCTION__));
		goto err;
	}

	ASSERT(sdio_info->pub->nrxq > 0);
	pktq_init(&sdio_info->rxq, 1, sdio_info->pub->nrxq);

	ASSERT(sdio_info->pub->ntxq > 0);
	pktq_init(&sdio_info->txq, (PRIOMASK+1), sdio_info->pub->ntxq);

	/* Locate an appropriately-aligned portion of hdrbuf */
	sdio_info->rxhdr = (uint8*)ROUNDUP((uintptr)&sdio_info->hdrbuf[0], SDALIGN);

	/* Set the poll and/or interrupt flags */
	if ((sdio_info->poll = FALSE))
		sdio_info->pollrate = 1;

	/* Save SDIO OS-specific driver entry points */
	sdio_info->drvintf = g_dbusintf;

	if (sdio_info->devready == TRUE)
		sdio_info->pub->busstate = DBUS_STATE_DL_DONE;

	return (void *) sdio_info->sdos_info; /* Return Lower layer info */
err:
	dbus_sdif_detach(pub, sdio_info->sdos_info);
	return NULL;

}

static bool
dbus_sdif_device_exists(void *bus)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	if (!dbus_sdio_probe_device(sdio_info))
		return FALSE;
	return ((dbus_sdio_device_init(sdio_info) == DBUS_OK));
}

static int
dbus_sdio_alpclk(void *sdh)
{
	int err;
#ifndef BCMSPI
	uint8 clkctl = 0;
#endif /* !BCMSPI */

	/*
	 * Request ALP clock; ALP is required before starting a download
	 */
	dbus_sdos_cfg_write(sdh, SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, SBSDIO_ALP_AVAIL_REQ, &err);
	if (err) {
		DBUSERR(("%s: HT Avail request error: %d\n", __FUNCTION__, err));
		return DBUS_ERR;
	}

#ifndef BCMSPI
	/* Check current status */
	clkctl = dbus_sdos_cfg_read(sdh, SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, &err);
	if (err) {
		DBUSERR(("%s: HT Avail read error: %d\n", __FUNCTION__, err));
		return DBUS_ERR;
	}

	if (!SBSDIO_CLKAV(clkctl, TRUE)) {
		SPINWAIT(((clkctl = dbus_sdos_cfg_read(sdh, SDIO_FUNC_1,
			SBSDIO_FUNC1_CHIPCLKCSR, &err)),
			!SBSDIO_CLKAV(clkctl, TRUE)), PMU_MAX_TRANSITION_DLY);
	}
#endif

	return DBUS_OK;
}

/* Turn backplane clock on or off */
static int
dbus_sdio_htclk(sdio_info_t *sdio_info, bool on, bool pendok)
{
	int err;
	uint8 clkctl, clkreq, devctl;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	clkctl = 0;


	if (on) {
		/* Request HT Avail */
		clkreq = sdio_info->alp_only ? SBSDIO_ALP_AVAIL_REQ : SBSDIO_HT_AVAIL_REQ;


		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
			SBSDIO_FUNC1_CHIPCLKCSR, clkreq, &err);
		if (err) {
			DBUSERR(("%s: HT Avail request error: %d\n", __FUNCTION__, err));
			return BCME_ERROR;
		}

		if (pendok &&
		    ((sdio_info->sih->buscoretype == PCMCIA_CORE_ID) &&
			(sdio_info->sih->buscorerev == 9))) {
			uint32 dummy;
			dbus_sdio_reg_read(sdio_info, (uint32)&sdio_info->regs->clockctlstatus,
				sizeof(sdio_info->regs->clockctlstatus), &dummy, REGRETRIES);
		}

		/* Check current status */
		clkctl = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
			SBSDIO_FUNC1_CHIPCLKCSR, &err);
		if (err) {
			DBUSERR(("%s: HT Avail read error: %d\n", __FUNCTION__, err));
			return BCME_ERROR;
		}

		/* Go to pending and await interrupt if appropriate */
		if (!SBSDIO_CLKAV(clkctl, sdio_info->alp_only) && pendok) {
			DBUSINFO(("CLKCTL: set PENDING\n"));
			sdio_info->clkstate = CLK_PENDING;

			/* Allow only clock-available interrupt */
			devctl = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_DEVICE_CTL, &err);
			if (err) {
				DBUSERR(("%s: Devctl access error setting CA: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}

			devctl |= SBSDIO_DEVCTL_CA_INT_ONLY;
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_DEVICE_CTL, devctl, &err);
			return BCME_OK;
		} else if (sdio_info->clkstate == CLK_PENDING) {
			/* Cancel CA-only interrupt filter */
			devctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
				SDIO_FUNC_1, SBSDIO_DEVICE_CTL, &err);
			devctl &= ~SBSDIO_DEVCTL_CA_INT_ONLY;
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_DEVICE_CTL, devctl, &err);
		}

		/* Otherwise, wait here (polling) for HT Avail */
		if (!SBSDIO_CLKAV(clkctl, sdio_info->alp_only)) {
			SPINWAIT(((clkctl = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_FUNC1_CHIPCLKCSR, &err)),
				!SBSDIO_CLKAV(clkctl, sdio_info->alp_only)),
				PMU_MAX_TRANSITION_DLY);
		}
		if (err) {
			DBUSERR(("%s: HT Avail request error: %d\n", __FUNCTION__, err));
			return BCME_ERROR;
		}
		if (!SBSDIO_CLKAV(clkctl, sdio_info->alp_only)) {
			DBUSERR(("%s: HT Avail timeout (%d): clkctl 0x%02x\n",
			           __FUNCTION__, PMU_MAX_TRANSITION_DLY, clkctl));
			return BCME_ERROR;
		}

		/* Mark clock available */
		sdio_info->clkstate = CLK_AVAIL;
		DBUSINFO(("CLKCTL: turned ON\n"));

#if defined(BCMDBG)
		if (sdio_info->alp_only == TRUE) {
			if (!SBSDIO_ALPONLY(clkctl)) {
				DBUSERR(("%s: HT Clock, when ALP Only\n", __FUNCTION__));
			}
		} else {
			if (SBSDIO_ALPONLY(clkctl)) {
				DBUSERR(("%s: HT Clock should be on.\n", __FUNCTION__));
			}
		}
#endif /* defined (BCMDBG) */

		sdio_info->activity = TRUE;
	} else {
		clkreq = 0;

		if (sdio_info->clkstate == CLK_PENDING) {
			/* Cancel CA-only interrupt filter */
			devctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
				SDIO_FUNC_1, SBSDIO_DEVICE_CTL, &err);
			devctl &= ~SBSDIO_DEVCTL_CA_INT_ONLY;
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_DEVICE_CTL, devctl, &err);
		}

		sdio_info->clkstate = CLK_SDONLY;
		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
			SBSDIO_FUNC1_CHIPCLKCSR, clkreq, &err);
		DBUSINFO(("CLKCTL: turned OFF\n"));
		if (err) {
			DBUSERR(("%s: Failed access turning clock off: %d\n",
			           __FUNCTION__, err));
			return BCME_ERROR;
		}
	}
	return BCME_OK;
}

/* Change idle/active SD state */
static int
dbus_sdio_sdclk(sdio_info_t *sdio_info, bool on)
{
	int err;
	int32 iovalue;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	if (on) {
		if (sdio_info->idleclock == IDLE_STOP) {
			/* Turn on clock and restore mode */
			iovalue = 1;
			err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_clock", NULL, 0,
			                      &iovalue, sizeof(iovalue), TRUE);
			if (err) {
				DBUSERR(("%s: error enabling sd_clock: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}

			iovalue = sdio_info->sd_mode;
			err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_mode", NULL, 0,
			                      &iovalue, sizeof(iovalue), TRUE);
			if (err) {
				DBUSERR(("%s: error changing sd_mode: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}
		} else if (sdio_info->idleclock != IDLE_ACTIVE) {
			/* Restore clock speed */
			iovalue = sdio_info->sd_divisor;
			err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_divisor", NULL, 0,
			                      &iovalue, sizeof(iovalue), TRUE);
			if (err) {
				DBUSERR(("%s: error restoring sd_divisor: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}
		}
		sdio_info->clkstate = CLK_SDONLY;
	} else {
		/* Stop or slow the SD clock itself */
		if ((sdio_info->sd_divisor == -1) || (sdio_info->sd_mode == -1)) {
			DBUSTRACE(("%s: can't idle clock, divisor %d mode %d\n",
			           __FUNCTION__, sdio_info->sd_divisor, sdio_info->sd_mode));
			return BCME_ERROR;
		}
		if (sdio_info->idleclock == IDLE_STOP) {
			if (sd1idle) {
				/* Change to SD1 mode and turn off clock */
				iovalue = 1;
				err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_mode", NULL, 0,
				                      &iovalue, sizeof(iovalue), TRUE);
				if (err) {
					DBUSERR(("%s: error changing sd_clock: %d\n",
					           __FUNCTION__, err));
					return BCME_ERROR;
				}
			}

			iovalue = 0;
			err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_clock", NULL, 0,
			                      &iovalue, sizeof(iovalue), TRUE);
			if (err) {
				DBUSERR(("%s: error disabling sd_clock: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}
		} else if (sdio_info->idleclock != IDLE_ACTIVE) {
			/* Set divisor to idle value */
			iovalue = sdio_info->idleclock;
			err = dbus_sdos_iovar_op(sdio_info->sdos_info, "sd_divisor", NULL, 0,
			                      &iovalue, sizeof(iovalue), TRUE);
			if (err) {
				DBUSERR(("%s: error changing sd_divisor: %d\n",
				           __FUNCTION__, err));
				return BCME_ERROR;
			}
		}
		sdio_info->clkstate = CLK_NONE;
	}

	return BCME_OK;
}

/* Transition SD and backplane clock readiness */
static int
dbus_sdio_clkctl(sdio_info_t *sdio_info, uint target, bool pendok)
{
	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	/* Early exit if we're already there */
	if (sdio_info->clkstate == target) {
		if (target == CLK_AVAIL)
			sdio_info->activity = TRUE;
		return BCME_OK;
	}

	switch (target) {
	case CLK_AVAIL:
		/* Make sure SD clock is available */
		if (sdio_info->clkstate == CLK_NONE)
			dbus_sdio_sdclk(sdio_info, TRUE);
		/* Now request HT Avail on the backplane */
		dbus_sdio_htclk(sdio_info, TRUE, pendok);
		sdio_info->activity = TRUE;
		break;

	case CLK_SDONLY:
		/* Remove HT request, or bring up SD clock */
		if (sdio_info->clkstate == CLK_NONE)
			dbus_sdio_sdclk(sdio_info, TRUE);
		else if (sdio_info->clkstate == CLK_AVAIL)
			dbus_sdio_htclk(sdio_info, FALSE, FALSE);
		else
			DBUSERR(("dbus_sdio_clkctl: request for %d -> %d\n",
			           sdio_info->clkstate, target));
		break;

	case CLK_NONE:
		/* Make sure to remove HT request */
		if (sdio_info->clkstate == CLK_AVAIL)
			dbus_sdio_htclk(sdio_info, FALSE, FALSE);
		/* Now remove the SD clock */
		dbus_sdio_sdclk(sdio_info, FALSE);
		break;
	}
	DBUSINFO(("dbus_sdio_clkctl: %d\n", sdio_info->clkstate));

	return BCME_OK;
}

static void
dbus_bus_stop(sdio_info_t *sdio_info)
{
	uint8 saveclk;
	uint retries;
	int err;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	sdio_info->halting = TRUE;

	/* Enable clock for device interrupts */
	dbus_sdio_clkctl(sdio_info, CLK_AVAIL, FALSE);

	/* Disable and clear interrupts at the chip level also */
	dbus_sdio_reg_write(sdio_info, (uint32)&sdio_info->regs->hostintmask,
		sizeof(sdio_info->regs->hostintmask), 0, REGRETRIES);
	dbus_sdio_reg_write(sdio_info, (uint32)&sdio_info->regs->intstatus,
		sizeof(sdio_info->regs->intstatus), sdio_info->hostintmask, REGRETRIES);
	sdio_info->hostintmask = 0;


	/* Force clocks on backplane to be sure F2 interrupt propagates */
	saveclk = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
		SBSDIO_FUNC1_CHIPCLKCSR, &err);
	if (!err) {
		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR,
			(saveclk | SBSDIO_FORCE_HT), &err);
	}
	if (err) {
		DBUSERR(("%s: Failed to force clock for F2: err %d\n", __FUNCTION__, err));
	}

	/* Turn off the bus (F2), free any pending packets */
	DBUSINTR(("%s: disable SDIO interrupts\n", __FUNCTION__));
	dbus_sdos_intr_disable(sdio_info->sdos_info);
#ifndef BCMSPI
	dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_0,
		SDIOD_CCCR_IOEN, SDIO_FUNC_ENABLE_1, NULL);
#endif /* !BCMSPI */

	/* Turn off the backplane clock (only) */
	dbus_sdio_clkctl(sdio_info, CLK_SDONLY, FALSE);

	dbus_sdio_txq_flush(sdio_info);
	/* Change our idea of bus state */
	sdio_info->pub->busstate = DBUS_STATE_DOWN;

	/* Clear any held glomming stuff */
	if (sdio_info->glomd) {
		dbus_sdio_pktfree(sdio_info, sdio_info->glomd, FALSE);
		sdio_info->glomd = NULL;
	}

	if (sdio_info->glom) {
		dbus_sdio_pktfree(sdio_info, sdio_info->glom, FALSE);
		sdio_info->glom = NULL;
	}

	/* Clear rx control and wake any waiters */
	sdio_info->rxlen = 0;

	/* Reset some F2 state stuff */
	sdio_info->rxskip = FALSE;
	sdio_info->tx_seq = sdio_info->rx_seq = 0;
}

static int
dbus_sdio_init(sdio_info_t *sdio_info)
{
	uint retries = 0;

	uint8 ready = 0, enable;
	int err, ret = 0;
#ifdef BCMSPI
	uint32 dstatus = 0;	/* gSPI device status bits of */
#else /* BCMSPI */
	uint8 saveclk;
#endif /* BCMSPI */

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	/* Make sure backplane clock is on, needed to generate F2 interrupt */
	dbus_sdio_clkctl(sdio_info, CLK_AVAIL, FALSE);
	if (sdio_info->clkstate != CLK_AVAIL)
		goto exit;

#ifdef BCMSPI
	/* fake "ready" for spi, wake-wlan would have already enabled F1 and F2 */
	ready = (SDIO_FUNC_ENABLE_1 | SDIO_FUNC_ENABLE_2);

	/* Give the dongle some time to do its thing and set IOR2 */
	retries = WAIT_F2RXFIFORDY;
	enable = 0;
	while (retries-- && !enable) {
		OSL_DELAY(WAIT_F2RXFIFORDY_DELAY * 1000);
		dstatus = dbus_sdos_cfg_read_word(sdio_info->sdos_info,
			SDIO_FUNC_0, SPID_STATUS_REG, NULL);
		if (dstatus && STATUS_F2_RX_READY)
			enable = TRUE;
	}
	if (enable) {
		DBUSERR(("Took %d retries before dongle is ready with delay %d(ms) in between\n",
			WAIT_F2RXFIFORDY - retries, WAIT_F2RXFIFORDY_DELAY));
		enable = ready;
	}
	else {
		DBUSERR(("dstatus when timed out on f2-fifo not ready = 0x%x\n", dstatus));
		DBUSERR(("Waited %d retries, dongle is not ready with delay %d(ms) in between\n",
			WAIT_F2RXFIFORDY, WAIT_F2RXFIFORDY_DELAY));
		ret = -1;
		goto exit;
	}
#else /* BCMSPI */
	/* Force clocks on backplane to be sure F2 interrupt propagates */
	saveclk = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
		SBSDIO_FUNC1_CHIPCLKCSR, &err);
	if (!err) {
		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR,
		                 (saveclk | SBSDIO_FORCE_HT), &err);
	}
	if (err) {
		DBUSERR(("%s: Failed to force clock for F2: err %d\n", __FUNCTION__, err));
		goto exit;
	}

	DBUSERR(("%s: Card control Register 0x%8.8lx\n", __FUNCTION__,
		dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_0, 0xF1, &err)));

	/* Enable function 2 (frame transfers) */
	dbus_sdio_reg_write(sdio_info, (uint32)&sdio_info->regs->tosbmailboxdata,
		sizeof(sdio_info->regs->tosbmailboxdata),
		(SDPCM_PROT_VERSION << SMB_DATA_VERSION_SHIFT), REGRETRIES);
	enable = (SDIO_FUNC_ENABLE_1 | SDIO_FUNC_ENABLE_2);

	dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_0, SDIOD_CCCR_IOEN, enable, NULL);

	/* Give the dongle some time to do its thing and set IOR2 */
	retries = DHD_WAIT_F2RDY;

#ifdef BCMSLTGT
	retries *= htclkratio;
#endif /* BCMSLTGT */
	while ((enable !=
		((ready = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_0,
		SDIOD_CCCR_IORDY, NULL)))) && retries--) {
		OSL_DELAY(1000);
	}
#endif /* !BCMSPI */

	retries = 0;

	DBUSERR(("%s: enable 0x%02x, ready 0x%02x\n", __FUNCTION__, enable, ready));


	/* If F2 successfully enabled, set core and enable interrupts */
	if (ready == enable) {
		/* Make sure we're talking to the core. */
		if (!(sdio_info->regs = si_setcore(sdio_info->sih, PCMCIA_CORE_ID, 0)))
			sdio_info->regs = si_setcore(sdio_info->sih, SDIOD_CORE_ID, 0);

		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1, SBSDIO_WATERMARK,
			(uint8)watermark, &err);

		/* dbus_sdos_intr_unmask(sdio_info->sdos_info); */

		sdio_info->pub->busstate = DBUS_STATE_UP;
		dbus_sdos_intr_enable(sdio_info->sdos_info);

#ifdef DEBUG_LOST_INTERRUPTS
		{
			uint32 intstatus;
			bool hostpending;
			uint8 devena, devpend;
			uint sdr_retries = 0;

			hostpending = dbus_sdos_intr_pending(sdio_info->sdos_info);
			devena = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_0,
				SDIOD_CCCR_INTEN, NULL);
			devpend = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_0,
				SDIOD_CCCR_INTPEND, NULL);

			dbus_sdio_reg_read(sdio_info, (uint32)&sdio_info->regs->intstatus,
				sizeof(sdio_info->regs->intstatus), &intstatus, REGRETRIES);
			intstatus &= sdio_info->hostintmask;

			DBUSERR(("%s: interrupts -- host %s device ena/pend 0x%02x/0x%02x\n"
			           "intstatus 0x%08x, hostmask 0x%08x\n", __FUNCTION__,
			           (hostpending ? "PENDING" : "NOT PENDING"),
			           devena, devpend, intstatus, sdio_info->hostintmask));
		}
#endif /* DEBUG_LOST_INTERRUPTS */
	}

#ifndef BCMSPI

	else {
		ret = DBUS_ERR;

		/* Disable F2 again */
		enable = SDIO_FUNC_ENABLE_1;
		dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_0,
			SDIOD_CCCR_IOEN, enable, NULL);
	}

	/* Restore previous clock setting */
	dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
		SBSDIO_FUNC1_CHIPCLKCSR, saveclk, &err);

#endif /* !BCMSPI */

	/* If we didn't come up, turn off backplane clock */
	if (sdio_info->pub->busstate != DBUS_STATE_UP) {
		DBUSERR(("Error: Not up yet!\n"));
		dbus_sdio_clkctl(sdio_info, CLK_NONE, FALSE);
	}
exit:
	return ret;
}
static int
dbus_sdio_membytes(sdio_info_t *sdio_info, bool write, uint32 address, uint8 *data, uint size)
{
	int bcmerror = 0;
	uint32 sdaddr;
	uint dsize;
	uint32 cur_sbwad;

	/* save current sb window */
	cur_sbwad = dbus_sdos_get_sbaddr_window(sdio_info->sdos_info);

	/* Determine initial transfer parameters */
	sdaddr = address & SBSDIO_SB_OFT_ADDR_MASK;
	if ((sdaddr + size) & SBSDIO_SBWINDOW_MASK)
		dsize = (SBSDIO_SB_OFT_ADDR_LIMIT - sdaddr);
	else
		dsize = size;

	/* Set the backplane window to include the start address */
	if ((bcmerror = dbus_sdos_set_sbaddr_window(sdio_info->sdos_info, address, TRUE))) {
		DBUSERR(("%s: window change failed\n", __FUNCTION__));
		goto xfer_done;
	}

	/* Do the transfer(s) */
	while (size) {
		DBUSINFO(("%s: %s %d bytes at offset 0x%08x in window 0x%08x\n",
		          __FUNCTION__, (write ? "write" : "read"), dsize, sdaddr,
		          (address & SBSDIO_SBWINDOW_MASK)));
		if ((bcmerror = dbus_sdos_rwdata(sdio_info->sdos_info, write,
			sdaddr, data, dsize))) {
			DBUSERR(("%s: membytes transfer failed\n", __FUNCTION__));
			break;
		}

		/* Adjust for next transfer (if any) */
		if ((size -= dsize)) {
			data += dsize;
			address += dsize;
			if ((bcmerror = dbus_sdos_set_sbaddr_window(sdio_info->sdos_info,
				address, FALSE))) {
				DBUSERR(("%s: window change failed\n", __FUNCTION__));
				break;
			}
			sdaddr = 0;
			dsize = MIN(SBSDIO_SB_OFT_ADDR_LIMIT, size);
		}
	}

xfer_done:
	/* Return the window to backplane enumeration space for core access */
	if (dbus_sdos_set_sbaddr_window(sdio_info->sdos_info, cur_sbwad, FALSE)) {
		DBUSERR(("%s: FAILED to set window back to 0x%x\n", __FUNCTION__,
			cur_sbwad));
	}

	return bcmerror;
}

static int
dbus_sdio_downloadvars(sdio_info_t *sdio_info, void *arg, int len)
{
	int bcmerror = BCME_OK;

	if (!len) {
		bcmerror = BCME_BUFTOOSHORT;
		goto err;
	}

	if (sdio_info->vars) {
		MFREE(sdio_info->pub->osh, sdio_info->vars, sdio_info->varsz);
		sdio_info->vars = NULL;
		sdio_info->varsz = 0;
	}
	sdio_info->vars = MALLOC(sdio_info->pub->osh, len);
	sdio_info->varsz = sdio_info->vars ? len : 0;
	if (sdio_info->vars == NULL) {
		sdio_info->varsz = 0;
		bcmerror = BCME_NOMEM;
		goto err;
	}
	bcopy(arg, sdio_info->vars, sdio_info->varsz);
err:
	return bcmerror;
}

static int
dbus_sdio_write_vars(sdio_info_t *sdio_info)
{
	int bcmerror = 0;
	uint32 varsize;
	uint32 varaddr;
	char *vbuffer;
	uint32 varsizew;

	if (!sdio_info->varsz || !sdio_info->vars)
		return BCME_OK;

	varsize = ROUNDUP(sdio_info->varsz, 4);
	varaddr = (sdio_info->ramsize - 4) - varsize;

	vbuffer = (char*)MALLOC(sdio_info->pub->osh, varsize);
	if (!vbuffer)
		return BCME_NOMEM;

	bzero(vbuffer, varsize);
	bcopy(sdio_info->vars, vbuffer, sdio_info->varsz);

	/* Write the vars list */
	bcmerror = dbus_sdio_membytes(sdio_info, TRUE, varaddr, vbuffer, varsize);

	MFREE(sdio_info->pub->osh, vbuffer, varsize);

	/* adjust to the user specified RAM */
	DBUSINFO(("origram size is %d and used ramsize is %d, vars are at %d, orig varsize is %d\n",
		sdio_info->orig_ramsize, sdio_info->ramsize, varaddr, varsize));
	varsize = ((sdio_info->orig_ramsize - 4) - varaddr);
	varsizew = varsize >> 2;
	DBUSINFO(("new varsize is %d, varsizew is %d\n", varsize, varsizew));

	/* Write the length to the last word */
	if (bcmerror) {
		varsizew = 0;
		DBUSINFO(("bcmerror : Varsizew is being written as %d\n", varsizew));
		dbus_sdio_membytes(sdio_info, TRUE,
			(sdio_info->orig_ramsize - 4), (uint8*)&varsizew, 4);
	} else {
		DBUSINFO(("Varsize is %d and varsizew is %d\n", varsize, varsizew));
		varsizew = (~varsizew << 16) | (varsizew & 0x0000FFFF);
		varsizew = htol32(varsizew);
		DBUSINFO(("Varsizew is 0x%x and htol is 0x%x\n",
			varsizew, htol32(varsizew)));
		bcmerror = dbus_sdio_membytes(sdio_info, TRUE,
			(sdio_info->orig_ramsize - 4), (uint8*)&varsizew, 4);
	}

	return bcmerror;
}

static int
dbus_sdio_download_state(sdio_info_t *sdio_info, bool enter)
{
	int bcmerror = 0;
	si_t *sih;

	ASSERT(sdio_info->sih);
	ASSERT(sdio_info->sdos_info);

	sih = sdio_info->sih;

	/* To enter download state, disable ARM and reset SOCRAM.
	 * To exit download state, simply reset ARM (default is RAM boot).
	 */
	if (enter) {

		sdio_info->alp_only = TRUE;

		if (!(si_setcore(sih, ARM7S_CORE_ID, 0)) &&
		    !(si_setcore(sih, ARMCM3_CORE_ID, 0))) {
			DBUSERR(("%s: Failed to find ARM core!\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		si_core_disable(sih, 0);
		if (bcmsdh_regfail(sdio_info->sdos_info)) {
			DBUSERR(("%s: Failed to disable ARM core!\n", __FUNCTION__));
			bcmerror = BCME_SDIO_ERROR;
			goto fail;
		}

		if (!(si_setcore(sih, SOCRAM_CORE_ID, 0))) {
			DBUSERR(("%s: Failed to find SOCRAM core!\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		si_core_reset(sih, 0, 0);
		if (bcmsdh_regfail(sdio_info->sdos_info)) {
			DBUSERR(("%s: Failure trying reset SOCRAM core?\n", __FUNCTION__));
			bcmerror = BCME_SDIO_ERROR;
			goto fail;
		}

		/* Clear the top bit of memory */
		if (sdio_info->ramsize) {
			uint32 zeros = 0;
			dbus_sdio_membytes(sdio_info, TRUE,
				sdio_info->ramsize - 4, (uint8*)&zeros, 4);
		}
	} else {
		if (!(si_setcore(sih, SOCRAM_CORE_ID, 0))) {
			DBUSERR(("%s: Failed to find SOCRAM core!\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		if (!si_iscoreup(sih)) {
			DBUSERR(("%s: SOCRAM core is down after reset?\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		if ((bcmerror = dbus_sdio_write_vars(sdio_info))) {
			DBUSERR(("%s: could not write vars to ram\n", __FUNCTION__));
			goto fail;
		}

		if (!si_setcore(sih, PCMCIA_CORE_ID, 0) &&
		    !si_setcore(sih, SDIOD_CORE_ID, 0)) {
			DBUSERR(("%s: Can't change back to SDIO core?\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}
		W_REG(sdio_info->pub->osh, &sdio_info->regs->intstatus, 0xFFFFFFFF);


		if (!(si_setcore(sih, ARM7S_CORE_ID, 0)) &&
		    !(si_setcore(sih, ARMCM3_CORE_ID, 0))) {
			DBUSERR(("%s: Failed to find ARM core!\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		si_core_reset(sih, 0, 0);
		if (bcmsdh_regfail(sdio_info->sdos_info)) {
			DBUSERR(("%s: Failure trying to reset ARM core?\n", __FUNCTION__));
			bcmerror = BCME_SDIO_ERROR;
			goto fail;
		}

		/* Allow HT Clock now that the ARM is running. */
		sdio_info->alp_only = FALSE;
	}

fail:
	/* Always return to SDIOD core */
	if (!si_setcore(sih, PCMCIA_CORE_ID, 0))
		si_setcore(sih, SDIOD_CORE_ID, 0);

	return bcmerror;
}


#ifdef BCM_DNGL_EMBEDIMAGE
int
dbus_sdio_download_image_array(sdio_info_t *sdio_info, uint8 *fw, int len)
{
	int bcmerror = -1;
	int offset = 0;

	/* Download image */
	while ((offset + MEMBLOCK) < len) {
		bcmerror = dbus_sdio_membytes(sdio_info, TRUE,
			offset, fw + offset, MEMBLOCK);
		if (bcmerror) {
			DBUSERR(("%s: error %d on writing %d membytes at 0x%08x\n",
			        __FUNCTION__, bcmerror, MEMBLOCK, offset));
			goto err;
		}

		offset += MEMBLOCK;
	}

	if (offset < len) {
		bcmerror = dbus_sdio_membytes(sdio_info, TRUE, offset,
			fw + offset, len - offset);
		if (bcmerror) {
			DBUSERR(("%s: error %d on writing %d membytes at 0x%08x\n",
			        __FUNCTION__, bcmerror, len - offset, offset));
			goto err;
		}
	}

	/* Download SROM if provided externally through file */
	dbus_sdio_download_nvram_file(sdio_info);
err:
	return bcmerror;
}

/* 
 * ProcessVars:Takes a buffer of "<var>=<value>\n" lines read from a file and ending in a NUL.
 * Removes carriage returns, empty lines, comment lines, and converts newlines to NULs.
 * Shortens buffer as needed and pads with NULs.  End of buffer is marked by two NULs.
*/

static uint
process_nvram_vars(char *varbuf, uint len)
{
	char *dp;
	bool findNewline;
	int column;
	uint buf_len, n;

	dp = varbuf;

	findNewline = FALSE;
	column = 0;

	for (n = 0; n < len; n++) {
		if (varbuf[n] == 0)
			break;
		if (varbuf[n] == '\r')
			continue;
		if (findNewline && varbuf[n] != '\n')
			continue;
		findNewline = FALSE;
		if (varbuf[n] == '#') {
			findNewline = TRUE;
			continue;
		}
		if (varbuf[n] == '\n') {
			if (column == 0)
				continue;
			*dp++ = 0;
			column = 0;
			continue;
		}
		*dp++ = varbuf[n];
		column++;
	}
	buf_len = (int)(dp - varbuf);

	while (dp < varbuf + n)
		*dp++ = 0;

	return buf_len;
}

int
dbus_sdio_download_nvram_file(sdio_info_t *sdio_info)
{
	int bcmerror = -1;
	uint len = 0;
	void * image = NULL;
	uint8 * memblock = NULL;
	char *bufp;
	uint32 varsizew = 0;

	UNUSED_PARAMETER(image);
	UNUSED_PARAMETER(varsizew);

	if (!(defvarslen && (defvarslen < MEMBLOCK))) {
		sdio_info->varsz = 0;
		sdio_info->vars = NULL;
		goto err;
	}

	memblock = MALLOC(sdio_info->pub->osh, MEMBLOCK);
	if (memblock == NULL) {
		DBUSERR(("%s: Failed to allocate memory %d bytes\n",
		           __FUNCTION__, MEMBLOCK));
		goto err;
	}

	/* Download variables */
	/* FIX: Need to implement dhd_os_get_image_block() */
	/* len = dhd_os_get_image_block(memblock, MEMBLOCK, image); */
	len = defvarslen;
	bcopy(mfgsromvars, memblock, len);

	if (len != MEMBLOCK && len > 0) {
		bufp = (char *)memblock;
		len = process_nvram_vars(bufp, len);
		bufp += len;
		*bufp++ = 0;
		if (len)
			bcmerror = dbus_sdio_downloadvars(sdio_info, memblock, len + 1);
		if (bcmerror) {
			DBUSERR(("%s: error downloading vars: %d\n",
			           __FUNCTION__, bcmerror));
		}
	} else {
		DBUSERR(("%s: error reading nvram file: %d\n",
		           __FUNCTION__, len));
		bcmerror = BCME_SDIO_ERROR;
	}

err:
	if (memblock)
		MFREE(sdio_info->pub->osh, memblock, MEMBLOCK);

	/* FIX: Need to implement dhd_os_close_image() */

	return bcmerror;
}
#endif /* BCM_DNGL_EMBEDIMAGE */

static int
dbus_sdif_up(void *bus)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	int err = DBUS_ERR;

	if (sdio_info == NULL)
		return DBUS_ERR;

	err = dbus_sdio_init(sdio_info);
	if (err != 0)
		err = DBUS_ERR;

	if (sdio_info->drvintf && sdio_info->drvintf->up) {
		err = sdio_info->drvintf->up(sdio_info->sdos_info);
	}

	return err;
}

static bool
dbus_sdif_dlneeded(void *bus)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);

	if (sdio_info == NULL)
		return FALSE;
	return TRUE;
}

static int
dbus_sdif_dlstart(void *bus, uint8 *fw, int len)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	int err = DBUS_ERR;

	if (sdio_info == NULL)
		return DBUS_ERR;

	dbus_sdio_alpclk(sdio_info->sdos_info);
	sdio_info->clkstate = CLK_AVAIL;

	/* Put ARM in reset for download */
	err = dbus_sdio_download_state(sdio_info, TRUE);
	if (err) {
		DBUSERR(("%s: error placing ARM core in reset\n", __FUNCTION__));
		err = DBUS_ERR;
		goto err;
	}

	/* FIX: Which embedded image has priority?
	 */
#ifdef BCM_DNGL_EMBEDIMAGE
	if (dbus_sdio_download_image_array(sdio_info, fw, len)) {
		DBUSERR(("%s: dongle image download failed\n", __FUNCTION__));
		err = DBUS_ERR;
		goto err;
	}
#endif /* BCM_DNGL_EMBEDIMAGE */

	/* FIX: Skip this for now
	 * If above succeeds, do we still download this one?
	 */
	err = DBUS_OK;
	sdio_info->devready = TRUE;
	sdio_info->pub->busstate = DBUS_STATE_DL_DONE;
err:
	return err;
}

static int
dbus_sdif_dlrun(void *bus)
{
	sdio_info_t *sdio_info;
	int err = DBUS_ERR;

	sdio_info = BUS_INFO(bus, sdio_info_t);

	/* Take ARM out of reset */
	err = dbus_sdio_download_state(sdio_info, FALSE);
	if (err) {
		DBUSERR(("%s: error getting out of ARM reset\n", __FUNCTION__));
		err = DBUS_ERR;
	} else
		err = DBUS_OK;

	return err;
}

static int
dbus_sdif_down(void *bus)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	int err;

	dbus_bus_stop(sdio_info);
	if (sdio_info->drvintf && sdio_info->drvintf->down)
		err = sdio_info->drvintf->down(sdio_info->sdos_info);

	return DBUS_OK;
}

static int
dbus_sdif_get_attrib(void *bus, dbus_attrib_t *attrib)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);

	if ((sdio_info == NULL) || (attrib == NULL))
		return DBUS_ERR;

	attrib->bustype = DBUS_SDIO;
	attrib->vid = 0;
	attrib->pid = 0;
	attrib->devid = sdio_info->devid;
	attrib->nchan = 1;
	attrib->mtu = 512;

	return DBUS_OK;
}

static void
dbus_sdio_state_change(void *handle, int state)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->state_change)
		sdio_info->cbs->state_change(sdio_info->cbarg, state);

	if (state == DBUS_STATE_DISCONNECT) {
		if (sdio_info->drvintf && sdio_info->drvintf->remove)
			sdio_info->drvintf->remove(sdio_info->sdos_info);

		sdio_info->pub->busstate = DBUS_STATE_DOWN;
	} else if (state == DBUS_STATE_SLEEP) {
		sdio_info->halting = TRUE;
		dbus_sdio_device_terminate(sdio_info);
	} else if (state == DBUS_STATE_UP) {
		sdio_info->halting = FALSE;
	}
}

static void
dbus_sdio_errhandler(void *handle, int err)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->errhandler)
		sdio_info->cbs->errhandler(sdio_info->cbarg, err);
}

static void *
dbus_sdio_pktget(sdio_info_t *sdio_info, uint len, bool send)
{
	void *p = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (sdio_info == NULL)
		return NULL;

	if (sdio_info->cbs && sdio_info->cbs->pktget)
		p = sdio_info->cbs->pktget(sdio_info->cbarg, len, send);

	return p;
}

static void
dbus_sdio_pktfree(sdio_info_t *sdio_info, void *p, bool send)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->pktfree)
		sdio_info->cbs->pktfree(sdio_info->cbarg, p, send);
}

static dbus_irb_t *
dbus_sdio_getirb(sdio_info_t *sdio_info, bool send)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (sdio_info == NULL)
		return NULL;

	if (sdio_info->cbs && sdio_info->cbs->getirb)
		return sdio_info->cbs->getirb(sdio_info->cbarg, send);

	return NULL;
}

static void *
dbus_sdio_exec_rxlock(sdio_info_t *sdio_info, exec_cb_t cb, struct exec_parms *args)
{
	ASSERT(cb);
	if (sdio_info->drvintf && sdio_info->drvintf->exec_rxlock)
		return sdio_info->drvintf->exec_rxlock(sdio_info->sdos_info, cb, args);

	return NULL;
}

static void *
dbus_sdio_exec_txlock(sdio_info_t *sdio_info, exec_cb_t cb, struct exec_parms *args)
{
	ASSERT(cb);
	if (sdio_info->drvintf && sdio_info->drvintf->exec_txlock)
		return sdio_info->drvintf->exec_txlock(sdio_info->sdos_info, cb, args);

	return NULL;
}

static void *
dbus_prec_pkt_deq_exec(struct exec_parms *args)
{
	return pktq_mdeq(args->pdeq.q, args->pdeq.prec_map,
		args->pdeq.prec_out);
}

static void *
dbus_prec_pkt_enq_exec(struct exec_parms *args)

{
	return  pktq_penq(args->penq.q, args->penq.prec,
		args->penq.pkt);
}

static int
dbus_sdio_txbuf_submit(sdio_info_t *sdio_info, dbus_irb_tx_t *txirb)
{
	int ret = 0;
	void *berr;
	osl_t *osh;
	uint datalen, prec;
	void *pkt;
	pkttag_t *ptag;
	struct exec_parms exec_args;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	osh = sdio_info->pub->osh;
	pkt = txirb->pkt;
	if (pkt == NULL) {
		/*
		 * For BMAC sdio high driver that uses send_buf,
		 * we need to convert the buf into pkt for dbus.
		 */
		datalen = txirb->len;
		DBUSTRACE(("%s: Converting buf(%d bytes) to pkt.\n", __FUNCTION__, datalen));
		pkt = dbus_sdio_pktget(sdio_info, datalen, TRUE);
		if (pkt == NULL) {
			DBUSERR(("%s: Out of Tx buf.\n", __FUNCTION__));
			return DBUS_ERR_TXDROP;
		}

		txirb->pkt = pkt;
		bcopy(txirb->buf, PKTDATA(osh, pkt), datalen);
		PKTLEN(osh, pkt) = datalen;
	} else
		datalen = PKTLEN(osh, pkt);

	ASSERT(OSL_PKTTAG_SZ >= sizeof(pkttag_t));
	ptag = (pkttag_t *) PKTTAG(pkt);
	ptag->sdio_info = sdio_info;
	ptag->txirb = txirb;

	ASSERT(PKTHEADROOM(osh, pkt) >= SDPCM_HDRLEN);
	/* Add space for the header */
	PKTPUSH(osh, pkt, SDPCM_HDRLEN);
	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, pkt), 2));

	prec = PRIO2PREC((PKTPRIO(pkt) & PRIOMASK));

	sdio_info->fcqueued++;

	/* Priority based enq */
	exec_args.penq.sdio_info = sdio_info;
	exec_args.penq.q = &sdio_info->txq;
	exec_args.penq.pkt = pkt;
	exec_args.penq.prec = prec;
	berr = dbus_sdio_exec_txlock(sdio_info,
		(exec_cb_t) dbus_prec_pkt_enq_exec, &exec_args);
	if (berr == NULL) {
		DBUSERR(("%s: Dropping pkt!\n", __FUNCTION__));
		ASSERT(0); /* FIX: Should not be dropping pkts */
		ret = DBUS_ERR_TXFAIL;
		goto err;
	}
#ifdef BCMDBG
	if (pktq_plen(&sdio_info->txq, prec) > qcount[prec])
		qcount[prec] = pktq_plen(&sdio_info->txq, prec);
#endif
	if (!CHECK_FLAG(&sdio_info->rxtx_flag))
		dbus_sdio_txq_process(sdio_info);

err:
	return ret;
}

static void
dbus_sdio_txq_flush(sdio_info_t *sdio_info)
{
	int prec_out;
	struct exec_parms exec_args;
	pkttag_t *ptag;
	void *pkt;

	exec_args.pdeq.sdio_info = sdio_info;
	exec_args.penq.q = &sdio_info->txq;
	exec_args.pdeq.prec_map = ALLPRIO;
	exec_args.pdeq.prec_out = &prec_out;

	/* Cancel all pending pkts */
	while ((pkt = dbus_sdio_exec_txlock(sdio_info,
		(exec_cb_t) dbus_prec_pkt_deq_exec, &exec_args)) != NULL) {
		ptag = (pkttag_t *) PKTTAG(pkt);
		ASSERT(ptag);

		dbus_sdio_send_irb_complete(sdio_info, ptag->txirb, DBUS_STATUS_CANCELLED);
	}
}

/* Writes a HW/SW header into the packet and sends it. */
/* Assumes: (a) header space already there, (b) caller holds lock */
static int
dbus_sdio_txpkt(sdio_info_t *sdio_info, void *pkt, uint chan)
{
	int ret;
	osl_t *osh;
	uint8 *frame;
	uint16 len, pad;
	uint32 swheader;
	uint retries = 0;
	void *new;
	pkttag_t *ptag;
	int i;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	osh = sdio_info->pub->osh;

	if (sdio_info->dongle_reset) {
		ret = BCME_NOTREADY;
		goto done;
	}

	frame = (uint8*)PKTDATA(osh, pkt);

	/* Add alignment padding, allocate new packet if needed */
	if ((pad = ((uintptr)frame % SDALIGN))) {
		if (PKTHEADROOM(osh, pkt) < pad) {
			DBUSINFO(("%s: insufficient headroom %d for %d pad\n",
			          __FUNCTION__, (int)PKTHEADROOM(osh, pkt), pad));
			sdio_info->tx_realloc++;
			new = dbus_sdio_pktget(sdio_info, (PKTLEN(osh, pkt) + SDALIGN), TRUE);
			if (!new) {
				DBUSERR(("%s: couldn't allocate new %d-byte packet\n",
				           __FUNCTION__, PKTLEN(osh, pkt) + SDALIGN));
				ret = BCME_NOMEM;
				goto done;
			}

			PKTALIGN(osh, new, PKTLEN(osh, pkt), SDALIGN);
			bcopy(PKTDATA(osh, pkt), PKTDATA(osh, new), PKTLEN(osh, pkt));

			*((pkttag_t *)PKTTAG(new)) = *((pkttag_t *)PKTTAG(pkt));
			((pkttag_t *)PKTTAG(new))->txirb->pkt = new;
			((pkttag_t *)PKTTAG(new))->txirb->info = new;

			dbus_sdio_pktfree(sdio_info, pkt, TRUE);
			pkt = new;
			frame = (uint8*)PKTDATA(osh, pkt);
			ASSERT(((uintptr)frame % SDALIGN) == 0);
			pad = 0;
		} else {
			PKTPUSH(osh, pkt, pad);
			frame = (uint8*)PKTDATA(osh, pkt);
			bzero(frame, pad + SDPCM_HDRLEN);
		}
	}
	ASSERT(pad < SDALIGN);

	/* Hardware tag: 2 byte len followed by 2 byte ~len check (all LE) */
	len = (uint16)PKTLEN(osh, pkt);
	*(uint16*)frame = htol16(len);
	*(((uint16*)frame) + 1) = htol16(~len);

	/* Software tag: channel, sequence number, data offset */
	swheader = ((chan << SDPCM_CHANNEL_SHIFT) & SDPCM_CHANNEL_MASK) | sdio_info->tx_seq |
	        (((pad + SDPCM_HDRLEN) << SDPCM_DOFFSET_SHIFT) & SDPCM_DOFFSET_MASK);
	htol32_ua_store(swheader, frame + SDPCM_FRAMETAG_LEN);
	htol32_ua_store(0, frame + SDPCM_FRAMETAG_LEN + sizeof(swheader));
	sdio_info->tx_seq = (sdio_info->tx_seq + 1) % SDPCM_SEQUENCE_WRAP;

	/* Raise len to next SDIO block to eliminate tail command */
	if (sdio_info->roundup && sdio_info->blocksize && (len > sdio_info->blocksize)) {
		pad = sdio_info->blocksize - (len % sdio_info->blocksize);
		if ((pad <= sdio_info->roundup) && (pad < sdio_info->blocksize))
#ifdef NOTUSED
			if (pad <= PKTTAILROOM(osh, pkt))
#endif
				len += pad;
	}

	/* Some controllers have trouble with odd bytes -- round to even */
	if (forcealign && (len & (ALIGNMENT - 1))) {
#ifdef NOTUSED
		if (PKTTAILROOM(osh, pkt))
#endif
			len = ROUNDUP(len, ALIGNMENT);
#ifdef NOTUSED
		else
			DBUSERR(("%s: sending unrounded %d-byte packet\n", __FUNCTION__, len));
#endif
	}

	do {
		ret = dbus_sdos_send_buf(sdio_info->sdos_info, SI_ENUM_BASE, SDIO_FUNC_2, F2SYNC,
		                      frame, len);
		sdio_info->f2txdata++;
		ASSERT(ret != BCME_PENDING);

		if (ret < 0) {
			/* On failure, abort the command and terminate the frame */
			DBUSINFO(("%s: sdio error %d, abort command and terminate frame.\n",
			          __FUNCTION__, ret));
			sdio_info->tx_sderrs++;

			ret = dbus_sdos_abort(sdio_info->sdos_info, SDIO_FUNC_2);
			if (ret == BCME_NODEVICE) {
				dbus_sdio_state_change(sdio_info, DBUS_STATE_DISCONNECT);
				break;
			}
#ifdef BCMSPI
			DBUSERR(("%s: gSPI transmit error.  Check Overflow or F2-fifo-not-ready"
			           " counters.\n", __FUNCTION__));
#endif /* BCMSPI */
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_FUNC1_FRAMECTRL, SFC_WF_TERM, NULL);
			sdio_info->f1regdata++;

			for (i = 0; i < 3; i++) {
				uint8 hi, lo;
				hi = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
					SBSDIO_FUNC1_WFRAMEBCHI, NULL);
				lo = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
					SBSDIO_FUNC1_WFRAMEBCLO, NULL);
				sdio_info->f1regdata += 2;
				if ((hi == 0) && (lo == 0))
					break;
			}
		}
	} while ((ret < 0) && retrydata && retries++ < TXRETRIES);

done:
	ASSERT(OSL_PKTTAG_SZ >= sizeof(pkttag_t));
	ptag = (pkttag_t *) PKTTAG(pkt);
	ASSERT(ptag);
	dbus_sdio_send_irb_complete(sdio_info, ptag->txirb, (ret ? DBUS_ERR_TXFAIL : DBUS_OK));

	/* when sdio_info->cbs->send_irb_complete is defined,
	 * this send_irb_complete frees the packets 
	 */
	if (!(sdio_info->cbs && sdio_info->cbs->send_irb_complete)) {
		dbus_sdio_pktfree(sdio_info, pkt, TRUE);
	}

	return ret;
}

static uint
dbus_sdio_sendfromq(sdio_info_t *sdio_info)
{
	void *pkt;
	int ret = 0, prec_out;
	uint datalen, cnt = 0;
	uint8 tx_prec_map;
	struct exec_parms exec_args;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	tx_prec_map = ~sdio_info->flowcontrol;

	/* Send frames until the limit or some other event */
	for (cnt = 0; DATAOK(sdio_info); cnt++) {
		exec_args.pdeq.sdio_info = sdio_info;
		exec_args.penq.q = &sdio_info->txq;
		exec_args.pdeq.prec_map = tx_prec_map;
		exec_args.pdeq.prec_out = &prec_out;
		pkt = dbus_sdio_exec_txlock(sdio_info,
			(exec_cb_t) dbus_prec_pkt_deq_exec, &exec_args);
		if (pkt == NULL)
			break;

		datalen = PKTLEN(sdio_info->pub->osh, pkt) - SDPCM_HDRLEN;

		ret = dbus_sdio_txpkt(sdio_info, pkt, SDPCM_DATA_CHANNEL);

		if (ret) {
			sdio_info->pub->stats.tx_errors++;
			if (sdio_info->pub->busstate == DBUS_STATE_DOWN)
				break;
		}
	}

	return cnt;
}

static int
dbus_sdio_txctl(sdio_info_t *sdio_info, uchar *msg, uint msglen)
{
	uint8 *frame;
	uint16 len, pad;
	uint32 swheader;
	uint retries = 0;
	uint8 doff = 0;
	int ret = 0;
	int i;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	if (sdio_info->dongle_reset)
		return DBUS_ERR;

	/* Back the pointer to make a room for bus header */
	frame = msg - SDPCM_HDRLEN;
	len = (msglen += SDPCM_HDRLEN);

	/* Add alignment padding (optional for ctl frames) */
	if (dhd_alignctl) {
		if ((doff = ((uintptr)frame % SDALIGN))) {
			frame -= doff;
			len += doff;
			msglen += doff;
			bzero(frame, doff + SDPCM_HDRLEN);
		}
		ASSERT(doff < SDALIGN);
	}
	doff += SDPCM_HDRLEN;

	/* Round send length to next SDIO block */
	if (sdio_info->roundup && sdio_info->blocksize && (len > sdio_info->blocksize)) {
		pad = sdio_info->blocksize - (len % sdio_info->blocksize);
		if ((pad <= sdio_info->roundup) && (pad < sdio_info->blocksize))
			len += pad;
	}

	/* Satisfy length-alignment requirements */
	if (forcealign && (len & (ALIGNMENT - 1)))
		len = ROUNDUP(len, ALIGNMENT);

	ASSERT(ISALIGNED((uintptr)frame, 2));

	/* Make sure backplane clock is on */
	dbus_sdio_clkctl(sdio_info, CLK_AVAIL, FALSE);

	/* Hardware tag: 2 byte len followed by 2 byte ~len check (all LE) */
	*(uint16*)frame = htol16((uint16)msglen);
	*(((uint16*)frame) + 1) = htol16(~msglen);

	/* Software tag: channel, sequence number, data offset */
	swheader = ((SDPCM_CONTROL_CHANNEL << SDPCM_CHANNEL_SHIFT) & SDPCM_CHANNEL_MASK)
	        | sdio_info->tx_seq | ((doff << SDPCM_DOFFSET_SHIFT) & SDPCM_DOFFSET_MASK);
	htol32_ua_store(swheader, frame + SDPCM_FRAMETAG_LEN);
	htol32_ua_store(0, frame + SDPCM_FRAMETAG_LEN + sizeof(swheader));
	sdio_info->tx_seq = (sdio_info->tx_seq + 1) % SDPCM_SEQUENCE_WRAP;

	do {
		ret = dbus_sdos_send_buf(sdio_info->sdos_info, SI_ENUM_BASE, SDIO_FUNC_2, F2SYNC,
		                      frame, len);
		ASSERT(ret != BCME_PENDING);

		if (ret < 0) {
			/* On failure, abort the command and terminate the frame */
			DBUSINFO(("%s: sdio error %d, abort command and terminate frame.\n",
			          __FUNCTION__, ret));
			sdio_info->tx_sderrs++;

			ret = dbus_sdos_abort(sdio_info->sdos_info, SDIO_FUNC_2);
			if (ret == BCME_NODEVICE) {
				dbus_sdio_state_change(sdio_info, DBUS_STATE_DISCONNECT);
				break;
			}

#ifdef BCMSPI
			DBUSERR(("%s: Check Overflow or F2-fifo-not-ready counters."
			           " gSPI transmit error on control channel.\n", __FUNCTION__));
#endif /* BCMSPI */
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_FUNC1_FRAMECTRL, SFC_WF_TERM, NULL);
			sdio_info->f1regdata++;

			for (i = 0; i < 3; i++) {
				uint8 hi, lo;
				hi = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
				                     SBSDIO_FUNC1_WFRAMEBCHI, NULL);
				lo = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
				                     SBSDIO_FUNC1_WFRAMEBCLO, NULL);
				sdio_info->f1regdata += 2;
				if ((hi == 0) && (lo == 0))
					break;
			}
		}
	} while ((ret < 0) && retries++ < TXRETRIES);

	if (sdio_info->idletime == IDLE_IMMEDIATE) {
		sdio_info->activity = FALSE;
		dbus_sdio_clkctl(sdio_info, CLK_NONE, TRUE);
	}

	if (ret)
		sdio_info->tx_ctlerrs++;
	else
		sdio_info->tx_ctlpkts++;

	return ret ? DBUS_ERR : DBUS_OK;
}

static int
dbus_sdio_txctlq_process(void *bus)
{
	sdio_info_t *sdio_info = bus;
	int err = DBUS_OK;

	if (sdio_info->txctl_req.pending == TRUE) {
		ASSERT(sdio_info->txctl_req.buf);
		ASSERT(sdio_info->txctl_req.len);

		err = dbus_sdio_txctl(sdio_info, sdio_info->txctl_req.buf,
			sdio_info->txctl_req.len);

		bzero(&sdio_info->txctl_req, sizeof(sdio_info->txctl_req));
		dbus_sdio_ctl_complete(sdio_info, DBUS_CBCTL_WRITE, err);
	}

	return err;
}

int
dbus_sdio_txq_process(void *bus)
{
	sdio_info_t *sdio_info = bus;
	uint framecnt = 0;		  /* Temporary counter of tx/rx frames */

	if (sdio_info->pub->busstate == DBUS_STATE_DOWN) {
		dbus_sdio_txq_flush(sdio_info);
		goto exit;
	}

	/* Send ctl requests first */
	dbus_sdio_txctlq_process(bus);

	/* If waiting for HTAVAIL, check status */
	if (sdio_info->clkstate == CLK_PENDING) {
		int err;
		uint8 clkctl, devctl = 0;

		/* Read CSR, if clock on switch to AVAIL, else ignore */
		clkctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
			SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, &err);
		if (err) {
			DBUSERR(("%s: error reading CSR: %d\n", __FUNCTION__, err));
			sdio_info->pub->busstate = DBUS_STATE_DOWN;
		}

		DBUSINFO(("DPC: PENDING, devctl 0x%02x clkctl 0x%02x\n", devctl, clkctl));

		if (SBSDIO_HTAV(clkctl)) {
			devctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
				SDIO_FUNC_1, SBSDIO_DEVICE_CTL, &err);
			if (err) {
				DBUSERR(("%s: error reading DEVCTL: %d\n",
				           __FUNCTION__, err));
				sdio_info->pub->busstate = DBUS_STATE_DOWN;
			}
			devctl &= ~SBSDIO_DEVCTL_CA_INT_ONLY;
			dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
				SBSDIO_DEVICE_CTL, devctl, &err);
			if (err) {
				DBUSERR(("%s: error writing DEVCTL: %d\n",
				           __FUNCTION__, err));
				sdio_info->pub->busstate = DBUS_STATE_DOWN;
			}
			sdio_info->clkstate = CLK_AVAIL;
		}
		else {
			goto exit;
		}
	}

	/* Make sure backplane clock is on */
	dbus_sdio_clkctl(sdio_info, CLK_AVAIL, TRUE);
	if (sdio_info->clkstate == CLK_PENDING)
		goto exit;

	/* Send queued frames (limit 1 if rx may still be pending) */
	if ((sdio_info->clkstate != CLK_PENDING) &&
	    pktq_mlen(&sdio_info->txq, ~sdio_info->flowcontrol) && DATAOK(sdio_info)) {
		framecnt = dbus_sdio_sendfromq(sdio_info);
	}

	/* FIX: Check ctl requests again
	 * It's possible to have ctl request while dbus_sdio_sendfromq()
	 * is active.  Possibly check for pending ctl requests before sending
	 * each pkt??
	 */
	dbus_sdio_txctlq_process(sdio_info);

exit:

	return DBUS_OK;
}

/*
 * Interface functions
 */
static int
dbus_sdif_send_ctl(void *bus, uint8 *buf, int len)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	int err = DBUS_OK;

	if (sdio_info == NULL)
		return DBUS_ERR;

	if (sdio_info->txctl_req.pending == TRUE) {
		DBUSERR(("%s: ctl is pending!\n", __FUNCTION__));
		return DBUS_ERR_PENDING;
	}

	sdio_info->txctl_req.buf = buf;
	sdio_info->txctl_req.len = len;
	sdio_info->txctl_req.pending = TRUE;
	if (!CHECK_FLAG(&sdio_info->rxtx_flag))
		dbus_sdio_txq_process(sdio_info);
	return err;
}

static int
dbus_sdif_send_irb(void *bus, dbus_irb_tx_t *txirb)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);
	int err;

	if (sdio_info == NULL)
		return DBUS_ERR;

	err = dbus_sdio_txbuf_submit(sdio_info, txirb);
	if (err != DBUS_OK) {
		err = DBUS_ERR_TXFAIL;
	}

	return err;
}

static void
dbus_sdio_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->send_irb_timeout)
		sdio_info->cbs->send_irb_timeout(sdio_info->cbarg, txirb);
}

static void
dbus_sdio_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->send_irb_complete)
		sdio_info->cbs->send_irb_complete(sdio_info->cbarg, txirb, status);
}

static uint
dbus_sdio_rxfail(sdio_info_t *sdio_info, bool abort, bool rtx)
{
	sdpcmd_regs_t *regs = sdio_info->regs;
	uint retries = 0;
	uint16 lastrbc;
	uint8 hi, lo;
	int err;

	DBUSERR(("%s: %sterminate frame%s\n", __FUNCTION__,
	           (abort ? "abort command, " : ""), (rtx ? ", send NAK" : "")));

	if (sdio_info->pub->busstate != DBUS_STATE_UP) {
		return BCME_NOTUP;
	}

	if (abort) {
		err = dbus_sdos_abort(sdio_info->sdos_info, SDIO_FUNC_2);
		if (err == BCME_NODEVICE) {
			dbus_sdio_state_change(sdio_info, DBUS_STATE_DISCONNECT);
			return err;
		}
	}

	dbus_sdos_cfg_write(sdio_info->sdos_info, SDIO_FUNC_1,
		SBSDIO_FUNC1_FRAMECTRL, SFC_RF_TERM, &err);
	sdio_info->f1regdata++;

	if (err != BCME_OK)
		goto regfail_err;

	/* Wait until the packet has been flushed (device/FIFO stable) */
	for (lastrbc = retries = 0xffff; retries > 0; retries--) {
		hi = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
			SBSDIO_FUNC1_RFRAMEBCHI, &err);
		sdio_info->f1regdata += 1;
		if (err != BCME_OK)
			goto regfail_err;
		lo = dbus_sdos_cfg_read(sdio_info->sdos_info, SDIO_FUNC_1,
			SBSDIO_FUNC1_RFRAMEBCLO, &err);
		sdio_info->f1regdata += 1;
		if (err != BCME_OK)
			goto regfail_err;

		if ((hi == 0) && (lo == 0))
			break;

		if ((hi > (lastrbc >> 8)) && (lo > (lastrbc & 0x00ff))) {
			DBUSERR(("%s: count growing: last 0x%04x now 0x%04x\n",
				__FUNCTION__, lastrbc, ((hi << 8) + lo)));
		}
		lastrbc = (hi << 8) + lo;
	}

	if (!retries) {
		DBUSERR(("%s: count never zeroed: last 0x%04x\n", __FUNCTION__, lastrbc));
	} else {
		DBUSINFO(("%s: flush took %d iterations\n", __FUNCTION__, (0xffff - retries)));
	}

	if (rtx) {
		sdio_info->rxrtx++;
		if ((err = dbus_sdio_reg_write(sdio_info, (uint32)&regs->tosbmailbox,
			sizeof(regs->tosbmailbox), SMB_NAK, REGRETRIES)) == BCME_OK)
			sdio_info->rxskip = TRUE;
		sdio_info->f1regdata++;
	}

	/* Clear partial in any case */
	sdio_info->nextlen = 0;

regfail_err:
	/* If we can't reach the device, signal failure */
	if (err != BCME_OK)
		sdio_info->pub->busstate = DBUS_STATE_DOWN;
	return err;
}

static uint
dbus_sdio_read_control(sdio_info_t *sdio_info, uint8 *hdr, uint len, uint doff)
{
	uint rdlen, pad;

	int sdret;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	/* Control data already received in aligned rxctl */
	if ((sdio_info->bus == SPI_BUS) && (!sdio_info->usebufpool))
		goto gotpkt;

	ASSERT(sdio_info->rxbuf);
	/* Set rxctl for frame (w/optional alignment) */
	sdio_info->rxctl = sdio_info->rxbuf;
	if (dhd_alignctl) {
		sdio_info->rxctl += firstread;
		if ((pad = ((uintptr)sdio_info->rxctl % SDALIGN)))
			sdio_info->rxctl += (SDALIGN - pad);
		sdio_info->rxctl -= firstread;
	}
	ASSERT(sdio_info->rxctl >= sdio_info->rxbuf);

	/* Copy the already-read portion over */
	bcopy(hdr, sdio_info->rxctl, firstread);
	if (len <= firstread)
		goto gotpkt;

	/* Copy the full data pkt in gSPI case and process ioctl. */
	if (sdio_info->bus == SPI_BUS) {
		bcopy(hdr, sdio_info->rxctl, len);
		goto gotpkt;
	}

	/* Raise rdlen to next SDIO block to avoid tail command */
	rdlen = len - firstread;
	if (sdio_info->roundup && sdio_info->blocksize && (rdlen > sdio_info->blocksize)) {
		pad = sdio_info->blocksize - (rdlen % sdio_info->blocksize);
		if ((pad <= sdio_info->roundup) && (pad < sdio_info->blocksize) &&
		    ((len + pad) < sdio_info->maxctl))
			rdlen += pad;
	}

	/* Satisfy length-alignment requirements */
	if (forcealign && (rdlen & (ALIGNMENT - 1)))
		rdlen = ROUNDUP(rdlen, ALIGNMENT);

	/* Drop if the read is too big or it exceeds our maximum */
	if ((rdlen + firstread) > sdio_info->maxctl) {
		DBUSERR(("%s: %d-byte control read exceeds %d-byte buffer\n",
		           __FUNCTION__, rdlen, sdio_info->maxctl));
		sdio_info->pub->stats.rx_errors++;
		dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
		goto done;
	}

	if ((len - doff) > sdio_info->maxctl) {
		DBUSERR(("%s: %d-byte ctl frame (%d-byte ctl data) exceeds %d-byte limit\n",
		           __FUNCTION__, len, (len - doff), sdio_info->maxctl));
		sdio_info->pub->stats.rx_errors++; sdio_info->rx_toolong++;
		dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
		goto done;
	}


	/* Read remainder of frame body into the rxctl buffer */
	sdret = dbus_sdos_recv_buf(sdio_info->sdos_info, SI_ENUM_BASE, SDIO_FUNC_2, F2SYNC,
	                        (sdio_info->rxctl + firstread), rdlen);
	sdio_info->f2rxdata++;
	ASSERT(sdret != BCME_PENDING);

	/* Control frame failures need retransmission */
	if (sdret < 0) {
		DBUSERR(("%s: read %d control bytes failed: %d\n", __FUNCTION__, rdlen, sdret));
		sdio_info->rxc_errors++; /* dhd.rx_ctlerrs is higher level */
		dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
		goto done;
	}

gotpkt:
	/* Point to valid data and indicate its length */
	sdio_info->rxctl += doff;

	if (sdio_info->rxlen != 0) {
		DBUSERR(("dropping previous recv ctl pkt\n"));
	}
	sdio_info->rxlen = len - doff;

	if (sdio_info->cbarg && sdio_info->cbs) {
		if (sdio_info->drvintf && sdio_info->drvintf->lock)
			sdio_info->drvintf->lock(sdio_info->sdos_info);
		if (sdio_info->rxctl_req.pending == TRUE) {
			dbus_sdio_rxctl(sdio_info, sdio_info->rxctl_req.buf,
				sdio_info->rxctl_req.len);
			bzero(&sdio_info->rxctl_req, sizeof(sdio_info->rxctl_req));
			dbus_sdio_ctl_complete(sdio_info, DBUS_CBCTL_READ, DBUS_OK);
		}
		if (sdio_info->drvintf && sdio_info->drvintf->lock)
			sdio_info->drvintf->unlock(sdio_info->sdos_info);
	}
	return BCME_OK;
done:
	return BCME_ERROR;
}

static uint
dbus_sdio_rxglom(sdio_info_t *sdio_info, uint8 rxseq, uint *num_frames)
{
	uint16 dlen, totlen;
	uint8 *dptr, num = 0;

	uint16 sublen, check;
	void *pfirst, *plast, *pnext, *save_pfirst;
	osl_t *osh = sdio_info->pub->osh;

	int errcode;
	uint8 chan, seq, doff, sfdoff;
	uint16 txmax;

	bool usechain = sdio_info->use_rxchain;

	/* If packets, issue read(s) and send up packet chain */
	/* Return sequence numbers consumed? */

	DBUSTRACE(("dbus_sdio_rxglom: start: glomd %p glom %p\n",
		sdio_info->glomd, sdio_info->glom));

	/* If there's a descriptor, generate the packet chain */
	if (sdio_info->glomd) {
		dlen = (uint16)PKTLEN(osh, sdio_info->glomd);
		dptr = PKTDATA(osh, sdio_info->glomd);
		if (!dlen || (dlen & 1)) {
			DBUSERR(("%s: bad glomd len (%d), toss descriptor\n",
			           __FUNCTION__, dlen));
			dbus_sdio_pktfree(sdio_info, sdio_info->glomd, FALSE);
			sdio_info->glomd = NULL;
			sdio_info->nextlen = 0;
			*num_frames = 0;
			return BCME_ERROR;
		}

		pfirst = plast = pnext = NULL;

		for (totlen = num = 0; dlen; num++) {
			/* Get (and move past) next length */
			sublen = ltoh16_ua(dptr);
			dlen -= sizeof(uint16);
			dptr += sizeof(uint16);
			if ((sublen < SDPCM_HDRLEN) ||
			    ((num == 0) && (sublen < (2 * SDPCM_HDRLEN)))) {
				DBUSERR(("%s: desciptor len %d bad: %d\n",
				           __FUNCTION__, num, sublen));
				pnext = NULL;
				break;
			}
			if (sublen % SDALIGN) {
				DBUSERR(("%s: sublen %d not a multiple of %d\n",
				           __FUNCTION__, sublen, SDALIGN));
				usechain = FALSE;
			}
			totlen += sublen;

			/* For last frame, adjust read len so total is a block multiple */
			if (!dlen) {
				sublen += (ROUNDUP(totlen, sdio_info->blocksize) - totlen);
				totlen = ROUNDUP(totlen, sdio_info->blocksize);
			}

			/* Allocate/chain packet for next subframe */
			if ((pnext = dbus_sdio_pktget(sdio_info,
				sublen + SDALIGN, FALSE)) == NULL) {
				DBUSERR(("%s: dbus_sdio_pktget failed, num %d len %d\n",
				           __FUNCTION__, num, sublen));
				break;
			}
			ASSERT(!PKTLINK(pnext));
			if (!pfirst) {
				ASSERT(!plast);
				pfirst = plast = pnext;
			} else {
				ASSERT(plast);
				PKTSETNEXT(osh, plast, pnext);
				plast = pnext;
			}

			/* Adhere to start alignment requirements */
			PKTALIGN(osh, pnext, sublen, SDALIGN);
		}

		/* If allocation failed, toss entirely and increment count */
		if (!pnext) {
			if (pfirst)
				dbus_sdio_pktfree(sdio_info, pfirst, FALSE);
			dbus_sdio_pktfree(sdio_info, sdio_info->glomd, FALSE);
			sdio_info->glomd = NULL;
			sdio_info->nextlen = 0;
			*num_frames = 0;
			return BCME_ERROR;
		}

		/* Ok, we have a packet chain, save in bus structure */
		DBUSGLOM(("%s: allocated %d-byte packet chain for %d subframes\n",
		          __FUNCTION__, totlen, num));
		if (DBUSGLOM_ON() && sdio_info->nextlen) {
			if (totlen != sdio_info->nextlen) {
				DBUSGLOM(("%s: glomdesc mismatch: nextlen %d glomdesc %d "
				          "rxseq %d\n", __FUNCTION__, sdio_info->nextlen,
				          totlen, rxseq));
			}
		}
		sdio_info->glom = pfirst;

		/* Done with descriptor packet */
		dbus_sdio_pktfree(sdio_info, sdio_info->glomd, FALSE);
		sdio_info->glomd = NULL;
		sdio_info->nextlen = 0;
	}

	/* Ok -- either we just generated a packet chain, or had one from before */
	if (sdio_info->glom) {
		if (DBUSGLOM_ON()) {
			DBUSGLOM(("%s: attempt superframe read, packet chain:\n", __FUNCTION__));
			for (pnext = sdio_info->glom; pnext; pnext = PKTNEXT(osh, pnext)) {
				DBUSGLOM(("    %p: %p len 0x%04x (%d)\n",
				          pnext, (uint8*)PKTDATA(osh, pnext),
				          PKTLEN(osh, pnext), PKTLEN(osh, pnext)));
			}
		}

		pfirst = sdio_info->glom;
		dlen = (uint16)pkttotlen(osh, pfirst);

		/* Do an SDIO read for the superframe.  Configurable iovar to
		 * read directly into the chained packet, or allocate a large
		 * packet and and copy into the chain.
		 */
		if (usechain) {
			errcode = dbus_sdos_recv_buf(sdio_info->sdos_info,
				SI_ENUM_BASE, SDIO_FUNC_2, F2SYNC,
				(uint8*)PKTDATA(osh, pfirst), dlen);
		} else if (sdio_info->dataptr) {
			errcode = dbus_sdos_recv_buf(sdio_info->sdos_info,
				SI_ENUM_BASE, SDIO_FUNC_2,
				F2SYNC, sdio_info->dataptr, dlen);
			sublen = (uint16)pktfrombuf(osh, pfirst, 0, dlen, sdio_info->dataptr);
			if (sublen != dlen) {
				DBUSERR(("%s: FAILED TO COPY, dlen %d sublen %d\n",
					__FUNCTION__, dlen, sublen));
				errcode = -1;
			}
			pnext = NULL;
		} else {
			DBUSERR(("COULDN'T ALLOC %d-BYTE GLOM, FORCE FAILURE\n", dlen));
			errcode = -1;
		}
		sdio_info->f2rxdata++;
		ASSERT(errcode != BCME_PENDING);

		/* On failure, kill the superframe, allow a couple retries */
		if (errcode < 0) {
			DBUSERR(("%s: glom read of %d bytes failed: %d\n",
			           __FUNCTION__, dlen, errcode));
			sdio_info->pub->stats.rx_errors++;

			if (sdio_info->glomerr++ < 3) {
				*num_frames = 0;
				return dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
			} else {
				sdio_info->glomerr = 0;
				dbus_sdio_rxfail(sdio_info, TRUE, FALSE);
				dbus_sdio_pktfree(sdio_info, sdio_info->glom, FALSE);
				sdio_info->rxglomfail++;
				sdio_info->glom = NULL;
			}
			OSL_DELAY(dlen/128);
			*num_frames = 0;
			return BCME_ERROR;
		}

#ifdef BCMDBG
		if (DBUSGLOM_ON()) {
			prhex("SUPERFRAME", PKTDATA(osh, pfirst),
			      MIN(PKTLEN(osh, pfirst), 48));
		}
#endif


		/* Validate the superframe header */
		dptr = (uint8 *)PKTDATA(osh, pfirst);
		sublen = ltoh16_ua(dptr);
		check = ltoh16_ua(dptr + sizeof(uint16));

		chan = SDPCM_PACKET_CHANNEL(&dptr[SDPCM_FRAMETAG_LEN]);
		seq = SDPCM_PACKET_SEQUENCE(&dptr[SDPCM_FRAMETAG_LEN]);
		sdio_info->nextlen = dptr[SDPCM_FRAMETAG_LEN + SDPCM_NEXTLEN_OFFSET];
		if ((sdio_info->nextlen << 4) > MAX_RX_DATASZ) {
			DBUSINFO(("%s: got frame w/nextlen too large (%d) seq %d\n",
			          __FUNCTION__, sdio_info->nextlen, seq));
			sdio_info->nextlen = 0;
		}
		doff = SDPCM_DOFFSET_VALUE(&dptr[SDPCM_FRAMETAG_LEN]);
		txmax = SDPCM_WINDOW_VALUE(&dptr[SDPCM_FRAMETAG_LEN]);

		errcode = 0;
		if ((uint16)~(sublen^check)) {
			DBUSERR(("%s (superframe): HW hdr error: len/check 0x%04x/0x%04x\n",
			           __FUNCTION__, sublen, check));
			errcode = -1;
		} else if (ROUNDUP(sublen, sdio_info->blocksize) != dlen) {
			DBUSERR(("%s (superframe): len 0x%04x, rounded 0x%04x, expect 0x%04x\n",
				__FUNCTION__, sublen,
				ROUNDUP(sublen, sdio_info->blocksize), dlen));
			errcode = -1;
		} else if (SDPCM_PACKET_CHANNEL(&dptr[SDPCM_FRAMETAG_LEN]) != SDPCM_GLOM_CHANNEL) {
			DBUSERR(("%s (superframe): bad channel %d\n", __FUNCTION__,
			           SDPCM_PACKET_CHANNEL(&dptr[SDPCM_FRAMETAG_LEN])));
			errcode = -1;
		} else if (SDPCM_GLOMDESC(&dptr[SDPCM_FRAMETAG_LEN])) {
			DBUSERR(("%s (superframe): got second descriptor?\n", __FUNCTION__));
			errcode = -1;
		} else if ((doff < SDPCM_HDRLEN) ||
		           (doff > (PKTLEN(osh, pfirst) - SDPCM_HDRLEN))) {
			DBUSERR(("%s (superframe): Bad data offset %d: HW %d pkt %d min %d\n",
			           __FUNCTION__, doff, sublen, PKTLEN(osh, pfirst), SDPCM_HDRLEN));
			errcode = -1;
		}

		/* Check sequence number of superframe SW header */
		if (rxseq != seq) {
			DBUSINFO(("%s: (superframe) rx_seq %d, expected %d\n",
			          __FUNCTION__, seq, rxseq));
			sdio_info->rx_badseq++;
			rxseq = seq;
		}

		if (txmax < sdio_info->tx_seq) {
			txmax += SDPCM_SEQUENCE_WRAP;
		}

		sdio_info->tx_max = txmax;

		/* Remove superframe header, remember offset */
		PKTPULL(osh, pfirst, doff);
		sfdoff = doff;

		/* Validate all the subframe headers */
		for (num = 0, pnext = pfirst; pnext && !errcode;
		     num++, pnext = PKTNEXT(osh, pnext)) {
			dptr = (uint8 *)PKTDATA(osh, pnext);
			dlen = (uint16)PKTLEN(osh, pnext);
			sublen = ltoh16_ua(dptr);
			check = ltoh16_ua(dptr + sizeof(uint16));
			chan = SDPCM_PACKET_CHANNEL(&dptr[SDPCM_FRAMETAG_LEN]);
			doff = SDPCM_DOFFSET_VALUE(&dptr[SDPCM_FRAMETAG_LEN]);
#ifdef BCMDBG
			if (DBUSGLOM_ON()) {
				prhex("subframe", dptr, 32);
			}
#endif

			if ((uint16)~(sublen^check)) {
				DBUSERR(("%s (subframe %d): HW hdr error: "
				           "len/check 0x%04x/0x%04x\n",
				           __FUNCTION__, num, sublen, check));
				errcode = -1;
			} else if ((sublen > dlen) || (sublen < SDPCM_HDRLEN)) {
				DBUSERR(("%s (subframe %d): length mismatch: "
				           "len 0x%04x, expect 0x%04x\n",
				           __FUNCTION__, num, sublen, dlen));
				errcode = -1;
			} else if ((chan != SDPCM_DATA_CHANNEL) &&
			           (chan != SDPCM_EVENT_CHANNEL)) {
				DBUSERR(("%s (subframe %d): bad channel %d\n",
				           __FUNCTION__, num, chan));
				errcode = -1;
			} else if ((doff < SDPCM_HDRLEN) || (doff > sublen)) {
				DBUSERR(("%s (subframe %d): Bad data offset %d: HW %d min %d\n",
				           __FUNCTION__, num, doff, sublen, SDPCM_HDRLEN));
				errcode = -1;
			}
		}

		if (errcode) {
			/* Terminate frame on error, request a couple retries */
			if (sdio_info->glomerr++ < 3) {
				/* Restore superframe header space */
				PKTPUSH(osh, pfirst, sfdoff);
				sdio_info->nextlen = 0;
				*num_frames = 0;
				return dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
			} else {
				sdio_info->glomerr = 0;
				dbus_sdio_rxfail(sdio_info, TRUE, FALSE);
				dbus_sdio_pktfree(sdio_info, sdio_info->glom, FALSE);
				sdio_info->rxglomfail++;
				sdio_info->glom = NULL;
			}
			sdio_info->nextlen = 0;
			*num_frames = 0;
			return BCME_ERROR;
		}

		/* Basic SD framing looks ok - process each packet (header) */
		save_pfirst = pfirst;
		sdio_info->glom = NULL;
		plast = NULL;

		for (num = 0; pfirst; rxseq++, pfirst = pnext) {
			pnext = PKTNEXT(osh, pfirst);
			PKTSETNEXT(osh, pfirst, NULL);

			dptr = (uint8 *)PKTDATA(osh, pfirst);
			sublen = ltoh16_ua(dptr);
			chan = SDPCM_PACKET_CHANNEL(&dptr[SDPCM_FRAMETAG_LEN]);
			seq = SDPCM_PACKET_SEQUENCE(&dptr[SDPCM_FRAMETAG_LEN]);
			doff = SDPCM_DOFFSET_VALUE(&dptr[SDPCM_FRAMETAG_LEN]);

			DBUSGLOM(("%s: Get subframe %d, %p(%p/%d), sublen %d chan %d seq %d\n",
			          __FUNCTION__, num, pfirst, PKTDATA(osh, pfirst),
			          PKTLEN(osh, pfirst), sublen, chan, seq));

			ASSERT((chan == SDPCM_DATA_CHANNEL) || (chan == SDPCM_EVENT_CHANNEL));

			if (rxseq != seq) {
				DBUSGLOM(("%s: rx_seq %d, expected %d\n",
				          __FUNCTION__, seq, rxseq));
				sdio_info->rx_badseq++;
				rxseq = seq;
			}

			PKTSETLEN(osh, pfirst, sublen);
			PKTPULL(osh, pfirst, doff);

			if (PKTLEN(osh, pfirst) == 0) {
				dbus_sdio_pktfree(sdio_info, pfirst, FALSE);
				if (plast) {
					PKTSETNEXT(osh, plast, pnext);
				} else {
					ASSERT(save_pfirst == pfirst);
					save_pfirst = pnext;
				}
				continue;
			}

			/* this packet will go up, link back into chain and count it */
			PKTSETNEXT(osh, pfirst, pnext);
			plast = pfirst;
			num++;

#ifdef BCMDBG
			if (DBUSGLOM_ON()) {
				DBUSGLOM(("%s subframe %d to stack, %p(%p/%d) nxt/lnk %p/%p\n",
				          __FUNCTION__, num, pfirst,
				          PKTDATA(osh, pfirst), PKTLEN(osh, pfirst),
				          PKTNEXT(osh, pfirst), PKTLINK(pfirst)));
				prhex("", (uint8 *)PKTDATA(osh, pfirst),
				      MIN(PKTLEN(osh, pfirst), 32));
			}
#endif /* BCMDBG */
		}

		{
			int i;
			void *pnext;
			void *plist;
			dbus_irb_rx_t *rxirb;

			plist = save_pfirst;
			for (i = 0; plist && i < num; i++, plist = pnext) {
				pnext = PKTNEXT(osh, plist);
				PKTSETNEXT(osh, plist, NULL);

				rxirb = (dbus_irb_rx_t *) dbus_sdio_getirb(sdio_info, FALSE);
				if (rxirb != NULL) {
					rxirb->pkt = plist;
					dbus_sdio_recv_irb_complete(sdio_info, rxirb, DBUS_OK);
				} else {
					ASSERT(0); /* FIX: Handle this case */
				}
			}
		}

		sdio_info->rxglomframes++;
		sdio_info->rxglompkts += num;
	}

	*num_frames = num;
	return BCME_OK;
}


/* Return TRUE if there may be more frames to read */
static uint
dbus_sdio_readframes(sdio_info_t *sdio_info, bool *finished)
{
	uint16 len, check;	/* Extracted hardware header fields */
	uint8 chan, seq, doff;	/* Extracted software header fields */
	uint8 fcbits;		/* Extracted fcbits from software header */
	uint8 delta;

	void *pkt;	/* Packet for event or data frames */
	uint16 pad;	/* Number of pad bytes to read */
	uint16 rdlen;	/* Total number of bytes to read */
	uint8 *rxbuf;
	int sdret;	/* Return code from bcmsdh calls */
	uint16 txmax;	/* Maximum tx sequence offered */
	dbus_irb_rx_t *rxirb;
	uint16 nextlen = 0;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	do {

		if (!(pkt = dbus_sdio_pktget(sdio_info, PKTBUFSZ, FALSE))) {
			/* Give up on data, request rtx of events */
			DBUSERR(("%s (nextlen): dbus_sdio_pktget failed: rdlen %d "
				"expected rxseq %d\n",
				__FUNCTION__, rdlen, sdio_info->rx_seq));
			return BCME_NOMEM;
		}

		if (dhd_readahead && sdio_info->nextlen) {

			nextlen = sdio_info->nextlen;
			sdio_info->nextlen = 0;

			rdlen = nextlen << 4;

			/* Pad read to blocksize for efficiency */
			if (sdio_info->roundup && sdio_info->blocksize &&
				(rdlen > sdio_info->blocksize)) {
				pad = sdio_info->blocksize - (rdlen % sdio_info->blocksize);
				if ((pad <= sdio_info->roundup) &&
					(pad < sdio_info->blocksize) &&
					((rdlen + pad + firstread) < MAX_RX_DATASZ))
					rdlen += pad;
			}

			ASSERT(!PKTLINK(pkt));
			PKTALIGN(sdio_info->pub->osh, pkt, rdlen, SDALIGN);
			rxbuf = (uint8 *)PKTDATA(sdio_info->pub->osh, pkt);
			/* Read the entire frame */
			sdret = dbus_sdos_recv_buf(sdio_info->sdos_info, SI_ENUM_BASE,
				SDIO_FUNC_2, F2SYNC, rxbuf, rdlen);
				sdio_info->f2rxdata++;
			ASSERT(sdret != BCME_PENDING);

			if (bcmsdh_get_dstatus((void *)sdio_info->sdos_info) &
				STATUS_UNDERFLOW) {
				sdio_info->nextlen = 0;
				*finished = TRUE;
				DBUSERR(("%s (nextlen): read %d bytes failed due "
					"to spi underflow\n", __FUNCTION__, rdlen));
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				sdio_info->pub->stats.rx_errors++;
				return BCME_ERROR;
			}

			if (sdret < 0) {
				DBUSERR(("%s (nextlen): read %d bytes failed: %d\n",
					__FUNCTION__, rdlen, sdret));
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				sdio_info->pub->stats.rx_errors++;
				return dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
			}

			/* Now check the header */
			bcopy(rxbuf, sdio_info->rxhdr, SDPCM_HDRLEN);

		} else {
			/* Read frame header (hardware and software) */
			sdret = dbus_sdos_recv_buf(sdio_info->sdos_info, SI_ENUM_BASE,
				SDIO_FUNC_2, F2SYNC, sdio_info->rxhdr, firstread);
			sdio_info->f2rxhdrs++;
			ASSERT(sdret != BCME_PENDING);

			if (sdret < 0) {
				DBUSERR(("%s: RXHEADER FAILED: %d\n", __FUNCTION__, sdret));
				sdio_info->rx_hdrfail++;
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				return dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
			}
		}

		/* Extract hardware header fields */
		len = ltoh16_ua(sdio_info->rxhdr);
		check = ltoh16_ua(sdio_info->rxhdr + sizeof(uint16));

		/* All zeros means no more frames */
		if (!(len|check)) {
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			*finished = TRUE;
			return BCME_OK;
		}

		/* Validate check bytes */
		if ((uint16)~(len^check)) {
			DBUSERR(("%s: HW hdr error: len/check 0x%04x/0x%04x\n",
				__FUNCTION__, len, check));
			sdio_info->rx_badhdr++;
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			return dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
		}

		/* Validate frame length */

		if ((len < SDPCM_HDRLEN) ||
			((nextlen > 0) && (nextlen != (ROUNDUP(len, 16) >> 4)))) {
			/* Mismatch, force retry w/normal header (may be >4K) */
			DBUSERR(("%s (nextlen): mismatch, nextlen %d len %d rnd %d; "
				"expected rxseq %d\n", __FUNCTION__, nextlen, len,
				ROUNDUP(len, 16), sdio_info->rx_seq));
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			return dbus_sdio_rxfail(sdio_info, TRUE, TRUE);
		}

		/* Extract software header fields */
		chan = SDPCM_PACKET_CHANNEL(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN]);
		seq = SDPCM_PACKET_SEQUENCE(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN]);
		doff = SDPCM_DOFFSET_VALUE(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN]);
		txmax = SDPCM_WINDOW_VALUE(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN]);

		/* Validate data offset */
		if ((doff < SDPCM_HDRLEN) || (doff > len)) {
			DBUSERR(("%s: Bad data offset %d: HW len %d, min %d seq %d\n",
				__FUNCTION__, doff, len, SDPCM_HDRLEN, seq));
			sdio_info->rx_badhdr++;
			ASSERT(0);
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			return dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
		}

		sdio_info->rx_readahead_cnt += (nextlen > 0) ? 1 : 0;

		/* Save the readahead length if there is one */
		sdio_info->nextlen = sdio_info->rxhdr[SDPCM_FRAMETAG_LEN + SDPCM_NEXTLEN_OFFSET];
		if ((sdio_info->nextlen << 4) > MAX_RX_DATASZ) {
			DBUSINFO(("%s (nextlen): got frame w/nextlen too large (%d), seq %d\n",
				__FUNCTION__, sdio_info->nextlen, seq));
			sdio_info->nextlen = 0;
		}

		/* Handle Flow Control */
		fcbits = SDPCM_FCMASK_VALUE(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN]);

		delta = 0;

		if (~sdio_info->flowcontrol & fcbits) {
			sdio_info->fc_xoff++;
			delta = 1;
		}

		if (sdio_info->flowcontrol & ~fcbits) {
			sdio_info->fc_xon++;
			delta = 1;
		}

		if (delta) {
			sdio_info->fc_rcvd++;
			sdio_info->flowcontrol = fcbits;
		}

		/* Check and update sequence number */
		if (sdio_info->rx_seq != seq) {
			DBUSINFO(("%s: rx_seq %d, expected %d\n",
				__FUNCTION__, seq, sdio_info->rx_seq));
			sdio_info->rx_badseq++;
			sdio_info->rx_seq = seq;
		}

		if (txmax < sdio_info->tx_seq) {
			txmax += SDPCM_SEQUENCE_WRAP;
		}

		sdio_info->tx_max = txmax;

		/* Call a separate function for control frames */
		if (chan == SDPCM_CONTROL_CHANNEL) {
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			if (dbus_sdio_read_control(sdio_info, sdio_info->rxhdr, len, doff) !=
				BCME_OK) {
				return BCME_ERROR;
			}
			continue;
		}

		ASSERT((chan == SDPCM_DATA_CHANNEL) || (chan == SDPCM_EVENT_CHANNEL) ||
			(chan == SDPCM_TEST_CHANNEL) || (chan == SDPCM_GLOM_CHANNEL));

		if (nextlen == 0) {

			/* Length to read */
			rdlen = (len > firstread) ? (len - firstread) : 0;

			/* May pad read to blocksize for efficiency */
			if (sdio_info->roundup && sdio_info->blocksize &&
				(rdlen > sdio_info->blocksize)) {
				pad = sdio_info->blocksize - (rdlen % sdio_info->blocksize);
				if ((pad <= sdio_info->roundup) && (pad < sdio_info->blocksize) &&
					((rdlen + pad + firstread) < MAX_RX_DATASZ))
					rdlen += pad;
			}

			/* Satisfy length-alignment requirements */
			if (forcealign && (rdlen & (ALIGNMENT - 1)))
				rdlen = ROUNDUP(rdlen, ALIGNMENT);

			if ((rdlen + firstread) > MAX_RX_DATASZ) {
				/* Too long -- skip this frame */
				DBUSERR(("%s: too long: len %d rdlen %d\n",
					__FUNCTION__, len, rdlen));
				sdio_info->pub->stats.rx_errors++; sdio_info->rx_toolong++;
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				return dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
			}

			/* Leave room for what we already read, and align remainder */
			ASSERT(firstread < (PKTLEN(sdio_info->pub->osh, pkt)));
			PKTPULL(sdio_info->pub->osh, pkt, firstread);
			PKTALIGN(sdio_info->pub->osh, pkt, rdlen, SDALIGN);

			/* Read the remaining frame data */
			sdret = dbus_sdos_recv_buf(sdio_info->sdos_info, SI_ENUM_BASE,
				SDIO_FUNC_2, F2SYNC, ((uint8 *)PKTDATA(osh, pkt)), rdlen);
			sdio_info->f2rxdata++;
			ASSERT(sdret != BCME_PENDING);

			if (sdret < 0) {
				DBUSERR(("%s: read %d %s bytes failed: %d\n", __FUNCTION__, rdlen,
					((chan == SDPCM_EVENT_CHANNEL) ? "event" :
					((chan == SDPCM_DATA_CHANNEL) ? "data" : "test")), sdret));
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				sdio_info->pub->stats.rx_errors++;
				return dbus_sdio_rxfail(sdio_info, TRUE, RETRYCHAN(chan));
			}

			/* Copy the already-read portion */
			PKTPUSH(sdio_info->pub->osh, pkt, firstread);
			bcopy(sdio_info->rxhdr, PKTDATA(sdio_info->pub->osh, pkt), firstread);
		}

		/* Save superframe descriptor and allocate packet frame */
		if (chan == SDPCM_GLOM_CHANNEL) {
			if (SDPCM_GLOMDESC(&sdio_info->rxhdr[SDPCM_FRAMETAG_LEN])) {
				DBUSGLOM(("%s: got glom descriptor, %d bytes:\n",
					__FUNCTION__, len));
				DBUSINFO(("Glom Data", PKTDATA(sdio_info->pub->osh, pkt), len));
				PKTSETLEN(sdio_info->pub->osh, pkt, len);
				ASSERT(doff == SDPCM_HDRLEN);
				PKTPULL(sdio_info->pub->osh, pkt, SDPCM_HDRLEN);
				sdio_info->glomd = pkt;
			} else {
				DBUSERR(("%s: glom superframe w/o descriptor!\n", __FUNCTION__));
				dbus_sdio_pktfree(sdio_info, pkt, FALSE);
				return dbus_sdio_rxfail(sdio_info, FALSE, FALSE);
			}
			return BCME_OK;
		}

		/* Fill in packet len and prio, deliver upward */
		PKTSETLEN(sdio_info->pub->osh, pkt, len);
		PKTPULL(sdio_info->pub->osh, pkt, doff);

		if (PKTLEN(sdio_info->pub->osh, pkt) == 0) {
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
			return BCME_OK;
		}

		rxirb = (dbus_irb_rx_t *) dbus_sdio_getirb(sdio_info, FALSE);
		if (rxirb != NULL) {
			rxirb->pkt = pkt;
			dbus_sdio_recv_irb_complete(sdio_info, rxirb, DBUS_OK);
		} else {
			DBUSERR(("ERROR: failed to get rx irb\n"));
			dbus_sdio_pktfree(sdio_info, pkt, FALSE);
		}

		sdio_info->rx_seq++;

	} while (dhd_readahead && sdio_info->nextlen && !sdio_info->halting &&
		(sdio_info->pub->busstate != DBUS_STATE_DOWN));

	return BCME_OK;
}

static uint32
dbus_sdio_hostmail(sdio_info_t *sdio_info)
{
	sdpcmd_regs_t *regs = sdio_info->regs;
	uint32 intstatus = 0;
	uint32 hmb_data;
	uint8 fcbits;
	uint retries = 0;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	/* Read mailbox data and ack that we did so */
	if (dbus_sdio_reg_read(sdio_info, (uint32)&regs->tohostmailboxdata,
		sizeof(regs->tohostmailboxdata), &hmb_data, REGRETRIES) == BCME_OK) {
		dbus_sdio_reg_write(sdio_info, (uint32)&regs->tosbmailbox,
			sizeof(regs->tosbmailbox), SMB_INT_ACK, REGRETRIES);
	}
	sdio_info->f1regdata += 2;

	/* Dongle recomposed rx frames, accept them again */
	if (hmb_data & HMB_DATA_NAKHANDLED) {
		DBUSINFO(("Dongle reports NAK handled, expect rtx of %d\n", sdio_info->rx_seq));
		if (!sdio_info->rxskip) {
			DBUSERR(("%s: unexpected NAKHANDLED!\n", __FUNCTION__));
		}
		sdio_info->rxskip = FALSE;
		intstatus |= I_HMB_FRAME_IND;
	}

	/*
	 * Not using DEVREADY or FWREADY at the moment; just print.
	 * DEVREADY does not occur with gSPI.
	 */
	if (hmb_data & (HMB_DATA_DEVREADY | HMB_DATA_FWREADY)) {
		sdio_info->sdpcm_ver = (hmb_data & HMB_DATA_VERSION_MASK) >> HMB_DATA_VERSION_SHIFT;
		if (sdio_info->sdpcm_ver != SDPCM_PROT_VERSION)
			DBUSERR(("Version mismatch, dongle reports %d, expecting %d\n",
			           sdio_info->sdpcm_ver, SDPCM_PROT_VERSION));
		else
			DBUSINFO(("Dongle ready, protocol version %d\n", sdio_info->sdpcm_ver));
	}

	/*
	 * Flow Control has been moved into the RX headers and this out of band
	 * method isn't used any more.  Leae this here for possibly remaining backward
	 * compatible with older dongles
	 */
	if (hmb_data & HMB_DATA_FC) {
		fcbits = (hmb_data & HMB_DATA_FCDATA_MASK) >> HMB_DATA_FCDATA_SHIFT;

		if (fcbits & ~sdio_info->flowcontrol)
			sdio_info->fc_xoff++;
		if (sdio_info->flowcontrol & ~fcbits)
			sdio_info->fc_xon++;

		sdio_info->fc_rcvd++;
		sdio_info->flowcontrol = fcbits;
	}

	/* Shouldn't be any others */
	if (hmb_data & ~(HMB_DATA_DEVREADY |
	                 HMB_DATA_NAKHANDLED |
	                 HMB_DATA_FC |
	                 HMB_DATA_FWREADY |
	                 HMB_DATA_FCDATA_MASK |
	                 HMB_DATA_VERSION_MASK)) {
		DBUSERR(("Unknown mailbox data content: 0x%02x\n", hmb_data));
	}

	return intstatus;
}

static int
dbus_sdio_rxctl(sdio_info_t *sdio_info, uchar *msg, uint msglen)
{
	uint rxlen = 0;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	if (sdio_info->dongle_reset)
		return DBUS_ERR;

	/* FIX: Since rxctl() is async, need to fix case where ctl pkt is recevied
	 * before this function is called.  We need to buffer incoming ctl pkts.
	 */
	rxlen = sdio_info->rxlen;
	bcopy(sdio_info->rxctl, msg, MIN(msglen, rxlen));
	sdio_info->rxlen = 0;

	return DBUS_OK;
}

static int
dbus_sdif_recv_ctl(void *bus, uint8 *buf, int len)
{
	sdio_info_t *sdio_info = BUS_INFO(bus, sdio_info_t);

	if (sdio_info == NULL)
		return DBUS_ERR;

	if (sdio_info->drvintf && sdio_info->drvintf->lock)
		sdio_info->drvintf->lock(sdio_info->sdos_info);

	if (sdio_info->rxctl_req.pending == TRUE) {
		DBUSERR(("%s: ctl is pending!\n", __FUNCTION__));
		if (sdio_info->drvintf && sdio_info->drvintf->unlock)
			sdio_info->drvintf->unlock(sdio_info->sdos_info);
		return DBUS_ERR_PENDING;
	}

	/* Do have a rxctl pkt available? */
	if (sdio_info->rxlen > 0) {
		dbus_sdio_rxctl(sdio_info, buf, len);
		dbus_sdio_ctl_complete(sdio_info, DBUS_CBCTL_READ, DBUS_OK);
	} else {
		sdio_info->rxctl_req.buf = buf;
		sdio_info->rxctl_req.len = len;
		sdio_info->rxctl_req.pending = TRUE;
	}

	if (sdio_info->drvintf && sdio_info->drvintf->unlock)
		sdio_info->drvintf->unlock(sdio_info->sdos_info);

	return DBUS_OK;
}

static void
dbus_sdio_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->recv_irb_complete)
		sdio_info->cbs->recv_irb_complete(sdio_info->cbarg, rxirb, status);
}

static void
dbus_sdio_ctl_complete(void *handle, int type, int status)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;

	if (sdio_info == NULL)
		return;

	if (sdio_info->cbs && sdio_info->cbs->ctl_complete)
		sdio_info->cbs->ctl_complete(sdio_info->cbarg, type, status);
}

static bool
dbus_sdio_dpc(void *handle, bool bounded)
{
	sdio_info_t *sdio_info = (sdio_info_t *) handle;
	sdpcmd_regs_t *regs;
	uint32 newstatus = 0;

	bool rxdone = TRUE;		  /* Flag for no more read data */

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	if (sdio_info == NULL) {
		DBUSERR(("%s: sdio_info == NULL!\n", __FUNCTION__));
		return FALSE;
	}

	regs = sdio_info->regs;

	/* If waiting for HTAVAIL, check status */
	if (sdio_info->clkstate == CLK_PENDING) {
		int err;
		uint8 clkctl, devctl = 0;

		/* Read CSR, if clock on switch to AVAIL, else ignore */
		clkctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
			SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, &err);
		if (err) {
			DBUSERR(("%s: error reading CSR: %d\n", __FUNCTION__, err));
			sdio_info->pub->busstate = DBUS_STATE_DOWN;
		}

		DBUSINFO(("DPC: PENDING, devctl 0x%02x clkctl 0x%02x\n", devctl, clkctl));

		if (SBSDIO_HTAV(clkctl)) {
			devctl = dbus_sdos_cfg_read(sdio_info->sdos_info,
				SDIO_FUNC_1, SBSDIO_DEVICE_CTL, &err);
			if (err) {
				DBUSERR(("%s: error reading DEVCTL: %d\n",
				           __FUNCTION__, err));
				sdio_info->pub->busstate = DBUS_STATE_DOWN;
			}
			devctl &= ~SBSDIO_DEVCTL_CA_INT_ONLY;
			dbus_sdos_cfg_write(sdio_info->sdos_info,
				SDIO_FUNC_1, SBSDIO_DEVICE_CTL, devctl, &err);
			if (err) {
				DBUSERR(("%s: error writing DEVCTL: %d\n",
				           __FUNCTION__, err));
				sdio_info->pub->busstate = DBUS_STATE_DOWN;
			}
			sdio_info->clkstate = CLK_AVAIL;
		} else {
			goto clkwait;
		}
	}

	/* Make sure backplane clock is on */
	dbus_sdio_clkctl(sdio_info, CLK_AVAIL, TRUE);
	if (sdio_info->clkstate == CLK_PENDING)
		goto clkwait;

	/* Pending interrupt indicates new device status */
	if (dbus_sdio_reg_read(sdio_info, (uint32)&sdio_info->regs->intstatus,
		sizeof(sdio_info->regs->intstatus), &newstatus, REGRETRIES) != BCME_OK)
		newstatus = 0;
	sdio_info->f1regdata++;

	newstatus &= sdio_info->hostintmask;

	/* On frame indication, read available frames */
	if (newstatus & I_HMB_FRAME_IND) {
		newstatus = newstatus & I_HMB_FRAME_IND;
		dbus_sdio_reg_write(sdio_info, (uint32)&regs->intstatus,
			sizeof(regs->intstatus), newstatus, REGRETRIES);
		sdio_info->f1regdata++;

		/* Not finished unless we encounter no more frames indication */
		rxdone = FALSE;
		SET_FLAG(&sdio_info->rxtx_flag);
		while (!sdio_info->halting && (sdio_info->pub->busstate != DBUS_STATE_DOWN)) {

			/* Handle glomming separately */
			if (sdio_info->glom || sdio_info->glomd) {
				uint framecnt = 0;
				DBUSGLOM(("%s: calling rxglom: glomd %p, glom %p\n",
					__FUNCTION__, sdio_info->glomd, sdio_info->glom));
				rxdone = (dbus_sdio_rxglom(sdio_info,
					sdio_info->rx_seq, &framecnt) != BCME_OK);
				sdio_info->rx_seq += framecnt - 1;
				DBUSGLOM(("%s: rxglom returned %d\n", __FUNCTION__, framecnt));
			} else {
				if (dbus_sdio_readframes(sdio_info, &rxdone) != BCME_OK)
					break;
			}

			if (rxdone)
				break;

			if (pktq_mlen(&sdio_info->txq, ~sdio_info->flowcontrol) > 0) {
				dbus_sdio_txq_process(sdio_info);
			}
		}

		CLEAR_FLAG(&sdio_info->rxtx_flag);

		/* Back off rxseq if awaiting rtx, upate rx_seq */
		if (sdio_info->rxskip)
			sdio_info->rx_seq--;

	} else {
		dbus_sdio_reg_write(sdio_info, (uint32)&regs->intstatus,
			sizeof(regs->intstatus), newstatus, REGRETRIES);
		sdio_info->f1regdata++;

		/* Handle host mailbox indication */
		if (newstatus & I_HMB_HOST_INT) {
			newstatus |= dbus_sdio_hostmail(sdio_info);
		}
	}

	if (pktq_mlen(&sdio_info->txq, ~sdio_info->flowcontrol) > 0) {
		dbus_sdio_txq_process(sdio_info);
	}

	/* If we're done for now, turn off clock request. */
	if (sdio_info->idletime == IDLE_IMMEDIATE) {
		sdio_info->activity = FALSE;
		dbus_sdio_clkctl(sdio_info, CLK_NONE, FALSE);
	}

clkwait:
	if (sdio_info->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: failed backplane access over SDIO, halting operation\n",
		           __FUNCTION__));
		dbus_sdio_state_change(sdio_info, DBUS_STATE_DISCONNECT);
	}

	return TRUE;
}

static void
dbus_sdio_watchdog(void *handle)
{
}
