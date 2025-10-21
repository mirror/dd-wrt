/*
################################################################################
#
# r8127 is the Linux device driver released for Realtek 10 Gigabit Ethernet
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

#include "r8127.h"
#include "r8127_fiber.h"

// sds address
#define R8127_SDS_8127_CMD 0x2348
#define R8127_SDS_8127_ADDR 0x234A
#define R8127_SDS_8127_DATA_IN 0x234C
#define R8127_SDS_8127_DATA_OUT 0x234E

#define R8127_MAKE_SDS_8127_ADDR(_index, _page, _reg) ((_index << 11) | (_page << 5) | _reg)

// sds command
#define R8127_SDS_8127_CMD_IN BIT_0
#define R8127_SDS_8127_WE_IN BIT_1

static bool
rtl8127_wait_8127_sds_cmd_done(struct rtl8127_private *tp)
{
        u32 timeout = 0, waitcount = 100;

        do {
                if (RTL_R16(tp, R8127_SDS_8127_CMD) & R8127_SDS_8127_CMD_IN)
                        udelay(1);
                else
                        return true;
        } while (++timeout < waitcount);

        return false;
}

u16
rtl8127_sds_phy_read_8127(struct rtl8127_private *tp, u16 index, u16 page,
                          u16 reg)
{
        RTL_W16(tp, R8127_SDS_8127_ADDR,
                R8127_MAKE_SDS_8127_ADDR(index, page, reg));
        RTL_W16(tp, R8127_SDS_8127_CMD, R8127_SDS_8127_CMD_IN);

        if (rtl8127_wait_8127_sds_cmd_done(tp))
                return RTL_R16(tp, R8127_SDS_8127_DATA_OUT);
        else
                return 0xffff;
}

void
rtl8127_sds_phy_write_8127(struct rtl8127_private *tp, u16 index, u16 page,
                           u16 reg, u16 val)
{
        RTL_W16(tp, R8127_SDS_8127_DATA_IN, val);
        RTL_W16(tp, R8127_SDS_8127_ADDR,
                R8127_MAKE_SDS_8127_ADDR(index, page, reg));
        RTL_W16(tp, R8127_SDS_8127_CMD,
                R8127_SDS_8127_CMD_IN | R8127_SDS_8127_WE_IN);

        rtl8127_wait_8127_sds_cmd_done(tp);
}

static void
rtl8127_clear_and_set_sds_phy_bit(struct rtl8127_private *tp, u16 index,
                                  u16 page, u16 addr, u16 clearmask,
                                  u16 setmask)
{
        u16 val;

        val = rtl8127_sds_phy_read_8127(tp, index, page, addr);
        val &= ~clearmask;
        val |= setmask;
        rtl8127_sds_phy_write_8127(tp, index, page, addr, val);
}

static void
rtl8127_clear_sds_phy_bit(struct rtl8127_private *tp, u16 index, u16 page,
                          u16 addr, u16 mask)
{
        rtl8127_clear_and_set_sds_phy_bit(tp,
                                          index,
                                          page,
                                          addr,
                                          mask,
                                          0);
}

static void
rtl8127_set_sds_phy_bit(struct rtl8127_private *tp, u16 index,
                        u16 page, u16 addr, u16 mask)
{
        rtl8127_clear_and_set_sds_phy_bit(tp,
                                          index,
                                          page,
                                          addr,
                                          0,
                                          mask);
}

static void
rtl8127_sds_phy_reset_8127(struct rtl8127_private *tp)
{
        RTL_W8(tp, 0x2350, RTL_R8(tp, 0x2350) & ~BIT_0);
        udelay(1);

        RTL_W16(tp, 0x233A, 0x801F);
        RTL_W8(tp, 0x2350, RTL_R8(tp, 0x2350) | BIT_0);
        udelay(10);
}

static void
rtl8127_sds_phy_reset(struct rtl8127_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8127ATF:
                rtl8127_sds_phy_reset_8127(tp);
                break;
        default:
                break;
        }
}

static void
rtl8127_set_sds_phy_caps_1g_8127(struct rtl8127_private *tp)
{
        u16 val;

        rtl8127_set_sds_phy_bit(tp, 0, 1, 31, BIT_3);
        rtl8127_clear_and_set_sds_phy_bit(tp,
                                          0,
                                          2,
                                          0,
                                          BIT_13 | BIT_12 | BIT_6,
                                          BIT_12 | BIT_6);
        RTL_W16(tp, 0x233A, 0x8004);

        val = RTL_R16(tp, 0x233E);
        val &= (BIT_13 | BIT_12 | BIT_1 | BIT_0);
        val |= BIT_1;
        RTL_W16(tp, 0x233E, val);

        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC40A, 0x0);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC466, 0x0);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC808, 0x0);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC80A, 0x0);
        rtl8127_clear_and_set_eth_phy_ocp_bit(tp,
                                              0xC804,
                                              0x000F,
                                              0x000C);
}

static void
rtl8127_sds_phy_exit_1g_8127(struct rtl8127_private *tp)
{
        rtl8127_clear_sds_phy_bit(tp, 0, 1, 31, BIT_3);
        rtl8127_clear_and_set_sds_phy_bit(tp,
                                          0,
                                          2,
                                          0,
                                          BIT_13 | BIT_12 | BIT_6,
                                          BIT_6);

        rtl8127_sds_phy_reset(tp);
}

static void
rtl8127_set_sds_phy_caps_10g_8127(struct rtl8127_private *tp)
{
        u16 val;

        RTL_W16(tp, 0x233A, 0x801A);

        val = RTL_R16(tp, 0x233E);
        val &= (BIT_13 | BIT_12 | BIT_1 | BIT_0);
        val |= BIT_12;
        RTL_W16(tp, 0x233E, val);

        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC40A, 0x0);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC466, 0x3);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC808, 0x0);
        rtl8127_mdio_direct_write_phy_ocp(tp, 0xC80A, 0x0);
        rtl8127_clear_and_set_eth_phy_ocp_bit(tp,
                                              0xC804,
                                              0x000F,
                                              0x000C);
}

static void
rtl8127_set_sds_phy_caps_8127(struct rtl8127_private *tp)
{
        rtl8127_sds_phy_exit_1g_8127(tp);

        switch (tp->speed) {
        case SPEED_10000:
                rtl8127_set_sds_phy_caps_10g_8127(tp);
                break;
        case SPEED_1000:
                rtl8127_set_sds_phy_caps_1g_8127(tp);
                break;
        default:
                break;
        }
}

static void
rtl8127_set_sds_phy_caps(struct rtl8127_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8127ATF:
                rtl8127_set_sds_phy_caps_8127(tp);
                break;
        default:
                break;
        }
}

static void
rtl8127_hw_sds_phy_config(struct rtl8127_private *tp)
{
        rtl8127_set_sds_phy_caps(tp);
}

void
rtl8127_hw_fiber_phy_config(struct rtl8127_private *tp)
{
        switch (tp->HwFiberModeVer) {
        case FIBER_MODE_RTL8127ATF:
                rtl8127_hw_sds_phy_config(tp);
                break;
        default:
                break;
        }
}

bool
rtl8127_check_fiber_mode_support(struct rtl8127_private *tp)
{
        switch(tp->mcfg) {
        case CFG_METHOD_2: {
                u8 tmp = (u8)rtl8127_mac_ocp_read(tp, 0xD006);
                if (tmp == 0x07)
                        tp->HwFiberModeVer = FIBER_MODE_RTL8127ATF;
        }
        break;
        default:
                break;
        }

        return (HW_FIBER_MODE_ENABLED(tp)) ? true : false;
}
