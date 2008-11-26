/*
 * Zapata Telephony Interface Driver
 *
 * Written by Mark Spencer <markster@digium.com>
 * Based on previous works, designs, and architectures conceived and
 * written by Jim Dixon <jim@lambdatel.com>.
 * 
 * Special thanks to Steve Underwood <steve@coppice.org>
 * for substantial contributions to signal processing functions 
 * in zaptel and the zapata library.
 *
 * Yury Bokhoncovich <byg@cf1.ru>
 * Adaptation for 2.4.20+ kernels (HDLC API was changed)
 * The work has been performed as a part of our move
 * from Cisco 3620 to IBM x305 here in F1 Group
 *
 * Copyright (C) 2001 Jim Dixon / Zapata Telephony.
 * Copyright (C) 2001 -2006 Digium, Inc.
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


#include "zconfig.h"
#include "version.h"

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kmod.h>
#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif /* CONFIG_DEVFS_FS */
#ifdef CONFIG_ZAPATA_NET
#include <linux/netdevice.h>
#endif /* CONFIG_ZAPATA_NET */
#include <linux/ppp_defs.h>
#ifdef CONFIG_ZAPATA_PPP
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ppp.h>
#endif

#ifndef CONFIG_OLD_HDLC_API
#define NEW_HDLC_INTERFACE
#endif

#define __ECHO_STATE_MUTE			(1 << 8)
#define ECHO_STATE_IDLE				(0)
#define ECHO_STATE_PRETRAINING		(1 | (__ECHO_STATE_MUTE))
#define ECHO_STATE_STARTTRAINING	(2 | (__ECHO_STATE_MUTE))
#define ECHO_STATE_AWAITINGECHO		(3 | (__ECHO_STATE_MUTE))
#define ECHO_STATE_TRAINING			(4 | (__ECHO_STATE_MUTE))
#define ECHO_STATE_ACTIVE			(5)

/* #define BUF_MUNGE */

/* Grab fasthdlc with tables */
#define FAST_HDLC_NEED_TABLES
#include "fasthdlc.h"

#include "zaptel.h"

#ifdef LINUX26
#include <linux/moduleparam.h>
#endif

/* Get helper arithmetic */
#include "arith.h"
#if defined(CONFIG_ZAPTEL_MMX) || defined(ECHO_CAN_FP)
#include <asm/i387.h>
#endif

#define hdlc_to_ztchan(h) (((struct zt_hdlc *)(h))->chan)
#define dev_to_ztchan(h) (((struct zt_hdlc *)(dev_to_hdlc(h)->priv))->chan)
#ifdef LINUX26
#define ztchan_to_dev(h) ((h)->hdlcnetdev->netdev)
#else
#define ztchan_to_dev(h) (&((h)->hdlcnetdev->netdev.netdev))
#endif

/* macro-oni for determining a unit (channel) number */
#define	UNIT(file) MINOR(file->f_dentry->d_inode->i_rdev)

/* names of tx level settings */
static char *zt_txlevelnames[] = {
"0 db (CSU)/0-133 feet (DSX-1)",
"133-266 feet (DSX-1)",
"266-399 feet (DSX-1)",
"399-533 feet (DSX-1)",
"533-655 feet (DSX-1)",
"-7.5db (CSU)",
"-15db (CSU)",
"-22.5db (CSU)"
} ;

EXPORT_SYMBOL(zt_transcode_fops);
EXPORT_SYMBOL(zt_init_tone_state);
EXPORT_SYMBOL(zt_dtmf_tone);
EXPORT_SYMBOL(zt_register);
EXPORT_SYMBOL(zt_unregister);
EXPORT_SYMBOL(__zt_mulaw);
EXPORT_SYMBOL(__zt_alaw);
#ifdef CONFIG_CALC_XLAW
EXPORT_SYMBOL(__zt_lineartoulaw);
EXPORT_SYMBOL(__zt_lineartoalaw);
#else
EXPORT_SYMBOL(__zt_lin2mu);
EXPORT_SYMBOL(__zt_lin2a);
#endif
EXPORT_SYMBOL(zt_lboname);
EXPORT_SYMBOL(zt_transmit);
EXPORT_SYMBOL(zt_receive);
EXPORT_SYMBOL(zt_rbsbits);
EXPORT_SYMBOL(zt_qevent_nolock);
EXPORT_SYMBOL(zt_qevent_lock);
EXPORT_SYMBOL(zt_hooksig);
EXPORT_SYMBOL(zt_alarm_notify);
EXPORT_SYMBOL(zt_set_dynamic_ioctl);
EXPORT_SYMBOL(zt_ec_chunk);
EXPORT_SYMBOL(zt_ec_span);
EXPORT_SYMBOL(zt_hdlc_abort);
EXPORT_SYMBOL(zt_hdlc_finish);
EXPORT_SYMBOL(zt_hdlc_getbuf);
EXPORT_SYMBOL(zt_hdlc_putbuf);

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_entries[ZT_MAX_SPANS]; 
#endif

/* Here are a couple important little additions for devfs */
#ifdef CONFIG_DEVFS_FS
static devfs_handle_t zaptel_devfs_dir;
static devfs_handle_t channel;
static devfs_handle_t pseudo;
static devfs_handle_t ctl;
static devfs_handle_t timer;
static devfs_handle_t transcode;
#endif

/* udev necessary data structures.  Yeah! */
#ifdef CONFIG_ZAP_UDEV

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
#define CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, NULL, devt, device, name)
#else
#define CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, devt, device, name)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
static struct class *zap_class = NULL;
#else
static struct class_simple *zap_class = NULL;
#define class_create class_simple_create
#define class_destroy class_simple_destroy
#define class_device_create class_simple_device_add
#define class_device_destroy(a, b) class_simple_device_remove(b)
#endif

#endif /* CONFIG_ZAP_UDEV */


/* There is a table like this in the PPP driver, too */

static int deftaps = 64;

#if !defined(LINUX26)
static 
__u16 fcstab[256] =
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
#endif

static int debug;

/* states for transmit signalling */
typedef enum {ZT_TXSTATE_ONHOOK,ZT_TXSTATE_OFFHOOK,ZT_TXSTATE_START,
	ZT_TXSTATE_PREWINK,ZT_TXSTATE_WINK,ZT_TXSTATE_PREFLASH,
	ZT_TXSTATE_FLASH,ZT_TXSTATE_DEBOUNCE,ZT_TXSTATE_AFTERSTART,
	ZT_TXSTATE_RINGON,ZT_TXSTATE_RINGOFF,ZT_TXSTATE_KEWL,
	ZT_TXSTATE_AFTERKEWL,ZT_TXSTATE_PULSEBREAK,ZT_TXSTATE_PULSEMAKE,
	ZT_TXSTATE_PULSEAFTER
	} ZT_TXSTATE_t;

typedef short sumtype[ZT_MAX_CHUNKSIZE];

static sumtype sums[(ZT_MAX_CONF + 1) * 3];

/* Translate conference aliases into actual conferences 
   and vice-versa */
static short confalias[ZT_MAX_CONF + 1];
static short confrev[ZT_MAX_CONF + 1];

static sumtype *conf_sums_next;
static sumtype *conf_sums;
static sumtype *conf_sums_prev;

static struct zt_span *master;
static struct file_operations zt_fops;
struct file_operations *zt_transcode_fops = NULL;

static struct
{
	int	src;	/* source conf number */
	int	dst;	/* dst conf number */
} conf_links[ZT_MAX_CONF + 1];


/* There are three sets of conference sum accumulators. One for the current
sample chunk (conf_sums), one for the next sample chunk (conf_sums_next), and
one for the previous sample chunk (conf_sums_prev). The following routine 
(rotate_sums) "rotates" the pointers to these accululator arrays as part
of the events of sample chink processing as follows:

The following sequence is designed to be looked at from the reference point
of the receive routine of the master span.

1. All (real span) receive chunks are processed (with putbuf). The last one
to be processed is the master span. The data received is loaded into the
accumulators for the next chunk (conf_sums_next), to be in alignment with
current data after rotate_sums() is called (which immediately follows).
Keep in mind that putbuf is *also* a transmit routine for the pseudo parts
of channels that are in the REALANDPSEUDO conference mode. These channels
are processed from data in the current sample chunk (conf_sums), being
that this is a "transmit" function (for the pseudo part).

2. rotate_sums() is called.

3. All pseudo channel receive chunks are processed. This data is loaded into
the current sample chunk accumulators (conf_sums).

4. All conference links are processed (being that all receive data for this
chunk has already been processed by now).

5. All pseudo channel transmit chunks are processed. This data is loaded from
the current sample chunk accumulators (conf_sums).

6. All (real span) transmit chunks are processed (with getbuf).  This data is
loaded from the current sample chunk accumulators (conf_sums). Keep in mind
that getbuf is *also* a receive routine for the pseudo part of channels that
are in the REALANDPSEUDO conference mode. These samples are loaded into
the next sample chunk accumulators (conf_sums_next) to be processed as part
of the next sample chunk's data (next time around the world).

*/

#define DIGIT_MODE_DTMF 	0
#define DIGIT_MODE_MFV1		1
#define DIGIT_MODE_PULSE	2

#include "digits.h"

static struct zt_tone *dtmf_tones_continuous = NULL;
static struct zt_tone *mfv1_tones_continuous = NULL;

static int zt_chan_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data, int unit);

#if defined(CONFIG_ZAPTEL_MMX) || defined(ECHO_CAN_FP)
/* XXX kernel_fpu_begin() is NOT exported properly (in 2.4), so we have to make
       a local version.  Somebody fix this! XXX */

#ifndef LINUX26
static inline void __save_init_fpu( struct task_struct *tsk )
{
	if ( cpu_has_fxsr ) {
		asm volatile( "fxsave %0 ; fnclex"
			      : "=m" (tsk->thread.i387.fxsave) );
	} else {
		asm volatile( "fnsave %0 ; fwait"
			      : "=m" (tsk->thread.i387.fsave) );
	}
	tsk->flags &= ~PF_USEDFPU;
}

static inline void zt_kernel_fpu_begin(void)
{
	struct task_struct *tsk = current;
	if (tsk->flags & PF_USEDFPU) {
		__save_init_fpu(tsk);
		return;
	}
	clts();
}
#else
#define zt_kernel_fpu_begin kernel_fpu_begin
#endif /* LINUX26 */
#endif	

static struct zt_timer {
	int ms;			/* Countdown */
	int pos;		/* Position */
	int ping;		/* Whether we've been ping'd */
	int tripped;	/* Whether we're tripped */
	struct zt_timer *next;	/* Linked list */
	wait_queue_head_t sel;
} *zaptimers = NULL;

#ifdef DEFINE_SPINLOCK
static DEFINE_SPINLOCK(zaptimerlock);
static DEFINE_SPINLOCK(bigzaplock);
#else
static spinlock_t zaptimerlock = SPIN_LOCK_UNLOCKED;
static spinlock_t bigzaplock = SPIN_LOCK_UNLOCKED;
#endif

struct zt_zone {
	char name[40];	/* Informational, only */
	int ringcadence[ZT_MAX_CADENCE];
	struct zt_tone *tones[ZT_TONE_MAX]; 
	/* Each of these is a circular list
	   of zt_tones to generate what we
	   want.  Use NULL if the tone is
	   unavailable */
};

static struct zt_span *spans[ZT_MAX_SPANS];
static struct zt_chan *chans[ZT_MAX_CHANNELS]; 

static int maxspans = 0;
static int maxchans = 0;
static int maxconfs = 0;
static int maxlinks = 0;

static int default_zone = DEFAULT_TONE_ZONE;

short __zt_mulaw[256];
short __zt_alaw[256];

#ifndef CONFIG_CALC_XLAW
u_char __zt_lin2mu[16384];

u_char __zt_lin2a[16384];
#endif

static u_char defgain[256];

#ifdef DEFINE_RWLOCK
static DEFINE_RWLOCK(zone_lock);
static DEFINE_RWLOCK(chan_lock);
#else
static rwlock_t zone_lock = RW_LOCK_UNLOCKED;
static rwlock_t chan_lock = RW_LOCK_UNLOCKED;
#endif

static struct zt_zone *tone_zones[ZT_TONE_ZONE_MAX];

#define NUM_SIGS	10	


#ifdef AGGRESSIVE_SUPPRESSOR
#define ZAPTEL_ECHO_AGGRESSIVE " (aggressive)"
#else
#define ZAPTEL_ECHO_AGGRESSIVE ""
#endif

/* Echo cancellation */
#if defined(ECHO_CAN_HPEC)
#include "hpec/hpec_zaptel.h"
#elif defined(ECHO_CAN_STEVE)
#include "sec.h"
#elif defined(ECHO_CAN_STEVE2)
#include "sec-2.h"
#elif defined(ECHO_CAN_MARK)
#include "mec.h"
#elif defined(ECHO_CAN_MARK2)
#include "mec2.h"
#elif defined(ECHO_CAN_KB1)
#include "kb1ec.h"
#elif defined(ECHO_CAN_MG2)
#include "mg2ec.h"
#elif defined(ECHO_CAN_JP1)
#include "jpah.h"
#else
#include "mec3.h"
#endif

static inline void rotate_sums(void)
{
	/* Rotate where we sum and so forth */
	static int pos = 0;
	conf_sums_prev = sums + (ZT_MAX_CONF + 1) * pos;
	conf_sums = sums + (ZT_MAX_CONF + 1) * ((pos + 1) % 3);
	conf_sums_next = sums + (ZT_MAX_CONF + 1) * ((pos + 2) % 3);
	pos = (pos + 1) % 3;
	memset(conf_sums_next, 0, maxconfs * sizeof(sumtype));
}

  /* return quiescent (idle) signalling states, for the various signalling types */
static int zt_q_sig(struct zt_chan *chan)
{
int	x;

static unsigned int in_sig[NUM_SIGS][2] = {
	{ ZT_SIG_NONE, 0},
	{ ZT_SIG_EM, 0 | (ZT_ABIT << 8)},
	{ ZT_SIG_FXSLS,ZT_BBIT | (ZT_BBIT << 8)},
	{ ZT_SIG_FXSGS,ZT_ABIT | ZT_BBIT | ((ZT_ABIT | ZT_BBIT) << 8)},
	{ ZT_SIG_FXSKS,ZT_BBIT | ZT_BBIT | ((ZT_ABIT | ZT_BBIT) << 8)},
	{ ZT_SIG_FXOLS,0 | (ZT_ABIT << 8)},
	{ ZT_SIG_FXOGS,ZT_BBIT | ((ZT_ABIT | ZT_BBIT) << 8)},
	{ ZT_SIG_FXOKS,0 | (ZT_ABIT << 8)},
	{ ZT_SIG_SF, 0},
	{ ZT_SIG_EM_E1, ZT_DBIT | ((ZT_ABIT | ZT_DBIT) << 8) },
	} ;

	/* must have span to begin with */
	if (!chan->span) return(-1);
	  /* if RBS does not apply, return error */
	if (!(chan->span->flags & ZT_FLAG_RBS) || 
		!chan->span->rbsbits) return(-1);
	if (chan->sig == ZT_SIG_CAS) {
		static int printed = 0;
		if (printed < 10) {
			printed++;
		}
		return chan->idlebits;
	}
	for (x=0;x<NUM_SIGS;x++) {
		if (in_sig[x][0] == chan->sig) return(in_sig[x][1]);
	}	return(-1); /* not found -- error */
}

#ifdef CONFIG_PROC_FS
static char *sigstr(int sig)
{
	switch (sig) {
		case ZT_SIG_FXSLS:
			return "FXSLS";
		case ZT_SIG_FXSKS:
			return "FXSKS";
		case ZT_SIG_FXSGS:
			return "FXSGS";
		case ZT_SIG_FXOLS:
			return "FXOLS";
		case ZT_SIG_FXOKS:
			return "FXOKS";
		case ZT_SIG_FXOGS:
			return "FXOGS";
		case ZT_SIG_EM:
			return "E&M";
		case ZT_SIG_EM_E1:
			return "E&M-E1";
		case ZT_SIG_CLEAR:
			return "Clear";
		case ZT_SIG_HDLCRAW:
			return "HDLCRAW";
		case ZT_SIG_HDLCFCS:
			return "HDLCFCS";
		case ZT_SIG_HDLCNET:
			return "HDLCNET";
		case ZT_SIG_HARDHDLC:
			return "Hardware-assisted HDLC";
		case ZT_SIG_SLAVE:
			return "Slave";
		case ZT_SIG_CAS:
			return "CAS";
		case ZT_SIG_DACS:
			return "DACS";
		case ZT_SIG_DACS_RBS:
			return "DACS+RBS";
		case ZT_SIG_SF:
			return "SF (ToneOnly)";
		case ZT_SIG_NONE:
		default:
			return "Unconfigured";
	}

}

static int zaptel_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int x, len = 0;
	long span;

	/* In Linux 2.6, this MUST NOT EXECEED 1024 bytes in one read! */

	span = (long)data;

	if (!span)
		return 0;

	if (spans[span]->name) 
		len += sprintf(page + len, "Span %ld: %s ", span, spans[span]->name);
	if (spans[span]->desc)
		len += sprintf(page + len, "\"%s\"", spans[span]->desc);
	else
		len += sprintf(page + len, "\"\"");

	if (spans[span]->lineconfig) {
		/* framing first */
		if (spans[span]->lineconfig & ZT_CONFIG_B8ZS)
			len += sprintf(page + len, " B8ZS/");
		else if (spans[span]->lineconfig & ZT_CONFIG_AMI)
			len += sprintf(page + len, " AMI/");
		else if (spans[span]->lineconfig & ZT_CONFIG_HDB3)
			len += sprintf(page + len, " HDB3/");
		/* then coding */
		if (spans[span]->lineconfig & ZT_CONFIG_ESF)
			len += sprintf(page + len, "ESF");
		else if (spans[span]->lineconfig & ZT_CONFIG_D4)
			len += sprintf(page + len, "D4");
		else if (spans[span]->lineconfig & ZT_CONFIG_CCS)
			len += sprintf(page + len, "CCS");
		/* E1's can enable CRC checking */
		if (spans[span]->lineconfig & ZT_CONFIG_CRC4)
			len += sprintf(page + len, "/CRC4");
	}

	len += sprintf(page + len, " ");

	/* list alarms */
	if (spans[span]->alarms && (spans[span]->alarms > 0)) {
		if (spans[span]->alarms & ZT_ALARM_BLUE)
			len += sprintf(page + len, "BLUE ");
		if (spans[span]->alarms & ZT_ALARM_YELLOW)
			len += sprintf(page + len, "YELLOW ");
		if (spans[span]->alarms & ZT_ALARM_RED)
			len += sprintf(page + len, "RED ");
		if (spans[span]->alarms & ZT_ALARM_LOOPBACK)
			len += sprintf(page + len, "LOOP ");
		if (spans[span]->alarms & ZT_ALARM_RECOVER)
			len += sprintf(page + len, "RECOVERING ");
		if (spans[span]->alarms & ZT_ALARM_NOTOPEN)
			len += sprintf(page + len, "NOTOPEN ");
					
	}
	if (spans[span]->syncsrc && (spans[span]->syncsrc == spans[span]->spanno))
		len += sprintf(page + len, "ClockSource ");
	len += sprintf(page + len, "\n");
	if (spans[span]->bpvcount)
		len += sprintf(page + len, "\tBPV count: %d\n", spans[span]->bpvcount);
	if (spans[span]->crc4count)
		len += sprintf(page + len, "\tCRC4 error count: %d\n", spans[span]->crc4count);
	if (spans[span]->ebitcount)
		len += sprintf(page + len, "\tE-bit error count: %d\n", spans[span]->ebitcount);
	if (spans[span]->fascount)
		len += sprintf(page + len, "\tFAS error count: %d\n", spans[span]->fascount);
	if (spans[span]->irqmisses)
		len += sprintf(page + len, "\tIRQ misses: %d\n", spans[span]->irqmisses);
	if (spans[span]->timingslips)
		len += sprintf(page + len, "\tTiming slips: %d\n", spans[span]->timingslips);
	len += sprintf(page + len, "\n");


        for (x=1;x<ZT_MAX_CHANNELS;x++) {	
		if (chans[x]) {
			if (chans[x]->span && (chans[x]->span->spanno == span)) {
				if (chans[x]->name)
					len += sprintf(page + len, "\t%4d %s ", x, chans[x]->name);
				if (chans[x]->sig) {
					if (chans[x]->sig == ZT_SIG_SLAVE)
						len += sprintf(page + len, "%s ", sigstr(chans[x]->master->sig));
					else {
						len += sprintf(page + len, "%s ", sigstr(chans[x]->sig));
						if (chans[x]->nextslave && chans[x]->master->channo == x)
							len += sprintf(page + len, "Master ");
					}
				}
				if ((chans[x]->flags & ZT_FLAG_OPEN)) {
					len += sprintf(page + len, "(In use) ");
				}
				len += sprintf(page + len, "\n");
			}
			if (len <= off) { /* If everything printed so far is before beginning of request */
				off -= len;
				len = 0;
			}
			if (len > off+count) /* stop if we've already generated enough */
				break;
		}
	}
	if (len <= off) { /* If everything printed so far is before beginning of request */
		off -= len;
		len = 0;
	}
	*start = page + off;
	len -= off;     /* un-count any remaining offset */
	if (len > count) len = count;   /* don't return bytes not asked for */
	return len;
}
#endif

static int zt_first_empty_alias(void)
{
	/* Find the first conference which has no alias pointing to it */
	int x;
	for (x=1;x<ZT_MAX_CONF;x++) {
		if (!confrev[x])
			return x;
	}
	return -1;
}

static void recalc_maxconfs(void)
{
	int x;
	for (x=ZT_MAX_CONF-1;x>0;x--) {
		if (confrev[x]) {
			maxconfs = x+1;
			return;
		}
	}
	maxconfs = 0;
}

static void recalc_maxlinks(void)
{
	int x;
	for (x=ZT_MAX_CONF-1;x>0;x--) {
		if (conf_links[x].src || conf_links[x].dst) {
			maxlinks = x+1;
			return;
		}
	}
	maxlinks = 0;
}

static int zt_first_empty_conference(void)
{
	/* Find the first conference which has no alias */
	int x;
	for (x=ZT_MAX_CONF-1;x>0;x--) {
		if (!confalias[x])
			return x;
	}
	return -1;
}

static int zt_get_conf_alias(int x)
{
	int a;
	if (confalias[x]) {
		return confalias[x];
	}

	/* Allocate an alias */
	a = zt_first_empty_alias();
	confalias[x] = a;
	confrev[a] = x;

	/* Highest conference may have changed */
	recalc_maxconfs();
	return a;
}

static void zt_check_conf(int x)
{
	int y;

	/* return if no valid conf number */
	if (x <= 0) return;
	/* Return if there is no alias */
	if (!confalias[x])
		return;
	for (y=0;y<maxchans;y++) {
		if (chans[y] && (chans[y]->confna == x) &&
			((chans[y]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONF ||
			(chans[y]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFANN ||
			(chans[y]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFMON ||
			(chans[y]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFANNMON ||
			(chans[y]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_REALANDPSEUDO))
			return;
	}
	/* If we get here, nobody is in the conference anymore.  Clear it out
	   both forward and reverse */
	confrev[confalias[x]] = 0;
	confalias[x] = 0;

	/* Highest conference may have changed */
	recalc_maxconfs();
}

/* enqueue an event on a channel */
static void __qevent(struct zt_chan *chan, int event)
{

	  /* if full, ignore */
	if ((chan->eventoutidx == 0) && (chan->eventinidx == (ZT_MAX_EVENTSIZE - 1))) 
		return;
	  /* if full, ignore */
	if (chan->eventinidx == (chan->eventoutidx - 1)) return;
	  /* save the event */
	chan->eventbuf[chan->eventinidx++] = event;
	  /* wrap the index, if necessary */
	if (chan->eventinidx >= ZT_MAX_EVENTSIZE) chan->eventinidx = 0;
	  /* wake em all up */
	if (chan->iomask & ZT_IOMUX_SIGEVENT) wake_up_interruptible(&chan->eventbufq);
	wake_up_interruptible(&chan->readbufq);
	wake_up_interruptible(&chan->writebufq);
	wake_up_interruptible(&chan->sel);
	return;
}

void zt_qevent_nolock(struct zt_chan *chan, int event)
{
	__qevent(chan, event);
}

void zt_qevent_lock(struct zt_chan *chan, int event)
{
	unsigned long flags;
	spin_lock_irqsave(&chan->lock, flags);
	__qevent(chan, event);
	spin_unlock_irqrestore(&chan->lock, flags);
}

/* sleep in user space until woken up. Equivilant of tsleep() in BSD */
static int schluffen(wait_queue_head_t *q)
{
	DECLARE_WAITQUEUE(wait, current);
	add_wait_queue(q, &wait);
	current->state = TASK_INTERRUPTIBLE;
	if (!signal_pending(current)) schedule();
	current->state = TASK_RUNNING;
	remove_wait_queue(q, &wait);
	if (signal_pending(current)) {
		return -ERESTARTSYS;
	}
	return(0);
}

static inline void calc_fcs(struct zt_chan *ss, int inwritebuf)
{
	int x;
	unsigned int fcs=PPP_INITFCS;
	unsigned char *data = ss->writebuf[inwritebuf];
	int len = ss->writen[inwritebuf];
	/* Not enough space to do FCS calculation */
	if (len < 2)
		return;
	for (x=0;x<len-2;x++)
		fcs = PPP_FCS(fcs, data[x]);
	fcs ^= 0xffff;
	/* Send out the FCS */
	data[len-2] = (fcs & 0xff);
	data[len-1] = (fcs >> 8) & 0xff;
}

static int zt_reallocbufs(struct zt_chan *ss, int j, int numbufs)
{
	unsigned char *newbuf, *oldbuf;
	unsigned long flags;
	int x;
	/* Check numbufs */
	if (numbufs < 2)
		numbufs = 2;
	if (numbufs > ZT_MAX_NUM_BUFS)
		numbufs = ZT_MAX_NUM_BUFS;
	/* We need to allocate our buffers now */
	if (j) {
		newbuf = kmalloc(j * 2 * numbufs, GFP_KERNEL);
		if (!newbuf) 
			return (-ENOMEM);
	} else
		newbuf = NULL;
	  /* Now that we've allocated our new buffer, we can safely
	     move things around... */
	spin_lock_irqsave(&ss->lock, flags);
	ss->blocksize = j; /* set the blocksize */
	oldbuf = ss->readbuf[0]; /* Keep track of the old buffer */
	ss->readbuf[0] = NULL;
	if (newbuf) {
		for (x=0;x<numbufs;x++) {
			ss->readbuf[x] = newbuf + x * j;
			ss->writebuf[x] = newbuf + (numbufs + x) * j;
		}
	} else {
		for (x=0;x<numbufs;x++) {
			ss->readbuf[x] = NULL;
			ss->writebuf[x] = NULL;
		}
	}
	/* Mark all buffers as empty */
	for (x=0;x<numbufs;x++) 
		ss->writen[x] = 
		ss->writeidx[x]=
		ss->readn[x]=
		ss->readidx[x] = 0;
	
	/* Keep track of where our data goes (if it goes
	   anywhere at all) */
	if (newbuf) {
		ss->inreadbuf = 0;
		ss->inwritebuf = 0;
	} else {
		ss->inreadbuf = -1;
		ss->inwritebuf = -1;
	}
	ss->outreadbuf = -1;
	ss->outwritebuf = -1;
	ss->numbufs = numbufs;
	if (ss->txbufpolicy == ZT_POLICY_WHEN_FULL)
		ss->txdisable = 1;
	else
		ss->txdisable = 0;

	if (ss->rxbufpolicy == ZT_POLICY_WHEN_FULL)
		ss->rxdisable = 1;
	else
		ss->rxdisable = 0;

	spin_unlock_irqrestore(&ss->lock, flags);
	if (oldbuf)
		kfree(oldbuf);
	return 0;
}

static int zt_hangup(struct zt_chan *chan);
static void zt_set_law(struct zt_chan *chan, int law);

/* Pull a ZT_CHUNKSIZE piece off the queue.  Returns
   0 on success or -1 on failure.  If failed, provides
   silence */
static int __buf_pull(struct confq *q, u_char *data, struct zt_chan *c, char *label)
{
	int oldoutbuf = q->outbuf;
	/* Ain't nuffin to read */
	if (q->outbuf < 0) {
		if (data)
			memset(data, ZT_LIN2X(0,c), ZT_CHUNKSIZE);
		return -1;
	}
	if (data)
		memcpy(data, q->buf[q->outbuf], ZT_CHUNKSIZE);
	q->outbuf = (q->outbuf + 1) % ZT_CB_SIZE;

	/* Won't be nuffin next time */
	if (q->outbuf == q->inbuf) {
		q->outbuf = -1;
	}

	/* If they thought there was no space then
	   there is now where we just read */
	if (q->inbuf < 0) 
		q->inbuf = oldoutbuf;
	return 0;
}

/* Returns a place to put stuff, or NULL if there is
   no room */

static u_char *__buf_pushpeek(struct confq *q)
{
	if (q->inbuf < 0)
		return NULL;
	return q->buf[q->inbuf];
}

static u_char *__buf_peek(struct confq *q)
{
	if (q->outbuf < 0)
		return NULL;
	return q->buf[q->outbuf];
}

#ifdef BUF_MUNGE
static u_char *__buf_cpush(struct confq *q)
{
	int pos;
	/* If we have no space, return where the
	   last space that we *did* have was */
	if (q->inbuf > -1)
		return NULL;
	pos = q->outbuf - 1;
	if (pos < 0)
		pos += ZT_CB_SIZE;
	return q->buf[pos];
}

static void __buf_munge(struct zt_chan *chan, u_char *old, u_char *new)
{
	/* Run a weighted average of the old and new, in order to
	   mask a missing sample */
	int x;
	int val;
	for (x=0;x<ZT_CHUNKSIZE;x++) {
		val = x * ZT_XLAW(new[x], chan) + (ZT_CHUNKSIZE - x - 1) * ZT_XLAW(old[x], chan);
		val = val / (ZT_CHUNKSIZE - 1);
		old[x] = ZT_LIN2X(val, chan);
	}
}
#endif
/* Push something onto the queue, or assume what
   is there is valid if data is NULL */
static int __buf_push(struct confq *q, u_char *data, char *label)
{
	int oldinbuf = q->inbuf;
	if (q->inbuf < 0) {
		return -1;
	}
	if (data)
		/* Copy in the data */
		memcpy(q->buf[q->inbuf], data, ZT_CHUNKSIZE);

	/* Advance the inbuf pointer */
	q->inbuf = (q->inbuf + 1) % ZT_CB_SIZE;

	if (q->inbuf == q->outbuf) {
		/* No space anymore... */	
		q->inbuf = -1;
	}
	/* If they don't think data is ready, let
	   them know it is now */
	if (q->outbuf < 0) {
		q->outbuf = oldinbuf;
	}
	return 0;
}

static void reset_conf(struct zt_chan *chan)
{
	int x;
	/* Empty out buffers and reset to initialization */
	for (x=0;x<ZT_CB_SIZE;x++)
		chan->confin.buf[x] = chan->confin.buffer + ZT_CHUNKSIZE * x;
	chan->confin.inbuf = 0;
	chan->confin.outbuf = -1;

	for (x=0;x<ZT_CB_SIZE;x++)
		chan->confout.buf[x] = chan->confout.buffer + ZT_CHUNKSIZE * x;
	chan->confout.inbuf = 0;
	chan->confout.outbuf = -1;
}


static void close_channel(struct zt_chan *chan)
{
	unsigned long flags;
	void *rxgain = NULL;
	struct echo_can_state *ec = NULL;
	int oldconf;
	short *readchunkpreec;
#ifdef CONFIG_ZAPATA_PPP
	struct ppp_channel *ppp;
#endif

	/* XXX Buffers should be send out before reallocation!!! XXX */
	if (!(chan->flags & ZT_FLAG_NOSTDTXRX))
		zt_reallocbufs(chan, 0, 0); 
	spin_lock_irqsave(&chan->lock, flags);
#ifdef CONFIG_ZAPATA_PPP
	ppp = chan->ppp;
	chan->ppp = NULL;
#endif
	ec = chan->ec;
	chan->ec = NULL;
	readchunkpreec = chan->readchunkpreec;
	chan->readchunkpreec = NULL;
	chan->curtone = NULL;
	chan->curzone = NULL;
	chan->cadencepos = 0;
	chan->pdialcount = 0;
	zt_hangup(chan); 
	chan->itimerset = chan->itimer = 0;
	chan->pulsecount = 0;
	chan->pulsetimer = 0;
	chan->ringdebtimer = 0;
	init_waitqueue_head(&chan->sel);
	init_waitqueue_head(&chan->readbufq);
	init_waitqueue_head(&chan->writebufq);
	init_waitqueue_head(&chan->eventbufq);
	init_waitqueue_head(&chan->txstateq);
	chan->txdialbuf[0] = '\0';
	chan->digitmode = DIGIT_MODE_DTMF;
	chan->dialing = 0;
	chan->afterdialingtimer = 0;
	  /* initialize IO MUX mask */
	chan->iomask = 0;
	/* save old conf number, if any */
	oldconf = chan->confna;
	  /* initialize conference variables */
	chan->_confn = 0;
	if ((chan->sig & __ZT_SIG_DACS) != __ZT_SIG_DACS) {
		chan->confna = 0;
		chan->confmode = 0;
	}
	chan->confmute = 0;
	/* release conference resource, if any to release */
	if (oldconf) zt_check_conf(oldconf);
	chan->gotgs = 0;
	reset_conf(chan);
	
	if (chan->gainalloc && chan->rxgain)
		rxgain = chan->rxgain;

	chan->rxgain = defgain;
	chan->txgain = defgain;
	chan->gainalloc = 0;
	chan->eventinidx = chan->eventoutidx = 0;
	chan->flags &= ~(ZT_FLAG_LOOPED | ZT_FLAG_LINEAR | ZT_FLAG_PPP | ZT_FLAG_SIGFREEZE);

	zt_set_law(chan,0);

	memset(chan->conflast, 0, sizeof(chan->conflast));
	memset(chan->conflast1, 0, sizeof(chan->conflast1));
	memset(chan->conflast2, 0, sizeof(chan->conflast2));

	if (chan->span && chan->span->dacs && oldconf)
		chan->span->dacs(chan, NULL);

	spin_unlock_irqrestore(&chan->lock, flags);

	if (chan->span && chan->span->echocan)
		chan->span->echocan(chan, 0);

	if (rxgain)
		kfree(rxgain);
	if (ec)
		echo_can_free(ec);
	if (readchunkpreec)
		kfree(readchunkpreec);

#ifdef CONFIG_ZAPATA_PPP
	if (ppp) {
		tasklet_kill(&chan->ppp_calls);
		skb_queue_purge(&chan->ppp_rq);
		ppp_unregister_channel(ppp);
		kfree(ppp);
	}
#endif

}

static int tone_zone_init(void)
{
	int x;
	for (x=0;x<ZT_TONE_ZONE_MAX;x++)
		tone_zones[x] = NULL;
	return 0;
}

static int free_tone_zone(int num)
{
	struct zt_zone *z;
	if ((num < 0) || (num >= ZT_TONE_ZONE_MAX))
		return -EINVAL;
	write_lock(&zone_lock);
	z = tone_zones[num];
	tone_zones[num] = NULL;
	write_unlock(&zone_lock);
	kfree(z);
	return 0;
}

static int zt_register_tone_zone(int num, struct zt_zone *zone)
{
	int res=0;
	if ((num >= ZT_TONE_ZONE_MAX) || (num < 0))
		return -EINVAL;
	write_lock(&zone_lock);
	if (tone_zones[num]) {
		res = -EINVAL;
	} else {
		res = 0;
		tone_zones[num] = zone;
	}
	write_unlock(&zone_lock);
	if (!res)
		printk(KERN_INFO "Registered tone zone %d (%s)\n", num, zone->name);
	return res;
}

static int start_tone(struct zt_chan *chan, int tone)
{
	int res = -EINVAL;

	/* Stop the current tone, no matter what */
	chan->tonep = 0;
	chan->curtone = NULL;
	chan->pdialcount = 0;
	chan->txdialbuf[0] = '\0';
	chan->dialing = 0;

	if (tone == -1) {
		/* Just stop the current tone */
		res = 0;
	} else if ((tone >= 0 && tone <= ZT_TONE_MAX)) {
		if (chan->curzone) {
			/* Have a tone zone */
			if (chan->curzone->tones[tone]) {
				chan->curtone = chan->curzone->tones[tone];
				res = 0;
			} else	/* Indicate that zone is loaded but no such tone exists */
				res = -ENOSYS;
		} else	/* Note that no tone zone exists at the moment */
			res = -ENODATA;
	} else if (tone >= ZT_TONE_DTMF_BASE && tone <= ZT_TONE_DTMF_MAX) {
		/* ZT_SENDTONE should never be used on a channel configured for pulse dialing */
		chan->dialing = 1;
		res = 0;
		if (chan->digitmode == DIGIT_MODE_DTMF)
			chan->curtone = dtmf_tones_continuous + (tone - ZT_TONE_DTMF_BASE);
		else if (chan->digitmode == DIGIT_MODE_MFV1 && tone != ZT_TONE_DTMF_MAX) /* No 'D' */
			chan->curtone = mfv1_tones_continuous + (tone - ZT_TONE_DTMF_BASE);
		else {
			chan->dialing = 0;
			res = -EINVAL;
		}
	}

	if (chan->curtone)
		zt_init_tone_state(&chan->ts, chan->curtone);
	
	return res;
}

static int set_tone_zone(struct zt_chan *chan, int zone)
{
	int res=0;
	/* Assumes channel is already locked */
	if ((zone >= ZT_TONE_ZONE_MAX) || (zone < -1))
		return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
	/* Since this routine is called both from IRQ as well as from userspace,
	 * it is possible that we could be called during an IRQ while userspace
	 * has locked this.  However unlikely, this could possibly cause a
	 * deadlock. */
	if (! read_trylock(&zone_lock))
		return -EWOULDBLOCK;
#else
	/* But there are no trylock macros for kernel versions before 2.6.11,
	 * so we do the unsafe thing anyway.  Such is the problem for dealing
	 * with old, buggy kernels. */
	read_lock(&zone_lock);
#endif

	if (zone == -1) {
		zone = default_zone;
	}
	if (tone_zones[zone]) {
		chan->curzone = tone_zones[zone];
		chan->tonezone = zone;
		memcpy(chan->ringcadence, chan->curzone->ringcadence, sizeof(chan->ringcadence));
	} else {
		res = -ENODATA;
	}

	read_unlock(&zone_lock);
	return res;
}

static void zt_set_law(struct zt_chan *chan, int law)
{
	if (!law) {
		if (chan->deflaw)
			law = chan->deflaw;
		else
			if (chan->span) law = chan->span->deflaw;
			else law = ZT_LAW_MULAW;
	}
	if (law == ZT_LAW_ALAW) {
		chan->xlaw = __zt_alaw;
#ifdef CONFIG_CALC_XLAW
		chan->lineartoxlaw = __zt_lineartoalaw;
#else
		chan->lin2x = __zt_lin2a;
#endif
	} else {
		chan->xlaw = __zt_mulaw;
#ifdef CONFIG_CALC_XLAW
		chan->lineartoxlaw = __zt_lineartoulaw;
#else
		chan->lin2x = __zt_lin2mu;
#endif
	}
}

#ifdef CONFIG_DEVFS_FS
static devfs_handle_t register_devfs_channel(struct zt_chan *chan, devfs_handle_t dir)
{
	char path[100];
	char link[100];
	char buf[50];
	char tmp[100];
	int link_offset = 0;
	int tmp_offset = 0;
	int path_offset = 0;
	int err = 0;
	devfs_handle_t chan_dev;
	umode_t mode = S_IFCHR|S_IRUGO|S_IWUGO;
	unsigned int flags = DEVFS_FL_AUTO_OWNER;

	sprintf(path, "%d", chan->chanpos);
	chan_dev = devfs_register(dir, path, flags, ZT_MAJOR, chan->channo, mode, &zt_fops, NULL);
	if (!chan_dev) {
		printk("zaptel: Something really bad happened.  Unable to register devfs entry\n");
		return NULL;
	}

	/* Set up the path of the destination of the link */
	link_offset = devfs_generate_path(chan_dev, link, sizeof(link) - 1);
	/* Now we need to strip off the leading "zap/".  If we don't, then we build a broken symlink */
	path_offset = devfs_generate_path(zaptel_devfs_dir, path, sizeof(path) - 1); /* We'll just "borrow" path for a second */
	path_offset = strlen(path+path_offset);
	link_offset += path_offset; /* Taking out the "zap" */
	link_offset++; /* Add one more place for the '/'.  The path generated does not contain the '/' we need to strip */
	
	/* Set up the path of the file/link itself */
	tmp_offset = devfs_generate_path(zaptel_devfs_dir, tmp, sizeof(tmp) - 1);
	sprintf(buf, "/%d", chan->channo);
	strncpy(path, tmp+tmp_offset, sizeof(path) - 1);
	strncat(path, buf, sizeof(path) - 1);

	err = devfs_mk_symlink(NULL, path, DEVFS_FL_DEFAULT, link+link_offset, &chan->fhandle_symlink, NULL);
	if (err != 0) {
		printk("Problem with making devfs symlink: %d\n", err);
	}

	return chan_dev;
}
#endif /* CONFIG_DEVFS_FS */

static int zt_chan_reg(struct zt_chan *chan)
{
	int x;
	int res=0;
	unsigned long flags;
	
	write_lock_irqsave(&chan_lock, flags);
	for (x=1;x<ZT_MAX_CHANNELS;x++) {
		if (!chans[x]) {
			spin_lock_init(&chan->lock);
			chans[x] = chan;
			if (maxchans < x + 1)
				maxchans = x + 1;
			chan->channo = x;
			if (!chan->master)
				chan->master = chan;
			if (!chan->readchunk)
				chan->readchunk = chan->sreadchunk;
			if (!chan->writechunk)
				chan->writechunk = chan->swritechunk;
			zt_set_law(chan, 0);
			close_channel(chan); 
			/* set this AFTER running close_channel() so that
				HDLC channels wont cause hangage */
			chan->flags |= ZT_FLAG_REGISTERED;
			res = 0;
			break;
		}
	}
	write_unlock_irqrestore(&chan_lock, flags);	
	if (x >= ZT_MAX_CHANNELS)
		printk(KERN_ERR "No more channels available\n");
	return res;
}

char *zt_lboname(int x)
{
	if ((x < 0) || ( x > 7))
		return "Unknown";
	return zt_txlevelnames[x];
}

#if defined(CONFIG_ZAPATA_NET) || defined(CONFIG_ZAPATA_PPP)
#endif

#ifdef CONFIG_ZAPATA_NET
#ifdef NEW_HDLC_INTERFACE
static int zt_net_open(struct net_device *dev)
{
#ifdef LINUX26
	int res = hdlc_open(dev);
	struct zt_chan *ms = dev_to_ztchan(dev);
#else
	hdlc_device *hdlc = dev_to_hdlc(dev);
	struct zt_chan *ms = hdlc_to_ztchan(hdlc);
	int res = hdlc_open(hdlc);
#endif	
	                                                                                                                             
/*	if (!dev->hard_start_xmit) return res; is this really necessary? --byg */
	if (res) /* this is necessary to avoid kernel panic when UNSPEC link encap, proven --byg */
		return res;
#else
static int zt_net_open(hdlc_device *hdlc)
{
	struct zt_chan *ms = hdlc_to_ztchan(hdlc);
	int res;
#endif
	if (!ms) {
		printk("zt_net_open: nothing??\n");
		return -EINVAL;
	}
	if (ms->flags & ZT_FLAG_OPEN) {
		printk("%s is already open!\n", ms->name);
		return -EBUSY;
	}
	if (!(ms->flags & ZT_FLAG_NETDEV)) {
		printk("%s is not a net device!\n", ms->name);
		return -EINVAL;
	}
	ms->txbufpolicy = ZT_POLICY_IMMEDIATE;
	ms->rxbufpolicy = ZT_POLICY_IMMEDIATE;

	res = zt_reallocbufs(ms, ZT_DEFAULT_MTU_MRU, ZT_DEFAULT_NUM_BUFS);
	if (res) 
		return res;

	fasthdlc_init(&ms->rxhdlc);
	fasthdlc_init(&ms->txhdlc);
	ms->infcs = PPP_INITFCS;

	netif_start_queue(ztchan_to_dev(ms));

#ifndef LINUX26
	MOD_INC_USE_COUNT;
#endif	
#ifdef CONFIG_ZAPATA_DEBUG
	printk("ZAPNET: Opened channel %d name %s\n", ms->channo, ms->name);
#endif
	return 0;
}

#ifdef LINUX26
static int zt_register_hdlc_device(struct net_device *dev, const char *dev_name)
{
	int result;

	if (dev_name && *dev_name) {
		if ((result = dev_alloc_name(dev, dev_name)) < 0)
			return result;
	}
	result = register_netdev(dev);
	if (result != 0)
		return -EIO;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,14)
	if (netif_carrier_ok(dev))
		netif_carrier_off(dev); /* no carrier until DCD goes up */
#endif
	return 0;
}
#endif

#ifdef NEW_HDLC_INTERFACE
static int zt_net_stop(struct net_device *dev)
{
#ifdef LINUX26
    hdlc_device *h = dev_to_hdlc(dev);
    struct zt_hdlc *hdlc = h->priv;
#else
    hdlc_device *hdlc = dev_to_hdlc(dev);
#endif

#else
static void zt_net_close(hdlc_device *hdlc)
{
#endif
	struct zt_chan *ms = hdlc_to_ztchan(hdlc);
	if (!ms) {
#ifdef NEW_HDLC_INTERFACE
		printk("zt_net_stop: nothing??\n");
		return 0;
#else
		printk("zt_net_close: nothing??\n");
		return;
#endif
	}
	if (!(ms->flags & ZT_FLAG_NETDEV)) {
#ifdef NEW_HDLC_INTERFACE
		printk("zt_net_stop: %s is not a net device!\n", ms->name);
		return 0;
#else
		printk("zt_net_close: %s is not a net device!\n", ms->name);
		return;
#endif
	}
	/* Not much to do here.  Just deallocate the buffers */
        netif_stop_queue(ztchan_to_dev(ms));
	zt_reallocbufs(ms, 0, 0);
#ifdef LINUX26
	hdlc_close(dev);
#else
#ifndef CONFIG_OLD_HDLC_API
	hdlc_close(hdlc);
#endif
#endif	
#ifndef LINUX26
	MOD_DEC_USE_COUNT;
#endif	
#ifdef NEW_HDLC_INTERFACE
	return 0;
#else
	return;
#endif
}

#ifdef NEW_HDLC_INTERFACE
/* kernel 2.4.20+ has introduced attach function, dunno what to do,
 just copy sources from dscc4 to be sure and ready for further mastering,
 NOOP right now (i.e. really a stub)  --byg */
#ifdef LINUX26
static int zt_net_attach(struct net_device *dev, unsigned short encoding,
        unsigned short parity)
#else		
static int zt_net_attach(hdlc_device *hdlc, unsigned short encoding,
        unsigned short parity)
#endif
{
/*        struct net_device *dev = hdlc_to_dev(hdlc);
        struct dscc4_dev_priv *dpriv = dscc4_priv(dev);

        if (encoding != ENCODING_NRZ &&
            encoding != ENCODING_NRZI &&
            encoding != ENCODING_FM_MARK &&
            encoding != ENCODING_FM_SPACE &&
            encoding != ENCODING_MANCHESTER)
                return -EINVAL;

        if (parity != PARITY_NONE &&
            parity != PARITY_CRC16_PR0_CCITT &&
            parity != PARITY_CRC16_PR1_CCITT &&
            parity != PARITY_CRC32_PR0_CCITT &&
            parity != PARITY_CRC32_PR1_CCITT)
                return -EINVAL;

        dpriv->encoding = encoding;
        dpriv->parity = parity;*/
        return 0;
}
#endif
																								 
static struct zt_hdlc *zt_hdlc_alloc(void)
{
	struct zt_hdlc *tmp;
	tmp = kmalloc(sizeof(struct zt_hdlc), GFP_KERNEL);
	if (tmp) {
		memset(tmp, 0, sizeof(struct zt_hdlc));
	}
	return tmp;
}

#ifdef NEW_HDLC_INTERFACE
static int zt_xmit(struct sk_buff *skb, struct net_device *dev)
{
	/* FIXME: this construction seems to be not very optimal for me but I could find nothing better at the moment (Friday, 10PM :( )  --byg */
/*	struct zt_chan *ss = hdlc_to_ztchan(list_entry(dev, struct zt_hdlc, netdev.netdev));*/
#ifdef LINUX26
	struct zt_chan *ss = dev_to_ztchan(dev);
	struct net_device_stats *stats = hdlc_stats(dev);
#else
	struct zt_chan *ss = (list_entry(dev, struct zt_hdlc, netdev.netdev)->chan);
	struct net_device_stats *stats = &ss->hdlcnetdev->netdev.stats;
#endif	

#else
static int zt_xmit(hdlc_device *hdlc, struct sk_buff *skb)
{
	struct zt_chan *ss = hdlc_to_ztchan(hdlc);
	struct net_device *dev = &ss->hdlcnetdev->netdev.netdev;
	struct net_device_stats *stats = &ss->hdlcnetdev->netdev.stats;
#endif
	int retval = 1;
	int x,oldbuf;
	unsigned int fcs;
	unsigned char *data;
	unsigned long flags;
	/* See if we have any buffers */
	spin_lock_irqsave(&ss->lock, flags);
	if (skb->len > ss->blocksize - 2) {
		printk(KERN_ERR "zt_xmit(%s): skb is too large (%d > %d)\n", dev->name, skb->len, ss->blocksize -2);
		stats->tx_dropped++;
		retval = 0;
	} else if (ss->inwritebuf >= 0) {
		/* We have a place to put this packet */
		/* XXX We should keep the SKB and avoid the memcpy XXX */
		data = ss->writebuf[ss->inwritebuf];
		memcpy(data, skb->data, skb->len);
		ss->writen[ss->inwritebuf] = skb->len;
		ss->writeidx[ss->inwritebuf] = 0;
		/* Calculate the FCS */
		fcs = PPP_INITFCS;
		for (x=0;x<skb->len;x++)
			fcs = PPP_FCS(fcs, data[x]);
		/* Invert it */
		fcs ^= 0xffff;
		/* Send it out LSB first */
		data[ss->writen[ss->inwritebuf]++] = (fcs & 0xff);
		data[ss->writen[ss->inwritebuf]++] = (fcs >> 8) & 0xff;
		/* Advance to next window */
		oldbuf = ss->inwritebuf;
		ss->inwritebuf = (ss->inwritebuf + 1) % ss->numbufs;

		if (ss->inwritebuf == ss->outwritebuf) {
			/* Whoops, no more space.  */
		    ss->inwritebuf = -1;

		    netif_stop_queue(ztchan_to_dev(ss));
		}
		if (ss->outwritebuf < 0) {
			/* Let the interrupt handler know there's
			   some space for us */
			ss->outwritebuf = oldbuf;
		}
		dev->trans_start = jiffies;
		stats->tx_packets++;
		stats->tx_bytes += ss->writen[oldbuf];
#ifdef CONFIG_ZAPATA_DEBUG
		printk("Buffered %d bytes to go out in buffer %d\n", ss->writen[oldbuf], oldbuf);
		for (x=0;x<ss->writen[oldbuf];x++)
		     printk("%02x ", ss->writebuf[oldbuf][x]);
		printk("\n");
#endif
		retval = 0;
		/* Free the SKB */
		dev_kfree_skb_any(skb);
	}
	spin_unlock_irqrestore(&ss->lock, flags);
	return retval;
}

#ifdef NEW_HDLC_INTERFACE
static int zt_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	return hdlc_ioctl(dev, ifr, cmd);
}
#else
static int zt_net_ioctl(hdlc_device *hdlc, struct ifreq *ifr, int cmd)
{
	return -EIO;
}
#endif

#endif

#ifdef CONFIG_ZAPATA_PPP

static int zt_ppp_xmit(struct ppp_channel *ppp, struct sk_buff *skb)
{

	/* 
	 * If we can't handle the packet right now, return 0.  If we
	 * we handle or drop it, return 1.  Always free if we return
	 * 1 and never if we return 0
         */
	struct zt_chan *ss = ppp->private;
	int x,oldbuf;
	unsigned int fcs;
	unsigned char *data;
	long flags;
	int retval = 0;

	/* See if we have any buffers */
	spin_lock_irqsave(&ss->lock, flags);
	if (!(ss->flags & ZT_FLAG_OPEN)) {
		printk("Can't transmit on closed channel\n");
		retval = 1;
	} else if (skb->len > ss->blocksize - 4) {
		printk(KERN_ERR "zt_ppp_xmit(%s): skb is too large (%d > %d)\n", ss->name, skb->len, ss->blocksize -2);
		retval = 1;
	} else if (ss->inwritebuf >= 0) {
		/* We have a place to put this packet */
		/* XXX We should keep the SKB and avoid the memcpy XXX */
		data = ss->writebuf[ss->inwritebuf];
		/* Start with header of two bytes */
		/* Add "ALL STATIONS" and "UNNUMBERED" */
		data[0] = 0xff;
		data[1] = 0x03;
		ss->writen[ss->inwritebuf] = 2;

		/* Copy real data and increment amount written */
		memcpy(data + 2, skb->data, skb->len);

		ss->writen[ss->inwritebuf] += skb->len;

		/* Re-set index back to zero */
		ss->writeidx[ss->inwritebuf] = 0;

		/* Calculate the FCS */
		fcs = PPP_INITFCS;
		for (x=0;x<skb->len + 2;x++)
			fcs = PPP_FCS(fcs, data[x]);
		/* Invert it */
		fcs ^= 0xffff;

		/* Point past the real data now */
		data += (skb->len + 2);

		/* Send FCS out LSB first */
		data[0] = (fcs & 0xff);
		data[1] = (fcs >> 8) & 0xff;

		/* Account for FCS length */
		ss->writen[ss->inwritebuf]+=2;

		/* Advance to next window */
		oldbuf = ss->inwritebuf;
		ss->inwritebuf = (ss->inwritebuf + 1) % ss->numbufs;

		if (ss->inwritebuf == ss->outwritebuf) {
			/* Whoops, no more space.  */
			ss->inwritebuf = -1;
		}
		if (ss->outwritebuf < 0) {
			/* Let the interrupt handler know there's
			   some space for us */
			ss->outwritebuf = oldbuf;
		}
#ifdef CONFIG_ZAPATA_DEBUG
		printk("Buffered %d bytes (skblen = %d) to go out in buffer %d\n", ss->writen[oldbuf], skb->len, oldbuf);
		for (x=0;x<ss->writen[oldbuf];x++)
		     printk("%02x ", ss->writebuf[oldbuf][x]);
		printk("\n");
#endif
		retval = 1;
	}
	spin_unlock_irqrestore(&ss->lock, flags);
	if (retval) {
		/* Get rid of the SKB if we're returning non-zero */
		/* N.B. this is called in process or BH context so
		   dev_kfree_skb is OK. */
		dev_kfree_skb(skb);
	}
	return retval;
}

static int zt_ppp_ioctl(struct ppp_channel *ppp, unsigned int cmd, unsigned long flags)
{
	return -EIO;
}

static struct ppp_channel_ops ztppp_ops =
{
	start_xmit: zt_ppp_xmit,
	ioctl: zt_ppp_ioctl,
};

#endif

static void zt_chan_unreg(struct zt_chan *chan)
{
	int x;
	unsigned long flags;
#ifdef CONFIG_ZAPATA_NET
	if (chan->flags & ZT_FLAG_NETDEV) {
#ifdef LINUX26
		unregister_hdlc_device(chan->hdlcnetdev->netdev);
		free_netdev(chan->hdlcnetdev->netdev);
#else
		unregister_hdlc_device(&chan->hdlcnetdev->netdev);
#endif
		kfree(chan->hdlcnetdev);
		chan->hdlcnetdev = NULL;
	}
#endif
	write_lock_irqsave(&chan_lock, flags);
	if (chan->flags & ZT_FLAG_REGISTERED) {
		chans[chan->channo] = NULL;
		chan->flags &= ~ZT_FLAG_REGISTERED;
	}
#ifdef CONFIG_ZAPATA_PPP
	if (chan->ppp) {
		printk("HUH???  PPP still attached??\n");
	}
#endif
	maxchans = 0;
	for (x=1;x<ZT_MAX_CHANNELS;x++) 
		if (chans[x]) {
			maxchans = x + 1;
			/* Remove anyone pointing to us as master
			   and make them their own thing */
			if (chans[x]->master == chan) {
				chans[x]->master = chans[x];
			}
			if ((chans[x]->confna == chan->channo) &&
				((chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORTX ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORBOTH ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_RX_PREECHO ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_TX_PREECHO ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORBOTH_PREECHO ||
				(chans[x]->confmode & ZT_CONF_MODE_MASK) == ZT_CONF_DIGITALMON)) {
				/* Take them out of conference with us */
				/* release conference resource if any */
				if (chans[x]->confna) {
					zt_check_conf(chans[x]->confna);
					if (chans[x]->span && chans[x]->span->dacs)
						chans[x]->span->dacs(chans[x], NULL);
				}
				chans[x]->confna = 0;
				chans[x]->_confn = 0;
				chans[x]->confmode = 0;
			}
		}
	chan->channo = -1;
	write_unlock_irqrestore(&chan_lock, flags);
}

static ssize_t zt_chan_read(struct file *file, char *usrbuf, size_t count, int unit)
{
	struct zt_chan *chan = chans[unit];
	int amnt;
	int res, rv;
	int oldbuf,x;
	unsigned long flags;
	/* Make sure count never exceeds 65k, and make sure it's unsigned */
	count &= 0xffff;
	if (!chan) 
		return -EINVAL;
	if (count < 1)
		return -EINVAL;
	for(;;) {
		spin_lock_irqsave(&chan->lock, flags);
		if (chan->eventinidx != chan->eventoutidx) {
			spin_unlock_irqrestore(&chan->lock, flags);
			return -ELAST /* - chan->eventbuf[chan->eventoutidx]*/;
		}
		res = chan->outreadbuf;
		if (chan->rxdisable)
			res = -1;
		spin_unlock_irqrestore(&chan->lock, flags);
		if (res >= 0) break;
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		rv = schluffen(&chan->readbufq);
		if (rv) return (rv);
	}
	amnt = count;
/* added */
#if 0
	if ((unit == 24) || (unit == 48) || (unit == 16) || (unit == 47)) { 
		int myamnt = amnt;
		int x;
		if (amnt > chan->readn[res])
			myamnt = chan->readn[res];
		printk("zt_chan_read(unit: %d, inwritebuf: %d, outwritebuf: %d amnt: %d\n", 
			unit, chan->inwritebuf, chan->outwritebuf, myamnt);
		printk("\t("); for (x = 0; x < myamnt; x++) printk((x ? " %02x" : "%02x"), (unsigned char)usrbuf[x]);
		printk(")\n");
	}
#endif
/* end addition */
	if (chan->flags & ZT_FLAG_LINEAR) {
		if (amnt > (chan->readn[res] << 1))
			amnt = chan->readn[res] << 1;
		if (amnt) {
			/* There seems to be a max stack size, so we have
			   to do this in smaller pieces */
			short lindata[128];
			int left = amnt >> 1; /* amnt is in bytes */
			int pos = 0;
			int pass;
			while(left) {
				pass = left;
				if (pass > 128)
					pass = 128;
				for (x=0;x<pass;x++)
					lindata[x] = ZT_XLAW(chan->readbuf[res][x + pos], chan);
				if (copy_to_user(usrbuf + (pos << 1), lindata, pass << 1))
					return -EFAULT;
				left -= pass;
				pos += pass;
			}
		}
	} else {
		if (amnt > chan->readn[res])
			amnt = chan->readn[res];
		if (amnt) {
			if (copy_to_user(usrbuf, chan->readbuf[res], amnt))
				return -EFAULT;
		}
	}
	spin_lock_irqsave(&chan->lock, flags);
	chan->readidx[res] = 0;
	chan->readn[res] = 0;
	oldbuf = res;
	chan->outreadbuf = (res + 1) % chan->numbufs;
	if (chan->outreadbuf == chan->inreadbuf) {
		/* Out of stuff */
		chan->outreadbuf = -1;
		if (chan->rxbufpolicy == ZT_POLICY_WHEN_FULL)
			chan->rxdisable = 1;
	}
	if (chan->inreadbuf < 0) {
		/* Notify interrupt handler that we have some space now */
		chan->inreadbuf = oldbuf;
	}
	spin_unlock_irqrestore(&chan->lock, flags);
	
	return amnt;
}

static ssize_t zt_chan_write(struct file *file, const char *usrbuf, size_t count, int unit)
{
	unsigned long flags;
	struct zt_chan *chan = chans[unit];
	int res, amnt, oldbuf, rv,x;
	/* Make sure count never exceeds 65k, and make sure it's unsigned */
	count &= 0xffff;
	if (!chan) 
		return -EINVAL;
	if (count < 1)
		return -EINVAL;
	for(;;) {
		spin_lock_irqsave(&chan->lock, flags);
		if ((chan->curtone || chan->pdialcount) && !(chan->flags & ZT_FLAG_PSEUDO)) {
			chan->curtone = NULL;
			chan->tonep = 0;
			chan->dialing = 0;
			chan->txdialbuf[0] = '\0';
			chan->pdialcount = 0;
		}
		if (chan->eventinidx != chan->eventoutidx) {
			spin_unlock_irqrestore(&chan->lock, flags);
			return -ELAST;
		}
		res = chan->inwritebuf;
		spin_unlock_irqrestore(&chan->lock, flags);
		if (res >= 0) 
			break;
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		/* Wait for something to be available */
		rv = schluffen(&chan->writebufq);
		if (rv)
			return rv;
	}

	amnt = count;
	if (chan->flags & ZT_FLAG_LINEAR) {
		if (amnt > (chan->blocksize << 1))
			amnt = chan->blocksize << 1;
	} else {
		if (amnt > chan->blocksize)
			amnt = chan->blocksize;
	}

#ifdef CONFIG_ZAPATA_DEBUG
	printk("zt_chan_write(unit: %d, res: %d, outwritebuf: %d amnt: %d\n",
		unit, chan->res, chan->outwritebuf, amnt);
#endif
#if 0
 	if ((unit == 24) || (unit == 48) || (unit == 16) || (unit == 47)) { 
 		int x;
 		printk("zt_chan_write/in(unit: %d, res: %d, outwritebuf: %d amnt: %d, txdisable: %d)\n",
 			unit, res, chan->outwritebuf, amnt, chan->txdisable);
 		printk("\t("); for (x = 0; x < amnt; x++) printk((x ? " %02x" : "%02x"), (unsigned char)usrbuf[x]);
 		printk(")\n");
 	}
#endif

	if (amnt) {
		if (chan->flags & ZT_FLAG_LINEAR) {
			/* There seems to be a max stack size, so we have
			   to do this in smaller pieces */
			short lindata[128];
			int left = amnt >> 1; /* amnt is in bytes */
			int pos = 0;
			int pass;
			while(left) {
				pass = left;
				if (pass > 128)
					pass = 128;
				if (copy_from_user(lindata, usrbuf + (pos << 1), pass << 1))
					return -EFAULT;
				left -= pass;
				for (x=0;x<pass;x++)
					chan->writebuf[res][x + pos] = ZT_LIN2X(lindata[x], chan);
				pos += pass;
			}
			chan->writen[res] = amnt >> 1;
		} else {
			if (copy_from_user(chan->writebuf[res], usrbuf, amnt))
				return -EFAULT;
			chan->writen[res] = amnt;
		}
		chan->writeidx[res] = 0;
		if (chan->flags & ZT_FLAG_FCS)
			calc_fcs(chan, res);
		oldbuf = res;
		spin_lock_irqsave(&chan->lock, flags);
		chan->inwritebuf = (res + 1) % chan->numbufs;
		if (chan->inwritebuf == chan->outwritebuf) {
			/* Don't stomp on the transmitter, just wait for them to 
			   wake us up */
			chan->inwritebuf = -1;
			/* Make sure the transmitter is transmitting in case of POLICY_WHEN_FULL */
			chan->txdisable = 0;
		}
		if (chan->outwritebuf < 0) {
			/* Okay, the interrupt handler has been waiting for us.  Give them a buffer */
			chan->outwritebuf = oldbuf;
		}
		spin_unlock_irqrestore(&chan->lock, flags);

		if (chan->flags & ZT_FLAG_NOSTDTXRX && chan->span->hdlc_hard_xmit)
			chan->span->hdlc_hard_xmit(chan);
	}
	return amnt;
}

static int zt_ctl_open(struct inode *inode, struct file *file)
{
	/* Nothing to do, really */
#ifndef LINUX26
	MOD_INC_USE_COUNT;
#endif
	return 0;
}

static int zt_chan_open(struct inode *inode, struct file *file)
{
	/* Nothing to do here for now either */
#ifndef LINUX26
	MOD_INC_USE_COUNT;
#endif
	return 0;
}

static int zt_ctl_release(struct inode *inode, struct file *file)
{
	/* Nothing to do */
#ifndef LINUX26
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

static int zt_chan_release(struct inode *inode, struct file *file)
{
	/* Nothing to do for now */
#ifndef LINUX26
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

static void set_txtone(struct zt_chan *ss,int fac, int init_v2, int init_v3)
{
	if (fac == 0)
	{
		ss->v2_1 = 0;
		ss->v3_1 = 0;
		return;
	}
	ss->txtone = fac;
	ss->v1_1 = 0;
	ss->v2_1 = init_v2;
	ss->v3_1 = init_v3;
	return;
}

static void zt_rbs_sethook(struct zt_chan *chan, int txsig, int txstate, int timeout)
{
static int outs[NUM_SIGS][5] = {
/* We set the idle case of the ZT_SIG_NONE to this pattern to make idle E1 CAS
channels happy. Should not matter with T1, since on an un-configured channel, 
who cares what the sig bits are as long as they are stable */
	{ ZT_SIG_NONE, 		ZT_ABIT | ZT_CBIT | ZT_DBIT, 0, 0, 0 },  /* no signalling */
	{ ZT_SIG_EM, 		0, ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT,
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 0 },  /* E and M */
	{ ZT_SIG_FXSLS, 	ZT_BBIT | ZT_DBIT, 
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT,
			ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 0 }, /* FXS Loopstart */
	{ ZT_SIG_FXSGS, 	ZT_BBIT | ZT_DBIT, 
#ifdef CONFIG_CAC_GROUNDSTART
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 0, 0 }, /* FXS Groundstart (CAC-style) */
#else
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, ZT_ABIT | ZT_CBIT, 0 }, /* FXS Groundstart (normal) */
#endif
	{ ZT_SIG_FXSKS,		ZT_BBIT | ZT_DBIT, 
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT,
			ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 0 }, /* FXS Kewlstart */
	{ ZT_SIG_FXOLS,		ZT_BBIT | ZT_DBIT, ZT_BBIT | ZT_DBIT, 0, 0 }, /* FXO Loopstart */
	{ ZT_SIG_FXOGS,		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT,
		 ZT_BBIT | ZT_DBIT, 0, 0 }, /* FXO Groundstart */
	{ ZT_SIG_FXOKS,		ZT_BBIT | ZT_DBIT, ZT_BBIT | ZT_DBIT, 0, 
		ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT }, /* FXO Kewlstart */
	{ ZT_SIG_SF,	ZT_BBIT | ZT_CBIT | ZT_DBIT, 
			ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 
			ZT_ABIT | ZT_BBIT | ZT_CBIT | ZT_DBIT, 
			ZT_BBIT | ZT_CBIT | ZT_DBIT },  /* no signalling */
	{ ZT_SIG_EM_E1, 	ZT_DBIT, ZT_ABIT | ZT_BBIT | ZT_DBIT,
		ZT_ABIT | ZT_BBIT | ZT_DBIT, ZT_DBIT },  /* E and M  E1 */
	} ;
	int x;

	/* if no span, return doing nothing */
	if (!chan->span) return;
	if (!chan->span->flags & ZT_FLAG_RBS) {
		printk("zt_rbs: Tried to set RBS hook state on non-RBS channel %s\n", chan->name);
		return;
	}
	if ((txsig > 3) || (txsig < 0)) {
		printk("zt_rbs: Tried to set RBS hook state %d (> 3) on  channel %s\n", txsig, chan->name);
		return;
	}
	if (!chan->span->rbsbits && !chan->span->hooksig) {
		printk("zt_rbs: Tried to set RBS hook state %d on channel %s while span %s lacks rbsbits or hooksig function\n",
			txsig, chan->name, chan->span->name);
		return;
	}
	/* Don't do anything for RBS */
	if (chan->sig == ZT_SIG_DACS_RBS)
		return;
	chan->txstate = txstate;
	
	/* if tone signalling */
	if (chan->sig == ZT_SIG_SF)
	{
		chan->txhooksig = txsig;
		if (chan->txtone) /* if set to make tone for tx */
		{
			if ((txsig && !(chan->toneflags & ZT_REVERSE_TXTONE)) ||
			 ((!txsig) && (chan->toneflags & ZT_REVERSE_TXTONE))) 
			{
				set_txtone(chan,chan->txtone,chan->tx_v2,chan->tx_v3);
			}
			else
			{
				set_txtone(chan,0,0,0);
			}
		}
		chan->otimer = timeout * ZT_CHUNKSIZE;			/* Otimer is timer in samples */
		return;
	}
	if (chan->span->hooksig) {
		if (chan->txhooksig != txsig) {
			chan->txhooksig = txsig;
			chan->span->hooksig(chan, txsig);
		}
		chan->otimer = timeout * ZT_CHUNKSIZE;			/* Otimer is timer in samples */
		return;
	} else {
		for (x=0;x<NUM_SIGS;x++) {
			if (outs[x][0] == chan->sig) {
#ifdef CONFIG_ZAPATA_DEBUG
				printk("Setting bits to %d for channel %s state %d in %d signalling\n", outs[x][txsig + 1], chan->name, txsig, chan->sig);
#endif
				chan->txhooksig = txsig;
				chan->txsig = outs[x][txsig+1];
				chan->span->rbsbits(chan, chan->txsig);
				chan->otimer = timeout * ZT_CHUNKSIZE;	/* Otimer is timer in samples */
				return;
			}
		}
	}
	printk("zt_rbs: Don't know RBS signalling type %d on channel %s\n", chan->sig, chan->name);
}

static int zt_cas_setbits(struct zt_chan *chan, int bits)
{
	/* if no span, return as error */
	if (!chan->span) return -1;
	if (chan->span->rbsbits) {
		chan->txsig = bits;
		chan->span->rbsbits(chan, bits);
	} else {
		printk("Huh?  CAS setbits, but no RBS bits function\n");
	}
	return 0;
}

static int zt_hangup(struct zt_chan *chan)
{
	int x,res=0;

	/* Can't hangup pseudo channels */
	if (!chan->span)
		return 0;
	/* Can't hang up a clear channel */
	if (chan->flags & (ZT_FLAG_CLEAR | ZT_FLAG_NOSTDTXRX))
		return -EINVAL;

	chan->kewlonhook = 0;


	if ((chan->sig == ZT_SIG_FXSLS) || (chan->sig == ZT_SIG_FXSKS) ||
		(chan->sig == ZT_SIG_FXSGS)) chan->ringdebtimer = RING_DEBOUNCE_TIME;

	if (chan->span->flags & ZT_FLAG_RBS) {
		if (chan->sig == ZT_SIG_CAS) {
			zt_cas_setbits(chan, chan->idlebits);
		} else if ((chan->sig == ZT_SIG_FXOKS) && (chan->txstate != ZT_TXSTATE_ONHOOK)) {
			/* Do RBS signalling on the channel's behalf */
			zt_rbs_sethook(chan, ZT_TXSIG_KEWL, ZT_TXSTATE_KEWL, ZT_KEWLTIME);
		} else
			zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_ONHOOK, 0);
	} else {
		/* Let the driver hang up the line if it wants to  */
		if (chan->span->sethook) {
			if (chan->txhooksig != ZT_ONHOOK) {
				chan->txhooksig = ZT_ONHOOK;
				res = chan->span->sethook(chan, ZT_ONHOOK);
			} else
				res = 0;
		}
	}
	/* if not registered yet, just return here */
	if (!(chan->flags & ZT_FLAG_REGISTERED)) return res;
	/* Mark all buffers as empty */
	for (x = 0;x < chan->numbufs;x++) {
		chan->writen[x] = 
		chan->writeidx[x]=
		chan->readn[x]=
		chan->readidx[x] = 0;
	}	
	if (chan->readbuf[0]) {
		chan->inreadbuf = 0;
		chan->inwritebuf = 0;
	} else {
		chan->inreadbuf = -1;
		chan->inwritebuf = -1;
	}
	chan->outreadbuf = -1;
	chan->outwritebuf = -1;
	chan->dialing = 0;
	chan->afterdialingtimer = 0;
	chan->curtone = NULL;
	chan->pdialcount = 0;
	chan->cadencepos = 0;
	chan->txdialbuf[0] = 0;
	return res;
}

static int initialize_channel(struct zt_chan *chan)
{
	int res;
	unsigned long flags;
	void *rxgain=NULL;
	struct echo_can_state *ec=NULL;
	if ((res = zt_reallocbufs(chan, ZT_DEFAULT_BLOCKSIZE, ZT_DEFAULT_NUM_BUFS)))
		return res;

	spin_lock_irqsave(&chan->lock, flags);

	chan->rxbufpolicy = ZT_POLICY_IMMEDIATE;
	chan->txbufpolicy = ZT_POLICY_IMMEDIATE;

	/* Free up the echo canceller if there is one */
	ec = chan->ec;
	chan->ec = NULL;
	chan->echocancel = 0;
	chan->echostate = ECHO_STATE_IDLE;
	chan->echolastupdate = 0;
	chan->echotimer = 0;

	chan->txdisable = 0;
	chan->rxdisable = 0;

	chan->digitmode = DIGIT_MODE_DTMF;
	chan->dialing = 0;
	chan->afterdialingtimer = 0;

	chan->cadencepos = 0;
	chan->firstcadencepos = 0; /* By default loop back to first cadence position */

	/* HDLC & FCS stuff */
	fasthdlc_init(&chan->rxhdlc);
	fasthdlc_init(&chan->txhdlc);
	chan->infcs = PPP_INITFCS;
	
	/* Timings for RBS */
	chan->prewinktime = ZT_DEFAULT_PREWINKTIME;
	chan->preflashtime = ZT_DEFAULT_PREFLASHTIME;
	chan->winktime = ZT_DEFAULT_WINKTIME;
	chan->flashtime = ZT_DEFAULT_FLASHTIME;
	
	if (chan->sig & __ZT_SIG_FXO)
		chan->starttime = ZT_DEFAULT_RINGTIME;
	else
		chan->starttime = ZT_DEFAULT_STARTTIME;
	chan->rxwinktime = ZT_DEFAULT_RXWINKTIME;
	chan->rxflashtime = ZT_DEFAULT_RXFLASHTIME;
	chan->debouncetime = ZT_DEFAULT_DEBOUNCETIME;
	chan->pulsemaketime = ZT_DEFAULT_PULSEMAKETIME;
	chan->pulsebreaktime = ZT_DEFAULT_PULSEBREAKTIME;
	chan->pulseaftertime = ZT_DEFAULT_PULSEAFTERTIME;
	
	/* Initialize RBS timers */
	chan->itimerset = chan->itimer = chan->otimer = 0;
	chan->ringdebtimer = 0;		

	init_waitqueue_head(&chan->sel);
	init_waitqueue_head(&chan->readbufq);
	init_waitqueue_head(&chan->writebufq);
	init_waitqueue_head(&chan->eventbufq);
	init_waitqueue_head(&chan->txstateq);

	/* Reset conferences */
	reset_conf(chan);
	
	/* I/O Mask, etc */
	chan->iomask = 0;
	/* release conference resource if any */
	if (chan->confna) zt_check_conf(chan->confna);
	if ((chan->sig & __ZT_SIG_DACS) != __ZT_SIG_DACS) {
		chan->confna = 0;
		chan->confmode = 0;
		if (chan->span && chan->span->dacs)
			chan->span->dacs(chan, NULL);
	}
	chan->_confn = 0;
	memset(chan->conflast, 0, sizeof(chan->conflast));
	memset(chan->conflast1, 0, sizeof(chan->conflast1));
	memset(chan->conflast2, 0, sizeof(chan->conflast2));
	chan->confmute = 0;
	chan->gotgs = 0;
	chan->curtone = NULL;
	chan->tonep = 0;
	chan->pdialcount = 0;
	set_tone_zone(chan, -1);
	if (chan->gainalloc && chan->rxgain)
		rxgain = chan->rxgain;
	chan->rxgain = defgain;
	chan->txgain = defgain;
	chan->gainalloc = 0;
	chan->eventinidx = chan->eventoutidx = 0;
	zt_set_law(chan,0);
	zt_hangup(chan);

	/* Make sure that the audio flag is cleared on a clear channel */
	if ((chan->sig & ZT_SIG_CLEAR) || (chan->sig & ZT_SIG_HARDHDLC))
		chan->flags &= ~ZT_FLAG_AUDIO;

	if ((chan->sig == ZT_SIG_CLEAR) || (chan->sig == ZT_SIG_HARDHDLC))
		chan->flags &= ~(ZT_FLAG_PPP | ZT_FLAG_FCS | ZT_FLAG_HDLC);

	chan->flags &= ~ZT_FLAG_LINEAR;
	if (chan->curzone) {
		/* Take cadence from tone zone */
		memcpy(chan->ringcadence, chan->curzone->ringcadence, sizeof(chan->ringcadence));
	} else {
		/* Do a default */
		memset(chan->ringcadence, 0, sizeof(chan->ringcadence));
		chan->ringcadence[0] = chan->starttime;
		chan->ringcadence[1] = ZT_RINGOFFTIME;
	}

	spin_unlock_irqrestore(&chan->lock, flags);

	if (chan->span && chan->span->echocan)
		chan->span->echocan(chan, 0);

	if (rxgain)
		kfree(rxgain);
	if (ec)
		echo_can_free(ec);
	return 0;
}

static int zt_timing_open(struct inode *inode, struct file *file)
{
	struct zt_timer *t;
	unsigned long flags;
	t = kmalloc(sizeof(struct zt_timer), GFP_KERNEL);
	if (!t)
		return -ENOMEM;
	/* Allocate a new timer */
	memset(t, 0, sizeof(struct zt_timer));
	init_waitqueue_head(&t->sel);
	file->private_data = t;
#ifndef LINUX26
	MOD_INC_USE_COUNT;
#endif
	spin_lock_irqsave(&zaptimerlock, flags);
	t->next = zaptimers;
	zaptimers = t;
	spin_unlock_irqrestore(&zaptimerlock, flags);
	return 0;
}

static int zt_timer_release(struct inode *inode, struct file *file)
{
	struct zt_timer *t, *cur, *prev;
	unsigned long flags;
	t = file->private_data;
	if (t) {
		spin_lock_irqsave(&zaptimerlock, flags);
		prev = NULL;
		cur = zaptimers;
		while(cur) {
			if (t == cur)
				break;
			prev = cur;
			cur = cur->next;
		}
		if (cur) {
			if (prev)
				prev->next = cur->next;
			else
				zaptimers = cur->next;
		}
		spin_unlock_irqrestore(&zaptimerlock, flags);
		if (!cur) {
			printk("Zap Timer: Not on list??\n");
			return 0;
		}
		kfree(t);
#ifndef LINUX26
		MOD_DEC_USE_COUNT;
#endif		
	}
	return 0;
}

static int zt_specchan_open(struct inode *inode, struct file *file, int unit, int inc)
{
	int res = 0;

	if (chans[unit] && chans[unit]->sig) {
		/* Make sure we're not already open, a net device, or a slave device */
		if (chans[unit]->flags & ZT_FLAG_OPEN) 
			res = -EBUSY;
		else if (chans[unit]->flags & ZT_FLAG_NETDEV)
			res = -EBUSY;
		else if (chans[unit]->master != chans[unit])
			res = -EBUSY;
		else if ((chans[unit]->sig & __ZT_SIG_DACS) == __ZT_SIG_DACS)
			res = -EBUSY;
		else {
			unsigned long flags;
			/* Assume everything is going to be okay */
			res = initialize_channel(chans[unit]);
			spin_lock_irqsave(&chans[unit]->lock, flags);
			if (chans[unit]->flags & ZT_FLAG_PSEUDO) 
				chans[unit]->flags |= ZT_FLAG_AUDIO;
			if (chans[unit]->span && chans[unit]->span->open)
				res = chans[unit]->span->open(chans[unit]);
			if (!res) {
				chans[unit]->file = file;
#ifndef LINUX26
				if (inc)
					MOD_INC_USE_COUNT;
#endif					
				chans[unit]->flags |= ZT_FLAG_OPEN;
				spin_unlock_irqrestore(&chans[unit]->lock, flags);
			} else {
				spin_unlock_irqrestore(&chans[unit]->lock, flags);
				close_channel(chans[unit]);
			}
		}
	} else
		res = -ENXIO;
	return res;
}

static int zt_specchan_release(struct inode *node, struct file *file, int unit)
{
	int res=0;
	if (chans[unit]) {
		unsigned long flags;
		spin_lock_irqsave(&chans[unit]->lock, flags);
		chans[unit]->flags &= ~ZT_FLAG_OPEN;
		spin_unlock_irqrestore(&chans[unit]->lock, flags);
		chans[unit]->file = NULL;
		close_channel(chans[unit]);
		if (chans[unit]->span && chans[unit]->span->close)
			res = chans[unit]->span->close(chans[unit]);
	} else
		res = -ENXIO;
#ifndef LINUX26
	MOD_DEC_USE_COUNT;
#endif
	return res;
}

static struct zt_chan *zt_alloc_pseudo(void)
{
	struct zt_chan *pseudo;
	unsigned long flags;
	/* Don't allow /dev/zap/pseudo to open if there are no spans */
	if (maxspans < 1)
		return NULL;
	pseudo = kmalloc(sizeof(struct zt_chan), GFP_KERNEL);
	if (!pseudo)
		return NULL;
	memset(pseudo, 0, sizeof(struct zt_chan));
	pseudo->sig = ZT_SIG_CLEAR;
	pseudo->sigcap = ZT_SIG_CLEAR;
	pseudo->flags = ZT_FLAG_PSEUDO | ZT_FLAG_AUDIO;
	spin_lock_irqsave(&bigzaplock, flags);
	if (zt_chan_reg(pseudo)) {
		kfree(pseudo);
		pseudo = NULL;
	} else
		sprintf(pseudo->name, "Pseudo/%d", pseudo->channo);
	spin_unlock_irqrestore(&bigzaplock, flags);
	return pseudo;	
}

static void zt_free_pseudo(struct zt_chan *pseudo)
{
	unsigned long flags;
	if (pseudo) {
		spin_lock_irqsave(&bigzaplock, flags);
		zt_chan_unreg(pseudo);
		spin_unlock_irqrestore(&bigzaplock, flags);
		kfree(pseudo);
	}
}

static int zt_open(struct inode *inode, struct file *file)
{
	int unit = UNIT(file);
	int ret = -ENXIO;
	struct zt_chan *chan;
	/* Minor 0: Special "control" descriptor */
	if (!unit) 
		return zt_ctl_open(inode, file);
	if (unit == 250) {
		if (!zt_transcode_fops)
			request_module("zttranscode");
		if (zt_transcode_fops && zt_transcode_fops->open) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			if (zt_transcode_fops->owner) {
				__MOD_INC_USE_COUNT (zt_transcode_fops->owner);
#else
			if (try_module_get(zt_transcode_fops->owner)) {
#endif
				ret = zt_transcode_fops->open(inode, file);
				if (ret)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
					__MOD_DEC_USE_COUNT (zt_transcode_fops->owner);
#else
					module_put(zt_transcode_fops->owner);
#endif
			}
			return ret;
		}
		return -ENXIO;
	}
	if (unit == 253) {
		if (maxspans) {
			return zt_timing_open(inode, file);
		} else {
			return -ENXIO;
		}
	}
	if (unit == 254)
		return zt_chan_open(inode, file);
	if (unit == 255) {
		if (maxspans) {
			chan = zt_alloc_pseudo();
			if (chan) {
				file->private_data = chan;
				return zt_specchan_open(inode, file, chan->channo, 1);
			} else {
				return -ENXIO;
			}
		} else
			return -ENXIO;
	}
	return zt_specchan_open(inode, file, unit, 1);
}

#if 0
static int zt_open(struct inode *inode, struct file *file)
{
	int res;
	unsigned long flags;
	spin_lock_irqsave(&bigzaplock, flags);
	res = __zt_open(inode, file);
	spin_unlock_irqrestore(&bigzaplock, flags);
	return res;
}
#endif

static ssize_t zt_read(struct file *file, char *usrbuf, size_t count, loff_t *ppos)
{
	int unit = UNIT(file);
	struct zt_chan *chan;

	/* Can't read from control */
	if (!unit) {
		return -EINVAL;
	}
	
	if (unit == 253) 
		return -EINVAL;
	
	if (unit == 254) {
		chan = file->private_data;
		if (!chan)
			return -EINVAL;
		return zt_chan_read(file, usrbuf, count, chan->channo);
	}
	
	if (unit == 255) {
		chan = file->private_data;
		if (!chan) {
			printk("No pseudo channel structure to read?\n");
			return -EINVAL;
		}
		return zt_chan_read(file, usrbuf, count, chan->channo);
	}
	if (count < 0)
		return -EINVAL;

	return zt_chan_read(file, usrbuf, count, unit);
}

static ssize_t zt_write(struct file *file, const char *usrbuf, size_t count, loff_t *ppos)
{
	int unit = UNIT(file);
	struct zt_chan *chan;
	/* Can't read from control */
	if (!unit)
		return -EINVAL;
	if (count < 0)
		return -EINVAL;
	if (unit == 253)
		return -EINVAL;
	if (unit == 254) {
		chan = file->private_data;
		if (!chan)
			return -EINVAL;
		return zt_chan_write(file, usrbuf, count, chan->channo);
	}
	if (unit == 255) {
		chan = file->private_data;
		if (!chan) {
			printk("No pseudo channel structure to read?\n");
			return -EINVAL;
		}
		return zt_chan_write(file, usrbuf, count, chan->channo);
	}
	return zt_chan_write(file, usrbuf, count, unit);
	
}

/* No bigger than 32k for everything per tone zone */
#define MAX_SIZE 32768
/* No more than 64 subtones */
#define MAX_TONES 64

static int
ioctl_load_zone(unsigned long data)
{
	struct zt_tone *samples[MAX_TONES];
	short next[MAX_TONES];
	struct zt_tone_def_header th;
	void *slab, *ptr;
	long size;
	struct zt_zone *z;
	struct zt_tone_def td;
	struct zt_tone *t;
	int x;
	int space;
	int res;
	
	/* XXX Unnecessary XXX */
	memset(samples, 0, sizeof(samples));
	/* XXX Unnecessary XXX */
	memset(next, 0, sizeof(next));
	if (copy_from_user(&th, (struct zt_tone_def_header *)data, sizeof(th)))
		return -EFAULT;
	if ((th.count < 0) || (th.count > MAX_TONES)) {
		printk("Too many tones included\n");
		return -EINVAL;
	}
	space = size = sizeof(struct zt_zone) +
			th.count * sizeof(struct zt_tone);
	if ((size > MAX_SIZE) || (size < 0))
		return -E2BIG;
	ptr = slab = (char *)kmalloc(size, GFP_KERNEL);
	if (!slab)
		return -ENOMEM;
	/* Zero it out for simplicity */
	memset(slab, 0, size);
	/* Grab the zone */
	z = (struct zt_zone *)slab;
	strncpy(z->name, th.name, sizeof(z->name) - 1);
	for (x=0;x<ZT_MAX_CADENCE;x++)
		z->ringcadence[x] = th.ringcadence[x];
	data += sizeof(struct zt_tone_def_header);
	ptr += sizeof(struct zt_zone);
	space -= sizeof(struct zt_zone);
	for (x=0;x<th.count;x++) {
		if (space < sizeof(struct zt_tone)) {
			/* Check space for zt_tone struct */
			kfree(slab);
			printk("Insufficient tone zone space\n");
			return -EINVAL;
		}
		if (copy_from_user(&td, (struct zt_tone_def *)data, sizeof(struct zt_tone_def))) {
			kfree(slab);
			return -EFAULT;
		}
		/* Index the current sample */
		samples[x] = t = (struct zt_tone *)ptr;
		/* Remember which sample is next */
		next[x] = td.next;
		/* Make sure the "next" one is sane */
		if ((next[x] >= th.count) || (next[x] < 0)) {
			printk("Invalid 'next' pointer: %d\n", next[x]);
			kfree(slab);
			return -EINVAL;
		}
		if (td.tone >= ZT_TONE_MAX) {
			printk("Too many tones defined\n");
			/* Make sure it's sane */
			kfree(slab);
			return -EINVAL;
		}
		/* Update pointers to account for zt_tone header */
		space -= sizeof(struct zt_tone);
		ptr += sizeof(struct zt_tone);
		data += sizeof(struct zt_tone_def);
		/* Fill in tonedata, datalen, and tonesamples fields */
		t->tonesamples = td.samples;
		t->fac1 = td.fac1;
		t->init_v2_1 = td.init_v2_1;
		t->init_v3_1 = td.init_v3_1;
		t->fac2 = td.fac2;
		t->init_v2_2 = td.init_v2_2;
		t->init_v3_2 = td.init_v3_2;
		t->modulate = td.modulate;
		t->next = NULL;					/* XXX Unnecessary XXX */
		if (!z->tones[td.tone])
			z->tones[td.tone] = t;
	}
	for (x=0;x<th.count;x++) 
		/* Set "next" pointers */
		samples[x]->next = samples[next[x]];

	/* Actually register zone */
	res = zt_register_tone_zone(th.zone, z);
	if (res)
		kfree(slab);
	return res;
}

void zt_init_tone_state(struct zt_tone_state *ts, struct zt_tone *zt)
{
	ts->v1_1 = 0;
	ts->v2_1 = zt->init_v2_1;
	ts->v3_1 = zt->init_v3_1;
	ts->v1_2 = 0;
	ts->v2_2 = zt->init_v2_2;
	ts->v3_2 = zt->init_v3_2;
	ts->modulate = zt->modulate;
}

struct zt_tone *zt_dtmf_tone(char digit, int mf)
{
	struct zt_tone *z;

	if (!mf)
		z = dtmf_tones;
	else
		z = mfv1_tones;
	switch(digit) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return z + (int)(digit - '0');
	case '*':
		return z + 10;
	case '#':
		return z + 11;
	case 'A':
	case 'B':
	case 'C':
		return z + (digit + 12 - 'A');
	case 'D':
		if (!mf)
			return z + ( digit + 12 - 'A');
		return NULL;
	case 'a':
	case 'b':
	case 'c':
		return z + (digit + 12 - 'a');
	case 'd':
		if (!mf)
			return z + ( digit + 12 - 'a');
		return NULL;
	case 'W':
	case 'w':
		return &tone_pause;
	}
	return NULL;
}

static void __do_dtmf(struct zt_chan *chan)
{
	char c;
	/* Called with chan->lock held */
	while (strlen(chan->txdialbuf)) {
		c = chan->txdialbuf[0];
		/* Skooch */
		memmove(chan->txdialbuf, chan->txdialbuf + 1, sizeof(chan->txdialbuf) - 1);
		switch(c) {
		case 'T':
		case 't':
			chan->digitmode = DIGIT_MODE_DTMF;
			chan->tonep = 0;
			break;
		case 'M':
		case 'm':
			chan->digitmode = DIGIT_MODE_MFV1;
			chan->tonep = 0;
			break;
		case 'P':
		case 'p':
			chan->digitmode = DIGIT_MODE_PULSE;
			chan->tonep = 0;
			break;
		default:
			if (chan->digitmode == DIGIT_MODE_PULSE)
			{
				if ((c >= '0') && (c <= '9') && (chan->txhooksig == ZT_TXSIG_OFFHOOK))
				{
					chan->pdialcount = c - '0';
					/* a '0' is ten pulses */
					if (!chan->pdialcount) chan->pdialcount = 10;
					zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, 
						ZT_TXSTATE_PULSEBREAK, chan->pulsebreaktime);
					return;
				}
			} else {
		case 'w':
		case 'W':
				chan->curtone = zt_dtmf_tone(c, (chan->digitmode == DIGIT_MODE_MFV1)); 
				chan->tonep = 0;
				/* All done */
				if (chan->curtone) {
					zt_init_tone_state(&chan->ts, chan->curtone);
					return;
				}
			}
		}
	}
	/* Notify userspace process if there is nothing left */
	chan->dialing = 0;
	__qevent(chan, ZT_EVENT_DIALCOMPLETE);
}

static int zt_release(struct inode *inode, struct file *file)
{
	int unit = UNIT(file);
	int res;
	struct zt_chan *chan;

	if (!unit) 
		return zt_ctl_release(inode, file);
	if (unit == 253) {
		return zt_timer_release(inode, file);
	}
	if (unit == 250) {
		res = zt_transcode_fops->release(inode, file);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
		if (zt_transcode_fops->owner)
			__MOD_DEC_USE_COUNT (zt_transcode_fops->owner);
#else
		module_put(zt_transcode_fops->owner);
#endif
		return res;
	}
	if (unit == 254) {
		chan = file->private_data;
		if (!chan)
			return zt_chan_release(inode, file);
		else
			return zt_specchan_release(inode, file, chan->channo);
	}
	if (unit == 255) {
		chan = file->private_data;
		if (chan) {
			res = zt_specchan_release(inode, file, chan->channo);
			zt_free_pseudo(chan);
		} else {
			printk("Pseudo release and no private data??\n");
			res = 0;
		}
		return res;
	}
	return zt_specchan_release(inode, file, unit);
}

#if 0
static int zt_release(struct inode *inode, struct file *file)
{
	/* Lock the big zap lock when handling a release */
	unsigned long flags;
	int res;
	spin_lock_irqsave(&bigzaplock, flags);
	res = __zt_release(inode, file);
	spin_unlock_irqrestore(&bigzaplock, flags);
	return res;
}
#endif

void zt_alarm_notify(struct zt_span *span)
{
	int j;
	int x;

	span->alarms &= ~ZT_ALARM_LOOPBACK;
	/* Determine maint status */
	if (span->maintstat || span->mainttimer)
		span->alarms |= ZT_ALARM_LOOPBACK;
	/* DON'T CHANGE THIS AGAIN. THIS WAS DONE FOR A REASON.
 	   The expression (a != b) does *NOT* do the same thing
	   as ((!a) != (!b)) */
	/* if change in general state */
	if ((!span->alarms) != (!span->lastalarms)) {
		if (span->alarms)
			j = ZT_EVENT_ALARM;
		else
			j = ZT_EVENT_NOALARM;
		span->lastalarms = span->alarms;
		for (x=0;x < span->channels;x++)
			zt_qevent_lock(&span->chans[x], j);
		/* Switch to other master if current master in alarm */
		for (x=1; x<maxspans; x++) {
			if (spans[x] && !spans[x]->alarms && (spans[x]->flags & ZT_FLAG_RUNNING)) {
				if(master != spans[x])
					printk("Zaptel: Master changed to %s\n", spans[x]->name);
				master = spans[x];
				break;
			}
		}
	}
}

#define VALID_SPAN(j) do { \
	if ((j >= ZT_MAX_SPANS) || (j < 1)) \
		return -EINVAL; \
	if (!spans[j]) \
		return -ENXIO; \
} while(0)

#define CHECK_VALID_SPAN(j) do { \
	/* Start a given span */ \
	if (get_user(j, (int *)data)) \
		return -EFAULT; \
	VALID_SPAN(j); \
} while(0)

#define VALID_CHANNEL(j) do { \
	if ((j >= ZT_MAX_CHANNELS) || (j < 1)) \
		return -EINVAL; \
	if (!chans[j]) \
		return -ENXIO; \
} while(0)

static int zt_timer_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long data, struct zt_timer *timer)
{
	int j;
	unsigned long flags;
	switch(cmd) {
	case ZT_TIMERCONFIG:
		get_user(j, (int *)data);
		if (j < 0)
			j = 0;
		spin_lock_irqsave(&zaptimerlock, flags);
		timer->ms = timer->pos = j;
		spin_unlock_irqrestore(&zaptimerlock, flags);
		break;
	case ZT_TIMERACK:
		get_user(j, (int *)data);
		spin_lock_irqsave(&zaptimerlock, flags);
		if ((j < 1) || (j > timer->tripped))
			j = timer->tripped;
		timer->tripped -= j;
		spin_unlock_irqrestore(&zaptimerlock, flags);
		break;
	case ZT_GETEVENT:  /* Get event on queue */
		j = ZT_EVENT_NONE;
		spin_lock_irqsave(&zaptimerlock, flags);
		  /* set up for no event */
		if (timer->tripped)
			j = ZT_EVENT_TIMER_EXPIRED;
		if (timer->ping)
			j = ZT_EVENT_TIMER_PING;
		spin_unlock_irqrestore(&zaptimerlock, flags);
		put_user(j,(int *)data);
		break;
	case ZT_TIMERPING:
		spin_lock_irqsave(&zaptimerlock, flags);
		timer->ping = 1;
		wake_up_interruptible(&timer->sel);
		spin_unlock_irqrestore(&zaptimerlock, flags);
		break;
	case ZT_TIMERPONG:
		spin_lock_irqsave(&zaptimerlock, flags);
		timer->ping = 0;
		spin_unlock_irqrestore(&zaptimerlock, flags);
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

static int zt_common_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long data, int unit)
{
	union {
		struct zt_gains gain;
		struct zt_spaninfo span;
		struct zt_spaninfo_compat span_compat;
		struct zt_params param;
	} stack;
	struct zt_chan *chan;
	unsigned long flags;
	unsigned char *txgain, *rxgain;
	struct zt_chan *mychan;
	int i,j;
	int return_master = 0;

	switch(cmd) {
	case ZT_GET_PARAMS: /* get channel timing parameters */
		if (copy_from_user(&stack.param,(struct zt_params *)data,sizeof(stack.param)))
			return -EFAULT;

		/* check to see if the caller wants to receive our master channel number */
		if (stack.param.channo & ZT_GET_PARAMS_RETURN_MASTER) {
			return_master = 1;
			stack.param.channo &= ~ZT_GET_PARAMS_RETURN_MASTER;
		}

		/* Pick the right channo's */
		if (!stack.param.channo || unit) {
			stack.param.channo = unit;
		}
		/* Check validity of channel */
		VALID_CHANNEL(stack.param.channo);
		chan = chans[stack.param.channo];

		/* point to relevant structure */
		stack.param.sigtype = chan->sig;  /* get signalling type */
		/* return non-zero if rx not in idle state */
		if (chan->span) {
			j = zt_q_sig(chan); 
			if (j >= 0) { /* if returned with success */
				stack.param.rxisoffhook = ((chan->rxsig & (j >> 8)) != (j & 0xff));
			} else {
				stack.param.rxisoffhook = ((chan->rxhooksig != ZT_RXSIG_ONHOOK) &&
					(chan->rxhooksig != ZT_RXSIG_INITIAL));
			}
		} else if ((chan->txstate == ZT_TXSTATE_KEWL) || (chan->txstate == ZT_TXSTATE_AFTERKEWL))
			stack.param.rxisoffhook = 1;
		else
			stack.param.rxisoffhook = 0;
		if (chan->span && chan->span->rbsbits && !(chan->sig & ZT_SIG_CLEAR)) {
			stack.param.rxbits = chan->rxsig;
			stack.param.txbits = chan->txsig;
			stack.param.idlebits = chan->idlebits;
		} else {
			stack.param.rxbits = -1;
			stack.param.txbits = -1;
			stack.param.idlebits = 0;
		}
		if (chan->span && (chan->span->rbsbits || chan->span->hooksig) && 
			!(chan->sig & ZT_SIG_CLEAR)) {
			stack.param.rxhooksig = chan->rxhooksig;
			stack.param.txhooksig = chan->txhooksig;
		} else {
			stack.param.rxhooksig = -1;
			stack.param.txhooksig = -1;
		}
		stack.param.prewinktime = chan->prewinktime; 
		stack.param.preflashtime = chan->preflashtime;		
		stack.param.winktime = chan->winktime;
		stack.param.flashtime = chan->flashtime;
		stack.param.starttime = chan->starttime;
		stack.param.rxwinktime = chan->rxwinktime;
		stack.param.rxflashtime = chan->rxflashtime;
		stack.param.debouncetime = chan->debouncetime;
		stack.param.channo = chan->channo;

		/* if requested, put the master channel number in the top 16 bits of the result */
		if (return_master)
			stack.param.channo |= chan->master->channo << 16;

		stack.param.pulsemaketime = chan->pulsemaketime;
		stack.param.pulsebreaktime = chan->pulsebreaktime;
		stack.param.pulseaftertime = chan->pulseaftertime;
		if (chan->span) stack.param.spanno = chan->span->spanno;
			else stack.param.spanno = 0;
		strncpy(stack.param.name, chan->name, sizeof(stack.param.name) - 1);
		stack.param.chanpos = chan->chanpos;
		stack.param.sigcap = chan->sigcap;
		/* Return current law */
		if (chan->xlaw == __zt_alaw)
			stack.param.curlaw = ZT_LAW_ALAW;
		else
			stack.param.curlaw = ZT_LAW_MULAW;
		if (copy_to_user((struct zt_params *)data,&stack.param,sizeof(stack.param)))
			return -EFAULT;
		break;
	case ZT_SET_PARAMS: /* set channel timing stack.paramters */
		if (copy_from_user(&stack.param,(struct zt_params *)data,sizeof(stack.param)))
			return -EFAULT;
		/* Pick the right channo's */
		if (!stack.param.channo || unit) {
			stack.param.channo = unit;
		}
		/* Check validity of channel */
		VALID_CHANNEL(stack.param.channo);
		chan = chans[stack.param.channo];
		  /* point to relevant structure */
		/* NOTE: sigtype is *not* included in this */
		  /* get timing stack.paramters */
		chan->prewinktime = stack.param.prewinktime;
		chan->preflashtime = stack.param.preflashtime;
		chan->winktime = stack.param.winktime;
		chan->flashtime = stack.param.flashtime;
		chan->starttime = stack.param.starttime;
		/* Update ringtime if not using a tone zone */
		if (!chan->curzone)
			chan->ringcadence[0] = chan->starttime;
		chan->rxwinktime = stack.param.rxwinktime;
		chan->rxflashtime = stack.param.rxflashtime;
		chan->debouncetime = stack.param.debouncetime;
		chan->pulsemaketime = stack.param.pulsemaketime;
		chan->pulsebreaktime = stack.param.pulsebreaktime;
		chan->pulseaftertime = stack.param.pulseaftertime;
		break;
	case ZT_GETGAINS:  /* get gain stuff */
		if (copy_from_user(&stack.gain,(struct zt_gains *) data,sizeof(stack.gain)))
			return -EFAULT;
		i = stack.gain.chan;  /* get channel no */
		   /* if zero, use current channel no */
		if (!i) i = unit;
		  /* make sure channel number makes sense */
		if ((i < 0) || (i > ZT_MAX_CHANNELS) || !chans[i]) return(-EINVAL);
		
		if (!(chans[i]->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		stack.gain.chan = i; /* put the span # in here */
		for (j=0;j<256;j++)  {
			stack.gain.txgain[j] = chans[i]->txgain[j];
			stack.gain.rxgain[j] = chans[i]->rxgain[j];
		}
		if (copy_to_user((struct zt_gains *) data,&stack.gain,sizeof(stack.gain)))
			return -EFAULT;
		break;
	case ZT_SETGAINS:  /* set gain stuff */
		if (copy_from_user(&stack.gain,(struct zt_gains *) data,sizeof(stack.gain)))
			return -EFAULT;
		i = stack.gain.chan;  /* get channel no */
		   /* if zero, use current channel no */
		if (!i) i = unit;
		  /* make sure channel number makes sense */
		if ((i < 0) || (i > ZT_MAX_CHANNELS) || !chans[i]) return(-EINVAL);
		if (!(chans[i]->flags & ZT_FLAG_AUDIO)) return (-EINVAL);

		rxgain = kmalloc(512, GFP_KERNEL);
		if (!rxgain)
			return -ENOMEM;

		stack.gain.chan = i; /* put the span # in here */
		txgain = rxgain + 256;

		for (j=0;j<256;j++) {
			rxgain[j] = stack.gain.rxgain[j];
			txgain[j] = stack.gain.txgain[j];
		}

		if (!memcmp(rxgain, defgain, 256) && 
		    !memcmp(txgain, defgain, 256)) {
			if (rxgain)
				kfree(rxgain);
			spin_lock_irqsave(&chans[i]->lock, flags);
			if (chans[i]->gainalloc)
				kfree(chans[i]->rxgain);
			chans[i]->gainalloc = 0;
			chans[i]->rxgain = defgain;
			chans[i]->txgain = defgain;
			spin_unlock_irqrestore(&chans[i]->lock, flags);
		} else {
			/* This is a custom gain setting */
			spin_lock_irqsave(&chans[i]->lock, flags);
			if (chans[i]->gainalloc)
				kfree(chans[i]->rxgain);
			chans[i]->gainalloc = 1;
			chans[i]->rxgain = rxgain;
			chans[i]->txgain = txgain;
			spin_unlock_irqrestore(&chans[i]->lock, flags);
		}
		if (copy_to_user((struct zt_gains *) data,&stack.gain,sizeof(stack.gain)))
			return -EFAULT;
		break;
	case ZT_SPANSTAT_COMPAT:
	case ZT_SPANSTAT:
		if (copy_from_user(&stack.span,(struct zt_spaninfo *) data, cmd == ZT_SPANSTAT_COMPAT ? sizeof(stack.span_compat) : sizeof(stack.span)))
			return -EFAULT;
		i = stack.span.spanno; /* get specified span number */
		if ((i < 0) || (i >= maxspans)) return(-EINVAL);  /* if bad span no */
		if (i == 0) /* if to figure it out for this chan */
		   {
		   	if (!chans[unit])
				return -EINVAL;
			i = chans[unit]->span->spanno;
		   }
		if (!spans[i])
			return -EINVAL;
		stack.span.spanno = i; /* put the span # in here */
		stack.span.totalspans = 0;
		if (maxspans) stack.span.totalspans = maxspans - 1; /* put total number of spans here */
		strncpy(stack.span.desc, spans[i]->desc, sizeof(stack.span.desc) - 1);
		strncpy(stack.span.name, spans[i]->name, sizeof(stack.span.name) - 1);
		stack.span.alarms = spans[i]->alarms;		/* get alarm status */
		stack.span.bpvcount = spans[i]->bpvcount;	/* get BPV count */
		stack.span.rxlevel = spans[i]->rxlevel;	/* get rx level */
		stack.span.txlevel = spans[i]->txlevel;	/* get tx level */
		stack.span.crc4count = spans[i]->crc4count;	/* get CRC4 error count */
		stack.span.ebitcount = spans[i]->ebitcount;	/* get E-bit error count */
		stack.span.fascount = spans[i]->fascount;	/* get FAS error count */
		stack.span.irqmisses = spans[i]->irqmisses;	/* get IRQ miss count */
		stack.span.syncsrc = spans[i]->syncsrc;	/* get active sync source */
		stack.span.totalchans = spans[i]->channels;
		/* We have space in the stack for these 2, even if they aren't copied back to userspace (for _COMPAT) */
		stack.span.lbo = spans[i]->lbo;
		stack.span.lineconfig = spans[i]->lineconfig;
		stack.span.numchans = 0;
		for (j=0; j < spans[i]->channels; j++)
			if (spans[i]->chans[j].sig)
				stack.span.numchans++;
		if (copy_to_user((struct zt_spaninfo *) data,&stack.span, cmd == ZT_SPANSTAT_COMPAT ? sizeof(stack.span_compat) : sizeof(stack.span)))
			return -EFAULT;
		break;
	case ZT_CHANDIAG:
		get_user(j, (int *)data); /* get channel number from user */
		/* make sure its a valid channel number */
		if ((j < 1) || (j >= maxchans))
			return -EINVAL;
		/* if channel not mapped, not there */
		if (!chans[j]) 
			return -EINVAL;

		if (!(mychan = kmalloc(sizeof(*mychan), GFP_KERNEL)))
			return -ENOMEM;

		/* lock channel */
		spin_lock_irqsave(&chans[j]->lock, flags);
		/* make static copy of channel */
		memcpy(mychan, chans[j], sizeof(*mychan));
		/* release it. */
		spin_unlock_irqrestore(&chans[j]->lock, flags);

		printk(KERN_INFO "Dump of Zaptel Channel %d (%s,%d,%d):\n\n",j,
			mychan->name,mychan->channo,mychan->chanpos);
		printk(KERN_INFO "flags: %x hex, writechunk: %08lx, readchunk: %08lx\n",
			mychan->flags, (long) mychan->writechunk, (long) mychan->readchunk);
		printk(KERN_INFO "rxgain: %08lx, txgain: %08lx, gainalloc: %d\n",
			(long) mychan->rxgain, (long)mychan->txgain, mychan->gainalloc);
		printk(KERN_INFO "span: %08lx, sig: %x hex, sigcap: %x hex\n",
			(long)mychan->span, mychan->sig, mychan->sigcap);
		printk(KERN_INFO "inreadbuf: %d, outreadbuf: %d, inwritebuf: %d, outwritebuf: %d\n",
			mychan->inreadbuf, mychan->outreadbuf, mychan->inwritebuf, mychan->outwritebuf);
		printk(KERN_INFO "blocksize: %d, numbufs: %d, txbufpolicy: %d, txbufpolicy: %d\n",
			mychan->blocksize, mychan->numbufs, mychan->txbufpolicy, mychan->rxbufpolicy);
		printk(KERN_INFO "txdisable: %d, rxdisable: %d, iomask: %d\n",
			mychan->txdisable, mychan->rxdisable, mychan->iomask);
		printk(KERN_INFO "curzone: %08lx, tonezone: %d, curtone: %08lx, tonep: %d\n",
			(long) mychan->curzone, mychan->tonezone, (long) mychan->curtone, mychan->tonep);
		printk(KERN_INFO "digitmode: %d, txdialbuf: %s, dialing: %d, aftdialtimer: %d, cadpos. %d\n",
			mychan->digitmode, mychan->txdialbuf, mychan->dialing,
				mychan->afterdialingtimer, mychan->cadencepos);
		printk(KERN_INFO "confna: %d, confn: %d, confmode: %d, confmute: %d\n",
			mychan->confna, mychan->_confn, mychan->confmode, mychan->confmute);
		printk(KERN_INFO "ec: %08lx, echocancel: %d, deflaw: %d, xlaw: %08lx\n",
			(long) mychan->ec, mychan->echocancel, mychan->deflaw, (long) mychan->xlaw);
		printk(KERN_INFO "echostate: %02x, echotimer: %d, echolastupdate: %d\n",
			(int) mychan->echostate, mychan->echotimer, mychan->echolastupdate);
		printk(KERN_INFO "itimer: %d, otimer: %d, ringdebtimer: %d\n\n",
			mychan->itimer, mychan->otimer, mychan->ringdebtimer);
#if 0
		if (mychan->ec) {
			int x;
			/* Dump the echo canceller parameters */
			for (x=0;x<mychan->ec->taps;x++) {
				printk(KERN_INFO "tap %d: %d\n", x, mychan->ec->fir_taps[x]);
			}
		}
#endif
		kfree(mychan);
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

static int (*zt_dynamic_ioctl)(unsigned int cmd, unsigned long data);

void zt_set_dynamic_ioctl(int (*func)(unsigned int cmd, unsigned long data)) 
{
	zt_dynamic_ioctl = func;
}

static void recalc_slaves(struct zt_chan *chan)
{
	int x;
	struct zt_chan *last = chan;

	/* Makes no sense if you don't have a span */
	if (!chan->span)
		return;

#ifdef CONFIG_ZAPATA_DEBUG
	printk("Recalculating slaves on %s\n", chan->name);
#endif

	/* Link all slaves appropriately */
	for (x=chan->chanpos;x<chan->span->channels;x++)
		if (chan->span->chans[x].master == chan) {
#ifdef CONFIG_ZAPATA_DEBUG
			printk("Channel %s, slave to %s, last is %s, its next will be %d\n", 
			       chan->span->chans[x].name, chan->name, last->name, x);
#endif
			last->nextslave = x;
			last = &chan->span->chans[x];
		}
	/* Terminate list */
	last->nextslave = 0;
#ifdef CONFIG_ZAPATA_DEBUG
	printk("Done Recalculating slaves on %s (last is %s)\n", chan->name, last->name);
#endif
}

static int zt_ctl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data)
{
	/* I/O CTL's for control interface */
	int i,j;
	struct zt_lineconfig lc;
	struct zt_chanconfig ch;
	struct zt_sfconfig sf;
	int sigcap;
	int res = 0;
	int x,y;
	struct zt_chan *newmaster;
	struct zt_dialparams tdp;
	struct zt_maintinfo maint;
	struct zt_indirect_data ind;
	struct zt_versioninfo vi;
	unsigned long flags;
	int rv;
	switch(cmd) {
	case ZT_INDIRECT:
		if (copy_from_user(&ind, (struct zt_indirect_data *)data, sizeof(ind)))
			return -EFAULT;
		VALID_CHANNEL(ind.chan);
		return zt_chan_ioctl(inode, file, ind.op, (unsigned long) ind.data, ind.chan);
	case ZT_SPANCONFIG:
		if (copy_from_user(&lc, (struct zt_lineconfig *)data, sizeof(lc)))
			return -EFAULT;
		VALID_SPAN(lc.span);
		if ((lc.lineconfig & 0xf0 & spans[lc.span]->linecompat) != (lc.lineconfig & 0xf0))
			return -EINVAL;
		if (spans[lc.span]->spanconfig)
			return spans[lc.span]->spanconfig(spans[lc.span], &lc);
		return 0;
	case ZT_STARTUP:
		CHECK_VALID_SPAN(j);
		if (spans[j]->flags & ZT_FLAG_RUNNING)
			return 0;
		if (spans[j]->startup)
			res = spans[j]->startup(spans[j]);
		if (!res) {
			/* Mark as running and hangup any channels */
			spans[j]->flags |= ZT_FLAG_RUNNING;
			for (x=0;x<spans[j]->channels;x++) {
				y = zt_q_sig(&spans[j]->chans[x]) & 0xff;
				if (y >= 0) spans[j]->chans[x].rxsig = (unsigned char)y;
				spin_lock_irqsave(&spans[j]->chans[x].lock, flags);
				zt_hangup(&spans[j]->chans[x]);
				spin_unlock_irqrestore(&spans[j]->chans[x].lock, flags);
				spans[j]->chans[x].rxhooksig = ZT_RXSIG_INITIAL;
			}
		}
		return 0;
	case ZT_SHUTDOWN:
		CHECK_VALID_SPAN(j);
		if (spans[j]->shutdown)
			res =  spans[j]->shutdown(spans[j]);
		spans[j]->flags &= ~ZT_FLAG_RUNNING;
		return 0;
	case ZT_CHANCONFIG:
		if (copy_from_user(&ch, (struct zt_chanconfig *)data, sizeof(ch)))
			return -EFAULT;
		VALID_CHANNEL(ch.chan);
		if (ch.sigtype == ZT_SIG_SLAVE) {
			/* We have to use the master's sigtype */
			if ((ch.master < 1) || (ch.master >= ZT_MAX_CHANNELS))
				return -EINVAL;
			if (!chans[ch.master])
				return -EINVAL;
			ch.sigtype = chans[ch.master]->sig;
			newmaster = chans[ch.master];
		} else if ((ch.sigtype & __ZT_SIG_DACS) == __ZT_SIG_DACS) {
			newmaster = chans[ch.chan];
			if ((ch.idlebits < 1) || (ch.idlebits >= ZT_MAX_CHANNELS))
				return -EINVAL;
			if (!chans[ch.idlebits])
				return -EINVAL;
		} else {
			newmaster = chans[ch.chan];
		}
		spin_lock_irqsave(&chans[ch.chan]->lock, flags);
#ifdef CONFIG_ZAPATA_NET
		if (chans[ch.chan]->flags & ZT_FLAG_NETDEV) {
			if (ztchan_to_dev(chans[ch.chan])->flags & IFF_UP) {
				spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
				printk(KERN_WARNING "Can't switch HDLC net mode on channel %s, since current interface is up\n", chans[ch.chan]->name);
				return -EBUSY;
			}
#ifdef LINUX26
			spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
			unregister_hdlc_device(chans[ch.chan]->hdlcnetdev->netdev);
			spin_lock_irqsave(&chans[ch.chan]->lock, flags);
			free_netdev(chans[ch.chan]->hdlcnetdev->netdev);
#else
			unregister_hdlc_device(&chans[ch.chan]->hdlcnetdev->netdev);
#endif			
			kfree(chans[ch.chan]->hdlcnetdev);
			chans[ch.chan]->hdlcnetdev = NULL;
			chans[ch.chan]->flags &= ~ZT_FLAG_NETDEV;
		}
#else
		if (ch.sigtype == ZT_SIG_HDLCNET) {
				spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
				printk(KERN_WARNING "Zaptel networking not supported by this build.\n");
				return -ENOSYS;
		}
#endif		
		sigcap = chans[ch.chan]->sigcap;
		/* If they support clear channel, then they support the HDLC and such through
		   us.  */
		if (sigcap & ZT_SIG_CLEAR) 
			sigcap |= (ZT_SIG_HDLCRAW | ZT_SIG_HDLCFCS | ZT_SIG_HDLCNET | ZT_SIG_DACS);
		
		if ((sigcap & ch.sigtype) != ch.sigtype)
			res =  -EINVAL;	
		
		if (!res && chans[ch.chan]->span->chanconfig)
			res = chans[ch.chan]->span->chanconfig(chans[ch.chan], ch.sigtype);
		if (chans[ch.chan]->master) {
			/* Clear the master channel */
			recalc_slaves(chans[ch.chan]->master);
			chans[ch.chan]->nextslave = 0;
		}
		if (!res) {
			chans[ch.chan]->sig = ch.sigtype;
			if (chans[ch.chan]->sig == ZT_SIG_CAS)
				chans[ch.chan]->idlebits = ch.idlebits;
			else
				chans[ch.chan]->idlebits = 0;
			if ((ch.sigtype & ZT_SIG_CLEAR) == ZT_SIG_CLEAR) {
				/* Set clear channel flag if appropriate */
				chans[ch.chan]->flags &= ~ZT_FLAG_AUDIO;
				chans[ch.chan]->flags |= ZT_FLAG_CLEAR;
			} else {
				/* Set audio flag and not clear channel otherwise */
				chans[ch.chan]->flags |= ZT_FLAG_AUDIO;
				chans[ch.chan]->flags &= ~ZT_FLAG_CLEAR;
			}
			if ((ch.sigtype & ZT_SIG_HDLCRAW) == ZT_SIG_HDLCRAW) {
				/* Set the HDLC flag */
				chans[ch.chan]->flags |= ZT_FLAG_HDLC;
			} else {
				/* Clear the HDLC flag */
				chans[ch.chan]->flags &= ~ZT_FLAG_HDLC;
			}
			if ((ch.sigtype & ZT_SIG_HDLCFCS) == ZT_SIG_HDLCFCS) {
				/* Set FCS to be calculated if appropriate */
				chans[ch.chan]->flags |= ZT_FLAG_FCS;
			} else {
				/* Clear FCS flag */
				chans[ch.chan]->flags &= ~ZT_FLAG_FCS;
			}
			if ((ch.sigtype & __ZT_SIG_DACS) == __ZT_SIG_DACS) {
				/* Setup conference properly */
				chans[ch.chan]->confmode = ZT_CONF_DIGITALMON;
				chans[ch.chan]->confna = ch.idlebits;
				if (chans[ch.chan]->span && 
				    chans[ch.chan]->span->dacs && 
					chans[ch.idlebits] && 
					chans[ch.chan]->span && 
					(chans[ch.chan]->span->dacs == chans[ch.idlebits]->span->dacs)) 
					chans[ch.chan]->span->dacs(chans[ch.chan], chans[ch.idlebits]);
			} else if (chans[ch.chan]->span && chans[ch.chan]->span->dacs)
				chans[ch.chan]->span->dacs(chans[ch.chan], NULL);
			chans[ch.chan]->master = newmaster;
			/* Note new slave if we are not our own master */
			if (newmaster != chans[ch.chan]) {
				recalc_slaves(chans[ch.chan]->master);
			}
			if ((ch.sigtype & ZT_SIG_HARDHDLC) == ZT_SIG_HARDHDLC) {
				chans[ch.chan]->flags &= ~ZT_FLAG_FCS;
				chans[ch.chan]->flags &= ~ZT_FLAG_HDLC;
				chans[ch.chan]->flags |= ZT_FLAG_NOSTDTXRX;
			} else
				chans[ch.chan]->flags &= ~ZT_FLAG_NOSTDTXRX;
		}
#ifdef CONFIG_ZAPATA_NET
		if (!res && 
			(newmaster == chans[ch.chan]) && 
		        (chans[ch.chan]->sig == ZT_SIG_HDLCNET)) {
			chans[ch.chan]->hdlcnetdev = zt_hdlc_alloc();
			if (chans[ch.chan]->hdlcnetdev) {
/*				struct hdlc_device *hdlc = chans[ch.chan]->hdlcnetdev;
				struct net_device *d = hdlc_to_dev(hdlc); mmm...get it right later --byg */
#ifdef LINUX26
				chans[ch.chan]->hdlcnetdev->netdev = alloc_hdlcdev(chans[ch.chan]->hdlcnetdev);
				if (chans[ch.chan]->hdlcnetdev->netdev) {
					chans[ch.chan]->hdlcnetdev->chan = chans[ch.chan];
					SET_MODULE_OWNER(chans[ch.chan]->hdlcnetdev->netdev);
					chans[ch.chan]->hdlcnetdev->netdev->irq = chans[ch.chan]->span->irq;
					chans[ch.chan]->hdlcnetdev->netdev->tx_queue_len = 50;
					chans[ch.chan]->hdlcnetdev->netdev->do_ioctl = zt_net_ioctl;
					chans[ch.chan]->hdlcnetdev->netdev->open = zt_net_open;
					chans[ch.chan]->hdlcnetdev->netdev->stop = zt_net_stop;
					dev_to_hdlc(chans[ch.chan]->hdlcnetdev->netdev)->attach = zt_net_attach;
					dev_to_hdlc(chans[ch.chan]->hdlcnetdev->netdev)->xmit = zt_xmit;
					spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
					/* Briefly restore interrupts while we register the device */
					res = zt_register_hdlc_device(chans[ch.chan]->hdlcnetdev->netdev, ch.netdev_name);
					spin_lock_irqsave(&chans[ch.chan]->lock, flags);
				} else {
					printk("Unable to allocate hdlc: *shrug*\n");
					res = -1;
				}
#else /* LINUX26 */
				chans[ch.chan]->hdlcnetdev->chan = chans[ch.chan];
#ifndef HDLC_MAINTAINERS_ARE_MORE_STUPID_THAN_I_THOUGHT
				chans[ch.chan]->hdlcnetdev->netdev.ioctl = zt_net_ioctl;
#endif
				chans[ch.chan]->hdlcnetdev->netdev.netdev.do_ioctl = zt_net_ioctl;
#ifdef NEW_HDLC_INTERFACE
				chans[ch.chan]->hdlcnetdev->netdev.netdev.open = zt_net_open;
				chans[ch.chan]->hdlcnetdev->netdev.netdev.stop = zt_net_stop;
				chans[ch.chan]->hdlcnetdev->netdev.xmit = zt_xmit;
				chans[ch.chan]->hdlcnetdev->netdev.attach = zt_net_attach;
#else
				chans[ch.chan]->hdlcnetdev->netdev.open = zt_net_open;
				chans[ch.chan]->hdlcnetdev->netdev.close = zt_net_close;
				chans[ch.chan]->hdlcnetdev->netdev.set_mode = NULL;
				chans[ch.chan]->hdlcnetdev->netdev.xmit = zt_xmit;
#endif /* NEW_HDLC_INTERFACE */
				chans[ch.chan]->hdlcnetdev->netdev.netdev.irq = chans[ch.chan]->span->irq;
				chans[ch.chan]->hdlcnetdev->netdev.netdev.tx_queue_len = 50;
				res = register_hdlc_device(&chans[ch.chan]->hdlcnetdev->netdev);
#endif /* LINUX26 */
				if (!res)
					chans[ch.chan]->flags |= ZT_FLAG_NETDEV;
			} else {
				printk("Unable to allocate netdev: out of memory\n");
				res = -1;
			}
		}
#endif		
		if ((chans[ch.chan]->sig == ZT_SIG_HDLCNET) && 
		    (chans[ch.chan] == newmaster) &&
		    !(chans[ch.chan]->flags & ZT_FLAG_NETDEV))
			printk("Unable to register HDLC device for channel %s\n", chans[ch.chan]->name);
		if (!res) {
			/* Setup default law */
			chans[ch.chan]->deflaw = ch.deflaw;
			/* Copy back any modified settings */
			spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
			if (copy_to_user((struct zt_chanconfig *)data, &ch, sizeof(ch)))
				return -EFAULT;
			spin_lock_irqsave(&chans[ch.chan]->lock, flags);
			/* And hangup */
			zt_hangup(chans[ch.chan]);
			y = zt_q_sig(chans[ch.chan]) & 0xff;
			if (y >= 0) chans[ch.chan]->rxsig = (unsigned char)y;
			chans[ch.chan]->rxhooksig = ZT_RXSIG_INITIAL;
		}
#ifdef CONFIG_ZAPATA_DEBUG
		printk("Configured channel %s, flags %04x, sig %04x\n", chans[ch.chan]->name, chans[ch.chan]->flags, chans[ch.chan]->sig);
#endif		
		spin_unlock_irqrestore(&chans[ch.chan]->lock, flags);
		return res;
	case ZT_SFCONFIG:
		if (copy_from_user(&sf, (struct zt_chanconfig *)data, sizeof(sf)))
			return -EFAULT;
		VALID_CHANNEL(sf.chan);
		if (chans[sf.chan]->sig != ZT_SIG_SF) return -EINVAL;
		spin_lock_irqsave(&chans[sf.chan]->lock, flags);
		chans[sf.chan]->rxp1 = sf.rxp1;
		chans[sf.chan]->rxp2 = sf.rxp2;
		chans[sf.chan]->rxp3 = sf.rxp3;
		chans[sf.chan]->txtone = sf.txtone;
		chans[sf.chan]->tx_v2 = sf.tx_v2;
		chans[sf.chan]->tx_v3 = sf.tx_v3;
		chans[sf.chan]->toneflags = sf.toneflag;
		if (sf.txtone) /* if set to make tone for tx */
		{
			if ((chans[sf.chan]->txhooksig && !(sf.toneflag & ZT_REVERSE_TXTONE)) ||
			 ((!chans[sf.chan]->txhooksig) && (sf.toneflag & ZT_REVERSE_TXTONE))) 
			{
				set_txtone(chans[sf.chan],sf.txtone,sf.tx_v2,sf.tx_v3);
			}
			else
			{
				set_txtone(chans[sf.chan],0,0,0);
			}
		}
		spin_unlock_irqrestore(&chans[sf.chan]->lock, flags);
		return res;
	case ZT_DEFAULTZONE:
		if (get_user(j,(int *)data))
			return -EFAULT;  /* get conf # */
		if ((j < 0) || (j >= ZT_TONE_ZONE_MAX)) return (-EINVAL);
		write_lock(&zone_lock);
		default_zone = j;
		write_unlock(&zone_lock);
		return 0;
	case ZT_LOADZONE:
		return ioctl_load_zone(data);
	case ZT_FREEZONE:
		get_user(j,(int *)data);  /* get conf # */
		if ((j < 0) || (j >= ZT_TONE_ZONE_MAX)) return (-EINVAL);
		free_tone_zone(j);
		return 0;
	case ZT_SET_DIALPARAMS:
		if (copy_from_user(&tdp, (struct zt_dialparams *)data, sizeof(tdp)))
			return -EFAULT;
		if ((tdp.dtmf_tonelen > 4000) || (tdp.dtmf_tonelen < 10))
			return -EINVAL;
		if ((tdp.mfv1_tonelen > 4000) || (tdp.mfv1_tonelen < 10))
			return -EINVAL;
		for (i=0;i<16;i++)
			dtmf_tones[i].tonesamples = tdp.dtmf_tonelen * ZT_CHUNKSIZE;
		dtmf_silence.tonesamples = tdp.dtmf_tonelen * ZT_CHUNKSIZE;
		for (i=0;i<15;i++)
			mfv1_tones[i].tonesamples = tdp.mfv1_tonelen * ZT_CHUNKSIZE;
		mfv1_silence.tonesamples = tdp.mfv1_tonelen * ZT_CHUNKSIZE;
		/* Special case for K/P tone */
		mfv1_tones[10].tonesamples = tdp.mfv1_tonelen * ZT_CHUNKSIZE * 5 / 3;
		break;
	case ZT_GET_DIALPARAMS:
		tdp.dtmf_tonelen = dtmf_tones[0].tonesamples / ZT_CHUNKSIZE;
		tdp.mfv1_tonelen = mfv1_tones[0].tonesamples / ZT_CHUNKSIZE;
		tdp.reserved[0] = 0;
		tdp.reserved[1] = 0;
		tdp.reserved[2] = 0;
		tdp.reserved[3] = 0;
		if (copy_to_user((struct zt_dialparams *)data, &tdp, sizeof(tdp)))
			return -EFAULT;
		break;
	case ZT_GETVERSION:
		memset(&vi, 0, sizeof(vi));
		strncpy(vi.version, ZAPTEL_VERSION, sizeof(vi.version) - 1);
		echo_can_identify(vi.echo_canceller, sizeof(vi.echo_canceller) - 1);
		if (copy_to_user((struct zt_versioninfo *) data, &vi, sizeof(vi)))
			return -EFAULT;
		break;
	case ZT_MAINT:  /* do maintenance stuff */
		  /* get struct from user */
		if (copy_from_user(&maint,(struct zt_maintinfo *) data, sizeof(maint)))
			return -EFAULT;
		/* must be valid span number */
		if ((maint.spanno < 1) || (maint.spanno > ZT_MAX_SPANS) || (!spans[maint.spanno]))
			return -EINVAL;
		if (!spans[maint.spanno]->maint)
			return -ENOSYS;
		spin_lock_irqsave(&spans[maint.spanno]->lock, flags);
		  /* save current maint state */
		i = spans[maint.spanno]->maintstat;
		  /* set maint mode */
		spans[maint.spanno]->maintstat = maint.command;
		switch(maint.command) {
		case ZT_MAINT_NONE:
		case ZT_MAINT_LOCALLOOP:
		case ZT_MAINT_REMOTELOOP:
			/* if same, ignore it */
			if (i == maint.command) break;
			rv = spans[maint.spanno]->maint(spans[maint.spanno], maint.command);
			spin_unlock_irqrestore(&spans[maint.spanno]->lock, flags);
			if (rv) return rv;
			spin_lock_irqsave(&spans[maint.spanno]->lock, flags);
			break;
		case ZT_MAINT_LOOPUP:
		case ZT_MAINT_LOOPDOWN:
			spans[maint.spanno]->mainttimer = ZT_LOOPCODE_TIME * ZT_CHUNKSIZE;
			rv = spans[maint.spanno]->maint(spans[maint.spanno], maint.command);
			spin_unlock_irqrestore(&spans[maint.spanno]->lock, flags);
			if (rv) return rv;
			rv = schluffen(&spans[maint.spanno]->maintq);
			if (rv) return rv;
			spin_lock_irqsave(&spans[maint.spanno]->lock, flags);
			break;
		default:
			printk("zaptel: Unknown maintenance event: %d\n", maint.command);
		}
		zt_alarm_notify(spans[maint.spanno]);  /* process alarm-related events */
		spin_unlock_irqrestore(&spans[maint.spanno]->lock, flags);
		break;
	case ZT_DYNAMIC_CREATE:
	case ZT_DYNAMIC_DESTROY:
		if (zt_dynamic_ioctl)
			return zt_dynamic_ioctl(cmd, data);
		else {
			request_module("ztdynamic");
			if (zt_dynamic_ioctl)
				return zt_dynamic_ioctl(cmd, data);
		}
		return -ENOSYS;
#if defined(ECHO_CAN_HPEC)
	case ZT_EC_LICENSE_CHALLENGE:
	case ZT_EC_LICENSE_RESPONSE:
		return hpec_license_ioctl(cmd, data);
#endif /* defined(ECHO_CAN_HPEC) */
	default:
		return zt_common_ioctl(inode, file, cmd, data, 0);
	}
	return 0;
}

static int zt_chanandpseudo_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data, int unit)
{
	struct zt_chan *chan = chans[unit];
	union {
		struct zt_dialoperation tdo;
		struct zt_bufferinfo bi;
		struct zt_confinfo conf;
		struct zt_ring_cadence cad;
	} stack;
	unsigned long flags, flagso;
	int i, j, k, rv;
	int ret, c;
	
	if (!chan)
		return -EINVAL;
	switch(cmd) {
	case ZT_DIALING:
		spin_lock_irqsave(&chan->lock, flags);
		j = chan->dialing;
		spin_unlock_irqrestore(&chan->lock, flags);
		if (copy_to_user((int *)data,&j,sizeof(int)))
			return -EFAULT;
		return 0;
	case ZT_DIAL:
		if (copy_from_user(&stack.tdo, (struct zt_dialoperation *)data, sizeof(stack.tdo)))
			return -EFAULT;
		rv = 0;
		/* Force proper NULL termination */
		stack.tdo.dialstr[ZT_MAX_DTMF_BUF - 1] = '\0';
		spin_lock_irqsave(&chan->lock, flags);
		switch(stack.tdo.op) {
		case ZT_DIAL_OP_CANCEL:
			chan->curtone = NULL;
			chan->dialing = 0;
			chan->txdialbuf[0] = '\0';
			chan->tonep = 0;
			chan->pdialcount = 0;
			break;
		case ZT_DIAL_OP_REPLACE:
			strcpy(chan->txdialbuf, stack.tdo.dialstr);
			chan->dialing = 1;
			__do_dtmf(chan);
			break;
		case ZT_DIAL_OP_APPEND:
			if (strlen(stack.tdo.dialstr) + strlen(chan->txdialbuf) >= ZT_MAX_DTMF_BUF)
			   {
				rv = -EBUSY;
				break;
			   }
			strncpy(chan->txdialbuf + strlen(chan->txdialbuf), stack.tdo.dialstr, ZT_MAX_DTMF_BUF - strlen(chan->txdialbuf));
			if (!chan->dialing)
			   {
				chan->dialing = 1;
				__do_dtmf(chan);
			   }
			break;
		default:
			rv = -EINVAL;
		}
		spin_unlock_irqrestore(&chan->lock, flags);
		return rv;
	case ZT_GET_BUFINFO:
		stack.bi.rxbufpolicy = chan->rxbufpolicy;
		stack.bi.txbufpolicy = chan->txbufpolicy;
		stack.bi.numbufs = chan->numbufs;
		stack.bi.bufsize = chan->blocksize;
		/* XXX FIXME! XXX */
		stack.bi.readbufs = -1;
		stack.bi.writebufs = -1;
		if (copy_to_user((struct zt_bufferinfo *)data, &stack.bi, sizeof(stack.bi)))
			return -EFAULT;
		break;
	case ZT_SET_BUFINFO:
		if (copy_from_user(&stack.bi, (struct zt_bufferinfo *)data, sizeof(stack.bi)))
			return -EFAULT;
		if (stack.bi.bufsize > ZT_MAX_BLOCKSIZE)
			return -EINVAL;
		if (stack.bi.bufsize < 16)
			return -EINVAL;
		if (stack.bi.bufsize * stack.bi.numbufs > ZT_MAX_BUF_SPACE)
			return -EINVAL;
		chan->rxbufpolicy = stack.bi.rxbufpolicy & 0x1;
		chan->txbufpolicy = stack.bi.txbufpolicy & 0x1;
		if ((rv = zt_reallocbufs(chan,  stack.bi.bufsize, stack.bi.numbufs)))
			return (rv);
		break;
	case ZT_GET_BLOCKSIZE:  /* get blocksize */
		put_user(chan->blocksize,(int *)data); /* return block size */
		break;
	case ZT_SET_BLOCKSIZE:  /* set blocksize */
		get_user(j,(int *)data);
		  /* cannot be larger than max amount */
		if (j > ZT_MAX_BLOCKSIZE) return(-EINVAL);
		  /* cannot be less then 16 */
		if (j < 16) return(-EINVAL);
		  /* allocate a single kernel buffer which we then
		     sub divide into four pieces */
		if ((rv = zt_reallocbufs(chan, j, chan->numbufs)))
			return (rv);
		break;
	case ZT_FLUSH:  /* flush input buffer, output buffer, and/or event queue */
		get_user(i,(int *)data);  /* get param */
		spin_lock_irqsave(&chan->lock, flags);
		if (i & ZT_FLUSH_READ)  /* if for read (input) */
		   {
			  /* initialize read buffers and pointers */
			chan->inreadbuf = 0;
			chan->outreadbuf = -1;
			for (j=0;j<chan->numbufs;j++) {
				/* Do we need this? */
				chan->readn[j] = 0;
				chan->readidx[j] = 0;
			}
			wake_up_interruptible(&chan->readbufq);  /* wake_up_interruptible waiting on read */
			wake_up_interruptible(&chan->sel); /* wake_up_interruptible waiting on select */
		   }
		if (i & ZT_FLUSH_WRITE) /* if for write (output) */
		   {
			  /* initialize write buffers and pointers */
			chan->outwritebuf = -1;
			chan->inwritebuf = 0;
			for (j=0;j<chan->numbufs;j++) {
				/* Do we need this? */
				chan->writen[j] = 0;
				chan->writeidx[j] = 0;
			}
			wake_up_interruptible(&chan->writebufq); /* wake_up_interruptible waiting on write */
			wake_up_interruptible(&chan->sel);  /* wake_up_interruptible waiting on select */
			   /* if IO MUX wait on write empty, well, this
				certainly *did* empty the write */
			if (chan->iomask & ZT_IOMUX_WRITEEMPTY)
				wake_up_interruptible(&chan->eventbufq); /* wake_up_interruptible waiting on IOMUX */
		   }
		if (i & ZT_FLUSH_EVENT) /* if for events */
		   {
			   /* initialize the event pointers */
			chan->eventinidx = chan->eventoutidx = 0;
		   }
		spin_unlock_irqrestore(&chan->lock, flags);
		break;
	case ZT_SYNC:  /* wait for no tx */
		for(;;)  /* loop forever */
		   {
			spin_lock_irqsave(&chan->lock, flags);
			  /* Know if there is a write pending */
			i = (chan->outwritebuf > -1);
			spin_unlock_irqrestore(&chan->lock, flags);
			if (!i) break; /* skip if none */
			rv = schluffen(&chan->writebufq);
			if (rv) return(rv);
		   }
		break;
	case ZT_IOMUX: /* wait for something to happen */
		get_user(chan->iomask,(int*)data);  /* save mask */
		if (!chan->iomask) return(-EINVAL);  /* cant wait for nothing */
		for(;;)  /* loop forever */
		   {
			  /* has to have SOME mask */
			ret = 0;  /* start with empty return value */
			spin_lock_irqsave(&chan->lock, flags);
			  /* if looking for read */
			if (chan->iomask & ZT_IOMUX_READ)
			   {
				/* if read available */
				if ((chan->outreadbuf > -1)  && !chan->rxdisable)
					ret |= ZT_IOMUX_READ;
			   }
			  /* if looking for write avail */
			if (chan->iomask & ZT_IOMUX_WRITE)
			   {
				if (chan->inwritebuf > -1)
					ret |= ZT_IOMUX_WRITE;
			   }
			  /* if looking for write empty */
			if (chan->iomask & ZT_IOMUX_WRITEEMPTY)
			   {
				  /* if everything empty -- be sure the transmitter is enabled */
				chan->txdisable = 0;
				if (chan->outwritebuf < 0)
					ret |= ZT_IOMUX_WRITEEMPTY;
			   }
			  /* if looking for signalling event */
			if (chan->iomask & ZT_IOMUX_SIGEVENT)
			   {
				  /* if event */
				if (chan->eventinidx != chan->eventoutidx)
					ret |= ZT_IOMUX_SIGEVENT;
			   }
			spin_unlock_irqrestore(&chan->lock, flags);
			  /* if something to return, or not to wait */
			if (ret || (chan->iomask & ZT_IOMUX_NOWAIT))
			   {
				  /* set return value */
				put_user(ret,(int *)data);
				break; /* get out of loop */
			   }
			rv = schluffen(&chan->eventbufq);
			if (rv) return(rv);
		   }
		  /* clear IO MUX mask */
		chan->iomask = 0;
		break;
	case ZT_GETEVENT:  /* Get event on queue */
		  /* set up for no event */
		j = ZT_EVENT_NONE;
		spin_lock_irqsave(&chan->lock, flags);
		  /* if some event in queue */
		if (chan->eventinidx != chan->eventoutidx)
		   {
			j = chan->eventbuf[chan->eventoutidx++];
			  /* get the data, bump index */
			  /* if index overflow, set to beginning */
			if (chan->eventoutidx >= ZT_MAX_EVENTSIZE)
				chan->eventoutidx = 0;
		   }		
		spin_unlock_irqrestore(&chan->lock, flags);
		put_user(j,(int *)data);
		break;
	case ZT_CONFMUTE:  /* set confmute flag */
		get_user(j,(int *)data);  /* get conf # */
		if (!(chan->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		spin_lock_irqsave(&bigzaplock, flags);
		chan->confmute = j;
		spin_unlock_irqrestore(&bigzaplock, flags);
		break;
	case ZT_GETCONFMUTE:  /* get confmute flag */
		if (!(chan->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		j = chan->confmute;
		put_user(j,(int *)data);  /* get conf # */
		rv = 0;
		break;
	case ZT_SETTONEZONE:
		get_user(j,(int *)data);
		spin_lock_irqsave(&chan->lock, flags);
		rv =  set_tone_zone(chan, j);
		spin_unlock_irqrestore(&chan->lock, flags);
		return rv;
	case ZT_GETTONEZONE:
		spin_lock_irqsave(&chan->lock, flags);
		if (chan->curzone)
			rv = chan->tonezone;
		else
			rv = default_zone;
		spin_unlock_irqrestore(&chan->lock, flags);
		put_user(rv,(int *)data); /* return value */
		break;
	case ZT_SENDTONE:
		get_user(j,(int *)data);
		spin_lock_irqsave(&chan->lock, flags);
		rv = start_tone(chan, j);	
		spin_unlock_irqrestore(&chan->lock, flags);
		return rv;
	case ZT_GETCONF:  /* get conf stuff */
		if (copy_from_user(&stack.conf,(struct zt_confinfo *) data,sizeof(stack.conf)))
			return -EFAULT;
		i = stack.conf.chan;  /* get channel no */
		   /* if zero, use current channel no */
		if (!i) i = chan->channo;
		  /* make sure channel number makes sense */
		if ((i < 0) || (i > ZT_MAX_CONF) || (!chans[i])) return(-EINVAL);
		if (!(chans[i]->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		stack.conf.chan = i;  /* get channel number */
		stack.conf.confno = chans[i]->confna;  /* get conference number */
		stack.conf.confmode = chans[i]->confmode; /* get conference mode */
		if (copy_to_user((struct zt_confinfo *) data,&stack.conf,sizeof(stack.conf)))
			return -EFAULT;
		break;
	case ZT_SETCONF:  /* set conf stuff */
		if (copy_from_user(&stack.conf,(struct zt_confinfo *) data,sizeof(stack.conf)))
			return -EFAULT;
		i = stack.conf.chan;  /* get channel no */
		   /* if zero, use current channel no */
		if (!i) i = chan->channo;
		  /* make sure channel number makes sense */
		if ((i < 1) || (i > ZT_MAX_CHANNELS) || (!chans[i])) return(-EINVAL);
		if (!(chans[i]->flags & ZT_FLAG_AUDIO)) return (-EINVAL); 
		if ((stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORTX ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORBOTH ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_RX_PREECHO ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_TX_PREECHO ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORBOTH_PREECHO) {
			/* Monitor mode -- it's a channel */
			if ((stack.conf.confno < 0) || (stack.conf.confno >= ZT_MAX_CHANNELS) || !chans[stack.conf.confno]) return(-EINVAL);
		} else {
			  /* make sure conf number makes sense, too */
			if ((stack.conf.confno < -1) || (stack.conf.confno > ZT_MAX_CONF)) return(-EINVAL);
		}
			
		  /* if taking off of any conf, must have 0 mode */
		if ((!stack.conf.confno) && stack.conf.confmode) return(-EINVAL);
		  /* likewise if 0 mode must have no conf */
		if ((!stack.conf.confmode) && stack.conf.confno) return (-EINVAL);
		stack.conf.chan = i;  /* return with real channel # */
		spin_lock_irqsave(&bigzaplock, flagso);
		spin_lock_irqsave(&chan->lock, flags);
		if (stack.conf.confno == -1) 
			stack.conf.confno = zt_first_empty_conference();
		if ((stack.conf.confno < 1) && (stack.conf.confmode)) {
			/* No more empty conferences */
			spin_unlock_irqrestore(&chan->lock, flags);
			spin_unlock_irqrestore(&bigzaplock, flagso);
			return -EBUSY;
		}
		  /* if changing confs, clear last added info */
		if (stack.conf.confno != chans[i]->confna) {
			memset(chans[i]->conflast, 0, ZT_MAX_CHUNKSIZE);
			memset(chans[i]->conflast1, 0, ZT_MAX_CHUNKSIZE);
			memset(chans[i]->conflast2, 0, ZT_MAX_CHUNKSIZE);
		}
		j = chans[i]->confna;  /* save old conference number */
		chans[i]->confna = stack.conf.confno;   /* set conference number */
		chans[i]->confmode = stack.conf.confmode;  /* set conference mode */
		chans[i]->_confn = 0;		     /* Clear confn */
		zt_check_conf(j);
		zt_check_conf(stack.conf.confno);
		if (chans[i]->span && chans[i]->span->dacs) {
			if (((stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_DIGITALMON) &&
			    chans[stack.conf.confno]->span &&
			    chans[stack.conf.confno]->span->dacs == chans[i]->span->dacs &&
			    chans[i]->txgain == defgain &&
			    chans[i]->rxgain == defgain &&
			    chans[stack.conf.confno]->txgain == defgain &&
			    chans[stack.conf.confno]->rxgain == defgain) {
				chans[i]->span->dacs(chans[i], chans[stack.conf.confno]);
			} else {
				chans[i]->span->dacs(chans[i], NULL);
			}
		}
		/* if we are going onto a conf */
		if (stack.conf.confno &&
			((stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONF ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFANN ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFMON ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_CONFANNMON ||
			(stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_REALANDPSEUDO)) {
			/* Get alias */
			chans[i]->_confn = zt_get_conf_alias(stack.conf.confno);
		}

		if (chans[stack.conf.confno]) {
			if ((stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_RX_PREECHO ||
			    (stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITOR_TX_PREECHO ||
			    (stack.conf.confmode & ZT_CONF_MODE_MASK) == ZT_CONF_MONITORBOTH_PREECHO)
				chans[stack.conf.confno]->readchunkpreec = kmalloc(sizeof(*chans[stack.conf.confno]->readchunkpreec) * ZT_CHUNKSIZE, GFP_ATOMIC);
			else {
				if (chans[stack.conf.confno]->readchunkpreec) {
					kfree(chans[stack.conf.confno]->readchunkpreec);
					chans[stack.conf.confno]->readchunkpreec = NULL;
				}
			}
		}

		spin_unlock_irqrestore(&chan->lock, flags);
		spin_unlock_irqrestore(&bigzaplock, flagso);
		if (copy_to_user((struct zt_confinfo *) data,&stack.conf,sizeof(stack.conf)))
			return -EFAULT;
		break;
	case ZT_CONFLINK:  /* do conf link stuff */
		if (!(chan->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		if (copy_from_user(&stack.conf,(struct zt_confinfo *) data,sizeof(stack.conf)))
			return -EFAULT;
		  /* check sanity of arguments */
		if ((stack.conf.chan < 0) || (stack.conf.chan > ZT_MAX_CONF)) return(-EINVAL);
		if ((stack.conf.confno < 0) || (stack.conf.confno > ZT_MAX_CONF)) return(-EINVAL);
		  /* cant listen to self!! */
		if (stack.conf.chan && (stack.conf.chan == stack.conf.confno)) return(-EINVAL);
		spin_lock_irqsave(&bigzaplock, flagso);
		spin_lock_irqsave(&chan->lock, flags);
		  /* if to clear all links */
		if ((!stack.conf.chan) && (!stack.conf.confno))
		   {
			   /* clear all the links */
			memset(conf_links,0,sizeof(conf_links));
			recalc_maxlinks();
			spin_unlock_irqrestore(&chan->lock, flags);
			spin_unlock_irqrestore(&bigzaplock, flagso);
			break;
		   }
		rv = 0;  /* clear return value */
		/* look for already existant specified combination */
		for(i = 1; i <= ZT_MAX_CONF; i++)
		   {
			  /* if found, exit */
			if ((conf_links[i].src == stack.conf.chan) &&
				(conf_links[i].dst == stack.conf.confno)) break;
		   }
		if (i <= ZT_MAX_CONF) /* if found */
		   {
			if (!stack.conf.confmode) /* if to remove link */
			   {
				conf_links[i].src = conf_links[i].dst = 0;
			   }
			else /* if to add and already there, error */
			   {
				rv = -EEXIST;
			   }
		   }
		else  /* if not found */
		   {
			if (stack.conf.confmode) /* if to add link */
			   {
				/* look for empty location */
				for(i = 1; i <= ZT_MAX_CONF; i++)
				   {
					  /* if empty, exit loop */
					if ((!conf_links[i].src) &&
						 (!conf_links[i].dst)) break;
				   }
				   /* if empty spot found */
				if (i <= ZT_MAX_CONF)
				   {
					conf_links[i].src = stack.conf.chan;
					conf_links[i].dst = stack.conf.confno;
				   }
				else /* if no empties -- error */
				   {
					rv = -ENOSPC;
				   }
			   }
			else /* if to remove, and not found -- error */
			   {
				rv = -ENOENT;
			   }
		   }
		recalc_maxlinks();
		spin_unlock_irqrestore(&chan->lock, flags);
		spin_unlock_irqrestore(&bigzaplock, flagso);
		return(rv);
	case ZT_CONFDIAG:  /* output diagnostic info to console */
		if (!(chan->flags & ZT_FLAG_AUDIO)) return (-EINVAL);
		get_user(j,(int *)data);  /* get conf # */
 		  /* loop thru the interesting ones */
		for(i = ((j) ? j : 1); i <= ((j) ? j : ZT_MAX_CONF); i++)
		   {
			c = 0;
			for(k = 1; k < ZT_MAX_CHANNELS; k++)
			   {
				  /* skip if no pointer */
				if (!chans[k]) continue;
				  /* skip if not in this conf */
				if (chans[k]->confna != i) continue;
				if (!c) printk("Conf #%d:\n",i);
				c = 1;
				printk("chan %d, mode %x\n",
					k,chans[k]->confmode);
			   }
			rv = 0;
			for(k = 1; k <= ZT_MAX_CONF; k++)
			   {
				if (conf_links[k].dst == i)
				   {
					if (!c) printk("Conf #%d:\n",i);
					c = 1;
					if (!rv) printk("Snooping on:\n");
					rv = 1;
					printk("conf %d\n",conf_links[k].src);
				   }
			   }
			if (c) printk("\n");
		   }
		break;
	case ZT_CHANNO:  /* get channel number of stream */
		put_user(unit,(int *)data); /* return unit/channel number */
		break;
	case ZT_SETLAW:
		get_user(j, (int *)data);
		if ((j < 0) || (j > ZT_LAW_ALAW))
			return -EINVAL;
		zt_set_law(chan, j);
		break;
	case ZT_SETLINEAR:
		get_user(j, (int *)data);
		/* Makes no sense on non-audio channels */
		if (!(chan->flags & ZT_FLAG_AUDIO))
			return -EINVAL;

		if (j)
			chan->flags |= ZT_FLAG_LINEAR;
		else
			chan->flags &= ~ZT_FLAG_LINEAR;
		break;
	case ZT_SETCADENCE:
		if (data) {
			/* Use specific ring cadence */
			if (copy_from_user(&stack.cad, (struct zt_ring_cadence *)data, sizeof(stack.cad)))
				return -EFAULT;
			memcpy(chan->ringcadence, &stack.cad, sizeof(chan->ringcadence));
			chan->firstcadencepos = 0;
			/* Looking for negative ringing time indicating where to loop back into ringcadence */
			for (i=0; i<ZT_MAX_CADENCE; i+=2 ) {
				if (chan->ringcadence[i]<0) {
					chan->ringcadence[i] *= -1;
					chan->firstcadencepos = i;
					break;
				}
			}
		} else {
			/* Reset to default */
			chan->firstcadencepos = 0;
			if (chan->curzone) {
				memcpy(chan->ringcadence, chan->curzone->ringcadence, sizeof(chan->ringcadence));
				/* Looking for negative ringing time indicating where to loop back into ringcadence */
				for (i=0; i<ZT_MAX_CADENCE; i+=2 ) {
					if (chan->ringcadence[i]<0) {
						chan->ringcadence[i] *= -1;
						chan->firstcadencepos = i;
						break;
					}
				}
			} else {
				memset(chan->ringcadence, 0, sizeof(chan->ringcadence));
				chan->ringcadence[0] = chan->starttime;
				chan->ringcadence[1] = ZT_RINGOFFTIME;
			}
		}
		break;
	default:
		/* Check for common ioctl's and private ones */
		rv = zt_common_ioctl(inode, file, cmd, data, unit);
		/* if no span, just return with value */
		if (!chan->span) return rv;
		if ((rv == -ENOTTY) && chan->span->ioctl) 
			rv = chan->span->ioctl(chan, cmd, data);
		return rv;
		
	}
	return 0;
}

#ifdef CONFIG_ZAPATA_PPP
/*
 * This is called at softirq (BH) level when there are calls
 * we need to make to the ppp_generic layer.  We do it this
 * way because the ppp_generic layer functions may not be called
 * at interrupt level.
 */
static void do_ppp_calls(unsigned long data)
{
	struct zt_chan *chan = (struct zt_chan *) data;
	struct sk_buff *skb;

	if (!chan->ppp)
		return;
	if (chan->do_ppp_wakeup) {
		chan->do_ppp_wakeup = 0;
		ppp_output_wakeup(chan->ppp);
	}
	while ((skb = skb_dequeue(&chan->ppp_rq)) != NULL)
		ppp_input(chan->ppp, skb);
	if (chan->do_ppp_error) {
		chan->do_ppp_error = 0;
		ppp_input_error(chan->ppp, 0);
	}
}
#endif

static int zt_chan_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data, int unit)
{
	struct zt_chan *chan = chans[unit];
	unsigned long flags;
	int j, rv;
	int ret;
	int oldconf;
	void *rxgain=NULL;
	struct echo_can_state *ec, *tec;

	if (!chan)
		return -ENOSYS;

	switch(cmd) {
	case ZT_SIGFREEZE:
		get_user(j, (int *)data);
		spin_lock_irqsave(&chan->lock, flags);
		if (j) {
			chan->flags |= ZT_FLAG_SIGFREEZE;
		} else {
			chan->flags &= ~ZT_FLAG_SIGFREEZE;
		}
		spin_unlock_irqrestore(&chan->lock, flags);
		break;
	case ZT_GETSIGFREEZE:
		spin_lock_irqsave(&chan->lock, flags);
		if (chan->flags & ZT_FLAG_SIGFREEZE)
			j = 1;
		else
			j = 0;
		spin_unlock_irqrestore(&chan->lock, flags);
		put_user(j, (int *)data);
		break;
	case ZT_AUDIOMODE:
		/* Only literal clear channels can be put in  */
		if (chan->sig != ZT_SIG_CLEAR) return (-EINVAL);
		get_user(j, (int *)data);
		if (j) {
			spin_lock_irqsave(&chan->lock, flags);
			chan->flags |= ZT_FLAG_AUDIO;
			chan->flags &= ~(ZT_FLAG_HDLC | ZT_FLAG_FCS);
			spin_unlock_irqrestore(&chan->lock, flags);
		} else {
			/* Coming out of audio mode, also clear all 
			   conferencing and gain related info as well
			   as echo canceller */
			spin_lock_irqsave(&chan->lock, flags);
			chan->flags &= ~ZT_FLAG_AUDIO;
			/* save old conf number, if any */
			oldconf = chan->confna;
			  /* initialize conference variables */
			chan->_confn = 0;
			chan->confna = 0;
			if (chan->span && chan->span->dacs)
				chan->span->dacs(chan, NULL);
			chan->confmode = 0;
			chan->confmute = 0;
			memset(chan->conflast, 0, sizeof(chan->conflast));
			memset(chan->conflast1, 0, sizeof(chan->conflast1));
			memset(chan->conflast2, 0, sizeof(chan->conflast2));
			ec = chan->ec;
			chan->ec = NULL;
			/* release conference resource, if any to release */
			reset_conf(chan);
			if (chan->gainalloc && chan->rxgain)
				rxgain = chan->rxgain;
			else
				rxgain = NULL;

			chan->rxgain = defgain;
			chan->txgain = defgain;
			chan->gainalloc = 0;
			/* Disable any native echo cancellation as well */
			spin_unlock_irqrestore(&chan->lock, flags);

			if (chan->span && chan->span->echocan)
				chan->span->echocan(chan, 0);

			if (rxgain)
				kfree(rxgain);
			if (ec)
				echo_can_free(ec);
			if (oldconf) zt_check_conf(oldconf);
		}
		break;
	case ZT_HDLCPPP:
#ifdef CONFIG_ZAPATA_PPP
		if (chan->sig != ZT_SIG_CLEAR) return (-EINVAL);
		get_user(j, (int *)data);
		if (j) {
			if (!chan->ppp) {
				chan->ppp = kmalloc(sizeof(struct ppp_channel), GFP_KERNEL);
				if (chan->ppp) {
					memset(chan->ppp, 0, sizeof(struct ppp_channel));
					chan->ppp->private = chan;
					chan->ppp->ops = &ztppp_ops;
					chan->ppp->mtu = ZT_DEFAULT_MTU_MRU;
					chan->ppp->hdrlen = 0;
					skb_queue_head_init(&chan->ppp_rq);
					chan->do_ppp_wakeup = 0;
					tasklet_init(&chan->ppp_calls, do_ppp_calls,
						     (unsigned long)chan);
					if ((ret = zt_reallocbufs(chan, ZT_DEFAULT_MTU_MRU, ZT_DEFAULT_NUM_BUFS))) {
						kfree(chan->ppp);
						chan->ppp = NULL;
						return ret;
					}
						
					if ((ret = ppp_register_channel(chan->ppp))) {
						kfree(chan->ppp);
						chan->ppp = NULL;
						return ret;
					}
					tec = chan->ec;
					chan->ec = NULL;
					chan->echocancel = 0;
					chan->echostate = ECHO_STATE_IDLE;
					chan->echolastupdate = 0;
					chan->echotimer = 0;
					/* Make sure there's no gain */
					if (chan->gainalloc)
						kfree(chan->rxgain);
					chan->rxgain = defgain;
					chan->txgain = defgain;
					chan->gainalloc = 0;
					chan->flags &= ~ZT_FLAG_AUDIO;
					chan->flags |= (ZT_FLAG_PPP | ZT_FLAG_HDLC | ZT_FLAG_FCS);
					if (chan->span && chan->span->echocan)
						chan->span->echocan(chan, 0);
					if (tec)
						echo_can_free(tec);
				} else
					return -ENOMEM;
			}
		} else {
			chan->flags &= ~(ZT_FLAG_PPP | ZT_FLAG_HDLC | ZT_FLAG_FCS);
			if (chan->ppp) {
				struct ppp_channel *ppp = chan->ppp;
				chan->ppp = NULL;
				tasklet_kill(&chan->ppp_calls);
				skb_queue_purge(&chan->ppp_rq);
				ppp_unregister_channel(ppp);
				kfree(ppp);
			}
		}
#else
		printk("Zaptel: Zaptel PPP support not compiled in\n");
		return -ENOSYS;
#endif
		break;
	case ZT_HDLCRAWMODE:
		if (chan->sig != ZT_SIG_CLEAR)	return (-EINVAL);
		get_user(j, (int *)data);
		chan->flags &= ~(ZT_FLAG_AUDIO | ZT_FLAG_HDLC | ZT_FLAG_FCS);
		if (j) {
			chan->flags |= ZT_FLAG_HDLC;
			fasthdlc_init(&chan->rxhdlc);
			fasthdlc_init(&chan->txhdlc);
		}
		break;
	case ZT_HDLCFCSMODE:
		if (chan->sig != ZT_SIG_CLEAR)	return (-EINVAL);
		get_user(j, (int *)data);
		chan->flags &= ~(ZT_FLAG_AUDIO | ZT_FLAG_HDLC | ZT_FLAG_FCS);
		if (j) {
			chan->flags |= ZT_FLAG_HDLC | ZT_FLAG_FCS;
			fasthdlc_init(&chan->rxhdlc);
			fasthdlc_init(&chan->txhdlc);
		}
		break;
	case ZT_ECHOCANCEL:
		if (!(chan->flags & ZT_FLAG_AUDIO))
			return -EINVAL;
		get_user(j, (int *)data);
		if (j) {
			spin_lock_irqsave(&chan->lock, flags);
			/* If we had an old echo can, zap it now */
			tec = chan->ec;
			chan->ec = NULL;
			/* Attempt hardware native echo can */
			spin_unlock_irqrestore(&chan->lock, flags);

			if (chan->span && chan->span->echocan)
				ret = chan->span->echocan(chan, j);
			else
				ret = -ENOTTY;

			if (ret) {
				/* Use built-in echo can */
				if ((j == 32) ||
				    (j == 64) ||
				    (j == 128) ||
				    (j == 256) ||
				    (j == 512) ||
				    (j == 1024)) {
					/* Okay */
				} else {
					j = deftaps;
				}
				ec = echo_can_create(j, 0);
				if (!ec)
					return -ENOMEM;
				spin_lock_irqsave(&chan->lock, flags);
				chan->echocancel = j;
				chan->ec = ec;
				chan->echostate = ECHO_STATE_IDLE;
				chan->echolastupdate = 0;
				chan->echotimer = 0;
				echo_can_disable_detector_init(&chan->txecdis);
				echo_can_disable_detector_init(&chan->rxecdis);
				spin_unlock_irqrestore(&chan->lock, flags);
			}
			if (tec)
				echo_can_free(tec);
		} else {
			spin_lock_irqsave(&chan->lock, flags);
			tec = chan->ec;
			chan->echocancel = 0;
			chan->ec = NULL;
			chan->echostate = ECHO_STATE_IDLE;
			chan->echolastupdate = 0;
			chan->echotimer = 0;
			spin_unlock_irqrestore(&chan->lock, flags);
			/* Attempt hardware native echo can */
			if (chan->span && chan->span->echocan)
				chan->span->echocan(chan, 0);
			if (tec)
				echo_can_free(tec);
		}
		break;
	case ZT_ECHOTRAIN:
		get_user(j, (int *)data); /* get pre-training time from user */
		if ((j < 0) || (j >= ZT_MAX_PRETRAINING))
			return -EINVAL;
		j <<= 3;
		if (chan->ec) {
			/* Start pretraining stage */
			chan->echostate = ECHO_STATE_PRETRAINING;
			chan->echotimer = j;
		} else
			return -EINVAL;
		break;
	case ZT_SETTXBITS:
		if (chan->sig != ZT_SIG_CAS)
			return -EINVAL;
		get_user(j,(int *)data);
		zt_cas_setbits(chan, j);
		rv = 0;
		break;
	case ZT_GETRXBITS:
		put_user(chan->rxsig, (int *)data);
		rv = 0;
		break;
	case ZT_LOOPBACK:
		get_user(j, (int *)data);
		spin_lock_irqsave(&chan->lock, flags);
		if (j)
			chan->flags |= ZT_FLAG_LOOPED;
		else
			chan->flags &= ~ZT_FLAG_LOOPED;
		spin_unlock_irqrestore(&chan->lock, flags);
		rv = 0;
		break;
	case ZT_HOOK:
		get_user(j,(int *)data);
		if (chan->flags & ZT_FLAG_CLEAR)
			return -EINVAL;
		if (chan->sig == ZT_SIG_CAS) 
			return -EINVAL;
		/* if no span, just do nothing */
		if (!chan->span) return(0);
		spin_lock_irqsave(&chan->lock, flags);
		/* if dialing, stop it */
		chan->curtone = NULL;
		chan->dialing = 0;
		chan->txdialbuf[0] = '\0';
		chan->tonep = 0;
		chan->pdialcount = 0;
		spin_unlock_irqrestore(&chan->lock, flags);
		if (chan->span->flags & ZT_FLAG_RBS) {
			switch (j) {
			case ZT_ONHOOK:
				spin_lock_irqsave(&chan->lock, flags);
				zt_hangup(chan);
				spin_unlock_irqrestore(&chan->lock, flags);
				break;
			case ZT_OFFHOOK:
				spin_lock_irqsave(&chan->lock, flags);
				if ((chan->txstate == ZT_TXSTATE_KEWL) ||
				  (chan->txstate == ZT_TXSTATE_AFTERKEWL)) {
					spin_unlock_irqrestore(&chan->lock, flags);
					return -EBUSY;
				}
				zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_DEBOUNCE, chan->debouncetime);
				spin_unlock_irqrestore(&chan->lock, flags);
				break;
			case ZT_RING:
			case ZT_START:
				spin_lock_irqsave(&chan->lock, flags);
				if (chan->txstate != ZT_TXSTATE_ONHOOK) {
					spin_unlock_irqrestore(&chan->lock, flags);
					return -EBUSY;
				}
				if (chan->sig & __ZT_SIG_FXO) {
					ret = 0;
					chan->cadencepos = 0;
					ret = chan->ringcadence[0];
					zt_rbs_sethook(chan, ZT_TXSIG_START, ZT_TXSTATE_RINGON, ret);
				} else
					zt_rbs_sethook(chan, ZT_TXSIG_START, ZT_TXSTATE_START, chan->starttime);
				spin_unlock_irqrestore(&chan->lock, flags);
				if (file->f_flags & O_NONBLOCK)
					return -EINPROGRESS;
#if 0
				rv = schluffen(&chan->txstateq);
				if (rv) return rv;
#endif				
				rv = 0;
				break;
			case ZT_WINK:
				spin_lock_irqsave(&chan->lock, flags);
				if (chan->txstate != ZT_TXSTATE_ONHOOK) {
					spin_unlock_irqrestore(&chan->lock, flags);
					return -EBUSY;
				}
				zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_PREWINK, chan->prewinktime);
				spin_unlock_irqrestore(&chan->lock, flags);
				if (file->f_flags & O_NONBLOCK)
					return -EINPROGRESS;
				rv = schluffen(&chan->txstateq);
				if (rv) return rv;
				break;
			case ZT_FLASH:
				spin_lock_irqsave(&chan->lock, flags);
				if (chan->txstate != ZT_TXSTATE_OFFHOOK) {
					spin_unlock_irqrestore(&chan->lock, flags);
					return -EBUSY;
				}
				zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_PREFLASH, chan->preflashtime);
				spin_unlock_irqrestore(&chan->lock, flags);
				if (file->f_flags & O_NONBLOCK)
					return -EINPROGRESS;
				rv = schluffen(&chan->txstateq);
				if (rv) return rv;
				break;
			case ZT_RINGOFF:
				spin_lock_irqsave(&chan->lock, flags);
				zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_ONHOOK, 0);
				spin_unlock_irqrestore(&chan->lock, flags);
				break;
			default:
				return -EINVAL;
			}
		} else if (chan->span->sethook) {
			if (chan->txhooksig != j) {
				chan->txhooksig = j;
				chan->span->sethook(chan, j);
			}
		} else
			return -ENOSYS;
		break;
#ifdef CONFIG_ZAPATA_PPP
	case PPPIOCGCHAN:
		if (chan->flags & ZT_FLAG_PPP)
			return put_user(ppp_channel_index(chan->ppp), (int *)data) ? -EFAULT : 0;
		else
			return -EINVAL;
		break;
	case PPPIOCGUNIT:
		if (chan->flags & ZT_FLAG_PPP)
			return put_user(ppp_unit_number(chan->ppp), (int *)data) ? -EFAULT : 0;
		else
			return -EINVAL;
		break;
#endif
	default:
		return zt_chanandpseudo_ioctl(inode, file, cmd, data, unit);
	}
	return 0;
}

static int zt_prechan_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data, int unit)
{
	struct zt_chan *chan = file->private_data;
	int channo;
	int res;

	if (chan) {
		printk("Huh?  Prechan already has private data??\n");
	}
	switch(cmd) {
	case ZT_SPECIFY:
		get_user(channo,(int *)data);
		if (channo < 1)
			return -EINVAL;
		if (channo > ZT_MAX_CHANNELS)
			return -EINVAL;
		res = zt_specchan_open(inode, file, channo, 0);
		if (!res) {
			/* Setup the pointer for future stuff */
			chan = chans[channo];
			file->private_data = chan;
			/* Return success */
			return 0;
		}
		return res;
	default:
		return -ENOSYS;
	}
	return 0;
}

static int zt_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data)
{
	int unit = UNIT(file);
	struct zt_chan *chan;
	struct zt_timer *timer;

	if (!unit)
		return zt_ctl_ioctl(inode, file, cmd, data);

	if (unit == 250)
		return zt_transcode_fops->ioctl(inode, file, cmd, data);

	if (unit == 253) {
		timer = file->private_data;
		if (timer)
			return zt_timer_ioctl(inode, file, cmd, data, timer);
		else
			return -EINVAL;
	}
	if (unit == 254) {
		chan = file->private_data;
		if (chan)
			return zt_chan_ioctl(inode, file, cmd, data, chan->channo);
		else
			return zt_prechan_ioctl(inode, file, cmd, data, unit);
	}
	if (unit == 255) {
		chan = file->private_data;
		if (!chan) {
			printk("No pseudo channel structure to read?\n");
			return -EINVAL;
		}
		return zt_chanandpseudo_ioctl(inode, file, cmd, data, chan->channo);
	}
	return zt_chan_ioctl(inode, file, cmd, data, unit);
}

int zt_register(struct zt_span *span, int prefmaster)
{
	int x;

#ifdef CONFIG_PROC_FS
	char tempfile[17];
#endif
	if (!span)
		return -EINVAL;
	if (span->flags & ZT_FLAG_REGISTERED) {
		printk(KERN_ERR "Span %s already appears to be registered\n", span->name);
		return -EBUSY;
	}
	for (x=1;x<maxspans;x++)
		if (spans[x] == span) {
			printk(KERN_ERR "Span %s already in list\n", span->name);
			return -EBUSY;
		}
	for (x=1;x<ZT_MAX_SPANS;x++)
		if (!spans[x])
			break;
	if (x < ZT_MAX_SPANS) {
		spans[x] = span;
		if (maxspans < x + 1)
			maxspans = x + 1;
	} else {
		printk(KERN_ERR "Too many zapata spans registered\n");
		return -EBUSY;
	}
	span->flags |= ZT_FLAG_REGISTERED;
	span->spanno = x;
	spin_lock_init(&span->lock);
	if (!span->deflaw) {
		printk("zaptel: Span %s didn't specify default law.  Assuming mulaw, please fix driver!\n", span->name);
		span->deflaw = ZT_LAW_MULAW;
	}

	for (x=0;x<span->channels;x++) {
		span->chans[x].span = span;
		zt_chan_reg(&span->chans[x]); 
	}

#ifdef CONFIG_PROC_FS
			sprintf(tempfile, "zaptel/%d", span->spanno);
			proc_entries[span->spanno] = create_proc_read_entry(tempfile, 0444, NULL , zaptel_proc_read, (int *)(long)span->spanno);
#endif

#ifdef CONFIG_DEVFS_FS
	{
		char span_name[50];
		sprintf(span_name, "span%d", span->spanno);
		span->dhandle = devfs_mk_dir(zaptel_devfs_dir, span_name, NULL);
		for (x = 0; x < span->channels; x++) {
			struct zt_chan *chan = &span->chans[x];
			chan->fhandle = register_devfs_channel(chan, chan->span->dhandle); /* Register our stuff with devfs */
		}
	}
#endif /* CONFIG_DEVFS_FS */

#ifdef CONFIG_ZAP_UDEV
	for (x = 0; x < span->channels; x++) {
		char chan_name[50];
		if (span->chans[x].channo < 250) {
			sprintf(chan_name, "zap%d", span->chans[x].channo);
			CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, span->chans[x].channo), NULL, chan_name);
		}
	}
#endif /* CONFIG_ZAP_UDEV */

	if (debug)
		printk("Registered Span %d ('%s') with %d channels\n", span->spanno, span->name, span->channels);
	if (!master || prefmaster) {
		master = span;
		if (debug)
			printk("Span ('%s') is new master\n", span->name);
	}
	return 0;
}

int zt_unregister(struct zt_span *span)
{
	int x;
	int new_maxspans;
	static struct zt_span *new_master;

#ifdef CONFIG_PROC_FS
	char tempfile[17];
#endif /* CONFIG_PROC_FS */

	if (!(span->flags & ZT_FLAG_REGISTERED)) {
		printk(KERN_ERR "Span %s does not appear to be registered\n", span->name);
		return -1;
	}
	/* Shutdown the span if it's running */
	if (span->flags & ZT_FLAG_RUNNING)
		if (span->shutdown)
			span->shutdown(span);
			
	if (spans[span->spanno] != span) {
		printk(KERN_ERR "Span %s has spanno %d which is something else\n", span->name, span->spanno);
		return -1;
	}
	if (debug)
		printk("Unregistering Span '%s' with %d channels\n", span->name, span->channels);
#ifdef CONFIG_PROC_FS
	sprintf(tempfile, "zaptel/%d", span->spanno);
        remove_proc_entry(tempfile, NULL);
#endif /* CONFIG_PROC_FS */
#ifdef CONFIG_DEVFS_FS
	for (x = 0; x < span->channels; x++) {
		devfs_unregister(span->chans[x].fhandle);
		devfs_unregister(span->chans[x].fhandle_symlink);
	}
	devfs_unregister(span->dhandle);
#endif /* CONFIG_DEVFS_FS */

#ifdef CONFIG_ZAP_UDEV
	for (x = 0; x < span->channels; x++) {
		if (span->chans[x].channo < 250)
			class_device_destroy(zap_class, MKDEV(ZT_MAJOR, span->chans[x].channo));
	}
#endif /* CONFIG_ZAP_UDEV */

	spans[span->spanno] = NULL;
	span->spanno = 0;
	span->flags &= ~ZT_FLAG_REGISTERED;
	for (x=0;x<span->channels;x++)
		zt_chan_unreg(&span->chans[x]);
	new_maxspans = 0;
	new_master = master; /* FIXME: locking */
	if (master == span)
		new_master = NULL;
	for (x=1;x<ZT_MAX_SPANS;x++) {
		if (spans[x]) {
			new_maxspans = x+1;
			if (!new_master)
				new_master = spans[x];
		}
	}
	maxspans = new_maxspans;
	if (master != new_master)
		if (debug)
			printk("%s: Span ('%s') is new master\n", __FUNCTION__, 
				(new_master)? new_master->name: "no master");
	master = new_master;

	return 0;
}

/*
** This routine converts from linear to ulaw
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/

#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

#ifdef CONFIG_CALC_XLAW
unsigned char
#else
static unsigned char  __init
#endif
__zt_lineartoulaw(short sample)
{
  static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                             4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;          /* set aside the sign */
  if (sign != 0) sample = -sample;              /* get magnitude */
  if (sample > CLIP) sample = CLIP;             /* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[(sample >> 7) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
  if (ulawbyte == 0) ulawbyte = 0x02;   /* optional CCITT trap */
#endif
  if (ulawbyte == 0xff) ulawbyte = 0x7f;   /* never return 0xff */
  return(ulawbyte);
}

#define AMI_MASK 0x55

#ifdef CONFIG_CALC_XLAW
unsigned char
#else
static inline unsigned char __init
#endif
__zt_lineartoalaw (short linear)
{
    int mask;
    int seg;
    int pcm_val;
    static int seg_end[8] =
    {
         0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF
    };
    
    pcm_val = linear;
    if (pcm_val >= 0)
    {
        /* Sign (7th) bit = 1 */
        mask = AMI_MASK | 0x80;
    }
    else
    {
        /* Sign bit = 0 */
        mask = AMI_MASK;
        pcm_val = -pcm_val;
    }

    /* Convert the scaled magnitude to segment number. */
    for (seg = 0;  seg < 8;  seg++)
    {
        if (pcm_val <= seg_end[seg])
	    break;
    }
    /* Combine the sign, segment, and quantization bits. */
    return  ((seg << 4) | ((pcm_val >> ((seg)  ?  (seg + 3)  :  4)) & 0x0F)) ^ mask;
}
/*- End of function --------------------------------------------------------*/

static inline short int __init alaw2linear (uint8_t alaw)
{
    int i;
    int seg;

    alaw ^= AMI_MASK;
    i = ((alaw & 0x0F) << 4);
    seg = (((int) alaw & 0x70) >> 4);
    if (seg)
        i = (i + 0x100) << (seg - 1);
    return (short int) ((alaw & 0x80)  ?  i  :  -i);
}
/*- End of function --------------------------------------------------------*/
static void  __init zt_conv_init(void)
{
	int i;

	/* 
	 *  Set up mu-law conversion table
	 */
	for(i = 0;i < 256;i++)
	   {
		short mu,e,f,y;
		static short etab[]={0,132,396,924,1980,4092,8316,16764};

		mu = 255-i;
		e = (mu & 0x70)/16;
		f = mu & 0x0f;
		y = f * (1 << (e + 3));
		y += etab[e];
		if (mu & 0x80) y = -y;
	        __zt_mulaw[i] = y;
		__zt_alaw[i] = alaw2linear(i);
		/* Default (0.0 db) gain table */
		defgain[i] = i;
	   }
#ifndef CONFIG_CALC_XLAW
	  /* set up the reverse (mu-law) conversion table */
	for(i = -32768; i < 32768; i += 4)
	   {
		__zt_lin2mu[((unsigned short)(short)i) >> 2] = __zt_lineartoulaw(i);
		__zt_lin2a[((unsigned short)(short)i) >> 2] = __zt_lineartoalaw(i);
	   }
#endif
}

static inline void __zt_process_getaudio_chunk(struct zt_chan *ss, unsigned char *txb)
{
	/* We transmit data from our master channel */
	/* Called with ss->lock held */
	struct zt_chan *ms = ss->master;
	/* Linear representation */
	short getlin[ZT_CHUNKSIZE], k[ZT_CHUNKSIZE];
	int x;

	/* Okay, now we've got something to transmit */
	for (x=0;x<ZT_CHUNKSIZE;x++)
		getlin[x] = ZT_XLAW(txb[x], ms);
#ifndef NO_ECHOCAN_DISABLE
	if (ms->ec) {
		for (x=0;x<ZT_CHUNKSIZE;x++) {
			/* Check for echo cancel disabling tone */
			if (echo_can_disable_detector_update(&ms->txecdis, getlin[x])) {
				printk("zaptel Disabled echo canceller because of tone (tx) on channel %d\n", ss->channo);
				ms->echocancel = 0;
				ms->echostate = ECHO_STATE_IDLE;
				ms->echolastupdate = 0;
				ms->echotimer = 0;
				echo_can_free(ms->ec);
				ms->ec = NULL;
				__qevent(ss, ZT_EVENT_EC_DISABLED);
				break;
			}
		}
	}
#endif
	if ((!ms->confmute && !ms->dialing) || (ms->flags & ZT_FLAG_PSEUDO)) {
		/* Handle conferencing on non-clear channel and non-HDLC channels */
		switch(ms->confmode & ZT_CONF_MODE_MASK) {
		case ZT_CONF_NORMAL:
			/* Do nuffin */
			break;
		case ZT_CONF_MONITOR:	/* Monitor a channel's rx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO) break;
			/* Add monitored channel */
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				ACSS(getlin, chans[ms->confna]->getlin);
			} else {
				ACSS(getlin, chans[ms->confna]->putlin);
			}
			for (x=0;x<ZT_CHUNKSIZE;x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);
			break;
		case ZT_CONF_MONITORTX: /* Monitor a channel's tx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO) break;
			/* Add monitored channel */
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				ACSS(getlin, chans[ms->confna]->putlin);
			} else {
				ACSS(getlin, chans[ms->confna]->getlin);
			}

			for (x=0;x<ZT_CHUNKSIZE;x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);
			break;
		case ZT_CONF_MONITORBOTH: /* monitor a channel's rx and tx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO) break;
			ACSS(getlin, chans[ms->confna]->putlin);
			ACSS(getlin, chans[ms->confna]->getlin);
			for (x=0;x<ZT_CHUNKSIZE;x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);
			break;
		case ZT_CONF_MONITOR_RX_PREECHO:	/* Monitor a channel's rx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO)
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			/* Add monitored channel */
			ACSS(getlin, chans[ms->confna]->flags & ZT_FLAG_PSEUDO ?
			     chans[ms->confna]->readchunkpreec : chans[ms->confna]->putlin);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);

			break;
		case ZT_CONF_MONITOR_TX_PREECHO: /* Monitor a channel's tx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO)
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			/* Add monitored channel */
			ACSS(getlin, chans[ms->confna]->flags & ZT_FLAG_PSEUDO ?
			     chans[ms->confna]->putlin : chans[ms->confna]->readchunkpreec);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);

			break;
		case ZT_CONF_MONITORBOTH_PREECHO: /* monitor a channel's rx and tx mode */
			  /* if a pseudo-channel, ignore */
			if (ms->flags & ZT_FLAG_PSEUDO)
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			ACSS(getlin, chans[ms->confna]->putlin);
			ACSS(getlin, chans[ms->confna]->readchunkpreec);

			for (x = 0; x < ZT_CHUNKSIZE; x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);

			break;
		case ZT_CONF_REALANDPSEUDO:
			/* This strange mode takes the transmit buffer and
				puts it on the conference, minus its last sample,
				then outputs from the conference minus the 
				real channel's last sample. */
			  /* if to talk on conf */
			if (ms->confmode & ZT_CONF_PSEUDO_TALKER) {
				/* Store temp value */
				memcpy(k, getlin, ZT_CHUNKSIZE * sizeof(short));
				/* Add conf value */
				ACSS(k, conf_sums_next[ms->_confn]);
				/* save last one */
				memcpy(ms->conflast2, ms->conflast1, ZT_CHUNKSIZE * sizeof(short));
				memcpy(ms->conflast1, k, ZT_CHUNKSIZE * sizeof(short));
				/*  get amount actually added */
				SCSS(ms->conflast1, conf_sums_next[ms->_confn]);
				/* Really add in new value */
				ACSS(conf_sums_next[ms->_confn], ms->conflast1);
			} else {
				memset(ms->conflast1, 0, ZT_CHUNKSIZE * sizeof(short));
				memset(ms->conflast2, 0, ZT_CHUNKSIZE * sizeof(short));
			}
			memset(getlin, 0, ZT_CHUNKSIZE * sizeof(short));
			txb[0] = ZT_LIN2X(0, ms);
			memset(txb + 1, txb[0], ZT_CHUNKSIZE - 1);
			/* fall through to normal conf mode */
		case ZT_CONF_CONF:	/* Normal conference mode */
			if (ms->flags & ZT_FLAG_PSEUDO) /* if pseudo-channel */
			   {
				  /* if to talk on conf */
				if (ms->confmode & ZT_CONF_TALKER) {
					/* Store temp value */
					memcpy(k, getlin, ZT_CHUNKSIZE * sizeof(short));
					/* Add conf value */
					ACSS(k, conf_sums[ms->_confn]);
					/*  get amount actually added */
					memcpy(ms->conflast, k, ZT_CHUNKSIZE * sizeof(short));
					SCSS(ms->conflast, conf_sums[ms->_confn]);
					/* Really add in new value */
					ACSS(conf_sums[ms->_confn], ms->conflast);
				} else memset(ms->conflast, 0, ZT_CHUNKSIZE * sizeof(short));
				memcpy(getlin, ms->getlin, ZT_CHUNKSIZE * sizeof(short));
				txb[0] = ZT_LIN2X(0, ms);
				memset(txb + 1, txb[0], ZT_CHUNKSIZE - 1);
				break;
		 	   }
			/* fall through */
		case ZT_CONF_CONFMON:	/* Conference monitor mode */
			if (ms->confmode & ZT_CONF_LISTENER) {
				/* Subtract out last sample written to conf */
				SCSS(getlin, ms->conflast);
				/* Add in conference */
				ACSS(getlin, conf_sums[ms->_confn]);
			}
			for (x=0;x<ZT_CHUNKSIZE;x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);
			break;
		case ZT_CONF_CONFANN:
		case ZT_CONF_CONFANNMON:
			/* First, add tx buffer to conf */
			ACSS(conf_sums_next[ms->_confn], getlin);
			/* Start with silence */
			memset(getlin, 0, ZT_CHUNKSIZE * sizeof(short));
			/* If a listener on the conf... */
			if (ms->confmode & ZT_CONF_LISTENER) {
				/* Subtract last value written */
				SCSS(getlin, ms->conflast);
				/* Add in conf */
				ACSS(getlin, conf_sums[ms->_confn]);
			}
			for (x=0;x<ZT_CHUNKSIZE;x++)
				txb[x] = ZT_LIN2X(getlin[x], ms);
			break;
		case ZT_CONF_DIGITALMON:
			/* Real digital monitoring, but still echo cancel if desired */
			if (!chans[ms->confna])
				break;
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				if (ms->ec) {
					for (x=0;x<ZT_CHUNKSIZE;x++)
						txb[x] = ZT_LIN2X(chans[ms->confna]->getlin[x], ms);
				} else {
					memcpy(txb, chans[ms->confna]->getraw, ZT_CHUNKSIZE);
				}
			} else {
				if (ms->ec) {
					for (x=0;x<ZT_CHUNKSIZE;x++)
						txb[x] = ZT_LIN2X(chans[ms->confna]->putlin[x], ms);
				} else {
					memcpy(txb, chans[ms->confna]->putraw, ZT_CHUNKSIZE);
				}
			}
			for (x=0;x<ZT_CHUNKSIZE;x++)
				getlin[x] = ZT_XLAW(txb[x], ms);
			break;
		}
	}
	if (ms->confmute || (ms->echostate & __ECHO_STATE_MUTE)) {
		txb[0] = ZT_LIN2X(0, ms);
		memset(txb + 1, txb[0], ZT_CHUNKSIZE - 1);
		if (ms->echostate == ECHO_STATE_STARTTRAINING) {
			/* Transmit impulse now */
			txb[0] = ZT_LIN2X(16384, ms);
			ms->echostate = ECHO_STATE_AWAITINGECHO;
		}
	}
	/* save value from last chunk */
	memcpy(ms->getlin_lastchunk, ms->getlin, ZT_CHUNKSIZE * sizeof(short));
	/* save value from current */
	memcpy(ms->getlin, getlin, ZT_CHUNKSIZE * sizeof(short));
	/* save value from current */
	memcpy(ms->getraw, txb, ZT_CHUNKSIZE);
	/* if to make tx tone */
	if (ms->v1_1 || ms->v2_1 || ms->v3_1)
	{
		for (x=0;x<ZT_CHUNKSIZE;x++)
		{
			getlin[x] += zt_txtone_nextsample(ms);
			txb[x] = ZT_LIN2X(getlin[x], ms);
		}
	}
	/* This is what to send (after having applied gain) */
	for (x=0;x<ZT_CHUNKSIZE;x++)
		txb[x] = ms->txgain[txb[x]];
}

static inline void __zt_getbuf_chunk(struct zt_chan *ss, unsigned char *txb)
{
	/* Called with ss->lock held */
	/* We transmit data from our master channel */
	struct zt_chan *ms = ss->master;
	/* Buffer we're using */
	unsigned char *buf;
	/* Old buffer number */
	int oldbuf;
	/* Linear representation */
	int getlin;
	/* How many bytes we need to process */
	int bytes = ZT_CHUNKSIZE, left;
	int x;

	/* Let's pick something to transmit.  First source to
	   try is our write-out buffer.  Always check it first because
	   its our 'fast path' for whatever that's worth. */
	while(bytes) {
		if ((ms->outwritebuf > -1) && !ms->txdisable) {
			buf= ms->writebuf[ms->outwritebuf];
			left = ms->writen[ms->outwritebuf] - ms->writeidx[ms->outwritebuf];
			if (left > bytes)
				left = bytes;
			if (ms->flags & ZT_FLAG_HDLC) {
				/* If this is an HDLC channel we only send a byte of
				   HDLC. */
				for(x=0;x<left;x++) {
					if (ms->txhdlc.bits < 8)
						/* Load a byte of data only if needed */
						fasthdlc_tx_load_nocheck(&ms->txhdlc, buf[ms->writeidx[ms->outwritebuf]++]);
					*(txb++) = fasthdlc_tx_run_nocheck(&ms->txhdlc);
				}
				bytes -= left;
			} else {
				memcpy(txb, buf + ms->writeidx[ms->outwritebuf], left);
				ms->writeidx[ms->outwritebuf]+=left;
				txb += left;
				bytes -= left;
			}
			/* Check buffer status */
			if (ms->writeidx[ms->outwritebuf] >= ms->writen[ms->outwritebuf]) {
				/* We've reached the end of our buffer.  Go to the next. */
				oldbuf = ms->outwritebuf;
				/* Clear out write index and such */
				ms->writeidx[oldbuf] = 0;
				ms->writen[oldbuf] = 0;
				ms->outwritebuf = (ms->outwritebuf + 1) % ms->numbufs;
				if (ms->outwritebuf == ms->inwritebuf) {
					/* Whoopsies, we're run out of buffers.  Mark ours
					as -1 and wait for the filler to notify us that
					there is something to write */
					ms->outwritebuf = -1;
					if (ms->iomask & (ZT_IOMUX_WRITE | ZT_IOMUX_WRITEEMPTY))
						wake_up_interruptible(&ms->eventbufq);
					/* If we're only supposed to start when full, disable the transmitter */
					if (ms->txbufpolicy == ZT_POLICY_WHEN_FULL)
						ms->txdisable = 1;
				}
				if (ms->inwritebuf < 0) {
					/* The filler doesn't have a place to put data.  Now
					that we're done with this buffer, notify them. */
					ms->inwritebuf = oldbuf;
				}
/* In the very orignal driver, it was quite well known to me (Jim) that there
was a possibility that a channel sleeping on a write block needed to
be potentially woken up EVERY time a buffer was emptied, not just on the first
one, because if only done on the first one there is a slight timing potential
of missing the wakeup (between where it senses the (lack of) active condition
(with interrupts disabled) and where it does the sleep (interrupts enabled)
in the read or iomux call, etc). That is why the write and iomux calls start
with an infinite loop that gets broken out of upon an active condition,
otherwise keeps sleeping and looking. The part in this code got "optimized"
out in the later versions, and is put back now. */
				if (!(ms->flags & (ZT_FLAG_NETDEV | ZT_FLAG_PPP))) {
					wake_up_interruptible(&ms->writebufq);
					wake_up_interruptible(&ms->sel);
					if (ms->iomask & ZT_IOMUX_WRITE)
						wake_up_interruptible(&ms->eventbufq);
				}
				/* Transmit a flag if this is an HDLC channel */
				if (ms->flags & ZT_FLAG_HDLC)
					fasthdlc_tx_frame_nocheck(&ms->txhdlc);
#ifdef CONFIG_ZAPATA_NET
				if (ms->flags & ZT_FLAG_NETDEV)
					netif_wake_queue(ztchan_to_dev(ms));
#endif				
#ifdef CONFIG_ZAPATA_PPP
				if (ms->flags & ZT_FLAG_PPP) {
					ms->do_ppp_wakeup = 1;
					tasklet_schedule(&ms->ppp_calls);
				}
#endif
			}
		} else if (ms->curtone && !(ms->flags & ZT_FLAG_PSEUDO)) {
			left = ms->curtone->tonesamples - ms->tonep;
			if (left > bytes)
				left = bytes;
			for (x=0;x<left;x++) {
				/* Pick our default value from the next sample of the current tone */
				getlin = zt_tone_nextsample(&ms->ts, ms->curtone);
				*(txb++) = ZT_LIN2X(getlin, ms);
			}
			ms->tonep+=left;
			bytes -= left;
			if (ms->tonep >= ms->curtone->tonesamples) {
				struct zt_tone *last;
				/* Go to the next sample of the tone */
				ms->tonep = 0;
				last = ms->curtone;
				ms->curtone = ms->curtone->next;
				if (!ms->curtone) {
					/* No more tones...  Is this dtmf or mf?  If so, go to the next digit */
					if (ms->dialing)
						__do_dtmf(ms);
				} else {
					if (last != ms->curtone)
						zt_init_tone_state(&ms->ts, ms->curtone);
				}
			}
		} else if (ms->flags & ZT_FLAG_LOOPED) {
			for (x = 0; x < bytes; x++)
				txb[x] = ms->readchunk[x];
			bytes = 0;
		} else if (ms->flags & ZT_FLAG_HDLC) {
			for (x=0;x<bytes;x++) {
				/* Okay, if we're HDLC, then transmit a flag by default */
				if (ms->txhdlc.bits < 8) 
					fasthdlc_tx_frame_nocheck(&ms->txhdlc);
				*(txb++) = fasthdlc_tx_run_nocheck(&ms->txhdlc);
			}
			bytes = 0;
		} else if (ms->flags & ZT_FLAG_CLEAR) {
			/* Clear channels that are idle in audio mode need
			   to send silence; in non-audio mode, always send 0xff
			   so stupid switches won't consider the channel active
			*/
			if (ms->flags & ZT_FLAG_AUDIO) {
				memset(txb, ZT_LIN2X(0, ms), bytes);
			} else {
				memset(txb, 0xFF, bytes);
			}
			bytes = 0;
		} else {
			memset(txb, ZT_LIN2X(0, ms), bytes);	/* Lastly we use silence on telephony channels */
			bytes = 0;
		}
	}	
}

static inline void rbs_itimer_expire(struct zt_chan *chan)
{
	/* the only way this could have gotten here, is if a channel
	    went onf hook longer then the wink or flash detect timeout */
	/* Called with chan->lock held */
	switch(chan->sig)
	{
	    case ZT_SIG_FXOLS:  /* if FXO, its definitely on hook */
	    case ZT_SIG_FXOGS:
	    case ZT_SIG_FXOKS:
		__qevent(chan,ZT_EVENT_ONHOOK);
		chan->gotgs = 0; 
		break;
#if defined(EMFLASH) || defined(EMPULSE)
	    case ZT_SIG_EM:
	    case ZT_SIG_EM_E1:
		if (chan->rxhooksig == ZT_RXSIG_ONHOOK) {
			__qevent(chan,ZT_EVENT_ONHOOK); 
			break;
		}
		__qevent(chan,ZT_EVENT_RINGOFFHOOK); 
		break;
#endif
#ifdef	FXSFLASH
	    case ZT_SIG_FXSKS:
		if (chan->rxhooksig == ZT_RXSIG_ONHOOK) {
			__qevent(chan, ZT_EVENT_ONHOOK); 
			break;
		}
#endif
		/* fall thru intentionally */
	    default:  /* otherwise, its definitely off hook */
		__qevent(chan,ZT_EVENT_RINGOFFHOOK); 
		break;
	}
}

static inline void __rbs_otimer_expire(struct zt_chan *chan)
{
	int len = 0;
	/* Called with chan->lock held */

	chan->otimer = 0;
	/* Move to the next timer state */	
	switch(chan->txstate) {
	case ZT_TXSTATE_RINGOFF:
		/* Turn on the ringer now that the silent time has passed */
		++chan->cadencepos;
		if (chan->cadencepos >= ZT_MAX_CADENCE)
			chan->cadencepos = chan->firstcadencepos;
		len = chan->ringcadence[chan->cadencepos];

		if (!len) {
			chan->cadencepos = chan->firstcadencepos;
			len = chan->ringcadence[chan->cadencepos];
		}

		zt_rbs_sethook(chan, ZT_TXSIG_START, ZT_TXSTATE_RINGON, len);
		__qevent(chan, ZT_EVENT_RINGERON);
		break;
		
	case ZT_TXSTATE_RINGON:
		/* Turn off the ringer now that the loud time has passed */
		++chan->cadencepos;
		if (chan->cadencepos >= ZT_MAX_CADENCE)
			chan->cadencepos = 0;
		len = chan->ringcadence[chan->cadencepos];

		if (!len) {
			chan->cadencepos = 0;
			len = chan->curzone->ringcadence[chan->cadencepos];
		}

		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_RINGOFF, len);
		__qevent(chan, ZT_EVENT_RINGEROFF);
		break;
		
	case ZT_TXSTATE_START:
		/* If we were starting, go off hook now ready to debounce */
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_AFTERSTART, ZT_AFTERSTART_TIME);
		wake_up_interruptible(&chan->txstateq);
		break;
		
	case ZT_TXSTATE_PREWINK:
		/* Actually wink */
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_WINK, chan->winktime);
		break;
		
	case ZT_TXSTATE_WINK:
		/* Wink complete, go on hook and stabalize */
		zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_ONHOOK, 0);
		if (chan->file && (chan->file->f_flags & O_NONBLOCK))
			__qevent(chan, ZT_EVENT_HOOKCOMPLETE);
		wake_up_interruptible(&chan->txstateq);
		break;
		
	case ZT_TXSTATE_PREFLASH:
		/* Actually flash */
		zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_FLASH, chan->flashtime);
		break;

	case ZT_TXSTATE_FLASH:
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_OFFHOOK, 0);
		if (chan->file && (chan->file->f_flags & O_NONBLOCK))
			__qevent(chan, ZT_EVENT_HOOKCOMPLETE);
		wake_up_interruptible(&chan->txstateq);
		break;
	
	case ZT_TXSTATE_DEBOUNCE:
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_OFFHOOK, 0);
		/* See if we've gone back on hook */
		if ((chan->rxhooksig == ZT_RXSIG_ONHOOK) && (chan->rxflashtime > 2))
			chan->itimerset = chan->itimer = chan->rxflashtime * ZT_CHUNKSIZE;
		wake_up_interruptible(&chan->txstateq);
		break;
		
	case ZT_TXSTATE_AFTERSTART:
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_OFFHOOK, 0);
		if (chan->file && (chan->file->f_flags & O_NONBLOCK))
			__qevent(chan, ZT_EVENT_HOOKCOMPLETE);
		wake_up_interruptible(&chan->txstateq);
		break;

	case ZT_TXSTATE_KEWL:
		zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, ZT_TXSTATE_AFTERKEWL, ZT_AFTERKEWLTIME);
		if (chan->file && (chan->file->f_flags & O_NONBLOCK))
			__qevent(chan, ZT_EVENT_HOOKCOMPLETE);
		wake_up_interruptible(&chan->txstateq);
		break;

	case ZT_TXSTATE_AFTERKEWL:
		if (chan->kewlonhook)  {
			__qevent(chan,ZT_EVENT_ONHOOK);
		}
		chan->txstate = ZT_TXSTATE_ONHOOK;
		chan->gotgs = 0;
		break;

	case ZT_TXSTATE_PULSEBREAK:
		zt_rbs_sethook(chan, ZT_TXSIG_OFFHOOK, ZT_TXSTATE_PULSEMAKE, 
			chan->pulsemaketime);
		wake_up_interruptible(&chan->txstateq);
		break;

	case ZT_TXSTATE_PULSEMAKE:
		if (chan->pdialcount)
			chan->pdialcount--;
		if (chan->pdialcount)
		{
			zt_rbs_sethook(chan, ZT_TXSIG_ONHOOK, 
				ZT_TXSTATE_PULSEBREAK, chan->pulsebreaktime);
			break;
		}
		chan->txstate = ZT_TXSTATE_PULSEAFTER;
		chan->otimer = chan->pulseaftertime * ZT_CHUNKSIZE;
		wake_up_interruptible(&chan->txstateq);
		break;

	case ZT_TXSTATE_PULSEAFTER:
		chan->txstate = ZT_TXSTATE_OFFHOOK;
		__do_dtmf(chan);
		wake_up_interruptible(&chan->txstateq);
		break;

	default:
		break;
	}
}

static void __zt_hooksig_pvt(struct zt_chan *chan, zt_rxsig_t rxsig)
{

	/* State machines for receive hookstate transitions 
		called with chan->lock held */

	if ((chan->rxhooksig) == rxsig) return;
	
	if ((chan->flags & ZT_FLAG_SIGFREEZE)) return;

	chan->rxhooksig = rxsig;
#ifdef	RINGBEGIN
	if ((chan->sig & __ZT_SIG_FXS) && (rxsig == ZT_RXSIG_RING) &&
	    (!chan->ringdebtimer))
		__qevent(chan,ZT_EVENT_RINGBEGIN);  
#endif
	switch(chan->sig) {
	    case ZT_SIG_EM:  /* E and M */
	    case ZT_SIG_EM_E1:
		switch(rxsig) {
		    case ZT_RXSIG_OFFHOOK: /* went off hook */
			/* The interface is going off hook */
#ifdef	EMFLASH
			if (chan->itimer)
			{
				__qevent(chan,ZT_EVENT_WINKFLASH); 
				chan->itimerset = chan->itimer = 0;
				break;				
			}
#endif
#ifdef EMPULSE
			if (chan->itimer) /* if timer still running */
			{
			    int plen = chan->itimerset - chan->itimer;
			    if (plen <= ZT_MAXPULSETIME)
			    {
					if (plen >= ZT_MINPULSETIME)
					{
						chan->pulsecount++;

						chan->pulsetimer = ZT_PULSETIMEOUT;
                                                chan->itimerset = chan->itimer = 0;
						if (chan->pulsecount == 1)
							__qevent(chan,ZT_EVENT_PULSE_START); 
					} 
			    } 
			    break;
			}
#endif
			/* set wink timer */
			chan->itimerset = chan->itimer = chan->rxwinktime * ZT_CHUNKSIZE;
			break;
		    case ZT_RXSIG_ONHOOK: /* went on hook */
			/* This interface is now going on hook.
			   Check for WINK, etc */
			if (chan->itimer)
				__qevent(chan,ZT_EVENT_WINKFLASH); 
#if defined(EMFLASH) || defined(EMPULSE)
			else {
#ifdef EMFLASH
				chan->itimerset = chan->itimer = chan->rxflashtime * ZT_CHUNKSIZE;

#else /* EMFLASH */
				chan->itimerset = chan->itimer = chan->rxwinktime * ZT_CHUNKSIZE;

#endif /* EMFLASH */
				chan->gotgs = 0;
				break;				
			}
#else /* EMFLASH || EMPULSE */
			else {
				__qevent(chan,ZT_EVENT_ONHOOK); 
				chan->gotgs = 0;
			}
#endif
			chan->itimerset = chan->itimer = 0;
			break;
		    default:
			break;
		}
		break;
	   case ZT_SIG_FXSKS:  /* FXS Kewlstart */
		  /* ignore a bit poopy if loop not closed and stable */
		if (chan->txstate != ZT_TXSTATE_OFFHOOK) break;
#ifdef	FXSFLASH
		if (rxsig == ZT_RXSIG_ONHOOK) {
			chan->itimer = ZT_FXSFLASHMAXTIME * ZT_CHUNKSIZE;
			break;
		} else 	if (rxsig == ZT_RXSIG_OFFHOOK) {
			if (chan->itimer) {
				/* did the offhook occur in the window? if not, ignore both events */
				if (chan->itimer <= ((ZT_FXSFLASHMAXTIME - ZT_FXSFLASHMINTIME) * ZT_CHUNKSIZE))
					__qevent(chan, ZT_EVENT_WINKFLASH);
			}
			chan->itimer = 0;
			break;
		}
#endif
		/* fall through intentionally */
	   case ZT_SIG_FXSGS:  /* FXS Groundstart */
		if (rxsig == ZT_RXSIG_ONHOOK) {
			chan->ringdebtimer = RING_DEBOUNCE_TIME;
			chan->ringtrailer = 0;
			if (chan->txstate != ZT_TXSTATE_DEBOUNCE) {
				chan->gotgs = 0;
				__qevent(chan,ZT_EVENT_ONHOOK);
			}
		}
		break;
	   case ZT_SIG_FXOGS: /* FXO Groundstart */
		if (rxsig == ZT_RXSIG_START) {
			  /* if havent got gs, report it */
			if (!chan->gotgs) {
				__qevent(chan,ZT_EVENT_RINGOFFHOOK);
				chan->gotgs = 1;
			}
		}
		/* fall through intentionally */
	   case ZT_SIG_FXOLS: /* FXO Loopstart */
	   case ZT_SIG_FXOKS: /* FXO Kewlstart */
		switch(rxsig) {
		    case ZT_RXSIG_OFFHOOK: /* went off hook */
			  /* if asserti ng ring, stop it */
			if (chan->txstate == ZT_TXSTATE_START) {
				zt_rbs_sethook(chan,ZT_TXSIG_OFFHOOK, ZT_TXSTATE_AFTERSTART, ZT_AFTERSTART_TIME);
			}
			chan->kewlonhook = 0;
#ifdef CONFIG_ZAPATA_DEBUG
			printk("Off hook on channel %d, itimer = %d, gotgs = %d\n", chan->channo, chan->itimer, chan->gotgs);
#endif
			if (chan->itimer) /* if timer still running */
			{
			    int plen = chan->itimerset - chan->itimer;
			    if (plen <= ZT_MAXPULSETIME)
			    {
					if (plen >= ZT_MINPULSETIME)
					{
						chan->pulsecount++;
						chan->pulsetimer = ZT_PULSETIMEOUT;
						chan->itimer = chan->itimerset;
						if (chan->pulsecount == 1)
							__qevent(chan,ZT_EVENT_PULSE_START); 
					} 
			    } else 
					__qevent(chan,ZT_EVENT_WINKFLASH); 
			} else {
				  /* if havent got GS detect */
				if (!chan->gotgs) {
					__qevent(chan,ZT_EVENT_RINGOFFHOOK); 
					chan->gotgs = 1;
					chan->itimerset = chan->itimer = 0;
				}
			}
			chan->itimerset = chan->itimer = 0;
			break;
		    case ZT_RXSIG_ONHOOK: /* went on hook */
			  /* if not during offhook debounce time */
			if ((chan->txstate != ZT_TXSTATE_DEBOUNCE) &&
			    (chan->txstate != ZT_TXSTATE_KEWL) && 
			    (chan->txstate != ZT_TXSTATE_AFTERKEWL)) {
				chan->itimerset = chan->itimer = chan->rxflashtime * ZT_CHUNKSIZE;
			}
			if (chan->txstate == ZT_TXSTATE_KEWL)
				chan->kewlonhook = 1;
			break;
		    default:
			break;
		}
	    default:
		break;
	}
}

void zt_hooksig(struct zt_chan *chan, zt_rxsig_t rxsig)
{
	  /* skip if no change */
	unsigned long flags;
	spin_lock_irqsave(&chan->lock, flags);
	__zt_hooksig_pvt(chan,rxsig);
	spin_unlock_irqrestore(&chan->lock, flags);
}

void zt_rbsbits(struct zt_chan *chan, int cursig)
{
	unsigned long flags;
	if (cursig == chan->rxsig)
		return;

	if ((chan->flags & ZT_FLAG_SIGFREEZE)) return;

	spin_lock_irqsave(&chan->lock, flags);
	switch(chan->sig) {
	    case ZT_SIG_FXOGS: /* FXO Groundstart */
		/* B-bit only matters for FXO GS */
		if (!(cursig & ZT_BBIT)) {
			__zt_hooksig_pvt(chan, ZT_RXSIG_START);
			break;
		}
		/* Fall through */
	    case ZT_SIG_EM:  /* E and M */
	    case ZT_SIG_EM_E1:
	    case ZT_SIG_FXOLS: /* FXO Loopstart */
	    case ZT_SIG_FXOKS: /* FXO Kewlstart */
		if (cursig & ZT_ABIT)  /* off hook */
			__zt_hooksig_pvt(chan,ZT_RXSIG_OFFHOOK);
		else /* on hook */
			__zt_hooksig_pvt(chan,ZT_RXSIG_ONHOOK);
		break;

	   case ZT_SIG_FXSKS:  /* FXS Kewlstart */
	   case ZT_SIG_FXSGS:  /* FXS Groundstart */
		/* Fall through */
	   case ZT_SIG_FXSLS:
		if (!(cursig & ZT_BBIT)) {
			/* Check for ringing first */
			__zt_hooksig_pvt(chan, ZT_RXSIG_RING);
			break;
		}
		if ((chan->sig != ZT_SIG_FXSLS) && (cursig & ZT_ABIT)) { 
			    /* if went on hook */
			__zt_hooksig_pvt(chan, ZT_RXSIG_ONHOOK);
		} else {
			__zt_hooksig_pvt(chan, ZT_RXSIG_OFFHOOK);
		}
		break;
	   case ZT_SIG_CAS:
		/* send event that something changed */
		__qevent(chan, ZT_EVENT_BITSCHANGED);
		break;

	   default:
		break;
	}
	/* Keep track of signalling for next time */
	chan->rxsig = cursig;
	spin_unlock_irqrestore(&chan->lock, flags);
}

static inline void __zt_ec_chunk(struct zt_chan *ss, unsigned char *rxchunk, const unsigned char *txchunk)
{
	short rxlin, txlin;
	int x;
	unsigned long flags;

	spin_lock_irqsave(&ss->lock, flags);

	if (ss->readchunkpreec) {
		/* Save a copy of the audio before the echo can has its way with it */
		for (x = 0; x < ZT_CHUNKSIZE; x++)
			/* We only ever really need to deal with signed linear - let's just convert it now */
			ss->readchunkpreec[x] = ZT_XLAW(rxchunk[x], ss);
	}

	/* Perform echo cancellation on a chunk if necessary */
	if (ss->ec) {
#if defined(CONFIG_ZAPTEL_MMX) || defined(ECHO_CAN_FP)
		zt_kernel_fpu_begin();
#endif		
		if (ss->echostate & __ECHO_STATE_MUTE) {
			/* Special stuff for training the echo can */
			for (x=0;x<ZT_CHUNKSIZE;x++) {
				rxlin = ZT_XLAW(rxchunk[x], ss);
				txlin = ZT_XLAW(txchunk[x], ss);
				if (ss->echostate == ECHO_STATE_PRETRAINING) {
					if (--ss->echotimer <= 0) {
						ss->echotimer = 0;
						ss->echostate = ECHO_STATE_STARTTRAINING;
					}
				}
				if ((ss->echostate == ECHO_STATE_AWAITINGECHO) && (txlin > 8000)) {
					ss->echolastupdate = 0;
					ss->echostate = ECHO_STATE_TRAINING;
				}
				if (ss->echostate == ECHO_STATE_TRAINING) {
					if (echo_can_traintap(ss->ec, ss->echolastupdate++, rxlin)) {
#if 0
						printk("Finished training (%d taps trained)!\n", ss->echolastupdate);
#endif						
						ss->echostate = ECHO_STATE_ACTIVE;
					}
				}
				rxlin = 0;
				rxchunk[x] = ZT_LIN2X((int)rxlin, ss);
			}
		} else {
#if !defined(ZT_EC_ARRAY_UPDATE)
			for (x=0;x<ZT_CHUNKSIZE;x++) {
				rxlin = ZT_XLAW(rxchunk[x], ss);
				rxlin = echo_can_update(ss->ec, ZT_XLAW(txchunk[x], ss), rxlin);
				rxchunk[x] = ZT_LIN2X((int) rxlin, ss);
			}
#else /* defined(ZT_EC_ARRAY_UPDATE) */
			short rxlins[ZT_CHUNKSIZE], txlins[ZT_CHUNKSIZE];
			for (x = 0; x < ZT_CHUNKSIZE; x++) {
				rxlins[x] = ZT_XLAW(rxchunk[x], ss);
				txlins[x] = ZT_XLAW(txchunk[x], ss);
			}
			echo_can_array_update(ss->ec, rxlins, txlins);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				rxchunk[x] = ZT_LIN2X((int) rxlins[x], ss);
#endif /* defined(ZT_EC_ARRAY_UPDATE) */
		}
#if defined(CONFIG_ZAPTEL_MMX) || defined(ECHO_CAN_FP)
		kernel_fpu_end();
#endif		
	}
	spin_unlock_irqrestore(&ss->lock, flags);
}

void zt_ec_chunk(struct zt_chan *ss, unsigned char *rxchunk, const unsigned char *txchunk)
{
	__zt_ec_chunk(ss, rxchunk, txchunk);
}

void zt_ec_span(struct zt_span *span)
{
	int x;
	for (x = 0; x < span->channels; x++) {
		if (span->chans[x].ec)
			__zt_ec_chunk(&span->chans[x], span->chans[x].readchunk, span->chans[x].writechunk);
	}
}

/* return 0 if nothing detected, 1 if lack of tone, 2 if presence of tone */
/* modifies buffer pointed to by 'amp' with notched-out values */
static inline int sf_detect (sf_detect_state_t *s,
                 short *amp,
                 int samples,long p1, long p2, long p3)
{
int     i,rv = 0;
long x,y;

#define	SF_DETECT_SAMPLES (ZT_CHUNKSIZE * 5)
#define	SF_DETECT_MIN_ENERGY 500
#define	NB 14  /* number of bits to shift left */
         
        /* determine energy level before filtering */
        for(i = 0; i < samples; i++)
        {
                if (amp[i] < 0) s->e1 -= amp[i];
                else s->e1 += amp[i];
        }
	/* do 2nd order IIR notch filter at given freq. and calculate
	    energy */
        for(i = 0; i < samples; i++)
        {
                x = amp[i] << NB;
                y = s->x2 + (p1 * (s->x1 >> NB)) + x;
                y += (p2 * (s->y2 >> NB)) + 
			(p3 * (s->y1 >> NB));
                s->x2 = s->x1;
                s->x1 = x;
                s->y2 = s->y1;
                s->y1 = y;
                amp[i] = y >> NB;
                if (amp[i] < 0) s->e2 -= amp[i];
                else s->e2 += amp[i];
        }
	s->samps += i;
	/* if time to do determination */
	if ((s->samps) >= SF_DETECT_SAMPLES)
	{
		rv = 1; /* default to no tone */
		/* if enough energy, it is determined to be a tone */
		if (((s->e1 - s->e2) / s->samps) > SF_DETECT_MIN_ENERGY) rv = 2;
		/* reset energy processing variables */
		s->samps = 0;
		s->e1 = s->e2 = 0;
	}
	return(rv);		
}

static inline void __zt_process_putaudio_chunk(struct zt_chan *ss, unsigned char *rxb)
{
	/* We transmit data from our master channel */
	/* Called with ss->lock held */
	struct zt_chan *ms = ss->master;
	/* Linear version of received data */
	short putlin[ZT_CHUNKSIZE],k[ZT_CHUNKSIZE];
	int x,r;

	if (ms->dialing) ms->afterdialingtimer = 50;
	else if (ms->afterdialingtimer) ms->afterdialingtimer--;
	if (ms->afterdialingtimer && (!(ms->flags & ZT_FLAG_PSEUDO))) {
		/* Be careful since memset is likely a macro */
		rxb[0] = ZT_LIN2X(0, ms);
		memset(&rxb[1], rxb[0], ZT_CHUNKSIZE - 1);  /* receive as silence if dialing */
	} 
	for (x=0;x<ZT_CHUNKSIZE;x++) {
		rxb[x] = ms->rxgain[rxb[x]];
		putlin[x] = ZT_XLAW(rxb[x], ms);
	}

#ifndef NO_ECHOCAN_DISABLE
	if (ms->ec) {
		for (x=0;x<ZT_CHUNKSIZE;x++) {
			if (echo_can_disable_detector_update(&ms->rxecdis, putlin[x])) {
				printk("zaptel Disabled echo canceller because of tone (rx) on channel %d\n", ss->channo);
				ms->echocancel = 0;
				ms->echostate = ECHO_STATE_IDLE;
				ms->echolastupdate = 0;
				ms->echotimer = 0;
				echo_can_free(ms->ec);
				ms->ec = NULL;
				break;
			}
		}
	}
#endif	
	/* if doing rx tone decoding */
	if (ms->rxp1 && ms->rxp2 && ms->rxp3)
	{
		r = sf_detect(&ms->rd,putlin,ZT_CHUNKSIZE,ms->rxp1,
			ms->rxp2,ms->rxp3);
		/* Convert back */
		for(x=0;x<ZT_CHUNKSIZE;x++)
			rxb[x] = ZT_LIN2X(putlin[x], ms);
		if (r) /* if something happened */
		{
			if (r != ms->rd.lastdetect)
			{
				if (((r == 2) && !(ms->toneflags & ZT_REVERSE_RXTONE)) ||
				    ((r == 1) && (ms->toneflags & ZT_REVERSE_RXTONE)))
				{
					__qevent(ms,ZT_EVENT_RINGOFFHOOK);
				}
				else
				{
					__qevent(ms,ZT_EVENT_ONHOOK);
				}
				ms->rd.lastdetect = r;
			}
		}
	}		

	if (!(ms->flags &  ZT_FLAG_PSEUDO)) {
		memcpy(ms->putlin, putlin, ZT_CHUNKSIZE * sizeof(short));
		memcpy(ms->putraw, rxb, ZT_CHUNKSIZE);
	}
	
	/* Take the rxc, twiddle it for conferencing if appropriate and put it
	   back */
	if ((!ms->confmute && !ms->afterdialingtimer) ||
	    (ms->flags & ZT_FLAG_PSEUDO)) {
		switch(ms->confmode & ZT_CONF_MODE_MASK) {
		case ZT_CONF_NORMAL:		/* Normal mode */
			/* Do nothing.  rx goes output */
			break;
		case ZT_CONF_MONITOR:		/* Monitor a channel's rx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO)) break;
			/* Add monitored channel */
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				ACSS(putlin, chans[ms->confna]->getlin);
			} else {
				ACSS(putlin, chans[ms->confna]->putlin);
			}
			/* Convert back */
			for(x=0;x<ZT_CHUNKSIZE;x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);
			break;
		case ZT_CONF_MONITORTX:	/* Monitor a channel's tx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO)) break;
			/* Add monitored channel */
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				ACSS(putlin, chans[ms->confna]->putlin);
			} else {
				ACSS(putlin, chans[ms->confna]->getlin);
			}
			/* Convert back */
			for(x=0;x<ZT_CHUNKSIZE;x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);
			break;
		case ZT_CONF_MONITORBOTH:	/* Monitor a channel's tx and rx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO)) break;
			/* Note: Technically, saturation should be done at 
			   the end of the whole addition, but for performance
			   reasons, we don't do that.  Besides, it only matters
			   when you're so loud you're clipping anyway */
			ACSS(putlin, chans[ms->confna]->getlin);
			ACSS(putlin, chans[ms->confna]->putlin);
			/* Convert back */
			for(x=0;x<ZT_CHUNKSIZE;x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);
			break;
		case ZT_CONF_MONITOR_RX_PREECHO:		/* Monitor a channel's rx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO))
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			/* Add monitored channel */
			ACSS(putlin, chans[ms->confna]->flags & ZT_FLAG_PSEUDO ?
			     chans[ms->confna]->getlin : chans[ms->confna]->readchunkpreec);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);

			break;
		case ZT_CONF_MONITOR_TX_PREECHO:	/* Monitor a channel's tx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO))
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			/* Add monitored channel */
			ACSS(putlin, chans[ms->confna]->flags & ZT_FLAG_PSEUDO ?
			     chans[ms->confna]->readchunkpreec : chans[ms->confna]->getlin);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);

			break;
		case ZT_CONF_MONITORBOTH_PREECHO:	/* Monitor a channel's tx and rx mode */
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO))
				break;

			if (!chans[ms->confna]->readchunkpreec)
				break;

			/* Note: Technically, saturation should be done at 
			   the end of the whole addition, but for performance
			   reasons, we don't do that.  Besides, it only matters
			   when you're so loud you're clipping anyway */
			ACSS(putlin, chans[ms->confna]->getlin);
			ACSS(putlin, chans[ms->confna]->readchunkpreec);
			for (x = 0; x < ZT_CHUNKSIZE; x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);

			break;
		case ZT_CONF_REALANDPSEUDO:
			  /* do normal conf mode processing */
			if (ms->confmode & ZT_CONF_TALKER) {
				/* Store temp value */
				memcpy(k, putlin, ZT_CHUNKSIZE * sizeof(short));
				/* Add conf value */
				ACSS(k, conf_sums_next[ms->_confn]);
				/*  get amount actually added */
				memcpy(ms->conflast, k, ZT_CHUNKSIZE * sizeof(short));
				SCSS(ms->conflast, conf_sums_next[ms->_confn]);
				/* Really add in new value */
				ACSS(conf_sums_next[ms->_confn], ms->conflast);
			} else memset(ms->conflast, 0, ZT_CHUNKSIZE * sizeof(short));
			  /* do the pseudo-channel part processing */
			memset(putlin, 0, ZT_CHUNKSIZE * sizeof(short));
			if (ms->confmode & ZT_CONF_PSEUDO_LISTENER) {
				/* Subtract out previous last sample written to conf */
				SCSS(putlin, ms->conflast2);
				/* Add in conference */
				ACSS(putlin, conf_sums[ms->_confn]);
			}
			/* Convert back */
			for(x=0;x<ZT_CHUNKSIZE;x++)
				rxb[x] = ZT_LIN2X(putlin[x], ms);
			break;
		case ZT_CONF_CONF:	/* Normal conference mode */
			if (ms->flags & ZT_FLAG_PSEUDO) /* if a pseudo-channel */
			   {
				if (ms->confmode & ZT_CONF_LISTENER) {
					/* Subtract out last sample written to conf */
					SCSS(putlin, ms->conflast);
					/* Add in conference */
					ACSS(putlin, conf_sums[ms->_confn]);
				}
				/* Convert back */
				for(x=0;x<ZT_CHUNKSIZE;x++)
					rxb[x] = ZT_LIN2X(putlin[x], ms);
				memcpy(ss->getlin, putlin, ZT_CHUNKSIZE * sizeof(short));
				break;
			   }
			/* fall through */
		case ZT_CONF_CONFANN:  /* Conference with announce */
			if (ms->confmode & ZT_CONF_TALKER) {
				/* Store temp value */
				memcpy(k, putlin, ZT_CHUNKSIZE * sizeof(short));
				/* Add conf value */
				ACSS(k, conf_sums_next[ms->_confn]);
				/*  get amount actually added */
				memcpy(ms->conflast, k, ZT_CHUNKSIZE * sizeof(short));
				SCSS(ms->conflast, conf_sums_next[ms->_confn]);
				/* Really add in new value */
				ACSS(conf_sums_next[ms->_confn], ms->conflast);
			} else 
				memset(ms->conflast, 0, ZT_CHUNKSIZE * sizeof(short));
			  /* rxc unmodified */
			break;
		case ZT_CONF_CONFMON:
		case ZT_CONF_CONFANNMON:
			if (ms->confmode & ZT_CONF_TALKER) {
				/* Store temp value */
				memcpy(k, putlin, ZT_CHUNKSIZE * sizeof(short));
				/* Subtract last value */
				SCSS(conf_sums[ms->_confn], ms->conflast);
				/* Add conf value */
				ACSS(k, conf_sums[ms->_confn]);
				/*  get amount actually added */
				memcpy(ms->conflast, k, ZT_CHUNKSIZE * sizeof(short));
				SCSS(ms->conflast, conf_sums[ms->_confn]);
				/* Really add in new value */
				ACSS(conf_sums[ms->_confn], ms->conflast);
			} else 
				memset(ms->conflast, 0, ZT_CHUNKSIZE * sizeof(short));
			for (x=0;x<ZT_CHUNKSIZE;x++)
				rxb[x] = ZT_LIN2X((int)conf_sums_prev[ms->_confn][x], ms);
			break;
		case ZT_CONF_DIGITALMON:
			  /* if not a pseudo-channel, ignore */
			if (!(ms->flags & ZT_FLAG_PSEUDO)) break;
			/* Add monitored channel */
			if (chans[ms->confna]->flags & ZT_FLAG_PSEUDO) {
				memcpy(rxb, chans[ms->confna]->getraw, ZT_CHUNKSIZE);
			} else {
				memcpy(rxb, chans[ms->confna]->putraw, ZT_CHUNKSIZE);
			}
			break;			
		}
	}
}

/* HDLC (or other) receiver buffer functions for read side */
static inline void __putbuf_chunk(struct zt_chan *ss, unsigned char *rxb, int bytes)
{
	/* We transmit data from our master channel */
	/* Called with ss->lock held */
	struct zt_chan *ms = ss->master;
	/* Our receive buffer */
	unsigned char *buf;
#if defined(CONFIG_ZAPATA_NET)  || defined(CONFIG_ZAPATA_PPP)
	/* SKB for receiving network stuff */
	struct sk_buff *skb=NULL;
#endif	
	int oldbuf;
	int eof=0;
	int abort=0;
	int res;
	int left, x;

	while(bytes) {
#if defined(CONFIG_ZAPATA_NET)  || defined(CONFIG_ZAPATA_PPP)
		skb = NULL;
#endif	
		abort = 0;
		eof = 0;
		/* Next, figure out if we've got a buffer to receive into */
		if (ms->inreadbuf > -1) {
			/* Read into the current buffer */
			buf = ms->readbuf[ms->inreadbuf];
			left = ms->blocksize - ms->readidx[ms->inreadbuf];
			if (left > bytes)
				left = bytes;
			if (ms->flags & ZT_FLAG_HDLC) {
				for (x=0;x<left;x++) {
					/* Handle HDLC deframing */
					fasthdlc_rx_load_nocheck(&ms->rxhdlc, *(rxb++));
					bytes--;
					res = fasthdlc_rx_run(&ms->rxhdlc);
					/* If there is nothing there, continue */
					if (res & RETURN_EMPTY_FLAG)
						continue;
					else if (res & RETURN_COMPLETE_FLAG) {
						/* Only count this if it's a non-empty frame */
						if (ms->readidx[ms->inreadbuf]) {
							if ((ms->flags & ZT_FLAG_FCS) && (ms->infcs != PPP_GOODFCS)) {
								abort = ZT_EVENT_BADFCS;
							} else
								eof=1;
							break;
						}
						continue;
					} else if (res & RETURN_DISCARD_FLAG) {
						/* This could be someone idling with 
						  "idle" instead of "flag" */
						if (!ms->readidx[ms->inreadbuf])
							continue;
						abort = ZT_EVENT_ABORT;
						break;
					} else {
						unsigned char rxc;
						rxc = res;
						ms->infcs = PPP_FCS(ms->infcs, rxc);
						buf[ms->readidx[ms->inreadbuf]++] = rxc;
						/* Pay attention to the possibility of an overrun */
						if (ms->readidx[ms->inreadbuf] >= ms->blocksize) {
							if (!ss->span->alarms) 
								printk(KERN_WARNING "HDLC Receiver overrun on channel %s (master=%s)\n", ss->name, ss->master->name);
							abort=ZT_EVENT_OVERRUN;
							/* Force the HDLC state back to frame-search mode */
							ms->rxhdlc.state = 0;
							ms->rxhdlc.bits = 0;
							ms->readidx[ms->inreadbuf]=0;
							break;
						}
					}
				}
			} else {
				/* Not HDLC */
				memcpy(buf + ms->readidx[ms->inreadbuf], rxb, left);
				rxb += left;
				ms->readidx[ms->inreadbuf] += left;
				bytes -= left;
				/* End of frame is decided by block size of 'N' */
				eof = (ms->readidx[ms->inreadbuf] >= ms->blocksize);
				if (eof && (ss->flags & ZT_FLAG_NOSTDTXRX)) {
					eof = 0;
					abort = ZT_EVENT_OVERRUN;
				}
			}
			if (eof)  {
				/* Finished with this buffer, try another. */
				oldbuf = ms->inreadbuf;
				ms->infcs = PPP_INITFCS;
				ms->readn[ms->inreadbuf] = ms->readidx[ms->inreadbuf];
#ifdef CONFIG_ZAPATA_DEBUG
				printk("EOF, len is %d\n", ms->readn[ms->inreadbuf]);
#endif
#if defined(CONFIG_ZAPATA_NET) || defined(CONFIG_ZAPATA_PPP)
				if (ms->flags & (ZT_FLAG_NETDEV | ZT_FLAG_PPP)) {
#ifdef CONFIG_ZAPATA_NET
#endif /* CONFIG_ZAPATA_NET */
					/* Our network receiver logic is MUCH
					  different.  We actually only use a single
					  buffer */
					if (ms->readn[ms->inreadbuf] > 1) {
						/* Drop the FCS */
						ms->readn[ms->inreadbuf] -= 2;
						/* Allocate an SKB */
#ifdef CONFIG_ZAPATA_PPP
						if (!ms->do_ppp_error)
#endif
							skb = dev_alloc_skb(ms->readn[ms->inreadbuf]);
						if (skb) {
							/* XXX Get rid of this memcpy XXX */
							memcpy(skb->data, ms->readbuf[ms->inreadbuf], ms->readn[ms->inreadbuf]);
							skb_put(skb, ms->readn[ms->inreadbuf]);
#ifdef CONFIG_ZAPATA_NET
							if (ms->flags & ZT_FLAG_NETDEV) {
#ifdef LINUX26
								struct net_device_stats *stats = hdlc_stats(ms->hdlcnetdev->netdev);
#else  /* LINUX26 */
								struct net_device_stats *stats = &ms->hdlcnetdev->netdev.stats;
#endif /* LINUX26 */
								stats->rx_packets++;
								stats->rx_bytes += ms->readn[ms->inreadbuf];
							}
#endif

						} else {
#ifdef CONFIG_ZAPATA_NET
							if (ms->flags & ZT_FLAG_NETDEV) {
#ifdef LINUX26
								struct net_device_stats *stats = hdlc_stats(ms->hdlcnetdev->netdev);
#else  /* LINUX26 */
								struct net_device_stats *stats = &ms->hdlcnetdev->netdev.stats;
#endif /* LINUX26 */
								stats->rx_dropped++;
							}
#endif
#ifdef CONFIG_ZAPATA_PPP
							if (ms->flags & ZT_FLAG_PPP) {
								abort = ZT_EVENT_OVERRUN;
							}
#endif
#if 1
#ifdef CONFIG_ZAPATA_PPP
							if (!ms->do_ppp_error)
#endif
								printk("Memory squeeze, dropped one\n");
#endif
						}
					}
					/* We don't cycle through buffers, just
					reuse the same one */
					ms->readn[ms->inreadbuf] = 0;
					ms->readidx[ms->inreadbuf] = 0;
				} else 
#endif
				{
					ms->inreadbuf = (ms->inreadbuf + 1) % ms->numbufs;
					if (ms->inreadbuf == ms->outreadbuf) {
						/* Whoops, we're full, and have no where else
						to store into at the moment.  We'll drop it
						until there's a buffer available */
#ifdef CONFIG_ZAPATA_DEBUG
						printk("Out of storage space\n");
#endif
						ms->inreadbuf = -1;
						/* Enable the receiver in case they've got POLICY_WHEN_FULL */
						ms->rxdisable = 0;
					}
					if (ms->outreadbuf < 0) { /* start out buffer if not already */
						ms->outreadbuf = oldbuf;
					}
/* In the very orignal driver, it was quite well known to me (Jim) that there
was a possibility that a channel sleeping on a receive block needed to
be potentially woken up EVERY time a buffer was filled, not just on the first
one, because if only done on the first one there is a slight timing potential
of missing the wakeup (between where it senses the (lack of) active condition
(with interrupts disabled) and where it does the sleep (interrupts enabled)
in the read or iomux call, etc). That is why the read and iomux calls start
with an infinite loop that gets broken out of upon an active condition,
otherwise keeps sleeping and looking. The part in this code got "optimized"
out in the later versions, and is put back now. */
					if (!ms->rxdisable) { /* if receiver enabled */
						/* Notify a blocked reader that there is data available
						to be read, unless we're waiting for it to be full */
#ifdef CONFIG_ZAPATA_DEBUG
						printk("Notifying reader data in block %d\n", oldbuf);
#endif
						wake_up_interruptible(&ms->readbufq);
						wake_up_interruptible(&ms->sel);
						if (ms->iomask & ZT_IOMUX_READ)
							wake_up_interruptible(&ms->eventbufq);
					}
				}
			}
			if (abort) {
				/* Start over reading frame */
				ms->readidx[ms->inreadbuf] = 0;
				ms->infcs = PPP_INITFCS;

#ifdef CONFIG_ZAPATA_NET
				if (ms->flags & ZT_FLAG_NETDEV) {
#ifdef LINUX26
					struct net_device_stats *stats = hdlc_stats(ms->hdlcnetdev->netdev);
#else  /* LINUX26 */
					struct net_device_stats *stats = &ms->hdlcnetdev->netdev.stats;
#endif /* LINUX26 */
					stats->rx_errors++;
					if (abort == ZT_EVENT_OVERRUN)
						stats->rx_over_errors++;
					if (abort == ZT_EVENT_BADFCS)
						stats->rx_crc_errors++;
					if (abort == ZT_EVENT_ABORT)
						stats->rx_frame_errors++;
				} else 
#endif			
#ifdef CONFIG_ZAPATA_PPP
				if (ms->flags & ZT_FLAG_PPP) {
					ms->do_ppp_error = 1;
					tasklet_schedule(&ms->ppp_calls);
				} else
#endif

				if ((ms->flags & ZT_FLAG_OPEN) && !ss->span->alarms) 
						/* Notify the receiver... */
					__qevent(ss->master, abort);
#if 0
				printk("torintr_receive: Aborted %d bytes of frame on %d\n", amt, ss->master);
#endif

			}
		} else /* No place to receive -- drop on the floor */
			break;
#ifdef CONFIG_ZAPATA_NET
		if (skb && (ms->flags & ZT_FLAG_NETDEV))
#ifdef NEW_HDLC_INTERFACE
		{
			skb->mac.raw = skb->data;
			skb->dev = ztchan_to_dev(ms);
#ifdef ZAP_HDLC_TYPE_TRANS
			skb->protocol = hdlc_type_trans(skb, ztchan_to_dev(ms));
#else
			skb->protocol = htons (ETH_P_HDLC);
#endif
			netif_rx(skb);
		}
#else
			hdlc_netif_rx(&ms->hdlcnetdev->netdev, skb);
#endif
#endif
#ifdef CONFIG_ZAPATA_PPP
		if (skb && (ms->flags & ZT_FLAG_PPP)) {
			unsigned char *tmp;
			tmp = skb->data;
			skb_pull(skb, 2);
			/* Make sure that it's addressed to ALL STATIONS and UNNUMBERED */
			if (!tmp || (tmp[0] != 0xff) || (tmp[1] != 0x03)) {
				/* Invalid SKB -- drop */
				if (tmp)
					printk("Received invalid SKB (%02x, %02x)\n", tmp[0], tmp[1]);
				dev_kfree_skb_irq(skb);
			} else {
				skb_queue_tail(&ms->ppp_rq, skb);
				tasklet_schedule(&ms->ppp_calls);
			}
		}
#endif
	}
}

static inline void __zt_putbuf_chunk(struct zt_chan *ss, unsigned char *rxb)
{
	__putbuf_chunk(ss, rxb, ZT_CHUNKSIZE);
}

static void __zt_hdlc_abort(struct zt_chan *ss, int event)
{
	if (ss->inreadbuf >= 0)
		ss->readidx[ss->inreadbuf] = 0;
	if ((ss->flags & ZT_FLAG_OPEN) && !ss->span->alarms)
		__qevent(ss->master, event);
}

extern void zt_hdlc_abort(struct zt_chan *ss, int event)
{
	unsigned long flags;
	spin_lock_irqsave(&ss->lock, flags);
	__zt_hdlc_abort(ss, event);
	spin_unlock_irqrestore(&ss->lock, flags);
}

extern void zt_hdlc_putbuf(struct zt_chan *ss, unsigned char *rxb, int bytes)
{
	unsigned long flags;
	int res;
	int left;

	spin_lock_irqsave(&ss->lock, flags);
	if (ss->inreadbuf < 0) {
#ifdef CONFIG_ZAPATA_DEBUG
		printk("No place to receive HDLC frame\n");
#endif
		spin_unlock_irqrestore(&ss->lock, flags);
		return;
	}
	/* Read into the current buffer */
	left = ss->blocksize - ss->readidx[ss->inreadbuf];
	if (left > bytes)
		left = bytes;
	if (left > 0) {
		memcpy(ss->readbuf[ss->inreadbuf] + ss->readidx[ss->inreadbuf], rxb, left);
		rxb += left;
		ss->readidx[ss->inreadbuf] += left;
		bytes -= left;
	}
	/* Something isn't fit into buffer */
	if (bytes) {
#ifdef CONFIG_ZAPATA_DEBUG
		printk("HDLC frame isn't fit into buffer space\n");
#endif
		__zt_hdlc_abort(ss, ZT_EVENT_OVERRUN);
	}
	res = left;
	spin_unlock_irqrestore(&ss->lock, flags);
}

extern void zt_hdlc_finish(struct zt_chan *ss)
{
	int oldreadbuf;
	unsigned long flags;

	spin_lock_irqsave(&ss->lock, flags);

	if ((oldreadbuf = ss->inreadbuf) < 0) {
#ifdef CONFIG_ZAPATA_DEBUG
		printk("No buffers to finish\n");
#endif
		spin_unlock_irqrestore(&ss->lock, flags);
		return;
	}

	if (!ss->readidx[ss->inreadbuf]) {
#ifdef CONFIG_ZAPATA_DEBUG
		printk("Empty HDLC frame received\n");
#endif
		spin_unlock_irqrestore(&ss->lock, flags);
		return;
	}

	ss->readn[ss->inreadbuf] = ss->readidx[ss->inreadbuf];
	ss->inreadbuf = (ss->inreadbuf + 1) % ss->numbufs;
	if (ss->inreadbuf == ss->outreadbuf) {
		ss->inreadbuf = -1;
#ifdef CONFIG_ZAPATA_DEBUG
		printk("Notifying reader data in block %d\n", oldreadbuf);
#endif
		ss->rxdisable = 0;
	}
	if (ss->outreadbuf < 0) {
		ss->outreadbuf = oldreadbuf;
	}

	if (!ss->rxdisable) {
		wake_up_interruptible(&ss->readbufq);
		wake_up_interruptible(&ss->sel);
		if (ss->iomask & ZT_IOMUX_READ)
			wake_up_interruptible(&ss->eventbufq);
	}
	spin_unlock_irqrestore(&ss->lock, flags);
}

/* Returns 1 if EOF, 0 if data is still in frame, -1 if EOF and no buffers left */
extern int zt_hdlc_getbuf(struct zt_chan *ss, unsigned char *bufptr, unsigned int *size)
{
	unsigned char *buf;
	unsigned long flags;
	int left = 0;
	int res;
	int oldbuf;

	spin_lock_irqsave(&ss->lock, flags);
	if (ss->outwritebuf > -1) {
		buf = ss->writebuf[ss->outwritebuf];
		left = ss->writen[ss->outwritebuf] - ss->writeidx[ss->outwritebuf];
		/* Strip off the empty HDLC CRC end */
		left -= 2;
		if (left <= *size) {
			*size = left;
			res = 1;
		} else
			res = 0;

		memcpy(bufptr, &buf[ss->writeidx[ss->outwritebuf]], *size);
		ss->writeidx[ss->outwritebuf] += *size;

		if (res) {
			/* Rotate buffers */
			oldbuf = ss->outwritebuf;
			ss->writeidx[oldbuf] = 0;
			ss->writen[oldbuf] = 0;
			ss->outwritebuf = (ss->outwritebuf + 1) % ss->numbufs;
			if (ss->outwritebuf == ss->inwritebuf) {
				ss->outwritebuf = -1;
				if (ss->iomask & (ZT_IOMUX_WRITE | ZT_IOMUX_WRITEEMPTY))
					wake_up_interruptible(&ss->eventbufq);
				/* If we're only supposed to start when full, disable the transmitter */
				if (ss->txbufpolicy == ZT_POLICY_WHEN_FULL)
					ss->txdisable = 1;
				res = -1;
			}

			if (ss->inwritebuf < 0)
				ss->inwritebuf = oldbuf;

			if (!(ss->flags & (ZT_FLAG_NETDEV | ZT_FLAG_PPP))) {
				wake_up_interruptible(&ss->writebufq);
				wake_up_interruptible(&ss->sel);
				if ((ss->iomask & ZT_IOMUX_WRITE) && (res >= 0))
					wake_up_interruptible(&ss->eventbufq);
			}
		}
	} else {
		res = -1;
		*size = 0;
	}
	spin_unlock_irqrestore(&ss->lock, flags);

	return res;
}


static void process_timers(void)
{
	unsigned long flags;
	struct zt_timer *cur;
	spin_lock_irqsave(&zaptimerlock, flags);
	cur = zaptimers;
	while(cur) {
		if (cur->ms) {
			cur->pos -= ZT_CHUNKSIZE;
			if (cur->pos <= 0) {
				cur->tripped++;
				cur->pos = cur->ms;
				wake_up_interruptible(&cur->sel);
			}
		}
		cur = cur->next;
	}
	spin_unlock_irqrestore(&zaptimerlock, flags);
}

static unsigned int zt_timer_poll(struct file *file, struct poll_table_struct *wait_table)
{
	struct zt_timer *timer = file->private_data;
	unsigned long flags;
	int ret = 0;
	if (timer) {
		poll_wait(file, &timer->sel, wait_table);
		spin_lock_irqsave(&zaptimerlock, flags);
		if (timer->tripped || timer->ping) 
			ret |= POLLPRI;
		spin_unlock_irqrestore(&zaptimerlock, flags);
	} else
		ret = -EINVAL;
	return ret;
}

/* device poll routine */
static unsigned int
zt_chan_poll(struct file *file, struct poll_table_struct *wait_table, int unit)
{   
	
	struct zt_chan *chan = chans[unit];
	int	ret;
	unsigned long flags;

	  /* do the poll wait */
	if (chan) {
		poll_wait(file, &chan->sel, wait_table);
		ret = 0; /* start with nothing to return */
		spin_lock_irqsave(&chan->lock, flags);
		   /* if at least 1 write buffer avail */
		if (chan->inwritebuf > -1) {
			ret |= POLLOUT | POLLWRNORM;
		}
		if ((chan->outreadbuf > -1) && !chan->rxdisable) {
			ret |= POLLIN | POLLRDNORM;
		}
		if (chan->eventoutidx != chan->eventinidx)
		   {
			/* Indicate an exception */
			ret |= POLLPRI;
		   }
		spin_unlock_irqrestore(&chan->lock, flags);
	} else
		ret = -EINVAL;
	return(ret);  /* return what we found */
}

static int zt_mmap(struct file *file, struct vm_area_struct *vm)
{
	int unit = UNIT(file);
	if (unit == 250)
		return zt_transcode_fops->mmap(file, vm);
	return -ENOSYS;
}

static unsigned int zt_poll(struct file *file, struct poll_table_struct *wait_table)
{
	int unit = UNIT(file);
	struct zt_chan *chan;

	if (!unit)
		return -EINVAL;

	if (unit == 250)
		return zt_transcode_fops->poll(file, wait_table);

	if (unit == 253)
		return zt_timer_poll(file, wait_table);
		
	if (unit == 254) {
		chan = file->private_data;
		if (!chan)
			return -EINVAL;
		return zt_chan_poll(file, wait_table,chan->channo);
	}
	if (unit == 255) {
		chan = file->private_data;
		if (!chan) {
			printk("No pseudo channel structure to read?\n");
			return -EINVAL;
		}
		return zt_chan_poll(file, wait_table, chan->channo);
	}
	return zt_chan_poll(file, wait_table, unit);
}

static void __zt_transmit_chunk(struct zt_chan *chan, unsigned char *buf)
{
	unsigned char silly[ZT_CHUNKSIZE];
	/* Called with chan->lock locked */
	if (!buf)
		buf = silly;
	__zt_getbuf_chunk(chan, buf);

	if ((chan->flags & ZT_FLAG_AUDIO) || (chan->confmode)) {
#ifdef CONFIG_ZAPTEL_MMX
		zt_kernel_fpu_begin();
#endif
		__zt_process_getaudio_chunk(chan, buf);
#ifdef CONFIG_ZAPTEL_MMX
		kernel_fpu_end();
#endif
	}
}

static inline void __zt_real_transmit(struct zt_chan *chan)
{
	/* Called with chan->lock held */
	if (chan->confmode) {
		/* Pull queued data off the conference */
		__buf_pull(&chan->confout, chan->writechunk, chan, "zt_real_transmit");
	} else {
		__zt_transmit_chunk(chan, chan->writechunk);
	}
}

static void __zt_getempty(struct zt_chan *ms, unsigned char *buf)
{
	int bytes = ZT_CHUNKSIZE;
	int left;
	unsigned char *txb = buf;
	int x;
	short getlin;
	/* Called with ms->lock held */

	while(bytes) {
		/* Receive silence, or tone */
		if (ms->curtone) {
			left = ms->curtone->tonesamples - ms->tonep;
			if (left > bytes)
				left = bytes;
			for (x=0;x<left;x++) {
				/* Pick our default value from the next sample of the current tone */
				getlin = zt_tone_nextsample(&ms->ts, ms->curtone);
				*(txb++) = ZT_LIN2X(getlin, ms);
			}
			ms->tonep+=left;
			bytes -= left;
			if (ms->tonep >= ms->curtone->tonesamples) {
				struct zt_tone *last;
				/* Go to the next sample of the tone */
				ms->tonep = 0;
				last = ms->curtone;
				ms->curtone = ms->curtone->next;
				if (!ms->curtone) {
					/* No more tones...  Is this dtmf or mf?  If so, go to the next digit */
					if (ms->dialing)
						__do_dtmf(ms);
				} else {
					if (last != ms->curtone)
						zt_init_tone_state(&ms->ts, ms->curtone);
				}
			}
		} else {
			/* Use silence */
			memset(txb, ZT_LIN2X(0, ms), bytes);
			bytes = 0;
		}
	}
		
}

static void __zt_receive_chunk(struct zt_chan *chan, unsigned char *buf)
{
	/* Receive chunk of audio -- called with chan->lock held */
	unsigned char waste[ZT_CHUNKSIZE];

	if (!buf) {
		memset(waste, ZT_LIN2X(0, chan), sizeof(waste));
		buf = waste;
	}
	if ((chan->flags & ZT_FLAG_AUDIO) || (chan->confmode)) {
#ifdef CONFIG_ZAPTEL_MMX                         
		zt_kernel_fpu_begin();
#endif
		__zt_process_putaudio_chunk(chan, buf);
#ifdef CONFIG_ZAPTEL_MMX
		kernel_fpu_end();
#endif
	}
	__zt_putbuf_chunk(chan, buf);
}

static inline void __zt_real_receive(struct zt_chan *chan)
{
	/* Called with chan->lock held */
	if (chan->confmode) {
		/* Load into queue if we have space */
		__buf_push(&chan->confin, chan->readchunk, "zt_real_receive");
	} else {
		__zt_receive_chunk(chan, chan->readchunk);
	}
}

int zt_transmit(struct zt_span *span)
{
	int x,y,z;
	unsigned long flags;

#if 1
	for (x=0;x<span->channels;x++) {
		spin_lock_irqsave(&span->chans[x].lock, flags);
		if (span->chans[x].flags & ZT_FLAG_NOSTDTXRX) {
			spin_unlock_irqrestore(&span->chans[x].lock, flags);
			continue;
		}
		if (&span->chans[x] == span->chans[x].master) {
			if (span->chans[x].otimer) {
				span->chans[x].otimer -= ZT_CHUNKSIZE;
				if (span->chans[x].otimer <= 0) {
					__rbs_otimer_expire(&span->chans[x]);
				}
			}
			if (span->chans[x].flags & ZT_FLAG_AUDIO) {
				__zt_real_transmit(&span->chans[x]);
			} else {
				if (span->chans[x].nextslave) {
					u_char data[ZT_CHUNKSIZE];
					int pos=ZT_CHUNKSIZE;
					/* Process master/slaves one way */
					for (y=0;y<ZT_CHUNKSIZE;y++) {
						/* Process slaves for this byte too */
						z = x;
						do {
							if (pos==ZT_CHUNKSIZE) {
								/* Get next chunk */
								__zt_transmit_chunk(&span->chans[x], data);
								pos = 0;
							}
							span->chans[z].writechunk[y] = data[pos++]; 
							z = span->chans[z].nextslave;
						} while(z);
					}
				} else {
					/* Process independents elsewise */
					__zt_real_transmit(&span->chans[x]);
				}
			}
			if (span->chans[x].sig == ZT_SIG_DACS_RBS) {
				if (chans[span->chans[x].confna]) {
				    	/* Just set bits for our destination */
					if (span->chans[x].txsig != chans[span->chans[x].confna]->rxsig) {
						span->chans[x].txsig = chans[span->chans[x].confna]->rxsig;
						span->rbsbits(&span->chans[x], chans[span->chans[x].confna]->rxsig);
					}
				}
			}

		}
		spin_unlock_irqrestore(&span->chans[x].lock, flags);
	}
	if (span->mainttimer) {
		span->mainttimer -= ZT_CHUNKSIZE;
		if (span->mainttimer <= 0) {
			span->mainttimer = 0;
			if (span->maint)
				span->maint(span, ZT_MAINT_LOOPSTOP);
			span->maintstat = 0;
			wake_up_interruptible(&span->maintq);
		}
	}
#endif
	return 0;
}

int zt_receive(struct zt_span *span)
{
	int x,y,z;
	unsigned long flags, flagso;

#if 1
#ifdef CONFIG_ZAPTEL_WATCHDOG
	span->watchcounter--;
#endif	
	for (x=0;x<span->channels;x++) {
		if (span->chans[x].master == &span->chans[x]) {
			spin_lock_irqsave(&span->chans[x].lock, flags);
			if (span->chans[x].nextslave) {
				/* Must process each slave at the same time */
				u_char data[ZT_CHUNKSIZE];
				int pos = 0;
				for (y=0;y<ZT_CHUNKSIZE;y++) {
					/* Put all its slaves, too */
					z = x;
					do {
						data[pos++] = span->chans[z].readchunk[y];
						if (pos == ZT_CHUNKSIZE) {
							if(!(span->chans[x].flags & ZT_FLAG_NOSTDTXRX))
								__zt_receive_chunk(&span->chans[x], data);
							pos = 0;
						}
						z=span->chans[z].nextslave;
					} while(z);
				}
			} else {
				/* Process a normal channel */
				if (!(span->chans[x].flags & ZT_FLAG_NOSTDTXRX))
					__zt_real_receive(&span->chans[x]);
			}
			if (span->chans[x].itimer) {
				span->chans[x].itimer -= ZT_CHUNKSIZE;
				if (span->chans[x].itimer <= 0) {
					rbs_itimer_expire(&span->chans[x]);
				}
			}
			if (span->chans[x].ringdebtimer)
				span->chans[x].ringdebtimer--;
			if (span->chans[x].sig & __ZT_SIG_FXS) {
				if (span->chans[x].rxhooksig == ZT_RXSIG_RING)
					span->chans[x].ringtrailer = ZT_RINGTRAILER;
				else if (span->chans[x].ringtrailer) {
					span->chans[x].ringtrailer-= ZT_CHUNKSIZE;
					/* See if RING trailer is expired */
					if (!span->chans[x].ringtrailer && !span->chans[x].ringdebtimer) 
						__qevent(&span->chans[x],ZT_EVENT_RINGOFFHOOK);
				}
			}
			if (span->chans[x].pulsetimer)
			{
				span->chans[x].pulsetimer--;
				if (span->chans[x].pulsetimer <= 0)
				{
					if (span->chans[x].pulsecount)
					{
						if (span->chans[x].pulsecount > 12) {
						
							printk("Got pulse digit %d on %s???\n",
						    span->chans[x].pulsecount,
							span->chans[x].name);
						} else if (span->chans[x].pulsecount > 11) {
							__qevent(&span->chans[x], ZT_EVENT_PULSEDIGIT | '#');
						} else if (span->chans[x].pulsecount > 10) {
							__qevent(&span->chans[x], ZT_EVENT_PULSEDIGIT | '*');
						} else if (span->chans[x].pulsecount > 9) {
							__qevent(&span->chans[x], ZT_EVENT_PULSEDIGIT | '0');
						} else {
							__qevent(&span->chans[x], ZT_EVENT_PULSEDIGIT | ('0' + 
								span->chans[x].pulsecount));
						}
						span->chans[x].pulsecount = 0;
					}
				}
			}
			spin_unlock_irqrestore(&span->chans[x].lock, flags);
		}
	}

	if (span == master) {
		/* Hold the big zap lock for the duration of major
		   activities which touch all sorts of channels */
		spin_lock_irqsave(&bigzaplock, flagso);			
		/* Process any timers */
		process_timers();
		/* If we have dynamic stuff, call the ioctl with 0,0 parameters to
		   make it run */
		if (zt_dynamic_ioctl)
			zt_dynamic_ioctl(0,0);
		for (x=1;x<maxchans;x++) {
			if (chans[x] && chans[x]->confmode && !(chans[x]->flags & ZT_FLAG_PSEUDO)) {
				u_char *data;
				spin_lock_irqsave(&chans[x]->lock, flags);
				data = __buf_peek(&chans[x]->confin);
				__zt_receive_chunk(chans[x], data);
				if (data)
					__buf_pull(&chans[x]->confin, NULL,chans[x], "confreceive");
				spin_unlock_irqrestore(&chans[x]->lock, flags);
			}
		}
		/* This is the master channel, so make things switch over */
		rotate_sums();
		/* do all the pseudo and/or conferenced channel receives (getbuf's) */
		for (x=1;x<maxchans;x++) {
			if (chans[x] && (chans[x]->flags & ZT_FLAG_PSEUDO)) {
				spin_lock_irqsave(&chans[x]->lock, flags);
				__zt_transmit_chunk(chans[x], NULL);
				spin_unlock_irqrestore(&chans[x]->lock, flags);
			}
		}
		if (maxlinks) {
#ifdef CONFIG_ZAPTEL_MMX
			zt_kernel_fpu_begin();
#endif			
			  /* process all the conf links */
			for(x = 1; x <= maxlinks; x++) {
				  /* if we have a destination conf */
				if (((z = confalias[conf_links[x].dst]) > 0) &&
				    ((y = confalias[conf_links[x].src]) > 0)) {
					ACSS(conf_sums[z], conf_sums[y]);
				}
			}
#ifdef CONFIG_ZAPTEL_MMX
			kernel_fpu_end();
#endif			
		}
		/* do all the pseudo/conferenced channel transmits (putbuf's) */
		for (x=1;x<maxchans;x++) {
			if (chans[x] && (chans[x]->flags & ZT_FLAG_PSEUDO)) {
				unsigned char tmp[ZT_CHUNKSIZE];
				spin_lock_irqsave(&chans[x]->lock, flags);
				__zt_getempty(chans[x], tmp);
				__zt_receive_chunk(chans[x], tmp);
				spin_unlock_irqrestore(&chans[x]->lock, flags);
			}
		}
		for (x=1;x<maxchans;x++) {
			if (chans[x] && chans[x]->confmode && !(chans[x]->flags & ZT_FLAG_PSEUDO)) {
				u_char *data;
				spin_lock_irqsave(&chans[x]->lock, flags);
				data = __buf_pushpeek(&chans[x]->confout);
				__zt_transmit_chunk(chans[x], data);
				if (data)
					__buf_push(&chans[x]->confout, NULL, "conftransmit");
				spin_unlock_irqrestore(&chans[x]->lock, flags);
			}
		}
#ifdef	ZAPTEL_SYNC_TICK
		for (x=0;x<maxspans;x++) {
			struct zt_span	*s = spans[x];

			if (s && s->sync_tick)
				s->sync_tick(s, s == master);
		}
#endif
		spin_unlock_irqrestore(&bigzaplock, flagso);			
	}
#endif
	return 0;
}

MODULE_AUTHOR("Mark Spencer <markster@digium.com>");
MODULE_DESCRIPTION("Zapata Telephony Interface");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(ZAPTEL_VERSION);
#endif

#ifdef LINUX26
module_param(debug, int, 0600);
module_param(deftaps, int, 0600);
#else
MODULE_PARM(debug, "i");
MODULE_PARM(deftaps, "i");
#endif

static struct file_operations zt_fops = {
	owner: THIS_MODULE,
	llseek: NULL,
	open: zt_open,
	release: zt_release,
	ioctl: zt_ioctl,
	read: zt_read,
	write: zt_write,
	poll: zt_poll,
	mmap: zt_mmap,
	flush: NULL,
	fsync: NULL,
	fasync: NULL,
};

#ifdef CONFIG_ZAPTEL_WATCHDOG
static struct timer_list watchdogtimer;

static void watchdog_check(unsigned long ignored)
{
	int x;
	unsigned long flags;
	static int wdcheck=0;
	
	local_irq_save(flags);
	for (x=0;x<maxspans;x++) {
		if (spans[x] && (spans[x]->flags & ZT_FLAG_RUNNING)) {
			if (spans[x]->watchcounter == ZT_WATCHDOG_INIT) {
				/* Whoops, dead card */
				if ((spans[x]->watchstate == ZT_WATCHSTATE_OK) || 
					(spans[x]->watchstate == ZT_WATCHSTATE_UNKNOWN)) {
					spans[x]->watchstate = ZT_WATCHSTATE_RECOVERING;
					if (spans[x]->watchdog) {
						printk("Kicking span %s\n", spans[x]->name);
						spans[x]->watchdog(spans[x], ZT_WATCHDOG_NOINTS);
					} else {
						printk("Span %s is dead with no revival\n", spans[x]->name);
						spans[x]->watchstate = ZT_WATCHSTATE_FAILED;
					}
				}
			} else {
				if ((spans[x]->watchstate != ZT_WATCHSTATE_OK) &&
					(spans[x]->watchstate != ZT_WATCHSTATE_UNKNOWN))
						printk("Span %s is alive!\n", spans[x]->name);
				spans[x]->watchstate = ZT_WATCHSTATE_OK;
			}
			spans[x]->watchcounter = ZT_WATCHDOG_INIT;
		}
	}
	local_irq_restore(flags);
	if (!wdcheck) {
		printk("Zaptel watchdog on duty!\n");
		wdcheck=1;
	}
	mod_timer(&watchdogtimer, jiffies + 2);
}

static int __init watchdog_init(void)
{
	init_timer(&watchdogtimer);
	watchdogtimer.expires = 0;
	watchdogtimer.data =0;
	watchdogtimer.function = watchdog_check;
	/* Run every couple of jiffy or so */
	mod_timer(&watchdogtimer, jiffies + 2);
	return 0;
}

static void __exit watchdog_cleanup(void)
{
	del_timer(&watchdogtimer);
}

#endif

static int __init zt_init(void) {
	int res = 0;
	int i = 0;

#ifdef CONFIG_PROC_FS
	proc_entries[0] = proc_mkdir("zaptel", NULL);
#endif

#ifdef CONFIG_ZAP_UDEV /* udev support functions */
	zap_class = class_create(THIS_MODULE, "zaptel");
	CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, 250), NULL, "zaptranscode");
	CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, 253), NULL, "zaptimer");
	CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, 254), NULL, "zapchannel");
	CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, 255), NULL, "zappseudo");
	CLASS_DEV_CREATE(zap_class, MKDEV(ZT_MAJOR, 0), NULL, "zapctl");
#endif /* CONFIG_ZAP_UDEV */

#ifdef CONFIG_DEVFS_FS
	{
	umode_t mode = S_IFCHR|S_IRUGO|S_IWUGO;
	devfs_register_chrdev(ZT_MAJOR, "zaptel", &zt_fops);
	zaptel_devfs_dir = devfs_mk_dir(NULL, "zap", NULL);
	if (!zaptel_devfs_dir) return -EBUSY; /* This would be bad */
	timer = devfs_register(zaptel_devfs_dir, "timer", DEVFS_FL_DEFAULT, ZT_MAJOR, 253, mode, &zt_fops, NULL);
	channel = devfs_register(zaptel_devfs_dir, "channel", DEVFS_FL_DEFAULT, ZT_MAJOR, 254, mode, &zt_fops, NULL);
	pseudo = devfs_register(zaptel_devfs_dir, "pseudo", DEVFS_FL_DEFAULT, ZT_MAJOR, 255, mode, &zt_fops, NULL);
	transcode = devfs_register(zaptel_devfs_dir, "transcode", DEVFS_FL_DEFAULT, ZT_MAJOR, 250, mode, &zt_fops, NULL);
	ctl = devfs_register(zaptel_devfs_dir, "ctl", DEVFS_FL_DEFAULT, ZT_MAJOR, 0, mode, &zt_fops, NULL);
	}
#else
	if ((res = register_chrdev(ZT_MAJOR, "zaptel", &zt_fops))) {
		printk(KERN_ERR "Unable to register tor device on %d\n", ZT_MAJOR);
		return res;
	}
#endif /* CONFIG_DEVFS_FS */

	if (!(dtmf_tones_continuous = kmalloc(sizeof(dtmf_tones), GFP_KERNEL))) {
		printk(KERN_ERR "Zaptel: THERE IS A CRISIS IN THE BATCAVE!"
			" Unable to allocate memory for continuous DTMF tones list!\n");
		return -ENOMEM;
	}

	if (!(mfv1_tones_continuous = kmalloc(sizeof(mfv1_tones), GFP_KERNEL))) {
		printk(KERN_ERR "Zaptel: THERE IS A CRISIS IN THE BATCAVE!"
			" Unable to allocate memory for continuous MFV1 tones list!\n");
		return -ENOMEM;
	}

	memcpy(dtmf_tones_continuous, dtmf_tones, sizeof(dtmf_tones));
	for (i = 0; i < (sizeof(dtmf_tones) / sizeof(dtmf_tones[0])); i++)
		dtmf_tones_continuous[i].next = dtmf_tones_continuous + i;

	memcpy(mfv1_tones_continuous, mfv1_tones, sizeof(mfv1_tones));
	for (i = 0; i < (sizeof(mfv1_tones) / sizeof(mfv1_tones[0])); i++)
		mfv1_tones_continuous[i].next = mfv1_tones_continuous + i;

	printk(KERN_INFO "Zapata Telephony Interface Registered on major %d\n", ZT_MAJOR);
	printk(KERN_INFO "Zaptel Version: %s\n", ZAPTEL_VERSION);
	echo_can_init();
	zt_conv_init();
	tone_zone_init();
	fasthdlc_precalc();
	rotate_sums();
	rwlock_init(&chan_lock);
#ifdef CONFIG_ZAPTEL_WATCHDOG
	watchdog_init();
#endif	
	return res;
}

static void __exit zt_cleanup(void) {
	int x;

#ifdef CONFIG_PROC_FS
	remove_proc_entry("zaptel", NULL);
#endif

	printk(KERN_INFO "Zapata Telephony Interface Unloaded\n");
	for (x = 0; x < ZT_TONE_ZONE_MAX; x++) {
		if (tone_zones[x])
			kfree(tone_zones[x]);
	}

	if (dtmf_tones_continuous) {
		kfree(dtmf_tones_continuous);
		dtmf_tones_continuous = NULL;
	}

	if (mfv1_tones_continuous) {
		kfree(mfv1_tones_continuous);
		mfv1_tones_continuous = NULL;
	}

#ifdef CONFIG_DEVFS_FS
	devfs_unregister(timer);
	devfs_unregister(transcode);
	devfs_unregister(channel);
	devfs_unregister(pseudo);
	devfs_unregister(ctl);
	devfs_unregister(zaptel_devfs_dir);
	devfs_unregister_chrdev(ZT_MAJOR, "zaptel");
#else
#ifdef CONFIG_ZAP_UDEV
	class_device_destroy(zap_class, MKDEV(ZT_MAJOR, 250)); /* transcode */
	class_device_destroy(zap_class, MKDEV(ZT_MAJOR, 253)); /* timer */
	class_device_destroy(zap_class, MKDEV(ZT_MAJOR, 254)); /* channel */
	class_device_destroy(zap_class, MKDEV(ZT_MAJOR, 255)); /* pseudo */
	class_device_destroy(zap_class, MKDEV(ZT_MAJOR, 0)); /* ctl */
	class_destroy(zap_class);
#endif /* CONFIG_ZAP_UDEV */
	unregister_chrdev(ZT_MAJOR, "zaptel");
#endif
#ifdef CONFIG_ZAPTEL_WATCHDOG
	watchdog_cleanup();
#endif

	echo_can_shutdown();
}

module_init(zt_init);
module_exit(zt_cleanup);
