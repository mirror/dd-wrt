/*
################################################################################
#
# r8126 is the Linux device driver released for Realtek 5 Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2025 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>

#include "r8126.h"
#include "r8126_fiber.h"

static void
rtl8126_fiber_set_mdc_gpio_c45(struct rtl8126_private *tp, bool pu)
{
        if (pu)
                rtl8126_set_mac_ocp_bit(tp, 0xDC52, BIT_7);
        else
                rtl8126_clear_mac_ocp_bit(tp, 0xDC52, BIT_7);

        //RtPciCommittp);
}

static void
rtl8126_fiber_set_mdc(struct rtl8126_private *tp, bool pu)
{
        rtl8126_fiber_set_mdc_gpio_c45(tp, pu);
}

static void
rtl8126_fiber_set_mdcDownUp(struct rtl8126_private *tp)
{
        udelay(1);
        rtl8126_fiber_set_mdc(tp, 0);
        udelay(1);
        rtl8126_fiber_set_mdc(tp, 1);
}

static void
rtl8126_fiber_set_mdio_bit_gpio_c45(struct rtl8126_private *tp, bool pu)
{
        if (pu)
                rtl8126_set_mac_ocp_bit(tp, 0xDC52, BIT_2);
        else
                rtl8126_clear_mac_ocp_bit(tp, 0xDC52, BIT_2);

        //RtPciCommittp);

        rtl8126_fiber_set_mdcDownUp(tp);
}

static void
rtl8126_fiber_set_mdio_bit(struct rtl8126_private *tp, bool pu)
{
        rtl8126_fiber_set_mdio_bit_gpio_c45(tp, pu);
}

static u16
rtl8126_fiber_get_mdio_bit_gpio_c45(struct rtl8126_private *tp)
{
        rtl8126_fiber_set_mdcDownUp(tp);

        return !!(rtl8126_mac_ocp_read(tp, 0xDC58) & BIT(2));
}

static u16
rtl8126_fiber_get_mdio_bit(struct rtl8126_private *tp)
{
        return rtl8126_fiber_get_mdio_bit_gpio_c45(tp);
}

static void
rtl8126_fiber_shift_bit_in(struct rtl8126_private *tp, u32 val, int count)
{
        int i;

        for (i = (count - 1); i >= 0; i--)
                rtl8126_fiber_set_mdio_bit(tp, !!(val & BIT(i)));
}

static u16
rtl8126_fiber_shift_bit_out(struct rtl8126_private *tp)
{
        u16 data = 0;
        int i;

        for (i = 15; i >= 0; i--)
                data += (rtl8126_fiber_get_mdio_bit(tp) << i);

        return data;
}

static void
rtl8126_fiber_dir_gpio_c45(struct rtl8126_private *tp, bool output_mode)
{
        if (output_mode)
                rtl8126_set_mac_ocp_bit(tp, 0xDC4C, BIT_2);
        else
                rtl8126_clear_mac_ocp_bit(tp, 0xDC4C, BIT_2);
}

static void
rtl8126_fiber_dir(struct rtl8126_private *tp, bool output_mode)
{
        rtl8126_fiber_dir_gpio_c45(tp, output_mode);
}

//fiber
#define R8126_FIBER_C22 (0)
#define R8126_FIBER_C45 (1)

// sfp opcodes
#define R8126_FIBER_ST (1)
#define R8126_FIBER_OP_W (1)
#define R8126_FIBER_OP_R (2)
#define R8126_FIBER_TA (2)

// sfp C45 opcodes
#define R8126_FIBER_MDIO_C45 (BIT(15))
#define R8126_FIBER_C45_ST (R8126_FIBER_MDIO_C45 | 0)
#define R8126_FIBER_C45_OP_ADDR (R8126_FIBER_MDIO_C45 | 0)
#define R8126_FIBER_C45_OP_W (R8126_FIBER_MDIO_C45 | 1)
#define R8126_FIBER_C45_OP_R (R8126_FIBER_MDIO_C45 | 3)

static void
rtl8126_fiber_cmd(struct rtl8126_private *tp, u32 cmd, u8 phy_addr,
                  u32 reg)
{
        /* change to output mode */
        rtl8126_fiber_dir(tp, 1);

        /* preamble 32bit of 1 */
        rtl8126_fiber_shift_bit_in(tp, UINT_MAX, 32);

        /* start bit */
        if (cmd & R8126_FIBER_MDIO_C45)
                rtl8126_fiber_shift_bit_in(tp, R8126_FIBER_C45_ST, 2);
        else
                rtl8126_fiber_shift_bit_in(tp, R8126_FIBER_ST, 2);

        /* opcode */
        rtl8126_fiber_shift_bit_in(tp, cmd, 2);

        /* phy address */
        rtl8126_fiber_shift_bit_in(tp, phy_addr, 5);

        /* phy reg */
        rtl8126_fiber_shift_bit_in(tp, reg, 5);
}

static u8
rtl8126_fiber_cmdAddr(struct rtl8126_private *tp, u8 phy_addr, u32 reg)
{
        u8 dev_addr = (reg >> 16) & 0x1F;
        u16 addr = (u16)reg;

        rtl8126_fiber_cmd(tp, R8126_FIBER_C45_OP_ADDR, phy_addr, dev_addr);

        /* turn-around(TA) */
        rtl8126_fiber_shift_bit_in(tp, R8126_FIBER_TA, 2);

        rtl8126_fiber_shift_bit_in(tp, addr, 16);

        rtl8126_fiber_dir(tp, 0);

        rtl8126_fiber_get_mdio_bit(tp);

        return dev_addr;
}

static void
rtl8126_fiber_reset_gpio_c45(struct rtl8126_private *tp)
{
        rtl8126_set_mac_ocp_bit(tp, 0xDC4C, (BIT_7 | BIT_2));

        /* init sfp interface */
        rtl8126_clear_mac_ocp_bit(tp, 0xDC52, BIT_7);
        rtl8126_set_mac_ocp_bit(tp, 0xDC52, BIT_2);
}

static void
rtl8126_fiber_write_common(struct rtl8126_private *tp, u16 val)
{
        /* turn-around(TA) */
        rtl8126_fiber_shift_bit_in(tp, R8126_FIBER_TA, 2);

        /* write phy data */
        rtl8126_fiber_shift_bit_in(tp, val, 16);

        /* change to input mode */
        rtl8126_fiber_dir(tp, 0);

        rtl8126_fiber_get_mdio_bit(tp);
}

static void
rtl8126_fiber_mdio_write_gpio_c45(
        struct rtl8126_private *tp,
        u32 reg,
        u16 val,
        u8 phy_addr)
{
        /* opcode write */
        reg = rtl8126_fiber_cmdAddr(tp, phy_addr, reg);
        rtl8126_fiber_cmd(tp, R8126_FIBER_C45_OP_W, phy_addr, reg);

        rtl8126_fiber_write_common(tp, val);
}

static u16
rtl8126_fiber_read_common(struct rtl8126_private *tp)
{
        u16 data = 0;

        /* change to input mode */
        rtl8126_fiber_dir(tp, 0);

        /* TA 0 */
        rtl8126_fiber_get_mdio_bit(tp);

        /* read phy data */
        data = rtl8126_fiber_shift_bit_out(tp);

        rtl8126_fiber_get_mdio_bit(tp);

        return data;
}

static u16
rtl8126_fiber_mdio_read_gpio_c45(
        struct rtl8126_private *tp,
        u32 reg,
        u8 phy_addr)
{
        reg = rtl8126_fiber_cmdAddr(tp, phy_addr, reg);
        rtl8126_fiber_cmd(tp, R8126_FIBER_C45_OP_R, phy_addr, reg);

        return rtl8126_fiber_read_common(tp);
}

void
rtl8126_fiber_mdio_write(
        struct rtl8126_private *tp,
        u32 reg,
        u16 val)
{
        switch(tp->HwFiberStat) {
        case FIBER_STAT_CONNECT_GPO_C45:
                return rtl8126_fiber_mdio_write_gpio_c45(tp, reg, val, 0);
        default:
                return;
        }
}

u16
rtl8126_fiber_mdio_read(
        struct rtl8126_private *tp,
        u32 reg)
{
        switch(tp->HwFiberStat) {
        case FIBER_STAT_CONNECT_GPO_C45:
                return rtl8126_fiber_mdio_read_gpio_c45(tp, reg, 0);
        default:
                return 0xffff;
        }
}

static void
rtl8126_fiber_clear_and_set_phy_bit(struct rtl8126_private *tp, u32 addr, u16 clearmask, u16 setmask)
{
        u16 PhyRegValue;

        PhyRegValue = rtl8126_fiber_mdio_read(tp, addr);
        PhyRegValue &= ~clearmask;
        PhyRegValue |= setmask;
        rtl8126_fiber_mdio_write(tp, addr, PhyRegValue);
}

static void
rtl8126_fiber_clear_phy_bit(struct rtl8126_private *tp, u32 addr, u16 mask)
{
        rtl8126_fiber_clear_and_set_phy_bit(tp, addr, mask, 0);
}

static void
rtl8126_fiber_set_phy_bit(struct rtl8126_private *tp, u32 addr, u16 mask)
{
        rtl8126_fiber_clear_and_set_phy_bit(tp, addr, 0, mask);
}

#define R8126_MAKE_C45_ADDR(_mmd, _addr) (_mmd << 16 | _addr)

static void
rtl8126_fiber_set_ra_8251b(struct rtl8126_private *tp)
{
        struct pci_dev *pdev = tp->pci_dev;
        u16 const svid = pdev->subsystem_vendor;

        rtl8126_fiber_clear_and_set_phy_bit(tp,
                                            R8126_MAKE_C45_ADDR(30, 0x6973),
                                            0x00FF,
                                            (svid == PCI_VENDOR_ID_DELL) ?
                                            0x03 : 0x12);
        rtl8126_fiber_clear_and_set_phy_bit(tp,
                                            R8126_MAKE_C45_ADDR(30, 0x6974),
                                            0x00FF,
                                            0x0005);
        rtl8126_fiber_clear_and_set_phy_bit(tp,
                                            R8126_MAKE_C45_ADDR(30, 0x6975),
                                            0x00FF,
                                            0x0008);
}

static void
rtl8126_fiber_set_ra(struct rtl8126_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8126_RTL8251B:
                rtl8126_fiber_set_ra_8251b(tp);
                break;
        }
}

static void
rtl8126_fiber_phy_reset_8251b(struct rtl8126_private *tp)
{
        u16 PhyRegValue;
        u32 Timeout;

        rtl8126_fiber_set_phy_bit(tp, R8126_MAKE_C45_ADDR(0x01, 0x00), BIT_15);

        Timeout = 0;
        do {
                udelay(1000);

                PhyRegValue = rtl8126_fiber_mdio_read(tp, R8126_MAKE_C45_ADDR(0x01, 0x00));

                Timeout++;
        } while ((PhyRegValue & BIT_15) && (Timeout < 20));
}

static void
rtl8126_fiber_phy_reset(struct rtl8126_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8126_RTL8251B:
                rtl8126_fiber_phy_reset_8251b(tp);
                break;
        }
}

static void
rtl8126_hw_rtl8251b_phy_config(struct rtl8126_private *tp)
{
        rtl8126_fiber_reset_gpio_c45(tp);

        rtl8126_fiber_set_ra(tp);

        rtl8126_fiber_clear_phy_bit(tp, R8126_MAKE_C45_ADDR(0x07, 0x3C), (BIT_2 | BIT_1));
        rtl8126_fiber_clear_phy_bit(tp, R8126_MAKE_C45_ADDR(0x07, 0x3E), (BIT_1 | BIT_0));

        rtl8126_fiber_phy_reset(tp);
}

void
rtl8126_hw_fiber_phy_config(struct rtl8126_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8126_RTL8251B:
                rtl8126_hw_rtl8251b_phy_config(tp);
                break;
        }
}

#define RTL8251B_PHY_ID_1 0x001C
#define RTL8251B_PHY_ID_2 0xC868
static u32
rtl8126_fiber_get_connect_status_8251b(struct rtl8126_private *tp)
{
        int i;
        int const checkcnt = 4;

        rtl8126_fiber_reset_gpio_c45(tp);

        for (i = 0; i < checkcnt; i++) {
                if (RTL8251B_PHY_ID_1 != rtl8126_fiber_mdio_read_gpio_c45(tp, R8126_MAKE_C45_ADDR(0x01, 0x02), 0) ||
                    RTL8251B_PHY_ID_2 != rtl8126_fiber_mdio_read_gpio_c45(tp, R8126_MAKE_C45_ADDR(0x01, 0x03), 0))
                        return FIBER_STAT_DISCONNECT;
        }

        return FIBER_STAT_CONNECT_GPO_C45;
}

static u32
rtl8126_fiber_get_connect_status(struct rtl8126_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8126_RTL8251B:
                return rtl8126_fiber_get_connect_status_8251b(tp);
        default:
                return FIBER_STAT_NOT_CHECKED;
        }
}

void
rtl8126_check_fiber_mode_support(struct rtl8126_private *tp)
{
        switch(tp->mcfg) {
        case CFG_METHOD_3: {
                u8 tmp = (u8)rtl8126_mac_ocp_read(tp, 0xD006);
                if (tmp == 0x03)
                        tp->HwFiberModeVer = FIBER_MODE_RTL8126_RTL8251B;
        }
        break;
        }

        if (HW_FIBER_MODE_ENABLED(tp))
                tp->HwFiberStat = rtl8126_fiber_get_connect_status(tp);
}

unsigned int
rtl8126_fiber_link_ok(struct net_device *dev)
{
        struct rtl8126_private *tp = netdev_priv(dev);
        u16 status;

        switch (tp->HwFiberStat) {
        case FIBER_STAT_CONNECT_GPO_C45:
                status = rtl8126_fiber_mdio_read(tp, R8126_MAKE_C45_ADDR(30, 0x758D));
                if (status != USHRT_MAX && status & BIT_1)
                        return 1;
                else
                        return 0;
                break;
        default:
                return 0;
        }
}
