/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2009-2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>

#include "ralink_soc_eth.h"
#include "mdio_rt2880.h"

#define SYSC_REG_RESET_CTRL		0x034
#define RT2880_RESET_FE			BIT(18)

void rt2880_fe_reset(void)
{
	rt_sysc_w32(RT2880_RESET_FE, SYSC_REG_RESET_CTRL);
}

struct fe_soc_data rt2880_data = {
	.mac = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
	.reset_fe = rt2880_fe_reset,
	.min_pkt_len = 64,
        .pdma_glo_cfg = FE_PDMA_SIZE_4DWORDS,
	.checksum_bit = RX_DMA_L4VALID,
	.rx_dly_int = FE_RX_DLY_INT,
	.tx_dly_int = FE_TX_DLY_INT,
	.mdio_read = rt2880_mdio_read,
	.mdio_write = rt2880_mdio_write,
	.mdio_adjust_link = rt2880_mdio_link_adjust,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "ralink,rt2880-eth", .data = &rt2880_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
