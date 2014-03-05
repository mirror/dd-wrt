/*****************************************************************************

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

*****************************************************************************/

/**************************************************************************
 * @ingroup GCU_INTERFACE
 *
 * @file gcu_if.c
 *
 * @description
 *   This module contains shared functions for accessing and configuring 
 *   the GCU.
 *
 **************************************************************************/

#include "gcu.h"
#include "gcu_reg.h"
#include "gcu_if.h"

/* forward declaration for write verify used in gcu_write_eth_phy */
int32_t gcu_write_verify(uint32_t phy_num, 
                         uint32_t reg_addr, 
                         uint16_t written_data, 
                         const struct gcu_adapter *adapter);

/**
 * gcu_write_eth_phy
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be written
 *
 * interface function for other modules to access the GCU
 **/
int32_t 
gcu_write_eth_phy(uint32_t phy_num, uint32_t reg_addr, uint16_t phy_data)
{
    const struct gcu_adapter *adapter;
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete;

    GCU_DBG("%s\n", __func__);

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return -1;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return -1;
    }

    /* format the data to be written to the MDIO_COMMAND_REG */
    data = phy_data;
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_OPER_MASK | MDIO_COMMAND_GO_MASK;

   /*
     * get_gcu_adapter contains a spinlock, this may pause for a bit
     */
    adapter = gcu_get_adapter();
    if(!adapter)
    {
      GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
      return -1;
    }

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);
    /* KAD !complete to complete */

    if(timeoutCounter == timeoutCounterMax && !complete)
    {
      GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

      gcu_release_adapter(&adapter);

      return -1;
    }    

    /* validate the write during debug */
#ifdef DBG
    if(!gcu_write_verify(phy_num, reg_addr, phy_data, adapter))
    {
        GCU_ERR("Write verification failed for PHY=%d and addr=%d\n",
                phy_num, reg_addr);

        gcu_release_adapter(&adapter);

        return -1;
    }
#endif
    
    gcu_release_adapter(&adapter);

    return 0;
}
EXPORT_SYMBOL(gcu_write_eth_phy);


/**
 * gcu_read_eth_phy
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be written
 *
 * interface function for other modules to access the GCU
 **/
int32_t 
gcu_read_eth_phy(uint32_t phy_num, uint32_t reg_addr, uint16_t *phy_data)
{
    const struct gcu_adapter *adapter;
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete = 0;
    
    GCU_DBG("%s\n", __func__);

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return -1;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return -1;
    }

    /* format the data to be written to MDIO_COMMAND_REG */
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_GO_MASK;

    /* 
     * this call contains a spinlock, so this may pause for a bit
     */
    adapter = gcu_get_adapter();
    if(!adapter)
    {
        GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
        return -1;
    }

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);
    /* KAD !complete to complete */

    if(timeoutCounter == timeoutCounterMax && !complete)
    {
        GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

         gcu_release_adapter(&adapter);

        return -1;
    }

    /* we retrieve the data from the MDIO_STATUS_REGISTER */
    data = ioread32(adapter->hw_addr + MDIO_STATUS_REG);
    if((data & MDIO_STATUS_STATUS_MASK) != 0)
    {
        GCU_ERR("Unable to retrieve data from MDIO_STATUS_REG\n");

        gcu_release_adapter(&adapter);

        return -1;
    }

    *phy_data = (uint16_t) (data & MDIO_STATUS_READ_DATA_MASK);

    gcu_release_adapter(&adapter);

    return 0;
}
EXPORT_SYMBOL(gcu_read_eth_phy);


/**
 * gcu_write_verify
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be checked
 * @adapter: pointer to global adapter struct
 *
 * This f(n) assumes that the spinlock acquired for adapter is
 * still in force.
 **/
int32_t 
gcu_write_verify(uint32_t phy_num, uint32_t reg_addr, uint16_t written_data,
                 const struct gcu_adapter *adapter)
{
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete = 0;

    GCU_DBG("%s\n", __func__);

    if(!adapter)
    {
        GCU_ERR("Invalid adapter pointer\n");
        return 0;
    }

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return 0;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return 0;
    }

    /* format the data to be written to MDIO_COMMAND_REG */
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_GO_MASK;

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);


    if(timeoutCounter == timeoutCounterMax && !complete)
    {
        GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

        return 0;
    }

    /* we retrieve the data from the MDIO_STATUS_REGISTER */
    data = ioread32(adapter->hw_addr + MDIO_STATUS_REG);
    if((data & MDIO_STATUS_STATUS_MASK) != 0)
    {
        GCU_ERR("Unable to retrieve data from MDIO_STATUS_REG\n");

        return 0;
    }

    return written_data == (uint16_t) (data & MDIO_STATUS_READ_DATA_MASK);
}
 
/*
 * gcu_iegbe_resume
 * @pdev: gcu pci_dev  
 * purpose - exported PM resume function used by iegbe 
 *           driver to enable the GCU device.
 */
void gcu_iegbe_resume(struct pci_dev *pdev)
{
#if ( ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,6) ) && \
      ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) ) )
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct gcu_adapter *adapter = netdev_priv(netdev);
#endif

    GCU_DBG("%s\n", __func__);

    pci_restore_state(pdev);
    if(!pci_enable_device(pdev))
        GCU_DBG("pci_enable_device failed!\n",);

    return;
}
EXPORT_SYMBOL(gcu_iegbe_resume);

/*
 * gcu_iegbe_suspend
 * @pdev: gcu pci_dev  
 * @state: PM state  
 * purpose - exported PM suspend function used by iegbe 
 *           driver to disable the GCU device.   
 */
int gcu_iegbe_suspend(struct pci_dev *pdev, uint32_t state)
{
#if ( ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,6) ) && \
      ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) ) )
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct gcu_adapter *adapter = netdev_priv(netdev);
#endif

    GCU_DBG("%s\n", __func__);

    pci_save_state(pdev);
    pci_disable_device(pdev);
    state = (state > 0) ? 0 : 0;

    return state;
}

EXPORT_SYMBOL(gcu_iegbe_suspend);
