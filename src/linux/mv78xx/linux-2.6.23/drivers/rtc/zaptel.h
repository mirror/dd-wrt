/*
 * Zapata Telephony Interface
 *
 * Written by Mark Spencer <markster@linux-suppot.net>
 * Based on previous works, designs, and architectures conceived and
 * written by Jim Dixon <jim@lambdatel.com>.
 *
 * Copyright (C) 2001 Jim Dixon / Zapata Telephony.
 * Copyright (C) 2001 - 2006 Digium, Inc.
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
 */

#ifndef _LINUX_ZAPTEL_H
#define _LINUX_ZAPTEL_H

#ifdef __KERNEL__
#include "zconfig.h"
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#include <linux/config.h>
#endif
#include <linux/fs.h>
#include <linux/ioctl.h>

#ifdef CONFIG_ZAPATA_NET	
#include <linux/hdlc.h>
#endif

#ifdef CONFIG_ZAPATA_PPP
#include <linux/ppp_channel.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
#define zap_pci_module pci_register_driver
#else
#define zap_pci_module pci_module_init
#endif

#ifdef LINUX26
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
#define ZAP_IRQ_HANDLER(a) static irqreturn_t a(int irq, void *dev_id)
#else
#define ZAP_IRQ_HANDLER(a) static irqreturn_t a(int irq, void *dev_id, struct pt_regs *regs)
#endif
#else
#define ZAP_IRQ_HANDLER(a) static void a(int irq, void *dev_id, struct pt_regs *regs)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#define ZAP_IRQ_SHARED IRQF_SHARED
#define ZAP_IRQ_DISABLED IRQF_DISABLED
#define ZAP_IRQ_SHARED_DISABLED IRQF_SHARED | IRQF_DISABLED
#else
#define ZAP_IRQ_SHARED SA_SHIRQ
#define ZAP_IRQ_DISABLED SA_INTERRUPT
#define ZAP_IRQ_SHARED_DISABLED SA_SHIRQ | SA_INTERRUPT
#endif

#include "ecdis.h"
#include "fasthdlc.h"

#ifdef CONFIG_DEVFS_FS
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/devfs_fs_kernel.h>
#else
#undef CONFIG_DEVFS_FS
//#warning "Zaptel doesn't support DEVFS in post 2.4 kernels.  Disabling DEVFS in zaptel"
#endif
#endif /* CONFIG_DEVFS_FS */
#endif /* __KERNEL__ */

#ifndef ELAST
#define ELAST 500
#endif

/* Per-span configuration values */
#define	ZT_CONFIG_TXLEVEL	7	/* bits 0-2 are tx level */

/* Line configuration */
/* These apply to T1 */
#define ZT_CONFIG_D4	 (1 << 4)	
#define ZT_CONFIG_ESF	 (1 << 5)
#define ZT_CONFIG_AMI	 (1 << 6)
#define ZT_CONFIG_B8ZS 	 (1 << 7)
/* These apply to E1 */
#define	ZT_CONFIG_CCS	 (1 << 8)	/* CCS (ISDN) instead of CAS (Robbed Bit) */
#define	ZT_CONFIG_HDB3	 (1 << 9)	/* HDB3 instead of AMI (line coding) */
#define	ZT_CONFIG_CRC4   (1 << 10)	/* CRC4 framing */
#define ZT_CONFIG_NOTOPEN (1 << 16)

/* Signalling types */
#define __ZT_SIG_FXO	 (1 << 12)	/* Never use directly */
#define __ZT_SIG_FXS	 (1 << 13)	/* Never use directly */

#define ZT_SIG_NONE		(0)			/* Channel not configured */
#define ZT_SIG_FXSLS	((1 << 0) | __ZT_SIG_FXS)	/* FXS, Loopstart */
#define ZT_SIG_FXSGS	((1 << 1) | __ZT_SIG_FXS)	/* FXS, Groundstart */
#define ZT_SIG_FXSKS	((1 << 2) | __ZT_SIG_FXS)	/* FXS, Kewlstart */

#define ZT_SIG_FXOLS	((1 << 3) | __ZT_SIG_FXO)	/* FXO, Loopstart */
#define ZT_SIG_FXOGS	((1 << 4) | __ZT_SIG_FXO)	/* FXO, Groupstart */
#define ZT_SIG_FXOKS	((1 << 5) | __ZT_SIG_FXO)	/* FXO, Kewlstart */

#define ZT_SIG_EM	(1 << 6)		/* Ear & Mouth (E&M) */

/* The following are all variations on clear channel */

#define __ZT_SIG_DACS	 (1 << 16)

#define ZT_SIG_CLEAR	 (1 << 7)					/* Clear channel */
#define ZT_SIG_HDLCRAW	((1 << 8)  | ZT_SIG_CLEAR)	/* Raw unchecked HDLC */
#define ZT_SIG_HDLCFCS	((1 << 9)  | ZT_SIG_HDLCRAW)	/* HDLC with FCS calculation */
#define ZT_SIG_HDLCNET	((1 << 10) | ZT_SIG_HDLCFCS)	/* HDLC Network */
#define ZT_SIG_SLAVE	 (1 << 11) 					/* Slave to another channel */
#define	ZT_SIG_SF	 (1 << 14)			/* Single Freq. tone only, no sig bits */
#define ZT_SIG_CAS	 (1 << 15)			/* Just get bits */
#define ZT_SIG_DACS	(__ZT_SIG_DACS | ZT_SIG_CLEAR)	/* Cross connect */
#define ZT_SIG_EM_E1	 (1 << 17)			/* E1 E&M Variation */
#define ZT_SIG_DACS_RBS	 ((1 << 18) | __ZT_SIG_DACS)	/* Cross connect w/ RBS */
#define ZT_SIG_HARDHDLC	((1 << 19) | ZT_SIG_CLEAR)

/* tone flag values */
#define	ZT_REVERSE_RXTONE 1  /* reverse polarity rx tone logic */
#define	ZT_REVERSE_TXTONE 2  /* reverse polarity tx tone logic */

#define ZT_ABIT			8
#define ZT_BBIT			4
#define	ZT_CBIT			2
#define	ZT_DBIT			1

#define ZT_MAJOR	196

#define ZT_CODE	'J'

/* Default chunk size for conferences and such -- static right now, might make
   variable sometime.  8 samples = 1 ms = most frequent service interval possible
   for a USB device */
#define ZT_CHUNKSIZE		 8
#define ZT_MIN_CHUNKSIZE	 ZT_CHUNKSIZE
#define ZT_DEFAULT_CHUNKSIZE	 ZT_CHUNKSIZE
#define ZT_MAX_CHUNKSIZE 	 ZT_CHUNKSIZE
#define ZT_CB_SIZE		 2

#define ZT_MAX_BLOCKSIZE 	 8192
#define ZT_DEFAULT_NUM_BUFS	 2
#define ZT_MAX_NUM_BUFS		 32
#define ZT_MAX_BUF_SPACE         32768

#define ZT_DEFAULT_BLOCKSIZE 1024
#define ZT_DEFAULT_MTR_MRU	 2048

#define ZT_POLICY_IMMEDIATE	 0		/* Start play/record immediately */
#define ZT_POLICY_WHEN_FULL  1		/* Start play/record when buffer is full */

#define	RING_DEBOUNCE_TIME	2000	/* 2000 ms ring debounce time */

#define ZT_GET_PARAMS_RETURN_MASTER 0x40000000

/* Extended attributes in lineconfig structure */
#define ZT_SPANINFO_HAS_LINECONFIG

typedef struct zt_params
{
int channo;		/* Channel number */
int spanno;		/* Span itself */
int chanpos;	/* Channel number in span */
int	sigtype;  /* read-only */
int sigcap;	 /* read-only */
int rxisoffhook; /* read-only */
int rxbits;	/* read-only */
int txbits;	/* read-only */
int txhooksig;	/* read-only */
int rxhooksig;	/* read-only */
int curlaw;	/* read-only  -- one of ZT_LAW_MULAW or ZT_LAW_ALAW */
int idlebits;	/* read-only  -- What is considered the idle state */
char name[40];	/* Name of channel */
int	prewinktime;
int	preflashtime;
int	winktime;
int	flashtime;
int	starttime;
int	rxwinktime;
int	rxflashtime;
int	debouncetime;
int	pulsebreaktime;
int	pulsemaketime;
int	pulseaftertime;
} ZT_PARAMS;

typedef struct zt_spaninfo_compat
{
int	spanno;		/* span number */
char name[20];	/* Name of span */
char desc[40];	/* Description of span */
int	alarms;		/* alarms status */
int	txlevel;	/* what TX level is set to */
int	rxlevel;	/* current RX level */
int	bpvcount;	/* current BPV count */
int	crc4count;	/* current CRC4 error count */
int	ebitcount;	/* current E-bit error count */
int	fascount;	/* current FAS error count */
int	irqmisses;	/* current IRQ misses */
int	syncsrc;	/* span # of current sync source, or 0 for free run  */
int	numchans;	/* number of configured channels on this span */
int	totalchans;	/* total number of channels on the span */
int	totalspans;	/* total number of zaptel spans in entire system */
} ZT_SPANINFO_COMPAT;

typedef struct zt_spaninfo
{
int	spanno;		/* span number */
char name[20];	/* Name of span */
char desc[40];	/* Description of span */
int	alarms;		/* alarms status */
int	txlevel;	/* what TX level is set to */
int	rxlevel;	/* current RX level */
int	bpvcount;	/* current BPV count */
int	crc4count;	/* current CRC4 error count */
int	ebitcount;	/* current E-bit error count */
int	fascount;	/* current FAS error count */
int	irqmisses;	/* current IRQ misses */
int	syncsrc;	/* span # of current sync source, or 0 for free run  */
int	numchans;	/* number of configured channels on this span */
int	totalchans;	/* total number of channels on the span */
int	totalspans;	/* total number of zaptel spans in entire system */
int	lbo;        /* line build out */
int	lineconfig; /* framing/coding */
} ZT_SPANINFO;

typedef struct zt_maintinfo
{
int	spanno;		/* span number 1-2 */
int	command;	/* command */
} ZT_MAINTINFO;

typedef struct zt_confinfo
{
int	chan;		/* channel number, 0 for current */
int	confno;		/* conference number */
int	confmode;	/* conferencing mode */
} ZT_CONFINFO;

typedef struct zt_gains
{
int	chan;		/* channel number, 0 for current */
unsigned char rxgain[256];	/* Receive gain table */
unsigned char txgain[256];	/* Transmit gain table */
} ZT_GAINS;

typedef struct zt_lineconfig
{
int span;		/* Which span number (0 to use name) */
char name[20];	/* Name of span to use */
int	lbo;		/* line build-outs */
int	lineconfig;	/* line config parameters (framing, coding) */
int	sync;		/* what level of sync source we are */
} ZT_LINECONFIG;

typedef struct zt_chanconfig
{
int	chan;		/* Channel we're applying this to (0 to use name) */
char name[40];		/* Name of channel to use */
int	sigtype;	/* Signal type */
int	deflaw;		/* Default law (ZT_LAW_DEFAULT, ZT_LAW_MULAW, or ZT_LAW_ALAW */
int	master;		/* Master channel if sigtype is ZT_SLAVE */
int	idlebits;	/* Idle bits (if this is a CAS channel) or
			   channel to monitor (if this is DACS channel) */
char netdev_name[16]; /*name for the hdlc network device*/
} ZT_CHANCONFIG;

typedef struct zt_sfconfig
{
int	chan;		/* Channel we're applying this to (0 to use name) */
char name[40];		/* Name of channel to use */
long	rxp1;		/* receive tone det. p1 */
long	rxp2;		/* receive tone det. p2 */
long	rxp3;		/* receive tone det. p3 */
int	txtone;		/* Tx tone factor */
int	tx_v2;		/* initial v2 value */
int	tx_v3;		/* initial v3 value */
int	toneflag;	/* Tone flags */
} ZT_SFCONFIG;

typedef struct zt_bufferinfo
{
int txbufpolicy;	/* Policy for handling receive buffers */
int rxbufpolicy;	/* Policy for handling receive buffers */
int numbufs;		/* How many buffers to use */
int bufsize;		/* How big each buffer is */
int readbufs;		/* How many read buffers are full (read-only) */
int writebufs;		/* How many write buffers are full (read-only) */
} ZT_BUFFERINFO;

typedef struct zt_dialparams
{
int mfv1_tonelen;	/* MF tone length (KP = this * 5/3) */
int dtmf_tonelen;	/* DTMF tone length */
int reserved[4];	/* Reserved for future expansion -- always set to 0 */
} ZT_DIAL_PARAMS;


typedef struct zt_dynamic_span {
	char driver[20];	/* Which low-level driver to use */
	char addr[40];		/* Destination address */
	int numchans;		/* Number of channels */
	int timing;		/* Timing source preference */
	int spanno;		/* Span number (filled in by zaptel) */
} ZT_DYNAMIC_SPAN;

/* Define the max # of outgoing DTMF or MFv1 digits to queue in-kernel */
#define ZT_MAX_DTMF_BUF 256

#define ZT_DIAL_OP_APPEND	1
#define ZT_DIAL_OP_REPLACE	2
#define ZT_DIAL_OP_CANCEL	3

#define ZT_LAW_DEFAULT	0	/* Default law for span */
#define ZT_LAW_MULAW	1	/* Mu-law */
#define ZT_LAW_ALAW	2	/* A-law */

typedef struct zt_dialoperation
{
int op;
char dialstr[ZT_MAX_DTMF_BUF];
} ZT_DIAL_OPERATION;


typedef struct zt_indirect_data
{
int	chan;
int	op;
void	*data;
} ZT_INDIRECT_DATA;	

struct zt_versioninfo {
	char version[80];
	char echo_canceller[80];
};


/* ioctl definitions */
#define ZT_CODE	'J'

/*
 * Get Transfer Block Size.
 */
#define ZT_GET_BLOCKSIZE	_IOR (ZT_CODE, 1, int)

/*
 * Set Transfer Block Size.
 */
#define ZT_SET_BLOCKSIZE	_IOW (ZT_CODE, 2, int)

/*
 * Flush Buffer(s) and stop I/O
 */
#define	ZT_FLUSH		_IOW (ZT_CODE, 3, int)

/*
 * Wait for Write to Finish
 */
#define	ZT_SYNC		_IOW (ZT_CODE, 4, int)

/*
 * Get channel parameters
 */
#define ZT_GET_PARAMS		_IOR (ZT_CODE, 5, struct zt_params)

/*
 * Get channel parameters
 */
#define ZT_SET_PARAMS		_IOW (ZT_CODE, 6, struct zt_params)

/*
 * Set Hookswitch Status
 */
#define ZT_HOOK		_IOW (ZT_CODE, 7, int)

/*
 * Get Signalling Event
 */
#define ZT_GETEVENT		_IOR (ZT_CODE, 8, int)

/*
 * Wait for something to happen (IO Mux)
 */
#define ZT_IOMUX		_IOWR (ZT_CODE, 9, int)

/*
 * Get Span Status (_COMPAT is deprecated)
 */
#define ZT_SPANSTAT_COMPAT		_IOWR (ZT_CODE, 10, struct zt_spaninfo_compat)
#define ZT_SPANSTAT		_IOWR (ZT_CODE, 10, struct zt_spaninfo)

/*
 * Set Maintenance Mode
 */
#define ZT_MAINT		_IOW (ZT_CODE, 11, struct zt_maintinfo)

/*
 * Get Conference Mode
 */
#define ZT_GETCONF		_IOWR (ZT_CODE, 12, struct zt_confinfo)

/*
 * Set Conference Mode
 */
#define ZT_SETCONF		_IOWR (ZT_CODE, 13, struct zt_confinfo)

/*
 * Setup or Remove Conference Link
 */
#define ZT_CONFLINK		_IOW (ZT_CODE, 14, struct zt_confinfo)

/*
 * Display Conference Diagnostic Information on Console
 */
#define ZT_CONFDIAG		_IOR (ZT_CODE, 15, int)

/*
 * Get Channel audio gains
 */
#define ZT_GETGAINS		_IOWR (ZT_CODE, 16, struct zt_gains)

/*
 * Set Channel audio gains
 */
#define ZT_SETGAINS		_IOWR (ZT_CODE, 17, struct zt_gains)

/*
 * Set Line (T1) Configurations and start system
 */
#define	ZT_SPANCONFIG		_IOW (ZT_CODE, 18, struct zt_lineconfig)

/*
 * Set Channel Configuration
 */
#define	ZT_CHANCONFIG		_IOW (ZT_CODE, 19, struct zt_chanconfig)

/*
 * Set Conference to mute mode
 */
#define	ZT_CONFMUTE		_IOW (ZT_CODE, 20, int)

/*
 * Send a particular tone (see ZT_TONE_*)
 */
#define	ZT_SENDTONE		_IOW (ZT_CODE, 21, int)

/*
 * Set your region for tones (see ZT_TONE_ZONE_*)
 */
#define	ZT_SETTONEZONE		_IOW (ZT_CODE, 22, int)

/*
 * Retrieve current region for tones (see ZT_TONE_ZONE_*)
 */
#define	ZT_GETTONEZONE		_IOR (ZT_CODE, 23, int)

/*
 * Master unit only -- set default zone (see ZT_TONE_ZONE_*)
 */
#define	ZT_DEFAULTZONE		_IOW (ZT_CODE, 24, int)

/*
 * Load a tone zone from a ZT_tone_def_header, see
 * below...
 */
#define ZT_LOADZONE		_IOW (ZT_CODE, 25, struct zt_tone_def_header)

/*
 * Free a tone zone 
 */
#define ZT_FREEZONE		_IOW (ZT_CODE, 26, int)

/*
 * Set buffer policy 
 */
#define ZT_SET_BUFINFO		_IOW (ZT_CODE, 27, struct zt_bufferinfo)

/*
 * Get current buffer info
 */
#define ZT_GET_BUFINFO		_IOR (ZT_CODE, 28, struct zt_bufferinfo)

/*
 * Get dialing parameters
 */
#define ZT_GET_DIALPARAMS	_IOR (ZT_CODE, 29, struct zt_dialparams)

/*
 * Set dialing parameters
 */
#define ZT_SET_DIALPARAMS	_IOW (ZT_CODE, 30, struct zt_dialparams)

/*
 * Append, replace, or cancel a dial string
 */
#define ZT_DIAL			_IOW (ZT_CODE, 31, struct zt_dialoperation)

/*
 * Set a clear channel into audio mode
 */
#define ZT_AUDIOMODE		_IOW (ZT_CODE, 32, int)

/*
 * Enable or disable echo cancellation on a channel 
 * The number is zero to disable echo cancellation and non-zero
 * to enable echo cancellation.  If the number is between 32
 * and 256, it will also set the number of taps in the echo canceller
 */
#define ZT_ECHOCANCEL		_IOW (ZT_CODE, 33, int)

/*
 * Return a channel's channel number (useful for the /dev/zap/pseudo type interfaces 
 */
#define ZT_CHANNO		_IOR (ZT_CODE, 34, int)

/*
 * Return a flag indicating whether channel is currently dialing
 */
#define ZT_DIALING		_IOR (ZT_CODE, 35, int)

/* Numbers 60 to 90 are reserved for private use of low level hardware
   drivers */

/*
 * Set a clear channel into HDLC w/out FCS checking/calculation mode
 */
#define ZT_HDLCRAWMODE		_IOW (ZT_CODE, 36, int)

/*
 * Set a clear channel into HDLC w/ FCS mode
 */
#define ZT_HDLCFCSMODE		_IOW (ZT_CODE, 37, int)

/* 
 * Specify a channel on /dev/zap/chan -- must be done before any other ioctl's and is only
 * valid on /dev/zap/chan
 */
#define ZT_SPECIFY		_IOW (ZT_CODE, 38, int)

/*
 * Temporarily set the law on a channel to 
 * ZT_LAW_DEFAULT, ZT_LAW_ALAW, or ZT_LAW_MULAW.  Is reset on close.  
 */
#define ZT_SETLAW		_IOW (ZT_CODE, 39, int)

/*
 * Temporarily set the channel to operate in linear mode when non-zero
 * or default law if 0
 */
#define ZT_SETLINEAR		_IOW (ZT_CODE, 40, int)

/*
 * Set a clear channel into HDLC w/ PPP interface mode
 */
#define ZT_HDLCPPP		_IOW (ZT_CODE, 41, int)

/*
 * Set the ring cadence for FXS interfaces
 */
#define ZT_SETCADENCE		_IOW (ZT_CODE, 42, struct zt_ring_cadence)

/*
 * Set the bits going out for CAS interface
 */
#define ZT_SETTXBITS			_IOW (ZT_CODE, 43, int)


/*
 * Display Channel Diagnostic Information on Console
 */
#define ZT_CHANDIAG		_IOR (ZT_CODE, 44, int) 

/* 
 * Obtain received signalling
 */
#define ZT_GETRXBITS _IOR (ZT_CODE, 45, int)

/*
 * Set Channel's SF Tone Configuration
 */
#define	ZT_SFCONFIG		_IOW (ZT_CODE, 46, struct zt_sfconfig)

/*
 * Set timer expiration (in samples)
 */
#define ZT_TIMERCONFIG	_IOW (ZT_CODE, 47, int)

/*
 * Acknowledge timer expiration (number to acknowledge, or -1 for all)
 */
#define ZT_TIMERACK _IOW (ZT_CODE, 48, int)

/*
 * Get Conference to mute mode
 */
#define	ZT_GETCONFMUTE		_IOR (ZT_CODE, 49, int)

/*
 * Request echo training in some number of ms (with muting in the mean time)
 */
#define	ZT_ECHOTRAIN		_IOW (ZT_CODE, 50, int)

/*
 * Set on hook transfer for n number of ms -- implemnted by low level driver
 */
#define	ZT_ONHOOKTRANSFER		_IOW (ZT_CODE, 51, int)

/*
 * Queue Ping
 */
#define ZT_TIMERPING _IOW (ZT_CODE, 42, int) /* Should be 52, but works */

/*
 * Acknowledge ping
 */
#define ZT_TIMERPONG _IOW (ZT_CODE, 53, int)

/*
 * Set/get signalling freeze
 */
#define ZT_SIGFREEZE _IOW (ZT_CODE, 54, int)
#define ZT_GETSIGFREEZE _IOR (ZT_CODE, 55, int)

/*
 * Do a channel IOCTL from the /dev/zap/ctl interface
 */
#define ZT_INDIRECT _IOWR (ZT_CODE, 56, struct zt_indirect_data)


/*
 * Get the version of Zaptel that is running, and a description
 * of the compiled-in echo canceller (if any)
 */
#define ZT_GETVERSION _IOR(ZT_CODE, 57, struct zt_versioninfo)

/*
 * Put the channel in loopback mode (receive from the channel is
 * transmitted back on the interface)
 */
#define ZT_LOOPBACK _IOW(ZT_CODE, 58, int)

/*
 *  60-80 are reserved for private drivers
 *  80-85 are reserved for dynamic span stuff
 */

/*
 * Create a dynamic span
 */
#define ZT_DYNAMIC_CREATE	_IOWR (ZT_CODE, 80, struct zt_dynamic_span)

/* 
 * Destroy a dynamic span 
 */
#define ZT_DYNAMIC_DESTROY	_IOW (ZT_CODE, 81, struct zt_dynamic_span)

/*
 * Enable tone detection -- implemented by low level driver
 */
#define ZT_TONEDETECT _IOW (ZT_CODE, 91, int)


/*
 * Set polarity -- implemented by individual driver.  0 = forward, 1 = reverse
 */
#define	ZT_SETPOLARITY		_IOW (ZT_CODE, 92, int)

/*
 * Transcoder operations
 */
#define ZT_TRANSCODE_OP		_IOWR(ZT_CODE, 93, int)

/*
 * VoiceMail Waiting Indication (WMWI) -- implemented by low-level driver.
 * Value: number of waiting messages (hence 0: switch messages off).
 */
#define ZT_VMWI			_IOWR(ZT_CODE, 94, int)

/* 
 * Startup or Shutdown a span
 */
#define ZT_STARTUP		_IOW (ZT_CODE, 99, int)
#define ZT_SHUTDOWN		_IOW (ZT_CODE, 100, int)

#define ZT_TONE_ZONE_MAX		128

#define ZT_TONE_ZONE_DEFAULT 	-1	/* To restore default */

#define ZT_TONE_STOP		-1
#define ZT_TONE_DIALTONE	0
#define ZT_TONE_BUSY		1
#define ZT_TONE_RINGTONE	2
#define ZT_TONE_CONGESTION	3
#define ZT_TONE_CALLWAIT	4
#define ZT_TONE_DIALRECALL	5
#define ZT_TONE_RECORDTONE	6
#define ZT_TONE_INFO		7
#define ZT_TONE_CUST1		8
#define ZT_TONE_CUST2		9
#define ZT_TONE_STUTTER		10
#define ZT_TONE_MAX		16

#define ZT_TONE_DTMF_BASE	64

/*
 * These must be in the same order as the dtmf_tones array in tones.h 
 */
enum {
	ZT_TONE_DTMF_0 = ZT_TONE_DTMF_BASE,
	ZT_TONE_DTMF_1,
	ZT_TONE_DTMF_2,
	ZT_TONE_DTMF_3,
	ZT_TONE_DTMF_4,
	ZT_TONE_DTMF_5,
	ZT_TONE_DTMF_6,
	ZT_TONE_DTMF_7,
	ZT_TONE_DTMF_8,
	ZT_TONE_DTMF_9,
	ZT_TONE_DTMF_s,
	ZT_TONE_DTMF_p,
	ZT_TONE_DTMF_A,
	ZT_TONE_DTMF_B,
	ZT_TONE_DTMF_C,
	ZT_TONE_DTMF_D
};

#define ZT_TONE_DTMF_MAX ZT_TONE_DTMF_D

#define ZT_MAX_CADENCE		16

#define ZT_TONEDETECT_ON	(1 << 0)		/* Detect tones */
#define ZT_TONEDETECT_MUTE	(1 << 1)		/* Mute audio in received channel */

#define ZT_TRANSCODE_MAGIC 0x74a9c0de

/* Operations */
#define ZT_TCOP_ALLOCATE	1			/* Allocate/reset DTE channel */
#define ZT_TCOP_TRANSCODE	2			/* Begin transcoding a block */
#define ZT_TCOP_GETINFO		3			/* Get information (use zt_transcode_info) */
#define ZT_TCOP_RELEASE         4                       /* Release DTE channel */
#define ZT_TCOP_TEST            5                       /* test DTE device */
typedef struct zt_transcode_info {
	unsigned int op;
	unsigned int tcnum;
	char name[80];
	int numchannels;
	unsigned int srcfmts;
	unsigned int dstfmts;
} ZT_TRANSCODE_INFO;

#define ZT_TCCONF_USETS		(1 << 0)		/* Use/update timestamp field */
#define ZT_TCCONF_USESEQ	(1 << 1)		/* Use/update seqno field */

#define ZT_TCSTAT_DSTRDY	(1 << 0)		/* Destination data is ready */
#define ZT_TCSTAT_DSTBUSY	(1 << 1)		/* Destination data is outstanding */

#define __ZT_TRANSCODE_BUFSIZ	16384
#define ZT_TRANSCODE_HDRLEN	256
#define ZT_TRANSCODE_BUFSIZ	((__ZT_TRANSCODE_BUFSIZ) - (ZT_TRANSCODE_HDRLEN))
#define ZT_TRANSCODE_DSTOFFSET	(((ZT_TRANSCODE_BUFSIZ) / 2) + ZT_TRANSCODE_HDRLEN)
#define ZT_TRANSCODE_SRCOFFSET	(((ZT_TRANSCODE_BUFSIZ) / 2) + ZT_TRANSCODE_HDRLEN)

typedef struct zt_transcode_header {
	unsigned int srcfmt;		/* See formats.h -- use TCOP_RESET when you change */
	unsigned int srcoffset; 	/* In bytes -- written by user */
	unsigned int srclen;		/* In bytes -- written by user */
	unsigned int srctimestamp;	/* In samples -- written by user (only used if ZT_TCCONF_USETS is set) */
	unsigned int srcseqno;		/* In units -- written by user (only used if ZT_TCCONF_USESEQ is set) */

	unsigned int dstfmt;		/* See formats.h -- use TCOP_RESET when you change */
	unsigned int dstoffset;  	/* In bytes -- written by user */
	unsigned int dsttimestamp;	/* In samples -- read by user */
	unsigned int dstseqno;		/* In units -- read by user (only used if ZT_TCCONF_USESEQ is set) */
	unsigned int dstlen;  		/* In bytes -- read by user */
	unsigned int dstsamples;	/* In timestamp units -- read by user */

	unsigned int magic;		/* Magic value -- ZT_TRANSCODE_MAGIC, read by user */
	unsigned int config;		/* Read/write by user */
	unsigned int status;		/* Read/write by user */
	unsigned char userhdr[ZT_TRANSCODE_HDRLEN - (sizeof(unsigned int) * 14)];	/* Storage for user parameters */
	unsigned char srcdata[ZT_TRANSCODE_BUFSIZ / 2];	/* Storage of source data */
	unsigned char dstdata[ZT_TRANSCODE_BUFSIZ / 2];	/* Storage of destination data */
} ZT_TRANSCODE_HEADER;

struct zt_ring_cadence {
	int ringcadence[ZT_MAX_CADENCE];
};

struct zt_tone_def_header {
	int count;		/* How many samples follow */
	int zone;		/* Which zone we are loading */
	int ringcadence[ZT_MAX_CADENCE];	/* Ring cadence in ms (0=on, 1=off, ends with 0 value) */
	char name[40];		/* Informational name of zone */
	/* Immediately follow the ZT_tone_def_header by ZT_tone_def's */
};

struct zt_tone_def {		/* Structure for zone programming */
	int tone;		/* See ZT_TONE_* */
	int next;		/* What the next position in the cadence is
				   (They're numbered by the order the appear here) */
	int samples;		/* How many samples to play for this cadence */
				/* Now come the constants we need to make tones */
	int shift;		/* How much to scale down the volume (2 is nice) */

	/* 
		Calculate the next 6 factors using the following equations:
		l = <level in dbm>, f1 = <freq1>, f2 = <freq2>
		gain = pow(10.0, (l - 3.14) / 20.0) * 65536.0 / 2.0;

		// Frequency factor 1 
		fac_1 = 2.0 * cos(2.0 * M_PI * (f1/8000.0)) * 32768.0;
		// Last previous two samples 
		init_v2_1 = sin(-4.0 * M_PI * (f1/8000.0)) * gain;
		init_v3_1 = sin(-2.0 * M_PI * (f1/8000.0)) * gain;

		// Frequency factor 2 
		fac_2 = 2.0 * cos(2.0 * M_PI * (f2/8000.0)) * 32768.0;
		// Last previous two samples 
		init_v2_2 = sin(-4.0 * M_PI * (f2/8000.0)) * gain;
		init_v3_2 = sin(-2.0 * M_PI * (f2/8000.0)) * gain;
	*/
	int fac1;		
	int init_v2_1;		
	int init_v3_1;		
	int fac2;		
	int init_v2_2;		
	int init_v3_2;
	int modulate;

};

#ifdef __KERNEL__
#endif /* KERNEL */

/* Define the maximum block size */
#define	ZT_MAX_BLOCKSIZE	8192

/* Define the default network block size */
#define ZT_DEFAULT_MTU_MRU	2048

/* Flush and stop the read (input) process */
#define	ZT_FLUSH_READ		1

/* Flush and stop the write (output) process */
#define	ZT_FLUSH_WRITE		2

/* Flush and stop both (input and output) processes */
#define	ZT_FLUSH_BOTH		(ZT_FLUSH_READ | ZT_FLUSH_WRITE)

/* Flush the event queue */
#define	ZT_FLUSH_EVENT		4

/* Flush everything */
#define	ZT_FLUSH_ALL		(ZT_FLUSH_READ | ZT_FLUSH_WRITE | ZT_FLUSH_EVENT)


/* Value for ZT_HOOK, set to ON hook */
#define	ZT_ONHOOK	0

/* Value for ZT_HOOK, set to OFF hook */
#define	ZT_OFFHOOK	1

/* Value for ZT_HOOK, wink (off hook momentarily) */
#define	ZT_WINK		2

/* Value for ZT_HOOK, flash (on hook momentarily) */
#define	ZT_FLASH	3

/* Value for ZT_HOOK, start line */
#define	ZT_START	4

/* Value for ZT_HOOK, ring line (same as start line) */
#define	ZT_RING		5

/* Value for ZT_HOOK, turn ringer off */
#define ZT_RINGOFF  6

/* Ret. Value for GET/WAIT Event, no event */
#define	ZT_EVENT_NONE	0

/* Ret. Value for GET/WAIT Event, Went Onhook */
#define	ZT_EVENT_ONHOOK 1

/* Ret. Value for GET/WAIT Event, Went Offhook or got Ring */
#define	ZT_EVENT_RINGOFFHOOK 2

/* Ret. Value for GET/WAIT Event, Got Wink or Flash */
#define	ZT_EVENT_WINKFLASH 3

/* Ret. Value for GET/WAIT Event, Got Alarm */
#define	ZT_EVENT_ALARM	4

/* Ret. Value for GET/WAIT Event, Got No Alarm (after alarm) */
#define	ZT_EVENT_NOALARM 5

/* Ret. Value for GET/WAIT Event, HDLC Abort frame */
#define ZT_EVENT_ABORT 6

/* Ret. Value for GET/WAIT Event, HDLC Frame overrun */
#define ZT_EVENT_OVERRUN 7

/* Ret. Value for GET/WAIT Event, Bad FCS */
#define ZT_EVENT_BADFCS 8

/* Ret. Value for dial complete */
#define ZT_EVENT_DIALCOMPLETE	9

/* Ret Value for ringer going on */
#define ZT_EVENT_RINGERON 10

/* Ret Value for ringer going off */
#define ZT_EVENT_RINGEROFF 11

/* Ret Value for hook change complete */
#define ZT_EVENT_HOOKCOMPLETE 12

/* Ret Value for bits changing on a CAS / User channel */
#define ZT_EVENT_BITSCHANGED 13

/* Ret value for the beginning of a pulse coming on its way */
#define ZT_EVENT_PULSE_START 14

/* Timer event -- timer expired */
#define ZT_EVENT_TIMER_EXPIRED	15

/* Timer event -- ping ready */
#define ZT_EVENT_TIMER_PING		16

/* Polarity reversal event */
#define ZT_EVENT_POLARITY  17

/* Ring Begin event */
#define ZT_EVENT_RINGBEGIN  18

/* Echo can disabled event */
#define ZT_EVENT_EC_DISABLED 19

/* Channel was disconnected. Hint user to close channel */
#define ZT_EVENT_REMOVED   20

#define ZT_EVENT_PULSEDIGIT (1 << 16)	/* This is OR'd with the digit received */
#define ZT_EVENT_DTMFDOWN  (1 << 17)	/* Ditto for DTMF key down event */
#define ZT_EVENT_DTMFUP (1 << 18)	/* Ditto for DTMF key up event */

/* Flag Value for IOMUX, read avail */
#define	ZT_IOMUX_READ	1

/* Flag Value for IOMUX, write avail */
#define	ZT_IOMUX_WRITE	2

/* Flag Value for IOMUX, write done */
#define	ZT_IOMUX_WRITEEMPTY	4

/* Flag Value for IOMUX, signalling event avail */
#define	ZT_IOMUX_SIGEVENT	8

/* Flag Value for IOMUX, Do Not Wait if nothing to report */
#define	ZT_IOMUX_NOWAIT	0x100

/* Alarm Condition bits */
#define	ZT_ALARM_NONE		0	/* No alarms */
#define	ZT_ALARM_RECOVER	1	/* Recovering from alarm */
#define	ZT_ALARM_LOOPBACK	2	/* In loopback */
#define	ZT_ALARM_YELLOW		4	/* Yellow Alarm */
#define	ZT_ALARM_RED		8	/* Red Alarm */
#define	ZT_ALARM_BLUE		16	/* Blue Alarm */
#define ZT_ALARM_NOTOPEN	32
/* Maintenance modes */
#define	ZT_MAINT_NONE		0	/* Normal Mode */
#define	ZT_MAINT_LOCALLOOP	1	/* Local Loopback */
#define	ZT_MAINT_REMOTELOOP	2	/* Remote Loopback */
#define	ZT_MAINT_LOOPUP	3	/* send loopup code */
#define	ZT_MAINT_LOOPDOWN	4	/* send loopdown code */
#define	ZT_MAINT_LOOPSTOP	5	/* stop sending loop codes */


/* Conference modes */
#define	ZT_CONF_MODE_MASK 0xff		/* mask for modes */
#define	ZT_CONF_NORMAL	0		/* normal mode */
#define	ZT_CONF_MONITOR 1		/* monitor mode (rx of other chan) */
#define	ZT_CONF_MONITORTX 2		/* monitor mode (tx of other chan) */
#define	ZT_CONF_MONITORBOTH 3		/* monitor mode (rx & tx of other chan) */
#define	ZT_CONF_CONF 4			/* conference mode */
#define	ZT_CONF_CONFANN 5		/* conference announce mode */
#define	ZT_CONF_CONFMON 6		/* conference monitor mode */
#define	ZT_CONF_CONFANNMON 7		/* conference announce/monitor mode */
#define	ZT_CONF_REALANDPSEUDO 8	/* real and pseudo port both on conf */
#define ZT_CONF_DIGITALMON 9	/* Do not decode or interpret */
#define	ZT_CONF_MONITOR_RX_PREECHO 10	/* monitor mode (rx of other chan) - before echo can is done */
#define	ZT_CONF_MONITOR_TX_PREECHO 11	/* monitor mode (tx of other chan) - before echo can is done */
#define	ZT_CONF_MONITORBOTH_PREECHO 12	/* monitor mode (rx & tx of other chan) - before echo can is done */
#define	ZT_CONF_FLAG_MASK 0xff00	/* mask for flags */
#define	ZT_CONF_LISTENER 0x100		/* is a listener on the conference */
#define	ZT_CONF_TALKER 0x200		/* is a talker on the conference */
#define	ZT_CONF_PSEUDO_LISTENER 0x400	/* pseudo is a listener on the conference */
#define	ZT_CONF_PSEUDO_TALKER 0x800	/* pseudo is a talker on the conference */


#define	ZT_DEFAULT_WINKTIME	150	/* 150 ms default wink time */
#define	ZT_DEFAULT_FLASHTIME	750	/* 750 ms default flash time */

#define	ZT_DEFAULT_PREWINKTIME	50	/* 50 ms before wink */
#define	ZT_DEFAULT_PREFLASHTIME 50	/* 50 ms before flash */
#define	ZT_DEFAULT_STARTTIME 1500	/* 1500 ms of start */
#define	ZT_DEFAULT_RINGTIME 2000	/* 2000 ms of ring on (start, FXO) */
#if 0
#define	ZT_DEFAULT_RXWINKTIME 250	/* 250ms longest rx wink */
#endif
#define	ZT_DEFAULT_RXWINKTIME 300	/* 300ms longest rx wink (to work with the Atlas) */
#define	ZT_DEFAULT_RXFLASHTIME 1250	/* 1250ms longest rx flash */
#define	ZT_DEFAULT_DEBOUNCETIME 600	/* 600ms of FXS GS signalling debounce */
#define	ZT_DEFAULT_PULSEMAKETIME 50	/* 50 ms of line closed when dial pulsing */
#define	ZT_DEFAULT_PULSEBREAKTIME 50	/* 50 ms of line open when dial pulsing */
#define	ZT_DEFAULT_PULSEAFTERTIME 750	/* 750ms between dial pulse digits */

#define	ZT_MINPULSETIME (15 * 8)	/* 15 ms minimum */

#ifdef SHORT_FLASH_TIME
#define	ZT_MAXPULSETIME (80 * 8)	/* we need 80 ms, not 200ms, as we have a short flash */
#else
#define	ZT_MAXPULSETIME (200 * 8)	/* 200 ms maximum */
#endif

#define	ZT_PULSETIMEOUT ((ZT_MAXPULSETIME / 8) + 50)

#define ZT_RINGTRAILER (50 * 8)	/* Don't consider a ring "over" until it's been gone at least this
									   much time */

#define	ZT_LOOPCODE_TIME 10000		/* send loop codes for 10 secs */
#define	ZT_ALARMSETTLE_TIME	5000	/* allow alarms to settle for 5 secs */
#define	ZT_AFTERSTART_TIME 500		/* 500ms after start */

#define ZT_RINGOFFTIME 4000		/* Turn off ringer for 4000 ms */
#define	ZT_KEWLTIME 500		/* 500ms for kewl pulse */
#define	ZT_AFTERKEWLTIME 300    /* 300ms after kewl pulse */

#define ZT_MAX_PRETRAINING   1000	/* 1000ms max pretraining time */

#define ZT_MAX_SPANS		128		/* Max, 128 spans */
#define ZT_MAX_CHANNELS		1024	/* Max, 1024 channels */
#define ZT_MAX_CONF			1024	/* Max, 1024 conferences */

#ifdef	FXSFLASH
#define ZT_FXSFLASHMINTIME	450	/* min 450ms */
#define ZT_FXSFLASHMAXTIME	550	/* max 550ms */
#endif

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/poll.h>

#define	ZT_MAX_EVENTSIZE	64	/* 64 events max in buffer */

struct zt_span;
struct zt_chan;

struct zt_tone_state {
	int v1_1;
	int v2_1;
	int v3_1;
	int v1_2;
	int v2_2;
	int v3_2;
	int modulate;
};

#ifdef CONFIG_ZAPATA_NET
struct zt_hdlc {
#ifdef LINUX26	
	struct net_device *netdev;
#else
	hdlc_device netdev;
#endif
	struct zt_chan *chan;
};
#endif

/* Echo cancellation */
struct echo_can_state;
#if 0
/* echo can API consists of these functions */
void echo_can_init(void);
void echo_chan_shutdown(void);
void echo_can_identify(char *buf, size_t len);
struct echo_can_state *echo_can_create(int len, int adaption_mode);
void echo_can_free(struct echo_can_state *ec);
short echo_can_update(struct echo_can_state *ec, short iref, short isig);
void echo_can_array_update(struct echo_can_state *ec, short *iref, short *isig);
int echo_can_traintap(struct echo_can_state *ec, int pos, short val);
#endif

/* Conference queue stucture */
struct confq {
	u_char buffer[ZT_CHUNKSIZE * ZT_CB_SIZE];
	u_char *buf[ZT_CB_SIZE];
	int inbuf;
	int outbuf;
};

typedef struct
{
	long	x1;
	long	x2;
	long	y1;
	long	y2;
	long	e1;
	long	e2;
	int	samps;
	int	lastdetect;
} sf_detect_state_t;

struct zt_chan {
#ifdef CONFIG_ZAPATA_NET
	/* Must be first */
	struct zt_hdlc *hdlcnetdev;
#endif
#ifdef CONFIG_ZAPATA_PPP
	struct ppp_channel *ppp;
	struct tasklet_struct ppp_calls;
	int do_ppp_wakeup;
	int do_ppp_error;
	struct sk_buff_head ppp_rq;
#endif
	spinlock_t lock;
	char name[40];		/* Name */
	/* Specified by zaptel */
	int channo;			/* Zaptel Channel number */
	int chanpos;
	int flags;
	long rxp1;
	long rxp2;
	long rxp3;
	int txtone;
	int tx_v2;
	int tx_v3;
	int v1_1;
	int v2_1;
	int v3_1;
	int toneflags;
	sf_detect_state_t rd;

	struct zt_chan *master;	/* Our Master channel (could be us) */
	/* Next slave (if appropriate) */
	int nextslave;

	u_char *writechunk;						/* Actual place to write to */
	u_char swritechunk[ZT_MAX_CHUNKSIZE];	/* Buffer to be written */
	u_char *readchunk;						/* Actual place to read from */
	u_char sreadchunk[ZT_MAX_CHUNKSIZE];	/* Preallocated static area */
	short *readchunkpreec;

	/* Pointer to tx and rx gain tables */
	u_char *rxgain;
	u_char *txgain;
	
	/* Whether or not we have allocated gains or are using the default */
	int gainalloc;

	/* Specified by driver, readable by zaptel */
	void *pvt;			/* Private channel data */
	struct file *file;	/* File structure */
	
	
	struct zt_span *span;		/* Span we're a member of */
	int sig;			/* Signalling */
	int sigcap;			/* Capability for signalling */
	

	/* Used only by zaptel -- NO DRIVER SERVICEABLE PARTS BELOW */
	/* Buffer declarations */
	u_char		*readbuf[ZT_MAX_NUM_BUFS];	/* read buffer */
	int		inreadbuf;
	int		outreadbuf;
	wait_queue_head_t readbufq; /* read wait queue */

	u_char		*writebuf[ZT_MAX_NUM_BUFS]; /* write buffers */
	int		inwritebuf;
	int		outwritebuf;
	wait_queue_head_t writebufq; /* write wait queue */
	
	int		blocksize;	/* Block size */

	int		eventinidx;  /* out index in event buf (circular) */
	int		eventoutidx;  /* in index in event buf (circular) */
	unsigned int	eventbuf[ZT_MAX_EVENTSIZE];  /* event circ. buffer */
	wait_queue_head_t eventbufq; /* event wait queue */
	
	wait_queue_head_t txstateq;	/* waiting on the tx state to change */
	
	int		readn[ZT_MAX_NUM_BUFS];  /* # of bytes ready in read buf */
	int		readidx[ZT_MAX_NUM_BUFS];  /* current read pointer */
	int		writen[ZT_MAX_NUM_BUFS];  /* # of bytes ready in write buf */
	int		writeidx[ZT_MAX_NUM_BUFS];  /* current write pointer */
	
	int		numbufs;			/* How many buffers in channel */
	int		txbufpolicy;			/* Buffer policy */
	int		rxbufpolicy;			/* Buffer policy */
	int		txdisable;				/* Disable transmitter */
	int 	rxdisable;				/* Disable receiver */
	
	
	/* Tone zone stuff */
	struct zt_zone *curzone;		/* Zone for selecting tones */
	int 	tonezone;				/* Tone zone for this channel */
	struct zt_tone *curtone;		/* Current tone we're playing (if any) */
	int		tonep;					/* Current position in tone */
	struct zt_tone_state ts;		/* Tone state */

	/* Pulse dial stuff */
	int	pdialcount;			/* pulse dial count */

	/* Ring cadence */
	int ringcadence[ZT_MAX_CADENCE];
	int firstcadencepos;				/* Where to restart ring cadence */

	/* Digit string dialing stuff */
	int		digitmode;			/* What kind of tones are we sending? */
	char	txdialbuf[ZT_MAX_DTMF_BUF];
	int 	dialing;
	int	afterdialingtimer;
	int		cadencepos;				/* Where in the cadence we are */

	/* I/O Mask */	
	int		iomask;  /* I/O Mux signal mask */
	wait_queue_head_t sel;	/* thingy for select stuff */
	
	/* HDLC state machines */
	struct fasthdlc_state txhdlc;
	struct fasthdlc_state rxhdlc;
	int infcs;

	/* Conferencing stuff */
	int		confna;	/* conference number (alias) */
	int		_confn;	/* Actual conference number */
	int		confmode;  /* conference mode */
	int		confmute; /* conference mute mode */

	/* Incoming and outgoing conference chunk queues for
	   communicating between zaptel master time and
	   other boards */
	struct confq confin;
	struct confq confout;

	short	getlin[ZT_MAX_CHUNKSIZE];			/* Last transmitted samples */
	unsigned char getraw[ZT_MAX_CHUNKSIZE];		/* Last received raw data */
	short	getlin_lastchunk[ZT_MAX_CHUNKSIZE];	/* Last transmitted samples from last chunk */
	short	putlin[ZT_MAX_CHUNKSIZE];			/* Last received samples */
	unsigned char putraw[ZT_MAX_CHUNKSIZE];		/* Last received raw data */
	short	conflast[ZT_MAX_CHUNKSIZE];			/* Last conference sample -- base part of channel */
	short	conflast1[ZT_MAX_CHUNKSIZE];		/* Last conference sample  -- pseudo part of channel */
	short	conflast2[ZT_MAX_CHUNKSIZE];		/* Previous last conference sample -- pseudo part of channel */
	

	/* Is echo cancellation enabled or disabled */
	int		echocancel;
	struct echo_can_state	*ec;
	echo_can_disable_detector_state_t txecdis;
	echo_can_disable_detector_state_t rxecdis;
	
	int 	echostate;		/* State of echo canceller */
	int		echolastupdate;	/* Last echo can update pos */
	int		echotimer;		/* Timer for echo update */

	/* RBS timings  */
	int		prewinktime;  /* pre-wink time (ms) */
	int		preflashtime;	/* pre-flash time (ms) */
	int		winktime;  /* wink time (ms) */
	int		flashtime;  /* flash time (ms) */
	int		starttime;  /* start time (ms) */
	int		rxwinktime;  /* rx wink time (ms) */
	int		rxflashtime; /* rx flash time (ms) */
	int		debouncetime;  /* FXS GS sig debounce time (ms) */
	int		pulsebreaktime; /* pulse line open time (ms) */
	int		pulsemaketime;  /* pulse line closed time (ms) */
	int		pulseaftertime; /* pulse time between digits (ms) */

	/* RING debounce timer */
	int	ringdebtimer;
	
	/* RING trailing detector to make sure a RING is really over */
	int ringtrailer;

	/* PULSE digit receiver stuff */
	int	pulsecount;
	int	pulsetimer;

	/* RBS timers */
	int 	itimerset;		/* what the itimer was set to last */
	int 	itimer;
	int 	otimer;
	
	/* RBS state */
	int gotgs;
	int txstate;
	int rxsig;
	int txsig;
	int rxsigstate;

	/* non-RBS rx state */
	int rxhooksig;
	int txhooksig;
	int kewlonhook;

	/* Idle signalling if CAS signalling */
	int idlebits;

	int deflaw;		/* 1 = mulaw, 2=alaw, 0=undefined */
	short *xlaw;
#ifdef CONFIG_CALC_XLAW
	unsigned char (*lineartoxlaw)(short a);
#else
	unsigned char *lin2x;
#endif

#ifdef CONFIG_DEVFS_FS
	devfs_handle_t fhandle;  /* File handle in devfs for the channel */
	devfs_handle_t fhandle_symlink;
#endif /* CONFIG_DEVFS_FS */
};

/* defines for transmit signalling */
typedef enum {
	ZT_TXSIG_ONHOOK,			/* On hook */
	ZT_TXSIG_OFFHOOK,			/* Off hook */
	ZT_TXSIG_START,				/* Start / Ring */
	ZT_TXSIG_KEWL				/* Drop battery if possible */
} zt_txsig_t;

typedef enum {
	ZT_RXSIG_ONHOOK,
	ZT_RXSIG_OFFHOOK,
	ZT_RXSIG_START,
	ZT_RXSIG_RING,
	ZT_RXSIG_INITIAL
} zt_rxsig_t;
	

/* Span flags */
#define ZT_FLAG_REGISTERED		(1 << 0)
#define ZT_FLAG_RUNNING			(1 << 1)
#define ZT_FLAG_RBS			(1 << 12)	/* Span uses RBS signalling */

/* Channel flags */
#define ZT_FLAG_DTMFDECODE		(1 << 2)	/* Channel supports native DTMF decode */
#define ZT_FLAG_MFDECODE		(1 << 3)	/* Channel supports native MFr2 decode */
#define ZT_FLAG_ECHOCANCEL		(1 << 4)	/* Channel supports native echo cancellation */

#define ZT_FLAG_HDLC			(1 << 5)	/* Perform HDLC */
#define ZT_FLAG_NETDEV			(1 << 6)	/* Send to network */
#define ZT_FLAG_PSEUDO			(1 << 7)	/* Pseudo channel */
#define ZT_FLAG_CLEAR			(1 << 8)	/* Clear channel */
#define ZT_FLAG_AUDIO			(1 << 9)	/* Audio mode channel */

#define ZT_FLAG_OPEN			(1 << 10)	/* Channel is open */
#define ZT_FLAG_FCS			(1 << 11)	/* Calculate FCS */
/* Reserve 12 for uniqueness with span flags */
#define ZT_FLAG_LINEAR			(1 << 13)	/* Talk to user space in linear */
#define ZT_FLAG_PPP			(1 << 14)	/* PPP is available */
#define ZT_FLAG_T1PPP			(1 << 15)
#define ZT_FLAG_SIGFREEZE		(1 << 16)	/* Freeze signalling */
#define ZT_FLAG_NOSTDTXRX		(1 << 17)	/* Do NOT do standard transmit and receive on every interrupt */
#define ZT_FLAG_LOOPED			(1 << 18)	/* Loopback the receive data from the channel to the transmit */

struct zt_span {
	spinlock_t lock;
	void *pvt;			/* Private stuff */
	char name[40];			/* Span name */
	char desc[80];			/* Span description */
	int deflaw;			/* Default law (ZT_MULAW or ZT_ALAW) */
	int alarms;			/* Pending alarms on span */
	int flags;
	int irq;			/* IRQ for this span's hardware */
	int lbo;			/* Span Line-Buildout */
	int lineconfig;			/* Span line configuration */
	int linecompat;			/* Span line compatibility */
	int channels;			/* Number of channels in span */
	int txlevel;			/* Tx level */
	int rxlevel;			/* Rx level */
	int syncsrc;			/* current sync src (gets copied here) */
	unsigned int bpvcount;		/* BPV counter */
	unsigned int crc4count;	        /* CRC4 error counter */
	unsigned int ebitcount;		/* current E-bit error count */
	unsigned int fascount;		/* current FAS error count */

	int maintstat;			/* Maintenance state */
	wait_queue_head_t maintq;	/* Maintenance queue */
	int mainttimer;			/* Maintenance timer */
	
	int irqmisses;			/* Interrupt misses */

	int timingslips;			/* Clock slips */

	struct zt_chan *chans;		/* Member channel structures */

	/*   ==== Span Callback Operations ====   */
	/* Req: Set the requested chunk size.  This is the unit in which you must
	   report results for conferencing, etc */
	int (*setchunksize)(struct zt_span *span, int chunksize);

	/* Opt: Configure the span (if appropriate) */
	int (*spanconfig)(struct zt_span *span, struct zt_lineconfig *lc);
	
	/* Opt: Start the span */
	int (*startup)(struct zt_span *span);
	
	/* Opt: Shutdown the span */
	int (*shutdown)(struct zt_span *span);
	
	/* Opt: Enable maintenance modes */
	int (*maint)(struct zt_span *span, int mode);

#ifdef	ZAPTEL_SYNC_TICK
	/* Opt: send sync to spans */
	int (*sync_tick)(struct zt_span *span, int is_master);
#endif

	/* ====  Channel Callback Operations ==== */
	/* Opt: Set signalling type (if appropriate) */
	int (*chanconfig)(struct zt_chan *chan, int sigtype);

	/* Opt: Prepare a channel for I/O */
	int (*open)(struct zt_chan *chan);

	/* Opt: Close channel for I/O */
	int (*close)(struct zt_chan *chan);
	
	/* Opt: IOCTL */
	int (*ioctl)(struct zt_chan *chan, unsigned int cmd, unsigned long data);
	
	/* Opt: Native echo cancellation */
	int (*echocan)(struct zt_chan *chan, int ecval);

	/* Okay, now we get to the signalling.  You have several options: */

	/* Option 1: If you're a T1 like interface, you can just provide a
	   rbsbits function and we'll assert robbed bits for you.  Be sure to 
	   set the ZT_FLAG_RBS in this case.  */

	/* Opt: If the span uses A/B bits, set them here */
	int (*rbsbits)(struct zt_chan *chan, int bits);
	
	/* Option 2: If you don't know about sig bits, but do have their
	   equivalents (i.e. you can disconnect battery, detect off hook,
	   generate ring, etc directly) then you can just specify a
	   sethook function, and we'll call you with appropriate hook states
	   to set.  Still set the ZT_FLAG_RBS in this case as well */
	int (*hooksig)(struct zt_chan *chan, zt_txsig_t hookstate);
	
	/* Option 3: If you can't use sig bits, you can write a function
	   which handles the individual hook states  */
	int (*sethook)(struct zt_chan *chan, int hookstate);
	
	/* Opt: Dacs the contents of chan2 into chan1 if possible */
	int (*dacs)(struct zt_chan *chan1, struct zt_chan *chan2);

	/* Opt: Used to tell an onboard HDLC controller that there is data ready to transmit */
	void (*hdlc_hard_xmit)(struct zt_chan *chan);

	/* Used by zaptel only -- no user servicable parts inside */
	int spanno;			/* Span number for zaptel */
	int offset;			/* Offset within a given card */
	int lastalarms;		/* Previous alarms */

#ifdef CONFIG_DEVFS_FS
	devfs_handle_t dhandle;  /* Directory name */
#endif	
	/* If the watchdog detects no received data, it will call the
	   watchdog routine */
	int (*watchdog)(struct zt_span *span, int cause);
#ifdef CONFIG_ZAPTEL_WATCHDOG
	int watchcounter;
	int watchstate;
#endif	
};

struct zt_transcoder_channel {
	void *pvt;
	struct zt_transcoder *parent;
	wait_queue_head_t ready;
	int errorstatus;
	int offset;
	unsigned int chan_built;
	unsigned int built_fmts;
	unsigned int flags;
	unsigned int srcfmt;
	unsigned int dstfmt;
	struct zt_transcode_header *tch;
};

#define ZT_TC_FLAG_BUSY       (1 << 0)
#define ZT_TC_FLAG_TRANSIENT  (1 << 1)


struct zt_transcoder {
	struct zt_transcoder *next;
	char name[80];
	int numchannels;
	unsigned int srcfmts;
	unsigned int dstfmts;
	int (*operation)(struct zt_transcoder_channel *channel, int op);
	/* Transcoder channels */
	struct zt_transcoder_channel channels[0];
};

#define ZT_WATCHDOG_NOINTS		(1 << 0)

#define ZT_WATCHDOG_INIT			1000

#define ZT_WATCHSTATE_UNKNOWN		0
#define ZT_WATCHSTATE_OK			1
#define ZT_WATCHSTATE_RECOVERING	2
#define ZT_WATCHSTATE_FAILED		3


struct zt_dynamic_driver {
	/* Driver name (e.g. Eth) */
	char name[20];

	/* Driver description */
	char desc[80];

	/* Create a new transmission pipe */
	void *(*create)(struct zt_span *span, char *address);

	/* Destroy a created transmission pipe */
	void (*destroy)(void *tpipe);

	/* Transmit a given message */
	int (*transmit)(void *tpipe, unsigned char *msg, int msglen);

	/* Flush any pending messages */
	int (*flush)(void);

	struct zt_dynamic_driver *next;
};

/* Receive a dynamic span message */
void zt_dynamic_receive(struct zt_span *span, unsigned char *msg, int msglen);

/* Register a dynamic driver */
int zt_dynamic_register(struct zt_dynamic_driver *driver);

/* Unregister a dynamic driver */
void zt_dynamic_unregister(struct zt_dynamic_driver *driver);

/* Receive on a span.  The zaptel interface will handle all the calculations for
   all member channels of the span, pulling the data from the readchunk buffer */
int zt_receive(struct zt_span *span);

/* Prepare writechunk buffers on all channels for this span */
int zt_transmit(struct zt_span *span);

/* Abort the buffer currently being receive with event "event" */
void zt_hdlc_abort(struct zt_chan *ss, int event);

/* Indicate to zaptel that the end of frame was received and rotate buffers */
void zt_hdlc_finish(struct zt_chan *ss);

/* Put a chunk of data into the current receive buffer */
void zt_hdlc_putbuf(struct zt_chan *ss, unsigned char *rxb, int bytes);

/* Get a chunk of data from the current transmit buffer.  Returns -1 if no data
 * is left to send, 0 if there is data remaining in the current message to be sent
 * and 1 if the currently transmitted message is now done */
int zt_hdlc_getbuf(struct zt_chan *ss, unsigned char *bufptr, unsigned int *size);


/* Register a span.  Returns 0 on success, -1 on failure.  Pref-master is non-zero if
   we should have preference in being the master device */
int zt_register(struct zt_span *span, int prefmaster);

/* Allocate / free memory for a transcoder */
struct zt_transcoder *zt_transcoder_alloc(int numchans);
void zt_transcoder_free(struct zt_transcoder *ztc);

/* Register a transcoder */
int zt_transcoder_register(struct zt_transcoder *tc);

/* Unregister a transcoder */
int zt_transcoder_unregister(struct zt_transcoder *tc);

/* Alert a transcoder */
int zt_transcoder_alert(struct zt_transcoder_channel *ztc);

/* Unregister a span */
int zt_unregister(struct zt_span *span);

/* Gives a name to an LBO */
char *zt_lboname(int lbo);

/* Tell Zaptel about changes in received rbs bits */
void zt_rbsbits(struct zt_chan *chan, int bits);

/* Tell Zaptel abou changes in received signalling */
void zt_hooksig(struct zt_chan *chan, zt_rxsig_t rxsig);

/* Queue an event on a channel */
void zt_qevent_nolock(struct zt_chan *chan, int event);

/* Queue an event on a channel, locking it first */
void zt_qevent_lock(struct zt_chan *chan, int event);

/* Notify a change possible change in alarm status */
void zt_alarm_notify(struct zt_span *span);

/* Initialize a tone state */
void zt_init_tone_state(struct zt_tone_state *ts, struct zt_tone *zt);

/* Get a given DTMF or MF tone struct, suitable for zt_tone_nextsample.
   Set 'mf' to 0 for DTMF or 1 for MFv1 */
struct zt_tone *zt_dtmf_tone(char digit, int mf);

/* Echo cancel a receive and transmit chunk for a given channel.  This
   should be called by the low-level driver as close to the interface
   as possible.  ECHO CANCELLATION IS NO LONGER AUTOMATICALLY DONE
   AT THE ZAPTEL LEVEL.  zt_ec_chunk will not echo cancel if it should
   not be doing so.  rxchunk is modified in-place */

void zt_ec_chunk(struct zt_chan *chan, unsigned char *rxchunk, const unsigned char *txchunk);
void zt_ec_span(struct zt_span *span);

extern struct file_operations *zt_transcode_fops;

/* Don't use these directly -- they're not guaranteed to
   be there. */
extern short __zt_mulaw[256];
extern short __zt_alaw[256];
#ifdef CONFIG_CALC_XLAW
u_char __zt_lineartoulaw(short a);
u_char __zt_lineartoalaw(short a);
#else
extern u_char __zt_lin2mu[16384];
extern u_char __zt_lin2a[16384];
#endif

/* Used by dynamic zaptel -- don't use directly */
void zt_set_dynamic_ioctl(int (*func)(unsigned int cmd, unsigned long data));

/* Used privately by zaptel.  Avoid touching directly */
struct zt_tone {
	int fac1;
	int init_v2_1;
	int init_v3_1;

	int fac2;
	int init_v2_2;
	int init_v3_2;

	int tonesamples;		/* How long to play this tone before 
					   going to the next (in samples) */
	struct zt_tone *next;		/* Next tone in this sequence */

	int modulate;
};

static inline short zt_tone_nextsample(struct zt_tone_state *ts, struct zt_tone *zt)
{
	/* follow the curves, return the sum */

	int p;

	ts->v1_1 = ts->v2_1;
	ts->v2_1 = ts->v3_1;
	ts->v3_1 = (zt->fac1 * ts->v2_1 >> 15) - ts->v1_1;

	ts->v1_2 = ts->v2_2;
	ts->v2_2 = ts->v3_2;
	ts->v3_2 = (zt->fac2 * ts->v2_2 >> 15) - ts->v1_2;

	/* Return top 16 bits */
	if (!ts->modulate) return ts->v3_1 + ts->v3_2;
	/* we are modulating */
	p = ts->v3_2 - 32768;
	if (p < 0) p = -p;
	p = ((p * 9) / 10) + 1;
	return (ts->v3_1 * p) >> 15;

}

static inline short zt_txtone_nextsample(struct zt_chan *ss)
{
	/* follow the curves, return the sum */

	ss->v1_1 = ss->v2_1;
	ss->v2_1 = ss->v3_1;
	ss->v3_1 = (ss->txtone * ss->v2_1 >> 15) - ss->v1_1;
	return ss->v3_1;
}

/* These are the right functions to use.  */

#define ZT_MULAW(a) (__zt_mulaw[(a)])
#define ZT_ALAW(a) (__zt_alaw[(a)])
#define ZT_XLAW(a,c) (c->xlaw[(a)])

#ifdef CONFIG_CALC_XLAW
#define ZT_LIN2MU(a) (__zt_lineartoulaw((a)))
#define ZT_LIN2A(a) (__zt_lineartoalaw((a)))

#define ZT_LIN2X(a,c) ((c)->lineartoxlaw((a)))

#else
/* Use tables */
#define ZT_LIN2MU(a) (__zt_lin2mu[((unsigned short)(a)) >> 2])
#define ZT_LIN2A(a) (__zt_lin2a[((unsigned short)(a)) >> 2])

/* Manipulate as appropriate for x-law */
#define ZT_LIN2X(a,c) ((c)->lin2x[((unsigned short)(a)) >> 2])

#endif /* CONFIG_CALC_XLAW */

#endif /* __KERNEL__ */

/* The following is for the PCI RADIO interface only. This is specified in
this file because external processes need to interact with the device.
Some devices have private functions used for test/diagnostic only, but
this is not the case here. */

struct zt_radio_stat {
	unsigned short ctcode_rx;	/* code of currently received CTCSS 
					   or DCS, 0 for none */
	unsigned short ctclass;		/* class of currently received CTCSS or
					    DCS code */
	unsigned short ctcode_tx;	/* code of currently encoded CTCSS or
					   DCS, 0 for none */
	unsigned char radstat;		/* status bits of radio */
};

#define	RAD_SERIAL_BUFLEN 128

struct zt_radio_param {
	unsigned short radpar;	/* param identifier */
	unsigned short index;	/* tone number */
	int data;		/* param */
	int data2;		/* param 2 */
	unsigned char buf[RAD_SERIAL_BUFLEN];
};


/* Get current status IOCTL */
#define	ZT_RADIO_GETSTAT	_IOR (ZT_CODE, 57, struct zt_radio_stat)
/* Set a channel parameter IOCTL */
#define	ZT_RADIO_SETPARAM	_IOW (ZT_CODE, 58, struct zt_radio_param)
/* Get a channel parameter IOCTL */
#define	ZT_RADIO_GETPARAM	_IOR (ZT_CODE, 59, struct zt_radio_param)


/* Defines for Radio Status (zt_radio_stat.radstat) bits */

#define	ZT_RADSTAT_RX	1	/* currently "receiving " */
#define	ZT_RADSTAT_TX	2	/* currently "transmitting" */
#define	ZT_RADSTAT_RXCT	4	/* currently receiving continuous tone with 
				   current settings */
#define	ZT_RADSTAT_RXCOR	8	/* currently receiving COR (irrelevant of COR
				   ignore) */
#define	ZT_RADSTAT_IGNCOR	16	/* currently ignoring COR */
#define	ZT_RADSTAT_IGNCT	32	/* currently ignoring CTCSS/DCS decode */
#define	ZT_RADSTAT_NOENCODE 64	/* currently blocking CTCSS/DCS encode */

/* Defines for Radio Parameters (zt_radio_param.radpar) */

#define	ZT_RADPAR_INVERTCOR 1	/* invert the COR signal (0/1) */
#define	ZT_RADPAR_IGNORECOR 2	/* ignore the COR signal (0/1) */
#define	ZT_RADPAR_IGNORECT 3	/* ignore the CTCSS/DCS decode (0/1) */
#define	ZT_RADPAR_NOENCODE 4	/* block the CTCSS/DCS encode (0/1) */
#define	ZT_RADPAR_CORTHRESH 5	/* COR trigger threshold (0-7) */

#define	ZT_RADPAR_EXTRXTONE 6	/* 0 means use internal decoder, 1 means UIOA
				   logic true is CT decode, 2 means UIOA logic
				   false is CT decode */
#define	ZT_RADPAR_NUMTONES	7	/* returns maximum tone index (curently 15) */
#define	ZT_RADPAR_INITTONE	8	/* init all tone indexes to 0 (no tones) */
#define	ZT_RADPAR_RXTONE	9	/* CTCSS tone, (1-32) or DCS tone (1-777),
				   or 0 meaning no tone, set index also (1-15) */
#define	ZT_RADPAR_RXTONECLASS 10	/* Tone class (0-65535), set index also (1-15) */
#define	ZT_RADPAR_TXTONE 11	/* CTCSS tone (1-32) or DCS tone (1-777) or 0
				   to indicate no tone, to transmit 
				   for this tone index (0-32, 0 disables
				   transmit CTCSS), set index also (0-15) */
#define	ZT_RADPAR_DEBOUNCETIME 12	/* receive indication debounce time, 
				   milliseconds (1-999) */
#define	ZT_RADPAR_BURSTTIME 13	/* end of transmit with no CT tone in
				   milliseconds (0-999) */


#define	ZT_RADPAR_UIODATA 14	/* read/write UIOA and UIOB data. Bit 0 is
				   UIOA, bit 1 is UIOB */
#define	ZT_RADPAR_UIOMODE 15	/* 0 means UIOA and UIOB are both outputs, 1
				   means UIOA is input, UIOB is output, 2 
				   means UIOB is input and UIOA is output,
				   3 means both UIOA and UIOB are inputs. Note
				   mode for UIOA is overridden when in
				   EXTRXTONE mode. */

#define	ZT_RADPAR_REMMODE 16	/* Remote control data mode */
	#define	ZT_RADPAR_REM_NONE 0 	/* no remote control data mode */
	#define	ZT_RADPAR_REM_RBI1 1	/* Doug Hall RBI-1 data mode */
	#define	ZT_RADPAR_REM_SERIAL 2	/* Serial Data, 9600 BPS */
	#define	ZT_RADPAR_REM_SERIAL_ASCII 3	/* Serial Ascii Data, 9600 BPS */

#define	ZT_RADPAR_REMCOMMAND 17	/* Remote conrtol write data block & do cmd */

/* Data formats for capabilities and frames alike (from Asterisk) */
/*! G.723.1 compression */
#define ZT_FORMAT_G723_1	(1 << 0)
/*! GSM compression */
#define ZT_FORMAT_GSM		(1 << 1)
/*! Raw mu-law data (G.711) */
#define ZT_FORMAT_ULAW		(1 << 2)
/*! Raw A-law data (G.711) */
#define ZT_FORMAT_ALAW		(1 << 3)
/*! ADPCM (G.726, 32kbps) */
#define ZT_FORMAT_G726		(1 << 4)
/*! ADPCM (IMA) */
#define ZT_FORMAT_ADPCM		(1 << 5)
/*! Raw 16-bit Signed Linear (8000 Hz) PCM */
#define ZT_FORMAT_SLINEAR	(1 << 6)
/*! LPC10, 180 samples/frame */
#define ZT_FORMAT_LPC10		(1 << 7)
/*! G.729A audio */
#define ZT_FORMAT_G729A		(1 << 8)
/*! SpeeX Free Compression */
#define ZT_FORMAT_SPEEX		(1 << 9)
/*! iLBC Free Compression */
#define ZT_FORMAT_ILBC		(1 << 10)
/*! Maximum audio format */
#define ZT_FORMAT_MAX_AUDIO	(1 << 15)
/*! Maximum audio mask */
#define ZT_FORMAT_AUDIO_MASK	((1 << 16) - 1)

#define	ZT_RADPAR_DEEMP 18 /* Audio De-empahsis (on or off) */ 

#define	ZT_RADPAR_PREEMP 19 /* Audio Pre-empahsis (on or off) */ 

#define	ZT_RADPAR_RXGAIN 20 /* Audio (In to system) Rx Gain */ 

#define	ZT_RADPAR_TXGAIN 21 /* Audio (Out from system) Tx Gain */ 

struct torisa_debug {
	unsigned int txerrors;
	unsigned int irqcount;
	unsigned int taskletsched;
	unsigned int taskletrun;
	unsigned int taskletexec;
	int span1flags;
	int span2flags;
};

/* Special torisa ioctl */
#define TORISA_GETDEBUG		_IOW (ZT_CODE, 60, struct torisa_debug)

#endif /* _LINUX_ZAPTEL_H */
