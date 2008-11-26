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

#include "mvCommon.h"  /* Should be included before mvSysHwConfig */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <net/ip.h>
#include <net/xfrm.h>

#include "mvOs.h"
#include "dbg-trace.h"
#include "mvSysHwConfig.h"
#include "eth/mvEth.h"
#include "eth-phy/mvEthPhy.h"
#include "ctrlEnv/sys/mvSysGbe.h"

#include "mv_netdev.h"

static int __init mv_eth_init_module( void );
static void __exit mv_eth_exit_module( void );
module_init( mv_eth_init_module );
module_exit( mv_eth_exit_module);
MODULE_DESCRIPTION("Marvell Ethernet Driver - www.marvell.com");
MODULE_AUTHOR("Dmitri Epshtein <dima@marvell.com>");
MODULE_LICENSE("GPL");

extern u8 mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][6];
extern u16 mvMtu[CONFIG_MV_ETH_PORTS_NUM];

int     mv_eth_rxq_desc[MV_ETH_RX_Q_NUM];
int     mv_eth_txq_desc[MV_ETH_TX_Q_NUM];

int     mv_eth_rx_desc_total = 0;
int     mv_eth_tx_desc_total = 0;

int     mv_eth_tx_done_quota = 16;

mv_eth_priv**       mv_eth_ports;
int                 mv_eth_ports_num = 0;

struct net_device** mv_net_devs;
int                 mv_net_devs_num = 0;

spinlock_t          mii_lock = SPIN_LOCK_UNLOCKED;
spinlock_t          nfp_lock = SPIN_LOCK_UNLOCKED;

void eth_print_irq_status(mv_eth_priv *priv);

/***********************************************************************************
 ***  get device by index
 ***********************************************************************************/
static struct net_device* eth_net_device_by_idx(unsigned int idx) 
{
    if(idx >= mv_net_devs_num)    
    {
        printk("No net_device for index %d\n", idx);
        return NULL;
    }
    return mv_net_devs[idx];
}

/***********************************************************************************
 ***  get eth_priv structure by port number
 ***********************************************************************************/
static INLINE mv_eth_priv*     eth_priv_by_port(unsigned int port)
{
    if(port >= mv_eth_ports_num)    
    {
        printk("No eth device for port %d\n", port);
        return NULL;
    }
    return mv_eth_ports[port];
}


static void eth_print_link_status( struct net_device *dev ) 
{
    mv_eth_priv *priv = MV_ETH_PRIV(dev);
    int         port = priv->port;
    u32         port_status;

    port_status = MV_REG_READ( ETH_PORT_STATUS_REG( port ) );
    if(port_status & ETH_LINK_UP_MASK)
    {
	    printk( KERN_NOTICE "%s: link up", dev->name );
        
        /* check port status register */
        printk(", %s",(port_status & ETH_FULL_DUPLEX_MASK) ? "full duplex" : "half duplex" );

        if( port_status & ETH_GMII_SPEED_1000_MASK ) 
	        printk(", speed 1 Gbps" );
        else 
            printk(", %s", (port_status & ETH_MII_SPEED_100_MASK) ? "speed 100 Mbps" : "speed 10 Mbps" );
	    printk("\n" );
    }
    else {
	    printk( KERN_NOTICE "%s: link down\n", dev->name );
    }
}

static int eth_get_config(mv_eth_priv* priv, MV_U8* mac_addr)
{
    char*   mac_str = NULL;
    u8      zero_mac[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int     mtu;

    switch(priv->port)
    {
        case 0:
	    if (mvMtu[0] != 0)
		mtu = mvMtu[0];
            else
		mtu = CONFIG_MV_ETH_0_MTU;
	    /* use default MAC address from Kconfig only if the MAC address we got is all 0 */
	    if (memcmp(mvMacAddr[0], zero_mac, ETH_ALEN) == 0)
            	mac_str = CONFIG_MV_ETH_0_MACADDR; 
	    else
		memcpy(mac_addr, mvMacAddr[0], ETH_ALEN);
            break;

#if (CONFIG_MV_ETH_PORTS_NUM > 1)
        case 1:
	    if (mvMtu[1] != 0)
		mtu = mvMtu[1];
            else
		mtu = CONFIG_MV_ETH_1_MTU;
	    /* use default MAC address from Kconfig only if the MAC address we got is all 0 */
	    if (memcmp(mvMacAddr[1], zero_mac, ETH_ALEN) == 0)
            	mac_str = CONFIG_MV_ETH_1_MACADDR;
	    else
		memcpy(mac_addr, mvMacAddr[1], ETH_ALEN);
            break;
#endif /* CONFIG_MV_ETH_PORTS_NUM > 1 */

#if (CONFIG_MV_ETH_PORTS_NUM > 2)
        case 2:
	    if (mvMtu[2] != 0)
		mtu = mvMtu[2];
            else
		mtu = CONFIG_MV_ETH_2_MTU;
	    /* use default MAC address from Kconfig only if the MAC address we got is all 0 */
	    if (memcmp(mvMacAddr[2], zero_mac, ETH_ALEN) == 0)
            	mac_str = CONFIG_MV_ETH_2_MACADDR;
	    else
		memcpy(mac_addr, mvMacAddr[2], ETH_ALEN);
            break;
#endif /* CONFIG_MV_ETH_PORTS_NUM > 2 */

#if (CONFIG_MV_ETH_PORTS_NUM > 3)
        case 3:
	    if (mvMtu[3] != 0)
		mtu = mvMtu[3];
            else
		mtu = CONFIG_MV_ETH_3_MTU;
	    /* use default MAC address from Kconfig only if the MAC address we got is all 0 */
	    if (memcmp(mvMacAddr[3], zero_mac, ETH_ALEN) == 0)
            	mac_str = CONFIG_MV_ETH_3_MACADDR;
	    else
		memcpy(mac_addr, mvMacAddr[3], ETH_ALEN);
            break;
#endif /* CONFIG_MV_ETH_PORTS_NUM > 3 */

        default:
            printk("eth_get_config: Unexpected port number %d\n", priv->port);
            return -1;
    }
    if ((mac_str != NULL) && (mac_addr != NULL))
            mvMacStrToHex(mac_str, mac_addr);

    return mtu;
}

static void eth_print_rx_errors(MV_U32 pkt_status)
{
    switch(pkt_status & ETH_RX_ERROR_CODE_MASK)
    {
        case ETH_RX_CRC_ERROR:
            printk("bad rx status %08x, (crc error)", (unsigned int)pkt_status);
            break;

        case ETH_RX_OVERRUN_ERROR:
            printk("bad rx status %08x, (overrun error)", (unsigned int)pkt_status);
            break;

        case ETH_RX_MAX_FRAME_LEN_ERROR:
            printk("bad rx status %08x, (max frame length error)", (unsigned int)pkt_status);
            break;

        case ETH_RX_RESOURCE_ERROR:
            printk("bad rx status %08x, (resource error)", (unsigned int)pkt_status);
            break;
    }
    printk("\n");
}

static INLINE struct sk_buff* eth_skb_alloc(mv_eth_priv *priv, MV_PKT_INFO* pPktInfo,
                                            int mtu)
{
    int             buf_size;
    struct sk_buff  *skb;

    /* allocate new skb */
    /* 32(extra for cache prefetch) + 8 to align on 8B */
    buf_size = MV_RX_BUF_SIZE(mtu) + CPU_D_CACHE_LINE_SIZE  + 8;

    skb = dev_alloc_skb( buf_size ); 
    if( !skb ) {
        ETH_DBG( ETH_DBG_RX_FILL, ("%s: rx_fill cannot allocate skb\n", dev->name) );
        priv->eth_stat.skb_alloc_fail++;
        return NULL;
    }
    ETH_STAT(priv->eth_stat.skb_alloc_ok++);

    /* align the buffer on 8B */
    if( (unsigned long)(skb->data) & 0x7 ) {
        skb_reserve( skb, 8 - ((unsigned long)(skb->data) & 0x7) );
    }
        
    /* Most of PktInfo and BufInfo parameters left unchanged */
    pPktInfo->osInfo = (MV_ULONG)skb;
    pPktInfo->pFrags->bufSize = MV_RX_BUF_SIZE( mtu);
    pPktInfo->pktSize = pPktInfo->pFrags->bufSize;
    pPktInfo->pFrags->bufPhysAddr = mvOsIoVirtToPhy(NULL, skb->data);
    pPktInfo->pFrags->bufVirtPtr = skb->data;
	
	return skb;
} 

#ifdef CONFIG_MV_SKB_REUSE
int eth_skb_reuse_enable = MV_ETH_SKB_REUSE_DEFAULT;
static INLINE mv_eth_priv* eth_skb_reusable(struct sk_buff *skb)
{
    int i;

    if( (!eth_skb_reuse_enable) )
        return NULL;

    if( (atomic_read(&skb->users) == 1) &&
		(!skb_cloned(skb)) &&        
		(skb->data_len == 0) &&
        (skb_shinfo(skb)->nr_frags == 0) )
    {
        for(i=0; i<mv_eth_ports_num; i++) 
        {
            mv_eth_priv *priv = mv_eth_ports[i];

	        if (priv != NULL) 
            {
                int         size = MV_RX_BUF_SIZE( priv->net_dev->mtu) +
                                    CPU_D_CACHE_LINE_SIZE + 8 + NET_SKB_PAD;

                size = SKB_DATA_ALIGN(size) + sizeof(struct sk_buff);
/*
                printk("port=%d (%d): skb=%p, iff=%d, truesize=%d, size=%d, idx=%d\n", 
                            priv->port, dev->ifindex, skb, skb->iif, skb->truesize, 
                            size, mvStackIndex(priv->skbReusePool));
*/
                if( (mvStackIndex(priv->skbReusePool) > 0) &&
                    (skb->truesize >= size) )  
                {
/*                    printk("skb=%p is Reusable\n", skb);*/
                    return priv;
                }
            }
        }
    }
/*    printk("skb=%p is NOT reusable\n", skb); */
    return NULL;
}

static INLINE void  eth_skb_reset(struct sk_buff *skb)
{
    /* Taken from __kfree_skb() */
    dst_release(skb->dst);
    skb->dst = NULL;

#ifdef CONFIG_XFRM
    secpath_put(skb->sp);
#endif /* CONFIG_XFRM */ 

    if (skb->destructor) {
            WARN_ON(in_irq());
            skb->destructor(skb);
            skb->destructor = NULL;
    }

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
    if(skb->nfct)
    {
        nf_conntrack_put(skb->nfct);
        skb->nfct = 0;
    }
    if(skb->nfct_reasm)
    {
        nf_conntrack_put_reasm(skb->nfct_reasm);
        skb->nfct_reasm = 0;
    }
#endif /* CONFIG_NF_CONNTRACK || CONFIG_NF_CONNTRACK_MODULE */

#ifdef CONFIG_BRIDGE_NETFILTER
        nf_bridge_put(skb->nf_bridge);
        skb->nf_bridge = 0;
#endif /* CONFIG_BRIDGE_NETFILTER */
/* XXX: IS this still necessary? - JHS */
#ifdef CONFIG_NET_SCHED
        skb->tc_index = 0;
#ifdef CONFIG_NET_CLS_ACT
        skb->tc_verd = 0;
#endif /* CONFIG_NET_CLS_ACT */
#endif /* CONFIG_NET_SCHED */

    skb->pkt_type = 0; 
    skb->iif = 0;
    skb->sk = 0; 
    skb->len = 0;

    skb->data = skb->tail = skb->head + NET_SKB_PAD;

    /* align the buffer on 8B */
    if( (unsigned long)(skb->data) & 0x7 ) {
        skb_reserve( skb, 8 - ((unsigned long)(skb->data) & 0x7) );
    }
}

static INLINE void eth_skb_alloc_for_reuse(mv_eth_priv *priv, int mtu)
{
    int             idx;
    MV_PKT_INFO     *pPktInfo = NULL;
    struct sk_buff  *skb;

    idx = mvStackIndex(priv->skbReusePool);
    while(idx > 0)
    {
        ETH_STAT(priv->eth_stat.skb_reuse_alloc++);
        pPktInfo = (MV_PKT_INFO*)mvStackPop(priv->skbReusePool);
        idx--;

        skb = eth_skb_alloc(priv, pPktInfo, mtu);
        if(skb == NULL)
        {
            /* return PktInfo to Pool */
            mvStackPush(priv->skbReusePool, (MV_U32)pPktInfo);
            break;
        }
        mvStackPush(priv->fpRxPool, (MV_U32)pPktInfo);
    }
}

#endif /* CONFIG_MV_SKB_REUSE */

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
static INLINE MV_PKT_INFO* eth_pkt_info_get(mv_eth_priv  *priv)
{
    MV_PKT_INFO*    pkt_info = NULL;

    if(mvStackIndex(priv->fpRxPool) > 0)
        pkt_info = (MV_PKT_INFO*)mvStackPop(priv->fpRxPool);

    return pkt_info;
}
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

static INLINE void  eth_pkt_info_free(mv_eth_priv *priv, MV_PKT_INFO* pPktInfo)
{
#ifdef CONFIG_MV_ETH_NFP
    if(pPktInfo->srcIdx != (char)-1)
    {
        mv_eth_priv  *src_priv = mv_eth_ports[(int)pPktInfo->srcIdx];

        /* Return to the NFP pool */
#ifdef CONFIG_MV_GATEWAY
        if(!(priv->isGtw))
		    pPktInfo->pFrags->bufPhysAddr -= ETH_MV_HEADER_SIZE;
#else
        pPktInfo->pFrags->bufPhysAddr -= ETH_MV_HEADER_SIZE;
#endif /* CONFIG_MV_GATEWAY */

        mvStackPush(src_priv->fpRxPool, (MV_U32)pPktInfo);
	    return;
    }
#endif /* CONFIG_MV_ETH_NFP */

    if(pPktInfo->osInfo)
    {
        struct sk_buff *skb = (struct sk_buff *)pPktInfo->osInfo;

#ifdef CONFIG_MV_SKB_REUSE
        mv_eth_priv  *rx_priv = eth_skb_reusable(skb);

        if( rx_priv != NULL)
        {
            MV_PKT_INFO*    pkt_info = NULL;

            ETH_STAT(rx_priv->eth_stat.skb_reuse_tx++);
            pkt_info = (MV_PKT_INFO*)mvStackPop(rx_priv->skbReusePool);
            eth_skb_reset(skb);

            pkt_info->osInfo = (MV_ULONG)skb;       
            pkt_info->pktSize = pkt_info->pFrags->bufSize;
            pkt_info->pFrags->bufPhysAddr = mvOsIoVirtToPhy(NULL, skb->data);
            pkt_info->pFrags->bufVirtPtr = skb->data;

            mvStackPush(rx_priv->fpRxPool, (MV_U32)pkt_info);
            pPktInfo->osInfo = 0;
            mvStackPush(priv->txPktInfoPool, (MV_U32)pPktInfo);
            return;
        }
#endif /* CONFIG_MV_SKB_REUSE */

        dev_kfree_skb_any(skb);
        ETH_STAT(priv->eth_stat.skb_free_ok++);
        pPktInfo->osInfo = 0;
    }
    mvStackPush(priv->txPktInfoPool, (MV_U32)pPktInfo);
}
/**************************************************************************************************************/


#ifdef ETH_MV_TX_EN
void    eth_tx_en_config(int port, int value)
{
    mv_eth_priv         *priv = eth_priv_by_port(port);

    if (priv == NULL)
    {
        printk("eth_tx_en_config: wrong port number %d\n", port);
	    return;
    }

    if(value == 0)
    {
        /* Disable TX_EN support */
        priv->tx_en = MV_FALSE;
    }
    else
    {
        priv->tx_en_deep = value;
        priv->tx_en = MV_TRUE;
    }
    printk("eth%d: TxEnable WA - %s, deep=%d\n\n", 
            priv->port, priv->tx_en ? "Enabled" : "Disabled", priv->tx_en_deep);    
}

static INLINE void  eth_tx_enable(mv_eth_priv *priv, int queue)
{
    int             ret;

    ret = mvEthPortTxEnable(priv->hal_priv, queue, priv->tx_en_deep);
    if(ret < 0)
    {
        ETH_STAT(priv->eth_stat.tx_en_busy++);
    }
    else 
    {
        if(ret > 0)
        {
            ETH_STAT(priv->eth_stat.tx_en_wait++);
            ETH_STAT(priv->eth_stat.tx_en_wait_count += ret);
        }        
        else
        {
            ETH_STAT(priv->eth_stat.tx_en_done++);
        }
    }
}
#endif /* ETH_MV_TX_EN */
/**************************************************************************************************************/



/***********************************************************
 * mv_eth_down_internals --                                 *
 *   down port rx/tx activity. free skb's from rx/tx rings.*
 ***********************************************************/
int     mv_eth_down_internals( struct net_device *dev )
{
    mv_eth_priv     *priv = MV_ETH_PRIV(dev);
    MV_PKT_INFO     *pPktInfo;
    unsigned int    queue;

    /* stop the port activity, mask all interrupts */
    if( mvEthPortDown( priv->hal_priv ) != MV_OK ) {
        printk( KERN_ERR "%s: ethPortDown failed\n", dev->name );
        goto error;
    }

    /* free the skb's in the hal tx ring */
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++) 
    {
        while( (pPktInfo = mvEthPortForceTxDone(priv->hal_priv, queue)) ) 
        {
            eth_pkt_info_free(priv, pPktInfo);
        }
        priv->tx_count[queue] = 0;
    }
    
    return 0;

error:
    printk( KERN_ERR "%s: stop internals failed\n", dev->name );
    return -1;
}
/**************************************************************************************************************/

static void eth_check_link_status(struct net_device *dev)
{
    mv_eth_priv     *priv = MV_ETH_PRIV(dev);
    MV_U32          port_status = 0;

    priv->eth_stat.link_events++;

    /* Check Link status on ethernet port */
    port_status = MV_REG_READ( ETH_PORT_STATUS_REG( priv->port ) );

    spin_lock(priv->lock);

    if(port_status & ETH_LINK_UP_MASK) { 

#ifdef ETH_MV_TX_EN
        /* Disable TX Enable WA if one of Giga ports is Half Duplex or 10 Mbps */
        if( ((port_status & ETH_FULL_DUPLEX_MASK) ||
             (port_status & ETH_GMII_SPEED_1000_MASK) ||
             (port_status & ETH_MII_SPEED_100_MASK)) )
        {
            priv->tx_en = priv->tx_en_bk;
        }
        else
        {
            priv->tx_en_bk = priv->tx_en;
            priv->tx_en = MV_FALSE;
        }
#endif /* ETH_MV_TX_EN */

        mvEthPortUp( priv->hal_priv );
        netif_carrier_on( dev );
        netif_wake_queue( dev );            
    }
    else
    {
        netif_carrier_off( dev );
        netif_stop_queue( dev );
        mv_eth_down_internals( dev );
    }
    spin_unlock(priv->lock);
   	eth_print_link_status( dev );
}
/**************************************************************************************************************/

static INLINE int   eth_rx_policy(mv_eth_priv *priv)
{
    u32     rxq_map;

    rxq_map = ( ( (priv->picr & ETH_RXQ_MASK) >> ETH_CAUSE_RX_READY_OFFSET) | 
		        ( (priv->picr & ETH_RXQ_RES_MASK) >> ETH_CAUSE_RX_ERROR_OFFSET) );
    
    return (fls(rxq_map) - 1);
}
static INLINE int   eth_tx_policy(mv_eth_priv *priv, struct sk_buff *skb)
{
    int     queue;

#if (MV_ETH_TX_Q_NUM > 1)
    if( ip_hdr(skb) )
    {
        /* get queue from TOS value */
        char prio;

        /* Map higher values of PRECEDENCE field to existing TX queues */
        prio = ip_hdr(skb)->tos >> 5;
        if(prio < (8 - MV_ETH_TX_Q_NUM))
            queue = 0;
        else
            queue = prio - (8 - MV_ETH_TX_Q_NUM);
    }
    else
#endif /* (MV_ETH_TX_Q_NUM > 1) */
    {
        /* no multiqueue. all packets go to one default queue. */
        queue = ETH_DEF_TXQ;
    }

    return queue;
}
/**************************************************************************************************************/



/*********************************************************** 
 * eth_tx_done --                                             *
 *   release transmitted packets. interrupt context.       *
 ***********************************************************/
static INLINE u32 eth_tx_done(mv_eth_priv *priv, int queue)
{
    MV_PKT_INFO*            pPktInfo;
    u32                     count = 0;

    ETH_DBG( ETH_DBG_TX_DONE, ("eth%s: tx-done ", priv->port) );
    priv->eth_stat.tx_done_events++;

    /* release the transmitted packets */
    while( 1 ) 
    {
        /* get a packet */ 
        pPktInfo = mvEthPortTxDone(priv->hal_priv, queue);
        if(pPktInfo) 
        {
             /* handle tx error */
            if( pPktInfo->status & (ETH_ERROR_SUMMARY_MASK) ) {
                ETH_DBG( ETH_DBG_TX_DONE, ("eth%d: bad tx-done status\n", priv->port) );
                priv->net_dev->stats.tx_errors++;
            }

            count++;
            ETH_STAT( priv->eth_stat.tx_done_hal_ok[queue]++);

            eth_pkt_info_free(priv, pPktInfo);
            continue;
        }
        else 
        {
            /* no more work */
            break;
        }
    }
#if (MV_ETH_TX_Q_NUM == 1)
    /* if transmission was previously stopped, now it can be restarted. */
    if( netif_queue_stopped( priv->net_dev ) && (priv->net_dev->flags & IFF_UP) && (count > 0) ) {
        ETH_DBG( ETH_DBG_TX_DONE, ("%s: restart transmit\n", priv->net_dev->name) );
        ETH_STAT( priv->eth_stat.tx_done_netif_wake++);
        netif_wake_queue( priv->net_dev );    
    }
#endif /* MV_ETH_TX_Q_NUM == 1 */

    ETH_STAT( priv->eth_stat.tx_done_dist[count]++);
    ETH_DBG( ETH_DBG_TX_DONE, ("eth%d: tx-done %d\n", priv->port, count) );
    return count;
}
/**************************************************************************************************************/

static INLINE int eth_rx(struct net_device *dev, unsigned int work_to_do, int queue)
{
    mv_eth_priv             *priv = MV_ETH_PRIV(dev);
    struct net_device_stats	*stats = MV_NETDEV_STATS(dev);
    struct sk_buff          *skb;
    MV_PKT_INFO             *pPktInfo;
    int                     work_done = 0;
    MV_STATUS               status;
    struct net_device       *out_dev = NULL;
    MV_U32		            rx_status; 
#ifdef CONFIG_MV_ETH_NFP
    int			            out_if_index;
    MV_IP_HEADER            *pIpHdr;
    int                     nfp = fp_is_enabled(); 
#endif /* CONFIG_MV_ETH_NFP */

    ETH_DBG( ETH_DBG_RX, ("%s: rx_poll work_to_do %d\n", dev->name, work_to_do) );

    /* fairness NAPI loop */
    while( work_done < work_to_do ) {

        /* get rx packet */ 
        pPktInfo = mvEthPortRx( priv->hal_priv, queue);
        /* check status */
        if( pPktInfo == NULL) 
        {
            /* no more rx packets ready */
            break;
        }
        prefetch(pPktInfo->pFrags->bufVirtPtr);

	    rx_status = pPktInfo->status;
	    if (rx_status  & (ETH_ERROR_SUMMARY_MASK)) 
	    {
            eth_print_rx_errors(rx_status);
	        mvEthPortRxDone(priv->hal_priv, queue, pPktInfo);
            stats->rx_errors++;
            continue;
	    }

        work_done++;
        ETH_STAT( priv->eth_stat.rx_hal_ok[queue]++);
        /* good rx */

        skb = (struct sk_buff *)( pPktInfo->osInfo );
        ETH_DBG( ETH_DBG_RX, ("good rx: skb=%p, skb->data=%p\n", skb, skb->data) );

#ifdef CONFIG_MV_GATEWAY
        if(priv->isGtw)
        {
	        dev = mv_gtw_get_rx_dev(skb, NULL, NULL);
	        stats = MV_NETDEV_STATS(dev);
        }
#endif /* CONFIG_MV_GATEWAY */
	
        stats->rx_packets++;
        stats->rx_bytes += pPktInfo->pFrags->dataSize - ETH_MV_HEADER_SIZE;

#ifdef CONFIG_MV_ETH_NFP
	    if (nfp)
	    {
            int txq = ETH_DEF_TXQ;

            out_dev = NULL;
            pIpHdr = mvFpParsing(pPktInfo, &priv->fpStats);
            if(pIpHdr != NULL)
            {
                out_if_index = mvFpProcess(dev->ifindex, pPktInfo->pFrags->bufVirtPtr, pIpHdr, &priv->fpStats);
                if(out_if_index != -1)
                {
                    out_dev = fp_mgr_get_net_dev(out_if_index);
                    if(fp_mgr_get_if_type(out_if_index) == MV_FP_IF_INT)
                    {
                	    mv_eth_priv    *out_priv = MV_ETH_PRIV(out_dev);
#if defined(CONFIG_MV_GATEWAY)
                        if(out_priv->isGtw)
                        {
				            struct mv_vlan_cfg* vlan_cfg = MV_NETDEV_VLAN(out_dev);
				            *(unsigned short *)(pPktInfo->pFrags->bufVirtPtr) = vlan_cfg->header;
                        }
                        else
#endif /* CONFIG_MV_GATEWAY */
                        {
				            pPktInfo->pFrags->bufPhysAddr += ETH_MV_HEADER_SIZE;
				            pPktInfo->pFrags->dataSize -= ETH_MV_HEADER_SIZE;
                        }
                        mvOsCacheLineFlushInv(NULL, pPktInfo->pFrags->bufVirtPtr);	
                        mvOsCacheLineFlushInv(NULL, pPktInfo->pFrags->bufVirtPtr 
                                                        + CPU_D_CACHE_LINE_SIZE);

            	        spin_lock(out_priv->lock);

            	        status = mvEthPortTx(out_priv->hal_priv, txq, pPktInfo);
            	        if( status != MV_OK ) 
            	        {
				            out_dev->stats.tx_dropped++;
				            spin_unlock(out_priv->lock);
#if defined(CONFIG_MV_GATEWAY)
				            if(!(out_priv->isGtw))
#endif /* CONFIG_MV_GATEWAY */
					            pPktInfo->pFrags->bufPhysAddr -= ETH_MV_HEADER_SIZE;

				            mvEthPortRxDone( priv->hal_priv, queue, pPktInfo );
				            continue;
			            }
                	    out_priv->tx_count[txq]++;
                	    out_dev->stats.tx_packets++;
                	    out_dev->stats.tx_bytes += pPktInfo->pFrags->dataSize;
                	    ETH_STAT( out_priv->eth_stat.tx_hal_ok[txq]++);

                	    spin_unlock(out_priv->lock);
                	    /* refill RX queue */
                	    pPktInfo = eth_pkt_info_get(priv);
			            if (pPktInfo != NULL)
                	        mvEthPortRxDone( priv->hal_priv, queue, pPktInfo );
			            else
			            {
                            priv->eth_stat.rx_pool_empty++;
			                priv->refill_needed_flag = 1;
			            }
                	    continue;
                    }
                }
            }
	    }
#endif /* CONFIG_MV_ETH_NFP */

	    /* skip on 2B Marvell-header */
	    skb_reserve(skb, ETH_MV_HEADER_SIZE);
	    skb_put(skb, pPktInfo->pFrags->dataSize - ETH_MV_HEADER_SIZE);

        if(out_dev != NULL)
        {
            /* Send to external net_device */
            out_dev->hard_start_xmit(skb, out_dev);
        }
        else
        {
            skb->dev = dev;

            skb->ip_summed = CHECKSUM_NONE;
            skb->protocol = eth_type_trans(skb, dev);

#ifdef RX_CSUM_OFFLOAD
	    if (((pPktInfo->pFrags->dataSize + 4) > ETH_CSUM_MIN_BYTE_COUNT)  && 
		    (rx_status & ETH_RX_IP_FRAME_TYPE_MASK) && 
		    (rx_status & ETH_RX_IP_HEADER_OK_MASK)) {  
			if (!(pPktInfo->fragIP)		&&
			   (!(rx_status & ETH_RX_L4_OTHER_TYPE)) &&
			   (rx_status & ETH_RX_L4_CHECKSUM_OK_MASK)) { 
				skb->csum = 0;
				skb->ip_summed = CHECKSUM_UNNECESSARY;	
				ETH_STAT(priv->eth_stat.rx_csum_hw++);
			}
			else if (pPktInfo->fragIP && (rx_status & ETH_RX_L4_UDP_TYPE)) {	/* TBD: UDP only */
				skb->csum = ntohl(0xFFFF ^ ((rx_status & ETH_RX_L4_CHECKSUM_MASK) >> ETH_RX_L4_CHECKSUM_OFFSET));
				skb->ip_summed = CHECKSUM_COMPLETE;
				ETH_STAT( priv->eth_stat.rx_csum_hw_frags++);
			}
            else
            {
                ETH_STAT(priv->eth_stat.rx_csum_sw++);
            }
	    }
        else {
            ETH_DBG( ETH_DBG_RX, ("%s: no RX csum offload\n", dev->name) );
		    ETH_STAT(priv->eth_stat.rx_csum_sw++);
        }
#endif /* RX_CSUM_OFFLOAD*/

#ifdef ETH_INCLUDE_LRO
	    status = 0; /* FIXME: LRO receive is void */

	    /* non-fragmented TCP */
	    if (!(pPktInfo->fragIP || (rx_status & ETH_RX_L4_OTHER_TYPE) || (rx_status & ETH_RX_L4_UDP_TYPE)))
		lro_receive_skb(&priv->lro_mgr, skb, priv);
	    else
#endif
#ifdef ETH_INCLUDE_LRO_UDP
		/* fragmented UDP */
		if (pPktInfo->fragIP && (rx_status & ETH_RX_L4_UDP_TYPE))
		    lro_receive_skb(&priv->lro_mgr, skb, priv);
		else 
#endif			
            status = netif_receive_skb(skb);
            ETH_STAT( if(status) (priv->eth_stat.rx_netif_drop++) );

#ifdef CONFIG_MV_SKB_REUSE
            if( eth_skb_reuse_enable && 
            	( !mvStackIsFull(priv->skbReusePool)) )
            {
            	ETH_STAT(priv->eth_stat.skb_reuse_rx++);
            	mvStackPush(priv->skbReusePool, (MV_U32)pPktInfo);
            	/* refill RX queue */
            	pPktInfo = eth_pkt_info_get(priv);
            	if(pPktInfo != NULL)
                    mvEthPortRxDone( priv->hal_priv, queue, pPktInfo );
                else
                {
                    priv->eth_stat.rx_pool_empty++;
			        priv->refill_needed_flag = 1;
                }
            	continue;
            }
#endif /* CONFIG_MV_SKB_REUSE */
	    }

        /* Refill this buffer */
        skb = eth_skb_alloc(priv, pPktInfo, dev->mtu);
        if(skb == NULL)
            continue;

        /* give the buffer to hal */
        status = mvEthPortRxDone(priv->hal_priv, queue, pPktInfo);
        if( (status != MV_OK) && (status != MV_FULL) )
        {
            printk( KERN_ERR "%s: error in rx-fill, status=%d\n", dev->name, status );
        }
        ETH_STAT( priv->eth_stat.rx_fill_ok[queue]++);
    }

    ETH_STAT( priv->eth_stat.rx_dist[work_done]++); 
    ETH_DBG( ETH_DBG_RX, ("\nwork_done %d", work_done));

    /* notify upper layer about more work to do */
    return work_done;
}
/**************************************************************************************************************/


/*********************************************************** 
 * eth_rx_fill --                                        *
 *   fill new rx buffers to ring.                          *
 ***********************************************************/
MV_STATUS   eth_rx_fill(mv_eth_priv *priv, int pool_size, int mtu)
{
    MV_PKT_INFO     *pPktInfo;
    MV_BUF_INFO     *pBufInfo;
    struct sk_buff  *skb;
    u32             count;
    MV_STATUS       status;
    int             rxq = 0;

	count = 0;
    while(pool_size > count) {

    /* First of all fill all RX QUEUEs */
        for(; rxq<MV_ETH_RX_Q_NUM; rxq++)
        {
            if( mvEthRxResourceGet(priv->hal_priv, rxq) < mv_eth_rxq_desc[rxq] )
            {
                break;
            }       
        }
#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
#else
        if(rxq >= MV_ETH_RX_Q_NUM)
            return MV_OK;
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

        pPktInfo = mvOsMalloc(sizeof(MV_PKT_INFO));
        if(pPktInfo == NULL)
        {
            printk("eth%d: Can't allocate %d bytes for MV_PKT_INFO\n",
                        priv->port, sizeof(MV_PKT_INFO));
            return MV_OUT_OF_CPU_MEM;
        }

        pBufInfo = mvOsMalloc(sizeof(MV_BUF_INFO));
        if(pBufInfo == NULL)
        {
            printk("eth%d: Can't allocate %d bytes for MV_BUF_INFO\n",
                        priv->port, sizeof(MV_BUF_INFO));
            mvOsFree(pPktInfo);
            return MV_OUT_OF_CPU_MEM;
        }

        pBufInfo->dataSize = 0;
        pPktInfo->pFrags = pBufInfo;
        pPktInfo->numFrags = 1;
        pPktInfo->status = 0;
        pPktInfo->srcIdx = (char)priv->port;

        skb = eth_skb_alloc(priv, pPktInfo, mtu);
        if(skb == NULL)
        {
            mvOsFree(pPktInfo);
            mvOsFree(pBufInfo);
            return MV_OUT_OF_CPU_MEM;
        }
        count++;

        if(rxq < MV_ETH_RX_Q_NUM)
        {
            status = mvEthPortRxDone(priv->hal_priv, rxq, pPktInfo);
            if( (status != MV_OK) && (status != MV_FULL) )
            {
               printk("rx_fill: q=%d, resource=%d, status=%d\n", 
                               rxq, mvEthRxResourceGet(priv->hal_priv, rxq), status);
            }
		    ETH_STAT( priv->eth_stat.rx_fill_ok[rxq]++);
            continue;
        }

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
        /* Push extra RX buffers to fpRxPool */
        mvStackPush(priv->fpRxPool, (MV_U32)pPktInfo);
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE*/
    }
    return MV_OK;
}


/*********************************************************** 
 * mv_eth_interrupt_handler --                              *
 *   serve rx-q0, tx-done-q0, phy/link state change.       *
 *   phy is served in interrupt context.           *
 *   tx and rx are scheduled out of interrupt context (NAPI poll)  *
 ***********************************************************/
irqreturn_t     mv_eth_interrupt_handler(int irq , void *dev_id)
{
    struct net_device *dev = (struct net_device *)dev_id;
    mv_eth_priv *priv = MV_ETH_PRIV(dev);
 
    ETH_DBG( ETH_DBG_INT, ("\n%s: isr ", dev->name) );
    priv->eth_stat.irq_total++;

    /* read port interrupt cause register */
    mv_eth_save_interrupts(priv);

    ETH_DBG(ETH_DBG_INT, ("%s: mv_eth_interrupt_handler: picr=%x, picer=%x\n", 
                            dev->name, priv->picr, priv->picer));

#ifdef ETH_DEBUG
    if( !priv->picr && !priv->picer) {
        priv->eth_stat.irq_none++;
        return IRQ_HANDLED;
    }
#endif /* ETH_DEBUG */

    /* Verify that the device not already on the polling list */
    if (netif_rx_schedule_prep(dev)) {
	    /* schedule the work (rx+txdone+link) out of interrupt contxet */
        mv_eth_mask_interrupts(priv);
	    __netif_rx_schedule(dev);
    }
    else {
	    if(netif_running(dev)) {
            priv->eth_stat.irq_while_polling++;
#ifdef ETH_DEBUG
            printk("%s: Interrupt while in polling list\n", dev->name);
	        eth_print_irq_status(priv);
#endif /* ETH_DEBUG */
	    }
    }
    mv_eth_clear_saved_interrupts(priv);

    return IRQ_HANDLED;
}


static int eth_poll( struct net_device *dev, int *budget )
{
    int             rx_done=0, rx_todo;
    mv_eth_priv     *priv = MV_ETH_PRIV(dev);

    priv->eth_stat.poll_events++;
    ETH_DBG(ETH_DBG_POLL, ("%s: Start eth_poll\n", dev->name));

    if( priv->picer & (1 << ETH_CAUSE_LINK_STATE_CHANGE_BIT) ) 
    {
        eth_check_link_status(dev);
        priv->picer &= ~ETH_LINK_MASK;
    }

#ifdef ETH_TX_DONE_ISR
    {
        int txq;

        spin_lock(priv->lock);
        for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
        {
#ifdef ETH_MV_TX_EN
            if(priv->tx_en)
                eth_tx_enable(priv, txq);
#endif /* ETH_MV_TX_EN */
            priv->tx_count[txq] -= eth_tx_done(priv, txq);
        }
        spin_unlock(priv->lock);
    }
#endif /* ETH_TX_DONE_ISR */

    rx_todo = min(*budget, dev->quota);

#if (MV_ETH_RX_Q_NUM > 1)
    while(MV_TRUE)
    {
        int rxq;

        rxq = eth_rx_policy(priv);
        if(rxq == -1)
            break;

        rx_done += eth_rx( dev, rx_todo - rx_done, rxq);
        if(rx_done < rx_todo)
            priv->picr &= ~(ETH_CAUSE_RX_READY_MASK(rxq) | 
                            ETH_CAUSE_RX_ERROR_MASK(rxq));
        else
            break;
    }
#else
    rx_done = eth_rx( dev, rx_todo, ETH_DEF_RXQ);
#endif /* (MV_ETH_RX_Q_NUM > 1) */

    *budget -= rx_done;
    dev->quota -= rx_done;

#if defined(CONFIG_MV_ETH_NFP) && !defined(ETH_TX_DONE_ISR)
    if( fp_is_enabled() )
    {
        int i, txq, tx_done_count=0;

        for(i=0; i<mv_eth_ports_num; i++) 
        {
          	mv_eth_priv  *tx_priv = mv_eth_ports[i];
	        if (tx_priv != NULL) {
		
	       	    spin_lock(tx_priv->lock);

#ifdef ETH_MV_TX_EN
                if( tx_priv->tx_en )
                {
                    for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
                    {
                        if(tx_priv->tx_count[txq] > 0)
                            eth_tx_enable(tx_priv, txq);
                    }
                }
#endif /* ETH_MV_TX_EN */

                for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
                {
            	    if(tx_priv->tx_count[txq] > mv_eth_tx_done_quota)
            	    {
                        tx_done_count = eth_tx_done(tx_priv, txq);
                        tx_priv->tx_count[txq] -= tx_done_count;
                    } 
                }
               	spin_unlock(tx_priv->lock);
	        }
        }
    }
#endif /* CONFIG_MV_ETH_NFP && !ETH_TX_DONE_ISR */

#ifdef ETH_INCLUDE_LRO
    if(rx_done)
        lro_flush_all(&priv->lro_mgr);
#endif	

    ETH_DBG( ETH_DBG_POLL, ("poll work done: rx-%d\n", rx_done) );

    if ( (!netif_running(dev)) || (rx_done < rx_todo) )
    { 
        unsigned long flags;
        local_irq_save(flags);
        netif_rx_complete(dev);
        priv->eth_stat.poll_complete++;
        mv_eth_unmask_interrupts(priv);
	    ETH_DBG( ETH_DBG_RX, ("unmask\n") );
        local_irq_restore(flags);
        return 0;
    }
    else
    {
#if (MV_ETH_RX_Q_NUM > 1)
        /* Leave in NAPI context, so update picr and picer */
        mv_eth_save_interrupts(priv);
	    mv_eth_clear_saved_interrupts(priv);
#endif /* (MV_ETH_RX_Q_NUM > 1) */
    }
    return 1;
}


/* Show network driver configuration */
void	mv_netdev_config_show(void)
{
    int     i;

    printk( "  o %s\n", ETH_DESCR_CONFIG_STR );

#if defined(ETH_DESCR_IN_SRAM)
    printk( "  o %s\n", INTEG_SRAM_CONFIG_STR );
#endif

    printk( "  o %s\n", ETH_SDRAM_CONFIG_STR );

#if defined(ETH_INCLUDE_TSO) && !defined(TX_CSUM_OFFLOAD)
#error "If TSO enabled - TX checksum offload must be enabled too"
#endif

#if (MV_ETH_RX_Q_NUM > 1)
    printk( "  o Multi RX Queue support - %d RX queues\n", MV_ETH_RX_Q_NUM);
#else
    printk( "  o Single RX Queue support - ETH_DEF_RXQ=%d\n", ETH_DEF_RXQ);
#endif /* MV_ETH_RX_Q_NUM > 1 */

#if (MV_ETH_TX_Q_NUM > 1)
    printk( "  o Multi TX Queue support - %d TX Queues\n", MV_ETH_TX_Q_NUM);
#else
    printk( "  o Single TX Queue support - ETH_DEF_TXQ=%d\n", ETH_DEF_TXQ);
#endif /* MV_ETH_TX_Q_NUM > 1 */

#if defined(ETH_INCLUDE_TSO)
    printk("  o TCP segmentation offload enabled\n");
#endif /* ETH_INCLUDE_TSO */

#if defined(ETH_INCLUDE_UFO)
    printk("  o UDP fragmentation offload enabled\n");
#endif /* ETH_INCLUDE_UFO */

#if defined(RX_CSUM_OFFLOAD)
    printk("  o Receive checksum offload enabled\n");
#endif
#if defined(TX_CSUM_OFFLOAD)
    printk("  o Transmit checksum offload enabled\n");
#endif

#ifdef CONFIG_MV_ETH_NFP
    printk("  o Network Fast Processing (Routing) supported\n");

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
    printk("  o Network Fast Processing (NAT) supported\n");
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#endif /* CONFIG_MV_ETH_NFP */

#ifdef ETH_STATISTICS
    printk("  o Driver statistics enabled\n");
#endif

#ifdef ETH_DEBUG
    printk("  o Driver debug messages enabled\n");
#endif

#ifdef CONFIG_MV_ETH_PROC
    printk("  o Proc tool API enabled\n");
#endif

#ifdef ETH_INCLUDE_LRO
    printk("  o LRO TCP support enabled\n");
#endif

#ifdef ETH_INCLUDE_LRO_UDP
    printk("  o LRO UDP support enabled\n");
#endif

#if defined(CONFIG_MV_GATEWAY)
    printk("  o Gateway support enabled\n");

    printk("     o Using Marvell Header Mode\n");

#ifdef CONFIG_MV_GTW_QOS_VOIP
    printk("     o VoIP QoS support (ToS %s)\n", CONFIG_MV_GTW_QOS_VOIP_TOS);
#endif

#ifdef CONFIG_MV_GTW_QOS_ROUTING
    printk("     o Routing QoS support (ToS %s)\n", CONFIG_MV_GTW_QOS_ROUTING_TOS);
#endif

#ifdef CONFIG_MV_GTW_IGMP
    printk("     o L2 IGMP support\n");
#endif /* CONFIG_MV_GTW_IGMP */

#endif /* CONFIG_MV_GATEWAY */

#if MV_ETH_RX_Q_NUM > MV_ETH_MAX_RXQ
#error "MV_ETH_RX_Q_NUM is large than MV_ETH_MAX_RXQ"
#endif 

    printk("  o Rx descripors:");
    for(i=0; i<MV_ETH_RX_Q_NUM; i++) {
        printk(" q%d=%-3d", i, mv_eth_rxq_desc[i]);
    }
    printk("\n");

#if MV_ETH_TX_Q_NUM > MV_ETH_MAX_TXQ
#error "MV_ETH_TX_Q_NUM is large than MV_ETH_MAX_TXQ"
#endif 

    printk("  o Tx descripors:");
    for(i=0; i<MV_ETH_TX_Q_NUM; i++) {
        printk(" q%d=%-3d", i, mv_eth_txq_desc[i]);
    }
    printk("\n");
}

void    mv_netdev_set_features(struct net_device *dev)
{
    dev->features = NETIF_F_SG;

#ifdef TX_CSUM_OFFLOAD
        if(dev->mtu <= ETH_CSUM_MAX_BYTE_COUNT) 
        {
            dev->features |= NETIF_F_IP_CSUM;
        }
#endif /* TX_CSUM_OFFLOAD */

#ifdef ETH_INCLUDE_TSO
    if(dev->features & NETIF_F_IP_CSUM)
	    dev->features |= NETIF_F_TSO;
#endif /* ETH_INCLUDE_TSO */

#ifdef ETH_INCLUDE_UFO
    /* FIXME: HW_CSUM is required by dev.c */
    if(dev->features & NETIF_F_IP_CSUM)
        dev->features |= NETIF_F_UFO | NETIF_F_HW_CSUM; 
#endif /* ETH_INCLUDE_UFO */
}

/*********************************************************** 
 * mv_eth_start_internals --                                *
 *   fill rx buffers. start rx/tx activity. set coalesing. *
 *   clear and unmask interrupt bits                       *
 ***********************************************************/
int     mv_eth_start_internals(mv_eth_priv *priv, int mtu)
{
    unsigned long flags;
    unsigned int status;

    spin_lock_irqsave( priv->lock, flags); 

    /* fill rx ring with buffers */
    eth_rx_fill(priv, (mv_eth_rx_desc_total*2 + mv_eth_tx_desc_total), mtu);

    mv_eth_clear_interrupts(priv);

    /* start the hal - rx/tx activity */
    status = mvEthPortEnable( priv->hal_priv );
    if( (status != MV_OK) && (status != MV_NOT_READY)) {
        printk( KERN_ERR "eth%d: ethPortEnable failed\n", priv->port);
        spin_unlock_irqrestore( priv->lock, flags);
        return -1;
    }

    /* set tx/rx coalescing mechanism */
    mvEthTxCoalSet( priv->hal_priv, ETH_TX_COAL );
    mvEthRxCoalSet( priv->hal_priv, ETH_RX_COAL );

    spin_unlock_irqrestore( priv->lock, flags);

    return 0;
}

/*********************************************************** 
 * mv_eth_stop_internals --                                 *
 *   stop port rx/tx activity. free skb's from rx/tx rings.*
 ***********************************************************/
int     mv_eth_stop_internals(mv_eth_priv *priv)
{
    MV_PKT_INFO     *pPktInfo;
    unsigned int    queue;

    /* stop the port activity, mask all interrupts */
    if( mvEthPortDisable( priv->hal_priv ) != MV_OK ) {
        printk( KERN_ERR "eth%d: ethPortDisable failed\n", priv->port );
        goto error;
    }
    
    /* clear all ethernet port interrupts */
    mv_eth_clear_interrupts(priv);

    mv_eth_mask_interrupts(priv);

    /* free the skb's in the hal tx ring and release memory */
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        while( (pPktInfo = mvEthPortForceTxDone(priv->hal_priv, queue)) ) 
        {
            eth_pkt_info_free(priv, pPktInfo);
        }
        priv->tx_count[queue] = 0;
    }

    /* free the skb's in the hal rx ring */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {    
        while( (pPktInfo = mvEthPortForceRx( priv->hal_priv, queue)) ) {
            dev_kfree_skb_any( (struct sk_buff *)pPktInfo->osInfo );
            ETH_STAT(priv->eth_stat.skb_free_ok++);
            mvOsFree(pPktInfo->pFrags);
            mvOsFree(pPktInfo);
        }
    }

#if defined(CONFIG_MV_ETH_NFP) || defined (CONFIG_MV_SKB_REUSE)
    while( (pPktInfo = eth_pkt_info_get(priv)) )
    {
        dev_kfree_skb_any( (struct sk_buff *)pPktInfo->osInfo );
        ETH_STAT(priv->eth_stat.skb_free_ok++);
        mvOsFree(pPktInfo->pFrags);
        mvOsFree(pPktInfo);
    }
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

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
    printk( KERN_ERR "eth%d: stop internals failed\n", priv->port );
    return -1;
}


/*********************************************************** 
 * mv_eth_change_mtu_internals --                                     *
 *   stop port activity. release skb from rings. set new   *
 *   mtu in device and hw. restart port activity and       *
 *   and fill rx-buiffers with size according to new mtu.  *
 ***********************************************************/
int     mv_eth_change_mtu_internals( struct net_device *dev, int mtu )
{
    mv_eth_priv *priv = MV_ETH_PRIV(dev);

    if(mtu < 1498 /* 1518 - 20 */) {
        printk( "%s: Illegal MTU value %d, ", dev->name, mtu);
        mtu = 1500;
        printk(" rounding MTU to: %d \n",mtu);
    }
    else if(mtu > 9676 /* 9700 - 20 and rounding to 8 */) {
        printk( "%s: Illegal MTU value %d, ", dev->name, mtu);
        mtu = 9676;
        printk(" rounding MTU to: %d \n",mtu);  
    }
      
    if(MV_RX_BUF_SIZE( mtu) & ~ETH_RX_BUFFER_MASK) {
        printk( "%s: Illegal MTU value %d, ", dev->name, mtu);
        mtu = 8 - (MV_RX_BUF_SIZE( mtu) & ~ETH_RX_BUFFER_MASK) + mtu;
        printk(" rounding MTU to: %d \n",mtu);
    }

    /* set mtu in device and in hal sw structures */
    if( mvEthMaxRxSizeSet( priv->hal_priv, MV_RX_BUF_SIZE( mtu)) ) {
        printk( KERN_ERR "%s: ethPortSetMaxBufSize failed\n", dev->name );
        return -1;
    }
    
    dev->mtu = mtu;

    mv_netdev_set_features(dev);

    return 0;
}


/*********************************************************** 
 * mv_netdev_timer_callback --                                *
 *   N msec periodic callback for cleanup.                 *
 ***********************************************************/
static void mv_netdev_timer_callback( unsigned long data )
{
    struct net_device   *dev = (struct net_device *)data;
    mv_eth_priv         *priv = MV_ETH_PRIV(dev);
    int                 tx_done_count, txq;
#ifdef CONFIG_MV_ETH_NFP
    int			        i = 0;
    MV_PKT_INFO* 	    pPktInfo;
#endif /* CONFIG_MV_ETH_NFP */

    ETH_DBG( ETH_DBG_TX, ("%s: timer_callback", dev->name) );
    priv->eth_stat.timer_events++;
    
    spin_lock(priv->lock);

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
    /* Refill pktInfo */
    if (priv->refill_needed_flag)
    {
  	    priv->refill_needed_flag = 0;
        for(i=0; i<MV_ETH_RX_Q_NUM; i++)
        {
            while(mvEthRxResourceGet(priv->hal_priv, i) < mv_eth_rxq_desc[i])
            {
		        pPktInfo = eth_pkt_info_get(priv);
		        if (pPktInfo != NULL) {
                	mvEthPortRxDone(priv->hal_priv, i, pPktInfo);
			        ETH_STAT( priv->eth_stat.rx_fill_ok[i]++);
		        }
		        else {
                    priv->eth_stat.rx_pool_empty++;
                    priv->refill_needed_flag = 1;
			        break;
		        }
            }
        }
    }
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

    mvEthPortTxRestart(priv->hal_priv);

    /* Call TX done */
    for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
    {
        if(priv->tx_count[txq] > 0)
        {
#ifdef ETH_TX_DONE_ISR
#else
            tx_done_count = eth_tx_done(priv, txq);
            priv->tx_count[txq] -= tx_done_count;
#endif /* ETH_TX_DONE_ISR */ 
        }
    }

#ifdef CONFIG_MV_SKB_REUSE
    eth_skb_alloc_for_reuse(priv, dev->mtu);
#endif /* CONFIG_MV_SKB_REUSE */

    spin_unlock(priv->lock);

    if(priv->timer_flag)
    {
        priv->timer.expires = jiffies + ((HZ*CONFIG_MV_ETH_TIMER_PERIOD)/1000); /*ms*/
        add_timer( &priv->timer );
    }
}

/* Initialize Ethernet port on chip */
int    mv_eth_hal_init(mv_eth_priv *priv, int mtu, u8* mac)
{
    MV_ETH_PORT_INIT    hal_init_struct;

        /* init the hal */
    hal_init_struct.maxRxPktSize = MV_RX_BUF_SIZE(mtu);
    hal_init_struct.rxDefQ = ETH_DEF_RXQ;
    memcpy(hal_init_struct.rxDescrNum,  mv_eth_rxq_desc, sizeof(int)*MV_ETH_RX_Q_NUM);
    memcpy(hal_init_struct.txDescrNum,  mv_eth_txq_desc, sizeof(int)*MV_ETH_TX_Q_NUM);
    hal_init_struct.osHandle = NULL;

    priv->hal_priv = mvEthPortInit( priv->port, &hal_init_struct );
    if( !priv->hal_priv ) {
        printk( KERN_ERR "eth port=%d: load failed\n", priv->port);
        return -ENODEV;
    }

#ifdef CONFIG_ETH_FLOW_CONTROL
    /* enable flow Control in MAC level */
    mvEthFlowCtrlSet(priv->hal_priv, MV_ETH_FC_ENABLE);
#endif

    if(mac)
    {
        /* set new addr in hw */
        if( mvEthMacAddrSet( priv->hal_priv, mac, ETH_DEF_RXQ) != MV_OK ) 
        {
            printk("mv_eth_hal_init: ethSetMacAddr failed for port=%d\n", priv->port);
            return -ENODEV;
        }
    }
    else
    {
        mvEthRxFilterModeSet(priv->hal_priv, MV_TRUE);
    }

    if( mvEthMaxRxSizeSet( priv->hal_priv, MV_RX_BUF_SIZE(mtu)) ) {
        printk( "mv_eth_hal_init: ethPortSetMaxBufSize failed for port=%d\n", priv->port);
        return -ENODEV;
    }
    return 0;
}

/* Initialize HAL level of Ethernet driver */
int    mv_eth_priv_init(mv_eth_priv *priv, int port)
{
    int         txq, i;
    MV_PKT_INFO *pkt_info;

    memset(priv, 0, sizeof(mv_eth_priv) );

#if defined(ETH_INCLUDE_TSO) || defined(ETH_INCLUDE_UFO) || defined(CONFIG_MV_GATEWAY)
    for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
    {
        priv->tx_extra_bufs[txq] = mvOsMalloc(mv_eth_txq_desc[txq]*sizeof(char*));
        if(priv->tx_extra_bufs[txq] == NULL)
        {
            printk("eth%d TSO/UFO: txq=%d - Can't alloc %d bytes for tx_extra_bufs array\n", 
                        port, txq, mv_eth_txq_desc[txq]*sizeof(char*));
            return -ENOMEM;
        }
        for(i=0; i<mv_eth_txq_desc[txq]; i++)
        {
            priv->tx_extra_bufs[txq][i] = mvOsMalloc(TX_EXTRA_BUF_SIZE);
            if(priv->tx_extra_bufs[txq][i] == NULL)
            {
                printk("eth%d TSO/UFO: txq=%d - Can't alloc %d extra TX buffer (%d bytes)\n", 
                        port, txq, i, TX_EXTRA_BUF_SIZE);
                return -ENOMEM;
            }
        }
        priv->tx_extra_buf_idx[txq] = 0;
    }
#endif /* ETH_INCLUDE_TSO || ETH_INCLUDE_UFO || CONFIG_MV_GATEWAY */

    priv->txPktInfoPool = mvStackCreate(mv_eth_tx_desc_total);
    if(priv->txPktInfoPool == NULL)
    {
        printk("eth%d: Can't create txPktInfoPool for %d elements\n", 
                    port, mv_eth_tx_desc_total);
        return -ENOMEM;
    }

    for(i=0; i<mv_eth_tx_desc_total; i++)
    {
        pkt_info = mvOsMalloc(sizeof(MV_PKT_INFO));
        if(pkt_info == NULL)
        {
            printk("eth%d: Can't alloc %d bytes for %d MV_PKT_INFO structure\n", 
                    port, sizeof(MV_PKT_INFO), i);
            return -ENOMEM;
        }
        memset(pkt_info, 0, sizeof(MV_PKT_INFO));
        pkt_info->srcIdx = (char)-1; 

        pkt_info->pFrags = mvOsMalloc(sizeof(MV_BUF_INFO)*(MAX_SKB_FRAGS+3));
        if(pkt_info->pFrags == NULL)
        {
            printk("eth%d: Can't alloc %d bytes for %d MV_BUF_INFO array\n", 
                    port, (int)(sizeof(MV_BUF_INFO)*(MAX_SKB_FRAGS+3)), i);
            return -ENOMEM;
        }
        memset(pkt_info->pFrags, 0, sizeof(MV_BUF_INFO)*(MAX_SKB_FRAGS+3));
        mvStackPush(priv->txPktInfoPool, (MV_U32)pkt_info);
    }

    memset(priv->tx_count, 0, sizeof(priv->tx_count));

    /* init mv_eth_priv */
    priv->port = port;

    memset( &priv->timer, 0, sizeof(struct timer_list) );
    priv->timer.function = mv_netdev_timer_callback;
    init_timer(&priv->timer);
    priv->timer_flag = 0;

#ifdef ETH_INCLUDE_LRO
    priv->lro_mgr.max_aggr = ETH_LRO_MAX_AGGR;
    priv->lro_mgr.max_desc = ETH_LRO_MAX_DESCRIPTORS;
    priv->lro_mgr.lro_arr = priv->lro_desc;
    priv->lro_mgr.get_skb_header = eth_get_skb_hdr;
    priv->lro_mgr.features = LRO_F_NAPI | LRO_F_EXTRACT_VLAN_ID;
    priv->lro_mgr.dev = dev;
    priv->lro_mgr.ip_summed = CHECKSUM_UNNECESSARY;
    priv->lro_mgr.ip_summed_aggr = CHECKSUM_UNNECESSARY;
#endif

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
    priv->fpRxPool = mvStackCreate(mv_eth_rx_desc_total*2 + mv_eth_tx_desc_total);
    if(priv->fpRxPool == NULL)
    {
        mvOsPrintf("eth_priv_init_%d: Can't create fpRxPool for %d elements\n", 
                port, (mv_eth_rx_desc_total*2 + mv_eth_tx_desc_total));
        return -ENOMEM;
    }
#endif /* (CONFIG_MV_ETH_NFP) || (CONFIG_MV_SKB_REUSE) */

#ifdef CONFIG_MV_ETH_NFP
    priv->refill_needed_flag = 0;
    memset(&priv->fpStats, 0, sizeof(priv->fpStats) );
    priv->lock = &nfp_lock;
#else
    priv->lock = kmalloc(sizeof(spinlock_t), GFP_ATOMIC);
    spin_lock_init( priv->lock );
#endif /* CONFIG_MV_ETH_NFP */

#ifdef CONFIG_MV_SKB_REUSE
    priv->skbReusePool = mvStackCreate(mv_eth_rx_desc_total*2);
    if(priv->skbReusePool == NULL)
    {
        printk("eth_priv_init_%d: Can't create skbReusePool for %d elements\n", 
                port, mv_eth_rx_desc_total*2);
        return -ENOMEM;
    }
#endif /* CONFIG_MV_SKB_REUSE */

#ifdef ETH_MV_TX_EN
    priv->tx_en = priv->tx_en_bk = MV_ETH_TX_EN_DEFAULT;
    priv->tx_en_deep = 1;
#endif /* ETH_MV_TX_EN */

    return 0;
}

/* Release all allocated memory */
void    mv_eth_priv_cleanup(mv_eth_priv *priv)
{
    MV_PKT_INFO *pkt_info;

#ifdef CONFIG_MV_SKB_REUSE
    if(priv->skbReusePool != NULL)
    {
        mvStackDelete(priv->skbReusePool);
        priv->skbReusePool = NULL;
    }
#endif /* CONFIG_MV_SKB_REUSE */

#ifdef CONFIG_MV_ETH_NFP
#else
    if(priv->lock != NULL)
    {
        kfree(priv->lock);
        priv->lock = NULL;
    }
#endif /* CONFIG_MV_ETH_NFP */

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
    if(priv->fpRxPool != NULL)
    {
        mvStackDelete(priv->fpRxPool);
        priv->fpRxPool = NULL;
    }
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

#if defined(ETH_INCLUDE_TSO) || defined(ETH_INCLUDE_UFO) || defined(CONFIG_MV_GATEWAY)
    {
        int     i, txq;

        for(txq=0; txq<MV_ETH_TX_Q_NUM; txq++)
        {
            if(priv->tx_extra_bufs[txq] != NULL)
            {
                for(i=0; i<mv_eth_txq_desc[txq]; i++)
                {
                    if(priv->tx_extra_bufs[txq][i] != NULL)
                    {
                        mvOsFree(priv->tx_extra_bufs[txq][i]);
                        priv->tx_extra_bufs[txq][i] = NULL;
                    }
                }
                    mvOsFree(priv->tx_extra_bufs[txq]);
                    priv->tx_extra_bufs[txq] = NULL;
            }
        }
    }
#endif /* ETH_INCLUDE_TSO || ETH_INCLUDE_UFO || CONFIG_MV_GATEWAY */

    if(priv->txPktInfoPool)
    {
    while(mvStackIsEmpty(priv->txPktInfoPool) == MV_FALSE)
    {
        pkt_info = (MV_PKT_INFO*)mvStackPop(priv->txPktInfoPool);
        if(pkt_info != NULL)
        {
            if(pkt_info->pFrags != NULL)
            {
                mvOsFree(pkt_info->pFrags);
            }
            mvOsFree(pkt_info);
        }
    }
    mvStackDelete(priv->txPktInfoPool);
    }
}


#ifdef ETH_INCLUDE_TSO
/*********************************************************** 
 * eth_tso_tx --                                             *
 *   send a packet.                                        *
 ***********************************************************/
static int eth_tso_tx(struct sk_buff *skb, struct net_device *dev, int txq)
{
    MV_STATUS       status;
    MV_PKT_INFO     *pPktInfo;
    int             pkt, frag, buf;
    int             total_len, hdr_len, mac_hdr_len, size, frag_size, data_left;
    char            *frag_ptr, *extra_ptr;
    MV_U16          ip_id;
    MV_U32          tcp_seq;
    struct iphdr    *iph;
    struct tcphdr   *tcph;
    skb_frag_t      *skb_frag_ptr;
    mv_eth_priv     *priv = MV_ETH_PRIV(dev);
    const struct tcphdr *th = tcp_hdr(skb);

    pkt = 0;        
    frag = 0;
    total_len = skb->len;
    hdr_len = (skb_transport_offset(skb) + tcp_hdrlen(skb));
    mac_hdr_len = skb_network_offset(skb);

    total_len -= hdr_len;

    if(skb_shinfo(skb)->frag_list != NULL)
    {
        printk("***** ERROR: frag_list is not null\n");
        print_skb(skb);
    }

    if(skb_shinfo(skb)->gso_segs == 1)
    {
        printk("***** ERROR: only one TSO segment\n");
        print_skb(skb);
    }

    if(total_len <= skb_shinfo(skb)->gso_size)
    {
        printk("***** ERROR: total_len less than gso_size\n");
        print_skb(skb);
    }

    if( (htons(ETH_P_IP) != skb->protocol) || 
        (ip_hdr(skb)->protocol != IPPROTO_TCP) )
    {
        printk("***** ERROR: Unexpected protocol\n");
        print_skb(skb);
    }

    ip_id = ntohs(ip_hdr(skb)->id);
    tcp_seq = ntohl(th->seq);

    frag_size = skb_headlen(skb);
    frag_ptr = skb->data;

    if(frag_size < hdr_len){
        printk("***** ERROR: frag_size=%d, hdr_len=%d\n", frag_size, hdr_len);
        print_skb(skb);
    }

    frag_size -= hdr_len;
    frag_ptr += hdr_len;
    if(frag_size == 0)
    {
        skb_frag_ptr = &skb_shinfo(skb)->frags[frag];

        /* Move to next segment */
        frag_size = skb_frag_ptr->size;
        frag_ptr = page_address(skb_frag_ptr->page) + skb_frag_ptr->page_offset;
        frag++;
    }
    
    while(total_len > 0)
    {            
        pPktInfo = (MV_PKT_INFO*)mvStackPop(priv->txPktInfoPool);

        extra_ptr = priv->tx_extra_bufs[txq][priv->tx_extra_buf_idx[txq]++];
        if(priv->tx_extra_buf_idx[txq] == mv_eth_txq_desc[txq])
            priv->tx_extra_buf_idx[txq] = 0;

#ifdef CONFIG_MV_GATEWAY
	    /* Note: supports Marvell Header mode, not VLAN mode */
        if(priv->isGtw)
        {
            struct mv_vlan_cfg* vlan_cfg = MV_NETDEV_VLAN(dev);

	        *(unsigned short *)(extra_ptr) = vlan_cfg->header;
        
            pPktInfo->pFrags[0].bufVirtPtr = extra_ptr;
            pPktInfo->pFrags[0].dataSize = ETH_MV_HEADER_SIZE;
          	pPktInfo->pktSize = ETH_MV_HEADER_SIZE;
        }
        else
#endif /* CONFIG_MV_GATEWAY */
        {
            pPktInfo->pFrags[0].dataSize = 0;
            pPktInfo->pktSize = 0;
            pPktInfo->pFrags[0].bufVirtPtr = extra_ptr + ETH_MV_HEADER_SIZE;
        }
 
        extra_ptr += ETH_MV_HEADER_SIZE;
        memcpy(extra_ptr, skb->data, hdr_len);

        pPktInfo->pFrags[0].dataSize += hdr_len;
        pPktInfo->pFrags[0].bufSize = TX_EXTRA_BUF_SIZE;

        data_left = MV_MIN(skb_shinfo(skb)->gso_size, total_len);
        pPktInfo->pktSize += (data_left + hdr_len);
        total_len -= data_left;

        /* Update fields */
        iph = (struct iphdr*)(extra_ptr + mac_hdr_len);
        iph->tot_len = htons(data_left + hdr_len - mac_hdr_len);
        iph->id = htons(ip_id);

        tcph = (struct tcphdr*)(extra_ptr + skb_transport_offset(skb));
        tcph->seq = htonl(tcp_seq);
/*
        printk("pkt=%d, extra=%p, left=%d, total=%d, iph=%p, tcph=%p, id=%d, seq=0x%x\n",
                pkt, extra_ptr, data_left, total_len, iph, tcph, ip_id, tcp_seq);
*/
        tcp_seq += data_left;
        ip_id++;
        if(total_len == 0)
        {
            /* Only for last packet */
            pPktInfo->osInfo = (MV_ULONG)skb;
        }
        else
        {
            /* Clear all special flags for not last packet */
            tcph->psh = 0;
            tcph->fin = 0;
            tcph->rst = 0;
            pPktInfo->osInfo = (MV_ULONG)0;
        }

        buf = 1;
        while(data_left > 0)
        {
            size = MV_MIN(frag_size, data_left);
            if(size == 0)
            {
                printk("***** ERROR: data_left=%d, frag_size=%d\n", data_left, frag_size);
                print_skb(skb);
            }
            data_left -= size;
            frag_size -= size;
            pPktInfo->pFrags[buf].bufVirtPtr = frag_ptr;
            pPktInfo->pFrags[buf].dataSize = size;
            frag_ptr += size;
            buf++;
            if( (frag < skb_shinfo(skb)->nr_frags) && (frag_size == 0) )
            {                 
                skb_frag_ptr = &skb_shinfo(skb)->frags[frag];

                /* Move to next segment */
                frag_size = skb_frag_ptr->size;
                frag_ptr = page_address(skb_frag_ptr->page) + skb_frag_ptr->page_offset;
                frag++;
            }
        }
        /* packet is full */
        pPktInfo->numFrags = buf;
        pPktInfo->status =  
                (ETH_TX_IP_NO_FRAG | ETH_TX_L4_TCP_TYPE |
                 ETH_TX_GENERATE_L4_CHKSUM_MASK | ETH_TX_GENERATE_IP_CHKSUM_MASK |
                 ((ip_hdr(skb)->ihl) << ETH_TX_IP_HEADER_LEN_OFFSET) );

        status = mvEthPortSgTx( priv->hal_priv, txq, pPktInfo);
        if( status == MV_OK ) {
            priv->tx_count[txq]++;
            dev->stats.tx_packets++;
            dev->stats.tx_bytes +=  pPktInfo->pktSize;
            dev->trans_start = jiffies;
            ETH_STAT( priv->eth_stat.tx_hal_ok[txq]++);
        }
        else
        {
            /* tx failed. */

            /* For single TX queue it must not happen because 
            *   we stop call to netif_stop_queue in advance 
            * For Multu TX queue config, free skb and continue without stopping. 
            */
            dev->stats.tx_dropped++;

            ETH_DBG( ETH_DBG_TX, ("%s: queue=%d is full, stop transmit\n", dev->name, txq) );

            /* we need to reuse this pPktInfo because TX failed */
            dev_kfree_skb_any(skb);
            pPktInfo->osInfo = 0;
            mvStackPush(priv->txPktInfoPool, (MV_U32)pPktInfo);

            /* Release extra buffer too */            
  			if(priv->tx_extra_buf_idx[txq] == 0)
            {
                priv->tx_extra_buf_idx[txq] = mv_eth_txq_desc[txq]-1;
            }
            else
	    	{
                priv->tx_extra_buf_idx[txq]--;
            }

            ETH_STAT( priv->eth_stat.tx_hal_no_resource[txq]++);
            return 0;
        }   
        pkt++;
    }    
    return 0;
}
#endif /* ETH_INCLUDE_TSO */

#ifdef ETH_INCLUDE_UFO
/*********************************************************** 
 * eth_ufo_tx --                                           *
 *   send a large UDP packet.                              *
 ***********************************************************/
static int eth_ufo_tx(struct sk_buff *skb, struct net_device *dev, int txq)
{
   	MV_STATUS       status;
	unsigned int    nr, fn;
	skb_frag_t      *fp;
	mv_eth_priv     *priv = MV_ETH_PRIV(dev);
	struct iphdr    *iph;
	MV_PKT_INFO     *pPktInfo;
	int             buf;
	char            *data;
	unsigned int    pkt_sz, pkt_nm, size;
	unsigned int    left, mtu, hlen, hhlen, dlen, offset;
	int             extra_buf_used = 0;

	ETH_DBG(ETH_DBG_GSO, ("UFO: sbk=%p len=%d gso_type=%d gso_segs=%d, gso_size=%d nr_frags=%d\n",              
						  skb, skb->len, 
						  skb_shinfo(skb)->gso_type, 
						  skb_shinfo(skb)->gso_segs, 
						  skb_shinfo(skb)->gso_size,
						  skb_shinfo(skb)->nr_frags));

	ETH_DBG(ETH_DBG_GSO, ("UFO: page[h] %d bytes\n", skb_headlen(skb)));

#ifdef ETH_DEBUG
	for (fn=0; fn<skb_shinfo(skb)->nr_frags; fn++)
		ETH_DBG(ETH_DBG_GSO, ("UFO: page[%d] %d bytes\n", fn, skb_shinfo(skb)->frags[fn].size));
#endif

	iph = ip_hdr(skb);
	hlen = iph->ihl * 4;
	hhlen = skb_network_offset(skb);
	mtu = dev->mtu + hhlen;

	if (skb->ip_summed != CHECKSUM_NONE) {
		struct udphdr *udph = udp_hdr(skb);
		udph->check = 0;
		offset = hhlen + hlen;
		skb->csum = skb_checksum(skb, offset, skb->len - offset, 0);
		udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, skb->len - hhlen - hlen, IPPROTO_UDP, skb->csum);
	}

	fn = 0;
	nr = skb_shinfo(skb)->nr_frags;

	/* skb_headlen */
	size = skb_headlen(skb);
	data = skb->data;
	left = skb->len - hhlen - hlen; /* actual data to send */
	offset = 0;

	while (left) {
       	pPktInfo = (MV_PKT_INFO*)mvStackPop(priv->txPktInfoPool);

		extra_buf_used = 0;
		/* ip header */
		if (offset) { 	
			data = priv->tx_extra_bufs[txq][priv->tx_extra_buf_idx[txq]++];
            if(priv->tx_extra_buf_idx[txq] == mv_eth_txq_desc[txq])
                priv->tx_extra_buf_idx[txq] = 0;

#ifdef CONFIG_MV_GATEWAY
	        /* Note: supports Marvell Header mode, not VLAN mode */
            if(priv->isGtw)
            {
                struct mv_vlan_cfg *vlan_cfg = MV_NETDEV_VLAN(dev);

	            *(unsigned short *)(data) = vlan_cfg->header;
        
                pPktInfo->pFrags[0].bufVirtPtr = data;
                pPktInfo->pFrags[0].dataSize = ETH_MV_HEADER_SIZE;
          	    pPktInfo->pktSize = ETH_MV_HEADER_SIZE;
            }
            else
#endif /* CONFIG_MV_GATEWAY */
            {
                pPktInfo->pFrags[0].dataSize = 0;
                pPktInfo->pktSize = 0;
                pPktInfo->pFrags[0].bufVirtPtr = data + ETH_MV_HEADER_SIZE;
            }

			extra_buf_used = 1;
			data += 2; 
			size = hhlen + hlen;
			memcpy(data, skb->data, size);
		}

		iph = (struct iphdr*)(((u32)data)+hhlen);	
		pPktInfo->pFrags[0].dataSize += size;
		buf = 1;
		pkt_nm = 1;
		pkt_sz = size;
		ETH_DBG(ETH_DBG_GSO, ("UFO: add pkt[%d] %d bytes total=%d\n", pkt_nm-1, size, pkt_sz));	

		/* payload */
		while (fn < nr) {
			fp = &skb_shinfo(skb)->frags[fn];

			BUG_ON(mtu < pkt_sz);

			size = min_t(int, fp->size, mtu - pkt_sz);
			data = page_address(fp->page) + fp->page_offset;
	
			fp->page_offset += size;
			fp->size -= size;
	
			if (fp->size == 0)
				fn++;

			if (size) {
				pPktInfo->pFrags[buf].dataSize = size;
				pPktInfo->pFrags[buf].bufVirtPtr = data;
				buf++;
				pkt_sz += size; 
				pkt_nm++;
				BUG_ON(pkt_nm == MAX_SKB_FRAGS);
				ETH_DBG(ETH_DBG_GSO, ("UFO: add pkt[%d] %d bytes total=%d frag=%d\n", 
							pkt_nm-1, size, pkt_sz, fn));	
			}

			if (mtu == pkt_sz)
				break;
		}

		/* fill ip header */
		dlen = pkt_sz - hhlen - hlen;

		ETH_DBG(ETH_DBG_GSO, ("UFO: ip_payload=%d (bad=%d), offset=%d\n", 
							   dlen, dlen & 7, offset));

		iph->tot_len = htons(pkt_sz - hhlen);		
		iph->frag_off = htons(offset>>3);
		offset += dlen;
		left -= dlen;
		if (left)
			iph->frag_off |= htons(IP_MF);
		
		pPktInfo->osInfo = left ? 0 : (MV_ULONG)skb;
		pPktInfo->pktSize += pkt_sz;
		pPktInfo->numFrags = pkt_nm;
		pPktInfo->status = ETH_TX_GENERATE_IP_CHKSUM_MASK | ((iph->ihl) << ETH_TX_IP_HEADER_LEN_OFFSET);

		status = mvEthPortSgTx(priv->hal_priv, txq, pPktInfo);
		ETH_DBG(ETH_DBG_GSO, ("UFO: Tx (ok=%d) %d bytes in %d bufs left=%d\n", 
			   status, pPktInfo->pktSize, pPktInfo->numFrags, left));

		if (status == MV_OK) {
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += pPktInfo->pktSize;
			dev->trans_start = jiffies;
			priv->tx_count[txq]++;
			ETH_STAT(priv->eth_stat.tx_hal_ok[txq]++);
		}
		else {
            /* tx failed. */

            /* For single TX queue it must not happen because 
            *   we stop call to netif_stop_queue in advance 
            * For Multu TX queue config, free skb and continue without stopping. 
            */
			dev->stats.tx_dropped++;

			ETH_DBG( ETH_DBG_TX, ("%s: q=%d is full, stop transmit\n", dev->name, txq) );

            /* we need to reuse this pPktInfo because TX failed */
            dev_kfree_skb_any(skb);
            pPktInfo->osInfo = 0;
            mvStackPush(priv->txPktInfoPool, (MV_U32)pPktInfo);

            /* Release extra buffer too */            
  			if(priv->tx_extra_buf_idx[txq] == 0)
            {
                priv->tx_extra_buf_idx[txq] = mv_eth_txq_desc[txq]-1;
            }
            else
	    	{
                priv->tx_extra_buf_idx[txq]--;
            }

			ETH_STAT(priv->eth_stat.tx_hal_no_resource[txq]++);
			return 0;
		}   		
	}

    return 0;
}
#endif /* ETH_INCLUDE_UFO */


#ifdef ETH_INCLUDE_LRO
static int eth_get_skb_hdr(struct sk_buff *skb, void **iphdr,
		       void **tcph, u32 *hdr_flags, void *private)
{
	unsigned int hlen;
	struct iphdr *iph;

	skb_reset_network_header(skb);
	iph = ip_hdr(skb);

	if (iph->protocol == IPPROTO_TCP) {
		hlen = ip_hdrlen(skb);
		skb_set_transport_header(skb, hlen);
		*tcph = tcp_hdr(skb);
	
		/* check if ip header and tcp header are complete */
		if (iph->tot_len < hlen + tcp_hdrlen(skb))
			return -1;
	
		*hdr_flags = LRO_IPV4 | LRO_TCP;
		*iphdr = iph;
	
		return 0;
	}
#ifdef ETH_INCLUDE_LRO_UDP
	else if (iph->protocol == IPPROTO_UDP) {
			hlen = ip_hdrlen(skb);
			skb_set_transport_header(skb, hlen);
			*tcph = NULL;

			/* fragmented udp/ip only */
			if (!(iph->frag_off & htons(IP_MF | IP_OFFSET)))
				return -1;

			*hdr_flags = LRO_IPV4;
			*iphdr = iph;
			return 0;
	}
#endif /* ETH_INCLUDE_LRO_UDP */
	return -1;
}
#endif /* ETH_INCLUDE_LRO */


/*********************************************************** 
 * mv_eth_tx --                                         *
 *   send a packet.                                        *
 ***********************************************************/
static int eth_tx( struct sk_buff *skb , struct net_device *dev )
{
    mv_eth_priv             *priv = MV_ETH_PRIV(dev);
    struct net_device_stats *stats = MV_NETDEV_STATS(dev);
    unsigned long           flags = 0;
    MV_STATUS               status;
    MV_PKT_INFO             *pPktInfo;
    int                     ret = 0, i, queue, tx_done_count;
    int tx_in_interrupt	    = in_interrupt();
#ifdef CONFIG_MV_GATEWAY
    unsigned int	    switch_info;
#endif /* CONFIG_MV_GATEWAY */

    if( netif_queue_stopped( dev ) ) {
        printk( KERN_ERR "%s: transmitting while stopped\n", dev->name );
        return 1;
    }

    if (!tx_in_interrupt)
    	local_irq_save(flags);

    if (!spin_trylock(priv->lock)) {
        /* Collision - tell upper layer to requeue */
	    if (!tx_in_interrupt)
            local_irq_restore(flags);
        return NETDEV_TX_LOCKED;
    }

    ETH_DBG( ETH_DBG_TX, ("%s: tx len=%d headlen=%d frags=%d, ip_summed=%d gso_type=%d\n",
             dev->name, skb->len, skb_headlen(skb), skb_shinfo(skb)->nr_frags, skb->ip_summed,skb_shinfo(skb)->gso_type));
    priv->eth_stat.tx_events++;

    /* At this point we need to decide to which tx queue this packet goes, */
    /* and whether we need to prepend a proprietary header.                */
    queue = eth_tx_policy(priv, skb);

#ifdef ETH_INCLUDE_TSO
    if (skb_shinfo(skb)->gso_type & SKB_GSO_TCPV4) {
        ret = eth_tso_tx(skb, dev, queue);
	    ETH_STAT( priv->eth_stat.tso_stats[skb->len >> 10]++);
        goto tx_end;
    }
#endif /* ETH_INCLUDE_TSO */

#ifdef ETH_INCLUDE_UFO
    if (skb_shinfo(skb)->gso_type & SKB_GSO_UDP) {
	    if (skb->len > dev->mtu) {
	        ETH_STAT(priv->eth_stat.ufo_stats[skb->len >> 10]++);
	        ret = eth_ufo_tx(skb, dev, queue);
	        goto tx_end;
	    }
    }
#endif /* ETH_INCLUDE_UFO */

    pPktInfo = (MV_PKT_INFO*)mvStackPop(priv->txPktInfoPool);
    pPktInfo->osInfo = (MV_ULONG)skb;
    pPktInfo->pktSize = skb->len;
    pPktInfo->status = 0;
    
    /* see if this is a single/multiple buffered skb */
    if( skb_shinfo(skb)->nr_frags == 0 ) {
        pPktInfo->pFrags->bufVirtPtr = skb->data;
        pPktInfo->pFrags->dataSize = skb->len;
        pPktInfo->numFrags = 1;
    }
    else {

        MV_BUF_INFO *p_buf_info = pPktInfo->pFrags;

        /* first skb fragment */
        p_buf_info->dataSize = skb_headlen(skb);
        p_buf_info->bufVirtPtr = skb->data;
        p_buf_info++;

        /* now handle all other skb fragments */
        for ( i = 0; i < skb_shinfo(skb)->nr_frags; i++ ) {

            skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

            p_buf_info->dataSize = frag->size;
            p_buf_info->bufVirtPtr = page_address(frag->page) + frag->page_offset;
            p_buf_info++;
        }
        pPktInfo->numFrags = skb_shinfo(skb)->nr_frags + 1;
    }

#ifdef TX_CSUM_OFFLOAD
    /* if HW is suppose to offload layer3 and layer4 checksum, 
     *   set some bits in the first pkt_info command.
    */
    if(skb->ip_summed == CHECKSUM_PARTIAL) {
        struct iphdr *iph = ip_hdr(skb); 

        ETH_DBG( ETH_DBG_TX, ("%s: tx csum offload\n", dev->name) );
        ETH_STAT( priv->eth_stat.tx_csum_hw++);

        /* we do not handle fragmented IP packets. add check inside iph!! */
        pPktInfo->status = ETH_TX_IP_NO_FRAG | ETH_TX_GENERATE_IP_CHKSUM_MASK |          
                           (iph->ihl << ETH_TX_IP_HEADER_LEN_OFFSET);   

        if(iph->protocol == IPPROTO_TCP) 
            pPktInfo->status |= (ETH_TX_L4_TCP_TYPE | ETH_TX_GENERATE_L4_CHKSUM_MASK);
        else if(iph->protocol == IPPROTO_UDP)
            pPktInfo->status |= (ETH_TX_L4_UDP_TYPE | ETH_TX_GENERATE_L4_CHKSUM_MASK);
    }
    else {
        ETH_DBG( ETH_DBG_TX, ("%s: no tx csum offload\n", dev->name) );
        ETH_STAT( priv->eth_stat.tx_csum_sw++);
        pPktInfo->status = 0x5 << ETH_TX_IP_HEADER_LEN_OFFSET; /* Errata BTS #50 */
    }
#endif

#ifdef CONFIG_MV_GATEWAY
    if(priv->isGtw)
        mv_gtw_update_tx_skb(&switch_info, dev, pPktInfo);
#endif /* CONFIG_MV_GATEWAY */

    /* now send the packet */
    status = mvEthPortSgTx( priv->hal_priv, queue, pPktInfo);
    /* check status */
    if( status == MV_OK ) {
        priv->tx_count[queue]++;
        stats->tx_bytes += skb->len;
        stats->tx_packets ++;
        dev->trans_start = jiffies;
        ETH_STAT( priv->eth_stat.tx_hal_ok[queue]++);
    }
    else {
        /* tx failed. */

        /* For single TX queue it must not happen because 
         *   we stop call to netif_stop_queue in advance 
         * For Multu TX queue config, free skb and continue without stopping. 
         */
        stats->tx_dropped++;

        /* it must not happen because we call to netif_stop_queue in advance. */
        ETH_DBG( ETH_DBG_TX, ("%s: TX queue=%d is full\n", dev->name, queue) );

        /* we need to reuse this pPktInfo because TX failed */
        dev_kfree_skb_any(skb);
        pPktInfo->osInfo = 0;
        mvStackPush(priv->txPktInfoPool, (MV_U32)pPktInfo);

        ETH_STAT( priv->eth_stat.tx_hal_no_resource[queue]++);
    }

#if defined(ETH_INCLUDE_TSO) || defined(ETH_INCLUDE_UFO)
tx_end:
#endif

#ifdef ETH_TX_DONE_ISR
#else
    if( priv->tx_count[queue] >= mv_eth_tx_done_quota)
    {
#ifdef ETH_MV_TX_EN
        if(priv->tx_en)
            eth_tx_enable(priv, queue);
#endif /* ETH_MV_TX_EN */

        tx_done_count = eth_tx_done(priv, queue);
        priv->tx_count[queue] -= tx_done_count;
    }
#endif /* ETH_TX_DONE_ISR */ 

#if (MV_ETH_TX_Q_NUM == 1)
    /* if number of available descriptors left is less than  */
    /* MAX_SKB_FRAGS stop the stack. if multi queue is used, */
    /* don't stop the stack just because one queue is full.  */
    if( mvEthTxResourceGet(priv->hal_priv, ETH_DEF_TXQ) <= MAX_SKB_FRAGS ) {
        ETH_DBG( ETH_DBG_TX, ("%s: stopping network tx interface\n", dev->name) );
        netif_stop_queue( dev );
        priv->eth_stat.tx_netif_stop++;
    }
#endif /* (MV_ETH_TX_Q_NUM > 1) */

    if (!tx_in_interrupt)
	    spin_unlock_irqrestore(priv->lock, flags);	    
    else
        spin_unlock(priv->lock);

    return ret;
}

/*********************************************************** 
 * eth_tx_timeout --                                       *
 *   nothing to be done (?)                                *
 ***********************************************************/
static void eth_tx_timeout( struct net_device *dev ) 
{
    mv_eth_priv        *priv = MV_ETH_PRIV(dev);

    priv->eth_stat.tx_timeout++;
    printk( KERN_INFO "%s: tx timeout\n", dev->name );
}

/*********************************************************** 
 * mv_netdev_init -- Allocate and initialize net_device    *
 *                   structure                             *
 ***********************************************************/
struct net_device* mv_netdev_init(mv_eth_priv *priv, int mtu, u8* mac)
{
    struct net_device   *dev;
    mv_net_priv         *net_priv;

    dev = alloc_etherdev(sizeof(mv_net_priv));
    if( !dev ) {
        return NULL;
    }

    net_priv = (mv_net_priv *)dev->priv;
    if( !net_priv ) { 
        return NULL;
    }
    memset( net_priv , 0, sizeof(mv_net_priv) );
    net_priv->giga_priv = priv;

    dev->irq = ETH_PORT_IRQ_NUM(priv->port);
    dev->mtu = mtu;
	memcpy(dev->dev_addr, mac, 6);
    dev->weight = (ETH_NUM_OF_RX_DESCR / 2);
    dev->tx_queue_len = ETH_NUM_OF_TX_DESCR;
	dev->watchdog_timeo = 5*HZ;

	dev->hard_start_xmit = eth_tx;
    dev->tx_timeout = eth_tx_timeout;
	dev->poll = eth_poll;

  	dev->open = mv_eth_open;
    dev->stop = mv_eth_stop;
	dev->set_mac_address = mv_eth_set_mac_addr;
	dev->set_multicast_list = mv_eth_set_multicast_list;
    dev->change_mtu = &mv_eth_change_mtu; 

#ifdef CONFIG_MV_GATEWAY
    if(priv->isGtw)
    {
        /* For Gateway driver replace some of callback functions */
	    dev->open = mv_gtw_start;
        dev->stop = mv_gtw_stop;
        dev->set_mac_address = mv_gtw_set_mac_addr;
	    dev->set_multicast_list = mv_gtw_set_multicast_list;
        dev->change_mtu = &mv_gtw_change_mtu;
    }
#endif /* CONFIG_MV_GATEWAY */

    priv->timer.data = (unsigned long)dev;

    mv_netdev_set_features(dev);

	if(register_netdev(dev)) {
	    printk(KERN_ERR "failed to register %s\n",dev->name);
        free_netdev(dev);
        return NULL;
	}
	else {
	    printk("%s ",dev->name);
  	}
    return dev;
}


/***********************************************************************************
 ***  print net device status
 ***********************************************************************************/
void    mv_netdev_status_print(unsigned int idx)
{
    struct net_device   *dev = eth_net_device_by_idx(idx);

    printk("%s net_device status: dev=%p, priv=%p\n\n", 
                dev->name, dev, MV_ETH_PRIV(dev));
    printk("ifIdx=%d, features=0x%x, flags=0x%x, mtu=%u, size=%d, weight=%d\n", 
            dev->ifindex, (unsigned int)(dev->features), (unsigned int)(dev->flags), 
            dev->mtu, MV_RX_BUF_SIZE(dev->mtu), dev->weight);
}


/***********************************************************************************
 ***  print Ethernet port status
 ***********************************************************************************/
void    mv_eth_status_print( unsigned int port )
{
    mv_eth_priv         *priv = eth_priv_by_port(port);

    if (priv == NULL)
    {
        printk("eth_status_print: wrong port number %d\n", port);
	    return;
    }
    printk("ethPort_%d Status: priv=%p\n\n", port, priv);

    printk("tx_total=%d, rx_total=%d, tx_done_quota=%d\n\n", 
            mv_eth_tx_desc_total, mv_eth_rx_desc_total, mv_eth_tx_done_quota);

#ifdef ETH_MV_TX_EN
    printk("TxEnable WA - %s. deep=%d\n\n", 
            priv->tx_en ? "Enabled" : "Disabled", priv->tx_en_deep);    
#endif /* ETH_MV_TX_EN */

#ifdef CONFIG_MV_SKB_REUSE
    printk("SKB Reuse - %s. pktInfo pool status:\n", 
            eth_skb_reuse_enable ? "Enabled" : "Disabled");
    mvStackStatus(priv->skbReusePool, 0);
    printk("\n");
#endif /* CONFIG_MV_SKB_REUSE */

    printk("TX pktInfo pool status:\n");
    mvStackStatus(priv->txPktInfoPool, 0);
    printk("\n");

#ifdef CONFIG_MV_ETH_NFP
    printk("NFP RX pool status:\n");
    mvStackStatus(priv->fpRxPool, 0);
#endif /* CONFIG_MV_ETH_NFP */
}

/***********************************************************************************
 ***  print port statistics
 ***********************************************************************************/
#define   STAT_PER_Q(qnum,x) for(queue = 0; queue < qnum; queue++) \
                printk("%10u ",x[queue]); \
                    printk("\n");

void mv_eth_stats_print( unsigned int port )
{
    mv_eth_priv         *priv = eth_priv_by_port(port);
    eth_statistics      *stat = NULL;

    TRC_OUTPUT();

    if (priv == NULL)
    {
        printk("eth_stats_print: wrong port number %d\n", port);
	    return;
    }
    stat = &(priv->eth_stat);

    printk( "\n====================================================\n" );
    printk( "ethPort_%d: interrupt statistics", port );
    printk( "\n-------------------------------\n" );
    printk( "irq_total.....................%10u\n", stat->irq_total );
    printk( "irq_none_events...............%10u\n", stat->irq_none );
    printk( "irq_while_polling.............%10u\n", stat->irq_while_polling );
    printk( "picr is.......................%10x\n", priv->picr);
    printk( "picer is......................%10x\n", priv->picer);

    printk( "\n====================================================\n" );
    printk( "ethPort_%d: Events", port );
    printk( "\n-------------------------------\n" );
    printk( "poll_events...................%10u\n", stat->poll_events );
    printk( "poll_complete.................%10u\n", stat->poll_complete );
    printk( "tx_events.....................%10u\n", stat->tx_events );
    printk( "tx_done_events................%10u\n", stat->tx_done_events );
    printk( "timer_events..................%10u\n", stat->timer_events);

    printk( "\n====================================================\n" );
    printk( "ethPort_%d: Errors", port);
    printk( "\n-------------------------------\n" );
    printk( "skb_alloc_fail................%10u\n", stat->skb_alloc_fail );
    printk( "tx_timeout....................%10u\n", stat->tx_timeout );
    printk( "tx_netif_stop.................%10u\n", stat->tx_netif_stop );
    printk( "tx_done_netif_wake............%10u\n", stat->tx_done_netif_wake );

#if defined(CONFIG_MV_ETH_NFP) || defined(CONFIG_MV_SKB_REUSE)
    printk( "rx_pool_empty.................%10u\n", stat->rx_pool_empty );
#endif /* CONFIG_MV_ETH_NFP || CONFIG_MV_SKB_REUSE */

#ifndef ETH_STATISTICS
    printk(" ethPort_%d: eth is compiled without statistics support!! \n", port);
#else
    {
        unsigned int queue=0, i=0;

        printk("\n");
        printk("RXQs:.........................");
        for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++) 
            printk( "%10d ", queue);
        printk("\n");

        printk( "rx_hal_ok....................."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_hal_ok);
        printk( "rx_fill_ok...................."); STAT_PER_Q(MV_ETH_RX_Q_NUM, stat->rx_fill_ok);
        printk("\n");

        printk("TXQs:.........................");
        for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++) 
            printk( "%10d ", queue);
        printk("\n");
        printk( "tx_count......................"); STAT_PER_Q(MV_ETH_TX_Q_NUM, priv->tx_count);
        printk( "tx_hal_ok....................."); STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_ok);
        printk( "tx_hal_no_resource............"); STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_hal_no_resource );
        printk( "tx_done_hal_ok................"); STAT_PER_Q(MV_ETH_TX_Q_NUM, stat->tx_done_hal_ok);
        printk("\n");

        printk( "skb_alloc_ok..................%10u\n", stat->skb_alloc_ok );
        printk( "skb_free_ok...................%10u\n", stat->skb_free_ok );
#ifdef CONFIG_MV_SKB_REUSE
        printk( "skb_reuse_rx..................%10u\n", stat->skb_reuse_rx);
        printk( "skb_reuse_tx..................%10u\n", stat->skb_reuse_tx);
        printk( "skb_reuse_alloc...............%10u\n", stat->skb_reuse_alloc);
#endif /* CONFIG_MV_SKB_REUSE */
        printk("\n");

	    printk( "tx_csum_hw....................%10u\n", stat->tx_csum_hw);
	    printk( "tx_csum_sw....................%10u\n", stat->tx_csum_sw);
        printk("\n");

        printk( "rx_netif_drop.................%10u\n", stat->rx_netif_drop );
	    printk( "rx_csum_hw....................%10u\n", stat->rx_csum_hw);
	    printk( "rx_csum_hw_frags..............%10u\n", stat->rx_csum_hw_frags);
	    printk( "rx_csum_sw....................%10u\n", stat->rx_csum_sw);

#ifdef ETH_INCLUDE_LRO
        printk( "\n");
	    printk( "rx_lro_aggregated.............%10u\n", (u32)priv->lro_mgr.stats.aggregated);
	    printk( "rx_lro_flushed................%10u\n", (u32)priv->lro_mgr.stats.flushed);
	    printk( "rx_lro_defragmented...........%10u\n", (u32)priv->lro_mgr.stats.defragmented);
	    printk( "rx_lro_no_resources...........%10u\n", (u32)priv->lro_mgr.stats.no_desc);
#endif /* ETH_INCLUDE_LRO */



#ifdef ETH_MV_TX_EN
        printk( "\n");
        printk( "tx_en_done....................%10u\n", stat->tx_en_done);
        printk( "tx_en_busy....................%10u\n", stat->tx_en_busy);
        printk( "tx_en_wait....................%10u\n", stat->tx_en_wait);
        printk( "tx_en_wait_count..............%10u\n", stat->tx_en_wait_count);
#endif /* ETH_MV_TX_EN */

        printk("\n      Linux Path RX distribution\n");
        for(i=0; i<sizeof(stat->rx_dist)/sizeof(u32); i++)
        {
            if(stat->rx_dist[i] != 0)
                printk("%d RxPkts - %d times\n", i, stat->rx_dist[i]);
        } 

#ifdef ETH_INCLUDE_TSO
        printk("\n      TSO stats\n");
        for(i=0; i<64; i++)
        {
            if(stat->tso_stats[i] != 0)
            {
                printk("\t %d KBytes - %d times\n", i, stat->tso_stats[i]);
                stat->tso_stats[i] = 0;
            }
        }
#endif /* ETH_INCLUDE_TSO */

#ifdef ETH_INCLUDE_UFO
        printk("\n      UFO stats\n");
        for(i=0; i<64; i++)
        {
	        if(stat->ufo_stats[i] != 0)
	        {
		        printk("\t %d KBytes - %d times\n", i, stat->ufo_stats[i]);
		        stat->ufo_stats[i] = 0;
	        }
	    }
#endif /* ETH_INCLUDE_UFO */

        printk("\n      tx-done stats\n");
        for(i=0; i<sizeof(stat->tx_done_dist)/sizeof(u32); i++)
        {
          if(stat->tx_done_dist[i] != 0)
              printk("%d TxDonePkts - %d times\n", i, stat->tx_done_dist[i]);
        } 
    }
#endif /* ETH_STATISTICS */

    memset( stat, 0, sizeof(eth_statistics) );

#ifdef ETH_INCLUDE_LRO
	memset( &(priv->lro_mgr.stats), 0, sizeof(struct net_lro_stats) );
#endif /* ETH_INCLUDE_LRO */

}

#ifdef CONFIG_MV_ETH_NFP
void     mv_eth_nfp_stats_print(unsigned int port)
{
    mv_eth_priv         *priv = eth_priv_by_port(port);

    if (priv == NULL)
    {
        printk("eth_nfp_stats_print: wrong port number %d\n", port);
	    return;
    }

    mvFpStatsPrint(&priv->fpStats);
    printk("\n");
    printk("NFP RX pool status:\n");
    mvStackStatus(priv->fpRxPool, 0);
}
#endif /* CONFIG_MV_ETH_NFP */ 

void eth_print_irq_status(mv_eth_priv *priv)
{
    printk("Interrupt Cause Register = 0x%08x (0x%08x)\n", 
            priv->picr, MV_REG_READ(ETH_INTR_CAUSE_REG(priv->port)));

    printk("Interrupt Mask Register = 0x%08x\n", 
            MV_REG_READ(ETH_INTR_MASK_REG(priv->port)));

    printk("Interrupt Cause Extend Register = 0x%08x (0x%08x)\n", 
            priv->picer, MV_REG_READ(ETH_INTR_CAUSE_EXT_REG(priv->port)));

    printk("Interrupt Mask Extend Register = 0x%08x\n", 
            MV_REG_READ(ETH_INTR_MASK_EXT_REG(priv->port)));
}

void print_iph(struct iphdr* iph)
{
    printk("**************** IP Header: ver=%d, ihl=%d ******************\n", 
            iph->version, iph->ihl);
    printk("tot_len=%d, id=0x%x, proto=%d, csum=0x%x, sip=0x%x, dip=0x%x\n",
            ntohs(iph->tot_len & 0xFFFF), ntohs(iph->id & 0xFFFF), iph->protocol & 0xFF, 
            ntohs(iph->check & 0xFFFF), ntohl(iph->saddr), ntohl(iph->daddr));
}

void print_tcph(struct tcphdr* hdr)
{
    printk("################## TCP Header: doff=%d ####################\n", hdr->doff); 
    printk("sPort=%d, dPort=%d, seqId=0x%x, ackId=0x%x, win=0x%x, csum=0x%x\n", 
            ntohs(hdr->source), ntohs(hdr->dest), ntohl(hdr->seq), ntohl(hdr->ack_seq),
            ntohs(hdr->window), ntohs(hdr->check) );
    printk("Flags: fin=%d, syn=%d, rst=%d, psh=%d, ack=%d, urg=%d, ece=%d, cwr=%d\n", 
            hdr->fin, hdr->syn, hdr->rst, hdr->psh, hdr->ack, hdr->urg, hdr->ece, hdr->cwr);
}


void print_skb(struct sk_buff* skb)
{
    int i;

    printk("\nskb=%p: head=%p, data=%p, tail=%p, end=%p\n", 
                skb, skb->head, skb->data, skb->tail, skb->end);
    printk("\t users=%d, truesize=%d, len=%d, data_len=%d, mac_len=%d\n", 
            atomic_read(&skb->users), skb->truesize, skb->len, skb->data_len, skb->mac_len);
    printk("\t next=%p, prev=%p, csum=0x%x, ip_summed=%d, pkt_type=%d, proto=0x%x, cloned=%d\n",
            skb->next, skb->prev, skb->csum, skb->ip_summed, skb->pkt_type, 
            ntohs(skb->protocol & 0xFFFF), skb->cloned);
    printk("\t mac=%p, nh=%p, h=%p\n", skb_mac_header(skb),  ip_hdr(skb), tcp_hdr(skb));
    printk("\t dataref=0x%x, nr_frags=%d, gso_size=%d, tso_segs=%d, frag_list=%p\n",
            atomic_read(&skb_shinfo(skb)->dataref), skb_shinfo(skb)->nr_frags, skb_shinfo(skb)->gso_size,
            skb_shinfo(skb)->gso_segs, skb_shinfo(skb)->frag_list);
    for(i=0; i<skb_shinfo(skb)->nr_frags; i++)
    {
        printk("\t frag_%d. page=%p, page_offset=0x%x, size=%d\n",
            i, page_address(skb_shinfo(skb)->frags[i].page), 
            skb_shinfo(skb)->frags[i].page_offset & 0xFFFF, 
            skb_shinfo(skb)->frags[i].size & 0xFFFF);
    }
    if( (skb->protocol == ntohs(ETH_P_IP)) && (ip_hdr(skb) != NULL) )
    {
        print_iph(ip_hdr(skb));
        if(ip_hdr(skb)->protocol == IPPROTO_TCP)
            print_tcph(tcp_hdr(skb));
    }
    printk("\n");
}

/*********************************************************** 
 * eth_init_module --                                    *
 *   main driver initialization. loading the interfaces.   *
 ***********************************************************/
static int __init mv_eth_init_module( void ) 
{
    u32             i, port, netdev=0;
    mv_eth_priv     *priv;
    u8              mac_addr[6];
    int             mtu;


#ifdef CONFIG_MV78200
    /*if no ports assigned to this CPU, return*/
    mv_eth_ports_num = mvCtrlEthMaxPortGet();
#ifdef CONFIG_MV_ETH_PORTS_NUM
    mv_eth_ports_num = min(mv_eth_ports_num, CONFIG_MV_ETH_PORTS_NUM); 
#endif
    for(port=0; port < mv_eth_ports_num; port++)
    {
	if (MV_TRUE == mvSocUnitIsMappedToThisCpu(GIGA0+port))
	{
		break;
	}			
    }
    if (port == mv_eth_ports_num)
    {    
	printk("No Giga ports mapped to this CPU\n");
	return 1;
    }
#endif
    
    printk( "Load Marvell Ethernet Driver\n");

    /* Initialize mv_eth_rxq_desc array */
    for(i=0; i<MV_ETH_RX_Q_NUM; i++) 
    {
        if (i == ETH_DEF_RXQ)
            mv_eth_rxq_desc[i] = (ETH_NUM_OF_RX_DESCR);
        else
            mv_eth_rxq_desc[i] = ETH_NUM_OF_RX_DESCR/2;

	    mv_eth_rx_desc_total += mv_eth_rxq_desc[i];
    }
    /* Initialize mv_eth_txq_desc array */
    for(i=0; i<MV_ETH_TX_Q_NUM; i++) {
        mv_eth_txq_desc[i] = ETH_NUM_OF_TX_DESCR;

        mv_eth_tx_desc_total += mv_eth_txq_desc[i];
    }

    spin_lock_init( &mii_lock );

    /* init MAC Unit */
    mvEthInit();

    mv_netdev_config_show();

    mv_eth_ports_num = mvCtrlEthMaxPortGet();
#ifdef CONFIG_MV_ETH_PORTS_NUM
    if (CONFIG_MV_ETH_PORTS_NUM < mv_eth_ports_num)
    {
	    mv_eth_ports_num = CONFIG_MV_ETH_PORTS_NUM;
    }
#endif
    mv_eth_ports = mvOsMalloc(mv_eth_ports_num*sizeof(mv_eth_priv*));
    if(mv_eth_ports == NULL)
    {
        printk("eth_init_module: can't allocate mv_net_devs for %d devices\n", 
                mv_eth_ports_num);
        return 1; 
    }
    memset(mv_eth_ports, 0, (mv_eth_ports_num*sizeof(struct net_device*)));

    mv_net_devs = mvOsMalloc((mv_eth_ports_num + GTW_MAX_NUM_OF_IFS)*sizeof(struct net_device*));
    if(mv_net_devs == NULL)
    {
        printk("eth_init_module: can't allocate mv_net_devs for %d devices\n", 
                mv_net_devs_num);
        return 1; 
    }
    memset(mv_net_devs, 0, ((mv_eth_ports_num + GTW_MAX_NUM_OF_IFS) * sizeof(struct net_device*)));

    printk("  o Loading network interface: \n");
    for(port=0; port<mv_eth_ports_num; port++)
    {

#ifdef CONFIG_MV78200
	if (MV_FALSE == mvSocUnitIsMappedToThisCpu(GIGA0+port))
	{
		printk(KERN_INFO"GbE %d is not mapped to this CPU\n", port);
		continue;
	}		
#endif

	if (MV_FALSE == mvCtrlPwrClckGet(ETH_GIG_UNIT_ID, port)) 
	{
		printk("\nWarning: Giga %d is Powered Off\n", port);
		continue;
	}
    
        /* Allocate mv_eth_priv structure */
        priv = mvOsMalloc(sizeof(mv_eth_priv));
        if(priv == NULL)
        {
            printk("eth%d: can't allocate mv_eth_priv structure\n", port);
            return 1;
        }
        mv_eth_ports[port] = priv;
        
        if( mv_eth_priv_init(priv, port) )
        {
            printk("eth%d: can't create mv_eth_priv structure\n", port);
            mv_eth_priv_cleanup(priv);
            return 1;
        }
#if defined(CONFIG_MV_GATEWAY)
        priv->isGtw = mvBoardIsSwitchConnected(port);
        priv->isGtw = 0;
        if(priv->isGtw)
        {
            if( mv_gtw_net_setup(port) < 0)
            {
                printk("eth%d: mv_gtw_net_setup FAILED\n", port);
                mv_eth_priv_cleanup(priv);
                return 1;
            }
            if( mv_eth_hal_init(priv, gtw_config.mtu, NULL) )
            {
                printk("eth_init_module: can't init eth_hal driver\n");
                mv_eth_priv_cleanup(priv);
                return 1;
            }
            for(i=0; i<gtw_config.vlans_num; i++)
            {
                mv_net_devs[netdev] = mv_netdev_init(priv, gtw_config.mtu, gtw_config.vlan_cfg[i].macaddr);
                if(mv_net_devs[netdev] == NULL)
                {
                    printk("eth_init_module: can't create netdevice\n");
                    mv_eth_priv_cleanup(priv);
                    return 1;
                }
                ((mv_net_priv*)(mv_net_devs[netdev]->priv))->vlan_cfg = &gtw_config.vlan_cfg[i];
                netdev++;
            }
            mv_gtw_init_complete(priv);
            priv->net_dev = NULL;
        }
        else
#endif /* CONFIG_MV_GATEWAY */
        {
            mtu = eth_get_config(priv, mac_addr);
            if( mv_eth_hal_init(priv, mtu, mac_addr) )
            {
                printk("eth_init_module: can't init eth_hal driver\n");
                mv_eth_priv_cleanup(priv);
                return 1;
            }

            mv_net_devs[netdev] = mv_netdev_init(priv, mtu, mac_addr);
            if(mv_net_devs[netdev] == NULL)
            {
                printk("eth_init_module: can't create netdevice\n");
                mv_eth_priv_cleanup(priv);
                return 1;
            }
            priv->net_dev = mv_net_devs[netdev];
            netdev++;
        }        
    }
    mv_net_devs_num = netdev;
    printk("\n");

#ifdef CONFIG_MV_ETH_NFP
    {
        MV_STATUS       status;

        status = fp_mgr_init();
        if (status != MV_OK) {
            printk("fp_mgr_init failed\n");
	        return 1;
        }
        spin_lock_init( &nfp_lock );
	    for(i=0; i<mv_net_devs_num; i++) 
        {
		    if (mv_net_devs[i] != NULL) 
            {
			    status = fp_mgr_if_register(mv_net_devs[i]->ifindex, MV_FP_IF_INT, 
                                            mv_net_devs[i]);
			    if (status != MV_OK) {
				    printk("fp_mgr_if_register failed\n");
				    return 1;
			    }
		    }
        }
    }
#endif /* CONFIG_MV_ETH_NFP */
   
    TRC_INIT(0, 0, 0, 0);
    TRC_START();

    return 0;
}

static void __exit mv_eth_exit_module(void)
{
    printk( "EXIT Marvell Ethernet Driver\n");
}
