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

#define RT3883_SYSC_REG_RSTCTRL		0x34
#define RT3883_RSTCTRL_FE		BIT(21)

static void rt3883_fe_reset(void)
{
	u32 t;

	t = rt_sysc_r32(RT3883_SYSC_REG_RSTCTRL);
	t |= RT3883_RSTCTRL_FE;
	rt_sysc_w32(t , RT3883_SYSC_REG_RSTCTRL);

	t &= ~RT3883_RSTCTRL_FE;
	rt_sysc_w32(t, RT3883_SYSC_REG_RSTCTRL);
}

static struct fe_soc_data rt3883_data = {
	.mac = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
	.reset_fe = rt3883_fe_reset,
	.min_pkt_len = 64,
        .pdma_glo_cfg = FE_PDMA_SIZE_4DWORDS,
	.rx_dly_int = FE_RX_DLY_INT,
	.tx_dly_int = FE_TX_DLY_INT,
	.checksum_bit = RX_DMA_L4VALID,
	.mdio_read = rt2880_mdio_read,
	.mdio_write = rt2880_mdio_write,
	.mdio_adjust_link = rt2880_mdio_link_adjust,
	.port_init = rt2880_port_init,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "ralink,rt3883-eth", .data = &rt3883_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);

