/************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98

  Contact Information:

  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226

**************************************************************/
/**************************************************************************
 * @ingroup IEGBE_GENERAL
 *
 * @file iegbe_main.c
 *
 * @description
 *   This module contains the upper-edge routines of the driver
 *   interface that handle initialization, resets, and shutdowns.
 *
 **************************************************************************/

#include "iegbe.h"
#include "gcu_if.h"
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>

char iegbe_driver_name[] = "iegbe";
char iegbe_driver_string[] = "Gigabit Ethernet Controller Driver";
#define DRV_VERSION "1.0.0-K28-NAPI"
char iegbe_driver_version[] = DRV_VERSION;
char iegbe_copyright[] = "Copyright (c) 1999-2009 Intel Corporation.";


/* iegbe_pci_tbl - PCI Device ID Table
 *
 * Last entry must be all 0s
 *
 * Macro expands to...
 *   {PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}
 */
static struct pci_device_id iegbe_pci_tbl[] = {
    INTEL_E1000_ETHERNET_DEVICE(0x5040),
    INTEL_E1000_ETHERNET_DEVICE(0x5041),
    INTEL_E1000_ETHERNET_DEVICE(0x5042),
    INTEL_E1000_ETHERNET_DEVICE(0x5043),
    INTEL_E1000_ETHERNET_DEVICE(0x5044),
    INTEL_E1000_ETHERNET_DEVICE(0x5045),
    INTEL_E1000_ETHERNET_DEVICE(0x5046),
    INTEL_E1000_ETHERNET_DEVICE(0x5047),
    INTEL_E1000_ETHERNET_DEVICE(0x5048),
    INTEL_E1000_ETHERNET_DEVICE(0x5049),
    INTEL_E1000_ETHERNET_DEVICE(0x504A),
    INTEL_E1000_ETHERNET_DEVICE(0x504B),
    /* required last entry */
	{0,}
};

MODULE_DEVICE_TABLE(pci, iegbe_pci_tbl);


int iegbe_up(struct iegbe_adapter *adapter);
void iegbe_down(struct iegbe_adapter *adapter);
void iegbe_reinit_locked(struct iegbe_adapter *adapter);
void iegbe_reset(struct iegbe_adapter *adapter);
int iegbe_set_spd_dplx(struct iegbe_adapter *adapter, uint16_t spddplx);
int iegbe_setup_all_tx_resources(struct iegbe_adapter *adapter);
int iegbe_setup_all_rx_resources(struct iegbe_adapter *adapter);
void iegbe_free_all_tx_resources(struct iegbe_adapter *adapter);
void iegbe_free_all_rx_resources(struct iegbe_adapter *adapter);
static int iegbe_setup_tx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_tx_ring *txdr);
static int iegbe_setup_rx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_rx_ring *rxdr);
static void iegbe_free_tx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_tx_ring *tx_ring);
static void iegbe_free_rx_resources(struct iegbe_adapter *adapter,
                             struct iegbe_rx_ring *rx_ring);
void iegbe_update_stats(struct iegbe_adapter *adapter);
static int iegbe_init_module(void);
static void iegbe_exit_module(void);
static int iegbe_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void iegbe_remove(struct pci_dev *pdev);
static int iegbe_alloc_queues(struct iegbe_adapter *adapter);
static int iegbe_sw_init(struct iegbe_adapter *adapter);
static int iegbe_open(struct net_device *netdev);
static int iegbe_close(struct net_device *netdev);
static void iegbe_configure_tx(struct iegbe_adapter *adapter);
static void iegbe_configure_rx(struct iegbe_adapter *adapter);
static void iegbe_setup_rctl(struct iegbe_adapter *adapter);
static void iegbe_clean_all_tx_rings(struct iegbe_adapter *adapter);
static void iegbe_clean_all_rx_rings(struct iegbe_adapter *adapter);
static void iegbe_clean_tx_ring(struct iegbe_adapter *adapter,
                                struct iegbe_tx_ring *tx_ring);
static void iegbe_clean_rx_ring(struct iegbe_adapter *adapter,
                                struct iegbe_rx_ring *rx_ring);

static void iegbe_set_rx_mode(struct net_device *netdev);
static void iegbe_update_phy_info(unsigned long data);
static void iegbe_watchdog(unsigned long data);
static void iegbe_82547_tx_fifo_stall(unsigned long data);
static int iegbe_xmit_frame(struct sk_buff *skb, struct net_device *netdev);
static struct net_device_stats * iegbe_get_stats(struct net_device *netdev);
static int iegbe_change_mtu(struct net_device *netdev, int new_mtu);
static int iegbe_set_mac(struct net_device *netdev, void *p);
static irqreturn_t iegbe_intr(int irq, void *data);

static irqreturn_t iegbe_intr_msi(int irq, void *data);

static bool iegbe_clean_tx_irq(struct iegbe_adapter *adapter,
                                    struct iegbe_tx_ring *tx_ring);
static int iegbe_clean(struct napi_struct *napi, int budget);
static bool iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                                    struct iegbe_rx_ring *rx_ring,
                                    int *work_done, int work_to_do);
static bool iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                                       struct iegbe_rx_ring *rx_ring,
                                       int *work_done, int work_to_do);


static void iegbe_alloc_rx_buffers(struct iegbe_adapter *adapter,
                                   struct iegbe_rx_ring *rx_ring,
                                   int cleaned_count);
static void iegbe_alloc_rx_buffers_ps(struct iegbe_adapter *adapter,
                                      struct iegbe_rx_ring *rx_ring,
                                      int cleaned_count);


static int iegbe_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
static int iegbe_mii_ioctl(struct net_device *netdev, struct ifreq *ifr,
               int cmd);
void set_ethtool_ops(struct net_device *netdev);
extern int ethtool_ioctl(struct ifreq *ifr);
static void iegbe_enter_82542_rst(struct iegbe_adapter *adapter);
static void iegbe_leave_82542_rst(struct iegbe_adapter *adapter);
static void iegbe_tx_timeout(struct net_device *dev);
static void iegbe_reset_task(struct work_struct *work);
static void iegbe_smartspeed(struct iegbe_adapter *adapter);
static inline int iegbe_82547_fifo_workaround(struct iegbe_adapter *adapter,
                          struct sk_buff *skb);

static bool iegbe_vlan_used(struct iegbe_adapter *adapter);
static int iegbe_vlan_rx_add_vid(struct net_device *netdev,__be16 proto, uint16_t vid);
static int iegbe_vlan_rx_kill_vid(struct net_device *netdev,__be16 proto, uint16_t vid);
static void iegbe_restore_vlan(struct iegbe_adapter *adapter);

static int iegbe_notify_reboot(struct notifier_block *,
                               unsigned long event,
                               void *ptr);
static int iegbe_suspend(struct pci_dev *pdev, pm_message_t state);
#ifdef CONFIG_PM
static int iegbe_resume(struct pci_dev *pdev);
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/* for netdump / net console */
static void iegbe_netpoll (struct net_device *netdev);
#endif

#define COPYBREAK_DEFAULT 256
static unsigned int copybreak __read_mostly = COPYBREAK_DEFAULT;
module_param(copybreak, uint, 0644);
MODULE_PARM_DESC(copybreak,
	"Maximum size of packet that is copied to a new buffer on receive");
void iegbe_rx_schedule(void *data);

struct notifier_block iegbe_notifier_reboot = {
    .notifier_call    = iegbe_notify_reboot,
    .next        = NULL,
    .priority    = 0
};

/* Exported from other modules */

extern void iegbe_check_options(struct iegbe_adapter *adapter);

static struct pci_driver iegbe_driver = {
    .name     = iegbe_driver_name,
    .id_table = iegbe_pci_tbl,
    .probe    = iegbe_probe,
    .remove   = iegbe_remove,
    /* Power Managment Hooks */
#ifdef CONFIG_PM
    .suspend  = iegbe_suspend,
    .resume   = iegbe_resume
#endif
};

MODULE_AUTHOR("Intel(R) Corporation");
MODULE_DESCRIPTION("Gigabit Ethernet Controller Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

static int debug = NETIF_MSG_DRV | NETIF_MSG_PROBE;
module_param(debug, int, 0x0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

static uint8_t gcu_suspend = 0x0;
static uint8_t gcu_resume = 0x0;
struct pci_dev *gcu = NULL;


/**
 * iegbe_iegbe_tasklet -*
 **/
/**
 * iegbe_init_module - Driver Registration Routine
 *
 * iegbe_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/

static int __init iegbe_init_module(void)
{
    int ret;

    printk(KERN_INFO "%s - version %s\n",
           iegbe_driver_string, iegbe_driver_version);

    printk(KERN_INFO "%s\n", iegbe_copyright);

    ret = pci_register_driver(&iegbe_driver);
	if (copybreak != COPYBREAK_DEFAULT) {
		if (copybreak == 0)
			printk(KERN_INFO "iegbe: copybreak disabled\n");
		else
			printk(KERN_INFO "iegbe: copybreak enabled for "
			       "packets <= %u bytes\n", copybreak);
    }
    return ret;
}

module_init(iegbe_init_module);

/**
 * iegbe_exit_module - Driver Exit Cleanup Routine
 *
 * iegbe_exit_module is called just before the driver is removed
 * from memory.
 **/

static void __exit iegbe_exit_module(void)
{
    pci_unregister_driver(&iegbe_driver);
}

module_exit(iegbe_exit_module);

static int iegbe_request_irq(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	irq_handler_t handler = iegbe_intr;
	int irq_flags = IRQF_SHARED;
	int err;
	adapter->have_msi = !pci_enable_msi(adapter->pdev);
	if (adapter->have_msi) {
		handler = iegbe_intr_msi;
		irq_flags = 0;
	}
	err = request_irq(adapter->pdev->irq, handler, irq_flags, netdev->name,
	                  netdev);
	if (err) {
		if (adapter->have_msi)
			pci_disable_msi(adapter->pdev);
		DPRINTK(PROBE, ERR,
		        "Unable to allocate interrupt Error: %d\n", err);
	}
	return err;
}
static void iegbe_free_irq(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	free_irq(adapter->pdev->irq, netdev);
	if (adapter->have_msi)
		pci_disable_msi(adapter->pdev);
}
/**
 * iegbe_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 **/

static void iegbe_irq_disable(struct iegbe_adapter *adapter)
{
    E1000_WRITE_REG(&adapter->hw, IMC, ~0);
    E1000_WRITE_FLUSH(&adapter->hw);
    synchronize_irq(adapter->pdev->irq);
}

/**
 * iegbe_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 **/

static inline void
iegbe_irq_enable(struct iegbe_adapter *adapter)
{
        E1000_WRITE_REG(&adapter->hw, IMS, IMS_ENABLE_MASK);
        E1000_WRITE_FLUSH(&adapter->hw);
}

static void iegbe_update_mng_vlan(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
        struct net_device *netdev = adapter->netdev;
	u16 vid = hw->mng_cookie.vlan_id;
        u16 old_vid = adapter->mng_vlan_id;
        if (iegbe_vlan_used(adapter)) {
                if (!test_bit(old_vid, adapter->active_vlans)) {
			if (hw->mng_cookie.status &
                                E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) {
                                iegbe_vlan_rx_add_vid(netdev, htons(ETH_P_8021Q), vid);
                                adapter->mng_vlan_id = vid;
                        } else
                                adapter->mng_vlan_id = E1000_MNG_VLAN_NONE;

                        if ((old_vid != (u16)E1000_MNG_VLAN_NONE) &&
                                        (vid != old_vid) &&
                            !test_bit(old_vid, adapter->active_vlans))
                                iegbe_vlan_rx_kill_vid(netdev, htons(ETH_P_8021Q), old_vid);
                } else
                        adapter->mng_vlan_id = vid;
        }
}

/**
 * iegbe_configure - configure the hardware for RX and TX
 * @adapter = private board structure
 **/
static void iegbe_configure(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int i;

	iegbe_set_rx_mode(netdev);

	iegbe_restore_vlan(adapter);

	iegbe_configure_tx(adapter);
	iegbe_setup_rctl(adapter);
	iegbe_configure_rx(adapter);
	/* call E1000_DESC_UNUSED which always leaves
	 * at least 1 descriptor unused to make sure
	 * next_to_use != next_to_clean */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct iegbe_rx_ring *ring = &adapter->rx_ring[i];
		adapter->alloc_rx_buf(adapter, ring,
		                      E1000_DESC_UNUSED(ring));
	}

	adapter->tx_queue_len = netdev->tx_queue_len;
}

int iegbe_up(struct iegbe_adapter *adapter)
{
	/* hardware has been reset, we need to reload some things */
	iegbe_configure(adapter);

	clear_bit(__E1000_DOWN, &adapter->flags);

	napi_enable(&adapter->napi);

	iegbe_irq_enable(adapter);

         adapter->hw.get_link_status = 0x1;
	return 0;
}

/**
 * iegbe_power_up_phy - restore link in case the phy was powered down
 * @adapter: address of board private structure
 *
 * The phy may be powered down to save power and turn off link when the
 * driver is unloaded and wake on lan is not enabled (among others)
 * *** this routine MUST be followed by a call to iegbe_reset ***
 *
 **/

void iegbe_power_up_phy(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	u16 mii_reg = 0;

	/* Just clear the power down bit to wake the phy back up */
	if (hw->media_type == iegbe_media_type_copper) {
		/* according to the manual, the phy will retain its
		 * settings across a power-down/up cycle */
		iegbe_read_phy_reg(hw, PHY_CTRL, &mii_reg);
		mii_reg &= ~MII_CR_POWER_DOWN;
		iegbe_write_phy_reg(hw, PHY_CTRL, mii_reg);
	}
}

static void iegbe_power_down_phy(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;

	/* Power down the PHY so no link is implied when interface is down *
	 * The PHY cannot be powered down if any of the following is true *
	 * (a) WoL is enabled
	 * (b) AMT is active
	 * (c) SoL/IDER session is active */
	if (!adapter->wol && hw->mac_type >= iegbe_82540 &&
	   hw->media_type == iegbe_media_type_copper) {
		u16 mii_reg = 0;

		switch (hw->mac_type) {
		case iegbe_82540:
		case iegbe_82545:
		case iegbe_82545_rev_3:
		case iegbe_82546:
		case iegbe_82546_rev_3:
		case iegbe_82541:
		case iegbe_82541_rev_2:
		case iegbe_82547:
		case iegbe_82547_rev_2:
			if (E1000_READ_REG(&adapter->hw, MANC) & E1000_MANC_SMBUS_EN)
				goto out;
			break;
		case iegbe_82571:
		case iegbe_82572:
		case iegbe_82573:
			if (iegbe_check_mng_mode(hw) ||
			    iegbe_check_phy_reset_block(hw))
				goto out;
			break;
		default:
			goto out;
		}
		iegbe_read_phy_reg(hw, PHY_CTRL, &mii_reg);
		mii_reg |= MII_CR_POWER_DOWN;
		iegbe_write_phy_reg(hw, PHY_CTRL, mii_reg);
		mdelay(1);
	}
out:
	return;
}

void iegbe_down(struct iegbe_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;

	/* signal that we're down so the interrupt handler does not
	 * reschedule our watchdog timer */
	set_bit(__E1000_DOWN, &adapter->flags);

	napi_disable(&adapter->napi);

    iegbe_irq_disable(adapter);

    del_timer_sync(&adapter->tx_fifo_stall_timer);
    del_timer_sync(&adapter->watchdog_timer);
    del_timer_sync(&adapter->phy_info_timer);

	netdev->tx_queue_len = adapter->tx_queue_len;
	adapter->link_speed = 0;
	adapter->link_duplex = 0;
    netif_carrier_off(netdev);
    netif_stop_queue(netdev);

    iegbe_reset(adapter);
    iegbe_clean_all_tx_rings(adapter);
    iegbe_clean_all_rx_rings(adapter);
}
void iegbe_reinit_locked(struct iegbe_adapter *adapter)
{
	WARN_ON(in_interrupt());
	while (test_and_set_bit(__E1000_RESETTING, &adapter->flags))
		msleep(1);
	iegbe_down(adapter);
	iegbe_up(adapter);
	clear_bit(__E1000_RESETTING, &adapter->flags);
}

void iegbe_reset(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	u32 pba = 0, tx_space, min_tx_space, min_rx_space;
	u16 fc_high_water_mark = E1000_FC_HIGH_DIFF;
	bool legacy_pba_adjust = false;

    /* Repartition Pba for greater than 9k mtu
     * To take effect CTRL.RST is required.
     */

	switch (hw->mac_type) {
	case iegbe_82542_rev2_0:
	case iegbe_82542_rev2_1:
	case iegbe_82543:
	case iegbe_82544:
	case iegbe_82540:
	case iegbe_82541:
	case iegbe_82541_rev_2:
	case iegbe_icp_xxxx:
		legacy_pba_adjust = true;
		pba = E1000_PBA_48K;
		break;
	case iegbe_82545:
	case iegbe_82545_rev_3:
	case iegbe_82546:
	case iegbe_82546_rev_3:
		pba = E1000_PBA_48K;
		break;
    case iegbe_82547:
	case iegbe_82573:
    case iegbe_82547_rev_2:
		legacy_pba_adjust = true;
        pba = E1000_PBA_30K;
        break;
    case iegbe_82571:
    case iegbe_82572:
	case iegbe_undefined:
	case iegbe_num_macs:
        break;
    }

	if (legacy_pba_adjust) {
		if (adapter->netdev->mtu > E1000_RXBUFFER_8192)
			pba -= 8; /* allocate more FIFO for Tx */
        /* send an XOFF when there is enough space in the
         * Rx FIFO to hold one extra full size Rx packet
        */


		if (hw->mac_type == iegbe_82547) {
			adapter->tx_fifo_head = 0;
        adapter->tx_head_addr = pba << E1000_TX_HEAD_ADDR_SHIFT;
        adapter->tx_fifo_size =
            (E1000_PBA_40K - pba) << E1000_PBA_BYTES_SHIFT;
			atomic_set(&adapter->tx_fifo_stall, 0);
    }
	} else if (hw->max_frame_size > MAXIMUM_ETHERNET_FRAME_SIZE) {
		E1000_WRITE_REG(&adapter->hw, PBA, pba);

		/* To maintain wire speed transmits, the Tx FIFO should be
		 * large enough to accomodate two full transmit packets,
		 * rounded up to the next 1KB and expressed in KB.  Likewise,
		 * the Rx FIFO should be large enough to accomodate at least
		 * one full receive packet and is similarly rounded up and
		 * expressed in KB. */
		pba = 	E1000_READ_REG(&adapter->hw, PBA);
		/* upper 16 bits has Tx packet buffer allocation size in KB */
		tx_space = pba >> 16;
		/* lower 16 bits has Rx packet buffer allocation size in KB */
		pba &= 0xffff;
		/* don't include ethernet FCS because hardware appends/strips */
		min_rx_space = adapter->netdev->mtu + ENET_HEADER_SIZE +
		               VLAN_TAG_SIZE;
		min_tx_space = min_rx_space;
		min_tx_space *= 2;
		min_tx_space = ALIGN(min_tx_space, 1024);
		min_tx_space >>= 10;
		min_rx_space = ALIGN(min_rx_space, 1024);
		min_rx_space >>= 10;

		/* If current Tx allocation is less than the min Tx FIFO size,
		 * and the min Tx FIFO size is less than the current Rx FIFO
		 * allocation, take space away from current Rx allocation */
		if (tx_space < min_tx_space &&
		    ((min_tx_space - tx_space) < pba)) {
			pba = pba - (min_tx_space - tx_space);

			/* PCI/PCIx hardware has PBA alignment constraints */
			switch (hw->mac_type) {
			case iegbe_82545 ... iegbe_82546_rev_3:
				pba &= ~(E1000_PBA_8K - 1);
				break;
			default:
				break;
			}

			/* if short on rx space, rx wins and must trump tx
			 * adjustment or use Early Receive if available */
			if (pba < min_rx_space) {
				switch (hw->mac_type) {
				case iegbe_82573:
					/* ERT enabled in iegbe_configure_rx */
					break;
				default:
					pba = min_rx_space;
					break;
				}
			}
		}
	}

	E1000_WRITE_REG(&adapter->hw, PBA, pba);

	/* flow control settings */
	/* Set the FC high water mark to 90% of the FIFO size.
	 * Required to clear last 3 LSB */
	fc_high_water_mark = ((pba * 9216)/10) & 0xFFF8;
	/* We can't use 90% on small FIFOs because the remainder
	 * would be less than 1 full frame.  In this case, we size
	 * it to allow at least a full frame above the high water
	 *  mark. */
	if (pba < E1000_PBA_16K)
		fc_high_water_mark = (pba * 1024) - 1600;

	hw->fc_high_water = fc_high_water_mark;
	hw->fc_low_water = fc_high_water_mark - 8;
		hw->fc_pause_time = E1000_FC_PAUSE_TIME;
	hw->fc_send_xon = 1;
	hw->fc = hw->original_fc;

	/* Allow time for pending master requests to run */
	iegbe_reset_hw(hw);
	if (hw->mac_type >= iegbe_82544)
		E1000_WRITE_REG(&adapter->hw, WUC, 0);

	if (iegbe_init_hw(hw))
		DPRINTK(PROBE, ERR, "Hardware Error\n");
	iegbe_update_mng_vlan(adapter);

	/* if (adapter->hwflags & HWFLAGS_PHY_PWR_BIT) { */
	if (hw->mac_type >= iegbe_82544 &&
	    hw->mac_type <= iegbe_82547_rev_2 &&
	    hw->autoneg == 1 &&
	    hw->autoneg_advertised == ADVERTISE_1000_FULL) {
		u32 ctrl = E1000_READ_REG(&adapter->hw, CTRL);
		/* clear phy power management bit if we are in gig only mode,
		 * which if enabled will attempt negotiation to 100Mb, which
		 * can cause a loss of link at power off or driver unload */
		ctrl &= ~E1000_CTRL_SWDPIN3;
		E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);
	}

	/* Enable h/w to recognize an 802.1Q VLAN Ethernet packet */
	E1000_WRITE_REG(&adapter->hw, VET, ETHERNET_IEEE_VLAN_TYPE);

	iegbe_reset_adaptive(hw);
	iegbe_phy_get_info(hw, &adapter->phy_info);

	if (!adapter->smart_power_down &&
	    (hw->mac_type == iegbe_82571 ||
	     hw->mac_type == iegbe_82572)) {
		u16 phy_data = 0;
		/* speed up time to link by disabling smart power down, ignore
		 * the return value of this function because there is nothing
		 * different we would do if it failed */
		iegbe_read_phy_reg(hw, IGP02E1000_PHY_POWER_MGMT,
		                   &phy_data);
		phy_data &= ~IGP02E1000_PM_SPD;
		iegbe_write_phy_reg(hw, IGP02E1000_PHY_POWER_MGMT,
		                    phy_data);
	}

}

/**
 *  Dump the eeprom for users having checksum issues
 **/
static void iegbe_dump_eeprom(struct iegbe_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct ethtool_eeprom eeprom;
	const struct ethtool_ops *ops = netdev->ethtool_ops;
	u8 *data;
	int i;
	u16 csum_old, csum_new = 0;

	eeprom.len = ops->get_eeprom_len(netdev);
	eeprom.offset = 0;

	data = kmalloc(eeprom.len, GFP_KERNEL);
	if (!data) {
		printk(KERN_ERR "Unable to allocate memory to dump EEPROM"
		       " data\n");
		return;
	}

	ops->get_eeprom(netdev, &eeprom, data);

	csum_old = (data[EEPROM_CHECKSUM_REG * 2]) +
		   (data[EEPROM_CHECKSUM_REG * 2 + 1] << 8);
	for (i = 0; i < EEPROM_CHECKSUM_REG * 2; i += 2)
		csum_new += data[i] + (data[i + 1] << 8);
	csum_new = EEPROM_SUM - csum_new;

	printk(KERN_ERR "/*********************/\n");
	printk(KERN_ERR "Current EEPROM Checksum : 0x%04x\n", csum_old);
	printk(KERN_ERR "Calculated              : 0x%04x\n", csum_new);

	printk(KERN_ERR "Offset    Values\n");
	printk(KERN_ERR "========  ======\n");
	print_hex_dump(KERN_ERR, "", DUMP_PREFIX_OFFSET, 16, 1, data, 128, 0);

	printk(KERN_ERR "Include this output when contacting your support "
	       "provider.\n");
	printk(KERN_ERR "This is not a software error! Something bad "
	       "happened to your hardware or\n");
	printk(KERN_ERR "EEPROM image. Ignoring this "
	       "problem could result in further problems,\n");
	printk(KERN_ERR "possibly loss of data, corruption or system hangs!\n");
	printk(KERN_ERR "The MAC Address will be reset to 00:00:00:00:00:00, "
	       "which is invalid\n");
	printk(KERN_ERR "and requires you to set the proper MAC "
	       "address manually before continuing\n");
	printk(KERN_ERR "to enable this network device.\n");
	printk(KERN_ERR "Please inspect the EEPROM dump and report the issue "
	       "to your hardware vendor\n");
	printk(KERN_ERR "or Intel Customer Support.\n");
	printk(KERN_ERR "/*********************/\n");

	kfree(data);
}

static const struct net_device_ops iegbe_netdev_ops = {
	.ndo_open		= iegbe_open,
	.ndo_stop		= iegbe_close,
	.ndo_start_xmit		= iegbe_xmit_frame,
	.ndo_get_stats		= iegbe_get_stats,
	.ndo_set_rx_mode	= iegbe_set_rx_mode,
	.ndo_set_mac_address	= iegbe_set_mac,
	.ndo_tx_timeout		= iegbe_tx_timeout,
	.ndo_change_mtu		= iegbe_change_mtu,
	.ndo_do_ioctl		= iegbe_ioctl,
	.ndo_validate_addr	= eth_validate_addr,

	.ndo_vlan_rx_add_vid	= iegbe_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= iegbe_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= iegbe_netpoll,
#endif
};

/**
 * iegbe_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in iegbe_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * iegbe_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
#define DECLARE_MAC_BUF(var) char var[18] __maybe_unused


static int iegbe_probe(struct pci_dev *pdev,
            const struct pci_device_id *ent)
{
    struct net_device *netdev;
    struct iegbe_adapter *adapter;
	struct iegbe_hw *hw;

	static int cards_found = 0;
	int err, pci_using_dac;
	u16 eeprom_data = 0;
	u16 eeprom_apme_mask = E1000_EEPROM_APME;
	int bars; 

	bars = pci_select_bars(pdev, IORESOURCE_MEM);
	err = pci_enable_device(pdev);

	if (err)
        return err;

	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64)) &&
	    !pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64))) {
		pci_using_dac = 1;
        } else {
             err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
   	     if (err) {
			err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
			if (err) {
				E1000_ERR("No usable DMA configuration, "
					  "aborting\n");
				goto err_dma;
        }
    }
		pci_using_dac = 0;
    }

	err = pci_request_selected_regions(pdev, bars, iegbe_driver_name);
	if (err)
		goto err_pci_reg;

    pci_set_master(pdev);

	err = -ENOMEM;
    netdev = alloc_etherdev(sizeof(struct iegbe_adapter));
	if (!netdev)
        goto err_alloc_etherdev;

	SET_NETDEV_DEV(netdev, &pdev->dev);

    pci_set_drvdata(pdev, netdev);
    adapter = netdev_priv(netdev);
    adapter->netdev = netdev;
    adapter->pdev = pdev;
	adapter->msg_enable = (1 << debug) - 1;
	adapter->bars = bars;

	hw = &adapter->hw;
	hw->back = adapter;

        err = -EIO;
	hw->hw_addr = ioremap(pci_resource_start(pdev, BAR_0),
			      pci_resource_len(pdev, BAR_0));
	if (!hw->hw_addr)
        goto err_ioremap;

	netdev->netdev_ops = &iegbe_netdev_ops;
	set_ethtool_ops(netdev);
	netdev->watchdog_timeo = 5 * HZ;
	netif_napi_add(netdev, &adapter->napi, iegbe_clean, 64);

	strncpy(netdev->name, pci_name(pdev), sizeof(netdev->name) - 1);


    adapter->bd_number = cards_found;

    /* setup the private structure */

	err = iegbe_sw_init(adapter);
	if (err)
        goto err_sw_init;
	err = -EIO;
	if (iegbe_check_phy_reset_block(hw))
        DPRINTK(PROBE, INFO, "PHY reset is blocked due to SOL/IDER session.\n");

	if (hw->mac_type >= iegbe_82543) {
        netdev->features = NETIF_F_SG |
                   NETIF_F_HW_CSUM;
    }

	if ((hw->mac_type >= iegbe_82544) &&
	   (hw->mac_type != iegbe_82547))
        netdev->features |= NETIF_F_TSO;

	if (hw->mac_type > iegbe_82547_rev_2)
		netdev->features |= NETIF_F_TSO6;
	if (pci_using_dac)
        netdev->features |= NETIF_F_HIGHDMA;

    netdev->features |= NETIF_F_LLTX;

	adapter->en_mng_pt = iegbe_enable_mng_pass_thru(hw);

	/* initialize eeprom parameters */

	if (iegbe_init_eeprom_params(hw)) {
		E1000_ERR("EEPROM initialization failed\n");
		goto err_eeprom;
	}

	/* before reading the EEPROM, reset the controller to
	 * put the device in a known good starting state */

	iegbe_reset_hw(hw);

    /* make sure the EEPROM is good */
	if (iegbe_validate_eeprom_checksum(hw) < 0) {
        DPRINTK(PROBE, ERR, "The EEPROM Checksum Is Not Valid\n");
		iegbe_dump_eeprom(adapter);
		/*
		 * set MAC address to all zeroes to invalidate and temporary
		 * disable this device for the user. This blocks regular
		 * traffic while still permitting ethtool ioctls from reaching
		 * the hardware as well as allowing the user to run the
		 * interface after manually setting a hw addr using
		 * `ip set address`
		 */
		memset(hw->mac_addr, 0, netdev->addr_len);
	} else {
		/* copy the MAC address out of the EEPROM */
		if (iegbe_read_mac_addr(hw))
			DPRINTK(PROBE, ERR, "EEPROM Read Error\n");
	}
	/* don't block initalization here due to bad MAC address */
	memcpy(netdev->dev_addr, hw->mac_addr, netdev->addr_len);
	memcpy(netdev->perm_addr, hw->mac_addr, netdev->addr_len);

	if (!is_valid_ether_addr(netdev->perm_addr))
		DPRINTK(PROBE, ERR, "Invalid MAC Address\n");

	iegbe_get_bus_info(hw);

	init_timer(&adapter->tx_fifo_stall_timer);
	adapter->tx_fifo_stall_timer.function = &iegbe_82547_tx_fifo_stall;
	adapter->tx_fifo_stall_timer.data = (unsigned long)adapter;

	init_timer(&adapter->watchdog_timer);
	adapter->watchdog_timer.function = &iegbe_watchdog;
	adapter->watchdog_timer.data = (unsigned long) adapter;

	init_timer(&adapter->phy_info_timer);
	adapter->phy_info_timer.function = &iegbe_update_phy_info;
	adapter->phy_info_timer.data = (unsigned long)adapter;

	INIT_WORK(&adapter->reset_task, iegbe_reset_task);

    iegbe_check_options(adapter);

    /* Initial Wake on LAN setting
     * If APM wake is enabled in the EEPROM,
     * enable the ACPI Magic Packet filter
     */

    switch(adapter->hw.mac_type) {
    case iegbe_82542_rev2_0:
    case iegbe_82542_rev2_1:
    case iegbe_82543:
        break;
    case iegbe_82544:
        iegbe_read_eeprom(&adapter->hw,
            EEPROM_INIT_CONTROL2_REG, 1, &eeprom_data);
        eeprom_apme_mask = E1000_EEPROM_82544_APM;
        break;
    case iegbe_icp_xxxx:
        iegbe_read_eeprom(&adapter->hw,
            EEPROM_INIT_CONTROL3_ICP_xxxx(adapter->bd_number),
            1, &eeprom_data);
        eeprom_apme_mask = EEPROM_CTRL3_APME_ICP_xxxx;
        break;
    case iegbe_82546:
    case iegbe_82546_rev_3:
        if((E1000_READ_REG(&adapter->hw, STATUS) & E1000_STATUS_FUNC_1)
           && (adapter->hw.media_type == iegbe_media_type_copper)) {
            iegbe_read_eeprom(&adapter->hw,
                EEPROM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);
            break;
        }
        /* Fall Through */
    default:
        iegbe_read_eeprom(&adapter->hw,
            EEPROM_INIT_CONTROL3_PORT_A, 1, &eeprom_data);
        break;
    }
    if(eeprom_data & eeprom_apme_mask) {
        adapter->wol |= E1000_WUFC_MAG;
    }

    /* The ICP_xxxx device has multiple, duplicate interrupt
     * registers, so disable all but the first one
     */
    if(adapter->hw.mac_type == iegbe_icp_xxxx) {
        int offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_ST)
                     + PCI_ST_SMIA_OFFSET;
        pci_write_config_dword(adapter->pdev, offset, 0x00000006);
        E1000_WRITE_REG(&adapter->hw, IMC1, ~0UL);
        E1000_WRITE_REG(&adapter->hw, IMC2, ~0UL);
    }

	iegbe_reset(adapter);
	netif_carrier_off(netdev);
	netif_stop_queue(netdev);
    strcpy(netdev->name, "eth%d");
	err = register_netdev(netdev);
	if (err)
        goto err_register;

    DPRINTK(PROBE, INFO, "Intel(R) PRO/1000 Network Connection\n");

    cards_found++;
    return 0;

err_register:
err_eeprom:
	if (!iegbe_check_phy_reset_block(hw))
		iegbe_phy_hw_reset(hw);
	if (hw->flash_address)
		iounmap(hw->flash_address);
	kfree(adapter->tx_ring);
	kfree(adapter->rx_ring);
err_sw_init:
	iounmap(hw->hw_addr);
err_ioremap:
    free_netdev(netdev);
err_alloc_etherdev:
	pci_release_selected_regions(pdev, bars);
err_pci_reg:
err_dma:
	pci_disable_device(pdev);
    return err;
}

/**
 * iegbe_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * iegbe_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/

static void 
iegbe_remove(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    uint32_t manc;

    if(adapter->hw.mac_type >= iegbe_82540
       && adapter->hw.mac_type != iegbe_icp_xxxx
       && adapter->hw.media_type == iegbe_media_type_copper) {
        manc = E1000_READ_REG(&adapter->hw, MANC);
        if(manc & E1000_MANC_SMBUS_EN) {
            manc |= E1000_MANC_ARP_EN;
            E1000_WRITE_REG(&adapter->hw, MANC, manc);
        }
    }

    unregister_netdev(netdev);
    if(!iegbe_check_phy_reset_block(&adapter->hw)) {
        iegbe_phy_hw_reset(&adapter->hw);
    }
    kfree(adapter->tx_ring);
    kfree(adapter->rx_ring);

    iounmap(adapter->hw.hw_addr);
    pci_release_regions(pdev);

    free_netdev(netdev);
}

/**
 * iegbe_sw_init - Initialize general software structures (struct iegbe_adapter)
 * @adapter: board private structure to initialize
 *
 * iegbe_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/

static int 
iegbe_sw_init(struct iegbe_adapter *adapter)
{
    struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;

    /* PCI config space info */

    hw->vendor_id = pdev->vendor;
    hw->device_id = pdev->device;
    hw->subsystem_vendor_id = pdev->subsystem_vendor;
    hw->subsystem_id = pdev->subsystem_device;

    pci_read_config_byte(pdev, PCI_REVISION_ID, &hw->revision_id);

    pci_read_config_word(pdev, PCI_COMMAND, &hw->pci_cmd_word);

    adapter->rx_buffer_len = E1000_RXBUFFER_2048;
    adapter->rx_ps_bsize0 = E1000_RXBUFFER_256;
    hw->max_frame_size = netdev->mtu +
                 ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;
    hw->min_frame_size = MINIMUM_ETHERNET_FRAME_SIZE;

    /* identify the MAC */

	if (iegbe_set_mac_type(hw)) {
		DPRINTK(PROBE, ERR, "Unknown MAC Type\n");
		return -EIO;
	}

    iegbe_set_media_type(hw);

    hw->wait_autoneg_complete = FALSE;
    hw->tbi_compatibility_en = TRUE;
    hw->adaptive_ifs = TRUE;

    /* Copper options */

    if(hw->media_type == iegbe_media_type_copper
        || (hw->media_type == iegbe_media_type_oem
            && iegbe_oem_phy_is_copper(&adapter->hw))) {
        hw->mdix = AUTO_ALL_MODES;
        hw->disable_polarity_correction = FALSE;
        hw->master_slave = E1000_MASTER_SLAVE;
    }

	adapter->num_tx_queues = 0x1;
	adapter->num_rx_queues = 0x1;

	if (iegbe_alloc_queues(adapter)) {
		DPRINTK(PROBE, ERR, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	spin_lock_init(&adapter->tx_queue_lock);

        /*
     * for ICP_XXXX style controllers, it is necessary to keep
     * track of the last known state of the link to determine if
     * the link experienced a change in state when iegbe_watchdog
     * fires
     */
    adapter->hw.icp_xxxx_is_link_up = FALSE;

    spin_lock_init(&adapter->stats_lock);

	set_bit(__E1000_DOWN, &adapter->flags);
    return 0x0;
}

/**
 * iegbe_alloc_queues - Allocate memory for all rings
 * @adapter: board private structure to initialize
 *
 * We allocate one ring per queue at run-time since we don't know the
 * number of queues at compile-time.
 **/

static int 
iegbe_alloc_queues(struct iegbe_adapter *adapter)
{


	adapter->tx_ring = kcalloc(adapter->num_tx_queues,
	                           sizeof(struct iegbe_tx_ring), GFP_KERNEL);
	if (!adapter->tx_ring)
		return -ENOMEM;

	adapter->rx_ring = kcalloc(adapter->num_rx_queues,
	                           sizeof(struct iegbe_rx_ring), GFP_KERNEL);
	if (!adapter->rx_ring) {
		kfree(adapter->tx_ring);
		return -ENOMEM;
	}

    return E1000_SUCCESS;
}

/**
 * iegbe_open - Called when a network interface is made active
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
iegbe_open(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
    int err;


    /* allocate receive descriptors */
	if (test_bit(__E1000_TESTING, &adapter->flags))
		return -EBUSY;

    /* allocate transmit descriptors */
	err = iegbe_setup_all_tx_resources(adapter);
	if (err)
        goto err_setup_tx;

	err = iegbe_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	iegbe_power_up_phy(adapter);
    adapter->mng_vlan_id = E1000_MNG_VLAN_NONE;
	if ((hw->mng_cookie.status &
              E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT)) {
        iegbe_update_mng_vlan(adapter);
    }

	/* before we allocate an interrupt, we must be ready to handle it.
	 * Setting DEBUG_SHIRQ in the kernel makes it fire an interrupt
	 * as soon as we call pci_request_irq, so we have to setup our
	 * clean_rx handler before we do so.  */
	iegbe_configure(adapter);
	err = iegbe_request_irq(adapter);
	if (err)
		goto err_req_irq;

	/* From here on the code is the same as iegbe_up() */
	clear_bit(__E1000_DOWN, &adapter->flags);

	napi_enable(&adapter->napi);

	iegbe_irq_enable(adapter);

	netif_start_queue(netdev);

	/* fire a link status change interrupt to start the watchdog */

    return E1000_SUCCESS;

err_req_irq:
    iegbe_power_down_phy(adapter);
    iegbe_free_all_rx_resources(adapter);
err_setup_rx:
    iegbe_free_all_tx_resources(adapter);
err_setup_tx:
    iegbe_reset(adapter);

    return err;
}

/**
 * iegbe_close - Disables a network interface
 * @netdev: network interface device structure
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the drivers control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 **/

static int iegbe_close(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;

	WARN_ON(test_bit(__E1000_RESETTING, &adapter->flags));
    iegbe_down(adapter);
	iegbe_power_down_phy(adapter);
	iegbe_free_irq(adapter);

    iegbe_free_all_tx_resources(adapter);
    iegbe_free_all_rx_resources(adapter);

	if ((hw->mng_cookie.status &
			  E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) &&
	     !test_bit(adapter->mng_vlan_id, adapter->active_vlans)) {
		iegbe_vlan_rx_kill_vid(netdev, htons(ETH_P_8021Q), adapter->mng_vlan_id);
	}
	return 0;
}

/**
 * iegbe_check_64k_bound - check that memory doesn't cross 64kB boundary
 * @adapter: address of board private structure
 * @start: address of beginning of memory
 * @len: length of memory
 **/
static inline boolean_t
iegbe_check_64k_bound(struct iegbe_adapter *adapter,
              void *start, unsigned long len)
{
    unsigned long begin = (unsigned long) start;
    unsigned long end = begin + len;

    /* First rev 82545 and 82546 need to not allow any memory
     * write location to cross 64k boundary due to errata 23 */
    if(adapter->hw.mac_type == iegbe_82545 ||
        adapter->hw.mac_type == iegbe_82546) {
        return ((begin ^ (end - 1)) >> 0x10) != 0x0 ? FALSE : TRUE;
    }

    return TRUE;
}

/**
 * iegbe_setup_tx_resources - allocate Tx resources (Descriptors)
 * @adapter: board private structure
 * @txdr:    tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 **/

static int iegbe_setup_tx_resources(struct iegbe_adapter *adapter,
                         struct iegbe_tx_ring *txdr)
{
    struct pci_dev *pdev = adapter->pdev;
    int size;

    size = sizeof(struct iegbe_buffer) * txdr->count;
    txdr->buffer_info = vmalloc(size);
    if (!txdr->buffer_info) {
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the transmit descriptor ring\n");
        return -ENOMEM;
    }
	memset(txdr->buffer_info, 0, size);

    /* round up to nearest 4K */

    txdr->size = txdr->count * sizeof(struct iegbe_tx_desc);
	txdr->size = ALIGN(txdr->size, 4096);

    txdr->desc = pci_alloc_consistent(pdev, txdr->size, &txdr->dma);
    if (!txdr->desc) {
setup_tx_desc_die:
        vfree(txdr->buffer_info);
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the transmit descriptor ring\n");
        return -ENOMEM;
    }

    /* Fix for errata 23, can't cross 64kB boundary */
    if (!iegbe_check_64k_bound(adapter, txdr->desc, txdr->size)) {
        void *olddesc = txdr->desc;
        dma_addr_t olddma = txdr->dma;
        DPRINTK(TX_ERR, ERR, "txdr align check failed: %u bytes "
                     "at %p\n", txdr->size, txdr->desc);
        /* Try again, without freeing the previous */
        txdr->desc = pci_alloc_consistent(pdev, txdr->size, &txdr->dma);
        /* Failed allocation, critical failure */
        if (!txdr->desc) {
            pci_free_consistent(pdev, txdr->size, olddesc, olddma);
            goto setup_tx_desc_die;
        }

        if (!iegbe_check_64k_bound(adapter, txdr->desc, txdr->size)) {
            /* give up */
            pci_free_consistent(pdev, txdr->size, txdr->desc,
                        txdr->dma);
            pci_free_consistent(pdev, txdr->size, olddesc, olddma);
            DPRINTK(PROBE, ERR,
                "Unable to allocate aligned memory "
                "for the transmit descriptor ring\n");
            vfree(txdr->buffer_info);
            return -ENOMEM;
        } else {
            /* Free old allocation, new allocation was successful */
            pci_free_consistent(pdev, txdr->size, olddesc, olddma);
        }
    }
	memset(txdr->desc, 0, txdr->size);

	txdr->next_to_use = 0;
	txdr->next_to_clean = 0;
    spin_lock_init(&txdr->tx_lock);

	return 0;
}

/**
 * iegbe_setup_all_tx_resources - wrapper to allocate Tx resources
 *                   (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 **/

int iegbe_setup_all_tx_resources(struct iegbe_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		err = iegbe_setup_tx_resources(adapter, &adapter->tx_ring[i]);
		if (err) {
			DPRINTK(PROBE, ERR,
				"Allocation for Tx Queue %u failed\n", i);
			for (i-- ; i >= 0; i--)
				iegbe_free_tx_resources(adapter,
							&adapter->tx_ring[i]);
			break;
		}
	}

    return err;
}

/**
 * iegbe_configure_tx - Configure 8254x Transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/

static void
iegbe_configure_tx(struct iegbe_adapter *adapter)
{
	uint64_t tdba;
	struct iegbe_hw *hw = &adapter->hw;
	uint32_t tdlen, tctl, tipg, tarc;

	/* Setup the HW Tx Head and Tail descriptor pointers */

	switch (adapter->num_tx_queues) {
    case 0x2:
        tdba = adapter->tx_ring[0x1].dma;
        tdlen = adapter->tx_ring[0x1].count *
            sizeof(struct iegbe_tx_desc);
        E1000_WRITE_REG(hw, TDBAL1, (tdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, TDBAH1, (tdba >> 0x20));
        E1000_WRITE_REG(hw, TDLEN1, tdlen);
        E1000_WRITE_REG(hw, TDH1, 0x0);
        E1000_WRITE_REG(hw, TDT1, 0x0);
        adapter->tx_ring[0x1].tdh = E1000_TDH1;
        adapter->tx_ring[0x1].tdt = E1000_TDT1;
        /* Fall Through */
    case 0x1:
    default:
        tdba = adapter->tx_ring[0x0].dma;
        tdlen = adapter->tx_ring[0x0].count *
            sizeof(struct iegbe_tx_desc);
        E1000_WRITE_REG(hw, TDBAL, (tdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, TDBAH, (tdba >> 0x20));
        E1000_WRITE_REG(hw, TDLEN, tdlen);
        E1000_WRITE_REG(hw, TDH, 0x0);
        E1000_WRITE_REG(hw, TDT, 0x0);
        adapter->tx_ring[0x0].tdh = E1000_TDH;
        adapter->tx_ring[0x0].tdt = E1000_TDT;
        break;
    }

    /* Set the default values for the Tx Inter Packet Gap timer */

    switch (hw->mac_type) {
    case iegbe_82542_rev2_0:
    case iegbe_82542_rev2_1:
        tipg = DEFAULT_82542_TIPG_IPGT;
        tipg |= DEFAULT_82542_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
        tipg |= DEFAULT_82542_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
        break;
    default:
        switch(hw->media_type) {
        case iegbe_media_type_fiber:
        case iegbe_media_type_internal_serdes:
            tipg = DEFAULT_82543_TIPG_IPGT_FIBER;
            break;
        case iegbe_media_type_copper:
            tipg = DEFAULT_82543_TIPG_IPGT_COPPER;
            break;
        case iegbe_media_type_oem:
        default:
            tipg =  (0xFFFFFFFFUL >> (sizeof(tipg)*0x8 -
                E1000_TIPG_IPGR1_SHIFT))
                & iegbe_oem_get_tipg(&adapter->hw);
            break;
        }
        tipg |= DEFAULT_82543_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
        tipg |= DEFAULT_82543_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
    }
    E1000_WRITE_REG(hw, TIPG, tipg);

    /* Set the Tx Interrupt Delay register */

    E1000_WRITE_REG(hw, TIDV, adapter->tx_int_delay);
    if (hw->mac_type >= iegbe_82540) {
        E1000_WRITE_REG(hw, TADV, adapter->tx_abs_int_delay);
    }
    /* Program the Transmit Control Register */

    tctl = E1000_READ_REG(hw, TCTL);

    tctl &= ~E1000_TCTL_CT;
    tctl |= E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_RTLC |
        (E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);

    E1000_WRITE_REG(hw, TCTL, tctl);

    if (hw->mac_type == iegbe_82571 || hw->mac_type == iegbe_82572) {
        tarc = E1000_READ_REG(hw, TARC0);
        tarc |= ((0x1 << 0x19) | (0x1 << 0x15));
        E1000_WRITE_REG(hw, TARC0, tarc);
        tarc = E1000_READ_REG(hw, TARC1);
        tarc |= (0x1 << 0x19);
        if (tctl & E1000_TCTL_MULR) {
            tarc &= ~(0x1 << 0x1c);
        } else {
            tarc |= (0x1 << 0x1c);
        }
        E1000_WRITE_REG(hw, TARC1, tarc);
    }

    iegbe_config_collision_dist(hw);

    /* Setup Transmit Descriptor Settings for eop descriptor */
    adapter->txd_cmd = E1000_TXD_CMD_IDE | E1000_TXD_CMD_EOP |
        E1000_TXD_CMD_IFCS;

    if (hw->mac_type < iegbe_82543) {
        adapter->txd_cmd |= E1000_TXD_CMD_RPS;
    } else {
        adapter->txd_cmd |= E1000_TXD_CMD_RS;
    }
    /* Cache if we're 82544 running in PCI-X because we'll
     * need this to apply a workaround later in the send path. */
    if (hw->mac_type == iegbe_82544 &&
        hw->bus_type == iegbe_bus_type_pcix) {
        adapter->pcix_82544 = 0x1;
     }
}

/**
 * iegbe_setup_rx_resources - allocate Rx resources (Descriptors)
 * @adapter: board private structure
 * @rxdr:    rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/

static int iegbe_setup_rx_resources(struct iegbe_adapter *adapter,
                         struct iegbe_rx_ring *rxdr)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct pci_dev *pdev = adapter->pdev;
    int size, desc_len;

    size = sizeof(struct iegbe_buffer) * rxdr->count;
    rxdr->buffer_info = vmalloc(size);
    if (!rxdr->buffer_info) {
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the receive descriptor ring\n");
        return -ENOMEM;
    }
	memset(rxdr->buffer_info, 0, size);

	rxdr->ps_page = kcalloc(rxdr->count, sizeof(struct iegbe_ps_page),
	                        GFP_KERNEL);
    if (!rxdr->ps_page) {
        vfree(rxdr->buffer_info);
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the receive descriptor ring\n");
        return -ENOMEM;
    }

	rxdr->ps_page_dma = kcalloc(rxdr->count,
	                            sizeof(struct iegbe_ps_page_dma),
	                            GFP_KERNEL);
    if (!rxdr->ps_page_dma) {
        vfree(rxdr->buffer_info);
        kfree(rxdr->ps_page);
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the receive descriptor ring\n");
        return -ENOMEM;
    }

	if (hw->mac_type <= iegbe_82547_rev_2)
        desc_len = sizeof(struct iegbe_rx_desc);
	else
        desc_len = sizeof(union iegbe_rx_desc_packet_split);

    /* Round up to nearest 4K */

    rxdr->size = rxdr->count * desc_len;
	rxdr->size = ALIGN(rxdr->size, 4096);

    rxdr->desc = pci_alloc_consistent(pdev, rxdr->size, &rxdr->dma);

    if (!rxdr->desc) {
        DPRINTK(PROBE, ERR,
        "Unable to allocate memory for the receive descriptor ring\n");
setup_rx_desc_die:
        vfree(rxdr->buffer_info);
        kfree(rxdr->ps_page);
        kfree(rxdr->ps_page_dma);
        return -ENOMEM;
    }

    /* Fix for errata 23, can't cross 64kB boundary */
    if (!iegbe_check_64k_bound(adapter, rxdr->desc, rxdr->size)) {
        void *olddesc = rxdr->desc;
        dma_addr_t olddma = rxdr->dma;
        DPRINTK(RX_ERR, ERR, "rxdr align check failed: %u bytes "
                     "at %p\n", rxdr->size, rxdr->desc);
        /* Try again, without freeing the previous */
        rxdr->desc = pci_alloc_consistent(pdev, rxdr->size, &rxdr->dma);
        /* Failed allocation, critical failure */
        if (!rxdr->desc) {
            pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
            DPRINTK(PROBE, ERR,
                "Unable to allocate memory "
                "for the receive descriptor ring\n");
            goto setup_rx_desc_die;
        }

        if (!iegbe_check_64k_bound(adapter, rxdr->desc, rxdr->size)) {
            /* give up */
            pci_free_consistent(pdev, rxdr->size, rxdr->desc,
                        rxdr->dma);
            pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
            DPRINTK(PROBE, ERR,
                "Unable to allocate aligned memory "
                "for the receive descriptor ring\n");
            goto setup_rx_desc_die;
        } else {
            /* Free old allocation, new allocation was successful */
            pci_free_consistent(pdev, rxdr->size, olddesc, olddma);
        }
    }
	memset(rxdr->desc, 0, rxdr->size);

	rxdr->next_to_clean = 0;
	rxdr->next_to_use = 0;

	return 0;
}

/**
 * iegbe_setup_all_rx_resources - wrapper to allocate Rx resources
 *                   (Descriptors) for all queues
 * @adapter: board private structure
 *
 * If this function returns with an error, then it's possible one or
 * more of the rings is populated (while the rest are not).  It is the
 * callers duty to clean those orphaned rings.
 *
 * Return 0 on success, negative on failure
 **/

int iegbe_setup_all_rx_resources(struct iegbe_adapter *adapter)
{
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = iegbe_setup_rx_resources(adapter, &adapter->rx_ring[i]);
		if (err) {
			DPRINTK(PROBE, ERR,
				"Allocation for Rx Queue %u failed\n", i);
			for (i-- ; i >= 0; i--)
				iegbe_free_rx_resources(adapter,
							&adapter->rx_ring[i]);
			break;
		}
	}

    return err;
}

/**
 * iegbe_setup_rctl - configure the receive control registers
 * @adapter: Board private structure
 **/
#define PAGE_USE_COUNT(S) (((S) >> PAGE_SHIFT) + \
			(((S) & (PAGE_SIZE - 1)) ? 1 : 0))
static void iegbe_setup_rctl(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	u32 rctl, rfctl;
	u32 psrctl = 0;
#ifndef CONFIG_E1000_DISABLE_PACKET_SPLIT
	u32 pages = 0;
#endif

    rctl = E1000_READ_REG(&adapter->hw, RCTL);

	rctl &= ~(3 << E1000_RCTL_MO_SHIFT);

    rctl |= E1000_RCTL_EN | E1000_RCTL_BAM |
        E1000_RCTL_LBM_NO | E1000_RCTL_RDMTS_HALF |
		(hw->mc_filter_type << E1000_RCTL_MO_SHIFT);

	if (hw->tbi_compatibility_on == 1)
        rctl |= E1000_RCTL_SBP;
	else
        rctl &= ~E1000_RCTL_SBP;

	if (adapter->netdev->mtu <= ETH_DATA_LEN)
        rctl &= ~E1000_RCTL_LPE;
	else
        rctl |= E1000_RCTL_LPE;

    /* Setup buffer sizes */
        /* We can now specify buffers in 1K increments.
         * BSIZE and BSEX are ignored in this case. */
        rctl &= ~E1000_RCTL_SZ_4096;
        rctl |= E1000_RCTL_BSEX;
        switch (adapter->rx_buffer_len) {
		case E1000_RXBUFFER_256:
			rctl |= E1000_RCTL_SZ_256;
			rctl &= ~E1000_RCTL_BSEX;
			break;
        case E1000_RXBUFFER_2048:
        default:
            rctl |= E1000_RCTL_SZ_2048;
            rctl &= ~E1000_RCTL_BSEX;
            break;
        case E1000_RXBUFFER_4096:
            rctl |= E1000_RCTL_SZ_4096;
            break;
        case E1000_RXBUFFER_8192:
            rctl |= E1000_RCTL_SZ_8192;
            break;
        case E1000_RXBUFFER_16384:
            rctl |= E1000_RCTL_SZ_16384;
            break;
        }

#ifndef CONFIG_E1000_DISABLE_PACKET_SPLIT
    /* 82571 and greater support packet-split where the protocol
     * header is placed in skb->data and the packet data is
     * placed in pages hanging off of skb_shinfo(skb)->nr_frags.
     * In the case of a non-split, skb->data is linearly filled,
     * followed by the page buffers.  Therefore, skb->data is
     * sized to hold the largest protocol header.
     */
    pages = PAGE_USE_COUNT(adapter->netdev->mtu);
	if ((hw->mac_type >= iegbe_82571) && (pages <= 3) &&
	    PAGE_SIZE <= 16384 && (rctl & E1000_RCTL_LPE))
        adapter->rx_ps_pages = pages;
	else
		adapter->rx_ps_pages = 0;
#endif
    if (adapter->rx_ps_pages) {
        /* Configure extra packet-split registers */
        rfctl = E1000_READ_REG(&adapter->hw, RFCTL);
        rfctl |= E1000_RFCTL_EXTEN;
        /* disable IPv6 packet split support */
		rfctl |= (E1000_RFCTL_IPV6_EX_DIS |
		          E1000_RFCTL_NEW_IPV6_EXT_DIS);

		rctl |= E1000_RCTL_DTYP_PS;

        psrctl |= adapter->rx_ps_bsize0 >>
            E1000_PSRCTL_BSIZE0_SHIFT;

        switch (adapter->rx_ps_pages) {
		case 3:
            psrctl |= PAGE_SIZE <<
                E1000_PSRCTL_BSIZE3_SHIFT;
		case 2:
            psrctl |= PAGE_SIZE <<
                E1000_PSRCTL_BSIZE2_SHIFT;
		case 1:
            psrctl |= PAGE_SIZE >>
                E1000_PSRCTL_BSIZE1_SHIFT;
            break;
        }

        E1000_WRITE_REG(&adapter->hw, PSRCTL, psrctl);
    }

    E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
}

/**
 * iegbe_configure_rx - Configure 8254x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/

static void iegbe_configure_rx(struct iegbe_adapter *adapter)
{
	u64 rdba;
    struct iegbe_hw *hw = &adapter->hw;
	u32 rdlen, rctl, rxcsum, ctrl_ext;

    if (adapter->rx_ps_pages) {
		rdlen = adapter->rx_ring[0].count *
            sizeof(union iegbe_rx_desc_packet_split);
        adapter->clean_rx = iegbe_clean_rx_irq_ps;
        adapter->alloc_rx_buf = iegbe_alloc_rx_buffers_ps;
    } else {
		rdlen = adapter->rx_ring[0].count *
            sizeof(struct iegbe_rx_desc);
        adapter->clean_rx = iegbe_clean_rx_irq;
        adapter->alloc_rx_buf = iegbe_alloc_rx_buffers;
    }

    /* disable receives while setting up the descriptors */
    rctl = E1000_READ_REG(hw, RCTL);
    E1000_WRITE_REG(hw, RCTL, rctl & ~E1000_RCTL_EN);

    /* set the Receive Delay Timer Register */
    E1000_WRITE_REG(hw, RDTR, adapter->rx_int_delay);

    if (hw->mac_type >= iegbe_82540) {
        E1000_WRITE_REG(hw, RADV, adapter->rx_abs_int_delay);
		if (adapter->itr_setting != 0)
			E1000_WRITE_REG(&adapter->hw, ITR, 1000000000 / (adapter->itr * 256));
        }

    if (hw->mac_type >= iegbe_82571) {
        /* Reset delay timers after every interrupt */
        ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
        ctrl_ext |= E1000_CTRL_EXT_CANC;
        E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
        E1000_WRITE_FLUSH(hw);
    }

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring */
	switch (adapter->num_rx_queues) {
	case 1:
    default:
		rdba = adapter->rx_ring[0].dma;
        E1000_WRITE_REG(hw, RDBAL, (rdba & 0x00000000ffffffffULL));
        E1000_WRITE_REG(hw, RDBAH, (rdba >> 0x20));
        E1000_WRITE_REG(hw, RDLEN, rdlen);
		adapter->rx_ring[0].rdh = ((hw->mac_type >= iegbe_82543) ? E1000_RDH : E1000_82542_RDH);
		adapter->rx_ring[0].rdt = ((hw->mac_type >= iegbe_82543) ? E1000_RDT : E1000_82542_RDT);
        break;
    }


    /* Enable 82543 Receive Checksum Offload for TCP and UDP */
    if (hw->mac_type >= iegbe_82543) {
        rxcsum = E1000_READ_REG(hw, RXCSUM);
        if(adapter->rx_csum == TRUE) {
            rxcsum |= E1000_RXCSUM_TUOFL;

            /* Enable 82571 IPv4 payload checksum for UDP fragments
             * Must be used in conjunction with packet-split. */
            if ((hw->mac_type >= iegbe_82571) &&
               (adapter->rx_ps_pages)) {
                rxcsum |= E1000_RXCSUM_IPPCSE;
            }
        } else {
            rxcsum &= ~E1000_RXCSUM_TUOFL;
            /* don't need to clear IPPCSE as it defaults to 0 */
        }
        E1000_WRITE_REG(hw, RXCSUM, rxcsum);
    }

	/* enable early receives on 82573, only takes effect if using > 2048
	 * byte total frame size.  for example only for jumbo frames */
#define E1000_ERT_2048 0x100
	if (hw->mac_type == iegbe_82573)
		E1000_WRITE_REG(&adapter->hw, ERT, E1000_ERT_2048);

	/* Enable Receives */
    E1000_WRITE_REG(hw, RCTL, rctl);
}

/**
 * iegbe_free_tx_resources - Free Tx Resources per Queue
 * @adapter: board private structure
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/

static void iegbe_free_tx_resources(struct iegbe_adapter *adapter,
                        struct iegbe_tx_ring *tx_ring)
{
    struct pci_dev *pdev = adapter->pdev;

    iegbe_clean_tx_ring(adapter, tx_ring);

    vfree(tx_ring->buffer_info);
    tx_ring->buffer_info = NULL;

    pci_free_consistent(pdev, tx_ring->size, tx_ring->desc, tx_ring->dma);

    tx_ring->desc = NULL;
}

/**
 * iegbe_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 **/

void
iegbe_free_all_tx_resources(struct iegbe_adapter *adapter)
{
    int i;

	for (i = 0x0; i < adapter->num_tx_queues; i++)
		iegbe_free_tx_resources(adapter, &adapter->tx_ring[i]);
}

static inline void
iegbe_unmap_and_free_tx_resource(struct iegbe_adapter *adapter,
            struct iegbe_buffer *buffer_info)
{
    if(buffer_info->dma) {
        pci_unmap_page(adapter->pdev,
                buffer_info->dma,
                buffer_info->length,
                PCI_DMA_TODEVICE);
        buffer_info->dma = 0x0;
    }
    if(buffer_info->skb) {
        dev_kfree_skb_any(buffer_info->skb);
        buffer_info->skb = NULL;
    }
}


/**
 * iegbe_clean_tx_ring - Free Tx Buffers
 * @adapter: board private structure
 * @tx_ring: ring to be cleaned
 **/

static void iegbe_clean_tx_ring(struct iegbe_adapter *adapter,
                    struct iegbe_tx_ring *tx_ring)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct iegbe_buffer *buffer_info;
    unsigned long size;
    unsigned int i;

    /* Free all the Tx ring sk_buffs */

	for (i = 0; i < tx_ring->count; i++) {
        buffer_info = &tx_ring->buffer_info[i];
        iegbe_unmap_and_free_tx_resource(adapter, buffer_info);
    }

    size = sizeof(struct iegbe_buffer) * tx_ring->count;
	memset(tx_ring->buffer_info, 0, size);

    /* Zero out the descriptor ring */

	memset(tx_ring->desc, 0, tx_ring->size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
	tx_ring->last_tx_tso = 0;

	writel(0, hw->hw_addr + tx_ring->tdh);
	writel(0, hw->hw_addr + tx_ring->tdt);
}

/**
 * iegbe_clean_all_tx_rings - Free Tx Buffers for all queues
 * @adapter: board private structure
 **/

static void iegbe_clean_all_tx_rings(struct iegbe_adapter *adapter)
{
    int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		iegbe_clean_tx_ring(adapter, &adapter->tx_ring[i]);
}

/**
 * iegbe_free_rx_resources - Free Rx Resources
 * @adapter: board private structure
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/

static void iegbe_free_rx_resources(struct iegbe_adapter *adapter,
                        struct iegbe_rx_ring *rx_ring)
{
    struct pci_dev *pdev = adapter->pdev;

    iegbe_clean_rx_ring(adapter, rx_ring);

    vfree(rx_ring->buffer_info);
    rx_ring->buffer_info = NULL;
    kfree(rx_ring->ps_page);
    rx_ring->ps_page = NULL;
    kfree(rx_ring->ps_page_dma);
    rx_ring->ps_page_dma = NULL;

    pci_free_consistent(pdev, rx_ring->size, rx_ring->desc, rx_ring->dma);

    rx_ring->desc = NULL;
}

/**
 * iegbe_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 **/

void iegbe_free_all_rx_resources(struct iegbe_adapter *adapter)
{
    int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		iegbe_free_rx_resources(adapter, &adapter->rx_ring[i]);
}

/**
 * iegbe_clean_rx_ring - Free Rx Buffers per Queue
 * @adapter: board private structure
 * @rx_ring: ring to free buffers from
 **/

static void iegbe_clean_rx_ring(struct iegbe_adapter *adapter,
                    struct iegbe_rx_ring *rx_ring)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct iegbe_buffer *buffer_info;
    struct iegbe_ps_page *ps_page;
    struct iegbe_ps_page_dma *ps_page_dma;
    struct pci_dev *pdev = adapter->pdev;
    unsigned long size;
    unsigned int i, j;

    /* Free all the Rx ring sk_buffs */

	for (i = 0; i < rx_ring->count; i++) {
        buffer_info = &rx_ring->buffer_info[i];
        if(buffer_info->skb) {
            pci_unmap_single(pdev,
                     buffer_info->dma,
                     buffer_info->length,
                     PCI_DMA_FROMDEVICE);

            dev_kfree_skb(buffer_info->skb);
            buffer_info->skb = NULL;
		}
		ps_page = &rx_ring->ps_page[i];
		ps_page_dma = &rx_ring->ps_page_dma[i];
		for (j = 0; j < adapter->rx_ps_pages; j++) {
			if (!ps_page->ps_page[j]) break;
			pci_unmap_page(pdev,
                         ps_page_dma->ps_page_dma[j],
                         PAGE_SIZE, PCI_DMA_FROMDEVICE);
			ps_page_dma->ps_page_dma[j] = 0;
                put_page(ps_page->ps_page[j]);
                ps_page->ps_page[j] = NULL;
            }
        }

    size = sizeof(struct iegbe_buffer) * rx_ring->count;
	memset(rx_ring->buffer_info, 0, size);
    size = sizeof(struct iegbe_ps_page) * rx_ring->count;
	memset(rx_ring->ps_page, 0, size);
    size = sizeof(struct iegbe_ps_page_dma) * rx_ring->count;
	memset(rx_ring->ps_page_dma, 0, size);

    /* Zero out the descriptor ring */

	memset(rx_ring->desc, 0, rx_ring->size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	writel(0, hw->hw_addr + rx_ring->rdh);
	writel(0, hw->hw_addr + rx_ring->rdt);
}

/**
 * iegbe_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 **/

static void iegbe_clean_all_rx_rings(struct iegbe_adapter *adapter)
{
    int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		iegbe_clean_rx_ring(adapter, &adapter->rx_ring[i]);
}

/* The 82542 2.0 (revision 2) needs to have the receive unit in reset
 * and memory write and invalidate disabled for certain operations
 */
static void iegbe_enter_82542_rst(struct iegbe_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    uint32_t rctl;

    iegbe_pci_clear_mwi(&adapter->hw);

    rctl = E1000_READ_REG(&adapter->hw, RCTL);
    rctl |= E1000_RCTL_RST;
    E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
    E1000_WRITE_FLUSH(&adapter->hw);
    mdelay(0x5);

    if(netif_running(netdev)) {
        iegbe_clean_all_rx_rings(adapter);
    }
}

static void
iegbe_leave_82542_rst(struct iegbe_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    uint32_t rctl;

    rctl = E1000_READ_REG(&adapter->hw, RCTL);
    rctl &= ~E1000_RCTL_RST;
    E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
    E1000_WRITE_FLUSH(&adapter->hw);
    mdelay(0x5);

    if(adapter->hw.pci_cmd_word & PCI_COMMAND_INVALIDATE) {
        iegbe_pci_set_mwi(&adapter->hw);
    }
	if(netif_running(netdev)) {
		struct iegbe_rx_ring *ring = &adapter->rx_ring[0x0];
		iegbe_configure_rx(adapter);
		adapter->alloc_rx_buf(adapter, ring, E1000_DESC_UNUSED(ring));
	}
}

/**
 * iegbe_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 **/

static int iegbe_set_mac(struct net_device *netdev, void *p)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct sockaddr *addr = p;

    if(!is_valid_ether_addr(addr->sa_data)) {
        return -EADDRNOTAVAIL;
    }
    /* 82542 2.0 needs to be in reset to write receive address registers */

    if(adapter->hw.mac_type == iegbe_82542_rev2_0) {
        iegbe_enter_82542_rst(adapter);
    }
    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
    memcpy(adapter->hw.mac_addr, addr->sa_data, netdev->addr_len);

    iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr, 0x0);

    /* With 82571 controllers, LAA may be overwritten (with the default)
     * due to controller reset from the other port. */
    if (adapter->hw.mac_type == iegbe_82571) {
        /* activate the work around */
        adapter->hw.laa_is_present = 0x1;

        /* Hold a copy of the LAA in RAR[14] This is done so that
         * between the time RAR[0] gets clobbered  and the time it
         * gets fixed (in iegbe_watchdog), the actual LAA is in one
         * of the RARs and no incoming packets directed to this port
         * are dropped. Eventaully the LAA will be in RAR[0] and
         * RAR[14] */
        iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr,
                    E1000_RAR_ENTRIES - 0x1);
    }

    if(adapter->hw.mac_type == iegbe_82542_rev2_0) {
        iegbe_leave_82542_rst(adapter);
    }
    return 0x0;
}

/**
 * iegbe_set_rx_mode - Secondary Unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_rx_mode entry point is called whenever the unicast or multicast
 * address lists or the network interface flags are updated. This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 **/

static void iegbe_set_rx_mode(struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
	struct netdev_hw_addr *ha;
	bool use_uc = false;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
 	struct dev_addr_list *mc_ptr;
#endif
 	u32 hash_value;
	int mta_reg_count = E1000_NUM_MTA_REGISTERS;
	u32 rctl;
	int i, rar_entries = E1000_RAR_ENTRIES;

	/* reserve RAR[14] for LAA over-write work-around */
	if (hw->mac_type == iegbe_82571)
		rar_entries--;

	/* Check for Promiscuous and All Multicast modes */

	rctl = E1000_READ_REG(&adapter->hw, RCTL);

	if (netdev->flags & IFF_PROMISC) {
		rctl |= (E1000_RCTL_UPE | E1000_RCTL_MPE);
		rctl &= ~E1000_RCTL_VFE;
	} else {
		if (netdev->flags & IFF_ALLMULTI) {
			rctl |= E1000_RCTL_MPE;
		} else {
			rctl &= ~E1000_RCTL_MPE;
		}
	}

	if (netdev->uc.count > rar_entries - 1) {
		rctl |= E1000_RCTL_UPE;
	} else if (!(netdev->flags & IFF_PROMISC)) {
		rctl &= ~E1000_RCTL_UPE;
		use_uc = true;
	}

	E1000_WRITE_REG(&adapter->hw, RCTL, rctl);

	/* 82542 2.0 needs to be in reset to write receive address registers */

	if (hw->mac_type == iegbe_82542_rev2_0)
		iegbe_enter_82542_rst(adapter);

	/* load the first 14 addresses into the exact filters 1-14. Unicast
	 * addresses take precedence to avoid disabling unicast filtering
	 * when possible.
	 *
	 * RAR 0 is used for the station MAC adddress
	 * if there are not 14 addresses, go ahead and clear the filters
	 * -- with 82571 controllers only 0-13 entries are filled here
	 */
	i = 1;
	if (use_uc)
		list_for_each_entry(ha, &netdev->uc.list, list) {
			if (i == rar_entries)
				break;
			iegbe_rar_set(hw, ha->addr, i++);
		}

	WARN_ON(i == rar_entries);

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35) 
		int once=0;
		netdev_for_each_mc_addr(ha, netdev) {
		if (i>=rar_entries)
		    {
		/* clear the old settings from the multicast hash table */
		    if (!once)
		    {
		    once=1;
		for (i = 0; i < mta_reg_count; i++) {
		    E1000_WRITE_REG_ARRAY(hw, MTA, i, 0);
		    E1000_WRITE_FLUSH(&adapter->hw);
		    }
		    }
    
			hash_value = iegbe_hash_mc_addr(hw, ha->addr);
			    iegbe_mta_set(hw, hash_value);
		    
		    
		    }else{
		if (ha) {
			iegbe_rar_set(hw, ha->addr, i);
		} else {
			E1000_WRITE_REG_ARRAY(hw, RA, i << 1, 0);
			E1000_WRITE_FLUSH(&adapter->hw);
			E1000_WRITE_REG_ARRAY(hw, RA, (i << 1) + 1, 0);
			E1000_WRITE_FLUSH(&adapter->hw);
		}
		}
		i++;
		}

	#else

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
	mc_ptr = netdev->mc_list;

	for (; i < rar_entries; i++) {
		if (mc_ptr) {
			iegbe_rar_set(hw, mc_ptr->da_addr, i);
			mc_ptr = mc_ptr->next;
		} else {
			E1000_WRITE_REG_ARRAY(hw, RA, i << 1, 0);
			E1000_WRITE_FLUSH(&adapter->hw);
			E1000_WRITE_REG_ARRAY(hw, RA, (i << 1) + 1, 0);
			E1000_WRITE_FLUSH(&adapter->hw);
		}
	}
	/* clear the old settings from the multicast hash table */

	for (i = 0; i < mta_reg_count; i++) {
		E1000_WRITE_REG_ARRAY(hw, MTA, i, 0);
		E1000_WRITE_FLUSH(&adapter->hw);
	}
#endif

	/* load any remaining addresses into the hash table */

	for (; mc_ptr; mc_ptr = mc_ptr->next) {
		hash_value = iegbe_hash_mc_addr(hw, mc_ptr->da_addr);
		iegbe_mta_set(hw, hash_value);
	}
	#endif

	if (hw->mac_type == iegbe_82542_rev2_0)
		iegbe_leave_82542_rst(adapter);
}

/* Need to wait a few seconds after link up to get diagnostic information from
 * the phy */

static void iegbe_update_phy_info(unsigned long data)
{
    struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
	struct iegbe_hw *hw = &adapter->hw;
	iegbe_phy_get_info(hw, &adapter->phy_info);
}

/**
 * iegbe_82547_tx_fifo_stall - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/

static void iegbe_82547_tx_fifo_stall(unsigned long data)
{
    struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
    struct net_device *netdev = adapter->netdev;
	u32 tctl;

    if(atomic_read(&adapter->tx_fifo_stall)) {
        if((E1000_READ_REG(&adapter->hw, TDT) ==
            E1000_READ_REG(&adapter->hw, TDH)) &&
           (E1000_READ_REG(&adapter->hw, TDFT) ==
            E1000_READ_REG(&adapter->hw, TDFH)) &&
           (E1000_READ_REG(&adapter->hw, TDFTS) ==
            E1000_READ_REG(&adapter->hw, TDFHS))) {
            tctl = E1000_READ_REG(&adapter->hw, TCTL);
            E1000_WRITE_REG(&adapter->hw, TCTL,
                    tctl & ~E1000_TCTL_EN);
            E1000_WRITE_REG(&adapter->hw, TDFT,
                    adapter->tx_head_addr);
            E1000_WRITE_REG(&adapter->hw, TDFH,
                    adapter->tx_head_addr);
            E1000_WRITE_REG(&adapter->hw, TDFTS,
                    adapter->tx_head_addr);
            E1000_WRITE_REG(&adapter->hw, TDFHS,
                    adapter->tx_head_addr);
            E1000_WRITE_REG(&adapter->hw, TCTL, tctl);
            E1000_WRITE_FLUSH(&adapter->hw);

            adapter->tx_fifo_head = 0x0;
            atomic_set(&adapter->tx_fifo_stall, 0x0);
            netif_wake_queue(netdev);
        } else {
            mod_timer(&adapter->tx_fifo_stall_timer, jiffies + 0x1);
        }
    }
}


/**
 * iegbe_watchdog - Timer Call-back
 * @data: pointer to adapter cast into an unsigned long
 **/
static void iegbe_watchdog(unsigned long data)
{
    struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;
	struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
	struct iegbe_tx_ring *txdr = adapter->tx_ring;
	u32 link, tctl;

   /*
    * Test the PHY for link status on icp_xxxx MACs.
    * If the link status is different than the last link status stored
    * in the adapter->hw structure, then set hw->get_link_status = 1
    */
    if(adapter->hw.mac_type == iegbe_icp_xxxx) {
        int isUp = 0x0;
        int32_t ret_val;

        ret_val = iegbe_oem_phy_is_link_up(&adapter->hw, &isUp);
        if(ret_val != E1000_SUCCESS) {
            isUp = 0x0;
        }
        if(isUp != adapter->hw.icp_xxxx_is_link_up) {
            adapter->hw.get_link_status = 0x1;
        }
    }

    iegbe_check_for_link(&adapter->hw);
    if (adapter->hw.mac_type == iegbe_82573) {
        iegbe_enable_tx_pkt_filtering(&adapter->hw);
#ifdef NETIF_F_HW_VLAN_TX
        if (adapter->mng_vlan_id != adapter->hw.mng_cookie.vlan_id) {
            iegbe_update_mng_vlan(adapter);
        }
#endif
    }

    if ((adapter->hw.media_type == iegbe_media_type_internal_serdes) &&
       !(E1000_READ_REG(&adapter->hw, TXCW) & E1000_TXCW_ANE)) {
        link = !adapter->hw.serdes_link_down;
    } else {

        if(adapter->hw.mac_type != iegbe_icp_xxxx) {
            link = E1000_READ_REG(&adapter->hw, STATUS) & E1000_STATUS_LU;
        } else {
            int isUp = 0x0;
            if(iegbe_oem_phy_is_link_up(&adapter->hw, &isUp) != E1000_SUCCESS) {
                isUp = 0x0;
                }
            link = isUp;
        }
    }

    if (link) {
        if (!netif_carrier_ok(netdev)) {
			u32 ctrl;
			bool txb2b = true;
			iegbe_get_speed_and_duplex(hw,
                                       &adapter->link_speed,
                                       &adapter->link_duplex);

			ctrl = E1000_READ_REG(&adapter->hw, CTRL);
			DPRINTK(LINK, INFO, "NIC Link is Up %d Mbps %s, "
			        "Flow Control: %s\n",
                   adapter->link_speed,
                   adapter->link_duplex == FULL_DUPLEX ?
			        "Full Duplex" : "Half Duplex",
			        ((ctrl & E1000_CTRL_TFCE) && (ctrl &
			        E1000_CTRL_RFCE)) ? "RX/TX" : ((ctrl &
			        E1000_CTRL_RFCE) ? "RX" : ((ctrl &
			        E1000_CTRL_TFCE) ? "TX" : "None" )));

			/* tweak tx_queue_len according to speed/duplex
			 * and adjust the timeout factor */
			netdev->tx_queue_len = adapter->tx_queue_len;
			adapter->tx_timeout_factor = 1;
			switch (adapter->link_speed) {
			case SPEED_10:
				txb2b = false;
				netdev->tx_queue_len = 10;
				adapter->tx_timeout_factor = 8;
				break;
			case SPEED_100:
				txb2b = false;
				netdev->tx_queue_len = 100;
				break;
			}
			if ((hw->mac_type == iegbe_82571 ||
			     hw->mac_type == iegbe_82572) &&
			    !txb2b) {
				u32 tarc0;
				tarc0 = E1000_READ_REG(&adapter->hw, TARC0);
				tarc0 &= ~(1 << 21);
				E1000_WRITE_REG(&adapter->hw, TARC0, tarc0);
			}
			/* disable TSO for pcie and 10/100 speeds, to avoid
			 * some hardware issues */
			if (!adapter->tso_force &&
			    hw->bus_type == iegbe_bus_type_pci_express){
				switch (adapter->link_speed) {
				case SPEED_10:
				case SPEED_100:
					DPRINTK(PROBE,INFO,
				        "10/100 speed: disabling TSO\n");
					netdev->features &= ~NETIF_F_TSO;
					netdev->features &= ~NETIF_F_TSO6;
					break;
				case SPEED_1000:
					netdev->features |= NETIF_F_TSO;
					netdev->features |= NETIF_F_TSO6;
					break;
				default:
					break;
				}
			}
			tctl = E1000_READ_REG(&adapter->hw, TCTL);
			tctl |= E1000_TCTL_EN;
			E1000_WRITE_REG(&adapter->hw, TCTL, tctl);
            netif_carrier_on(netdev);
            netif_wake_queue(netdev);
			mod_timer(&adapter->phy_info_timer, round_jiffies(jiffies + 2 * HZ));
			adapter->smartspeed = 0;
		} else {
			if (hw->rx_needs_kicking) {
				u32 rctl = E1000_READ_REG(&adapter->hw, RCTL);
				E1000_WRITE_REG(&adapter->hw, RCTL, rctl | E1000_RCTL_EN);
        }
		}
    } else {
        if (netif_carrier_ok(netdev)) {
			adapter->link_speed = 0;
			adapter->link_duplex = 0;
            DPRINTK(LINK, INFO, "NIC Link is Down\n");
            netif_carrier_off(netdev);
            netif_stop_queue(netdev);
			mod_timer(&adapter->phy_info_timer, round_jiffies(jiffies + 2 * HZ));
        }

        iegbe_smartspeed(adapter);
    }

    iegbe_update_stats(adapter);

	hw->tx_packet_delta = adapter->stats.tpt - adapter->tpt_old;
    adapter->tpt_old = adapter->stats.tpt;
	hw->collision_delta = adapter->stats.colc - adapter->colc_old;
    adapter->colc_old = adapter->stats.colc;

    adapter->gorcl = adapter->stats.gorcl - adapter->gorcl_old;
    adapter->gorcl_old = adapter->stats.gorcl;
    adapter->gotcl = adapter->stats.gotcl - adapter->gotcl_old;
    adapter->gotcl_old = adapter->stats.gotcl;

	iegbe_update_adaptive(hw);

	if (!netif_carrier_ok(netdev)) {
		if (E1000_DESC_UNUSED(txdr) + 1 < txdr->count) {
            /* We've lost link, so the controller stops DMA,
             * but we've got queued Tx work that's never going
             * to get done, so reset controller to flush Tx.
             * (Do the reset outside of interrupt context). */
			adapter->tx_timeout_count++;
			schedule_work(&adapter->reset_task);
        }
    }

    /* Cause software interrupt to ensure rx ring is cleaned */
    E1000_WRITE_REG(&adapter->hw, ICS, E1000_ICS_RXDMT0);

    /* Force detection of hung controller every watchdog period */
    adapter->detect_tx_hung = TRUE;

    /* With 82571 controllers, LAA may be overwritten due to controller
     * reset from the other port. Set the appropriate LAA in RAR[0] */
    if (adapter->hw.mac_type == iegbe_82571 && adapter->hw.laa_is_present) {
        iegbe_rar_set(&adapter->hw, adapter->hw.mac_addr, 0x0);
    }
    /* Reset the timer */
	mod_timer(&adapter->watchdog_timer, round_jiffies(jiffies + 2 * HZ));
}

enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};

/**
 * iegbe_update_itr - update the dynamic ITR value based on statistics
 *      Stores a new ITR value based on packets and byte
 *      counts during the last interrupt.  The advantage of per interrupt
 *      computation is faster updates and more accurate ITR for the current
 *      traffic pattern.  Constants in this function were computed
 *      based on theoretical maximum wire speed and thresholds were set based
 *      on testing data as well as attempting to minimize response time
 *      while increasing bulk throughput.
 *      this functionality is controlled by the InterruptThrottleRate module
 *      parameter (see iegbe_param.c)
 * @adapter: pointer to adapter
 * @itr_setting: current adapter->itr
 * @packets: the number of packets during this measurement interval
 * @bytes: the number of bytes during this measurement interval
 **/
static unsigned int iegbe_update_itr(struct iegbe_adapter *adapter,
				     u16 itr_setting, int packets, int bytes)
{
	unsigned int retval = itr_setting;
	struct iegbe_hw *hw = &adapter->hw;

	if (unlikely(hw->mac_type < iegbe_82540))
		goto update_itr_done;

	if (packets == 0)
		goto update_itr_done;

	switch (itr_setting) {
	case lowest_latency:
		/* jumbo frames get bulk treatment*/
		if (bytes/packets > 8000)
			retval = bulk_latency;
		else if ((packets < 5) && (bytes > 512))
			retval = low_latency;
		break;
	case low_latency:  /* 50 usec aka 20000 ints/s */
		if (bytes > 10000) {
			/* jumbo frames need bulk latency setting */
			if (bytes/packets > 8000)
				retval = bulk_latency;
			else if ((packets < 10) || ((bytes/packets) > 1200))
				retval = bulk_latency;
			else if ((packets > 35))
				retval = lowest_latency;
		} else if (bytes/packets > 2000)
			retval = bulk_latency;
		else if (packets <= 2 && bytes < 512)
			retval = lowest_latency;
		break;
	case bulk_latency: /* 250 usec aka 4000 ints/s */
		if (bytes > 25000) {
			if (packets > 35)
				retval = low_latency;
		} else if (bytes < 6000) {
			retval = low_latency;
		}
		break;
	}

update_itr_done:
	return retval;
}

static void iegbe_set_itr(struct iegbe_adapter *adapter)
{
	struct iegbe_hw *hw = &adapter->hw;
	u16 current_itr;
	u32 new_itr = adapter->itr;

	if (unlikely(hw->mac_type < iegbe_82540))
		return;

	/* for non-gigabit speeds, just fix the interrupt rate at 4000 */
	if (unlikely(adapter->link_speed != SPEED_1000)) {
		current_itr = 0;
		new_itr = 4000;
		goto set_itr_now;
	}

	adapter->tx_itr = iegbe_update_itr(adapter,
	                            adapter->tx_itr,
	                            adapter->total_tx_packets,
	                            adapter->total_tx_bytes);
	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (adapter->itr_setting == 3 && adapter->tx_itr == lowest_latency)
		adapter->tx_itr = low_latency;

	adapter->rx_itr = iegbe_update_itr(adapter,
	                            adapter->rx_itr,
	                            adapter->total_rx_packets,
	                            adapter->total_rx_bytes);
	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (adapter->itr_setting == 3 && adapter->rx_itr == lowest_latency)
		adapter->rx_itr = low_latency;

	current_itr = max(adapter->rx_itr, adapter->tx_itr);

	switch (current_itr) {
	/* counts and packets in update_itr are dependent on these numbers */
	case lowest_latency:
		new_itr = 70000;
		break;
	case low_latency:
		new_itr = 20000; /* aka hwitr = ~200 */
		break;
	case bulk_latency:
		new_itr = 4000;
		break;
	default:
		break;
	}

set_itr_now:
	if (new_itr != adapter->itr) {
		/* this attempts to bias the interrupt rate towards Bulk
		 * by adding intermediate steps when interrupt rate is
		 * increasing */
		new_itr = new_itr > adapter->itr ?
		             min(adapter->itr + (new_itr >> 2), new_itr) :
		             new_itr;
		adapter->itr = new_itr;
		E1000_WRITE_REG(&adapter->hw, ITR, 1000000000 / (new_itr * 256));
	}

	return;
}

#define E1000_TX_FLAGS_CSUM		0x00000001
#define E1000_TX_FLAGS_VLAN		0x00000002
#define E1000_TX_FLAGS_TSO		0x00000004
#define E1000_TX_FLAGS_IPV4		0x00000008
#define E1000_TX_FLAGS_VLAN_MASK	0xffff0000
#define E1000_TX_FLAGS_VLAN_SHIFT	16

static int iegbe_tso(struct iegbe_adapter *adapter,
		     struct iegbe_tx_ring *tx_ring, struct sk_buff *skb)
{
	struct iegbe_context_desc *context_desc;
	struct iegbe_buffer *buffer_info;
	unsigned int i;
	u32 cmd_length = 0;
	u16 ipcse = 0, tucse, mss;
	u8 ipcss, ipcso, tucss, tucso, hdr_len;
	int err;

	if (skb_is_gso(skb)) {
		if (skb_header_cloned(skb)) {
			err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
			if (err)
				return err;
		}

		hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
		mss = skb_shinfo(skb)->gso_size;
		if (skb->protocol == htons(ETH_P_IP)) {
			struct iphdr *iph = ip_hdr(skb);
			iph->tot_len = 0;
			iph->check = 0;
			tcp_hdr(skb)->check = ~csum_tcpudp_magic(iph->saddr,
								 iph->daddr, 0,
								 IPPROTO_TCP,
								 0);
			cmd_length = E1000_TXD_CMD_IP;
			ipcse = skb_transport_offset(skb) - 1;
		} else if (skb->protocol == htons(ETH_P_IPV6)) {
			ipv6_hdr(skb)->payload_len = 0;
			tcp_hdr(skb)->check =
				~csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
						 &ipv6_hdr(skb)->daddr,
						 0, IPPROTO_TCP, 0);
			ipcse = 0;
		}
		ipcss = skb_network_offset(skb);
		ipcso = (void *)&(ip_hdr(skb)->check) - (void *)skb->data;
		tucss = skb_transport_offset(skb);
		tucso = (void *)&(tcp_hdr(skb)->check) - (void *)skb->data;
		tucse = 0;

		cmd_length |= (E1000_TXD_CMD_DEXT | E1000_TXD_CMD_TSE |
			       E1000_TXD_CMD_TCP | (skb->len - (hdr_len)));

		i = tx_ring->next_to_use;
		context_desc = E1000_CONTEXT_DESC(*tx_ring, i);
		buffer_info = &tx_ring->buffer_info[i];

		context_desc->lower_setup.ip_fields.ipcss  = ipcss;
		context_desc->lower_setup.ip_fields.ipcso  = ipcso;
		context_desc->lower_setup.ip_fields.ipcse  = cpu_to_le16(ipcse);
		context_desc->upper_setup.tcp_fields.tucss = tucss;
		context_desc->upper_setup.tcp_fields.tucso = tucso;
		context_desc->upper_setup.tcp_fields.tucse = cpu_to_le16(tucse);
		context_desc->tcp_seg_setup.fields.mss     = cpu_to_le16(mss);
		context_desc->tcp_seg_setup.fields.hdr_len = hdr_len;
		context_desc->cmd_and_length = cpu_to_le32(cmd_length);

		buffer_info->time_stamp = jiffies;
		buffer_info->next_to_watch = i;

		if (++i == tx_ring->count) i = 0;
		tx_ring->next_to_use = i;

		return true;
	}
	return false;
}

static bool iegbe_tx_csum(struct iegbe_adapter *adapter,
			  struct iegbe_tx_ring *tx_ring, struct sk_buff *skb)
{
	struct iegbe_context_desc *context_desc;
	struct iegbe_buffer *buffer_info;
	unsigned int i;
	u8 css;

	if (likely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		css = skb_transport_offset(skb);

	i = tx_ring->next_to_use;
	buffer_info = &tx_ring->buffer_info[i];
	context_desc = E1000_CONTEXT_DESC(*tx_ring, i);

		context_desc->lower_setup.ip_config = 0;
		context_desc->upper_setup.tcp_fields.tucss = css;
		context_desc->upper_setup.tcp_fields.tucso =
			css + skb->csum_offset;
		context_desc->upper_setup.tcp_fields.tucse = 0;
		context_desc->tcp_seg_setup.data = 0;
		context_desc->cmd_and_length = cpu_to_le32(E1000_TXD_CMD_DEXT);

		buffer_info->time_stamp = jiffies;
		buffer_info->next_to_watch = i;

		if (unlikely(++i == tx_ring->count)) i = 0;
		tx_ring->next_to_use = i;

		return true;
	}

	return false;
}

#define E1000_MAX_TXD_PWR    12
#define E1000_MAX_DATA_PER_TXD    (1<<E1000_MAX_TXD_PWR)

static int iegbe_tx_map(struct iegbe_adapter *adapter,
			struct iegbe_tx_ring *tx_ring,
			struct sk_buff *skb, unsigned int first,
			unsigned int max_per_txd, unsigned int nr_frags,
			unsigned int mss)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct iegbe_buffer *buffer_info;
    unsigned int len = skb->len;
	unsigned int offset = 0, size, count = 0, i;
    unsigned int f;
    len -= skb->data_len;

    i = tx_ring->next_to_use;

    while(len) {
        buffer_info = &tx_ring->buffer_info[i];
        size = min(len, max_per_txd);
		/* Workaround for Controller erratum --
		 * descriptor for non-tso packet in a linear SKB that follows a
		 * tso gets written back prematurely before the data is fully
		 * DMA'd to the controller */
		if (!skb->data_len && tx_ring->last_tx_tso &&
		    !skb_is_gso(skb)) {
			tx_ring->last_tx_tso = 0;
			size -= 4;
        }

		/* Workaround for premature desc write-backs
		 * in TSO mode.  Append 4-byte sentinel desc */
		if (unlikely(mss && !nr_frags && size == len && size > 8))
			size -= 4;
        /* work-around for errata 10 and it applies
         * to all controllers in PCI-X mode
         * The fix is to make sure that the first descriptor of a
         * packet is smaller than 2048 - 16 - 16 (or 2016) bytes
         */
		if (unlikely((hw->bus_type == iegbe_bus_type_pcix) &&
		                (size > 2015) && count == 0))
		        size = 2015;

        /* Workaround for potential 82544 hang in PCI-X.  Avoid
         * terminating buffers within evenly-aligned dwords. */
        if(unlikely(adapter->pcix_82544 &&
		   !((unsigned long)(skb->data + offset + size - 1) & 4) &&
		   size > 4))
			size -= 4;

        buffer_info->length = size;
        buffer_info->dma =
            pci_map_single(adapter->pdev,
                skb->data + offset,
                size,
                PCI_DMA_TODEVICE);
        buffer_info->time_stamp = jiffies;
		buffer_info->next_to_watch = i;

        len -= size;
        offset += size;
        count++;
		if (unlikely(++i == tx_ring->count)) i = 0;
    }

	for (f = 0; f < nr_frags; f++) {
        struct skb_frag_struct *frag;

        frag = &skb_shinfo(skb)->frags[f];
        len = frag->size;
        offset = frag->page_offset;

        while(len) {
            buffer_info = &tx_ring->buffer_info[i];
            size = min(len, max_per_txd);
            /* Workaround for premature desc write-backs
             * in TSO mode.  Append 4-byte sentinel desc */
			if (unlikely(mss && f == (nr_frags-1) && size == len && size > 8))
				size -= 4;
            /* Workaround for potential 82544 hang in PCI-X.
             * Avoid terminating buffers within evenly-aligned
             * dwords. */
            if(unlikely(adapter->pcix_82544 &&
			   !((unsigned long)(frag->page.p+offset+size-1) & 4) &&
			   size > 4))
				size -= 4;

            buffer_info->length = size;
            buffer_info->dma =
                pci_map_page(adapter->pdev,
                    frag->page.p,
                    offset,
                    size,
                    PCI_DMA_TODEVICE);
            buffer_info->time_stamp = jiffies;
			buffer_info->next_to_watch = i;

            len -= size;
            offset += size;
            count++;
			if (unlikely(++i == tx_ring->count)) i = 0;
        }
    }

	i = (i == 0) ? tx_ring->count - 1 : i - 1;
    tx_ring->buffer_info[i].skb = skb;
    tx_ring->buffer_info[first].next_to_watch = i;

    return count;
}

static void iegbe_tx_queue(struct iegbe_adapter *adapter,
			   struct iegbe_tx_ring *tx_ring, int tx_flags,
			   int count)
{
	struct iegbe_hw *hw = &adapter->hw;
	struct iegbe_tx_desc *tx_desc = NULL;
	struct iegbe_buffer *buffer_info;
	u32 txd_upper = 0, txd_lower = E1000_TXD_CMD_IFCS;
	unsigned int i;

	if (likely(tx_flags & E1000_TX_FLAGS_TSO)) {
		txd_lower |= E1000_TXD_CMD_DEXT | E1000_TXD_DTYP_D |
		             E1000_TXD_CMD_TSE;
		txd_upper |= E1000_TXD_POPTS_TXSM << 8;

		if (likely(tx_flags & E1000_TX_FLAGS_IPV4))
			txd_upper |= E1000_TXD_POPTS_IXSM << 8;
	}

	if (likely(tx_flags & E1000_TX_FLAGS_CSUM)) {
		txd_lower |= E1000_TXD_CMD_DEXT | E1000_TXD_DTYP_D;
		txd_upper |= E1000_TXD_POPTS_TXSM << 8;
	}

    if(unlikely(tx_flags & E1000_TX_FLAGS_VLAN)) {
        txd_lower |= E1000_TXD_CMD_VLE;
        txd_upper |= (tx_flags & E1000_TX_FLAGS_VLAN_MASK);
    }

    i = tx_ring->next_to_use;

    while(count--) {
        buffer_info = &tx_ring->buffer_info[i];
        tx_desc = E1000_TX_DESC(*tx_ring, i);
        tx_desc->buffer_addr = cpu_to_le64(buffer_info->dma);
        tx_desc->lower.data =
            cpu_to_le32(txd_lower | buffer_info->length);
        tx_desc->upper.data = cpu_to_le32(txd_upper);
		if (unlikely(++i == tx_ring->count)) i = 0;
    }

        tx_desc->lower.data |= cpu_to_le32(adapter->txd_cmd);

    /* Force memory writes to complete before letting h/w
     * know there are new descriptors to fetch.  (Only
     * applicable for weak-ordered memory model archs,
     * such as IA-64). */
    wmb();

    tx_ring->next_to_use = i;
	writel(i, hw->hw_addr + tx_ring->tdt);
	/* we need this if more than one processor can write to our tail
	 * at a time, it syncronizes IO on IA64/Altix systems */
	mmiowb();
}

/**
 * 82547 workaround to avoid controller hang in half-duplex environment.
 * The workaround is to avoid queuing a large packet that would span
 * the internal Tx FIFO ring boundary by notifying the stack to resend
 * the packet at a later time.  This gives the Tx FIFO an opportunity to
 * flush all packets.  When that occurs, we reset the Tx FIFO pointers
 * to the beginning of the Tx FIFO.
 **/

#define E1000_FIFO_HDR			0x10
#define E1000_82547_PAD_LEN		0x3E0
static int iegbe_82547_fifo_workaround(struct iegbe_adapter *adapter,
				       struct sk_buff *skb)
{
	u32 fifo_space = adapter->tx_fifo_size - adapter->tx_fifo_head;
	u32 skb_fifo_len = skb->len + E1000_FIFO_HDR;

	skb_fifo_len = ALIGN(skb_fifo_len, E1000_FIFO_HDR);

	if (adapter->link_duplex != HALF_DUPLEX)
        goto no_fifo_stall_required;

	if (atomic_read(&adapter->tx_fifo_stall))
        return 1;

    if(skb_fifo_len >= (E1000_82547_PAD_LEN + fifo_space)) {
		atomic_set(&adapter->tx_fifo_stall, 1);
        return 1;
    }

no_fifo_stall_required:
    adapter->tx_fifo_head += skb_fifo_len;
	if (adapter->tx_fifo_head >= adapter->tx_fifo_size)
        adapter->tx_fifo_head -= adapter->tx_fifo_size;
	return 0;
}

#define MINIMUM_DHCP_PACKET_SIZE 282
static int iegbe_transfer_dhcp_info(struct iegbe_adapter *adapter,
				    struct sk_buff *skb)
{
	struct iegbe_hw *hw =  &adapter->hw;
	u16 length, offset;
	if (vlan_tx_tag_present(skb)) {
		if (!((vlan_tx_tag_get(skb) == hw->mng_cookie.vlan_id) &&
			( hw->mng_cookie.status &
			  E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT)) )
			return 0;
	}
	if (skb->len > MINIMUM_DHCP_PACKET_SIZE) {
		struct ethhdr *eth = (struct ethhdr *)skb->data;
		if ((htons(ETH_P_IP) == eth->h_proto)) {
			const struct iphdr *ip =
				(struct iphdr *)((u8 *)skb->data+14);
			if (IPPROTO_UDP == ip->protocol) {
				struct udphdr *udp =
					(struct udphdr *)((u8 *)ip +
						(ip->ihl << 2));
				if (ntohs(udp->dest) == 67) {
					offset = (u8 *)udp + 8 - skb->data;
					length = skb->len - offset;

					return iegbe_mng_write_dhcp_info(hw,
							(u8 *)udp + 8,
							length);
    }
			}
		}
	}
	return 0;
}

static int __iegbe_maybe_stop_tx(struct net_device *netdev, int size)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_tx_ring *tx_ring = adapter->tx_ring;

	netif_stop_queue(netdev);
	/* Herbert's original patch had:
	 *  smp_mb__after_netif_stop_queue();
	 * but since that doesn't exist yet, just open code it. */
	smp_mb();

	/* We need to check again in a case another CPU has just
	 * made room available. */
	if (likely(E1000_DESC_UNUSED(tx_ring) < size))
		return -EBUSY;

	/* A reprieve! */
	netif_start_queue(netdev);
	++adapter->restart_queue;
	return 0;
}

static int iegbe_maybe_stop_tx(struct net_device *netdev,
                               struct iegbe_tx_ring *tx_ring, int size)
{
	if (likely(E1000_DESC_UNUSED(tx_ring) >= size))
		return 0;
	return __iegbe_maybe_stop_tx(netdev, size);
}

#define TXD_USE_COUNT(S, X) (((S) >> (X)) + 1 )
static int iegbe_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
	struct iegbe_tx_ring *tx_ring;
	unsigned int first, max_per_txd = E1000_MAX_DATA_PER_TXD;
	unsigned int max_txd_pwr = E1000_MAX_TXD_PWR;
	unsigned int tx_flags = 0;
	unsigned int len = skb->len - skb->data_len;
	unsigned long flags = 0;
	unsigned int nr_frags;
	unsigned int mss;
	int count = 0;
	int tso;
	unsigned int f;

	/* This goes back to the question of how to logically map a tx queue
	 * to a flow.  Right now, performance is impacted slightly negatively
	 * if using multiple tx queues.  If the stack breaks away from a
	 * single qdisc implementation, we can look at this again. */
	tx_ring = adapter->tx_ring;

	if (unlikely(skb->len <= 0)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* 82571 and newer doesn't need the workaround that limited descriptor
	 * length to 4kB */
	if (hw->mac_type >= iegbe_82571)
		max_per_txd = 8192;

	mss = skb_shinfo(skb)->gso_size;
	/* The controller does a simple calculation to
	 * make sure there is enough room in the FIFO before
	 * initiating the DMA for each buffer.  The calc is:
	 * 4 = ceil(buffer len/mss).  To make sure we don't
	 * overrun the FIFO, adjust the max buffer len if mss
	 * drops. */
	if (mss) {
		u8 hdr_len;
		max_per_txd = min(mss << 2, max_per_txd);
		max_txd_pwr = fls(max_per_txd) - 1;

		/* TSO Workaround for 82571/2/3 Controllers -- if skb->data
		* points to just header, pull a few bytes of payload from
		* frags into skb->data */
		hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
		if (skb->data_len && hdr_len == len) {
			switch (hw->mac_type) {
			case iegbe_82544:
				/* Make sure we have room to chop off 4 bytes,
				 * and that the end alignment will work out to
				 * this hardware's requirements
				 * NOTE: this is a TSO only workaround
				 * if end byte alignment not correct move us
				 * into the next dword */
					break;
				/* fall through */
			case iegbe_82571:
			case iegbe_82572:
			case iegbe_82573:
				break;
			default:
				/* do nothing */
				break;
			}
		}
	}

	/* reserve a descriptor for the offload context */
	if ((mss) || (skb->ip_summed == CHECKSUM_PARTIAL))
		count++;
	count++;

	/* Controller Erratum workaround */
	if (!skb->data_len && tx_ring->last_tx_tso && !skb_is_gso(skb))
		count++;

	count += TXD_USE_COUNT(len, max_txd_pwr);

	if (adapter->pcix_82544)
		count++;

	/* work-around for errata 10 and it applies to all controllers
	 * in PCI-X mode, so add one more descriptor to the count
	 */
	if (unlikely((hw->bus_type == iegbe_bus_type_pcix) &&
			(len > 2015)))
		count++;

	nr_frags = skb_shinfo(skb)->nr_frags;
	for (f = 0; f < nr_frags; f++)
		count += TXD_USE_COUNT(skb_shinfo(skb)->frags[f].size,
				       max_txd_pwr);
	if (adapter->pcix_82544)
		count += nr_frags;


	if (hw->tx_pkt_filtering &&
	    (hw->mac_type == iegbe_82573))
		iegbe_transfer_dhcp_info(adapter, skb);

	if (!spin_trylock_irqsave(&tx_ring->tx_lock, flags))
		/* Collision - tell upper layer to requeue */
		return NETDEV_TX_LOCKED;

	/* need: count + 2 desc gap to keep tail from touching
	 * head, otherwise try next time */
	if (unlikely(iegbe_maybe_stop_tx(netdev, tx_ring, count + 2))) {
		spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
		return NETDEV_TX_BUSY;
	}

	if (unlikely(hw->mac_type == iegbe_82547)) {
		if (unlikely(iegbe_82547_fifo_workaround(adapter, skb))) {
			netif_stop_queue(netdev);
			mod_timer(&adapter->tx_fifo_stall_timer, jiffies + 1);
			spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
			return NETDEV_TX_BUSY;
		}
	}

	if (unlikely(iegbe_vlan_used(adapter) && vlan_tx_tag_present(skb))) {
		tx_flags |= E1000_TX_FLAGS_VLAN;
		tx_flags |= (vlan_tx_tag_get(skb) << E1000_TX_FLAGS_VLAN_SHIFT);
	}

	first = tx_ring->next_to_use;

	tso = iegbe_tso(adapter, tx_ring, skb);
	if (tso < 0) {
		dev_kfree_skb_any(skb);
		spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
		return NETDEV_TX_OK;
	}

	if (likely(tso)) {
		tx_ring->last_tx_tso = 1;
		tx_flags |= E1000_TX_FLAGS_TSO;
	} else if (likely(iegbe_tx_csum(adapter, tx_ring, skb)))
		tx_flags |= E1000_TX_FLAGS_CSUM;

	/* Old method was to assume IPv4 packet by default if TSO was enabled.
	 * 82571 hardware supports TSO capabilities for IPv6 as well...
	 * no longer assume, we must. */
	if (likely(skb->protocol == htons(ETH_P_IP)))
		tx_flags |= E1000_TX_FLAGS_IPV4;

	iegbe_tx_queue(adapter, tx_ring, tx_flags,
	               iegbe_tx_map(adapter, tx_ring, skb, first,
	                            max_per_txd, nr_frags, mss));

	netdev->trans_start = jiffies;

	/* Make sure there is space in the ring for the next send. */
	iegbe_maybe_stop_tx(netdev, tx_ring, MAX_SKB_FRAGS + 2);

	spin_unlock_irqrestore(&tx_ring->tx_lock, flags);
	return NETDEV_TX_OK;
}


/**
 * iegbe_tx_timeout - Respond to a Tx Hang
 * @netdev: network interface device structure
 **/

static void iegbe_tx_timeout(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);

    /* Do the reset outside of interrupt context */
	adapter->tx_timeout_count++;
	schedule_work(&adapter->reset_task);
}

static void iegbe_reset_task(struct work_struct *work)
{
	struct iegbe_adapter *adapter =
		container_of(work, struct iegbe_adapter, reset_task);

	iegbe_reinit_locked(adapter);
}

/**
 * iegbe_get_stats - Get System Network Statistics
 * @netdev: network interface device structure
 *
 * Returns the address of the device statistics structure.
 * The statistics are actually updated from the timer callback.
 **/

static struct net_device_stats *iegbe_get_stats(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);

	/* only return the current stats */
    return &adapter->net_stats;
}

/**
 * iegbe_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 **/

static int iegbe_change_mtu(struct net_device *netdev, int new_mtu)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
    int max_frame = new_mtu + ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;

    if((max_frame < MINIMUM_ETHERNET_FRAME_SIZE) ||
       (max_frame > MAX_JUMBO_FRAME_SIZE)) {
        DPRINTK(PROBE, ERR, "Invalid MTU setting\n");
        return -EINVAL;
    }

	/* Adapter-specific max frame size limits. */
	switch (hw->mac_type) {
	case iegbe_undefined ... iegbe_82542_rev2_1:
		if (max_frame > MAXIMUM_ETHERNET_FRAME_SIZE) {
			DPRINTK(PROBE, ERR, "Jumbo Frames not supported.\n");
        return -EINVAL;
    }
		break;
	case iegbe_82571:
	case iegbe_82572:
#define MAX_STD_JUMBO_FRAME_SIZE 9234
		if (max_frame > MAX_STD_JUMBO_FRAME_SIZE) {
			DPRINTK(PROBE, ERR, "MTU > 9216 not supported.\n");
            return -EINVAL;
		}
		break;
	default:
		break;
	}
	if (max_frame <= E1000_RXBUFFER_256)
		adapter->rx_buffer_len = E1000_RXBUFFER_256;
	else if (max_frame <= E1000_RXBUFFER_2048)
                adapter->rx_buffer_len = E1000_RXBUFFER_2048;
	else if (max_frame <= E1000_RXBUFFER_4096)
                adapter->rx_buffer_len = E1000_RXBUFFER_4096;
	else if (max_frame <= E1000_RXBUFFER_8192)
                adapter->rx_buffer_len = E1000_RXBUFFER_8192;
	else if (max_frame <= E1000_RXBUFFER_16384)
                adapter->rx_buffer_len = E1000_RXBUFFER_16384;

	/* adjust allocation if LPE protects us, and we aren't using SBP */

    netdev->mtu = new_mtu;
	hw->max_frame_size = max_frame;

	if (netif_running(netdev))
		iegbe_reinit_locked(adapter);

	return 0;
}

/**
 * iegbe_update_stats - Update the board statistics counters
 * @adapter: board private structure
 **/

void iegbe_update_stats(struct iegbe_adapter *adapter)
{
    struct iegbe_hw *hw = &adapter->hw;
    unsigned long flags = 0x0;
    uint16_t phy_tmp;

#define PHY_IDLE_ERROR_COUNT_MASK 0x00FF

    spin_lock_irqsave(&adapter->stats_lock, flags);

    /* these counters are modified from iegbe_adjust_tbi_stats,
     * called from the interrupt context, so they must only
     * be written while holding adapter->stats_lock
     */

    adapter->stats.crcerrs += E1000_READ_REG(hw, CRCERRS);
    adapter->stats.gprc += E1000_READ_REG(hw, GPRC);
    adapter->stats.gorcl += E1000_READ_REG(hw, GORCL);
    adapter->stats.gorch += E1000_READ_REG(hw, GORCH);
    adapter->stats.bprc += E1000_READ_REG(hw, BPRC);
    adapter->stats.mprc += E1000_READ_REG(hw, MPRC);
    adapter->stats.roc += E1000_READ_REG(hw, ROC);
    adapter->stats.prc64 += E1000_READ_REG(hw, PRC64);
    adapter->stats.prc127 += E1000_READ_REG(hw, PRC127);
    adapter->stats.prc255 += E1000_READ_REG(hw, PRC255);
    adapter->stats.prc511 += E1000_READ_REG(hw, PRC511);
    adapter->stats.prc1023 += E1000_READ_REG(hw, PRC1023);
    adapter->stats.prc1522 += E1000_READ_REG(hw, PRC1522);

    adapter->stats.symerrs += E1000_READ_REG(hw, SYMERRS);
    adapter->stats.mpc += E1000_READ_REG(hw, MPC);
    adapter->stats.scc += E1000_READ_REG(hw, SCC);
    adapter->stats.ecol += E1000_READ_REG(hw, ECOL);
    adapter->stats.mcc += E1000_READ_REG(hw, MCC);
    adapter->stats.latecol += E1000_READ_REG(hw, LATECOL);
    adapter->stats.dc += E1000_READ_REG(hw, DC);
    adapter->stats.sec += E1000_READ_REG(hw, SEC);
    adapter->stats.rlec += E1000_READ_REG(hw, RLEC);
    adapter->stats.xonrxc += E1000_READ_REG(hw, XONRXC);
    adapter->stats.xontxc += E1000_READ_REG(hw, XONTXC);
    adapter->stats.xoffrxc += E1000_READ_REG(hw, XOFFRXC);
    adapter->stats.xofftxc += E1000_READ_REG(hw, XOFFTXC);
    adapter->stats.fcruc += E1000_READ_REG(hw, FCRUC);
    adapter->stats.gptc += E1000_READ_REG(hw, GPTC);
    adapter->stats.gotcl += E1000_READ_REG(hw, GOTCL);
    adapter->stats.gotch += E1000_READ_REG(hw, GOTCH);
    adapter->stats.rnbc += E1000_READ_REG(hw, RNBC);
    adapter->stats.ruc += E1000_READ_REG(hw, RUC);
    adapter->stats.rfc += E1000_READ_REG(hw, RFC);
    adapter->stats.rjc += E1000_READ_REG(hw, RJC);
    adapter->stats.torl += E1000_READ_REG(hw, TORL);
    adapter->stats.torh += E1000_READ_REG(hw, TORH);
    adapter->stats.totl += E1000_READ_REG(hw, TOTL);
    adapter->stats.toth += E1000_READ_REG(hw, TOTH);
    adapter->stats.tpr += E1000_READ_REG(hw, TPR);
    adapter->stats.ptc64 += E1000_READ_REG(hw, PTC64);
    adapter->stats.ptc127 += E1000_READ_REG(hw, PTC127);
    adapter->stats.ptc255 += E1000_READ_REG(hw, PTC255);
    adapter->stats.ptc511 += E1000_READ_REG(hw, PTC511);
    adapter->stats.ptc1023 += E1000_READ_REG(hw, PTC1023);
    adapter->stats.ptc1522 += E1000_READ_REG(hw, PTC1522);
    adapter->stats.mptc += E1000_READ_REG(hw, MPTC);
    adapter->stats.bptc += E1000_READ_REG(hw, BPTC);

    /* used for adaptive IFS */

    hw->tx_packet_delta = E1000_READ_REG(hw, TPT);
    adapter->stats.tpt += hw->tx_packet_delta;
    hw->collision_delta = E1000_READ_REG(hw, COLC);
    adapter->stats.colc += hw->collision_delta;

    if(hw->mac_type >= iegbe_82543) {
        adapter->stats.algnerrc += E1000_READ_REG(hw, ALGNERRC);
        adapter->stats.rxerrc += E1000_READ_REG(hw, RXERRC);
        adapter->stats.tncrs += E1000_READ_REG(hw, TNCRS);
        adapter->stats.cexterr += E1000_READ_REG(hw, CEXTERR);
        adapter->stats.tsctc += E1000_READ_REG(hw, TSCTC);
        adapter->stats.tsctfc += E1000_READ_REG(hw, TSCTFC);
    }
    if(hw->mac_type > iegbe_82547_rev_2) {
        adapter->stats.iac += E1000_READ_REG(hw, IAC);
        adapter->stats.icrxoc += E1000_READ_REG(hw, ICRXOC);
        adapter->stats.icrxptc += E1000_READ_REG(hw, ICRXPTC);
        adapter->stats.icrxatc += E1000_READ_REG(hw, ICRXATC);
        adapter->stats.ictxptc += E1000_READ_REG(hw, ICTXPTC);
        adapter->stats.ictxatc += E1000_READ_REG(hw, ICTXATC);
        adapter->stats.ictxqec += E1000_READ_REG(hw, ICTXQEC);
        adapter->stats.ictxqmtc += E1000_READ_REG(hw, ICTXQMTC);
        adapter->stats.icrxdmtc += E1000_READ_REG(hw, ICRXDMTC);
    }

    /* Fill out the OS statistics structure */

    adapter->net_stats.rx_packets = adapter->stats.gprc;
    adapter->net_stats.tx_packets = adapter->stats.gptc;
    adapter->net_stats.rx_bytes = adapter->stats.gorcl;
    adapter->net_stats.tx_bytes = adapter->stats.gotcl;
    adapter->net_stats.multicast = adapter->stats.mprc;
    adapter->net_stats.collisions = adapter->stats.colc;

    /* Rx Errors */

    adapter->net_stats.rx_errors = adapter->stats.rxerrc +
        adapter->stats.crcerrs + adapter->stats.algnerrc +
        adapter->stats.rlec + adapter->stats.mpc +
        adapter->stats.cexterr;
    adapter->net_stats.rx_dropped = adapter->stats.mpc;
    adapter->net_stats.rx_length_errors = adapter->stats.rlec;
    adapter->net_stats.rx_crc_errors = adapter->stats.crcerrs;
    adapter->net_stats.rx_frame_errors = adapter->stats.algnerrc;
    adapter->net_stats.rx_fifo_errors = adapter->stats.mpc;
    adapter->net_stats.rx_missed_errors = adapter->stats.mpc;

    /* Tx Errors */

    adapter->net_stats.tx_errors = adapter->stats.ecol +
                                   adapter->stats.latecol;
    adapter->net_stats.tx_aborted_errors = adapter->stats.ecol;
    adapter->net_stats.tx_window_errors = adapter->stats.latecol;
    adapter->net_stats.tx_carrier_errors = adapter->stats.tncrs;

    /* Tx Dropped needs to be maintained elsewhere */

    /* Phy Stats */

    if(hw->media_type == iegbe_media_type_copper
       || (hw->media_type == iegbe_media_type_oem
           && iegbe_oem_phy_is_copper(&adapter->hw))) {
        if((adapter->link_speed == SPEED_1000) &&
           (!iegbe_read_phy_reg(hw, PHY_1000T_STATUS, &phy_tmp))) {
            phy_tmp &= PHY_IDLE_ERROR_COUNT_MASK;
            adapter->phy_stats.idle_errors += phy_tmp;
        }

        if((hw->mac_type <= iegbe_82546) &&
           (hw->phy_type == iegbe_phy_m88) &&
           !iegbe_read_phy_reg(hw, M88E1000_RX_ERR_CNTR, &phy_tmp)) {
            adapter->phy_stats.receive_errors += phy_tmp;
        }
    }

    spin_unlock_irqrestore(&adapter->stats_lock, flags);
}

/**
 * iegbe_intr_msi - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 **/

static irqreturn_t iegbe_intr_msi(int irq, void *data)
{
	struct net_device *netdev = data;
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	struct iegbe_hw *hw = &adapter->hw;
	u32 icr = E1000_READ_REG(&adapter->hw, ICR);
	if (icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC)) {
		hw->get_link_status = 1;
		if (!test_bit(__E1000_DOWN, &adapter->flags))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	if(unlikely(icr & (E1000_ICR_RX_DESC_FIFO_PAR
                       	 | E1000_ICR_TX_DESC_FIFO_PAR
			 | E1000_ICR_PB
			 | E1000_ICR_CPP_TARGET
			 | E1000_ICR_CPP_MASTER ))) {

	    iegbe_irq_disable(adapter);
	    printk("Critical error! ICR = 0x%x\n", icr);
	    return IRQ_HANDLED;
	}
	if (likely(napi_schedule_prep(&adapter->napi))) {
		adapter->total_tx_bytes = 0;
		adapter->total_tx_packets = 0;
		adapter->total_rx_bytes = 0;
		adapter->total_rx_packets = 0;
		__napi_schedule(&adapter->napi);
	} else
		iegbe_irq_enable(adapter);

	return IRQ_HANDLED;
}

/**
 * iegbe_intr - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 * @pt_regs: CPU registers structure
 **/

static irqreturn_t
iegbe_intr(int irq, void *data)
{
    struct net_device *netdev = data;
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;
	u32 icr = E1000_READ_REG(&adapter->hw, ICR);

	if (unlikely(!icr))
		return IRQ_NONE;  /* Not our interrupt */

	/* IMS will not auto-mask if INT_ASSERTED is not set, and if it is
	 * not set, then the adapter didn't send an interrupt */
	if (unlikely(hw->mac_type >= iegbe_82571 &&
	             !(icr & E1000_ICR_INT_ASSERTED)))
		return IRQ_NONE;


	if(unlikely(icr & (E1000_ICR_RX_DESC_FIFO_PAR
                       	 | E1000_ICR_TX_DESC_FIFO_PAR
			 | E1000_ICR_PB
			 | E1000_ICR_CPP_TARGET
			 | E1000_ICR_CPP_MASTER ))) {

	    iegbe_irq_disable(adapter);
	    printk("Critical error! ICR = 0x%x\n", icr);
	    return IRQ_HANDLED;
	}

	/* Interrupt Auto-Mask...upon reading ICR, interrupts are masked.  No
	 * need for the IMC write */
	if (unlikely(icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC))) {
		hw->get_link_status = 1;
		/* guard against interrupt when we're going down */
		if (!test_bit(__E1000_DOWN, &adapter->flags)) 
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	
    }

	if (unlikely(hw->mac_type < iegbe_82571)) {
		E1000_WRITE_REG(&adapter->hw, IMC, ~0);
		E1000_WRITE_FLUSH(&adapter->hw);
	}
	if (likely(napi_schedule_prep(&adapter->napi))) {
		adapter->total_tx_bytes = 0;
		adapter->total_tx_packets = 0;
		adapter->total_rx_bytes = 0;
		adapter->total_rx_packets = 0;
		__napi_schedule(&adapter->napi);
	} else
		/* this really should not happen! if it does it is basically a
		 * bug, but not a hard error, so enable ints and continue */
		iegbe_irq_enable(adapter);

	return IRQ_HANDLED;
}

/**
 * iegbe_clean - NAPI Rx polling callback
 * @adapter: board private structure
 **/
static int iegbe_clean(struct napi_struct *napi, int budget)
{
	struct iegbe_adapter *adapter = container_of(napi, struct iegbe_adapter, napi);
	struct net_device *poll_dev = adapter->netdev;
	int tx_cleaned = 0, work_done = 0;

	/* Must NOT use netdev_priv macro here. */
	adapter = netdev_priv(poll_dev);

	/* iegbe_clean is called per-cpu.  This lock protects
	 * tx_ring[0] from being cleaned by multiple cpus
	 * simultaneously.  A failure obtaining the lock means
	 * tx_ring[0] is currently being cleaned anyway. */
	if (spin_trylock(&adapter->tx_queue_lock)) {
		tx_cleaned = iegbe_clean_tx_irq(adapter,
						&adapter->tx_ring[0]);
		spin_unlock(&adapter->tx_queue_lock);
	}

	adapter->clean_rx(adapter, &adapter->rx_ring[0],
	                  &work_done, budget);

	if (tx_cleaned)
		work_done = budget;

	/* If budget not fully consumed, exit the polling mode */
	if (work_done < budget) {
		if (likely(adapter->itr_setting & 3))
			iegbe_set_itr(adapter);
		napi_complete(napi);
		iegbe_irq_enable(adapter);
	}

	return work_done;
}

/**
 * iegbe_clean_tx_irq - Reclaim resources after transmit completes
 * @adapter: board private structure
 **/
static bool iegbe_clean_tx_irq(struct iegbe_adapter *adapter,
                   struct iegbe_tx_ring *tx_ring)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
    struct iegbe_tx_desc *tx_desc, *eop_desc;
    struct iegbe_buffer *buffer_info;
    unsigned int i, eop;
	unsigned int count = 0;
	bool cleaned = false;
	unsigned int total_tx_bytes=0, total_tx_packets=0;

    i = tx_ring->next_to_clean;
    eop = tx_ring->buffer_info[i].next_to_watch;
    eop_desc = E1000_TX_DESC(*tx_ring, eop);

	while (eop_desc->upper.data & cpu_to_le32(E1000_TXD_STAT_DD)) {
		for (cleaned = false; !cleaned; ) {
            tx_desc = E1000_TX_DESC(*tx_ring, i);
            buffer_info = &tx_ring->buffer_info[i];
            cleaned = (i == eop);

                if (cleaned) {
				struct sk_buff *skb = buffer_info->skb;
				unsigned int segs = 0, bytecount;
				segs = skb_shinfo(skb)->gso_segs ?: 1;
				bytecount = ((segs - 1) * skb_headlen(skb)) +
				            skb->len;
				total_tx_packets += segs;
				total_tx_bytes += bytecount;
                }
			iegbe_unmap_and_free_tx_resource(adapter, buffer_info);
			tx_desc->upper.data = 0;

			if (unlikely(++i == tx_ring->count)) i = 0;
        }

        eop = tx_ring->buffer_info[i].next_to_watch;
        eop_desc = E1000_TX_DESC(*tx_ring, eop);
#define E1000_TX_WEIGHT 64
		/* weight of a sort for tx, to avoid endless transmit cleanup */
		if (count++ == E1000_TX_WEIGHT)
			break;
    }

	tx_ring->next_to_clean = i;

#define TX_WAKE_THRESHOLD 32

	if (unlikely(cleaned && netif_carrier_ok(netdev) &&
		     E1000_DESC_UNUSED(tx_ring) >= TX_WAKE_THRESHOLD)) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
		if (netif_queue_stopped(netdev)) {
			netif_wake_queue(netdev);
			++adapter->restart_queue;
		}
	}

    if (adapter->detect_tx_hung) {
        /* Detect a transmit hang in hardware, this serializes the
         * check with the clearing of time_stamp and movement of i */
		adapter->detect_tx_hung = false;

		if (tx_ring->buffer_info[eop].dma &&
		    time_after(jiffies, tx_ring->buffer_info[eop].time_stamp +
		               (adapter->tx_timeout_factor * HZ))
		    && !(E1000_READ_REG(hw, STATUS) & E1000_STATUS_TXOFF)) {

            /* detected Tx unit hang */
            DPRINTK(DRV, ERR, "Detected Tx Unit Hang\n"
					"  Tx Queue             <%lu>\n"
                    "  TDH                  <%x>\n"
                    "  TDT                  <%x>\n"
                    "  next_to_use          <%x>\n"
                    "  next_to_clean        <%x>\n"
                    "buffer_info[next_to_clean]\n"
                    "  time_stamp           <%lx>\n"
                    "  next_to_watch        <%x>\n"
                    "  jiffies              <%lx>\n"
                    "  next_to_watch.status <%x>\n",
				(unsigned long)((tx_ring - adapter->tx_ring) /
					sizeof(struct iegbe_tx_ring)),
				readl(hw->hw_addr + tx_ring->tdh),
				readl(hw->hw_addr + tx_ring->tdt),
                tx_ring->next_to_use,
				tx_ring->next_to_clean,
				tx_ring->buffer_info[eop].time_stamp,
                eop,
                jiffies,
                eop_desc->upper.fields.status);
            netif_stop_queue(netdev);
        }
    }
	adapter->total_tx_bytes += total_tx_bytes;
	adapter->total_tx_packets += total_tx_packets;
	adapter->net_stats.tx_bytes += total_tx_bytes;
	adapter->net_stats.tx_packets += total_tx_packets;
    return cleaned;
}

/**
 * iegbe_rx_checksum - Receive Checksum Offload for 82543
 * @adapter:     board private structure
 * @status_err:  receive descriptor status and error fields
 * @csum:        receive descriptor csum field
 * @sk_buff:     socket buffer with received data
 **/

static void iegbe_rx_checksum(struct iegbe_adapter *adapter, u32 status_err,
			      u32 csum, struct sk_buff *skb)
{
	struct iegbe_hw *hw = &adapter->hw;
	u16 status = (u16)status_err;
	u8 errors = (u8)(status_err >> 24);
	skb->ip_summed = CHECKSUM_NONE;

    /* 82543 or newer only */
	if (unlikely(hw->mac_type < iegbe_82543)) return;
    /* Ignore Checksum bit is set */
	if (unlikely(status & E1000_RXD_STAT_IXSM)) return;
    /* TCP/UDP checksum error bit is set */
    if(unlikely(errors & E1000_RXD_ERR_TCPE)) {
        /* let the stack verify checksum errors */
        adapter->hw_csum_err++;
        return;
    }
    /* TCP/UDP Checksum has not been calculated */
	if (hw->mac_type <= iegbe_82547_rev_2) {
		if (!(status & E1000_RXD_STAT_TCPCS))
            return;
    } else {
		if (!(status & (E1000_RXD_STAT_TCPCS | E1000_RXD_STAT_UDPCS)))
            return;
        }
    /* It must be a TCP or UDP packet with a valid checksum */
    if(likely(status & E1000_RXD_STAT_TCPCS)) {
        /* TCP checksum is good */
        skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else if (hw->mac_type > iegbe_82547_rev_2) {
        /* IP fragment with UDP payload */
        /* Hardware complements the payload checksum, so we undo it
         * and then put the value in host order for further stack use.
         */
		__sum16 sum = (__force __sum16)htons(csum);
		skb->csum = csum_unfold(~sum);
		skb->ip_summed = CHECKSUM_COMPLETE;
    }
    adapter->hw_csum_good++;
}

/**
 * iegbe_clean_rx_irq - Send received data up the network stack; legacy
 * @adapter: board private structure
 **/
static bool iegbe_clean_rx_irq(struct iegbe_adapter *adapter,
                   struct iegbe_rx_ring *rx_ring,
                   int *work_done, int work_to_do)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
	struct iegbe_rx_desc *rx_desc, *next_rxd;
	struct iegbe_buffer *buffer_info, *next_buffer;
	unsigned long flags;
	u32 length;
	u8 last_byte;
    unsigned int i;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes=0, total_rx_packets=0;

    i = rx_ring->next_to_clean;
    rx_desc = E1000_RX_DESC(*rx_ring, i);
	buffer_info = &rx_ring->buffer_info[i];

    while(rx_desc->status & E1000_RXD_STAT_DD) {
		struct sk_buff *skb;
		u8 status;
		if (*work_done >= work_to_do)
            break;
        (*work_done)++;

		status = rx_desc->status;
		skb = buffer_info->skb;
		buffer_info->skb = NULL;
		prefetch(skb->data - NET_IP_ALIGN);
		if (++i == rx_ring->count) i = 0;
		next_rxd = E1000_RX_DESC(*rx_ring, i);
		prefetch(next_rxd);
		next_buffer = &rx_ring->buffer_info[i];
		cleaned = true;
		cleaned_count++;
        pci_unmap_single(pdev,
                         buffer_info->dma,
                         buffer_info->length,
                         PCI_DMA_FROMDEVICE);

        length = le16_to_cpu(rx_desc->length);

		if (unlikely(!(status & E1000_RXD_STAT_EOP))) {
            /* All receives must fit into a single buffer */
            E1000_DBG("%s: Receive packet consumed multiple"
                  " buffers\n", netdev->name);
			buffer_info->skb = skb;
            goto next_desc;
        }

        if(unlikely(rx_desc->errors & E1000_RXD_ERR_FRAME_ERR_MASK)) {
			last_byte = *(skb->data + length - 1);
			if (TBI_ACCEPT(hw, status, rx_desc->errors, length,
				       last_byte)) {
                spin_lock_irqsave(&adapter->stats_lock, flags);
				iegbe_tbi_adjust_stats(hw, &adapter->stats,
                                       length, skb->data);
                spin_unlock_irqrestore(&adapter->stats_lock,
                                       flags);
                length--;
            } else {
				buffer_info->skb = skb;
                goto next_desc;
            }
        }

		/* adjust length to remove Ethernet CRC, this must be
		 * done after the TBI_ACCEPT workaround above */
		length -= 4;

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += length;
		total_rx_packets++;

		/* code added for copybreak, this should improve
		 * performance for small packets with large amounts
		 * of reassembly being done in the stack */
		if (length < copybreak) {
			struct sk_buff *new_skb =
			    netdev_alloc_skb(netdev, length + NET_IP_ALIGN);
			if (new_skb) {
				skb_reserve(new_skb, NET_IP_ALIGN);
				skb_copy_to_linear_data_offset(new_skb,
							       -NET_IP_ALIGN,
							       (skb->data -
							        NET_IP_ALIGN),
							       (length +
							        NET_IP_ALIGN));
				/* save the skb in buffer_info as good */
				buffer_info->skb = skb;
				skb = new_skb;
			}
			/* else just continue with the old one */
		}
        /* Good Receive */
		skb_put(skb, length);

		/* Receive Checksum Offload */
		iegbe_rx_checksum(adapter,
				  (u32)(status) |
				  ((u32)(rx_desc->errors) << 24),
				  le16_to_cpu(rx_desc->csum), skb);

		skb->protocol = eth_type_trans(skb, netdev);

		if (unlikely(iegbe_vlan_used(adapter) &&
			    (status & E1000_RXD_STAT_VP))) {
			u16 vid;

			vid = le16_to_cpu(rx_desc->special);
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
		} else {
			netif_receive_skb(skb);
		}

		netdev->last_rx = jiffies;

next_desc:
		rx_desc->status = 0;

		/* return some buffers to hardware, one at a time is too slow */
		if (unlikely(cleaned_count >= E1000_RX_BUFFER_WRITE)) {
			adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		/* use prefetched values */
		rx_desc = next_rxd;
		buffer_info = next_buffer;
	}
	rx_ring->next_to_clean = i;

	cleaned_count = E1000_DESC_UNUSED(rx_ring);
	if (cleaned_count)
		adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);

	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;
	adapter->net_stats.rx_bytes += total_rx_bytes;
	adapter->net_stats.rx_packets += total_rx_packets;
	return cleaned;
}

/**
 * iegbe_clean_rx_irq_ps - Send received data up the network stack; packet split
 * @adapter: board private structure
 **/

static bool iegbe_clean_rx_irq_ps(struct iegbe_adapter *adapter,
                      struct iegbe_rx_ring *rx_ring,
                      int *work_done, int work_to_do)
{
	union iegbe_rx_desc_packet_split *rx_desc, *next_rxd;
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
	struct iegbe_buffer *buffer_info, *next_buffer;
    struct iegbe_ps_page *ps_page;
    struct iegbe_ps_page_dma *ps_page_dma;
    struct sk_buff *skb;
    unsigned int i, j;
	u32 length, staterr;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes=0, total_rx_packets=0;

    i = rx_ring->next_to_clean;
    rx_desc = E1000_RX_DESC_PS(*rx_ring, i);
    staterr = le32_to_cpu(rx_desc->wb.middle.status_error);
	buffer_info = &rx_ring->buffer_info[i];

    while(staterr & E1000_RXD_STAT_DD) {
        ps_page = &rx_ring->ps_page[i];
        ps_page_dma = &rx_ring->ps_page_dma[i];

		if (unlikely(*work_done >= work_to_do))
            break;
        (*work_done)++;

		skb = buffer_info->skb;
		prefetch(skb->data - NET_IP_ALIGN);
		if (++i == rx_ring->count) i = 0;
		next_rxd = E1000_RX_DESC_PS(*rx_ring, i);
		prefetch(next_rxd);
		next_buffer = &rx_ring->buffer_info[i];
		cleaned = true;
		cleaned_count++;
        pci_unmap_single(pdev, buffer_info->dma,
                 buffer_info->length,
                 PCI_DMA_FROMDEVICE);

        if(unlikely(!(staterr & E1000_RXD_STAT_EOP))) {
            E1000_DBG("%s: Packet Split buffers didn't pick up"
                  " the full packet\n", netdev->name);
            dev_kfree_skb_irq(skb);
            goto next_desc;
        }

        if(unlikely(staterr & E1000_RXDEXT_ERR_FRAME_ERR_MASK)) {
            dev_kfree_skb_irq(skb);
            goto next_desc;
        }

        length = le16_to_cpu(rx_desc->wb.middle.length0);

        if(unlikely(!length)) {
            E1000_DBG("%s: Last part of the packet spanning"
                  " multiple descriptors\n", netdev->name);
            dev_kfree_skb_irq(skb);
            goto next_desc;
        }

        /* Good Receive */
        skb_put(skb, length);

		{
		int l1 = le16_to_cpu(rx_desc->wb.upper.length[0]);
		if (l1 && (l1 <= copybreak) && ((length + l1) <= adapter->rx_ps_bsize0)) {
			u8 *vaddr;
			pci_dma_sync_single_for_cpu(pdev,
				ps_page_dma->ps_page_dma[0],
				PAGE_SIZE,
				PCI_DMA_FROMDEVICE);
			vaddr = kmap_atomic(ps_page->ps_page[0]);
			memcpy(skb_tail_pointer(skb), vaddr, l1);
			kunmap_atomic(vaddr);
			pci_dma_sync_single_for_device(pdev,
				ps_page_dma->ps_page_dma[0],
				PAGE_SIZE, PCI_DMA_FROMDEVICE);
			l1 -= 4;
			skb_put(skb, l1);
			goto copydone;
		} /* if */
		}
		for (j = 0; j < adapter->rx_ps_pages; j++) {
			length = le16_to_cpu(rx_desc->wb.upper.length[j]);
			if (!length)
                break;
            pci_unmap_page(pdev, ps_page_dma->ps_page_dma[j],
                    PAGE_SIZE, PCI_DMA_FROMDEVICE);
			ps_page_dma->ps_page_dma[j] = 0;
			skb_fill_page_desc(skb, j, ps_page->ps_page[j], 0,
			                   length);
            ps_page->ps_page[j] = NULL;
            skb->len += length;
            skb->data_len += length;
			skb->truesize += length;
        }

		pskb_trim(skb, skb->len - 4);
copydone:
		total_rx_bytes += skb->len;
		total_rx_packets++;
        iegbe_rx_checksum(adapter, staterr,
				  le16_to_cpu(rx_desc->wb.lower.hi_dword.csum_ip.csum), skb);
        skb->protocol = eth_type_trans(skb, netdev);

        if(likely(rx_desc->wb.upper.header_status &
			   cpu_to_le16(E1000_RXDPS_HDRSTAT_HDRSP)))
            adapter->rx_hdr_split++;

        if(unlikely(iegbe_vlan_used(adapter) && (staterr & E1000_RXD_STAT_VP))) {
	    u16 vid;
	    vid = le16_to_cpu(rx_desc->wb.middle.vlan);
	    __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
        } else {
            netif_receive_skb(skb);
        }

        netdev->last_rx = jiffies;

next_desc:
		rx_desc->wb.middle.status_error &= cpu_to_le32(~0xFF);
        buffer_info->skb = NULL;

		if (unlikely(cleaned_count >= E1000_RX_BUFFER_WRITE)) {
			adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		/* use prefetched values */
		rx_desc = next_rxd;
		buffer_info = next_buffer;
        staterr = le32_to_cpu(rx_desc->wb.middle.status_error);
    }
    rx_ring->next_to_clean = i;

	cleaned_count = E1000_DESC_UNUSED(rx_ring);
	if (cleaned_count)
		adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);

	adapter->total_rx_packets += total_rx_packets;
	adapter->total_rx_bytes += total_rx_bytes;
	adapter->net_stats.rx_bytes += total_rx_bytes;
	adapter->net_stats.rx_packets += total_rx_packets;
    return cleaned;
}

/**
 * iegbe_alloc_rx_buffers - Replace used receive buffers; legacy & extended
 * @adapter: address of board private structure
 **/


static void iegbe_alloc_rx_buffers(struct iegbe_adapter *adapter,
                       struct iegbe_rx_ring *rx_ring,
                       int cleaned_count)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
    struct iegbe_rx_desc *rx_desc;
    struct iegbe_buffer *buffer_info;
    struct sk_buff *skb;
    unsigned int i;
    unsigned int bufsz = adapter->rx_buffer_len + NET_IP_ALIGN;

    i = rx_ring->next_to_use;
    buffer_info = &rx_ring->buffer_info[i];

	while (cleaned_count--) {
		skb = buffer_info->skb;
		if (skb) {
			skb_trim(skb, 0);
			goto map_skb;
		}
		skb = netdev_alloc_skb(netdev, bufsz);

        if(unlikely(!skb)) {
            /* Better luck next round */
			adapter->alloc_rx_buff_failed++;
            break;
        }

        /* Fix for errata 23, can't cross 64kB boundary */
        if(!iegbe_check_64k_bound(adapter, skb->data, bufsz)) {
            struct sk_buff *oldskb = skb;
            DPRINTK(RX_ERR, ERR, "skb align check failed: %u bytes "
                         "at %p\n", bufsz, skb->data);
            /* Try again, without freeing the previous */
			skb = netdev_alloc_skb(netdev, bufsz);
            /* Failed allocation, critical failure */
            if(!skb) {
                dev_kfree_skb(oldskb);
                break;
            }

            if(!iegbe_check_64k_bound(adapter, skb->data, bufsz)) {
                /* give up */
                dev_kfree_skb(skb);
                dev_kfree_skb(oldskb);
                break; /* while !buffer_info->skb */
			}
                /* Use new allocation */
                dev_kfree_skb(oldskb);
            }
        /* Make buffer alignment 2 beyond a 16 byte boundary
         * this will result in a 16 byte aligned IP header after
         * the 14 byte MAC header is removed
         */
        skb_reserve(skb, NET_IP_ALIGN);


        buffer_info->skb = skb;
        buffer_info->length = adapter->rx_buffer_len;
map_skb:
        buffer_info->dma = pci_map_single(pdev,
                          skb->data,
                          adapter->rx_buffer_len,
                          PCI_DMA_FROMDEVICE);

        /* Fix for errata 23, can't cross 64kB boundary */
        if(!iegbe_check_64k_bound(adapter,
                    (void *)(unsigned long)buffer_info->dma,
                    adapter->rx_buffer_len)) {
            DPRINTK(RX_ERR, ERR,
                "dma align check failed: %u bytes at %p\n",
                adapter->rx_buffer_len,
                (void *)(unsigned long)buffer_info->dma);
            dev_kfree_skb(skb);
            buffer_info->skb = NULL;

            pci_unmap_single(pdev, buffer_info->dma,
                     adapter->rx_buffer_len,
                     PCI_DMA_FROMDEVICE);

            break; /* while !buffer_info->skb */
        }
        rx_desc = E1000_RX_DESC(*rx_ring, i);
        rx_desc->buffer_addr = cpu_to_le64(buffer_info->dma);

            /* Force memory writes to complete before letting h/w
             * know there are new descriptors to fetch.  (Only
             * applicable for weak-ordered memory model archs,
             * such as IA-64). */
		if (unlikely(++i == rx_ring->count))
			i = 0;
        buffer_info = &rx_ring->buffer_info[i];
    }

	if (likely(rx_ring->next_to_use != i)) {
    rx_ring->next_to_use = i;
		if (unlikely(i-- == 0))
			i = (rx_ring->count - 1);

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64). */
		wmb();
		writel(i, hw->hw_addr + rx_ring->rdt);
	}
}

/**
 * iegbe_alloc_rx_buffers_ps - Replace used receive buffers; packet split
 * @adapter: address of board private structure
 **/


static void iegbe_alloc_rx_buffers_ps(struct iegbe_adapter *adapter,
                          struct iegbe_rx_ring *rx_ring,
                          int cleaned_count)
{
	struct iegbe_hw *hw = &adapter->hw;
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
    union iegbe_rx_desc_packet_split *rx_desc;
    struct iegbe_buffer *buffer_info;
    struct iegbe_ps_page *ps_page;
    struct iegbe_ps_page_dma *ps_page_dma;
    struct sk_buff *skb;
    unsigned int i, j;

    i = rx_ring->next_to_use;
    buffer_info = &rx_ring->buffer_info[i];
    ps_page = &rx_ring->ps_page[i];
    ps_page_dma = &rx_ring->ps_page_dma[i];

	while (cleaned_count--) {
        rx_desc = E1000_RX_DESC_PS(*rx_ring, i);

		for (j = 0; j < PS_PAGE_BUFFERS; j++) {
            if (j < adapter->rx_ps_pages) {
                if (likely(!ps_page->ps_page[j])) {
                    ps_page->ps_page[j] =
                        alloc_page(GFP_ATOMIC);
                    if (unlikely(!ps_page->ps_page[j])) {
						adapter->alloc_rx_buff_failed++;
                        goto no_buffers;
                    }
                    ps_page_dma->ps_page_dma[j] =
                        pci_map_page(pdev,
                                ps_page->ps_page[j],
							    0, PAGE_SIZE,
							    PCI_DMA_FROMDEVICE);
				}
				/* Refresh the desc even if buffer_addrs didn't
				 * change because each write-back erases
				 * this info.
				 */
				rx_desc->read.buffer_addr[j+1] =
				     cpu_to_le64(ps_page_dma->ps_page_dma[j]);
			} else
				rx_desc->read.buffer_addr[j+1] = ~cpu_to_le64(0);
		}

		skb = netdev_alloc_skb(netdev,
		                       adapter->rx_ps_bsize0 + NET_IP_ALIGN);

		if (unlikely(!skb)) {
			adapter->alloc_rx_buff_failed++;
			break;
		}

		/* Make buffer alignment 2 beyond a 16 byte boundary
		 * this will result in a 16 byte aligned IP header after
		 * the 14 byte MAC header is removed
		 */
		skb_reserve(skb, NET_IP_ALIGN);

		buffer_info->skb = skb;
		buffer_info->length = adapter->rx_ps_bsize0;
		buffer_info->dma = pci_map_single(pdev, skb->data,
						  adapter->rx_ps_bsize0,
						  PCI_DMA_FROMDEVICE);

		rx_desc->read.buffer_addr[0] = cpu_to_le64(buffer_info->dma);

		if (unlikely(++i == rx_ring->count)) i = 0;
		buffer_info = &rx_ring->buffer_info[i];
		ps_page = &rx_ring->ps_page[i];
		ps_page_dma = &rx_ring->ps_page_dma[i];
	}

no_buffers:
	if (likely(rx_ring->next_to_use != i)) {
    rx_ring->next_to_use = i;
		if (unlikely(i-- == 0)) i = (rx_ring->count - 1);

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64). */
		wmb();
		/* Hardware increments by 16 bytes, but packet split
		 * descriptors are 32 bytes...so we increment tail
		 * twice as much.
		 */
		writel(i<<1, hw->hw_addr + rx_ring->rdt);
	}
}

/**
 * iegbe_smartspeed - Workaround for SmartSpeed on 82541 and 82547 controllers.
 * @adapter:
 **/

static void
iegbe_smartspeed(struct iegbe_adapter *adapter)
{
    uint16_t phy_status;
    uint16_t phy_ctrl;

    if((adapter->hw.phy_type != iegbe_phy_igp) || !adapter->hw.autoneg ||
       !(adapter->hw.autoneg_advertised & ADVERTISE_1000_FULL)) {
        return;
    }
    if(adapter->smartspeed == 0x0) {
        /* If Master/Slave config fault is asserted twice,
         * we assume back-to-back */
        iegbe_read_phy_reg(&adapter->hw, PHY_1000T_STATUS, &phy_status);
        if(!(phy_status & SR_1000T_MS_CONFIG_FAULT)) { return; }
        iegbe_read_phy_reg(&adapter->hw, PHY_1000T_STATUS, &phy_status);
        if(!(phy_status & SR_1000T_MS_CONFIG_FAULT)) { return; }
        iegbe_read_phy_reg(&adapter->hw, PHY_1000T_CTRL, &phy_ctrl);
        if(phy_ctrl & CR_1000T_MS_ENABLE) {
            phy_ctrl &= ~CR_1000T_MS_ENABLE;
            iegbe_write_phy_reg(&adapter->hw, PHY_1000T_CTRL,
                        phy_ctrl);
            adapter->smartspeed++;
            if(!iegbe_phy_setup_autoneg(&adapter->hw) &&
               !iegbe_read_phy_reg(&adapter->hw, PHY_CTRL,
                              &phy_ctrl)) {
                phy_ctrl |= (MII_CR_AUTO_NEG_EN |
                         MII_CR_RESTART_AUTO_NEG);
                iegbe_write_phy_reg(&adapter->hw, PHY_CTRL,
                            phy_ctrl);
            }
        }
        return;
    } else if(adapter->smartspeed == E1000_SMARTSPEED_DOWNSHIFT) {
        /* If still no link, perhaps using 2/3 pair cable */
        iegbe_read_phy_reg(&adapter->hw, PHY_1000T_CTRL, &phy_ctrl);
        phy_ctrl |= CR_1000T_MS_ENABLE;
        iegbe_write_phy_reg(&adapter->hw, PHY_1000T_CTRL, phy_ctrl);
        if(!iegbe_phy_setup_autoneg(&adapter->hw) &&
           !iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_ctrl)) {
            phy_ctrl |= (MII_CR_AUTO_NEG_EN |
                     MII_CR_RESTART_AUTO_NEG);
            iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, phy_ctrl);
        }
    }
    /* Restart process after E1000_SMARTSPEED_MAX iterations */
    if(adapter->smartspeed++ == E1000_SMARTSPEED_MAX) {
        adapter->smartspeed = 0x0;
    }
}

/**
 * iegbe_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int iegbe_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
    switch (cmd) {
#ifdef SIOCGMIIPHY
    case SIOCGMIIPHY:
    case SIOCGMIIREG:
    case SIOCSMIIREG:
        return iegbe_mii_ioctl(netdev, ifr, cmd);
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
 * iegbe_mii_ioctl -
 * @netdev:
 * @ifreq:
 * @cmd:
 **/

static int iegbe_mii_ioctl(struct net_device *netdev, struct ifreq *ifr,
			   int cmd)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct mii_ioctl_data *data = if_mii(ifr);
    int retval;
    uint16_t mii_reg;
    uint16_t spddplx;
    unsigned long flags = 0;

    if((adapter->hw.media_type == iegbe_media_type_oem
        && !iegbe_oem_phy_is_copper(&adapter->hw))
       ||adapter->hw.media_type != iegbe_media_type_copper) {
        return -EOPNOTSUPP;
    }
    switch (cmd) {
    case SIOCGMIIPHY:
        data->phy_id = adapter->hw.phy_addr;
        break;
    case SIOCGMIIREG:
        if(!capable(CAP_NET_ADMIN)) {
            return -EPERM;
        }
        spin_lock_irqsave(&adapter->stats_lock, flags);
        if(iegbe_read_phy_reg(&adapter->hw, data->reg_num & 0x1F,
                   &data->val_out)) {
            spin_unlock_irqrestore(&adapter->stats_lock, flags);
            return -EIO;
        }
        spin_unlock_irqrestore(&adapter->stats_lock, flags);
        break;
    case SIOCSMIIREG:
        if(!capable(CAP_NET_ADMIN)){
            return -EPERM;
        }
        if(data->reg_num & ~(0x1F)) {
            return -EFAULT;
        }
        mii_reg = data->val_in;
        spin_lock_irqsave(&adapter->stats_lock, flags);
        if(iegbe_write_phy_reg(&adapter->hw, data->reg_num,
                    mii_reg)) {
            spin_unlock_irqrestore(&adapter->stats_lock, flags);
            return -EIO;
        }
        switch(adapter->hw.phy_type) {
        case iegbe_phy_m88:
            switch (data->reg_num) {
            case PHY_CTRL:
                if(mii_reg & MII_CR_POWER_DOWN) {
                    break;
                }
                if(mii_reg & MII_CR_AUTO_NEG_EN) {
                    adapter->hw.autoneg = 1;
                    adapter->hw.autoneg_advertised = 0x2F;
                } else {
                    if(mii_reg & 0x40){
                        spddplx = SPEED_1000;
                    } else if(mii_reg & 0x2000) {
                        spddplx = SPEED_100;
                    } else {
                        spddplx = SPEED_10;
                          }
                    spddplx += (mii_reg & 0x100)
                           ? FULL_DUPLEX :
                           HALF_DUPLEX;
                    retval = iegbe_set_spd_dplx(adapter,
                                    spddplx);
                    if(retval) {
                        spin_unlock_irqrestore(
                            &adapter->stats_lock,
                            flags);
                        return retval;
                    }
                }
                if(netif_running(adapter->netdev)) {
                    iegbe_down(adapter);
                    iegbe_up(adapter);
                } else {
                    iegbe_reset(adapter);
                }
                break;
            case M88E1000_PHY_SPEC_CTRL:
            case M88E1000_EXT_PHY_SPEC_CTRL:
                if(iegbe_phy_reset(&adapter->hw)) {
                    spin_unlock_irqrestore(
                        &adapter->stats_lock, flags);
                    return -EIO;
                }
                break;
            }
            break;

        case iegbe_phy_oem:
            retval = iegbe_oem_mii_ioctl(adapter, flags, ifr, cmd);
            if(retval) {
                spin_unlock_irqrestore(
                    &adapter->stats_lock, flags);
                return retval;
            }
            break;

        default:
            switch (data->reg_num) {
            case PHY_CTRL:
                if(mii_reg & MII_CR_POWER_DOWN) {
                    break;
                }
                if(netif_running(adapter->netdev)) {
                    iegbe_down(adapter);
                    iegbe_up(adapter);
                } else {
                    iegbe_reset(adapter);
                }
                break;
            }
        }
        spin_unlock_irqrestore(&adapter->stats_lock, flags);
        break;
    default:
        return -EOPNOTSUPP;
    }
    return E1000_SUCCESS;
}
#endif

void iegbe_pci_set_mwi(struct iegbe_hw *hw)
{
    struct iegbe_adapter *adapter = hw->back;
    int ret_val = pci_set_mwi(adapter->pdev);

	if (ret_val)
        DPRINTK(PROBE, ERR, "Error in setting MWI\n");
}

void iegbe_pci_clear_mwi(struct iegbe_hw *hw)
{
    struct iegbe_adapter *adapter = hw->back;

    pci_clear_mwi(adapter->pdev);
}

void
iegbe_read_pci_cfg(struct iegbe_hw *hw, uint32_t reg, uint16_t *value)
{
    struct iegbe_adapter *adapter = hw->back;

    pci_read_config_word(adapter->pdev, reg, value);
}

void
iegbe_write_pci_cfg(struct iegbe_hw *hw, uint32_t reg, uint16_t *value)
{
    struct iegbe_adapter *adapter = hw->back;

    pci_write_config_word(adapter->pdev, reg, *value);
}

uint32_t
iegbe_io_read(struct iegbe_hw *hw, unsigned long port)
{
    return inl(port);
}

void
iegbe_io_write(struct iegbe_hw *hw, unsigned long port, uint32_t value)
{
    outl(value, port);
}

static bool iegbe_vlan_used(struct iegbe_adapter *adapter)
{
	u16 vid;

	for_each_set_bit(vid, adapter->active_vlans, VLAN_N_VID)
		return true;

	return false;
}

static void iegbe_vlan_mode(struct net_device *netdev, bool vlan_on)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    uint32_t ctrl, rctl;

	if (!test_bit(__E1000_DOWN, &adapter->flags))
    iegbe_irq_disable(adapter);

    if(vlan_on) {
        /* enable VLAN tag insert/strip */
        ctrl = E1000_READ_REG(&adapter->hw, CTRL);
        ctrl |= E1000_CTRL_VME;
        E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);

        /* enable VLAN receive filtering */
        rctl = E1000_READ_REG(&adapter->hw, RCTL);
        rctl |= E1000_RCTL_VFE;
        rctl &= ~E1000_RCTL_CFIEN;
        E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
        iegbe_update_mng_vlan(adapter);
    } else {
        /* disable VLAN tag insert/strip */
        ctrl = E1000_READ_REG(&adapter->hw, CTRL);
        ctrl &= ~E1000_CTRL_VME;
        E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);

        /* disable VLAN filtering */
        rctl = E1000_READ_REG(&adapter->hw, RCTL);
        rctl &= ~E1000_RCTL_VFE;
        E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
        if(adapter->mng_vlan_id != (uint16_t)E1000_MNG_VLAN_NONE) {
            iegbe_vlan_rx_kill_vid(netdev, htons(ETH_P_8021Q), adapter->mng_vlan_id);
            adapter->mng_vlan_id = E1000_MNG_VLAN_NONE;
        }
    }

	if (!test_bit(__E1000_DOWN, &adapter->flags))
    iegbe_irq_enable(adapter);
}

static int iegbe_vlan_rx_add_vid(struct net_device *netdev,__be16 proto, u16 vid)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    uint32_t vfta, index;
    if((adapter->hw.mng_cookie.status &
        E1000_MNG_DHCP_COOKIE_STATUS_VLAN_SUPPORT) &&
        (vid == adapter->mng_vlan_id)) {
        return 0;
    }

    if (!iegbe_vlan_used(adapter))
	iegbe_vlan_mode(netdev, true);

    /* add VID to filter table */
    index = (vid >> 0x5) & 0x7F;
    vfta = E1000_READ_REG_ARRAY(&adapter->hw, VFTA, index);
    vfta |= (0x1 << (vid & 0x1F));
    iegbe_write_vfta(&adapter->hw, index, vfta);

    set_bit(vid, adapter->active_vlans);

    return 0;
}

static int iegbe_vlan_rx_kill_vid(struct net_device *netdev,__be16 proto, u16 vid)
{
	struct iegbe_adapter *adapter = netdev_priv(netdev);
	u32 vfta, index;

	if (!test_bit(__E1000_DOWN, &adapter->flags))
	iegbe_irq_disable(adapter);
	if (!test_bit(__E1000_DOWN, &adapter->flags))
	iegbe_irq_enable(adapter);

	/* remove VID from filter table */
	index = (vid >> 0x5) & 0x7F;
	vfta = E1000_READ_REG_ARRAY(&adapter->hw, VFTA, index);
	vfta &= ~(0x1 << (vid & 0x1F));
	iegbe_write_vfta(&adapter->hw, index, vfta);

	clear_bit(vid, adapter->active_vlans);

	if (!iegbe_vlan_used(adapter))
		iegbe_vlan_mode(netdev, false);

	return 0;

}

static void iegbe_restore_vlan(struct iegbe_adapter *adapter)
{
		u16 vid;
	if (!iegbe_vlan_used(adapter))
		return;

	iegbe_vlan_mode(adapter->netdev, true);
	for_each_set_bit(vid, adapter->active_vlans, VLAN_N_VID)
 			iegbe_vlan_rx_add_vid(adapter->netdev, htons(ETH_P_8021Q), vid);
}


int iegbe_set_spd_dplx(struct iegbe_adapter *adapter, u16 spddplx)
{
    adapter->hw.autoneg = 0x0;

    /* Fiber NICs only allow 1000 gbps Full duplex */
    if((adapter->hw.media_type == iegbe_media_type_fiber
        || (adapter->hw.media_type == iegbe_media_type_oem
            && !iegbe_oem_phy_is_copper(&adapter->hw)))
       && spddplx != (SPEED_1000 + DUPLEX_FULL)) {
        DPRINTK(PROBE, ERR, "Unsupported Speed/Duplex configuration\n");
        return -EINVAL;
    }

    switch(spddplx) {
    case SPEED_10 + DUPLEX_HALF:
        adapter->hw.forced_speed_duplex = iegbe_10_half;
        break;
    case SPEED_10 + DUPLEX_FULL:
        adapter->hw.forced_speed_duplex = iegbe_10_full;
        break;
    case SPEED_100 + DUPLEX_HALF:
        adapter->hw.forced_speed_duplex = iegbe_100_half;
        break;
    case SPEED_100 + DUPLEX_FULL:
        adapter->hw.forced_speed_duplex = iegbe_100_full;
        break;
    case SPEED_1000 + DUPLEX_FULL:
        adapter->hw.autoneg = 0x1;
        adapter->hw.autoneg_advertised = ADVERTISE_1000_FULL;
        break;
    case SPEED_1000 + DUPLEX_HALF: /* not supported */
    default:
        DPRINTK(PROBE, ERR, "Unsupported Speed/Duplex configuration\n");
        return -EINVAL;
    }
    return 0x0;
}

static int
iegbe_notify_reboot(struct notifier_block *nb, unsigned long event, void *p)
{
    struct pci_dev *pdev = NULL;
    pm_message_t state = {0x3};


    switch(event) {
    case SYS_DOWN:
    case SYS_HALT:
    case SYS_POWER_OFF:
        while((pdev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
            if(pci_dev_driver(pdev) == &iegbe_driver) {
                iegbe_suspend(pdev, state);
            }
        }
    }
    return NOTIFY_DONE;
}

static int
iegbe_suspend(struct pci_dev *pdev, pm_message_t state)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    uint32_t ctrl, ctrl_ext, rctl, manc, status, swsm;
    uint32_t wufc = adapter->wol;
    uint16_t cmd_word;

    netif_device_detach(netdev);

    if(netif_running(netdev)) {
		WARN_ON(test_bit(__E1000_RESETTING, &adapter->flags));
        iegbe_down(adapter);
    }
    /*
     * ICP_XXXX style MACs do not have a link up bit in
     * the STATUS register, query the PHY directly
     */
    if(adapter->hw.mac_type != iegbe_icp_xxxx) {
        status = E1000_READ_REG(&adapter->hw, STATUS);
        if(status & E1000_STATUS_LU) {
            wufc &= ~E1000_WUFC_LNKC;
        }
    } else {
        int isUp = 0x0;
        if(iegbe_oem_phy_is_link_up(&adapter->hw, &isUp) != E1000_SUCCESS) {
            isUp = 0x0;
        }
        if(isUp) {
            wufc &= ~E1000_WUFC_LNKC;
        }
    }

    if(wufc) {
        iegbe_setup_rctl(adapter);
		iegbe_set_rx_mode(netdev);

        /* turn on all-multi mode if wake on multicast is enabled */
        if(adapter->wol & E1000_WUFC_MC) {
            rctl = E1000_READ_REG(&adapter->hw, RCTL);
            rctl |= E1000_RCTL_MPE;
            E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
        }

        if(adapter->hw.mac_type >= iegbe_82540) {
            ctrl = E1000_READ_REG(&adapter->hw, CTRL);
            /* advertise wake from D3Cold */
            #define E1000_CTRL_ADVD3WUC 0x00100000
            /* phy power management enable */
			#define E1000_CTRL_EN_PHY_PWR_MGMT 0x00200000
            ctrl |= E1000_CTRL_ADVD3WUC |
                    (adapter->hw.mac_type != iegbe_icp_xxxx
                                 ? E1000_CTRL_EN_PHY_PWR_MGMT : 0x0);

            E1000_WRITE_REG(&adapter->hw, CTRL, ctrl);
        }

        if(adapter->hw.media_type == iegbe_media_type_fiber ||
           adapter->hw.media_type == iegbe_media_type_internal_serdes) {
            /* keep the laser running in D3 */
            ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
            ctrl_ext |= E1000_CTRL_EXT_SDP7_DATA;
            E1000_WRITE_REG(&adapter->hw, CTRL_EXT, ctrl_ext);
        }

        /* Allow OEM PHYs (if any exist) to keep the laser
         *running in D3 */
        iegbe_oem_fiber_live_in_suspend(&adapter->hw);

        /* Allow time for pending master requests to run */
        iegbe_disable_pciex_master(&adapter->hw);

        E1000_WRITE_REG(&adapter->hw, WUC, E1000_WUC_PME_EN);
        E1000_WRITE_REG(&adapter->hw, WUFC, wufc);
        pci_enable_wake(pdev, 0x3, 0x1);
        pci_enable_wake(pdev, 0x4, 0x1); /* 4 == D3 cold */
    } else {
        E1000_WRITE_REG(&adapter->hw, WUC, 0x0);
        E1000_WRITE_REG(&adapter->hw, WUFC, 0x0);
        pci_enable_wake(pdev, 0x3, 0x0);
        pci_enable_wake(pdev, 0x4, 0x0); /* 4 == D3 cold */
    }

    pci_save_state(pdev);

    if(adapter->hw.mac_type >= iegbe_82540
       && adapter->hw.mac_type != iegbe_icp_xxxx
       && adapter->hw.media_type == iegbe_media_type_copper) {
        manc = E1000_READ_REG(&adapter->hw, MANC);
        if(manc & E1000_MANC_SMBUS_EN) {
            manc |= E1000_MANC_ARP_EN;
            E1000_WRITE_REG(&adapter->hw, MANC, manc);
            pci_enable_wake(pdev, 0x3, 0x1);
            pci_enable_wake(pdev, 0x4, 0x1); /* 4 == D3 cold */
        }
    }

    switch(adapter->hw.mac_type) {
    case iegbe_82571:
    case iegbe_82572:
        ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
        E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
                ctrl_ext & ~E1000_CTRL_EXT_DRV_LOAD);
        break;
    case iegbe_82573:
        swsm = E1000_READ_REG(&adapter->hw, SWSM);
        E1000_WRITE_REG(&adapter->hw, SWSM,
                swsm & ~E1000_SWSM_DRV_LOAD);
        break;
    default:
        break;
    }

    pci_disable_device(pdev);
    if(adapter->hw.mac_type == iegbe_icp_xxxx) {
        /*
         * ICP xxxx devices are not true PCI devices, in the context
         * of power management, disabling the bus mastership is not
         * sufficient to disable the device, it is also necessary to
         * disable IO, Memory, and Interrupts if they are enabled.
         */
        pci_read_config_word(pdev, PCI_COMMAND, &cmd_word);
        if(cmd_word & PCI_COMMAND_IO) {
            cmd_word &= ~PCI_COMMAND_IO;
        }
        if(cmd_word & PCI_COMMAND_MEMORY) {
            cmd_word &= ~PCI_COMMAND_MEMORY;
        }
        if(cmd_word & PCI_COMMAND_INTX_DISABLE) {
            cmd_word &= ~PCI_COMMAND_INTX_DISABLE;
        }
        pci_write_config_word(pdev, PCI_COMMAND, cmd_word);
    }

    state.event = (state.event > 0x0) ? 0x3 : 0x0;
    pci_set_power_state(pdev, state.event);
	 if(gcu_suspend == 0x0)
	 {
	 	if(gcu == NULL) {
    		gcu = pci_get_device(PCI_VENDOR_ID_INTEL, GCU_DEVID, NULL); 
		}		
	 	gcu_iegbe_suspend(gcu, 0x3);
	 	gcu_suspend = 0x1;
	 	gcu_resume = 0x0;
	 }
    return 0x0;
}

#ifdef CONFIG_PM
static int
iegbe_resume(struct pci_dev *pdev)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    uint32_t manc, ret_val, swsm;
    uint32_t ctrl_ext;
	 int offset;
    uint32_t vdid;

	 if(gcu_resume == 0x0)
	 {
	 	if(gcu == NULL) {
    		gcu = pci_get_device(PCI_VENDOR_ID_INTEL, GCU_DEVID, NULL); 
		   pci_read_config_dword(gcu, 0x00, &vdid);
		}		
		
	 	if(gcu) {
			gcu_iegbe_resume(gcu);
	 		gcu_resume = 0x1;
	 		gcu_suspend = 0x0;
		} else {
			printk("Unable to resume GCU!\n");
		}	
	 }
    pci_set_power_state(pdev, 0x0);
    pci_restore_state(pdev);
    ret_val = pci_enable_device(pdev);
    pci_set_master(pdev);

    pci_enable_wake(pdev, 0x3, 0x0);
    pci_enable_wake(pdev, 0x4, 0x0); /* 4 == D3 cold */

    iegbe_reset(adapter);
    E1000_WRITE_REG(&adapter->hw, WUS, ~0);
    offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_ST)
                 + PCI_ST_SMIA_OFFSET;
    pci_write_config_dword(adapter->pdev, offset, 0x00000006);
    E1000_WRITE_REG(&adapter->hw, IMC1, ~0UL);
    E1000_WRITE_REG(&adapter->hw, IMC2, ~0UL);

    if(netif_running(netdev)) {
        iegbe_up(adapter);
    }
    netif_device_attach(netdev);

    if(adapter->hw.mac_type >= iegbe_82540
       && adapter->hw.mac_type != iegbe_icp_xxxx
       && adapter->hw.media_type == iegbe_media_type_copper) {
        manc = E1000_READ_REG(&adapter->hw, MANC);
        manc &= ~(E1000_MANC_ARP_EN);
        E1000_WRITE_REG(&adapter->hw, MANC, manc);
    }

    switch(adapter->hw.mac_type) {
    case iegbe_82571:
    case iegbe_82572:
        ctrl_ext = E1000_READ_REG(&adapter->hw, CTRL_EXT);
        E1000_WRITE_REG(&adapter->hw, CTRL_EXT,
                ctrl_ext | E1000_CTRL_EXT_DRV_LOAD);
        break;
    case iegbe_82573:
        swsm = E1000_READ_REG(&adapter->hw, SWSM);
        E1000_WRITE_REG(&adapter->hw, SWSM,
                swsm | E1000_SWSM_DRV_LOAD);
        break;
    default:
        break;
    }

    return 0x0;
}
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void iegbe_netpoll(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    disable_irq(adapter->pdev->irq);
    iegbe_intr(adapter->pdev->irq, netdev);
    enable_irq(adapter->pdev->irq);
}
#endif


/* iegbe_main.c */
