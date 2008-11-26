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
#ifdef CONFIG_MV78200
#include "mv78200/mvSemaphore.h"
#endif

int mv_eth_read_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int* value)
{
    unsigned long flags;
    unsigned short tmp;
    MV_STATUS status;

    spin_lock_irqsave(&mii_lock, flags);
#ifdef CONFIG_MV78200
    mvSemaLock(MV_SEMA_SMI);
#endif
    status = mvEthPhyRegRead(mvBoardPhyAddrGet(portNumber), MIIReg, &tmp);
#ifdef CONFIG_MV78200
    mvSemaUnlock(MV_SEMA_SMI);
#endif
    spin_unlock_irqrestore(&mii_lock, flags);	
    *value = tmp;
    if (status == MV_OK)
        return 0;

    return -1;
}


int mv_eth_write_mii(unsigned int portNumber, unsigned int MIIReg, unsigned int data)
{
    unsigned long   flags;
    unsigned short  tmp;
    MV_STATUS       status;

    spin_lock_irqsave(&mii_lock, flags);
    tmp = (unsigned short)data;
#ifdef CONFIG_MV78200
    mvSemaLock(MV_SEMA_SMI);
#endif
    status = mvEthPhyRegWrite(mvBoardPhyAddrGet(portNumber), MIIReg, tmp);
#ifdef CONFIG_MV78200
    mvSemaUnlock(MV_SEMA_SMI);
#endif
    spin_unlock_irqrestore(&mii_lock, flags);

    if (status == MV_OK)
        return 0;

    return -1;
}

static int mv_eth_restart_autoneg( int port )
{
    u32 phy_reg_val = 0;

    /* enable auto-negotiation */
    mv_eth_read_mii(port, ETH_PHY_CTRL_REG, &phy_reg_val);
    phy_reg_val |= BIT12;
    mv_eth_write_mii(port, ETH_PHY_CTRL_REG, phy_reg_val);

    mdelay(10);

    /* restart auto-negotiation */
    phy_reg_val |= BIT9;
    mv_eth_write_mii(port, ETH_PHY_CTRL_REG, phy_reg_val);

    mdelay(10);

    return 0;
}


/*********************************************************** 
 * mv_eth_start --                                          *
 *   start a network device. connect and enable interrupts *
 *   set hw defaults. fill rx buffers. restart phy link    *
 *   auto neg. set device link flags. report status.       *
 ***********************************************************/
static int mv_eth_start( struct net_device *dev ) 
{
    mv_eth_priv *priv = MV_ETH_PRIV(dev);

    ETH_DBG( ETH_DBG_LOAD, ("%s: starting... ", dev->name ) );

    /* in default link is down */
    netif_carrier_off( dev );

    /* Stop the TX queue - it will be enabled upon PHY status change after link-up interrupt/timer */
    netif_stop_queue( dev );

    /* enable polling on the port, must be used after netif_poll_disable */
    netif_poll_enable(dev);

    /* fill rx buffers, start rx/tx activity, set coalescing */
    if( mv_eth_start_internals( priv, dev->mtu) != 0 ) {
        printk( KERN_ERR "%s: start internals failed\n", dev->name );
        goto error;
    }
    mv_eth_restart_autoneg( priv->port );

    if(priv->timer_flag == 0)
    {
        priv->timer.expires = jiffies + ((HZ*CONFIG_MV_ETH_TIMER_PERIOD)/1000); /*ms*/
        add_timer( &priv->timer );
        priv->timer_flag = 1;
    }

    /* connect to port interrupt line */
    if( request_irq( dev->irq, mv_eth_interrupt_handler,
        (IRQF_DISABLED | IRQF_SAMPLE_RANDOM) , dev->name, dev ) ) {
        printk( KERN_ERR "cannot assign irq%d to %s port%d\n", dev->irq, dev->name, priv->port );
        dev->irq = 0;
    	goto error;
    }

    mv_eth_unmask_interrupts(priv);

    ETH_DBG( ETH_DBG_LOAD, ("%s: start ok\n", dev->name) );

    printk( KERN_NOTICE "%s: started\n", dev->name );

    return 0;

 error:

    if( dev->irq != 0 )
    {
        free_irq( dev->irq, dev );
    }

    printk( KERN_ERR "%s: start failed\n", dev->name );
    return -1;
}

/*********************************************************** 
 * mv_eth_stop --                                       *
 *   stop interface with linux core. stop port activity.   *
 *   free skb's from rings.                                *
 ***********************************************************/
int mv_eth_stop( struct net_device *dev )
{
    unsigned long   flags;
    mv_eth_priv     *priv = MV_ETH_PRIV(dev);

    /* first make sure that the port finished its Rx polling - see tg3 */
    /* otherwise it may cause issue in SMP, one CPU is here and the other is doing the polling
    and both of it are messing with the descriptors rings!! */
    netif_poll_disable( dev );

    spin_lock_irqsave( priv->lock, flags);

    /* stop upper layer */
    netif_carrier_off( dev );
    netif_stop_queue( dev );

    /* stop tx/rx activity, mask all interrupts, relese skb in rings,*/
    mv_eth_stop_internals( priv );
    
    spin_unlock_irqrestore( priv->lock, flags);

    if( dev->irq != 0 )
    {
        free_irq( dev->irq, dev );
    }
    printk( KERN_NOTICE "%s: stopped\n", dev->name );

    return 0;
}


int mv_eth_change_mtu( struct net_device *dev, int mtu )
{
    int old_mtu = dev->mtu;

    if(!netif_running(dev)) {
    	if(mv_eth_change_mtu_internals(dev, mtu) == -1) {
            goto error;
    }
        printk( KERN_NOTICE "%s: change mtu %d (buffer-size %d) to %d (buffer-size %d)\n",
                dev->name, old_mtu, MV_RX_BUF_SIZE( old_mtu), 
                dev->mtu, MV_RX_BUF_SIZE( dev->mtu) );
        return 0;
    }

    if( mv_eth_stop( dev )) {
        printk( KERN_ERR "%s: stop interface failed\n", dev->name );
        goto error;
    }

    if(mv_eth_change_mtu_internals(dev, mtu) == -1) {
        goto error;
    }

    if(mv_eth_start( dev )) {
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
        goto error;
    } 
    printk( KERN_NOTICE "%s: change mtu %d (buffer-size %d) to %d (buffer-size %d)\n",
                dev->name, old_mtu, MV_RX_BUF_SIZE(old_mtu), dev->mtu, 
                MV_RX_BUF_SIZE(dev->mtu));
 
    return 0;

 error:
    printk( "%s: change mtu failed\n", dev->name );
    return -1;
}

/*********************************************************** 
 * eth_set_mac_addr --                                   *
 *   stop port activity. set new addr in device and hw.    *
 *   restart port activity.                                *
 ***********************************************************/
static int mv_eth_set_mac_addr_internals(struct net_device *dev, void *addr )
{
    mv_eth_priv *priv = MV_ETH_PRIV(dev);
    u8          *mac = &(((u8*)addr)[2]);  /* skip on first 2B (ether HW addr type) */
    int i;

    /* remove previous address table entry */
    if( mvEthMacAddrSet( priv->hal_priv, dev->dev_addr, -1) != MV_OK ) {
        printk( KERN_ERR "%s: ethSetMacAddr failed\n", dev->name );
        return -1;
    }

    /* set new addr in hw */
    if( mvEthMacAddrSet( priv->hal_priv, mac, ETH_DEF_RXQ) != MV_OK ) {
        printk( KERN_ERR "%s: ethSetMacAddr failed\n", dev->name );
    return -1;
    }

    /* set addr in the device */ 
    for( i = 0; i < 6; i++ )
        dev->dev_addr[i] = mac[i];

    printk( KERN_NOTICE "%s: mac address changed\n", dev->name );

    return 0;
}

/***********************************************************
 * eth_set_multicast_list --                             *
 *   Add multicast addresses or set promiscuous mode.      *
 *   This function should have been but was not included   *
 *   by Marvell. -bbozarth                                 *
 ***********************************************************/
void mv_eth_set_multicast_list(struct net_device *dev) {

     mv_eth_priv        *priv = MV_ETH_PRIV(dev);
     int                queue = ETH_DEF_RXQ;
     struct dev_mc_list *curr_addr = dev->mc_list;
     int                i;

     if (dev->flags & IFF_PROMISC)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 1);
     }
     else if (dev->flags & IFF_ALLMULTI)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 0);
        mvEthMacAddrSet(priv->hal_priv, dev->dev_addr, queue);
        mvEthSetSpecialMcastTable(priv->port, queue);
        mvEthSetOtherMcastTable(priv->port, queue);
     }
     else if (dev->mc_count)
     {
        mvEthRxFilterModeSet(priv->hal_priv, 0);
        mvEthMacAddrSet(priv->hal_priv, dev->dev_addr, queue);
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
        mvEthMacAddrSet(priv->hal_priv, dev->dev_addr, queue);
     }
}


int     mv_eth_set_mac_addr( struct net_device *dev, void *addr )
{
    if(!netif_running(dev)) {
        if(mv_eth_set_mac_addr_internals(dev, addr) == -1)
            goto error;
        return 0;
    }

    if( mv_eth_stop( dev )) {
        printk( KERN_ERR "%s: stop interface failed\n", dev->name );
        goto error;
    }

    if(mv_eth_set_mac_addr_internals(dev, addr) == -1)
        goto error;

    if(mv_eth_start( dev )) {
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
    goto error;
    } 

    return 0;

 error:
    printk( "%s: set mac addr failed\n", dev->name );
    return -1;
}


/************************************************************ 
 * mv_eth_open -- Restore MAC address and call to   *
 *                mv_eth_start                               *
 ************************************************************/
int mv_eth_open( struct net_device *dev )
{
    mv_eth_priv	*priv = MV_ETH_PRIV(dev);
    int         queue = ETH_DEF_RXQ;

    if( mvEthMacAddrSet( priv->hal_priv, dev->dev_addr, queue) != MV_OK ) {
        printk( KERN_ERR "%s: ethSetMacAddr failed\n", dev->name );
        return -1;
    }

    if(mv_eth_start( dev )){
        printk( KERN_ERR "%s: start interface failed\n", dev->name );
        return -1;
    } 
    return 0;
}
