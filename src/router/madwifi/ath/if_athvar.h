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
 * $Id: if_athvar.h 1520 2006-04-21 16:57:59Z dyqith $
 */

/*
 * Defintions for the Atheros Wireless LAN controller driver.
 */
#ifndef _DEV_ATH_ATHVAR_H
#define _DEV_ATH_ATHVAR_H

#include "ah.h"
#include "if_athioctl.h"
#include "net80211/ieee80211.h"		/* XXX for WME_NUM_AC */

/*
 * Deduce if tasklets are available.  If not then
 * fall back to using the immediate work queue.
 */
#include <linux/interrupt.h>
#ifdef DECLARE_TASKLET			/* native tasklets */
#define ATH_TQ_STRUCT tasklet_struct
#define ATH_INIT_TQUEUE(a,b,c)		tasklet_init((a),(b),(unsigned long)(c))
#define ATH_SCHEDULE_TQUEUE(a,b)	tasklet_schedule((a))
typedef unsigned long TQUEUE_ARG;
#define mark_bh(a) do {} while (0)
#else					/* immediate work queue */
#define ATH_TQ_STRUCT tq_struct
#define ATH_INIT_TQUEUE(a,b,c)		INIT_TQUEUE(a,b,c)
#define ATH_SCHEDULE_TQUEUE(a,b) do {		\
	*(b) |= queue_task((a), &tq_immediate);	\
} while(0)
typedef void *TQUEUE_ARG;
#define	tasklet_disable(t)	do { (void) t; local_bh_disable(); } while (0)
#define	tasklet_enable(t)	do { (void) t; local_bh_enable(); } while (0)
#endif /* !DECLARE_TASKLET */

#include <linux/sched.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,41)
#include <linux/tqueue.h>
#define ATH_WORK_THREAD			tq_struct
#define ATH_SCHEDULE_TASK(t)		schedule_task((t))
#define ATH_INIT_SCHED_TASK(t, f, d) do { 	\
	memset((t),0,sizeof(struct tq_struct)); 	\
	(t)->routine = (void (*)(void*)) (f); 	\
	(t)->data=(void *) (d); 		\
} while (0)
#define ATH_FLUSH_TASKS			flush_scheduled_tasks
#else
#include <linux/workqueue.h>
#define ATH_SCHEDULE_TASK(t)		schedule_work((t))

#define ATH_INIT_SCHED_TASK(_t, _f, _d)	INIT_WORK((_t), (void (*)(void *))(_f), (void *)(_d));

#define ATH_WORK_THREAD			work_struct
#define	ATH_FLUSH_TASKS			flush_scheduled_work
#endif /* KERNEL_VERSION < 2.5.41 */

/*
 * Guess how the interrupt handler should work.
 */
#if !defined(IRQ_NONE)
typedef void irqreturn_t;
#define	IRQ_NONE
#define	IRQ_HANDLED
#endif /* !defined(IRQ_NONE) */

#ifndef SET_MODULE_OWNER
#define	SET_MODULE_OWNER(dev) do {		\
	dev->owner = THIS_MODULE;		\
} while (0)
#endif

#ifndef SET_NETDEV_DEV
#define	SET_NETDEV_DEV(ndev, pdev)
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
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8) */
#define	ATH_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
	f(ctl_table *ctl, int write, struct file *filp, \
	  void __user *buffer, size_t *lenp, loff_t *ppos)
#define	ATH_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, filp, buffer, lenp, ppos)
#endif

#define	ATH_TIMEOUT	1000

#define ATH_DFS_WAIT_POLL_PERIOD	2	/* 2 seconds */
#define	ATH_DFS_TEST_RETURN_PERIOD	15	/* 15 seconds */

#define	ATH_LONG_CALINTERVAL		30	/* 30 seconds between calibrations */
#define	ATH_SHORT_CALINTERVAL		1	/* 1 second between calibrations */

/*
 * Maximum acceptable MTU
 * MAXFRAMEBODY - WEP - QOS - RSN/WPA:
 * 2312 - 8 - 2 - 12 = 2290
 */
#define ATH_MAX_MTU     2290
#define ATH_MIN_MTU     32  

#define	ATH_RXBUF	40		/* number of RX buffers */
#define	ATH_TXBUF	200		/* number of TX buffers */

#define	ATH_BCBUF	16		/* number of beacon buffers */

/* free buffer threshold to restart net dev */
#define	ATH_TXBUF_FREE_THRESHOLD  (ATH_TXBUF / 20) 

#define TAIL_DROP_COUNT 50             /* maximum number of queued frames allowed */

/*
 * dynamic turbo specific macros.
 */
#define ATH_TURBO_UP_THRESH	750000	/* bytes/sec */ 
#define ATH_TURBO_DN_THRESH 	1000000	/* bytes/sec */ 
#define ATH_TURBO_PERIOD_HOLD	1 	/* in seconds */ 

/*
 * The only case where we see skbuff chains is due to FF aggregation in
 * the driver.
 */
#ifdef ATH_SUPERG_FF
#define	ATH_TXDESC	2		/* number of descriptors per buffer */
#else
#define	ATH_TXDESC	1		/* number of descriptors per buffer */
#endif

#define	ATH_TXMAXTRY	11		/* max number of transmit attempts */

/* Compress settings */
#define ATH_COMP_THRESHOLD  256         /* no compression for frames
					   longer than this threshold  */
#define ATH_COMP_PROC_NO_COMP_NO_CCS    3
#define ATH_COMP_PROC_NO_COMP_ADD_CCS   2
#define ATH_COMP_PROC_COMP_NO_OPTIAML   1
#define ATH_COMP_PROC_COMP_OPTIMAL      0
#define ATH_DEFAULT_COMP_PROC           ATH_COMP_PROC_COMP_OPTIMAL

#define INVALID_DECOMP_INDEX		0xFFFF

#define WEP_IV_FIELD_SIZE       4       /* wep IV field size */
#define WEP_ICV_FIELD_SIZE      4       /* wep ICV field size */
#define AES_ICV_FIELD_SIZE      8       /* AES ICV field size */
#define EXT_IV_FIELD_SIZE       4       /* ext IV field size */

/* XR specific macros */

#define XR_DEFAULT_GRPPOLL_RATE_STR 	"0.25 1 1 3 3 6 6 20"
#define GRPPOLL_RATE_STR_LEN 	64 
#define XR_SLOT_DELAY		30      /* in usec */
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
 * Macros to obtain the Group Poll Periodicty in various situations
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
#define	ATH_KEYMAX	128		/* max key cache size we handle */
#define	ATH_KEYBYTES	(ATH_KEYMAX / NBBY)	/* storage space in bytes */
#define	ATH_MIN_FF_RATE	12000		/* min rate fof ff aggragattion.in Kbps  */
#define	ATH_MIN_FF_RATE	12000		/* min rate fof ff aggragattion.in Kbps  */
struct ath_buf;
typedef STAILQ_HEAD(, ath_buf) ath_bufhead;

/* driver-specific node state */
struct ath_node {
	struct ieee80211_node an_node;		/* base class */
	u_int16_t an_decomp_index; 		/* decompression mask index */
	u_int32_t an_avgrssi;			/* average rssi over all rx frames */
	u_int8_t  an_prevdatarix;		/* rate ix of last data frame */
	u_int16_t an_minffrate;			/* mimum rate in kbps for ff to aggragate */
	HAL_NODE_STATS an_halstats;		/* rssi statistics used by hal */
	struct ath_buf *an_tx_ffbuf[WME_NUM_AC]; /* ff staging area */
	ath_bufhead an_uapsd_q;			/* U-APSD delivery queue */
	int an_uapsd_qdepth; 			/* U-APSD delivery queue depth */
	ath_bufhead an_uapsd_overflowq; 	/* U-APSD overflow queue (for > MaxSp frames) */
	int an_uapsd_overflowqdepth; 		/* U-APSD overflow queue depth */
	spinlock_t an_uapsd_lock; 		/* U-APSD deleivery queue lock */
	/* variable-length rate control state follows */
};
#define	ATH_NODE(_n)			((struct ath_node *)(_n))
#define	ATH_NODE_CONST(ni)		((const struct ath_node *)(ni))
#define ATH_NODE_UAPSD_LOCK_INIT(_an)	spin_lock_init(&(_an)->an_uapsd_lock)
#define ATH_NODE_UAPSD_LOCK(_an) 	spin_lock(&(_an)->an_uapsd_lock)
#define ATH_NODE_UAPSD_UNLOCK(_an)	spin_unlock(&(_an)->an_uapsd_lock)
#define ATH_NODE_UAPSD_LOCK_IRQ(_an)	do {	\
	unsigned long __an_uapsd_lockflags;	\
	spin_lock_irqsave(&(_an)->an_uapsd_lock, __an_uapsd_lockflags);
#define ATH_NODE_UAPSD_UNLOCK_IRQ(_an)		\
	spin_unlock_irqrestore(&(_an)->an_uapsd_lock, __an_uapsd_lockflags); \
} while (0)
#define ATH_NODE_UAPSD_UNLOCK_IRQ_EARLY(_an)		\
	spin_unlock_irqrestore(&(_an)->an_uapsd_lock, __an_uapsd_lockflags);


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
	struct ath_desc	*bf_desc;		/* virtual addr of desc */
	dma_addr_t bf_daddr;			/* physical addr of desc */
	struct sk_buff *bf_skb;			/* skbuff for buf */
	dma_addr_t bf_skbaddr;			/* physical addr of skb data */
	struct ieee80211_node *bf_node;		/* pointer to the node */
	u_int32_t bf_status;			/* status flags */
#ifdef ATH_SUPERG_FF
	/* XXX: combine this with bf_skbaddr if it ever changes to accomodate
	 *      multiple segments.
	 */
	u_int32_t bf_queueage; 			/* "age" of txq when this buffer placed on stageq */
	u_int16_t bf_numdesc;			/* number of descs used */
	u_int16_t bf_flags;			/* tx descriptor flags */
	dma_addr_t bf_skbaddrff[ATH_TXDESC-1]; 	/* extra addrs for ff */
#endif
};

/*
 * reset the rx buffer.
 * any new fields added to the athbuf and require 
 * reset need to be added to this macro.
 * currently bf_status is the only one requires that
 * requires reset.
 */
#define ATH_RXBUF_RESET(bf)	bf->bf_status=0

/* XXX: only managed for rx at the moment */
#define ATH_BUFSTATUS_DONE	0x00000001	/* hw processing complete, desc processed by hal */

/*
 * DMA state for tx/rx descriptors.
 */
struct ath_descdma {
	const char *dd_name;
	struct ath_desc	*dd_desc;	/* descriptors */
	dma_addr_t dd_desc_paddr;	/* physical addr of dd_desc */
	size_t dd_desc_len;		/* size of dd_desc */
	struct ath_buf *dd_bufptr;	/* associated buffers */
};

struct ath_hal;
struct ath_desc;
struct ath_ratectrl;
struct ath_tx99;
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
	u_int axq_qnum;			/* hardware q number */
	u_int32_t *axq_link;		/* link ptr in last TX desc */
	STAILQ_HEAD(, ath_buf) axq_q;	/* transmit queue */
	spinlock_t axq_lock;		/* lock on q and link */
	int axq_depth;			/* queue depth */
	u_int32_t axq_totalqueued;	/* total ever queued */
	u_int axq_intrcnt;		/* count to determine if descriptor
					 * should generate int on this txq.
					 */
	/*
	 * Staging queue for frames awaiting a fast-frame pairing.
	 */
	TAILQ_HEAD(axq_headtype, ath_buf) axq_stageq;

        /* scratch compression buffer */
        char *axq_compbuf;		/* scratch comp buffer */
        dma_addr_t axq_compbufp;	/* scratch comp buffer (phys)*/
        u_int axq_compbufsz;		/* scratch comp buffer size */
};

/* driver-specific vap state */
struct ath_vap {
	struct ieee80211vap av_vap;	/* base class */
	int (*av_newstate)(struct ieee80211vap *, enum ieee80211_state, int);
	/* XXX beacon state */
	struct ath_buf *av_bcbuf;	/* beacon buffer */
	struct ieee80211_beacon_offsets av_boff;/* dynamic update state */
	int av_bslot;			/* beacon slot index */
	struct ath_txq av_mcastq;	/* multicast transmit queue */
	u_int8_t	av_dfswait_run;
};
#define	ATH_VAP(_v)	((struct ath_vap *)(_v))

#define	ATH_BEACON_AIFS_DEFAULT		0  /* Default aifs for ap beacon q */
#define	ATH_BEACON_CWMIN_DEFAULT	0  /* Default cwmin for ap beacon q */
#define	ATH_BEACON_CWMAX_DEFAULT	0  /* Default cwmax for ap beacon q */

#define ATH_TXQ_INTR_PERIOD		5  /* axq_intrcnt period for intr gen */
#define	ATH_TXQ_LOCK_INIT(_tq)		spin_lock_init(&(_tq)->axq_lock)
#define	ATH_TXQ_LOCK_DESTROY(_tq)	
#define	ATH_TXQ_LOCK(_tq)		spin_lock(&(_tq)->axq_lock)
#define	ATH_TXQ_UNLOCK(_tq)		spin_unlock(&(_tq)->axq_lock)
#define	ATH_TXQ_LOCK_BH(_tq)		spin_lock_bh(&(_tq)->axq_lock)
#define	ATH_TXQ_UNLOCK_BH(_tq)		spin_unlock_bh(&(_tq)->axq_lock)
#define ATH_TXQ_LOCK_IRQ(_tq)		do {	\
	unsigned long __axq_lockflags;		\
	spin_lock_irqsave(&(_tq)->axq_lock, __axq_lockflags);
#define ATH_TXQ_UNLOCK_IRQ(_tq)			\
	spin_unlock_irqrestore(&(_tq)->axq_lock, __axq_lockflags); \
} while (0)
#define ATH_TXQ_UNLOCK_IRQ_EARLY(_tq)			\
	spin_unlock_irqrestore(&(_tq)->axq_lock, __axq_lockflags);

#define ATH_TXQ_UAPSDQ_LOCK_IRQ(_tq)	spin_lock_irqsave(&(_tq)->axq_lock, uapsdq_lockflags)
#define ATH_TXQ_UAPSDQ_UNLOCK_IRQ(_tq)	spin_unlock_irqrestore(&(_tq)->axq_lock, uapsdq_lockflags)




#define	ATH_TXQ_LOCK_ASSERT(_tq) \
	KASSERT(spin_is_locked(&(_tq)->axq_lock), ("txq not locked!"))
#define ATH_TXQ_INSERT_TAIL(_tq, _elm, _field) do { \
	STAILQ_INSERT_TAIL( &(_tq)->axq_q, (_elm), _field); \
	(_tq)->axq_depth++; \
	(_tq)->axq_totalqueued++; \
} while (0)
#define ATH_TXQ_REMOVE_HEAD(_tq, _field) do { \
	STAILQ_REMOVE_HEAD(&(_tq)->axq_q, _field); \
	(_tq)->axq_depth--; \
} while (0)
/* move buffers from MCASTQ to CABQ */
#define ATH_TXQ_MOVE_MCASTQ(_tqs,_tqd) do { \
	(_tqd)->axq_depth += (_tqs)->axq_depth; \
	(_tqd)->axq_totalqueued += (_tqs)->axq_totalqueued; \
	(_tqd)->axq_link = (_tqs)->axq_link; \
	STAILQ_CONCAT(&(_tqd)->axq_q,&(_tqs)->axq_q); \
	(_tqs)->axq_depth=0; \
	(_tqs)->axq_totalqueued = 0; \
	(_tqs)->axq_link = NULL; \
} while (0)

/* 
 * concat buffers from one queue to other
 */
#define ATH_TXQ_MOVE_Q(_tqs,_tqd)  ATH_TXQ_MOVE_MCASTQ(_tqs,_tqd)

#define	BSTUCK_THRESH	3	/* # of stuck beacons before resetting NB: this is a guess*/

struct ath_softc {
	struct ieee80211com sc_ic;		/* NB: must be first */
	struct net_device *sc_dev;
	struct semaphore sc_lock;		/* dev-level lock */
	struct net_device_stats	sc_devstats;	/* device statistics */
	struct ath_stats	sc_stats;		/* private statistics */
	int devid;
	int sc_debug;
	void (*sc_recv_mgmt)(struct ieee80211_node *, struct sk_buff *, int, int, u_int32_t);
	void (*sc_node_cleanup)(struct ieee80211_node *);
	void (*sc_node_free)(struct ieee80211_node *);
	void *sc_bdev;				/* associated bus device */
	struct ath_hal *sc_ah;			/* Atheros HAL */
	struct ath_ratectrl *sc_rc;		/* tx rate control support */
	struct ath_tx99 *sc_tx99; 		/* tx99 support */
	void (*sc_setdefantenna)(struct ath_softc *, u_int);
	unsigned int 	sc_invalid:1,		/* being detached */
			sc_mrretry:1,		/* multi-rate retry support */
			sc_softled:1,		/* enable LED gpio status */
			sc_splitmic:1,		/* split TKIP MIC keys */
			sc_needmib:1,		/* enable MIB stats intr */
			sc_hasdiversity:1,	/* rx diversity available */
			sc_diversity:1, 	/* enable rx diversity */
			sc_olddiversity:1, 	/* diversity setting before XR enable */	       
			sc_hasveol:1,		/* tx VEOL support */
			sc_hastpc:1,		/* per-packet TPC support */
			sc_dturbo:1,		/* dynamic turbo capable */
			sc_dturbo_switch:1,	/* turbo switch mode*/
			sc_dturbo_hold:1,	/* dynamic turbo hold state */
			sc_rate_recn_state:1,	/* dynamic turbo state recmded by ratectrl */
			sc_ignore_ar:1,		/* ignore AR during transision*/
			sc_ledstate:1,		/* LED on/off state */
			sc_blinking:1,		/* LED blink operation active */
			sc_beacons:1,		/* beacons running */
			sc_hasbmask:1,		/* bssid mask support */
			sc_mcastkey:1,		/* mcast key cache search */
			sc_hastsfadd:1,		/* tsf adjust support */
			sc_scanning:1,		/* scanning active */
			sc_nostabeacons:1,	/* no beacons for station */
			sc_xrgrppoll:1,		/* xr group polls are active */
			sc_syncbeacon:1,	/* sync/resync beacon timers */
			sc_hasclrkey:1,		/* CLR key supported */
			sc_devstopped:1,	/* stopped due to of no tx bufs */
			sc_stagbeacons:1,	/* use staggered beacons */
			sc_rtasksched:1, 	/* radar task is scheduled */
			sc_dfswait:1,    	/* waiting on channel for radar detect */
			sc_dfstest:1,		/* Test timer in progress */
		        sc_ackrate:1;           /* send acks at high bitrate */
	/* rate tables */
	const HAL_RATE_TABLE *sc_rates[IEEE80211_MODE_MAX];
	const HAL_RATE_TABLE *sc_currates;	/* current rate table */
	const HAL_RATE_TABLE *sc_xr_rates;	/* XR rate table */
	const HAL_RATE_TABLE *sc_half_rates;	/* half rate table */
	const HAL_RATE_TABLE *sc_quarter_rates;	/* quarter rate table */
	HAL_OPMODE sc_opmode;			/* current hal operating mode */
	enum ieee80211_phymode sc_curmode;	/* current phy mode */
	u_int16_t sc_curtxpow;			/* current tx power limit */
	u_int16_t sc_curaid;			/* current association id */
	HAL_CHANNEL sc_curchan;			/* current h/w channel */
	u_int8_t sc_curbssid[IEEE80211_ADDR_LEN];
	u_int8_t	sc_rixmap[256];			/* IEEE to h/w rate table ix */
	
	struct {
		u_int8_t	ieeerate;		/* IEEE rate */
		u_int8_t	flags;			/* radiotap flags */
		u_int16_t ledon;			/* softled on time */
		u_int16_t ledoff;		/* softled off time */
	} sc_hwmap[32];				/* h/w rate ix mappings */
	u_int8_t sc_minrateix;			/* min h/w rate index */
	u_int8_t sc_protrix;			/* protection rate index */
	u_int8_t sc_mcastantenna;		/* Multicast antenna number */
	u_int8_t sc_txantenna;			/* data tx antenna (fixed or auto) */
	u_int8_t sc_dfstest_ieeechan;		/* IEEE channel number to return to after a dfs mute test */
	u_int32_t sc_dfstesttime;		/* Time to stay off chan during dfs test */
	u_int16_t sc_nvaps;			/* # of active virtual ap's */
	u_int8_t sc_nstavaps;			/* # of active station vaps */
	u_int8_t sc_nmonvaps;			/* # of monitor vaps */
	u_int8_t sc_nbcnvaps;			/* # of vaps sending beacons */
	u_int sc_fftxqmin;			/* aggregation threshold */
	HAL_INT sc_imask;			/* interrupt mask copy */
	u_int sc_keymax;				/* size of key cache */
	u_int8_t sc_keymap[ATH_KEYBYTES];	/* key use bit map */
	struct ieee80211_node *sc_keyixmap[ATH_KEYMAX];/* key ix->node map */
	u_int8_t sc_bssidmask[IEEE80211_ADDR_LEN];

	u_int sc_ledpin;				/* GPIO pin for driving LED */
	u_int sc_ledon;				/* pin setting for LED on */
	u_int sc_ledidle;			/* idle polling interval */
	int sc_ledevent;				/* time of last LED event */
	u_int8_t sc_rxrate;			/* current rx rate for LED */
	u_int8_t sc_txrate;			/* current tx rate for LED */
	u_int16_t sc_ledoff;			/* off time for current blink */
	struct timer_list sc_ledtimer;		/* led off timer */
	struct timer_list sc_dfswaittimer;	/* dfs wait timer */
	struct timer_list sc_dfstesttimer;	/* dfs mute test timer */

	struct ATH_TQ_STRUCT sc_fataltq;	/* fatal error intr tasklet */

	int sc_rxbufsize;			/* rx size based on mtu */
	struct ath_descdma sc_rxdma;		/* RX descriptors */
	ath_bufhead sc_rxbuf;			/* receive buffer */
	struct ath_buf *sc_rxbufcur;		/* current rx buffer */
	u_int32_t *sc_rxlink;			/* link ptr in last RX desc */
	spinlock_t sc_rxbuflock; 
	struct ATH_TQ_STRUCT sc_rxorntq;	/* rxorn intr tasklet */
	u_int8_t sc_defant;			/* current default antenna */
	u_int8_t sc_rxotherant;			/* rx's on non-default antenna*/
	u_int16_t sc_cachelsz;			/* cache line size */

	struct ath_descdma sc_txdma;		/* TX descriptors */
	ath_bufhead sc_txbuf;			/* transmit buffer */
	spinlock_t sc_txbuflock;		/* txbuf lock */
	u_int sc_txqsetup;			/* h/w queues setup */
	u_int sc_txintrperiod;			/* tx interrupt batching */
	struct ath_txq sc_txq[HAL_NUM_TX_QUEUES];
	struct ath_txq *sc_ac2q[WME_NUM_AC];	/* WME AC -> h/w qnum */ 
	HAL_INT sc_isr;				/* unmasked ISR state */
	struct ATH_TQ_STRUCT sc_txtq;		/* tx intr tasklet */
	u_int8_t sc_grppoll_str[GRPPOLL_RATE_STR_LEN];  
	struct ath_descdma sc_bdma;		/* beacon descriptors */
	ath_bufhead sc_bbuf;			/* beacon buffers */
	u_int sc_bhalq;				/* HAL q for outgoing beacons */
	u_int sc_bmisscount;			/* missed beacon transmits */
	u_int32_t sc_ant_tx[8];			/* recent tx frames/antenna */
	struct ath_txq *sc_cabq;			/* tx q for cab frames */
	struct ath_txq sc_grpplq;		/* tx q for XR group polls */
	struct ath_txq *sc_xrtxq;		/* tx q for XR data */
	struct ath_descdma sc_grppolldma;	/* TX descriptors for grppoll */
	ath_bufhead sc_grppollbuf;		/* transmit buffers for grouppoll  */
	u_int16_t sc_xrpollint;			/* xr poll interval */
	u_int16_t sc_xrpollcount;		/* xr poll count */
	struct ath_txq *sc_uapsdq;		/* tx q for uapsd */
	struct ATH_TQ_STRUCT sc_bmisstq;	/* bmiss intr tasklet */
	struct ATH_TQ_STRUCT sc_bstucktq;	/* beacon stuck intr tasklet */
	enum {
		OK,				/* no change needed */
		UPDATE,				/* update pending */
		COMMIT				/* beacon sent, commit change */
	} sc_updateslot;			/* slot time update fsm */
	int sc_slotupdate;			/* slot to next advance fsm */
	struct ieee80211vap *sc_bslot[ATH_BCBUF];/* beacon xmit slots */
	int sc_bnext;				/* next slot for beacon xmit */

	struct timer_list sc_cal_ch;		/* calibration timer */
	HAL_NODE_STATS sc_halstats;		/* station-mode rssi stats */
	struct ATH_WORK_THREAD sc_radartask;	/* Schedule task for DFS handling */

#ifdef CONFIG_SYSCTL
	struct ctl_table_header *sc_sysctl_header;
	struct ctl_table *sc_sysctls;
#endif

	u_int16_t sc_reapcount;  		/* # of tx buffers reaped after net dev stopped */

#ifdef ATH_SUPERG_DYNTURBO
	struct timer_list sc_dturbo_switch_mode;/* AP scan timer */
	u_int32_t sc_dturbo_tcount;		/* beacon intval count */
	u_int32_t sc_dturbo_hold_max;		/* hold count before switching to base*/
	u_int16_t sc_dturbo_hold_count;		/* hold count before switching to base*/
	u_int16_t sc_dturbo_turbo_tmin;		/* min turbo count */
	u_int32_t sc_dturbo_bytes;		/* bandwidth stats */ 
	u_int32_t sc_dturbo_base_tmin;		/* min time in base */
	u_int32_t sc_dturbo_turbo_tmax;		/* max time in turbo */
	u_int32_t sc_dturbo_bw_base;		/* bandwidth threshold */
	u_int32_t sc_dturbo_bw_turbo;		/* bandwidth threshold */
#endif
	u_int sc_slottimeconf;			/* manual override for slottime */
	u_int8_t sc_channelbw;
	u_int8_t sc_countrycode;
	
};

typedef void (*ath_callback) (struct ath_softc *);
#define	ATH_TXQ_SETUP(sc, i)	((sc)->sc_txqsetup & (1<<i))

#define	ATH_TXBUF_LOCK_INIT(_sc)	spin_lock_init(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_LOCK_DESTROY(_sc)
#define	ATH_TXBUF_LOCK(_sc)		spin_lock(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_UNLOCK(_sc)		spin_unlock(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_LOCK_BH(_sc)		spin_lock_bh(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_UNLOCK_BH(_sc)	spin_unlock_bh(&(_sc)->sc_txbuflock)
#define	ATH_TXBUF_LOCK_IRQ(_sc)		do {	\
	unsigned long __txbuflockflags;		\
	spin_lock_irqsave(&(_sc)->sc_txbuflock, __txbuflockflags);
#define	ATH_TXBUF_UNLOCK_IRQ(_sc)		\
	spin_unlock_irqrestore(&(_sc)->sc_txbuflock, __txbuflockflags); \
} while (0)
#define	ATH_TXBUF_UNLOCK_IRQ_EARLY(_sc)		\
	spin_unlock_irqrestore(&(_sc)->sc_txbuflock, __txbuflockflags);

#define	ATH_TXBUF_LOCK_ASSERT(_sc) \
	KASSERT(spin_is_locked(&(_sc)->sc_txbuflock), ("txbuf not locked!"))


#define	ATH_RXBUF_LOCK_INIT(_sc)	spin_lock_init(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_LOCK_DESTROY(_sc)
#define	ATH_RXBUF_LOCK(_sc)		spin_lock(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_UNLOCK(_sc)		spin_unlock(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_LOCK_BH(_sc)		spin_lock_bh(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_UNLOCK_BH(_sc)	spin_unlock_bh(&(_sc)->sc_rxbuflock)
#define	ATH_RXBUF_LOCK_IRQ(_sc)		do {	\
	unsigned long __rxbuflockflags;		\
	spin_lock_irqsave(&(_sc)->sc_rxbuflock, __rxbuflockflags);
#define	ATH_RXBUF_UNLOCK_IRQ(_sc)		\
	spin_unlock_irqrestore(&(_sc)->sc_rxbuflock, __rxbuflockflags); \
} while (0)
#define	ATH_RXBUF_UNLOCK_IRQ_EARLY(_sc)		\
	spin_unlock_irqrestore(&(_sc)->sc_rxbuflock, __rxbuflockflags);


/* Protects the device from concurrent accesses */
#define	ATH_LOCK_INIT(_sc)		init_MUTEX(&(_sc)->sc_lock)
#define	ATH_LOCK_DESTROY(_sc)
#define	ATH_LOCK(_sc)			down(&(_sc)->sc_lock)
#define	ATH_UNLOCK(_sc)			up(&(_sc)->sc_lock)

int ath_attach(u_int16_t, struct net_device *);
int ath_detach(struct net_device *);
void ath_resume(struct net_device *);
void ath_suspend(struct net_device *);
void ath_shutdown(struct net_device *);
irqreturn_t ath_intr(int, void *, struct pt_regs *);
int ath_ioctl_ethtool(struct ath_softc *, int, void __user *);
void bus_read_cachesize(struct ath_softc *, u_int8_t *);
#ifdef CONFIG_SYSCTL
void ath_sysctl_register(void);
void ath_sysctl_unregister(void);
#endif /* CONFIG_SYSCTL */

/*
 * HAL definitions to comply with local coding convention.
 */
#define	ath_hal_reset(_ah, _opmode, _chan, _outdoor, _pstatus) \
	((*(_ah)->ah_reset)((_ah), (_opmode), (_chan), (_outdoor), (_pstatus)))
#define	ath_hal_getratetable(_ah, _mode) \
	((*(_ah)->ah_getRateTable)((_ah), (_mode)))
#define	ath_hal_getmac(_ah, _mac) \
	((*(_ah)->ah_getMacAddress)((_ah), (_mac)))
#define	ath_hal_setmac(_ah, _mac) \
	((*(_ah)->ah_setMacAddress)((_ah), (_mac)))
#define	ath_hal_getbssidmask(_ah, _mask) \
	((*(_ah)->ah_getBssIdMask)((_ah), (_mask)))
#define	ath_hal_setbssidmask(_ah, _mask) \
	((*(_ah)->ah_setBssIdMask)((_ah), (_mask)))
#define	ath_hal_intrset(_ah, _mask) \
	((*(_ah)->ah_setInterrupts)((_ah), (_mask)))
#define	ath_hal_intrget(_ah) \
	((*(_ah)->ah_getInterrupts)((_ah)))
#define	ath_hal_intrpend(_ah) \
	((*(_ah)->ah_isInterruptPending)((_ah)))
#define	ath_hal_getisr(_ah, _pmask) \
	((*(_ah)->ah_getPendingInterrupts)((_ah), (_pmask)))
#define	ath_hal_updatetxtriglevel(_ah, _inc) \
	((*(_ah)->ah_updateTxTrigLevel)((_ah), (_inc)))
#define	ath_hal_setpower(_ah, _mode) \
	((*(_ah)->ah_setPowerMode)((_ah), (_mode), AH_TRUE))
#define	ath_hal_keycachesize(_ah) \
	((*(_ah)->ah_getKeyCacheSize)((_ah)))
#define	ath_hal_keyreset(_ah, _ix) \
	((*(_ah)->ah_resetKeyCacheEntry)((_ah), (_ix)))
#define	ath_hal_keyset(_ah, _ix, _pk, _mac) \
	((*(_ah)->ah_setKeyCacheEntry)((_ah), (_ix), (_pk), (_mac), AH_FALSE))
#define	ath_hal_keyisvalid(_ah, _ix) \
	(((*(_ah)->ah_isKeyCacheEntryValid)((_ah), (_ix))))
#define	ath_hal_keysetmac(_ah, _ix, _mac) \
	((*(_ah)->ah_setKeyCacheEntryMac)((_ah), (_ix), (_mac)))
#define	ath_hal_getrxfilter(_ah) \
	((*(_ah)->ah_getRxFilter)((_ah)))
#define	ath_hal_setrxfilter(_ah, _filter) \
	((*(_ah)->ah_setRxFilter)((_ah), (_filter)))
#define	ath_hal_setmcastfilter(_ah, _mfilt0, _mfilt1) \
	((*(_ah)->ah_setMulticastFilter)((_ah), (_mfilt0), (_mfilt1)))
#define	ath_hal_waitforbeacon(_ah, _bf) \
	((*(_ah)->ah_waitForBeaconDone)((_ah), (_bf)->bf_daddr))
#define	ath_hal_putrxbuf(_ah, _bufaddr) \
	((*(_ah)->ah_setRxDP)((_ah), (_bufaddr)))
#define	ath_hal_gettsf32(_ah) \
	((*(_ah)->ah_getTsf32)((_ah)))
#define	ath_hal_gettsf64(_ah) \
	((*(_ah)->ah_getTsf64)((_ah)))
#define	ath_hal_resettsf(_ah) \
	((*(_ah)->ah_resetTsf)((_ah)))
#define	ath_hal_rxena(_ah) \
	((*(_ah)->ah_enableReceive)((_ah)))
#define	ath_hal_numtxpending(_ah, _q) \
	((*(_ah)->ah_numTxPending)((_ah), (_q)))
#define	ath_hal_puttxbuf(_ah, _q, _bufaddr) \
	((*(_ah)->ah_setTxDP)((_ah), (_q), (_bufaddr)))
#define	ath_hal_gettxbuf(_ah, _q) \
	((*(_ah)->ah_getTxDP)((_ah), (_q)))
#define	ath_hal_getrxbuf(_ah) \
	((*(_ah)->ah_getRxDP)((_ah)))
#define	ath_hal_txstart(_ah, _q) \
	((*(_ah)->ah_startTxDma)((_ah), (_q)))
#define	ath_hal_setchannel(_ah, _chan) \
	((*(_ah)->ah_setChannel)((_ah), (_chan)))
#define	ath_hal_calibrate(_ah, _chan, _isIQdone) \
	((*(_ah)->ah_perCalibration)((_ah), (_chan), (_isIQdone)))
#define	ath_hal_setledstate(_ah, _state) \
	((*(_ah)->ah_setLedState)((_ah), (_state)))
#define	ath_hal_beaconinit(_ah, _nextb, _bperiod) \
	((*(_ah)->ah_beaconInit)((_ah), (_nextb), (_bperiod)))
#define	ath_hal_beaconreset(_ah) \
	((*(_ah)->ah_resetStationBeaconTimers)((_ah)))
#define	ath_hal_beacontimers(_ah, _bs) \
	((*(_ah)->ah_setStationBeaconTimers)((_ah), (_bs)))
#define	ath_hal_setassocid(_ah, _bss, _associd) \
	((*(_ah)->ah_writeAssocid)((_ah), (_bss), (_associd)))
#define	ath_hal_phydisable(_ah) \
	((*(_ah)->ah_phyDisable)((_ah)))
#define	ath_hal_setopmode(_ah) \
	((*(_ah)->ah_setPCUConfig)((_ah)))
#define	ath_hal_stoptxdma(_ah, _qnum) \
	((*(_ah)->ah_stopTxDma)((_ah), (_qnum)))
#define	ath_hal_stoppcurecv(_ah) \
	((*(_ah)->ah_stopPcuReceive)((_ah)))
#define	ath_hal_startpcurecv(_ah) \
	((*(_ah)->ah_startPcuReceive)((_ah)))
#define	ath_hal_stopdmarecv(_ah) \
	((*(_ah)->ah_stopDmaReceive)((_ah)))
#define	ath_hal_getdiagstate(_ah, _id, _indata, _insize, _outdata, _outsize) \
	((*(_ah)->ah_getDiagState)((_ah), (_id), \
		(_indata), (_insize), (_outdata), (_outsize)))
#define	ath_hal_gettxqueueprops(_ah, _q, _qi) \
	((*(_ah)->ah_getTxQueueProps)((_ah), (_q), (_qi)))
#define	ath_hal_settxqueueprops(_ah, _q, _qi) \
	((*(_ah)->ah_setTxQueueProps)((_ah), (_q), (_qi)))
#define	ath_hal_setuptxqueue(_ah, _type, _irq) \
	((*(_ah)->ah_setupTxQueue)((_ah), (_type), (_irq)))
#define	ath_hal_resettxqueue(_ah, _q) \
	((*(_ah)->ah_resetTxQueue)((_ah), (_q)))
#define	ath_hal_releasetxqueue(_ah, _q) \
	((*(_ah)->ah_releaseTxQueue)((_ah), (_q)))
#define	ath_hal_getrfgain(_ah) \
	((*(_ah)->ah_getRfGain)((_ah)))
#define	ath_hal_getdefantenna(_ah) \
	((*(_ah)->ah_getDefAntenna)((_ah)))
#define	ath_hal_setdefantenna(_ah, _ant) \
	((*(_ah)->ah_setDefAntenna)((_ah), (_ant)))
#define	ath_hal_rxmonitor(_ah, _arg, _chan) \
	((*(_ah)->ah_rxMonitor)((_ah), (_arg), (_chan)))
#define	ath_hal_mibevent(_ah, _stats) \
	((*(_ah)->ah_procMibEvent)((_ah), (_stats)))
#define	ath_hal_setslottime(_ah, _us) \
	((*(_ah)->ah_setSlotTime)((_ah), (_us)))
#define	ath_hal_getslottime(_ah) \
	((*(_ah)->ah_getSlotTime)((_ah)))
#define	ath_hal_setacktimeout(_ah, _us) \
	((*(_ah)->ah_setAckTimeout)((_ah), (_us)))
#define	ath_hal_getacktimeout(_ah) \
	((*(_ah)->ah_getAckTimeout)((_ah)))
#define	ath_hal_setctstimeout(_ah, _us) \
	((*(_ah)->ah_setCTSTimeout)((_ah), (_us)))
#define	ath_hal_getctstimeout(_ah) \
	((*(_ah)->ah_getCTSTimeout)((_ah)))
#define ath_hal_setdecompmask(_ah, _keyid, _b) \
        ((*(_ah)->ah_setDecompMask)((_ah), (_keyid), (_b)))
#define	ath_hal_enablePhyDiag(_ah) \
	((*(_ah)->ah_enablePhyErrDiag)((_ah)))
#define	ath_hal_disablePhyDiag(_ah) \
	((*(_ah)->ah_disablePhyErrDiag)((_ah)))
#define	ath_hal_getcapability(_ah, _cap, _param, _result) \
	((*(_ah)->ah_getCapability)((_ah), (_cap), (_param), (_result)))
#define	ath_hal_setcapability(_ah, _cap, _param, _v, _status) \
	((*(_ah)->ah_setCapability)((_ah), (_cap), (_param), (_v), (_status)))
#define	ath_hal_ciphersupported(_ah, _cipher) \
	(ath_hal_getcapability(_ah, HAL_CAP_CIPHER, _cipher, NULL) == HAL_OK)
#define	ath_hal_fastframesupported(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_FASTFRAME, 0, NULL) == HAL_OK)
#define ath_hal_burstsupported(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_BURST, 0, NULL) == HAL_OK)
#define ath_hal_xrsupported(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_XR, 0, NULL) == HAL_OK)
#define ath_hal_compressionsupported(_ah) \
        (ath_hal_getcapability(_ah, HAL_CAP_COMPRESSION, 0, NULL) == HAL_OK)
#define ath_hal_turboagsupported(_ah) \
	(ath_hal_getwirelessmodes(_ah, ath_countrycode) & (HAL_MODE_108G|HAL_MODE_TURBO))
#define ath_hal_halfrate_chansupported(_ah) \
        (ath_hal_getcapability(_ah, HAL_CAP_CHAN_HALFRATE, 0, NULL) == HAL_OK)
#define ath_hal_quarterrate_chansupported(_ah) \
        (ath_hal_getcapability(_ah, HAL_CAP_CHAN_QUARTERRATE, 0, NULL) == HAL_OK)
#define	ath_hal_getregdomain(_ah, _prd) \
	ath_hal_getcapability(_ah, HAL_CAP_REG_DMN, 0, (_prd))
#define	ath_hal_getcountrycode(_ah, _pcc) \
	(*(_pcc) = (_ah)->ah_countryCode)
#define	ath_hal_tkipsplit(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TKIP_SPLIT, 0, NULL) == HAL_OK)
#define	ath_hal_wmetkipmic(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_WME_TKIPMIC, 0, NULL) == HAL_OK)
#define	ath_hal_hwphycounters(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_PHYCOUNTERS, 0, NULL) == HAL_OK)
#define	ath_hal_hasdiversity(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_DIVERSITY, 0, NULL) == HAL_OK)
#define	ath_hal_getdiversity(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_DIVERSITY, 1, NULL) == HAL_OK)
#define	ath_hal_setdiversity(_ah, _v) \
	ath_hal_setcapability(_ah, HAL_CAP_DIVERSITY, 1, _v, NULL)
#define	ath_hal_getnumtxqueues(_ah, _pv) \
	(ath_hal_getcapability(_ah, HAL_CAP_NUM_TXQUEUES, 0, _pv) == HAL_OK)
#define	ath_hal_hasveol(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_VEOL, 0, NULL) == HAL_OK)
#define	ath_hal_hastxpowlimit(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TXPOW, 0, NULL) == HAL_OK)
#define	ath_hal_settxpowlimit(_ah, _pow) \
	((*(_ah)->ah_setTxPowerLimit)((_ah), (_pow)))
#define	ath_hal_gettxpowlimit(_ah, _ppow) \
	(ath_hal_getcapability(_ah, HAL_CAP_TXPOW, 1, _ppow) == HAL_OK)
#define	ath_hal_getmaxtxpow(_ah, _ppow) \
	(ath_hal_getcapability(_ah, HAL_CAP_TXPOW, 2, _ppow) == HAL_OK)
#define	ath_hal_gettpscale(_ah, _scale) \
	(ath_hal_getcapability(_ah, HAL_CAP_TXPOW, 3, _scale) == HAL_OK)
#define	ath_hal_settpscale(_ah, _v) \
	ath_hal_setcapability(_ah, HAL_CAP_TXPOW, 3, _v, NULL)
#define	ath_hal_hastpc(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TPC, 0, NULL) == HAL_OK)
#define	ath_hal_gettpc(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TPC, 1, NULL) == HAL_OK)
#define	ath_hal_settpc(_ah, _v) \
	ath_hal_setcapability(_ah, HAL_CAP_TPC, 1, _v, NULL)
#define	ath_hal_hasbursting(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_BURST, 0, NULL) == HAL_OK)
#define ath_hal_hascompression(_ah) \
        (ath_hal_getcapability(_ah, HAL_CAP_COMPRESSION, 0, NULL) == HAL_OK)
#define	ath_hal_hasfastframes(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_FASTFRAME, 0, NULL) == HAL_OK)
#define	ath_hal_hasbssidmask(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_BSSIDMASK, 0, NULL) == HAL_OK)
#define	ath_hal_hasmcastkeysearch(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_MCAST_KEYSRCH, 0, NULL) == HAL_OK)
#define	ath_hal_getmcastkeysearch(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_MCAST_KEYSRCH, 1, NULL) == HAL_OK)
#define	ath_hal_hastkipmic(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TKIP_MIC, 0, NULL) == HAL_OK)
#define	ath_hal_gettkipmic(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TKIP_MIC, 1, NULL) == HAL_OK)
#define	ath_hal_settkipmic(_ah, _v) \
	ath_hal_setcapability(_ah, HAL_CAP_TKIP_MIC, 1, _v, NULL)
#define	ath_hal_hastsfadjust(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TSF_ADJUST, 0, NULL) == HAL_OK)
#define	ath_hal_gettsfadjust(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_TSF_ADJUST, 1, NULL) == HAL_OK)
#define	ath_hal_settsfadjust(_ah, _v) \
	ath_hal_setcapability(_ah, HAL_CAP_TSF_ADJUST, 1, _v, NULL)
#define ath_hal_setrfsilent(_ah, _v) \
        ath_hal_setcapability(_ah, HAL_CAP_RFSILENT, 1, _v, NULL)
#define ath_hal_hasrfsilent(_ah) \
	(ath_hal_getcapability(_ah, HAL_CAP_RFSILENT, 0, NULL) == HAL_OK)

#define	ath_hal_setuprxdesc(_ah, _ds, _size, _intreq) \
	((*(_ah)->ah_setupRxDesc)((_ah), (_ds), (_size), (_intreq)))
#define	ath_hal_rxprocdesc(_ah, _ds, _dspa, _dsnext, _tsf) \
	((*(_ah)->ah_procRxDesc)((_ah), (_ds), (_dspa), (_dsnext), (_tsf)))
#define	ath_hal_updateCTSForBursting(_ah, _ds, _prevds, _prevdsWithCTS, _gatingds,    \
                                     _txOpLimit, _ctsDuration)			      \
	((*(_ah)->ah_updateCTSForBursting)((_ah), (_ds), (_prevds), (_prevdsWithCTS), \
	                                   (_gatingds), (_txOpLimit), (_ctsDuration)))
#define	ath_hal_setuptxdesc(_ah, _ds, _plen, _hlen, _atype, _txpow, \
		_txr0, _txtr0, _keyix, _ant, _flags, \
		_rtsrate, _rtsdura, \
		_compicvlen, _compivlen, _comp) \
	((*(_ah)->ah_setupTxDesc)((_ah), (_ds), (_plen), (_hlen), (_atype), \
		(_txpow), (_txr0), (_txtr0), (_keyix), (_ant), \
		(_flags), (_rtsrate), (_rtsdura), \
		(_compicvlen), (_compivlen), (_comp)))
#define	ath_hal_setupxtxdesc(_ah, _ds, \
		_txr1, _txtr1, _txr2, _txtr2, _txr3, _txtr3) \
	((*(_ah)->ah_setupXTxDesc)((_ah), (_ds), \
		(_txr1), (_txtr1), (_txr2), (_txtr2), (_txr3), (_txtr3)))
#define	ath_hal_filltxdesc(_ah, _ds, _l, _first, _last, _ds0) \
	((*(_ah)->ah_fillTxDesc)((_ah), (_ds), (_l), (_first), (_last), (_ds0)))
#define	ath_hal_txprocdesc(_ah, _ds) \
	((*(_ah)->ah_procTxDesc)((_ah), (_ds)))
#define ath_hal_gettxintrtxqs(_ah, _txqs) \
	((*(_ah)->ah_getTxIntrQueue)((_ah), (_txqs)))
#define	ath_hal_txreqintrdesc(_ah, _ds) \
	((*(_ah)->ah_reqTxIntrDesc)((_ah), (_ds)))

#define ath_hal_gpioCfgOutput(_ah, _gpio) \
        ((*(_ah)->ah_gpioCfgOutput)((_ah), (_gpio)))
#define ath_hal_gpioset(_ah, _gpio, _b) \
        ((*(_ah)->ah_gpioSet)((_ah), (_gpio), (_b)))
#define	ath_hal_ar_enable(_ah) \
	((*(_ah)->ah_arEnable)((_ah)))
#define	ath_hal_ar_disable(_ah) \
	((*(_ah)->ah_arDisable)((_ah)))
#define	ath_hal_ar_reset(_ah) \
	((*(_ah)->ah_arReset)((_ah)))
#define	ath_hal_setcoverageclass(_ah, _coverageclass, _now) \
	((*(_ah)->ah_setCoverageClass)((_ah), (_coverageclass), (_now)))
#define ath_hal_radar_event(_ah) \
	((*(_ah)->ah_radarHaveEvent)((_ah)))
#define ath_hal_procdfs(_ah, _chan) \
	((*(_ah)->ah_processDfs)((_ah), (_chan)))
#define ath_hal_checknol(_ah, _chan, _nchans) \
	((*(_ah)->ah_dfsNolCheck)((_ah), (_chan), (_nchans)))
#define ath_hal_radar_wait(_ah, _chan) \
	((*(_ah)->ah_radarWait)((_ah), (_chan)))
	
#ifdef AH_ABI_EXT
#define ath_hal_xr_enable(_ah) \
    ((*(_ah)->ah_xrEnable)((_ah)))
#define ath_hal_xr_disable(_ah) \
    ((*(_ah)->ah_xrDisable)((_ah)))
#else
#define ath_hal_xr_enable(_ah)
#define ath_hal_xr_disable(_ah)
#endif

#endif /* _DEV_ATH_ATHVAR_H */
