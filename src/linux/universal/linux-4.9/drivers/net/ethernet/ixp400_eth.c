/*
 * File: ixp400_eth.c
 *
 * Author: Intel Corporation
 *
 * IXP400 Ethernet Driver for Linux
 *
 * @par
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2004-2007  Intel Corporation. All Rights Reserved. 
 * 
 * @par 
 * This software program is licensed subject to the GNU
 * General Public License (GPL). Version 2, June 1991, available at
 * http://www.fsf.org/copyleft/gpl.html
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

/* 
 * DESIGN NOTES:
 * This driver is written and optimized for Intel Xscale technology.
 *
 * SETUP NOTES:
 * By default, this driver uses predefined MAC addresses.
 * These are set in global var 'default_mac_addr' in this file.
 * If required, these can be changed at run-time using
 * the 'ifconfig' tool.
 *
 * Example - to set ixp0 MAC address to 00:02:B3:66:88:AA, 
 * run ifconfig with the following arguments:
 *
 *   ifconfig ixp0 hw ether 0002B36688AA
 *
 * (more information about ifconfig is available thru ifconfig -h)
 *
 * Example - to set up the ixp1 IP address to 192.168.10.1
 * run ifconfig with the following arguments:
 * 
 * ifconfig ixp1 192.168.10.1 up
 *
 */


/*
 * System-defined header files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/socket.h>
#include <linux/cache.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/mach-types.h>
#include <net/pkt_sched.h>
#include <net/ip.h>
#include <linux/sysctl.h>
#include <linux/unistd.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#ifdef CONFIG_XFRM
#include <net/xfrm.h>
#endif

/*
 * Intel IXP400 Software specific header files
 */
#include <IxQMgr.h>
#include <IxEthAcc.h>
#include <IxEthDB.h>
#include <IxEthMii.h>
#include <IxEthNpe.h>
#include <IxNpeDl.h>
#include <IxNpeMh.h>
#include <IxFeatureCtrl.h>
#include <IxVersionId.h>
#include <IxOsal.h>
#include <IxQueueAssignments.h>
#include <IxErrHdlAcc.h>
#include <IxParityENAcc.h>

#define __get_cpu_var(var)	(*this_cpu_ptr(&(var)))

#ifdef CONFIG_XSCALE_PMU_TIMER
/* We want to use interrupts from the XScale PMU timer to
 * drive our NPE Queue Dispatcher loop.  But if this #define
 * is set, then it means the system is already using this timer
 * so we cannot.
 */
#error "XScale PMU Timer not available (CONFIG_XSCALE_PMU_TIMER is defined)"
#endif

/*
 * Module version information
 */
MODULE_DESCRIPTION("IXP400 NPE Ethernet driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Intel Corporation");
#define MODULE_NAME "ixp400_eth"
#define MOD_VERSION "1.7"

/* 
 * Non-user configurable private variable 
 */

#if defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
/* NPE-A enabled flag for parity error detection configuration */
static IxParityENAccConfigOption parity_npeA_enabled = IX_PARITYENACC_DISABLE;
/* NPE-B enabled flag for parity error detection configuration */
static IxParityENAccConfigOption parity_npeB_enabled = IX_PARITYENACC_DISABLE;
/* NPE-C enabled flag for parity error detection configuration */
static IxParityENAccConfigOption parity_npeC_enabled = IX_PARITYENACC_DISABLE;
/* Flag to determine if NPE softreset is being initialized */
static u32 npe_error_handler_initialized = 0;
#endif

/* 
 * Module parameters 
 */
static int npe_learning = 0;      /* default : NPE learning & filtering enable */
static int log_level = 0;         /* default : no log */
static int no_ixp400_sw_init = 0; /* default : init core components of the IXP400 Software */
#if defined(CONFIG_MACH_CAMBRIA)
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#elif  defined(CONFIG_TONZE)
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#elif  defined(CONFIG_MI424WR)
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#elif  defined(CONFIG_USR8200)
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#elif  defined(CONFIG_ARCH_ADI_COYOTE_WRT300N)
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#else
static int no_phy_scan = 0;       /* default : do phy discovery */
static int hss_coexist = 0;	  /* default : HSS coexist disabled */
#endif
static int phy_reset = 0;         /* default : no phy reset */
static int npe_error_handler = 0; /* default : no npe error handler */

/* 
 * maximum number of ports supported by this driver ixp0, ixp1 ....
 * The default is to configure all ports defined in EthAcc component
 */
#ifdef CONFIG_IXP400_ETH_NPEC_ONLY
static int dev_max_count = 1; /* only NPEC is used */
#elif defined (CONFIG_IXP400_ETH_NPEB_ONLY) || defined (CONFIG_MACH_WG302V2)
static int dev_max_count = 1; /* only NPEB is used */
#elif defined (CONFIG_ARCH_IXDP425) || defined(CONFIG_ARCH_IXDPG425)\
      || defined (CONFIG_ARCH_ADI_COYOTE) || defined (CONFIG_MACH_AVILA) || defined (CONFIG_MACH_CAMBRIA) || defined (CONFIG_TONZE) || defined (CONFIG_MACH_KIXRP435) || defined (CONFIG_MACH_USR8200) \
      || defined (CONFIG_MACH_PRONGHORNMETRO) \
      || defined (CONFIG_MACH_PRONGHORN) || defined (CONFIG_MACH_MI424WR)

static int dev_max_count = 2; /* only NPEB and NPEC */
#elif defined (CONFIG_ARCH_IXDP465) || defined(CONFIG_MACH_IXDP465)
static int dev_max_count = 3; /* all NPEs are used */
#endif

static int datapath_poll = 1;     /* default : rx/tx polling, not interrupt driven*/

#ifndef CONFIG_IXP400_NAPI
/* 
 * netdev_max_backlog: ideally /proc/sys/net/core/netdev_max_backlog, but any 
 * value > 46 looks to work. This is used to control the maximum number of 
 * skbuf to push into the linux stack, and avoid the performance degradations 
 * during overflow.
 */
static int ixpdev_max_backlog = 290;

module_param(ixpdev_max_backlog, int, 0);
MODULE_PARM_DESC(ixpdev_max_backlog, "Should be set to the value of /proc/sys/net/core/netdev_max_backlog (perf affecting)");
#endif /* CONFIG_IXP400_NAPI */
module_param(datapath_poll, int, 0);
MODULE_PARM_DESC(datapath_poll, "If non-zero, use polling method for datapath instead of interrupts");
module_param(npe_learning, int, 0);
MODULE_PARM_DESC(npe_learning, "If non-zero, NPE MAC Address Learning & Filtering feature will be enabled");
module_param(log_level, int, 0);
MODULE_PARM_DESC(log_level, "Set log level: 0 - None, 1 - Verbose, 2 - Debug");
module_param(no_ixp400_sw_init, int, 0);
MODULE_PARM_DESC(no_ixp400_sw_init, "If non-zero, do not initialise Intel IXP400 Software Release core components");
module_param(no_phy_scan, int, 0);
MODULE_PARM_DESC(no_phy_scan, "If non-zero, use hard-coded phy addresses");
module_param(phy_reset, int, 0);
MODULE_PARM_DESC(phy_reset, "If non-zero, reset the phys");
module_param(dev_max_count, int, 0);
MODULE_PARM_DESC(dev_max_count, "Number of devices to initialize");
module_param(npe_error_handler, int, 0);
MODULE_PARM_DESC(npe_error_handler, "If non-zero, NPE error handling feature will be enabled");
module_param(hss_coexist, int, 0);
MODULE_PARM_DESC(hss_coexist, "If non-zero, HSS-Ethernet coexist feature will be enabled");
/* devices will be called ixp0 and ixp1 */
#define DEVICE_NAME "ixp"

/* boolean values for PHY link speed, duplex, and autonegotiation */
#define PHY_SPEED_10    		(0)
#define PHY_SPEED_100   		(1)
#define PHY_DUPLEX_HALF 		(0)
#define PHY_DUPLEX_FULL 		(1)
#define PHY_AUTONEG_OFF 		(0)
#define PHY_AUTONEG_ON  		(1)

/* 
 * will clean skbufs from the sw queues when they are older
 * than this time (this mechanism is needed to prevent the driver 
 * holding skbuf and memory space for too long). Unit are in seconds
 * because the timestamp in buffers are in seconds.
 */
#define BUFFER_MAX_HOLD_TIME_S 		(3)

/* maintenance time (jiffies) */
#define DB_MAINTENANCE_TIME 		(IX_ETH_DB_MAINTENANCE_TIME*HZ)

/* 
 * Time before kernel will decide that the driver had stuck in transmit 
 * (jiffies) 
 */
#define DEV_WATCHDOG_TIMEO		(10*HZ)

/* Interval between media-sense/duplex checks (jiffies) */
#define MEDIA_CHECK_INTERVAL		(3*HZ)

/* Dictates the rate (per second) at which the NPE queues are serviced */
/*   4000 times/sec = 37 mbufs/interrupt at line rate */
#define QUEUE_DISPATCH_TIMER_RATE	(4000)

/* NPE Message Handler Polling Frequency (in milliseconds) */
#define NPEMH_MS_POLL_FREQ		(5)

/* 
 * Macro to convert number PMU timer rate to interrupt cycles. The value
 * computed is based on NPEMH_MS_POLL_FREQ and QUEUE_DISPATCH_TIMER_RATE 
 */
#define QUEUE_DISPATCH_TIMER_TO_INT_CYCLE_CONVERT \
		(NPEMH_MS_POLL_FREQ * 1000) / \
		(USEC_PER_SEC / QUEUE_DISPATCH_TIMER_RATE)  

#ifdef CONFIG_IXP400_NAPI

/* The "weight" value for NAPI defines how many receive buffers
 * are processed each time the device is polled.
 * This value is configurable.
 * - smaller values: polled more often, perhaps less latency
 * - higher values: polled less often, perhaps less CPU utilization
 * - recommended value: 32
 */
#define IXP400_NAPI_WEIGHT 		(64)

#else

/* Tunable value, the highest helps for steady rate traffic, but too high
 * increases the packet drops. E.g. TCP or UDP traffic generated from a PC.
 * The lowest helps for well-timed traffic (e.g. smartApps). Also, the lowest
 * is sometimes better for extremely bursty traffic
 *
 * Value range from 5 up to 8. 6 is a "good enough" compromize.
 *
 * With this setting (6), small drops of traffic may be observed at 
 * high traffic rates. To measure the maximum throughput between the
 * ports of the driver,
 * - Modify /proc/sys/net/core/netdev_max_backlog value in the kernel 
 * - Adjust netdev_max_backlog=n in the driver's command line
 * in order to get the best rates depending on the testing tool 
 * and the OS load.
 *
 */
#define BACKLOG_TUNE 			(6)

#endif /* CONFIG_IXP400_NAPI */

/* number of packets to prealloc for the Rx pool (per driver instance) */
#define RX_MBUF_POOL_SIZE		(128)

/* Maximum number of packets in Tx+TxDone queue */
#define TX_MBUF_POOL_SIZE		(256)

/* VLAN header size definition. Only if VLAN 802.1Q frames are enabled */
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define VLAN_HDR			(4)
#else
#define VLAN_HDR			(0)
#endif

/* Number of mbufs in sw queue */
#define MB_QSIZE  (256) /* must be a power of 2 and > TX_MBUF_POOL_SIZE */
#define SKB_QSIZE (256) /* must be a power of 2 and > RX_MBUF_POOL_SIZE */

/* Maximum number of addresses the EthAcc multicast filter can hold */
#define IX_ETH_ACC_MAX_MULTICAST_ADDRESSES (256)

/* Parameter passed to Linux in ISR registration (cannot be 0) */
#define IRQ_ANY_PARAMETER 		(1)

/* 
 * The size of the SKBs to allocate is more than the MSDU size.
 *
 * skb->head starts here
 * (a) 16 bytes reserved in dev_sk_alloc()
 * (b) 48 RNDIS optional reserved bytes
 * (c) 2 bytes for necessary alignment of the IP header on a word boundary
 * skb->data starts here, the NPE will store the payload at this address
 * (d)      14 bytes  (dev->dev_hard_len)
 * (e)      1500 bytes (dev->mtu, can grow up for Jumbo frames)
 * (f)      4 bytes fcs (stripped out by MAC core)
 * (g)      xxx round-up needed for a NPE 64 bytes boundary
 * skb->tail the NPE will not write more than these value
 * (h)      yyy round-up needed for a 32 bytes cache line boundary
 * (i) sizeof(struct sk_buff). there is some shared info sometimes
 *     used by the system (allocated in net/core/dev.c)
 *
 * The driver private structure stores the following fields
 *
 *  msdu_size = d+e : used to set the msdu size
 *  replenish_size = d+e+g : used for replenish
 *  pkt_size = b+c+d+e+g+h : used to allocate skbufs
 *  alloc_size = a+b+c+d+e+g+h+i, compare to skb->truesize
*/

#ifdef CONFIG_USB_RNDIS
#define HDR_SIZE			(2 + 48)
#else
#define HDR_SIZE			(2)
#endif

/* dev_alloc_skb reserves 16 bytes in the beginning of the skbuf
 * and sh_shared_info at the end of the skbuf. But the value used
 * used to set truesize in skbuff.c is struct sk_buff (???). This
 * behaviour is reproduced here for consistency.
*/
#define SKB_RESERVED_HEADER_SIZE	(16)
#define SKB_RESERVED_TRAILER_SIZE	sizeof(struct sk_buff)

/* NPE-A Functionality: Ethernet only */
#define IX_ETH_NPE_A_IMAGE_ID		IX_NPEDL_NPEIMAGE_NPEA_ETH

/* NPE-A Functionality: HSS co-exist */
//#define IX_HSS_ETH_NPE_A_IMAGE_ID	IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST

/* NPE-B Functionality: Ethernet only */
#define IX_ETH_NPE_B_IMAGE_ID		IX_NPEDL_NPEIMAGE_NPEB_ETH

/* NPE-C Functionality: Ethernet only  */
#define IX_ETH_NPE_C_IMAGE_ID		IX_NPEDL_NPEIMAGE_NPEC_ETH

/*
 * Macros to turn on/off debug messages
 */
/* Print kernel error */
#define P_ERROR(args...) \
    printk(KERN_ERR MODULE_NAME ": " args)
/* Print kernel warning */
#define P_WARN(args...) \
    printk(KERN_WARNING MODULE_NAME ": " args)
/* Print kernel notice */
#define P_NOTICE(args...) \
    printk(KERN_NOTICE MODULE_NAME ": " args)
/* Print kernel info */
#define P_INFO(args...) \
    printk(KERN_INFO MODULE_NAME ": " args)
/* Print verbose message. Enabled/disabled by 'log_level' param */
#define P_VERBOSE(args...) \
    if (log_level >= 1) printk(MODULE_NAME ": " args)
/* Print debug message. Enabled/disabled by 'log_level' param  */
#define P_DEBUG(args...) \
    if (log_level >= 2) { \
        printk("%s: %s()\n", MODULE_NAME, __FUNCTION__); \
        printk(args); }


#ifdef DEBUG
/* Print trace message */
#define TRACE \
    if (log_level >= 2) \
    	printk("%s: %s(): line %d\n", MODULE_NAME, __FUNCTION__, __LINE__)
#else
/* no trace */
#define TRACE 
#endif

/* extern Linux kernel data */
extern unsigned long loops_per_jiffy; /* used to calculate CPU clock speed */

/* 
 * prototypes for locally defined pmu_timer functions
 */
static int dev_pmu_timer_setup(void);
static void dev_pmu_timer_unload(void);
static void dev_pmu_timer_disable(void);
static void dev_pmu_timer_restart(void);
static int dev_pmu_timer_init(void);
static int datapath_poll_activatable_check(void);

/*
 * Prototype for message handler polling function
 */
static inline int npemh_poll(void *data);

/*
 * Prototype for parity error detection and handler
 */

#if defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
static int __init parity_npe_error_handler_init(void);
static void parity_npe_error_handler_uninit(void);
static void parity_npe_recovery_done_cb(IxErrHdlAccErrorEventType event_type);
static void parity_error_cb(void);
#else
static int __init parity_npe_error_handler_init(void){ return 0; }
static void parity_npe_error_handler_uninit(void){}
#endif

/* internal Access layer entry point */
extern void 
ixEthTxFrameDoneQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId);

/* Private device data */
typedef struct {
    spinlock_t lock;  /* multicast management lock */
    
    unsigned int msdu_size;
    unsigned int replenish_size;
    unsigned int pkt_size;
    unsigned int alloc_size;

    struct net_device_stats stats; /* device statistics */

    IxEthAccPortId port_id; /* EthAcc port_id */

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED
    /* private scheduling discipline */
    struct Qdisc *qdisc;
#endif

    /* Implements a software queue for mbufs 
     * This queue is written in the tx done process and 
     * read during tx. Because there is 
     * 1 reader and 1 writer, there is no need for
     * a locking algorithm.
     */

    /* mbuf Tx queue indexes */
    unsigned int mbTxQueueHead;
    unsigned int mbTxQueueTail;

    /* mbuf Rx queue indexes */
    unsigned int mbRxQueueHead;
    unsigned int mbRxQueueTail;

    /* software queue containers */
    IX_OSAL_MBUF *mbTxQueue[MB_QSIZE];
    IX_OSAL_MBUF *mbRxQueue[MB_QSIZE];

    /* RX MBUF pool */
    IX_OSAL_MBUF_POOL *rx_pool;

    /* TX MBUF pool */
    IX_OSAL_MBUF_POOL *tx_pool;

    /* id of thread for the link duplex monitoring */
    struct task_struct *maintenanceCheckThreadId;

    /* mutex locked by thread, until the thread exits */
    struct semaphore *maintenanceCheckThreadComplete;

    /* Used to stop the kernel thread for link monitoring. */
    volatile BOOL maintenanceCheckStopped;

    /* workqueue used for tx timeout */
    struct workqueue_struct *timeout_workq;
    struct work_struct timeout_work;

    /* used to control the message output */
    UINT32 devFlags;
    struct net_device *ndev;
} priv_data_t;

/* Collection of boolean PHY configuration parameters */
typedef struct {
    BOOL speed100;
    BOOL duplexFull;
    BOOL autoNegEnabled;
    BOOL linkMonitor;
} phy_cfg_t;

/* Structure to associate NPE image ID with NPE port ID */
typedef struct {
    UINT32 npeImageId;
    IxNpeDlNpeId npeId;
} npe_info_t;


static int dev_eth_probe(struct device *dev);
static int dev_eth_remove(struct device *dev);
static void dev_eth_release(struct device *dev);

static struct device_driver ixp400_eth_driver = {
    .name	= MODULE_NAME,
    .bus	= &platform_bus_type,
    .probe	= dev_eth_probe,
    .remove 	= dev_eth_remove,
};

static struct platform_device ixp400_eth_devices[IX_ETH_ACC_NUMBER_OF_PORTS] = {
    {
#if IX_ETH_ACC_NUMBER_OF_PORTS > 0
	.name 	= MODULE_NAME,
#if defined (CONFIG_IXP400_ETH_NPEC_ONLY) || defined (CONFIG_MACH_KIXRP435) || defined (CONFIG_MACH_CAMBRIA)
	.id   	= IX_ETH_PORT_2,
#else
	.id   	= IX_ETH_PORT_1,
#endif
	.dev	= 
	    {
		.release	= dev_eth_release,
	    },
#if IX_ETH_ACC_NUMBER_OF_PORTS > 1
    },
    {
	.name 	= MODULE_NAME,
#if defined(CONFIG_MACH_KIXRP435)
	.id   	= IX_ETH_PORT_3,
#elif defined(CONFIG_MACH_CAMBRIA)
	.id			= IX_ETH_PORT_3,
#else
	.id   	= IX_ETH_PORT_2,
#endif
	.dev	= 
	    {
		.release	= dev_eth_release,
	    },
#if IX_ETH_ACC_NUMBER_OF_PORTS > 2 && !defined (CONFIG_CPU_IXP43X)
    },
    {
	.name 	= MODULE_NAME,
	.id   	= IX_ETH_PORT_3,
	.dev	= 
	    {
		.release	= dev_eth_release,
	    },
#endif /* IX_ETH_ACC_NUMBER_OF_PORTS > 2 */
#endif /* IX_ETH_ACC_NUMBER_OF_PORTS > 1 */
#endif /* IX_ETH_ACC_NUMBER_OF_PORTS > 0 */
    }
};

/* Platform device is statically allocated, nothing to be released */
static void dev_eth_release(struct device *dev)
{
}


/*
 * STATIC VARIABLES
 *
 * This section sets several default values for each port.
 * These may be edited if required.
 */

/* values used inside the irq */
static unsigned long timer_countup_ticks;
static IxQMgrDispatcherFuncPtr dispatcherFunc;
static struct timeval  irq_stamp;  /* time of interrupt */

/* Implements a software queue for skbufs 
 * This queue is written in the tx done process and 
 * read during rxfree. Because there is 
 * 1 reader and 1 writer, there is no need for
 * a locking algorithm.
 *
 * This software queue is shared by both ports.
 */

/* skbuf queue indexes */
static unsigned int skQueueHead;
static unsigned int skQueueTail;

#ifdef CONFIG_IXP400_ETH_SKB_RECYCLE
/* software queue containers */
static struct sk_buff *skQueue[SKB_QSIZE];
#endif

/* 
 * The PHY addresses mapped to Intel IXP400 Software EthAcc ports.
 *
 * These are hardcoded and ordered by increasing ports.
 * Overwriting these values by a PHY discovery is disabled by default but
 * can optionally be enabled if required
 * by passing module param no_phy_scan with a zero value
 *
 * Up to 32 PHYs may be discovered by a phy scan. Addresses
 * of all PHYs found will be stored here, but only the first
 * 2 will be used with the Intel IXP400 Software EthAcc ports.
 *
 * See also the function phy_init() in this file.
 *
 * NOTE: The hardcoded PHY addresses have been verified on
 * the IXDP425 and Coyote (IXP4XX RG) Development platforms.
 * However, they may differ on other platforms.
 */
static int phyAddresses[IXP400_ETH_ACC_MII_MAX_ADDR] =
{
//#if defined(CONFIG_ARCH_ADI_COYOTE)
//    4, /* Port 1 (IX_ETH_PORT_1) - Connected to PHYs 1-4      */
//    5, /* Port 2 (IX_ETH_PORT_2) - Only connected to PHY 5    */
//
//    3,  /******************************************************/
//    2,  /* PHY addresses on Coyote platform (physical layout) */
//    1   /* (4 LAN ports, switch)  (1 WAN port)                */
        /*       ixp0              ixp1                       */
        /*  ________________       ____                       */
        /* /_______________/|     /___/|                      */
	/* | 1 | 2 | 3 | 4 |      | 5 |                       */
        /* ----------------------------------------           */
#if defined(CONFIG_TONZE)
    32,
    9
#elif defined(CONFIG_MACH_USR8200)
    /* 1 PHY per NPE port */
    16, /* Port 1 (IX_ETH_PORT_1 / NPE B) */
    9,  /* Port 2 (IX_ETH_PORT_2 / NPE C) */

#elif defined(CONFIG_ARCH_IXDP425)
    /* 1 PHY per NPE port */
    0, /* Port 1 (IX_ETH_PORT_1 / NPE B) */
    1  /* Port 2 (IX_ETH_PORT_2 / NPE C) */

#elif defined(CONFIG_ARCH_IXDP465) || defined(CONFIG_MACH_IXDP465)
    /* 1 PHY per NPE port */
    0, /* Port 1 (IX_ETH_PORT_1 / NPE B) */
    1, /* Port 2 (IX_ETH_PORT_2 / NPE C) */
    2  /* Port 3 (IX_ETH_PORT_3 / NPE A) */

#elif defined(CONFIG_MACH_IXDPG425)
    5, /* Port 1 (ixp0) - Connected to switch via PHY 5 */
    4, /* Port 2 (ixp1) - Only connected to PHY 4       */
    0, /* 4 port switch - PHY 0..3                      */
    1,
    2,
    3
#elif defined(CONFIG_MACH_KIXRP435)
    1, /* Port 1 (IX_ETH_PORT_2) - Connected to PHYs 1-4      */
    5, /* Port 2 (IX_ETH_PORT_3) - Only connected to PHY 5    */

    2,  /*********************************************************/
    3,  /* PHY addresses on KIXRP435 platform (physical layout)  */
    4   /* (4 LAN ports, switch)  (1 WAN port)                   */
        /*       ixp1              ixp2                          */
        /*  ________________       ____                          */
        /* /_______________/|     /___/|                         */
	/* | 1 | 2 | 3 | 4 |      | 5 |                          */
        /* ----------------------------------------              */
#elif defined(CONFIG_MACH_CAMBRIA)
    2, /* Port 1 (IX_ETH_PORT_2) - Connected to PHYs 1-4      */
    1, /* Port 2 (IX_ETH_PORT_3) - Only connected to PHY 5    */
#else
    /* other platforms : suppose 1 PHY per NPE port */
    0, /* PHY address for EthAcc Port 1 (IX_ETH_PORT_1 / NPE B) */
    1  /* PHY address for EthAcc Port 2 (IX_ETH_PORT_2 / NPE C) */

#endif
};

/* 
 * Port ID to A default_phy_cfg and phyAddresses index loopkup mapping table
 */
static long portIdPhyIndexMap[] =
{
#if defined(CONFIG_CPU_IXP46X)
	0, /* NPE-B */
	1, /* NPE-C */
	2  /* NPE-A */
#elif defined(CONFIG_CPU_IXP43X)
	-1, /* Invalid */
	0,  /* NPE-C */
	1   /* NPE-A */
#else
/* 
 * CONFIG_CPU_IXP42X is not define by the kernel. Hence we assume IXP42X if
 * flags above is not match
 */
	0, /* NPE-B */
	1  /* NPE-C */
#endif
};

/* The default configuration of the phy on each Intel IXP400 Software EthAcc port.
 *
 * This configuration is loaded when the phy's are discovered.
 * More PHYs can optionally be configured here by adding more
 * configuration entries in the array below.  The PHYs will be
 * configured in the order that they are found, starting with the
 * lowest PHY address.
 *
 * See also function phy_init() in this file
 */
static phy_cfg_t default_phy_cfg[32] =
{
//#if defined(CONFIG_ARCH_ADI_COYOTE)
//    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE},/* Port 0: NO link */
//    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 1: monitor the link */
//    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
//    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
//    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}
#if defined(CONFIG_TONZE)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}, /* Port 0: monitor the link*/
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}  /* Port 1: monitor the link*/
#elif defined(CONFIG_MACH_USR8200)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}, /* Port 0: monitor the phy */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}  /* Port 1: monitor the link */
#elif defined(CONFIG_ARCH_ADI_COYOTE_WRT300N)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}, /* Port 0: monitor the phy */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}  /* Port 1: monitor the link */
#elif defined(CONFIG_ARCH_IXDP425)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 0: monitor the phy */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}  /* Port 1: monitor the link */

#elif defined(CONFIG_ARCH_IXDP465) || defined(CONFIG_MACH_IXDP465)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 0: monitor the phy */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 1: monitor the link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE} /* Port 2: ignore the link */


#elif defined(CONFIG_MACH_IXDPG425)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}, /* Port 0: NO link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE},  /* Port 1: monitor the link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}
#elif defined(CONFIG_MACH_KIXRP435)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}, /* Port 1: NO link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE},  /* Port 2: monitor the link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE},
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,FALSE}
#elif defined(CONFIG_MACH_CAMBRIA)
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 1: monitor the link */
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}  /* Port 2: monitor the link */
#else
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}, /* Port 0: monitor the link*/
    {PHY_SPEED_100, PHY_DUPLEX_FULL, PHY_AUTONEG_ON,TRUE}  /* Port 1: monitor the link*/

#endif
};

/* Default MAC addresses for EthAcc Ports 1 and 2 (using Intel MAC prefix) 
 * Default is 
 *   IX_ETH_PORT_1 -> MAC 00:02:b3:01:01:01
 *   IX_ETH_PORT_2 -> MAC 00:02:b3:02:02:02
 *   IX_ETH_PORT_3 -> MAC 00:02:b3:03:03:03
*/
static IxEthAccMacAddr default_mac_addr[] =
{
    {{0x00, 0x02, 0xB3, 0x01, 0x01, 0x01}}  /* EthAcc Port 0 */
    ,{{0x00, 0x02, 0xB3, 0x02, 0x02, 0x02}} /* EthAcc Port 1 */
#if defined (CONFIG_ARCH_IXDP465) || defined(CONFIG_MACH_IXDP465) ||\
    defined (CONFIG_MACH_KIXRP435) || defined(CONFIG_MACH_CAMBRIA)
    ,{{0x00, 0x02, 0xB3, 0x03, 0x03, 0x03}} /* EthAcc Port 2 */
#endif
};

/* Default mapping of  NpeImageIds for EthAcc Ports 
 * Default is 
 *   IX_ETH_PORT_1 -> IX_ETH_NPE_B
 *   IX_ETH_PORT_2 -> IX_ETH_NPE_C
 *   IX_ETH_PORT_3 -> IX_ETH_NPE_A
 *
 * This mapping cannot be changed.
 * 
 */
static npe_info_t default_npeImageId[]=
{
    {IX_ETH_NPE_B_IMAGE_ID, IX_NPEDL_NPEID_NPEB}, /* Npe firmware for EthAcc Port 0  */
    {IX_ETH_NPE_C_IMAGE_ID, IX_NPEDL_NPEID_NPEC}, /* Npe firmware for EthAcc Port 1  */
#if defined (CONFIG_ARCH_IXDP465) || defined(CONFIG_MACH_IXDP465) || \
    defined (CONFIG_MACH_KIXRP435) || defined (CONFIG_MACH_CAMBRIA)
    {IX_ETH_NPE_A_IMAGE_ID, IX_NPEDL_NPEID_NPEA}  /* Npe firmware for EthAcc Port 2  */
#endif
};

/* Default mapping of ethAcc portId for devices ixp0 and ixp1 
 * Default is 
 *   device ixp0 ---> IX_ETH_PORT_1
 *   device ixp1 ---> IX_ETH_PORT_2
 *
 * On a system with NPE C, and a single PHY, 
 * - swap the entries from this array to assign ixp0 to IX_ETH_PORT_2
 * - set phyAddress[] with a single entry and the correct PHY address
 * - set default_phy_cfg[] with a single entry and the initialisation PHY setup
 * - starts the driver with the option dev_max_count=1 no_phy_scan=1 
 *      dev_max_count=1 will create 1 driver instance ixp0
 *      no_phy_scan=1 will bypass the default mapping set by phy_init
 *
*/
static IxEthAccPortId default_portId[] =
{
#ifdef CONFIG_IXP400_ETH_NPEC_ONLY
    /* configure port for NPE C first */
    IX_ETH_PORT_2, /* EthAcc Port 2 for ixp0 */
    IX_ETH_PORT_1
#elif defined (CONFIG_IXP400_ETH_NPEB_ONLY)
    /* configure port for NPE B first */
    IX_ETH_PORT_1, /* EthAcc Port 1 for ixp0 */
    IX_ETH_PORT_2
#elif defined (CONFIG_ARCH_IXDP465) || defined (CONFIG_MACH_IXDP465)
    /* configure port for NPE B first */
    IX_ETH_PORT_1, /* EthAcc Port 1 for ixp0 */
    IX_ETH_PORT_2, /* EthAcc Port 2 for ixp1 */
    IX_ETH_PORT_3  /* EthAcc Port 3 for ixp2 */
#elif defined (CONFIG_MACH_KIXRP435)
    /* configure port for NPE C first */
    IX_ETH_PORT_2, /* EthAcc Port 2 for ixp1 */
    IX_ETH_PORT_3  /* EthAcc Port 3 for ixp2 */
#elif defined (CONFIG_MACH_CAMBRIA)
    /* configure port for NPE C first */
    IX_ETH_PORT_2, /* EthAcc Port 2 for ixp1 */
    IX_ETH_PORT_3  /* EthAcc Port 3 for ixp2 */
#elif defined (CONFIG_MACH_USR8200)
    /* configure port for NPE C first */
    IX_ETH_PORT_2, /* EthAcc Port 2 for ixp1 */
    IX_ETH_PORT_1  /* EthAcc Port 3 for ixp2 */
#else
    /* configure and use both ports */
    IX_ETH_PORT_1, /* EthAcc Port 1 for ixp0 */
    IX_ETH_PORT_2  /* EthAcc Port 2 for ixp1 */
#endif
};

/* Mutex lock used to coordinate access to IxEthAcc functions
 * which manipulate the MII registers on the PHYs
 */
static struct mutex *miiAccessMutex;

/* mutex locked when maintenance is being performed */
static struct semaphore *maintenance_mutex;

#ifdef CONFIG_IXP400_NAPI
/* Pointer to net device which can handle NAPI rx polling. 
 * This device must be in the "open" state for NAPI rx polling to work.
 * When this device is closed, set it to another open device if possible.
 * Should be initialized to a default device in the probe routine,
 * then set/reset whenever a device is opened/closed.
 */
static struct net_device *rx_poll_dev = NULL;
#endif

/*
 * ERROR COUNTERS
 */

static UINT32 skbAllocFailErrorCount = 0;
static UINT32 replenishErrorCount = 0;

/* Workqueue for maintenance task */
static struct workqueue_struct *maintenance_workq = NULL;


/*
 * ERROR NUMBER FUNCTIONS
 */

/* Convert IxEthAcc return codes to suitable Linux return codes */
static int convert_error_ethAcc (IxEthAccStatus error)
{
    switch (error)
    {
	case IX_ETH_ACC_SUCCESS:            return  0;
	case IX_ETH_ACC_FAIL:               return -1;
	case IX_ETH_ACC_INVALID_PORT:       return -ENODEV;
	case IX_ETH_ACC_PORT_UNINITIALIZED: return -EPERM;
	case IX_ETH_ACC_MAC_UNINITIALIZED:  return -EPERM;
	case IX_ETH_ACC_INVALID_ARG:        return -EINVAL;
	case IX_ETH_TX_Q_FULL:              return -EAGAIN;
	case IX_ETH_ACC_NO_SUCH_ADDR:       return -EFAULT;
	default:                            return -1;
    };
}

/*
 * DEBUG UTILITY FUNCTIONS
 */
#ifdef DEBUG_DUMP

static void hex_dump(void *buf, int len)
{
    int i;

    for (i = 0 ; i < len; i++)
    {
	printk("%02x", ((u8*)buf)[i]);
	if (i%2)
	    printk(" ");
	if (15 == i%16)
	    printk("\n");
    }
    printk("\n");
}

static void mbuf_dump(char *name, IX_OSAL_MBUF *mbuf)
{
    printk("+++++++++++++++++++++++++++\n"
	"%s MBUF dump mbuf=%p, m_data=%p, m_len=%d, len=%d\n",
	name, mbuf, IX_OSAL_MBUF_MDATA(mbuf), IX_OSAL_MBUF_MLEN(mbuf), IX_OSAL_MBUF_PKT_LEN(mbuf));
    printk(">> mbuf:\n");
    hex_dump(mbuf, sizeof(*mbuf));
    printk(">> m_data:\n");
    hex_dump(__va(IX_OSAL_MBUF_MDATA(mbuf)), IX_OSAL_MBUF_MLEN(mbuf));
    printk("\n-------------------------\n");
}

static void skb_dump(char *name, struct sk_buff *skb)
{
    printk("+++++++++++++++++++++++++++\n"
	"%s SKB dump skb=%p, data=%p, tail=%p, len=%d\n",
	name, skb, skb->data, skb->tail, skb->len);
    printk(">> data:\n");
    hex_dump(skb->data, skb->len);
    printk("\n-------------------------\n");
}
#endif

/*
 * MEMORY MANAGEMENT
 */

/* XScale specific : preload a cache line to memeory */
static inline void dev_preload(void *virtualPtr) 
{
   __asm__ (" pld [%0]\n" : : "r" (virtualPtr));
}

static inline void dev_skb_preload(struct sk_buff *skb)
{
    /* from include/linux/skbuff.h, skb->len field and skb->data filed 
     * don't share the same cache line (more than 32 bytes between the fields)
     */
    dev_preload(&skb->len);
    dev_preload(&skb->data);
}

/*
 * BUFFER MANAGEMENT
 */

/*
 * Utility functions to handle software queues
 * Each driver has a software queue of skbufs and 
 * a software queue of mbufs for tx and mbufs for rx
 */
#define MB_QINC(index) ((index)++ & (MB_QSIZE - 1))
#define SKB_QINC(index) ((index)++ & (SKB_QSIZE - 1))
#define SKB_QFETCH(index) ((index) & (SKB_QSIZE - 1))

/**
 * mbuf_swap_skb : links/unlinks mbuf to skb
 */
static inline struct sk_buff *mbuf_swap_skb(IX_OSAL_MBUF *mbuf, struct sk_buff *skb)
{
    struct sk_buff *res = IX_OSAL_MBUF_PRIV(mbuf);

    IX_OSAL_MBUF_PRIV(mbuf) = skb;

    if (!skb)
	return res;
    
    IX_OSAL_MBUF_MDATA(mbuf) = skb->data;
    IX_OSAL_MBUF_MLEN(mbuf) = IX_OSAL_MBUF_PKT_LEN(mbuf) = skb->len;

    return res;
}

/*
 * dev_skb_mbuf_free: relaese skb to the kernel and mbuf to the pool 
 * This function is called during port_disable and has no constraint
 * of performances.
*/
static inline void mbuf_free_skb(IX_OSAL_MBUF *mbuf)
{
    TRACE;
    /* chained buffers can be received during port
     * disable after a mtu size change.
     */
    do
    {
	/* unchain and free the buffers */
	IX_OSAL_MBUF *next = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf);
	IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf) = NULL;
	if (IX_OSAL_MBUF_PRIV(mbuf) != NULL)
	{
	    /* this buffer has an skb attached, free both of them */	    
	    dev_kfree_skb_any(mbuf_swap_skb(mbuf, NULL));
	    IX_OSAL_MBUF_POOL_PUT(mbuf);
	}
	else
	{
	    /* this buffer has no skb attached, just ignore it */
	}
	mbuf = next;
    }
    while (mbuf != NULL);
    
    TRACE;
}


/* dev_skb_dequeue: remove a skb from the skb queue */
static inline struct sk_buff * dev_skb_dequeue(priv_data_t *priv)
{
    struct sk_buff *skb;

#ifdef CONFIG_IXP400_ETH_SKB_RECYCLE
    unsigned int repeat = 0;

    do {
	if (skQueueHead != skQueueTail)
	{
	    /* get from queue (fast) packet is ready for use 
	     * because the fields are reset during the enqueue
	     * operations
	     */
	    skb = skQueue[SKB_QINC(skQueueTail)];
	    if (skQueueHead != skQueueTail)
	    {
		/* preload the next skbuf : this is an optimisation to 
		 * avoid stall cycles when acessing memory.
		 */
		dev_skb_preload(skQueue[SKB_QFETCH(skQueueTail)]);
	    }
	    /* check the skb size fits the driver requirements 
	     */
	    if (skb->truesize >= priv->alloc_size)
	    {
		return skb;
	    }
	    /* the skbuf is too small : put it to pool (slow)
	     * This may occur when the ports are configured
	     * with a different mtu size.
	     */
	    dev_kfree_skb_any(skb);
	}


	/*
	 * Attempt to replenish the skb pool from txDone queue once before
	 * trying allocating a new one
	 */
	if (!repeat++)
	{
	    UINT32 key = ixOsalIrqLock();
	    /*
	     * Service the txDone queue to replenish the skb queue entries
	     */
	    ixEthTxFrameDoneQMCallback(0, 0);

	    ixOsalIrqUnlock(key);
	}
    } while (repeat <= 1);
#endif

    /* get a skbuf from pool (slow) */
#ifdef CONFIG_IXP400_NAPI
    skb = __dev_alloc_skb(priv->pkt_size, GFP_ATOMIC|__GFP_NOWARN);
#else
    skb = dev_alloc_skb(priv->pkt_size);
#endif

    if (skb != NULL)
    {
	skb_reserve(skb, HDR_SIZE);
	skb->len = priv->replenish_size;
	IX_ACC_DATA_CACHE_INVALIDATE(skb->data, priv->replenish_size);
    }
    else
    {
	skbAllocFailErrorCount++;
    }
    
    return skb;
}

#ifdef CONFIG_IXP400_ETH_SKB_RECYCLE
/* dev_skb_enqueue: add a skb to the skb queue */
static inline void dev_skb_enqueue(priv_data_t *priv, struct sk_buff *skb)
{
    /* check for big-enough unshared skb, and check the queue 
     * is not full If the queue is full or the complete ownership of the
     * skb is not guaranteed, just free the skb.
     * (atomic counters are read on the fly, there is no need for lock)
     */

    if ((skb->truesize >= priv->alloc_size) && 
	(atomic_read(&skb->users) == 1) && 
	(!skb_cloned(skb)) &&
	(skb->fclone == SKB_FCLONE_UNAVAILABLE) &&
        (skb->destructor == NULL) && 
	(atomic_read(&skb_shinfo(skb)->dataref) == 1) &&
	(skb->nohdr == 0) &&
        (skb_shinfo(skb)->nr_frags == 0) &&
        (skb_shinfo(skb)->frag_list == NULL) &&
	(skQueueHead - skQueueTail < SKB_QSIZE))
    {
 	/* put big unshared mbuf to queue (fast)
	 *
	 * The purpose of this part of code is to reset the skb fields
	 * so they will be ready for reuse within the driver.
	 * 
	 * The following fields are reset during a skb_free and 
	 * skb_alloc sequence (see dev/core/skbuf.c). We
	 * should call the function skb_headerinit(). Unfortunately,
	 * this function is static.
	 *
	 * This code resets the current skb fields to zero,
	 * resets the data pointer to the beginning of the 
	 * skb data area, then invalidates the all payload.
	 */

	dst_release(skb->dst);

#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
	skb->sp = NULL;
#endif

#ifdef CONFIG_NETFILTER
	/* Some packets may get incorrectly process by netfilter firewall 
	 * software if CONFIG_NETFILTER is enabled and filtering is in use. 
	 * The solution is to reset the following fields in the skbuff 
	 * before re-using it on the Rx-path
	 */
        skb->nfmark = 0;
        nf_conntrack_put(skb->nfct);
        skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
        skb->nf_debug = 0;
#endif
#ifdef CONFIG_BRIDGE_NETFILTER
/* We need to free the memory attached to the nf_bridge pointer to avoid a memory leak */
	nf_bridge_put(skb->nf_bridge);
	skb->nf_bridge = NULL;
#endif /* CONFIG_BRIDGE_NETFILTER */
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put_reasm(skb->nfct_reasm);
	skb->nfct_reasm = NULL;
#endif /* CONFIG_NF_CONNTRACK || CONFIG_NF_CONNTRACK_MODULE */
#endif /* CONFIG_NETFILTER */
	skb->sk = NULL;
        skb->dst = NULL;
	skb->pkt_type = PACKET_HOST;    /* Default type */
        skb->ip_summed = 0;
        skb->priority = 0;
	skb->ipvs_property = 0;
	skb->nfctinfo = 0;
	skb->local_df = 0;
#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
#endif /* CONFIG_NET_CLS_ACT */
#endif /* CONFIG_NET_SCHED */

	/* reset the data pointer (skb_reserve is not used for efficiency) */
	skb->data = skb->head + SKB_RESERVED_HEADER_SIZE + HDR_SIZE;
	skb->len = priv->replenish_size;

	/* invalidate the payload
	 * This buffer is now ready to be used for replenish.
	 */
	IX_ACC_DATA_CACHE_INVALIDATE(skb->data, skb->len);

	skQueue[SKB_QINC(skQueueHead)] = skb;
    }
    else
    {
	/* put to pool (slow) */
	dev_kfree_skb_any(skb);
    }
}
#endif /* ifdef CONFIG_IXP400_ETH_SKB_RECYCLE */


/* dev_skb_queue_drain: remove all entries from the skb queue */
static void dev_skb_queue_drain(priv_data_t *priv)
{
    struct sk_buff *skb;
    int key = ixOsalIrqLock();

    /* check for skbuf, then get it and release it */
    while (skQueueHead != skQueueTail)
    {
	skb = dev_skb_dequeue(priv);
	dev_kfree_skb_any(skb);
    }

    ixOsalIrqUnlock(key);
}

/* dev_rx_mb_dequeue: return one mbuf from the rx mbuf queue */
static inline IX_OSAL_MBUF * dev_rx_mb_dequeue(priv_data_t *priv)
{
    IX_OSAL_MBUF *mbuf;
    if (priv->mbRxQueueHead != priv->mbRxQueueTail)
    {
	/* get from queue (fast) */
	mbuf = priv->mbRxQueue[MB_QINC(priv->mbRxQueueTail)];
    }
    else
    {
	/* get from pool (slow) */
	mbuf = IX_OSAL_MBUF_POOL_GET(priv->rx_pool);
    }
    return mbuf;
}

/* dev_rx_mb_enqueue: add one mbuf to the rx mbuf queue */
static inline void dev_rx_mb_enqueue(priv_data_t *priv, IX_OSAL_MBUF *mbuf)
{
    /* check for queue not full */
    if (priv->mbRxQueueHead - priv->mbRxQueueTail < MB_QSIZE)
    {
	/* put to queue (fast) */
	priv->mbRxQueue[MB_QINC(priv->mbRxQueueHead)] = mbuf;
    }
    else
    {
	IX_OSAL_MBUF_POOL_PUT(mbuf);
    }
}

/* dev_rx_mb_queue_drain: remove all mbufs from the rx mbuf queue */
static void dev_rx_mb_queue_drain(priv_data_t *priv)
{
    IX_OSAL_MBUF *mbuf;
    int key = ixOsalIrqLock();

    /* free all queue entries */
    while(priv->mbRxQueueHead != priv->mbRxQueueTail)
    {
	mbuf = dev_rx_mb_dequeue(priv);
	IX_OSAL_MBUF_POOL_PUT(mbuf);
    }

    ixOsalIrqUnlock(key); 
}

/* dev_tx_mb_dequeue: remove one mbuf from the tx mbuf queue */
static inline IX_OSAL_MBUF * dev_tx_mb_dequeue(priv_data_t *priv)
{
    IX_OSAL_MBUF *mbuf;
    if (priv->mbTxQueueHead != priv->mbTxQueueTail)
    {
	/* get from queue (fast) */
	mbuf = priv->mbTxQueue[MB_QINC(priv->mbTxQueueTail)];
    }
    else
    {
	/* get from pool (slow) */
	mbuf = IX_OSAL_MBUF_POOL_GET(priv->tx_pool);
    }
    return mbuf;
}

/* dev_tx_mb_enqueue: add one mbuf to the tx mbuf queue */
static inline void dev_tx_mb_enqueue(priv_data_t *priv, IX_OSAL_MBUF *mbuf)
{
    /* check for queue not full */
    if (priv->mbTxQueueHead - priv->mbTxQueueTail < MB_QSIZE)
    {
	/* put to queue (fast) */
	priv->mbTxQueue[MB_QINC(priv->mbTxQueueHead)] = mbuf;
    }
    else
    {
	IX_OSAL_MBUF_POOL_PUT(mbuf);
    }
}

/* dev_tx_mb_queue_drain: remove all mbufs from the tx mbuf queue */
static void dev_tx_mb_queue_drain(priv_data_t *priv)
{
    IX_OSAL_MBUF *mbuf;
    int key = ixOsalIrqLock();

    /* free all queue entries */
    while(priv->mbTxQueueHead != priv->mbTxQueueTail)
    {
	mbuf = dev_tx_mb_dequeue(priv);
	IX_OSAL_MBUF_POOL_PUT(mbuf);
    }

    ixOsalIrqUnlock(key);
}

/* provides mbuf+skb pair to the NPEs.
 * In the case of an error, free skb and return mbuf to the pool
 */
static void dev_rx_buff_replenish(int port_id, IX_OSAL_MBUF *mbuf)
{
    IX_STATUS status;

    /* send mbuf to the NPEs */
    if ((status = ixEthAccPortRxFreeReplenish(port_id, mbuf)) != IX_SUCCESS)
    {
	replenishErrorCount++;

	P_ERROR("ixEthAccPortRxFreeReplenish failed for port %d, res = %d",
		port_id, status);
	
	/* detach the skb from the mbuf, free it, then free the mbuf */
	dev_kfree_skb_any(mbuf_swap_skb(mbuf, NULL));
	IX_OSAL_MBUF_POOL_PUT(mbuf);
    }
}

/* Allocate an skb for every mbuf in the rx_pool
 * and pass the pair to the NPEs
 */
static int dev_rx_buff_prealloc(priv_data_t *priv)
{
    int res = 0;
    IX_OSAL_MBUF *mbuf;
    struct sk_buff *skb;
    int key;

    while (NULL != (mbuf = IX_OSAL_MBUF_POOL_GET(priv->rx_pool)))
    {
	/* because this function may be called during monitoring steps
	 * interrupt are disabled so it won't conflict with the
	 * QMgr context.
	 */
	key = ixOsalIrqLock();
	/* get a skbuf (alloc from pool if necessary) */
	skb = dev_skb_dequeue(priv);

	if (skb == NULL)
	{
	    /* failed to alloc skb -> return mbuf to the pool, it'll be
	     * picked up later by the monitoring task
	     */
	    IX_OSAL_MBUF_POOL_PUT(mbuf);
	    res = -ENOMEM;
	}
	else
	{
	    /* attach a skb to the mbuf */
	    mbuf_swap_skb(mbuf, skb);
	    dev_rx_buff_replenish(priv->port_id, mbuf);
	}
	ixOsalIrqUnlock(key);

	if (res != 0)
	    return res;
    }

    return 0;
}


/* Replenish if necessary and slowly drain the sw queues
 * to limit the driver holding the memory space permanently.
 * This action should run at a low frequency, e.g during
 * the link maintenance.
 */
static int dev_buff_maintenance(struct net_device *dev)
{
    struct sk_buff *skb;
    int key;
    priv_data_t *priv = netdev_priv(dev);

    dev_rx_buff_prealloc(priv);

    /* Drain slowly the skb queue and free the skbufs to the 
     * system. This way, after a while, skbufs will be 
     * reused by other components, instead of being
     * parmanently held by this driver.
     */
    key = ixOsalIrqLock();

#ifdef CONFIG_IXP400_NAPI
    /* service the tx done queue so as not to hold on to transmits */
    ixEthTxFrameDoneQMCallback(0, 0);
#endif

    /* check for queue not empty, then get the skbuff and release it */
    if (skQueueHead != skQueueTail)
    {
	skb = dev_skb_dequeue(priv);
	dev_kfree_skb_any(skb);
    }
    ixOsalIrqUnlock(key);

    return 0;
}


/* 
 * KERNEL THREADS
 */

/* flush the pending signals for a thread and 
 * check if a thread is killed  (e.g. system shutdown)
 */
static BOOL dev_thread_signal_killed(void)
{
    int killed = FALSE;
    if (signal_pending (current))
    {
 	spin_lock_irq(&current->sighand->siglock);

	if (sigismember(&(current->pending.signal), SIGKILL)
	    || sigismember(&(current->pending.signal), SIGTERM))
	{
	    /* someone kills this thread */
	    killed = TRUE;
	}
	flush_signals(current);

	spin_unlock_irq(&current->sighand->siglock);
    }
    return killed;
}

/* This timer will check the PHY for the link duplex and
 * update the MAC accordingly. It also executes some buffer
 * maintenance to release mbuf in excess or replenish after
 * a severe starvation
 *
 * This function loops and wake up every 3 seconds.
 */
static int dev_media_check_thread (void* arg)
{
    struct net_device *dev = (struct net_device *) arg;
    priv_data_t *priv = netdev_priv(dev);
    int linkUp;
    int speed100;
    int fullDuplex = -1; /* unknown duplex mode */
    int newDuplex;
    int autonegotiate;
    unsigned phyNum = phyAddresses[portIdPhyIndexMap[priv->port_id]];
    u32 res;


    TRACE;

    /* Lock the mutex for this thread.
       This mutex can be used to wait until the thread exits
    */
    down (priv->maintenanceCheckThreadComplete);

//    daemonize("ixp400 MediaCheck"); 
    spin_lock_irq(&current->sighand->siglock);

    sigemptyset(&current->blocked);
   
    recalc_sigpending();
    spin_unlock_irq(&current->sighand->siglock);
    
    snprintf(current->comm, sizeof(current->comm), "ixp400 %s", dev->name);

    TRACE;
    
    while (1)
    {
	/* We may have been woken up by a signal. If so, we need to
	 * flush it out and check for thread termination 
	 */ 
	if (dev_thread_signal_killed())
	{
	    priv->maintenanceCheckStopped = TRUE;
	}

	/* If the interface is down, or the thread is killed,
	 * or gracefully aborted, we need to exit this loop
	 */
	if (priv->maintenanceCheckStopped)
	{
	    break;
	}

	/*
	 * Determine the link status
	 */

	TRACE;

	if (default_phy_cfg[portIdPhyIndexMap[priv->port_id]].linkMonitor)
	{
	    /* lock the MII register access mutex */
	    mutex_lock(miiAccessMutex);

	    res = ixEthMiiLinkStatus(phyNum,
				     &linkUp,
				     &speed100,
				     &newDuplex, 
				     &autonegotiate);
	    /* release the MII register access mutex */
	    mutex_unlock(miiAccessMutex);

	    /* We may have been woken up by a signal. If so, we need to
	     * flush it out and check for thread termination 
	     */ 
	    if (dev_thread_signal_killed())
	    {
		priv->maintenanceCheckStopped = TRUE;
	    }
	    
	    /* If the interface is down, or the thread is killed,
	     * or gracefully aborted, we need to exit this loop
	     */
	    if (priv->maintenanceCheckStopped)
	    {
		break;
	    }
	
	    if (res != IX_SUCCESS)
	    {
		P_WARN("ixEthMiiLinkStatus failed on PHY%d.\n"
		       "\tCan't determine\nthe auto negotiated parameters. "
		       "Using default values.\n",
		       phyNum); 
		/* something is bad, gracefully stops the loop */
		priv->maintenanceCheckStopped = TRUE;
		break;
	    }
	    
	    if (linkUp)
	    {
		if (! netif_carrier_ok(dev))
		{
		    /* inform the kernel of a change in link state */
		    P_ERROR("\nixp400_eth: %s PHY:%d link down->up ",dev->name,phyNum);
		    netif_carrier_on(dev);
		}

		/*
		 * Update the MAC mode to match the PHY mode if 
		 * there is a phy mode change.
		 */
		if (newDuplex != fullDuplex)
		{
		    fullDuplex = newDuplex;
		    if (fullDuplex)
		    {
			ixEthAccPortDuplexModeSet (priv->port_id, IX_ETH_ACC_FULL_DUPLEX);
		    }
		    else
		    {
			ixEthAccPortDuplexModeSet (priv->port_id, IX_ETH_ACC_HALF_DUPLEX);
		    }
		}
	    }
	    else
	    {
		fullDuplex = -1;
		if (netif_carrier_ok(dev))
		{
		    /* inform the kernel of a change in link state */
		    P_ERROR("\nixp400_eth: %s PHY:%d link up->down ",dev->name,phyNum);
		    netif_carrier_off(dev);
		}
	    }
	}
	else
	{
	    /* if no link monitoring is set (because the PHY do not
	     * require (or support) a link-monitoring follow-up, then assume 
	     * the link is up 
	     */
	    if (!netif_carrier_ok(dev))
	    {
		    P_ERROR("\nixp400_eth: %s PHY:%d link not monitored up ",dev->name,phyNum);
		netif_carrier_on(dev);
	    }
	}
    
	TRACE;
    
	/* this is to prevent the rx pool from emptying when
	 * there's not enough memory for a long time
	 * It prevents also from holding the memory for too
	 * long
	 */
	dev_buff_maintenance(dev);

	/* Now sleep for 3 seconds */
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(MEDIA_CHECK_INTERVAL);
    } /* while (1) ... */

    /* free the mutex for this thread. */
    up (priv->maintenanceCheckThreadComplete);

    return 0;
}

/*
 * NPE message polling function
 */
static inline int npemh_poll(void *data)
{

    /* Polling for NPE-A */
#if !defined (CONFIG_IXP400_ETH_NPEB_ONLY) &&\
    !defined (CONFIG_IXP400_ETH_NPEC_ONLY)

#if defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
    if (unlikely(IX_SUCCESS != ixNpeMhMessagesReceive(IX_NPEMH_NPEID_NPEA)))
    {
	P_ERROR("NPE-A npeMh read failure!\n");
    }
#endif /* defined CONFIG_CPU_IXP46X || defined CONFIG_CPU_IXP43X */
#endif

    /* Polling for NPE-B */
#if !defined (CONFIG_CPU_IXP43X)
#if defined (CONFIG_IXP400_ETH_NPEB_ONLY) || defined (CONFIG_IXP400_ETH_ALL)
    if (unlikely(IX_SUCCESS != ixNpeMhMessagesReceive(IX_NPEMH_NPEID_NPEB)))
    {
	P_ERROR("NPE-B npeMh read failure!\n");
    }
#endif
#endif 

    /* Polling for NPE-C */
#if defined (CONFIG_IXP400_ETH_NPEC_ONLY) || defined (CONFIG_IXP400_ETH_ALL)
    if (unlikely(IX_SUCCESS != ixNpeMhMessagesReceive(IX_NPEMH_NPEID_NPEC)))
    {
	P_ERROR("NPE-C npeMh read failure!\n");
    }
#endif

    return 0;
}
	    


/*
 * TIMERS
 *
 * PMU Timer : This timer based on IRQ  will call the qmgr dispatcher 
 * function a few thousand times per second.
 *
 * Maintenance Timer : This timer run the maintanance action every 
 * 60 seconds approximatively.
 *
 */

/* PMU Timer reload : this should be done at each interrupt 
 *
 * Because the timer may overflow exactly between the time we
 * write the counter and the time we clear the overflow bit, all
 * irqs are disabled. Missing the everflow event and the timer 
 * will trigger only after a wrap-around.
*/
static void dev_pmu_timer_restart(void)
{
    unsigned long flags;

    local_irq_save(flags);

     __asm__(" mcr p14,0,%0,c1,c1,0\n"  /* write current counter */
            : : "r" (timer_countup_ticks));

    __asm__(" mrc p14,0,r1,c4,c1,0; "  /* get int enable register */
            " orr r1,r1,#1; "
            " mcr p14,0,r1,c5,c1,0; "  /* clear overflow */
            " mcr p14,0,r1,c4,c1,0\n"  /* enable interrupts */
            : : : "r1");

    local_irq_restore(flags);
}

static irqreturn_t dev_pmu_timer_npemhpoll_os_isr(int irg, void *dev_id)
{
    dev_pmu_timer_restart(); /* set up the timer for the next interrupt */

    npemh_poll(NULL);

    return IRQ_HANDLED;
}

static irqreturn_t dev_pmu_timer_datapathpoll_os_isr(int irg, void *dev_id)
{
    dev_pmu_timer_restart(); /* set up the timer for the next interrupt */

    /* 
     * get the time of this interrupt : all buffers received during this
     * interrupt will be assigned the same time
     */
    do_gettimeofday(&irq_stamp);

    ixEthRxPriorityPoll(0, 128);
    ixEthTxFrameDoneQMCallback(0, 0);

    return IRQ_HANDLED;
}

/* initialize the PMU timer */
static int dev_pmu_timer_init(void)
{
    UINT32 controlRegisterMask =
        BIT(0) | /* enable counters */
        BIT(2);  /* reset clock counter; */

    /* 
    *   Compute the number of xscale cycles needed between each 
    *   PMU IRQ. This is done from the result of an OS calibration loop.
    *
    *   For 533MHz CPU, 533000000 tick/s / 4000 times/sec = 138250
    *   4000 times/sec = 37 mbufs/interrupt at line rate 
    *   The pmu timer is reset to -138250 = 0xfffde3f6, to trigger an IRQ
    *   when this up counter overflows.
    *
    *   The multiplication gives a number of instructions per second.
    *   which is close to the processor frequency, and then close to the
    *   PMU clock rate.
    *
    *      HZ : jiffies/second (global OS constant)
    *      loops/jiffy : global OS value cumputed at boot time
    *      2 is the number of instructions per loop
    *
    */
    UINT32 timer_countdown_ticks = (loops_per_jiffy * HZ * 2) /
        QUEUE_DISPATCH_TIMER_RATE;

    if (npe_error_handler)
    {
    	/*
	 * NPE error handling require slower polling (default 5ms), so we
	 * adjust the ticks count according to our polling frequency relatively
	 * to QUEUE_DISPATCH_TIMER_RATE
	 */
    	timer_countdown_ticks *= QUEUE_DISPATCH_TIMER_TO_INT_CYCLE_CONVERT;
    }

    timer_countup_ticks = -timer_countdown_ticks;

    /* enable the CCNT (clock count) timer from the PMU */
    __asm__(" mcr p14,0,%0,c0,c1,0\n"  /* write control register */
            : : "r" (controlRegisterMask));

    return 0;
}

/* stops the timer when the module terminates 
 *
 * This is protected from re-entrancy while the timeer is being restarted.
*/
static void dev_pmu_timer_disable(void)
{
    unsigned long flags;

    local_irq_save(flags);

    __asm__(" mrc p14,0,r1,c4,c1,0; "  /* get int enable register */
            " and r1,r1,#0x1e; "
            " mcr p14,0,r1,c4,c1,0\n"  /* disable interrupts */
            : : : "r1");
    local_irq_restore(flags);
}

static int dev_pmu_timer_setup(void)
{
    /* poll the datapath from a timer IRQ */
    if (npe_error_handler)
    {
    	TRACE;

    	/*
	 * Setting up NPE error handler NPE message handler polling
	 */
	if (request_irq(IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL,
			dev_pmu_timer_npemhpoll_os_isr,
			IRQF_SHARED,
			"ixp400_eth PMU timer",
			(void *)IRQ_ANY_PARAMETER))
	{
	    P_ERROR("Failed to reassign irq to PMU timer interrupt!\n");
	    return -1;
	}
    }
    else
    {
    	TRACE;

    	/*
	 * Setting up datapath polling
	 */
	if (request_irq(IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL,
			dev_pmu_timer_datapathpoll_os_isr,
			IRQF_SHARED,
			"ixp400_eth PMU timer",
			(void *)IRQ_ANY_PARAMETER))
	{
	    P_ERROR("Failed to reassign irq to PMU timer interrupt!\n");
	    return -1;
	}
    }

    TRACE;

    if (dev_pmu_timer_init())
    {
        P_ERROR("Error initializing IXP400 PMU timer!\n");
        return -1;
    }

    TRACE;

    dev_pmu_timer_restart(); /* set up the timer for the next interrupt */

    return 0;
}

static void dev_pmu_timer_unload(void)
{
        dev_pmu_timer_disable(); /* stop the timer */
        free_irq(IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL,(void *)IRQ_ANY_PARAMETER);
}

/* Internal ISR : run a few thousand times per second and calls 
 * the queue manager dispatcher entry point.
 */
static irqreturn_t dev_qmgr_os_isr( int irg, void *dev_id)
{
#ifdef CONFIG_IXP400_NAPI

    /* Note: there are 2 possible race conditions where the normal
     * EthAcc QMgr receive callback will be invoked by dispatcherFunc()
     * to drain the rx queues rather than polling them via dev_rx_poll:
     * 1. interrupt occurs while dev_rx_poll is being closed and
     *    netif_rx_schedule_prep() fails because the device is no 
     *    longer open.
     * 2. traffic is received after dev_rx_poll enables interrupts,
     *    but before it de-schedules itself.
     */ 
    if(netif_rx_schedule_prep(rx_poll_dev))
    {
        ixEthAccQMgrRxNotificationDisable();
        __netif_rx_schedule(rx_poll_dev);
    }
#endif /* CONFIG_IXP400_NAPI */

    /* get the time of this interrupt : all buffers received during this
     * interrupt will be assigned the same time */
    do_gettimeofday(&irq_stamp);

    /* call the queue manager entry point */
    dispatcherFunc(IX_QMGR_QUELOW_GROUP);

    return IRQ_HANDLED;
}

/* Internal ISR : run a few thousand times per second and calls 
 * the ethernet entry point.
 */
#ifdef CONFIG_IXP400_NAPI
static int dev_rx_poll(struct net_device *netdev, int *budget)
{
    UINT32 entries_to_process = min(*budget, netdev->quota);
    UINT32 entries_processed = 0;
    UINT32 queue_entries = 0;

restart_poll:

    /* get the time of this interrupt : all buffers received during this
     * interrupt will be assigned the same time */
    do_gettimeofday(&irq_stamp);

    entries_processed = ixEthRxPriorityPoll(0, entries_to_process);

    *budget -= entries_processed;
    netdev->quota -= entries_processed;

    /* if not enough entries processed */
    if(entries_processed < entries_to_process)
    {
        ixEthAccQMgrRxNotificationEnable();
        netif_rx_complete(netdev);
        ixEthAccQMgrRxQEntryGet(&queue_entries);
        if(queue_entries > 0 && netif_rx_reschedule(netdev,entries_processed))
        {
            ixEthAccQMgrRxNotificationDisable();
            goto restart_poll;
        }
        return 0;
    }
    return 1;
}
#endif


/* This timer will call ixEthDBDatabaseMaintenance every
 * IX_ETH_DB_MAINTENANCE_TIME jiffies
 */

static void maintenance_timer_task(struct work_struct *data);

/* task spawned by timer interrupt for EthDB maintenance */
static DECLARE_DELAYED_WORK(ethdb_maintenance_work, maintenance_timer_task);	

static void maintenance_timer_set(void)
{
    queue_delayed_work(maintenance_workq, &ethdb_maintenance_work,
	DB_MAINTENANCE_TIME);
}

static void maintenance_timer_clear(void)
{
    cancel_delayed_work(&ethdb_maintenance_work);
    flush_workqueue(maintenance_workq);
}

static void maintenance_timer_task(struct work_struct *data)
{
    down(maintenance_mutex);
    ixEthDBDatabaseMaintenance();
    up(maintenance_mutex);
    maintenance_timer_set();
}

/*
 *  DATAPLANE
 */

/* This callback is called when transmission of the packed is done, and
 * IxEthAcc does not need the buffer anymore. The port is down or
 * a portDisable is running. The action is to free the buffers
 * to the pools.
 */
static void tx_done_disable_cb(UINT32 callbackTag, IX_OSAL_MBUF *mbuf)
{
    struct net_device *dev = (struct net_device *)callbackTag;
    priv_data_t *priv = netdev_priv(dev);

    TRACE;

    priv->stats.tx_packets++; /* total packets transmitted */
    priv->stats.tx_bytes += IX_OSAL_MBUF_MLEN(mbuf); /* total bytes transmitted */

    /* extract skb from the mbuf, free skb and return the mbuf to the pool */
    mbuf_free_skb(mbuf);

    TRACE;
}


/* This callback is called when transmission of the packed is done, and
 * IxEthAcc does not need the buffer anymore. The buffers will be returned to
 * the software queues.
 */
static void tx_done_cb(UINT32 callbackTag, IX_OSAL_MBUF *mbuf)
{
    struct net_device *dev = (struct net_device *)callbackTag;
    priv_data_t *priv = netdev_priv(dev);

    TRACE;
    priv->stats.tx_packets++; /* total packets transmitted */
    priv->stats.tx_bytes += IX_OSAL_MBUF_MLEN(mbuf);/* total bytes transmitted*/
    /* extract skb from the mbuf, free skb */
#ifdef CONFIG_IXP400_ETH_SKB_RECYCLE
    /* recycle skb for later use in rx (fast) */
    dev_skb_enqueue(priv, mbuf_swap_skb(mbuf, NULL));
#else
    /* put to kernel pool (slow) */
    dev_kfree_skb_any(mbuf_swap_skb(mbuf, NULL));
#endif
    /* return the mbuf to the queue */
    dev_tx_mb_enqueue(priv, mbuf);
}

/* This callback is called when transmission of the packed is done, and
 * IxEthAcc does not need the buffer anymore. The buffers will be returned to
 * the software queues.  Also, it checks to see if the netif_queue has been
 * stopped (due to buffer starvation) and it restarts it because it now knows
 * that there is at least 1 mbuf available for Tx.
 */
static void tx_done_queue_stopped_cb(UINT32 callbackTag, IX_OSAL_MBUF *mbuf)
{
    struct net_device *dev = (struct net_device *)callbackTag;
    priv_data_t *priv = netdev_priv(dev);

    TRACE;

    tx_done_cb(callbackTag, mbuf);

    if (netif_queue_stopped(dev))
    {
        ixEthAccPortTxDoneCallbackRegister(priv->port_id, 
                                           tx_done_cb,
                                           (UINT32)dev);
        netif_wake_queue(dev);
    }
}

/* the following function performs the operations normally done
 * in eth_type_trans() (see net/ethernet/eth.c) , and takes care about 
 * the flags returned by the NPE, so a payload lookup is not needed
 * in most of the cases.
 */
static inline void dev_eth_type_trans(unsigned int mflags, 
				      struct sk_buff *skb, 
				      struct net_device *dev)
{
    unsigned header_len = dev->hard_header_len;
    skb_reset_mac_header(skb);
//    skb->mac.raw=skb->data;
    /* skip the mac header : there is no need for length comparison since
     * the skb during a receive is always greater than the header size and 
     * runt frames are not enabled.
     */
    skb->data += header_len;
    skb->len -= header_len;
   
    /* fill the pkt arrival time (set at the irq callback entry) */
    skb->tstamp = timeval_to_ktime(irq_stamp);
//    skb_set_timestamp (skb, &irq_stamp);
 
    /* fill the input device field */
    skb->dev = dev;
    
    /* set the protocol from the bits filled by the NPE */
    if (mflags & IX_ETHACC_NE_IPMASK)
    { 
	/* the type_length field is 0x0800 */
	skb->protocol = htons(ETH_P_IP); 
    } 
    else if (mflags & IX_ETHACC_NE_IPV6MASK)
    {
	/* the type_length field is 0x86DD */
	skb->protocol = htons(ETH_P_IPV6); 
    }
    else
    {
	/* use linux algorithm to find the protocol 
	 * from the type-length field. This costs a
	 * a lookup inside the packet payload. The algorithm
	 * and its constants are taken from the eth_type_trans()
	 * function.
	 */
	struct ethhdr *eth = eth_hdr(skb);
	unsigned short hproto = ntohs(eth->h_proto);
	
	if (hproto >= 1536)
	{
	    skb->protocol = eth->h_proto;
	}
	else
	{
	    unsigned short rawp = *(unsigned short *)skb->data;
	    if (rawp == 0xFFFF)
		skb->protocol = htons(ETH_P_802_3);
	    else
		skb->protocol = htons(ETH_P_802_2);
	}
    }

    /* set the packet type 
     * check mcast and bcast bits as filled by the NPE 
     */
    if (mflags & (IX_ETHACC_NE_MCASTMASK | IX_ETHACC_NE_BCASTMASK))
    {
	if (mflags & IX_ETHACC_NE_BCASTMASK)
	{
	    /* the packet is a broadcast one */
	    skb->pkt_type=PACKET_BROADCAST;
	}
	else
	{
	    /* the packet is a multicast one */
	    skb->pkt_type=PACKET_MULTICAST;
	    ((priv_data_t *)netdev_priv(dev))->stats.multicast++;
	}
    }
    else
    {
	if (dev->flags & IFF_PROMISC)
	{
	    /* check dest mac address only if promiscuous
	     * mode is set This costs
	     * a lookup inside the packet payload.
	     */
	    struct ethhdr *eth = eth_hdr(skb);
	    unsigned char *hdest = eth->h_dest;
	    
	    if (memcmp(hdest, dev->dev_addr, ETH_ALEN)!=0)
	    {
		skb->pkt_type = PACKET_OTHERHOST;
	    }
	}
	else
	{
	    /* promiscuous mode is not set, All packets are filtered
	     * by the NPE and the destination MAC address matches
	     * the driver setup. There is no need for a lookup in the 
	     * payload and skb->pkt_type is already set to PACKET_LOCALHOST;
	     */
	}
    }

    return;
}

/* This callback is called when new packet received from MAC
 * and not ready to be transfered up-stack. (portDisable
 * is in progress or device is down)
 *
 */
static void rx_disable_cb(UINT32 callbackTag, IX_OSAL_MBUF *mbuf, IxEthAccPortId portId)
{
    TRACE;

    /* this is a buffer returned by NPEs during a call to PortDisable: 
     * free the skb & return the mbuf to the pool 
     */
    mbuf_free_skb(mbuf);

    TRACE;
}


/* This callback is called when new packet received from MAC
 * and ready to be transfered up-stack.
 *
 * If this is a valid packet, then new skb is allocated and switched
 * with the one in mbuf, which is pushed upstack.
 *
 */
static void rx_cb(UINT32 callbackTag, IX_OSAL_MBUF *mbuf, IxEthAccPortId portId)
{
    struct net_device *dev;
    priv_data_t *priv;
    struct sk_buff *skb;
    int len;
    unsigned int mcastFlags;
#ifndef CONFIG_IXP400_NAPI
    unsigned int qlevel;
#endif
    u8 *vlanh;
    TRACE;
    dev = (struct net_device *)callbackTag;
    priv = netdev_priv(dev);

#ifndef CONFIG_IXP400_NAPI

    qlevel = __get_cpu_var(softnet_data).input_pkt_queue.qlen;
    
    /* check if the system accepts more traffic and
     * against chained mbufs 
     */
    if ((qlevel < ixpdev_max_backlog)
        && (IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbuf) == NULL))
#else
    /* check against chained mbufs
     */
    if (IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbuf) == NULL)
#endif
    {      
	/* the netif_rx queue is not overloaded */
	TRACE;
  
	len = IX_OSAL_MBUF_MLEN(mbuf);
	mcastFlags = IX_ETHACC_NE_FLAGS(mbuf);
	

	/* allocate new skb and "swap" it with the skb that is tied to the
	 * mbuf. then return the mbuf + new skb to the NPEs.
	 */
	skb = dev_skb_dequeue(priv);
	
	/* extract skb from mbuf and replace it with the new skbuf */
	skb = mbuf_swap_skb(mbuf, skb);

	if (IX_OSAL_MBUF_PRIV(mbuf))
	{
	    TRACE;

	    /* a skb is attached to the mbuf */
	    dev_rx_buff_replenish(priv->port_id, mbuf);
	}
	else
	{
	    /* failed to alloc skb -> return mbuf to the pool, it'll be
	     * picked up later by the monitoring task when skb will
	     * be available again
	     */
	    TRACE;

	    IX_OSAL_MBUF_POOL_PUT(mbuf);
	}

	/* set the length of the received skb from the mbuf length  */
	skb->tail = skb->data + len;
	skb->len = len;
	vlanh=(u8 *)skb->data;
	if (*(vlanh+12)==0x81) 
	    *(vlanh+13)=0;
#ifdef DEBUG_DUMP
	skb_dump("rx", skb);
#endif
	/* Set the skb protocol and set mcast/bcast flags */
	dev_eth_type_trans(mcastFlags, skb, dev);

	/* update the stats */
	priv->stats.rx_packets++; /* total packets received */
	priv->stats.rx_bytes += len; /* total bytes received */
   
	TRACE;
	
	/* push upstack */
#ifdef CONFIG_IXP400_NAPI
        netif_receive_skb(skb);
#else
        netif_rx(skb);
#endif
    }
    else
    {
	/* netif_rx queue threshold reached, stop hammering the system
	 * or we received an unexpected unsupported chained mbuf
	 */
	TRACE;

	/* update the stats */
	priv->stats.rx_dropped++; 

	/* replenish with unchained mbufs */
	do
	{
	    IX_OSAL_MBUF *next = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf);
	    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf) = NULL;
	    IX_OSAL_MBUF_MLEN(mbuf) = priv->replenish_size;
	    dev_rx_buff_replenish(priv->port_id, mbuf);
	    mbuf = next;
	}
	while (mbuf != NULL);
    }
    
    TRACE;
}

/* Set promiscuous/multicast mode for the MAC */
static void ixp400_dev_set_multicast_list(struct net_device *dev)
{
    int res;
    priv_data_t *priv = netdev_priv(dev);
    IxEthAccMacAddr addr1 = {};

/* 4 possible scenarios here
 *
 * scenarios:
 * #1 - promiscuous mode ON
 * #2 - promiscuous mode OFF, accept NO multicast addresses
 * #3 - promiscuous mode OFF, accept ALL multicast addresses
 * #4 - promiscuous mode OFF, accept LIST of multicast addresses 
 */

    TRACE;

    /* 
     * WR19880: We removed IRQ lock from this API in order to allow NPE message
     * send with response API to work. Hence, ixp400_dev_set_multicast_list is
     * now not callable from IRQ!
     *
     * if called from irq handler, lock already acquired
     * if (!in_irq())
     *     spin_lock_irq(&priv->lock);
     */


    /* clear multicast addresses that were set the last time (if exist) */
    ixEthAccPortMulticastAddressLeaveAll (priv->port_id);

    TRACE;

/**** SCENARIO #1 ****/
    /* Set promiscuous mode */
    if (dev->flags & IFF_PROMISC)
    {
	if ((res = ixEthAccPortPromiscuousModeSet(priv->port_id)))
	{
	    P_ERROR("%s: ixEthAccPortPromiscuousModeSet failed on port %d\n",
		    dev->name, priv->port_id);
	}
	else
	{
	    /* avoid redundant messages */
	    if (!(priv->devFlags & IFF_PROMISC))
	    {
		P_VERBOSE("%s: Entering promiscuous mode\n", dev->name);
	    }
	    priv->devFlags = dev->flags;
	}

	goto Exit;
    }

    TRACE;


/**** SCENARIO #2 ****/

    /* Clear promiscuous mode */
    if ((res = ixEthAccPortPromiscuousModeClear(priv->port_id)))
    {
	/* should not get here */
	P_ERROR("%s: ixEthAccPortPromiscuousModeClear failed for port %d\n",
		dev->name, priv->port_id);
    }
    else
    {
	/* avoid redundant messages */
	if (priv->devFlags & IFF_PROMISC)
	{
	    P_VERBOSE("%s: Leaving promiscuous mode\n", dev->name);
	}
	priv->devFlags = dev->flags;
    }

    TRACE;


/**** SCENARIO #3 ****/
    /* If there's more addresses than we can handle, get all multicast
     * packets and sort the out in software
     */
    /* Set multicast mode */
    if ((dev->flags & IFF_ALLMULTI) || 
	(netdev_mc_count(dev) > IX_ETH_ACC_MAX_MULTICAST_ADDRESSES))
    {
	/* ALL MULTICAST addresses will be accepted */
        ixEthAccPortMulticastAddressJoinAll(priv->port_id);

	P_VERBOSE("%s: Accepting ALL multicast packets\n", dev->name);
	goto Exit;
    }

    TRACE;

/**** SCENARIO #4 ****/
    /* Store all of the multicast addresses in the hardware filter */
    if (netdev_mc_count(dev))
    {
	/* now join the current address list */
	/* Get only multicasts from the list */
	struct netdev_hw_addr *ha; 

	/* Rewrite each multicast address */
	netdev_for_each_mc_addr(ha, dev) { 
	    memcpy (&addr1.macAddress[0], &ha->addr[0],
		    IX_IEEE803_MAC_ADDRESS_SIZE);
	    ixEthAccPortMulticastAddressJoin (priv->port_id, &addr1);
	}
    }

Exit:

    TRACE;

    /*
     * WR19880: IRQ is not lock
     *
     * if (!in_irq())
     *     spin_unlock_irq(&priv->lock);
     */
}


/* Enable the MAC port.
 * Called on do_dev_open, dev_tx_timeout and mtu size changes
 */
static int port_enable(struct net_device *dev)
{
    int res;
    IxEthAccMacAddr npeMacAddr;
    priv_data_t *priv = netdev_priv(dev);

    P_DEBUG("port_enable(%s)\n", dev->name);

    /* Set MAC addr in h/w (ethAcc checks for MAC address to be valid) */
    memcpy(&npeMacAddr.macAddress,
	   dev->dev_addr,
	   IX_IEEE803_MAC_ADDRESS_SIZE);
    if ((res = ixEthAccPortUnicastMacAddressSet(priv->port_id, &npeMacAddr)))
    {
        P_VERBOSE("Failed to set MAC address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x for port %d\n",
	       (unsigned)npeMacAddr.macAddress[0],
	       (unsigned)npeMacAddr.macAddress[1],
	       (unsigned)npeMacAddr.macAddress[2],
	       (unsigned)npeMacAddr.macAddress[3],
	       (unsigned)npeMacAddr.macAddress[4],
	       (unsigned)npeMacAddr.macAddress[5],
	       priv->port_id);
	return convert_error_ethAcc(res);	
    }

    /* restart the link-monitoring thread if necessary */
    if (priv->maintenanceCheckStopped)
    {
	/* Starts the driver monitoring thread, if configured */
	priv->maintenanceCheckStopped = FALSE;
	
	priv->maintenanceCheckThreadId = kthread_create(dev_media_check_thread, (void *) dev, "ixp400 MediaCheck");

	if (IS_ERR(priv->maintenanceCheckThreadId))
	{
	    P_ERROR("%s: Failed to start thread for media checks\n", dev->name);
	    priv->maintenanceCheckStopped = TRUE;
	}else
	{
	    wake_up_process(priv->maintenanceCheckThreadId);
	}
    }

    /* force replenish if necessary */
    dev_rx_buff_prealloc(priv);

    /* set the callback supporting the traffic */
    ixEthAccPortTxDoneCallbackRegister(priv->port_id, 
				       tx_done_cb,
				       (UINT32)dev);
    ixEthAccPortRxCallbackRegister(priv->port_id, 
				   rx_cb, 
				   (UINT32)dev);
    

    if ((res = ixEthAccPortEnable(priv->port_id)))
    {
	P_ERROR("%s: ixEthAccPortEnable failed for port %d, res = %d\n",
		dev->name, priv->port_id, res);
	return convert_error_ethAcc(res);
    }

    /* Do not enable aging unless learning is enabled (nothing to age otherwise) */
    if (npe_learning)
    {
        if ((res = ixEthDBPortAgingEnable(priv->port_id)))
        {
            P_ERROR("%s: ixEthDBPortAgingEnable failed for port %d, res = %d\n",
                    dev->name, priv->port_id, res);
            return -1;
        }
    }

    TRACE;

    /* reset the current time for the watchdog timer */
    netif_trans_update(dev);

    netif_start_queue(dev);

    TRACE;

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED
    /* restore the driver's own TX queueing discipline */
    dev->qdisc_sleeping = priv->qdisc;
    dev->qdisc = priv->qdisc;
#endif

    TRACE;

    return 0;
}

/* Disable the MAC port.
 * Called on do_dev_stop and dev_tx_timeout
 */
static void port_disable(struct net_device *dev)
{
    priv_data_t *priv = netdev_priv(dev);
    int res;
    IX_STATUS status;
#ifdef CONFIG_IXP400_NAPI
    unsigned long flags;
#endif

    P_DEBUG("port_disable(%s)\n", dev->name);

    if (!netif_queue_stopped(dev))
    {
	netif_trans_update(dev);
        netif_stop_queue(dev);
    }

    if (priv->maintenanceCheckStopped)
    {
	/* thread is not running */
    }
    else
    {
	/* thread is running */
	priv->maintenanceCheckStopped = TRUE;
	/* Wake up the media-check thread with a signal.
	   It will check the 'running' flag and exit */

	if ((res = kthread_stop(priv->maintenanceCheckThreadId)))
	{
	    P_ERROR("%s: unable to signal thread\n", dev->name);
	}
	else
	{
	    /* wait for the thread to exit. */
	    down (priv->maintenanceCheckThreadComplete);
	    up (priv->maintenanceCheckThreadComplete);
	}
    }

    /* Set callbacks when port is disabled */
    ixEthAccPortTxDoneCallbackRegister(priv->port_id, 
				       tx_done_disable_cb,
				       (UINT32)dev);
    ixEthAccPortRxCallbackRegister(priv->port_id, 
				   rx_disable_cb, 
				   (UINT32)dev);

#ifdef CONFIG_IXP400_NAPI
    /* For NAPI we are using the "nearly full" queue entry condition
     * for the Tx Done Q in order to limit unecessary interrupts, but
     * EthAcc expects the "not empty" condition to be set when flushing
     * buffers during port disable. So we must temporarily set it back 
     */

    /* set queue interrrupt condition for TxDone back to "not empty" */
    ixQMgrNotificationEnable(IX_QMGR_QUEUE_31,IX_QMGR_Q_SOURCE_ID_NOT_E);

    /* disable interrupts briefly while calling the TxDone callback */
    local_irq_save(flags);
    /* now service the queue to satisfy the "empty" condition */
    ixEthTxFrameDoneQMCallback(0, 0);

    local_irq_restore(flags);

#endif /* CONFIG_IXP400_NAPI */

    if ((status = ixEthAccPortDisable(priv->port_id)) != IX_SUCCESS)
    {
	/* should not get here */
	P_ERROR("%s: ixEthAccPortDisable(%d) failed\n",
		dev->name, priv->port_id);
    }

#ifdef CONFIG_IXP400_NAPI
    /* finished with port disable, continue using "nearly full" now */
    ixQMgrNotificationEnable(IX_QMGR_QUEUE_31,IX_QMGR_Q_SOURCE_ID_NF);
#endif

    /* remove all entries from the sw queues */
    dev_skb_queue_drain(priv);
    dev_tx_mb_queue_drain(priv);
    dev_rx_mb_queue_drain(priv);

    TRACE;
}

/* this function is called by the kernel to transmit packet 
 * It is expected to run in the context of the ksoftirq thread.
*/
int p_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int res;
    IX_OSAL_MBUF *mbuf;
    priv_data_t *priv = netdev_priv(dev);

    TRACE;

    /* get mbuf struct from tx software queue */
    mbuf = dev_tx_mb_dequeue(priv);

    if (mbuf == NULL)
    {
	/* No mbuf available, free the skbuf */
	dev_kfree_skb_any(skb);
	priv->stats.tx_dropped++;

        if (!netif_queue_stopped(dev))
        {
            ixEthAccPortTxDoneCallbackRegister(priv->port_id, 
                                               tx_done_queue_stopped_cb,
                                               (UINT32)dev);
	    netif_trans_update(dev);
            netif_stop_queue (dev);
        }
	return 0;
    }

#ifdef DEBUG_DUMP
    skb_dump("tx", skb);
#endif

    mbuf_swap_skb(mbuf, skb); /* this should return NULL, as mbufs in the pool
    			       * have no skb attached
			       */

    /* set ethernet flags to zero */
    IX_ETHACC_NE_FLAGS(mbuf) = 0;

    /* flush the mbuf data from the cache */
    IX_OSAL_CACHE_FLUSH(IX_OSAL_MBUF_MDATA(mbuf), IX_OSAL_MBUF_MLEN(mbuf));

    if ((res = ixEthAccPortTxFrameSubmit(priv->port_id, mbuf,
	IX_ETH_ACC_TX_DEFAULT_PRIORITY)))
    {
#ifdef CONFIG_IXP400_ETH_SKB_RECYCLE
	/* recycle skb for later use in rx (fast) */
	dev_skb_enqueue(priv, mbuf_swap_skb(mbuf, NULL));
#else
	/* put to kernel pool (slow) */
	dev_kfree_skb_any(mbuf_swap_skb(mbuf, NULL)); 
#endif
	/* return the mbuf to the queue */
	dev_tx_mb_enqueue(priv, mbuf);

	priv->stats.tx_dropped++;
	P_ERROR("%s: ixEthAccPortTxFrameSubmit failed for port %d, res = %d\n",
		dev->name, priv->port_id, res);
    }

    TRACE;

    return 0;
}

/* Open the device.
 * Request resources and start interrupts
 */
static int do_dev_open(struct net_device *dev)
{
    int res;

#ifdef CONFIG_IXP400_NAPI
    /* if the current rx_poll_dev isn't running, set it to this one */
    if(!netif_running(rx_poll_dev))
    {
        rx_poll_dev = dev;
    }
#endif

    /* prevent the maintenance task from running while bringing up port */
    down(maintenance_mutex);

    /* bring up the port */
    res = port_enable(dev);

    up(maintenance_mutex);

    if (res == 0)
    {
	try_module_get(THIS_MODULE);
    }

    return res;
}

/* Close the device.
 * Free resources acquired in dev_start
 */
static int do_dev_stop(struct net_device *dev)
{
#ifdef CONFIG_IXP400_NAPI
    int dev_idx = 0;
    struct net_device *tmp_dev;

    TRACE;
    if(dev == rx_poll_dev)
    {
        /* closing the current poll device, change to an open one. 
         * if none are open, do nothing.
         */
        for(dev_idx = 0; dev_idx < dev_max_count; dev_idx++)
        {
	    tmp_dev = dev_get_drvdata(&ixp400_eth_devices[dev_idx].dev);

            if(netif_running(tmp_dev))
            {
                rx_poll_dev = tmp_dev;
                break;
            }
        }
    }
#endif

    TRACE;

    /* prevent the maintenance task from running while bringing up port */
    down(maintenance_mutex);

    /* bring the port down */
    port_disable(dev);

    up(maintenance_mutex);

    module_put(THIS_MODULE);

    return 0;
}

static void
dev_tx_timeout_task(struct work_struct *work)
{
    priv_data_t *priv = container_of(work, priv_data_t, timeout_work);
    struct net_device *dev = priv->ndev;

    P_WARN("%s: Tx Timeout for port %d\n", dev->name, priv->port_id);

    down(maintenance_mutex);
    port_disable(dev);

    /* Note to user: Consider performing other reset operations here
     * (such as PHY reset), if it is known to help the Tx Flow to 
     * become "unstuck". This scenario is application/board-specific.
     * 
     * e.g.
     *
     *  if (netif_carrier_ok(dev))
     *  {
     *	down(miiAccessMutex);
     *	ixEthMiiPhyReset(phyAddresses[priv->port_id]);
     *	up(miiAccessMutex);
     *  }
     */
    
    /* enable traffic again if the port is up */
    if (dev->flags & IFF_UP)
    {
	port_enable(dev);
    }

    up(maintenance_mutex);
}


/* This function is called when kernel thinks that TX is stuck */
static void dev_tx_timeout(struct net_device *dev)
{
    priv_data_t *priv = netdev_priv(dev);

    TRACE;
    queue_work(priv->timeout_workq, &priv->timeout_work);
}

/* update the maximum msdu value for this device */
static void dev_change_msdu(struct net_device *dev, int new_msdu_size)
{
    priv_data_t *priv = netdev_priv(dev);
    unsigned int new_size = new_msdu_size;

    priv->msdu_size = new_size;

    /* ensure buffers are large enough (do not use too small buffers
     * even if it is possible to configure so. 
     */
    if (new_size < IX_ETHNPE_ACC_FRAME_LENGTH_DEFAULT)
    {
	new_size = IX_ETHNPE_ACC_FRAME_LENGTH_DEFAULT;
    }

    /* the NPE needs 64 bytes boundaries : round-up to the next
    * frame boundary. This size is used to invalidate and replenish.
    */
    new_size = IX_ETHNPE_ACC_RXFREE_BUFFER_ROUND_UP(new_size);
    priv->replenish_size = new_size;
    
    /* Xscale MMU needs a cache line boundary : round-up to the next
     * cache line boundary. This will be the size used to allocate
     * skbufs from the kernel.
     */
    new_size = HDR_SIZE + new_size;
    new_size = L1_CACHE_ALIGN(new_size);
    priv->pkt_size = new_size;

    /* Linux stack uses a reserved header.
     * skb contain shared info about fragment lists 
     * this will be the value stored in skb->truesize
     */
    priv->alloc_size = SKB_DATA_ALIGN(SKB_RESERVED_HEADER_SIZE + new_size) + 
    	SKB_RESERVED_TRAILER_SIZE;
}

static int dev_change_mtu(struct net_device *dev, int new_mtu_size)
{
    priv_data_t *priv = netdev_priv(dev);

    /* the msdu size includes the ethernet header plus the 
     * mtu (IP payload), but does not include the FCS which is 
     * stripped out by the access layer.
     */
    unsigned int new_msdu_size = new_mtu_size + dev->hard_header_len + VLAN_HDR;

    if (new_msdu_size > IX_ETHNPE_ACC_FRAME_LENGTH_MAX)
    {
	/* Unsupported msdu size */
	return -EINVAL;
    }

    /* safer to stop maintenance task while bringing port down and up */
    down(maintenance_mutex);

    if (ixEthDBFilteringPortMaximumFrameSizeSet(priv->port_id, 
						new_msdu_size))
    {
	P_ERROR("%s: ixEthDBFilteringPortMaximumFrameSizeSet failed for port %d\n",
		dev->name, priv->port_id);
	up(maintenance_mutex);

	return -1;
    }

    /* update the packet sizes needed */
    dev_change_msdu(dev, new_msdu_size);
    /* update the driver mtu value */
    dev->mtu = new_mtu_size;

    up(maintenance_mutex);

    return 0;
}

static int do_dev_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
    priv_data_t *priv = netdev_priv(dev);
    struct mii_ioctl_data *data = (struct mii_ioctl_data *) & req->ifr_data;
    int phy = phyAddresses[portIdPhyIndexMap[priv->port_id]];
    int res = 0;

    TRACE;

    switch (cmd)
    {
	/*
	 * IOCTL's for mii-tool support
	 */

	/* Get address of MII PHY in use */
	case SIOCGMIIPHY:
	case SIOCDEVPRIVATE:
	    data->phy_id = phy;
	    return 0;

        /* Read MII PHY register */
	case SIOCGMIIREG:		
	case SIOCDEVPRIVATE+1:
	    mutex_lock (miiAccessMutex);     /* lock the MII register access mutex */
	    if ((res = ixEthAccMiiReadRtn (data->phy_id, data->reg_num, &data->val_out)))
	    {
		P_ERROR("Error reading MII reg %d on phy %d\n",
		       data->reg_num, data->phy_id);
		res = -1;
	    }
	    mutex_unlock (miiAccessMutex);	/* release the MII register access mutex */
	    return res;

	/* Write MII PHY register */
	case SIOCSMIIREG:
	case SIOCDEVPRIVATE+2:
	    mutex_lock (miiAccessMutex);     /* lock the MII register access mutex */
	    if ((res = ixEthAccMiiWriteRtn (data->phy_id, data->reg_num, data->val_in)))
	    {
		P_ERROR("Error writing MII reg %d on phy %d\n",
                        data->reg_num, data->phy_id);
		res = -1;
	    }
	    mutex_unlock (miiAccessMutex);	/* release the MII register access mutex */
	    return res;

	/* set the MTU size */
	case SIOCSIFMTU:
	    return dev_change_mtu(dev, req->ifr_mtu);

	default:
	    return -EOPNOTSUPP;
    }

    return -EOPNOTSUPP;
}

static struct net_device_stats *p_dev_get_stats(struct net_device *dev)
{
    int res;
    /* "stats" should be cache-safe.
     * we alligne "stats" and "priv" by 32 bytes, so that the cache
     * operations will not affect "res" and "priv"
     */
    IxEthEthObjStats ethStats __attribute__ ((aligned(32))) = {};
    priv_data_t *priv = netdev_priv(dev);

    TRACE;

    priv = netdev_priv(dev);

    /* Get HW stats and translate to the net_device_stats */
    if (!netif_running(dev))
    {
	TRACE;
	return &priv->stats;
    }

    TRACE;

    IX_OSAL_CACHE_INVALIDATE(&ethStats, sizeof(ethStats));

    if ((res = ixEthAccMibIIStatsGetClear(priv->port_id, &ethStats)))
    {
	P_ERROR("%s: ixEthAccMibIIStatsGet failed for port %d, res = %d\n",
		dev->name, priv->port_id, res);
	return &priv->stats;
    }

    TRACE;

    /* bad packets received */
    priv->stats.rx_errors += 
        ethStats.dot3StatsAlignmentErrors +
        ethStats.dot3StatsFCSErrors +
        ethStats.dot3StatsInternalMacReceiveErrors;

    /* packet transmit problems */
    priv->stats.tx_errors += 
        ethStats.dot3StatsLateCollisions +
        ethStats.dot3StatsExcessiveCollsions +
        ethStats.dot3StatsInternalMacTransmitErrors +
        ethStats.dot3StatsCarrierSenseErrors;

    priv->stats.collisions +=
        ethStats.dot3StatsSingleCollisionFrames +
        ethStats.dot3StatsMultipleCollisionFrames;

    /* recved pkt with crc error */
    priv->stats.rx_crc_errors +=
        ethStats.dot3StatsFCSErrors;

    /* recv'd frame alignment error */
    priv->stats.rx_frame_errors += 
        ethStats.dot3StatsAlignmentErrors;

    /* detailed tx_errors */
    priv->stats.tx_carrier_errors +=
        ethStats.dot3StatsCarrierSenseErrors;

    /* Rx traffic dropped at the NPE level */
    priv->stats.rx_dropped +=
        ethStats.RxOverrunDiscards +
        ethStats.RxLearnedEntryDiscards +
        ethStats.RxLargeFramesDiscards +
        ethStats.RxSTPBlockedDiscards +
        ethStats.RxVLANTypeFilterDiscards +
        ethStats.RxVLANIdFilterDiscards +
        ethStats.RxInvalidSourceDiscards +
        ethStats.RxBlackListDiscards +
        ethStats.RxWhiteListDiscards +
        ethStats.RxUnderflowEntryDiscards;

    /* Tx traffic dropped at the NPE level */
    priv->stats.tx_dropped += 
        ethStats.TxLargeFrameDiscards +
        ethStats.TxVLANIdFilterDiscards;

    return &priv->stats;
}


/* Initialize QMgr and bind it's interrupts */
static int qmgr_init(void)
{
    int res;

    /*
     * Enable live lock and disable polling mode if HSS co-exist is enabled
     */
    if (hss_coexist)
    {
	ixFeatureCtrlSwConfigurationWrite (IX_FEATURECTRL_ORIGB0_DISPATCHER,
	    IX_FEATURE_CTRL_SWCONFIG_DISABLED);

	if (!npe_error_handler)
	    datapath_poll = 0;
    }

    /* Initialise Queue Manager */
    P_VERBOSE("Initialising Queue Manager...\n");
    if ((res = ixQMgrInit()))
    {
	P_ERROR("Error initialising queue manager!\n");
	return -1;
    }

    TRACE;

    /* Get the dispatcher entrypoint */
    ixQMgrDispatcherLoopGet (&dispatcherFunc);

    TRACE;

    if (request_irq(IX_OSAL_IXP400_QM1_IRQ_LVL,
                    dev_qmgr_os_isr,
                    IRQF_SHARED,
                    "ixp400_eth QM1",
                    (void *)IRQ_ANY_PARAMETER))
    {
        P_ERROR("Failed to request_irq to Queue Manager interrupt!\n");
        return -1;
    }

    ixQMgrDispatcherInterruptModeSet(TRUE);

    TRACE;
    return 0;
}

static int ethacc_uninit(void)
{ 
    int res;

    TRACE;

    /* we should uninitialize the components here */
    if ((res=ixEthDBUnload()))
    {
        P_ERROR("ixEthDBUnload Failed!\n");
    }
    TRACE;

    ixEthAccUnload();

    /* best effort, always succeed and return 0 */
    return 0;
}

static int ethacc_init(void)
{
    int res = 0;
    IxEthAccPortId portId;
    int dev_count;
    UINT8 imageId;

    /* start all NPEs before starting the access layer */
    TRACE;
    for (dev_count = 0; 
	 dev_count < dev_max_count;  /* module parameter */
	 dev_count++)
    {
    	u32 npe_active = 0;

	portId = default_portId[dev_count];

        TRACE;
        printk(KERN_EMERG "init port %d, dev %d\n",portId,dev_count);
	if (IX_SUCCESS == ixNpeDlLoadedImageFunctionalityGet(
		default_npeImageId[portId].npeId, &imageId))
	{
	    npe_active = 1;
	}

	/*
	 * Check if the NPE is being initialized with other NPE image. Do not
	 * download NPE image when the NPE containing other image
	 */
	if (0 == npe_active)
	{
	    /* Initialise and Start NPE */
	    if (IX_SUCCESS !=
		ixNpeDlNpeInitAndStart(default_npeImageId[portId].npeImageId))
	    {
		P_ERROR("Error starting NPE for Ethernet port %d!\n", portId);
		return -1;
	    }

	    /* 
	     * We reach here when the NPE image downloaded and the NPE engine
	     * started. So we mark the NPE engine started
	     */
	    npe_active = 1;
	}

	if (npe_error_handler && npe_active)
	{
	    /*
	     * Determine which NPE is to be enabled for parity error detection
	     * based on the NPE image ID that is successfully downloaded
	     */
#if defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
	    switch (default_npeImageId[portId].npeId)
	    {
	    	case IX_NPEDL_NPEID_NPEA:
			parity_npeA_enabled = IX_PARITYENACC_ENABLE;
			break;
	    	case IX_NPEDL_NPEID_NPEB:
			parity_npeB_enabled = IX_PARITYENACC_ENABLE;
			break;
	    	case IX_NPEDL_NPEID_NPEC:
			parity_npeC_enabled = IX_PARITYENACC_ENABLE;
			break;

		default:
			P_ERROR("Unknown NPE image ID\n");
	    }
#endif
	}
	printk(KERN_INFO "Loaded NPE%d from id 0x%08X\n",portId,default_npeImageId[portId].npeImageId);
    }

    /* initialize the Ethernet Access layer */
    TRACE;
    if ((res = ixEthAccInit()))
    {
	P_ERROR("ixEthAccInit failed with res=%d\n", res);
	return convert_error_ethAcc(res);
    }


    TRACE;

    return 0;
}

static int phy_init(void)
{
    int res;
    BOOL physcan[IXP400_ETH_ACC_MII_MAX_ADDR];
    int i, phy_found, num_phys_to_set, dev_count;

    /* initialise the MII register access mutex */
    miiAccessMutex = (struct mutex *) kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (!miiAccessMutex)
	return -ENOMEM;

    mutex_init(miiAccessMutex);

    TRACE;
    /* detect the PHYs (ethMii requires the PHYs to be detected) 
     * and provides a maximum number of PHYs to search for.
     */
    res = ixEthMiiPhyScan(physcan, 
			  sizeof(default_phy_cfg) / sizeof(phy_cfg_t));
    if (res != IX_SUCCESS)
    {
	P_ERROR("PHY scan failed\n");
	return convert_error_ethAcc(res);
    }

    /* Module parameter */
    if (no_phy_scan || machine_is_compex() || machine_is_wg302v1() || machine_is_usr8200())  
    { 
	/* Use hardcoded phy addresses */
	num_phys_to_set = (sizeof(default_phy_cfg) / sizeof(phy_cfg_t));
    }else if (machine_is_mi424wr() )
    {
	num_phys_to_set = 5;    
    }
    
    else
    {
	/* Update the hardcoded values with discovered parameters 
	 *
	 * This set the following mapping
	 *  ixp0 --> first PHY discovered  (lowest address)
	 *  ixp1 --> second PHY discovered (next address)
	 *  .... and so on
	 *
	 * If the Phy address and the wiring on the board do not
	 * match this mapping, then hardcode the values in the
	 * phyAddresses array and use no_phy_scan=1 parameter on 
	 * the command line.
	 */
	for (i=0, phy_found=0; i < IXP400_ETH_ACC_MII_MAX_ADDR; i++)
	{
	    if (physcan[i])
	    {
		P_INFO("Found PHY %d at address %d\n", phy_found, i);
                if (phy_found < dev_max_count)
                {
                    phyAddresses[portIdPhyIndexMap[
		    	default_portId[phy_found]]] = i;
                }
                else
                {
                    phyAddresses[phy_found] = i;
                }
		
		if (++phy_found == IXP400_ETH_ACC_MII_MAX_ADDR)
		    break;
	    }
	}

	num_phys_to_set = phy_found;
    }

    /* Reset and Set each phy properties */
    for (i=0; i < num_phys_to_set; i++)
    {
	P_VERBOSE("Configuring PHY %d\n", i);
	P_VERBOSE("\tSpeed %s\tDuplex %s\tAutonegotiation %s\n",
		  (default_phy_cfg[i].speed100) ? "100" : "10",   
		  (default_phy_cfg[i].duplexFull) ? "FULL" : "HALF",  
		  (default_phy_cfg[i].autoNegEnabled) ? "ON" : "OFF");

	if (phy_reset) /* module parameter */
	{
	    ixEthMiiPhyReset(phyAddresses[i]);
	}

	ixEthMiiPhyConfig(phyAddresses[i],
	    default_phy_cfg[i].speed100,   
	    default_phy_cfg[i].duplexFull,  
	    default_phy_cfg[i].autoNegEnabled);
    }

    /* for each device, display the mapping between the ixp device,
     * the IxEthAcc port, the NPE and the PHY address on MII bus. 
     * Also set the duplex mode of the MAC core depending
     * on the default configuration.
     */
    for (dev_count = 0; 
	 dev_count < dev_max_count  /* module parameter */
	     && dev_count <  num_phys_to_set;
	 dev_count++)
    {
	IxEthAccPortId port_id = default_portId[dev_count];
	char *npe_id = "?";

	if (port_id == IX_ETH_PORT_1) npe_id = "B";
	if (port_id == IX_ETH_PORT_2) npe_id = "C";
	if (port_id == IX_ETH_PORT_3) npe_id = "A";
#if defined(CONFIG_MACH_CAMBRIA)
	P_INFO("%s%d is using NPE%s and the PHY at address %d\n",
	       DEVICE_NAME, port_id-1, npe_id,
	       phyAddresses[portIdPhyIndexMap[port_id]]);
#else
	P_INFO("%s%d is using NPE%s and the PHY at address %d\n",
	       DEVICE_NAME, port_id, npe_id,
	       phyAddresses[portIdPhyIndexMap[port_id]]);
#endif
	/* Set the MAC to the same duplex mode as the phy */
	ixEthAccPortDuplexModeSet(port_id,
            (default_phy_cfg[portIdPhyIndexMap[port_id]].duplexFull) ?
                 IX_ETH_ACC_FULL_DUPLEX : IX_ETH_ACC_HALF_DUPLEX);
    }

    return 0;
}

/* set port MAC addr and update the dev struct if successfull */
int ixp400_dev_set_mac_address(struct net_device *dev, void *addr)
{
    int res;
    IxEthAccMacAddr npeMacAddr;
    struct sockaddr *saddr = (struct sockaddr *)addr;
    priv_data_t *priv = netdev_priv(dev);

    /* Get MAC addr from parameter */
    memcpy(&npeMacAddr.macAddress,
	   &saddr->sa_data[0],
	   IX_IEEE803_MAC_ADDRESS_SIZE);

    /* Set MAC addr in h/w (ethAcc checks for MAC address to be valid) */
    if ((res = ixEthAccPortUnicastMacAddressSet(priv->port_id, &npeMacAddr)))
    {
        P_VERBOSE("Failed to set MAC address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x for port %d\n",
	       (unsigned)npeMacAddr.macAddress[0],
	       (unsigned)npeMacAddr.macAddress[1],
	       (unsigned)npeMacAddr.macAddress[2],
	       (unsigned)npeMacAddr.macAddress[3],
	       (unsigned)npeMacAddr.macAddress[4],
	       (unsigned)npeMacAddr.macAddress[5],
	       priv->port_id);
        return convert_error_ethAcc(res);
    }

    /* update dev struct */
    memcpy(dev->dev_addr, 
	   &saddr->sa_data[0],
	   IX_IEEE803_MAC_ADDRESS_SIZE);

    return 0;
}


/* 
 *  TX QDISC
 */

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED

/* new tx scheduling discipline : the algorithm is based on a 
 * efficient JBI technology : Just Blast It. There is no need for
 * software queueing where the hardware provides this feature
 * and makes the internal transmission non-blocking.
 *
 * because of this reason, there is no need for throttling using
 * netif_stop_queue() and netif_start_queue() (there is no sw queue
 * that linux may restart)
 *
 * This tx queueing scheduling mechanism can be enabled
 * by defining CONFIG_IXP400_ETH_QDISC_ENABLED at compile time
 */
static int dev_qdisc_no_enqueue(struct sk_buff *skb, struct Qdisc * qdisc)
{
        return p_dev_hard_start_xmit(skb, qdisc->dev);     
}

static struct sk_buff *dev_qdisc_no_dequeue(struct Qdisc * qdisc)
{
	return NULL;
}

static struct Qdisc_ops dev_qdisc_ops =
{
	.id		= "ixp400_eth", 
	.priv_size	= 0,
	.enqueue	= dev_qdisc_no_enqueue, 
	.dequeue	= dev_qdisc_no_dequeue,
	.requeue	= dev_qdisc_no_enqueue, 
	.owner		= THIS_MODULE,
};

#endif

static int dev_port_init(IxEthAccPortId portId, UINT32 dev)
{
    int res;

    /* register "safe" callbacks. This ensure that no traffic will be 
     * sent to the stack until the port is brought up (ifconfig up)
     */
    if ((res = ixEthAccPortTxDoneCallbackRegister(portId, 
						  tx_done_disable_cb,
						  dev)))

    {
	TRACE;
	P_ERROR("Failed to register tx done callback register\n");

	return convert_error_ethAcc(res);
    }
    if ((res = ixEthAccPortRxCallbackRegister(portId, 
					      rx_disable_cb, 
					      dev)))
    {
	TRACE;
	P_ERROR("Failed to register tx callback register\n");

	return convert_error_ethAcc(res);
    }

    /* set tx scheduling discipline */
    if ((res = ixEthAccTxSchedulingDisciplineSet(portId,
                                                 FIFO_NO_PRIORITY)))
    {
        TRACE;
        return convert_error_ethAcc(res);
    }
    /* enable tx frame FCS append */
    if ((res = ixEthAccPortTxFrameAppendFCSEnable(portId)))
    {
        TRACE;
        return convert_error_ethAcc(res);
    }
    /* disable rx frame FCS append */
    if ((res = ixEthAccPortRxFrameAppendFCSDisable(portId)))
    {
        TRACE;
        return convert_error_ethAcc(res);
    }

    return 0;
}


#if defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
/**************************************************************
 *      PARITY ERROR DETECTION & NPE SOFT RESET FUNCTIONS     *
 **************************************************************/

/*
 * Parity error callback which will be called upon parity error detected
 */
static void parity_error_cb(void)
{
    IxParityENAccParityErrorContextMessage	error_context;
    UINT32					irqlock;

    irqlock = ixOsalIrqLock();

    /*
     * Initialize the error context structure
     */
    memset (&error_context, 0xFF, 
	    sizeof (IxParityENAccParityErrorContextMessage));

    /*
     * Collect the source of error. Silently ignore if the error context get did
     * not return success. This is because it might due to data abort is not
     * caused by parity error
     */
    if (likely(IX_PARITYENACC_SUCCESS == 
    	ixParityENAccParityErrorContextGet (&error_context)))
    {
    	IxErrHdlAccFuncHandler func = NULL;

	/*
	 * Now find out the source of the error and retrive respective handling
	 * function
	 */
	switch (error_context.pecParitySource)
	{
	    case IX_PARITYENACC_NPE_A_IMEM:
	    case IX_PARITYENACC_NPE_A_DMEM:
	    case IX_PARITYENACC_NPE_A_EXT:
		ixErrHdlAccErrorHandlerGet(IX_ERRHDLACC_NPEA_ERROR, &func);
		break;

	    case IX_PARITYENACC_NPE_B_IMEM:
	    case IX_PARITYENACC_NPE_B_DMEM:
	    case IX_PARITYENACC_NPE_B_EXT:
		ixErrHdlAccErrorHandlerGet(IX_ERRHDLACC_NPEB_ERROR, &func);
		break;

	    case IX_PARITYENACC_NPE_C_IMEM:
	    case IX_PARITYENACC_NPE_C_DMEM:
	    case IX_PARITYENACC_NPE_C_EXT:
		ixErrHdlAccErrorHandlerGet(IX_ERRHDLACC_NPEC_ERROR, &func);
		break;

	    default:
	    	P_ERROR("Unknown error source\n");
	}

	/* 
	 * Call the handling function if the function pointer is not null
	 */
	if (likely(func))
	{
	    (*func)();
	}
    }

    ixOsalIrqUnlock(irqlock);
}

/*
 * NPE recovery done callback. This callback is to notify of any recovery
 * failure.
 */
void parity_npe_recovery_done_cb(IxErrHdlAccErrorEventType event_type)
{
    UINT32 status;

    /* 
     * Requires a 10ms delay or so , This is to allow the other Interrupt error
     * e.g NPE B and NPE C to trigger. This is required if you have NPE A, NPE B
     * and NPE C parity error triggered at the same time. If you don't have
     * to de-schedule code here, the interrupt ISR of NPE B and NPE C will not
     * be allow to execute and the right status will not be read. Why? Because
     * the thread/task in ixErrHdlAcc priority is higher (which where this
     * call-back is called) than the ISR priority level here.
     */
     ixOsalSleep(10);

    /*
     * Collect the status from the handler to detect any possibility failure on
     * recovery
     */
    ixErrHdlAccStatusGet(&status);

    switch (event_type)
    {
    	case IX_ERRHDLACC_NPEA_ERROR:
	    if (unlikely(IX_ERRHDLACC_NPEA_ERROR_MASK_BIT & status))
	    {
		P_ERROR("NPE-A recovery failed\n");
	    }

	    break;

    	case IX_ERRHDLACC_NPEB_ERROR:
	    if (unlikely(IX_ERRHDLACC_NPEB_ERROR_MASK_BIT & status))
	    {
		P_ERROR("NPE-B recovery failed\n");
	    }

	    break;

    	case IX_ERRHDLACC_NPEC_ERROR:
	    if (unlikely(IX_ERRHDLACC_NPEC_ERROR_MASK_BIT & status))
	    {
		P_ERROR("NPE-C recovery failed\n");
	    }

	    break;

	default:
		P_ERROR("Unknown NPE recovery error\n");
    }
}

/*
 *  Parity detection and NPE recovery initialization
 */
static int __init parity_npe_error_handler_init()
{
    /*
     * Parity detection hardware configuration parameter. The configured
     * fields are well explained by the self-explanatory data member name
     */
    IxParityENAccHWParityConfig parity_config =
    {
    	.npeAConfig = 
	    {
	    	.ideEnabled 			= parity_npeA_enabled,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.npeBConfig =
	    {
	    	.ideEnabled			= parity_npeB_enabled,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.npeCConfig =
	    {
	    	.ideEnabled			= parity_npeC_enabled,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.mcuConfig =
	    {
	    	.singlebitDetectEnabled 	= IX_PARITYENACC_DISABLE,
	    	.singlebitCorrectionEnabled 	= IX_PARITYENACC_DISABLE,
	    	.multibitDetectionEnabled 	= IX_PARITYENACC_DISABLE,
	    },

	.swcpEnabled				= IX_PARITYENACC_DISABLE,

	.aqmEnabled				= IX_PARITYENACC_DISABLE,

	.pbcConfig =
	    {
	    	.pbcInitiatorEnabled		= IX_PARITYENACC_DISABLE,
		.pbcTargetEnabled		= IX_PARITYENACC_DISABLE,
	    },

	.ebcConfig =
	    {
	    	.ebcCs0Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs1Enabled 			= IX_PARITYENACC_DISABLE,
	    	.ebcCs2Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs3Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs4Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs5Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs6Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs7Enabled			= IX_PARITYENACC_DISABLE,
		.ebcExtMstEnabled		= IX_PARITYENACC_DISABLE,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },
    };
    IxParityENAccHWParityConfig parity_config_check;
    UINT32 error_handler_config = 0;

    if (npe_error_handler_initialized)
    {
    	return 0;
    }

    /*
     * We must set the initialized flag here so that un-initialization will pick
     * it up and uninitialize regardless of which phase in the initialization is
     * failed
     */
    npe_error_handler_initialized = 1;

    /*
     * Error recovery handler initialization 
     */
    if (IX_SUCCESS != ixErrHdlAccInit())
    {
    	P_ERROR("NPE error recovery initialization failed\n");

	return -EBUSY;
    }

    /* 
     * Error handler recovery done callback registration
     */
    if (IX_SUCCESS != ixErrHdlAccCallbackRegister(parity_npe_recovery_done_cb))
    {
    	P_ERROR("NPE error recovery callback registration failed\n");

	return -EBUSY;
    }

    /*
     * Configuring the events and hardware errors that the error recovery 
     * handler needs to pay attention on
     */
    if (IX_PARITYENACC_ENABLE == parity_npeA_enabled)
    {
    	error_handler_config |= IX_ERRHDLACC_NPEA_ERROR_MASK_BIT;
    }

    if (IX_PARITYENACC_ENABLE == parity_npeB_enabled)
    {
    	error_handler_config |= IX_ERRHDLACC_NPEB_ERROR_MASK_BIT;
    }

    if (IX_PARITYENACC_ENABLE == parity_npeC_enabled)
    {
    	error_handler_config |= IX_ERRHDLACC_NPEC_ERROR_MASK_BIT;
    }

    if (IX_SUCCESS != ixErrHdlAccEnableConfigSet(error_handler_config))
    {
    	P_ERROR("NPE error recovery configuration failed\n");

	return -EBUSY;
    }

    /*
     * Parity access layer initialization
     */
    if (unlikely(IX_PARITYENACC_SUCCESS != ixParityENAccInit()))
    {
	P_ERROR("Parity initialization failed\n");

	return -EBUSY;
    }

    /*
     * Registering parity error handling callback
     */
    if (unlikely(IX_PARITYENACC_SUCCESS != 
    	ixParityENAccCallbackRegister(parity_error_cb)))
    {
	P_ERROR("Parity callback registration failed\n");

	return -EBUSY;
    }

    /*
     * Configure parity error events that we are interested in
     */
    if (unlikely(IX_PARITYENACC_SUCCESS !=
	ixParityENAccParityDetectionConfigure(&parity_config)))
    {
	P_ERROR("Parity detection configuration failed\n");

	return -EBUSY;
    }
   	

    /*
     * Read back the configured value to detect possible inconsistency setup
     * errors
     */
    if (unlikely(IX_PARITYENACC_SUCCESS !=
	ixParityENAccParityDetectionQuery(&parity_config_check)))
    {
	P_ERROR("Parity detection query failed\n");

	return -EBUSY;
    }

    /*
     * Now we compare the value we read with the value we set. Report the error
     * if the data is inconsistent
     */
    if (unlikely(0 != memcmp((void*)&parity_config, (void*)&parity_config_check,
	sizeof(IxParityENAccHWParityConfig))))
    {
	P_ERROR("Parity configuration failed\n");

	return -EINVAL;
    }

    /* Initialization successful */
    P_INFO("NPE Error Handler Initialization Successful\n");

    return 0;
}

/*
 * Parity recovery and NPE error handling un-initialization
 */
static void parity_npe_error_handler_uninit(void)
{
    /*
     * We need to have ugly event disabling parameter here to disable the parity
     * access component due to lack of unload function
     */
    IxParityENAccHWParityConfig parity_config =
    {
    	.npeAConfig = 
	    {
	    	.ideEnabled 			= IX_PARITYENACC_DISABLE,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.npeBConfig =
	    {
	    	.ideEnabled			= IX_PARITYENACC_DISABLE,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.npeCConfig =
	    {
	    	.ideEnabled			= IX_PARITYENACC_DISABLE,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },

	.mcuConfig =
	    {
	    	.singlebitDetectEnabled 	= IX_PARITYENACC_DISABLE,
	    	.singlebitCorrectionEnabled 	= IX_PARITYENACC_DISABLE,
	    	.multibitDetectionEnabled 	= IX_PARITYENACC_DISABLE,
	    },

	.swcpEnabled				= IX_PARITYENACC_DISABLE,

	.aqmEnabled				= IX_PARITYENACC_DISABLE,

	.pbcConfig =
	    {
	    	.pbcInitiatorEnabled		= IX_PARITYENACC_DISABLE,
		.pbcTargetEnabled		= IX_PARITYENACC_DISABLE,
	    },

	.ebcConfig =
	    {
	    	.ebcCs0Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs1Enabled 			= IX_PARITYENACC_DISABLE,
	    	.ebcCs2Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs3Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs4Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs5Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs6Enabled			= IX_PARITYENACC_DISABLE,
	    	.ebcCs7Enabled			= IX_PARITYENACC_DISABLE,
		.ebcExtMstEnabled		= IX_PARITYENACC_DISABLE,
		.parityOddEven			= IX_PARITYENACC_EVEN_PARITY,
	    },
    };
    UINT32 irqlock;

    if (!npe_error_handler_initialized)
    {
	return;
    }

    /*
     * Disable the interrupts before changing the parity configuration
     */
    irqlock = ixOsalIrqLock();
    	
    /*
     * Disable all listening events
     */
    ixParityENAccParityDetectionConfigure(&parity_config);

    /*
     * Unregister the parity callback function by passing in NULL pointer
     */
    ixParityENAccCallbackRegister(NULL);

    /*
     * Unbind the IRQs bound by the parity access layer. This is ugly because it
     * is supposed to be unbind by the access layer that is having better
     * visibility on whether it is used...
     */
    ixOsalIrqUnbind(IX_OSAL_IXP400_NPEA_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_NPEB_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_NPEC_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_PCI_INT_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_SWCP_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_AQM_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_MCU_IRQ_LVL);
    ixOsalIrqUnbind(IX_OSAL_IXP400_EBC_IRQ_LVL);

    /*
     * Parity uninitialization completed. Enable the interrupt here, error
     * handler unload function is sleepable
     */
    ixOsalIrqUnlock(irqlock); 

    /*
     * Finally we unload the error handler
     */
    ixErrHdlAccUnload();

    npe_error_handler_initialized = 0;

    P_INFO("Parity NPE Error Handler Uninitialized\n");
}
#endif


static int get_otp_MACAddress(struct net_device *ndev, priv_data_t *priv)
{
	u32	faddr;
	u32	flashbase;
	u32	low_mac;
	int	i;
	
// We only need 6 bytes but a page is probably the smallest that
// gets mapped.
#define	OTP_IOREMAP_SIZE	PAGE_SIZE

//	flashbase = IXP425_EXP_BUS_CS0_BASE_PHYS;
	flashbase = IXP4XX_EXP_BUS_BASE(0);
	faddr = (u32)ioremap (flashbase, OTP_IOREMAP_SIZE);
	if (!faddr)
	{
		release_mem_region (flashbase, OTP_IOREMAP_SIZE);
		printk (KERN_ERR "%s: unable to ioremap flash\n", __FUNCTION__);
		return -1;
	}
	// Send the query command to the flash.
	writew (0x9090, faddr);
	for (i = 0; i < 6; i++) {
		ndev->dev_addr[i] = readb (faddr + 0x10a + i);
	}
	// Take flash out of query mode
	writew (0xffff, faddr);
	if (faddr) iounmap ((void *)faddr);

	// Check for a valid OTP
	if (ndev->dev_addr[0] != 0xFF)
	{
		// Valid OTP
		// Concatenate the lower 3 bytes of the base MAC address
		// to use to calculate the MAC address for the requested port
		low_mac = ((0x000000FF & ndev->dev_addr[3]) << 16) + ((0x000000FF & ndev->dev_addr[4]) << 8) + ndev->dev_addr[5];
		// There should never be an overflow here.  If there is,
		// we would have to violate the OUI domain.  So, just
		// take what we get (at least keep in the right domain)
		low_mac = low_mac + priv->port_id;
		ndev->dev_addr[3] = (unsigned char) ((low_mac & 0x00FF0000) >> 16);
		ndev->dev_addr[4] = (unsigned char) ((low_mac & 0x0000FF00) >> 8);
		ndev->dev_addr[5] = (unsigned char) (low_mac & 0x000000FF);
	} else {
		// Invalid OTP
		// Do it the old fashioned way
		memcpy(ndev->dev_addr,
		   &default_mac_addr[priv->port_id].macAddress,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
	}
	return 0;
}



#define CLK_LO()    (*((unsigned long *) (IXP4XX_GPIO_GPOUTR))) &=  0x0ffbf
#define CLK_HI()    (*((unsigned long *) (IXP4XX_GPIO_GPOUTR))) |= 0x040
#define CLK_EN()    (*((unsigned long *) (IXP4XX_GPIO_GPOER))) &= 0x0ffbf
#define DATA_LO()   (*((unsigned long *) (IXP4XX_GPIO_GPOUTR))) &= 0x0ff7f
#define DATA_HI()   (*((unsigned long *) (IXP4XX_GPIO_GPOUTR))) |= 0x080
#define DATA_EN()   (*((unsigned long *) (IXP4XX_GPIO_GPOER))) &= 0x0ff7f
#define DATA_DIS()  (*((unsigned long *) (IXP4XX_GPIO_GPOER))) |= 0x80
#define DATA_IN()   ((*((unsigned long *) (IXP4XX_GPIO_GPINR))) & 0x80) >> 7

static int eeprom_start(unsigned char b)
{

  int i;

  CLK_EN();
  udelay(5);
  CLK_HI();
  udelay(5);
  DATA_LO();
  udelay(5);
  CLK_LO();

  for (i = 7; i >= 0; i--) {
    if (b & (1 << i))
      DATA_HI();
    else
      DATA_LO();

    udelay(5);
    CLK_HI();
    udelay(5);
    CLK_LO();
  }

  udelay(5);
  DATA_DIS();
  CLK_HI();
  udelay(5);
  i = DATA_IN();
  CLK_LO();
  udelay(5);
  DATA_EN();

  return i;

}

static void eeprom_stop(void)
{

  udelay(5);
  DATA_LO();
  udelay(5);
  CLK_HI();
  udelay(5);
  DATA_HI();
  udelay(5);
  CLK_LO();
  udelay(5);
  CLK_HI();
  udelay(5);

}

static int eeprom_putb(unsigned char b)
{

  int i;


  for (i = 7; i >= 0; i--) {
    if (b & (1 << i))
      DATA_HI();
    else
      DATA_LO();

    CLK_HI();
    udelay(5);
    CLK_LO();
    udelay(5);
  }

  DATA_DIS();
  CLK_HI();
  udelay(5);
  i = DATA_IN();
  CLK_LO();
  udelay(5);

  DATA_HI();
  DATA_EN();

  return i;

}

static unsigned char eeprom_getb(int more)
{

  int i;
  unsigned char b = 0;

  DATA_DIS();
  udelay(5);

  for (i = 7; i >= 0; i--) {
    b <<= 1;
    if (DATA_IN() == 1)
      b |= 1;

    CLK_HI();
    udelay(5);
    CLK_LO();
    udelay(5);
  }

  DATA_EN();
  if (more)
    DATA_LO();
  else
    DATA_HI();

  udelay(5);
  CLK_HI();
  udelay(5);
  CLK_LO();
  udelay(5);

  return b;

}

static int eeprom_read(int addr, unsigned char *buf, int nbytes)
{

  unsigned char start_byte;
  int i;
  start_byte = 0xA0;

  if (addr & (1 << 8))
    start_byte |= 2;

        eeprom_start(start_byte);

        eeprom_stop();
  eeprom_start(start_byte);

  eeprom_putb(addr & 0xff);
  start_byte |= 1;
  eeprom_start(start_byte);

  for (i = 0; i < (nbytes - 1); i++){
    *buf++ = eeprom_getb(1);
        }

  *buf++ = eeprom_getb(0);
  udelay(5);

  eeprom_stop();

  return nbytes;

}
    
static struct net_device_ops mac_net_ops;

/* Initialize device structs.
 * Resource allocation is deffered until do_dev_open
 */
static int dev_eth_probe(struct device *dev)
{
    priv_data_t *priv = NULL;
    struct net_device *ndev = NULL;
    IxEthAccPortId portId = to_platform_device(dev)->id;
    unsigned char eeprom_mac[6];
    TRACE;

    BUG_ON(!(ndev = alloc_etherdev(sizeof(priv_data_t))));

    //SET_MODULE_OWNER(ndev);
    SET_NETDEV_DEV(ndev, dev);
    dev_set_drvdata(dev, ndev);
    priv = netdev_priv(ndev);

    TRACE;

    /* Initialize the ethAcc port */
    if (ixEthAccPortInit(portId))
    {
    	printk(KERN_ERR "portId %i: No such device\n", portId);
        return -ENODEV;
    }

    if (hss_coexist)
    {
    	IX_STATUS status;

	TRACE;

	/*
	 * Queue policy and priority setup for RX queue
	 */
	status  = ixQMgrCallbackTypeSet(4, IX_QMGR_TYPE_REALTIME_SPORADIC);
	status |= ixQMgrDispatcherPrioritySet (4,IX_QMGR_Q_PRIORITY_1);

	if (IX_ETH_PORT_3 == portId)
	{
	    /*
	     * Queue policy and priority setup for NPE-A TX queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_TX_NPEA_Q, 
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_TX_NPEA_Q,
			    IX_QMGR_Q_PRIORITY_1);

	    /*
	     * Queue policy and priority setup for NPE-A RX free queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_RX_FREE_NPEA_Q,
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_RX_FREE_NPEA_Q,
			    IX_QMGR_Q_PRIORITY_1);
	}

	if (IX_ETH_PORT_1 == portId)
	{
	    /*
	     * Queue policy and priority setup for NPE-B TX queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_TX_NPEB_Q, 
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_TX_NPEB_Q,
			    IX_QMGR_Q_PRIORITY_1);

	    /*
	     * Queue policy and priority setup for NPE-B RX free queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_RX_FREE_NPEB_Q,
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_RX_FREE_NPEB_Q,
			    IX_QMGR_Q_PRIORITY_1);
	}

	if (IX_ETH_PORT_2 == portId)
	{
	    /*
	     * Queue policy and priority setup for NPE-C TX queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_TX_NPEC_Q, 
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_TX_NPEC_Q,
			    IX_QMGR_Q_PRIORITY_1);


	    /*
	     * Queue policy and priority setup for NPE-C RX free queue
	     */
	    status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_RX_FREE_NPEC_Q,
			    IX_QMGR_TYPE_REALTIME_SPORADIC);
	    status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_RX_FREE_NPEC_Q,
			    IX_QMGR_Q_PRIORITY_1);
	}

	/*
	 * Queue policy and priority setup for TxDone queue
	 */
	status |= ixQMgrCallbackTypeSet(IX_ETH_ACC_TX_DONE_Q,
			IX_QMGR_TYPE_REALTIME_SPORADIC);
	status |= ixQMgrDispatcherPrioritySet (IX_ETH_ACC_TX_DONE_Q,
			IX_QMGR_Q_PRIORITY_1);


	if (IX_SUCCESS != status)
	{
	    P_ERROR("Failed to set queue policy for HSS-Ethernet co-exist\n");
	}

	TRACE;
    }
    /* set the private port ID */
    priv->port_id  = portId;

#if defined(CONFIG_MACH_CAMBRIA)
    /* set device name */
    sprintf(ndev->name, DEVICE_NAME"%d", priv->port_id-1);
#else
    sprintf(ndev->name, DEVICE_NAME"%d", priv->port_id);
#endif
    TRACE;

    /* initialize RX pool */
    priv->rx_pool = IX_OSAL_MBUF_POOL_INIT(RX_MBUF_POOL_SIZE, 0,
				           "IXP400 Ethernet driver Rx Pool");
    if(priv->rx_pool == NULL)
    {
	P_ERROR("%s: Buffer RX Pool init failed on port %d\n",
		ndev->name, priv->port_id);
	goto error;
    }

    TRACE;

    /* initialize TX pool */
    priv->tx_pool = IX_OSAL_MBUF_POOL_INIT(TX_MBUF_POOL_SIZE, 0, 
				           "IXP400 Ethernet driver Tx Pool");
    if(priv->tx_pool == NULL)
    {
	P_ERROR("%s: Buffer TX Pool init failed on port %d\n",
		ndev->name, priv->port_id);
	goto error;
    }

     TRACE;

   /* initialise the MII register access mutex */
    priv->maintenanceCheckThreadComplete = (struct semaphore *)
	kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (!priv->maintenanceCheckThreadComplete)
    {
	goto error;
    }
    priv->lock = (spinlock_t)__SPIN_LOCK_UNLOCKED(priv->lock);
    sema_init(priv->maintenanceCheckThreadComplete,1);
    priv->maintenanceCheckStopped = TRUE;

    /* initialize ethernet device (default handlers) */
    ether_setup(ndev);

    TRACE;

     /* fill in dev struct callbacks with customized handlers */

        mac_net_ops.ndo_open      = do_dev_open;
        mac_net_ops.ndo_stop      = do_dev_stop;
        mac_net_ops.ndo_start_xmit= p_dev_hard_start_xmit;
        mac_net_ops.ndo_get_stats = p_dev_get_stats;
        mac_net_ops.ndo_tx_timeout= dev_tx_timeout;
        mac_net_ops.ndo_do_ioctl        =  do_dev_ioctl;
	mac_net_ops.ndo_change_mtu		= dev_change_mtu;
	mac_net_ops.ndo_set_rx_mode = ixp400_dev_set_multicast_list;
	mac_net_ops.ndo_set_mac_address	= ixp400_dev_set_mac_address;
	mac_net_ops.ndo_validate_addr	= eth_validate_addr;
        ndev->netdev_ops = (const struct net_device_ops *)&mac_net_ops;             




	ndev->watchdog_timeo = DEV_WATCHDOG_TIMEO;
	ndev->flags |= IFF_MULTICAST;


#ifdef CONFIG_IXP400_NAPI
    ndev->poll = &dev_rx_poll;
    ndev->weight = IXP400_NAPI_WEIGHT;

    /* initialize the rx_poll_dev device */
    if(NULL == rx_poll_dev)
        rx_poll_dev = ndev;
#endif

    TRACE;
if (machine_is_pronghorn() || machine_is_pronghorn_metro())
    {
    get_otp_MACAddress(ndev, priv);
	P_WARN("try MAC address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x for port %d\n",
	       (unsigned)ndev->dev_addr[0],
	       (unsigned)ndev->dev_addr[1],
	       (unsigned)ndev->dev_addr[2],
	       (unsigned)ndev->dev_addr[3],
	       (unsigned)ndev->dev_addr[4],
	       (unsigned)ndev->dev_addr[5],
	       priv->port_id);
    }else{
//printk(KERN_EMERG "read eeprom mac\n");
/*    eeprom_read((0x100 + (priv->port_id * 6)), eeprom_mac, 6);

    if ( is_valid_ether_addr(eeprom_mac) )
    {
       default_mac_addr[priv->port_id].macAddress[0] = eeprom_mac[0];
       default_mac_addr[priv->port_id].macAddress[1] = eeprom_mac[1];
       default_mac_addr[priv->port_id].macAddress[2] = eeprom_mac[2];
       default_mac_addr[priv->port_id].macAddress[3] = eeprom_mac[3];
       default_mac_addr[priv->port_id].macAddress[4] = eeprom_mac[4];
       default_mac_addr[priv->port_id].macAddress[5] = eeprom_mac[5];
    }*/

    memcpy(ndev->dev_addr, 
	   &default_mac_addr[priv->port_id].macAddress,
	   IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    /* possibly remove this test and the message when a valid MAC address 
     * is not hardcoded in the driver source code. 
     */
    if (is_valid_ether_addr(ndev->dev_addr))
    {
	P_WARN("Use default MAC address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x for port %d\n",
	       (unsigned)ndev->dev_addr[0],
	       (unsigned)ndev->dev_addr[1],
	       (unsigned)ndev->dev_addr[2],
	       (unsigned)ndev->dev_addr[3],
	       (unsigned)ndev->dev_addr[4],
	       (unsigned)ndev->dev_addr[5],
	       priv->port_id);
    }
    
    /* Set/update the internal packet size 
     * This can be overriden later by the command
     *                      ifconfig ixp0 mtu 1504
     */
    TRACE;

    dev_change_msdu(ndev, ndev->mtu + ndev->hard_header_len + VLAN_HDR);

    /* create timeout queue to handle transmission timeout */
    priv->timeout_workq = create_singlethread_workqueue(MODULE_NAME);
    BUG_ON(!priv->timeout_workq);
    priv->ndev=ndev;
    INIT_WORK(&priv->timeout_work, dev_tx_timeout_task);

    /* set the internal maximum queueing capabilities */
    ndev->tx_queue_len = TX_MBUF_POOL_SIZE;

    if (!netif_queue_stopped(ndev))
    {
	TRACE;

	netif_trans_update(ndev);
        netif_stop_queue(ndev);
    }

    TRACE;

    if (register_netdev(ndev))
    	goto error;

    TRACE;

    /* configuring the port  */
    if (dev_port_init(portId, (UINT32)ndev))
    {
    	goto error;
    }

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED
    /* configure and enable a fast TX queuing discipline */
    TRACE;

    priv->qdisc = qdisc_create_dflt(ndev, &dev_qdisc_ops);
    ndev->qdisc_sleeping = priv->qdisc;
    ndev->qdisc = priv->qdisc;
    
    if (!ndev->qdisc_sleeping)
    {
	P_ERROR("%s: qdisc_create_dflt failed on port %d\n",
		ndev->name, priv->port_id);
	goto error;
    }
#endif /* CONFIG_IXP400_ETH_QDISC_ENABLED */

    goto done;

/* Error handling: enter here whenever error detected */
error:
    TRACE;

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED
    if (ndev->qdisc)
    	qdisc_destroy(ndev->qdisc);
#endif

    /* Step 1: Destoying workqueue */
    if (priv && priv->timeout_workq)
    {
    	flush_workqueue(priv->timeout_workq);
	destroy_workqueue(priv->timeout_workq);
	priv->timeout_workq = NULL;
    }

    /* Step 2: detaching ndev from dev */
    dev_set_drvdata(dev, NULL);

    /* Step 3: Unregister and free ndev */
    if (ndev)
    {
    	if (ndev->reg_state == NETREG_REGISTERED)
	    unregister_netdev(ndev);

	free_netdev(ndev);
    }

    TRACE;

    return -ENOMEM;

/* Escape from the routine if no error */
done:
    return 0;
}

static int dev_eth_remove(struct device *dev)
{
    struct net_device *ndev = dev_get_drvdata(dev);
    priv_data_t *priv = netdev_priv(ndev);

    TRACE;

    if (priv != NULL)
    {
	IxEthAccPortId portId = to_platform_device(dev)->id;

	if (IX_SUCCESS != 
	    ixNpeDlNpeStopAndReset(default_npeImageId[portId].npeId))
	{
	    P_NOTICE("Error Halting NPE for Ethernet port %d!\n", portId);
	}

	TRACE;

	if (priv->timeout_workq)
	{
	    flush_workqueue(priv->timeout_workq);
	    destroy_workqueue(priv->timeout_workq);
	    priv->timeout_workq = NULL;
	}

#ifdef CONFIG_IXP400_ETH_QDISC_ENABLED
	if (ndev->qdisc)
	{
	    qdisc_destroy(ndev->qdisc);
	}
#endif
	unregister_netdev(ndev);

	free_netdev(ndev);
	dev_set_drvdata(dev,NULL);
    }

    return 0;
}

static int datapath_poll_activatable_check(void)
{

    TRACE;

    /*
     * If feature control indicates that the livelock dispatcher is
     * used, it is not valid to also use datapath polling
     */
    if ((IX_FEATURE_CTRL_SWCONFIG_DISABLED ==
         ixFeatureCtrlSwConfigurationCheck(IX_FEATURECTRL_ORIGB0_DISPATCHER)) &&
         (datapath_poll == 1))
    {
        datapath_poll = 0;
        printk(KERN_NOTICE "\nInvalid to have datapath_poll=1 when the\n");
        printk(KERN_NOTICE "livelock dispatcher is being used.\n");
        printk(KERN_NOTICE "Datapath polling turned off.\n\n");

	return -1;
    }

    return 0;
}


static int __init ixp400_eth_init(void)
{
    int res, dev_count;


    TRACE;
//compex patch
    if (machine_is_compex())
	{
	phyAddresses[0]=0x10;
	phyAddresses[1]=0x3;
	default_phy_cfg[0].linkMonitor=FALSE;
	default_phy_cfg[1].linkMonitor=TRUE;
	}
    if (machine_is_mi424wr())
	{
	phyAddresses[0]=17;
	phyAddresses[1]=1;
	phyAddresses[2]=2;
	phyAddresses[3]=3;
	phyAddresses[4]=4;
	default_phy_cfg[0].linkMonitor=FALSE;
	default_phy_cfg[1].linkMonitor=TRUE;
	default_phy_cfg[2].linkMonitor=FALSE;
	default_phy_cfg[3].linkMonitor=FALSE;
	default_phy_cfg[4].linkMonitor=FALSE;
	}
    if (machine_is_usr8200())
	{
	phyAddresses[0]=16;
	phyAddresses[1]=9;
	default_phy_cfg[0].linkMonitor=FALSE;
	default_phy_cfg[1].linkMonitor=TRUE;
	}
    if (machine_is_wg302v1())
	{
	phyAddresses[0]=30;
	phyAddresses[1]=1;
	default_phy_cfg[0].linkMonitor=FALSE;
	default_phy_cfg[1].linkMonitor=TRUE;
	}
    P_INFO("Initializing IXP400 NPE Ethernet driver software v. " MOD_VERSION " \n");

    TRACE;

    /* check module parameter range */
    if (dev_max_count == 0 || dev_max_count > IX_ETH_ACC_NUMBER_OF_PORTS)
    {
	P_ERROR("Number of ports supported is dev_max_count <= %d\n", IX_ETH_ACC_NUMBER_OF_PORTS);
	return -1;
    }

    TRACE;

#ifndef DEBUG
    /* check module parameter range */
    if (log_level >= 2)  /* module parameter */
    {
	printk("Warning : log_level == %d and TRACE is disabled\n", log_level);
    }
#endif

    TRACE;

    /* display an approximate CPU clock information */
    P_INFO("CPU clock speed (approx) = %lu MHz\n",
	   loops_per_jiffy * 2 * HZ / 1000000);

    TRACE;

    /*
     * IXP43X and IXP46X supports the feature below:
     * 1. Ethernet on NPE-A
     * 2. Ethernet-HSS coexist capability
     * 3. NPE software error handler
     */
#if 0 //defined (CONFIG_CPU_IXP46X) || defined (CONFIG_CPU_IXP43X)
    {
         UINT32 expbusCtrlReg;

	/* Set the expansion bus fuse register to enable MUX for NPEA MII */
        expbusCtrlReg = ixFeatureCtrlRead ();
        expbusCtrlReg |= ((unsigned long)1<<8);
	ixFeatureCtrlWrite (expbusCtrlReg);

	/*
	 * HSS coexist NPE enabler
	 */
	if (hss_coexist)
	{
	    int i;
	    IxEthAccPortId portId;

	    for (i = 0; i < dev_max_count; i++)
	    {
		portId = default_portId[i];
	   	
		if (IX_NPEDL_NPEID_NPEA == default_npeImageId[portId].npeId)
		{
		    /*
		     * Prepare to download the HSS-Ethernet coexist image into 
		     * NPE-A
		     */
		    default_npeImageId[portId].npeImageId = 
		    	IX_HSS_ETH_NPE_A_IMAGE_ID;
		}
	    }
	}
    }
#else
    {
	if (npe_error_handler)
	{
	    P_ERROR("NPE error handling is not supported on this platform\n");
	    npe_error_handler = 0;
	}

	if (hss_coexist)
	{
	    P_ERROR("HSS-Ethernet co-exist is not supported on this "
		"platform\n");
	    hss_coexist = 0;
	}
    }
#endif

    TRACE;

    /* Enable/disable the EthDB MAC Learning & Filtering feature.
     * This is a half-bridge feature, and should be disabled if this interface 
     * is used on a bridge with other non-NPE ethernet interfaces.
     * This is because the NPE's are not aware of the other interfaces and thus
     * may incorrectly filter (drop) incoming traffic correctly bound for another
     * interface on the bridge.
     */
    if (npe_learning) /* module parameter */
    {
        ixFeatureCtrlSwConfigurationWrite (IX_FEATURECTRL_ETH_LEARNING, TRUE);
    }
    else
    {
        ixFeatureCtrlSwConfigurationWrite (IX_FEATURECTRL_ETH_LEARNING, FALSE);
    }

    TRACE;


    /* Do not initialise core components if no_ixp400_sw_init is set */
    if (no_ixp400_sw_init) /* module parameter */
    {
	P_WARN("no_ixp400_sw_init != 0, no IXP400 SW core component initialisation performed\n");
    }
    else
    {
	/* initialize the required components for this driver */
	if ((res = qmgr_init()))
	    return res;
        TRACE;

	/*
	 * NPE error handling is enabled, initializae the message handler with
	 * polling mode and setup the PMU timer to poll for NPE messages
	 */
	if (npe_error_handler)
	{

	    TRACE;

	    /*
	     * NPE error handling requires datapath poll to poll for NPE
	     * messages
	     */
	    datapath_poll = 1;

	    if (IX_SUCCESS != 
		(res = ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_NO)))
		return -1;

	    /* 
	     * Setting up PMU timer to poll for NPE messages
	     */
	    if ((res = dev_pmu_timer_setup()))
		return res;
	}
	else
	{
	    TRACE;

	    if (IX_SUCCESS != 
	    	(res = ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES)))
		return -1;
	}
    }


    /* Initialise the NPEs and access layer */
    TRACE;

    if ((res = ethacc_init()))
	return res;

    TRACE;

    /* Initialise the PHYs */
    if ((res = phy_init()))
	return res;

    TRACE;

    if ((res = driver_register(&ixp400_eth_driver)))
    	return res;

    TRACE;

    /* Initialise the driver structure */
    for (dev_count = 0; 
	 dev_count < dev_max_count;  /* module parameter */
	 dev_count++)
    {
        if ((res = platform_device_register(&ixp400_eth_devices[dev_count])))
        {
            P_ERROR("failure in registering device %d. res = %d\n", 
                    dev_count, res);
            return res;
        }

        TRACE;
    }

    TRACE;

    /* Create the workqueue for the maintenance task */
    maintenance_workq = create_singlethread_workqueue("ethDB wq");
    BUG_ON(!maintenance_workq);

    TRACE;

#ifdef CONFIG_IXP400_NAPI
    /* Set the queue interrrupt condition for TxDone to "nearly full"
     * This will reduce unecessary interrupts and improve performance
     */
    ixQMgrNotificationEnable(IX_QMGR_QUEUE_31,IX_QMGR_Q_SOURCE_ID_NF);

    /*
     * We disable datapath polling here because this feature is conflicting
     * with NAPI polling implementation.
     *
     * 1. If NAPI is ENABLED, NPE error handler is DISABLED ==>
     *    PMU timer will not be set up
     * 2. If NAPI is ENABLED, NPE error handler is ENABLED ==>
     *	  PMU timer will be setup for NPE message handler messages polling only
     */
    datapath_poll = 0;
#endif

    /*
     * We are about to check and activate the datapath polling, so we verify if
     * datapath poll can be enabled. The checking function will disable
     * datapath_poll if it's not supported.
     */
    datapath_poll_activatable_check();

    TRACE;

    /*
     * Activate datapath polling only if:
     * 1. no_ixp400_sw_init is zero (initializing QMgr and npeMh in this
     *    driver)
     * 2. datapath_poll value is non-zero (we WANT datapath poll and it's SAFE
     *    to use datapath polling
     * 3. npe_error_handler value is zero (we DO NOT want to enable NPE soft
     *    reset feature)
     */
    if ((no_ixp400_sw_init == 0) && (datapath_poll != 0) && 
    	(npe_error_handler == 0))
    {
    	TRACE; 

        /* The QMgr dispatch entry point is called from the 
         * IX_OSAL_IXP400_QM1_IRQ_LVL irq (which will trigger
         * an interrupt for every packet)
         * This function setup the datapath in polling mode
         * for better performances.
         */
        /* remove txdone queue and rx queues from qmgr dispatcher */
	ixQMgrNotificationDisable(IX_QMGR_QUEUE_31);
	ixEthAccQMgrRxNotificationDisable();

	/*
	 * The PMU timer is setting up for txDone queue and rx queue datapath
	 * polling
	 */
        if ((res = dev_pmu_timer_setup()))
        {
            TRACE;
            return res;
        }
    }

    TRACE;

    /*
     * Initialize parity detection and NPE error handler
     */
    if (npe_error_handler)
    {
	if (parity_npe_error_handler_init())
	{
	    P_ERROR("NPE error handler initialization failed\n");
	    parity_npe_error_handler_uninit();
	}
    }

    TRACE;

    /* initialise the DB Maintenance task mutex */
    maintenance_mutex = (struct semaphore *) kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (!maintenance_mutex)
	return -ENOMEM;

    sema_init(maintenance_mutex,1);

    TRACE;

    /* Do not start the EthDB maintenance thread if learning & filtering feature is disabled */
    if (npe_learning) /* module parameter */
    {
        maintenance_timer_set();
    }

    TRACE;
#ifndef CONFIG_IXP400_NAPI
    /* set the softirq rx queue thresholds 
     * (These numbers are based on tuning experiments)
     * maxbacklog =  (ixpdev_max_backlog * 10) / 63;
     */
    if (ixpdev_max_backlog == 0)
    {
	ixpdev_max_backlog = 290; /* system default */
    }
    ixpdev_max_backlog /= BACKLOG_TUNE;

    TRACE;
#endif

    return 0;
}

void __exit ixp400_eth_exit(void)
{
    int dev_count;

    TRACE;

    /* We can only get here when the module use count is 0,
     * so there's no need to stop devices.
     */

    if (no_ixp400_sw_init == 0) /* module parameter */
    {
        if(datapath_poll)
            dev_pmu_timer_unload();

        free_irq(IX_OSAL_IXP400_QM1_IRQ_LVL,(void *)IRQ_ANY_PARAMETER);
    }

    TRACE;

    /* stop the maintenance timer */
    maintenance_timer_clear();

    TRACE;

    /* Wait for maintenance task to complete (if started) */
    if (npe_learning) /* module parameter */
    {
	TRACE;

	down(maintenance_mutex);
	up(maintenance_mutex);
    }

    TRACE;

    /* uninitialize the access layers */
    ethacc_uninit();

    TRACE;

    for (dev_count = 0; 
	 dev_count < dev_max_count;  /* module parameter */
	 dev_count++)
    {
	platform_device_unregister(&ixp400_eth_devices[dev_count]);
    }

    TRACE;

    driver_unregister(&ixp400_eth_driver);

    TRACE;

    if (maintenance_workq)
    {
	destroy_workqueue(maintenance_workq);
	maintenance_workq = NULL;
    }

    TRACE;

    /*
     * Uninitialize parity detection and NPE error handler
     */
    if (npe_error_handler)
    {
	parity_npe_error_handler_uninit();
    }

    if (hss_coexist)
    {
	/*
	 * Disable live lock (default setting)
	 */
	ixFeatureCtrlSwConfigurationWrite (IX_FEATURECTRL_ORIGB0_DISPATCHER,
	    IX_FEATURE_CTRL_SWCONFIG_ENABLED);
    }

    P_VERBOSE("IXP400 NPE Ethernet driver software uninstalled\n");
}

module_init(ixp400_eth_init);
module_exit(ixp400_eth_exit);
