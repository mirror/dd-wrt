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
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "mvEth.h"
#include "mvEthPhy.h"
#include "msApi.h"
#include "dbg-trace.h"
#ifdef INCLUDE_MULTI_QUEUE
#include "mvEthPolicy.h"
#endif

#ifdef CONFIG_SCM_SUPPORT
#define CONFIG_MAC_MODIFY
#define CONFIG_MV_GTW_QOS_NET_IF "eth1" 
#endif

/* RD2-5181L-FE board/switch ports mapping */
#define FE_QD_PORT_CPU  5
#define FE_QD_PORT_0    0
#define FE_QD_PORT_1    1
#define FE_QD_PORT_2    2
#define FE_QD_PORT_3    3
#define FE_QD_PORT_4    4

/* RD2-5181L-GE board/switch ports mapping */
#define GE_QD_PORT_CPU  3
#define GE_QD_PORT_0    2
#define GE_QD_PORT_1    1
#define GE_QD_PORT_2    0
#define GE_QD_PORT_3    7
#define GE_QD_PORT_4    5

/* run time detection GE/FE */
int QD_PORT_CPU;
int QD_PORT_0;
int QD_PORT_1;
int QD_PORT_2;
int QD_PORT_3;
int QD_PORT_4;

/* Network interfaces configuration */
#define MV_NUM_OF_IFS  2
#define MV_IF0_NAME "eth0"
#define MV_IF1_NAME "eth1"
#define MV_IF2_NAME "eth2"

/* prioritization for incoming traffic (lower number = lower priority) */
#define MV_ARP_PRIO  2	    /* ARP requests only */
#define MV_VOIP_PRIO 3	    /* Marked with ToS 0xA0 */

/* helpers for VLAN tag handling */
#define MV_GTW_PORT_VLAN_ID(grp,port)      ((grp)+(port)+1)
#define MV_GTW_GROUP_VLAN_ID(grp)          (((grp)+1)<<8)
#define MV_GTW_VLANID_TO_GROUP(vlanid) ((((vlanid) & 0xf00) >> 8)-1)
#define MV_GTW_VLANID_TO_PORT(vlanid)  (((vlanid) & 0xf)-1)

#ifdef INCLUDE_MULTI_QUEUE
#define EGIGA_TXQ_MASK	     (BIT0)
#define EGIGA_RXQ_MASK	     (BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2)
#define EGIGA_RXQ_RES_MASK   (BIT18|BIT17|BIT16|BIT15|BIT14|BIT13|BIT12|BIT11)
#else
#define EGIGA_TXQ_MASK	     (BIT0)
#define EGIGA_RXQ_MASK       (BIT2)
#define EGIGA_RXQ_RES_MASK   (BIT11)
#endif

#define EGIGA_PICR_MASK      (BIT1|EGIGA_RXQ_MASK|EGIGA_RXQ_RES_MASK)
#define EGIGA_PICER_MASK     (EGIGA_TXQ_MASK)
#define EGIGA_DEF_PORT	    0
#define QD_MIN_ETH_PACKET_LEN 60
#define QD_VLANTAG_SIZE 4
#define MV_MTU MV_ALIGN_UP(1500 + 2 + 4 + ETH_HLEN + 4, 32)  /* 2(HW hdr) 4(QD extra) 14(MAC hdr) 4(CRC) */

/* Override MAC address with deafults? */
extern unsigned int overEthAddr;

/* number of descriptors per queue */
int gbe_desc_num_per_rxq[MV_ETH_RX_Q_NUM] = {
EGIGA_NUM_OF_RX_DESCR*2,
#ifdef INCLUDE_MULTI_QUEUE
EGIGA_NUM_OF_RX_DESCR,EGIGA_NUM_OF_RX_DESCR,EGIGA_NUM_OF_RX_DESCR,EGIGA_NUM_OF_RX_DESCR,
EGIGA_NUM_OF_RX_DESCR,EGIGA_NUM_OF_RX_DESCR,EGIGA_NUM_OF_RX_DESCR*2,
#endif
};
int gbe_desc_num_per_txq[MV_ETH_TX_Q_NUM] = {
EGIGA_NUM_OF_TX_DESCR*2, 
};

/* debug control */
#define GTW_DEBUG
#undef GTW_DEBUG
#define GTW_DBG_OFF     0x0000
#define GTW_DBG_RX      0x0001
#define GTW_DBG_TX      0x0002
#define GTW_DBG_RX_FILL 0x0004
#define GTW_DBG_TX_DONE 0x0008
#define GTW_DBG_LOAD    0x0010
#define GTW_DBG_IOCTL   0x0020
#define GTW_DBG_INT     0x0040
#define GTW_DBG_STATS   0x0080
#define GTW_DBG_MCAST   0x0100
#define GTW_DBG_VLAN    0x0200
#define GTW_DBG_IGMP    0x0400
#ifdef CONFIG_SCM_SUPPORT
#define GTW_DBG_MACADDR 0x0800
#endif
#define GTW_DBG_ALL     0xffff
#ifdef GTW_DEBUG
# define GTW_DBG(FLG, X) if( (mv_gtw_dbg & (FLG)) == (FLG) ) printk X
#else
# define GTW_DBG(FLG, X)
#endif
unsigned int mv_gtw_dbg = GTW_DBG_ALL;

/* gigabit statistics */
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
#define EGIGA_STAT_POLL    0x0080
#define EGIGA_STAT_ALL     0xffff
#ifdef EGIGA_STATISTICS
# define EGIGA_STAT(FLG, CODE) if( (mv_gtw_stat & (FLG)) == (FLG) ) CODE;
#else
# define EGIGA_STAT(FLG, CODE)
#endif
unsigned int mv_gtw_stat =  EGIGA_STAT_ALL;

typedef struct _egiga_statistics
{
    unsigned int irq_total, irq_none;
    unsigned int poll_events, poll_complete;
    unsigned int rx_events, rx_hal_ok[MV_ETH_RX_Q_NUM],
    rx_hal_no_resource[MV_ETH_RX_Q_NUM],rx_hal_no_more[MV_ETH_RX_Q_NUM],
    rx_hal_error[MV_ETH_RX_Q_NUM],rx_hal_invalid_skb[MV_ETH_RX_Q_NUM],
    rx_hal_bad_stat[MV_ETH_RX_Q_NUM],rx_netif_drop[MV_ETH_RX_Q_NUM];
    unsigned int tx_done_events,tx_done_hal_ok[MV_ETH_TX_Q_NUM],
    tx_done_hal_invalid_skb[MV_ETH_TX_Q_NUM],tx_done_hal_bad_stat[MV_ETH_TX_Q_NUM],
    tx_done_hal_still_tx[MV_ETH_TX_Q_NUM],tx_done_hal_no_more[MV_ETH_TX_Q_NUM],
    tx_done_hal_unrecognize[MV_ETH_TX_Q_NUM],tx_done_max[MV_ETH_TX_Q_NUM],
    tx_done_min[MV_ETH_TX_Q_NUM],tx_done_netif_wake[MV_ETH_TX_Q_NUM];
    unsigned int rx_fill_events[MV_ETH_RX_Q_NUM],rx_fill_alloc_skb_fail[MV_ETH_RX_Q_NUM],
    rx_fill_hal_ok[MV_ETH_RX_Q_NUM],rx_fill_hal_full[MV_ETH_RX_Q_NUM],
    rx_fill_hal_error[MV_ETH_RX_Q_NUM],rx_fill_timeout_events;
    unsigned int tx_events,tx_hal_ok[MV_ETH_TX_Q_NUM],tx_hal_no_resource[MV_ETH_TX_Q_NUM],
    tx_hal_error[MV_ETH_TX_Q_NUM],tx_hal_unrecognize[MV_ETH_TX_Q_NUM],tx_netif_stop[MV_ETH_TX_Q_NUM],
    tx_timeout;
} egiga_statistics;

typedef struct _egiga_priv
{
    void* hal_priv;
    void* rx_policy_priv;
    unsigned int rxq_count[MV_ETH_RX_Q_NUM], txq_count[MV_ETH_TX_Q_NUM];
    MV_BUF_INFO tx_buf_info_arr[MAX_SKB_FRAGS+3];
    MV_PKT_INFO tx_pkt_info;
#ifdef EGIGA_STATISTICS
    egiga_statistics egiga_stat;
#endif
    unsigned int rx_coal, tx_coal, rxcause, txcause, rxmask, txmask;
    spinlock_t lock;
    struct timer_list rx_fill_timer;
    unsigned rx_fill_flag;
} egiga_priv; 

struct mv_vlan_cfg {
    char name[IFNAMSIZ];
    unsigned int ports_mask;
    unsigned short vlan_grp_id;
    MV_8 macaddr[6];
};

typedef struct _mv_priv {
    struct net_device *net_dev;			/* back reference to the net_device */
    struct mv_vlan_cfg *vlan_cfg;		/* reference to entry in net config table */
    struct net_device_stats net_dev_stat;	/* statistic counters */
}mv_priv;

/* globals variables */
static mv_priv mv_privs[MV_NUM_OF_IFS];
static egiga_priv gbe_dev;
GT_QD_DEV qddev, *qd_dev = NULL;
static GT_SYS_CONFIG qd_cfg;
static struct net_device *main_net_dev = NULL;
static struct net_device *mv_netdev_stopped = NULL;
struct mv_vlan_cfg *net_cfg = NULL;
static unsigned char zero_pad[QD_MIN_ETH_PACKET_LEN] = {0};
extern unsigned int overEthAddr;

#if 0
struct mv_vlan_cfg ge_net_cfg[MV_NUM_OF_IFS] = { /* RD2-5181L-GE */
/*   name   |            switch ports belong to this subnet         |group vlan id + MAC addr|
 *          |                                                       |   (set in run time)   */
{MV_IF0_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_0)                  ,-1,{0}},
{MV_IF1_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_1)|(1<<GE_QD_PORT_2),-1,{0}},
{MV_IF2_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_3)|(1<<GE_QD_PORT_4),-1,{0}}};

struct mv_vlan_cfg fe_net_cfg[MV_NUM_OF_IFS] = { /* RD2-5181L-FE */
/*   name   |            switch ports belong to this subnet         |group vlan id + MAC addr|
 *          |                                                       |   (set in run time)   */
{MV_IF0_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_0)                  ,-1,{0}},
{MV_IF1_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_1)|(1<<FE_QD_PORT_2),-1,{0}},
{MV_IF2_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_3)|(1<<FE_QD_PORT_4),-1,{0}}};
#endif /* 0 */
#if 0
/* Two interfaces configuration (need also to change above #define MV_NUM_OF_IFS 2) */
struct mv_vlan_cfg ge_net_cfg[MV_NUM_OF_IFS] = { /* RD2-5181L-GE */
/*   name   |            switch ports belong to this subnet         |group vlan id + MAC addr|
 *          |                                                       |   (set in run time)   */
{MV_IF0_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_0)                  ,-1,{0}},
{MV_IF1_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_1)|(1<<GE_QD_PORT_2)|(1<<GE_QD_PORT_3)|(1<<GE_QD_PORT_4),-1,{0}}};
struct mv_vlan_cfg fe_net_cfg[MV_NUM_OF_IFS] = { /* RD2-5181L-FE */
/*   name   |            switch ports belong to this subnet         |group vlan id + MAC addr|
 *          |                                                       |   (set in run time)   */
{MV_IF0_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_0)                  ,-1,{0}},
{MV_IF1_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_1)|(1<<FE_QD_PORT_2)|(1<<FE_QD_PORT_3)|(1<<FE_QD_PORT_4),-1,{0}}};
#endif
/*
 * Two interfaces configuration
 *
 * LAN - eth0 - port 1-4  - switch port 0 1 5 7
 * WAN - eth1 - port 0    - switch port 2
 * 
 * (need also to change above #define MV_NUM_OF_IFS 2)
 */
struct mv_vlan_cfg ge_net_cfg[MV_NUM_OF_IFS] = {
    {MV_IF0_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_1)|(1<<GE_QD_PORT_2)|(1<<GE_QD_PORT_3)|(1<<GE_QD_PORT_4),-1,{0}},    
    {MV_IF1_NAME,(1<<GE_QD_PORT_CPU)|(1<<GE_QD_PORT_0),-1,{0}}
};
struct mv_vlan_cfg fe_net_cfg[MV_NUM_OF_IFS] = {
    {MV_IF0_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_1)|(1<<FE_QD_PORT_2)|(1<<FE_QD_PORT_3)|(1<<FE_QD_PORT_4),-1,{0}},
    {MV_IF1_NAME,(1<<FE_QD_PORT_CPU)|(1<<FE_QD_PORT_0),-1,{0}}
};

/* functions prototype */
static int __init mv_gtw_init_module( void );
static void __init mv_gtw_exit_module( void );
static int init_switch(void);
static int init_gigabit(void);
static int mv_gtw_start( struct net_device *dev );
static int mv_gtw_stop( struct net_device *dev );
static int mv_gtw_tx( struct sk_buff *skb, struct net_device *dev );
static unsigned int mv_gtw_tx_done(void);
static unsigned int mv_gtw_rx(unsigned int work_to_do);
static unsigned int mv_gtw_rx_fill(unsigned int queue, int count);
static void mv_gtw_tx_timeout( struct net_device *dev );
static void mv_gtw_rx_fill_on_timeout(unsigned long data);
static int mv_gtw_poll( struct net_device *dev, int *budget );
static irqreturn_t mv_gtw_interrupt_handler( int rq , void *dev_id );
GT_BOOL ReadMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int* value);
GT_BOOL WriteMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int data);
static int mv_gtw_set_port_based_vlan(unsigned int ports_mask);
static int mv_gtw_set_vlan_in_vtu(unsigned short vlan_id,unsigned int ports_mask);
static void mv_gtw_set_multicast_list(struct net_device *dev);
static struct net_device_stats* mv_gtw_get_stats( struct net_device *dev );
static struct net_device* mv_gtw_main_net_dev_get(void);
void print_mv_gtw_stat(void);
static int init_config(void);
void print_qd_port_counters(unsigned int port);
void print_qd_atu(void);
static void mv_gtw_convert_str_to_mac(char *source , char *dest);
static unsigned char mv_gtw_convert_str_to_tos(char *str);
static unsigned int mv_gtw_str_to_hex(char ch);
#ifdef CONFIG_MV_GTW_QOS
int mv_gtw_qos_tos_quota = -1;
extern int MAX_SOFTIRQ_RESTART;
int mv_gtw_qos_tos_enable(void);
int mv_gtw_qos_tos_disable(void);
EXPORT_SYMBOL(mv_gtw_qos_tos_enable);
EXPORT_SYMBOL(mv_gtw_qos_tos_disable);
#endif
#ifdef CONFIG_MV_GTW_IGMP
int mv_gtw_enable_igmp(void);
int mv_gtw_set_multicast_atu_entry(unsigned char *mac_addr, unsigned char db, unsigned int ports_mask);
EXPORT_SYMBOL(mv_gtw_enable_igmp);
EXPORT_SYMBOL(mv_gtw_set_multicast_atu_entry);
#endif

module_init(mv_gtw_init_module);
module_exit(mv_gtw_exit_module);
MODULE_DESCRIPTION("Marvell Gateway Driver - www.marvell.com");
MODULE_AUTHOR("Tzachi Perelstein <tzachi@marvell.com>");
MODULE_LICENSE("GPL");

#ifdef CONFIG_SCM_SUPPORT

#ifdef CONFIG_MAC_MODIFY
static int mv_gtw_set_mac_addr( struct net_device *dev, void *p );
static int mv_gtw_set_mac_addr_internal(struct net_device *dev, int op);

static int mv_gtw_set_mac_addr( struct net_device *dev, void *p )
{
    struct sockaddr *addr = p;
    if(!is_valid_ether_addr(addr->sa_data))
	return -EADDRNOTAVAIL;

    /* remove old mac addr from VLAN DB */
    if(mv_gtw_set_mac_addr_internal(dev, 0))
	return -1;

    memcpy(dev->dev_addr, addr->sa_data, 6);

    /* add new mac addr to VLAN DB */
    if(mv_gtw_set_mac_addr_internal(dev, 1))
	return -1;

    /* In order to coexist with QoS filtering at gigabit level, we
     * must have a match between incomming packet DA and the mac addr
     * (bits 47:4) set in gigabit level. Since different interfaces
     * has different MAC addresses, only the WAN interface benefits
     * from QoS.
     */

#ifdef CONFIG_SCM_SUPPORT
    if(strcmp(dev->name,CONFIG_MV_GTW_QOS_NET_IF)==0)
#endif
    mvEthMacAddrSet(gbe_dev.hal_priv, dev->dev_addr, EGIGA_DEF_RXQ);
    
    return 0;
}

static int mv_gtw_set_mac_addr_internal(struct net_device *dev, int op)
{
    mv_priv *mvp = (mv_priv *)dev->priv;
    struct mv_vlan_cfg *vlan_cfg = mvp->vlan_cfg;
    GT_ATU_ENTRY mac_entry;

    mac_entry.trunkMember = GT_FALSE;
    mac_entry.prio = 0;
    mac_entry.exPrio.useMacFPri = GT_FALSE;
    mac_entry.exPrio.macFPri = 0;
    mac_entry.exPrio.macQPri = 0;
    mac_entry.DBNum = MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id);
    mac_entry.portVec = (1<<QD_PORT_CPU);
    mac_entry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC_NRL;
    memcpy(mac_entry.macAddr.arEther,dev->dev_addr,6);

    GTW_DBG(GTW_DBG_MACADDR, ("mv_gateway: %s db%d port-mask=0x%x, %02x:%02x:%02x:%02x:%02x:%02x ",
        vlan_cfg->name, mac_entry.DBNum, (unsigned int)mac_entry.portVec,
        mac_entry.macAddr.arEther[0],mac_entry.macAddr.arEther[1],mac_entry.macAddr.arEther[2],
        mac_entry.macAddr.arEther[3],mac_entry.macAddr.arEther[4],mac_entry.macAddr.arEther[5]));

    if(op) { 

        if(gfdbAddMacEntry(qd_dev, &mac_entry) != GT_OK) {
	   	printk("gfdbAddMacEntry failed\n");
	    return -1;
        }
	GTW_DBG(GTW_DBG_MACADDR, ("added\n"));
    }
    else {
        if(gfdbDelAtuEntry(qd_dev, &mac_entry) != GT_OK) {
	   	printk("gfdbDelAtuEntry failed\n");
	    return -1;
        }
	GTW_DBG(GTW_DBG_MACADDR, ("deleted\n"));
    }
    return 0;
}

#endif /* CONFIG_MAC_MODIFY */


#define SIOCSPORTPRI    (SIOCDEVPRIVATE+1)
#define SIOCGLINKSTATUS (SIOCDEVPRIVATE+2)
#define SIOCGLINKSPEED  (SIOCDEVPRIVATE+3)

/*********************************************************************
 * mv_gtw_ioctl
 *
 * DESCRIPTION:
 *
 *   ioctl API for following functions:
 *
 *       - set port based QOS priority
 *       
 *           pri  : 1 ~ 4
 *
 *       - get link status,1 means UP,0 means unlink
 *
 *       - get link speed mode,1 means 100M/s,0 means else.
 *
 * PARAMETERS:
 *
 *   stats[2] - stats[0] is input value,indicate which port we should
 *              modify
 *
 *                    WAN LAN0,LAN1,LAN2,LAN3
 *              port : 4    0    1    2    3
 *
 *              stats[1] is output value
 *
 * RETURNS:
 *   
 *
 ********************************************************************/
static int
mv_gtw_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
    unsigned int stats[2];
    GT_LPORT port = 1;

    /* stats[0] indicate the port number */
    if(copy_from_user(stats, ifr->ifr_data, sizeof(stats)))
        return -EFAULT;

    switch(stats[0]) {
        case 0:
            port = GE_QD_PORT_1;
            break;
        case 1:
            port = GE_QD_PORT_2;
            break;                    
        case 2:
            port = GE_QD_PORT_4;
            break;
        case 3:
            port = GE_QD_PORT_3;
            break;
        case 4:
        default: /* wan port */
            port = GE_QD_PORT_0;
    }
            
    switch (cmd) {
        case SIOCSPORTPRI:

            gcosSetPortDefaultTc(&qddev,port,stats[1]);
            
            break;

        case SIOCGLINKSTATUS:
            /* reference to Board/QD-DSDT_2.5/src/msapi/gtPortStatus.c */
            
            gprtGetLinkState(&qddev,port,&stats[1]);
            
            if(copy_to_user(ifr->ifr_data, stats, sizeof(stats)))
                return -EFAULT;

            break;
            
        case SIOCGLINKSPEED:

            gprtGetSpeedMode(&qddev,port,&stats[1]);
            if(copy_to_user(ifr->ifr_data, stats, sizeof(stats)))
                return -EFAULT;            
            
            break;
            
        default:
            
            return -EOPNOTSUPP;
	}

    return 0;
}

#endif

static int __init mv_gtw_init_module( void ) 
{
    unsigned int i;
    struct net_device *dev;
    struct mv_vlan_cfg *nc;

    printk("Loading Marvell Gatway Driver:\n");
    
    if(init_config()) {
	printk("failed to init config\n");
	return -ENODEV;
    }

    if(init_switch()) {
	printk("failed to init switch\n");
	return -ENODEV;
    }

    if(init_gigabit()) {
	printk("failed to init gigabit ethernet\n");
	return -ENODEV;
    }
    
    /* load net_devices */
    printk("loading network interfaces: ");
    for(i=0, nc=net_cfg; i<MV_NUM_OF_IFS; i++,nc++) {
        dev = alloc_netdev(0, nc->name, ether_setup);
        mv_privs[i].net_dev = dev;	/* back reference to the net_device */
        mv_privs[i].vlan_cfg = nc;	/* reference to entry in net config table */
        memset(&(mv_privs[i].net_dev_stat), 0, sizeof(struct net_device_stats));/* statistic counters */
        dev->priv = &mv_privs[i];
        dev->irq = ETH_PORT0_IRQ_NUM;
        memcpy(dev->dev_addr,nc->macaddr,6);
        dev->open = mv_gtw_start;
        dev->stop = mv_gtw_stop;
        dev->hard_start_xmit = mv_gtw_tx;
        dev->tx_timeout = mv_gtw_tx_timeout;
        dev->watchdog_timeo = 5*HZ;
        dev->tx_queue_len = gbe_desc_num_per_txq[EGIGA_DEF_TXQ];
        dev->set_multicast_list = mv_gtw_set_multicast_list;
        dev->poll = &mv_gtw_poll;
        dev->weight = 64;
        dev->get_stats = mv_gtw_get_stats;
#ifdef CONFIG_SCM_SUPPORT
#ifdef CONFIG_MAC_MODIFY        
        dev->set_mac_address = mv_gtw_set_mac_addr;
#endif        
        dev->do_ioctl = mv_gtw_ioctl;
#endif        
        if(register_netdev(dev)) {
            printk(KERN_ERR "failed to register %s\n",dev->name);
        }
        else {
            printk("%s ",dev->name);
    	}
    }
    main_net_dev = NULL;
    printk("\n");

    /* connect to port interrupt line */
    if( request_irq(dev->irq, mv_gtw_interrupt_handler, (SA_INTERRUPT | SA_SAMPLE_RANDOM), "mv_gateway", NULL) ) {
        printk(KERN_ERR "failed to assign irq%d\n", dev->irq);
        dev->irq = 0;
    }

    /* unmask interrupts */
    MV_REG_WRITE( ETH_INTR_MASK_REG(EGIGA_DEF_PORT), EGIGA_PICR_MASK );
    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(EGIGA_DEF_PORT), EGIGA_PICER_MASK );
    gbe_dev.rxmask = EGIGA_PICR_MASK;
    gbe_dev.txmask = EGIGA_PICER_MASK;

    return 0;
}

static void __init mv_gtw_exit_module(void) 
{
    printk("Removing Marvell Gatway Driver: not implemented\n");
    /* 2do: currently this module is statically linked */
}

static struct net_device* mv_gtw_main_net_dev_get(void)
{
    int i;

    for(i=0; i<MV_NUM_OF_IFS; i++) {
        if(netif_running(mv_privs[i].net_dev)) {
            return mv_privs[i].net_dev;
	}
    }
    return NULL;
}

static int mv_gtw_start( struct net_device *dev ) 
{
    mv_priv *mvp = (mv_priv *)dev->priv;
    struct mv_vlan_cfg *vlan_cfg = mvp->vlan_cfg;
    int p;

    printk("mv_gateway: starting %s\n",dev->name);

    /* start upper layer */
    netif_carrier_on( dev );
    netif_wake_queue( dev );
    netif_poll_enable(dev);

#ifdef CONFIG_SCM_SUPPORT
#ifdef CONFIG_MAC_MODIFY    
    if(mv_gtw_set_mac_addr_internal(dev, 1))
        return -1;
#endif    
#endif
    
    /*
     * adding our mac addr eanables the gigabit port to recieve
     * packets for this net_device
     */
#ifdef CONFIG_SCM_SUPPORT
    if(strcmp(dev->name,CONFIG_MV_GTW_QOS_NET_IF)==0)
#endif        
    mvEthMacAddrSet( gbe_dev.hal_priv, dev->dev_addr, EGIGA_DEF_RXQ);
    
    /* connect switch CPU port to the port's VLAN */
    for(p=0; p<qd_dev->numOfPorts; p++) {
        if(MV_BIT_CHECK(vlan_cfg->ports_mask, p) && (p!=QD_PORT_CPU)) {
	    if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(vlan_cfg->vlan_grp_id,p),vlan_cfg->ports_mask) != 0) {
		printk("mv_gtw_set_vlan_in_vtu failed\n");
	    }
	}
    }

    /* add the CPU port to the port based list */
    mv_gtw_set_port_based_vlan(vlan_cfg->ports_mask);
    
    main_net_dev = mv_gtw_main_net_dev_get();

    return 0;
}

static int mv_gtw_stop( struct net_device *dev )
{
    mv_priv *mvp = (mv_priv *)dev->priv;
    struct mv_vlan_cfg *vlan_cfg = mvp->vlan_cfg;
    int p;

    printk("mv_gateway: stoping %s\n",dev->name);

    /* stop upper layer */
    netif_poll_disable(dev);
    netif_carrier_off(dev);
    netif_stop_queue(dev);

#ifdef CONFIG_SCM_SUPPORT
#ifdef CONFIG_MAC_MODIFY    
    /*
     * Delete our mac addr from CPU port in VALN DB at switch level
     * (GbE in unicast promisc mode + switch CPU port drops unknowns)
     */
    mv_gtw_set_mac_addr_internal(dev, 0);
#endif    
#endif
    
    /* deleting our mac addr disables the gigabit from recieving packets for this net_device */
#ifdef CONFIG_SCM_SUPPORT
    if(strcmp(dev->name,CONFIG_MV_GTW_QOS_NET_IF)==0)
#endif    
    mvEthMacAddrSet( gbe_dev.hal_priv, dev->dev_addr, -1);

    /* disconnect switch CPU port from the port's VLAN (allow bridging between other ports) */
    for(p=0; p<qd_dev->numOfPorts; p++) {
        if(MV_BIT_CHECK(vlan_cfg->ports_mask, p) && (p!=QD_PORT_CPU)) {
	    /* disconnect the CPU port from the group */
	    if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(vlan_cfg->vlan_grp_id,p),vlan_cfg->ports_mask & ~(1<<QD_PORT_CPU)) != 0) {
		printk("mv_gtw_set_vlan_in_vtu failed\n");
	    }
	}
    }

    /* remove the CPU port from the list */
    mv_gtw_set_port_based_vlan(vlan_cfg->ports_mask & ~(1<<QD_PORT_CPU) );

    main_net_dev = mv_gtw_main_net_dev_get();
    return 0;
}

static int init_config(void)
{
    struct mv_vlan_cfg *nc;
    int i;

#if defined(TX_CSUM_OFFLOAD) && defined(RX_CSUM_OFFLOAD)
    printk("checksum offload enabled\n");
#else
#if defined(RX_CSUM_OFFLOAD)
    printk("receive checksum offload enabled\n");
#endif
#if defined(TX_CSUM_OFFLOAD)
    printk("transmit checksum offload enabled\n");
#endif
#endif

#ifdef INCLUDE_MULTI_QUEUE
    printk("multi queue enabled\n");
#endif

#ifdef CONFIG_MV_GTW_QOS
    printk("prioritizing ToS %s\n", CONFIG_MV_GTW_QOS_TOS);
#endif

#ifdef EGIGA_STATISTICS
    printk("statistics enabled\n");
#endif

#ifdef MV_GTW_DEBUG
    printk("debug messages enabled\n");
#endif

#ifdef CONFIG_MV_GTW_PROC
    printk("proc tool enabled\n");
#endif

    /* switch ports on board mapping */
    if(mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
	net_cfg = ge_net_cfg;
        QD_PORT_CPU = GE_QD_PORT_CPU;
        QD_PORT_0   = GE_QD_PORT_0;
        QD_PORT_1   = GE_QD_PORT_1;
        QD_PORT_2   = GE_QD_PORT_2;
        QD_PORT_3   = GE_QD_PORT_3;
        QD_PORT_4   = GE_QD_PORT_4;
    }
    else if(mvBoardIdGet() == RD_88F5181L_VOIP_FE){
	net_cfg = fe_net_cfg;
        QD_PORT_CPU = FE_QD_PORT_CPU;
        QD_PORT_0   = FE_QD_PORT_0;
        QD_PORT_1   = FE_QD_PORT_1;
        QD_PORT_2   = FE_QD_PORT_2;
        QD_PORT_3   = FE_QD_PORT_3;
        QD_PORT_4   = FE_QD_PORT_4;
    }
    else return -1;

    /* build the net config table */
    for(i=0, nc=net_cfg; i<MV_NUM_OF_IFS; i++,nc++) {
	sprintf(nc->name,"eth%d", i);
	nc->vlan_grp_id = MV_GTW_GROUP_VLAN_ID(i);
    
	if(!overEthAddr)
	    mvEthMacAddrGet(EGIGA_DEF_PORT,nc->macaddr);
	else
	    mv_gtw_convert_str_to_mac(CONFIG_MV_GTW_BASE_MACADDR,nc->macaddr);

#ifdef CONFIG_SCM_SUPPORT
    nc->macaddr[5] += (i & 0x7);
#else    
	nc->macaddr[5] = (nc->macaddr[5] & ~0x7) | (i & 0x7);
#endif    
	printk("%s: %02x:%02x:%02x:%02x:%02x:%02x, group-id 0x%03x, group-members are ",
	nc->name,nc->macaddr[0],nc->macaddr[1],nc->macaddr[2],nc->macaddr[3],nc->macaddr[4],nc->macaddr[5],nc->vlan_grp_id);
	if(nc->ports_mask & (1<<QD_PORT_CPU))
	    printk("port-CPU ");
	if(nc->ports_mask & (1<<QD_PORT_0))
	    printk("port-0 ");
	if(nc->ports_mask & (1<<QD_PORT_1))
	    printk("port-1 ");
	if(nc->ports_mask & (1<<QD_PORT_2))
	    printk("port-2 ");
	if(nc->ports_mask & (1<<QD_PORT_3))
	    printk("port-3 ");
	if(nc->ports_mask & (1<<QD_PORT_4))
	    printk("port-4 ");
	printk("\n");
    }

    return 0;
}
static int init_switch(void)
{
    unsigned int i, p;
    unsigned char cnt;
    GT_LPORT port_list[MAX_SWITCH_PORTS];
    struct mv_vlan_cfg *nc;

    printk("init switch layer... ");

    memset((char*)&qd_cfg,0,sizeof(GT_SYS_CONFIG));

    /* init config structure for qd package */
    qd_cfg.BSPFunctions.readMii   = ReadMiiWrap;
    qd_cfg.BSPFunctions.writeMii  = WriteMiiWrap;
    qd_cfg.BSPFunctions.semCreate = NULL;
    qd_cfg.BSPFunctions.semDelete = NULL;
    qd_cfg.BSPFunctions.semTake   = NULL;
    qd_cfg.BSPFunctions.semGive   = NULL;
    qd_cfg.initPorts = GT_TRUE;
    qd_cfg.cpuPortNum = QD_PORT_CPU;
    if(mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
        qd_cfg.mode.scanMode = SMI_MANUAL_MODE;
    }

    /* load switch sw package */
    if( qdLoadDriver(&qd_cfg, &qddev) != GT_OK) {
	printk("qdLoadDriver failed\n");
        return -1;
    }
    qd_dev = &qddev;

    GTW_DBG( GTW_DBG_LOAD, ("qd_dev->numOfPorts=%d\n",qd_dev->numOfPorts));

    /* disable all ports */
    for(p=0; p<qd_dev->numOfPorts; p++) {
	gstpSetPortState(qd_dev, p, GT_PORT_DISABLE);
    }

    /* special settings for GE and FE */
    if(mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
#if 0 //def CONFIG_SCM_SUPPORT /* has been set in bootloader */
        /*
         * Configure registers for LED display:
         * 
         *   10/100 - green
         *   1000   - orange
         * 
         * 1. Move register page to page 3
         * 
         * 2. Set STATUS[0:1] to LED mode 3
         * 
         * 3. 100% mix mode,Status[1]:Solid Mix(100%),Status[0]:Solid
         *    Mix(100%),Status[1:0] Mix On=Low,Off=High
         * 
         */
        if(gsysSetPPUEn(qd_dev, GT_FALSE) != GT_OK) {
            printk("gsysSetPPUEn failed\n");
            return -1;
        }

        /* internal PHYs */
        mvEthPhyRegWrite(QD_PORT_0, 22 ,0x0003);
        mvEthPhyRegWrite(QD_PORT_0, 16 ,0x02ee);
        mvEthPhyRegWrite(QD_PORT_0, 17 ,0x8000);

        mvEthPhyRegWrite(QD_PORT_1, 22 ,0x0003);
        mvEthPhyRegWrite(QD_PORT_1, 16 ,0x02ee);
        mvEthPhyRegWrite(QD_PORT_1, 17 ,0x8000);

        mvEthPhyRegWrite(QD_PORT_2, 22 ,0x0003);
        mvEthPhyRegWrite(QD_PORT_2, 16 ,0x02ee);
        mvEthPhyRegWrite(QD_PORT_2, 17 ,0x8000);  

        /* external PHYs - connect to 88E1112 */
        mvEthPhyRegWrite(QD_PORT_3, 22 ,0x0003);
        mvEthPhyRegWrite(QD_PORT_3, 16 ,0x02ee);
        mvEthPhyRegWrite(QD_PORT_3, 17 ,0x8050);

        mvEthPhyRegWrite(QD_PORT_4, 22 ,0x0003);
        mvEthPhyRegWrite(QD_PORT_4, 16 ,0x02ee);
        mvEthPhyRegWrite(QD_PORT_4, 17 ,0x8050);
#endif        
        
        
	/* enable external ports */
	GTW_DBG( GTW_DBG_LOAD, ("enable phy polling for external ports\n"));
	if(gsysSetPPUEn(qd_dev, GT_TRUE) != GT_OK) {
	    printk("gsysSetPPUEn failed\n");
	    return -1;
	}
	/* set cpu-port with ingress double-tag mode */
	GTW_DBG( GTW_DBG_LOAD, ("cpu port ingress double-tag mode\n"));
	if(gprtSetDoubleTag(qd_dev, QD_PORT_CPU, GT_TRUE) != GT_OK) {
	    printk("gprtSetDoubleTag failed\n");
            return -1;
        }
        /* config the switch to use the double tag data (relevant to cpu-port only) */
	GTW_DBG( GTW_DBG_LOAD, ("use double-tag and remove\n"));
        if(gsysSetUseDoubleTagData(qd_dev,GT_TRUE) != GT_OK) {
            printk("gsysSetUseDoubleTagData failed\n");
            return -1;
        }
    }
    else if (mvBoardIdGet() == RD_88F5181L_VOIP_FE) {
	/* set CPU port number */
        if(gsysSetCPUPort(qd_dev, QD_PORT_CPU) != GT_OK) {
	    printk("gsysSetCPUPort failed\n");
	    return -1;
	}
        /* Set VLAN Tag ID = 0x8100 */
        if(gprtSetProviderTag(qd_dev, QD_PORT_CPU, 0x8100) != GT_OK) {
            printk("gprtSetProviderTag failed\n");
            return -1;
        }
        /* init counters */
	if(gprtClearAllCtr(qd_dev) != GT_OK)
	    printk("gprtClearAllCtr failed\n"); 
        if(gprtSetCtrMode(qd_dev, GT_CTR_ALL) != GT_OK)
	    printk("gprtSetCtrMode failed\n"); 
    }

    if(gstatsFlushAll(qd_dev) != GT_OK) {
	printk("gstatsFlushAll failed\n");
    }
    
    /* set priorities rules */
    for(i=0; i<qd_dev->numOfPorts; i++) {
        /* default port priority to queue zero */
	if(gcosSetPortDefaultTc(qd_dev, i, 0) != GT_OK)
	    printk("gcosSetPortDefaultTc failed (port %d)\n", i);
        /* enable IP TOS Prio */
	if(gqosIpPrioMapEn(qd_dev, i, GT_TRUE) != GT_OK)
	    printk("gqosIpPrioMapEn failed (port %d)\n",i); 
	/* set IP QoS */
	if(gqosSetPrioMapRule(qd_dev, i, GT_FALSE) != GT_OK)
	    printk("gqosSetPrioMapRule failed (port %d)\n",i); 
        /* disable Vlan QoS Prio */
	if(gqosUserPrioMapEn(qd_dev, i, GT_FALSE) != GT_OK)
	    printk("gqosUserPrioMapEn failed (port %d)\n",i); 
        /* Force flow control for all ports */
	if(gprtSetForceFc(qd_dev, i, GT_FALSE) != GT_OK)
	    printk("gprtSetForceFc failed (port %d)\n",i); 
    }

#ifdef CONFIG_SCM_SUPPORT
{
    int status;

    /* Enable QoS,set all port default priority to 3 */
    for(p=0; p<qd_dev->numOfPorts; p++) {
        if(gqosUserPrioMapEn(qd_dev, p, GT_TRUE) != GT_OK)
            printk("gqosUserPrioMapEn failed (port %d)\n",p);

		/*
		 *  Do not use IPv4/IPv6 priority fields (use IP)
		 */
		if((status = gqosIpPrioMapEn(qd_dev,p,GT_FALSE)) != GT_OK)
		{
            printk("gqosIpPrioMapEn failed (port %d)\n",p);
		}

		/*
		 *  IEEE Tag has higher priority than IP priority fields
		 */
		if((status = gqosSetPrioMapRule(qd_dev,p,GT_TRUE)) != GT_OK)
		{
            printk("gqosSetPrioMapRule failed (port %d)\n",p);
		}
        
        if(gcosSetPortDefaultTc(qd_dev, i, 3) != GT_OK)
        {
            printk("gcosSetPortDefaultTc failed (port %d)\n", i);
        }
    }    
}
#endif
    
    /* clear the zero pad buffer */
    memset(zero_pad,0,QD_MIN_ETH_PACKET_LEN);

    /* configure ports (excluding CPU port) for each network interface (VLAN): */
    for(i=0, nc=net_cfg; i<MV_NUM_OF_IFS; i++,nc++) {
	GTW_DBG( GTW_DBG_LOAD, ("%s configuration\n", nc->name));

	/* set port's defaul private vlan id and database number (DB per group): */
	for(p=0; p<qd_dev->numOfPorts; p++) {
	    if( MV_BIT_CHECK(nc->ports_mask, p) && (p != QD_PORT_CPU) ) {
		GTW_DBG( GTW_DBG_LOAD, ("port %d default private vlan id: 0x%x\n", p, MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p)));
		if( gvlnSetPortVid(qd_dev, p, MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p)) != GT_OK ) {
			printk("gvlnSetPortVid failed"); 
			return -1;
		}
		if( gvlnSetPortVlanDBNum(qd_dev, p, MV_GTW_VLANID_TO_GROUP(nc->vlan_grp_id)) != GT_OK) {
		    printk("gvlnSetPortVlanDBNum failed\n");
		    return -1;
		}
	    }
	}

	/* set port's port-based vlan. CPU port join the group later in gtw_start. */
        if(mv_gtw_set_port_based_vlan(nc->ports_mask & ~(1<<QD_PORT_CPU)) != 0) {
	    printk("mv_gtw_set_port_based_vlan failed\n");
	}

        /* set vtu with group vlan id (used in tx) */
        if(mv_gtw_set_vlan_in_vtu(nc->vlan_grp_id,nc->ports_mask) != 0) {
	    printk("mv_gtw_set_vlan_in_vtu failed\n");
	}

        /* set vtu with each port private vlan id (used in rx) */
 	for(p=0; p<qd_dev->numOfPorts; p++) {
	    if(MV_BIT_CHECK(nc->ports_mask, p) && (p!=QD_PORT_CPU)) {
		/* CPU port join the group in gtw_start */
		if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p),nc->ports_mask & ~(1<<QD_PORT_CPU)) != 0) {
		    printk("mv_gtw_set_vlan_in_vtu failed\n");
		}
	    }
	}
    }

    /* set cpu-port with port-based vlan to all other ports */
    GTW_DBG( GTW_DBG_LOAD, ("cpu port-based vlan:"));
    for(p=0,cnt=0; p<qd_dev->numOfPorts; p++) {
        if(p != QD_PORT_CPU) {
	    GTW_DBG( GTW_DBG_LOAD, ("%d ",p));
            port_list[cnt] = p;
            cnt++;
        }
    }
    GTW_DBG( GTW_DBG_LOAD, ("\n"));
    if( gvlnSetPortVlanPorts(qd_dev, QD_PORT_CPU, port_list, cnt) != GT_OK) {
        printk("gvlnSetPortVlanPorts failed\n"); 
        return -1;
    }
    
    /* set cpu-port with 802.1q secured mode */
    GTW_DBG( GTW_DBG_LOAD, ("cpu port-based 802.1q secure mode\n"));
    if(gvlnSetPortVlanDot1qMode(qd_dev,QD_PORT_CPU,GT_SECURE) != GT_OK) {
        printk("gvlnSetPortVlanDot1qMode failed\n"); 
        return -1;
    }

    /* set cpu-port with egrees add-tag mode */
    GTW_DBG( GTW_DBG_LOAD, ("cpu port egrees add-tag mode\n"));
    if(gprtSetEgressMode(qd_dev, QD_PORT_CPU, GT_ADD_TAG) != GT_OK) {
	printk("gprtSetEgressMode failed\n");
	return -1;
    }

#ifdef CONFIG_MV_GTW_QOS
    /* move VoIP packets to queue 3 */ 
    GTW_DBG( GTW_DBG_LOAD, ("VoIP ToS 0x%x to queue %d\n",mv_gtw_convert_str_to_tos(CONFIG_MV_GTW_QOS_TOS),MV_VOIP_PRIO));
    for(i=0; i<=0x40; i++) {
        if(gcosSetDscp2Tc(qd_dev, i, 0) != GT_OK) {
	    printk("gcosSetDscp2Tc failed\n");
	}
    }
    if(gcosSetDscp2Tc(qd_dev, mv_gtw_convert_str_to_tos(CONFIG_MV_GTW_QOS_TOS)>>2, MV_VOIP_PRIO) != GT_OK) {
	printk("gcosSetDscp2Tc failed\n");
    }
#endif

    if(mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
        /* move ARP packets to queue 2 */
	GTW_DBG( GTW_DBG_LOAD, ("ARP to queue %d\n",MV_ARP_PRIO));
	if(gsysSetARPPri(qd_dev,MV_ARP_PRIO) != GT_OK) {
	    printk("gqosSetArpQPri failed\n");
	}
	if(gsysSetForceARPPri(qd_dev,GT_TRUE) != GT_OK) {
	    printk("gqosSetARPQPriOverride failed\n");
	}
    }

    /* done! enable all ports */
    GTW_DBG( GTW_DBG_LOAD, ("enabling: ports "));
    for(p=0; p<qd_dev->numOfPorts; p++) {
	GTW_DBG( GTW_DBG_LOAD, ("%d ",p));
	if(gstpSetPortState(qd_dev, p, GT_PORT_FORWARDING) != GT_OK) {
	    printk("gstpSetPortState failed\n");
	}
    }
    GTW_DBG( GTW_DBG_LOAD, ("\n"));

#if 0//def CONFIG_SCM_SUPPORT
    {
        /* Configure WAN port to 10/100 auto speed */
        GT_BOOL en;
        
        gsysGetPPUEn(qd_dev,&en);
        gsysSetPPUEn(qd_dev, GT_TRUE);

        if(gprtSetPortAutoMode(
               qd_dev,QD_PORT_0,SPEED_10_100_DUPLEX_AUTO
               ) != GT_OK)
        {
            printk(KERN_ERR "gprtSetPortAutoMode failed on port %d\n",
                   QD_PORT_0);
        }
        gsysSetPPUEn(qd_dev, en);
    }
#endif
    
    printk("done\n");

    return 0;
}

static int init_gigabit(void)
{
    MV_ETH_PORT_INIT hal_init_struct;
    int i;
    MV_STATUS status;

    printk("init gigabit layer... ");

    mvEthInit();

    hal_init_struct.maxRxPktSize = MV_MTU;
    hal_init_struct.rxDefQ = EGIGA_DEF_RXQ; 
    memcpy(hal_init_struct.rxDescrNum,  gbe_desc_num_per_rxq, sizeof(gbe_desc_num_per_rxq));
    memcpy(hal_init_struct.txDescrNum,  gbe_desc_num_per_txq, sizeof(gbe_desc_num_per_txq));
    gbe_dev.hal_priv = mvEthPortInit(EGIGA_DEF_PORT, &hal_init_struct);
    if(!gbe_dev.hal_priv) {
        printk(KERN_ERR "failed to init gigabit hal\n");
	return -1;
    }
    spin_lock_init( &gbe_dev.lock );
    memset( &gbe_dev.rx_fill_timer, 0, sizeof(struct timer_list) );
    gbe_dev.rx_fill_timer.function = mv_gtw_rx_fill_on_timeout;
    gbe_dev.rx_fill_timer.data = -1;
    gbe_dev.rx_fill_flag = 0;

#ifdef INCLUDE_MULTI_QUEUE
    /* init rx policy */
    gbe_dev.rx_policy_priv = mvEthRxPolicyInit(EGIGA_DEF_PORT, EGIGA_RX_QUEUE_QUOTA, MV_ETH_PRIO_FIXED);
    if(!gbe_dev.rx_policy_priv) {
        printk(KERN_ERR "failed to init rx policy\n");
    }

    /* set ARP packets (marked with vlan-prio 2) to gigabit queue 2 */
    if(mvEthVlanPrioRxQueue(gbe_dev.hal_priv,(MV_ARP_PRIO<<1),MV_ARP_PRIO) != MV_OK) {
        printk(KERN_ERR "failed to priorities VlanPrio=%d (queue %d)\n",(MV_ARP_PRIO << 1), MV_ARP_PRIO);
    }

    /* set VoIP packets (marked with vlan-prio 3) to gigabit queue 3 */
    if(mvEthVlanPrioRxQueue(gbe_dev.hal_priv,(MV_VOIP_PRIO<<1),MV_VOIP_PRIO) != MV_OK) {
        printk(KERN_ERR "failed to priorities VlanPrio=%d (queue %d)\n",(MV_VOIP_PRIO << 1), MV_VOIP_PRIO);
    }
#endif /* INCLUDE_MULTI_QUEUE */

    /* fill rx ring with buffers */
    for(i = 0; i < MV_ETH_RX_Q_NUM; i++) {
    	mv_gtw_rx_fill(i, gbe_desc_num_per_rxq[i]);
    }

    /* start the hal - rx/tx activity */
    status = mvEthPortEnable( gbe_dev.hal_priv );
    if( (status != MV_OK) && (status != MV_NOT_READY)){
         printk(KERN_ERR "ethPortEnable failed");
	 return -1;
    }

    /* set tx/rx coalescing mechanism */
    gbe_dev.tx_coal = mvEthTxCoalSet( gbe_dev.hal_priv, EGIGA_TX_COAL );
    gbe_dev.rx_coal = mvEthRxCoalSet( gbe_dev.hal_priv, EGIGA_RX_COAL );

    /* clear and mask interrupts */
    MV_REG_WRITE( ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT), 0 );
    MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT), 0 );
    gbe_dev.rxcause = 0;
    gbe_dev.txcause = 0;
    MV_REG_WRITE( ETH_INTR_MASK_REG(EGIGA_DEF_PORT), 0 );
    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(EGIGA_DEF_PORT), 0 );
    gbe_dev.rxmask = 0;
    gbe_dev.txmask = 0;
    
    printk("done\n");
    return 0;
}


static int mv_gtw_tx( struct sk_buff *skb , struct net_device *dev )
{
    mv_priv *mvp = dev->priv;
    unsigned short grp_vlan_id = mvp->vlan_cfg->vlan_grp_id; 
    struct net_device_stats *stats = &(mvp->net_dev_stat);
    unsigned long flags;
    MV_STATUS status;
    int ret = 0, i, queue;
    unsigned int vlan_tag;

    if( netif_queue_stopped( dev ) ) {
        printk( KERN_ERR "%s: transmitting while stopped\n", dev->name );
        return 1;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
    local_irq_save(flags);
    if (!spin_trylock(&gbe_dev.lock)) {
    	/* Collision - tell upper layer to requeue */
        local_irq_restore(flags);
        return NETDEV_TX_LOCKED;
    }
#else
    spin_lock_irqsave( &gbe_dev.lock, flags );
#endif

    GTW_DBG( GTW_DBG_TX, ("%s: tx, #%d frag(s), csum by %s\n",
             dev->name, skb_shinfo(skb)->nr_frags+1, (skb->ip_summed==CHECKSUM_HW)?"HW":"CPU") );
    EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_events++) );

    /* basic init of pkt_info. first cell in buf_info_arr is left for header prepending if necessary */
    gbe_dev.tx_pkt_info.osInfo = (MV_ULONG)skb;
    gbe_dev.tx_pkt_info.pktSize = skb->len;
    gbe_dev.tx_pkt_info.pFrags = &gbe_dev.tx_buf_info_arr[0];
    gbe_dev.tx_pkt_info.numFrags = 0;
    gbe_dev.tx_pkt_info.status = 0;
    
    /* see if this is a single/multiple buffered skb */
    if( skb_shinfo(skb)->nr_frags == 0 ) {
        gbe_dev.tx_pkt_info.pFrags->bufVirtPtr = skb->data;
        gbe_dev.tx_pkt_info.pFrags->bufSize = skb->len;
        gbe_dev.tx_pkt_info.numFrags = 1;
    }
    else {
        MV_BUF_INFO *p_buf_info = gbe_dev.tx_pkt_info.pFrags;

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

        gbe_dev.tx_pkt_info.numFrags = skb_shinfo(skb)->nr_frags + 1;
    }

#ifdef TX_CSUM_OFFLOAD
    /* if HW is suppose to offload layer4 checksum, set some bits in the first buf_info command */
    if(skb->ip_summed == CHECKSUM_HW) {
        GTW_DBG( GTW_DBG_TX, ("%s: tx csum offload\n", dev->name) );
        /*EGIGA_STAT( EGIGA_STAT_TX, Add counter here );*/
        gbe_dev.tx_pkt_info.status =
        ETH_TX_IP_NO_FRAG |           /* we do not handle fragmented IP packets. add check inside iph!! */
        ((skb->nh.iph->ihl) << ETH_TX_IP_HEADER_LEN_OFFSET) |                            /* 32bit units */
        ((skb->nh.iph->protocol == IPPROTO_TCP) ? ETH_TX_L4_TCP_TYPE : ETH_TX_L4_UDP_TYPE) | /* TCP/UDP */
        ETH_TX_GENERATE_L4_CHKSUM_MASK |                                /* generate layer4 csum command */
        ETH_TX_GENERATE_IP_CHKSUM_BIT;                              /* generate IP csum (already done?) */
    }
    else {
        GTW_DBG( GTW_DBG_TX, ("%s: no tx csum offload\n", dev->name) );
        /*EGIGA_STAT( EGIGA_STAT_TX, Add counter here );*/
        gbe_dev.tx_pkt_info.status = 0x5 << ETH_TX_IP_HEADER_LEN_OFFSET; /* Errata BTS #50 */
    }
#endif

    queue = EGIGA_DEF_TXQ;

    /* add zero pad for small packets */
    if(gbe_dev.tx_pkt_info.pktSize < QD_MIN_ETH_PACKET_LEN) {
        /* add zero pad in last cell of packet in buf_info_arr */
        GTW_DBG(GTW_DBG_TX,("add zero pad\n"));
        gbe_dev.tx_buf_info_arr[gbe_dev.tx_pkt_info.numFrags].bufVirtPtr = zero_pad;
        gbe_dev.tx_buf_info_arr[gbe_dev.tx_pkt_info.numFrags].bufSize = QD_MIN_ETH_PACKET_LEN - gbe_dev.tx_pkt_info.pktSize;
        gbe_dev.tx_pkt_info.pktSize += QD_MIN_ETH_PACKET_LEN - gbe_dev.tx_pkt_info.pktSize;
        gbe_dev.tx_pkt_info.numFrags++;
    }

    /* add vlan tag by breaking first frag to three parts: */
    /* (1) DA+SA in skb (2) the new tag (3) rest of skb    */
    if(gbe_dev.tx_pkt_info.pFrags->bufSize < 12) {
        /* never happens, but for completeness */
        printk(KERN_ERR "mv_gtw cannot transmit first frag when len < 12\n");
    }
    else {
        MV_BUF_INFO *p_buf_info_first = gbe_dev.tx_pkt_info.pFrags;
	MV_BUF_INFO *p_buf_info_last = gbe_dev.tx_pkt_info.pFrags + gbe_dev.tx_pkt_info.numFrags - 1;

	/* make room for two cells */
        while(p_buf_info_last != p_buf_info_first) {
	    *(p_buf_info_last+2) = *p_buf_info_last;
	    p_buf_info_last--;
	}

	/* the second half of the original first frag */
	(p_buf_info_first+2)->bufSize = p_buf_info_first->bufSize - 12;
	(p_buf_info_first+2)->bufVirtPtr = p_buf_info_first->bufVirtPtr + 12;

	/* the tag (safe on stack) */
	vlan_tag = cpu_to_be32((0x8100 << 16) | grp_vlan_id);
        (p_buf_info_first+1)->bufVirtPtr = (unsigned char*)&vlan_tag;
	(p_buf_info_first+1)->bufSize = QD_VLANTAG_SIZE;

	/* the first half of the original first frag */
        p_buf_info_first->bufSize = 12;

	/* count the new frags */
	gbe_dev.tx_pkt_info.numFrags += 2;
        gbe_dev.tx_pkt_info.pktSize += QD_VLANTAG_SIZE;
    }

    GTW_DBG(GTW_DBG_VLAN, ("%s TX: q=%d, vid=0x%x\n",dev->name,queue,grp_vlan_id));

    /* now send the packet */
    status = mvEthPortTx( gbe_dev.hal_priv, queue, &gbe_dev.tx_pkt_info );

    /* check status */
    if( status == MV_OK ) {
        stats->tx_bytes += skb->len;
        stats->tx_packets ++;
        dev->trans_start = jiffies;
        gbe_dev.txq_count[queue]++;
        GTW_DBG( GTW_DBG_TX, ("ok (%d); ", gbe_dev.txq_count[queue]) );
        EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_hal_ok[queue]++) );
    }
    else {
        /* tx failed. higher layers will free the skb */
        ret = 1;
        stats->tx_dropped++;

        if( status == MV_NO_RESOURCE ) {
            /* it must not happen because we call to netif_stop_queue in advance. */
            GTW_DBG( GTW_DBG_TX, ("%s: queue is full, stop transmit\n", dev->name) );
            netif_stop_queue(dev);
	    mv_netdev_stopped = dev;
            EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_hal_no_resource[queue]++) );
            EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_netif_stop[queue]++) );
        }
        else if( status == MV_ERROR ) {
            printk( KERN_ERR "%s: error on transmit\n", dev->name );
            EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_hal_error[queue]++) );
        }
        else {
            printk( KERN_ERR "%s: unrecognize status on transmit\n", dev->name );
            EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_hal_unrecognize[queue]++) );
        }
    }

#ifndef INCLUDE_MULTI_QUEUE
    /* if number of available descriptors left is less than  */
    /* MAX_SKB_FRAGS stop the stack. if multi queue is used, */
    /* don't stop the stack just because one queue is full.  */
    if( mvEthTxResourceGet(gbe_dev.hal_priv, queue) - skb_shinfo(skb)->nr_frags <= MAX_SKB_FRAGS ) {
        GTW_DBG( GTW_DBG_TX, ("%s: stopping network tx interface\n", dev->name) );
        netif_stop_queue(dev);
	mv_netdev_stopped = dev;
        EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_netif_stop[queue]++) );
    }
#endif
    spin_unlock_irqrestore( &gbe_dev.lock, flags );

    return ret;
}


static unsigned int mv_gtw_tx_done(void)
{
    MV_PKT_INFO pkt_info;
    unsigned int count = 0;
    MV_STATUS status;
    unsigned int queue = 0;

    EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_events++) );

    /* read cause once more and clear tx bits */
    gbe_dev.txcause |= MV_REG_READ(ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT));
    MV_REG_WRITE(ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT),gbe_dev.txcause & (~EGIGA_TXQ_MASK) );
    gbe_dev.txcause &= EGIGA_TXQ_MASK;

    /* release the transmitted packets */
    while( 1 ) {

        if(!gbe_dev.txcause)
	    break;

        queue = EGIGA_DEF_TXQ;

        /* get a packet */  
        status = mvEthPortTxDone( gbe_dev.hal_priv, queue, &pkt_info );

	if( status == MV_OK ) {

	    gbe_dev.txq_count[queue]--;

	    /* validate skb */
	    if( !(pkt_info.osInfo) ) {
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_invalid_skb[queue]++) );
		continue;
	    }

	    /* handle tx error */
	    if( pkt_info.status & (ETH_ERROR_SUMMARY_BIT) ) {
	        GTW_DBG( GTW_DBG_TX_DONE, ("bad tx-done status\n") );
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_bad_stat[queue]++) );
	    }

	    /* it transmission was previously stopped, now it can be restarted. */
	    if( mv_netdev_stopped && netif_queue_stopped(mv_netdev_stopped) && (mv_netdev_stopped->flags & IFF_UP) ) {
	        GTW_DBG( GTW_DBG_TX_DONE, ("%s: restart transmit\n",mv_netdev_stopped->name) );
		EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_netif_wake[queue]++) );
		netif_wake_queue(mv_netdev_stopped);
		mv_netdev_stopped = NULL;
	    }

	    /* release the skb */
	    dev_kfree_skb( (struct sk_buff *)pkt_info.osInfo );
	    count++;
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_ok[queue]++) );
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, if(gbe_dev.egiga_stat.tx_done_max[queue] < count) gbe_dev.egiga_stat.tx_done_max[queue] = count );
	    EGIGA_STAT( EGIGA_STAT_TX_DONE, if(gbe_dev.egiga_stat.tx_done_min[queue] > count) gbe_dev.egiga_stat.tx_done_min[queue] = count );
	}
	else {
	    if( status == MV_EMPTY ) {
	    	/* no more work */
	    	GTW_DBG( GTW_DBG_TX_DONE, ("no more work ") );
	    	EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_no_more[queue]++) );
	    }
	    else if( status == MV_NOT_FOUND ) {
	    	/* hw still in tx */
	    	GTW_DBG( GTW_DBG_TX_DONE, ("hw still in tx ") );
	    	EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_still_tx[queue]++) );
	    }
	    else {
	    	printk( KERN_ERR "unrecognize status on tx done\n");
	    	EGIGA_STAT( EGIGA_STAT_TX_DONE, (gbe_dev.egiga_stat.tx_done_hal_unrecognize[queue]++) );
	    }

	    gbe_dev.txcause &= ~ETH_CAUSE_TX_BUF_MASK(queue);
	    break;
    	}
    }

    GTW_DBG( GTW_DBG_TX_DONE, ("tx-done %d (%d)\n", count, gbe_dev.txq_count[queue]) );

    return count;
}

static void mv_gtw_tx_timeout( struct net_device *dev ) 
{
    EGIGA_STAT( EGIGA_STAT_TX, (gbe_dev.egiga_stat.tx_timeout++) );
    printk( KERN_INFO "%s: tx timeout\n", dev->name );
}

#ifdef RX_CSUM_OFFLOAD
static MV_STATUS mv_gtw_rx_csum_offload(MV_PKT_INFO *pkt_info)
{
    if( (pkt_info->pktSize > RX_CSUM_MIN_BYTE_COUNT)   && /* Minimum        */
        (pkt_info->status & ETH_RX_IP_FRAME_TYPE_MASK) && /* IPv4 packet    */
        (pkt_info->status & ETH_RX_IP_HEADER_OK_MASK)  && /* IP header OK   */
        (!(pkt_info->fragIP))                          && /* non frag IP    */
        (!(pkt_info->status & ETH_RX_L4_OTHER_TYPE))   && /* L4 is TCP/UDP  */
        (pkt_info->status & ETH_RX_L4_CHECKSUM_OK_MASK) ) /* L4 checksum OK */
            return MV_OK;

    if(!(pkt_info->pktSize > RX_CSUM_MIN_BYTE_COUNT))
        GTW_DBG( GTW_DBG_RX, ("Byte count smaller than %d\n", RX_CSUM_MIN_BYTE_COUNT) );
    if(!(pkt_info->status & ETH_RX_IP_FRAME_TYPE_MASK))
        GTW_DBG( GTW_DBG_RX, ("Unknown L3 protocol\n") );
    if(!(pkt_info->status & ETH_RX_IP_HEADER_OK_MASK))
        GTW_DBG( GTW_DBG_RX, ("Bad IP csum\n") );
    if(pkt_info->fragIP)
        GTW_DBG( GTW_DBG_RX, ("Fragmented IP\n") );
    if(pkt_info->status & ETH_RX_L4_OTHER_TYPE)
        GTW_DBG( GTW_DBG_RX, ("Unknown L4 protocol\n") );
    if(!(pkt_info->status & ETH_RX_L4_CHECKSUM_OK_MASK))
        GTW_DBG( GTW_DBG_RX, ("Bad L4 csum\n") );

    return MV_FAIL;
}
#endif

static unsigned int mv_gtw_rx(unsigned int work_to_do)
{
    mv_priv *mvp;
    struct net_device_stats *stats;
    struct net_device *dev=NULL;
    unsigned int vlan_tag;
    struct sk_buff *skb;
    MV_PKT_INFO pkt_info;
    int work_done = 0;
    MV_STATUS status;
    unsigned int queue = 0;
    unsigned int done_per_q[MV_ETH_RX_Q_NUM] = {0};

#ifdef INCLUDE_MULTI_QUEUE
    unsigned int temp = MV_REG_READ(ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT));
    gbe_dev.rxcause |= temp & EGIGA_RXQ_MASK;
    gbe_dev.rxcause |= (temp & EGIGA_RXQ_RES_MASK) >> (ETH_CAUSE_RX_ERROR_OFFSET - ETH_CAUSE_RX_READY_OFFSET);
    MV_REG_WRITE(ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT), ~(gbe_dev.rxcause | (gbe_dev.rxcause << (ETH_CAUSE_RX_ERROR_OFFSET - ETH_CAUSE_RX_READY_OFFSET)) ) );
    GTW_DBG( GTW_DBG_RX,("%s: cause = 0x%08x\n\n", dev->name, gbe_dev.rxcause) );
#endif /* INCLUDE_MULTI_QUEUE */

    EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_events++) );

#ifdef CONFIG_MV_GTW_QOS
    if(mv_gtw_qos_tos_quota != -1) {
        work_to_do = (work_to_do<mv_gtw_qos_tos_quota) ? work_to_do : mv_gtw_qos_tos_quota;
    }
#endif

    GTW_DBG( GTW_DBG_RX, ("^rx work_to_do %d\n",work_to_do) );

    while( work_done < work_to_do ) {

#ifdef INCLUDE_MULTI_QUEUE
        if(!gbe_dev.rxcause)
            break;
        if(gbe_dev.rx_policy_priv)
	    queue = mvEthRxPolicyGet(gbe_dev.rx_policy_priv, gbe_dev.rxcause);
        else
	    queue = EGIGA_DEF_RXQ;
#else
        queue = EGIGA_DEF_RXQ;
#endif /* INCLUDE_MULTI_QUEUE */

        /* get rx packet */ 
	status = mvEthPortRx( gbe_dev.hal_priv, queue, &pkt_info );

        /* check status */
	if( status == MV_OK ) {
	    work_done++;
	    done_per_q[queue]++;
	    gbe_dev.rxq_count[queue]--;
	    EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_ok[queue]++) );
	} else{ 
	    if( status == MV_NO_RESOURCE ) {
	    	/* no buffers for rx */
	    	GTW_DBG( GTW_DBG_RX, ("rx no resource ") );
	    	EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_no_resource[queue]++) );
		TRC_REC("rx no resource\n");
	    } 
	    else if( status == MV_NO_MORE ) {
	    	/* no more rx packets ready */
	    	GTW_DBG( GTW_DBG_RX, ("rx no more ") );
	    	EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_no_more[queue]++) );
		TRC_REC("rx no more\n");
	    } 
	    else {
	    	printk( KERN_ERR "undefined status on rx poll\n");
	    	EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_error[queue]++) );
		TRC_REC("rx undefined status\n");
	    }

#ifdef INCLUDE_MULTI_QUEUE
	    gbe_dev.rxcause &= ~ETH_CAUSE_RX_READY_MASK(queue);
	    continue;
#else
	    break;
#endif
	}

	/* validate skb */ 
	if( !(pkt_info.osInfo) ) {
	    printk( KERN_ERR "error in rx\n");
	    EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_invalid_skb[queue]++) );
	    continue;
	}

	skb = (struct sk_buff *)( pkt_info.osInfo );

	/* handle rx error */
	if( pkt_info.status & (ETH_ERROR_SUMMARY_MASK) ) {
            unsigned int err = pkt_info.status & ETH_RX_ERROR_CODE_MASK;
	    TRC_REC("rx general error\n");
            /* RX resource error is likely to happen when receiving packets, which are     */
            /* longer then the Rx buffer size, and they are spreading on multiple buffers. */
            /* Rx resource error - No descriptor in the middle of a frame.                 */
	    if( err == ETH_RX_RESOURCE_ERROR ) {
	        GTW_DBG( GTW_DBG_RX, ("bad rx status %08x, (resource error)",(unsigned int)pkt_info.status));
            }
	    else if( err == ETH_RX_OVERRUN_ERROR ) {
		GTW_DBG( GTW_DBG_RX, ("bad rx status %08x, (overrun error)",(unsigned int)pkt_info.status));
            }
	    else {
		printk( KERN_INFO "bad rx status %08x, ",(unsigned int)pkt_info.status );
	    	if( err == ETH_RX_MAX_FRAME_LEN_ERROR )
	        	printk( KERN_INFO "(max frame length error)" );
	    	else if( err == ETH_RX_CRC_ERROR )
	        	printk( KERN_INFO "(crc error)" );
	    	else
	        	printk( KERN_INFO "(unknown error)" );
	    	printk( KERN_INFO "\n" );
	    }
	    
	    dev_kfree_skb( skb );
	    EGIGA_STAT( EGIGA_STAT_RX, (gbe_dev.egiga_stat.rx_hal_bad_stat[queue]++) );
	    continue;
	}

	/* good rx */
    GTW_DBG( GTW_DBG_RX, ("good rx (skb=%p skb->data=%p skb->len=%d)\n", skb, skb->data, skb->len) );
                        
	/* find our net_device by the vid group id from vlan-tag */
	vlan_tag = ((skb->data[14]<<8) | skb->data[15]);
	mvp = &mv_privs[MV_GTW_VLANID_TO_GROUP(vlan_tag)];
	GTW_DBG( GTW_DBG_RX, ("vid = 0x%x map to dev %s\n",vlan_tag,mvp->net_dev->name) );
	dev = mvp->net_dev;
	stats = &mvp->net_dev_stat;
	stats->rx_packets++;
	stats->rx_bytes += pkt_info.pktSize;
	skb->dev = dev;
	GTW_DBG( GTW_DBG_VLAN, ("%s RX: q=%d, vlan_tag=0x%x, prio=%d\n",dev->name,queue,vlan_tag,(skb->data[14] >> 5)));

#ifdef CONFIG_SCM_SUPPORT
    skb->mark = (skb->data[14] >> 5);
#endif
    
	/* set skb length to the total size minus 2 bytes shifted by GbE to align IP header minus 4 byte CRC */
	skb_put(skb, pkt_info.pktSize - 4 - 2);

	/* remove the vlan tag */
	memmove(skb->data+QD_VLANTAG_SIZE,skb->data,12);
	skb_pull(skb, QD_VLANTAG_SIZE);
    
    skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = eth_type_trans(skb,dev); 
    
#ifdef CONFIG_MV_GTW_IGMP
	/* store the ingress port number and the valn's database at the tail of skb buffer (override crc) */
	*(skb->tail) = MV_GTW_VLANID_TO_GROUP(vlan_tag);
	*(skb->tail+1) = MV_GTW_VLANID_TO_PORT(vlan_tag);
	GTW_DBG( GTW_DBG_IGMP, ("skb->tail @ %p = 0x%x, skb->tail+1 @ %p = 0x%x\n",skb->tail,*(skb->tail),skb->tail+1,*(skb->tail+1)));
#endif    
    
	status = netif_receive_skb( skb );
        EGIGA_STAT( EGIGA_STAT_RX, if(status) (gbe_dev.egiga_stat.rx_netif_drop[queue]++) );
    }

    GTW_DBG( GTW_DBG_RX, ("\nwork_done %d (%d)", work_done, gbe_dev.rxq_count[queue]) );

    /* refill rx ring with new buffers */
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++) {
	if(done_per_q[queue] > 0) {
	    TRC_REC("^rx receive %d packets from queue %d\n",done_per_q[queue],queue);
	    mv_gtw_rx_fill( queue, gbe_desc_num_per_rxq[queue] );
	}
    }

    return(work_done);
}



static unsigned int mv_gtw_rx_fill(unsigned int queue, int total)
{
    MV_PKT_INFO pkt_info;
    MV_BUF_INFO bufInfo;
    struct sk_buff *skb;
    unsigned int count = 0, buf_size;
    MV_STATUS status;
    int alloc_skb_failed = 0;

    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_events[queue]++) );

    while( total-- ) {

        
        /* allocate a buffer */
	buf_size = MV_MTU + 32 /* 32(extra for cache prefetch) */ + 8 /* +8 to align on 8B */;
#ifdef CONFIG_SCM_SUPPORT        
#define ALIGN_FOR_80211	   80
    buf_size += ALIGN_FOR_80211;
#endif
        skb = dev_alloc_skb( buf_size ); 
	if( !skb ) {
	    GTW_DBG( GTW_DBG_RX_FILL, ("rx_fill failed alloc skb\n") );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_alloc_skb_fail[queue]++) );
	    alloc_skb_failed = 1;
	    break;
	}

#ifdef CONFIG_SCM_SUPPORT
    skb_reserve(skb,ALIGN_FOR_80211);
#endif
    
	/* align the buffer on 8B */
	if( (unsigned long)(skb->data) & 0x7 ) {
	    skb_reserve( skb, 8 - ((unsigned long)(skb->data) & 0x7) );
	}

        bufInfo.bufVirtPtr = skb->data;
        bufInfo.bufSize = MV_MTU;
        pkt_info.osInfo = (MV_ULONG)skb;
        pkt_info.pFrags = &bufInfo;
	pkt_info.pktSize = MV_MTU; /* how much to invalidate */

	/* skip on first 2B (HW header) */
	skb_reserve( skb, 2 );
	skb->len = 0;

	/* give the buffer to hal */
	status = mvEthPortRxDone( gbe_dev.hal_priv, queue, &pkt_info );
	
	if( status == MV_OK ) {
	    count++;
	    gbe_dev.rxq_count[queue]++;
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_hal_ok[queue]++) );	    
	}
	else if( status == MV_FULL ) {
	    /* the ring is full */
	    count++;
	    gbe_dev.rxq_count[queue]++;
	    GTW_DBG( GTW_DBG_RX_FILL, ("rxq full\n") );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_hal_full[queue]++) );
	    if( gbe_dev.rxq_count[queue] != gbe_desc_num_per_rxq[queue])
	        printk( KERN_ERR "Q %d: error in status fill (%d != %d)\n", queue, gbe_dev.rxq_count[queue], gbe_desc_num_per_rxq[queue]);
	    break;
	} 
	else {
	    printk( KERN_ERR "Q %d: error in rx-fill\n", queue );
	    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_hal_error[queue]++) );
	    break;
	}
    }

    /* if allocation failed and the number of rx buffers in the ring is less than */
    /* half of the ring size, then set a timer to try again later.                */
    if( alloc_skb_failed && (gbe_dev.rxq_count[queue] < (gbe_desc_num_per_rxq[queue]/2)) ) {
        if( gbe_dev.rx_fill_flag == 0 ) {
	    printk( KERN_INFO "Q %d: set rx timeout to allocate skb\n", queue );
	    gbe_dev.rx_fill_timer.expires = jiffies + (HZ/10); /*100ms*/
	    add_timer( &gbe_dev.rx_fill_timer );
	    gbe_dev.rx_fill_flag = 1;
	}
    }

    GTW_DBG( GTW_DBG_RX_FILL, ("rx fill %d (total %d)", count, gbe_dev.rxq_count[queue]) );
    
    return count;
}

static void mv_gtw_rx_fill_on_timeout( unsigned long data ) 
{
    unsigned int queue;

    GTW_DBG( GTW_DBG_RX_FILL, ("rx_fill_on_timeout") );
    EGIGA_STAT( EGIGA_STAT_RX_FILL, (gbe_dev.egiga_stat.rx_fill_timeout_events++) );
   
    gbe_dev.rx_fill_flag = 0;
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++)
    	mv_gtw_rx_fill(queue, gbe_desc_num_per_rxq[queue]);
}

static int mv_gtw_poll( struct net_device *dev, int *budget )
{
    int rx_work_done;
    int tx_work_done;
    unsigned long flags;

    EGIGA_STAT( EGIGA_STAT_INT, (gbe_dev.egiga_stat.poll_events++) );

    tx_work_done = mv_gtw_tx_done();
    rx_work_done = mv_gtw_rx( min(*budget,dev->quota) );

    *budget -= rx_work_done;
    dev->quota -= rx_work_done;

    GTW_DBG( GTW_DBG_RX, ("poll work done: tx-%d rx-%d\n",tx_work_done,rx_work_done) );
    TRC_REC("^mv_gtw_poll work done: tx-%d rx-%d\n",tx_work_done,rx_work_done);

    if( ((tx_work_done==0) && (rx_work_done==0)) || (!netif_running(dev)) ) {
	    local_irq_save(flags);
	    TRC_REC("^mv_gtw_poll remove from napi\n");
            netif_rx_complete(dev);
	    EGIGA_STAT( EGIGA_STAT_INT, (gbe_dev.egiga_stat.poll_complete++) );
	    /* unmask interrupts */
	    MV_REG_WRITE( ETH_INTR_MASK_REG(EGIGA_DEF_PORT), EGIGA_PICR_MASK );
	    MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(EGIGA_DEF_PORT), EGIGA_PICER_MASK );
	    gbe_dev.rxmask = EGIGA_PICR_MASK;
	    gbe_dev.txmask = EGIGA_PICER_MASK;
	    GTW_DBG( GTW_DBG_RX, ("unmask\n") );
	    local_irq_restore(flags);
            return 0;
    }

    return 1;
}

static irqreturn_t mv_gtw_interrupt_handler( int irq , void *dev_id )
{
    unsigned int picr, picer=0;

    spin_lock( &gbe_dev.lock );
    EGIGA_STAT( EGIGA_STAT_INT, (gbe_dev.egiga_stat.irq_total++) );

    picr = MV_REG_READ( ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT) );
    GTW_DBG( GTW_DBG_INT, ("%s [picr %08x]",__FUNCTION__,picr) );
    TRC_REC("^mv_gtw_interrupt_handler\n");

    if( !picr ) {
        EGIGA_STAT( EGIGA_STAT_INT, (gbe_dev.egiga_stat.irq_none++) );
	spin_unlock( &gbe_dev.lock );
        return IRQ_NONE;
    }
    MV_REG_WRITE( ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT), ~picr );

    if(picr & BIT1) {
	picer = MV_REG_READ( ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT) );
	if(picer)
	    MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT), ~picer );
    }

    /* schedule the first net_device to do the work out of interrupt context (NAPI) */
    if (netif_rx_schedule_prep(main_net_dev)) {
        TRC_REC("^sched napi\n");

	/* mask cause */
        gbe_dev.rxmask = 0;
        MV_REG_WRITE( ETH_INTR_MASK_REG(EGIGA_DEF_PORT), 0 );

	/* save rx cause and clear */
        gbe_dev.rxcause |= (picr&EGIGA_RXQ_MASK) | ((picr&EGIGA_RXQ_RES_MASK) >> (ETH_CAUSE_RX_ERROR_OFFSET-ETH_CAUSE_RX_READY_OFFSET));
	MV_REG_WRITE( ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT), 0 );

	/* mask tx-event */
        gbe_dev.txmask = 0;
        MV_REG_WRITE( ETH_INTR_MASK_EXT_REG(EGIGA_DEF_PORT), 0 );

	/* save tx cause and clear */
        gbe_dev.txcause |= picer & EGIGA_TXQ_MASK;
	MV_REG_WRITE( ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT), 0 );

        /* schedule the work (rx+txdone) out of interrupt contxet */
        __netif_rx_schedule(main_net_dev);
    }
    else {
        if(netif_running(main_net_dev)) {
	    printk("rx interrupt while in polling list\n");
	    printk("rx-cause=0x%08x\n",MV_REG_READ(ETH_INTR_CAUSE_REG(EGIGA_DEF_PORT)));
	    printk("rx-mask =0x%08x\n",MV_REG_READ(ETH_INTR_MASK_REG(EGIGA_DEF_PORT)));
	    printk("tx-cause=0x%08x\n",MV_REG_READ(ETH_INTR_CAUSE_EXT_REG(EGIGA_DEF_PORT)));
	    printk("tx-mask =0x%08x\n",MV_REG_READ(ETH_INTR_MASK_EXT_REG(EGIGA_DEF_PORT)));
	}
    }

    spin_unlock( &gbe_dev.lock );
    return IRQ_HANDLED;
}

static struct net_device_stats* mv_gtw_get_stats( struct net_device *dev )
{
    return &(((mv_priv *)dev->priv)->net_dev_stat);
}

static void mv_gtw_set_multicast_list(struct net_device *dev) 
{
    struct dev_mc_list *curr_addr = dev->mc_list;
    struct mv_vlan_cfg *nc;
    int i;

    disable_irq(ETH_PORT0_IRQ_NUM);

    if(dev->flags & IFF_PROMISC) { /* GbE accept everything */
	GTW_DBG(GTW_DBG_MCAST, ("mv_gateway: promiscuous mode on\n"));
	mvEthRxFilterModeSet(gbe_dev.hal_priv, 1);
    }
    else if(dev->flags & IFF_ALLMULTI) { /* GbE accept all multicasts */
	GTW_DBG(GTW_DBG_MCAST, ("mv_gateway: all-multicasts mode on\n"));
#ifndef CONFIG_SCM_SUPPORT
    /* gigabit always in promisc unicast */
	mvEthRxFilterModeSet(gbe_dev.hal_priv, 0);
#endif    
	mvEthSetSpecialMcastTable(EGIGA_DEF_PORT, EGIGA_DEF_RXQ);
	mvEthSetOtherMcastTable(EGIGA_DEF_PORT, EGIGA_DEF_RXQ);
    }
    else if(dev->mc_count) { /* GbE accept specific multicasts */
#ifndef CONFIG_SCM_SUPPORT
    /* gigabit always in promisc unicast */        
        mvEthRxFilterModeSet(gbe_dev.hal_priv, 0);
#endif        
	for(i=0; i<dev->mc_count; i++, curr_addr = curr_addr->next) {
	    if (!curr_addr)
		break;
	    GTW_DBG(GTW_DBG_MCAST, ("mv_gateway: adding multicast %02x:%02x:%02x:%02x:%02x:%02x\n",
		curr_addr->dmi_addr[0],curr_addr->dmi_addr[1],curr_addr->dmi_addr[2],
		curr_addr->dmi_addr[3],curr_addr->dmi_addr[4],curr_addr->dmi_addr[5]));
	   mvEthMcastAddrSet(gbe_dev.hal_priv, curr_addr->dmi_addr, EGIGA_DEF_RXQ);
	}
    }
    else { /* GbE does not accept any multicasts */ 
	GTW_DBG(GTW_DBG_MCAST, ("mv_gateway: promiscuous mode off all multicasts discarded\n"));
	mvEthRxFilterModeSet(gbe_dev.hal_priv, 0);
     }

    /* GbE accept driver's net_devices unicasts */
    for(i=0,nc=net_cfg;i<MV_NUM_OF_IFS;i++,nc++) {
        mvEthMacAddrSet(gbe_dev.hal_priv,nc->macaddr,EGIGA_DEF_RXQ);
    }

    enable_irq(ETH_PORT0_IRQ_NUM);
}

GT_BOOL ReadMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int* value)
{
    if(mvEthPhyRegRead(portNumber, MIIReg , (unsigned short *)value) == MV_OK)
        return GT_TRUE;
    return GT_FALSE;
}
GT_BOOL WriteMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int data)
{
    /*printk("MII write: port=0x%x, reg=%d, data=0x%x\n",portNumber,MIIReg,data);*/
    if (mvEthPhyRegWrite(portNumber, MIIReg ,data) == MV_OK)
        return GT_TRUE;
    return GT_FALSE;
}
static int mv_gtw_set_port_based_vlan(unsigned int ports_mask)
{
    unsigned int p, pl;
    unsigned char cnt;
    GT_LPORT port_list[MAX_SWITCH_PORTS];

    for(p=0; p<qd_dev->numOfPorts; p++) {
	if( MV_BIT_CHECK(ports_mask, p) && (p != QD_PORT_CPU) ) {
	    for(pl=0,cnt=0; pl<qd_dev->numOfPorts; pl++) {
		if( MV_BIT_CHECK(ports_mask, pl) && (pl != p) ) {
		    port_list[cnt] = pl;
                    cnt++;
                }
	    }
            if( gvlnSetPortVlanPorts(qd_dev, p, port_list, cnt) != GT_OK) {
	        printk("gvlnSetPortVlanPorts failed\n");
                return -1;
            }
        }
    }
    return 0;
}
static int mv_gtw_set_vlan_in_vtu(unsigned short vlan_id,unsigned int ports_mask)
{
    GT_VTU_ENTRY vtu_entry;
    unsigned int p;

    vtu_entry.vid = vlan_id;
    vtu_entry.DBNum = MV_GTW_VLANID_TO_GROUP(vlan_id);
    vtu_entry.vidPriOverride = GT_FALSE;
    vtu_entry.vidPriority = 0;
    vtu_entry.vidExInfo.useVIDFPri = GT_FALSE;
    vtu_entry.vidExInfo.vidFPri = 0;
    vtu_entry.vidExInfo.useVIDQPri = GT_FALSE;
    vtu_entry.vidExInfo.vidQPri = 0;
    vtu_entry.vidExInfo.vidNRateLimit = GT_FALSE;
    GTW_DBG( GTW_DBG_LOAD, ("vtu entry: vid=0x%x, port ", vtu_entry.vid));
    for(p=0; p<qd_dev->numOfPorts; p++) {
        if(MV_BIT_CHECK(ports_mask, p)) {
	    GTW_DBG( GTW_DBG_LOAD, ("%d ", p));
	    vtu_entry.vtuData.memberTagP[p] = MEMBER_EGRESS_UNMODIFIED;
	}
	else {
	    vtu_entry.vtuData.memberTagP[p] = NOT_A_MEMBER;
	}
	vtu_entry.vtuData.portStateP[p] = 0;
    }

    if(gvtuAddEntry(qd_dev, &vtu_entry) != GT_OK) {
        printk("gvtuAddEntry failed\n");
        return -1;
    }

    GTW_DBG( GTW_DBG_LOAD, ("\n"));
    return 0;
}

#ifdef CONFIG_MV_GTW_QOS
int mv_gtw_qos_tos_enable(void)
{
    printk("MV-Gatway VoIP QoS ON\n");
    netdev_max_backlog = 15;
    MAX_SOFTIRQ_RESTART = 1;
    mv_gtw_qos_tos_quota = 5;
    TRC_REC("^QoS ON sched-iter=%d quota=%d netdev_max_backlog=%d\n",MAX_SOFTIRQ_RESTART,mv_gtw_qos_tos_quota,netdev_max_backlog);
    return 0;
}
int mv_gtw_qos_tos_disable(void)
{
    printk("MV-Gatway VoIP QoS OFF\n");
    netdev_max_backlog = 300;
    MAX_SOFTIRQ_RESTART = 10;
    mv_gtw_qos_tos_quota = -1;
    TRC_REC("^QoS OFF sched-iter=%d quota=%d\n",MAX_SOFTIRQ_RESTART,mv_gtw_qos_tos_quota);
    return 0;
}
#endif /*CONFIG_MV_GTW_QOS*/

#ifdef CONFIG_MV_GTW_IGMP
int mv_gtw_enable_igmp(void)
{
    unsigned char p;

    printk("mv_gateway: enabling L2 IGMP snooping\n");

    /* enable IGMP snoop on all ports (except cpu port) */
    for(p=0; p<qd_dev->numOfPorts; p++) {
	if(p != QD_PORT_CPU) {
	    if(gprtSetIGMPSnoop(qd_dev, p, GT_TRUE) != GT_OK) {
		printk("gprtSetIGMPSnoop failed\n");
		return -1;
	    }
	}
    }
    return -1;
}

int mv_gtw_set_multicast_atu_entry(unsigned char *mac_addr, unsigned char db, unsigned int ports_mask)
{
    GT_ATU_ENTRY mac_entry;
    struct mv_vlan_cfg *nc;

    /* validate db with VLAN id */
    nc = &net_cfg[db];
    if(MV_GTW_VLANID_TO_GROUP(nc->vlan_grp_id) != db) {
        printk("mv_gtw_set_multicast_atu_entry failed (invalid database)\n");
	return -1;
    }

    mac_entry.trunkMember = GT_FALSE;
    mac_entry.prio = 0;
    mac_entry.exPrio.useMacFPri = GT_FALSE;
    mac_entry.exPrio.macFPri = 0;
    mac_entry.exPrio.macQPri = 0;
    mac_entry.DBNum = db;
    mac_entry.portVec = ports_mask & nc->ports_mask;
    memcpy(mac_entry.macAddr.arEther,mac_addr,6);
    if(mac_entry.portVec) {
	printk("mv_gateway: setting entry for %s (db%d): ",nc->name,db);
	mac_entry.entryState.mcEntryState = GT_MC_STATIC;
    }
    else {
	printk("mv_gateway: deleteing entry for %s (db%d): ",nc->name,db);
	mac_entry.entryState.mcEntryState = GT_MC_INVALID;
    }
    printk("%02x:%02x:%02x:%02x:%02x:%02x, ports mask 0x%x",
	mac_entry.macAddr.arEther[0],mac_entry.macAddr.arEther[1],
	mac_entry.macAddr.arEther[2],mac_entry.macAddr.arEther[3],
	mac_entry.macAddr.arEther[4],mac_entry.macAddr.arEther[5],ports_mask);

    if(gfdbAddMacEntry(qd_dev, &mac_entry) != GT_OK) {
        printk("gfdbAddMacEntry failed\n");
        return -1;
    }
    return 0;
}
#endif

static void mv_gtw_convert_str_to_mac( char *source , char *dest ) 
{
    dest[0] = (mv_gtw_str_to_hex( source[0] ) << 4) + mv_gtw_str_to_hex( source[1] );
    dest[1] = (mv_gtw_str_to_hex( source[2] ) << 4) + mv_gtw_str_to_hex( source[3] );
    dest[2] = (mv_gtw_str_to_hex( source[4] ) << 4) + mv_gtw_str_to_hex( source[5] );
    dest[3] = (mv_gtw_str_to_hex( source[6] ) << 4) + mv_gtw_str_to_hex( source[7] );
    dest[4] = (mv_gtw_str_to_hex( source[8] ) << 4) + mv_gtw_str_to_hex( source[9] );
    dest[5] = (mv_gtw_str_to_hex( source[10] ) << 4) + mv_gtw_str_to_hex( source[11] );
}
static unsigned char mv_gtw_convert_str_to_tos( char *str )
{
    /* skip on first two chars 0x */
    return( (mv_gtw_str_to_hex(str[2])<<4) + (mv_gtw_str_to_hex(str[3])) );
}
static unsigned int mv_gtw_str_to_hex( char ch ) 
{
    if( (ch >= '0') && (ch <= '9') )
        return( ch - '0' );

    if( (ch >= 'a') && (ch <= 'f') )
	return( ch - 'a' + 10 );

    if( (ch >= 'A') && (ch <= 'F') )
	return( ch - 'A' + 10 );

    return 0;
}

#define STAT_PER_Q(qnum,x)  for(queue = 0; queue < qnum; queue++) \
				printk("%10u ",x[queue]); \
			    printk("\n");

void print_mv_gtw_stat(void)
{
#ifndef EGIGA_STATISTICS
  printk(" Error: driver is compiled without statistics support!! \n");
  return;
#else
    egiga_statistics *stat = &gbe_dev.egiga_stat;
    unsigned int queue;

    printk("QUEUS:.........................");
    for(queue = 0; queue < MV_ETH_RX_Q_NUM; queue++) 
	printk( "%10d ",queue);
    printk("\n");

  if( mv_gtw_stat & EGIGA_STAT_INT ) {
      printk( "\n====================================================\n" );
      printk( "interrupt statistics" );
      printk( "\n-------------------------------\n" );
      printk( "irq_total.....................%10u\n", stat->irq_total );
      printk( "irq_none......................%10u\n", stat->irq_none );
  }
  if( mv_gtw_stat & EGIGA_STAT_POLL ) {
      printk( "\n====================================================\n" );
      printk( "polling statistics" );
      printk( "\n-------------------------------\n" );
      printk( "poll_events...................%10u\n", stat->poll_events );
      printk( "poll_complete.................%10u\n", stat->poll_complete );
  }
  if( mv_gtw_stat & EGIGA_STAT_RX ) {
      printk( "\n====================================================\n" );
      printk( "rx statistics" );
      printk( "\n-------------------------------\n" );
      printk( "rx_events................%10u\n", stat->rx_events );
      printk( "rx_hal_ok................");STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_ok);
      printk( "rx_hal_no_resource.......");STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_no_resource );
      printk( "rx_hal_no_more..........."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_no_more );
      printk( "rx_hal_error............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_error );
      printk( "rx_hal_invalid_skb......."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_invalid_skb );
      printk( "rx_hal_bad_stat.........."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_bad_stat );
      printk( "rx_netif_drop............"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_netif_drop );
  }
  if( mv_gtw_stat & EGIGA_STAT_RX_FILL ) {
      printk( "\n====================================================\n" );
      printk( "rx fill statistics" );
      printk( "\n-------------------------------\n" );
      printk( "rx_fill_events................"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_events );
      printk( "rx_fill_alloc_skb_fail........"); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_alloc_skb_fail );
      printk( "rx_fill_hal_ok................"); STAT_PER_Q(MV_ETH_RX_Q_NUM,stat->rx_fill_hal_ok);
      printk( "rx_fill_hal_full.............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_hal_full );
      printk( "rx_fill_hal_error............."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_hal_error );
      printk( "rx_fill_timeout_events........%10u\n", stat->rx_fill_timeout_events );
      printk( "rx buffer size................%10u\n", MV_MTU);
  }
  if( mv_gtw_stat & EGIGA_STAT_TX ) {
      printk( "\n====================================================\n" );
      printk( "tx statistics" );
      printk( "\n-------------------------------\n" );
      printk( "tx_events.....................%10u\n", stat->tx_events );
      printk( "tx_hal_ok.....................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_ok);
      printk( "tx_hal_no_resource............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_no_resource );
      printk( "tx_hal_no_error...............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_error );
      printk( "tx_hal_unrecognize............");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_unrecognize );
      printk( "tx_netif_stop.................");STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_netif_stop );
      printk( "tx_timeout....................%10u\n", stat->tx_timeout );
  }
  if( mv_gtw_stat & EGIGA_STAT_TX_DONE ) {
      printk( "\n====================================================\n" );
      printk( "tx-done statistics" );
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

  printk("QD CPU_PORT statistics: \n");
  print_qd_port_counters(QD_PORT_CPU);
#endif /*EGIGA_STATISTICS*/
}

void print_qd_port_counters(unsigned int port)
{
    GT_STATS_COUNTER_SET3 counters;

    printk("Getting QD counters for port %d.\n", port);

    if(gstatsGetPortAllCounters3(qd_dev,port,&counters) != GT_OK) {
    	printk("gstatsGetPortAllCounters3 failed");
	return;
    }

    printk("InGoodOctetsLo  %lu    ", counters.InGoodOctetsLo);
    printk("InGoodOctetsHi  %lu   \n", counters.InGoodOctetsHi);
    printk("InBadOctets     %lu    ", counters.InBadOctets);
    printk("OutFCSErr       %lu   \n", counters.OutFCSErr);
    printk("Deferred        %lu   \n", counters.Deferred);
    printk("InBroadcasts    %lu    ", counters.InBroadcasts);
    printk("InMulticasts    %lu   \n", counters.InMulticasts);
    printk("Octets64        %lu    ", counters.Octets64);
    printk("Octets127       %lu   \n", counters.Octets127);
    printk("Octets255       %lu    ", counters.Octets255);
    printk("Octets511       %lu   \n", counters.Octets511);
    printk("Octets1023      %lu    ", counters.Octets1023);
    printk("OctetsMax       %lu   \n", counters.OctetsMax);
    printk("OutOctetsLo     %lu    ", counters.OutOctetsLo);
    printk("OutOctetsHi     %lu   \n", counters.OutOctetsHi);
    printk("OutUnicasts     %lu    ", counters.OutOctetsHi);
    printk("Excessive       %lu   \n", counters.Excessive);
    printk("OutMulticasts   %lu    ", counters.OutMulticasts);
    printk("OutBroadcasts   %lu   \n", counters.OutBroadcasts);
    printk("Single          %lu    ", counters.OutBroadcasts);
    printk("OutPause        %lu   \n", counters.OutPause);
    printk("InPause         %lu    ", counters.InPause);
    printk("Multiple        %lu   \n", counters.InPause);
    printk("Undersize       %lu    ", counters.Undersize);
    printk("Fragments       %lu   \n", counters.Fragments);
    printk("Oversize        %lu    ", counters.Oversize);
    printk("Jabber          %lu   \n", counters.Jabber);
    printk("InMACRcvErr     %lu    ", counters.InMACRcvErr);
    printk("InFCSErr        %lu   \n", counters.InFCSErr);
    printk("Collisions      %lu    ", counters.Collisions);
    printk("Late            %lu   \n", counters.Late);

    return;
}

void print_qd_atu(void)
{
    GT_ATU_ENTRY atu_entry;
    int i;
    for(i=0;i<MV_NUM_OF_IFS;i++) {
	memset(&atu_entry,0,sizeof(GT_ATU_ENTRY));
	atu_entry.DBNum = i;
	printk("ATU DB%d display:\n",i);
	while(gfdbGetAtuEntryNext(qd_dev,&atu_entry) == GT_OK) {
	    printk("addr=%02x:%02x:%02x:%02x:%02x:%02x, ports_mask=0x%x\n",
		atu_entry.macAddr.arEther[0],atu_entry.macAddr.arEther[1],
		atu_entry.macAddr.arEther[2],atu_entry.macAddr.arEther[3],
		atu_entry.macAddr.arEther[4],atu_entry.macAddr.arEther[5],
		(unsigned int)atu_entry.portVec);
	}
    }
}


