/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/ip.h>
#include <linux/in.h>

#include "mvOs.h"
#include "mvSysHwConfig.h"
#include "mvEth.h"
#include "mvEthPhy.h"
#ifdef INCLUDE_MULTI_QUEUE
#include "mvEthPolicy.h"
#endif


#ifdef CONFIG_QUARTER_DECK
#include "msApiDefs.h"
#include "mv_unimac.h"
#include "qdModule.h"
#include "mv_unm_netconf.h"
#define MIN_ETH_PACKET_LEN 60


#define HEADER_SIZE        2
#define TRAILER_SIZE       4
#define WAN_PORT 1
/***************************************************/
/* Gloabl vars                                     */
/***************************************************/

/* The _binding structs represents per vlan binding 
 * the binding is between the ethernet device (struct net_device)
 * and its fields: HW access, link state, dev state and trailer/header
 */ 
typedef struct _binding {
  struct net_device *dev;       /* the device associated with this binding entry */
  MV_BOOL               boLinkUp;  /* link state */
  MV_BOOL               boOpened;  /* device has allready been opened by the stack (hasn't been closed yet) */
  unsigned char         macAddr[6];
#ifdef HEADERS
  char               header[HEADER_SIZE];
#elif defined (TRAILERS)
  char               trailer[TRAILER_SIZE];
#endif
} BINDING, *PBINDING;


/*
 * The static global array of bindings.
 * each vlan has a binding that represents it
 * the index in the array is the binding for it
 * we are allocating the number of needed bindings in 
 * mv_eth_start
 */ 
static BINDING * mvBindings[MV_UNM_VID_ISOLATED]= {0};

typedef enum {
  HW_UNKNOWN = 0,
  HW_INIT,
  HW_READY,
  HW_ACTIVE
} HW_STATE;

HW_STATE hwState;
int active_ifs;
int global_mtu;
void* global_hal_priv;

extern GT_QD_DEV* qd_dev;



GT_STATUS mvDisassociatePort(int qdPort, int fromVlanId, int newPortsBitMask) {
    return GT_OK;
//  return mv_eth_remove_port_from_vlan(qdPort, fromVlanId, newPortsBitMask);
}
GT_STATUS mvAssociatePort(int qdPort, int toVlanId, int newPortsBitMask, int numOfPorts) {
    return GT_OK;
  //return mv_eth_add_port_to_vlan( qdPort, toVlanId, newPortsBitMask, numOfPorts);
}
int mv_eth_start(void) {
    printk("Error: %s called\n", __FUNCTION__);
    return -1;     
}


static int egiga_qd_load( int port, char *mac_addr, int mtu, int irq );
#else
/*
#define TRAILER_SIZE       0
#define TRAILER_SIZE       0
*/
#endif

/****************************************************** 
 * driver internal definitions --                     *
 ******************************************************/ 

#ifdef INCLUDE_MULTI_QUEUE
#define EGIGA_TXQ_MASK	     (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
#define EGIGA_RXQ_MASK	     (BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2)
#define EGIGA_RXQ_RES_MASK   (BIT18|BIT17|BIT16|BIT15|BIT14|BIT13|BIT12|BIT11)	    
#else
#define EGIGA_TXQ_MASK	     (BIT0)
#define EGIGA_RXQ_MASK       (BIT2)
#define EGIGA_RXQ_RES_MASK   (BIT11)
#endif

/* port interrupt cause reg bits: rx-ready-q0, picer-flag */
#define EGIGA_PICR_MASK      (BIT1|EGIGA_RXQ_MASK|EGIGA_RXQ_RES_MASK)

/* port interrup cause extend reg bits: phy/link-status-change, tx-done-q0 - q7*/
#define EGIGA_PICER_MASK     (BIT20|EGIGA_TXQ_MASK)

/* rx buffer size */ 
#define WRAP          (2 + ETH_HLEN + 4)  /* 2(HW hdr) 14(MAC hdr) 4(CRC) */
#if defined(CONFIG_MV_ETH_HEADER)
 #define RX_BUFFER_SIZE(MTU, PRIV) (MTU + WRAP + PRIV->rx_header_size)
#elif defined(CONFIG_QUARTER_DECK)
 #if defined(TRAILERS)
  #define RX_BUFFER_SIZE(MTU, PRIV) (MTU + WRAP + TRAILER_SIZE)
 #else
  #define RX_BUFFER_SIZE(MTU, PRIV) (MTU + WRAP + HEADER_SIZE)
 #endif
#else
 #define RX_BUFFER_SIZE(MTU, PRIV) (MTU + WRAP)
#endif

int egigaDescRxQ[MV_ETH_RX_Q_NUM] =
{
/*                                      descNum */
/* rxQ = 0 */ EGIGA_NUM_OF_RX_DESCR*2 ,

#ifdef INCLUDE_MULTI_QUEUE
/* rxQ = 1 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 2 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 3 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 4 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 5 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 6 */  EGIGA_NUM_OF_RX_DESCR   ,
/* rxQ = 7 */  EGIGA_NUM_OF_RX_DESCR*2 ,
#endif /* INCLUDE_MULTI_QUEUE */
};

#define EGIGA_Q_DESC(q)  (egigaDescRxQ[q])

int egigaDescTxQ[MV_ETH_TX_Q_NUM] =
{
/* txQ = 0 */  EGIGA_NUM_OF_TX_DESCR*2, 

#ifdef INCLUDE_MULTI_QUEUE
/* txQ = 1 */  EGIGA_NUM_OF_TX_DESCR, 
/* txQ = 2 */  EGIGA_NUM_OF_TX_DESCR, 
/* txQ = 3 */  EGIGA_NUM_OF_TX_DESCR,
/* txQ = 4 */  EGIGA_NUM_OF_TX_DESCR, 
/* txQ = 5 */  EGIGA_NUM_OF_TX_DESCR,
/* txQ = 6 */  EGIGA_NUM_OF_TX_DESCR,
/* txQ = 7 */  EGIGA_NUM_OF_TX_DESCR*2, 
#endif /* INCLUDE_MULTI_QUEUE */
};

/****************************************************** 
 * driver debug control --                            *
 ******************************************************/
/* debug main on/off switch (more in debug control below ) */
#define EGIGA_DEBUG
#undef EGIGA_DEBUG

#define EGIGA_DBG_OFF     0x0000
#define EGIGA_DBG_RX      0x0001
#define EGIGA_DBG_TX      0x0002
#define EGIGA_DBG_RX_FILL 0x0004
#define EGIGA_DBG_TX_DONE 0x0008
#define EGIGA_DBG_LOAD    0x0010
#define EGIGA_DBG_IOCTL   0x0020
#define EGIGA_DBG_INT     0x0040
#define EGIGA_DBG_STATS   0x0080
#define EGIGA_DBG_ALL     0xffff

#ifdef EGIGA_DEBUG
# define EGIGA_DBG(FLG, X) if( (egiga_dbg & (FLG)) == (FLG) ) printk X
#else
# define EGIGA_DBG(FLG, X)
#endif

u32 egiga_dbg = EGIGA_DBG_LOAD | EGIGA_DBG_INT | EGIGA_DBG_TX | EGIGA_DBG_TX_DONE | EGIGA_DBG_RX | EGIGA_DBG_RX_FILL | EGIGA_DBG_STATS;

/****************************************************** 
 * driver statistics control --                       *
 ******************************************************/
/* statistics main on/off switch (more in statistics control below ) */
#ifdef CONFIG_EGIGA_STATIS
#define EGIGA_STATISTICS
#else
#undef EGIGA_STATISTICS
#endif

#define EGIGA_STAT_OFF     0x0000
#define EGIGA_STAT_RX      0x0001
#define EGIGA_STAT_TX      0x0002
#define EGIGA_STAT_RX_FILL 0x0004
#define EGIGA_STAT_TX_DONE 0x0008
#define EGIGA_STAT_LOAD    0x0010
#define EGIGA_STAT_IOCTL   0x0020
#define EGIGA_STAT_INT     0x0040
#define EGIGA_STAT_ALL     0xffff

#ifdef EGIGA_STATISTICS
# define EGIGA_STAT(FLG, CODE) if( (egiga_stat & (FLG)) == (FLG) ) CODE;
#else
# define EGIGA_STAT(FLG, CODE)
#endif

u32 egiga_stat =  EGIGA_DBG_LOAD | EGIGA_DBG_INT | EGIGA_DBG_TX | EGIGA_DBG_TX_DONE | EGIGA_DBG_RX | EGIGA_DBG_RX_FILL | EGIGA_DBG_STATS;

extern u32 overEthAddr;

/****************************************************** 
 * device private information --                      *
 ******************************************************/
typedef struct _egiga_statistics
{
    /* interrupt stats */
    u32 int_total, int_rx_events, int_tx_done_events;
    u32 int_phy_events, int_none_events;

    /* rx stats */
    u32 rx_poll_events, rx_poll_hal_ok[MV_ETH_RX_Q_NUM], rx_poll_hal_no_resource[MV_ETH_RX_Q_NUM];
    u32 rx_poll_hal_no_more[MV_ETH_RX_Q_NUM], rx_poll_hal_error[MV_ETH_RX_Q_NUM], rx_poll_hal_invalid_skb[MV_ETH_RX_Q_NUM];
    u32 rx_poll_hal_bad_stat[MV_ETH_RX_Q_NUM], rx_poll_netif_drop[MV_ETH_RX_Q_NUM], rx_poll_netif_complete;

    /* rx-fill stats */
    u32 rx_fill_events[MV_ETH_RX_Q_NUM], rx_fill_alloc_skb_fail[MV_ETH_RX_Q_NUM], rx_fill_hal_ok[MV_ETH_RX_Q_NUM];
    u32 rx_fill_hal_full[MV_ETH_RX_Q_NUM], rx_fill_hal_error[MV_ETH_RX_Q_NUM], rx_fill_timeout_events;

    /* tx stats */
    u32 tx_events, tx_hal_ok[MV_ETH_TX_Q_NUM], tx_hal_no_resource[MV_ETH_TX_Q_NUM], tx_hal_error[MV_ETH_TX_Q_NUM];
    u32 tx_hal_unrecognize[MV_ETH_TX_Q_NUM], tx_netif_stop[MV_ETH_TX_Q_NUM], tx_timeout;

    /* tx-done stats */
    u32 tx_done_events, tx_done_hal_invalid_skb[MV_ETH_TX_Q_NUM], tx_done_hal_bad_stat[MV_ETH_TX_Q_NUM];
    u32 tx_done_hal_still_tx[MV_ETH_TX_Q_NUM], tx_done_hal_ok[MV_ETH_TX_Q_NUM], tx_done_hal_no_more[MV_ETH_TX_Q_NUM];
    u32 tx_done_hal_unrecognize[MV_ETH_TX_Q_NUM], tx_done_max[MV_ETH_TX_Q_NUM], tx_done_min[MV_ETH_TX_Q_NUM], tx_done_netif_wake[MV_ETH_TX_Q_NUM];

} egiga_statistics;

typedef struct _egiga_priv
{
    int port;
    int vid;       /* the VLAN ID (VID) */
    void* hal_priv;
    void* pRxPolicyHndl;
    void* pTxPolicyHndl;
    u32 rxq_count[MV_ETH_RX_Q_NUM];
    u32 txq_count[MV_ETH_TX_Q_NUM];
    spinlock_t lock;
    struct net_device_stats stats;
    MV_BUF_INFO tx_buf_info_arr[MAX_SKB_FRAGS+3];
    MV_PKT_INFO tx_pkt_info;
#ifdef EGIGA_STATISTICS
    egiga_statistics egiga_stat;
#endif
    struct timer_list rx_fill_timer;
    unsigned rx_fill_flag;
    u32 rx_coal;
    u32 tx_coal;
    u32 rxcause;
    u32 txcause;
    u32 txmask;
    u32 rxmask;
    u32 ex_intr_mask;
#ifdef CONFIG_MV_ETH_HEADER
    int rx_header_size;
#endif
#if defined (CONFIG_QUARTER_DECK)
    /* in this case we (and not HW) must take care of padding */
    char zero_pad[MIN_ETH_PACKET_LEN];
#endif

} egiga_priv; 



/****************************************************** 
 * functions prototype --                             *
 ******************************************************/
static int __init egiga_init_module( void );
static void __init egiga_exit_module( void );
module_init( egiga_init_module );
module_exit( egiga_exit_module);
static int egiga_load( int port, char *name, char *mac_addr, int mtu, int irq );
static int egiga_unload( int port, char *name );
static int egiga_start( struct net_device *dev );
static int egiga_start_internals( struct net_device *dev );
static int egiga_stop( struct net_device *dev );
static int egiga_close( struct net_device *dev );
static int egiga_stop_internals( struct net_device *dev );
static int egiga_down_internals( struct net_device *dev );
static int egiga_tx( struct sk_buff *skb, struct net_device *dev );
static u32 egiga_tx_done( struct net_device *dev );
static void egiga_tx_timeout( struct net_device *dev );
static int  egiga_rx( struct net_device *dev,unsigned int work_to_do );

static u32 egiga_rx_fill( struct net_device *dev, unsigned int queue, int count );
static void egiga_rx_fill_on_timeout( unsigned long data );

static int egiga_poll( struct net_device *dev, int *budget );

static irqreturn_t egiga_interrupt_handler( int rq , void *dev_id , struct pt_regs *regs );
static struct net_device_stats* egiga_get_stats( struct net_device *dev );
static void egiga_set_multicast_list(struct net_device *dev);
static int egiga_set_mac_addr( struct net_device *dev, void *addr );
static int egiga_change_mtu_internals( struct net_device *dev, int mtu );
int egiga_change_rx_header_internals( int port, int header_len, int rx_header_enable );
static int egiga_change_mtu( struct net_device *dev, int mtu );
static void egiga_print_phy_status( struct net_device *dev );
static void egiga_convert_str_to_mac( char *source , char *dest );
static unsigned int egiga_str_to_hex( char ch );
void print_egiga_stat( unsigned int port );
static int restart_autoneg( int port );
#if defined(CONFIG_MV_ETH_HEADER) || defined(EGIGA_STATISTICS)
static struct net_device* get_net_device_by_port_num(unsigned int port);
#endif

/*********************************************************** 
 * egiga_init_module --                                    *
 *   main driver initialization. loading the interfaces.   *
 ***********************************************************/
static int __init egiga_init_module( void ) 
{
    u32 i, err;
    printk( "Marvell Gigabit Ethernet Driver 'egiga':\n" );

    printk( "  o %s\n", ETH_DESCR_CONFIG_STR );

#if defined(ETH_DESCR_IN_SRAM)
    printk( "  o %s\n", INTEG_SRAM_CONFIG_STR );
#endif

    printk( "  o %s\n", ETH_SDRAM_CONFIG_STR );

#if defined(TX_CSUM_OFFLOAD) && defined(RX_CSUM_OFFLOAD)
    printk( "  o Checksum offload enabled\n");
#else
#if defined(RX_CSUM_OFFLOAD)
    printk( "  o Receive checksum offload enabled\n");
#endif
#if defined(TX_CSUM_OFFLOAD)
    printk( "  o Transmit checksum offload enabled\n");
#endif
#endif

#ifdef INCLUDE_MULTI_QUEUE
    printk( "  o Multi Queue enabled\n");
#ifdef CONFIG_MV_ETH_HEADER
    printk( "  o Marvell Header supported\n");
#endif
#endif

#ifdef ETH_HALFDUPLEX_ERRATA
    if(MV64465_DEV_ID != mvCtrlModelGet())
    	printk( "  o Half-duplex workaround enabled\n");
#endif

#ifdef EGIGA_STATISTICS
    printk( "  o Driver statistics enabled\n");
#endif

#ifdef EGIGA_DEBUG
    printk( "  o Driver debug messages enabled\n");
#endif

#ifdef CONFIG_EGIGA_PROC
    printk("  o Marvell ethtool proc enabled\n");
#endif

    printk( "  o Loading network interface " );

    /* init G-Unit */
    mvEthInit();

#ifdef CONFIG_QUARTER_DECK
    printk("\n");
#if defined (HEADERS)
    printk("  o Using switch header mode\n");
#elif defined (TRAILERS)
    printk("  o Using switch trailer mode\n");
#else
#error "HEADERS/TRAILERS must be defined"
    printk("  o Error. Switch header/trailer mode is missing\n");
    return -1;
#endif

    qdEntryPoint();
    if (egiga_qd_load(0, CONFIG_ETH_0_MACADDR, CONFIG_ETH_0_MTU, ETH_PORT0_IRQ_NUM )) 
    {
         printk( KERN_ERR "Error loading ethernet port with quarterdeck\n");
    }
    return 0;
#endif

    /* load interfaces */
    for( i=0; i<mvCtrlEthMaxPortGet(); i++ ) {
        err = 0;
        switch(i) {
            case 0:
                printk( "'egiga0' " );
                if( egiga_load( 0, "egiga0", CONFIG_ETH_0_MACADDR, CONFIG_ETH_0_MTU, ETH_PORT0_IRQ_NUM ) )
                    err = 1;
                break;
            case 1:
                printk( "'egiga1' " );
                if( egiga_load( 1, "egiga1", CONFIG_ETH_1_MACADDR, CONFIG_ETH_1_MTU, ETH_PORT1_IRQ_NUM ) )
                    err = 1;
                break;
            case 2:
                printk( "'egiga2' " );
                if( egiga_load( 2, "egiga2", CONFIG_ETH_2_MACADDR, CONFIG_ETH_2_MTU, ETH_PORT2_IRQ_NUM ) )
                    err = 1;
                break;
            default:
                err = 1;
                break;  
        }
        if( err ) printk( KERN_ERR "Error loading ethernet port %d\n", i );
    }
    printk( "\n" );

    return 0;
}



/*********************************************************** 
 * egiga_exit_module --                                    *
 *   main driver termination. unloading the interfaces.    *
 ***********************************************************/
static void __init egiga_exit_module(void) 
{
    u32 i, err;

    for( i=0; i<mvCtrlEthMaxPortGet(); i++ ) {
        err = 0;
        switch(i) {
            case 0:
                if( egiga_unload( 0, "egiga0" ) )
                    err = 1;
                break;
            case 1:
                if( egiga_unload( 1, "egiga1" ) )
                    err = 1;
                break;
            case 2:
                if( egiga_unload( 2, "egiga2" ) )
                    err = 1;
                break;
            default:
                err = 1;
                break; 
        }
        if( err ) printk( KERN_ERR "Error unloading ethernet port %d\n", i);
    }
}
#ifdef CONFIG_QUARTER_DECK

static int egiga_qd_load( int port, char *mac_addr, int mtu, int irq ) 
{
    int iNumOfVlans,  vid;
    BINDING * pBinding;
    int status = GT_OK;
    int cards = 0;
    struct net_device *dev = NULL;
    egiga_priv *priv = NULL;
    MV_ETH_PORT_INIT hal_init_struct;
    int ret = 0;
#ifdef INCLUDE_MULTI_QUEUE
    MV_ETH_TX_POLICY_ENTRY  egigaTxDefPolicy =
    	{
      	 NULL,                   /* pHeader */
      	 0,                      /* headerSize */
      	 EGIGA_DEF_TXQ    /* txQ */
    	};
#endif
    hwState = HW_UNKNOWN;
    active_ifs = 0; 
    memset(mvBindings, 0, sizeof(mvBindings));

    if (qdModuleStart() != 0) {
#ifdef ETH_DBG_ERROR
        printk("Error in QD init\n");
#endif
        return -ENODEV;
    }

    if ((status = mvUnmInitialize()) != GT_OK) {  
#ifdef ETH_DBG_ERROR
        printk("Error - NetConfig is invalid, can't start network device\n");
#endif
        return -ENODEV;
  }

#ifdef ETH_DBG_INFO
    printk("egiga_qd_load: QD is initialized\n");
#endif
  
  iNumOfVlans = mvUnmGetNumOfVlans();
#ifdef ETH_DBG_INFO
  printk ("egiga_qd_loadmv_eth init: iNumOfVlans = %d\n", iNumOfVlans);
  mv_nc_printConf();
#endif

  /* When the first interface (represented by network device) is
     started, it creates and registers all interfaces.  Note that
     VID 0 is reserved */
    for (vid = 1; vid <= iNumOfVlans; vid++) {
        unsigned int portsBitMask = mvUnmGetPortMaskOfVid(vid);

        if( mvUnmCreateVlan(vid, portsBitMask) != GT_OK) {
            panic("egiga_qd_load: cannot create VID %d with bitmask %x\n",
	          vid, portsBitMask);
                ret = -1;    
            goto error;
        }

        printk("load virtual interface vid = %d\n", vid);
        /* allocate and clean data structures */
        pBinding = (BINDING*)kmalloc(sizeof(BINDING), GFP_KERNEL);
        dev = alloc_etherdev(sizeof(egiga_priv));

        if (!dev || !pBinding) {
            panic("mv_eth_module_init : kmalloc failed\n");
            ret = -ENOMEM;
            goto error;         
        }
    
        priv = (egiga_priv *)dev->priv;
        if( !priv ) { 
            ret = -ENOMEM;
	    goto error;
        }

        memset( priv , 0, sizeof(egiga_priv) );
        memset(pBinding, 0, sizeof(BINDING));

        /* initialize structs */
        mvBindings[vid] = pBinding;
        pBinding->dev = dev;   
   
#if 1
        /* init device mac addr */
        {
            if(!overEthAddr)
                mvEthMacAddrGet(port, dev->dev_addr);
            else
                egiga_convert_str_to_mac( mac_addr, dev->dev_addr );

            dev->dev_addr[5] += (vid - WAN_PORT);
            memcpy(pBinding->macAddr, dev->dev_addr, 6);
        }
    
#endif
          /* get the interface names from netconf */    
        {
            int size;
            char *namebuf;

            mv_nc_GetVIDName(vid, &size, &namebuf);
            
             if( size > IFNAMSIZ ) { /* defined in netdevice.h */
                printk( KERN_ERR "interface name must be less than %d chars\n", IFNAMSIZ );
                ret = -1;
                goto error;
            }
    
            memcpy(dev->name, namebuf, size + 1);
            dev->name[size] = '\0'; /* just in case... */
            printk(" register if with name %s \n", dev->name);
        }
            dev->base_addr = 0;
        dev->irq = irq;
        dev->open = egiga_start;
        dev->stop = egiga_close;    
        dev->hard_start_xmit = egiga_tx;
        dev->tx_timeout = egiga_tx_timeout;
        dev->watchdog_timeo = 5*HZ;
        dev->tx_queue_len = egigaDescTxQ[EGIGA_DEF_TXQ];
        dev->poll = &egiga_rx_poll;
        dev->weight = 64;
        dev->get_stats = egiga_get_stats;
        dev->set_mac_address = egiga_set_mac_addr;
        dev->change_mtu = &egiga_change_mtu;
        dev->set_multicast_list = egiga_set_multicast_list; 

#ifdef TX_CSUM_OFFLOAD
        dev->features = NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
    
        /* init egiga_priv */   
        priv->vid = vid;
        priv->port = port;
        spin_lock_init( &priv->lock );
        memset( &priv->rx_fill_timer, 0, sizeof(struct timer_list) );
        priv->rx_fill_timer.function = egiga_rx_fill_on_timeout;
        priv->rx_fill_timer.data = (unsigned long)dev;
        priv->rx_fill_flag = 0;

        if ( hwState == HW_UNKNOWN)
        {  
            printk("Init the hal\n"); 
            /* init the hal */  
            memcpy(hal_init_struct.macAddr, dev->dev_addr, MV_MAC_ADDR_SIZE);
            hal_init_struct.maxRxPktSize = RX_BUFFER_SIZE( mtu, priv); /*why dev->mtu?*/
            hal_init_struct.rxDefQ = EGIGA_DEF_RXQ; 
            memcpy(hal_init_struct.rxDescrNum,  egigaDescRxQ, sizeof(egigaDescRxQ));
            memcpy(hal_init_struct.txDescrNum,  egigaDescTxQ, sizeof(egigaDescTxQ));
#ifdef INCLUDE_MULTI_QUEUE
dffgvwfvsfd
#endif
            /* create internal port control structure and descriptor rings.               */
            /* open address decode windows, disable rx and tx operations. mask interrupts */
            priv->hal_priv = mvEthPortInit( port, &hal_init_struct );
            
            if( !priv->hal_priv ) {
                printk( KERN_ERR "%s: load failed\n", dev->name);
                kfree( priv );
    	        kfree( dev );
                return -ENODEV;
            }
            /* add the mac address of the virtual interface to the unicast filtering table*/
            mvEthMacAddrSet(priv->hal_priv, dev->dev_addr, EGIGA_DEF_RXQ);

            egiga_change_mtu_internals(dev, mtu);
            global_mtu = dev->mtu;
            global_hal_priv = priv->hal_priv;
            hwState = HW_INIT;
        
        }
        else
        {
            dev->mtu = global_mtu;
            priv->hal_priv = global_hal_priv;   
        }
        
        if (register_netdev(mvBindings[vid]->dev) == 0) {
          cards++;  
            printk(" if %s registered\n", dev->name);
        }
    }  

#if 0
  /* CleanUP - deregister net devices */
  if (cards < iNumOfVlans) {
    panic("mv_eth_module_init : bad status\n");
  }
#endif

#ifdef ETH_DBG_TRACE
  printk("mv_eth module inited with %d devices\n", cards);
#endif
  return (cards > 0)? 0: -ENODEV;

error:
    if( priv )
        kfree( dev->priv );

    if( dev )
        kfree( dev );

    return ret;
}
#endif

/*********************************************************** 
 * egiga_load --                                           *
 *   load a network interface instance into linux core.    *
 *   initialize sw structures e.g. private, rings, etc.    *
 ***********************************************************/
static int egiga_load( int port, char *name, char *mac_addr, int mtu, int irq ) 
{
    struct net_device *dev = NULL;
    egiga_priv *priv = NULL;
    MV_ETH_PORT_INIT hal_init_struct;
    int ret = 0;
#ifdef INCLUDE_MULTI_QUEUE
    MV_ETH_TX_POLICY_ENTRY  egigaTxDefPolicy =
    	{
      	 NULL,                   /* pHeader */
      	 0,                      /* headerSize */
      	 EGIGA_DEF_TXQ    /* txQ */
    	};
#endif

    if( strlen(name) > IFNAMSIZ ) { /* defined in netdevice.h */
        printk( KERN_ERR "%s must be less than %d chars\n", name, IFNAMSIZ );
	ret = -1;
	goto error;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
    dev = alloc_etherdev(sizeof(egiga_priv));
#else
    dev = init_etherdev( dev, sizeof(egiga_priv) );
#endif

    if( !dev ) {
        ret = -ENOMEM;
	goto error;
    }

    priv = (egiga_priv *)dev->priv;
    if( !priv ) { 
        ret = -ENOMEM;
	goto error;
    }

    memset( priv , 0, sizeof(egiga_priv) );

    /* init device mac addr */
    if(!overEthAddr)
	mvEthMacAddrGet(port, dev->dev_addr);
    else
    	egiga_convert_str_to_mac( mac_addr, dev->dev_addr );

    /* init device methods */
    strcpy( dev->name, name );
    dev->base_addr = 0;
    dev->irq = irq;
    dev->open = egiga_start;
    dev->stop = egiga_close;
    dev->hard_start_xmit = egiga_tx;
    dev->tx_timeout = egiga_tx_timeout;
    dev->watchdog_timeo = 5*HZ;
    dev->tx_queue_len = egigaDescTxQ[EGIGA_DEF_TXQ];
    dev->poll = &egiga_poll;
    dev->weight = 64;
    dev->get_stats = egiga_get_stats;
    dev->set_mac_address = egiga_set_mac_addr;
    dev->change_mtu = &egiga_change_mtu;
    dev->set_multicast_list = egiga_set_multicast_list;

#ifdef TX_CSUM_OFFLOAD
    dev->features = NETIF_F_SG | NETIF_F_IP_CSUM;
#endif

    /* init egiga_priv */
    priv->port = port;
    spin_lock_init( &priv->lock );
    memset( &priv->rx_fill_timer, 0, sizeof(struct timer_list) );
    priv->rx_fill_timer.function = egiga_rx_fill_on_timeout;
    priv->rx_fill_timer.data = (unsigned long)dev;
    priv->rx_fill_flag = 0;

    /* init the hal */
    memcpy(hal_init_struct.macAddr, dev->dev_addr, MV_MAC_ADDR_SIZE);
    hal_init_struct.maxRxPktSize = RX_BUFFER_SIZE( dev->mtu, priv);
    hal_init_struct.rxDefQ = EGIGA_DEF_RXQ;
    memcpy(hal_init_struct.rxDescrNum,  egigaDescRxQ, sizeof(egigaDescRxQ));
    memcpy(hal_init_struct.txDescrNum,  egigaDescTxQ, sizeof(egigaDescTxQ));

#ifdef INCLUDE_MULTI_QUEUE
    /* Initialize RX policy */
    priv->pRxPolicyHndl = mvEthRxPolicyInit(port, EGIGA_RX_QUEUE_QUOTA, MV_ETH_PRIO_FIXED);
    if(priv->pRxPolicyHndl == NULL)
    {
        mvOsPrintf("egiga: Can't init RX Policy for Eth port #%d\n",
                    port);
	kfree( priv );
	kfree( dev );
	return -ENODEV;
    }

    /* Initialize TX policy */
    priv->pTxPolicyHndl = mvEthTxPolicyInit(port, &egigaTxDefPolicy);
    if(priv->pTxPolicyHndl == NULL)
    {
        mvOsPrintf("egiga: Can't init TX Policy for Eth port #%d\n",
                    port);
	kfree( priv );
	kfree( dev );
	return -ENODEV;
    }
#endif /* INCLUDE_MULTI_QUEUE */

    /* create internal port control structure and descriptor rings.               */
    /* open address decode windows, disable rx and tx operations. mask interrupts */
    priv->hal_priv = mvEthPortInit( port, &hal_init_struct );

    if( !priv->hal_priv ) {
        printk( KERN_ERR "%s: load failed\n", dev->name );
	kfree( priv );
	kfree( dev );
	return -ENODEV;
    }

    egiga_change_mtu_internals(dev, mtu);

#ifdef CONFIG_MV_ETH_HEADER
        switch(port) {
            case 0:
		#ifdef CONFIG_ETH_0_SHIM_ENA 
		egiga_change_rx_header_internals( 0, CONFIG_ETH_0_HEADER_SIZE, 1 );
		#endif	
                break;
            case 1:
		#ifdef CONFIG_ETH_1_SHIM_ENA 
		egiga_change_rx_header_internals( 1, CONFIG_ETH_1_HEADER_SIZE, 1 );
		#endif
		break;	
            case 2:
		#ifdef CONFIG_ETH_2_SHIM_ENA 
		egiga_change_rx_header_internals( 2, CONFIG_ETH_2_HEADER_SIZE, 1 );
		#endif
		break;	
            default:
                break; 
        }
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
   /* register the device */
   if(register_netdev(dev)) {
        printk( KERN_ERR "%s: register failed\n", dev->name );
        kfree( priv );
        kfree( dev );
   }
#endif

    return 0;

 error:
    if( priv )
        kfree( dev->priv );

    if( dev )
        kfree( dev );

    return ret;
}



/*********************************************************** 
 * egiga_unload --                                         *
 *   this is not a loadable module. nothig to be done here *
 ***********************************************************/
static int egiga_unload( int port, char *name )
{
    /* shut down ethernet port if needed. free descriptor rings. */
    /* free internal port control structure.                     */
    /* ethPortFinish( port );                                    */

    return 0;
}



/*********************************************************** 
 * egiga_start --                                          *
 *   start a network device. connect and enable interrupts *
 *   set hw defaults. fill rx buffers. restart phy link    *
 *   auto neg. set device link flags. report status.       *
 ***********************************************************/
static int egiga_start( struct net_device *dev ) 
{
    unsigned long flags;
    egiga_priv *priv = dev->priv;

    EGIGA_DBG( EGIGA_DBG_LOAD, ("%s: starting... ", dev->name ) );

    spin_lock_irqsave( &(priv->lock), flags);

    /* connect to port interrupt line */
   if( request_irq( dev->irq, egiga_interrupt_handler,
       	(SA_INTERRUPT | SA_SAMPLE_RANDOM | SA_SHIRQ) , dev->name, dev ) ) {
        printk( KERN_ERR "cannot assign irq%d to %s port%d\n", dev->irq, dev->name, priv->port );
        dev->irq = 0;
	goto error;
    }

#ifdef CONFIG_QUARTER_DECK
    if (hwState == HW_INIT)
        hwState = HW_READY;
#endif

    /* in default link is down */
    netif_carrier_off( dev );

    /* Stop the TX queue - it will be enabled upon PHY status change after link-up interrupt */
    netif_stop_queue( dev );

    /* enable polling on the port, must be used after netif_poll_disable */
    netif_poll_enable(dev);

    /* fill rx buffers, start rx/tx activity, set coalescing */
#ifdef CONFIG_QUARTER_DECK
    if (active_ifs == 0)
#endif
    if( egiga_start_internals( dev ) != 0 ) {
        printk( KERN_ERR "%s: start internals failed\n", dev->name );
	goto error;
    }
    
    switch(mvBoardIdGet())
    {
        case RD_88F5181_VOIP:
        case RD_88F5181L_VOIP_FE:
        case RD_88F5181L_VOIP_GE:
#ifdef CONFIG_QUARTER_DECK
        {
            unsigned int portsBitMask;
            portsBitMask = mvUnmGetPortMaskOfVid(priv->vid);
            memset(priv->zero_pad,0,MIN_ETH_PACKET_LEN);
#if defined (HEADERS)
            mvBindings[priv->vid]->header[0] = 0; /* DBNUM = 0 */
            mvBindings[priv->vid]->header[1] = portsBitMask;
            mvEthRxFilterModeSet(priv->hal_priv, MV_TRUE/* MV_BOOL isPromisc*/);
#elif defined (TRAILERS)
            mvBindings[priv->vid]->trailer[0] = (1<<7); /* DBNUM = 0 */
            mvBindings[priv->vid]->trailer[1] = portsBitMask;
            mvBindings[priv->vid]->trailer[2] = 0; /* DBNUM = 0 */
            mvBindings[priv->vid]->trailer[3] = 0;
            mvEthRxFilterModeSet(priv->hal_priv, MV_FALSE/* MV_BOOL isPromisc*/);
#endif
            mvEthPortUp( priv->hal_priv );
            netif_carrier_on( dev );
            netif_wake_queue( dev );
        }
#else /*CONFIG_QUARTER_DECK*/
        panic(" VOIP board used without configuring QUARTER DECK support");
#endif
        break;
        default:   
#ifdef CONFIG_QUARTER_DECK
         panic("This board (%d)doesn't support QUARTER DECK ",mvBoardIdGet());
#endif
	/* restart phy link auto negotiation */
    	restart_autoneg( priv->port );
    }
#ifdef CONFIG_QUARTER_DECK
     active_ifs++;
#endif

    EGIGA_DBG( EGIGA_DBG_LOAD, ("%s: start ok\n", dev->name) );

    spin_unlock_irqrestore( &(priv->lock), flags);

    return 0;

 error:
    spin_unlock_irqrestore( &(priv->lock), flags);

#ifdef CONFIG_QUARTER_DECK
    if(active_ifs == 0)
#endif
    if( dev->irq != 0 )
    {
       	free_irq( dev->irq, dev );
#ifdef CONFIG_QUARTER_DECK
        hwState = HW_INIT;
#endif		
    }

    printk( KERN_ERR "%s: start failed\n", dev->name );
    return -1;
}



/*********************************************************** 
 * egiga_start_internals --                                *
 *   fill rx buffers. start rx/tx activity. set coalesing. *
 *   clear and unmask interrupt bits                       *
 ***********************************************************/
static int egiga_start_internals( struct net_device *dev )
{
    unsigned int status;
    unsigned int queue;

    egiga_priv *priv = dev->priv;
 
    /* fill rx ring with buffers */
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++) {
    	egiga_rx_fill( dev, queue, EGIGA_Q_DESC(queue));
    }

    /* clear all ethernet port interrupts */
    MV_REG_WRITE( ETH_INTR_CAUSE_REG( priv->port ), 0 );
    MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG( priv->port ), 0 );

    /* start the hal - rx/tx activity */
    status = mvEthPortEnable( priv->hal_priv );
    if( (status != MV_OK) && (status != MV_NOT_READY)){
         printk( KERN_ERR "%s: ethPortEnable failed", dev->name );
	 return -1;
    }

    /* set tx/rx coalescing mechanism */
    priv->tx_coal = mvEthTxCoalSet( priv->hal_priv, EGIGA_TX_COAL );
    priv->rx_coal = mvEthRxCoalSet( priv->hal_priv, EGIGA_RX_COAL );

    /* unmask rx-ready-q0, tx-done-q0, phy-statust-change, and link-status-changes */
    MV_REG_WRITE( ETH_INTR_MASK_REG( priv->port ), EGIGA_PICR_MASK );
    priv->rxmask = EGIGA_PICR_MASK;
    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG( priv->port ), EGIGA_PICER_MASK );
    priv->txmask = EGIGA_PICER_MASK;

    return 0;
}



/*********************************************************** 
 * egiga_close --	                                   *
 *   stop interface with linux core. stop port activity.   *
 *   free skb's from rings. set defaults to hw. disconnect *
 *   interrupt line.                                       *
 ***********************************************************/
static int egiga_close( struct net_device *dev )
{
    unsigned long flags;
    egiga_priv *priv = dev->priv;

    spin_lock_irqsave( &(priv->lock), flags);

    /* stop upper layer */
    netif_carrier_off( dev );
    netif_stop_queue( dev );
#ifdef CONFIG_QUARTER_DECK
    active_ifs--;
    if( active_ifs == 0)
    {
#endif
        /* stop tx/rx activity, mask all interrupts, relese skb in rings,*/
        egiga_stop_internals( dev );

        /* clear cause registers. mask interrupts. clear MAC tables. */
        /* set defaults. reset descriptors ring. reset PHY.          */
        if( mvEthDefaultsSet( priv->hal_priv ) != MV_OK ) {
            printk( KERN_ERR "%s: error set default on stop", dev->name );
	    goto error;
        }
#ifdef CONFIG_QUARTER_DECK
    }
#endif

#ifdef CONFIG_MV_ETH_HEADER
    priv->rx_header_size = 0;
#endif

    spin_unlock_irqrestore( &priv->lock, flags);


#ifdef CONFIG_QUARTER_DECK
    if(active_ifs == 0)
#endif
    if( dev->irq != 0 )
    {
       	free_irq( dev->irq, dev );
#ifdef CONFIG_QUARTER_DECK
        hwState = HW_INIT;
#endif		
    }
    
    return 0;

 error:
    printk( KERN_ERR "%s: stop failed\n", dev->name );
    spin_unlock_irqrestore( &priv->lock, flags);
    return -1;
    
}

/*********************************************************** 
 * egiga_stop --                                 	   *
 *   stop interface with linux core. stop port activity.   *
 *   free skb's from rings.                                *
 ***********************************************************/
static int egiga_stop( struct net_device *dev )
{
    unsigned long flags;
    egiga_priv *priv = dev->priv;

    /* first make sure that the port finished its Rx polling - see tg3 */
    /* otherwise it may cause issue in SMP, one CPU is here and the other is doing the polling
	and both of it are messing with the descriptors rings!! */
    netif_poll_disable( dev );

    spin_lock_irqsave( &(priv->lock), flags);

    /* stop upper layer */
    netif_carrier_off( dev );
    netif_stop_queue( dev );

#ifdef CONFIG_QUARTER_DECK
    active_ifs--;
    if( active_ifs == 0)
    {
#endif
        /* stop tx/rx activity, mask all interrupts, relese skb in rings,*/
        egiga_stop_internals( dev );
#ifdef CONFIG_QUARTER_DECK
    } 
#endif
    

    spin_unlock_irqrestore( &priv->lock, flags);

#ifdef CONFIG_QUARTER_DECK
    if(active_ifs == 0)
#endif
    if( dev->irq != 0 )
    {
       	free_irq( dev->irq, dev );
#ifdef CONFIG_QUARTER_DECK
        hwState = HW_INIT;
#endif		
    }

    return 0;
}


/***********************************************************
 * egiga_down_internals --                                 *
 *   down port rx/tx activity. free skb's from rx/tx rings.*
 ***********************************************************/
static int egiga_down_internals( struct net_device *dev )
{
    egiga_priv *priv = dev->priv;
    MV_PKT_INFO pkt_info;
    unsigned int queue;

    /* stop the port activity, mask all interrupts */
    if( mvEthPortDown( priv->hal_priv ) != MV_OK ) {
        printk( KERN_ERR "%s: ethPortDown failed\n", dev->name );
        goto error;
    }

    /* free the skb's in the hal tx ring */
    for(queue = 0; queue < MV_ETH_TX_Q_NUM; queue++) {
    	while( mvEthPortForceTxDone( priv->hal_priv, queue, &pkt_info ) == MV_OK ) {
        	priv->txq_count[queue]--;
        	if( pkt_info.osInfo )
            		dev_kfree_skb_any( (struct sk_buff *)pkt_info.osInfo );
        	else {
            		printk( KERN_ERR "%s: error in ethGetNextRxBuf\n", dev->name );
            		goto error;
        	}
    	}
    }

    return 0;

 error:
    printk( KERN_ERR "%s: stop internals failed\n", dev->name );
    return -1;
}


/*********************************************************** 
 * egiga_stop_internals --                                 *
 *   stop port rx/tx activity. free skb's from rx/tx rings.*
 ***********************************************************/
static int egiga_stop_internals( struct net_device *dev )
{
    egiga_priv *priv = dev->priv;
    MV_PKT_INFO pkt_info;
    unsigned int queue;

    /* stop the port activity, mask all interrupts */
    if( mvEthPortDisable( priv->hal_priv ) != MV_OK ) {
        printk( KERN_ERR "%s: ethPortDisable failed\n", dev->name );
        goto error;
    }
    
    /* clear all ethernet port interrupts */
    MV_REG_WRITE( ETH_INTR_CAUSE_REG( priv->port ), 0 );
    MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG( priv->port ), 0 );

    /* mask rx-ready-q0, tx-done-q0, phy-statust-change, and link-status-changes */
    MV_REG_WRITE( ETH_INTR_MASK_REG( priv->port ), 0 );
    priv->rxmask = 0;
    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG( priv->port ), 0 );
    priv->txmask = 0;

    /* free the skb's in the hal tx ring */
    for(queue = 0; queue < MV_ETH_TX_Q_NUM; queue++)
    {
    	while( mvEthPortForceTxDone( priv->hal_priv, queue, &pkt_info ) == MV_OK ) {
        	priv->txq_count[queue]--;
		if( pkt_info.osInfo )
	    		dev_kfree_skb_any( (struct sk_buff *)pkt_info.osInfo );
		else {
	    		printk( KERN_ERR "%s: error in ethGetNextRxBuf\n", dev->name );
	    		goto error;
		}
    	}
    }
    /* free the skb's in the hal rx ring */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {    
    	while( mvEthPortForceRx( priv->hal_priv, queue, &pkt_info) == MV_OK ) {
        	priv->rxq_count[queue]--;
		if( pkt_info.osInfo )
	    		dev_kfree_skb_any( (struct sk_buff *)pkt_info.osInfo );
		else {
	    		printk( KERN_ERR "%s: error in ethGetNextRxBuf\n", dev->name );
	    		goto error;
		}
    	}
    }

    /* Reset Rx descriptors ring */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
	ethResetRxDescRing(priv->hal_priv, queue);
    }
    /* Reset Tx descriptors ring */
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
	ethResetTxDescRing(priv->hal_priv, queue);
    }

    return 0;

 error:
    printk( KERN_ERR "%s: stop internals failed\n", dev->name );
    return -1;
}


/*********************************************************** 
 * egiga_tx --                                             *
 *   send a packet.                                        *
 ***********************************************************/
static int egiga_tx( struct sk_buff *skb , struct net_device *dev )
{
    egiga_priv *priv = dev->priv;
    struct net_device_stats *stats = &priv->stats;
    unsigned long flags;
    MV_STATUS status;
    int ret = 0, i, queue;

    if( netif_queue_stopped( dev ) ) {
        printk( KERN_ERR "%s: transmitting while stopped\n", dev->name );
        return 1;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
    local_irq_save(flags);
    if (!spin_trylock(&priv->lock)) {
    	/* Collision - tell upper layer to requeue */
        local_irq_restore(flags);
        return NETDEV_TX_LOCKED;
    }
#else
    spin_lock_irqsave( &(priv->lock), flags );
#endif

    EGIGA_DBG( EGIGA_DBG_TX, ("%s: tx, #%d frag(s), csum by %s\n",
             dev->name, skb_shinfo(skb)->nr_frags+1, (skb->ip_summed==CHECKSUM_HW)?"HW":"CPU") );
    EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_events++) );

    /* basic init of pkt_info. first cell in buf_info_arr is left for header prepending if necessary */
    priv->tx_pkt_info.osInfo = (MV_ULONG)skb;
    priv->tx_pkt_info.pktSize = skb->len;
    priv->tx_pkt_info.pFrags = &priv->tx_buf_info_arr[1];
    priv->tx_pkt_info.status = 0;
    
    /* see if this is a single/multiple buffered skb */
    if( skb_shinfo(skb)->nr_frags == 0 ) {
        priv->tx_pkt_info.pFrags->bufVirtPtr = skb->data;
        priv->tx_pkt_info.pFrags->bufSize = skb->len;
        priv->tx_pkt_info.numFrags = 1;
    }
    else {

        MV_BUF_INFO *p_buf_info = priv->tx_pkt_info.pFrags;

        /* first skb fragment */
        p_buf_info->bufSize = skb_headlen(skb);
        p_buf_info->bufVirtPtr = skb->data;
        p_buf_info++;

        /* now handle all other skb fragments */
        for ( i = 0; i < skb_shinfo(skb)->nr_frags; i++ ) {

            skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

            p_buf_info->bufSize = frag->size;
            p_buf_info->bufVirtPtr = page_address(frag->page) + frag->page_offset;
            p_buf_info++;
        }

        priv->tx_pkt_info.numFrags = skb_shinfo(skb)->nr_frags + 1;
    }

#ifdef TX_CSUM_OFFLOAD
    /* if HW is suppose to offload layer4 checksum, set some bits in the first buf_info command */
    if(skb->ip_summed == CHECKSUM_HW) {
        EGIGA_DBG( EGIGA_DBG_TX, ("%s: tx csum offload\n", dev->name) );
        /*EGIGA_STAT( EGIGA_STAT_TX, Add counter here );*/
        priv->tx_pkt_info.status =
        ETH_TX_IP_NO_FRAG |           /* we do not handle fragmented IP packets. add check inside iph!! */
        ((skb->nh.iph->ihl) << ETH_TX_IP_HEADER_LEN_OFFSET) |                            /* 32bit units */
        ((skb->nh.iph->protocol == IPPROTO_TCP) ? ETH_TX_L4_TCP_TYPE : ETH_TX_L4_UDP_TYPE) | /* TCP/UDP */
        ETH_TX_GENERATE_L4_CHKSUM_MASK |                                /* generate layer4 csum command */
        ETH_TX_GENERATE_IP_CHKSUM_BIT;                              /* generate IP csum (already done?) */
    }
    else {
        EGIGA_DBG( EGIGA_DBG_TX, ("%s: no tx csum offload\n", dev->name) );
        /*EGIGA_STAT( EGIGA_STAT_TX, Add counter here );*/
        priv->tx_pkt_info.status = 0x5 << ETH_TX_IP_HEADER_LEN_OFFSET; /* Errata BTS #50 */
    }
#endif

    /* At this point we need to decide to which tx queue this packet goes, */
    /* and whether we need to prepend a proprietary header.                */
#ifdef INCLUDE_MULTI_QUEUE
#ifdef CONFIG_MV_ETH_HEADER
    /* First case: tx queue number + prepended header */
    queue = mvEthTxPolicyGet(priv->pTxPolicyHndl, &priv->tx_pkt_info, &tx_policy_entry);

    if(tx_policy_entry.headerSize != 0) {

        /* use the first empty cell in buf_info_arr */
        priv->tx_pkt_info.pFrags = &priv->tx_buf_info_arr[0];
        priv->tx_pkt_info.pFrags->bufVirtPtr = tx_policy_entry.pHeader;
        priv->tx_pkt_info.pFrags->bufSize = tx_policy_entry.headerSize;

        priv->tx_pkt_info.pktSize += tx_policy_entry.headerSize;
        priv->tx_pkt_info.numFrags++;
    }
#else
    /* Second case: tx queue number (no prepended header) */
    queue = mvEthTxPolicyGet(priv->pTxPolicyHndl, &priv->tx_pkt_info, NULL);
#endif
#else
    /* no multiqueue. all packets go to one default queue. */
    queue = EGIGA_DEF_TXQ;
#endif

#ifdef CONFIG_QUARTER_DECK
    if(priv->tx_pkt_info.pktSize < MIN_ETH_PACKET_LEN) {
        /* add zero pad into the next cell in buf_info_arr */
        EGIGA_DBG(EGIGA_DBG_TX,("add zero pad\n"));
        priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufVirtPtr = &(priv->zero_pad[skb->len]);
        priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufSize = MIN_ETH_PACKET_LEN - priv->tx_pkt_info.pktSize;
        priv->tx_pkt_info.pktSize += priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufSize;
        priv->tx_pkt_info.numFrags++;
    }
#if defined (HEADERS)
    if( (skb->data - skb->head) >= HEADER_SIZE ) {
        /* push the header inside skb and update first frag */
        EGIGA_DBG(EGIGA_DBG_TX,("add header in skb\n"));
        memcpy(skb_push(skb,HEADER_SIZE),mvBindings[priv->vid]->header,HEADER_SIZE);
        priv->tx_pkt_info.pFrags->bufVirtPtr -= HEADER_SIZE;
        priv->tx_pkt_info.pFrags->bufSize += HEADER_SIZE;
        priv->tx_pkt_info.pktSize += priv->tx_pkt_info.pFrags->bufSize;
    }
    else {
        /* add header as a new frag into the first empty cell in buf_info_arr */
        EGIGA_DBG(EGIGA_DBG_TX,("add header in first frag\n"));
        priv->tx_buf_info_arr[0].bufVirtPtr = mvBindings[priv->vid]->header;
        priv->tx_buf_info_arr[0].bufSize = HEADER_SIZE;
        priv->tx_pkt_info.pFrags = &priv->tx_buf_info_arr[0];
        priv->tx_pkt_info.pktSize += priv->tx_buf_info_arr[0].bufSize;
        priv->tx_pkt_info.numFrags++;
    }
#elif defined (TRAILERS)
    if((priv->tx_pkt_info.numFrags == 1) && (skb->end - skb->tail >= TRAILER_SIZE)) {
        /* add the trailer into the skb (no zero pad frag, no frag skb)*/
        EGIGA_DBG(EGIGA_DBG_TX,("add trailer in skb\n"));
        memcpy(skb_put(skb,TRAILER_SIZE),mvBindings[priv->vid]->trailer,TRAILER_SIZE);
        priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags].bufSize += TRAILER_SIZE;
        priv->tx_pkt_info.pktSize += priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags].bufSize;
    }
    else {
        /* add trailer as a new frag into the next cell in buf_info_arr */
        EGIGA_DBG(EGIGA_DBG_TX,("add trailer in next frag\n"));
        priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufVirtPtr = mvBindings[priv->vid]->trailer;
        priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufSize = TRAILER_SIZE;
        priv->tx_pkt_info.pktSize += priv->tx_buf_info_arr[priv->tx_pkt_info.numFrags+1].bufSize;
        priv->tx_pkt_info.numFrags++;
    }
#endif
#endif

    /* now send the packet */
    status = mvEthPortTx( priv->hal_priv, queue, &priv->tx_pkt_info );

    /* check status */
    if( status == MV_OK ) {
        stats->tx_bytes += skb->len;
        stats->tx_packets ++;
        dev->trans_start = jiffies;
        priv->txq_count[queue]++;
        EGIGA_DBG( EGIGA_DBG_TX, ("ok (%d); ", priv->txq_count[queue]) );
        EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_hal_ok[queue]++) );
    }
    else {
        /* tx failed. higher layers will free the skb */
        ret = 1;
        stats->tx_dropped++;

        if( status == MV_NO_RESOURCE ) {
            /* it must not happen because we call to netif_stop_queue in advance. */
            EGIGA_DBG( EGIGA_DBG_TX, ("%s: queue is full, stop transmit\n", dev->name) );
            netif_stop_queue( dev );
            EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_hal_no_resource[queue]++) );
            EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_netif_stop[queue]++) );
        }
        else if( status == MV_ERROR ) {
            printk( KERN_ERR "%s: error on transmit\n", dev->name );
            EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_hal_error[queue]++) );
        }
        else {
            printk( KERN_ERR "%s: unrecognize status on transmit\n", dev->name );
            EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_hal_unrecognize[queue]++) );
        }
    }

#ifndef INCLUDE_MULTI_QUEUE
    /* if number of available descriptors left is less than  */
    /* MAX_SKB_FRAGS stop the stack. if multi queue is used, */
    /* don't stop the stack just because one queue is full.  */
    if( mvEthTxResourceGet(priv->hal_priv, queue) - skb_shinfo(skb)->nr_frags <= MAX_SKB_FRAGS ) {
        EGIGA_DBG( EGIGA_DBG_TX, ("%s: stopping network tx interface\n", dev->name) );
        netif_stop_queue( dev );
        EGIGA_STAT( EGIGA_STAT_TX, (priv->egiga_stat.tx_netif_stop[queue]++) );
    }
#endif
    spin_unlock_irqrestore( &(priv->lock), flags );

    return ret;
}

/*********************************************************** 
 * egiga_tx_done --                                             *
 *   release transmitted packets. interrupt context.       *
 ***********************************************************/
static u32 egiga_tx_done( struct net_device *dev )
{
    egiga_priv *priv = dev->priv;
    struct net_device_stats *stats = &priv->stats;
    MV_PKT_INFO pkt_info;
    u32 count = 0;
    MV_STATUS status;
    unsigned int queue = 0;

    EGIGA_DBG( EGIGA_DBG_TX_DONE, ("%s: tx-done ", dev->name) );
    EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_events++) );

    /* release the transmitted packets */
    while( 1 ) {

#ifdef INCLUDE_MULTI_QUEUE
        if(priv->txcause == 0)
            break;

        while( (priv->txcause & ETH_CAUSE_TX_BUF_MASK(queue)) == 0) 
        {
            queue++; /* Can't pass MAX Q */
        }
#else
        queue = EGIGA_DEF_TXQ;
#endif /* INCLUDE_MULTI_QUEUE */

        /* get a packet */  
        status = mvEthPortTxDone( priv->hal_priv, queue, &pkt_info );

	if( status == MV_OK ) {

	    priv->txq_count[queue]--;

	    /* validate skb */
	    if( !(pkt_info.osInfo) ) {
	        printk( KERN_ERR "%s: error in tx-done\n",dev->name );
		stats->tx_errors++;
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_invalid_skb[queue]++) );
		continue;
	    }

	    /* handle tx error */
	    if( pkt_info.status & (ETH_ERROR_SUMMARY_BIT) ) {
	        EGIGA_DBG( EGIGA_DBG_TX_DONE, ("%s: bad tx-done status\n",dev->name) );
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_bad_stat[queue]++) );
		stats->tx_errors++;
	    }

	    /* it transmission was previously stopped, now it can be restarted. */
	    if( netif_queue_stopped( dev ) && (dev->flags & IFF_UP) ) {

	        EGIGA_DBG( EGIGA_DBG_TX_DONE, ("%s: restart transmit\n", dev->name) );
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_netif_wake[queue]++) );
		netif_wake_queue( dev );	
	    }

	    /* release the skb */
	    dev_kfree_skb_irq( (struct sk_buff *)pkt_info.osInfo );
	    count++;
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_ok[queue]++) );
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, if(priv->egiga_stat.tx_done_max[queue] < count) priv->egiga_stat.tx_done_max[queue] = count );
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, if(priv->egiga_stat.tx_done_min[queue] > count) priv->egiga_stat.tx_done_min[queue] = count );
	}
	else {
		if( status == MV_EMPTY ) {
	    		/* no more work */
	    		EGIGA_DBG( EGIGA_DBG_TX_DONE, ("no more work ") );
	    		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_no_more[queue]++) );
		}
		else if( status == MV_NOT_FOUND ) {
	    		/* hw still in tx */
	    		EGIGA_DBG( EGIGA_DBG_TX_DONE, ("hw still in tx ") );
	    		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_still_tx[queue]++) );
		}
		else {
	    		printk( KERN_ERR "%s: unrecognize status on tx done\n", dev->name );
	    		EGIGA_STAT( EGIGA_STAT_TX_DONE, (priv->egiga_stat.tx_done_hal_unrecognize[queue]++) );
	    		stats->tx_errors++;
		}
#ifdef INCLUDE_MULTI_QUEUE
	        priv->txcause &= ~ETH_CAUSE_TX_BUF_MASK(queue);
#else
		break;
#endif
    	}
    }

    EGIGA_DBG( EGIGA_DBG_TX_DONE, ("%s: tx-done %d (%d)\n", dev->name, count, priv->txq_count[queue]) );
    return count;
}



/*********************************************************** 
 * egiga_tx_timeout --                                     *
 *   nothing to be done (?)                                *
 ***********************************************************/
static void egiga_tx_timeout( struct net_device *dev ) 
{
    EGIGA_STAT( EGIGA_STAT_TX, ( ((egiga_priv*)&(dev->priv))->egiga_stat.tx_timeout++) );
    printk( KERN_INFO "%s: tx timeout\n", dev->name );
}

#ifdef RX_CSUM_OFFLOAD
static MV_STATUS egiga_rx_csum_offload(MV_PKT_INFO *pkt_info)
{
    if( (pkt_info->pktSize > RX_CSUM_MIN_BYTE_COUNT)   && /* Minimum        */
        (pkt_info->status & ETH_RX_IP_FRAME_TYPE_MASK) && /* IPv4 packet    */
        (pkt_info->status & ETH_RX_IP_HEADER_OK_MASK)  && /* IP header OK   */
        (!(pkt_info->fragIP))                          && /* non frag IP    */
        (!(pkt_info->status & ETH_RX_L4_OTHER_TYPE))   && /* L4 is TCP/UDP  */
        (pkt_info->status & ETH_RX_L4_CHECKSUM_OK_MASK) ) /* L4 checksum OK */
            return MV_OK;

    if(!(pkt_info->pktSize > RX_CSUM_MIN_BYTE_COUNT))
        EGIGA_DBG( EGIGA_DBG_RX, ("Byte count smaller than %d\n", RX_CSUM_MIN_BYTE_COUNT) );
    if(!(pkt_info->status & ETH_RX_IP_FRAME_TYPE_MASK))
        EGIGA_DBG( EGIGA_DBG_RX, ("Unknown L3 protocol\n") );
    if(!(pkt_info->status & ETH_RX_IP_HEADER_OK_MASK))
        EGIGA_DBG( EGIGA_DBG_RX, ("Bad IP csum\n") );
    if(pkt_info->fragIP)
        EGIGA_DBG( EGIGA_DBG_RX, ("Fragmented IP\n") );
    if(pkt_info->status & ETH_RX_L4_OTHER_TYPE)
        EGIGA_DBG( EGIGA_DBG_RX, ("Unknown L4 protocol\n") );
    if(!(pkt_info->status & ETH_RX_L4_CHECKSUM_OK_MASK))
        EGIGA_DBG( EGIGA_DBG_RX, ("Bad L4 csum\n") );

    return MV_FAIL;
}
#endif
static int egiga_poll( struct net_device *dev, int *budget )
{
    int rx_work_done;
    int tx_work_done;
    unsigned long flags;
    egiga_priv *priv = dev->priv;

    EGIGA_STAT( EGIGA_STAT_INT, (priv->egiga_stat.poll_events++) );

    tx_work_done = egiga_tx_done(dev);
    rx_work_done = egiga_rx( dev, min(*budget,dev->quota) );

    *budget -= rx_work_done;
    dev->quota -= rx_work_done;

    EGIGA_DBG( EGIGA_DBG_INT, ("poll work done: tx-%d rx-%d\n",tx_work_done,rx_work_done) );

    if( ((tx_work_done==0) && (rx_work_done==0)) || (!netif_running(dev)) ) {
	    local_irq_save(flags);
            netif_rx_complete(dev);
	    EGIGA_STAT( EGIGA_STAT_INT, (priv->egiga_stat.poll_complete++) );
	    /* unmask interrupts */
	    MV_REG_WRITE( ETH_INTR_MASK_REG(priv->port), EGIGA_PICR_MASK );
	    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(priv->port), EGIGA_PICER_MASK );
	    priv->rxmask = EGIGA_PICR_MASK;
	    priv->txmask = EGIGA_PICER_MASK;
	    EGIGA_DBG( EGIGA_DBG_RX, ("unmask\n") );
	    local_irq_restore(flags);
            return 0;
    }

    return 1;
}

/*********************************************************** 
 * egiga_rx_poll --                                        *
 *   NAPI rx polling method. deliver rx packets to linux   *
 *   core. refill new rx buffers. unmaks rx interrupt only *
 *   if all packets were delivered.                        *
 ***********************************************************/
static int egiga_rx( struct net_device *dev,unsigned int work_to_do )
{
    egiga_priv *priv = dev->priv;
    struct net_device_stats *stats = &(priv->stats);
    struct sk_buff *skb;
    MV_PKT_INFO pkt_info;
    int work_done = 0;
    MV_STATUS status;
    unsigned int queue = 0;
    unsigned int done_per_q[MV_ETH_RX_Q_NUM] = {0,};
#if defined (CONFIG_QUARTER_DECK)
    unsigned char ucSrcPort;
    MV_UNM_VID vid;
#endif

#ifdef INCLUDE_MULTI_QUEUE
    unsigned int temp;
    /* Read cause once more */
    temp = MV_REG_READ(ETH_INTR_CAUSE_REG(port));
    priv->rxcause |= temp & EGIGA_RXQ_MASK;
    priv->rxcause |= (temp & EGIGA_RXQ_RES_MASK) >> (ETH_CAUSE_RX_ERROR_OFFSET - ETH_CAUSE_RX_READY_OFFSET);
    MV_REG_WRITE(ETH_INTR_CAUSE_REG(port), ~(priv->rxcause | (priv->rxcause << (ETH_CAUSE_RX_ERROR_OFFSET - ETH_CAUSE_RX_READY_OFFSET)) ) );

    EGIGA_DBG( EGIGA_DBG_RX,("%s: cause = 0x%08x\n\n", dev->name, priv->rxcause) );
#endif /* INCLUDE_MULTI_QUEUE */

    EGIGA_DBG( EGIGA_DBG_RX, ("%s: rx_poll work_to_do %d\n", dev->name, work_to_do) );

    EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_events++) );

    /* fairness NAPI loop */
    while( work_done < work_to_do ) {

#ifdef INCLUDE_MULTI_QUEUE
        if(priv->rxcause == 0)
            break;
        queue = mvEthRxPolicyGet(priv->pRxPolicyHndl, priv->rxcause);
#else
        queue = EGIGA_DEF_RXQ;
#endif /* INCLUDE_MULTI_QUEUE */

        /* get rx packet */ 
	status = mvEthPortRx( priv->hal_priv, queue, &pkt_info );

        /* check status */
	if( status == MV_OK ) {
	    work_done++;
	    done_per_q[queue]++;
	    priv->rxq_count[queue]--;
	    EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_ok[queue]++) );

	} else{ 
		if( status == MV_NO_RESOURCE ) {
	    		/* no buffers for rx */
	    		EGIGA_DBG( EGIGA_DBG_RX, ("%s: rx_poll no resource ", dev->name) );
	    		stats->rx_errors++;
	    		EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_no_resource[queue]++) );

		} else if( status == MV_NO_MORE ) {
	    		/* no more rx packets ready */
	    		EGIGA_DBG( EGIGA_DBG_RX, ("%s: rx_poll no more ", dev->name) );
	    		EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_no_more[queue]++) );

		} else {
	    		printk( KERN_ERR "%s: unrecognize status on rx poll\n", dev->name );
	    		stats->rx_errors++;
	    		EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_error[queue]++) );
		}

#ifdef INCLUDE_MULTI_QUEUE
		priv->rxcause &= ~ETH_CAUSE_RX_READY_MASK(queue);
		continue;
#else
		break;
#endif
	}

	/* validate skb */ 
	if( !(pkt_info.osInfo) ) {
	    printk( KERN_ERR "%s: error in rx\n",dev->name );
	    stats->rx_errors++;
	    EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_invalid_skb[queue]++) );
	    continue;
	}

	skb = (struct sk_buff *)( pkt_info.osInfo );

	/* handle rx error */
	if( pkt_info.status & (ETH_ERROR_SUMMARY_MASK) ) {
            u32 err = pkt_info.status & ETH_RX_ERROR_CODE_MASK;
            /* RX resource error is likely to happen when receiving packets, which are     */
            /* longer then the Rx buffer size, and they are spreading on multiple buffers. */
            /* Rx resource error - No descriptor in the middle of a frame.                 */
	    if( err == ETH_RX_RESOURCE_ERROR ) {
	        EGIGA_DBG( EGIGA_DBG_RX, ("%s: bad rx status %08x, (resource error)",dev->name, (unsigned int)pkt_info.status));
            }
	    else if( err == ETH_RX_OVERRUN_ERROR ) {
		EGIGA_DBG( EGIGA_DBG_RX, ("%s: bad rx status %08x, (overrun error)",dev->name, (unsigned int)pkt_info.status));
            }
	    else {
		printk( KERN_INFO "%s: bad rx status %08x, ",dev->name, (unsigned int)pkt_info.status );
	    	if( err == ETH_RX_MAX_FRAME_LEN_ERROR )
	        	printk( KERN_INFO "(max frame length error)" );
	    	else if( err == ETH_RX_CRC_ERROR )
	        	printk( KERN_INFO "(crc error)" );
	    	else
	        	printk( KERN_INFO "(unknown error)" );
	    	printk( KERN_INFO "\n" );
	    }
	    
	    dev_kfree_skb( skb );
	    stats->rx_errors++;
	    EGIGA_STAT( EGIGA_STAT_RX, (priv->egiga_stat.rx_poll_hal_bad_stat[queue]++) );
	    continue;
	}

	/* good rx */
        EGIGA_DBG( EGIGA_DBG_RX, ("good rx. skb=%p, skb->data=%p\n", skb, skb->data) );
	stats->rx_packets++;
	stats->rx_bytes += pkt_info.pktSize; /* include 4B crc */

        prefetch( (void *)(skb->data) );

#if defined CONFIG_QUARTER_DECK
#if defined (TRAILERS)
        ucSrcPort = ((skb->data[pkt_info.pktSize - 8 + 1 - 2])    & 0xf );
#elif defined (HEADERS)
        ucSrcPort = ((skb->data[1]) & 0xf );
#endif
        mvOsAssert( ucSrcPort <= qd_dev->numOfPorts );
        mvOsAssert( ucSrcPort != qd_dev->cpuPortNum );
        vid = mvUnmGetVidOfPort(ucSrcPort);
        mvOsAssert( vid != 0);
        mvOsAssert( mvBindings[vid] != NULL );
        mvOsAssert( mvBindings[vid]->dev != NULL);
        skb->dev = mvBindings[vid]->dev;
#if defined (TRAILERS)
        /* reduce 4B crc, 2B added by hw, and the TRAILER */
        EGIGA_DBG( EGIGA_DBG_RX, ("skip QD trailer\n") );
        skb_put( skb, pkt_info.pktSize - 4 - 2 - TRAILER_SIZE );
#elif defined (HEADERS)
        /* reduce 4B crc, 2B added by gigabit hw */
        EGIGA_DBG( EGIGA_DBG_RX, ("skip QD header\n") );
        skb_put(skb, (pkt_info.pktSize - 4 - 2));
	/* make IP header align by copying the data 2B backwords and overriding switch header */
	memmove(skb->data, skb->data+2, skb->len);
#endif
        if(mvBindings[vid]->dev != dev) {
            egiga_priv *vpriv = mvBindings[vid]->dev->priv;
            struct net_device_stats *vstats = &(vpriv->stats);
            vstats->rx_packets++;
            vstats->rx_bytes += pkt_info.pktSize;
            stats->rx_packets--;
            stats->rx_bytes -= pkt_info.pktSize;
        }

#else /* !CONFIG_QUARTER_DECK */
       	/* reduce 4B crc, 2B added by hw */
        skb_put( skb, pkt_info.pktSize - 4 - 2 );
	skb->dev = dev;
#endif

#ifdef RX_CSUM_OFFLOAD
        /* checksum offload */
        if( egiga_rx_csum_offload( &pkt_info ) == MV_OK ) {

                EGIGA_DBG( EGIGA_DBG_RX, ("%s: rx csum offload ok\n", dev->name) );
                /* EGIGA_STAT( EGIGA_STAT_RX, Add counter here) */

                skb->ip_summed = CHECKSUM_UNNECESSARY;

                /* Is this necessary? */
                skb->csum = htons((pkt_info.status & ETH_RX_L4_CHECKSUM_MASK) >> ETH_RX_L4_CHECKSUM_OFFSET);
        }
        else {
                EGIGA_DBG( EGIGA_DBG_RX, ("%s: rx csum offload failed\n", dev->name) );
                /* EGIGA_STAT( EGIGA_STAT_RX, Add counter here) */
                skb->ip_summed = CHECKSUM_NONE;
        }
#else
        skb->ip_summed = CHECKSUM_NONE;
#endif

#if defined (HEADERS) || defined (TRAILERS) 
       skb->protocol = eth_type_trans(skb,mvBindings[vid]->dev); 
#else
	skb->protocol = eth_type_trans(skb, dev); 
#endif

	status = netif_receive_skb( skb );
        EGIGA_STAT( EGIGA_STAT_RX, if(status) (priv->egiga_stat.rx_poll_netif_drop[queue]++) );
    }

    EGIGA_DBG( EGIGA_DBG_RX, ("\nwork_done %d (%d)", work_done, priv->rxq_count[queue]) );

    /* refill rx ring with new buffers */
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++) {
	if(done_per_q[queue] > 0) {
	    	egiga_rx_fill( dev, queue, EGIGA_Q_DESC(queue) );
	}
    }


    /* notify upper layer about more work to do */
    return( work_done );
}



/*********************************************************** 
 * egiga_rx_fill --                                        *
 *   fill new rx buffers to ring.                          *
 ***********************************************************/
static u32 egiga_rx_fill( struct net_device *dev, unsigned int queue, int total )
{
    egiga_priv *priv = dev->priv;
    MV_PKT_INFO pkt_info;
    MV_BUF_INFO bufInfo;
    struct sk_buff *skb;
    u32 count = 0, buf_size;
    MV_STATUS status;
    int alloc_skb_failed = 0;

    EGIGA_DBG( EGIGA_DBG_RX_FILL, ("%s: rx fill queue %d", dev->name, queue) );
    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_events[queue]++) );

    while( total-- ) {

        /* allocate a buffer */
	buf_size = RX_BUFFER_SIZE( dev->mtu, priv) + 32 /* 32(extra for cache prefetch) */ + 8 /* +8 to align on 8B */;

        skb = dev_alloc_skb( buf_size ); 
	if( !skb ) {
	    EGIGA_DBG( EGIGA_DBG_RX_FILL, ("%s: rx_fill cannot allocate skb\n", dev->name) );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_alloc_skb_fail[queue]++) );
	    alloc_skb_failed = 1;
	    break;
	}

	/* align the buffer on 8B */
	if( (unsigned long)(skb->data) & 0x7 ) {
	    skb_reserve( skb, 8 - ((unsigned long)(skb->data) & 0x7) );
	}

        bufInfo.bufVirtPtr = skb->data;
        bufInfo.bufSize = RX_BUFFER_SIZE( dev->mtu, priv);
        pkt_info.osInfo = (MV_ULONG)skb;
        pkt_info.pFrags = &bufInfo;
	pkt_info.pktSize = RX_BUFFER_SIZE( dev->mtu, priv); /* how much to invalidate */

	/* skip on first 2B (HW header) */
	skb_reserve( skb, 2 );

#ifdef CONFIG_MV_ETH_HEADER
	/* reserve place for Marvell header */
	skb_reserve( skb, priv->rx_header_size);
#endif
	/* give the buffer to hal */
	status = mvEthPortRxDone( priv->hal_priv, queue, &pkt_info );
	
	if( status == MV_OK ) {
	    count++;
	    priv->rxq_count[queue]++;
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_hal_ok[queue]++) );	    
	}
	else if( status == MV_FULL ) {
	    /* the ring is full */
	    count++;
	    priv->rxq_count[queue]++;
	    EGIGA_DBG( EGIGA_DBG_RX_FILL, ("%s: rxq full\n", dev->name) );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_hal_full[queue]++) );
	    if( priv->rxq_count[queue] != EGIGA_Q_DESC(queue))
	        printk( KERN_ERR "%s Q %d: error in status fill (%d != %d)\n", dev->name, queue, priv->rxq_count[queue], 
												EGIGA_Q_DESC(queue));
	    break;
	} 
	else {
	    printk( KERN_ERR "%s Q %d: error in rx-fill\n", dev->name, queue );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_hal_error[queue]++) );
	    break;
	}
    }

    /* if allocation failed and the number of rx buffers in the ring is less than */
    /* half of the ring size, then set a timer to try again later.                */
    if( alloc_skb_failed && (priv->rxq_count[queue] < (EGIGA_Q_DESC(queue)/2)) ) {
        if( priv->rx_fill_flag == 0 ) {
	    printk( KERN_INFO "%s Q %d: set rx timeout to allocate skb\n", dev->name, queue );
	    priv->rx_fill_timer.expires = jiffies + (HZ/10); /*100ms*/
	    add_timer( &priv->rx_fill_timer );
	    priv->rx_fill_flag = 1;
	}
    }

    EGIGA_DBG( EGIGA_DBG_RX_FILL, ("rx fill %d (total %d)", count, priv->rxq_count[queue]) );
    
    return count;
}



/*********************************************************** 
 * egiga_rx_fill_on_timeout --                             *
 *   previous rx fill failed allocate skb. try now again.  *
 ***********************************************************/
static void egiga_rx_fill_on_timeout( unsigned long data ) 
{
    struct net_device *dev = (struct net_device *)data;
    egiga_priv *priv = dev->priv;
    unsigned int queue;

    EGIGA_DBG( EGIGA_DBG_RX_FILL, ("%s: rx_fill_on_timeout", dev->name) );
    EGIGA_STAT( EGIGA_STAT_RX_FILL, (priv->egiga_stat.rx_fill_timeout_events++) );
   
    priv->rx_fill_flag = 0;
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++)
    {    
    	egiga_rx_fill( dev, queue, EGIGA_Q_DESC(queue));
    }
}


/*********************************************************** 
 * egiga_interrupt_handler --                              *
 *   serve rx-q0, tx-done-q0, phy/link state change.       *
 *   tx and phy are served in interrupt context.           *
 *   rx is scheduled out of interrupt context (NAPI poll)  *
 ***********************************************************/
static irqreturn_t egiga_interrupt_handler( int irq , void *dev_id , struct pt_regs *regs )
{
    struct net_device *dev = (struct net_device *)dev_id;
    egiga_priv *priv = dev->priv;
    int port = priv->port;
    u32 picr, picer = 0;
 
    spin_lock( &(priv->lock) );
	
    EGIGA_DBG( EGIGA_DBG_INT, ("\n%s: isr ", dev->name) );
    EGIGA_STAT( EGIGA_STAT_INT, (priv->egiga_stat.int_total++) );

    /* read port interrupt cause register */
    picr = MV_REG_READ( ETH_INTR_CAUSE_REG( port ) );


    EGIGA_DBG( EGIGA_DBG_INT, ("[picr %08x]", picr) );
    if( !picr ) {
        EGIGA_STAT( EGIGA_STAT_INT, (priv->egiga_stat.int_none_events++) );
	spin_unlock( &(priv->lock) );
        return IRQ_NONE;
    }
    MV_REG_WRITE( ETH_INTR_CAUSE_REG(port), ~picr );

    if(picr & BIT1) {
	picer = MV_REG_READ( ETH_INTR_CAUSE_EXT_REG(port) );
	if(picer)
	    MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG(port), ~picer );
    }

     /* PHY status changed event */
    if( picer & (BIT16 | BIT20) ) {
        u16 phy_reg_data;
	EGIGA_STAT( EGIGA_STAT_INT, (priv->egiga_stat.int_phy_events++) );

	/* Check Link status on ethernet port */
	mvEthPhyRegRead( mvBoardPhyAddrGet( port ), ETH_PHY_STATUS_REG ,&phy_reg_data);

	if( !(phy_reg_data & ETH_PHY_STATUS_AN_DONE_MASK) ) { 
            netif_carrier_off( dev );
            netif_stop_queue( dev );
	    egiga_down_internals( dev );
        }
	else
        {
            mvEthPortUp( priv->hal_priv );
            netif_carrier_on( dev );
	    netif_wake_queue( dev );		    
	}

	egiga_print_phy_status( dev );
    } 
    
    /* schedule the first net_device to do the work out of interrupt context (NAPI) */
    if (netif_rx_schedule_prep(dev)) {

	/* mask cause */
        priv->rxmask = 0;
        MV_REG_WRITE( ETH_INTR_MASK_REG(port), 0 );

	/* save rx cause and clear */
        priv->rxcause |= (picr&EGIGA_RXQ_MASK) | ((picr&EGIGA_RXQ_RES_MASK) >> (ETH_CAUSE_RX_ERROR_OFFSET-ETH_CAUSE_RX_READY_OFFSET));
	MV_REG_WRITE( ETH_INTR_CAUSE_REG(port), 0 );

	/* mask tx-event */
        priv->txmask = 0;
        MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(port), 0 );

	/* save tx cause and clear */
        priv->txcause |= picer & EGIGA_TXQ_MASK;
	MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG(port), 0 );
        /* schedule the work (rx+txdone) out of interrupt contxet */
        __netif_rx_schedule(dev);
    }
    else {
        if(netif_running(dev)) {
	    printk("rx interrupt while in polling list\n");
	    printk("rx-cause=0x%08x\n",MV_REG_READ(ETH_INTR_CAUSE_REG(port)));
	    printk("rx-mask =0x%08x\n",MV_REG_READ(ETH_INTR_MASK_REG(port)));
	    printk("tx-cause=0x%08x\n",MV_REG_READ(ETH_INTR_CAUSE_EXT_REG(port)));
	    printk("tx-mask =0x%08x\n",MV_REG_READ(ETH_INTR_MASK_EXT_REG(port)));
	}
    }

    spin_unlock( &(priv->lock) );

    return IRQ_HANDLED;
}



/*********************************************************** 
 * egiga_get_stats --                                      *
 *   return the device statistics.                         *
 *   print private statistics if compile flag set.         *
 ***********************************************************/
static struct net_device_stats* egiga_get_stats( struct net_device *dev )
{
    return &(((egiga_priv *)dev->priv)->stats);
}

/***********************************************************
 * egiga_set_multicast_list --                             *
 *   Add multicast addresses or set promiscuous mode.      *
 *   This function should have been but was not included   *
 *   by Marvell. -bbozarth                                 *
 ***********************************************************/
static void egiga_set_multicast_list(struct net_device *dev) {

     egiga_priv *priv = dev->priv;
     int queue = EGIGA_DEF_RXQ;
     struct dev_mc_list *curr_addr = dev->mc_list;
     int i;

     if (dev->flags & IFF_PROMISC)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 1);
     }
     else if (dev->flags & IFF_ALLMULTI)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 0);
        mvEthSetSpecialMcastTable(priv->port, queue);
        mvEthSetOtherMcastTable(priv->port, queue);
     }
     else if (dev->mc_count)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 0);

        for (i=0; i<dev->mc_count; i++, curr_addr = curr_addr->next)
        {
            if (!curr_addr)
                break;
            mvEthMcastAddrSet(priv->hal_priv, curr_addr->dmi_addr, queue);
        }
     }
     else /* No Mcast addrs, not promisc or all multi - clear tables */
     {
        mvEthRxFilterModeSet(priv->hal_priv, 0);
     }
#ifdef CONFIG_QUARTER_DECK
    {
        int iNumOfVlans = mvUnmGetNumOfVlans();
        int  vid;

        for (vid = 1; vid <= iNumOfVlans; vid++)
        {
            mvEthMacAddrSet(priv->hal_priv, mvBindings[vid]->macAddr, EGIGA_DEF_RXQ);
        }
    }
#ifdef HEADERS
    /* set the promisc bit (the header shift the MAC-header 2 byte ahead) */
    mvEthRxFilterModeSet(priv->hal_priv, 1);
//   MV_REG_WRITE(ETH_PORT_CONFIG_REG(priv->port),ETH_UNICAST_PROMISCUOUS_MODE_MASK);
#endif
#endif

}


/*********************************************************** 
 * egiga_set_mac_addr --                                   *
 *   stop port activity. set new addr in device and hw.    *
 *   restart port activity.                                *
 ***********************************************************/
static int egiga_set_mac_addr_internals(struct net_device *dev, void *addr )
{
    egiga_priv *priv = dev->priv;
    u8* mac = &(((u8*)addr)[2]);  /* skip on first 2B (ether HW addr type) */
    int i;

    /* set new addr in hw */
    if( mvEthMacAddrSet( priv->hal_priv, mac, EGIGA_DEF_RXQ) != MV_OK ) {
        printk( KERN_ERR "%s: ethSetMacAddr failed\n", dev->name );
	return -1;
    }

    /* set addr in the device */ 
    for( i = 0; i < 6; i++ )
        dev->dev_addr[i] = mac[i];

    printk( KERN_NOTICE "%s: mac address changed\n", dev->name );

    return 0;
}
static int egiga_set_mac_addr( struct net_device *dev, void *addr )
{

    if(!netif_running(dev)) {
	if(egiga_set_mac_addr_internals(dev, addr) == -1)
		goto error;
    	return 0;
    }

    if( egiga_stop( dev )){
        printk( KERN_ERR "%s: stop interface failed\n", dev->name );
	goto error;
    }

    if(egiga_set_mac_addr_internals(dev, addr) == -1)
	goto error;


    if(egiga_start( dev )){
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
	goto error;
    } 

    return 0;

 error:
    printk( "%s: set mac addr failed\n", dev->name );
    return -1;
}

/*********************************************************** 
 * egiga_change_mtu --                                     *
 *   stop port activity. release skb from rings. set new   *
 *   mtu in device and hw. restart port activity and       *
 *   and fill rx-buiffers with size according to new mtu.  *
 ***********************************************************/
static int egiga_change_mtu_internals( struct net_device *dev, int mtu )
{
    	egiga_priv *priv = dev->priv;
    	if(mtu < 1498 /* 1518 - 20 */) {
		printk(	"%s: Ilegal MTU value %d, ", dev->name, mtu);
		mtu = 1500;
		printk(" rounding MTU to: %d \n",mtu);
    	}
	else if(mtu > 9676 /* 9700 - 20 and rounding to 8 */) {
		printk(	"%s: Ilegal MTU value %d, ", dev->name, mtu);
		mtu = 9676;
		printk(" rounding MTU to: %d \n",mtu);	
  	}
      
    	if(RX_BUFFER_SIZE( mtu, priv) & ~ETH_RX_BUFFER_MASK) {
		printk(	"%s: Ilegal MTU value %d, ", dev->name, mtu);
		mtu = 8 - (RX_BUFFER_SIZE( mtu, priv) & ~ETH_RX_BUFFER_MASK) + mtu;
		printk(" rounding MTU to: %d \n",mtu);
    	}

    	/* set mtu in device and in hal sw structures */
    	if( mvEthMaxRxSizeSet( priv->hal_priv, RX_BUFFER_SIZE( mtu, priv)) ) {
        	printk( KERN_ERR "%s: ethPortSetMaxBufSize failed\n", dev->name );
		return -1;
    	}
        
    	dev->mtu = mtu;

	return 0;
}

static int egiga_change_mtu( struct net_device *dev, int mtu )
{
    int old_mtu = dev->mtu;

    if(!netif_running(dev)) {
  	if(egiga_change_mtu_internals(dev, mtu) == -1) {
		goto error;
	}
    	printk( KERN_NOTICE "%s: change mtu %d (buffer-size %d) to %d (buffer-size %d)\n",
	    dev->name, old_mtu, RX_BUFFER_SIZE( old_mtu, dev->priv), dev->mtu, RX_BUFFER_SIZE( dev->mtu, dev->priv) );
	return 0;
    }

    if( egiga_stop( dev )){
        printk( KERN_ERR "%s: stop interface failed\n", dev->name );
	goto error;
    }

    if(egiga_change_mtu_internals(dev, mtu) == -1) {
	goto error;
    }

    if(egiga_start( dev )){
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
	goto error;
    } 
    printk( KERN_NOTICE "%s: change mtu %d (buffer-size %d) to %d (buffer-size %d)\n",
	dev->name, old_mtu, RX_BUFFER_SIZE( old_mtu, priv), dev->mtu, RX_BUFFER_SIZE( dev->mtu, priv) );
 
    return 0;

 error:
    printk( "%s: change mtu failed\n", dev->name );
    return -1;
}

#ifdef CONFIG_MV_ETH_HEADER
extern MV_STATUS	mvEthRxHeaderCfg(void* pPortHandle, int shimLen, MV_BOOL isShimEnable);
/*********************************************************** 
 * egiga_change_rx_header --                               *
 *   stop port activity. release skb from rings. Change    *
 *   the Marvel Rx header settings. restart port activity  *
 *   and fill rx-buiffers.				   * 
 ***********************************************************/
int egiga_change_rx_header_internals( int port, int header_len, int rx_header_enable )
{
    struct net_device *dev = get_net_device_by_port_num(port);
    egiga_priv *priv = dev->priv;

    /* change Rx header configuration */
    if(MV_OK != mvEthRxHeaderCfg(priv->hal_priv, header_len, rx_header_enable))
    {
	printk( KERN_ERR "%s: mvEthRxHeaderCfg failed\n", dev->name );
	return -1;
    }

    if(rx_header_enable == 1)
    	priv->rx_header_size = header_len;
    else
	priv->rx_header_size = 0;

    /* Change MRU of port */
    if( egiga_change_mtu_internals( dev, dev->mtu ) == -1) {
	return -1;
    }

    return 0;
}
int egiga_change_rx_header( int port, int header_len, int rx_header_enable )
{
    struct net_device *dev = get_net_device_by_port_num(port);

    if( ((header_len % 8) != 0) || (header_len == 0 ) || (header_len > 64))
    {
        printk("%s: header length #%d is not legal\n", dev->name, header_len);
        goto error;
    }

    if(!netif_running(dev)) {
    	if(egiga_change_rx_header_internals(port, header_len, rx_header_enable) == -1){
		goto error;
    	}
    	return 0;
    }

    if( egiga_stop( dev )){
        printk( KERN_ERR "%s: stop interface failed\n", dev->name );
	goto error;
    }

    /* change Rx header configuration */ 
    if(egiga_change_rx_header_internals(port, header_len, rx_header_enable) == -1){
	goto error;
    }

    if(egiga_start( dev )){
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
	goto error;
    } 
    
    return 0;

 error:
    printk( "%s: change rx_header failed\n", dev->name );
    return -1;

}
#endif

#if defined(CONFIG_MV_ETH_HEADER) || defined(EGIGA_STATISTICS)
/***********************************************************************************
 ***  get device by port number 
 ***********************************************************************************/
static struct net_device* get_net_device_by_port_num(unsigned int port) {
	
    struct net_device *dev = NULL;
    switch(port) {
	case 0:
		dev = __dev_get_by_name("egiga0");
		break;
	case 1:
		dev = __dev_get_by_name("egiga1");
		break;
	case 2:
		dev = __dev_get_by_name("egiga2");
		break;
	default:
		printk("get_net_device_by_port_num: unknown port number.\n");	
    }
    return dev;
}
#endif /* CONFIG_MV_ETH_HEADER */


/*********************************************************** 
 * string helpers for mac address setting                  *
 ***********************************************************/
static void egiga_convert_str_to_mac( char *source , char *dest ) 
{
    dest[0] = (egiga_str_to_hex( source[0] ) << 4) + egiga_str_to_hex( source[1] );
    dest[1] = (egiga_str_to_hex( source[2] ) << 4) + egiga_str_to_hex( source[3] );
    dest[2] = (egiga_str_to_hex( source[4] ) << 4) + egiga_str_to_hex( source[5] );
    dest[3] = (egiga_str_to_hex( source[6] ) << 4) + egiga_str_to_hex( source[7] );
    dest[4] = (egiga_str_to_hex( source[8] ) << 4) + egiga_str_to_hex( source[9] );
    dest[5] = (egiga_str_to_hex( source[10] ) << 4) + egiga_str_to_hex( source[11] );
}
static unsigned int egiga_str_to_hex( char ch ) 
{
    if( (ch >= '0') && (ch <= '9') )
        return( ch - '0' );

    if( (ch >= 'a') && (ch <= 'f') )
	return( ch - 'a' + 10 );

    if( (ch >= 'A') && (ch <= 'F') )
	return( ch - 'A' + 10 );

    return 0;
}


/***********************************************************************************
 ***  print port statistics
 ***********************************************************************************/
#define   STAT_PER_Q(qnum,x) for(queue = 0; queue < qnum; queue++) \
				printk("%10u ",x[queue]); \
      		      	printk("\n");

void print_egiga_stat( unsigned int port )
{
#ifndef EGIGA_STATISTICS
  printk(" Error: egiga is compiled without statistics support!! \n");
  return;
#else
  struct net_device *dev = get_net_device_by_port_num(port);
  egiga_priv *priv = (egiga_priv *)(dev->priv);
  egiga_statistics *stat = &(priv->egiga_stat);
  unsigned int queue;

      printk("QUEUS:.........................");
  for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++) 
      printk( "%10d ",queue);
  printk("\n");

  if( egiga_stat & EGIGA_STAT_INT ) {
      printk( "\n====================================================\n" );
      printk( "%s: interrupt statistics", dev->name );
      printk( "\n-------------------------------\n" );
      printk( "int_total.....................%10u\n", stat->int_total );
      printk( "int_rx_events.................%10u\n", stat->int_rx_events );
      printk( "int_tx_done_events............%10u\n", stat->int_tx_done_events );
      printk( "int_phy_events................%10u\n", stat->int_phy_events );
      printk( "int_none_events...............%10u\n", stat->int_none_events );
  }
  if( egiga_stat & EGIGA_STAT_RX ) {
      printk( "\n====================================================\n" );
      printk( "%s: rx statistics", dev->name );
      printk( "\n-------------------------------\n" );
      printk( "rx_poll_events................%10u\n", stat->rx_poll_events );
      printk( "rx_poll_hal_ok................");STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_ok);
      printk( "rx_poll_hal_no_resource.......");STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_no_resource );
      printk( "rx_poll_hal_no_more..........."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_no_more );
      printk( "rx_poll_hal_error............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_error );
      printk( "rx_poll_hal_invalid_skb......."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_invalid_skb );
      printk( "rx_poll_hal_bad_stat.........."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_hal_bad_stat );
      printk( "rx_poll_netif_drop............"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_poll_netif_drop );
      printk( "rx_poll_netif_complete........%10u\n",stat->rx_poll_netif_complete );
      printk( "Current Rx Cause is...........%10x\n",priv->rxcause);
  }
  if( egiga_stat & EGIGA_STAT_RX_FILL ) {
      printk( "\n====================================================\n" );
      printk( "%s: rx fill statistics", dev->name );
      printk( "\n-------------------------------\n" );
      printk( "rx_fill_events................"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_events );
      printk( "rx_fill_alloc_skb_fail........"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_alloc_skb_fail );
      printk( "rx_fill_hal_ok................"); STAT_PER_Q(MV_ETH_RX_Q_NUM,stat->rx_fill_hal_ok);
      printk( "rx_fill_hal_full.............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_hal_full );
      printk( "rx_fill_hal_error............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_hal_error );
      printk( "rx_fill_timeout_events........%10u\n", stat->rx_fill_timeout_events );
      printk( "rx buffer size................%10u\n",RX_BUFFER_SIZE(dev->mtu, priv));
  }
  if( egiga_stat & EGIGA_STAT_TX ) {
      printk( "\n====================================================\n" );
      printk( "%s: tx statistics", dev->name );
      printk( "\n-------------------------------\n" );
      printk( "tx_events.....................%10u\n", stat->tx_events );
      printk( "tx_hal_ok.....................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_ok);
      printk( "tx_hal_no_resource............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_no_resource );
      printk( "tx_hal_no_error...............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_error );
      printk( "tx_hal_unrecognize............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_unrecognize );
      printk( "tx_netif_stop.................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_netif_stop );
      printk( "tx_timeout....................%10u\n", stat->tx_timeout );
      printk( "Current Tx Cause is...........%10x\n",priv->txcause);
  }
  if( egiga_stat & EGIGA_STAT_TX_DONE ) {
      printk( "\n====================================================\n" );
      printk( "%s: tx-done statistics", dev->name );
      printk( "\n-------------------------------\n" );
      printk( "tx_done_events................%10u\n", stat->tx_done_events );
      printk( "tx_done_hal_ok................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_ok);
      printk( "tx_done_hal_invalid_skb.......");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_invalid_skb );
      printk( "tx_done_hal_bad_stat..........");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_bad_stat );
      printk( "tx_done_hal_still_tx..........");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_still_tx );
      printk( "tx_done_hal_no_more...........");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_no_more );
      printk( "tx_done_hal_unrecognize.......");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_unrecognize );
      printk( "tx_done_max...................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_max );
      printk( "tx_done_min...................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_min );
      printk( "tx_done_netif_wake............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_netif_wake );
  }

  memset( stat, 0, sizeof(egiga_statistics) );
#endif /*EGIGA_STATISTICS*/
}


/***********************************************************************************
 *** IN THE NEW ARCHITECTURE THE FOLLOWING FUNCTIONS SHOULD BE MOVED TO HAL/MCSP ***
 ***   phy_status, rx\tx_coal, get_dram\sram_base, adrress_decode, phy_addr      ***
 ***********************************************************************************/
static void egiga_print_phy_status( struct net_device *dev ) 
{
    egiga_priv *priv = dev->priv;
    int port = priv->port;
    u32 port_status;
    u16 phy_reg_val;
    
    /* check link status on phy */
    mvEthPhyRegRead( mvBoardPhyAddrGet( port ), ETH_PHY_STATUS_REG , &phy_reg_val);
		
    if( !(phy_reg_val & ETH_PHY_STATUS_AN_DONE_MASK) ) {
	printk( KERN_NOTICE "%s: link down\n", dev->name );
    }
    else {
	printk( KERN_NOTICE "%s: link up", dev->name );

        /* check port status register */
	port_status = MV_REG_READ( ETH_PORT_STATUS_REG( port ) );
	printk( KERN_NOTICE ", %s",(port_status & BIT2) ? "full duplex" : "half duplex" );
	if( port_status & BIT4 ) printk( KERN_NOTICE ", speed 1 Gbps" );
	else printk( KERN_NOTICE ", %s",(port_status & BIT5) ? "speed 100 Mbps" : "speed 10 Mbps" );
	printk( KERN_NOTICE "\n" );
    }
}


static int restart_autoneg( int port )
{
    u16 phy_reg_val = 0;

    /* enable auto-negotiation */
    mvEthPhyRegRead(mvBoardPhyAddrGet( port ), ETH_PHY_CTRL_REG, &phy_reg_val);
    phy_reg_val |= BIT12;
    mvEthPhyRegWrite( mvBoardPhyAddrGet( port ), ETH_PHY_CTRL_REG, phy_reg_val );

    mdelay( 10 );

    /* restart auto-negotiation */
    phy_reg_val |= BIT9;
    mvEthPhyRegWrite( mvBoardPhyAddrGet( port ), ETH_PHY_CTRL_REG, phy_reg_val );

    mdelay( 10 );

    return 0;
}

