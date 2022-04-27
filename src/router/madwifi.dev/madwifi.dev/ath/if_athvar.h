/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
	without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_athvar.h 3310 2008-01-30 20:23:49Z mentor $
 */

/*
 * Defintions for the Atheros Wireless LAN controller driver.
 */
#ifndef _DEV_ATH_ATHVAR_H
#define _DEV_ATH_ATHVAR_H

#include "ah.h"
#include "ah_desc.h"
#include "ah_os.h"
#include "if_athioctl.h"
#ifdef HAVE_WPROBE
#include "linux/wprobe.h"
#endif
#include "net80211/ieee80211.h"	/* XXX for WME_NUM_AC */
#include <asm/io.h>
#include <linux/list.h>
#include <sys/queue.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#include	<asm/bitops.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define irqs_disabled()			0
#endif

/*
 * Deduce if tasklets are available.  If not then
 * fall back to using the immediate work queue.
 */
#include <linux/interrupt.h>
#ifdef DECLARE_TASKLET		/* native tasklets */
#define ATH_TQ_STRUCT tasklet_struct
#define ATH_INIT_TQUEUE(a,b,c)		tasklet_init((a), (b), (unsigned long)(c))
#define ATH_SCHEDULE_TQUEUE(a,b)	tasklet_schedule((a))
typedef unsigned long TQUEUE_ARG;
#define mark_bh(a) do {} while (0)
#else				/* immediate work queue */
#define ATH_TQ_STRUCT tq_struct
#define ATH_INIT_TQUEUE(a,b,c)		INIT_TQUEUE(a,b,c)
#define ATH_SCHEDULE_TQUEUE(a,b) do {		\
	*(b) |= queue_task((a), &tq_immediate);	\
} while (0)
typedef void *TQUEUE_ARG;
#define	tasklet_disable(t)	do { (void) t; local_bh_disable(); } while (0)
#define	tasklet_enable(t)	do { (void) t; local_bh_enable(); } while (0)
#endif				/* !DECLARE_TASKLET */

/*
 * Guess how the interrupt handler should work.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
#if !defined(IRQ_NONE)
typedef void irqreturn_t;
#define	IRQ_NONE
#define	IRQ_HANDLED
#endif				/* !defined(IRQ_NONE) */
#endif				/* Linux < 2.6.29 */

#ifndef SET_MODULE_OWNER
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#define	SET_MODULE_OWNER(dev) do {		\
	dev->owner = THIS_MODULE;		\
} while (0)
#else
#define SET_MODULE_OWNER(dev) do { } while (0)
#endif
#endif

#ifndef SET_NETDEV_DEV
#define	SET_NETDEV_DEV(ndev, pdev)
#endif

#ifdef to_net_dev
#define ATH_GET_NETDEV_DEV(ndev)	((ndev)->dev.parent)
#else
#define ATH_GET_NETDEV_DEV(ndev)	((ndev)->class_dev.dev)
#endif

#ifndef NETDEV_TX_OK
#define NETDEV_TX_OK    0
#define NETDEV_TX_BUSY  1
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
static inline struct net_device *_alloc_netdev(int sizeof_priv, const char *mask, void (*setup)(struct net_device *))
{
	struct net_device *dev;
	int alloc_size;

	/* ensure 32-byte alignment of the private area */
	alloc_size = sizeof(*dev) + sizeof_priv + 31;

	dev = (struct net_device *)kmalloc(alloc_size, GFP_KERNEL);
	if (dev == NULL) {
		printk(KERN_ERR "alloc_dev: Unable to allocate device memory.\n");
		return NULL;
	}

	memset(dev, 0, alloc_size);

	if (sizeof_priv)
		dev->priv = (void *)(((long)(dev + 1) + 31) & ~31);

	setup(dev);
	strcpy(dev->name, mask);

	return dev;
}

/* Avoid name collision - some vendor kernels backport alloc_netdev() */
#undef alloc_netdev
#define alloc_netdev(s,m,d) _alloc_netdev(s, m, d)

/* Some vendors backport PDE, so make it a macro here */
#undef PDE
#define PDE(inode) ((struct proc_dir_entry *)(inode)->u.generic_ip)
#endif

/*
 * Macro to expand scalars to 64-bit objects
 */
#define	ito64(x) (sizeof(x)==8) ? (((unsigned long long int)(x)) & (0xff)) : \
		 (sizeof(x)==16) ? (((unsigned long long int)(x)) & 0xffff) : \
		 ((sizeof(x)==32) ? (((unsigned long long int)(x)) & 0xffffffff): (unsigned long long int)(x))

/*
 * Deal with the sysctl handler api changing.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
#define	ATH_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
	f(ctl_table *ctl, int write, struct file *filp, \
	  void __user *buffer, size_t *lenp)
#define	ATH_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, filp, buffer, lenp)
#define	ATH_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
	proc_dostring(ctl, write, filp, buffer, lenp)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
#define	ATH_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
 	f(ctl_table *ctl, int write, struct file *filp, \
 	  void __user *buffer, size_t *lenp, loff_t *ppos)
#define	ATH_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
 	proc_dointvec(ctl, write, filp, buffer, lenp, ppos)
#define ATH_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
        proc_dostring(ctl, write, filp, buffer, lenp, ppos)
#else				/* Linux 2.6.32+ */
#define	ATH_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
	f(ctl_table *ctl, int write, \
	  void __user *buffer, size_t *lenp, loff_t *ppos)
#define	ATH_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, buffer, lenp, ppos)
#define	ATH_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
	proc_dostring(ctl, write, buffer, lenp, ppos)
#endif

#define	ATH_TIMEOUT	1000

#define	ATH_DFS_WAIT_MIN_PERIOD		60	/* DFS wait is 60 seconds, per
						 * FCC/ETSI regulations. */

#define	ATH_DFS_WAIT_SHORT_POLL_PERIOD	2	/* 2 seconds, for consecutive
						 * waits if not done yet. */

#define	ATH_DFS_AVOID_MIN_PERIOD	1800	/* 30 minutes, per FCC/ETSI
						 * regulations */

#define	ATH_DFS_TEST_RETURN_PERIOD	15	/* 15 seconds -- for mute test
						 * only */

#define	ATH_LONG_CALINTERVAL		30	/* 30 seconds between calibrations */
#define	ATH_SHORT_CALINTERVAL		1	/* 1 second between calibrations */

/*
 * Maximum acceptable MTU
 * MAXFRAMEBODY - WEP - QOS - RSN/WPA:
 * 2312 - 8 - 2 - 12 = 2290
 */
#define ATH_MAX_MTU     2290
#define ATH_MIN_MTU     32

#define	ATH_RXBUF	40	/* number of RX buffers */
#define	ATH_TXBUF	300	/* number of TX buffers */

#define ATH_MAXVAPS_MIN 	2	/* minimum number of beacon buffers */
#define ATH_MAXVAPS_MAX 	64	/* maximum number of beacon buffers */
#define ATH_MAXVAPS_DEFAULT 	16	/* default number of beacon buffers */

/* free buffer threshold to restart net dev */
#define	ATH_TXBUF_FREE_THRESHOLD  (ATH_TXBUF / 20)
/* number of TX buffers reserved for mgt frames */
#define ATH_TXBUF_MGT_RESERVED	  5

#define TAIL_DROP_COUNT 50	/* maximum number of queued frames allowed */

/*
 * dynamic turbo specific macros.
 */
#define ATH_TURBO_UP_THRESH	750000	/* bytes/sec */
#define ATH_TURBO_DN_THRESH 	1000000	/* bytes/sec */
#define ATH_TURBO_PERIOD_HOLD	1	/* in seconds */

/*
 * The only case where we see skbuff chains is due to FF aggregation in
 * the driver.
 */
#ifdef ATH_SUPERG_FF
#define	ATH_TXDESC	2	/* number of descriptors per buffer */
#else
#define	ATH_TXDESC	1	/* number of descriptors per buffer */
#endif

#define	ATH_TXMAXTRY	11	/* max number of transmit attempts */

/* Compress settings */
#define ATH_COMP_THRESHOLD  256	/* no compression for frames
				   longer than this threshold  */
#define ATH_COMP_PROC_NO_COMP_NO_CCS    3
#define ATH_COMP_PROC_NO_COMP_ADD_CCS   2
#define ATH_COMP_PROC_COMP_NO_OPTIAML   1
#define ATH_COMP_PROC_COMP_OPTIMAL      0
#define ATH_DEFAULT_COMP_PROC           ATH_COMP_PROC_COMP_OPTIMAL

#define INVALID_DECOMP_INDEX		0xFFFF

#define WEP_IV_FIELD_SIZE       4	/* wep IV field size */
#define WEP_ICV_FIELD_SIZE      4	/* wep ICV field size */
#define AES_ICV_FIELD_SIZE      8	/* AES ICV field size */
#define EXT_IV_FIELD_SIZE       4	/* ext IV field size */

/* This is what the HAL uses by default for 11a+g */
#define ATH_DEFAULT_CWMIN	15
#define ATH_DEFAULT_CWMAX	1023

/* XR specific macros */

#define XR_DEFAULT_GRPPOLL_RATE_STR 	"0.25 1 1 3 3 6 6 20"
#define GRPPOLL_RATE_STR_LEN 	64
#define XR_SLOT_DELAY		30	/* in usec */
#define XR_AIFS			0
#define XR_NUM_RATES		5
#define XR_NUM_SUP_RATES	8
/* XR uplink should have same cwmin/cwmax value */
#define XR_CWMIN_CWMAX	7

#define XR_DATA_AIFS	3
#define XR_DATA_CWMIN	31
#define XR_DATA_CWMAX	1023

/* pick the threshold so that we meet most of the regulatory constraints */
#define XR_FRAGMENTATION_THRESHOLD		540
#define XR_TELEC_FRAGMENTATION_THRESHOLD	442

#define XR_MAX_GRP_POLL_PERIOD		1000	/* Maximum Group Poll Periodicity */

#define XR_DEFAULT_POLL_INTERVAL	100
#define XR_MIN_POLL_INTERVAL		30
#define XR_MAX_POLL_INTERVAL		1000
#define XR_DEFAULT_POLL_COUNT		32
#define XR_MIN_POLL_COUNT		16
#define XR_MAX_POLL_COUNT		64
#define XR_POLL_UPDATE_PERIOD		10	/* number of XR beacons */
#define XR_GRPPOLL_PERIOD_FACTOR	5	/* factor used in calculating grp poll interval */
#define XR_4MS_FRAG_THRESHOLD		128	/* fragmentation threshold for 4msec frame limit  */

/*
 * Maximum Values in ms for group poll periodicty
 */
#define GRP_POLL_PERIOD_NO_XR_STA_MAX	100
#define GRP_POLL_PERIOD_XR_STA_MAX	30

 /*
  * Percentage of the configured poll periodicity
  */
#define GRP_POLL_PERIOD_FACTOR_XR_STA	30	/* When XR Stations associated freq is 30% higher */

#define A_MAX(a,b) ((a) > (b) ? (a) : (b))

/*
 * Macros to obtain the Group Poll Periodicity in various situations
 *
 * Curerntly there are the two cases
 * (a) When there are no XR STAs associated
 * (b) When there is atleast one XR STA associated
 */
#define GRP_POLL_PERIOD_NO_XR_STA(sc) (sc->sc_xrpollint)
#define GRP_POLL_PERIOD_XR_STA(sc)                                                   \
	A_MAX(GRP_POLL_PERIOD_FACTOR_XR_STA * (sc->sc_xrpollint / 100), GRP_POLL_PERIOD_XR_STA_MAX)

/*
 * When there are no XR STAs and a valid double chirp is received then the Group Polls are
 * transmitted for 10 seconds from the time of the last valid double double-chirp
 */
#define NO_XR_STA_GRPPOLL_TX_DUR	10000

/*
 * The key cache is used for h/w cipher state and also for
 * tracking station state such as the current tx antenna.
 * We also setup a mapping table between key cache slot indices
 * and station state to short-circuit node lookups on rx.
 * Different parts have different size key caches.  We handle
 * up to ATH_KEYMAX entries (could dynamically allocate state).
 */
#define	ATH_KEYMAX	128	/* max key cache size we handle */
#define	ATH_KEYBYTES	(ATH_KEYMAX / NBBY)	/* storage space in bytes */

#define MIN_REGISTER_ADDRESS	0x0000	/* PCI register addresses are taken as releative to the appropriate BAR */
#define MAX_REGISTER_ADDRESS	0xc000	/* AR5212/AR5213 seems to have a 48k address range */
#define MAX_REGISTER_NAME_LEN	32	/* Maximum length of register nicknames in debug output */
#define UNKNOWN_NAME		"(unknown)"	/* Name used when reading/listing undocumented registers */
/*
 * Convert from net80211 layer values to Ath layer values. Hopefully this will
 * be optimised away when the two constants are the same.
 */
typedef unsigned int ath_keyix_t;
#define ATH_KEY(_keyix)	((_keyix == IEEE80211_KEYIX_NONE) ? HAL_TXKEYIX_INVALID : _keyix)

#define	ATH_MIN_FF_RATE	12000	/* min rate for ff aggregation in kbps */
#define	ATH_MIN_FF_RATE	12000	/* min rate for ff aggregation in kbps */
struct ath_buf;
typedef STAILQ_HEAD(, ath_buf) ath_bufhead;

/* driver-specific node state */
struct ath_node {
	struct ieee80211_node an_node;	/* base class */
#ifdef HAVE_WPROBE
	struct wprobe_link an_wplink;
	uint8_t an_wplink_active;
#endif
	struct work_struct an_destroy;
	u_int16_t an_decomp_index;	/* decompression mask index */
	u_int32_t an_avgrssi;	/* average rssi over all rx frames */
	u_int8_t an_prevdatarix;	/* rate ix of last data frame */
	u_int16_t an_minffrate;	/* min rate in kbps for ff to aggregate */
	HAL_NODE_STATS an_halstats;	/* rssi statistics used by hal */

	struct ath_buf *an_tx_ffbuf[WME_NUM_AC];	/* ff staging area */

	ath_bufhead an_uapsd_q;	/* U-APSD delivery queue */
	int an_uapsd_qdepth;	/* U-APSD delivery queue depth */
	ath_bufhead an_uapsd_overflowq;	/* U-APSD overflow queue (for > MaxSp frames) */
	int an_uapsd_overflowqdepth;	/* U-APSD overflow queue depth */
	spinlock_t an_uapsd_lock;	/* U-APSD delivery queue lock */
	/* variable-length rate control state follows */
};
#define	ATH_NODE(_n)			((struct ath_node *)(_n))
#define	ATH_NODE_CONST(ni)		((const struct ath_node *)(ni))
#define ATH_NODE_UAPSD_LOCK_INIT(_an)	spin_lock_init(&(_an)->an_uapsd_lock)
#define ATH_NODE_UAPSD_LOCK_IRQ(_an)	do {				     \
	unsigned long __an_uapsd_lockflags;				     \
	ATH_NODE_UAPSD_LOCK_CHECK(_an);				     	     \
	spin_lock_irqsave(&(_an)->an_uapsd_lock, __an_uapsd_lockflags);

#define ATH_NODE_UAPSD_UNLOCK_IRQ(_an)					     \
	ATH_NODE_UAPSD_LOCK_ASSERT(_an);				     \
	spin_unlock_irqrestore(&(_an)->an_uapsd_lock, __an_uapsd_lockflags); \
} while (0)

#define ATH_NODE_UAPSD_UNLOCK_IRQ_EARLY(_an) 		     \
	ATH_NODE_UAPSD_LOCK_ASSERT(_an);				     \
	spin_unlock_irqrestore(&(_an)->an_uapsd_lock, __an_uapsd_lockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ATH_NODE_UAPSD_LOCK_ASSERT(_an) \
	KASSERT(spin_is_locked(&(_an)->an_uapsd_lock), ("uapsd not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ATH_NODE_UAPSD_LOCK_CHECK(_an) do { \
	if (spin_is_locked(&(_an)->an_uapsd_lock)) \
		printk(KERN_DEBUG "%s:%d - about to block on uapsd lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ATH_NODE_UAPSD_LOCK_CHECK(_an)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	ATH_NODE_UAPSD_LOCK_ASSERT(_an)
#define	ATH_NODE_UAPSD_LOCK_CHECK(_an)
#endif

#define ATH_RSSI_LPF_LEN	10
#define ATH_RSSI_DUMMY_MARKER	0x127
#define ATH_EP_MUL(x, mul)	((x) * (mul))
#define ATH_RSSI_IN(x)		(ATH_EP_MUL((x), HAL_RSSI_EP_MULTIPLIER))
#define ATH_LPF_RSSI(x, y, len) \
	((x != ATH_RSSI_DUMMY_MARKER) ? (((x) * ((len) - 1) + (y)) / (len)) : (y))
#define ATH_RSSI_LPF(x, y) do {						\
	if ((y) >= -20)							\
		x = ATH_LPF_RSSI((x), ATH_RSSI_IN((y)), ATH_RSSI_LPF_LEN);	\
} while (0)

#define	ATH_ANTENNA_DIFF	2	/* Num frames difference in
					 * tx to flip default recv
					 * antenna
					 */

struct ath_buf {
	/* FFXXX: convert both list types to TAILQ to save a field? */
	STAILQ_ENTRY(ath_buf) bf_list;
#ifdef ATH_SUPERG_FF
	TAILQ_ENTRY(ath_buf) bf_stagelist;	/* fast-frame staging list */
#endif
	struct ath_desc *bf_desc;	/* virtual addr of desc */
	struct ath_desc_status bf_dsstatus;	/* tx/rx descriptor status */
	dma_addr_t bf_daddr;	/* physical addr of desc */
	struct sk_buff *bf_skb;	/* skbuff for buf */
	dma_addr_t bf_skbaddr;	/* physical addr of skb data - always used by one desc */
	struct ieee80211_node *bf_node;	/* pointer to the node */
	u_int32_t bf_status;	/* status flags */
	u_int16_t bf_flags;	/* tx descriptor flags */
	u_int64_t bf_tsf;
	int16_t bf_channoise;
	unsigned int rcflags;
#ifdef ATH_SUPERG_FF
	/* XXX: combine this with bf_skbaddr if it ever changes to accommodate
	 *      multiple segments.
	 */
	u_int16_t bf_numdescff;	/* number of descs used for FF (these are extra) */
	u_int32_t bf_queueage;	/* "age" of txq when this buffer placed on stageq */
	dma_addr_t bf_skbaddrff[ATH_TXDESC - 1];	/* extra addrs for FF */
#endif
};

/*
 * reset the rx buffer.
 * any new fields added to the athbuf and require 
 * reset need to be added to this macro.
 * currently bf_status is the only one that
 * requires reset.
 */
#define ATH_RXBUF_RESET(bf)	bf->bf_status=0

/* XXX: only managed for rx at the moment */
#define ATH_BUFSTATUS_DONE		0x00000001	/* hw processing complete, desc processed by hal */
#define ATH_BUFSTATUS_RADAR_DONE	0x00000002	/* marker to indicate a PHYERR for radar pulse
							   has already been handled.  We may receive
							   multiple interrupts before the rx_tasklet
							   clears the queue */
#define ATH_BUFSTATUS_RXTSTAMP          0x00000004	/* RX timestamps needs to be adjusted */

/* DMA state for tx/rx descriptors. */
struct ath_descdma {
	const char *dd_name;
	struct ath_desc *dd_desc;	/* descriptors */
	dma_addr_t dd_desc_paddr;	/* physical addr of dd_desc */
	size_t dd_desc_len;	/* size of dd_desc */
	struct ath_buf *dd_bufptr;	/* associated buffers */
};

struct ath_hal;
struct ath_desc;
struct ath_ratectrl;
struct ath_tx99;
struct ath_hw_detect;
struct proc_dir_entry;

/*
 * Data transmit queue state.  One of these exists for each
 * hardware transmit queue.  Packets sent to us from above
 * are assigned to queues based on their priority.  Not all
 * devices support a complete set of hardware transmit queues.
 * For those devices the array sc_ac2q will map multiple
 * priorities to fewer hardware queues (typically all to one
 * hardware queue).
 */
struct ath_txq {
	u_int axq_qnum;		/* hardware q number */
	u_int32_t *axq_link;	/* link ptr in last TX desc */
	STAILQ_HEAD(, ath_buf) axq_q;	/* transmit queue */
	spinlock_t axq_lock;	/* lock on q and link */
	int axq_depth;		/* queue depth */
	u_int32_t axq_totalqueued;	/* total ever queued */
	u_int axq_intrcnt;	/* count to determine if descriptor
				 * should generate int on this txq.
				 */
	/*
	 * Staging queue for frames awaiting a fast-frame pairing.
	 */
	TAILQ_HEAD(axq_headtype, ath_buf) axq_stageq;

	/* scratch compression buffer */
	char *axq_compbuf;	/* scratch comp buffer */
	dma_addr_t axq_compbufp;	/* scratch comp buffer (phys) */
	u_int axq_compbufsz;	/* scratch comp buffer size */
};

/* driver-specific vap state */
struct ath_vap {
	struct ieee80211vap av_vap;	/* base class */
	int (*av_newstate)(struct ieee80211vap *, enum ieee80211_state, int);
	/* XXX beacon state */
	struct ath_buf *av_bcbuf;	/* beacon buffer */
	struct ieee80211_beacon_offsets av_boff;	/* dynamic update state */
	int av_bslot;		/* beacon slot index */
	struct ath_txq av_mcastq;	/* multicast transmit queue */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
	atomic_t av_beacon_alloc;	/* set to 1 when the next beacon needs
					   to be recomputed */
#else
	unsigned int av_beacon_alloc;
#endif
#ifdef HAVE_WPROBE
	struct wprobe_iface av_wpif;
#endif
	u32 av_rxframes;
	u32 av_rxprobereq;
};
#define	ATH_VAP(_v)	((struct ath_vap *)(_v))

#define	ATH_BEACON_AIFS_DEFAULT		0	/* Default aifs for ap beacon q */
#define	ATH_BEACON_CWMIN_DEFAULT	0	/* Default cwmin for ap beacon q */
#define	ATH_BEACON_CWMAX_DEFAULT	0	/* Default cwmax for ap beacon q */

#define ATH_TXQ_INTR_PERIOD		5	/* axq_intrcnt period for intr gen */
#define	ATH_TXQ_LOCK_INIT(_tq)		spin_lock_init(&(_tq)->axq_lock)
#define	ATH_TXQ_LOCK_DESTROY(_tq)
#define ATH_TXQ_LOCK_IRQ(_tq)		do {				\
	unsigned long __axq_lockflags;					\
	ATH_TXQ_LOCK_CHECK(_tq); 					\
	spin_lock_irqsave(&(_tq)->axq_lock, __axq_lockflags);
#define ATH_TXQ_UNLOCK_IRQ(_tq)						\
	ATH_TXQ_LOCK_ASSERT(_tq); 					\
	spin_unlock_irqrestore(&(_tq)->axq_lock, __axq_lockflags);	\
} while (0)
#define ATH_TXQ_UNLOCK_IRQ_EARLY(_tq)					\
	ATH_TXQ_LOCK_ASSERT(_tq); 					\
	spin_unlock_irqrestore(&(_tq)->axq_lock, __axq_lockflags);
#define ATH_TXQ_LOCK_IRQ_INSIDE(_tq)   do { 				\
	ATH_TXQ_LOCK_CHECK(_tq); 					\
	spin_lock(&(_tq)->axq_lock); 					\
} while(0)
#define ATH_TXQ_UNLOCK_IRQ_INSIDE(_tq) do { 				\
	ATH_TXQ_LOCK_ASSERT(_tq);  					\
	spin_unlock(&(_tq)->axq_lock);					\
} while(0)

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ATH_TXQ_LOCK_ASSERT(_tq) \
	KASSERT(spin_is_locked(&(_tq)->axq_lock), ("txq not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ATH_TXQ_LOCK_CHECK(_tq) do { \
	if (spin_is_locked(&(_tq)->axq_lock)) \
		printk(KERN_DEBUG "%s:%d - about to block on txq lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ATH_TXQ_LOCK_CHECK(_tq)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	ATH_TXQ_LOCK_ASSERT(_tq)
#define	ATH_TXQ_LOCK_CHECK(_tq)
#endif

#define ATH_TXQ_INSERT_TAIL(_tq, _elm, _field) do { \
	STAILQ_INSERT_TAIL(&(_tq)->axq_q, (_elm), _field); \
	(_tq)->axq_depth++; \
	(_tq)->axq_totalqueued++; \
} while (0)
#define ATH_TXQ_REMOVE_HEAD(_tq, _field) do { \
	STAILQ_REMOVE_HEAD(&(_tq)->axq_q, _field); \
	if (--(_tq)->axq_depth <= 0) \
		(_tq)->axq_link = NULL; \
} while (0)
/* move buffers from MCASTQ to CABQ */
#define ATH_TXQ_MOVE_MCASTQ(_tqs,_tqd) do { \
	(_tqd)->axq_depth += (_tqs)->axq_depth; \
	(_tqd)->axq_totalqueued += (_tqs)->axq_totalqueued; \
	(_tqd)->axq_link = (_tqs)->axq_link; \
	STAILQ_CONCAT(&(_tqd)->axq_q, &(_tqs)->axq_q); \
	(_tqs)->axq_depth=0; \
	(_tqs)->axq_totalqueued = 0; \
	(_tqs)->axq_link = NULL; \
} while (0)

/*
 * Definitions for pulling the rate and trie counts from
 * a 5212 h/w descriptor. These Don't belong here; the
 * driver should record this information so the rate control
 * code doesn't go groveling around in the descriptor bits.
 */
#define	ds_ctl2	ds_hw[0]
#define	ds_ctl3	ds_hw[1]

/* TX ds_ctl3 */
#define	AR_XmitDataTries0	0x000f0000	/* series 0 max attempts */
#define	AR_XmitDataTries0_S	16
#define	AR_XmitDataTries1	0x00f00000	/* series 1 max attempts */
#define	AR_XmitDataTries1_S	20
#define	AR_XmitDataTries2	0x0f000000	/* series 2 max attempts */
#define	AR_XmitDataTries2_S	24
#define	AR_XmitDataTries3	0xf0000000	/* series 3 max attempts */
#define	AR_XmitDataTries3_S	28

/* TX ds_ctl3 */
#define	AR_XmitRate0		0x0000001f	/* series 0 tx rate */
#define	AR_XmitRate0_S		0
#define	AR_XmitRate1		0x000003e0	/* series 1 tx rate */
#define	AR_XmitRate1_S		5
#define	AR_XmitRate2		0x00007c00	/* series 2 tx rate */
#define	AR_XmitRate2_S		10
#define	AR_XmitRate3		0x000f8000	/* series 3 tx rate */
#define	AR_XmitRate3_S		15

#define AR5416_XmitRate0        0x000000ff
#define AR5416_XmitRate0_S      0
#define AR5416_XmitRate1        0x0000ff00
#define AR5416_XmitRate1_S      8
#define AR5416_XmitRate2        0x00ff0000
#define AR5416_XmitRate2_S      16
#define AR5416_XmitRate3        0xff000000
#define AR5416_XmitRate3_S      24

#define MS(_v, _f)	(((_v) & (_f)) >> _f##_S)

/* 
 * concat buffers from one queue to other
 */
#define ATH_TXQ_MOVE_Q(_tqs,_tqd)  ATH_TXQ_MOVE_MCASTQ(_tqs,_tqd)

#define	BSTUCK_THRESH	10	/* # of stuck beacons before resetting NB: this is a guess */

struct ath_rp {
	struct list_head list;
	u_int64_t rp_tsf;
	u_int8_t rp_rssi;
	u_int8_t rp_width;

	int rp_index;
	int rp_allocated;
	int rp_analyzed;
};

struct ath_timings {
	u_int slot;
	u_int ack;
	u_int cts;
	u_int sifs;
	u_int difs;
	u_int eifs;
};

struct ath_dynack {
#define	ATH_DYNACK_STAT_MAX			32
	struct {
		unsigned short ds_retries[ATH_DYNACK_STAT_MAX];
		unsigned short ds_ack[ATH_DYNACK_STAT_MAX];
		short ds_idx;
	} da_stats;

	unsigned int da_checkcount;
	unsigned int da_delay_period;
	unsigned int da_max_ack;

	unsigned int da_packet_count;

	unsigned int da_inc;
	unsigned int da_dec;
	unsigned int da_stay;

	short da_prevaction;
	short da_sameaction;
	unsigned long da_lasttime;
	int da_delay;
	short da_step;
};
/* UBNT End */

struct ath_softc {
	struct ieee80211com sc_ic;	/* NB: must be first */
	struct net_device *sc_dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	struct napi_struct sc_napi;
#endif
	void __iomem *sc_iobase;	/* address of the device */
	struct semaphore sc_lock;	/* dev-level lock */
	struct net_device_stats sc_devstats;	/* device statistics */
	struct ath_stats sc_stats;	/* private statistics */
	int devid;
	int sc_debug;
	u_int sc_poweroffset;	/* hardware power offset */
	int sc_vendor;
	int sc_dev_vendor;
	int sc_dev_device;
	int sc_default_ieee80211_debug;	/* default debug flags for new VAPs */
	void (*sc_recv_mgmt)(struct ieee80211vap *, struct ieee80211_node *, struct sk_buff *, int, int, u_int64_t);
#ifdef IEEE80211_DEBUG_REFCNT
	void (*sc_node_cleanup_debug)(struct ieee80211_node *, const char *func, int line);
	void (*sc_node_free_debug)(struct ieee80211_node *, const char *func, int line);
#else				/* #ifdef IEEE80211_DEBUG_REFCNT */
	void (*sc_node_cleanup)(struct ieee80211_node *);
	void (*sc_node_free)(struct ieee80211_node *);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */
	void *sc_bdev;		/* associated bus device */
	struct ath_hal *sc_ah;	/* Atheros HAL */
	spinlock_t sc_hal_lock;	/* hardware access lock */
	struct ath_ratectrl *sc_rc;	/* tx rate control support */
	struct ath_tx99 *sc_tx99;	/* tx99 support */
	const struct ath_hw_detect *sc_hwinfo;

	unsigned int sc_invalid:1;	/* being detached */
	unsigned int sc_mrretry:1;	/* multi-rate retry support */
	unsigned int sc_softled:1;	/* enable LED gpio status */
	unsigned int sc_splitmic:1;	/* split TKIP MIC keys */
	unsigned int sc_needmib:1;	/* enable MIB stats intr */
	unsigned int sc_hasdiversity:1;	/* rx diversity available */
	unsigned int sc_diversity:1;	/* enable rx diversity */
	unsigned int sc_olddiversity:1;	/* diversity setting before XR enable */
	unsigned int sc_hasveol:1;	/* tx VEOL support */
	unsigned int sc_hastpc:1;	/* per-packet TPC support */
	unsigned int sc_dturbo:1;	/* dynamic turbo capable */
	unsigned int sc_dturbo_switch:1;	/* turbo switch mode */
	unsigned int sc_dturbo_hold:1;	/* dynamic turbo hold state */
	unsigned int sc_rate_recn_state:1;	/* dynamic turbo state recmded by ratectrl */
	unsigned int sc_ignore_ar:1;	/* ignore AR during transition */
	unsigned int sc_ledstate:1;	/* LED on/off state */
	unsigned int sc_blinking:1;	/* LED blink operation active */
	unsigned int sc_beacons:1;	/* beacons running */
	unsigned int sc_hasbmask:1;	/* bssid mask support */
	unsigned int sc_mcastkey:1;	/* mcast key cache search */
	unsigned int sc_hastsfadd:1;	/* tsf adjust support */
	unsigned int sc_scanning:1;	/* scanning active */
	unsigned int sc_nostabeacons:1;	/* no beacons for station */
	unsigned int sc_xrgrppoll:1;	/* xr group polls are active */
	unsigned int sc_syncbeacon:1;	/* sync/resync beacon timers */
	unsigned int sc_hasclrkey:1;	/* CLR key supported */
	unsigned int sc_devstopped:1;	/* stopped due to of no tx bufs */
	unsigned int sc_stagbeacons:1;	/* use staggered beacons */
	unsigned int sc_dfswait:1;	/* waiting on channel for radar detect */
	unsigned int sc_ackrate_override:1;	/* override ack rate */
	unsigned int sc_ackrate:1;	/* send acks at high bitrate */
	unsigned int sc_dfs_cac:1;	/* waiting on channel for radar detect */
	unsigned int sc_hasintmit:1;	/* Interference mitigation */
	unsigned int sc_txcont:1;	/* Is continuous transmit enabled? */
	unsigned int sc_dfs_testmode:1;	/* IF this is on, AP vaps will stay in
					 * 'channel availability check' indefinately,
					 * reporting radar and interference detections.
					 */
	unsigned int sc_silent:1;	/* Turn RF silent */
	unsigned int sc_txcont_power;	/* Continuous transmit power in 0.5dBm units */
	unsigned int sc_txcont_rate;	/* Continuous transmit rate in Mbps */
	unsigned int sc_rxmon;
	struct timer_list sc_rxmon_timer;

	int8_t sc_intmit;	/* Interference mitigation enabled, -1 = auto, based on mode, 0/1 = off/on */
	int8_t sc_noise_immunity;	/* Noise immunity level, 0-4, -1 == auto) */
	int8_t sc_ofdm_weak_det;	/* OFDM weak frames detection, -1 == auto */

	/* rate tables */
#define ATH_MODE_HALF		(IEEE80211_MODE_MAX)
#define ATH_MODE_QUARTER	(IEEE80211_MODE_MAX + 1)
#define ATH_MODE_SUBQUARTER	(IEEE80211_MODE_MAX + 2)
	const HAL_RATE_TABLE *sc_rates[IEEE80211_MODE_MAX + 3];
	const HAL_RATE_TABLE *sc_currates;	/* current rate table */
	const HAL_RATE_TABLE *sc_xr_rates;	/* XR rate table */
	HAL_OPMODE sc_opmode;	/* current hal operating mode */
	enum ieee80211_phymode sc_curmode;	/* current phy mode */
	u_int16_t sc_curtxpow;	/* current tx power limit */
	u_int16_t sc_curaid;	/* current association id */
	HAL_CHANNEL sc_curchan;	/* current h/w channel */
	u_int8_t sc_chanbw;	/* channel bandwidth */
	u_int8_t sc_chanshift;	/* channel shift in mhz */
	u_int8_t sc_superch;	/* superchannel */
	u_int8_t sc_curbssid[IEEE80211_ADDR_LEN];
	u_int8_t sc_rixmap[256];	/* IEEE to h/w rate table ix */
	struct {
		u_int8_t ieeerate;	/* IEEE rate */
		u_int8_t flags;	/* radiotap flags */
		u_int16_t ledon;	/* softled on time */
		u_int16_t ledoff;	/* softled off time */
	} sc_hwmap[32];		/* h/w rate ix mappings */
	u_int8_t sc_minrateix;	/* min h/w rate index */
	u_int8_t sc_protrix;	/* protection rate index */
	u_int8_t sc_mcastantenna;	/* Multicast antenna number */
	u_int8_t sc_txantenna;	/* data tx antenna (fixed or auto) */
	u_int16_t sc_nvaps;	/* # of active virtual APs */
	u_int8_t sc_nstavaps;	/* # of active station VAPs */
	u_int8_t sc_nmonvaps;	/* # of monitor VAPs */
	u_int8_t sc_nbcnvaps;	/* # of vaps sending beacons */
	u_int sc_fftxqmin;	/* aggregation threshold */
	HAL_INT sc_imask;	/* interrupt mask copy */
	u_int sc_keymax;	/* size of key cache */
	u_int8_t sc_keymap[ATH_KEYBYTES];	/* key use bit map */
	struct ieee80211_node *sc_keyixmap[ATH_KEYMAX];	/* key ix->node map */
	u_int8_t sc_bssidmask[IEEE80211_ADDR_LEN];

	u_int sc_ledpin;	/* GPIO pin for driving LED */
	u_int sc_ledon;		/* pin setting for LED on */
	u_int sc_ledidle;	/* idle polling interval */
	int sc_ledevent;	/* time of last LED event */
	u_int8_t sc_rxrate;	/* current rx rate for LED */
	u_int8_t sc_txrate;	/* current tx rate for LED */
	u_int16_t sc_ledoff;	/* off time for current blink */
	struct timer_list sc_ledtimer;	/* led off timer */

	/* beacon watchdog timer */
	u_int32_t sc_bcntimer_reload;
	struct timer_list sc_bcntimer;

	struct ATH_TQ_STRUCT sc_fataltq;	/* fatal error intr tasklet */

	int sc_rxbufsize;	/* rx size based on mtu */
	struct ath_descdma sc_rxdma;	/* RX descriptors */
	ath_bufhead sc_rxbuf;	/* receive buffer */
	struct ath_buf *sc_rxbufcur;	/* current rx buffer */
	u_int32_t *sc_rxlink;	/* link ptr in last RX desc */
	spinlock_t sc_rxbuflock;
	u_int8_t sc_defant;	/* current default antenna */
	u_int8_t sc_rxotherant;	/* RXs on non-default antenna */
	u_int16_t sc_cachelsz;	/* cache line size */

	struct ath_descdma sc_txdma;	/* TX descriptors */
	ath_bufhead sc_txbuf;	/* TX buffers pool */
	atomic_t sc_txbuf_counter;	/* number of available TX
					 * buffers */
	spinlock_t sc_txbuflock;	/* txbuf lock */
	u_int sc_txqsetup;	/* h/w queues setup */
	u_int sc_txintrperiod;	/* tx interrupt batching */
	struct ath_txq sc_txq[HAL_NUM_TX_QUEUES];
	struct ath_txq *sc_ac2q[WME_NUM_AC];	/* WME AC -> h/w qnum */
	HAL_INT sc_isr;		/* unmasked ISR state */
	struct ATH_TQ_STRUCT sc_txtq;	/* tx intr tasklet */
	u_int8_t sc_grppoll_str[GRPPOLL_RATE_STR_LEN];
	struct ath_descdma sc_bdma;	/* beacon descriptors */
	ath_bufhead sc_bbuf;	/* beacon buffers */
	u_int sc_bhalq;		/* HAL q for outgoing beacons */
	u_int sc_bmisscount;	/* missed beacon transmits */
	u_int32_t sc_ant_tx[8];	/* recent tx frames/antenna */
	struct ath_txq *sc_cabq;	/* tx q for cab frames */
	struct ath_txq sc_grpplq;	/* tx q for XR group polls */
	struct ath_txq *sc_xrtxq;	/* tx q for XR data */
	struct ath_descdma sc_grppolldma;	/* TX descriptors for grppoll */
	ath_bufhead sc_grppollbuf;	/* transmit buffers for grouppoll  */
	u_int16_t sc_xrpollint;	/* xr poll interval */
	u_int16_t sc_xrpollcount;	/* xr poll count */
	struct ath_txq *sc_uapsdq;	/* tx q for uapsd */
	struct ATH_TQ_STRUCT sc_bmisstq;	/* bmiss intr tasklet */
	struct ATH_TQ_STRUCT sc_bstucktq;	/* beacon stuck intr tasklet */
	enum {
		OK,		/* no change needed */
		UPDATE,		/* update pending */
		COMMIT		/* beacon sent, commit change */
	} sc_updateslot;	/* slot time update fsm */
	int sc_slotupdate;	/* slot to next advance fsm */
	struct ieee80211vap **sc_bslot;	/* beacon xmit slots */
	int sc_bnext;		/* next slot for beacon xmit */

	struct ieee80211_channel *sc_last_chan;
	int sc_beacon_cal;	/* use beacon timer for calibration */
	u_int64_t sc_lastlongcal;	/* last long cal completed */
	u_int64_t sc_nextcal;	/* last time the calibration was performed */
	int sc_cal_interval;	/* current calibration interval */
	struct timer_list sc_cal_ch;	/* calibration timer */
	HAL_NODE_STATS sc_halstats;	/* station-mode rssi stats */
	int sc_cca_thresh;	/* configured CCA threshold */

	struct ctl_table_header *sc_sysctl_header;
	struct ctl_table *sc_sysctls;

	u_int16_t sc_reapcount;	/* # of tx buffers reaped after net dev stopped */
	struct timer_list sc_mib_enable;

#ifdef ATH_REVERSE_ENGINEERING
	u_int8_t register_snapshot[MAX_REGISTER_ADDRESS];
#endif				/* #ifdef ATH_REVERSE_ENGINEERING */

#ifdef ATH_SUPERG_DYNTURBO
	struct timer_list sc_dturbo_switch_mode;	/* AP scan timer */
	u_int32_t sc_dturbo_tcount;	/* beacon intval count */
	u_int32_t sc_dturbo_hold_max;	/* hold count before switching to base */
	u_int16_t sc_dturbo_hold_count;	/* hold count before switching to base */
	u_int16_t sc_dturbo_turbo_tmin;	/* min turbo count */
	u_int32_t sc_dturbo_bytes;	/* bandwidth stats */
	u_int32_t sc_dturbo_base_tmin;	/* min time in base */
	u_int32_t sc_dturbo_turbo_tmax;	/* max time in turbo */
	u_int32_t sc_dturbo_bw_base;	/* bandwidth threshold */
	u_int32_t sc_dturbo_bw_turbo;	/* bandwidth threshold */
#endif
	struct ath_dynack sc_dynack;	/* data for dynamic ack implementation */
	u_int sc_slottimeconf;	/* manual override for slottime */
	u_int sc_acktimeconf;	/* manual override for acktime */
	u_int sc_ctstimeconf;	/* manual override for ctstime */

	struct timer_list sc_dfs_excl_timer;	/* mark expiration timer task */
	struct timer_list sc_dfs_cac_timer;	/* dfs wait timer */
	u_int32_t sc_dfs_cac_period;	/* DFS wait time before accessing a
					 * channel (in seconds). FCC 
					 * requires 60s. */
	u_int32_t sc_dfs_excl_period;	/* DFS channel non-occupancy limit
					 * after radar is detected (in seconds).
					 * FCC requires 30m. */
	u_int64_t sc_rp_lasttsf;	/* TSF at last detected radar pulse */

	struct ath_rp *sc_rp;	/* radar pulse circular array */
	struct list_head sc_rp_list;
	int sc_rp_num;
	int sc_rp_min;
	HAL_BOOL (*sc_rp_analyse)(struct ath_softc * sc);
	struct ATH_TQ_STRUCT sc_refresh_tq;
	struct ATH_TQ_STRUCT sc_rp_tq;

	int sc_rp_ignored;	/* if set, we ignored all 
				 * received pulses */
	int sc_radar_ignored;	/* if set, we ignored all 
				 * detected radars */
	u_int32_t sc_nexttbtt;
	u_int64_t sc_last_tsf;
	u_int8_t sc_antgain;
	u_int8_t sc_antgainsub;
	u_int sc_coverage;
	struct ath_timings sc_timings;
};

typedef void (*ath_callback)(struct ath_softc *);
#define	ATH_TXQ_SETUP(sc, i)	((sc)->sc_txqsetup & (1 << i))

#define	ATH_TXBUF_LOCK_INIT(_sc)	spin_lock_init(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_LOCK_DESTROY(_sc)
#define	ATH_TXBUF_LOCK_IRQ(_sc)		do {	\
	unsigned long __txbuflockflags;		\
	ATH_TXBUF_LOCK_CHECK(_sc);		\
	spin_lock_irqsave(&(_sc)->sc_txbuflock, __txbuflockflags);
#define	ATH_TXBUF_UNLOCK_IRQ(_sc)		\
	ATH_TXBUF_LOCK_ASSERT(_sc);		\
	spin_unlock_irqrestore(&(_sc)->sc_txbuflock, __txbuflockflags); \
} while (0)
#define	ATH_TXBUF_UNLOCK_IRQ_EARLY(_sc)		\
	ATH_TXBUF_LOCK_ASSERT(_sc);		\
	spin_unlock_irqrestore(&(_sc)->sc_txbuflock, __txbuflockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ATH_TXBUF_LOCK_ASSERT(_sc) \
	KASSERT(spin_is_locked(&(_sc)->sc_txbuflock), ("txbuf not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ATH_TXBUF_LOCK_CHECK(_sc) do { \
	if (spin_is_locked(&(_sc)->sc_txbuflock)) \
		printk(KERN_DEBUG "%s:%d - about to block on txbuf lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ATH_TXBUF_LOCK_CHECK(_sc)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	ATH_TXBUF_LOCK_ASSERT(_sc)
#define	ATH_TXBUF_LOCK_CHECK(_sc)
#endif

#define ATH_DISABLE_INTR		local_irq_disable
#define ATH_ENABLE_INTR 		local_irq_enable

#define	ATH_RXBUF_LOCK_INIT(_sc)	spin_lock_init(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_LOCK_DESTROY(_sc)
#define	ATH_RXBUF_LOCK_IRQ(_sc)		do {	\
	unsigned long __rxbuflockflags;		\
	ATH_RXBUF_LOCK_CHECK(_sc); 		\
	spin_lock_irqsave(&(_sc)->sc_rxbuflock, __rxbuflockflags);
#define	ATH_RXBUF_UNLOCK_IRQ(_sc)		\
	ATH_RXBUF_LOCK_ASSERT(_sc); 		\
	spin_unlock_irqrestore(&(_sc)->sc_rxbuflock, __rxbuflockflags); \
} while (0)
#define	ATH_RXBUF_UNLOCK_IRQ_EARLY(_sc)		\
	ATH_RXBUF_LOCK_ASSERT(_sc); 		\
	spin_unlock_irqrestore(&(_sc)->sc_rxbuflock, __rxbuflockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ATH_RXBUF_LOCK_ASSERT(_sc) \
	KASSERT(spin_is_locked(&(_sc)->sc_rxbuflock), ("rxbuf not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ATH_RXBUF_LOCK_CHECK(_sc) do { \
	if (spin_is_locked(&(_sc)->sc_rxbuflock)) \
		printk(KERN_DEBUG "%s:%d - about to block on rxbuf lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ATH_RXBUF_LOCK_CHECK(_sc)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	ATH_RXBUF_LOCK_ASSERT(_sc)
#define	ATH_RXBUF_LOCK_CHECK(_sc)
#endif

/* Protects the device from concurrent accesses */
#define	ATH_LOCK_INIT(_sc)		sema_init(&(_sc)->sc_lock,1)
#define	ATH_LOCK_DESTROY(_sc)
#define	ATH_LOCK(_sc)			down(&(_sc)->sc_lock)
#define	ATH_UNLOCK(_sc)			up(&(_sc)->sc_lock)

int ath_attach(u_int16_t, struct net_device *, HAL_BUS_TAG);
int ath_detach(struct net_device *);
void ath_resume(struct net_device *);
void ath_suspend(struct net_device *);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
irqreturn_t ath_intr(int, void *);
#else
irqreturn_t ath_intr(int, void *, struct pt_regs *);
#endif
int ath_ioctl_ethtool(struct ath_softc *, int, void __user *);
void bus_read_cachesize(struct ath_softc *, u_int8_t *);
void ath_sysctl_register(void);
void ath_sysctl_unregister(void);
int ar_device(int devid);

#define DEV_NAME(_d) \
	 ((NULL == _d || NULL == _d->name || 0 == strncmp(_d->name, "wifi%d", 6)) ? \
	  "MadWifi" : \
	  _d->name)
#define VAP_DEV_NAME(_v) \
	 ((NULL == _v) ? \
	  "MadWifi" : \
	  DEV_NAME(_v->iv_dev))
#define SC_DEV_NAME(_sc) \
	 ((NULL == _sc) ? \
	  "MadWifi" : \
	  DEV_NAME(_sc->sc_dev))
#define VAP_IC_DEV_NAME(_v) \
	 ((NULL == _v || NULL == _v->iv_ic) ? \
	  "MadWifi" : \
	  DEV_NAME(_v->iv_ic->ic_dev))

void ath_radar_detected(struct ath_softc *sc, const char *message);

#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#endif

/* Calculate the transmit duration of a frame. */
static inline unsigned calc_usecs_unicast_packet(struct ath_softc *sc, int length, int rix, int short_retries, int long_retries)
{
	const HAL_RATE_TABLE *rt = sc->sc_currates;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ath_timings *t = &sc->sc_timings;
	unsigned int x = 0, tt = 0;
	unsigned int cix = rt->info[rix].controlRate;
	int rts = 0, cts = 0;
	int cw = ATH_DEFAULT_CWMIN;

	KASSERT(rt != NULL, ("no rate table, mode %u", sc->sc_curmode));

	if (!rt->info[rix].rateKbps) {
		printk(KERN_WARNING "rix %d (%d) bad ratekbps %d mode %u\n", rix, rt->info[rix].dot11Rate, rt->info[rix].rateKbps, sc->sc_curmode);
		return 0;
	}

	if ((ic->ic_flags & IEEE80211_F_USEPROT) && (rt->info[rix].phy == IEEE80211_T_OFDM)) {

		if (ic->ic_protmode == IEEE80211_PROT_RTSCTS)
			rts = 1;
		else if (ic->ic_protmode == IEEE80211_PROT_CTSONLY)
			cts = 1;

		cix = rt->info[sc->sc_protrix].controlRate;
	}

	if ((rts || cts) && rt->info[cix].rateKbps) {
		int ctsrate = rt->info[cix].rateCode;
		int ctsduration = 0;

		ctsrate |= rt->info[cix].shortPreamble;
		if (rts)	/* SIFS + CTS */
			ctsduration += rt->info[cix].spAckDuration;

		ctsduration += ath_hal_computetxtime(sc->sc_ah, rt, length, rix, AH_TRUE);

		if (cts)	/* SIFS + ACK */
			ctsduration += rt->info[cix].spAckDuration;

		tt += (short_retries + 1) * ctsduration;
	}
	tt += t->difs;
	tt += (long_retries + 1) * (t->sifs + rt->info[rix].spAckDuration);
	tt += (long_retries + 1) * ath_hal_computetxtime(sc->sc_ah, rt, length, rix, AH_TRUE);
	for (x = 0; x <= short_retries + long_retries; x++) {
		cw = MIN(ATH_DEFAULT_CWMAX, (cw << 1) | 1);
		tt += (t->slot * cw / 2);
	}
	return tt;
}

struct ath_hw_detect {
	const char *vendor_name;
	const char *card_name;
	u32 vendor;
	u32 id;
	u32 subvendor;
	u32 subid;
	u32 poweroffset;
};

extern void ath_hw_detect(struct ath_softc *sc, const struct ath_hw_detect *cards, int n_cards, u32 vendor, u32 id, u32 subvendor, u32 subid);

#endif				/* _DEV_ATH_ATHVAR_H */
