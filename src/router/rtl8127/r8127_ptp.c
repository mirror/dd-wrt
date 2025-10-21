// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/in.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>

#include "r8127.h"
#include "r8127_ptp.h"

static void rtl8127_wait_clkadj_ready(struct rtl8127_private *tp)
{
        int i;

        for (i = 0; i < R8127_CHANNEL_WAIT_COUNT; i++)
                if (!(rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CLK_CFG_8126) & CLKADJ_MODE_SET))
                        break;
}

static void rtl8127_set_clkadj_mode(struct rtl8127_private *tp, u16 cmd)
{
        rtl8127_clear_and_set_eth_phy_ocp_bit(tp,
                                              PTP_CLK_CFG_8126,
                                              BIT_3 | BIT_2 | BIT_1,
                                              CLKADJ_MODE_SET | cmd);

        rtl8127_wait_clkadj_ready(tp);
}

static int _rtl8127_phc_gettime(struct rtl8127_private *tp, struct timespec64 *ts64)
{
        unsigned long flags;

        r8127_spin_lock(&tp->phy_lock, flags);

        //Direct Read
        rtl8127_set_clkadj_mode(tp, DIRECT_READ);

        /* nanoseconds */
        //Ns[29:16] E414[13:0]
        ts64->tv_nsec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_NS_HI_8126) & 0x3fff;
        ts64->tv_nsec <<= 16;
        //Ns[15:0]  E412[15:0]
        ts64->tv_nsec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_NS_LO_8126);


        /* seconds */
        //S[47:32] E41A[15:0]
        ts64->tv_sec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_HI_8126);
        ts64->tv_sec <<= 16;
        //S[31:16] E418[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_MI_8126);
        ts64->tv_sec <<= 16;
        //S[15:0]  E416[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_LO_8126);

        r8127_spin_unlock(&tp->phy_lock, flags);

        return 0;
}

static int _rtl8127_phc_settime(struct rtl8127_private *tp, const struct timespec64 *ts64)
{
        unsigned long flags;

        r8127_spin_lock(&tp->phy_lock, flags);

        /* nanoseconds */
        //Ns[15:0]  E412[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_LO_8126, ts64->tv_nsec);
        //Ns[29:16] E414[13:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_HI_8126, (ts64->tv_nsec & 0x3fff0000) >> 16);

        /* seconds */
        //S[15:0]  E416[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_LO_8126, ts64->tv_sec);
        //S[31:16] E418[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_MI_8126, (ts64->tv_sec >> 16));
        //S[47:32] E41A[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_HI_8126, (ts64->tv_sec >> 32));

        //Direct Write
        rtl8127_set_clkadj_mode(tp, DIRECT_WRITE);

        r8127_spin_unlock(&tp->phy_lock, flags);

        return 0;
}

static int _rtl8127_phc_adjtime(struct rtl8127_private *tp, s64 delta)
{
        unsigned long flags;
        struct timespec64 d;
        bool negative;
        u64 tohw;
        u32 nsec;
        u64 sec;

        if (delta < 0) {
                negative = true;
                tohw = -delta;
        } else {
                negative = false;
                tohw = delta;
        }

        d = ns_to_timespec64(tohw);

        nsec = d.tv_nsec;
        sec = d.tv_sec;

        nsec &= 0x3fffffff;
        sec &= 0x0000ffffffffffff;

        r8127_spin_lock(&tp->phy_lock, flags);

        /* nanoseconds */
        //Ns[15:0]  E412[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_LO_8126, nsec);
        //Ns[29:16] E414[13:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_HI_8126, (nsec >> 16));

        /* seconds */
        //S[15:0]  E416[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_LO_8126, sec);
        //S[31:16] E418[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_MI_8126, (sec >> 16));
        //S[47:32] E41A[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_S_HI_8126, (sec >> 32));

        if (negative)
                rtl8127_set_clkadj_mode(tp, DECREMENT_STEP);
        else
                rtl8127_set_clkadj_mode(tp, INCREMENT_STEP);

        r8127_spin_unlock(&tp->phy_lock, flags);

        return 0;
}

static int rtl8127_phc_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        int ret;

        //netif_info(tp, drv, tp->dev, "phc adjust time\n");

        ret = _rtl8127_phc_adjtime(tp, delta);

        return ret;
}

/*
 * delta = delta * 10^6 ppm = delta * 10^9 ppb (in this equation ppm and ppb are not variable)
 *
 * in adjfreq ppb is a variable
 * ppb = delta * 10^9
 * delta = ppb / 10^9
 * rate_value = |delta| * 2^32 = |ppb| / 10^9 * 2^32 = (|ppb| << 32) / 10^9
 */
static int _rtl8127_phc_adjfreq(struct ptp_clock_info *ptp, s32 ppb)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        unsigned long flags;
        u32 rate_value;

        if (ppb < 0) {
                rate_value = ((u64)-ppb << 32) / 1000000000;
                rate_value = ~rate_value + 1;
        } else
                rate_value = ((u64)ppb << 32) / 1000000000;

        r8127_spin_lock(&tp->phy_lock, flags);

        /* nanoseconds */
        //Ns[15:0]  E412[15:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_LO_8126, rate_value);
        //Ns[22:16] E414[13:0]
        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CFG_NS_HI_8126, (rate_value & 0x003f0000) >> 16);

        rtl8127_set_clkadj_mode(tp, RATE_WRITE);

        r8127_spin_unlock(&tp->phy_lock, flags);

        return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,2,0)
static int rtl8127_ptp_adjfine(struct ptp_clock_info *ptp, long scaled_ppm)
{
        s32 ppb = scaled_ppm_to_ppb(scaled_ppm);

        if (ppb > ptp->max_adj || ppb < -ptp->max_adj)
                return -EINVAL;

        _rtl8127_phc_adjfreq(ptp, ppb);

        return 0;
}

#else
static int rtl8127_phc_adjfreq(struct ptp_clock_info *ptp, s32 delta)
{
        //struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);

        //netif_info(tp, drv, tp->dev, "phc adjust freq\n");

        if (delta > ptp->max_adj || delta < -ptp->max_adj)
                return -EINVAL;

        _rtl8127_phc_adjfreq(ptp, delta);

        return 0;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6,2,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
static int rtl8127_phc_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts64,
                               struct ptp_system_timestamp *sts)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        int ret;

        //netif_info(tp, drv, tp->dev, "phc get ts\n");

        ptp_read_system_prets(sts);
        ret = _rtl8127_phc_gettime(tp, ts64);
        ptp_read_system_postts(sts);

        return ret;
}
#else
static int rtl8127_phc_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts64)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        int ret;

        //netif_info(tp, drv, tp->dev, "phc get ts\n");

        ret = _rtl8127_phc_gettime(tp, ts64);

        return ret;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0) */

static int rtl8127_phc_settime(struct ptp_clock_info *ptp,
                               const struct timespec64 *ts64)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        int ret;

        //netif_info(tp, drv, tp->dev, "phc set ts\n");

        ret = _rtl8127_phc_settime(tp, ts64);

        return ret;
}

static void _rtl8127_phc_enable(struct ptp_clock_info *ptp,
                                struct ptp_clock_request *rq, int on)
{
        struct rtl8127_private *tp = container_of(ptp, struct rtl8127_private, ptp_clock_info);
        unsigned long flags;
        u16 phy_ocp_data;

        if (on) {
                tp->pps_enable = 1;
                rtl8127_clear_mac_ocp_bit(tp, 0xDC00, BIT_6);
                rtl8127_clear_mac_ocp_bit(tp, 0xDC20, BIT_1);

                r8127_spin_lock(&tp->phy_lock, flags);

                /* Set periodic pulse 1pps */
                /* E432[8:0] = 0x017d */
                phy_ocp_data = rtl8127_mdio_direct_read_phy_ocp(tp, 0xE432);
                phy_ocp_data &= 0xFE00;
                phy_ocp_data |= 0x017d;
                rtl8127_mdio_direct_write_phy_ocp(tp, 0xE432, phy_ocp_data);

                rtl8127_mdio_direct_write_phy_ocp(tp, 0xE434, 0x7840);

                /* E436[8:0] = 0xbe */
                phy_ocp_data = rtl8127_mdio_direct_read_phy_ocp(tp, 0xE436);
                phy_ocp_data &= 0xFE00;
                phy_ocp_data |= 0xbe;
                rtl8127_mdio_direct_write_phy_ocp(tp, 0xE436, phy_ocp_data);

                rtl8127_mdio_direct_write_phy_ocp(tp, 0xE438, 0xbc20);

                r8127_spin_unlock(&tp->phy_lock, flags);

                /* start hrtimer */
                hrtimer_start(&tp->pps_timer, 1000000000, HRTIMER_MODE_REL);
        } else
                tp->pps_enable = 0;
}

static int rtl8127_phc_enable(struct ptp_clock_info *ptp,
                              struct ptp_clock_request *rq, int on)
{
        switch (rq->type) {
        case PTP_CLK_REQ_PPS:
                _rtl8127_phc_enable(ptp, rq, on);
                return 0;
        default:
                return -EOPNOTSUPP;
        }
}

static void rtl8127_ptp_enable_config(struct rtl8127_private *tp)
{
        if (tp->syncE_en)
                rtl8127_set_eth_phy_ocp_bit(tp, PTP_SYNCE_CTL, BIT_0);
        else
                rtl8127_clear_eth_phy_ocp_bit(tp, PTP_SYNCE_CTL, BIT_0);

        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_CTL, PTP_CTL_TYPE_3 | BIT_12);

        rtl8127_set_eth_phy_ocp_bit(tp, 0xA640, BIT_15);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,11,0)
int rtl8127_get_ts_info(struct net_device *netdev,
                        struct ethtool_ts_info *info)
#else
int rtl8127_get_ts_info(struct net_device *netdev,
                        struct kernel_ethtool_ts_info *info)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(6,11,0) */
{
        struct rtl8127_private *tp = netdev_priv(netdev);

        /* we always support timestamping disabled */
        info->rx_filters = BIT(HWTSTAMP_FILTER_NONE);

        if (tp->HwSuppPtpVer == 0)
                return ethtool_op_get_ts_info(netdev, info);

        info->so_timestamping =  SOF_TIMESTAMPING_TX_SOFTWARE |
                                 SOF_TIMESTAMPING_RX_SOFTWARE |
                                 SOF_TIMESTAMPING_SOFTWARE |
                                 SOF_TIMESTAMPING_TX_HARDWARE |
                                 SOF_TIMESTAMPING_RX_HARDWARE |
                                 SOF_TIMESTAMPING_RAW_HARDWARE;

        if (tp->ptp_clock)
                info->phc_index = ptp_clock_index(tp->ptp_clock);
        else
                info->phc_index = -1;

        info->tx_types = BIT(HWTSTAMP_TX_OFF) | BIT(HWTSTAMP_TX_ON);

        info->rx_filters = BIT(HWTSTAMP_FILTER_NONE) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_EVENT) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_L4_EVENT) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_SYNC) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_L4_SYNC) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_DELAY_REQ) |
                           BIT(HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ);

        return 0;
}

static const struct ptp_clock_info rtl_ptp_clock_info = {
        .owner      = THIS_MODULE,
        .n_alarm    = 0,
        .n_ext_ts   = 0,
        .n_per_out  = 0,
        .n_pins     = 0,
        .pps        = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,2,0)
        .adjfine   = rtl8127_ptp_adjfine,
#else
        .adjfreq    = rtl8127_phc_adjfreq,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6,2,0) */
        .adjtime    = rtl8127_phc_adjtime,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
        .gettimex64 = rtl8127_phc_gettime,
#else
        .gettime64  = rtl8127_phc_gettime,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0) */

        .settime64  = rtl8127_phc_settime,
        .enable     = rtl8127_phc_enable,
};

static u16 rtl8127_ptp_get_tx_msgtype(struct rtl8127_private *tp)
{
        u16 tx_ts_ready = 0;
        int i;

        for (i = 0; i < R8127_CHANNEL_WAIT_COUNT; i++) {
                tx_ts_ready = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_STA) & 0xF000;
                if (tx_ts_ready)
                        break;
        }

        switch (tx_ts_ready) {
        case TX_TS_PDLYRSP_RDY:
                return PTP_MSGTYPE_PDELAY_RESP;
        case TX_TS_PDLYREQ_RDY:
                return PTP_MSGTYPE_PDELAY_REQ;
        case TX_TS_DLYREQ_RDY:
                return PTP_MSGTYPE_DELAY_REQ;
        case TX_TS_SYNC_RDY:
        default:
                return PTP_MSGTYPE_SYNC;
        }
}

/*
static u16 rtl8127_ptp_get_rx_msgtype(struct rtl8127_private *tp)
{
        u16 rx_ts_ready = 0;
        int i;

        for (i = 0; i < R8127_CHANNEL_WAIT_COUNT; i++) {
                rx_ts_ready = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_STA) & 0x0F00;
                if (rx_ts_ready)
                        break;
        }

        switch (rx_ts_ready) {
        case RX_TS_PDLYRSP_RDY:
                return PTP_MSGTYPE_PDELAY_RESP;
        case RX_TS_PDLYREQ_RDY:
                return PTP_MSGTYPE_PDELAY_REQ;
        case RX_TS_DLYREQ_RDY:
                return PTP_MSGTYPE_DELAY_REQ;
        case RX_TS_SYNC_RDY:
        default:
                return PTP_MSGTYPE_SYNC;
        }
}
*/

static void rtl8127_wait_trx_ts_ready(struct rtl8127_private *tp)
{
        int i;

        for (i = 0; i < R8127_CHANNEL_WAIT_COUNT; i++)
                if (!(rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_STA) & TRX_TS_RD))
                        break;
}

static void rtl8127_set_trx_ts_cmd(struct rtl8127_private *tp, u16 cmd)
{
        rtl8127_clear_and_set_eth_phy_ocp_bit(tp,
                                              PTP_TRX_TS_STA,
                                              TRXTS_SEL | BIT_3 | BIT_2,
                                              TRX_TS_RD | cmd);

        rtl8127_wait_trx_ts_ready(tp);
}

static void rtl8127_ptp_egresstime(struct rtl8127_private *tp, struct timespec64 *ts64)
{
        u16 msgtype;

        msgtype = rtl8127_ptp_get_tx_msgtype(tp);

        msgtype <<= 2;

        rtl8127_set_trx_ts_cmd(tp, (msgtype | BIT_4));

        /* nanoseconds */
        //Ns[29:16] E448[13:0]
        ts64->tv_nsec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_NS_HI) & 0x3fff;
        ts64->tv_nsec <<= 16;
        //Ns[15:0]  E446[15:0]
        ts64->tv_nsec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_NS_LO);

        /* seconds */
        //S[47:32] E44E[15:0]
        ts64->tv_sec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_HI);
        ts64->tv_sec <<= 16;
        //S[31:16] E44C[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_MI);
        ts64->tv_sec <<= 16;
        //S[15:0]  E44A[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_LO);
}

static void rtl8127_ptp_ingresstime(struct rtl8127_private *tp, struct timespec64 *ts64, u8 type)
{
        u16 msgtype;

        switch (type) {
        case PTP_MSGTYPE_PDELAY_RESP:
        case PTP_MSGTYPE_PDELAY_REQ:
        case PTP_MSGTYPE_DELAY_REQ:
        case PTP_MSGTYPE_SYNC:
                msgtype = type << 2;
                break;
        default:
                return;
        }

        rtl8127_set_trx_ts_cmd(tp, (TRXTS_SEL | msgtype | BIT_4));

        /* nanoseconds */
        //Ns[29:16] E448[13:0]
        ts64->tv_nsec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_NS_HI) & 0x3fff;
        ts64->tv_nsec <<= 16;
        //Ns[15:0]  E446[15:0]
        ts64->tv_nsec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_NS_LO);

        /* seconds */
        //S[47:32] E44E[15:0]
        ts64->tv_sec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_HI);
        ts64->tv_sec <<= 16;
        //S[31:16] E44C[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_MI);
        ts64->tv_sec <<= 16;
        //S[15:0]  E44A[15:0]
        ts64->tv_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_TRX_TS_S_LO);
}

static void rtl8127_ptp_tx_hwtstamp(struct rtl8127_private *tp)
{
        struct sk_buff *skb = tp->ptp_tx_skb;
        struct skb_shared_hwtstamps shhwtstamps = { 0 };
        struct timespec64 ts64;

        rtl8127_mdio_direct_write_phy_ocp(tp, PTP_INSR, TX_TX_INTR);

        rtl8127_ptp_egresstime(tp, &ts64);

        /* Upper 32 bits contain s, lower 32 bits contain ns. */
        shhwtstamps.hwtstamp = ktime_set(ts64.tv_sec,
                                         ts64.tv_nsec);

        /* Clear the lock early before calling skb_tstamp_tx so that
         * applications are not woken up before the lock bit is clear. We use
         * a copy of the skb pointer to ensure other threads can't change it
         * while we're notifying the stack.
         */
        tp->ptp_tx_skb = NULL;
        clear_bit_unlock(__RTL8127_PTP_TX_IN_PROGRESS, &tp->state);

        /* Notify the stack and free the skb after we've unlocked */
        skb_tstamp_tx(skb, &shhwtstamps);
        dev_kfree_skb_any(skb);
}

#define RTL8127_PTP_TX_TIMEOUT      (HZ * 15)
static void rtl8127_ptp_tx_work(struct work_struct *work)
{
        struct rtl8127_private *tp = container_of(work, struct rtl8127_private,
                                     ptp_tx_work);
        unsigned long flags;
        bool tx_intr;

        if (!tp->ptp_tx_skb)
                return;

        if (time_is_before_jiffies(tp->ptp_tx_start +
                                   RTL8127_PTP_TX_TIMEOUT)) {
                dev_kfree_skb_any(tp->ptp_tx_skb);
                tp->ptp_tx_skb = NULL;
                clear_bit_unlock(__RTL8127_PTP_TX_IN_PROGRESS, &tp->state);
                tp->tx_hwtstamp_timeouts++;
                /* Clear the tx valid bit in TSYNCTXCTL register to enable
                 * interrupt
                 */
                r8127_spin_lock(&tp->phy_lock, flags);
                rtl8127_mdio_direct_write_phy_ocp(tp, PTP_INSR, TX_TX_INTR);
                r8127_spin_unlock(&tp->phy_lock, flags);
                return;
        }

        r8127_spin_lock(&tp->phy_lock, flags);
        if (rtl8127_mdio_direct_read_phy_ocp(tp, PTP_INSR) & TX_TX_INTR) {
                tx_intr = true;
                rtl8127_ptp_tx_hwtstamp(tp);
        } else {
                tx_intr = false;
        }
        r8127_spin_unlock(&tp->phy_lock, flags);

        if (!tx_intr) {
                /* reschedule to check later */
                schedule_work(&tp->ptp_tx_work);
        }
}

static int rtl8127_hwtstamp_enable(struct rtl8127_private *tp, bool enable)
{
        unsigned long flags;

        r8127_spin_lock(&tp->phy_lock, flags);

        if (enable) {
                //trx timestamp interrupt enable
                rtl8127_set_eth_phy_ocp_bit(tp, PTP_INER, BIT_2 | BIT_3);

                //set isr clear mode
                rtl8127_set_eth_phy_ocp_bit(tp, PTP_GEN_CFG, BIT_0);

                //clear ptp isr
                rtl8127_mdio_direct_write_phy_ocp(tp, PTP_INSR, 0xFFFF);

                //enable ptp
                rtl8127_ptp_enable_config(tp);

                //rtl8127_set_local_time(tp);
        } else {
                /* trx timestamp interrupt disable */
                rtl8127_clear_eth_phy_ocp_bit(tp, PTP_INER, BIT_2 | BIT_3);

                /* disable ptp */
                rtl8127_clear_eth_phy_ocp_bit(tp, PTP_SYNCE_CTL, BIT_0);
                rtl8127_clear_eth_phy_ocp_bit(tp, PTP_CTL, BIT_0);
                rtl8127_set_eth_phy_ocp_bit(tp, 0xA640, BIT_15);
        }

        r8127_spin_unlock(&tp->phy_lock, flags);

        return 0;
}

void rtl8127_set_local_time(struct rtl8127_private *tp)
{
        struct timespec64 ts64;
        //set system time
        ktime_get_real_ts64(&ts64);
        _rtl8127_phc_settime(tp, &ts64);
}

static long rtl8127_ptp_create_clock(struct rtl8127_private *tp)
{
        struct net_device *netdev = tp->dev;
        long err;

        if (!IS_ERR_OR_NULL(tp->ptp_clock))
                return 0;

        if (tp->HwSuppPtpVer == 0) {
                tp->ptp_clock = NULL;
                return -EOPNOTSUPP;
        }

        tp->ptp_clock_info = rtl_ptp_clock_info;
        tp->ptp_clock_info.max_adj = 488281;//0x1FFFFF * 10^9 / 2^32

        snprintf(tp->ptp_clock_info.name, sizeof(tp->ptp_clock_info.name),
                 "%pm", tp->dev->dev_addr);
        tp->ptp_clock = ptp_clock_register(&tp->ptp_clock_info, &tp->pci_dev->dev);
        if (IS_ERR(tp->ptp_clock)) {
                err = PTR_ERR(tp->ptp_clock);
                tp->ptp_clock = NULL;
                netif_err(tp, drv, tp->dev, "ptp_clock_register failed\n");
                return err;
        } else
                netif_info(tp, drv, tp->dev, "registered PHC device on %s\n", netdev->name);

        return 0;
}

static enum hrtimer_restart
rtl8127_hrtimer_for_pps(struct hrtimer *timer) {
        struct rtl8127_private *tp = container_of(timer, struct rtl8127_private, pps_timer);
        u16 tai_cfg = BIT_8 | BIT_3 | BIT_1 | BIT_0;
        s64 pps_sec;

        if (tp->pps_enable)
        {
                unsigned long flags;

                r8127_spin_lock(&tp->phy_lock, flags);

                //Direct Read
                rtl8127_set_clkadj_mode(tp, DIRECT_READ);

                pps_sec = rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_HI_8126);
                pps_sec <<= 16;
                pps_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_MI_8126);
                pps_sec <<= 16;
                pps_sec |= rtl8127_mdio_direct_read_phy_ocp(tp, PTP_CFG_S_LO_8126);
                pps_sec++;

                //E42A[15:0]
                rtl8127_mdio_direct_write_phy_ocp(tp, PTP_TAI_TS_S_LO, pps_sec & 0xffff);
                //E42C[31:16]
                rtl8127_mdio_direct_write_phy_ocp(tp, PTP_TAI_TS_S_HI, (pps_sec & 0xffff0000) >> 16);
                //Periodic Tai start
                rtl8127_mdio_direct_write_phy_ocp(tp, PTP_TAI_CFG, tai_cfg);

                r8127_spin_unlock(&tp->phy_lock, flags);

                hrtimer_forward_now(&tp->pps_timer, 1000000000); //rekick
                return HRTIMER_RESTART;
        } else
                return HRTIMER_NORESTART;
}

void rtl8127_ptp_reset(struct rtl8127_private *tp)
{
        if (!tp->ptp_clock)
                return;

        netif_info(tp, drv, tp->dev, "reset PHC clock\n");

        rtl8127_hwtstamp_enable(tp, false);
}

void rtl8127_ptp_init(struct rtl8127_private *tp)
{
        /* obtain a PTP device, or re-use an existing device */
        if (rtl8127_ptp_create_clock(tp))
                return;

        /* we have a clock so we can initialize work now */
        INIT_WORK(&tp->ptp_tx_work, rtl8127_ptp_tx_work);

        /* init a hrtimer for pps */
        tp->pps_enable = 0;
        hrtimer_init(&tp->pps_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        tp->pps_timer.function = rtl8127_hrtimer_for_pps;

        /* reset the PTP related hardware bits */
        rtl8127_ptp_reset(tp);

        return;
}

void rtl8127_ptp_suspend(struct rtl8127_private *tp)
{
        if (!tp->ptp_clock)
                return;

        netif_info(tp, drv, tp->dev, "suspend PHC clock\n");

        rtl8127_hwtstamp_enable(tp, false);

        /* ensure that we cancel any pending PTP Tx work item in progress */
        cancel_work_sync(&tp->ptp_tx_work);

        hrtimer_cancel(&tp->pps_timer);
}

void rtl8127_ptp_stop(struct rtl8127_private *tp)
{
        struct net_device *netdev = tp->dev;

        netif_info(tp, drv, tp->dev, "stop PHC clock\n");

        /* first, suspend PTP activity */
        rtl8127_ptp_suspend(tp);

        /* disable the PTP clock device */
        if (tp->ptp_clock) {
                ptp_clock_unregister(tp->ptp_clock);
                tp->ptp_clock = NULL;
                netif_info(tp, drv, tp->dev, "removed PHC on %s\n",
                           netdev->name);
        }
}

static int rtl8127_set_tstamp(struct net_device *netdev, struct ifreq *ifr)
{
        struct rtl8127_private *tp = netdev_priv(netdev);
        struct hwtstamp_config config;
        bool hwtstamp = 0;

        //netif_info(tp, drv, tp->dev, "ptp set ts\n");

        if (copy_from_user(&config, ifr->ifr_data, sizeof(config)))
                return -EFAULT;

        if (config.flags)
                return -EINVAL;

        switch (config.tx_type) {
        case HWTSTAMP_TX_ON:
                hwtstamp = 1;
                break;
        case HWTSTAMP_TX_OFF:
                break;
        case HWTSTAMP_TX_ONESTEP_SYNC:
        default:
                return -ERANGE;
        }

        switch (config.rx_filter) {
        case HWTSTAMP_FILTER_PTP_V2_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
        case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
        case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
                config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
                hwtstamp = 1;
                tp->flags |= RTL_FLAG_RX_HWTSTAMP_ENABLED;
                break;
        case HWTSTAMP_FILTER_NONE:
                tp->flags &= ~RTL_FLAG_RX_HWTSTAMP_ENABLED;
                break;
        default:
                tp->flags &= ~RTL_FLAG_RX_HWTSTAMP_ENABLED;
                return -ERANGE;
        }

        if (tp->hwtstamp_config.tx_type != config.tx_type ||
            tp->hwtstamp_config.rx_filter != config.rx_filter) {
                tp->hwtstamp_config = config;

                rtl8127_hwtstamp_enable(tp, hwtstamp);
        }

        return copy_to_user(ifr->ifr_data, &config,
                            sizeof(config)) ? -EFAULT : 0;
}

static int rtl8127_get_tstamp(struct net_device *netdev, struct ifreq *ifr)
{
        struct rtl8127_private *tp = netdev_priv(netdev);

        //netif_info(tp, drv, tp->dev, "ptp get ts\n");

        return copy_to_user(ifr->ifr_data, &tp->hwtstamp_config,
                            sizeof(tp->hwtstamp_config)) ? -EFAULT : 0;
}

int rtl8127_ptp_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
        int ret;

        //netif_info(tp, drv, tp->dev, "ptp ioctl\n");

        switch (cmd) {
#ifdef ENABLE_PTP_SUPPORT
        case SIOCSHWTSTAMP:
                ret = rtl8127_set_tstamp(netdev, ifr);
                break;
        case SIOCGHWTSTAMP:
                ret = rtl8127_get_tstamp(netdev, ifr);
                break;
#endif
        default:
                ret = -EOPNOTSUPP;
                break;
        }

        return ret;
}

static void rtl8127_rx_ptp_pktstamp(struct rtl8127_private *tp, struct sk_buff *skb, u8 type)
{
        struct timespec64 ts64;
        unsigned long flags;

        r8127_spin_lock(&tp->phy_lock, flags);

        rtl8127_ptp_ingresstime(tp, &ts64, type);

        r8127_spin_unlock(&tp->phy_lock, flags);

        skb_hwtstamps(skb)->hwtstamp = ktime_set(ts64.tv_sec, ts64.tv_nsec);

        return;
}

void rtl8127_rx_ptp_timestamp(struct rtl8127_private *tp, struct sk_buff *skb)
{
        unsigned int ptp_class;
        struct ptp_header *hdr;
        u8 msgtype;

        ptp_class = ptp_classify_raw(skb);
        if (ptp_class == PTP_CLASS_NONE)
                return;

        skb_reset_mac_header(skb);
        hdr = ptp_parse_header(skb, ptp_class);
        if (unlikely(!hdr))
                return;

        msgtype = ptp_get_msgtype(hdr, ptp_class);
        rtl8127_rx_ptp_pktstamp(tp, skb, msgtype);

        return;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
struct ptp_header *ptp_parse_header(struct sk_buff *skb, unsigned int type)
{
        u8 *ptr = skb_mac_header(skb);

        if (type & PTP_CLASS_VLAN)
                //ptr += VLAN_HLEN;
                ptr += 4;

        switch (type & PTP_CLASS_PMASK) {
        case PTP_CLASS_IPV4:
                ptr += IPV4_HLEN(ptr) + UDP_HLEN;
                break;
        case PTP_CLASS_IPV6:
                ptr += IP6_HLEN + UDP_HLEN;
                break;
        case PTP_CLASS_L2:
                break;
        default:
                return NULL;
        }

        ptr += ETH_HLEN;

        /* Ensure that the entire header is present in this packet. */
        if (ptr + sizeof(struct ptp_header) > skb->data + skb->len)
                return NULL;

        return (struct ptp_header *)ptr;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0) */
