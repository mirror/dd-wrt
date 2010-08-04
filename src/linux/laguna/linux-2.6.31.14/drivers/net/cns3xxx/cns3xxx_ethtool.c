/*******************************************************************************
 *
 *
 *   Copyright (c) 2009 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 ********************************************************************************/

//#include <linux/module.h>
#include <linux/types.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include "cns3xxx_symbol.h"
#include "cns3xxx.h"
#include "cns3xxx_tool.h"

// ethtool support reference e100.c and e1000_ethtool.c .
static void cns3xxx_get_drvinfo(struct net_device *netdev, struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "cns3xxx");
        strcpy(info->version, DRV_VERSION);
        strcpy(info->fw_version, "N/A");
        strcpy(info->bus_info, "N/A");
}

static void cns3xxx_get_ringparam(struct net_device *netdev,
        struct ethtool_ringparam *ring)
{
	CNS3XXXPrivate *priv = netdev_priv(netdev);

        ring->rx_max_pending = priv->rx_ring->max_ring_size;
        ring->tx_max_pending = priv->tx_ring->max_ring_size;
        ring->rx_pending = priv->rx_ring->ring_size;
        ring->tx_pending = priv->tx_ring->ring_size;
#if 0
        struct nic *nic = netdev_priv(netdev);
        struct param_range *rfds = &nic->params.rfds;
        struct param_range *cbs = &nic->params.cbs;

        ring->rx_max_pending = rfds->max;
        ring->tx_max_pending = cbs->max;
        ring->rx_mini_max_pending = 0;
        ring->rx_jumbo_max_pending = 0;
        ring->rx_pending = rfds->count;
        ring->tx_pending = cbs->count;
        ring->rx_mini_pending = 0;
        ring->rx_jumbo_pending = 0;
#endif
}



static int cns3xxx_set_ringparam(struct net_device *netdev, struct ethtool_ringparam *ring)
{
	int cns3xxx_up(void);
	int cns3xxx_down(void);
	int cns3xxx_close(struct net_device *dev);
	int cns3xxx_open(struct net_device *dev);
	extern struct net_device *net_dev_array[];

	CNS3XXXPrivate *priv = netdev_priv(netdev);

	int i=0;

#if 0
        struct nic *nic = netdev_priv(netdev);
        struct param_range *rfds = &nic->params.rfds;
        struct param_range *cbs = &nic->params.cbs;

        if ((ring->rx_mini_pending) || (ring->rx_jumbo_pending))
                return -EINVAL;

        if(netif_running(netdev))
                e100_down(nic);
        rfds->count = max(ring->rx_pending, rfds->min);
        rfds->count = min(rfds->count, rfds->max);
        cbs->count = max(ring->tx_pending, cbs->min);
        cbs->count = min(cbs->count, cbs->max);
        DPRINTK(DRV, INFO, "Ring Param settings: rx: %d, tx %d\n",
                rfds->count, cbs->count);
        if(netif_running(netdev))
                e100_up(nic);

#endif
        //ring->rx_max_pending = RX_DESC_SIZE;
        //ring->tx_max_pending = TX_DESC_SIZE;

#if 0
	printk("ring->rx_max_pending: %d\n", ring->rx_max_pending);
	printk("ring->tx_max_pending: %d\n", ring->tx_max_pending);
	printk("ring->rx_pending: %d\n", ring->rx_pending);
	printk("ring->tx_pending: %d\n", ring->tx_pending);
#endif

	for (i=0 ; i < NETDEV_SIZE ; ++i) {
        	if(net_dev_array[i] && netif_running(net_dev_array[i])) {
			//printk("close net_dev_array[%d]: %s\n", i, net_dev_array[i]);
			cns3xxx_close(net_dev_array[i]);
		}
	}

                //cns3xxx_down();

        priv->rx_ring->ring_size = min(ring->rx_pending, priv->rx_ring->max_ring_size);
        priv->tx_ring->ring_size = min(ring->rx_pending, priv->tx_ring->max_ring_size);

	for (i=0 ; i < NETDEV_SIZE ; ++i) {
        	if(net_dev_array[i] && netif_running(net_dev_array[i])) {
			//printk("open net_dev_array[%d]: %s\n", i, net_dev_array[i]);
			cns3xxx_open(net_dev_array[i]);
		}
	}
                //cns3xxx_up();

        return 0;
}

static uint32_t cns3xxx_get_tx_csum(struct net_device *netdev)
{
        //return (netdev->features & NETIF_F_HW_CSUM) != 0;
        return (netdev->features & NETIF_F_IP_CSUM) != 0;
}

static int cns3xxx_set_tx_csum(struct net_device *netdev, uint32_t data)
{
	if (data)
                netdev->features |= NETIF_F_IP_CSUM;
        else
                netdev->features &= ~NETIF_F_IP_CSUM;
	return 0;
}

static uint32_t cns3xxx_get_rx_csum(struct net_device *netdev)
{
        //struct e1000_adapter *adapter = netdev_priv(netdev);
        //return adapter->rx_csum;
	return 1;
}

static int cns3xxx_set_rx_csum(struct net_device *netdev, uint32_t data)
{
	return 0;
}

u32 cns3xxx_get_sg(struct net_device *dev)
{
#ifdef NETIF_F_SG
        return (dev->features & NETIF_F_SG) != 0;
#else
        return 0;
#endif
}

int cns3xxx_set_sg(struct net_device *dev, u32 data)
{
#ifdef NETIF_F_SG 
        if (data)
                dev->features |= NETIF_F_SG;
        else
                dev->features &= ~NETIF_F_SG;
#endif

        return 0;
}

static void cns3xxx_get_pauseparam(struct net_device *netdev, struct ethtool_pauseparam *pause)
{
        u32 mac_port_config = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);

	switch (priv->net_device_priv->which_port)
	{
		case MAC_PORT0:
		{
			mac_port_config = MAC0_CFG_REG;
			break;
		}
		case MAC_PORT1:
		{
			mac_port_config = MAC1_CFG_REG;
			break;
		}
		case MAC_PORT2:
		{
			mac_port_config = MAC2_CFG_REG;
			break;
		}
	}


        pause->autoneg = ( ((mac_port_config >> 7) & 1) ? AUTONEG_ENABLE : AUTONEG_DISABLE);
	pause->tx_pause = (mac_port_config >> 6) & 1;
	pause->rx_pause = (mac_port_config >> 5) & 1;
}

static int cns3xxx_set_pauseparam(struct net_device *netdev, struct ethtool_pauseparam *pause)
{
        u32 mac_port_config = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);

	switch (priv->net_device_priv->which_port)
	{
		case MAC_PORT0:
		{
			mac_port_config = MAC0_CFG_REG;
			break;
		}
		case MAC_PORT1:
		{
			mac_port_config = MAC1_CFG_REG;
			break;
		}
		case MAC_PORT2:
		{
			mac_port_config = MAC2_CFG_REG;
			break;
		}
	}


	mac_port_config &= ~(0x1 << 7); // clean AN
	mac_port_config &= ~(0x1 << 11); // clean rx flow control
	mac_port_config &= ~(0x1 << 12); // clean tx flow control

	mac_port_config |= ( (pause->autoneg << 7) | (pause->rx_pause << 11) | (pause->tx_pause << 12) );


	switch (priv->net_device_priv->which_port)
	{
		case MAC_PORT0:
		{
			MAC0_CFG_REG  = mac_port_config;
			break;
		}
		case MAC_PORT1:
		{
			MAC1_CFG_REG  = mac_port_config;
			break;
		}
		case MAC_PORT2:
		{
			MAC2_CFG_REG  = mac_port_config;
			break;
		}
	}
	return 0;
}

u32 cns3xxx_get_link(struct net_device *netdev)
{
        u32 mac_port_config = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);

	switch (priv->net_device_priv->which_port)
	{
		case MAC_PORT0:
		{
			mac_port_config = MAC0_CFG_REG;
			break;
		}
		case MAC_PORT1:
		{
			mac_port_config = MAC1_CFG_REG;
			break;
		}
		case MAC_PORT2:
		{
			mac_port_config = MAC2_CFG_REG;
			break;
		}
	}

        return (mac_port_config & 1 ) ? 1 : 0;
        //return netif_carrier_ok(dev) ? 1 : 0;
}


static int cns3xxx_get_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
	u8 value;
        u32 mac_port_config = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);


	if (priv->net_device_priv->nic_setting == 0) { // connect to switch chip

		GET_MAC_PORT_CFG(priv->net_device_priv->which_port, mac_port_config)

		ecmd->supported = (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full | SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full | SUPPORTED_1000baseT_Half | SUPPORTED_1000baseT_Full| SUPPORTED_Autoneg | SUPPORTED_TP | SUPPORTED_MII | SUPPORTED_Pause);
	
		ecmd->duplex = ((mac_port_config >> 4) & 0x1) ? DUPLEX_FULL : DUPLEX_HALF ;
	
		value = ((mac_port_config >> 2) & 0x3);
		switch (value)
		{
			case 0:
				ecmd->speed = SPEED_10;
				break;
			case 1:
				ecmd->speed = SPEED_100;
				break;
			case 2:
				ecmd->speed = SPEED_1000;
				break;
		}
	
		ecmd->autoneg = ((mac_port_config >> 7) & 1) ? AUTONEG_ENABLE : AUTONEG_DISABLE;



	} else { // connect to PHY chip

	}

	return 0;
}

// set speed and duplex
int cns3xxx_set_spd_dplx(struct net_device *netdev, u16 spddplx)
{
        u32 mac_port_config = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);

	GET_MAC_PORT_CFG(priv->net_device_priv->which_port, mac_port_config)

	//printk("mac_port_config: %x\n", mac_port_config);

	mac_port_config &= ~(0x3 << 8); // clear speed
	mac_port_config &= ~(0x1 << 10); // clear duplex
	mac_port_config &= ~(0x1 << 7); // disable AN

        switch (spddplx) {
        case AUTONEG_ENABLE:
		mac_port_config |= (0x1 << 7); // enable AN
                break;
        case SPEED_10 + DUPLEX_HALF:
		printk("10, halt\n");
                mac_port_config |= (0 << 8);    // set speed
                mac_port_config |= (0 << 10);    // set duplex
		//printk("xxx mac_port_config: %x\n", mac_port_config);
                break;
        case SPEED_10 + DUPLEX_FULL:
                mac_port_config |= (0 << 8);    // set speed
                mac_port_config |= (1 << 10);    // set duplex
                break;
        case SPEED_100 + DUPLEX_HALF:
                mac_port_config |= (1 << 8);    // set speed
                mac_port_config |= (0 << 10);    // set duplex
                break;
        case SPEED_100 + DUPLEX_FULL:
                mac_port_config |= (1 << 8);    // set speed
                mac_port_config |= (1 << 10);    // set duplex
                break;
        case SPEED_1000 + DUPLEX_HALF: 
                mac_port_config |= (2 << 8);    // set speed
                mac_port_config |= (0 << 10);    // set duplex
                break;
        case SPEED_1000 + DUPLEX_FULL:
                mac_port_config |= (2 << 8);    // set speed
                mac_port_config |= (1 << 10);    // set duplex
                break;
        default:
                //printk("Unsupported Speed/Duplex configuration\n");
                return -EINVAL;
	}

	SET_MAC_PORT_CFG(priv->net_device_priv->which_port, mac_port_config)

	return 0;
}

static int cns3xxx_set_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
	u8 value = 0;
	CNS3XXXPrivate *priv = netdev_priv(netdev);

	if (priv->net_device_priv->nic_setting == 0) { // connect to switch chip
		if (ecmd->autoneg == AUTONEG_ENABLE) {
			printk("autoneg\n");
			if ((value=cns3xxx_set_spd_dplx(netdev, AUTONEG_ENABLE)) != 0) {
				return -EINVAL;
			} 
		} else {
			printk("no autoneg\n");
			if ((value=cns3xxx_set_spd_dplx(netdev, ecmd->speed + ecmd->duplex)) != 0) {
				return -EINVAL;
			} 


		}

	} else { // connect to PHY chip 

	}

	// down then up
	return 0;
}

static const struct ethtool_ops cns3xxx_ethtool_ops = {
	.get_drvinfo            = cns3xxx_get_drvinfo,
        .get_ringparam          = cns3xxx_get_ringparam,
        .set_ringparam          = cns3xxx_set_ringparam,
        .get_rx_csum            = cns3xxx_get_rx_csum,
        .set_rx_csum            = cns3xxx_set_rx_csum,
        .get_tx_csum            = cns3xxx_get_tx_csum,
        .set_tx_csum            = cns3xxx_set_tx_csum,
        .get_sg  	        = cns3xxx_get_sg,
        .set_sg                 = cns3xxx_set_sg,
        .get_pauseparam         = cns3xxx_get_pauseparam,
        .set_pauseparam         = cns3xxx_set_pauseparam,
        .get_link               = cns3xxx_get_link,
        .get_settings           = cns3xxx_get_settings,
        .set_settings           = cns3xxx_set_settings,
};

void cns3xxx_set_ethtool_ops(struct net_device *netdev)
{
	SET_ETHTOOL_OPS(netdev, &cns3xxx_ethtool_ops);
}
