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
1*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
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

#ifndef CNS3XXX_PHY_H
#define CNS3XXX_PHY_H

#define LINUX_KERNEL // if don't define LINUX_KERNEL, mean u-boot

#if defined(LINUX_KERNEL)
#include <linux/version.h>
#include <linux/types.h>
#else // u-boot
#define __init_or_module
#include "cns3xxx_symbol.h"
#endif

void disable_AN(int port, int y);

static u16 get_phy_id(u8 phy_addr);
int cns3xxx_std_phy_power_down(int phy_addr, int y);
u32 get_vsc8601_recv_err_counter(u8 phy_addr);
u32 get_crc_good_counter(u8 phy_addr);
int cns3xxx_config_VSC8601(u8 mac_port, u8 phy_addr);
int vsc8601_power_down(int phy_addr, int y);
int icp_101a_init(u8 mac_port, u8 phy_addr);
int bcm53115M_init(u8 mac_port, u16 phy_addr);
int icp_ip1001_init(u8 mac_port, u8 phy_addr);

int cns3xxx_phy_auto_polling_enable(u8 port, u8 en);

int cns3xxx_read_phy(u8 phy_addr, u8 phy_reg, u16 *read_data);
int cns3xxx_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data);

// wrap cns3xxx_spi_tx_rx() for argument order
int cns3xxx_spi_tx_rx_n(u32 tx_data, u32 *rx_data, u32 tx_channel, u32 tx_eof_flag);

// for bcm53115M
#define ROBO_SPIF_BIT 7
#define BCM53115_SPI_CHANNEL 1
#define ROBO_RACK_BIT 5

#define VLAN_START_BIT 7
#define VLAN_WRITE_CMD 0

//#define BCM_PORT_1G 2
typedef enum
{
   BCM_PORT_10M = 0,
   BCM_PORT_100M,
   BCM_PORT_1G,
}BCM_PORT_SPEED;

#define BCM_PORT_0 0
#define BCM_PORT_1 1
#define BCM_PORT_2 2
#define BCM_PORT_3 3
#define BCM_PORT_4 4
#define BCM_PORT_5 5
#define BCM_PORT_IMP 6

#endif // end #ifndef CNS3XXX_PHY_H
