/******************************************************************************
 *
 * Name:        skdrv2nd.h
 * Project:     Yukon/Yukon2, PCI Gigabit Ethernet Adapter
 * Purpose:     Second header file for driver and all other modules
 *
 ******************************************************************************/

/******************************************************************************
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This is the second include file of the driver, which includes all other
 * neccessary files and defines all structures and constants used by the
 * driver and the common modules.
 *
 * Include File Hierarchy:
 *
 *	see skge.c
 *
 ******************************************************************************/

#ifndef __INC_SKDRV2ND_H
#define __INC_SKDRV2ND_H

#include "h/skqueue.h"
#include "h/skgehwt.h"
#include "h/sktimer.h"
#include "h/sktwsi.h"
#include "h/skgepnmi.h"
#include "h/skvpd.h"
#include "h/skgehw.h"
#include "h/sky2le.h"
#include "h/skgeinit.h"
#include "h/skaddr.h"
#include "h/skgesirq.h"
#include "h/skcsum.h"
#include "h/skgedrv.h"
#include "h/mvyexhw.h"
#include "h/fwos.h"


#ifdef	SK_AVB
#include "h/skavb.h"
#endif

/* ESX4(.x) definitions */
#if ((LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)) || defined (SK98LIN_VMESX5))
#define SK_NEW_ISR_DEFINITION
#endif


/* ESX4(.x) definitions */
#if ((LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)) || defined (SK98LIN_VMESX4))
#define SK_NEW_NAPI_HANDLING
#endif

#if ((LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)) || defined (SK98LIN_VMESX4))
#define SK_IRQ_SHARED	IRQF_SHARED
#else
#define SK_IRQ_SHARED	SA_SHIRQ
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,33))
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define USE_SK_VLAN_SUPPORT
#endif
#endif

/* GSO definitions */
#if defined(SLE_VERSION_CODE) 
#if LINUX_VERSION_CODE > SLE_VERSION(2,6,15)
#define MBUF_SIZE_TYPE_GSO
#endif
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#define MBUF_SIZE_TYPE_GSO
#endif

/* Disable ProcFS */
#undef CONFIG_PROC_FS

/* Enable SDK support */
#ifdef MV_INCLUDE_SDK_SUPPORT
// #define VERBOSE_ASF_FIFO_DBUG_RPINTS
// #define MV_SET_FW_IP_DRIVER
#include "h/skspi.h"
#include "h/skpflash.h"
#include "h/skgeasfapi.h"
#include "h/fwcommon.h"

/* Set SDK parameters */
#define MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
// #define MV_FW_SDK_LINK_MAINTENANCE_DEBUG
#endif

/* Required for 64 bit division */
#include "asm/div64.h"

/* EEPROM device magic number */
#define SK98LIN_EEPROM_MAGIC		0x9955

/* Define the priv structure */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
#define PRIV	dev->priv
#define PPRIV	(DEV_NET*)dev->priv
#else
#define PRIV	netdev_priv(dev)
#define PPRIV	netdev_priv(dev)
#endif

/* Defines for the poll cotroller */
#if !defined(SK_NETDUMP_POLL)
#define SK_NETDUMP_POLL
#endif

#if defined(SK_NETDUMP_POLL)
#if defined(HAVE_POLL_CONTROLLER)
#define SK_POLL_CONTROLLER
#define CONFIG_SK98LIN_NAPI
#elif defined(CONFIG_NET_POLL_CONTROLLER)
#define SK_POLL_CONTROLLER
#if !defined(CONFIG_SK98LIN_NAPI)
#define CONFIG_SK98LIN_NAPI
#endif
#endif
#endif

/* Defines for diag */
#if defined(SK98LIN_DIAG)
#define SK98LIN_DIAG_LOOPBACK
#endif

/******************************************************************************
 *
 * SDK driver defines (CHANGE IT!!!!)
 *
 ******************************************************************************/
#ifdef SK_NO_RLMT
#undef SKGE_RLMT
#define SKGE_RLMT SKGE_DRV
#undef SK_RLMT_LINK_UP
#define SK_RLMT_LINK_UP SK_DRV_LINK_UP
#undef SK_RLMT_LINK_DOWN
#define SK_RLMT_LINK_DOWN SK_DRV_LINK_DOWN
#define SkRlmtEvent(a,b,c,d)	SkDrvEvent(a,b,c,d)
#endif

#define SKGE_ASF	14	/* ASF Event Class */

/******************************************************************************
 *
 * Generic driver defines
 *
 ******************************************************************************/

#define USE_TIST_FOR_RESET	/* Use timestamp for reset */
#define Y2_RECOVERY		/* use specific recovery yukon2 functions */
#define Y2_LE_CHECK		/* activate check for LE order */
#define Y2_SYNC_CHECK		/* activate check for receiver in sync */

// Disable RSS for older kernel versions
#ifdef USE_SK_RSS_SUPPORT
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
#undef USE_SK_RSS_SUPPORT
#else
#undef Y2_LE_CHECK		/* deactivate check for LE order */
#undef Y2_SYNC_CHECK		/* deactivate check for receiver in sync */
#endif
#endif


#define SK_YUKON2		/* Enable Yukon2 dual net support */
#define USE_SK_TX_CHECKSUM	/* use the tx hw checksum driver functionality */
#define USE_SK_RX_CHECKSUM	/* use the rx hw checksum driver functionality */
#define USE_SK_TSO_FEATURE	/* use TCP segmentation offload if possible */
#define SK_COPY_THRESHOLD 50	/* threshold for copying small RX frames;
				 * 0 avoids copying, 9001 copies all */
#define SK_MAX_CARD_PARAM 16	/* number of adapters that can be configured via
				 * command line params */
#if 0
#define USE_TX_COMPLETE	/* use of a transmit complete interrupt */
#endif
#define Y2_RX_CHECK		/* RX Check timestamp */

#define SK_REL_SPIN_LOCK(IoC)
#define SK_ACQ_SPIN_LOCK(IoC)

#ifdef	SK_AVB
#define	ALLOC_ETHDEV(VAL)		alloc_etherdev_mqs((VAL), TX_LE_Q_CNT, 1)
#define	NETIF_START_ALLQ(DEV)	netif_tx_start_all_queues(DEV)
#define	NETIF_STOP_ALLQ(DEV)	netif_tx_stop_all_queues(DEV)
#define	NETIF_Q_STOPPED(DEV,Q)	netif_tx_queue_stopped(netdev_get_tx_queue((DEV), (Q)))
#define	NETIF_WAKE_ALLQ(DEV)	netif_tx_wake_all_queues(DEV)
#define	NETIF_START_Q(DEV,Q)	netif_start_subqueue((DEV),(Q))
#define	NETIF_STOP_Q(DEV,Q)		netif_stop_subqueue((DEV),(Q))
#define	NETIF_WAKE_Q(DEV,Q)		netif_wake_subqueue((DEV),(Q))
#else
#define	ALLOC_ETHDEV(VAL)		alloc_etherdev(VAL)
#define	NETIF_START_ALLQ(DEV)	netif_start_queue(DEV)
#define	NETIF_STOP_ALLQ(DEV)	netif_stop_queue(DEV)
#define	NETIF_WAKE_ALLQ(DEV)	netif_wake_queue(DEV)
#define	NETIF_Q_STOPPED(DEV,Q)	netif_queue_stopped(DEV)
#define	NETIF_START_Q(DEV,Q)	netif_start_queue(DEV)
#define	NETIF_STOP_Q(DEV,Q)		netif_stop_queue(DEV)
#define	NETIF_WAKE_Q(DEV,Q)		netif_wake_queue(DEV)
#endif

/*
 * use those defines for a compile-in version of the driver instead
 * of command line parameters
 */
// #define LINK_SPEED_A {"Auto",}
// #define LINK_SPEED_B {"Auto",}
// #define AUTO_NEG_A   {"Sense",}
// #define AUTO_NEG_B   {"Sense"}
// #define DUP_CAP_A    {"Both",}
// #define DUP_CAP_B    {"Both",}
// #define FLOW_CTRL_A  {"SymOrRem",}
// #define FLOW_CTRL_B  {"SymOrRem",}
// #define ROLE_A       {"Auto",}
// #define ROLE_B       {"Auto",}
// #define CON_TYPE     {"Auto",}
// #define WOL_TYPE     {"Auto",}

#ifdef Y2_RECOVERY
#define CHECK_TRANSMIT_TIMEOUT
#define Y2_RESYNC_WATERMARK     1000000L
#endif

/******************************************************************************
 *
 * Generic ISR defines
 *
 ******************************************************************************/

#define SkIsrRetVar     irqreturn_t
#define SkIsrRetNone    IRQ_NONE
#define SkIsrRetHandled IRQ_HANDLED
#define DEV_KFREE_SKB(skb) dev_kfree_skb(skb)
#define DEV_KFREE_SKB_IRQ(skb) dev_kfree_skb_irq(skb)
#define DEV_KFREE_SKB_ANY(skb) dev_kfree_skb_any(skb)


/******************************************************************************
 *
 * Generic diag loopback defines
 *
 ******************************************************************************/
#ifdef SK98LIN_DIAG_LOOPBACK
#define GBE_LOOPBACK_TIMEOUT		2 /* 2sec */
#define GBE_LOOPBACK_NONE		0
#define GBE_LOOPBACK_START		1
#define GBE_LOOPBACK_VERIFYING		2
#define GBE_LOOPBACK_OK		3
#define GBE_LOOPBACK_ERROR		4
#define GBE_LOOPBACK_ERROR_TIMEOUT	5

#define GBE_LOOPBACK_MAX_EP_SIZE	1514
#define GBE_LOOPBACK_MID_EP_SIZE	526
#define GBE_LOOPBACK_MIN_EP_SIZE	60
#define GBE_LOOPBACK_ADDRESS_SIZE	6

#endif

/******************************************************************************
 *
 * Global function prototypes
 *
 ******************************************************************************/

extern SK_U64 SkOsGetTime(SK_AC*);
extern int SkPciReadCfgDWord(SK_AC*, int, SK_U32*);
extern int SkPciReadCfgWord(SK_AC*, int, SK_U16*);
extern int SkPciReadCfgByte(SK_AC*, int, SK_U8*);
extern int SkPciWriteCfgDWord(SK_AC*, int, SK_U32);
extern int SkPciWriteCfgWord(SK_AC*, int, SK_U16);
extern int SkPciWriteCfgByte(SK_AC*, int, SK_U8);
extern int SkDrvEvent(SK_AC*, SK_IOC IoC, SK_U32, SK_EVPARA);
extern int SkDrvEnterDiagMode(SK_AC *pAc);
extern int SkDrvLeaveDiagMode(SK_AC *pAc);
extern int spi_timer(SK_AC *pAC, unsigned int t);

/******************************************************************************
 *
 * Linux specific TIME defines
 *
 ******************************************************************************/

#if SK_TICKS_PER_SEC == 100
#define SK_PNMI_HUNDREDS_SEC(t)	(t)
#else
#define SK_PNMI_HUNDREDS_SEC(t) ((((unsigned long)t)*100)/(SK_TICKS_PER_SEC))
#endif

#ifdef MV_INCLUDE_SDK_SUPPORT
#define SK_FLASH_RESCHED cond_resched
#endif

#define SkOsGetTimeCurrent(pAC, pUsec) {\
	static struct timeval prev_t; \
	struct timeval t;\
	do_gettimeofday(&t);\
	if (prev_t.tv_sec == t.tv_sec) { \
		if (prev_t.tv_usec > t.tv_usec) { \
			t.tv_usec = prev_t.tv_usec; \
		} else { \
			prev_t.tv_usec = t.tv_usec; \
		} \
	} else { \
		prev_t = t; \
	} \
	*pUsec = ((t.tv_sec*100L)+(t.tv_usec/10000));\
}

#define SK_WOL_LINK_WAIT_DELAY	200	/* milliseconds */
#define SK_WOL_LINK_MAX_LOOP	25	/* Loop for 5 seconds, then give up */

/******************************************************************************
 *
 * Linux specific IOCTL defines and typedefs
 *
 ******************************************************************************/

#define	SK_IOCTL_BASE        (SIOCDEVPRIVATE)
#define	SK_IOCTL_GETMIB      (SK_IOCTL_BASE + 0)
#define	SK_IOCTL_SETMIB      (SK_IOCTL_BASE + 1)
#define	SK_IOCTL_PRESETMIB   (SK_IOCTL_BASE + 2)
#define	SK_IOCTL_GEN         (SK_IOCTL_BASE + 3)
#define	SK_IOCTL_DIAG        (SK_IOCTL_BASE + 4)

/* Embedded SDK driver ioctl */
#define SK_IOCTL_GETLINKCAP  (SK_IOCTL_BASE + 14)
#define SK_AUTONEG_SUCCESS 0
#define SK_AUTONEG_FAIL    1
#define SK_AUTONEG_OFF     2
#define SK_AUTONEG_NOLINK  3

/* Firmware ioctl defines and commands */
#define SK_IOCTL_APPTOFW     (SK_IOCTL_BASE + 5)
#define SK_IOCTL_APPTODRV    (SK_IOCTL_BASE + 6)
#define SK_IOCTL_FW_SEND	1
#define SK_IOCTL_FW_RECEIVE	2
#define SK_IOCTL_COMMAND	5

/* Firmware ioctl defines and commands */
#define SK_IOCTL_AVB_CMD	(SK_IOCTL_BASE + 20)
#define SK_IOCTL_AVB_WR		1
#define SK_IOCTL_AVB_RD		2

typedef struct _DrvCmd SK_GE_CMDIOCTL;
struct _DrvCmd {
	SK_U32   Command;
};

typedef struct s_IOCTL SK_GE_IOCTL;
struct s_IOCTL {
	char __user *	pData;
	unsigned int	Len;
};

/******************************************************************************
 *
 * Generic sizes and length definitions
 *
 ******************************************************************************/
#ifdef	SK_AVB
#define	TX_LE_Q_CNT	AVB_LE_Q_CNT
#else
#ifdef	USE_SYNC_TX_QUEUE
#define	TX_LE_Q_CNT	2
#else
#define	TX_LE_Q_CNT	1
#endif
#endif

#define TX_RING_SIZE  (24*1024)			/* Yukon */
#define RX_RING_SIZE  (24*1024)			/* Yukon */
#define RX_MAX_NBR_BUFFERS   128		/* Yukon-EC/-II */
#define TX_MAX_NBR_BUFFERS   128		/* Yukon-EC/-II per queue */
#define MAXIMUM_LOW_ADDRESS 0xFFFFFFFF		/* Max. low address */

#define	ETH_BUF_SIZE        1560		/* multiples of 8 bytes */
#define	ETH_MAX_MTU         1500
#define	ETH_MAX_LEN         1518
#ifndef ETH_MIN_MTU
#define ETH_MIN_MTU         60
#endif
#define ETH_MULTICAST_BIT   0x01
#define SK_JUMBO_MTU        9000

#define TX_PRIO_LOW    0 /* asynchronous queue */
#define TX_PRIO_HIGH   1 /* synchronous queue */
#define DESCR_ALIGN   64 /* alignment of Rx/Tx descriptors */

/******************************************************************************
 *
 * PNMI related definitions
 *
 ******************************************************************************/

#define SK_DRIVER_RESET(pAC, IoC)	0
#define SK_DRIVER_SENDEVENT(pAC, IoC)	0
#define SK_DRIVER_SELFTEST(pAC, IoC)	0
/* For get mtu you must add an own function */
#define SK_DRIVER_GET_MTU(pAc,IoC,i)	0
#define SK_DRIVER_SET_MTU(pAc,IoC,i,v)	0
#define SK_DRIVER_PRESET_MTU(pAc,IoC,i,v)	0


/******************************************************************************
 *
 * Various offsets and sizes
 *
 ******************************************************************************/

#define	SK_DRV_MODERATION_TIMER         1   /* id */
#define SK_DRV_MODERATION_TIMER_LENGTH  1   /* 1 second */

#define C_LEN_ETHERMAC_HEADER_DEST_ADDR 6
#define C_LEN_ETHERMAC_HEADER_SRC_ADDR  6
#define C_LEN_ETHERMAC_HEADER_LENTYPE   2
#define C_LEN_ETHERMAC_HEADER           ( (C_LEN_ETHERMAC_HEADER_DEST_ADDR) + \
                                          (C_LEN_ETHERMAC_HEADER_SRC_ADDR)  + \
                                          (C_LEN_ETHERMAC_HEADER_LENTYPE) )

#define C_LEN_ETHERMTU_MINSIZE          46
#define C_LEN_ETHERMTU_MAXSIZE_STD      1500
#define C_LEN_ETHERMTU_MAXSIZE_JUMBO    9000

#define C_LEN_ETHERNET_MINSIZE          ( (C_LEN_ETHERMAC_HEADER) + \
                                          (C_LEN_ETHERMTU_MINSIZE) )

#define C_OFFSET_IPHEADER               C_LEN_ETHERMAC_HEADER
#define C_OFFSET_IPHEADER_IPPROTO       9
#define C_OFFSET_TCPHEADER_TCPCS        16
#define C_OFFSET_UDPHEADER_UDPCS        6

#define C_OFFSET_IPPROTO                ( (C_LEN_ETHERMAC_HEADER) + \
                                          (C_OFFSET_IPHEADER_IPPROTO) )

#define C_PROTO_ID_UDP                  17       /* refer to RFC 790 or Stevens'   */
#define C_PROTO_ID_TCP                  6        /* TCP/IP illustrated for details */

/******************************************************************************
 *
 * Tx and Rx descriptor definitions
 *
 ******************************************************************************/

typedef struct s_RxD RXD; /* the receive descriptor */
struct s_RxD {
	volatile SK_U32  RBControl;     /* Receive Buffer Control            */
	SK_U32           VNextRxd;      /* Next receive descriptor,low dword */
	SK_U32           VDataLow;      /* Receive buffer Addr, low dword    */
	SK_U32           VDataHigh;     /* Receive buffer Addr, high dword   */
	SK_U32           FrameStat;     /* Receive Frame Status word         */
	SK_U32           TimeStamp;     /* Time stamp from XMAC              */
	SK_U32           TcpSums;       /* TCP Sum 2 / TCP Sum 1             */
	SK_U32           TcpSumStarts;  /* TCP Sum Start 2 / TCP Sum Start 1 */
	RXD             *pNextRxd;      /* Pointer to next Rxd               */
	struct sk_buff  *pMBuf;         /* Pointer to Linux' socket buffer   */
};

typedef struct s_TxD TXD; /* the transmit descriptor */
struct s_TxD {
	volatile SK_U32  TBControl;     /* Transmit Buffer Control            */
	SK_U32           VNextTxd;      /* Next transmit descriptor,low dword */
	SK_U32           VDataLow;      /* Transmit Buffer Addr, low dword    */
	SK_U32           VDataHigh;     /* Transmit Buffer Addr, high dword   */
	SK_U32           FrameStat;     /* Transmit Frame Status Word         */
	SK_U32           TcpSumOfs;     /* Reserved / TCP Sum Offset          */
#ifndef SK_USE_REV_DESC
	SK_U16           TcpSumWr;      /* TCP Sum Write                      */
	SK_U16           TcpSumSt;      /* TCP Sum Start                      */
#else
	SK_U16           TcpSumSt;      /* TCP Sum Start                      */
	SK_U16           TcpSumWr;      /* TCP Sum Write                      */
#endif
	SK_U32           TcpReserved;   /* not used                           */
	TXD             *pNextTxd;      /* Pointer to next Txd                */
	struct sk_buff  *pMBuf;         /* Pointer to Linux' socket buffer    */
};

/******************************************************************************
 *
 * Generic Yukon-II defines
 *
 ******************************************************************************/


/* Number of Status LE which will be allocated at init time. */
#define NUMBER_OF_ST_LE 4096L

/* Number of revceive LE which will be allocated at init time. */
#define NUMBER_OF_RX_LE 512

/* Number of transmit LE which will be allocated at init time. */
#define NUMBER_OF_TX_LE 1024L

/* Min number of TX LE which will be allocated at init time. */
#define MIN_NUMBER_OF_TX_LE 128

/* Min number of RX LE which will be allocated at init time. */
#define MIN_NUMBER_OF_RX_LE 128


#define LE_SIZE   sizeof(SK_HWLE)
#define MAX_NUM_FRAGS   (MAX_SKB_FRAGS + 1)
#define MIN_LEN_OF_LE_TAB   128
#define MAX_UNUSED_RX_LE_WORKING   8
#ifdef MAX_FRAG_OVERHEAD
#undef MAX_FRAG_OVERHEAD
#define MAX_FRAG_OVERHEAD   4
#endif
// as we have a maximum of 16 physical fragments,
// maximum 1 ADDR64 per physical fragment
// maximum 4 LEs for VLAN, Csum, LargeSend, Packet
#define MIN_LE_FREE_REQUIRED   ((16*2) + 4)
//#define IS_GMAC(pAc)   (!pAc->GIni.GIGenesis)
#define IS_GMAC(pAc)   (0)
#ifdef USE_SYNC_TX_QUEUE
#define TXS_MAX_LE   256
#else /* !USE_SYNC_TX_QUEUE */
#define TXS_MAX_LE   0
#endif

#define ETHER_MAC_HDR_LEN   (6+6+2) // MAC SRC ADDR, MAC DST ADDR, TYPE
#define IP_HDR_LEN   20
#define TCP_CSUM_OFFS   0x10
#define UDP_CSUM_OFFS   0x06

#if (defined (Y2_RECOVERY) || defined (Y2_LE_CHECK))
/* event for recovery from rx out of sync */
#define SK_DRV_RECOVER    SK_DRV_PRIVATE_BASE + 1
#endif
/******************************************************************************
 *
 * Structures specific for Yukon-II
 *
 ******************************************************************************/

typedef	struct s_frag SK_FRAG;
struct s_frag {
 	SK_FRAG       *pNext;
 	char          *pVirt;
  	SK_U64         pPhys;
 	unsigned int   FragLen;
};

typedef	struct s_packet SK_PACKET;
struct s_packet {
	/* Common infos: */
	SK_PACKET       *pNext;         /* pointer for packet queues          */
	unsigned int     PacketLen;     /* length of packet                   */
	unsigned int     NumFrags;      /* nbr of fragments (for Rx always 1) */
	SK_FRAG         *pFrag;         /* fragment list                      */
	SK_FRAG          FragArray[MAX_NUM_FRAGS]; /* TX fragment array       */
	unsigned int     NextLE;        /* next LE to use for the next packet */

	/* Private infos: */
	struct sk_buff	*pMBuf;         /* Pointer to Linux' socket buffer    */
};

typedef	struct s_queue SK_PKT_QUEUE;
struct s_queue {
 	SK_PACKET       *pHead;
 	SK_PACKET       *pTail;
	spinlock_t       QueueLock;     /* serialize packet accesses          */
};

/*******************************************************************************
 *
 * Macros specific for Yukon-II queues
 *
 ******************************************************************************/

#define IS_Q_EMPTY(pQueue)  ((pQueue)->pHead != NULL) ? SK_FALSE : SK_TRUE
#define IS_Q_LOCKED(pQueue) spin_is_locked(&((pQueue)->QueueLock))

#define PLAIN_POP_FIRST_PKT_FROM_QUEUE(pQueue, pPacket)	{	\
        if ((pQueue)->pHead != NULL) {				\
		(pPacket)       = (pQueue)->pHead;		\
		(pQueue)->pHead = (pPacket)->pNext;		\
		if ((pQueue)->pHead == NULL) {			\
			(pQueue)->pTail = NULL;			\
		}						\
		(pPacket)->pNext = NULL;			\
	} else {						\
		(pPacket) = NULL;				\
	}							\
}

#define PLAIN_PUSH_PKT_AS_FIRST_IN_QUEUE(pQueue, pPacket) {	\
	if ((pQueue)->pHead != NULL) {				\
		(pPacket)->pNext = (pQueue)->pHead;		\
	} else {						\
		(pPacket)->pNext = NULL;			\
		(pQueue)->pTail  = (pPacket);			\
	}							\
      	(pQueue)->pHead  = (pPacket);				\
}

#define PLAIN_PUSH_PKT_AS_LAST_IN_QUEUE(pQueue, pPacket) {	\
	(pPacket)->pNext = NULL;				\
	if ((pQueue)->pTail != NULL) {				\
		(pQueue)->pTail->pNext = (pPacket);		\
	} else {						\
		(pQueue)->pHead        = (pPacket);		\
	}							\
	(pQueue)->pTail = (pPacket);				\
}

#define PLAIN_PUSH_MULTIPLE_PKT_AS_LAST_IN_QUEUE(pQueue,pPktGrpStart,pPktGrpEnd) { \
	if ((pPktGrpStart) != NULL) {					\
		if ((pQueue)->pTail != NULL) {				\
			(pQueue)->pTail->pNext = (pPktGrpStart);	\
		} else {						\
			(pQueue)->pHead = (pPktGrpStart);		\
		}							\
		(pQueue)->pTail = (pPktGrpEnd);				\
	}								\
}

/* Required: 'Flags' */
#define POP_FIRST_PKT_FROM_QUEUE(pQueue, pPacket)	{	\
	spin_lock_irqsave(&((pQueue)->QueueLock), Flags);	\
	if ((pQueue)->pHead != NULL) {				\
		(pPacket)       = (pQueue)->pHead;		\
		(pQueue)->pHead = (pPacket)->pNext;		\
		if ((pQueue)->pHead == NULL) {			\
			(pQueue)->pTail = NULL;			\
		}						\
		(pPacket)->pNext = NULL;			\
	} else {						\
		(pPacket) = NULL;				\
	}							\
	spin_unlock_irqrestore(&((pQueue)->QueueLock), Flags);	\
}

/* Required: 'Flags' */
#define PUSH_PKT_AS_FIRST_IN_QUEUE(pQueue, pPacket)	{	\
	spin_lock_irqsave(&(pQueue)->QueueLock, Flags);		\
	if ((pQueue)->pHead != NULL) {				\
		(pPacket)->pNext = (pQueue)->pHead;		\
	} else {						\
		(pPacket)->pNext = NULL;			\
		(pQueue)->pTail  = (pPacket);			\
	}							\
	(pQueue)->pHead = (pPacket);				\
	spin_unlock_irqrestore(&(pQueue)->QueueLock, Flags);	\
}

/* Required: 'Flags' */
#define PUSH_PKT_AS_LAST_IN_QUEUE(pQueue, pPacket)	{	\
	spin_lock_irqsave(&(pQueue)->QueueLock, Flags);		\
	(pPacket)->pNext = NULL;				\
	if ((pQueue)->pTail != NULL) {				\
		(pQueue)->pTail->pNext = (pPacket);		\
	} else {						\
		(pQueue)->pHead = (pPacket);			\
	}							\
	(pQueue)->pTail = (pPacket);				\
	spin_unlock_irqrestore(&(pQueue)->QueueLock, Flags);	\
}

/* Required: 'Flags' */
#define PUSH_MULTIPLE_PKT_AS_LAST_IN_QUEUE(pQueue,pPktGrpStart,pPktGrpEnd) {	\
	if ((pPktGrpStart) != NULL) {					\
		spin_lock_irqsave(&(pQueue)->QueueLock, Flags);		\
		if ((pQueue)->pTail != NULL) {				\
			(pQueue)->pTail->pNext = (pPktGrpStart);	\
		} else {						\
			(pQueue)->pHead = (pPktGrpStart);		\
		}							\
		(pQueue)->pTail = (pPktGrpEnd);				\
		spin_unlock_irqrestore(&(pQueue)->QueueLock, Flags);	\
	}								\
}

/*
 *Check if the low address (32 bit) is near the 4G limit or over it.
 * Set the high address to a wrong value.
 * Doing so we force to write the ADDR64 LE.
 */
#define CHECK_LOW_ADDRESS( _HighAddress, _LowAddress , _Length) {	\
	if ((~0-_LowAddress) <_Length) {				\
		_HighAddress= MAXIMUM_LOW_ADDRESS;			\
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,			\
			("High Address must be set for HW. LowAddr = %d  Length = %d\n",	\
			_LowAddress, _Length));				\
	}								\
}

/*******************************************************************************
 *
 * Macros specific for Yukon-II queues (tist)
 *
 ******************************************************************************/

#ifdef USE_TIST_FOR_RESET
/* port is fully operational */
#define SK_PSTATE_NOT_WAITING_FOR_TIST                  0
/* port in reset until any tist LE */
#define SK_PSTATE_WAITING_FOR_ANY_TIST          BIT_0
/* port in reset until timer reaches pAC->MinTistLo */
#define SK_PSTATE_WAITING_FOR_SPECIFIC_TIST     BIT_1
#define SK_PSTATE_PORT_SHIFT    4
#define SK_PSTATE_PORT_MASK             ((1 << SK_PSTATE_PORT_SHIFT) - 1)

/* use this + Port to build OP_MOD_TXINDEX_NO_PORT_A|B */
#define OP_MOD_TXINDEX 0x71
/* opcode for a TX_INDEX LE in which Port A has to be ignored */
#define OP_MOD_TXINDEX_NO_PORT_A 0x71
/* opcode for a TX_INDEX LE in which Port B has to be ignored */
#define OP_MOD_TXINDEX_NO_PORT_B 0x72
/* opcode for LE to be ignored because port is still in reset */
#define OP_MOD_LE 0x7F

/* set tist wait mode Bit for port */
#define SK_SET_WAIT_BIT_FOR_PORT(pAC, Bit, Port)        \
	{ \
		(pAC)->AdapterResetState |= ((Bit) << (SK_PSTATE_PORT_SHIFT * Port)); \
	}

/* reset tist waiting for specified port */
#define SK_CLR_STATE_FOR_PORT(pAC, Port)        \
	{ \
		(pAC)->AdapterResetState &= \
			~(SK_PSTATE_PORT_MASK << (SK_PSTATE_PORT_SHIFT * Port)); \
	}

/* return SK_TRUE when port is in reset waiting for tist */
#define SK_PORT_WAITING_FOR_TIST(pAC, Port) \
	((((pAC)->AdapterResetState >> (SK_PSTATE_PORT_SHIFT * Port)) & \
		SK_PSTATE_PORT_MASK) != SK_PSTATE_NOT_WAITING_FOR_TIST)

/* return SK_TRUE when port is in reset waiting for any tist */
#define SK_PORT_WAITING_FOR_ANY_TIST(pAC, Port) \
	((((pAC)->AdapterResetState >> (SK_PSTATE_PORT_SHIFT * Port)) & \
		SK_PSTATE_WAITING_FOR_ANY_TIST) == SK_PSTATE_WAITING_FOR_ANY_TIST)

/* return SK_TRUE when port is in reset waiting for a specific tist */
#define SK_PORT_WAITING_FOR_SPECIFIC_TIST(pAC, Port) \
	((((pAC)->AdapterResetState >> (SK_PSTATE_PORT_SHIFT * Port)) & \
		SK_PSTATE_WAITING_FOR_SPECIFIC_TIST) == \
		SK_PSTATE_WAITING_FOR_SPECIFIC_TIST)

/* return whether adapter is expecting a tist LE */
#define SK_ADAPTER_WAITING_FOR_TIST(pAC)        ((pAC)->AdapterResetState != 0)

/* enable timestamp timer and force creation of tist LEs */
#define Y2_ENABLE_TIST(IoC) \
	SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8) GMT_ST_START)

/* disable timestamp timer and stop creation of tist LEs */
#define Y2_DISABLE_TIST(IoC) \
	SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8) GMT_ST_STOP)

/* get current value of timestamp timer */
#define Y2_GET_TIST_LOW_VAL(IoC, pVal) \
	SK_IN32(IoC, GMAC_TI_ST_VAL, pVal)

#endif


/*******************************************************************************
 *
 * Used interrupt bits in the interrupts source register
 *
 ******************************************************************************/

#define DRIVER_IRQS	((IS_IRQ_SW) | \
			 (IS_R1_F)   | (IS_R2_F)  | \
			 (IS_XS1_F)  | (IS_XA1_F) | \
			 (IS_XS2_F)  | (IS_XA2_F))

#define TX_COMPL_IRQS	((IS_XS1_B)  | (IS_XS1_F) | \
			 (IS_XA1_B)  | (IS_XA1_F) | \
			 (IS_XS2_B)  | (IS_XS2_F) | \
			 (IS_XA2_B)  | (IS_XA2_F))

#define NAPI_DRV_IRQS	((IS_R1_F)   | (IS_R2_F) | \
			 (IS_XS1_F)  | (IS_XA1_F)| \
			 (IS_XS2_F)  | (IS_XA2_F))

#define Y2_DRIVER_IRQS	((Y2_IS_STAT_BMU) | (Y2_IS_IRQ_SW) | (Y2_IS_POLL_CHK))

#define SPECIAL_IRQS	((IS_HW_ERR)    |(IS_I2C_READY)  | \
			 (IS_EXT_REG)   |(IS_TIMINT)     | \
			 (IS_PA_TO_RX1) |(IS_PA_TO_RX2)  | \
			 (IS_PA_TO_TX1) |(IS_PA_TO_TX2)  | \
			 (IS_MAC1)      |(IS_LNK_SYNC_M1)| \
			 (IS_MAC2)      |(IS_LNK_SYNC_M2)| \
			 (IS_R1_C)      |(IS_R2_C)       | \
			 (IS_XS1_C)     |(IS_XA1_C)      | \
			 (IS_XS2_C)     |(IS_XA2_C))

#define Y2_SPECIAL_IRQS	((Y2_IS_HW_ERR)   |(Y2_IS_ASF)      | \
			 (Y2_IS_TWSI_RDY) |(Y2_IS_TIMINT)   | \
			 (Y2_IS_IRQ_PHY2) |(Y2_IS_IRQ_MAC2) | \
			 (Y2_IS_CHK_RX2)  |(Y2_IS_CHK_TXS2) | \
			 (Y2_IS_CHK_TXA2) |(Y2_IS_IRQ_PHY1) | \
			 (Y2_IS_IRQ_MAC1) |(Y2_IS_CHK_RX1)  | \
			 (Y2_IS_CHK_TXS1) |(Y2_IS_CHK_TXA1))

#define IRQ_MASK	((IS_IRQ_SW)    | \
			 (IS_R1_F)      |(IS_R2_F)     | \
			 (IS_XS1_F)     |(IS_XA1_F)    | \
			 (IS_XS2_F)     |(IS_XA2_F)    | \
			 (IS_HW_ERR)    |(IS_I2C_READY)| \
			 (IS_EXT_REG)   |(IS_TIMINT)   | \
			 (IS_PA_TO_RX1) |(IS_PA_TO_RX2)| \
			 (IS_PA_TO_TX1) |(IS_PA_TO_TX2)| \
			 (IS_MAC1)      |(IS_MAC2)     | \
			 (IS_R1_C)      |(IS_R2_C)     | \
			 (IS_XS1_C)     |(IS_XA1_C)    | \
			 (IS_XS2_C)     |(IS_XA2_C))

#define Y2_IRQ_MASK	((Y2_DRIVER_IRQS) | (Y2_SPECIAL_IRQS))

#define IRQ_HWE_MASK	(IS_ERR_MSK)		/* enable all HW irqs */
#define Y2_IRQ_HWE_MASK	(Y2_HWE_ALL_MSK)	/* enable all HW irqs */
#define Y2_IS_BASE      ((Y2_IS_HW_ERR) | (Y2_IS_STAT_BMU))

typedef struct s_DevNet DEV_NET;

struct s_DevNet {
	struct      proc_dir_entry *proc;
	int         PortNr;
	int         NetNr;
	char        InitialDevName[20];
	char        CurrentName[20];
	SK_BOOL     NetConsoleMode;
#ifdef Y2_RECOVERY
	struct      timer_list KernelTimer;	/* Kernel timer struct  */
	int         TransmitTimeoutTimer; 	/* Transmit timer       */
	SK_BOOL     TimerExpired;		/* Transmit timer       */
	SK_BOOL     InRecover;		/* Recover flag		*/
#ifdef Y2_RX_CHECK
	SK_U32      PreviousMACFifoRP;	/* Backup of the FRP    */
	SK_U32      PreviousMACFifoRLev;	/* Backup of the FRL    */
	SK_U32      PreviousRXFifoRP;	/* Backup of the RX FRP */
	SK_U8       PreviousRXFifoRLev;	/* Backup of the RX FRL */
	SK_U32      LastJiffies;		/* Backup of the jiffies*/
#endif
#endif
	SK_AC       *pAC;
	struct      timer_list ProcfsTimer;	/* Procfs timer struct  */
#ifdef SK_NEW_NAPI_HANDLING
	struct      napi_struct napi;	/* NAPI structure */
#endif
};

/*******************************************************************************
 *
 * Rx/Tx Port structures
 *
 ******************************************************************************/

typedef struct s_TxPort	TX_PORT;
struct s_TxPort {                       /* the transmit descriptor rings */
	caddr_t         pTxDescrRing;   /* descriptor area memory        */
	SK_U64          VTxDescrRing;   /* descr. area bus virt. addr.   */
	TXD            *pTxdRingHead;   /* Head of Tx rings              */
	TXD            *pTxdRingTail;   /* Tail of Tx rings              */
	TXD            *pTxdRingPrev;   /* descriptor sent previously    */
	int             TxdRingPrevFree;/* previously # of free entrys   */
	int             TxdRingFree;    /* # of free entrys              */
	spinlock_t      TxDesRingLock;  /* serialize descriptor accesses */
	SK_IOC          HwAddr;         /* bmu registers address         */
	int             PortIndex;      /* index number of port (0 or 1) */
	SK_PACKET      *TransmitPacketTable;
	SK_LE_TABLE		TxLET[TX_LE_Q_CNT];
	SK_PKT_QUEUE    TxQ_free[TX_LE_Q_CNT];
	SK_PKT_QUEUE	TxQ_waiting[TX_LE_Q_CNT];
	SK_PKT_QUEUE	TxQ_working[TX_LE_Q_CNT];
	unsigned		LastDoneIdx[TX_LE_Q_CNT];
	SK_U16			vlan_tci;		/* last tci passed to hw */
};

typedef struct s_RxPort	RX_PORT;
struct s_RxPort {                       /* the receive descriptor rings  */
	caddr_t         pRxDescrRing;   /* descriptor area memory        */
	SK_U64          VRxDescrRing;   /* descr. area bus virt. addr.   */
	RXD            *pRxdRingHead;   /* Head of Rx rings              */
	RXD            *pRxdRingTail;   /* Tail of Rx rings              */
	RXD            *pRxdRingPrev;   /* descr given to BMU previously */
	int             RxdRingFree;    /* # of free entrys              */
	spinlock_t      RxDesRingLock;  /* serialize descriptor accesses */
	int             RxFillLimit;    /* limit for buffers in ring     */
	SK_IOC          HwAddr;         /* bmu registers address         */
	int             PortIndex;      /* index number of port (0 or 1) */
	SK_BOOL         UseRxCsum;      /* use Rx checksumming (yes/no)  */
	SK_PACKET      *ReceivePacketTable;
	SK_LE_TABLE     RxLET;          /* rx list element table         */
	SK_PKT_QUEUE    RxQ_working;
	SK_PKT_QUEUE    RxQ_waiting;
	int             RxBufSize;
};

/*******************************************************************************
 *
 * Interrupt masks used in combination with interrupt moderation
 *
 ******************************************************************************/

#define IRQ_EOF_AS_TX     ((IS_XA1_F)     | (IS_XA2_F))
#define IRQ_EOF_SY_TX     ((IS_XS1_F)     | (IS_XS2_F))
#define IRQ_MASK_TX_ONLY  ((IRQ_EOF_AS_TX)| (IRQ_EOF_SY_TX))
#define IRQ_MASK_RX_ONLY  ((IS_R1_F)      | (IS_R2_F))
#define IRQ_MASK_SP_ONLY  (SPECIAL_IRQS)
#define IRQ_MASK_TX_RX    ((IRQ_MASK_TX_ONLY)| (IRQ_MASK_RX_ONLY))
#define IRQ_MASK_SP_RX    ((SPECIAL_IRQS)    | (IRQ_MASK_RX_ONLY))
#define IRQ_MASK_SP_TX    ((SPECIAL_IRQS)    | (IRQ_MASK_TX_ONLY))
#define IRQ_MASK_RX_TX_SP ((SPECIAL_IRQS)    | (IRQ_MASK_TX_RX))

#define IRQ_MASK_Y2_TX_ONLY  (Y2_IS_STAT_BMU)
#define IRQ_MASK_Y2_RX_ONLY  (Y2_IS_STAT_BMU)
#define IRQ_MASK_Y2_SP_ONLY  (SPECIAL_IRQS)
#define IRQ_MASK_Y2_TX_RX    ((IRQ_MASK_TX_ONLY)| (IRQ_MASK_RX_ONLY))
#define IRQ_MASK_Y2_SP_RX    ((SPECIAL_IRQS)    | (IRQ_MASK_RX_ONLY))
#define IRQ_MASK_Y2_SP_TX    ((SPECIAL_IRQS)    | (IRQ_MASK_TX_ONLY))
#define IRQ_MASK_Y2_RX_TX_SP ((SPECIAL_IRQS)    | (IRQ_MASK_TX_RX))

/*******************************************************************************
 *
 * Defines and typedefs regarding interrupt moderation
 *
 ******************************************************************************/

#define C_INT_MOD_NONE			1
#define C_INT_MOD_STATIC		2
#define C_INT_MOD_DYNAMIC		4

#define C_CLK_FREQ_GENESIS		 53215000 /* or:  53.125 MHz */
#define C_CLK_FREQ_YUKON		 78215000 /* or:  78.125 MHz */
#define C_CLK_FREQ_YUKON_EC		125000000 /* or: 125.000 MHz */

#define C_Y2_INTS_PER_SEC_DEFAULT	5000
#define C_INTS_PER_SEC_DEFAULT		2000
#define C_INT_MOD_IPS_LOWER_RANGE	30        /* in IRQs/second */
#define C_INT_MOD_IPS_UPPER_RANGE	40000     /* in IRQs/second */

#define C_TX_INT_MOD_UPPER_RANGE	4095     /* in IRQs/second */


typedef struct s_DynIrqModInfo {
	SK_U64     PrevPort0RxIntrCts;
	SK_U64     PrevPort1RxIntrCts;
	SK_U64     PrevPort0TxIntrCts;
	SK_U64     PrevPort1TxIntrCts;
	SK_U64     PrevPort0StatusLeIntrCts;
	SK_U64     PrevPort1StatusLeIntrCts;
	int        MaxModIntsPerSec;            /* Moderation Threshold   */
	int        MaxModIntsPerSecUpperLimit;  /* Upper limit for DIM    */
	int        MaxModIntsPerSecLowerLimit;  /* Lower limit for DIM    */
	long       MaskIrqModeration;           /* IRQ Mask (eg. 'TxRx')  */
	int        IntModTypeSelect;            /* Type  (eg. 'dynamic')  */
	int        DynIrqModSampleInterval;     /* expressed in seconds!  */
	SK_TIMER   ModTimer;                    /* Timer for dynamic mod. */
} DIM_INFO;

/*******************************************************************************
 *
 * Defines and typedefs regarding wake-on-lan
 *
 ******************************************************************************/

typedef struct s_WakeOnLanInfo {
	SK_U32     SupportedWolOptions;         /* e.g. WAKE_PHY...         */
	SK_U32     ConfiguredWolOptions;        /* e.g. WAKE_PHY...         */
} WOL_INFO;

/*******************************************************************************
 *
 * Defines and typedefs regarding link configuration
 *
 ******************************************************************************/
#define SKL_DC_BOTH	0
#define SKL_DC_FULL 1
#define SKL_DC_HALF 2
#define SKL_AN_OFF	0
#define SKL_AN_ON	1
#define SKL_AN_SENS	2

typedef struct s_LinkInfo	LINK_INFO;
struct s_LinkInfo {
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	SK_BOOL		LinkMaintenance; 	/* Link maintenance */
	SK_BOOL		PLinkMaintenance; 	/* Link maintenance parameter */
#endif
	SK_U8		PLinkModeConf;		/* Link Mode configured */
	SK_U8		PFlowCtrlMode;		/* Flow Control Mode */
	SK_U8		PMSMode;		    /* Master/Slave Mode */
	SK_U8		PLinkSpeed;		    /* configured Link Speed */
	SK_U8		PAutoNeg;		    /* Auto negotiation */
	SK_U8		PDuplexCap;		    /* Duplex */
    SK_BOOL		RSS;				/* Receive-Side Scaling parameter */
};

/* Alloc flags */
#define SK_ALLOC_OK     BIT_0
#define SK_ALLOC_MSI    BIT_1
#define SK_ALLOC_IRQ    BIT_2

#define	DIAG_ACTIVE		1
#define	DIAG_NOTACTIVE		0

/****************************************************************************
 *
 * Per board structure / Adapter Context structure:
 * Contains all 'per device' necessary handles, flags, locks etc.:
 *
 ******************************************************************************/

struct s_AC  {
	SK_GEINIT                GIni;          /* GE init struct             */
	SK_PNMI                  Pnmi;          /* PNMI data struct           */
	SK_VPD                   vpd;           /* vpd data struct            */
	SK_QUEUE                 Event;         /* Event queue                */
	SK_HWT                   Hwt;           /* Hardware Timer ctrl struct */
	SK_TIMCTRL               Tim;           /* Software Timer ctrl struct */
	SK_I2C                   I2c;           /* I2C relevant data structure*/
	SK_ADDR                  Addr;          /* for Address module         */
	SK_CSUM                  Csum;          /* for checksum module        */
#ifdef MV_INCLUDE_SDK_SUPPORT
	SK_U64                   timebuf;
	SK_FWOS                  FwOs;
	SK_FWCOMMON              FwCommon;
	SK_FWAPP                 FwApp;
	SK_SPI                   spi;           /* structure for SPI flash */
	SK_PFL                   pfl;
	SK_U32                   RamAddr;
	SK_U32                   RamSelect;
#endif
	spinlock_t               SlowPathLock;  /* Normal IRQ lock            */
	spinlock_t               InitLock;      /* Init lock                  */
	spinlock_t               TxQueueLock;   /* TX Queue lock              */
	SK_PNMI_STRUCT_DATA      PnmiStruct;    /* struct for all Pnmi-Data   */
	int                      RlmtNets;      /* Number of nets             */
	SK_IOC                   IoBase;        /* register set of adapter    */
	int                      BoardLevel;    /* level of hw init (0-2)     */
	char                     DeviceStr[80]; /* adapter string from vpd    */
	struct pci_dev          *PciDev;        /* for access to pci cfg space*/
	SK_U32                   PciDevId;      /* pci device id              */
	struct SK_NET_DEVICE    *dev[2];        /* pointer to device struct   */
	struct SK_NET_DEVICE    *IrqDev;        /* pointer to device struct   */
	char                     Name[30];      /* driver name                */
	struct SK_NET_DEVICE    *Next;          /* link all devs for cleanup  */
	struct net_device_stats  stats;         /* linux 'netstat -i' stats   */
	int                      Index;         /* internal board idx number  */
	int                      RxQueueSize;   /* memory used for RX queue   */
	int                      TxSQueueSize;  /* memory used for TXS queue  */
	int                      TxAQueueSize;  /* memory used for TXA queue  */
	int                      PromiscCount;  /* promiscuous mode counter   */
	int                      AllMultiCount; /* allmulticast mode counter  */
	int                      MulticCount;   /* number of MC addresses used*/
	int                      HWRevision;	/* Hardware revision          */
	int                      ActivePort;	/* the active XMAC port       */
	int                      MaxPorts;      /* number of activated ports  */
	int                      TxDescrPerRing;/* # of descriptors TX ring   */
	int                      RxDescrPerRing;/* # of descriptors RX ring   */
	caddr_t                  pDescrMem;     /* Ptr to the descriptor area */
	dma_addr_t               pDescrMemDMA;  /* PCI DMA address of area    */
	SK_U32                   PciState[16];  /* PCI state */
	TX_PORT                  TxPort[SK_MAX_MACS];
	RX_PORT                  RxPort[SK_MAX_MACS];
	SK_LE_TABLE              StatusLETable;
	unsigned                 SizeOfAlignedLETables;
	spinlock_t               SetPutIndexLock;
	int                      NrOfRxLe[SK_MAX_MACS];
	int                      NrOfTxLe[SK_MAX_MACS];
	int                      MaxUnusedRxLeWorking;
	int                      InterfaceUp[2];
	unsigned int             CsOfs1;        /* for checksum calculation   */
	unsigned int             CsOfs2;        /* for checksum calculation   */
	SK_U32                   CsOfs;         /* for checksum calculation   */
	SK_BOOL                  CheckQueue;    /* check event queue soon     */
	DIM_INFO                 DynIrqModInfo; /* all data related to IntMod */
	LINK_INFO                LinkInfo[SK_MAX_MACS];
	WOL_INFO                 WolInfo;       /* all info regarding WOL     */
	int                      ChipsetType;   /* 1=Yukon, 2=Yukon2          */
	SK_BOOL                  LowLatency;    /* LowLatency optimization on?*/
	SK_BOOL                  MsiIrq;        /* MSI Irq handling on?       */
	SK_U32                   TxModeration;  /* TxModeration optimization  */
	SK_U32                   DiagModeActive;/* is diag active?            */
	SK_BOOL                  DiagFlowCtrl;  /* for control purposes       */
	SK_PNMI_STRUCT_DATA      PnmiBackup;    /* backup structure for PNMI  */
	SK_BOOL                  WasIfUp[SK_MAX_MACS];
	SK_U32                   WolSpeedType;   /* Wol speed type */
	SK_U32                   WolDuplexType;  /* Wol type; TRUE==FD; FALSE==HD */
	char                     *pVirtMemAddr;
	wait_queue_head_t        irq_wait;
	SK_U32                   AllocFlag;     /* alloc flag of resources    */
#ifdef USE_TIST_FOR_RESET
	int                      AdapterResetState;
	SK_U32                   MinTistLo;
	SK_U32                   MinTistHi;
#endif
	SK_BOOL					AvbModeEnabled;
	int						NumTxQueues;
	int						DefaultTxQ;	/* Queue index for non classified traffic */
#ifdef Y2_RECOVERY
	int                      LastPort;       /* port for curr. handled rx */
	int                      LastOpc;        /* last rx LEs opcode	      */
#endif
#ifdef Y2_SYNC_CHECK
	unsigned long            FramesWithoutSyncCheck; /* since last check  */
#endif
	SK_BOOL                  RequestedIrq;
	unsigned int             InterruptSource;
	SK_BOOL                  Suspended;
#ifdef MV_INCLUDE_SDK_SUPPORT
	SK_BOOL                  FwRun;
	SK_BOOL                  FwState;
	SK_U32                   SdkQH;
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	SK_U16                   DriverLinkStat;
#endif
	spinlock_t               FwFifoLock;
	SK_U32                   *pFwBuffer;
	int                      FwBufferLen;
	SK_U64                   TimeBuf;
#endif

#ifdef SK98LIN_DIAG_LOOPBACK
	SK_BOOL                  LoopbackRunning;
	SK_U32                   YukonLoopbackStatus;
	SK_U32                   YukonLoopbackTestPattern;
	struct sk_buff           *pYukonLoopbackSendMsg;
#endif
};

#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/

