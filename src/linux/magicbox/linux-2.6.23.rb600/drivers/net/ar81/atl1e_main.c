/*
 * Copyright(c) 2007 Atheros Corporation. All rights reserved.
 *
 * Derived from Intel e1000 driver
 * Copyright(c) 1999 - 2005 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "at_common.h"
#include "atl1e.h"

#ifdef CONFIG_AT_MQ
#include <linux/cpu.h>
#include <linux/smp.h>
#endif
char atl1e_driver_name[] = "ATL1e";
char atl1e_driver_version[] = "1.0.0.4";

/*
 * atl1e_pci_tbl - PCI Device ID Table
 *
 * Wildcard entries (PCI_ANY_ID) should come last
 * Last entry must be all 0s
 *
 * { Vendor ID, Device ID, SubVendor ID, SubDevice ID,
 *   Class, Class Mask, private data (not used) }
 */
int atl1e_up(struct atl1e_adapter *adapter);
void atl1e_down(struct atl1e_adapter *adapter);
int atl1e_reset(struct atl1e_adapter *adapter);
s32 atl1e_setup_ring_resources(struct atl1e_adapter *adapter);
void atl1e_free_ring_resources(struct atl1e_adapter *adapter);
void atl1e_reinit_locked(struct atl1e_adapter *adapter);


/* Local Function Prototypes */
int atl1e_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static int atl1e_alloc_queues(struct atl1e_adapter *adapter);
#ifdef CONFIG_AT_MQ
static void atl1e_setup_queue_mapping(struct atl1e_adapter *adapter);
void atl1e_rx_schedule(void *data);
#endif

#ifdef CONFIG_AT_NAPI
static int atl1e_clean(struct net_device *poll_dev, int *budget);
#endif

void __devexit atl1e_remove(struct pci_dev *pdev);
static int atl1e_sw_init(struct atl1e_adapter *adapter);
static int atl1e_open(struct net_device *netdev);
static int atl1e_close(struct net_device *netdev);
static int atl1e_xmit_frame(struct sk_buff *skb, struct net_device *netdev);
static struct net_device_stats * atl1e_get_stats(struct net_device *netdev);
static int atl1e_tso(struct atl1e_adapter* adapter, struct sk_buff *skb, TpdDescr* pTpd);
static void atl1e_tx_map(struct atl1e_adapter *adapter, struct sk_buff *skb, TpdDescr* pTpd);
static void atl1e_tx_queue(struct atl1e_adapter *adapter, u16 count, TpdDescr* pTpd);
static int atl1e_tx_csum(struct atl1e_adapter *adapter, struct sk_buff *skb, TpdDescr* pTpd);
static int atl1e_change_mtu(struct net_device *netdev, int new_mtu);
static void atl1e_set_multi(struct net_device *netdev);
static int atl1e_set_mac(struct net_device *netdev, void *p);
static int atl1e_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
static void atl1e_tx_timeout(struct net_device *dev);
static irqreturn_t atl1e_intr(int irq, void *data);
static boolean_t atl1e_clean_tx_irq(struct atl1e_adapter* adapter);
static void atl1e_watchdog(unsigned long data);
static void atl1e_phy_config(unsigned long data);
static void atl1e_reset_task(struct work_struct *work);
static void atl1e_link_chg_task(struct work_struct *work);
static void atl1e_check_for_link(struct atl1e_adapter* adapter);
void atl1e_set_ethtool_ops(struct net_device *netdev);
#ifdef ETHTOOL_OPS_COMPAT
extern int ethtool_ioctl(struct ifreq *ifr);
#endif
static int atl1e_check_link(struct atl1e_adapter* adapter);
void init_ring_ptrs(struct atl1e_adapter *adapter);
static s32 atl1e_configure(struct atl1e_adapter *adapter);
#ifdef CONFIG_AT_NAPI
static void atl1e_clean_rx_irq(struct atl1e_adapter *adapter, u8 que, int *work_done, int work_to_do);
#else
static void atl1e_clean_rx_irq(struct atl1e_adapter *adapter, u8 que);
#endif

static void atl1e_clean_tx_ring(struct atl1e_adapter *adapter);
static void atl1e_clean_rx_ring(struct atl1e_adapter *adapter);

#ifdef SIOCGMIIPHY
static int atl1e_mii_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
#endif


#ifdef NETIF_F_HW_VLAN_TX
static void atl1e_vlan_rx_register(struct net_device *netdev, struct vlan_group *grp);
static void atl1e_restore_vlan(struct atl1e_adapter *adapter);
#endif

int atl1e_suspend(struct pci_dev *pdev, pm_message_t state);
#ifdef CONFIG_PM
int atl1e_resume(struct pci_dev *pdev);
#endif

#ifndef USE_REBOOT_NOTIFIER
void atl1e_shutdown(struct pci_dev *pdev);
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/* for netdump / net console */
static void atl1e_netpoll (struct net_device *netdev);
#endif


/* Exported from other modules */

extern void atl1e_check_options(struct atl1e_adapter *adapter);

static int atl1e_request_irq(struct atl1e_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    int flags, err = 0;

    flags = IRQF_SHARED;
#ifdef CONFIG_PCI_MSI
    adapter->have_msi = TRUE;
    if ((err = pci_enable_msi(adapter->pdev))) {
        AT_DBG("Unable to allocate MSI interrupt Error: %d\n", err);
        adapter->have_msi = FALSE;
    }
    
    if (adapter->have_msi) {
        flags &= ~IRQF_SHARED;
    }
#endif
    netdev->irq = adapter->pdev->irq;
    if ((err = request_irq(adapter->pdev->irq, atl1e_intr, flags,
                           netdev->name, netdev))) {
        AT_DBG("Unable to allocate interrupt Error: %d\n", err);                            
    }
    
    AT_DBG("atl1e_request_irq OK\n");

    return err;
}


static void atl1e_free_irq(struct atl1e_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;

    free_irq(adapter->pdev->irq, netdev);

#ifdef CONFIG_PCI_MSI
    if (adapter->have_msi)
        pci_disable_msi(adapter->pdev);
#endif
}


static void atl1e_setup_pcicmd(struct pci_dev* pdev)
{
    u16 cmd;

    pci_read_config_word(pdev, PCI_COMMAND, &cmd);
    
    if (cmd & PCI_COMMAND_INTX_DISABLE)
        cmd &= ~PCI_COMMAND_INTX_DISABLE;
    if (cmd & PCI_COMMAND_IO)
        cmd &= ~PCI_COMMAND_IO;
    if (0 == (cmd & PCI_COMMAND_MEMORY))
        cmd |= PCI_COMMAND_MEMORY;
    if (0 == (cmd & PCI_COMMAND_MASTER))
        cmd |= PCI_COMMAND_MASTER;
    pci_write_config_word(pdev, PCI_COMMAND, cmd);
    
    /* 
     * some motherboards BIOS(PXE/EFI) driver may set PME
     * while they transfer control to OS (Windows/Linux)
     * so we should clear this bit before NIC work normally
     */
    pci_write_config_dword(pdev, REG_PM_CTRLSTAT, 0);  
    msec_delay(1);         
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) )
static const struct net_device_ops atl1e_netdev_ops = {
        .ndo_open               = atl1e_open,
        .ndo_stop               = atl1e_close,
        .ndo_validate_addr      = eth_validate_addr,
        .ndo_start_xmit         = atl1e_xmit_frame,
        .ndo_set_multicast_list = atl1e_set_multi,
        .ndo_change_mtu         = atl1e_change_mtu,
        .ndo_set_mac_address    = atl1e_set_mac,
        .ndo_do_ioctl           = atl1e_ioctl,
        .ndo_tx_timeout         = atl1e_tx_timeout,
        .ndo_get_stats          = atl1e_get_stats,
        .ndo_vlan_rx_register   = atl1e_vlan_rx_register,
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller    = atl1e_netpoll,
#endif
};
#endif

/**
 * atl1e_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in atl1e_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * atl1e_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/

int __devinit
atl1e_probe(struct pci_dev *pdev,
            const struct pci_device_id *ent)
{
    struct net_device *netdev;
    struct atl1e_adapter *adapter;
    unsigned long mmio_start;
    int mmio_len;
    boolean_t pci_using_64 = TRUE;
    int err;
#ifdef CONFIG_AT_NAPI
    int i;
#endif

    DEBUGFUNC("atl1e_probe !");

    if((err = pci_enable_device(pdev)))
        return err;

    if (!(err = pci_set_dma_mask(pdev, DMA_64BIT_MASK)) &&
        !(err = pci_set_consistent_dma_mask(pdev, DMA_64BIT_MASK))) {
        pci_using_64 = TRUE; 
    } else {
        if ((err = pci_set_dma_mask(pdev, DMA_32BIT_MASK)) &&
            (err = pci_set_consistent_dma_mask(pdev, DMA_32BIT_MASK))) {
            AT_ERR("No usable DMA configuration, aborting\n");
            goto err_dma;
        }
        pci_using_64 = FALSE;
    }       
        
   
    // Mark all PCI regions associated with PCI device 
    // pdev as being reserved by owner atl1e_driver_name
    if((err = pci_request_regions(pdev, atl1e_driver_name)))
       goto err_pci_reg;

    // Enables bus-mastering on the device and calls 
    // pcibios_set_master to do the needed arch specific settings
    pci_set_master(pdev);

    err = -ENOMEM;
    netdev = alloc_etherdev(sizeof(struct atl1e_adapter));
    if(!netdev)
        goto err_alloc_etherdev;
    
    SET_MODULE_OWNER(netdev);
    SET_NETDEV_DEV(netdev, &pdev->dev);

    pci_set_drvdata(pdev, netdev);
    adapter = netdev_priv(netdev);
    adapter->netdev = netdev;
    adapter->pdev = pdev;
    adapter->hw.back = adapter;

    mmio_start = pci_resource_start(pdev, BAR_0);
    mmio_len = pci_resource_len(pdev, BAR_0);

    AT_DBG("base memory = %lx memory length = %x \n", 
        mmio_start, mmio_len);
    adapter->hw.mem_rang = (u32)mmio_len;
    adapter->hw.hw_addr = ioremap(mmio_start, mmio_len);
    if(!adapter->hw.hw_addr) {
        err = -EIO;
        goto err_ioremap;
    }
    
    atl1e_setup_pcicmd(pdev);

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30) )  
    netdev->base_addr = (unsigned long)adapter->hw.hw_addr; 
    netdev->open = &atl1e_open;
    netdev->stop = &atl1e_close;
    netdev->hard_start_xmit = &atl1e_xmit_frame;
    netdev->get_stats = &atl1e_get_stats;
    netdev->set_multicast_list = &atl1e_set_multi;
    netdev->set_mac_address = &atl1e_set_mac;
    netdev->change_mtu = &atl1e_change_mtu;
    netdev->do_ioctl = &atl1e_ioctl;
    netdev->tx_timeout = &atl1e_tx_timeout;
#ifdef CONFIG_AT_NAPI
    netdev->weight = 64;
    netdev->poll = &atl1e_clean;
#endif

#ifdef NETIF_F_HW_VLAN_TX
    netdev->vlan_rx_register = atl1e_vlan_rx_register;
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
    netdev->poll_controller = atl1e_netpoll;
#endif
#else
    netdev->netdev_ops = &atl1e_netdev_ops;	
#endif

    atl1e_set_ethtool_ops(netdev);
/*
printk("tdp:(%lu)(%lu), rrs(%lu),rx-flag(%lu), Xsum(%lu)\n",
       sizeof(TpdDescr), sizeof(TpdIPv6Descr),
       sizeof(RecvRetStatus), sizeof(RxFlags),
       sizeof(TpdCXsumDescr));
*/


#ifdef HAVE_TX_TIMEOUT
    netdev->watchdog_timeo = 5 * HZ;
#endif
    strncpy(netdev->name, pci_name(pdev), sizeof(netdev->name) - 1);
    
    netdev->mem_start = mmio_start;
    netdev->mem_end = mmio_start + mmio_len;
    adapter->bd_number = cards_found;
    adapter->pci_using_64 = pci_using_64;

    /* setup the private structure */

    if((err = atl1e_sw_init(adapter)))
        goto err_sw_init;

    err = -EIO;

#ifdef MAX_SKB_FRAGS
#ifdef NETIF_F_HW_VLAN_TX
        netdev->features = NETIF_F_SG |
                   NETIF_F_HW_CSUM |
                   NETIF_F_HW_VLAN_TX |
                   NETIF_F_HW_VLAN_RX ;
#else
        netdev->features = NETIF_F_SG | NETIF_F_HW_CSUM;
#endif

#ifdef NETIF_F_TSO
        netdev->features |= NETIF_F_TSO;
#ifdef NETIF_F_TSO6
        netdev->features |= NETIF_F_TSO6;
#endif//NETIF_F_TSO6
#endif//NETIF_F_TSO
#endif//MAX_SKB_FRAGS

#ifdef NETIF_F_LLTX
    netdev->features |= NETIF_F_LLTX;
#endif

    if(pci_using_64) {
        netdev->features |= NETIF_F_HIGHDMA;
        AT_DBG("pci using 64bit address\n");
    }
    
    /* get user settings */
    atl1e_check_options(adapter);

    /* Init GPHY as early as possible due to power saving issue  */
    atl1e_phy_init(&adapter->hw);
    
    
    /* reset the controller to 
     * put the device in a known good starting state */
    
    if (atl1e_reset_hw(&adapter->hw)) {
        err = -EIO;
        goto err_reset;
    }

    /* copy the MAC address out of the EEPROM */

    atl1e_read_mac_addr(&adapter->hw);
    memcpy(netdev->dev_addr, adapter->hw.mac_addr, netdev->addr_len);
#ifdef ETHTOOL_GPERMADDR
    memcpy(netdev->perm_addr, adapter->hw.mac_addr, netdev->addr_len);

    if (!is_valid_ether_addr(netdev->perm_addr)) {
#else
    if (!is_valid_ether_addr(netdev->dev_addr)) {
#endif
        AT_DBG("Invalid MAC Address\n");
        err = -EIO;
        goto err_eeprom;
    }
    AT_DBG("mac address : %02x-%02x-%02x-%02x-%02x-%02x\n",
        adapter->hw.mac_addr[0],
        adapter->hw.mac_addr[1],
        adapter->hw.mac_addr[2],
        adapter->hw.mac_addr[3],
        adapter->hw.mac_addr[4],
        adapter->hw.mac_addr[5] );

    init_timer(&adapter->watchdog_timer);
    adapter->watchdog_timer.function = &atl1e_watchdog;
    adapter->watchdog_timer.data = (unsigned long) adapter;
    
    init_timer(&adapter->phy_config_timer);
    adapter->phy_config_timer.function = &atl1e_phy_config;
    adapter->phy_config_timer.data = (unsigned long) adapter;
    
    INIT_WORK(&adapter->reset_task, atl1e_reset_task);
    INIT_WORK(&adapter->link_chg_task, atl1e_link_chg_task);


    strcpy(netdev->name, "eth%d"); // ??
    if((err = register_netdev(netdev)))
        goto err_register;

    /* assume we have no link for now */
    netif_carrier_off(netdev);
    netif_stop_queue(netdev);
    
    
    cards_found++;

    return 0;

//err_init_hw:
err_reset:
err_register:   
err_sw_init:
err_eeprom:
    
#ifdef CONFIG_AT_NAPI
    if (adapter->polling_netdev) {
        for (i = 0; i < adapter->num_rx_queues; i++)
            dev_put(&adapter->polling_netdev[i]);   
        kfree(adapter->polling_netdev);
    }
 #endif 
 
 #ifdef CONFIG_AT_MQ
    if (adapter->cpu_netdev) {
        free_percpu(adapter->cpu_netdev);
    }
#endif
 
    iounmap(adapter->hw.hw_addr);
err_ioremap:
    free_netdev(netdev);
err_alloc_etherdev:
    pci_release_regions(pdev);
err_pci_reg:
err_dma:
    pci_disable_device(pdev);
    return err;
}

/**
 * atl1e_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * atl1e_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/

void __devexit
atl1e_remove(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev);
#ifdef CONFIG_AT_NAPI
    int i;
#endif
    
    DEBUGFUNC("atl1e_remove");


    /* flush_scheduled work may reschedule our watchdog task, so
     * explicitly disable watchdog tasks from being rescheduled  */
    set_bit(__AT_DOWN, &adapter->flags);

    del_timer_sync(&adapter->watchdog_timer);
    del_timer_sync(&adapter->phy_config_timer);

    flush_scheduled_work();

    unregister_netdev(netdev);

#ifdef CONFIG_AT_NAPI
    for (i = 0; i < adapter->num_rx_queues; i++)
        dev_put(&adapter->polling_netdev[i]);

    kfree(adapter->polling_netdev);
    
#endif
 
    atl1e_force_ps(&adapter->hw);
    
    iounmap(adapter->hw.hw_addr);
    pci_release_regions(pdev);

 #ifdef CONFIG_AT_MQ
    free_percpu(adapter->cpu_netdev);//free previously allocated percpu memory  ....arg-->>pointer returned by alloc_percpu.
 #endif
 
    free_netdev(netdev);

    pci_disable_device(pdev);
}


#ifndef USE_REBOOT_NOTIFIER
void atl1e_shutdown(struct pci_dev *pdev)
{
    atl1e_suspend(pdev, PMSG_SUSPEND);
}
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void
atl1e_netpoll(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);

    disable_irq(adapter->pdev->irq);
    atl1e_intr(adapter->pdev->irq, netdev);
    atl1e_clean_tx_irq(adapter);
#ifndef CONFIG_AT_NAPI
    atl1e_clean_rx_irq(adapter, 0);
#endif
    enable_irq(adapter->pdev->irq);
}
#endif

static int atl1e_power_ctrl_suspend_init(struct atl1e_hw *hw)
{
    return hw->ops.power_ctrl_suspend_init(hw);
}

int
atl1e_suspend(struct pci_dev *pdev, pm_message_t state)
{
#define AT_SUSPEND_LINK_TIMEOUT 50
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    struct atl1e_hw * hw = &adapter->hw;
    //u16 speed, duplex;
    u32 ctrl = 0;
    u32 mac_ctrl_data = 0;
    u32 wol_ctrl_data = 0;
    u16 mii_advertise_data = 0;
    u16 mii_bmsr_data = 0;
    u16 mii_intr_status_data = 0;
    u32 wufc = adapter->wol;
    u32 i;
    int retval = 0;

    DEBUGFUNC("atl1e_suspend !"); 

    if (netif_running(netdev)) {
        WARN_ON(test_bit(__AT_RESETTING, &adapter->flags));
        atl1e_down(adapter);
    }

    netif_device_detach(netdev);

#ifdef CONFIG_PM
    retval = pci_save_state(pdev);
    if (retval)
        return retval;
#endif
    retval = atl1e_power_ctrl_suspend_init(hw);
    if (retval)
        return retval;
    
    if (wufc) {
        /* get link status */  
        atl1e_read_phy_reg(hw, MII_BMSR, (u16 *)&mii_bmsr_data);
        atl1e_read_phy_reg(hw, MII_BMSR, (u16 *)&mii_bmsr_data);
        
        mii_advertise_data = MII_AR_10T_HD_CAPS; 
        printk(KERN_DEBUG "THe mii advertise data is %x\n", mii_advertise_data);

        if ((atl1e_write_phy_reg(hw, MII_ADVERTISE, mii_advertise_data) != 0) ||
	    (atl1e_write_phy_reg(hw, MII_AT001_CR, 0) != 0) || 
		/* not advertise 1000M capacity */
            (atl1e_phy_commit(hw)) != 0) { 
            printk(KERN_DEBUG "set phy register failed\n");
        }
                 
        hw->phy_configured = FALSE; /* re-init PHY when resume */
          
        /* turn on magic packet wol */
        if (wufc & AT_WUFC_MAG) {
            wol_ctrl_data |= WOL_MAGIC_EN | WOL_MAGIC_PME_EN;
        }
        if (wufc & AT_WUFC_LNKC) {
            /* if orignal link status is link, just wait for retrive link */ 
            for (i=0; i < AT_SUSPEND_LINK_TIMEOUT; i++) {
                msec_delay(100);
                atl1e_read_phy_reg(hw, MII_BMSR, (u16 *)&mii_bmsr_data); 
                if (mii_bmsr_data & BMSR_LSTATUS) {
                    break;
                }
            }
            
            if (0 == (mii_bmsr_data & BMSR_LSTATUS)) {
                printk(KERN_DEBUG "%s: Link may change when suspend\n",
                    atl1e_driver_name);
            }
            wol_ctrl_data |=  WOL_LINK_CHG_EN | WOL_LINK_CHG_PME_EN; 
            /* only link up can wake up */
            if (atl1e_write_phy_reg(hw, 18, 0x400) != 0) {
                printk(KERN_DEBUG "%s: read write phy register failed.\n",
                        atl1e_driver_name);
                goto wol_dis;
            }
        }
        /* clear phy interrupt */
        atl1e_read_phy_reg(hw, 19, &mii_intr_status_data); 
        /* Config MAC Ctrl register */
        mac_ctrl_data = MAC_CTRL_RX_EN;
        /* set to 10/100M halt duplex */
        mac_ctrl_data |= MAC_CTRL_SPEED_10_100 << MAC_CTRL_SPEED_SHIFT;
        mac_ctrl_data |= (((u32)adapter->hw.preamble_len & MAC_CTRL_PRMLEN_MASK) 
                        << MAC_CTRL_PRMLEN_SHIFT);
        if (adapter->vlgrp) {
            mac_ctrl_data |= MAC_CTRL_RMV_VLAN;
        }
        if (wufc & AT_WUFC_MAG) {
            /* magic packet maybe Broadcast&multicast&Unicast frame */
            mac_ctrl_data |= MAC_CTRL_BC_EN;
        }
        AT_DBG("%s: suspend MAC=0x%x\n", atl1e_driver_name, mac_ctrl_data);
            
        AT_WRITE_REG(hw, REG_WOL_CTRL, wol_ctrl_data);
        AT_WRITE_REG(hw, REG_MAC_CTRL, mac_ctrl_data);
        /* pcie patch */
        ctrl = AT_READ_REG(hw, REG_PCIE_PHYMISC);
        ctrl |= PCIE_PHYMISC_FORCE_RCV_DET;
        AT_WRITE_REG(hw, REG_PCIE_PHYMISC, ctrl);
        pci_enable_wake(pdev, pci_choose_state(pdev, state), 1);
        goto suspend_exit; 
    }    
wol_dis:
    
    /* WOL disabled */
    AT_WRITE_REG(hw, REG_WOL_CTRL, 0);
    
    /* pcie patch */
    ctrl = AT_READ_REG(hw, REG_PCIE_PHYMISC);
    ctrl |= PCIE_PHYMISC_FORCE_RCV_DET;
    AT_WRITE_REG(hw, REG_PCIE_PHYMISC, ctrl);               
    
    atl1e_force_ps(hw);
    hw->phy_configured = FALSE; /* re-init PHY when resume */
    
    pci_enable_wake(pdev, pci_choose_state(pdev, state), 0);

suspend_exit:   

    if (netif_running(netdev))
        atl1e_free_irq(adapter);

    pci_disable_device(pdev);

    pci_set_power_state(pdev, pci_choose_state(pdev, state));

    return 0;
#undef AT_SUSPEND_LINK_TIMEOUT
}


#ifdef CONFIG_PM
int atl1e_resume(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    u32 err;

    DEBUGFUNC("atl1e_resume !");

    pci_set_power_state(pdev, PCI_D0);
    pci_restore_state(pdev);
    
    if ((err = pci_enable_device(pdev))) {
        printk(KERN_ERR "ATL1e: Cannot enable PCI device from suspend\n");
        return err;
    }
    
    pci_set_master(pdev);
    
    AT_READ_REG(&adapter->hw, REG_WOL_CTRL); /* clear WOL status */

    pci_enable_wake(pdev, PCI_D3hot, 0);
    pci_enable_wake(pdev, PCI_D3cold, 0);
    
    AT_WRITE_REG(&adapter->hw, REG_WOL_CTRL, 0);
    
    if (netif_running(netdev) && (err = atl1e_request_irq(adapter)))
        return err;

    atl1e_reset_hw(&adapter->hw);

    if(netif_running(netdev))
        atl1e_up(adapter);

    netif_device_attach(netdev);

    return 0;
}
#endif


#ifdef CONFIG_AT_PCI_ERS
/**
 * atl1e_io_error_detected - called when PCI error is detected
 * @pdev: Pointer to PCI device
 * @state: The current pci connection state
 *
 * This function is called after a PCI bus error affecting
 * this device has been detected.
 */
pci_ers_result_t atl1e_io_error_detected(struct pci_dev *pdev, pci_channel_state_t state)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev); 

    netif_device_detach(netdev);

    if (netif_running(netdev))
        atl1e_down(adapter);
        
    pci_disable_device(pdev);

    /* Request a slot slot reset. */
    return PCI_ERS_RESULT_NEED_RESET;
}

/**
 * atl1e_io_slot_reset - called after the pci bus has been reset.
 * @pdev: Pointer to PCI device
 *
 * Restart the card from scratch, as if from a cold-boot. Implementation
 * resembles the first-half of the e1000_resume routine.
 */
pci_ers_result_t atl1e_io_slot_reset(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev); 

    if (pci_enable_device(pdev)) {
        printk(KERN_ERR "ATL1e: Cannot re-enable PCI device after reset.\n");
        return PCI_ERS_RESULT_DISCONNECT;
    }
    pci_set_master(pdev);

    pci_enable_wake(pdev, PCI_D3hot, 0);
    pci_enable_wake(pdev, PCI_D3cold, 0);

    atl1e_reset_hw(&adapter->hw);

    return PCI_ERS_RESULT_RECOVERED;
}

/**
 * atl1e_io_resume - called when traffic can start flowing again.
 * @pdev: Pointer to PCI device
 *
 * This callback is called when the error recovery driver tells us that
 * its OK to resume normal operation. Implementation resembles the
 * second-half of the atl1e_resume routine.
 */
void atl1e_io_resume(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct atl1e_adapter *adapter = netdev_priv(netdev); 

    if (netif_running(netdev)) {
        if (atl1e_up(adapter)) {
            printk("ATL1e: can't bring device back up after reset\n");
            return;
        }
    }

    netif_device_attach(netdev);
}
#endif /* CONFIG_AT_PCI_ERS */



/**
 * atl1e_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 **/

inline void
atl1e_irq_enable(struct atl1e_adapter *adapter)
{
    if (likely(atomic_dec_and_test(&adapter->irq_sem))) {
        AT_WRITE_REG(&adapter->hw, REG_ISR, 0);
        AT_WRITE_REG(&adapter->hw, REG_IMR, IMR_NORMAL_MASK);
        AT_WRITE_FLUSH(&adapter->hw);
    }
}

/**
 * atl1e_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 **/

inline void
atl1e_irq_disable(struct atl1e_adapter *adapter)
{
    atomic_inc(&adapter->irq_sem);
    AT_WRITE_REG(&adapter->hw, REG_IMR, 0);
    AT_WRITE_FLUSH(&adapter->hw);
    synchronize_irq(adapter->pdev->irq);
}

static int atl1e_set_mac_type(struct atl1e_hw *hw)
{
    int ret = 0;
    u32 val;
    
    val = AT_READ_REG(hw, REG_PHY_STATUS);
    if (val & PHY_STATUS_EMI_CA) 
        hw->emi_ca = TRUE;
    else
        hw->emi_ca = FALSE;
 
    switch (hw->device_id) {
    case DEV_ID_ATL1E: 
        if (hw->revision_id >= 0xF0) {
            hw->nic_type = athr_l2e_revB;
        } else {
            if (val & PHY_STATUS_100M)
                hw->nic_type = athr_l1e;
            else
                hw->nic_type = athr_l2e_revA;
        } 
        break;
    
    case DEV_ID_ATL1C:
        hw->nic_type = athr_l1c_revA;
        break;
    
    case DEV_ID_ATL2C:
        hw->nic_type = athr_l2c_revA;
        break; 
    
    default:
        break;
    }
    
    return ret;
}

static int atl1e_null_ops_generic(struct atl1e_hw *hw)
{
    return 0;
}

static int atl1c_power_ctrl_init(struct atl1e_hw *hw)
{
#if 0    
    u32 power_ctrl_data;
    u32 pcie_pm_data;
    int update_pcie_pm = 0;
 
    power_ctrl_data = (AT_READ_REG(hw, REG_LINK_CTRL) >> 
            LINK_CTRL_ASPM_SHIFT) & LINK_CTRL_ASPM_MASK;
    if (power_ctrl_data & LINK_POWER_ASPM_L0S_EN)
        hw->power_ctrl_flags |= POWER_ASPM_L0S_EN;
    if (power_ctrl_data & LINK_POWER_ASPM_L1_EN);
        hw->power_ctrl_flags |= POWER_ASPM_L1_EN;
   
    pcie_pm_data = AT_READ_REG(hw, REG_PCIE_PM_CTRL);
    /* disable L0S/L1 before link up */
    if (hw->power_ctrl_flags & POWER_ASPM_L0S_EN) {
        pcie_pm_data &= ~PCIE_PM_EN_ASPM_L0S;
        update_pcie_pm = 1;
    }
    if (hw->power_ctrl_flags & POWER_ASPM_L1_EN) {
        pcie_pm_data &= ~PCIE_PM_EN_ASPM_L1;
        update_pcie_pm = 1;
    }
    
    if (update_pcie_pm)
        AT_WRITE_REG(hw, REG_PCIE_PM_CTRL, pcie_pm_data);
#endif
#if 0    
    hw->power_ctrl_flags |= PCIE_PM_EN_ASPM_L1;
#endif
    AT_WRITE_REG(hw, REG_LTSSM_TEST_MODE, 0x6800);
    return 0;
}

static int atl1c_power_ctrl_link_chg(struct atl1e_hw *hw)
{
    struct atl1e_adapter *adapter = (struct atl1e_adapter *) hw->back;
    u32 pcie_pm_data;
      
    pcie_pm_data = AT_READ_REG(hw, REG_PCIE_PM_CTRL);

    if (adapter->link_speed == SPEED_0) {
        /* link down */
        if (hw->power_ctrl_flags & PCIE_PM_EN_ASPM_L0S)
            pcie_pm_data &= PCIE_PM_EN_ASPM_L0S;
        if (hw->power_ctrl_flags & PCIE_PM_EN_ASPM_L1) {
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_L1;
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_PLL_L1;
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_RX_L1;
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_BUFS_RX_L1;
            pcie_pm_data &= ~(PCIE_PM_L1_ENTRY_TIMER_MASK
                    << PCIE_PM_L1_ENTRY_TIMER_SHIFT);
            pcie_pm_data |= PCIE_PM_EN_CLK_SWH_L1;
        } 
    } else {
        /* link up */
        if (hw->power_ctrl_flags & PCIE_PM_EN_ASPM_L0S)
            pcie_pm_data |= PCIE_PM_EN_ASPM_L0S; 
        if (hw->power_ctrl_flags & PCIE_PM_EN_ASPM_L1) {
            pcie_pm_data |= PCIE_PM_EN_SERDES_L1;
            pcie_pm_data |= PCIE_PM_EN_SERDES_PLL_L1;
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_RX_L1;
            pcie_pm_data &= ~PCIE_PM_EN_CLK_SWH_L1;
            pcie_pm_data &= ~PCIE_PM_EN_SERDES_BUFS_RX_L1;
            pcie_pm_data &= ~(PCIE_PM_L1_ENTRY_TIMER_MASK 
                    << PCIE_PM_L1_ENTRY_TIMER_SHIFT);
            pcie_pm_data |= (hw->pm_l1_timer & PCIE_PM_L1_ENTRY_TIMER_MASK) 
                        << PCIE_PM_L1_ENTRY_TIMER_SHIFT;
        }
    } 
    AT_WRITE_REG(hw, REG_PCIE_PM_CTRL, pcie_pm_data);
    return 0;
}

static int atl1e_com_phy_ctrl_link_chg(struct atl1e_hw *hw)
{
    struct atl1e_adapter *adapter = (struct atl1e_adapter *) hw->back;
    
    if (adapter->link_speed == SPEED_0) {
        if (atl1e_write_phy_reg(hw, MII_DBG_ADDR, 0x18) != 0)
            return -1;
        if (atl1e_write_phy_reg(hw, MII_DBG_DATA, 0x2ea) != 0)
            return -1;
    } else if (adapter->link_speed == SPEED_100){
        if (atl1e_write_phy_reg(hw, MII_DBG_ADDR, 0x18) != 0)
            return -1;
        if (atl1e_write_phy_reg(hw, MII_DBG_DATA, 0xba8) != 0)
            return -1;
    } 
    
    return 0;
}

int atl1e_phy_ctrl_link_chg_pre(struct atl1e_hw *hw)
{
    if (atl1e_write_phy_reg(hw, MII_DBG_ADDR, 0x18) != 0)
        return -1;
    if (atl1e_write_phy_reg(hw, MII_DBG_DATA, 0x2ea) != 0)
        return -1;
    return 0;
}

#if 0
static int atl1c_power_suspend_init(struct atl1e_hw *hw)
{
    return hw->ops.power_ctrl_init(hw); 
}
#endif

static void atl1e_init_hw_ops_generic(struct atl1e_hw *hw)
{
    hw->ops.power_ctrl_init = atl1e_null_ops_generic; 
    hw->ops.power_ctrl_link_chg = atl1e_null_ops_generic;
    hw->ops.power_ctrl_suspend_init = atl1e_null_ops_generic;
    hw->ops.power_ctrl_suspend_exit = atl1e_null_ops_generic; 
    hw->ops.blink_led = atl1e_null_ops_generic;
    hw->ops.phy_ctrl_link_chg = atl1e_com_phy_ctrl_link_chg;
    return;
}

static int atl1e_setup_mac_funcs(struct atl1e_hw *hw)
{
    int ret;

    ret = atl1e_set_mac_type(hw);
    if (ret) {
        DEBUGOUT("MAC type could not be set properly\n");
        goto out; 
    }
    hw->power_ctrl_flags = 0;
    hw->link_cap_flags = 0;
    
    atl1e_init_hw_ops_generic(hw); 
    switch (hw->nic_type) {
    case athr_l1c_revA:
            hw->link_cap_flags |= LINK_CAP_SPEED_1000;
            hw->pm_l0s_timer = 6;
            hw->pm_l1_timer = 12;
            hw->ops.power_ctrl_init = atl1c_power_ctrl_init;
            hw->ops.power_ctrl_link_chg = atl1c_power_ctrl_link_chg;
            hw->ops.power_ctrl_suspend_init = atl1c_power_ctrl_link_chg;
        break;
    case athr_l2c_revA:
            hw->pm_l0s_timer = 6;
            hw->pm_l1_timer = 12;
            hw->ops.power_ctrl_init = atl1c_power_ctrl_init;
            hw->ops.power_ctrl_link_chg = atl1c_power_ctrl_link_chg;
            hw->ops.power_ctrl_suspend_init = atl1c_power_ctrl_link_chg;
        break;
    case athr_l1e:
            hw->link_cap_flags |= LINK_CAP_SPEED_1000;
        break;
    default:
        break;
    }   
out:    
    return ret;
}

/**
 * atl1e_sw_init - Initialize general software structures (struct atl1e_adapter)
 * @adapter: board private structure to initialize
 *
 * atl1e_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/

static int __devinit
atl1e_sw_init(struct atl1e_adapter *adapter)
{
    struct atl1e_hw *hw = &adapter->hw;
    struct pci_dev *pdev = adapter->pdev;
#ifdef CONFIG_AT_NAPI
    int i;
#endif

    /* PCI config space info */
    hw->vendor_id = pdev->vendor;
    hw->device_id = pdev->device;
    hw->subsystem_vendor_id = pdev->subsystem_vendor;
    hw->subsystem_id = pdev->subsystem_device;

    pci_read_config_byte(pdev, PCI_REVISION_ID, &hw->revision_id);
    pci_read_config_word(pdev, PCI_COMMAND, &hw->pci_cmd_word);

    if (atl1e_setup_mac_funcs(hw) != 0) {
        AT_ERR("set mac function pointers failed\n");
        return -1;
    }    
    
    adapter->wol = AT_WUFC_MAG;
    adapter->ict = 50000;  // 100ms
    adapter->link_speed = SPEED_0;   // hardware init
    adapter->link_duplex = FULL_DUPLEX; //

    hw->rrs_type = atl1e_rrs_disable;
    hw->phy_configured = FALSE;
    hw->preamble_len = 7;

    hw->smb_timer = 200000;    

    hw->max_frame_size = adapter->netdev->mtu;
    
    hw->dmar_block = atl1e_dma_req_1024;
    hw->dmaw_block = atl1e_dma_req_1024;
    
    hw->rx_jumbo_th = (hw->max_frame_size+
                            ENET_HEADER_SIZE + 
                            VLAN_SIZE +
                            ETHERNET_FCS_SIZE + 7)>>3;                          
    hw->indirect_tab = 0;
    hw->base_cpu = 0;
    hw->rrs_type = atl1e_rrs_disable;

    adapter->num_rx_queues = 1;

#ifdef CONFIG_AT_MQ    
    hw->indirect_tab = 0xE4E4E4E4;
    hw->rrs_type = 
        atl1e_rrs_ipv4 | atl1e_rrs_ipv4_tcp |
        atl1e_rrs_ipv6 | atl1e_rss_ipv6_tcp ;

    adapter->num_rx_queues = min(4, num_online_cpus());
    AT_DBG("Multiqueue Enabled: Rx Queue count = %u %s\n",
                 adapter->num_rx_queues,
                 ((adapter->num_rx_queues == 1)
                  ? ((num_online_cpus() > 1)
                         ? "(due to unsupported feature in current adapter)"
                         : "(due to unsupported system configuration)")
                  : ""));
                    
#endif//CONFIG_AT_MQ

    if (atl1e_alloc_queues(adapter)) {
        AT_ERR("Unable to allocate memory for queues\n");
        return -ENOMEM;
    }

#ifdef CONFIG_AT_NAPI
    for (i = 0; i < adapter->num_rx_queues; i++) {
        adapter->polling_netdev[i].priv = adapter;
        adapter->polling_netdev[i].poll = &atl1e_clean;
        adapter->polling_netdev[i].weight = 64;
        dev_hold(&adapter->polling_netdev[i]);
        set_bit(__LINK_STATE_START, &adapter->polling_netdev[i].state);
    }
    spin_lock_init(&adapter->tx_queue_lock);
#endif


    atomic_set(&adapter->irq_sem, 1);
    spin_lock_init(&adapter->stats_lock);
    spin_lock_init(&adapter->tx_lock);
    
    set_bit(__AT_DOWN, &adapter->flags);
    
    return 0;
}

#ifdef CONFIG_AT_NAPI
 /**
  * atl1e_clean - NAPI Rx polling callback
  * @adapter: board private structure
  **/
 
static int
atl1e_clean(struct net_device *poll_dev, int *budget)
{
    struct atl1e_adapter *adapter;
    int work_to_do = min(*budget, poll_dev->quota);
    int i = 0, work_done = 0;
    boolean_t tx_cleaned = FALSE;
 
    /* Must NOT use netdev_priv macro here. */
    adapter = poll_dev->priv;
 
    /* Keep link state information with original netdev */
    if (!netif_carrier_ok(adapter->netdev))
        goto quit_polling;
 
    while (poll_dev != &adapter->polling_netdev[i]) {
        i++;
        if (unlikely(i == adapter->num_rx_queues))
            BUG();
    }
 
    /* atl1e_clean is called per-cpu.  This lock protects
     * tx_ring[0] from being cleaned by multiple cpus
     * simultaneously.  A failure obtaining the lock means
     * tx_ring[0] is currently being cleaned anyway. */
     
    tx_cleaned = TRUE;

    atl1e_clean_rx_irq(adapter, i, &work_done, work_to_do);
         
    *budget -= work_done;
    poll_dev->quota -= work_done;
 
    /* If no Tx and not enough Rx work done, exit the polling mode */
    if ((tx_cleaned && (work_done < work_to_do)) || !netif_running(poll_dev)) {
quit_polling:
        netif_rx_complete(poll_dev);
        if (test_bit(__AT_DOWN, &adapter->flags))
            atomic_dec(&adapter->irq_sem);
        else
            atl1e_irq_enable(adapter);
            
        return 0;
    }
    return 1;
}
 
#endif
 
 
/**
  * atl1e_alloc_queues - Allocate memory for all rings
  * @adapter: board private structure to initialize
  *
  * We allocate one ring per queue at run-time since we don't know the
  * number of queues at compile-time.  The polling_netdev array is
  * intended for Multiqueue, but should work fine with a single queue.
  **/
 
static int __devinit
atl1e_alloc_queues(struct atl1e_adapter *adapter)
{
#ifdef CONFIG_AT_NAPI
    int size;
#endif
 
#ifdef CONFIG_AT_NAPI
    size = sizeof(struct net_device) * adapter->num_rx_queues;
    adapter->polling_netdev = kmalloc(size, GFP_KERNEL);
    if (!adapter->polling_netdev) {
        return -ENOMEM;
    }
    memset(adapter->polling_netdev, 0, size);
#endif
 
#ifdef CONFIG_AT_MQ
    adapter->rx_sched_call_data.func = atl1e_rx_schedule;
    adapter->rx_sched_call_data.info = adapter->netdev;
    //cpu in the system, zeroing them. Objects should be dereferenced using per_cpu_ptr/get_cpu_ptr macros only. 
    adapter->cpu_netdev = alloc_percpu(struct net_device *); //allocate one copy of the object for every present  ...arg->>size
#endif
 
    return AT_SUCCESS;
 }
 
#ifdef CONFIG_AT_MQ
static void __devinit
atl1e_setup_queue_mapping(struct atl1e_adapter *adapter)
{
    int i, cpu;
 
    adapter->rx_sched_call_data.func = atl1e_rx_schedule;
    adapter->rx_sched_call_data.info = adapter->netdev;
    cpus_clear(adapter->rx_sched_call_data.cpumask);
    
    lock_cpu_hotplug();
    
    i = 0;
    for_each_online_cpu(cpu) {
        /* This is incomplete because we'd like to assign separate
         * physical cpus to these netdev polling structures and
         * avoid saturating a subset of cpus.
         */
        if (i < adapter->num_rx_queues) {
                *per_cpu_ptr(adapter->cpu_netdev, cpu) = &adapter->polling_netdev[i];
                adapter->rx_cpu[i] = cpu;
                cpu_set(cpu, adapter->cpumask);
        } else
                *per_cpu_ptr(adapter->cpu_netdev, cpu) = NULL;
        
        i++;
    }
    
    unlock_cpu_hotplug();
}
#endif//CONFIG_AT_MQ

#ifdef CONFIG_AT_MQ
void
atl1e_rx_schedule(void *data)
{
    struct net_device *poll_dev, *netdev = data;
    struct atl1e_adapter *adapter = netdev->priv;
    int this_cpu = get_cpu();//preempt_disable(); smp_processor_id(); 

    poll_dev = *per_cpu_ptr(adapter->cpu_netdev, this_cpu);
    if (poll_dev == NULL) {
        put_cpu();
        return;
    }

    if (likely(netif_rx_schedule_prep(poll_dev)))
        __netif_rx_schedule(poll_dev);
    else
        atl1e_irq_enable(adapter);

    put_cpu();
}
#endif
 

int
atl1e_reset(struct atl1e_adapter *adapter)
{
    int ret;
    
    if (AT_SUCCESS != (ret = atl1e_reset_hw(&adapter->hw)))
        return ret;

    return atl1e_init_hw(&adapter->hw);
}

/**
 * atl1e_open - Called when a network interface is made active
 * @netdev: network interface device structure
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 **/

static int
atl1e_open(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    int err;
    u32 val;

    DEBUGFUNC("atl1e_open !");

    /* disallow open during test */
    if (test_bit(__AT_TESTING, &adapter->flags))
        return -EBUSY;
        
    /* allocate rx/tx dma buffer & descriptors */

    if((err = atl1e_setup_ring_resources(adapter)))
        return err;

    if((err = atl1e_init_hw(&adapter->hw))) {
        err = -EIO;
        goto err_init_hw;
    }
    
    /* hardware has been reset, we need to reload some things */

    atl1e_set_multi(netdev);
    init_ring_ptrs(adapter);

#ifdef NETIF_F_HW_VLAN_TX
    atl1e_restore_vlan(adapter);
#endif

    if (atl1e_configure(adapter)) {
        err = -EIO;
        goto err_config;
    }
   
    if ((err = atl1e_request_irq(adapter)))
        goto err_req_irq;
        
    clear_bit(__AT_DOWN, &adapter->flags);     
    
#ifdef CONFIG_AT_MQ
    atl1e_setup_queue_mapping(adapter);
#endif
  
#ifdef CONFIG_AT_NAPI
    netif_poll_enable(netdev);
#endif
      
    mod_timer(&adapter->watchdog_timer, jiffies + 4*HZ); 
    
    val = AT_READ_REG(&adapter->hw, REG_MASTER_CTRL);
    AT_WRITE_REG(&adapter->hw, REG_MASTER_CTRL, val|MASTER_CTRL_MANUAL_INT);

    
    atl1e_irq_enable(adapter);

    return 0;

err_init_hw:
err_req_irq:
err_config:
    atl1e_free_ring_resources(adapter);
    atl1e_reset_hw(&adapter->hw);
    
    return err;
}

/**
 * atl1e_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the drivers control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/

static int
atl1e_close(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    DEBUGFUNC("atl1e_close!");

    WARN_ON(test_bit(__AT_RESETTING, &adapter->flags));
    
    atl1e_down(adapter);
    atl1e_free_irq(adapter);
    atl1e_free_ring_resources(adapter);

    return 0;
}


/**
 * atl1e_setup_mem_resources - allocate Tx / RX descriptor resources 
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/

s32
atl1e_setup_ring_resources(struct atl1e_adapter *adapter)
{
    struct pci_dev *pdev = adapter->pdev;
    int size, i;
    u32 offset = 0;
    u32 page_size;

    DEBUGFUNC("atl1e_setup_ring_resources");

    /* real ring DMA buffer */
    page_size = adapter->rxf_length 
            + adapter->hw.max_frame_size 
            + ENET_HEADER_SIZE + VLAN_SIZE + ETHERNET_FCS_SIZE;
    page_size = (page_size + 31) & 0xFFFFFFE0;
            
    adapter->ring_size = size =   
          adapter->tpd_ring_size * sizeof(TpdDescr) + 7 // qword align
        + page_size * 2 * adapter->num_rx_queues + 31 // 32bytes algin
        + (1 + 2 * adapter->num_rx_queues) * 4 + 3; // cmb dma address
        
    adapter->ring_vir_addr = 
                    pci_alloc_consistent(pdev, size, &adapter->ring_dma);
    if (!adapter->ring_vir_addr) {
        DEBUGOUT1("pci_alloc_consistent failed, size = D%d", size);
        return -ENOMEM;
    }
 
    if (adapter->pci_using_64) { 
        // test whether HIDWORD dma buffer is not cross boundary
        if (    ((adapter->ring_dma       &0xffffffff00000000ULL)>>32)
             != (((adapter->ring_dma+size)&0xffffffff00000000ULL)>>32) ) {
            dma_addr_t dma;
            u8* addr;
            addr = pci_alloc_consistent(pdev, size, &dma);
            if (addr) {
                if ( ((dma&0xffffffff00000000ULL)>>32) !=
                     (((dma+size)&0xffffffff00000000ULL)>>32) ) {
                    pci_free_consistent(pdev, size, addr, dma);
                } else {
                     pci_free_consistent(
                        pdev, 
                        adapter->ring_size, 
                        adapter->ring_vir_addr, 
                        adapter->ring_dma);
                     adapter->ring_vir_addr = addr;
                     adapter->ring_dma = dma;
                     goto init_desc;
                }
            }

            pci_free_consistent(
                     pdev, 
                     adapter->ring_size, 
                     adapter->ring_vir_addr, 
                     adapter->ring_dma);
            DEBUGOUT("memory allocated cross 32bit boundary !");
            return -ENOMEM;
        }
    }
    
init_desc:
//    DEBUGOUT("memory allocated successfully !");    
    
    memset(adapter->ring_vir_addr, 0, adapter->ring_size);
    
    // tx buffer_infos
    size = sizeof(struct atl1e_buffer) * (adapter->tpd_ring_size);
    adapter->tx_buffer_info = kmalloc(size, GFP_KERNEL);
    if(!adapter->tx_buffer_info) {
        pci_free_consistent(
                     pdev, 
                     adapter->ring_size, 
                     adapter->ring_vir_addr, 
                     adapter->ring_dma);
        adapter->ring_vir_addr = NULL;
        DEBUGOUT1("kmalloc failed , size = D%d", size);
        return -ENOMEM;
    }
    memset(adapter->tx_buffer_info, 0, size);
    
    

    // Init TPD Ring
    adapter->tpd_ring_dma = adapter->ring_dma;
    offset = (adapter->tpd_ring_dma & 0x7) ? 
                (8 - (adapter->tpd_ring_dma & 0x7)) : 0;
    adapter->tpd_ring_dma += offset;
    adapter->tpd_ring = (TpdDescr*) (adapter->ring_vir_addr + offset);
    

    // Init RXF-Pages
    offset += (sizeof(TpdDescr) * adapter->tpd_ring_size);
    offset = (offset + 31) & 0xFFFFFFE0;
    
    for (i=0; i < adapter->num_rx_queues; i++) {
        adapter->rxf_page[i][0].dma = 
            adapter->ring_dma + (offset + i * 2 * page_size);
        adapter->rxf_page[i][0].addr = 
            adapter->ring_vir_addr + (offset + i * 2 * page_size);
        
        adapter->rxf_page[i][1].dma = 
            adapter->rxf_page[i][0].dma + page_size;
        adapter->rxf_page[i][1].addr = 
            adapter->rxf_page[i][0].addr + page_size;
    }
    
    // Init CMB dma address
    offset += page_size * 2 * adapter->num_rx_queues;
    adapter->tpd_cmb_dma = adapter->ring_dma + offset;
    adapter->tpd_cmb = (u32*)(adapter->ring_vir_addr + offset);
    offset += 4;
    for (i=0; i < adapter->num_rx_queues; i++) {
        adapter->rxf_page[i][0].WptrPhyAddr = adapter->ring_dma + offset;
        adapter->rxf_page[i][0].pWptr = adapter->ring_vir_addr + offset;
        offset += 4;
        adapter->rxf_page[i][1].WptrPhyAddr = adapter->ring_dma + offset;
        adapter->rxf_page[i][1].pWptr = adapter->ring_vir_addr + offset;
        offset += 4;
    }
            
    if (offset > adapter->ring_size) {
        DEBUGOUT2("offset(%d) > ring size(%d) !!\n", 
            offset, adapter->ring_size);
    }
    
      
    // Read / Write Ptr Initialize:
    //   init_ring_ptrs(adapter);

    return AT_SUCCESS;
}


void
init_ring_ptrs(struct atl1e_adapter *adapter)
{
    int i;
    // Read / Write Ptr Initialize:
    
    adapter->tpd_next_use = 0;
    atomic_set(&adapter->tpd_next_clean, 0);
    for (i=0; i < adapter->num_rx_queues; i++) {
        adapter->rxf_using[i] = 0;
        *adapter->rxf_page[i][0].pWptr = 0;
        *adapter->rxf_page[i][1].pWptr = 0;
        adapter->rxf_page[i][0].Rptr = 0;
        adapter->rxf_page[i][1].Rptr = 0;
        adapter->rxf_nxseq[i] = 0;
    }
}

/**
 * atl1e_free_ring_resources - Free Tx / RX descriptor Resources
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/

void
atl1e_free_ring_resources(struct atl1e_adapter *adapter)
{
    struct pci_dev *pdev = adapter->pdev;
    
    DEBUGFUNC("atl1e_free_ring_resources");

    atl1e_clean_tx_ring(adapter);
    atl1e_clean_rx_ring(adapter);
    
    if (adapter->ring_vir_addr) {
        pci_free_consistent(
             pdev, 
             adapter->ring_size,
             adapter->ring_vir_addr,
             adapter->ring_dma);
        adapter->ring_vir_addr = NULL;           
    }
    
    if (adapter->tx_buffer_info) {
        kfree(adapter->tx_buffer_info);
        adapter->tx_buffer_info = NULL;
    }
         
}


int
atl1e_up(struct atl1e_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    int err = 0;
    u32 val;

    DEBUGFUNC("atl1e_up !"); 

    /* hardware has been reset, we need to reload some things */

    err = atl1e_init_hw(&adapter->hw);
    if (err) {
        err = -EIO;
        return err;
    }

    atl1e_set_multi(netdev);
    init_ring_ptrs(adapter);

#ifdef NETIF_F_HW_VLAN_TX
    atl1e_restore_vlan(adapter);
#endif

    if (atl1e_configure(adapter)) {
        err = -EIO;
        goto err_up;
    }    

    clear_bit(__AT_DOWN, &adapter->flags);
    
    val = AT_READ_REG(&adapter->hw, REG_MASTER_CTRL);
    AT_WRITE_REG(&adapter->hw, REG_MASTER_CTRL, val|MASTER_CTRL_MANUAL_INT);

#ifdef CONFIG_AT_MQ
    atl1e_setup_queue_mapping(adapter);
#endif
  
#ifdef CONFIG_AT_NAPI
    netif_poll_enable(netdev);
#endif

    atl1e_irq_enable(adapter);
err_up:
    return err;
}

void atl1e_setup_mac_ctrl(struct atl1e_adapter* adapter)
{
    u32 value;
    struct atl1e_hw* hw = &adapter->hw;
    struct net_device* netdev = adapter->netdev;
    
    /* Config MAC CTRL Register */
    value = MAC_CTRL_TX_EN | 
            MAC_CTRL_RX_EN ;
    // duplex
    if (FULL_DUPLEX == adapter->link_duplex)
        value |= MAC_CTRL_DUPLX;
    // speed
    value |= ((u32)((SPEED_1000 == adapter->link_speed) ?
                 MAC_CTRL_SPEED_1000:MAC_CTRL_SPEED_10_100)<<MAC_CTRL_SPEED_SHIFT);
    // flow control
    value |= (MAC_CTRL_TX_FLOW|MAC_CTRL_RX_FLOW);

    // PAD & CRC
    value |= (MAC_CTRL_ADD_CRC|MAC_CTRL_PAD);
    // preamble length
    value |= (((u32)adapter->hw.preamble_len
                  &MAC_CTRL_PRMLEN_MASK)<< MAC_CTRL_PRMLEN_SHIFT);
    // vlan 
    if (adapter->vlgrp)     
        value |= MAC_CTRL_RMV_VLAN;
        
    // filter mode
    value |= MAC_CTRL_BC_EN;
    if (netdev->flags & IFF_PROMISC) 
        value |= MAC_CTRL_PROMIS_EN;
    else if (netdev->flags & IFF_ALLMULTI)
        value |= MAC_CTRL_MC_ALL_EN;

    AT_WRITE_REG(hw, REG_MAC_CTRL, value);
}

static int atl1e_power_ctrl_link_chg(struct atl1e_hw *hw)
{
    return hw->ops.power_ctrl_link_chg(hw);
}

static int atl1e_phy_ctrl_link_chg(struct atl1e_hw *hw)
{
    return hw->ops.phy_ctrl_link_chg(hw);
}

static int atl1e_link_chg_callback(struct atl1e_adapter *adapter)
{
    struct atl1e_hw *hw = &adapter->hw;
    u32 value;

    if (adapter->link_speed == SPEED_0) { /* link_down */
        value = AT_READ_REG(hw, REG_MAC_CTRL);
        value &= ~MAC_CTRL_RX_EN;
        AT_WRITE_REG(hw, REG_MAC_CTRL, value);            
        value = AT_READ_REG(hw, REG_TXQ_CTRL);
        value &= ~TXQ_CTRL_EN;
        AT_WRITE_REG(hw, REG_TXQ_CTRL, value);
        value = AT_READ_REG(hw, REG_RXQ_CTRL);
        value &= ~RXQ_CTRL_EN;
        AT_WRITE_REG(hw, REG_RXQ_CTRL, value);  
    } else {
        value = AT_READ_REG(hw, REG_TXQ_CTRL);
        value |= TXQ_CTRL_EN;
        AT_WRITE_REG(hw, REG_TXQ_CTRL, value);
        value = AT_READ_REG(hw, REG_RXQ_CTRL);
        value |= RXQ_CTRL_EN;
        AT_WRITE_REG(hw, REG_RXQ_CTRL, value);  
    }
    
    if (atl1e_phy_ctrl_link_chg(hw) != 0)
        printk(KERN_WARNING "phy ctrl link change failed\n"); 
    if (atl1e_power_ctrl_link_chg(hw) != 0)
        printk(KERN_WARNING "power ctrl link change failed\n");
    
    return 0;
}

static int atl1e_check_link(struct atl1e_adapter *adapter)
{
    struct atl1e_hw *hw = &adapter->hw;
    struct net_device * netdev = adapter->netdev;
    int ret_val;
    u16 speed, duplex, phy_data;

    DEBUGFUNC("atl1e_check_link !");

    // MII_BMSR must read twise
    atl1e_read_phy_reg(hw, MII_BMSR, &phy_data);
    atl1e_read_phy_reg(hw, MII_BMSR, &phy_data);
    if (!(phy_data & BMSR_LSTATUS)) { /* link down */
        if (netif_carrier_ok(netdev)) { /* old link state: Up */
            DEBUGOUT("NIC Link is Down");
            adapter->link_speed = SPEED_0;
            netif_carrier_off(netdev);
            netif_stop_queue(netdev);
        }
    } else { /* link up */
        ret_val = atl1e_get_speed_and_duplex(hw, &speed, &duplex);
        if (ret_val)
            return ret_val;

        if (adapter->link_speed != speed ||
            adapter->link_duplex != duplex ) {
            adapter->link_speed = speed;
            adapter->link_duplex = duplex;
            atl1e_setup_mac_ctrl(adapter); 
            printk(KERN_INFO
                   "%s: %s NIC Link is Up<%d Mbps %s>\n",
                    atl1e_driver_name,
                    netdev->name, adapter->link_speed,
                    adapter->link_duplex == FULL_DUPLEX ?
                    "Full Duplex" : "Half Duplex"); 
        }
        
        if (!netif_carrier_ok(netdev)) { /* Link down -> Up */
            netif_carrier_on(netdev);
            netif_wake_queue(netdev);
        }
    }     
    return atl1e_link_chg_callback(adapter);
}

void atl1e_down(struct atl1e_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    
    DEBUGFUNC("atl1e_down !");

     /* signal that we're down so the interrupt handler does not
      * reschedule our watchdog timer */
    set_bit(__AT_DOWN, &adapter->flags);

#ifdef NETIF_F_LLTX
    netif_stop_queue(netdev);
#else
    netif_tx_disable(netdev);
#endif

#ifdef CONFIG_AT_NAPI
    netif_poll_disable(netdev);
#endif    
    atl1e_irq_disable(adapter);
    AT_WRITE_REG(&adapter->hw, REG_ISR, 0xffffffff);
    /* reset MAC to disable all RX/TX */
    atl1e_reset_hw(&adapter->hw);
    msleep(1);
 
    del_timer_sync(&adapter->watchdog_timer);
    del_timer_sync(&adapter->phy_config_timer);
    clear_bit(0, &adapter->cfg_phy);

    
    netif_carrier_off(netdev);
    adapter->link_speed = SPEED_0;
    adapter->link_duplex = -1;
    
    atl1e_clean_tx_ring(adapter);
    atl1e_clean_rx_ring(adapter);
}



/**
 * atl1e_set_multi - Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_multi entry point is called whenever the multicast address
 * list or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper multicast,
 * promiscuous mode, and all-multi behavior.
 **/

static void
atl1e_set_multi(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    struct atl1e_hw *hw = &adapter->hw;
    struct dev_mc_list *mc_ptr;
    u32 rctl;
    u32 hash_value;

    DEBUGFUNC("atl1e_set_multi !");

    /* Check for Promiscuous and All Multicast modes */

    rctl = AT_READ_REG(hw, REG_MAC_CTRL);

    if(netdev->flags & IFF_PROMISC) {
        rctl |= MAC_CTRL_PROMIS_EN;
    } else if(netdev->flags & IFF_ALLMULTI) {
        rctl |= MAC_CTRL_MC_ALL_EN;
        rctl &= ~MAC_CTRL_PROMIS_EN;
    } else {
        rctl &= ~(MAC_CTRL_PROMIS_EN | MAC_CTRL_MC_ALL_EN);
    }

    AT_WRITE_REG(hw, REG_MAC_CTRL, rctl);

    /* clear the old settings from the multicast hash table */
    AT_WRITE_REG(hw, REG_RX_HASH_TABLE, 0);
    AT_WRITE_REG_ARRAY(hw, REG_RX_HASH_TABLE, 1, 0);

    /* comoute mc addresses' hash value ,and put it into hash table */

    for(mc_ptr = netdev->mc_list; mc_ptr; mc_ptr = mc_ptr->next) {
        hash_value = atl1e_hash_mc_addr(hw, mc_ptr->dmi_addr);
        atl1e_hash_set(hw, hash_value);
    }
}


#ifdef NETIF_F_HW_VLAN_TX
static void
atl1e_vlan_rx_register(struct net_device *netdev, struct vlan_group *grp)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    u32 ctrl;

    DEBUGFUNC("atl1e_vlan_rx_register !");    

    atl1e_irq_disable(adapter);
    adapter->vlgrp = grp;

    if(grp) {
        /* enable VLAN tag insert/strip */

        ctrl = AT_READ_REG(&adapter->hw, REG_MAC_CTRL);
        ctrl |= MAC_CTRL_RMV_VLAN; 
        AT_WRITE_REG(&adapter->hw, REG_MAC_CTRL, ctrl);
    } else {
        /* disable VLAN tag insert/strip */

        ctrl = AT_READ_REG(&adapter->hw, REG_MAC_CTRL);
        ctrl &= ~MAC_CTRL_RMV_VLAN;
        AT_WRITE_REG(&adapter->hw, REG_MAC_CTRL, ctrl);
    }

    atl1e_irq_enable(adapter);
}

static void
atl1e_restore_vlan(struct atl1e_adapter *adapter)
{
    DEBUGFUNC("atl1e_restore_vlan !");
    atl1e_vlan_rx_register(adapter->netdev, adapter->vlgrp);
}
#endif

static u16 gPayLoadSize[] = {
    128, 256, 512, 1024, 2048, 4096,
};
/**
 * atl1e_configure - Configure Transmit&Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx /Rx unit of the MAC after a reset.
 **/

static s32
atl1e_configure(struct atl1e_adapter *adapter)
{
    struct atl1e_hw * hw = &adapter->hw;
    u32 value, hi;
    
    DEBUGFUNC("atl1e_configure !");

    // clear interrupt status
    AT_WRITE_REG(&adapter->hw, REG_ISR, 0xffffffff);

    // 1. set MAC Address
    value = (((u32)hw->mac_addr[2]) << 24) |
            (((u32)hw->mac_addr[3]) << 16) |
            (((u32)hw->mac_addr[4]) << 8 ) |
            (((u32)hw->mac_addr[5])      ) ;
    AT_WRITE_REG(hw, REG_MAC_STA_ADDR, value);
    value = (((u32)hw->mac_addr[0]) << 8 ) |
            (((u32)hw->mac_addr[1])      ) ;
    AT_WRITE_REG(hw, (REG_MAC_STA_ADDR+4), value);

    // 2. Init the Multicast HASH table
    // done by set_muti

    // 3. Clear any WOL status
    value = AT_READ_REG(hw, REG_WOL_CTRL);
    AT_WRITE_REG(hw, REG_WOL_CTRL, 0);
    
    // 4. Descripter Ring BaseMem/Length/Read ptr/Write ptr */
    // Descripter Ring BaseMem/Length/Read ptr/Write ptr 
    // TPD Ring/SMB/RXF0 Page CMBs, they use the same High 32bits memory
    AT_WRITE_REG(hw, REG_DESC_BASE_ADDR_HI, (u32)((adapter->ring_dma&0xffffffff00000000ULL) >>32));
    AT_WRITE_REG(hw, REG_TPD_BASE_ADDR_LO, (u32)(adapter->tpd_ring_dma));
    AT_WRITE_REG(hw, REG_TPD_RING_SIZE, (u16)(adapter->tpd_ring_size));
    AT_WRITE_REG(hw, REG_HOST_TX_CMB_LO, (u32)adapter->tpd_cmb_dma);
    // RXF Page Physical address / Page Length
    // RXF0
    AT_WRITE_REG(hw, REG_HOST_RXF0_PAGE0_LO, (u32)(adapter->rxf_page[0][0].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF0_PAGE1_LO, (u32)(adapter->rxf_page[0][1].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF0_MB0_LO, (u32)(adapter->rxf_page[0][0].WptrPhyAddr));
    AT_WRITE_REG(hw, REG_HOST_RXF0_MB1_LO, (u32)(adapter->rxf_page[0][1].WptrPhyAddr));
    AT_WRITE_REGB(hw, REG_HOST_RXF0_PAGE0_VLD, 1);
    AT_WRITE_REGB(hw, REG_HOST_RXF0_PAGE1_VLD, 1);
    // RXF1
    AT_WRITE_REG(hw, REG_RXF1_BASE_ADDR_HI, (u32)((adapter->ring_dma&0xffffffff00000000ULL) >>32));
    AT_WRITE_REG(hw, REG_HOST_RXF1_PAGE0_LO, (u32)(adapter->rxf_page[1][0].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF1_PAGE1_LO, (u32)(adapter->rxf_page[1][1].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF1_MB0_LO, (u32)(adapter->rxf_page[1][0].WptrPhyAddr));
    AT_WRITE_REG(hw, REG_HOST_RXF1_MB1_LO, (u32)(adapter->rxf_page[1][1].WptrPhyAddr));  
    AT_WRITE_REGB(hw, REG_HOST_RXF1_PAGE0_VLD, 1);
    AT_WRITE_REGB(hw, REG_HOST_RXF1_PAGE1_VLD, 1);        
    // RXF2
    AT_WRITE_REG(hw, REG_RXF1_BASE_ADDR_HI, (u32)((adapter->ring_dma&0xffffffff00000000ULL) >>32));
    AT_WRITE_REG(hw, REG_HOST_RXF2_PAGE0_LO, (u32)(adapter->rxf_page[2][0].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF2_PAGE1_LO, (u32)(adapter->rxf_page[2][1].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF2_MB0_LO, (u32)(adapter->rxf_page[2][0].WptrPhyAddr));
    AT_WRITE_REG(hw, REG_HOST_RXF2_MB1_LO, (u32)(adapter->rxf_page[2][1].WptrPhyAddr)); 
    AT_WRITE_REGB(hw, REG_HOST_RXF2_PAGE0_VLD, 1);
    AT_WRITE_REGB(hw, REG_HOST_RXF2_PAGE1_VLD, 1);    
    // RXF3
    AT_WRITE_REG(hw, REG_RXF1_BASE_ADDR_HI, (u32)((adapter->ring_dma&0xffffffff00000000ULL) >>32));
    AT_WRITE_REG(hw, REG_HOST_RXF3_PAGE0_LO, (u32)(adapter->rxf_page[3][0].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF3_PAGE1_LO, (u32)(adapter->rxf_page[3][1].dma));
    AT_WRITE_REG(hw, REG_HOST_RXF3_MB0_LO, (u32)(adapter->rxf_page[3][0].WptrPhyAddr));
    AT_WRITE_REG(hw, REG_HOST_RXF3_MB1_LO, (u32)(adapter->rxf_page[3][1].WptrPhyAddr)); 
    AT_WRITE_REGB(hw, REG_HOST_RXF3_PAGE0_VLD, 1);
    AT_WRITE_REGB(hw, REG_HOST_RXF3_PAGE1_VLD, 1);  
    // Page Length
    AT_WRITE_REG(hw, REG_HOST_RXFPAGE_SIZE, adapter->rxf_length);
    // Load all of base address above
    AT_WRITE_REG(hw, REG_LOAD_PTR, 1);


    // 5. set Interrupt Moderator Timer
    AT_WRITE_REGW(hw, REG_IRQ_MODU_TIMER_INIT, adapter->imt);
    AT_WRITE_REGW(hw, REG_IRQ_MODU_TIMER2_INIT, adapter->imt);
    AT_WRITE_REG(hw, REG_MASTER_CTRL, 
                       MASTER_CTRL_LED_MODE|MASTER_CTRL_ITIMER_EN|MASTER_CTRL_ITIMER2_EN);
    
    // 13. rx/tx threshold to trig interrupt
    AT_WRITE_REGW(hw, REG_TRIG_RRD_THRESH, 1); // 1 packets
    AT_WRITE_REGW(hw, REG_TRIG_TPD_THRESH, (u16)(adapter->tpd_ring_size/2));
    AT_WRITE_REGW(hw, REG_TRIG_RXTIMER, 4);   // 10us
    AT_WRITE_REGW(hw, REG_TRIG_TXTIMER, (u16)(adapter->imt*4/3)); // 


    // 6. set Interrupt Clear Timer
    AT_WRITE_REGW(hw, REG_CMBDISDMA_TIMER, 10000);
    

    // 7. Enable Read-Clear Interrupt Mechanism
/*
    O_32(pL1e->MemBase+REG_MASTER_CTRL, 
            MASTER_CTRL_INT_RDCLR|MASTER_CTRL_ITIMER_EN|MASTER_CTRL_ITIMER2_EN);
*/        
    // 8. set MTU
    AT_WRITE_REG(hw, REG_MTU, 
            hw->max_frame_size +
            ENET_HEADER_SIZE + 
            VLAN_SIZE +
            ETHERNET_FCS_SIZE);

    // 9. config TXQ
    // early tx threshold
    if (hw->nic_type != athr_l2e_revB) {
	    if (hw->max_frame_size <= 1500)
	        value = hw->max_frame_size + ENET_HEADER_SIZE + VLAN_SIZE + ETHERNET_FCS_SIZE ;
	    else if (hw->max_frame_size < 6*1024)
	        value = (hw->max_frame_size + ENET_HEADER_SIZE + VLAN_SIZE + ETHERNET_FCS_SIZE) * 2 / 3;
	    else
	        value = (hw->max_frame_size + ENET_HEADER_SIZE + VLAN_SIZE + ETHERNET_FCS_SIZE) / 2;
	    AT_WRITE_REG(hw, REG_TX_EARLY_TH, (value + 7) >> 3);
	}
	
    // tx control
    value = AT_READ_REG(hw, REG_DEVICE_CTRL);
    hi = (value>>DEVICE_CTRL_MAX_PAYLOAD_SHIFT)&DEVICE_CTRL_MAX_PAYLOAD_MASK;
    if (hi < (u32)hw->dmaw_block)   
        hw->dmaw_block = (atl1e_dma_req_block) hi;
    hi = (value>>DEVICE_CTRL_MAX_RREQ_SZ_SHIFT)&DEVICE_CTRL_MAX_RREQ_SZ_MASK;
    if (hi < (u32)hw->dmar_block)
        hw->dmar_block = (atl1e_dma_req_block) hi;

	if (hw->nic_type != athr_l2e_revB) {
		AT_WRITE_REGW(hw, REG_TXQ_CTRL+2, gPayLoadSize[hw->dmar_block]);
	}

    AT_WRITE_REGW(hw, REG_TXQ_CTRL,  
        (((u16)5&TXQ_CTRL_NUM_TPD_BURST_MASK) << TXQ_CTRL_NUM_TPD_BURST_SHIFT) |
        TXQ_CTRL_ENH_MODE | TXQ_CTRL_EN);
        
    // 10. config RXQ
 
    if (hw->nic_type != athr_l2e_revB) {    
    	// jumbo size cut-through  
    	AT_WRITE_REGW(hw, REG_RXQ_JMBOSZ_RRDTIM, 
        	(u16) ((hw->rx_jumbo_th&RXQ_JMBOSZ_TH_MASK) << RXQ_JMBOSZ_TH_SHIFT |
            	   (1&RXQ_JMBO_LKAH_MASK) << RXQ_JMBO_LKAH_SHIFT)); 
            	
	    // flow control
	    value = AT_READ_REG(hw, REG_SRAM_RXF_LEN);
	    hi = value * 4 / 5;
	    value /= 5;
	    value = 
	          ((hi&RXQ_RXF_PAUSE_TH_HI_MASK) << RXQ_RXF_PAUSE_TH_HI_SHIFT)|
	          ((value&RXQ_RXF_PAUSE_TH_LO_MASK) << RXQ_RXF_PAUSE_TH_LO_SHIFT);
	    AT_WRITE_REG(hw, REG_RXQ_RXF_PAUSE_THRESH, value);
    }
    
    // RRS  
    AT_WRITE_REG(hw, REG_IDT_TABLE, hw->indirect_tab);
    AT_WRITE_REG(hw, REG_BASE_CPU_NUMBER, hw->base_cpu);
    value = 0;
    if (hw->rrs_type&atl1e_rrs_ipv4)       value |= RXQ_CTRL_HASH_TYPE_IPV4;
    if (hw->rrs_type&atl1e_rrs_ipv4_tcp)   value |= RXQ_CTRL_HASH_TYPE_IPV4_TCP;
    if (hw->rrs_type&atl1e_rrs_ipv6)       value |= RXQ_CTRL_HASH_TYPE_IPV6;
    if (hw->rrs_type&atl1e_rss_ipv6_tcp)   value |= RXQ_CTRL_HASH_TYPE_IPV6_TCP;
    if (hw->rrs_type&(atl1e_rrs_ipv4|atl1e_rrs_ipv4_tcp|atl1e_rrs_ipv6|atl1e_rss_ipv6_tcp)) {
        value |= RXQ_CTRL_HASH_ENABLE|RXQ_CTRL_RSS_MODE_MQUESINT;
    }
    value |= RXQ_CTRL_IPV6_XSUM_VERIFY_EN|RXQ_CTRL_PBA_ALIGN_32|
            RXQ_CTRL_CUT_THRU_EN|RXQ_CTRL_EN;
    AT_WRITE_REG(hw, REG_RXQ_CTRL, value);

    // 11. config  DMA Engine
    value =  
        ((((u32)hw->dmar_block)&DMA_CTRL_DMAR_BURST_LEN_MASK)
           << DMA_CTRL_DMAR_BURST_LEN_SHIFT)|
        ((((u32)hw->dmaw_block)&DMA_CTRL_DMAW_BURST_LEN_MASK) 
           << DMA_CTRL_DMAW_BURST_LEN_SHIFT) |
        DMA_CTRL_DMAR_REQ_PRI | 
        DMA_CTRL_DMAR_OUT_ORDER |
        ((15&DMA_CTRL_DMAR_DLY_CNT_MASK)<<DMA_CTRL_DMAR_DLY_CNT_SHIFT) |
        ((4&DMA_CTRL_DMAW_DLY_CNT_MASK)<<DMA_CTRL_DMAW_DLY_CNT_SHIFT) |
        DMA_CTRL_RXCMB_EN;
	/* disable TXCMB */
	/* DMA_CTRL_TXCMB_EN */
    AT_WRITE_REG(hw, REG_DMA_CTRL, value);

    // 12. smb timer to trig interrupt
    AT_WRITE_REG(hw, REG_SMB_STAT_TIMER, 50000);
    
    value = AT_READ_REG(hw, REG_ISR);
    if (unlikely((value & ISR_PHY_LINKDOWN) != 0))
        value = 1;  /* config failed */
    else
        value = 0;

    // clear all interrupt status
    AT_WRITE_REG(hw, REG_ISR, 0x7fffffff);
    AT_WRITE_REG(hw, REG_ISR, 0);
    
    return value;
}

/**
 * atl1e_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/

static int
atl1e_set_mac(struct net_device *netdev, void *p)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    struct sockaddr *addr = p;

    DEBUGFUNC("atl1e_set_mac !");
    
    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;
        
    if (netif_running(netdev))
        return -EBUSY; 

    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
    memcpy(adapter->hw.mac_addr, addr->sa_data, netdev->addr_len);

    set_mac_addr(&adapter->hw);

    return 0;
}



/**
 * atl1e_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 **/

static int
atl1e_change_mtu(struct net_device *netdev, int new_mtu)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    struct atl1e_hw* hw = &adapter->hw;
    
    DEBUGFUNC("atl1e_change_mtu !");
    
    if ((new_mtu < 40) || (new_mtu > (ETH_DATA_LEN + VLAN_SIZE)))
        return -EINVAL;


    /* set MTU */
    if (hw->max_frame_size != new_mtu) {
        
        while (test_and_set_bit(__AT_RESETTING, &adapter->flags))
            msleep(1);
        
        if (netif_running(netdev)) {
            atl1e_down(adapter);
        }
        
        netdev->mtu = new_mtu;   
        hw->max_frame_size = new_mtu;
        hw->rx_jumbo_th = (new_mtu+
                                ENET_HEADER_SIZE + 
                                VLAN_SIZE +
                                ETHERNET_FCS_SIZE + 7)>>3;
        
        if (netif_running(netdev))
            atl1e_up(adapter);
        else
            atl1e_reset(adapter);

        clear_bit(__AT_RESETTING, &adapter->flags);
    }

    return 0;
}



void
atl1e_read_pci_cfg(struct atl1e_hw *hw, u32 reg, u16 *value)
{
    struct atl1e_adapter *adapter = hw->back;

    pci_read_config_word(adapter->pdev, reg, value);
}

void
atl1e_write_pci_cfg(struct atl1e_hw *hw, u32 reg, u16 *value)
{
    struct atl1e_adapter *adapter = hw->back;

    pci_write_config_word(adapter->pdev, reg, *value);
}

/**
 * atl1e_clean_tx_ring - Free Tx-skb
 * @adapter: board private structure
 **/

static void
atl1e_clean_tx_ring(struct atl1e_adapter *adapter)
{
    struct atl1e_buffer *buffer_info;
    struct pci_dev *pdev = adapter->pdev;
    u16 index;

//    DEBUGFUNC("atl1e_clean_tx_ring !");

    if (NULL == adapter->tx_buffer_info ||
        NULL == adapter->tpd_ring)
        return;
    
    for (index=0; index < adapter->tpd_ring_size; index++) {
        buffer_info = adapter->tx_buffer_info + index;
        if (buffer_info->dma) {
            pci_unmap_page(pdev,
                           buffer_info->dma,
                           buffer_info->length,
                           PCI_DMA_TODEVICE);
            buffer_info->dma = 0;
        }
    }
    for(index = 0; index < adapter->tpd_ring_size; index++) {
        buffer_info = adapter->tx_buffer_info + index;
        if(buffer_info->skb) {
            dev_kfree_skb_any(buffer_info->skb);
            buffer_info->skb = NULL;
        }
    }
 
    /* Zero out Tx-buffers */
    memset(adapter->tx_buffer_info, 0, 
            sizeof(struct atl1e_buffer)*adapter->tpd_ring_size);
    
    memset(adapter->tpd_ring, 0, 
            sizeof(TpdDescr)*adapter->tpd_ring_size);
}

/**
 * atl1e_clean_rx_ring - Free rx-reservation skbs
 * @adapter: board private structure
 **/

static void
atl1e_clean_rx_ring(struct atl1e_adapter *adapter)
{
    u16 i;
//    DEBUGFUNC("atl1e_clean_rx_ring !");

    if (NULL == adapter->ring_vir_addr)
        return;
        
    /* Free all the Rx ring sk_buffs */


    /* Zero out the descriptor ring */
    
    for (i=0; i < adapter->num_rx_queues; i++) {
        if (adapter->rxf_page[i][0].addr) {
            memset(adapter->rxf_page[i][0].addr, 0, adapter->rxf_length);
            memset(adapter->rxf_page[i][1].addr, 0, adapter->rxf_length);
        }
    }
}


/**
 * atl1e_get_stats - Get System Network Statistics
 * @netdev: network interface device structure
 *
 * Returns the address of the device statistics structure.
 * The statistics are actually updated from the timer callback.
 **/

static struct net_device_stats *
atl1e_get_stats(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
       
    return &adapter->net_stats;
}       

/**
 * atl1e_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int
atl1e_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
    DEBUGFUNC("atl1e_ioctl !");
    switch (cmd) {
#ifdef SIOCGMIIPHY
    case SIOCGMIIPHY:
    case SIOCGMIIREG:
    case SIOCSMIIREG:
        return atl1e_mii_ioctl(netdev, ifr, cmd);
#endif

#ifdef ETHTOOL_OPS_COMPAT
    case SIOCETHTOOL:
        return ethtool_ioctl(ifr);
#endif

    default:
        return -EOPNOTSUPP;
    }
}


#ifdef SIOCGMIIPHY
/**
 * atl1e_mii_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int
atl1e_mii_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    struct mii_ioctl_data *data = if_mii(ifr);
    unsigned long flags;
    
    DEBUGFUNC("atl1e_mii_ioctl !");

    switch (cmd) {
    case SIOCGMIIPHY:
        data->phy_id = 0;
        break;
    case SIOCGMIIREG:
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;
        spin_lock_irqsave(&adapter->stats_lock, flags);
        if (atl1e_read_phy_reg(&adapter->hw, data->reg_num & 0x1F, &data->val_out)) {
            spin_unlock_irqrestore(&adapter->stats_lock, flags);
            return -EIO;
        }
        spin_unlock_irqrestore(&adapter->stats_lock, flags);
        break;
    case SIOCSMIIREG:
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;
        if (data->reg_num & ~(0x1F))
            return -EFAULT;
         
        spin_lock_irqsave(&adapter->stats_lock, flags);
        DEBUGOUT1("<atl1e_mii_ioctl> write %x %x", 
            data->reg_num, 
            data->val_in);
        if (atl1e_write_phy_reg(&adapter->hw, data->reg_num, data->val_in)) {
            spin_unlock_irqrestore(&adapter->stats_lock, flags);
            return -EIO;
        }
        // ......
        spin_unlock_irqrestore(&adapter->stats_lock, flags);
        break;
        
    default:
        return -EOPNOTSUPP;
    }
    return AT_SUCCESS;
}

#endif


/**
 * atl1e_tx_timeout - Respond to a Tx Hang
 * @netdev: network interface device structure
 **/

static void
atl1e_tx_timeout(struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);

    DEBUGFUNC("atl1e_tx_timeout !");

    /* Do the reset outside of interrupt context */
    schedule_work(&adapter->reset_task);
}

static void
atl1e_reset_task(struct work_struct *work)
{
    struct atl1e_adapter *adapter;
    adapter = container_of(work, struct atl1e_adapter, reset_task);

    atl1e_reinit_locked(adapter);
}


void
atl1e_reinit_locked(struct atl1e_adapter *adapter)
{
 
    DEBUGFUNC("atl1e_reinit_locked !");

    WARN_ON(in_interrupt());
    while (test_and_set_bit(__AT_RESETTING, &adapter->flags))
        msleep(1);
    atl1e_down(adapter);
    atl1e_up(adapter);
    clear_bit(__AT_RESETTING, &adapter->flags);
}


/**
 * atl1e_link_chg_task - deal with link change event Out of interrupt context
 * @netdev: network interface device structure
 **/
static void
atl1e_link_chg_task(struct work_struct *work)
{
    struct atl1e_adapter *adapter;
    unsigned long flags;

    DEBUGFUNC("atl1e_link_chg_task !");

    adapter = container_of(work, struct atl1e_adapter, link_chg_task);


    spin_lock_irqsave(&adapter->stats_lock, flags);
    atl1e_check_link(adapter);
    spin_unlock_irqrestore(&adapter->stats_lock, flags);
}

static void
atl1e_check_for_link(struct atl1e_adapter* adapter)
{
    spin_lock(&adapter->stats_lock);
    atl1e_check_link(adapter);
    spin_unlock(&adapter->stats_lock);

}

static void
atl1e_clear_phy_int(struct atl1e_adapter* adapter)
{
    u16 phy_data;
    
    spin_lock(&adapter->stats_lock);
    atl1e_read_phy_reg(&adapter->hw, 19, &phy_data);
    spin_unlock(&adapter->stats_lock);
}



/**
 * atl1e_intr - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 * @pt_regs: CPU registers structure
 **/

static irqreturn_t
atl1e_intr(int irq, void *data)
{
    struct atl1e_adapter *adapter = netdev_priv((struct net_device *)data);
    struct atl1e_hw *hw = &adapter->hw;
    u32 status;
   
    
#ifndef CONFIG_AT_NAPI
    int max_ints = 5;
#endif
    
    //DEBUGFUNC("atl1e_intr");    
    
    status = AT_READ_REG(hw, REG_ISR);
    if (0 == (status & IMR_NORMAL_MASK) ||
        0 != (status & ISR_DIS_INT))   
        return IRQ_NONE;

#ifndef CONFIG_AT_NAPI 
loopint:
#endif

    // link event
    if (status&ISR_GPHY) {
        atl1e_clear_phy_int(adapter);
    }
    
    // Ack ISR
    AT_WRITE_REG(hw, REG_ISR, status | ISR_DIS_INT);  
    
    // check if PCIE PHY Link down
   if (status&ISR_PHY_LINKDOWN) {
        DEBUGOUT1("pcie phy linkdown %x", status);
        if(netif_running(adapter->netdev)) { // reset MAC
            AT_WRITE_REG(hw, REG_ISR, 0);
            AT_WRITE_REG(hw, REG_IMR, 0);
            AT_WRITE_FLUSH(hw);
            schedule_work(&adapter->reset_task);
            return IRQ_HANDLED; 
        }
    }

    // check if DMA read/write error ?
    if (status&(ISR_DMAR_TO_RST|ISR_DMAW_TO_RST)) 
    {
        DEBUGOUT1("PCIE DMA RW error (status = 0x%x) !", status);
        AT_WRITE_REG(hw, REG_ISR, 0);
        AT_WRITE_REG(hw, REG_IMR, 0);
        AT_WRITE_FLUSH(hw);
        schedule_work(&adapter->reset_task);
        return IRQ_HANDLED; 
    }
    
    // link event
    if (status&(ISR_GPHY|ISR_MANUAL))
    {
        //AT_DBG("int-status=0x%x", status);
        adapter->net_stats.tx_carrier_errors++;
        atl1e_check_for_link(adapter);
    }
   
#ifdef CONFIG_AT_NAPI
    if ( status & ISR_RX_EVENT ) {
        
        /* disable interrupts, without the synchronize_irq bit */
    atomic_inc(&adapter->irq_sem);
    AT_WRITE_REG(hw, REG_IMR, IMR_NONRX_MASK);
    AT_WRITE_FLUSH(hw);
        
#ifdef CONFIG_AT_MQ
    if (atomic_read(&adapter->rx_sched_call_data.count) == 0) {
        /* We must setup the cpumask once count == 0 since
         * each cpu bit is cleared when the work is done. */
        adapter->rx_sched_call_data.cpumask = adapter->cpumask;
        atomic_add(adapter->num_rx_queues - 1, &adapter->irq_sem);
        atomic_set(&adapter->rx_sched_call_data.count,
                     adapter->num_rx_queues);
        smp_call_async_mask(&adapter->rx_sched_call_data);
    } else {
        printk("call_data.count == %u\n", atomic_read(&adapter->rx_sched_call_data.count));
    }
#else /* if !CONFIG_AT_MQ */

    if (likely(netif_rx_schedule_prep(&adapter->polling_netdev[0])))
        __netif_rx_schedule(&adapter->polling_netdev[0]);
//  else
//      atl1e_irq_enable(adapter);
#endif /* CONFIG_AT_MQ */
    }
 
#else /* if !CONFIG_AT_NAPI */

    // recv event
    if ( status & ISR_RX_EVENT ) {
        atl1e_clean_rx_irq(adapter, 0);
    }
    
    // transmit event
    if ( status & ISR_TX_EVENT ) {
        atl1e_clean_tx_irq(adapter);
    }
    
 
    if (--max_ints > 0) {
        status = AT_READ_REG(hw, REG_ISR);
        if (0 != (status & IMR_NORMAL_MASK))  
            goto  loopint;
    }
 
#endif /* CONFIG_AT_NAPI */     
    
    // re-enable Interrupt
    AT_WRITE_REG(&adapter->hw, REG_ISR, 0);
    
    return IRQ_HANDLED;
}


static boolean_t
atl1e_clean_tx_irq(struct atl1e_adapter* adapter)
{ 
    struct atl1e_buffer* buffer_info;
    TpdDescr* pTpd;    
    u16 hw_next_to_clean = AT_READ_REGW(&adapter->hw, REG_TPD_CONS_IDX);
    u16 next_to_clean = atomic_read(&adapter->tpd_next_clean);
    
    while (next_to_clean != hw_next_to_clean) {
        pTpd = adapter->tpd_ring + next_to_clean;
        buffer_info = adapter->tx_buffer_info + next_to_clean;
        if(buffer_info->dma) {
            pci_unmap_page(adapter->pdev,
                           buffer_info->dma,
                           buffer_info->length,
                           PCI_DMA_TODEVICE);
            buffer_info->dma = 0;
        }
        adapter->net_stats.tx_bytes += buffer_info->length;
        adapter->net_stats.tx_packets++;
        if(buffer_info->skb) {
            dev_kfree_skb_irq(buffer_info->skb);
            buffer_info->skb = NULL;
        }
        
        if (++next_to_clean == adapter->tpd_ring_size)  
            next_to_clean = 0;
    }

    atomic_set(&adapter->tpd_next_clean, next_to_clean);
    
    if(netif_queue_stopped(adapter->netdev) 
        && netif_carrier_ok(adapter->netdev))
        netif_wake_queue(adapter->netdev);
        
    return (next_to_clean == adapter->tpd_next_use) ? TRUE : FALSE;
}

static u16 vld_regs[] = 
{REG_HOST_RXF0_PAGE0_VLD, REG_HOST_RXF0_PAGE1_VLD,
 REG_HOST_RXF1_PAGE0_VLD, REG_HOST_RXF1_PAGE1_VLD,
 REG_HOST_RXF2_PAGE0_VLD, REG_HOST_RXF2_PAGE1_VLD,
 REG_HOST_RXF3_PAGE0_VLD, REG_HOST_RXF3_PAGE1_VLD};

static void
atl1e_rx_checksum(struct atl1e_adapter* adapter, struct sk_buff *skb, RecvRetStatus *prrs)
{
//    u8* packet = (u8*)(prrs+1);
//    struct iphdr* h;
    skb->ip_summed = CHECKSUM_NONE;
#if 0 
    if ((prrs->ipv4 || prrs->ipv6) && (prrs->tcp || prrs->udp)) {
        if (prrs->ipv4) {
            h = (struct iphdr*)(packet + (prrs->eth_type?22:14));
            if (0 != h->frag_off && 0 == prrs->ipv4_df) { // fragment packet
                goto hw_xsum;
            }
        }
        if (0 == prrs->ip_xsum && 0 == prrs->l4_xsum) { // good checksum
            skb->ip_summed = CHECKSUM_UNNECESSARY;
            return;
        }
    }
    
hw_xsum :
/*
    skb->csum = prrs->ippld_xsum;
    skb->ip_summed = CHECKSUM_COMPLETE;
*/
#endif
    return; 
}


static void
#ifdef CONFIG_AT_NAPI
atl1e_clean_rx_irq(struct atl1e_adapter *adapter, u8 que, int *work_done, int work_to_do)
#else
atl1e_clean_rx_irq(struct atl1e_adapter *adapter, u8 que)
#endif
{
    struct net_device *netdev = adapter->netdev;
    struct sk_buff* skb;
    struct atl1e_page* pPage = &(adapter->rxf_page[que][adapter->rxf_using[que]]);
    u32 packet_size, WrtOffset;
    RecvRetStatus*  prrs; 
    
    DEBUGFUNC("atl1e_clean_rx_irq");    
/*
    DEBUGOUT1("Page(%d) addr=%p, dma=%08X, wcmb=%p", que,
                pPage->addr, (u32)pPage->dma, 
                pPage->pWptr);
    return;
*/    
//    DEBUGOUT1("que=%d, wcmb=%p, *wcmb=%x",
//              que, pPage->pWptr, *pPage->pWptr);
    
    if (pPage->Rptr < (WrtOffset = *pPage->pWptr)) {
        
        do {
#ifdef CONFIG_AT_NAPI
            if (*work_done >= work_to_do)
                break;
            (*work_done)++;
 #endif         
            // get new packet's  rrs
            prrs = (RecvRetStatus*) (pPage->addr + pPage->Rptr);
            // check sequence number
            if (prrs->seq_num != adapter->rxf_nxseq[que]) {
                AT_ERR("rx sequence number error (rx=%d) (expect=%d)\n",
                    prrs->seq_num, adapter->rxf_nxseq[que]);
                adapter->rxf_nxseq[que]++;
                AT_WRITE_REG(&adapter->hw, 
                    0,  (((u32)prrs->seq_num)<<16)| adapter->rxf_nxseq[que]);
                AT_WRITE_REG(&adapter->hw, 0, que);
                goto fatal_err;
            }
            adapter->rxf_nxseq[que]++;

            // error packet ?
            if (prrs->err) {
                if (prrs->crc || prrs->dribble || prrs->code || prrs->trunc) { 
                    // hardware error, discard this packet
                    AT_ERR("rx packet desc error %x\n", *((u32*)prrs + 1));
                    adapter->net_stats.rx_errors++;
                    adapter->net_stats.rx_crc_errors++;
                    goto skip_pkt;
                }
            }
            
            packet_size = prrs->pkt_len - 4; // CRC
            skb = netdev_alloc_skb(netdev, packet_size + NET_IP_ALIGN);
            if (NULL == skb) {
                printk(KERN_WARNING"%s: Memory squeeze, deferring packet.\n", netdev->name);
                adapter->net_stats.rx_dropped++;
                goto skip_pkt;
            }
//            DEBUGOUT1("pktsize=%d", packet_size);
        
            skb_reserve(skb, NET_IP_ALIGN);
            skb->dev = netdev;
            
            // copy packet to user buffer
            memcpy(skb->data, (u8*)(prrs+1), packet_size);

            skb_put(skb, packet_size);
            skb->protocol = eth_type_trans(skb, netdev);
            atl1e_rx_checksum(adapter, skb, prrs);
            
#ifdef CONFIG_AT_NAPI
            if (unlikely(adapter->vlgrp && (prrs->vlan))) {
                u16 vlan_tag =  (prrs->vtag>>4)        |
                            ((prrs->vtag&7) << 13) |
                            ((prrs->vtag&8) << 9)  ;
                DEBUGOUT1("RXD VLAN TAG<RRD>=0x%04x", prrs->vtag);                          
                vlan_hwaccel_receive_skb(skb, adapter->vlgrp, vlan_tag);
            } else {
                netif_receive_skb(skb);
            }
#else /* CONFIG_AT_NAPI */
            if (unlikely(adapter->vlgrp && (prrs->vlan))) {
                u16 vlan_tag =  (prrs->vtag>>4)        |
                            ((prrs->vtag&7) << 13) |
                            ((prrs->vtag&8) << 9)  ;
                DEBUGOUT1("RXD VLAN TAG<RRD>=0x%04x", prrs->vtag);                          
                vlan_hwaccel_rx(skb, adapter->vlgrp, vlan_tag);
            } else {
                netif_rx(skb);
            }
#endif /* CONFIG_AT_NAPI */
 
            netdev->last_rx = jiffies;
        
            adapter->net_stats.rx_bytes += packet_size;
            adapter->net_stats.rx_packets++;
            if (prrs->mcast)  adapter->net_stats.multicast++;
        
skip_pkt:   // skip current packet whether it's ok or not.
    
            pPage->Rptr += 
                ((u32)(prrs->pkt_len + sizeof(RecvRetStatus) + 31)& 0xFFFFFFE0);
                
            if (pPage->Rptr >= adapter->rxf_length) { // reuse this page
                
                pPage->Rptr = *pPage->pWptr = 0;
                
                AT_WRITE_REGB(&adapter->hw, 
                    vld_regs[que*2+adapter->rxf_using[que]],
                    1);
                adapter->rxf_using[que] ^= 1;
                pPage = &(adapter->rxf_page[que][adapter->rxf_using[que]]);
                WrtOffset = *pPage->pWptr;
            }
            
        } while (pPage->Rptr < WrtOffset) ;
    }



    return;
    
fatal_err:
    
    if (!test_bit(__AT_DOWN, &adapter->flags)) {
        schedule_work(&adapter->reset_task);
    }
}


static inline u16 
tpd_avail(struct atl1e_adapter* adapter)
{
    u16 tpd_next_clean = atomic_read(&adapter->tpd_next_clean);
    
    return (u16)
        ((tpd_next_clean > adapter->tpd_next_use) ? \
         tpd_next_clean - adapter->tpd_next_use - 1 :   \
         adapter->tpd_ring_size + tpd_next_clean - adapter->tpd_next_use - 1);
}

static int
atl1e_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
    struct atl1e_adapter *adapter = netdev_priv(netdev);
    unsigned long flags;
    unsigned int len = skb->len;
    unsigned int nr_frags = 0;
    int tso;
    u16 tpd_req = 1;
    TpdDescr* pTpd;
    
#ifdef NETIF_F_TSO
    unsigned int mss = 0;
#endif
#ifdef MAX_SKB_FRAGS
    unsigned int f;
    len -= skb->data_len;
#endif    
    
    DEBUGFUNC("atl1e_xmit_frame");    
    
    if (test_bit(__AT_DOWN, &adapter->flags)) {
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }
    
    if (unlikely(skb->len <= 0)) {
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }

#ifdef MAX_SKB_FRAGS
    nr_frags = skb_shinfo(skb)->nr_frags;
    for(f = 0; f < nr_frags; f++) {
        u16 fg_size = skb_shinfo(skb)->frags[f].size;
        if (fg_size) {
            tpd_req += (fg_size + MAX_TX_BUF_LEN - 1) / MAX_TX_BUF_LEN;
        }
    }
#endif//MAX_SKB_FRAGS

#ifdef NETIF_F_TSO
    mss = skb_shinfo(skb)->gso_size;

    if(mss) {
        u8 proto_hdr_len;
        if(skb->protocol == ntohs(ETH_P_IP) 
#ifdef NETIF_F_TSO6            
            || (skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6)
#endif
                                                            ) {         
            proto_hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
            // need additional TPD ?
            if (proto_hdr_len != len) {
                tpd_req += (len - proto_hdr_len + MAX_TX_BUF_LEN - 1) / MAX_TX_BUF_LEN;
            }
        }
    }
#endif//NETIF_F_TSO 

#ifdef NETIF_F_LLTX
    local_irq_save(flags);
    if (!spin_trylock(&adapter->tx_lock)) {
        /* Collision - tell upper layer to requeue */
        local_irq_restore(flags);
        return NETDEV_TX_LOCKED;
    }
#else
    spin_lock_irqsave(&adapter->tx_lock, flags);
#endif

    if(tpd_avail(adapter) < tpd_req) {
        // no enough descriptor
        netif_stop_queue(netdev);
        spin_unlock_irqrestore(&adapter->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }
    
    // init tpd flags
    pTpd = adapter->tpd_ring + adapter->tpd_next_use;
    memset(pTpd, 0, sizeof(TpdDescr));
    
    
#ifdef NETIF_F_HW_VLAN_TX
    if(unlikely(adapter->vlgrp && vlan_tx_tag_present(skb))) {
            u16 vlan_tag = vlan_tx_tag_get(skb);
            vlan_tag = (vlan_tag << 4)          |
                       (vlan_tag >> 13)         |
                       ((vlan_tag >>9) & 0x8)   ;
        pTpd->ins_vlan = 1;
        pTpd->vlan = vlan_tag;
        DEBUGOUT1("TX VLAN TAG<TPD>=%04x", vlan_tag);
    }
#endif

    if (skb->protocol == ntohs(ETH_P_8021Q))
        pTpd->vlan_tag = 1; // may double-vlan

    // do TSO ?
    tso = atl1e_tso(adapter, skb, pTpd);
    
    if (tso < 0) {
        spin_unlock_irqrestore(&adapter->tx_lock, flags);       
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK; 
    }

    // do HW-CSUM ?
    if (0 == tso) {
        if (atl1e_tx_csum(adapter, skb, pTpd) < 0) {
            spin_unlock_irqrestore(&adapter->tx_lock, flags);
            dev_kfree_skb_any(skb);
            return NETDEV_TX_OK;
        }
    }
    
    atl1e_tx_map(adapter, skb, pTpd);
    atl1e_tx_queue(adapter, tpd_req, pTpd); 
    
    AT_WRITE_REG(&adapter->hw, REG_MB_TPD_PROD_IDX, adapter->tpd_next_use);
    
    spin_unlock_irqrestore(&adapter->tx_lock, flags);
    
    netdev->trans_start = jiffies;
    
    return NETDEV_TX_OK;
}
    

/*
 * atl1e_tso
 * return :
 *  0    -- no tso
 *  1    -- no tso, but tcp/ipv4 xsum
 *  2    -- no tso, but tcp/ipv6 xsum
 *  3    -- do ipv4 tso
 *  4    -- do ipv6 tso
 *  -x   -- bad packet, discard it
 */
static int
atl1e_tso(struct atl1e_adapter* adapter, struct sk_buff *skb, TpdDescr* pTpd)
{
#ifdef NETIF_F_TSO
    u8 hdr_len, ip_off;
    u32 real_len;
    
    int err;

    if (skb_is_gso(skb)) {
        if(skb_header_cloned(skb)) {
            err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
            if (err)
                return -1;
        }
        
        if(skb->protocol == htons(ETH_P_IP)) {
            
            real_len = (((unsigned char*)ip_hdr(skb) - skb->data) 
                    + ntohs(ip_hdr(skb)->tot_len));
            if (real_len < skb->len) {
                pskb_trim(skb, real_len);
            }
            
            hdr_len = (skb_transport_offset(skb) + tcp_hdrlen(skb));
            if (skb->len == hdr_len) { // only xsum need
                ip_hdr(skb)->check = 0;
                tcp_hdr(skb)->check = ~csum_tcpudp_magic(
                                        ip_hdr(skb)->saddr,
                                        ip_hdr(skb)->daddr,
                                        tcp_hdrlen(skb),
                                        IPPROTO_TCP,
                                        0);
                pTpd->iphdrlen = ip_hdr(skb)->ihl;
                pTpd->tcphdrlen = tcp_hdrlen(skb) >> 2;
                pTpd->ipxsum = 1;
                pTpd->tcpxsum = 1;
                return 1;
            }
            ip_hdr(skb)->check = 0;
            tcp_hdr(skb)->check = ~csum_tcpudp_magic(
                                        ip_hdr(skb)->saddr,
                                        ip_hdr(skb)->daddr,
                                        0,
                                        IPPROTO_TCP,
                                        0);
            ip_off = (unsigned char*)ip_hdr(skb) - (unsigned char*)skb_network_header(skb);
            if (ip_off == 8) // 802.3-SNAP Header
                pTpd->eth_type = 1;
            else if (ip_off != 0)
                return -2; // bad parameter
            // else // ethernet II
            
            pTpd->iphdrlen = ip_hdr(skb)->ihl;
            pTpd->tcphdrlen = tcp_hdrlen(skb) >> 2;
            pTpd->tcp_mss = skb_shinfo(skb)->gso_size;
            pTpd->segment = 1;
            return 3;
#ifdef NETIF_F_TSO6            
        } else if (skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6) {
            TpdIPv6Descr* p6 = (TpdIPv6Descr*) pTpd;
            real_len = (((unsigned char*)ipv6_hdr(skb) - skb->data) 
                        + ntohs(ipv6_hdr(skb)->payload_len));
            if (real_len < skb->len) {
                pskb_trim(skb, real_len);
            }
            
            // check payload == 0 byte ?
            hdr_len = (((unsigned char*)tcp_hdr(skb) - skb->data) 
                        + tcp_hdrlen(skb));
            if (skb->len == hdr_len) { // only xsum need
                TpdCXsumDescr* p = (TpdCXsumDescr*)pTpd;
                tcp_hdr(skb)->check = ~csum_tcpudp_magic(
                                        ip_hdr(skb)->saddr,
                                        ip_hdr(skb)->daddr,
                                        tcp_hdrlen(skb),
                                        IPPROTO_TCP,
                                        0);
                p->payld_ofs = ((unsigned char*)tcp_hdr(skb) - skb->data);
                p->psdxsum_ofs = ((unsigned char*)&(tcp_hdr(skb)->check) - skb->data);
                p->my_xsum = 1;
                return 2;                                        
            }
        
            tcp_hdr(skb)->check =
                ~csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
                         &ipv6_hdr(skb)->daddr,
                         0,
                         IPPROTO_TCP,
                         0);
            p6->ipver = 1; // IPV6
                        hdr_len >>= 1;
            p6->ipv6hdrlen_1 = (hdr_len&0x7);
            p6->ipv6hdrlen_2 = ((hdr_len >> 3)&0xF);
            p6->tcphdrlen = tcp_hdrlen(skb) >> 2;
            p6->tcp_mss = skb_shinfo(skb)->gso_size;
            p6->segment = 1;
            return 4;
#endif//NETIF_F_TSO6
        }
    }
#endif//NETIF_F_TSO 
    
    return 0;
}


/*
 * atl1e_tx_csum
 * return:
 *  >=0 : OK
 * -x   : failed, discard it
 */
static int
atl1e_tx_csum(struct atl1e_adapter *adapter, struct sk_buff *skb, TpdDescr* pTpd)
{
    TpdCXsumDescr* px = (TpdCXsumDescr*) pTpd;
    u8 css, cso;

    if(skb->ip_summed == CHECKSUM_PARTIAL) {
         
        cso = skb_transport_offset(skb);
        css = cso + skb->csum_offset;
        if(cso&0x1) {
            AT_ERR("payload offset != even number !\n");
            return  -1;
        }
        px->payld_ofs = cso;
        px->psdxsum_ofs = css;
        px->my_xsum = 1;
        //DEBUGOUT1("hardware chekcsum: css=%d, cso=%d", css, cso);
    }
    return 0;
}



static void
atl1e_tx_map(struct atl1e_adapter *adapter, struct sk_buff *skb, TpdDescr* pTpd)
{
    struct atl1e_buffer *buffer_info;
    u16 buf_len = skb->len;
    struct page *page;                                                       
    unsigned long offset;  
    u16 next_to_use;
    
#ifdef MAX_SKB_FRAGS
    unsigned int nr_frags;
    unsigned int f;
    
    buf_len -= skb->data_len;
    nr_frags = skb_shinfo(skb)->nr_frags;
#endif//MAX_SKB_FRAGS

    
    next_to_use = adapter->tpd_next_use;
    
    buffer_info = &adapter->tx_buffer_info[next_to_use]; 
    if (buffer_info->skb) {
        BUG();
    }
    buffer_info->skb = NULL;    // put skb in last TPD

#ifdef NETIF_F_TSO
    if (pTpd->segment) { // TSO
        u8 hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
        buffer_info->length = hdr_len;
        page = virt_to_page(skb->data);                                            
        offset = (unsigned long)skb->data & ~PAGE_MASK;                           
        buffer_info->dma =
                        pci_map_page(adapter->pdev,
                                     page,
                                     offset,
                                     hdr_len,
                                     PCI_DMA_TODEVICE);
                                     
        if(++next_to_use == adapter->tpd_ring_size) next_to_use = 0;
        
        if (buf_len > hdr_len) {
            u16 data_len = buf_len - hdr_len;
            u16 i, nSeg = (data_len + MAX_TX_BUF_LEN - 1) / MAX_TX_BUF_LEN;
            for (i=0; i < nSeg; i++) {
                buffer_info = &adapter->tx_buffer_info[next_to_use];
                buffer_info->skb = NULL;
                buffer_info->length = 
                    (data_len >= MAX_TX_BUF_LEN) ? MAX_TX_BUF_LEN : data_len;
                data_len -= buffer_info->length;
                page = virt_to_page(skb->data+(hdr_len+i*MAX_TX_BUF_LEN));                                            
                offset = (unsigned long)(skb->data+(i*MAX_TX_BUF_LEN+hdr_len)) & ~PAGE_MASK;
                buffer_info->dma = 
                        pci_map_page(adapter->pdev,
                                     page, 
                                     offset,
                                     buffer_info->length,
                                     PCI_DMA_TODEVICE);
                if(++next_to_use == adapter->tpd_ring_size) next_to_use = 0;  
            } 
        }
    } else {
#endif//NETIF_F_TSO
    
    buffer_info->length = buf_len;
    page = virt_to_page(skb->data);                                            
    offset = (unsigned long)skb->data & ~PAGE_MASK;
    buffer_info->dma =
                    pci_map_page(adapter->pdev,
                                 page,
                                 offset,
                                 buf_len,
                                 PCI_DMA_TODEVICE);
    if(++next_to_use == adapter->tpd_ring_size) next_to_use = 0;   
#ifdef NETIF_F_TSO
    }    
#endif//NETIF_F_TSO

#ifdef MAX_SKB_FRAGS
    for(f = 0; f < nr_frags; f++) {
        struct skb_frag_struct *frag;
        u16 i, nSeg;
        
        frag = &skb_shinfo(skb)->frags[f];
        buf_len = frag->size;
        
    nSeg = (buf_len + MAX_TX_BUF_LEN - 1) / MAX_TX_BUF_LEN;
        for (i=0; i < nSeg; i++) {
            buffer_info = &adapter->tx_buffer_info[next_to_use];
            if (buffer_info->skb) {
                BUG();
            }
            buffer_info->skb = NULL;
            buffer_info->length = 
                (buf_len > MAX_TX_BUF_LEN) ? MAX_TX_BUF_LEN : buf_len;
            buf_len -= buffer_info->length;
            
            buffer_info->dma =
                pci_map_page(adapter->pdev,
                             frag->page,
                             frag->page_offset+i*MAX_TX_BUF_LEN,
                             buffer_info->length,
                             PCI_DMA_TODEVICE);
                             
            if(++next_to_use == adapter->tpd_ring_size) next_to_use = 0; 
        } 
    }
#endif//MAX_SKB_FRAGS
    
    // last tpd's buffer-info
    buffer_info->skb = skb;
}


static void
atl1e_tx_queue(struct atl1e_adapter *adapter, u16 count, TpdDescr* pTpd)
{
    struct atl1e_buffer * buffer_info;
    TpdDescr* p;
    u16 j;
    u16 next_to_use = adapter->tpd_next_use;
    
    // DEBUGOUT1("<atl1e_tx_quue> count=%d, tpd_desc=%llx", count, descr->data);
        
    for(j=0; j < count; j++) {
        buffer_info = adapter->tx_buffer_info + next_to_use;
        p = adapter->tpd_ring + next_to_use;
        if (p != pTpd) {
            memcpy(p, pTpd, sizeof(TpdDescr));
        }
        p->addr = cpu_to_le64(buffer_info->dma);
        p->buf_len = cpu_to_le16(buffer_info->length);

       /* 
    DEBUGOUT1("<atl1e_tx_quue> TPD_NEXT_TO_USE: %d buf_len=%d", 
            tpd_next_to_use,
            buffer_info->length);
        */
#ifdef NETIF_F_TSO
        p->hdr_flag = ((p->segment)&& (0==j)) ? 1 : 0;
#endif//NETIF_F_TSO

        if (j == (count-1))
            p->eop = 1;
       
        if(++next_to_use == adapter->tpd_ring_size) 
            next_to_use = 0;
    }

    /* Force memory writes to complete before letting h/w
     * know there are new descriptors to fetch.  (Only
     * applicable for weak-ordered memory model archs,
     * such as IA-64). */
    wmb();
    
    adapter->tpd_next_use = next_to_use;
}


/**
 * atl1e_phy_config - Timer Call-back
 * @data: pointer to netdev cast into an unsigned long
 **/

static void
atl1e_phy_config(unsigned long data)
{
    struct atl1e_adapter *adapter = (struct atl1e_adapter *) data;
    struct atl1e_hw *hw = &adapter->hw;  
    unsigned long flags;

     DEBUGFUNC("atl1e_phy_reconfig!");
    
    spin_lock_irqsave(&adapter->stats_lock, flags);
    
    atl1e_restart_autoneg(hw);

    spin_unlock_irqrestore(&adapter->stats_lock, flags);
    clear_bit(0, &adapter->cfg_phy);
}


/**
 * atl1e_watchdog - Timer Call-back
 * @data: pointer to netdev cast into an unsigned long
 **/

static void
atl1e_watchdog(unsigned long data)
{
    struct atl1e_adapter *adapter = (struct atl1e_adapter *) data;
    unsigned long flags;
    
    spin_lock_irqsave(&adapter->stats_lock, flags);

    spin_unlock_irqrestore(&adapter->stats_lock, flags);
    
#if 0        
    if (!netif_carrier_ok(adapter->netdev)) {
        schedule_work(&adapter->link_chg_task);
    }
#endif

    /* Reset the timer */
    mod_timer(&adapter->watchdog_timer, jiffies + 4 * HZ);
}

