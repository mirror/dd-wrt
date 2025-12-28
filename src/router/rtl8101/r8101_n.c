// SPDX-License-Identifier: GPL-2.0-only
/*
################################################################################
#
# r8101 is the Linux device driver released for Realtek Fast Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2024 Realtek Semiconductor Corp. All rights reserved.
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

/*
This driver is modified from r8169.c in Linux kernel 2.6.18
*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/ip.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#endif
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/timer.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
#include <linux/prefetch.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
#include <linux/mdio.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,10)
#include <net/gso.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,10) */

#include <asm/io.h>
#include <asm/irq.h>

#include "r8101.h"
#include "rtl_eeprom.h"
#include "rtl_ethtool.h"
#include "rtltool.h"

#ifdef ENABLE_R8101_PROCFS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#define _R(NAME,MAC,MASK) \
	{ .name = NAME, .mcfg = MAC, .RxConfigMask = MASK }

static const struct {
        const char *name;
        u8 mcfg;
        u32 RxConfigMask;	/* Clears the bits supported by this chip */
} rtl_chip_info[] = {
        _R("RTL8101E", CFG_METHOD_1, 0xff7e1880),
        _R("RTL8101E", CFG_METHOD_2, 0xff7e1880),
        _R("RTL8101E", CFG_METHOD_3, 0xff7e1880),
        _R("RTL8102E", CFG_METHOD_4, 0xff7e1880),
        _R("RTL8102E", CFG_METHOD_5, 0xff7e1880),
        _R("RTL8103E", CFG_METHOD_6, 0xff7e1880),
        _R("RTL8103E", CFG_METHOD_7, 0xff7e1880),
        _R("RTL8103E", CFG_METHOD_8, 0xff7e1880),
        _R("RTL8401", CFG_METHOD_9, 0xff7e1880),
        _R("RTL8105E", CFG_METHOD_10, 0xff7e1880),
        _R("RTL8105E", CFG_METHOD_11, 0xff7e1880),
        _R("RTL8105E", CFG_METHOD_12, 0xff7e1880),
        _R("RTL8105E", CFG_METHOD_13, 0xff7e1880),
        _R("RTL8402", CFG_METHOD_14, 0xff7e1880),
        _R("RTL8106E", CFG_METHOD_15, 0xff7e1880),
        _R("RTL8106E", CFG_METHOD_16, 0xff7e1880),
        _R("RTL8106EUS", CFG_METHOD_17, 0xff7e5880),
        _R("RTL8107E", CFG_METHOD_18, 0xff7e5880),
        _R("RTL8107E", CFG_METHOD_19, 0xff7e5880),
        _R("RTL8107E", CFG_METHOD_20, 0xff7e5880),
        _R("Unknown", CFG_METHOD_DEFAULT, 0xff7e5880)
};
#undef _R

static struct pci_device_id rtl8101_pci_tbl[] = {
        { PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8136), },
        {0,},
};

MODULE_DEVICE_TABLE(pci, rtl8101_pci_tbl);

static int rx_copybreak = 0;
static int use_dac = 1;
static int timer_count = 0x2600;

static struct {
        u32 msg_enable;
} debug = { -1 };

static unsigned int speed_mode = SPEED_100;
static unsigned int duplex_mode = DUPLEX_FULL;
static unsigned int autoneg_mode = AUTONEG_ENABLE;
static unsigned int advertising_mode =  ADVERTISED_10baseT_Half |
                                        ADVERTISED_10baseT_Full |
                                        ADVERTISED_100baseT_Half |
                                        ADVERTISED_100baseT_Full;
#ifdef CONFIG_ASPM
static int aspm = 1;
#else
static int aspm = 0;
#endif
#ifdef ENABLE_S5WOL
static int s5wol = 1;
#else
static int s5wol = 0;
#endif
#ifdef ENABLE_S5_KEEP_CURR_MAC
static int s5_keep_curr_mac = 1;
#else
static int s5_keep_curr_mac = 0;
#endif
#ifdef ENABLE_EEE
static int eee_enable = 1;
#else
static int eee_enable = 0;
#endif
#ifdef CONFIG_SOC_LAN
static ulong hwoptimize = HW_PATCH_SOC_LAN;
#else
static ulong hwoptimize = 0;
#endif
#ifdef ENABLE_S0_MAGIC_PACKET
static int s0_magic_packet = 1;
#else
static int s0_magic_packet = 0;
#endif

MODULE_AUTHOR("Realtek and the Linux r8101 crew <netdev@vger.kernel.org>");
MODULE_DESCRIPTION("RealTek RTL-8101 Fast Ethernet driver");

module_param(speed_mode, uint, 0);
MODULE_PARM_DESC(speed_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(duplex_mode, uint, 0);
MODULE_PARM_DESC(duplex_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(autoneg_mode, uint, 0);
MODULE_PARM_DESC(autoneg_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(advertising_mode, uint, 0);
MODULE_PARM_DESC(advertising_mode, "force phy operation. Deprecated by ethtool (8).");

module_param(aspm, int, 0);
MODULE_PARM_DESC(aspm, "Enable ASPM.");

module_param(s5wol, int, 0);
MODULE_PARM_DESC(s5wol, "Enable Shutdown Wake On Lan.");

module_param(s5_keep_curr_mac, int, 0);
MODULE_PARM_DESC(s5_keep_curr_mac, "Enable Shutdown Keep Current MAC Address.");

module_param(rx_copybreak, int, 0);
MODULE_PARM_DESC(rx_copybreak, "Copy breakpoint for copy-only-tiny-frames");

module_param(use_dac, int, 0);
MODULE_PARM_DESC(use_dac, "Enable PCI DAC. Unsafe on 32 bit PCI slot.");

module_param(timer_count, int, 0);
MODULE_PARM_DESC(timer_count, "Timer Interrupt Interval.");

module_param(eee_enable, int, 0);
MODULE_PARM_DESC(eee_enable, "Enable Energy Efficient Ethernet.");

module_param(hwoptimize, ulong, 0);
MODULE_PARM_DESC(hwoptimize, "Enable HW optimization function.");

module_param(s0_magic_packet, int, 0);
MODULE_PARM_DESC(s0_magic_packet, "Enable S0 Magic Packet.");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
module_param_named(debug, debug.msg_enable, int, 0);
MODULE_PARM_DESC(debug, "Debug verbosity level (0=none, ..., 16=all)");
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

MODULE_LICENSE("GPL");

MODULE_VERSION(RTL8101_VERSION);

static void rtl8101_dsm(struct net_device *dev, int dev_state);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void rtl8101_esd_timer(unsigned long __opaque);
#else
static void rtl8101_esd_timer(struct timer_list *t);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void rtl8101_link_timer(unsigned long __opaque);
#else
static void rtl8101_link_timer(struct timer_list *t);
#endif

static void rtl8101_hw_phy_config(struct net_device *dev);

static void rtl8101_tx_clear(struct rtl8101_private *tp);
static void rtl8101_rx_clear(struct rtl8101_private *tp);

static void rtl8101_aspm_fix1(struct net_device *dev);

static int rtl8101_open(struct net_device *dev);
static netdev_tx_t rtl8101_start_xmit(struct sk_buff *skb, struct net_device *dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8101_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t rtl8101_interrupt(int irq, void *dev_instance);
#endif
static int rtl8101_init_ring(struct net_device *dev);
static void rtl8101_hw_start(struct net_device *dev);
static void rtl8101_hw_config(struct net_device *dev);
static int rtl8101_close(struct net_device *dev);
static void rtl8101_set_rx_mode(struct net_device *dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static void rtl8101_tx_timeout(struct net_device *dev, unsigned int txqueue);
#else
static void rtl8101_tx_timeout(struct net_device *dev);
#endif
static struct net_device_stats *rtl8101_get_stats(struct net_device *dev);
static int rtl8101_rx_interrupt(struct net_device *, struct rtl8101_private *, napi_budget);
static int rtl8101_change_mtu(struct net_device *dev, int new_mtu);
static void rtl8101_down(struct net_device *dev);

static int rtl8101_set_speed(struct net_device *dev, u8 autoneg, u32 speed, u8 duplex, u32 adv);
static int rtl8101_set_mac_address(struct net_device *dev, void *p);
void rtl8101_rar_set(struct rtl8101_private *tp, const u8 *addr);
static void rtl8101_desc_addr_fill(struct rtl8101_private *);
static void rtl8101_tx_desc_init(struct rtl8101_private *tp);
static void rtl8101_rx_desc_init(struct rtl8101_private *tp);

static void rtl8101_hw_reset(struct net_device *dev);
static void rtl8101_phy_power_down(struct net_device *dev);
static void rtl8101_phy_power_up(struct net_device *dev);

static int rtl8101_set_phy_mcu_patch_request(struct rtl8101_private *tp);
static int rtl8101_clear_phy_mcu_patch_request(struct rtl8101_private *tp);

#ifdef CONFIG_R8101_NAPI
static int rtl8101_poll(napi_ptr napi, napi_budget budget);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8101_reset_task(void *_data);
#else
static void rtl8101_reset_task(struct work_struct *work);
#endif

static const unsigned int rtl8101_rx_config_V1 =
        (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift);

static const unsigned int rtl8101_rx_config_V2 =
        (RX_DMA_BURST << RxCfgDMAShift);

static const unsigned int rtl8101_rx_config_V3 =
        (RxCfg_128_int_en | RxEarly_off_V2 | Rx_Single_fetch_V2) | (RX_DMA_BURST << RxCfgDMAShift);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)
/* copied from linux kernel 2.6.20 include/linux/netdev.h */
#define	NETDEV_ALIGN		32
#define	NETDEV_ALIGN_CONST	(NETDEV_ALIGN - 1)

static inline void *netdev_priv(struct net_device *dev)
{
        return (char *)dev + ((sizeof(struct net_device)
                               + NETDEV_ALIGN_CONST)
                              & ~NETDEV_ALIGN_CONST);
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0) && \
     LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,00)))
void ethtool_convert_legacy_u32_to_link_mode(unsigned long *dst,
                u32 legacy_u32)
{
        bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS);
        dst[0] = legacy_u32;
}

bool ethtool_convert_link_mode_to_legacy_u32(u32 *legacy_u32,
                const unsigned long *src)
{
        bool retval = true;

        /* TODO: following test will soon always be true */
        if (__ETHTOOL_LINK_MODE_MASK_NBITS > 32) {
                __ETHTOOL_DECLARE_LINK_MODE_MASK(ext);

                bitmap_zero(ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
                bitmap_fill(ext, 32);
                bitmap_complement(ext, ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
                if (bitmap_intersects(ext, src,
                                      __ETHTOOL_LINK_MODE_MASK_NBITS)) {
                        /* src mask goes beyond bit 31 */
                        retval = false;
                }
        }
        *legacy_u32 = src[0];
        return retval;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)

static inline u32 mii_adv_to_ethtool_adv_t(u32 adv)
{
        u32 result = 0;

        if (adv & ADVERTISE_10HALF)
                result |= ADVERTISED_10baseT_Half;
        if (adv & ADVERTISE_10FULL)
                result |= ADVERTISED_10baseT_Full;
        if (adv & ADVERTISE_100HALF)
                result |= ADVERTISED_100baseT_Half;
        if (adv & ADVERTISE_100FULL)
                result |= ADVERTISED_100baseT_Full;
        if (adv & ADVERTISE_PAUSE_CAP)
                result |= ADVERTISED_Pause;
        if (adv & ADVERTISE_PAUSE_ASYM)
                result |= ADVERTISED_Asym_Pause;

        return result;
}

static inline u32 mii_lpa_to_ethtool_lpa_t(u32 lpa)
{
        u32 result = 0;

        if (lpa & LPA_LPACK)
                result |= ADVERTISED_Autoneg;

        return result | mii_adv_to_ethtool_adv_t(lpa);
}

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
static inline void eth_hw_addr_random(struct net_device *dev)
{
        random_ether_addr(dev->dev_addr);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
/* copied from linux kernel 2.6.20 include/linux/netdevice.h */
static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
        /* use default */
        if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
                return default_msg_enable_bits;
        if (debug_value == 0)	/* no output */
                return 0;
        /* set low N bits */
        return (1 << debug_value) - 1;
}

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
static inline void eth_copy_and_sum (struct sk_buff *dest,
                                     const unsigned char *src,
                                     int len, int base)
{
        memcpy (dest->data, src, len);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 /include/linux/time.h */
/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC	1000L

/* copied from linux kernel 2.6.20 /include/linux/jiffies.h */
/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return MAX_LONG, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
        return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
        return (j + (HZ / MSEC_PER_SEC) - 1)/(HZ / MSEC_PER_SEC);
#else
        return (j * MSEC_PER_SEC) / HZ;
#endif
}

static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
        if (m > jiffies_to_msecs(MAX_JIFFY_OFFSET))
                return MAX_JIFFY_OFFSET;
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
        return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
        return m * (HZ / MSEC_PER_SEC);
#else
        return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
#endif
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
//for linux kernel 2.6.10 and earlier.

/* copied from linux kernel 2.6.12.6 /include/linux/pm.h */
typedef int __bitwise pci_power_t;

/* copied from linux kernel 2.6.12.6 /include/linux/pci.h */
typedef u32 __bitwise pm_message_t;

#define PCI_D0	((pci_power_t __force) 0)
#define PCI_D1	((pci_power_t __force) 1)
#define PCI_D2	((pci_power_t __force) 2)
#define PCI_D3hot	((pci_power_t __force) 3)
#define PCI_D3cold	((pci_power_t __force) 4)
#define PCI_POWER_ERROR	((pci_power_t __force) -1)

/* copied from linux kernel 2.6.12.6 /drivers/pci/pci.c */
/**
 * pci_choose_state - Choose the power state of a PCI device
 * @dev: PCI device to be suspended
 * @state: target sleep state for the whole system. This is the value
 *	that is passed to suspend() function.
 *
 * Returns PCI power state suitable for given device and given system
 * message.
 */

pci_power_t pci_choose_state(struct pci_dev *dev, pm_message_t state)
{
        if (!pci_find_capability(dev, PCI_CAP_ID_PM))
                return PCI_D0;

        switch (state) {
        case 0:
                return PCI_D0;
        case 3:
                return PCI_D3hot;
        default:
                printk("They asked me for state %d\n", state);
//		BUG();
        }
        return PCI_D0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
//+porting on 2.6.8.1 and earlier
/**
 * msleep_interruptible - sleep waiting for waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
unsigned long msleep_interruptible(unsigned int msecs)
{
        unsigned long timeout = msecs_to_jiffies(msecs);

        while (timeout && !signal_pending(current)) {
                set_current_state(TASK_INTERRUPTIBLE);
                timeout = schedule_timeout(timeout);
        }
        return jiffies_to_msecs(timeout);
}

/* copied from linux kernel 2.6.20 include/linux/mii.h */
#undef if_mii
#define if_mii _kc_if_mii
static inline struct mii_ioctl_data *if_mii(struct ifreq *rq)
{
        return (struct mii_ioctl_data *) &rq->ifr_ifru;
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)

#ifdef ENABLE_R8101_PROCFS
/****************************************************************************
*   -----------------------------PROCFS STUFF-------------------------
*****************************************************************************
*/

static struct proc_dir_entry *rtl8101_proc;
static int proc_init_num = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int proc_get_driver_variable(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        seq_puts(m, "\nDump Driver Variable\n");

        spin_lock_irqsave(&tp->lock, flags);
        seq_puts(m, "Variable\tValue\n----------\t-----\n");
        seq_printf(m, "MODULENAME\t%s\n", MODULENAME);
        seq_printf(m, "driver version\t%s\n", RTL8101_VERSION);
        seq_printf(m, "chipset\t%d\n", tp->chipset);
        seq_printf(m, "chipset_name\t%s\n", rtl_chip_info[tp->chipset].name);
        seq_printf(m, "mtu\t%d\n", dev->mtu);
        seq_printf(m, "NUM_RX_DESC\t0x%x\n", tp->num_rx_desc);
        seq_printf(m, "cur_rx\t0x%x\n", tp->cur_rx);
        seq_printf(m, "dirty_rx\t0x%x\n", tp->dirty_rx);
        seq_printf(m, "NUM_TX_DESC\t0x%x\n", tp->num_tx_desc);
        seq_printf(m, "cur_tx\t0x%x\n", tp->cur_tx);
        seq_printf(m, "dirty_tx\t0x%x\n", tp->dirty_tx);
        seq_printf(m, "rx_buf_sz\t0x%x\n", tp->rx_buf_sz);
        seq_printf(m, "esd_flag\t0x%x\n", tp->esd_flag);
        seq_printf(m, "pci_cfg_is_read\t0x%x\n", tp->pci_cfg_is_read);
        seq_printf(m, "cp_cmd\t0x%x\n", tp->cp_cmd);
        seq_printf(m, "intr_mask\t0x%x\n", tp->intr_mask);
        seq_printf(m, "wol_enabled\t0x%x\n", tp->wol_enabled);
        seq_printf(m, "wol_opts\t0x%x\n", tp->wol_opts);
        seq_printf(m, "eeprom_type\t0x%x\n", tp->eeprom_type);
        seq_printf(m, "autoneg\t0x%x\n", tp->autoneg);
        seq_printf(m, "duplex\t0x%x\n", tp->duplex);
        seq_printf(m, "speed\t%d\n", tp->speed);
        seq_printf(m, "advertising\t0x%x\n", tp->advertising);
        seq_printf(m, "eeprom_len\t0x%x\n", tp->eeprom_len);
        seq_printf(m, "cur_page\t0x%x\n", tp->cur_page);
        seq_printf(m, "bios_setting\t0x%x\n", tp->bios_setting);
        seq_printf(m, "features\t0x%x\n", tp->features);
        seq_printf(m, "org_pci_offset_99\t0x%x\n", tp->org_pci_offset_99);
        seq_printf(m, "org_pci_offset_180\t0x%x\n", tp->org_pci_offset_180);
        seq_printf(m, "issue_offset_99_event\t0x%x\n", tp->issue_offset_99_event);
        seq_printf(m, "org_pci_offset_80\t0x%x\n", tp->org_pci_offset_80);
        seq_printf(m, "org_pci_offset_81\t0x%x\n", tp->org_pci_offset_81);
        seq_printf(m, "use_timer_interrrupt\t0x%x\n", tp->use_timer_interrrupt);
        seq_printf(m, "HwIcVerUnknown\t0x%x\n", tp->HwIcVerUnknown);
        seq_printf(m, "NotWrRamCodeToMicroP\t0x%x\n", tp->NotWrRamCodeToMicroP);
        seq_printf(m, "NotWrMcuPatchCode\t0x%x\n", tp->NotWrMcuPatchCode);
        seq_printf(m, "HwHasWrRamCodeToMicroP\t0x%x\n", tp->HwHasWrRamCodeToMicroP);
        seq_printf(m, "sw_ram_code_ver\t0x%x\n", tp->sw_ram_code_ver);
        seq_printf(m, "hw_ram_code_ver\t0x%x\n", tp->hw_ram_code_ver);
        seq_printf(m, "rtk_enable_diag\t0x%x\n", tp->rtk_enable_diag);
        seq_printf(m, "RequireAdcBiasPatch\t0x%x\n", tp->RequireAdcBiasPatch);
        seq_printf(m, "AdcBiasPatchIoffset\t0x%x\n", tp->AdcBiasPatchIoffset);
        seq_printf(m, "RequireAdjustUpsTxLinkPulseTiming\t0x%x\n", tp->RequireAdjustUpsTxLinkPulseTiming);
        seq_printf(m, "SwrCnt1msIni\t0x%x\n", tp->SwrCnt1msIni);
        seq_printf(m, "HwSuppNowIsOobVer\t0x%x\n", tp->HwSuppNowIsOobVer);
        seq_printf(m, "RequiredSecLanDonglePatch\t0x%x\n", tp->RequiredSecLanDonglePatch);
        seq_printf(m, "RequireResetNctlBfrPhyResetOrNway\t0x%x\n", tp->RequireResetNctlBfrPhyResetOrNway);
        seq_printf(m, "RequireResetPhyToChgSpd\t0x%x\n", tp->RequireResetPhyToChgSpd);
        seq_printf(m, "speed_mode\t0x%x\n", speed_mode);
        seq_printf(m, "duplex_mode\t0x%x\n", duplex_mode);
        seq_printf(m, "autoneg_mode\t0x%x\n", autoneg_mode);
        seq_printf(m, "aspm\t0x%x\n", aspm);
        seq_printf(m, "s5wol\t0x%x\n", s5wol);
        seq_printf(m, "s5_keep_curr_mac\t0x%x\n", s5_keep_curr_mac);
        seq_printf(m, "eee_enable\t0x%x\n", tp->eee_enabled);
        seq_printf(m, "hwoptimize\t0x%lx\n", hwoptimize);
        seq_printf(m, "proc_init_num\t0x%x\n", proc_init_num);
        seq_printf(m, "s0_magic_packet\t0x%x\n", s0_magic_packet);
        seq_printf(m, "HwSuppMagicPktVer\t0x%x\n", tp->HwSuppMagicPktVer);
        seq_printf(m, "random_mac\t0x%x\n", tp->random_mac);
        seq_printf(m, "org_mac_addr\t%pM\n", tp->org_mac_addr);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        seq_printf(m, "perm_addr\t%pM\n", dev->perm_addr);
#endif
        seq_printf(m, "dev_addr\t%pM\n", dev->dev_addr);
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_tally_counter(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        struct rtl8101_private *tp = netdev_priv(dev);
        struct rtl8101_counters *counters;
        dma_addr_t paddr;
        u32 cmd;
        u32 WaitCnt;
        unsigned long flags;

        seq_puts(m, "\nDump Tally Counter\n");

        //ASSERT_RTNL();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters) {
                seq_puts(m, "\nDump Tally Counter Fail\n");
                return 0;
        }

        spin_lock_irqsave(&tp->lock, flags);
        RTL_W32(tp, CounterAddrHigh, (u64)paddr >> 32);
        cmd = (u64)paddr & DMA_BIT_MASK(32);
        RTL_W32(tp, CounterAddrLow, cmd);
        RTL_W32(tp, CounterAddrLow, cmd | CounterDump);

        WaitCnt = 0;
        while (RTL_R32(tp, CounterAddrLow) & CounterDump) {
                udelay(10);

                WaitCnt++;
                if (WaitCnt > 20)
                        break;
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_puts(m, "Statistics\tValue\n----------\t-----\n");
        seq_printf(m, "tx_packets\t%lld\n", le64_to_cpu(counters->tx_packets));
        seq_printf(m, "rx_packets\t%lld\n", le64_to_cpu(counters->rx_packets));
        seq_printf(m, "tx_errors\t%lld\n", le64_to_cpu(counters->tx_errors));
        seq_printf(m, "rx_errors\t%d\n", le32_to_cpu(counters->rx_errors));
        seq_printf(m, "rx_missed\t%d\n", le16_to_cpu(counters->rx_missed));
        seq_printf(m, "align_errors\t%d\n", le16_to_cpu(counters->align_errors));
        seq_printf(m, "tx_one_collision\t%d\n", le32_to_cpu(counters->tx_one_collision));
        seq_printf(m, "tx_multi_collision\t%d\n", le32_to_cpu(counters->tx_multi_collision));
        seq_printf(m, "rx_unicast\t%lld\n", le64_to_cpu(counters->rx_unicast));
        seq_printf(m, "rx_broadcast\t%lld\n", le64_to_cpu(counters->rx_broadcast));
        seq_printf(m, "rx_multicast\t%d\n", le32_to_cpu(counters->rx_multicast));
        seq_printf(m, "tx_aborted\t%d\n", le16_to_cpu(counters->tx_aborted));
        seq_printf(m, "tx_underrun\t%d\n", le16_to_cpu(counters->tx_underrun));

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8101_MAC_REGS_SIZE;
        u8 byte_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;
        unsigned long flags;

        seq_puts(m, "\nDump MAC Registers\n");
        seq_puts(m, "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 16 && n < max; i++, n++) {
                        byte_rd = readb(ioaddr + n);
                        seq_printf(m, "%02x ", byte_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_pcie_phy(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8101_EPHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        seq_puts(m, "\nDump PCIE PHY\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8101_ephy_read(tp, n);
                        seq_printf(m, "%04x ", word_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_eth_phy(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8101_PHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        seq_puts(m, "\nDump Ethernet PHY\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        spin_lock_irqsave(&tp->lock, flags);
        seq_puts(m, "\n####################page 0##################\n ");
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8101_mdio_read(tp, n);
                        seq_printf(m, "%04x ", word_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_extended_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8101_ERI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                /* RTL8101E does not support Extend GMAC */
                seq_puts(m, "\nNot Support Dump Extended Registers\n");
                return 0;
        }

        seq_puts(m, "\nDump Extended Registers\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%02x:\t", n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        dword_rd = rtl8101_eri_read(tp, n, 4, ERIAR_ExGMAC);
                        seq_printf(m, "%08x ", dword_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}

static int proc_get_pci_registers(struct seq_file *m, void *v)
{
        struct net_device *dev = m->private;
        int i, n, max = R8101_PCI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        seq_puts(m, "\nDump PCI Registers\n");
        seq_puts(m, "\nOffset\tValue\n------\t-----\n ");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                seq_printf(m, "\n0x%03x:\t", n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
                        seq_printf(m, "%08x ", dword_rd);
                }
        }

        n = 0x110;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        seq_printf(m, "\n0x%03x:\t%08x ", n, dword_rd);
        n = 0x70c;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        seq_printf(m, "\n0x%03x:\t%08x ", n, dword_rd);

        spin_unlock_irqrestore(&tp->lock, flags);

        seq_putc(m, '\n');
        return 0;
}
#else

static int proc_get_driver_variable(char *page, char **start,
                                    off_t offset, int count,
                                    int *eof, void *data)
{
        struct net_device *dev = data;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Driver Driver\n");

        spin_lock_irqsave(&tp->lock, flags);
        len += snprintf(page + len, count - len,
                        "Variable\tValue\n----------\t-----\n");

        len += snprintf(page + len, count - len,
                        "MODULENAME\t%s\n"
                        "driver version\t%s\n"
                        "chipset\t%d\n"
                        "chipset_name\t%s\n"
                        "mtu\t%d\n"
                        "NUM_RX_DESC\t0x%x\n"
                        "cur_rx\t0x%x\n"
                        "dirty_rx\t0x%x\n"
                        "NUM_TX_DESC\t0x%x\n"
                        "cur_tx\t0x%x\n"
                        "dirty_tx\t0x%x\n"
                        "rx_buf_sz\t0x%x\n"
                        "esd_flag\t0x%x\n"
                        "pci_cfg_is_read\t0x%x\n"
                        "cp_cmd\t0x%x\n"
                        "intr_mask\t0x%x\n"
                        "wol_enabled\t0x%x\n"
                        "wol_opts\t0x%x\n"
                        "eeprom_type\t0x%x\n"
                        "autoneg\t0x%x\n"
                        "duplex\t0x%x\n"
                        "speed\t%d\n"
                        "eeprom_len\t0x%x\n"
                        "advertising\t0x%x\n"
                        "cur_page\t0x%x\n"
                        "bios_setting\t0x%x\n"
                        "features\t0x%x\n"
                        "org_pci_offset_99\t0x%x\n"
                        "org_pci_offset_180\t0x%x\n"
                        "issue_offset_99_event\t0x%x\n"
                        "org_pci_offset_80\t0x%x\n"
                        "org_pci_offset_81\t0x%x\n"
                        "use_timer_interrrupt\t0x%x\n"
                        "HwIcVerUnknown\t0x%x\n"
                        "NotWrRamCodeToMicroP\t0x%x\n"
                        "NotWrMcuPatchCode\t0x%x\n"
                        "HwHasWrRamCodeToMicroP\t0x%x\n"
                        "sw_ram_code_ver\t0x%x\n"
                        "hw_ram_code_ver\t0x%x\n"
                        "rtk_enable_diag\t0x%x\n"
                        "RequireAdcBiasPatch\t0x%x\n"
                        "AdcBiasPatchIoffset\t0x%x\n"
                        "RequireAdjustUpsTxLinkPulseTiming\t0x%x\n"
                        "SwrCnt1msIni\t0x%x\n"
                        "HwSuppNowIsOobVer\t0x%x\n"
                        "RequiredSecLanDonglePatch\t0x%x\n"
                        "RequireResetNctlBfrPhyResetOrNway\t0x%x\n"
                        "RequireResetPhyToChgSpd\t0x%x\n"
                        "speed_mode\t0x%x\n"
                        "duplex_mode\t0x%x\n"
                        "autoneg_mode\t0x%x\n"
                        "aspm\t0x%x\n"
                        "s5wol\t0x%x\n"
                        "s5_keep_curr_mac\t0x%x\n"
                        "eee_enable\t0x%x\n"
                        "hwoptimize\t0x%lx\n"
                        "proc_init_num\t0x%x\n"
                        "s0_magic_packet\t0x%x\n"
                        "HwSuppMagicPktVer\t0x%x\n"
                        "random_mac\t0x%x\n"
                        "org_mac_addr\t%pM\n"
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                        "perm_addr\t%pM\n"
#endif
                        "dev_addr\t%pM\n",
                        MODULENAME,
                        RTL8101_VERSION,
                        tp->chipset,
                        rtl_chip_info[tp->chipset].name,
                        dev->mtu,
                        tp->num_rx_desc,
                        tp->cur_rx,
                        tp->dirty_rx,
                        tp->num_tx_desc,
                        tp->cur_tx,
                        tp->dirty_tx,
                        tp->rx_buf_sz,
                        tp->esd_flag,
                        tp->pci_cfg_is_read,
                        tp->cp_cmd,
                        tp->intr_mask,
                        tp->wol_enabled,
                        tp->wol_opts,
                        tp->eeprom_type,
                        tp->autoneg,
                        tp->duplex,
                        tp->speed,
                        tp->advertising,
                        tp->eeprom_len,
                        tp->cur_page,
                        tp->bios_setting,
                        tp->features,
                        tp->org_pci_offset_99,
                        tp->org_pci_offset_180,
                        tp->issue_offset_99_event,
                        tp->org_pci_offset_80,
                        tp->org_pci_offset_81,
                        tp->use_timer_interrrupt,
                        tp->HwIcVerUnknown,
                        tp->NotWrRamCodeToMicroP,
                        tp->NotWrMcuPatchCode,
                        tp->HwHasWrRamCodeToMicroP,
                        tp->sw_ram_code_ver,
                        tp->hw_ram_code_ver,
                        tp->rtk_enable_diag,
                        tp->RequireAdcBiasPatch,
                        tp->AdcBiasPatchIoffset,
                        tp->RequireAdjustUpsTxLinkPulseTiming,
                        tp->SwrCnt1msIni,
                        tp->HwSuppNowIsOobVer,
                        tp->RequiredSecLanDonglePatch,
                        tp->RequireResetNctlBfrPhyResetOrNway,
                        tp->RequireResetPhyToChgSpd,
                        speed_mode,
                        duplex_mode,
                        autoneg_mode,
                        aspm,
                        s5wol,
                        s5_keep_curr_mac,
                        tp->eee_enabled,
                        hwoptimize,
                        proc_init_num,
                        s0_magic_packet,
                        tp->HwSuppMagicPktVer,
                        tp->random_mac,
                        tp->org_mac_addr,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                        dev->perm_addr,
#endif
                        dev->dev_addr
                       );
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_tally_counter(char *page, char **start,
                                  off_t offset, int count,
                                  int *eof, void *data)
{
        struct net_device *dev = data;
        struct rtl8101_private *tp = netdev_priv(dev);
        struct rtl8101_counters *counters;
        dma_addr_t paddr;
        u32 cmd;
        u32 WaitCnt;
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Tally Counter\n");

        //ASSERT_RTNL();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters) {
                len += snprintf(page + len, count - len,
                                "\nDump Tally Counter Fail\n");
                goto out;
        }

        spin_lock_irqsave(&tp->lock, flags);
        RTL_W32(tp, CounterAddrHigh, (u64)paddr >> 32);
        cmd = (u64)paddr & DMA_BIT_MASK(32);
        RTL_W32(tp, CounterAddrLow, cmd);
        RTL_W32(tp, CounterAddrLow, cmd | CounterDump);

        WaitCnt = 0;
        while (RTL_R32(tp, CounterAddrLow) & CounterDump) {
                udelay(10);

                WaitCnt++;
                if (WaitCnt > 20)
                        break;
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len,
                        "Statistics\tValue\n----------\t-----\n");

        len += snprintf(page + len, count - len,
                        "tx_packets\t%lld\n"
                        "rx_packets\t%lld\n"
                        "tx_errors\t%lld\n"
                        "rx_errors\t%d\n"
                        "rx_missed\t%d\n"
                        "align_errors\t%d\n"
                        "tx_one_collision\t%d\n"
                        "tx_multi_collision\t%d\n"
                        "rx_unicast\t%lld\n"
                        "rx_broadcast\t%lld\n"
                        "rx_multicast\t%d\n"
                        "tx_aborted\t%d\n"
                        "tx_underrun\t%d\n",
                        le64_to_cpu(counters->tx_packets),
                        le64_to_cpu(counters->rx_packets),
                        le64_to_cpu(counters->tx_errors),
                        le32_to_cpu(counters->rx_errors),
                        le16_to_cpu(counters->rx_missed),
                        le16_to_cpu(counters->align_errors),
                        le32_to_cpu(counters->tx_one_collision),
                        le32_to_cpu(counters->tx_multi_collision),
                        le64_to_cpu(counters->rx_unicast),
                        le64_to_cpu(counters->rx_broadcast),
                        le32_to_cpu(counters->rx_multicast),
                        le16_to_cpu(counters->tx_aborted),
                        le16_to_cpu(counters->tx_underrun)
                       );

        len += snprintf(page + len, count - len, "\n");
out:
        *eof = 1;
        return len;
}

static int proc_get_registers(char *page, char **start,
                              off_t offset, int count,
                              int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8101_MAC_REGS_SIZE;
        u8 byte_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump MAC Registers\n"
                        "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 16 && n < max; i++, n++) {
                        byte_rd = readb(ioaddr + n);
                        len += snprintf(page + len, count - len,
                                        "%02x ",
                                        byte_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_pcie_phy(char *page, char **start,
                             off_t offset, int count,
                             int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8101_EPHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump PCIE PHY\n"
                        "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8101_ephy_read(tp, n);
                        len += snprintf(page + len, count - len,
                                        "%04x ",
                                        word_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_eth_phy(char *page, char **start,
                            off_t offset, int count,
                            int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8101_PHY_REGS_SIZE/2;
        u16 word_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump Ethernet PHY\n"
                        "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        len += snprintf(page + len, count - len,
                        "\n####################page 0##################\n");
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 8 && n < max; i++, n++) {
                        word_rd = rtl8101_mdio_read(tp, n);
                        len += snprintf(page + len, count - len,
                                        "%04x ",
                                        word_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}

static int proc_get_extended_registers(char *page, char **start,
                                       off_t offset, int count,
                                       int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8101_ERI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int len = 0;

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                /* RTL8101E does not support Extend GMAC */
                len += snprintf(page + len, count - len,
                                "\nNot Support Dump Extended Registers\n");

                goto out;
        }

        len += snprintf(page + len, count - len,
                        "\nDump Extended Registers\n"
                        "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%02x:\t",
                                n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        dword_rd = rtl8101_eri_read(tp, n, 4, ERIAR_ExGMAC);
                        len += snprintf(page + len, count - len,
                                        "%08x ",
                                        dword_rd);
                }
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");
out:
        *eof = 1;
        return len;
}

static int proc_get_pci_registers(char *page, char **start,
                                  off_t offset, int count,
                                  int *eof, void *data)
{
        struct net_device *dev = data;
        int i, n, max = R8101_PCI_REGS_SIZE;
        u32 dword_rd;
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int len = 0;

        len += snprintf(page + len, count - len,
                        "\nDump PCI Registers\n"
                        "Offset\tValue\n------\t-----\n");

        spin_lock_irqsave(&tp->lock, flags);
        for (n = 0; n < max;) {
                len += snprintf(page + len, count - len,
                                "\n0x%03x:\t",
                                n);

                for (i = 0; i < 4 && n < max; i++, n+=4) {
                        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
                        len += snprintf(page + len, count - len,
                                        "%08x ",
                                        dword_rd);
                }
        }

        n = 0x110;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        len += snprintf(page + len, count - len,
                        "\n0x%03x:\t%08x ",
                        n,
                        dword_rd);
        n = 0x70c;
        pci_read_config_dword(tp->pci_dev, n, &dword_rd);
        len += snprintf(page + len, count - len,
                        "\n0x%03x:\t%08x ",
                        n,
                        dword_rd);
        spin_unlock_irqrestore(&tp->lock, flags);

        len += snprintf(page + len, count - len, "\n");

        *eof = 1;
        return len;
}
#endif
static void rtl8101_proc_module_init(void)
{
        //create /proc/net/r8101
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
        rtl8101_proc = proc_mkdir(MODULENAME, init_net.proc_net);
#else
        rtl8101_proc = proc_mkdir(MODULENAME, proc_net);
#endif
        if (!rtl8101_proc)
                dprintk("cannot create %s proc entry \n", MODULENAME);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
/*
 * seq_file wrappers for procfile show routines.
 */
static int rtl8101_proc_open(struct inode *inode, struct file *file)
{
        struct net_device *dev = proc_get_parent_data(inode);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
        int (*show)(struct seq_file *, void *) = pde_data(inode);
#else
        int (*show)(struct seq_file *, void *) = PDE_DATA(inode);
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)

        return single_open(file, show, dev);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops rtl8101_proc_fops = {
        .proc_open           = rtl8101_proc_open,
        .proc_read           = seq_read,
        .proc_lseek          = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations rtl8101_proc_fops = {
        .open           = rtl8101_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#endif

/*
 * Table of proc files we need to create.
 */
struct rtl8101_proc_file {
        char name[12];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        int (*show)(struct seq_file *, void *);
#else
        int (*show)(char *, char **, off_t, int, int *, void *);
#endif
};

static const struct rtl8101_proc_file rtl8101_proc_files[] = {
        { "driver_var", &proc_get_driver_variable },
        { "tally", &proc_get_tally_counter },
        { "registers", &proc_get_registers },
        { "pcie_phy", &proc_get_pcie_phy },
        { "eth_phy", &proc_get_eth_phy },
        { "ext_regs", &proc_get_extended_registers },
        { "pci_regs", &proc_get_pci_registers },
        { "" }
};

static void rtl8101_proc_init(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        const struct rtl8101_proc_file *f;
        struct proc_dir_entry *dir;

        if (rtl8101_proc && !tp->proc_dir) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                dir = proc_mkdir_data(dev->name, 0, rtl8101_proc, dev);
                if (!dir) {
                        printk("Unable to initialize /proc/net/%s/%s\n",
                               MODULENAME, dev->name);
                        return;
                }

                tp->proc_dir = dir;
                proc_init_num++;

                for (f = rtl8101_proc_files; f->name[0]; f++) {
                        if (!proc_create_data(f->name, S_IFREG | S_IRUGO, dir,
                                              &rtl8101_proc_fops, f->show)) {
                                printk("Unable to initialize "
                                       "/proc/net/%s/%s/%s\n",
                                       MODULENAME, dev->name, f->name);
                                return;
                        }
                }
#else
                dir = proc_mkdir(dev->name, rtl8101_proc);
                if (!dir) {
                        printk("Unable to initialize /proc/net/%s/%s\n",
                               MODULENAME, dev->name);
                        return;
                }

                tp->proc_dir = dir;
                proc_init_num++;

                for (f = rtl8101_proc_files; f->name[0]; f++) {
                        if (!create_proc_read_entry(f->name, S_IFREG | S_IRUGO,
                                                    dir, f->show, dev)) {
                                printk("Unable to initialize "
                                       "/proc/net/%s/%s/%s\n",
                                       MODULENAME, dev->name, f->name);
                                return;
                        }
                }
#endif
        }
}

static void rtl8101_proc_remove(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->proc_dir) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                remove_proc_subtree(dev->name, rtl8101_proc);
                proc_init_num--;
#else
                const struct rtl8101_proc_file *f;
                struct rtl8101_private *tp = netdev_priv(dev);

                for (f = rtl8101_proc_files; f->name[0]; f++)
                        remove_proc_entry(f->name, tp->proc_dir);

                remove_proc_entry(dev->name, rtl8101_proc);
                proc_init_num--;
#endif
                tp->proc_dir = NULL;
        }
}

#endif //ENABLE_R8101_PROCFS

static inline u16 map_phy_ocp_addr(u16 PageNum, u8 RegNum)
{
        u16 OcpPageNum = 0;
        u8 OcpRegNum = 0;
        u16 OcpPhyAddress = 0;

        if( PageNum == 0 ) {
                OcpPageNum = OCP_STD_PHY_BASE_PAGE + ( RegNum / 8 );
                OcpRegNum = 0x10 + ( RegNum % 8 );
        } else {
                OcpPageNum = PageNum;
                OcpRegNum = RegNum;
        }

        OcpPageNum <<= 4;

        if( OcpRegNum < 16 ) {
                OcpPhyAddress = 0;
        } else {
                OcpRegNum -= 16;
                OcpRegNum <<= 1;

                OcpPhyAddress = OcpPageNum + OcpRegNum;
        }


        return OcpPhyAddress;
}

static void mdio_real_direct_write_phy_ocp(struct rtl8101_private *tp,
                u32 RegAddr,
                u32 value)
{
        u32 data32;
        int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(RegAddr % 2);
#endif
        data32 = RegAddr/2;
        data32 <<= OCPR_Addr_Reg_shift;
        data32 |= OCPR_Write | value;

        RTL_W32(tp, PHYOCP, data32);
        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                if (!(RTL_R32(tp, PHYOCP) & OCPR_Flag))
                        break;
        }
}

static void mdio_direct_write_phy_ocp(struct rtl8101_private *tp,
                                      u16 RegAddr,
                                      u16 value)
{
        if (tp->rtk_enable_diag) return;

        mdio_real_direct_write_phy_ocp(tp, RegAddr, value);
}

static void rtl8101_mdio_write_phy_ocp(struct rtl8101_private *tp,
                                       u16 PageNum,
                                       u32 RegAddr,
                                       u32 value)
{
        u16 ocp_addr;

        if (tp->rtk_enable_diag) return;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        mdio_direct_write_phy_ocp(tp, ocp_addr, value);
}

static void rtl8101_mdio_real_write_phy_ocp(struct rtl8101_private *tp,
                u16 PageNum,
                u32 RegAddr,
                u32 value)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        mdio_real_direct_write_phy_ocp(tp, ocp_addr, value);
}

static void mdio_real_write(struct rtl8101_private *tp,
                            u32 RegAddr,
                            u32 value)
{
        int i;

        if (RegAddr == 0x1F) {
                tp->cur_page = value;
        }

        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                if (RegAddr == 0x1F) {
                        return;
                }
                rtl8101_mdio_real_write_phy_ocp(tp, tp->cur_page, RegAddr, value);
        } else {
                RTL_W32(tp, PHYAR, PHYAR_Write |
                        (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift |
                        (value & PHYAR_Data_Mask));

                for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8101_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8101 has completed writing to the specified MII register */
                        if (!(RTL_R32(tp, PHYAR) & PHYAR_Flag)) {
                                udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
                                break;
                        }
                }
        }
}

static void
rtl8101_hw_phy_nctl_reset_start(struct rtl8101_private *tp)
{
        if(tp->RequireResetNctlBfrPhyResetOrNway == FALSE) return;

        mdio_real_write(tp, 0x1f, 0x0004);
        mdio_real_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) | BIT_6);
        mdio_real_write(tp, 0x1f, 0x0000);
}

static void
rtl8101_hw_phy_nctl_reset_end(struct rtl8101_private *tp)
{
        if(tp->RequireResetNctlBfrPhyResetOrNway == FALSE) return;

        mdio_real_write(tp, 0x1f, 0x0004);
        mdio_real_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) & ~BIT_6);
        mdio_real_write(tp, 0x1f, 0x0000);
}

void rtl8101_mdio_write(struct rtl8101_private *tp,
                        u32 RegAddr,
                        u32 value)
{
        u8 reset_nctl = 0;

        if (tp->rtk_enable_diag) return;

        if(tp->RequireResetNctlBfrPhyResetOrNway &&
            (tp->cur_page == 0 && RegAddr == MII_BMCR) &&
            (value & (BMCR_RESET|BMCR_ANRESTART)))
                reset_nctl = 1;

        if(reset_nctl)
                rtl8101_hw_phy_nctl_reset_start(tp);

        mdio_real_write(tp, RegAddr, value);

        if(reset_nctl)
                rtl8101_hw_phy_nctl_reset_end(tp);
}

void rtl8101_mdio_prot_write(struct rtl8101_private *tp,
                             u32 RegAddr,
                             u32 value)
{
        mdio_real_write(tp, RegAddr, value);
}

void rtl8101_mdio_prot_direct_write_phy_ocp(struct rtl8101_private *tp,
                u32 RegAddr,
                u32 value)
{
        mdio_real_direct_write_phy_ocp(tp, RegAddr, value);
}

static u32 mdio_real_direct_read_phy_ocp(struct rtl8101_private *tp,
                u32 RegAddr)
{
        u32 data32;
        int i, value = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(RegAddr % 2);
#endif
        data32 = RegAddr/2;
        data32 <<= OCPR_Addr_Reg_shift;

        RTL_W32(tp, PHYOCP, data32);
        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                if (RTL_R32(tp, PHYOCP) & OCPR_Flag)
                        break;
        }
        value = RTL_R32(tp, PHYOCP) & OCPDR_Data_Mask;

        return value;
}

static u32 mdio_direct_read_phy_ocp(struct rtl8101_private *tp,
                                    u16 RegAddr)
{
        if (tp->rtk_enable_diag) return 0xffffffff;

        return mdio_real_direct_read_phy_ocp(tp, RegAddr);
}

static u32 rtl8101_mdio_read_phy_ocp(struct rtl8101_private *tp,
                                     u16 PageNum,
                                     u32 RegAddr)
{
        u16 ocp_addr;

        if (tp->rtk_enable_diag) return 0xffffffff;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        return mdio_direct_read_phy_ocp(tp, ocp_addr);
}

static u32 rtl8101_mdio_real_read_phy_ocp(struct rtl8101_private *tp,
                u16 PageNum,
                u32 RegAddr)
{
        u16 ocp_addr;

        ocp_addr = map_phy_ocp_addr(PageNum, RegAddr);

        return mdio_real_direct_read_phy_ocp(tp, ocp_addr);
}

static u32 mdio_real_read(struct rtl8101_private *tp,
                          u32 RegAddr)
{
        int i, value = 0;

        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                value = rtl8101_mdio_real_read_phy_ocp(tp, tp->cur_page, RegAddr);
        } else {
                RTL_W32(tp, PHYAR,
                        PHYAR_Read | (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift);

                for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8101_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8101 has completed retrieving data from the specified MII register */
                        if (RTL_R32(tp, PHYAR) & PHYAR_Flag) {
                                value = RTL_R32(tp, PHYAR) & PHYAR_Data_Mask;
                                udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
                                break;
                        }
                }
        }

        return value;
}

u32 rtl8101_mdio_read(struct rtl8101_private *tp,
                      u32 RegAddr)
{
        if (tp->rtk_enable_diag) return 0xffffffff;

        return mdio_real_read(tp, RegAddr);
}

u32 rtl8101_mdio_prot_read(struct rtl8101_private *tp,
                           u32 RegAddr)
{
        return mdio_real_read(tp, RegAddr);
}

u32 rtl8101_mdio_prot_direct_read_phy_ocp(struct rtl8101_private *tp,
                u32 RegAddr)
{
        return mdio_real_direct_read_phy_ocp(tp, RegAddr);
}

static void ClearAndSetEthPhyBit(struct rtl8101_private *tp, u8  addr, u16 clearmask, u16 setmask)
{
        u16 PhyRegValue;


        PhyRegValue = rtl8101_mdio_read(tp, addr);
        PhyRegValue &= ~clearmask;
        PhyRegValue |= setmask;
        rtl8101_mdio_write(tp, addr, PhyRegValue);
}

static void rtl8101_clear_eth_phy_bit(struct rtl8101_private *tp, u8 addr, u16 mask)
{
        ClearAndSetEthPhyBit(tp,
                             addr,
                             mask,
                             0
                            );
}

static void rtl8101_set_eth_phy_bit(struct rtl8101_private *tp,  u8  addr, u16  mask)
{
        ClearAndSetEthPhyBit(tp,
                             addr,
                             0,
                             mask
                            );
}

void rtl8101_mac_ocp_write(struct rtl8101_private *tp, u16 reg_addr, u16 value)
{
        u32 data32;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(reg_addr % 2);
#endif

        data32 = reg_addr/2;
        data32 <<= OCPR_Addr_Reg_shift;
        data32 += value;
        data32 |= OCPR_Write;

        RTL_W32(tp, MACOCP, data32);
}

u16 rtl8101_mac_ocp_read(struct rtl8101_private *tp, u16 reg_addr)
{
        u32 data32;
        u16 data16 = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(reg_addr % 2);
#endif

        data32 = reg_addr/2;
        data32 <<= OCPR_Addr_Reg_shift;

        RTL_W32(tp, MACOCP, data32);
        data16 = (u16)RTL_R32(tp, MACOCP);

        return data16;
}

static void
rtl8101_clear_and_set_mcu_ocp_bit(
        struct rtl8101_private *tp,
        u16   addr,
        u16   clearmask,
        u16   setmask
)
{
        u16 RegValue;

        RegValue = rtl8101_mac_ocp_read(tp, addr);
        RegValue &= ~clearmask;
        RegValue |= setmask;
        rtl8101_mac_ocp_write(tp, addr, RegValue);
}

static void
rtl8101_clear_mcu_ocp_bit(
        struct rtl8101_private *tp,
        u16   addr,
        u16   mask
)
{
        rtl8101_clear_and_set_mcu_ocp_bit(tp,
                                          addr,
                                          mask,
                                          0
                                         );
}

static void
rtl8101_set_mcu_ocp_bit(
        struct rtl8101_private *tp,
        u16   addr,
        u16   mask
)
{
        rtl8101_clear_and_set_mcu_ocp_bit(tp,
                                          addr,
                                          0,
                                          mask
                                         );
}

static void
rtl8101_phyio_write(struct rtl8101_private *tp,
                    int RegAddr,
                    int value)
{
        int i;

        RTL_W32(tp, PHYIO, PHYIO_Write |
                (RegAddr & PHYIO_Reg_Mask) << PHYIO_Reg_shift |
                (value & PHYIO_Data_Mask));

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed writing to the specified MII register */
                if (!(RTL_R32(tp, PHYIO) & PHYIO_Flag))
                        break;
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
}

#if 0
static int
rtl8101_phyio_read(struct rtl8101_private *tp,
                   int RegAddr)
{
        int i, value = -1;

        RTL_W32(tp, PHYIO,
                PHYIO_Read | (RegAddr & PHYIO_Reg_Mask) << PHYIO_Reg_shift);

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed retrieving data from the specified MII register */
                if (RTL_R32(tp, PHYIO) & PHYIO_Flag) {
                        value = (int) (RTL_R32(tp, PHYIO) & PHYIO_Data_Mask);
                        break;
                }
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return value;
}
#endif

static u8 rtl8101_check_ephy_addr(struct rtl8101_private *tp, int addr)
{
        if ( tp->mcfg != CFG_METHOD_20) goto exit;

        if (addr & (BIT_6 | BIT_5))
                rtl8101_clear_and_set_mcu_ocp_bit(tp, 0xDE28,
                                                  (BIT_1 | BIT_0),
                                                  (addr >> 5) & (BIT_1 | BIT_0));

        addr &= 0x1F;

exit:
        return addr;
}

static void _rtl8101_ephy_write(struct rtl8101_private *tp, u32 addr, u32 value)
{
        int i;

        RTL_W32(tp, EPHYAR,
                EPHYAR_Write |
                (addr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift |
                (value & EPHYAR_Data_Mask));

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed EPHY write */
                if (!(RTL_R32(tp, EPHYAR) & EPHYAR_Flag))
                        break;
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
}

void rtl8101_ephy_write(struct rtl8101_private *tp, u32 addr, u32 value)
{
        _rtl8101_ephy_write(tp, rtl8101_check_ephy_addr(tp, addr), value);
}

static u16 _rtl8101_ephy_read(struct rtl8101_private *tp, u32 addr)
{
        int i;
        u16 value = 0xffff;

        RTL_W32(tp, EPHYAR,
                EPHYAR_Read | (addr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift);

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed EPHY read */
                if (RTL_R32(tp, EPHYAR) & EPHYAR_Flag) {
                        value = (u16) (RTL_R32(tp, EPHYAR) & EPHYAR_Data_Mask);
                        break;
                }
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return value;
}

u16 rtl8101_ephy_read(struct rtl8101_private *tp, u32 addr)
{
        return _rtl8101_ephy_read(tp, rtl8101_check_ephy_addr(tp, addr));
}

static void ClearAndSetPCIePhyBit(struct rtl8101_private *tp, u8 addr, u16 clearmask, u16 setmask)
{
        u16 EphyValue;

        EphyValue = rtl8101_ephy_read(tp, addr);
        EphyValue &= ~clearmask;
        EphyValue |= setmask;
        rtl8101_ephy_write(tp, addr, EphyValue);
}

static void ClearPCIePhyBit(struct rtl8101_private *tp, u8 addr, u16 mask)
{
        ClearAndSetPCIePhyBit( tp,
                               addr,
                               mask,
                               0
                             );
}

static void SetPCIePhyBit( struct rtl8101_private *tp, u8 addr, u16 mask)
{
        ClearAndSetPCIePhyBit( tp,
                               addr,
                               0,
                               mask
                             );
}

static u32
rtl8101_csi_other_fun_read(struct rtl8101_private *tp,
                           u8 multi_fun_sel_bit,
                           u32 addr)
{
        u32 cmd;
        int i;
        u32 value = 0;

        cmd = CSIAR_Read | CSIAR_ByteEn << CSIAR_ByteEn_shift | (addr & CSIAR_Addr_Mask);

        if (tp->mcfg != CFG_METHOD_14) {
                multi_fun_sel_bit = 0;
        }

        if( multi_fun_sel_bit > 7 ) {
                return 0xffffffff;
        }

        cmd |= multi_fun_sel_bit << 16;

        RTL_W32(tp, CSIAR, cmd);

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed CSI read */
                if (RTL_R32(tp, CSIAR) & CSIAR_Flag) {
                        value = (u32)RTL_R32(tp, CSIDR);
                        break;
                }
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return value;
}

static void
rtl8101_csi_other_fun_write(struct rtl8101_private *tp,
                            u8 multi_fun_sel_bit,
                            u32 addr,
                            u32 value)
{
        u32 cmd;
        int i;

        RTL_W32(tp, CSIDR, value);
        cmd = CSIAR_Write | CSIAR_ByteEn << CSIAR_ByteEn_shift | (addr & CSIAR_Addr_Mask);
        if (tp->mcfg != CFG_METHOD_14) {
                multi_fun_sel_bit = 0;
        }

        if( multi_fun_sel_bit > 7 ) {
                return;
        }

        cmd |= multi_fun_sel_bit << 16;

        RTL_W32(tp, CSIAR, cmd);

        for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                udelay(R8101_CHANNEL_WAIT_TIME);

                /* Check if the RTL8101 has completed CSI write */
                if (!(RTL_R32(tp, CSIAR) & CSIAR_Flag))
                        break;
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
}

static u32
rtl8101_csi_read(struct rtl8101_private *tp,
                 u32 addr)
{
        u8 multi_fun_sel_bit;

        if (tp->mcfg == CFG_METHOD_14)
                multi_fun_sel_bit = 2;
        else
                multi_fun_sel_bit = 0;


        return rtl8101_csi_other_fun_read(tp, multi_fun_sel_bit, addr);
}

static void
rtl8101_csi_write(struct rtl8101_private *tp,
                  u32 addr,
                  u32 value)
{
        u8 multi_fun_sel_bit;

        if (tp->mcfg == CFG_METHOD_14)
                multi_fun_sel_bit = 2;
        else
                multi_fun_sel_bit = 0;

        rtl8101_csi_other_fun_write(tp, multi_fun_sel_bit, addr, value);
}

static u8
rtl8101_csi_fun0_read_byte(struct rtl8101_private *tp,
                           u32 addr)
{
        u8 RetVal = 0;

        if (tp->mcfg == CFG_METHOD_14) {
                u32 TmpUlong;
                u16 RegAlignAddr;
                u8 ShiftByte;

                RegAlignAddr = addr & ~(0x3);
                ShiftByte = addr & (0x3);
                TmpUlong = rtl8101_csi_other_fun_read(tp, 0, RegAlignAddr);
                TmpUlong >>= (8*ShiftByte);
                RetVal = (u8)TmpUlong;
        } else {
                struct pci_dev *pdev = tp->pci_dev;

                pci_read_config_byte(pdev, addr, &RetVal);
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return RetVal;
}

static void
rtl8101_csi_fun0_write_byte(struct rtl8101_private *tp,
                            u32 addr,
                            u8 value)
{
        if (tp->mcfg == CFG_METHOD_14) {
                u32 TmpUlong;
                u16 RegAlignAddr;
                u8 ShiftByte;

                RegAlignAddr = addr & ~(0x3);
                ShiftByte = addr & (0x3);
                TmpUlong = rtl8101_csi_other_fun_read(tp, 0, RegAlignAddr);
                TmpUlong &= ~(0xFF << (8*ShiftByte));
                TmpUlong |= (value << (8*ShiftByte));
                rtl8101_csi_other_fun_write( tp, 0, RegAlignAddr, TmpUlong );
        } else {
                struct pci_dev *pdev = tp->pci_dev;

                pci_write_config_byte(pdev, addr, value);
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);
}

u32 rtl8101_eri_read(struct rtl8101_private *tp, int addr, int len, int type)
{
        int i, val_shift, shift = 0;
        u32 value1 = 0, value2 = 0, mask;

        if (len > 4 || len <= 0)
                return -1;

        while (len > 0) {
                val_shift = addr % ERIAR_Addr_Align;
                addr = addr & ~0x3;

                RTL_W32(tp, ERIAR,
                        ERIAR_Read |
                        type << ERIAR_Type_shift |
                        ERIAR_ByteEn << ERIAR_ByteEn_shift |
                        addr);

                for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8101_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8101 has completed ERI read */
                        if (RTL_R32(tp, ERIAR) & ERIAR_Flag)
                                break;
                }

                if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

                value1 = RTL_R32(tp, ERIDR) & mask;
                value2 |= (value1 >> val_shift * 8) << shift * 8;

                if (len <= 4 - val_shift)
                        len = 0;
                else {
                        len -= (4 - val_shift);
                        shift = 4 - val_shift;
                        addr += 4;
                }
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return value2;
}

int rtl8101_eri_write(struct rtl8101_private *tp, int addr, int len, u32 value, int type)
{
        int i, val_shift, shift = 0;
        u32 value1 = 0, mask;

        if (len > 4 || len <= 0)
                return -1;

        while (len > 0) {
                val_shift = addr % ERIAR_Addr_Align;
                addr = addr & ~0x3;

                if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
                else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

                value1 = rtl8101_eri_read(tp, addr, 4, type) & ~mask;
                value1 |= ((value << val_shift * 8) >> shift * 8);

                RTL_W32(tp, ERIDR, value1);
                RTL_W32(tp, ERIAR,
                        ERIAR_Write |
                        type << ERIAR_Type_shift |
                        ERIAR_ByteEn << ERIAR_ByteEn_shift |
                        addr);

                for (i = 0; i < R8101_CHANNEL_WAIT_COUNT; i++) {
                        udelay(R8101_CHANNEL_WAIT_TIME);

                        /* Check if the RTL8101 has completed ERI write */
                        if (!(RTL_R32(tp, ERIAR) & ERIAR_Flag))
                                break;
                }

                if (len <= 4 - val_shift)
                        len = 0;
                else {
                        len -= (4 - val_shift);
                        shift = 4 - val_shift;
                        addr += 4;
                }
        }

        udelay(R8101_CHANNEL_EXIT_DELAY_TIME);

        return 0;
}

static void
rtl8101_enable_rxdvgate(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_3);
                mdelay(2);
                break;
        }
}

static void
rtl8101_disable_rxdvgate(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_3);
                mdelay(2);
                break;
        }
}

static void
rtl8101_wait_txrx_fifo_empty(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                for (i = 0; i < 10; i++) {
                        udelay(100);
                        if (RTL_R32(tp, TxConfig) & BIT_11)
                                break;
                }

                for (i = 0; i < 10; i++) {
                        udelay(100);
                        if ((RTL_R8(tp, MCUCmd_reg) & (Txfifo_empty | Rxfifo_empty)) == (Txfifo_empty | Rxfifo_empty))
                                break;
                }

                mdelay(1);
                break;
        }
}

static inline void
rtl8101_enable_hw_interrupt(struct rtl8101_private *tp)
{
        RTL_W16(tp, IntrMask, tp->intr_mask);
}

static inline void
rtl8101_disable_hw_interrupt(struct rtl8101_private *tp)
{
        RTL_W16(tp, IntrMask, 0x0000);
}


static inline void
rtl8101_switch_to_hw_interrupt(struct rtl8101_private *tp)
{
        RTL_W16(tp, TimeInt0, 0x0000);
        rtl8101_enable_hw_interrupt(tp);
}

static inline void
rtl8101_switch_to_timer_interrupt(struct rtl8101_private *tp)
{
        if (tp->use_timer_interrrupt) {
                RTL_W32(tp, TimeInt0, timer_count);
                RTL_W32(tp, TCTR, timer_count);
                RTL_W16(tp, IntrMask, PCSTimeout);
        } else {
                rtl8101_switch_to_hw_interrupt(tp);
        }
}

static void
rtl8101_irq_mask_and_ack(struct rtl8101_private *tp)
{
        rtl8101_disable_hw_interrupt(tp);
        RTL_W16(tp, IntrStatus, RTL_R16(tp, IntrStatus));
}

static void
rtl8101_nic_reset(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i;

        RTL_W32(tp, RxConfig, (RX_DMA_BURST << RxCfgDMAShift));

        rtl8101_enable_rxdvgate(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                RTL_W8(tp, ChipCmd, StopReq | CmdRxEnb | CmdTxEnb);
                udelay(100);
                break;
        case CFG_METHOD_14:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                mdelay(2);
                break;
        default:
                mdelay(10);
                break;
        }

        rtl8101_wait_txrx_fifo_empty(dev);

        /* Soft reset the chip. */
        RTL_W8(tp, ChipCmd, CmdReset);

        /* Check that the chip has finished the reset. */
        for (i = 100; i > 0; i--) {
                if ((RTL_R8(tp, ChipCmd) & CmdReset) == 0)
                        break;
                udelay(100);
        }
}

static void
rtl8101_hw_clear_timer_int(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        RTL_W32(tp, TimeInt0, 0x0000);

        switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        case CFG_METHOD_9:
                RTL_W32(tp, TimeInt1, 0x0000);
                break;
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W32(tp, TimeInt1, 0x0000);
                RTL_W32(tp, TimeInt2, 0x0000);
                RTL_W32(tp, TimeInt3, 0x0000);
                break;
        }
}

static int rtl8101_enable_eee(struct rtl8101_private *tp)
{
        int ret;
        u16 data;
        u32 csi_tmp;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_10:
                rtl8101_mdio_write(tp, 0x1F, 0x0007);
                rtl8101_mdio_write(tp, 0x1E, 0x0020);
                data = rtl8101_mdio_read(tp, 0x15) | 0x0100;
                rtl8101_mdio_write(tp, 0x15, data);
                rtl8101_mdio_write(tp, 0x1F, 0x0006);
                rtl8101_mdio_write(tp, 0x00, 0x5A30);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0006);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                if ((RTL_R8(tp, Config4)&0x40) && (RTL_R8(tp, 0x6D) & BIT_7)) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0005);
                        rtl8101_mdio_write(tp, 0x05, 0x8AC8);
                        rtl8101_mdio_write(tp, 0x06, RTL_R16(tp, CustomLED));
                        rtl8101_mdio_write(tp, 0x05, 0x8B82);
                        data = rtl8101_mdio_read(tp, 0x06) | 0x0010;
                        rtl8101_mdio_write(tp, 0x05, 0x8B82);
                        rtl8101_mdio_write(tp, 0x06, data);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
                break;
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
                rtl8101_eri_write(tp, 0x1B0, 2, 0xED03, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                if (RTL_R8(tp, 0xEF) & 0x02) {
                        rtl8101_mdio_write(tp, 0x10, 0x731F);
                        rtl8101_mdio_write(tp, 0x19, 0x7630);
                } else {
                        rtl8101_mdio_write(tp, 0x10, 0x711F);
                        rtl8101_mdio_write(tp, 0x19, 0x7030);
                }
                rtl8101_mdio_write(tp, 0x1A, 0x1506);
                rtl8101_mdio_write(tp, 0x1B, 0x0551);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0002);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0003);
                rtl8101_mdio_write(tp, 0x0E, 0x0015);
                rtl8101_mdio_write(tp, 0x0D, 0x4003);
                rtl8101_mdio_write(tp, 0x0E, 0x0002);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                break;

        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                rtl8101_eri_write(tp, 0x1B0, 2, 0xED03, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                rtl8101_mdio_write(tp, 0x10, 0x731F);
                rtl8101_mdio_write(tp, 0x19, 0x7630);
                rtl8101_mdio_write(tp, 0x1A, 0x1506);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0002);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                break;

        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1B0, 4, ERIAR_ExGMAC);
                csi_tmp |= BIT_1 | BIT_0;
                rtl8101_eri_write(tp, 0x1B0, 4, csi_tmp, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                data = rtl8101_mdio_read(tp, 0x11);
                rtl8101_mdio_write(tp, 0x11, data | BIT_4);
                rtl8101_mdio_write(tp, 0x1F, 0x0A5D);
                rtl8101_mdio_write(tp, 0x10, tp->eee_adv_t);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;

        default:
                //dev_printk(KERN_DEBUG, &tp->pci_dev->dev, "Not Support EEE\n");
                ret = -EOPNOTSUPP;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, 0x1F, 0x0A4A);
                rtl8101_set_eth_phy_bit(tp, 0x11, BIT_9);
                rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                rtl8101_set_eth_phy_bit(tp, 0x14, BIT_7);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        }

        /*Advanced EEE*/
        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_set_phy_mcu_patch_request(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                rtl8101_eri_write(tp, 0x1EA, 1, 0xFA, ERIAR_ExGMAC);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                data = rtl8101_mdio_read(tp, 0x10);
                if (data & BIT_10) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                        data = rtl8101_mdio_read(tp, 0x16);
                        data &= ~(BIT_1);
                        rtl8101_mdio_write(tp, 0x16, data);
                } else {
                        rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                        data = rtl8101_mdio_read(tp, 0x16);
                        data |= BIT_1;
                        rtl8101_mdio_write(tp, 0x16, data);
                }
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                /*
                data = rtl8101_mac_ocp_read(tp, 0xE052);
                data |= BIT_0;
                rtl8101_mac_ocp_write(tp, 0xE052, data);
                */

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                data = rtl8101_mdio_read(tp, 0x10) | BIT_15;
                rtl8101_mdio_write(tp, 0x10, data);

                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                data = rtl8101_mdio_read(tp, 0x11) | BIT_13 | BIT_14;
                data &= ~(BIT_12);
                rtl8101_mdio_write(tp, 0x11, data);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_clear_phy_mcu_patch_request(tp);
                break;
        }

        return ret;
}

static int rtl8101_disable_eee(struct rtl8101_private *tp)
{
        int ret;
        u16 data;
        u32 csi_tmp;

        ret = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_10:
                rtl8101_mdio_write(tp, 0x1F, 0x0007);
                rtl8101_mdio_write(tp, 0x1E, 0x0020);
                data = rtl8101_mdio_read(tp, 0x15) & ~0x0100;
                rtl8101_mdio_write(tp, 0x15, data);
                rtl8101_mdio_write(tp, 0x1F, 0x0006);
                rtl8101_mdio_write(tp, 0x00, 0x5A00);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                if (RTL_R8(tp, Config4) & 0x40) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0005);
                        rtl8101_mdio_write(tp, 0x05, 0x8B82);
                        data = rtl8101_mdio_read(tp, 0x06) & ~0x0010;
                        rtl8101_mdio_write(tp, 0x05, 0x8B82);
                        rtl8101_mdio_write(tp, 0x06, data);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
                break;
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
                rtl8101_eri_write(tp, 0x1B0, 2, 0, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                rtl8101_mdio_write(tp, 0x10, 0x401F);
                rtl8101_mdio_write(tp, 0x19, 0x7030);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0003);
                rtl8101_mdio_write(tp, 0x0E, 0x0015);
                rtl8101_mdio_write(tp, 0x0D, 0x4003);
                rtl8101_mdio_write(tp, 0x0E, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                break;

        case CFG_METHOD_14:
                rtl8101_eri_write(tp, 0x1B0, 2, 0, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                rtl8101_mdio_write(tp, 0x10, 0x401F);
                rtl8101_mdio_write(tp, 0x19, 0x7030);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                break;

        case CFG_METHOD_15:
        case CFG_METHOD_16:
                rtl8101_eri_write(tp, 0x1B0, 2, 0, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                rtl8101_mdio_write(tp, 0x10, 0xC07F);
                rtl8101_mdio_write(tp, 0x19, 0x7030);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0007);
                rtl8101_mdio_write(tp, 0x0E, 0x003C);
                rtl8101_mdio_write(tp, 0x0D, 0x4007);
                rtl8101_mdio_write(tp, 0x0E, 0x0000);
                rtl8101_mdio_write(tp, 0x0D, 0x0000);
                break;

        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1B0, 4, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_1 | BIT_0);
                rtl8101_eri_write(tp, 0x1B0, 4, csi_tmp, ERIAR_ExGMAC);
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                data = rtl8101_mdio_read(tp, 0x11);
                rtl8101_mdio_write(tp, 0x11, data & ~BIT_4);
                rtl8101_mdio_write(tp, 0x1F, 0x0A5D);
                rtl8101_mdio_write(tp, 0x10, 0x0000);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;

        default:
                //dev_printk(KERN_DEBUG, &tp->pci_dev->dev, "Not Support EEE\n");
                ret = -EOPNOTSUPP;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                rtl8101_clear_eth_phy_bit(tp, 0x14, BIT_7);
                rtl8101_mdio_write(tp, 0x1F, 0x0A4A);
                rtl8101_clear_eth_phy_bit(tp, 0x11, BIT_9);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        }

        /*Advanced EEE*/
        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_set_phy_mcu_patch_request(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                rtl8101_eri_write(tp, 0x1EA, 1, 0x00, ERIAR_ExGMAC);

                rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                data = rtl8101_mdio_read(tp, 0x16);
                data &= ~(BIT_1);
                rtl8101_mdio_write(tp, 0x16, data);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                data = rtl8101_mac_ocp_read(tp, 0xE052);
                data &= ~(BIT_0);
                rtl8101_mac_ocp_write(tp, 0xE052, data);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                data = rtl8101_mdio_read(tp, 0x10) & ~(BIT_15);
                rtl8101_mdio_write(tp, 0x10, data);

                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                data = rtl8101_mdio_read(tp, 0x11) & ~(BIT_12 | BIT_13 | BIT_14);
                rtl8101_mdio_write(tp, 0x11, data);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_clear_phy_mcu_patch_request(tp);
                break;
        }

        return ret;
}

static void
rtl8101_hw_reset(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        /* Disable interrupts */
        rtl8101_irq_mask_and_ack(tp);

        rtl8101_hw_clear_timer_int(dev);

        rtl8101_nic_reset(dev);
}

static unsigned int
rtl8101_xmii_reset_pending(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int retval;

        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        retval = rtl8101_mdio_read(tp, MII_BMCR) & BMCR_RESET;

        return retval;
}

static unsigned int
rtl8101_xmii_link_ok(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int retval;

        retval = (RTL_R8(tp, PHYstatus) & LinkStatus) ? 1 : 0;

        return retval;
}

static void
rtl8101_xmii_reset_enable(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i, val = 0;

        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, MII_ADVERTISE, rtl8101_mdio_read(tp, MII_ADVERTISE) &
                           ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                             ADVERTISE_100HALF | ADVERTISE_100FULL));
        rtl8101_mdio_write(tp, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);

        for (i = 0; i < 2500; i++) {
                val = rtl8101_mdio_read(tp, MII_BMCR) & BMCR_RESET;

                if (!val) {
                        return;
                }

                mdelay(1);
        }

        if (netif_msg_link(tp))
                printk(KERN_ERR "%s: PHY reset failed.\n", dev->name);
}

static void
set_offset70F(struct rtl8101_private *tp, u8 setting)
{
        u32 csi_tmp;
        u32 temp = (u32)setting;
        temp = temp << 24;
        /*set PCI configuration space offset 0x70F to setting*/
        /*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/

        csi_tmp = rtl8101_csi_read(tp, 0x70c) & 0x00ffffff;
        rtl8101_csi_write(tp, 0x70c, csi_tmp | temp);
}

static void
set_offset79(struct rtl8101_private *tp, u8 setting)
{
        //Set PCI configuration space offset 0x79 to setting

        struct pci_dev *pdev = tp->pci_dev;
        u8 device_control;

        if (hwoptimize & HW_PATCH_SOC_LAN) return;

        pci_read_config_byte(pdev, 0x79, &device_control);
        device_control &= ~0x70;
        device_control |= setting;
        pci_write_config_byte(pdev, 0x79, device_control);

}

static void
rtl8101_init_ring_indexes(struct rtl8101_private *tp)
{
        tp->dirty_tx = 0;
        tp->dirty_rx = 0;
        tp->cur_tx = 0;
        tp->cur_rx = 0;
}

static void
rtl8101_issue_offset_99_event(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                csi_tmp = rtl8101_eri_read(tp, 0xC7, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_7;
                rtl8101_eri_write(tp, 0xC7, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
                rtl8101_eri_write(tp, 0x3FC, 4, 0x00000000, ERIAR_ExGMAC);

                csi_tmp = rtl8101_eri_read(tp, 0x3F8, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0x3F8, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1EA, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0x1EA, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

static unsigned int rtl8101_phy_duplex(u8 status)
{
        unsigned int duplex = DUPLEX_UNKNOWN;

        if (status & LinkStatus) {
                if (status & RTL8101_FULL_DUPLEX_MASK)
                        duplex = DUPLEX_FULL;
                else
                        duplex = DUPLEX_HALF;
        }

        return duplex;
}

static int rtl8101_phy_speed(u8 status)
{
        int speed = SPEED_UNKNOWN;

        if (status & LinkStatus) {
                if (status & _100bps)
                        speed = SPEED_100;
                else if (status & _10bps)
                        speed = SPEED_10;
        }

        return speed;
}

static void
rtl8101_check_link_status(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int link_status_on;
        u32 data32;

        link_status_on = tp->link_ok(dev);

        if (netif_carrier_ok(dev) != link_status_on) {

                if (link_status_on) {
                        rtl8101_hw_config(dev);

                        if (tp->mcfg == CFG_METHOD_5 || tp->mcfg == CFG_METHOD_6 ||
                            tp->mcfg == CFG_METHOD_7 || tp->mcfg == CFG_METHOD_8)
                                set_offset70F(tp, 0x3F);

                        if (tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
                            tp->mcfg == CFG_METHOD_13) {
                                if ((RTL_R8(tp, PHYstatus) & FullDup) == 0 && tp->eee_enabled == 1)
                                        rtl8101_disable_eee(tp);

                                if (RTL_R8(tp, PHYstatus) & _10bps) {
                                        rtl8101_eri_write(tp, 0x1D0, 2, 0x4D02, ERIAR_ExGMAC);
                                        rtl8101_eri_write(tp, 0x1DC, 2, 0x0060, ERIAR_ExGMAC);

                                        rtl8101_eri_write(tp, 0x1B0, 2, 0, ERIAR_ExGMAC);
                                        rtl8101_mdio_write( tp, 0x1F, 0x0004);
                                        data32 = rtl8101_mdio_read( tp, 0x10);
                                        data32 |= 0x0400;
                                        data32 &= ~0x0800;
                                        rtl8101_mdio_write(tp, 0x10, data32);
                                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                                } else {
                                        rtl8101_eri_write(tp, 0x1D0, 2, 0, ERIAR_ExGMAC);
                                        if ( tp->eee_enabled == 1 && (RTL_R8(tp, 0xEF) & BIT_0) == 0)
                                                rtl8101_eri_write(tp, 0x1B0, 2, 0xED03, ERIAR_ExGMAC);
                                }
                        } else if (tp->mcfg == CFG_METHOD_14 || tp->mcfg == CFG_METHOD_15 ||
                                   tp->mcfg == CFG_METHOD_16) {
                                if (RTL_R8(tp, PHYstatus) & _10bps) {
                                        rtl8101_eri_write(tp, 0x1D0, 2, 0x4d02, ERIAR_ExGMAC);
                                        rtl8101_eri_write(tp, 0x1DC, 2, 0x0060, ERIAR_ExGMAC);
                                } else {
                                        rtl8101_eri_write(tp, 0x1D0, 2, 0, ERIAR_ExGMAC);
                                }
                        }

                        if ((tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
                             tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20)
                            && netif_running(dev)) {
                                if (RTL_R8(tp, PHYstatus)&FullDup)
                                        RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | (BIT_24 | BIT_25)) & ~BIT_19);
                                else
                                        RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | BIT_25) & ~(BIT_19 | BIT_24));

                                /*half mode*/
                                if (!(RTL_R8(tp, PHYstatus)&FullDup)) {
                                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                                        rtl8101_mdio_write(tp, MII_ADVERTISE, rtl8101_mdio_read(tp, MII_ADVERTISE)&~(ADVERTISE_PAUSE_CAP|ADVERTISE_PAUSE_ASYM));
                                }
                        }


                        rtl8101_hw_start(dev);

                        netif_carrier_on(dev);

                        netif_wake_queue(dev);

                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        tp->phy_reg_aner = rtl8101_mdio_read(tp, MII_EXPANSION);
                        tp->phy_reg_anlpar = rtl8101_mdio_read(tp, MII_LPA);

                        if (netif_msg_ifup(tp)) {
                                const u8 phy_status = RTL_R8(tp, PHYstatus);
                                const unsigned int phy_duplex = rtl8101_phy_duplex(phy_status);
                                const int phy_speed = rtl8101_phy_speed(phy_status);
                                printk(KERN_INFO PFX "%s: Link is Up - %s/%s\n",
                                       dev->name,
                                       phy_speed_to_str(phy_speed),
                                       phy_duplex_to_str(phy_duplex));
                        }
                } else {
                        if (tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
                            tp->mcfg == CFG_METHOD_13) {
                                rtl8101_mdio_write(tp, 0x1F, 0x0004);
                                data32 = rtl8101_mdio_read( tp, 0x10);
                                data32 &= ~0x0C00;
                                rtl8101_mdio_write(tp, 0x10, data32);
                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        }
                        if (netif_msg_ifdown(tp))
                                printk(KERN_INFO PFX "%s: Link is Down\n", dev->name);

                        tp->phy_reg_aner = 0;
                        tp->phy_reg_anlpar = 0;

                        netif_stop_queue(dev);

                        netif_carrier_off(dev);

                        rtl8101_hw_reset(dev);

                        rtl8101_tx_clear(tp);

                        rtl8101_rx_clear(tp);

                        rtl8101_init_ring(dev);

                        rtl8101_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);

                        if (tp->mcfg == CFG_METHOD_5 || tp->mcfg == CFG_METHOD_6 ||
                            tp->mcfg == CFG_METHOD_7 || tp->mcfg == CFG_METHOD_8)
                                set_offset70F(tp, 0x27);

                        switch (tp->mcfg) {
                        case CFG_METHOD_15:
                        case CFG_METHOD_16:
                        case CFG_METHOD_17:
                                if (tp->org_pci_offset_99 & BIT_2)
                                        tp->issue_offset_99_event = TRUE;
                                break;
                        }
                }
        }

        switch (tp->mcfg) {
        case CFG_METHOD_4:
                rtl8101_aspm_fix1(dev);
                break;
        }

        if (!link_status_on) {
                switch (tp->mcfg) {
                case CFG_METHOD_15:
                case CFG_METHOD_16:
                case CFG_METHOD_17:
                        if (tp->issue_offset_99_event) {
                                if (!(RTL_R8(tp, PHYstatus) & PowerSaveStatus)) {
                                        tp->issue_offset_99_event = FALSE;
                                        rtl8101_issue_offset_99_event(tp);
                                }
                        }
                        break;
                }
        }
}

static void
rtl8101_enable_ocp_phy_power_saving(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 val;

        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                val = rtl8101_mdio_read_phy_ocp(tp, 0x0C41, 0x13);
                if (val != 0x0050) {
                        rtl8101_set_phy_mcu_patch_request(tp);
                        rtl8101_mdio_write_phy_ocp(tp, 0x0C41, 0x13, 0x0000);
                        rtl8101_mdio_write_phy_ocp(tp, 0x0C41, 0x13, 0x0050);
                        rtl8101_clear_phy_mcu_patch_request(tp);
                }
        }
}

static void
rtl8101_disable_ocp_phy_power_saving(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 val;

        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                val = rtl8101_mdio_read_phy_ocp(tp, 0x0C41, 0x13);
                if (val != 0x0500) {
                        rtl8101_set_phy_mcu_patch_request(tp);
                        rtl8101_mdio_write_phy_ocp(tp, 0x0C41, 0x13, 0x0000);
                        rtl8101_mdio_write_phy_ocp(tp, 0x0C41, 0x13, 0x0500);
                        rtl8101_clear_phy_mcu_patch_request(tp);
                }
        }
}

static void
rtl8101_wait_ll_share_fifo_ready(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i;

        for (i = 0; i < 10; i++) {
                udelay(100);
                if (RTL_R16(tp, 0xD2) & BIT_9)
                        break;
        }
}

static void
rtl8101_disable_pci_offset_99(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                csi_tmp = rtl8101_eri_read(tp, 0xC0, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_0 | BIT_1);
                rtl8101_eri_write(tp, 0xC0, 2, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x3F2, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_0 | BIT_1);
                rtl8101_eri_write(tp, 0x3F2, 2, csi_tmp, ERIAR_ExGMAC);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_csi_fun0_write_byte(tp, 0x99, 0x00);
                break;
        }
}

static void
rtl8101_enable_pci_offset_99(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_csi_fun0_write_byte(tp, 0x99, tp->org_pci_offset_99);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                csi_tmp = rtl8101_eri_read(tp, 0xC0, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_0 | BIT_1);
                if (tp->org_pci_offset_99 & (BIT_5 | BIT_6))
                        csi_tmp |= BIT_1;
                if (tp->org_pci_offset_99 & BIT_2)
                        csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0xC0, 2, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x3F2, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_0 | BIT_1);
                if (tp->org_pci_offset_99 & (BIT_5 | BIT_6))
                        csi_tmp |= BIT_1;
                if (tp->org_pci_offset_99 & BIT_2)
                        csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0x3F2, 2, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

static void
rtl8101_init_pci_offset_99(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x3F2, 2, ERIAR_ExGMAC);
                csi_tmp &= ~( BIT_8 | BIT_9  | BIT_10 | BIT_11  | BIT_12  | BIT_13  | BIT_14 | BIT_15 );
                csi_tmp |= ( BIT_9 | BIT_10 | BIT_13  | BIT_14 | BIT_15 );
                rtl8101_eri_write(tp, 0x3F2, 2, csi_tmp, ERIAR_ExGMAC);
                csi_tmp = rtl8101_eri_read(tp, 0x3F5, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_6 | BIT_7;
                rtl8101_eri_write(tp, 0x3F5, 1, csi_tmp, ERIAR_ExGMAC);
                rtl8101_mac_ocp_write(tp, 0xE02C, 0x1880);
                rtl8101_mac_ocp_write(tp, 0xE02E, 0x4880);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                if (tp->org_pci_offset_99 & BIT_2)
                        rtl8101_mac_ocp_write(tp, 0xE0A2,  rtl8101_mac_ocp_read(tp, 0xE0A2) | BIT_0);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                csi_tmp = rtl8101_eri_read(tp, 0xC0, 1, ERIAR_ExGMAC);
                csi_tmp |= ( BIT_2 | BIT_3 );
                rtl8101_eri_write(tp, 0xC0, 1, csi_tmp, ERIAR_ExGMAC);

                rtl8101_eri_write(tp, 0xC8, 2, 0x840A, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xCA, 2, 0x840A, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xCC, 2, 0x840A, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xCE, 2, 0x840A, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_eri_write(tp, 0x2E8, 2, 0x9003, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0x2EA, 2, 0x9003, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0x2EC, 2, 0x9003, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0x2E2, 2, 0x883C, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0x2E4, 2, 0x8C12, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0x2E6, 2, 0x9003, ERIAR_ExGMAC);

                if (tp->mcfg == CFG_METHOD_17) {
                        csi_tmp = rtl8101_eri_read(tp, 0x3FA, 2, ERIAR_ExGMAC);
                        csi_tmp |= BIT_14;
                        rtl8101_eri_write(tp, 0x3FA, 2, csi_tmp, ERIAR_ExGMAC);
                }
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                if (tp->org_pci_offset_99 & BIT_2)
                        RTL_W8(tp, 0xB6, RTL_R8(tp, 0xB6) | BIT_0);
                break;
        }

        rtl8101_enable_pci_offset_99(tp);
}

static void
rtl8101_disable_pci_offset_180(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1E2, 1, ERIAR_ExGMAC);
                csi_tmp &= ~BIT_2;
                rtl8101_eri_write(tp, 0x1E2, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

static void
rtl8101_enable_pci_offset_180(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1E8, 4, ERIAR_ExGMAC);
                csi_tmp &= ~(0x0000FF00);
                csi_tmp |= (0x00006400);
                rtl8101_eri_write(tp, 0x1E8, 4, csi_tmp, ERIAR_ExGMAC);

                csi_tmp = rtl8101_eri_read(tp, 0x1E4, 4, ERIAR_ExGMAC);
                csi_tmp &= ~(0x0000FF00);
                rtl8101_eri_write(tp, 0x1E4, 4, csi_tmp, ERIAR_ExGMAC);

                csi_tmp = rtl8101_eri_read(tp, 0x1E2, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_2;
                rtl8101_eri_write(tp, 0x1E2, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x1E8, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(0xFFF0);
                csi_tmp |= 0x0640;
                rtl8101_eri_write(tp,0x1E8, 2, csi_tmp, ERIAR_ExGMAC);

                csi_tmp = rtl8101_eri_read(tp, 0x1E4, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(0xFF00);
                rtl8101_eri_write(tp, 0x1E4, 2, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

static void
rtl8101_init_pci_offset_180(struct rtl8101_private *tp)
{
        rtl8101_enable_pci_offset_180(tp);
}

static void
rtl8101_set_pci_99_180_exit_driver_para(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                if (tp->org_pci_offset_99 & BIT_2)
                        rtl8101_issue_offset_99_event(tp);
                rtl8101_disable_pci_offset_99(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_disable_pci_offset_180(tp);
                break;
        }
}

static void
rtl8101_enable_cfg9346_write(struct rtl8101_private *tp)
{
        RTL_W8(tp, Cfg9346, RTL_R8(tp, Cfg9346) | Cfg9346_Unlock);
}

static void
rtl8101_enable_exit_l1_mask(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_14:
                csi_tmp = rtl8101_eri_read(tp, 0xD4, 4, ERIAR_ExGMAC);
                csi_tmp |= (BIT_10 | BIT_11);
                rtl8101_eri_write(tp, 0xD4, 4, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0xD4, 4, ERIAR_ExGMAC);
                csi_tmp |= (BIT_7 | BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12);
                rtl8101_eri_write(tp, 0xD4, 4, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

static void
rtl8101_disable_exit_l1_mask(struct rtl8101_private *tp)
{
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_14:
                csi_tmp = rtl8101_eri_read(tp, 0xD4, 4, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_10 | BIT_11);
                rtl8101_eri_write(tp, 0xD4, 4, csi_tmp, ERIAR_ExGMAC);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0xD4, 4, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_7 | BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12);
                rtl8101_eri_write(tp, 0xD4, 4, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}
static void
rtl8101_disable_cfg9346_write(struct rtl8101_private *tp)
{
        RTL_W8(tp, Cfg9346, RTL_R8(tp, Cfg9346) & ~Cfg9346_Unlock);
}

static void
rtl8101_hw_d3_para(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                rtl8101_enable_cfg9346_write(tp);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                rtl8101_disable_cfg9346_write(tp);
                break;
        }

        rtl8101_disable_exit_l1_mask(tp);

        if ((tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
             tp->mcfg == CFG_METHOD_13) && (tp->eee_enabled == 1))
                rtl8101_disable_eee(tp);

        rtl8101_set_pci_99_180_exit_driver_para(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_20:
                rtl8101_set_mcu_ocp_bit(tp, 0xD438, BIT_3);
                rtl8101_set_mcu_ocp_bit(tp, 0xDE38, BIT_2);
                rtl8101_clear_mcu_ocp_bit(tp, 0xDE28, (BIT_1 | BIT_0));
                rtl8101_set_mcu_ocp_bit(tp, 0xD438, (BIT_1 | BIT_0));
                break;
        }

        /*disable ocp phy power saving*/
        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20)
                rtl8101_disable_ocp_phy_power_saving(dev);

        if (tp->bios_setting & BIT_28) {
                if (tp->mcfg == CFG_METHOD_13) {
                        if (!(RTL_R8(tp, 0xEF) & BIT_2)) {
                                u32 gphy_val;

                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                                rtl8101_mdio_write(tp, 0x04, 0x0061);
                                rtl8101_mdio_write(tp, 0x00, 0x1200);
                                rtl8101_mdio_write(tp, 0x18, 0x0310);
                                mdelay(20);
                                rtl8101_mdio_write(tp, 0x1F, 0x0005);
                                gphy_val = rtl8101_mdio_read(tp, 0x1a);
                                gphy_val |= BIT_8 | BIT_0;
                                rtl8101_mdio_write(tp, 0x1a, gphy_val);
                                mdelay(30);
                                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                                rtl8101_mdio_write(tp, 0x18, 0x8310);
                        }
                }
        }

        rtl8101_disable_rxdvgate(dev);
}

static void
rtl8101_enable_magic_packet(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 csi_tmp;

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V1:
                rtl8101_enable_cfg9346_write(tp);
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) | MagicPacket);
                rtl8101_disable_cfg9346_write(tp);
                break;
        case WAKEUP_MAGIC_PACKET_V2:
                csi_tmp = rtl8101_eri_read(tp, 0xDE, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0xDE, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}
static void
rtl8101_disable_magic_packet(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 csi_tmp;

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V1:
                rtl8101_enable_cfg9346_write(tp);
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~MagicPacket);
                rtl8101_disable_cfg9346_write(tp);
                break;
        case WAKEUP_MAGIC_PACKET_V2:
                csi_tmp = rtl8101_eri_read(tp, 0xDE, 1, ERIAR_ExGMAC);
                csi_tmp &= ~BIT_0;
                rtl8101_eri_write(tp, 0xDE, 1, csi_tmp, ERIAR_ExGMAC);
                break;
        }
}

#define WAKE_ANY (WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST)

static void
rtl8101_get_hw_wol(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u8 options;
        u32 csi_tmp;
        unsigned long flags;


        spin_lock_irqsave(&tp->lock, flags);

        tp->wol_opts = 0;
        options = RTL_R8(tp, Config1);
        if (!(options & PMEnable))
                goto out_unlock;

        options = RTL_R8(tp, Config3);
        if (options & LinkUp)
                tp->wol_opts |= WAKE_PHY;

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V2:
                csi_tmp = rtl8101_eri_read(tp, 0xDE, 1, ERIAR_ExGMAC);
                if (csi_tmp & BIT_0)
                        tp->wol_opts |= WAKE_MAGIC;
                break;
        default:
                if (options & MagicPacket)
                        tp->wol_opts |= WAKE_MAGIC;
                break;
        }

        options = RTL_R8(tp, Config5);
        if (options & UWF)
                tp->wol_opts |= WAKE_UCAST;
        if (options & BWF)
                tp->wol_opts |= WAKE_BCAST;
        if (options & MWF)
                tp->wol_opts |= WAKE_MCAST;

out_unlock:
        tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

        spin_unlock_irqrestore(&tp->lock, flags);
}

static void
rtl8101_set_hw_wol(struct net_device *dev, u32 wolopts)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i,tmp;
        static struct {
                u32 opt;
                u16 reg;
                u8  mask;
        } cfg[] = {
                { WAKE_PHY,   Config3, LinkUp },
                { WAKE_UCAST, Config5, UWF },
                { WAKE_BCAST, Config5, BWF },
                { WAKE_MCAST, Config5, MWF },
                { WAKE_ANY,   Config5, LanWake },
                { WAKE_MAGIC, Config3, MagicPacket },
        };

        switch (tp->HwSuppMagicPktVer) {
        case WAKEUP_MAGIC_PACKET_V2:
                tmp = ARRAY_SIZE(cfg) - 1;

                if (wolopts & WAKE_MAGIC)
                        rtl8101_enable_magic_packet(dev);
                else
                        rtl8101_disable_magic_packet(dev);
                break;
        default:
                tmp = ARRAY_SIZE(cfg);
                break;
        }

        rtl8101_enable_cfg9346_write(tp);

        for (i = 0; i < tmp; i++) {
                u8 options = RTL_R8(tp, cfg[i].reg) & ~cfg[i].mask;
                if (wolopts & cfg[i].opt)
                        options |= cfg[i].mask;
                RTL_W8(tp, cfg[i].reg, options);
        }

        rtl8101_disable_cfg9346_write(tp);
}

static void
rtl8101_phy_setup_force_mode(struct net_device *dev, u32 speed, u8 duplex)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 bmcr_true_force = 0;

        if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
                bmcr_true_force = BMCR_SPEED10;
        } else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
                bmcr_true_force = BMCR_SPEED10 | BMCR_FULLDPLX;
        } else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
                bmcr_true_force = BMCR_SPEED100;
        } else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
                bmcr_true_force = BMCR_SPEED100 | BMCR_FULLDPLX;
        } else {
                netif_err(tp, drv, dev, "Failed to set phy force mode!\n");
                return;
        }

        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        rtl8101_mdio_write(tp, MII_BMCR, bmcr_true_force);
}

static void
rtl8101_set_wol_link_speed(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int auto_nego;
        u32 adv;
        u16 anlpar;
        u16 aner;

        if (tp->autoneg != AUTONEG_ENABLE)
                goto exit;

        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        auto_nego = rtl8101_mdio_read(tp, MII_ADVERTISE);
        auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL
                       | ADVERTISE_100HALF | ADVERTISE_100FULL);

        aner = tp->phy_reg_aner;
        anlpar = tp->phy_reg_anlpar;
        if (tp->link_ok(dev)) {
                aner = rtl8101_mdio_read(tp, MII_EXPANSION);
                anlpar = rtl8101_mdio_read(tp, MII_LPA);
        }

        adv = tp->advertising;
        if ((aner | anlpar) == 0) {
                int auto_nego_tmp = 0;
                if (adv & ADVERTISED_10baseT_Half)
                        auto_nego_tmp |= ADVERTISE_10HALF;
                if (adv & ADVERTISED_10baseT_Full)
                        auto_nego_tmp |= ADVERTISE_10FULL;
                if (adv & ADVERTISED_100baseT_Half)
                        auto_nego_tmp |= ADVERTISE_100HALF;
                if (adv & ADVERTISED_100baseT_Full)
                        auto_nego_tmp |= ADVERTISE_100FULL;

                if (auto_nego_tmp == 0)
                        goto exit;

                auto_nego |= auto_nego_tmp;
                goto skip_check_lpa;
        }
        if (!(aner & EXPANSION_NWAY)) goto exit;

        if ((adv & ADVERTISED_10baseT_Half) && (anlpar & LPA_10HALF))
                auto_nego |= ADVERTISE_10HALF;
        else if ((adv & ADVERTISED_10baseT_Full) && (anlpar & LPA_10FULL))
                auto_nego |= ADVERTISE_10FULL;
        else if ((adv & ADVERTISED_100baseT_Half) && (anlpar & LPA_100HALF))
                auto_nego |= ADVERTISE_100HALF;
        else if ((adv & ADVERTISED_100baseT_Full) && (anlpar & LPA_100FULL))
                auto_nego |= ADVERTISE_100FULL;
        else
                goto exit;

skip_check_lpa:
        rtl8101_mdio_write(tp, MII_ADVERTISE, auto_nego);
        rtl8101_mdio_write(tp, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);

exit:
        return;
}

static void
rtl8101_powerdown_pll(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->wol_enabled == WOL_ENABLED) {
                rtl8101_set_hw_wol(dev, tp->wol_opts);

                if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
                    tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                        rtl8101_enable_cfg9346_write(tp);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | PMSTS_En);
                        rtl8101_disable_cfg9346_write(tp);
                }

                if (tp->RequireResetPhyToChgSpd)
                        rtl8101_hw_phy_config(dev);

                rtl8101_set_wol_link_speed(dev);

                switch (tp->mcfg) {
                case CFG_METHOD_1:
                case CFG_METHOD_2:
                case CFG_METHOD_3:
                case CFG_METHOD_4:
                case CFG_METHOD_5:
                case CFG_METHOD_6:
                case CFG_METHOD_7:
                case CFG_METHOD_8:
                case CFG_METHOD_9:
                        break;
                default:
                        RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) | AcceptBroadcast | AcceptMulticast | AcceptMyPhys);
                        break;
                }

                return;
        }

        rtl8101_phy_power_down(dev);

        if (!tp->HwIcVerUnknown) {
                switch (tp->mcfg) {
                case CFG_METHOD_6:
                case CFG_METHOD_9:
                        RTL_W8(tp, DBG_reg, RTL_R8(tp, DBG_reg) | BIT_3);
                        RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) & ~BIT_7);
                        break;

                case CFG_METHOD_8:
                        pci_write_config_byte(tp->pci_dev, 0x81, 0);
                        RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) & ~BIT_7);
                        break;
                case CFG_METHOD_7:
                case CFG_METHOD_10:
                case CFG_METHOD_11:
                case CFG_METHOD_12:
                case CFG_METHOD_13:
                case CFG_METHOD_14:
                case CFG_METHOD_15:
                case CFG_METHOD_16:
                case CFG_METHOD_17:
                case CFG_METHOD_18:
                case CFG_METHOD_19:
                case CFG_METHOD_20:
                        RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) & ~BIT_7);
                        break;
                default:
                        break;
                }
        }

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) & ~BIT_6);
                break;
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) & ~BIT_6);
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_6);
                break;
        }
}

static void
rtl8101_powerup_pll(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_6:
        case CFG_METHOD_9:
                RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) | BIT_7);
                RTL_W8(tp, DBG_reg, RTL_R8(tp, DBG_reg) & ~BIT_3);
                break;
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) | BIT_7);
                break;
        }

        rtl8101_phy_power_up(dev);
}


static void
rtl8101_link_option(u8 *aut,
                    u32 *spd,
                    u8 *dup,
                    u32 *adv)
{
        if ((*spd != SPEED_100) && (*spd != SPEED_10))
                *spd = SPEED_100;

        if ((*dup != DUPLEX_FULL) && (*dup != DUPLEX_HALF))
                *dup = DUPLEX_FULL;

        if ((*aut != AUTONEG_ENABLE) && (*aut != AUTONEG_DISABLE))
                *aut = AUTONEG_ENABLE;

        *adv &= (ADVERTISED_10baseT_Half |
                 ADVERTISED_10baseT_Full |
                 ADVERTISED_100baseT_Half |
                 ADVERTISED_100baseT_Full);
        if (*adv == 0)
                *adv = (ADVERTISED_10baseT_Half |
                        ADVERTISED_10baseT_Full |
                        ADVERTISED_100baseT_Half |
                        ADVERTISED_100baseT_Full);
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static void
rtl8101_get_wol(struct net_device *dev,
                struct ethtool_wolinfo *wol)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u8 options;
        unsigned long flags;

        wol->wolopts = 0;

        if (tp->mcfg == CFG_METHOD_DEFAULT) {
                wol->supported = 0;
                return;
        } else {
                wol->supported = WAKE_ANY;
        }

        spin_lock_irqsave(&tp->lock, flags);

        options = RTL_R8(tp, Config1);
        if (!(options & PMEnable))
                goto out_unlock;

        wol->wolopts = tp->wol_opts;

out_unlock:
        spin_unlock_irqrestore(&tp->lock, flags);
}

static int
rtl8101_set_wol(struct net_device *dev,
                struct ethtool_wolinfo *wol)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

        spin_lock_irqsave(&tp->lock, flags);

        tp->wol_opts = wol->wolopts;

        tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

        spin_unlock_irqrestore(&tp->lock, flags);

        device_set_wakeup_enable(&tp->pci_dev->dev, tp->wol_enabled);

        return 0;
}

static void
rtl8101_get_drvinfo(struct net_device *dev,
                    struct ethtool_drvinfo *info)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        strcpy(info->driver, MODULENAME);
        strcpy(info->version, RTL8101_VERSION);
        strncpy(info->bus_info, pci_name(tp->pci_dev), sizeof(info->bus_info) - 1);
        info->regdump_len = R8101_REGS_DUMP_SIZE;
}

static int
rtl8101_get_regs_len(struct net_device *dev)
{
        return R8101_REGS_DUMP_SIZE;
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

static int
rtl8101_set_speed_xmii(struct net_device *dev,
                       u8 autoneg,
                       u32 speed,
                       u8 duplex,
                       u32 adv)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int auto_nego = 0;

        if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
            tp->mcfg == CFG_METHOD_20) {
                //Disable Giga Lite
                rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                rtl8101_clear_eth_phy_bit(tp, 0x14, BIT_9);
                rtl8101_mdio_write(tp, 0x1F, 0x0A40);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
        }

        if ((speed != SPEED_100) &&
            (speed != SPEED_10)) {
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
        }

        auto_nego = rtl8101_mdio_read(tp, MII_ADVERTISE);

        auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL | ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

        if (autoneg == AUTONEG_ENABLE) {
                /*n-way force*/
                auto_nego = rtl8101_mdio_read(tp, MII_ADVERTISE);
                auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                               ADVERTISE_100HALF | ADVERTISE_100FULL |
                               ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

                if (adv & ADVERTISED_10baseT_Half)
                        auto_nego |= ADVERTISE_10HALF;
                if (adv & ADVERTISED_10baseT_Full)
                        auto_nego |= ADVERTISE_10FULL;
                if (adv & ADVERTISED_100baseT_Half)
                        auto_nego |= ADVERTISE_100HALF;
                if (adv & ADVERTISED_100baseT_Full)
                        auto_nego |= ADVERTISE_100FULL;

                //flow control
                if (tp->fcpause == rtl8101_fc_full)
                        auto_nego |= ADVERTISE_PAUSE_CAP|ADVERTISE_PAUSE_ASYM;

                if ((tp->mcfg == CFG_METHOD_4) || (tp->mcfg == CFG_METHOD_5) ||
                    (tp->mcfg == CFG_METHOD_6) || (tp->mcfg == CFG_METHOD_7) ||
                    (tp->mcfg == CFG_METHOD_8) || (tp->mcfg == CFG_METHOD_9)) {
                        auto_nego &= ~(ADVERTISE_PAUSE_CAP|ADVERTISE_PAUSE_ASYM);
                }

                if ((tp->mcfg == CFG_METHOD_10) || (tp->mcfg == CFG_METHOD_11) ||
                    (tp->mcfg == CFG_METHOD_12) || (tp->mcfg == CFG_METHOD_13) ||
                    (tp->mcfg == CFG_METHOD_14) || (tp->mcfg == CFG_METHOD_15) ||
                    (tp->mcfg == CFG_METHOD_16)) {
                        if (tp->eee_enabled == 1)
                                auto_nego &= ~(ADVERTISE_PAUSE_CAP|ADVERTISE_PAUSE_ASYM);
                }

                tp->phy_auto_nego_reg = auto_nego;

                if ((tp->mcfg == CFG_METHOD_4) ||
                    (tp->mcfg == CFG_METHOD_5)) {
                        rtl8101_mdio_write(tp, 0x1f, 0x0000);
                        rtl8101_mdio_write(tp, MII_BMCR, BMCR_RESET);
                        udelay(100);
                        rtl8101_hw_phy_config(dev);
                } else if (((tp->mcfg == CFG_METHOD_1) ||
                            (tp->mcfg == CFG_METHOD_2) ||
                            (tp->mcfg == CFG_METHOD_3)) &&
                           (speed == SPEED_10)) {
                        rtl8101_mdio_write(tp, 0x1f, 0x0000);
                        rtl8101_mdio_write(tp, MII_BMCR, BMCR_RESET);
                        rtl8101_hw_phy_config(dev);
                }

                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, MII_ADVERTISE, auto_nego);
                if (tp->mcfg == CFG_METHOD_10)
                        rtl8101_mdio_write(tp, MII_BMCR, BMCR_RESET | BMCR_ANENABLE | BMCR_ANRESTART);
                else
                        rtl8101_mdio_write(tp, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);
                mdelay(20);
        } else {
                /*true force*/
                rtl8101_phy_setup_force_mode(dev, speed, duplex);
        }

        tp->autoneg = autoneg;
        tp->speed = speed;
        tp->duplex = duplex;
        tp->advertising = adv;

        return 0;
}

static int
rtl8101_set_speed(struct net_device *dev,
                  u8 autoneg,
                  u32 speed,
                  u8 duplex,
                  u32 adv)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int ret;

        ret = tp->set_speed(dev, autoneg, speed, duplex, adv);

        return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int
rtl8101_set_settings(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                     struct ethtool_cmd *cmd
#else
                     const struct ethtool_link_ksettings *cmd
#endif
                    )
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int ret;
        unsigned long flags;
        u8 autoneg;
        u32 speed;
        u8 duplex;
        u32 supported, advertising;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        autoneg = cmd->autoneg;
        speed = cmd->speed;
        duplex = cmd->duplex;
        supported = cmd->supported;
        advertising = cmd->advertising;
#else
        const struct ethtool_link_settings *base = &cmd->base;
        autoneg = base->autoneg;
        speed = base->speed;
        duplex = base->duplex;
        ethtool_convert_link_mode_to_legacy_u32(&supported,
                                                cmd->link_modes.supported);
        ethtool_convert_link_mode_to_legacy_u32(&advertising,
                                                cmd->link_modes.advertising);
#endif
        if (advertising & ~supported)
                return -EINVAL;

        spin_lock_irqsave(&tp->lock, flags);
        ret = rtl8101_set_speed(dev, autoneg, speed, duplex, advertising);
        spin_unlock_irqrestore(&tp->lock, flags);

        return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
static u32
rtl8101_get_tx_csum(struct net_device *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        return (dev->features & NETIF_F_IP_CSUM) != 0;
#else
        return (dev->features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) != 0;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
}

static u32
rtl8101_get_rx_csum(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        return tp->cp_cmd & RxChkSum;
}

static int
rtl8101_set_tx_csum(struct net_device *dev,
                    u32 data)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        if (data)
                dev->features |= NETIF_F_IP_CSUM;
        else
                dev->features &= ~NETIF_F_IP_CSUM;
#else
        if (data)
                if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3))
                        dev->features |= NETIF_F_IP_CSUM;
                else
                        dev->features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
        else
                dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

        return 0;
}

static int
rtl8101_set_rx_csum(struct net_device *dev,
                    u32 data)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

        spin_lock_irqsave(&tp->lock, flags);

        if (data)
                tp->cp_cmd |= RxChkSum;
        else
                tp->cp_cmd &= ~RxChkSum;

        RTL_W16(tp, CPlusCmd, tp->cp_cmd);

        spin_unlock_irqrestore(&tp->lock, flags);

        return 0;
}
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

#ifdef CONFIG_R8101_VLAN

static inline u32
rtl8101_tx_vlan_tag(struct rtl8101_private *tp,
                    struct sk_buff *skb)
{
        u32 tag;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        tag = (tp->vlgrp && vlan_tx_tag_present(skb)) ?
              TxVlanTag | swab16(vlan_tx_tag_get(skb)) : 0x00;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
        tag = (vlan_tx_tag_present(skb)) ?
              TxVlanTag | swab16(vlan_tx_tag_get(skb)) : 0x00;
#else
        tag = (skb_vlan_tag_present(skb)) ?
              TxVlanTag | swab16(skb_vlan_tag_get(skb)) : 0x00;
#endif

        return tag;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)

static void
rtl8101_vlan_rx_register(struct net_device *dev,
                         struct vlan_group *grp)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
        tp->vlgrp = grp;
        if (tp->vlgrp)
                tp->cp_cmd |= RxVlan;
        else
                tp->cp_cmd &= ~RxVlan;
        RTL_W16(tp, CPlusCmd, tp->cp_cmd);
        spin_unlock_irqrestore(&tp->lock, flags);
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
static void
rtl8101_vlan_rx_kill_vid(struct net_device *dev,
                         unsigned short vid)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
        if (tp->vlgrp)
                tp->vlgrp->vlan_devices[vid] = NULL;
#else
        vlan_group_set_device(tp->vlgrp, vid, NULL);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
        spin_unlock_irqrestore(&tp->lock, flags);
}
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)

static int
rtl8101_rx_vlan_skb(struct rtl8101_private *tp,
                    struct RxDesc *desc,
                    struct sk_buff *skb)
{
        u32 opts2 = le32_to_cpu(desc->opts2);
        int ret = -1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        if (tp->vlgrp && (opts2 & RxVlanTag)) {
                rtl8101_rx_hwaccel_skb(skb, tp->vlgrp, swab16(opts2 & 0xffff));
                ret = 0;
        }
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
        if (opts2 & RxVlanTag)
                __vlan_hwaccel_put_tag(skb, swab16(opts2 & 0xffff));
#else
        if (opts2 & RxVlanTag)
                __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), swab16(opts2 & 0xffff));
#endif

        desc->opts2 = 0;
        return ret;
}

#else /* !CONFIG_R8101_VLAN */

static inline u32
rtl8101_tx_vlan_tag(struct rtl8101_private *tp,
                    struct sk_buff *skb)
{
        return 0;
}

static int
rtl8101_rx_vlan_skb(struct rtl8101_private *tp,
                    struct RxDesc *desc,
                    struct sk_buff *skb)
{
        return -1;
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)

static netdev_features_t rtl8101_fix_features(struct net_device *dev,
                netdev_features_t features)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
        if (dev->mtu > MSS_MAX)
                features &= ~NETIF_F_ALL_TSO;
        spin_unlock_irqrestore(&tp->lock, flags);

        return features;
}

static int rtl8101_hw_set_features(struct net_device *dev,
                                   netdev_features_t features)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (features & NETIF_F_RXCSUM)
                tp->cp_cmd |= RxChkSum;
        else
                tp->cp_cmd &= ~RxChkSum;

        if (dev->features & NETIF_F_HW_VLAN_RX)
                tp->cp_cmd |= RxVlan;
        else
                tp->cp_cmd &= ~RxVlan;

        RTL_W16(tp, CPlusCmd, tp->cp_cmd);
        RTL_R16(tp, CPlusCmd);

        return 0;
}

static int rtl8101_set_features(struct net_device *dev,
                                netdev_features_t features)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        features &= NETIF_F_RXALL | NETIF_F_RXCSUM | NETIF_F_HW_VLAN_RX;

        spin_lock_irqsave(&tp->lock, flags);
        if (features ^ dev->features)
                rtl8101_hw_set_features(dev, features);
        spin_unlock_irqrestore(&tp->lock, flags);

        return 0;
}

#endif

static void
rtl8101_gset_xmii(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                  struct ethtool_cmd *cmd
#else
                  struct ethtool_link_ksettings *cmd
#endif
                 )
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u8 status;
        u8 autoneg, duplex;
        u32 speed = 0;
        u16 bmcr, bmsr, anlpar;
        u32 supported, advertising, lp_advertising;
        unsigned long flags;

        supported = SUPPORTED_10baseT_Half |
                    SUPPORTED_10baseT_Full |
                    SUPPORTED_100baseT_Half |
                    SUPPORTED_100baseT_Full |
                    SUPPORTED_Autoneg |
                    SUPPORTED_TP |
                    SUPPORTED_Pause	|
                    SUPPORTED_Asym_Pause;

        advertising = ADVERTISED_TP;

        spin_lock_irqsave(&tp->lock, flags);
        bmcr = rtl8101_mdio_read(tp, MII_BMCR);
        bmsr = rtl8101_mdio_read(tp, MII_BMSR);
        anlpar = rtl8101_mdio_read(tp, MII_LPA);
        spin_unlock_irqrestore(&tp->lock, flags);

        if (bmcr & BMCR_ANENABLE) {
                advertising |= ADVERTISED_Autoneg;
                autoneg = AUTONEG_ENABLE;

                if (bmsr & BMSR_ANEGCOMPLETE) {
                        lp_advertising = mii_lpa_to_ethtool_lpa_t(anlpar);
                } else {
                        lp_advertising = 0;
                }

                if (tp->phy_auto_nego_reg & ADVERTISE_10HALF)
                        advertising |= ADVERTISED_10baseT_Half;
                if (tp->phy_auto_nego_reg & ADVERTISE_10FULL)
                        advertising |= ADVERTISED_10baseT_Full;
                if (tp->phy_auto_nego_reg & ADVERTISE_100HALF)
                        advertising |= ADVERTISED_100baseT_Half;
                if (tp->phy_auto_nego_reg & ADVERTISE_100FULL)
                        advertising |= ADVERTISED_100baseT_Full;
        } else {
                autoneg = AUTONEG_DISABLE;
                lp_advertising = 0;
        }

        status = RTL_R8(tp, PHYstatus);

        if (status & LinkStatus) {
                /*link on*/
                if (status & _100bps)
                        speed = SPEED_100;
                else if (status & _10bps)
                        speed = SPEED_10;

                if (status & TxFlowCtrl)
                        advertising |= ADVERTISED_Asym_Pause;

                if (status & RxFlowCtrl)
                        advertising |= ADVERTISED_Pause;

                duplex = (status & FullDup) ? DUPLEX_FULL : DUPLEX_HALF;
        } else {
                /*link down*/
                speed = SPEED_UNKNOWN;
                duplex = DUPLEX_UNKNOWN;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        cmd->supported = supported;
        cmd->advertising = advertising;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
        cmd->lp_advertising = lp_advertising;
#endif
        cmd->autoneg = autoneg;
        cmd->speed = speed;
        cmd->duplex = duplex;
        cmd->port = PORT_TP;
#else
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported,
                                                supported);
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising,
                                                advertising);
        ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.lp_advertising,
                                                lp_advertising);
        cmd->base.autoneg = autoneg;
        cmd->base.speed = speed;
        cmd->base.duplex = duplex;
        cmd->base.port = PORT_TP;
#endif
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int
rtl8101_get_settings(struct net_device *dev,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
                     struct ethtool_cmd *cmd
#else
                     struct ethtool_link_ksettings *cmd
#endif
                    )
{
        struct rtl8101_private *tp = netdev_priv(dev);

        tp->get_settings(dev, cmd);

        return 0;
}

static void
rtl8101_get_regs(struct net_device *dev,
                 struct ethtool_regs *regs,
                 void *p)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;
        unsigned int i;
        u8 *data = p;
        unsigned long flags;

        if (regs->len < R8101_REGS_DUMP_SIZE)
                return /* -EINVAL */;

        memset(p, 0, regs->len);

        spin_lock_irqsave(&tp->lock, flags);
        for (i = 0; i < R8101_MAC_REGS_SIZE; i++)
                *data++ = readb(ioaddr + i);
        data = (u8*)p + 256;

        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        for (i = 0; i < R8101_PHY_REGS_SIZE/2; i++) {
                *(u16*)data = rtl8101_mdio_read(tp, i);
                data += 2;
        }
        data = (u8*)p + 256 * 2;

        for (i = 0; i < R8101_EPHY_REGS_SIZE/2; i++) {
                *(u16*)data = rtl8101_ephy_read(tp, i);
                data += 2;
        }
        data = (u8*)p + 256 * 3;

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                /* RTL8101E does not support Extend GMAC */
                break;
        default:
                for (i = 0; i < R8101_ERI_REGS_SIZE; i+=4) {
                        *(u32*)data = rtl8101_eri_read(tp, i , 4, ERIAR_ExGMAC);
                        data += 4;
                }
                break;
        }
        spin_unlock_irqrestore(&tp->lock, flags);
}

static void rtl8101_get_pauseparam(struct net_device *dev,
                                   struct ethtool_pauseparam *pause)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        pause->autoneg = (tp->autoneg ? AUTONEG_ENABLE : AUTONEG_DISABLE);
        if (tp->fcpause == rtl8101_fc_rx_pause)
                pause->rx_pause = 1;
        else if (tp->fcpause == rtl8101_fc_tx_pause)
                pause->tx_pause = 1;
        else if (tp->fcpause == rtl8101_fc_full) {
                pause->rx_pause = 1;
                pause->tx_pause = 1;
        }
}

static int rtl8101_set_pauseparam(struct net_device *dev,
                                  struct ethtool_pauseparam *pause)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        enum rtl8101_fc_mode newfc;

        if (pause->tx_pause || pause->rx_pause)
                newfc = rtl8101_fc_full;
        else
                newfc = rtl8101_fc_none;

        if (tp->fcpause != newfc) {
                tp->fcpause = newfc;

                rtl8101_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
        }

        return 0;

}

static u32
rtl8101_get_msglevel(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        return tp->msg_enable;
}

static void
rtl8101_set_msglevel(struct net_device *dev,
                     u32 value)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        tp->msg_enable = value;
}

static const char rtl8101_gstrings[][ETH_GSTRING_LEN] = {
        "tx_packets",
        "rx_packets",
        "tx_errors",
        "rx_errors",
        "rx_missed",
        "align_errors",
        "tx_single_collisions",
        "tx_multi_collisions",
        "unicast",
        "broadcast",
        "multicast",
        "tx_aborted",
        "tx_underrun",
};
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static int rtl8101_get_stats_count(struct net_device *dev)
{
        return ARRAY_SIZE(rtl8101_gstrings);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
#else
static int rtl8101_get_sset_count(struct net_device *dev, int sset)
{
        switch (sset) {
        case ETH_SS_STATS:
                return ARRAY_SIZE(rtl8101_gstrings);
        default:
                return -EOPNOTSUPP;
        }
}
#endif

static void
rtl8101_wait_for_quiescence(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        synchronize_irq(tp->irq);

        /* Wait for any pending NAPI task to complete */
#ifdef CONFIG_R8101_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        RTL_NAPI_DISABLE(dev, &tp->napi);
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#endif

        rtl8101_irq_mask_and_ack(tp);

#ifdef CONFIG_R8101_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        RTL_NAPI_ENABLE(dev, &tp->napi);
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
static void rtl8101_get_ringparam(struct net_device *dev,
                                  struct ethtool_ringparam *ring,
                                  struct kernel_ethtool_ringparam *kernel_ring,
                                  struct netlink_ext_ack *extack)
#else
static void rtl8101_get_ringparam(struct net_device *dev,
                                  struct ethtool_ringparam *ring)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        ring->rx_max_pending = MAX_NUM_TX_DESC;
        ring->tx_max_pending = MAX_NUM_RX_DESC;;
        ring->rx_pending = tp->num_rx_desc;
        ring->tx_pending = tp->num_tx_desc;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
static int rtl8101_set_ringparam(struct net_device *dev,
                                 struct ethtool_ringparam *ring,
                                 struct kernel_ethtool_ringparam *kernel_ring,
                                 struct netlink_ext_ack *extack)
#else
static int rtl8101_set_ringparam(struct net_device *dev,
                                 struct ethtool_ringparam *ring)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 new_rx_count, new_tx_count;
        int rc = 0;

        if ((ring->rx_mini_pending) || (ring->rx_jumbo_pending))
                return -EINVAL;

        new_tx_count = clamp_t(u32, ring->tx_pending,
                               MIN_NUM_TX_DESC, MAX_NUM_TX_DESC);

        new_rx_count = clamp_t(u32, ring->rx_pending,
                               MIN_NUM_RX_DESC, MAX_NUM_RX_DESC);

        if ((new_rx_count == tp->num_rx_desc) &&
            (new_tx_count == tp->num_tx_desc)) {
                /* nothing to do */
                return 0;
        }

        if (netif_running(dev)) {
                rtl8101_wait_for_quiescence(dev);
                rtl8101_close(dev);
        }

        tp->num_rx_desc = new_rx_count;
        tp->num_tx_desc = new_tx_count;

        if (netif_running(dev))
                rc = rtl8101_open(dev);

        return rc;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static void
rtl8101_get_ethtool_stats(struct net_device *dev,
                          struct ethtool_stats *stats,
                          u64 *data)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct rtl8101_counters *counters;
        dma_addr_t paddr;
        u32 cmd;
        u32 WaitCnt;
        unsigned long flags;

        ASSERT_RTNL();

        counters = tp->tally_vaddr;
        paddr = tp->tally_paddr;
        if (!counters)
                return;

        spin_lock_irqsave(&tp->lock, flags);
        RTL_W32(tp, CounterAddrHigh, (u64)paddr >> 32);
        cmd = (u64)paddr & DMA_BIT_MASK(32);
        RTL_W32(tp, CounterAddrLow, cmd);
        RTL_W32(tp, CounterAddrLow, cmd | CounterDump);

        WaitCnt = 0;
        while (RTL_R32(tp, CounterAddrLow) & CounterDump) {
                udelay(10);

                WaitCnt++;
                if (WaitCnt > 20)
                        break;
        }
        spin_unlock_irqrestore(&tp->lock, flags);

        data[0]	= le64_to_cpu(counters->tx_packets);
        data[1] = le64_to_cpu(counters->rx_packets);
        data[2] = le64_to_cpu(counters->tx_errors);
        data[3] = le32_to_cpu(counters->rx_errors);
        data[4] = le16_to_cpu(counters->rx_missed);
        data[5] = le16_to_cpu(counters->align_errors);
        data[6] = le32_to_cpu(counters->tx_one_collision);
        data[7] = le32_to_cpu(counters->tx_multi_collision);
        data[8] = le64_to_cpu(counters->rx_unicast);
        data[9] = le64_to_cpu(counters->rx_broadcast);
        data[10] = le32_to_cpu(counters->rx_multicast);
        data[11] = le16_to_cpu(counters->tx_aborted);
        data[12] = le16_to_cpu(counters->tx_underrun);
}

static void
rtl8101_get_strings(struct net_device *dev,
                    u32 stringset,
                    u8 *data)
{
        switch (stringset) {
        case ETH_SS_STATS:
                memcpy(data, rtl8101_gstrings, sizeof(rtl8101_gstrings));
                break;
        }
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

#undef ethtool_op_get_link
#define ethtool_op_get_link _kc_ethtool_op_get_link
static u32 _kc_ethtool_op_get_link(struct net_device *dev)
{
        return netif_carrier_ok(dev) ? 1 : 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#undef ethtool_op_get_sg
#define ethtool_op_get_sg _kc_ethtool_op_get_sg
static u32 _kc_ethtool_op_get_sg(struct net_device *dev)
{
#ifdef NETIF_F_SG
        return (dev->features & NETIF_F_SG) != 0;
#else
        return 0;
#endif
}

#undef ethtool_op_set_sg
#define ethtool_op_set_sg _kc_ethtool_op_set_sg
static int _kc_ethtool_op_set_sg(struct net_device *dev, u32 data)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->mcfg == CFG_METHOD_DEFAULT)
                return -EOPNOTSUPP;

#ifdef NETIF_F_SG
        if (data)
                dev->features |= NETIF_F_SG;
        else
                dev->features &= ~NETIF_F_SG;
#endif

        return 0;
}
#endif

static int rtl_nway_reset(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
        int ret, bmcr;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag)) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EBUSY;
        }

        /* if autoneg is off, it's an error */
        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        bmcr = rtl8101_mdio_read(tp, MII_BMCR);

        if (bmcr & BMCR_ANENABLE) {
                bmcr |= BMCR_ANRESTART;
                rtl8101_mdio_write(tp, MII_BMCR, bmcr);
                ret = 0;
        } else {
                ret = -EINVAL;
        }

        spin_unlock_irqrestore(&tp->lock, flags);

        return ret;
}

static bool
rtl8101_support_eee(struct rtl8101_private *tp)
{
        switch (tp->mcfg) {
        case CFG_METHOD_17 ... CFG_METHOD_20:
                return true;
        default:
                return false;
        }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,9,0)
static int
rtl_ethtool_get_eee(struct net_device *net, struct ethtool_keee *edata)
{
        __ETHTOOL_DECLARE_LINK_MODE_MASK(common);
        struct rtl8101_private *tp = netdev_priv(net);
        unsigned long flags;
        u16 val;

        if (!rtl8101_support_eee(tp))
                return -EOPNOTSUPP;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag)) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EBUSY;
        }

        rtl8101_mdio_write(tp, 0x1F, 0x0A5C);
        val = rtl8101_mdio_read(tp, 0x12);
        val &= BIT_1;
        mii_eee_cap1_mod_linkmode_t(edata->supported, val);

        rtl8101_mdio_write(tp, 0x1F, 0x0A5D);
        val = rtl8101_mdio_read(tp, 0x10);
        mii_eee_cap1_mod_linkmode_t(edata->advertised, val);

        val = rtl8101_mdio_read(tp, 0x11);
        mii_eee_cap1_mod_linkmode_t(edata->lp_advertised, val);

        val = rtl8101_eri_read(tp, 0x1B0, 2, ERIAR_ExGMAC);
        val &= BIT_1 | BIT_0;

        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        spin_unlock_irqrestore(&tp->lock, flags);

        edata->eee_enabled = !!val;
        linkmode_and(common, edata->advertised, edata->lp_advertised);
        edata->eee_active = !linkmode_empty(common);

        return 0;
}

static int
rtl_ethtool_set_eee(struct net_device *net, struct ethtool_keee *eee)
{
        struct rtl8101_private *tp = netdev_priv(net);
        unsigned long flags;

        if (!rtl8101_support_eee(tp))
                return -EOPNOTSUPP;

        if (!HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp))
                return -EOPNOTSUPP;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag)) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EBUSY;
        }

        if (tp->autoneg != AUTONEG_ENABLE) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EINVAL;
        }

        tp->eee_enabled = eee->eee_enabled;
        tp->eee_adv_t = linkmode_to_mii_eee_cap1_t(eee->advertised);

        if (eee->eee_enabled)
                rtl8101_enable_eee(tp);
        else
                rtl8101_disable_eee(tp);

        spin_unlock_irqrestore(&tp->lock, flags);

        rtl_nway_reset(net);

        return 0;
}
#else
static int
rtl_ethtool_get_eee(struct net_device *net, struct ethtool_eee *eee)
{
        struct rtl8101_private *tp = netdev_priv(net);
        u32 lp, adv, supported = 0;
        unsigned long flags;
        u16 val;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                break;
        default:
                return -EOPNOTSUPP;
        }

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag)) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EBUSY;
        }

        rtl8101_mdio_write(tp, 0x1F, 0x0A5C);
        val = rtl8101_mdio_read(tp, 0x12);
        val &= BIT_1;
        supported = mmd_eee_cap_to_ethtool_sup_t(val);

        rtl8101_mdio_write(tp, 0x1F, 0x0A5D);
        val = rtl8101_mdio_read(tp, 0x10);
        adv = mmd_eee_adv_to_ethtool_adv_t(val);

        val = rtl8101_mdio_read(tp, 0x11);
        lp = mmd_eee_adv_to_ethtool_adv_t(val);

        val = rtl8101_eri_read(tp, 0x1B0, 2, ERIAR_ExGMAC);
        val &= BIT_1 | BIT_0;

        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        spin_unlock_irqrestore(&tp->lock, flags);

        eee->eee_enabled = !!val;
        eee->eee_active = !!(supported & adv & lp);
        eee->supported = supported;
        eee->advertised = adv;
        eee->lp_advertised = lp;

        return 0;
}

static int
rtl_ethtool_set_eee(struct net_device *net, struct ethtool_eee *eee)
{
        struct rtl8101_private *tp = netdev_priv(net);
        unsigned long flags;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                break;
        default:
                return -EOPNOTSUPP;
        }

        if (!HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp))
                return -EOPNOTSUPP;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag)) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EBUSY;
        }

        if (tp->autoneg != AUTONEG_ENABLE) {
                spin_unlock_irqrestore(&tp->lock, flags);
                return -EINVAL;
        }

        tp->eee_enabled = eee->eee_enabled;
        tp->eee_adv_t = ethtool_adv_to_mmd_eee_adv_t(eee->advertised);

        if (tp->eee_enabled)
                rtl8101_enable_eee(tp);
        else
                rtl8101_disable_eee(tp);

        spin_unlock_irqrestore(&tp->lock, flags);

        rtl_nway_reset(net);

        return 0;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6,9,0) */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
static void rtl8101_get_channels(struct net_device *dev,
                                 struct ethtool_channels *channel)
{
        channel->max_rx = 1;
        channel->max_tx = 1;
        channel->rx_count = 1;
        channel->tx_count = 1;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
static const struct ethtool_ops rtl8101_ethtool_ops = {
        .get_drvinfo		= rtl8101_get_drvinfo,
        .get_regs_len		= rtl8101_get_regs_len,
        .get_link		= ethtool_op_get_link,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_ringparam      = rtl8101_get_ringparam,
        .set_ringparam      = rtl8101_set_ringparam,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        .get_settings       = rtl8101_get_settings,
        .set_settings       = rtl8101_set_settings,
#else
        .get_link_ksettings       = rtl8101_get_settings,
        .set_link_ksettings       = rtl8101_set_settings,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_pauseparam     = rtl8101_get_pauseparam,
        .set_pauseparam     = rtl8101_set_pauseparam,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        .get_msglevel		= rtl8101_get_msglevel,
        .set_msglevel		= rtl8101_set_msglevel,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        .get_rx_csum		= rtl8101_get_rx_csum,
        .set_rx_csum		= rtl8101_set_rx_csum,
        .get_tx_csum		= rtl8101_get_tx_csum,
        .set_tx_csum		= rtl8101_set_tx_csum,
        .get_sg			= ethtool_op_get_sg,
        .set_sg			= ethtool_op_set_sg,
#ifdef NETIF_F_TSO
        .get_tso		= ethtool_op_get_tso,
        .set_tso		= ethtool_op_set_tso,
#endif //NETIF_F_TSO
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        .get_regs		= rtl8101_get_regs,
        .get_wol		= rtl8101_get_wol,
        .set_wol		= rtl8101_set_wol,
        .get_strings		= rtl8101_get_strings,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        .get_stats_count    = rtl8101_get_stats_count,
#else
        .get_sset_count     = rtl8101_get_sset_count,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
        .get_ethtool_stats	= rtl8101_get_ethtool_stats,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#ifdef ETHTOOL_GPERMADDR
        .get_perm_addr		= ethtool_op_get_perm_addr,
#endif
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
        .get_eee = rtl_ethtool_get_eee,
        .set_eee = rtl_ethtool_set_eee,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
        .get_channels		= rtl8101_get_channels,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0) */
        .nway_reset = rtl_nway_reset,
};
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)

static void rtl8101_get_mac_version(struct rtl8101_private *tp)
{
        u32 reg,val32;
        u32 ICVerID;

        val32 = RTL_R32(tp, TxConfig);
        reg = val32 & 0x7c800000;
        ICVerID = val32 & 0x00700000;

        switch (reg) {
        case 0x34000000:
                if (ICVerID == 0x00000000) {
                        tp->mcfg = CFG_METHOD_1;
                } else if (ICVerID == 0x00200000) {
                        tp->mcfg = CFG_METHOD_2;
                } else if (ICVerID == 0x00300000) {
                        tp->mcfg = CFG_METHOD_3;
                } else {
                        tp->mcfg = CFG_METHOD_3;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x34800000:
        case 0x24800000:
                if (ICVerID == 0x00100000) {
                        tp->mcfg = CFG_METHOD_4;
                } else if (ICVerID == 0x00200000) {
                        tp->mcfg = CFG_METHOD_5;
                } else if (ICVerID == 0x00400000) {
                        tp->mcfg = CFG_METHOD_6;
                } else if (ICVerID == 0x00500000) {
                        tp->mcfg = CFG_METHOD_7;
                } else if (ICVerID == 0x00600000) {
                        tp->mcfg = CFG_METHOD_8;
                } else {
                        tp->mcfg = CFG_METHOD_8;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x24000000:
                tp->mcfg = CFG_METHOD_9;
                break;
        case 0x2C000000:
                if (ICVerID == 0x00000000 ||
                    ICVerID == 0x00100000 ||
                    ICVerID == 0x00200000) {
                        tp->mcfg = CFG_METHOD_10;
                } else {
                        tp->mcfg = CFG_METHOD_10;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x40800000:
                if (ICVerID == 0x00100000) {
                        tp->mcfg = CFG_METHOD_11;
                } else if (ICVerID == 0x00200000) {
                        tp->mcfg = CFG_METHOD_12;
                } else if (ICVerID == 0x00300000) {
                        tp->mcfg = CFG_METHOD_13;
                } else if (ICVerID == 0x00400000) {
                        tp->mcfg = CFG_METHOD_13;
                } else {
                        tp->mcfg = CFG_METHOD_13;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x44000000:
                tp->mcfg = CFG_METHOD_14;
                break;
        case 0x44800000:
                if (ICVerID == 0x00000000) {
                        tp->mcfg = CFG_METHOD_15;
                } else if (ICVerID == 0x00100000) {
                        tp->mcfg = CFG_METHOD_16;
                } else {
                        tp->mcfg = CFG_METHOD_16;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x50800000:
                if (ICVerID == 0x00100000) {
                        tp->mcfg = CFG_METHOD_17;
                } else {
                        tp->mcfg = CFG_METHOD_17;
                        tp->HwIcVerUnknown = TRUE;
                }
                break;
        case 0x54000000:
                if (ICVerID == 0x00000000) {
                        tp->mcfg = CFG_METHOD_18;
                } else if (ICVerID == 0x00100000) {
                        tp->mcfg = CFG_METHOD_19;
                } else {
                        tp->mcfg = CFG_METHOD_19;
                        tp->HwIcVerUnknown = TRUE;
                }

                if (tp->mcfg == CFG_METHOD_19 &&
                    (rtl8101_mac_ocp_read(tp, 0xD006) & 0xFF00) == 0x0100)
                        tp->mcfg = CFG_METHOD_20;
                break;
        default:
                printk("unknown chip version (%x)\n",reg);
                tp->mcfg = CFG_METHOD_DEFAULT;
                tp->HwIcVerUnknown = TRUE;
                break;
        }
}

static void
rtl8101_print_mac_version(struct rtl8101_private *tp)
{
        int i;
        for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
                if (tp->mcfg == rtl_chip_info[i].mcfg) {
                        dprintk("Realtek PCIe FE Family Controller mcfg = %04d\n",
                                rtl_chip_info[i].mcfg);
                        return;
                }
        }

        dprintk("mac_version == Unknown\n");
}

static void
rtl8101_tally_counter_addr_fill(struct rtl8101_private *tp)
{
        if (!tp->tally_paddr)
                return;

        RTL_W32(tp, CounterAddrHigh, (u64)tp->tally_paddr >> 32);
        RTL_W32(tp, CounterAddrLow, (u64)tp->tally_paddr & (DMA_BIT_MASK(32)));
}

static int
rtl8101_is_ups_resume(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        return (rtl8101_mac_ocp_read(tp, 0xD408) & BIT_0);
}

static void
rtl8101_clear_ups_resume_bit(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mac_ocp_write(tp, 0xD408, rtl8101_mac_ocp_read(tp, 0xD408) & ~(BIT_0));
}

static void
rtl8101_wait_phy_ups_resume(struct net_device *dev, u16 PhyState)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 TmpPhyState;
        int i=0;

        do {
                TmpPhyState = rtl8101_mdio_read_phy_ocp(tp, 0x0A42, 0x10);
                TmpPhyState &= 0x7;
                mdelay(1);
                i++;
        } while ((i < 100) && (TmpPhyState != PhyState));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        WARN_ON_ONCE(i == 100);
#endif
}

static void
rtl8101_tally_counter_clear(struct rtl8101_private *tp)
{
        if (tp->mcfg == CFG_METHOD_1 || tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 )
                return;

        if (!tp->tally_paddr)
                return;

        RTL_W32(tp, CounterAddrHigh, (u64)tp->tally_paddr >> 32);
        RTL_W32(tp, CounterAddrLow, ((u64)tp->tally_paddr & (DMA_BIT_MASK(32))) | CounterReset);
}

/*
static void
rtl8101_enable_now_is_oob(struct rtl8101_private *tp)
{
        if( tp->HwSuppNowIsOobVer == 1 ) {
                RTL_W8(tp, MCUCmd_reg, RTL_R8(tp, MCUCmd_reg) | Now_is_oob);
        }
}
*/

static void
rtl8101_disable_now_is_oob(struct rtl8101_private *tp)
{
        if( tp->HwSuppNowIsOobVer == 1 ) {
                RTL_W8(tp, MCUCmd_reg, RTL_R8(tp, MCUCmd_reg) & ~Now_is_oob);
        }
}

static void
rtl8101_exit_oob(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 data16;

        RTL_W32(tp, RxConfig, RTL_R32(tp, RxConfig) & ~(AcceptErr | AcceptRunt | AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys));

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_disable_now_is_oob(tp);

                data16 = rtl8101_mac_ocp_read(tp, 0xE8DE) & ~BIT_14;
                rtl8101_mac_ocp_write(tp, 0xE8DE, data16);
                rtl8101_wait_ll_share_fifo_ready(dev);

                data16 = rtl8101_mac_ocp_read(tp, 0xE8DE) | BIT_15;
                rtl8101_mac_ocp_write(tp, 0xE8DE, data16);

                rtl8101_wait_ll_share_fifo_ready(dev);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                rtl8101_disable_pci_offset_99(tp);
                break;
        };

        //wait ups resume (phy state 2)
        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                if (rtl8101_is_ups_resume(dev)) {
                        rtl8101_wait_phy_ups_resume(dev, 2);
                        rtl8101_clear_ups_resume_bit(dev);
                }
                break;
        };
}

static void
rtl8101_hw_disable_mac_mcu_bps(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_enable_cfg9346_write(tp);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                rtl8101_disable_cfg9346_write(tp);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mac_ocp_write(tp, 0xFC38, 0x0000);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mac_ocp_write(tp, 0xFC28, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC2A, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC2C, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC2E, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC30, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC32, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC34, 0x0000);
                rtl8101_mac_ocp_write(tp, 0xFC36, 0x0000);
                mdelay(3);
                rtl8101_mac_ocp_write(tp, 0xFC26, 0x0000);
                break;
        }
}

static void
rtl8101_set_mac_mcu_8106eus_1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_hw_disable_mac_mcu_bps(dev);

        rtl8101_mac_ocp_write( tp, 0xF800, 0xE008 );
        rtl8101_mac_ocp_write( tp, 0xF802, 0xE00A );
        rtl8101_mac_ocp_write( tp, 0xF804, 0xE00D );
        rtl8101_mac_ocp_write( tp, 0xF806, 0xE02F );
        rtl8101_mac_ocp_write( tp, 0xF808, 0xE031 );
        rtl8101_mac_ocp_write( tp, 0xF80A, 0xE038 );
        rtl8101_mac_ocp_write( tp, 0xF80C, 0xE03A );
        rtl8101_mac_ocp_write( tp, 0xF80E, 0xE051 );
        rtl8101_mac_ocp_write( tp, 0xF810, 0xC202 );
        rtl8101_mac_ocp_write( tp, 0xF812, 0xBA00 );
        rtl8101_mac_ocp_write( tp, 0xF814, 0x0DFC );
        rtl8101_mac_ocp_write( tp, 0xF816, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF818, 0xC502 );
        rtl8101_mac_ocp_write( tp, 0xF81A, 0xBD00 );
        rtl8101_mac_ocp_write( tp, 0xF81C, 0x0A30 );
        rtl8101_mac_ocp_write( tp, 0xF81E, 0x49D9 );
        rtl8101_mac_ocp_write( tp, 0xF820, 0xF019 );
        rtl8101_mac_ocp_write( tp, 0xF822, 0xC520 );
        rtl8101_mac_ocp_write( tp, 0xF824, 0x64A5 );
        rtl8101_mac_ocp_write( tp, 0xF826, 0x1400 );
        rtl8101_mac_ocp_write( tp, 0xF828, 0xF007 );
        rtl8101_mac_ocp_write( tp, 0xF82A, 0x0C01 );
        rtl8101_mac_ocp_write( tp, 0xF82C, 0x8CA5 );
        rtl8101_mac_ocp_write( tp, 0xF82E, 0x1C15 );
        rtl8101_mac_ocp_write( tp, 0xF830, 0xC515 );
        rtl8101_mac_ocp_write( tp, 0xF832, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xF834, 0xE00F );
        rtl8101_mac_ocp_write( tp, 0xF836, 0xC513 );
        rtl8101_mac_ocp_write( tp, 0xF838, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xF83A, 0x48C8 );
        rtl8101_mac_ocp_write( tp, 0xF83C, 0x48CA );
        rtl8101_mac_ocp_write( tp, 0xF83E, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xF840, 0xC510 );
        rtl8101_mac_ocp_write( tp, 0xF842, 0x1B00 );
        rtl8101_mac_ocp_write( tp, 0xF844, 0x9BA0 );
        rtl8101_mac_ocp_write( tp, 0xF846, 0x1B1C );
        rtl8101_mac_ocp_write( tp, 0xF848, 0x483F );
        rtl8101_mac_ocp_write( tp, 0xF84A, 0x9BA2 );
        rtl8101_mac_ocp_write( tp, 0xF84C, 0x1B04 );
        rtl8101_mac_ocp_write( tp, 0xF84E, 0xC506 );
        rtl8101_mac_ocp_write( tp, 0xF850, 0x9BA0 );
        rtl8101_mac_ocp_write( tp, 0xF852, 0xC603 );
        rtl8101_mac_ocp_write( tp, 0xF854, 0xBE00 );
        rtl8101_mac_ocp_write( tp, 0xF856, 0x0298 );
        rtl8101_mac_ocp_write( tp, 0xF858, 0x03DE );
        rtl8101_mac_ocp_write( tp, 0xF85A, 0xE434 );
        rtl8101_mac_ocp_write( tp, 0xF85C, 0xE096 );
        rtl8101_mac_ocp_write( tp, 0xF85E, 0xE860 );
        rtl8101_mac_ocp_write( tp, 0xF860, 0xDE20 );
        rtl8101_mac_ocp_write( tp, 0xF862, 0xD3C0 );
        rtl8101_mac_ocp_write( tp, 0xF864, 0xC602 );
        rtl8101_mac_ocp_write( tp, 0xF866, 0xBE00 );
        rtl8101_mac_ocp_write( tp, 0xF868, 0x0A64 );
        rtl8101_mac_ocp_write( tp, 0xF86A, 0xC707 );
        rtl8101_mac_ocp_write( tp, 0xF86C, 0x1D00 );
        rtl8101_mac_ocp_write( tp, 0xF86E, 0x8DE2 );
        rtl8101_mac_ocp_write( tp, 0xF870, 0x48C1 );
        rtl8101_mac_ocp_write( tp, 0xF872, 0xC502 );
        rtl8101_mac_ocp_write( tp, 0xF874, 0xBD00 );
        rtl8101_mac_ocp_write( tp, 0xF876, 0x00AA );
        rtl8101_mac_ocp_write( tp, 0xF878, 0xE0C0 );
        rtl8101_mac_ocp_write( tp, 0xF87A, 0xC502 );
        rtl8101_mac_ocp_write( tp, 0xF87C, 0xBD00 );
        rtl8101_mac_ocp_write( tp, 0xF87E, 0x0132 );
        rtl8101_mac_ocp_write( tp, 0xF880, 0xC50C );
        rtl8101_mac_ocp_write( tp, 0xF882, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xF884, 0x49CE );
        rtl8101_mac_ocp_write( tp, 0xF886, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xF888, 0x1C00 );
        rtl8101_mac_ocp_write( tp, 0xF88A, 0x9EA0 );
        rtl8101_mac_ocp_write( tp, 0xF88C, 0x1C1C );
        rtl8101_mac_ocp_write( tp, 0xF88E, 0x484F );
        rtl8101_mac_ocp_write( tp, 0xF890, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xF892, 0xC402 );
        rtl8101_mac_ocp_write( tp, 0xF894, 0xBC00 );
        rtl8101_mac_ocp_write( tp, 0xF896, 0x0AFA );
        rtl8101_mac_ocp_write( tp, 0xF898, 0xDE20 );
        rtl8101_mac_ocp_write( tp, 0xF89A, 0xE000 );
        rtl8101_mac_ocp_write( tp, 0xF89C, 0xE092 );
        rtl8101_mac_ocp_write( tp, 0xF89E, 0xE430 );
        rtl8101_mac_ocp_write( tp, 0xF8A0, 0xDE20 );
        rtl8101_mac_ocp_write( tp, 0xF8A2, 0xE0C0 );
        rtl8101_mac_ocp_write( tp, 0xF8A4, 0xE860 );
        rtl8101_mac_ocp_write( tp, 0xF8A6, 0xE84C );
        rtl8101_mac_ocp_write( tp, 0xF8A8, 0xB400 );
        rtl8101_mac_ocp_write( tp, 0xF8AA, 0xB430 );
        rtl8101_mac_ocp_write( tp, 0xF8AC, 0xE410 );
        rtl8101_mac_ocp_write( tp, 0xF8AE, 0xC0AE );
        rtl8101_mac_ocp_write( tp, 0xF8B0, 0xB407 );
        rtl8101_mac_ocp_write( tp, 0xF8B2, 0xB406 );
        rtl8101_mac_ocp_write( tp, 0xF8B4, 0xB405 );
        rtl8101_mac_ocp_write( tp, 0xF8B6, 0xB404 );
        rtl8101_mac_ocp_write( tp, 0xF8B8, 0xB403 );
        rtl8101_mac_ocp_write( tp, 0xF8BA, 0xB402 );
        rtl8101_mac_ocp_write( tp, 0xF8BC, 0xB401 );
        rtl8101_mac_ocp_write( tp, 0xF8BE, 0xC7EE );
        rtl8101_mac_ocp_write( tp, 0xF8C0, 0x76F4 );
        rtl8101_mac_ocp_write( tp, 0xF8C2, 0xC2ED );
        rtl8101_mac_ocp_write( tp, 0xF8C4, 0xC3ED );
        rtl8101_mac_ocp_write( tp, 0xF8C6, 0xC1EF );
        rtl8101_mac_ocp_write( tp, 0xF8C8, 0xC5F3 );
        rtl8101_mac_ocp_write( tp, 0xF8CA, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xF8CC, 0x49CD );
        rtl8101_mac_ocp_write( tp, 0xF8CE, 0xF001 );
        rtl8101_mac_ocp_write( tp, 0xF8D0, 0xC5EE );
        rtl8101_mac_ocp_write( tp, 0xF8D2, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xF8D4, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xF8D6, 0xF105 );
        rtl8101_mac_ocp_write( tp, 0xF8D8, 0xC5E4 );
        rtl8101_mac_ocp_write( tp, 0xF8DA, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xF8DC, 0x49CE );
        rtl8101_mac_ocp_write( tp, 0xF8DE, 0xF00B );
        rtl8101_mac_ocp_write( tp, 0xF8E0, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF8E2, 0x484B );
        rtl8101_mac_ocp_write( tp, 0xF8E4, 0x9C44 );
        rtl8101_mac_ocp_write( tp, 0xF8E6, 0x1C10 );
        rtl8101_mac_ocp_write( tp, 0xF8E8, 0x9C62 );
        rtl8101_mac_ocp_write( tp, 0xF8EA, 0x1C11 );
        rtl8101_mac_ocp_write( tp, 0xF8EC, 0x8C60 );
        rtl8101_mac_ocp_write( tp, 0xF8EE, 0x1C00 );
        rtl8101_mac_ocp_write( tp, 0xF8F0, 0x9CF6 );
        rtl8101_mac_ocp_write( tp, 0xF8F2, 0xE0EC );
        rtl8101_mac_ocp_write( tp, 0xF8F4, 0x49E7 );
        rtl8101_mac_ocp_write( tp, 0xF8F6, 0xF016 );
        rtl8101_mac_ocp_write( tp, 0xF8F8, 0x1D80 );
        rtl8101_mac_ocp_write( tp, 0xF8FA, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xF8FC, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF8FE, 0x4843 );
        rtl8101_mac_ocp_write( tp, 0xF900, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF902, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF904, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF906, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF908, 0x48C8 );
        rtl8101_mac_ocp_write( tp, 0xF90A, 0x48C9 );
        rtl8101_mac_ocp_write( tp, 0xF90C, 0x48CA );
        rtl8101_mac_ocp_write( tp, 0xF90E, 0x9C44 );
        rtl8101_mac_ocp_write( tp, 0xF910, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF912, 0x4844 );
        rtl8101_mac_ocp_write( tp, 0xF914, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF916, 0x1E01 );
        rtl8101_mac_ocp_write( tp, 0xF918, 0xE8DB );
        rtl8101_mac_ocp_write( tp, 0xF91A, 0x7420 );
        rtl8101_mac_ocp_write( tp, 0xF91C, 0x48C1 );
        rtl8101_mac_ocp_write( tp, 0xF91E, 0x9C20 );
        rtl8101_mac_ocp_write( tp, 0xF920, 0xE0D5 );
        rtl8101_mac_ocp_write( tp, 0xF922, 0x49E6 );
        rtl8101_mac_ocp_write( tp, 0xF924, 0xF02A );
        rtl8101_mac_ocp_write( tp, 0xF926, 0x1D40 );
        rtl8101_mac_ocp_write( tp, 0xF928, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xF92A, 0x74FC );
        rtl8101_mac_ocp_write( tp, 0xF92C, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xF92E, 0xF124 );
        rtl8101_mac_ocp_write( tp, 0xF930, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xF932, 0xF122 );
        rtl8101_mac_ocp_write( tp, 0xF934, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF936, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xF938, 0xF01F );
        rtl8101_mac_ocp_write( tp, 0xF93A, 0xE8D3 );
        rtl8101_mac_ocp_write( tp, 0xF93C, 0x48C4 );
        rtl8101_mac_ocp_write( tp, 0xF93E, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF940, 0x1E00 );
        rtl8101_mac_ocp_write( tp, 0xF942, 0xE8C6 );
        rtl8101_mac_ocp_write( tp, 0xF944, 0xC5B1 );
        rtl8101_mac_ocp_write( tp, 0xF946, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xF948, 0x49C3 );
        rtl8101_mac_ocp_write( tp, 0xF94A, 0xF016 );
        rtl8101_mac_ocp_write( tp, 0xF94C, 0xC5AF );
        rtl8101_mac_ocp_write( tp, 0xF94E, 0x74A4 );
        rtl8101_mac_ocp_write( tp, 0xF950, 0x49C2 );
        rtl8101_mac_ocp_write( tp, 0xF952, 0xF005 );
        rtl8101_mac_ocp_write( tp, 0xF954, 0xC5AA );
        rtl8101_mac_ocp_write( tp, 0xF956, 0x74B2 );
        rtl8101_mac_ocp_write( tp, 0xF958, 0x49C9 );
        rtl8101_mac_ocp_write( tp, 0xF95A, 0xF10E );
        rtl8101_mac_ocp_write( tp, 0xF95C, 0xC5A6 );
        rtl8101_mac_ocp_write( tp, 0xF95E, 0x74A8 );
        rtl8101_mac_ocp_write( tp, 0xF960, 0x4845 );
        rtl8101_mac_ocp_write( tp, 0xF962, 0x4846 );
        rtl8101_mac_ocp_write( tp, 0xF964, 0x4847 );
        rtl8101_mac_ocp_write( tp, 0xF966, 0x4848 );
        rtl8101_mac_ocp_write( tp, 0xF968, 0x9CA8 );
        rtl8101_mac_ocp_write( tp, 0xF96A, 0x74B2 );
        rtl8101_mac_ocp_write( tp, 0xF96C, 0x4849 );
        rtl8101_mac_ocp_write( tp, 0xF96E, 0x9CB2 );
        rtl8101_mac_ocp_write( tp, 0xF970, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xF972, 0x484F );
        rtl8101_mac_ocp_write( tp, 0xF974, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xF976, 0xE0AA );
        rtl8101_mac_ocp_write( tp, 0xF978, 0x49E4 );
        rtl8101_mac_ocp_write( tp, 0xF97A, 0xF018 );
        rtl8101_mac_ocp_write( tp, 0xF97C, 0x1D10 );
        rtl8101_mac_ocp_write( tp, 0xF97E, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xF980, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF982, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF984, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF986, 0x4843 );
        rtl8101_mac_ocp_write( tp, 0xF988, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF98A, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF98C, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF98E, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF990, 0x4844 );
        rtl8101_mac_ocp_write( tp, 0xF992, 0x4842 );
        rtl8101_mac_ocp_write( tp, 0xF994, 0x4841 );
        rtl8101_mac_ocp_write( tp, 0xF996, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF998, 0x1E01 );
        rtl8101_mac_ocp_write( tp, 0xF99A, 0xE89A );
        rtl8101_mac_ocp_write( tp, 0xF99C, 0x7420 );
        rtl8101_mac_ocp_write( tp, 0xF99E, 0x4841 );
        rtl8101_mac_ocp_write( tp, 0xF9A0, 0x9C20 );
        rtl8101_mac_ocp_write( tp, 0xF9A2, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF9A4, 0x4848 );
        rtl8101_mac_ocp_write( tp, 0xF9A6, 0x9C44 );
        rtl8101_mac_ocp_write( tp, 0xF9A8, 0xE091 );
        rtl8101_mac_ocp_write( tp, 0xF9AA, 0x49E5 );
        rtl8101_mac_ocp_write( tp, 0xF9AC, 0xF03E );
        rtl8101_mac_ocp_write( tp, 0xF9AE, 0x1D20 );
        rtl8101_mac_ocp_write( tp, 0xF9B0, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xF9B2, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF9B4, 0x48C2 );
        rtl8101_mac_ocp_write( tp, 0xF9B6, 0x4841 );
        rtl8101_mac_ocp_write( tp, 0xF9B8, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF9BA, 0x1E01 );
        rtl8101_mac_ocp_write( tp, 0xF9BC, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF9BE, 0x49CA );
        rtl8101_mac_ocp_write( tp, 0xF9C0, 0xF103 );
        rtl8101_mac_ocp_write( tp, 0xF9C2, 0x49C2 );
        rtl8101_mac_ocp_write( tp, 0xF9C4, 0xF00C );
        rtl8101_mac_ocp_write( tp, 0xF9C6, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xF9C8, 0xF004 );
        rtl8101_mac_ocp_write( tp, 0xF9CA, 0x6447 );
        rtl8101_mac_ocp_write( tp, 0xF9CC, 0x2244 );
        rtl8101_mac_ocp_write( tp, 0xF9CE, 0xE002 );
        rtl8101_mac_ocp_write( tp, 0xF9D0, 0x1C01 );
        rtl8101_mac_ocp_write( tp, 0xF9D2, 0x9C62 );
        rtl8101_mac_ocp_write( tp, 0xF9D4, 0x1C11 );
        rtl8101_mac_ocp_write( tp, 0xF9D6, 0x8C60 );
        rtl8101_mac_ocp_write( tp, 0xF9D8, 0x1C00 );
        rtl8101_mac_ocp_write( tp, 0xF9DA, 0x9CF6 );
        rtl8101_mac_ocp_write( tp, 0xF9DC, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xF9DE, 0x49C8 );
        rtl8101_mac_ocp_write( tp, 0xF9E0, 0xF01D );
        rtl8101_mac_ocp_write( tp, 0xF9E2, 0x74FC );
        rtl8101_mac_ocp_write( tp, 0xF9E4, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xF9E6, 0xF11A );
        rtl8101_mac_ocp_write( tp, 0xF9E8, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xF9EA, 0xF118 );
        rtl8101_mac_ocp_write( tp, 0xF9EC, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xF9EE, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xF9F0, 0xF015 );
        rtl8101_mac_ocp_write( tp, 0xF9F2, 0x49C6 );
        rtl8101_mac_ocp_write( tp, 0xF9F4, 0xF113 );
        rtl8101_mac_ocp_write( tp, 0xF9F6, 0xE875 );
        rtl8101_mac_ocp_write( tp, 0xF9F8, 0x48C4 );
        rtl8101_mac_ocp_write( tp, 0xF9FA, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xF9FC, 0x7420 );
        rtl8101_mac_ocp_write( tp, 0xF9FE, 0x48C1 );
        rtl8101_mac_ocp_write( tp, 0xFA00, 0x9C20 );
        rtl8101_mac_ocp_write( tp, 0xFA02, 0xC50A );
        rtl8101_mac_ocp_write( tp, 0xFA04, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFA06, 0x8CA5 );
        rtl8101_mac_ocp_write( tp, 0xFA08, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xFA0A, 0xC505 );
        rtl8101_mac_ocp_write( tp, 0xFA0C, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xFA0E, 0x1C11 );
        rtl8101_mac_ocp_write( tp, 0xFA10, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xFA12, 0xE00A );
        rtl8101_mac_ocp_write( tp, 0xFA14, 0xE434 );
        rtl8101_mac_ocp_write( tp, 0xFA16, 0xD3C0 );
        rtl8101_mac_ocp_write( tp, 0xFA18, 0xDC00 );
        rtl8101_mac_ocp_write( tp, 0xFA1A, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xFA1C, 0x49CA );
        rtl8101_mac_ocp_write( tp, 0xFA1E, 0xF004 );
        rtl8101_mac_ocp_write( tp, 0xFA20, 0x48CA );
        rtl8101_mac_ocp_write( tp, 0xFA22, 0x9C44 );
        rtl8101_mac_ocp_write( tp, 0xFA24, 0xE855 );
        rtl8101_mac_ocp_write( tp, 0xFA26, 0xE052 );
        rtl8101_mac_ocp_write( tp, 0xFA28, 0x49E8 );
        rtl8101_mac_ocp_write( tp, 0xFA2A, 0xF024 );
        rtl8101_mac_ocp_write( tp, 0xFA2C, 0x1D01 );
        rtl8101_mac_ocp_write( tp, 0xFA2E, 0x8DF5 );
        rtl8101_mac_ocp_write( tp, 0xFA30, 0x7440 );
        rtl8101_mac_ocp_write( tp, 0xFA32, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xFA34, 0xF11E );
        rtl8101_mac_ocp_write( tp, 0xFA36, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xFA38, 0x49C8 );
        rtl8101_mac_ocp_write( tp, 0xFA3A, 0xF01B );
        rtl8101_mac_ocp_write( tp, 0xFA3C, 0x49CA );
        rtl8101_mac_ocp_write( tp, 0xFA3E, 0xF119 );
        rtl8101_mac_ocp_write( tp, 0xFA40, 0xC5EC );
        rtl8101_mac_ocp_write( tp, 0xFA42, 0x76A4 );
        rtl8101_mac_ocp_write( tp, 0xFA44, 0x49E3 );
        rtl8101_mac_ocp_write( tp, 0xFA46, 0xF015 );
        rtl8101_mac_ocp_write( tp, 0xFA48, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xFA4A, 0xF103 );
        rtl8101_mac_ocp_write( tp, 0xFA4C, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xFA4E, 0xF011 );
        rtl8101_mac_ocp_write( tp, 0xFA50, 0x4849 );
        rtl8101_mac_ocp_write( tp, 0xFA52, 0x9C44 );
        rtl8101_mac_ocp_write( tp, 0xFA54, 0x1C00 );
        rtl8101_mac_ocp_write( tp, 0xFA56, 0x9CF6 );
        rtl8101_mac_ocp_write( tp, 0xFA58, 0x7444 );
        rtl8101_mac_ocp_write( tp, 0xFA5A, 0x49C1 );
        rtl8101_mac_ocp_write( tp, 0xFA5C, 0xF004 );
        rtl8101_mac_ocp_write( tp, 0xFA5E, 0x6446 );
        rtl8101_mac_ocp_write( tp, 0xFA60, 0x1E07 );
        rtl8101_mac_ocp_write( tp, 0xFA62, 0xE003 );
        rtl8101_mac_ocp_write( tp, 0xFA64, 0x1C01 );
        rtl8101_mac_ocp_write( tp, 0xFA66, 0x1E03 );
        rtl8101_mac_ocp_write( tp, 0xFA68, 0x9C62 );
        rtl8101_mac_ocp_write( tp, 0xFA6A, 0x1C11 );
        rtl8101_mac_ocp_write( tp, 0xFA6C, 0x8C60 );
        rtl8101_mac_ocp_write( tp, 0xFA6E, 0xE830 );
        rtl8101_mac_ocp_write( tp, 0xFA70, 0xE02D );
        rtl8101_mac_ocp_write( tp, 0xFA72, 0x49E9 );
        rtl8101_mac_ocp_write( tp, 0xFA74, 0xF004 );
        rtl8101_mac_ocp_write( tp, 0xFA76, 0x1D02 );
        rtl8101_mac_ocp_write( tp, 0xFA78, 0x8DF5 );
        rtl8101_mac_ocp_write( tp, 0xFA7A, 0xE79C );
        rtl8101_mac_ocp_write( tp, 0xFA7C, 0x49E3 );
        rtl8101_mac_ocp_write( tp, 0xFA7E, 0xF006 );
        rtl8101_mac_ocp_write( tp, 0xFA80, 0x1D08 );
        rtl8101_mac_ocp_write( tp, 0xFA82, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xFA84, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xFA86, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xFA88, 0xE73A );
        rtl8101_mac_ocp_write( tp, 0xFA8A, 0x49E1 );
        rtl8101_mac_ocp_write( tp, 0xFA8C, 0xF007 );
        rtl8101_mac_ocp_write( tp, 0xFA8E, 0x1D02 );
        rtl8101_mac_ocp_write( tp, 0xFA90, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xFA92, 0x1E01 );
        rtl8101_mac_ocp_write( tp, 0xFA94, 0xE7A7 );
        rtl8101_mac_ocp_write( tp, 0xFA96, 0xDE20 );
        rtl8101_mac_ocp_write( tp, 0xFA98, 0xE410 );
        rtl8101_mac_ocp_write( tp, 0xFA9A, 0x49E0 );
        rtl8101_mac_ocp_write( tp, 0xFA9C, 0xF017 );
        rtl8101_mac_ocp_write( tp, 0xFA9E, 0x1D01 );
        rtl8101_mac_ocp_write( tp, 0xFAA0, 0x8DF4 );
        rtl8101_mac_ocp_write( tp, 0xFAA2, 0xC5FA );
        rtl8101_mac_ocp_write( tp, 0xFAA4, 0x1C00 );
        rtl8101_mac_ocp_write( tp, 0xFAA6, 0x8CA0 );
        rtl8101_mac_ocp_write( tp, 0xFAA8, 0x1C1B );
        rtl8101_mac_ocp_write( tp, 0xFAAA, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xFAAC, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFAAE, 0x49CF );
        rtl8101_mac_ocp_write( tp, 0xFAB0, 0xF0FE );
        rtl8101_mac_ocp_write( tp, 0xFAB2, 0xC5F3 );
        rtl8101_mac_ocp_write( tp, 0xFAB4, 0x74A0 );
        rtl8101_mac_ocp_write( tp, 0xFAB6, 0x4849 );
        rtl8101_mac_ocp_write( tp, 0xFAB8, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xFABA, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xFABC, 0x49C0 );
        rtl8101_mac_ocp_write( tp, 0xFABE, 0xF006 );
        rtl8101_mac_ocp_write( tp, 0xFAC0, 0x48C3 );
        rtl8101_mac_ocp_write( tp, 0xFAC2, 0x8CF8 );
        rtl8101_mac_ocp_write( tp, 0xFAC4, 0xE820 );
        rtl8101_mac_ocp_write( tp, 0xFAC6, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xFAC8, 0x74F8 );
        rtl8101_mac_ocp_write( tp, 0xFACA, 0xC432 );
        rtl8101_mac_ocp_write( tp, 0xFACC, 0xBC00 );
        rtl8101_mac_ocp_write( tp, 0xFACE, 0xC5E4 );
        rtl8101_mac_ocp_write( tp, 0xFAD0, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFAD2, 0x49CE );
        rtl8101_mac_ocp_write( tp, 0xFAD4, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xFAD6, 0x9EA0 );
        rtl8101_mac_ocp_write( tp, 0xFAD8, 0x1C1C );
        rtl8101_mac_ocp_write( tp, 0xFADA, 0x484F );
        rtl8101_mac_ocp_write( tp, 0xFADC, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xFADE, 0xFF80 );
        rtl8101_mac_ocp_write( tp, 0xFAE0, 0xB404 );
        rtl8101_mac_ocp_write( tp, 0xFAE2, 0xB405 );
        rtl8101_mac_ocp_write( tp, 0xFAE4, 0xC5D9 );
        rtl8101_mac_ocp_write( tp, 0xFAE6, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFAE8, 0x49CE );
        rtl8101_mac_ocp_write( tp, 0xFAEA, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xFAEC, 0xC41F );
        rtl8101_mac_ocp_write( tp, 0xFAEE, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xFAF0, 0xC41C );
        rtl8101_mac_ocp_write( tp, 0xFAF2, 0x1C13 );
        rtl8101_mac_ocp_write( tp, 0xFAF4, 0x484F );
        rtl8101_mac_ocp_write( tp, 0xFAF6, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xFAF8, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFAFA, 0x49CF );
        rtl8101_mac_ocp_write( tp, 0xFAFC, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xFAFE, 0xB005 );
        rtl8101_mac_ocp_write( tp, 0xFB00, 0xB004 );
        rtl8101_mac_ocp_write( tp, 0xFB02, 0xFF80 );
        rtl8101_mac_ocp_write( tp, 0xFB04, 0xB404 );
        rtl8101_mac_ocp_write( tp, 0xFB06, 0xB405 );
        rtl8101_mac_ocp_write( tp, 0xFB08, 0xC5C7 );
        rtl8101_mac_ocp_write( tp, 0xFB0A, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFB0C, 0x49CE );
        rtl8101_mac_ocp_write( tp, 0xFB0E, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xFB10, 0xC40E );
        rtl8101_mac_ocp_write( tp, 0xFB12, 0x9CA0 );
        rtl8101_mac_ocp_write( tp, 0xFB14, 0xC40A );
        rtl8101_mac_ocp_write( tp, 0xFB16, 0x1C13 );
        rtl8101_mac_ocp_write( tp, 0xFB18, 0x484F );
        rtl8101_mac_ocp_write( tp, 0xFB1A, 0x9CA2 );
        rtl8101_mac_ocp_write( tp, 0xFB1C, 0x74A2 );
        rtl8101_mac_ocp_write( tp, 0xFB1E, 0x49CF );
        rtl8101_mac_ocp_write( tp, 0xFB20, 0xF1FE );
        rtl8101_mac_ocp_write( tp, 0xFB22, 0xB005 );
        rtl8101_mac_ocp_write( tp, 0xFB24, 0xB004 );
        rtl8101_mac_ocp_write( tp, 0xFB26, 0xFF80 );
        rtl8101_mac_ocp_write( tp, 0xFB28, 0x0000 );
        rtl8101_mac_ocp_write( tp, 0xFB2A, 0x0481 );
        rtl8101_mac_ocp_write( tp, 0xFB2C, 0x0C81 );
        rtl8101_mac_ocp_write( tp, 0xFB2E, 0x0AE0 );


        rtl8101_mac_ocp_write( tp, 0xFC26, 0x8000 );

        rtl8101_mac_ocp_write( tp, 0xFC28, 0x0000 );
        rtl8101_mac_ocp_write( tp, 0xFC2A, 0x0000 );
        rtl8101_mac_ocp_write( tp, 0xFC2C, 0x0297 );
        rtl8101_mac_ocp_write( tp, 0xFC2E, 0x0000 );
        rtl8101_mac_ocp_write( tp, 0xFC30, 0x00A9 );
        rtl8101_mac_ocp_write( tp, 0xFC32, 0x012D );
        rtl8101_mac_ocp_write( tp, 0xFC34, 0x0000 );
        rtl8101_mac_ocp_write( tp, 0xFC36, 0x08DF );
}

static void
rtl8101_set_mac_mcu_8107e_1(struct net_device *dev)
{
        rtl8101_hw_disable_mac_mcu_bps(dev);
}

static void
rtl8101_set_mac_mcu_8107e_2(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 i;
        static const u16 mcu_patch_code_8107e_2[] = {
                0xE008, 0xE00F, 0xE011, 0xE047, 0xE049, 0xE073, 0xE075, 0xE07A, 0xC707,
                0x1D00, 0x8DE2, 0x48C1, 0xC502, 0xBD00, 0x00E4, 0xE0C0, 0xC502, 0xBD00,
                0x0216, 0xC634, 0x75C0, 0x49D3, 0xF027, 0xC631, 0x75C0, 0x49D3, 0xF123,
                0xC627, 0x75C0, 0xB405, 0xC525, 0x9DC0, 0xC621, 0x75C8, 0x49D5, 0xF00A,
                0x49D6, 0xF008, 0x49D7, 0xF006, 0x49D8, 0xF004, 0x75D2, 0x49D9, 0xF111,
                0xC517, 0x9DC8, 0xC516, 0x9DD2, 0xC618, 0x75C0, 0x49D4, 0xF003, 0x49D0,
                0xF104, 0xC60A, 0xC50E, 0x9DC0, 0xB005, 0xC607, 0x9DC0, 0xB007, 0xC602,
                0xBE00, 0x1A06, 0xB400, 0xE86C, 0xA000, 0x01E1, 0x0200, 0x9200, 0xE84C,
                0xE004, 0xE908, 0xC502, 0xBD00, 0x0B58, 0xB407, 0xB404, 0x2195, 0x25BD,
                0x9BE0, 0x1C1C, 0x484F, 0x9CE2, 0x72E2, 0x49AE, 0xF1FE, 0x0B00, 0xF116,
                0xC71C, 0xC419, 0x9CE0, 0x1C13, 0x484F, 0x9CE2, 0x74E2, 0x49CE, 0xF1FE,
                0xC412, 0x9CE0, 0x1C13, 0x484F, 0x9CE2, 0x74E2, 0x49CE, 0xF1FE, 0xC70C,
                0x74F8, 0x48C3, 0x8CF8, 0xB004, 0xB007, 0xC502, 0xBD00, 0x0F24, 0x0481,
                0x0C81, 0xDE24, 0xE000, 0xC602, 0xBE00, 0x0CA4, 0x48C1, 0x48C2, 0x9C46,
                0xC402, 0xBC00, 0x0578, 0xC602, 0xBE00, 0x0000
        };

        rtl8101_hw_disable_mac_mcu_bps(dev);

        for (i = 0; i < ARRAY_SIZE(mcu_patch_code_8107e_2); i++) {
                rtl8101_mac_ocp_write(tp, 0xF800 + i * 2, mcu_patch_code_8107e_2[i]);
        }

        rtl8101_mac_ocp_write(tp, 0xFC26, 0x8000);

        rtl8101_mac_ocp_write(tp, 0xFC28, 0x00E2);
        rtl8101_mac_ocp_write(tp, 0xFC2A, 0x0210);
        rtl8101_mac_ocp_write(tp, 0xFC2C, 0x1A04);
        rtl8101_mac_ocp_write(tp, 0xFC2E, 0x0B26);
        rtl8101_mac_ocp_write(tp, 0xFC30, 0x0F02);
        rtl8101_mac_ocp_write(tp, 0xFC32, 0x0CA0);
        //rtl8101_mac_ocp_write(tp, 0xFC34, 0x056C);

        rtl8101_mac_ocp_write(tp, 0xFC38, 0x0033);
}

static void
rtl8101_set_mac_mcu_8107e_3(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 i;
        static const u16 mcu_patch_code_8107e_3[] = {
                0xE008, 0xE00A, 0xE00C, 0xE00E, 0xE010, 0xE039, 0xE03B, 0xE064, 0xC602,
                0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602, 0xBE00, 0x0000, 0xC602,
                0xBE00, 0x0000, 0xC727, 0x76E2, 0x49EE, 0xF1FD, 0x1E00, 0x8EE0, 0x1E1C,
                0x8EE2, 0x76E2, 0x49EE, 0xF1FE, 0xC61D, 0x8EE0, 0x1E1D, 0x486F, 0x8EE2,
                0x76E2, 0x49EE, 0xF12C, 0xC716, 0x76E0, 0x48E8, 0x48E9, 0x48EA, 0x48EB,
                0x48EC, 0x9EE0, 0xC709, 0xC609, 0x9EF4, 0xC608, 0x9EF6, 0xB007, 0xC602,
                0xBE00, 0x0ACC, 0xE000, 0x03BF, 0x07FF, 0xDE24, 0x3200, 0xE096, 0xC602,
                0xBE00, 0x0000, 0x8EE6, 0xC726, 0x76E2, 0x49EE, 0xF1FD, 0x1E00, 0x8EE0,
                0x1E1C, 0x8EE2, 0x76E2, 0x49EE, 0xF1FE, 0xC61C, 0x8EE0, 0x1E1D, 0x486F,
                0x8EE2, 0x76E2, 0x49EE, 0xF1FE, 0xC715, 0x76E0, 0x48E8, 0x48E9, 0x48EA,
                0x48EB, 0x48EC, 0x9EE0, 0xC708, 0xC608, 0x9EF4, 0xC607, 0x9EF6, 0xC602,
                0xBE00, 0x0ABE, 0xE000, 0x03BF, 0x07FF, 0xDE24, 0x3200, 0xE096, 0xC602,
                0xBE00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x6838, 0x0A16, 0x0901, 0x101C
        };

        rtl8101_hw_disable_mac_mcu_bps(dev);

        for (i = 0; i < ARRAY_SIZE(mcu_patch_code_8107e_3); i++) {
                rtl8101_mac_ocp_write(tp, 0xF800 + i * 2, mcu_patch_code_8107e_3[i]);
        }

        rtl8101_mac_ocp_write(tp, 0xFC26, 0x8000);

        rtl8101_mac_ocp_write(tp, 0xFC30, 0x0ACA);

        rtl8101_clear_mcu_ocp_bit(tp, 0xD438, BIT_3);

        rtl8101_mac_ocp_write(tp, 0xFC38, 0x0010);
}

static void
rtl8101_hw_mac_mcu_config(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->NotWrMcuPatchCode == TRUE) return;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                rtl8101_set_mac_mcu_8106eus_1(dev);
                break;
        case CFG_METHOD_18:
                rtl8101_set_mac_mcu_8107e_1(dev);
                break;
        case CFG_METHOD_19:
                rtl8101_set_mac_mcu_8107e_2(dev);
                break;
        case CFG_METHOD_20:
                rtl8101_set_mac_mcu_8107e_3(dev);
                break;
        }
}

static void
rtl8101_hw_init(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 csi_tmp;

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_enable_cfg9346_write(tp);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                rtl8101_disable_cfg9346_write(tp);
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                break;
        }

        //Disable UPS
        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mac_ocp_write(tp, 0xD400, rtl8101_mac_ocp_read( tp, 0xD400) & ~(BIT_0));
                break;
        }

        //Disable DMA Aggregation
        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mac_ocp_write(tp, 0xE63E, rtl8101_mac_ocp_read( tp, 0xE63E) & ~(BIT_3 | BIT_2 | BIT_1));
                rtl8101_mac_ocp_write(tp, 0xE63E, rtl8101_mac_ocp_read( tp, 0xE63E) | (BIT_0));
                rtl8101_mac_ocp_write(tp, 0xE63E, rtl8101_mac_ocp_read( tp, 0xE63E) & ~(BIT_0));
                rtl8101_mac_ocp_write(tp, 0xC094, 0x0);
                rtl8101_mac_ocp_write(tp, 0xC09E, 0x0);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                rtl8101_eri_write(tp, 0x174, 2, 0x00FF, ERIAR_ExGMAC);
                rtl8101_mac_ocp_write(tp, 0xE428, 0x0010);
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                csi_tmp = rtl8101_eri_read(tp, 0x174, 2, ERIAR_ExGMAC);
                csi_tmp &= ~(BIT_8);
                csi_tmp |= (BIT_15);
                rtl8101_eri_write(tp, 0x174, 2, csi_tmp, ERIAR_ExGMAC);
                rtl8101_mac_ocp_write(tp, 0xE428, 0x0010);
                break;
        }

        if (tp->mcfg == CFG_METHOD_10)
                RTL_W8(tp, 0xF3, RTL_R8(tp, 0xF3) | BIT_2);

        rtl8101_hw_mac_mcu_config(dev);

        /*disable ocp phy power saving*/
        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20)
                rtl8101_disable_ocp_phy_power_saving(dev);

        //Set PCIE uncorrectable error status mask pcie 0x108
        csi_tmp = rtl8101_csi_read(tp, 0x108);
        csi_tmp |= BIT_20;
        rtl8101_csi_write(tp, 0x108, csi_tmp);

        if (s0_magic_packet == 1)
                rtl8101_enable_magic_packet(dev);
}

static void
rtl8101_hw_ephy_config(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16 ephy_data;

        if (tp->mcfg == CFG_METHOD_4) {
                rtl8101_ephy_write(tp, 0x03, 0xc2f9);
        } else if (tp->mcfg == CFG_METHOD_5) {
                rtl8101_ephy_write(tp, 0x01, 0x6FE5);
                rtl8101_ephy_write(tp, 0x03, 0xD7D9);
        } else if (tp->mcfg == CFG_METHOD_6) {
                rtl8101_ephy_write(tp, 0x06, 0xAF35);
        } else if (tp->mcfg == CFG_METHOD_7) {
                rtl8101_ephy_write(tp, 0x19, 0xEC90);
                rtl8101_ephy_write(tp, 0x01, 0x6FE5);
                rtl8101_ephy_write(tp, 0x03, 0x05D9);
                rtl8101_ephy_write(tp, 0x06, 0xAF35);
        } else if (tp->mcfg == CFG_METHOD_8) {
                rtl8101_ephy_write(tp, 0x01, 0x6FE5);
                rtl8101_ephy_write(tp, 0x03, 0x05D9);
                rtl8101_ephy_write(tp, 0x06, 0xAF35);
                rtl8101_ephy_write(tp, 0x19, 0xECFA);
        } else if (tp->mcfg == CFG_METHOD_9) {
                rtl8101_ephy_write(tp, 0x01, 0x6FE5);
                rtl8101_ephy_write(tp, 0x03, 0x0599);
                rtl8101_ephy_write(tp, 0x06, 0xAF25);
                rtl8101_ephy_write(tp, 0x07, 0x8E68);
        } else if (tp->mcfg == CFG_METHOD_10) {
                ephy_data = rtl8101_ephy_read(tp, 0x00) & ~0x0200;
                ephy_data |= 0x0100;
                rtl8101_ephy_write(tp, 0x00, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x00);
                ephy_data |= 0x0004;
                rtl8101_ephy_write(tp, 0x00, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x06) & ~0x0002;
                ephy_data |= 0x0001;
                rtl8101_ephy_write(tp, 0x06, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x06);
                ephy_data |= 0x0030;
                rtl8101_ephy_write(tp, 0x06, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x07);
                ephy_data |= 0x2000;
                rtl8101_ephy_write(tp, 0x07, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x00);
                ephy_data |= 0x0020;
                rtl8101_ephy_write(tp, 0x00, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x03) & ~0x5800;
                ephy_data |= 0x2000;
                rtl8101_ephy_write(tp, 0x03, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x03);
                ephy_data |= 0x0001;
                rtl8101_ephy_write(tp, 0x03, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x01) & ~0x0800;
                ephy_data |= 0x1000;
                rtl8101_ephy_write(tp, 0x01, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x07);
                ephy_data |= 0x4000;
                rtl8101_ephy_write(tp, 0x07, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x1E);
                ephy_data |= 0x2000;
                rtl8101_ephy_write(tp, 0x1E, ephy_data);

                rtl8101_ephy_write(tp, 0x19, 0xFE6C);

                ephy_data = rtl8101_ephy_read(tp, 0x0A);
                ephy_data |= 0x0040;
                rtl8101_ephy_write(tp, 0x0A, ephy_data);

        } else if (tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
                   tp->mcfg == CFG_METHOD_13) {
                ephy_data = rtl8101_ephy_read(tp, 0x07);
                ephy_data |= 0x4000;
                rtl8101_ephy_write(tp, 0x07, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x19);
                ephy_data |= 0x0200;
                rtl8101_ephy_write(tp, 0x19, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x19);
                ephy_data |= 0x0020;
                rtl8101_ephy_write(tp, 0x19, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x1E);
                ephy_data |= 0x2000;
                rtl8101_ephy_write(tp, 0x1E, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x03);
                ephy_data |= 0x0001;
                rtl8101_ephy_write(tp, 0x03, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x19);
                ephy_data |= 0x0100;
                rtl8101_ephy_write(tp, 0x19, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x19);
                ephy_data |= 0x0004;
                rtl8101_ephy_write(tp, 0x19, ephy_data);

                ephy_data = rtl8101_ephy_read(tp, 0x0A);
                ephy_data |= 0x0020;
                rtl8101_ephy_write(tp, 0x0A, ephy_data);

                if (tp->mcfg == CFG_METHOD_11) {
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                } else if (tp->mcfg == CFG_METHOD_12 ||
                           tp->mcfg == CFG_METHOD_13) {
                        ephy_data = rtl8101_ephy_read(tp, 0x1E);
                        ephy_data |= 0x8000;
                        rtl8101_ephy_write(tp, 0x1E, ephy_data);
                }
        } else if (tp->mcfg == CFG_METHOD_14) {
                rtl8101_ephy_write(tp, 0x19, 0xff64);
        } else if (tp->mcfg == CFG_METHOD_17) {
                ephy_data = rtl8101_ephy_read(tp, 0x00);
                ephy_data &= ~BIT_3;
                rtl8101_ephy_write(tp, 0x00, ephy_data);
                ephy_data = rtl8101_ephy_read(tp, 0x0C);
                ephy_data &= ~(BIT_13 | BIT_12 | BIT_11 | BIT_10| BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4);
                ephy_data |= (BIT_5 | BIT_11);
                rtl8101_ephy_write(tp, 0x0C, ephy_data);

                rtl8101_ephy_write(tp, 0x19, 0x7C00);
                rtl8101_ephy_write(tp, 0x1E, 0x20EB);
                rtl8101_ephy_write(tp, 0x0D, 0x1666);
                rtl8101_ephy_write(tp, 0x00, 0x10A3);
                rtl8101_ephy_write(tp, 0x06, 0xF050);

                SetPCIePhyBit(tp, 0x04, BIT_4);
                ClearPCIePhyBit(tp, 0x1D, BIT_14);
        } else if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19) {
                ClearPCIePhyBit(tp, 0x1E, BIT_11);

                SetPCIePhyBit(tp, 0x1E, BIT_0);
                SetPCIePhyBit(tp, 0x1D, BIT_11);

                rtl8101_ephy_write(tp, 0x05, 0x2089);
                rtl8101_ephy_write(tp, 0x06, 0x5881);

                rtl8101_ephy_write(tp, 0x04, 0x854A);
                rtl8101_ephy_write(tp, 0x01, 0x068B);
        } else if (tp->mcfg == CFG_METHOD_20) {
                rtl8101_clear_mcu_ocp_bit(tp, 0xDE38, BIT_2);

                ClearPCIePhyBit(tp, 0x24, BIT_9);

                rtl8101_clear_mcu_ocp_bit(tp, 0xDE28, (BIT_1 | BIT_0));

                rtl8101_set_mcu_ocp_bit(tp, 0xDE38, BIT_2);
        }
}

static int
rtl8101_check_hw_phy_mcu_code_ver(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int ram_code_ver_match = 0;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x801E);
                tp->hw_ram_code_ver = rtl8101_mdio_read(tp, 0x14);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        default:
                tp->hw_ram_code_ver = ~0;
                break;
        }

        if(tp->hw_ram_code_ver == tp->sw_ram_code_ver)
                ram_code_ver_match = 1;
        tp->HwHasWrRamCodeToMicroP = TRUE;

        return ram_code_ver_match;
}

static int
rtl8101_set_phy_mcu_patch_request(struct rtl8101_private *tp)
{
        u16 PhyRegValue;
        u32 WaitCnt;
        int retval = TRUE;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp,0x1f, 0x0B82);
                rtl8101_set_eth_phy_bit(tp, 0x10, BIT_4);

                rtl8101_mdio_write(tp,0x1f, 0x0B80);
                WaitCnt = 0;
                do {
                        PhyRegValue = rtl8101_mdio_read(tp, 0x10);
                        udelay(100);
                        WaitCnt++;
                }  while (!(PhyRegValue & BIT_6) && (WaitCnt < 1000));

                if (!(PhyRegValue & BIT_6) && (WaitCnt == 1000)) retval = FALSE;

                rtl8101_mdio_write(tp,0x1f, 0x0000);
                break;
        }

        return retval;
}

static int
rtl8101_clear_phy_mcu_patch_request(struct rtl8101_private *tp)
{
        u16 PhyRegValue;
        u32 WaitCnt;
        int retval = TRUE;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, 0x1f, 0x0B82);
                rtl8101_clear_eth_phy_bit(tp, 0x10, BIT_4);

                rtl8101_mdio_write(tp,0x1f, 0x0B80);
                WaitCnt = 0;
                do {
                        PhyRegValue = rtl8101_mdio_read(tp, 0x10);
                        udelay(100);
                        WaitCnt++;
                } while ((PhyRegValue & BIT_6) && (WaitCnt < 1000));

                if ((PhyRegValue & BIT_6) && (WaitCnt == 1000)) retval = FALSE;

                rtl8101_mdio_write(tp,0x1f, 0x0000);
                break;
        }

        return retval;
}

static void
rtl8101_write_hw_phy_mcu_code_ver(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x801E);
                rtl8101_mdio_write(tp, 0x14, tp->sw_ram_code_ver);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                break;
        }
}

static void
rtl8101_set_phy_mcu_8105e_1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int gphy_val,i;

        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x00, 0x1800);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0023);
        rtl8101_mdio_write(tp, 0x17, 0x0117);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1E, 0x002C);
        rtl8101_mdio_write(tp, 0x1B, 0x5000);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x16, 0x4104);
        for (i = 0; i < 200; i++) {
                udelay(100);
                gphy_val = rtl8101_mdio_read(tp, 0x1E);
                gphy_val &= 0x03FF;
                if (gphy_val==0x000C)
                        break;
        }
        rtl8101_mdio_write(tp, 0x1f, 0x0005);
        for (i = 0; i < 200; i++) {
                udelay(100);
                gphy_val = rtl8101_mdio_read(tp, 0x07);
                if ((gphy_val & BIT_5)==0)
                        break;
        }
        gphy_val = rtl8101_mdio_read(tp, 0x07);
        if (gphy_val & BIT_5) {
                rtl8101_mdio_write(tp, 0x1f, 0x0007);
                rtl8101_mdio_write(tp, 0x1e, 0x00a1);
                rtl8101_mdio_write(tp, 0x17, 0x1000);
                rtl8101_mdio_write(tp, 0x17, 0x0000);
                rtl8101_mdio_write(tp, 0x17, 0x2000);
                rtl8101_mdio_write(tp, 0x1e, 0x002f);
                rtl8101_mdio_write(tp, 0x18, 0x9bfb);
                rtl8101_mdio_write(tp, 0x1f, 0x0005);
                rtl8101_mdio_write(tp, 0x07, 0x0000);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
        }
        rtl8101_mdio_write(tp, 0x1f, 0x0005);
        rtl8101_mdio_write(tp, 0x05, 0xfff6);
        rtl8101_mdio_write(tp, 0x06, 0x0080);
        gphy_val = rtl8101_mdio_read(tp, 0x00);
        gphy_val &= ~(BIT_7);
        rtl8101_mdio_write(tp, 0x00, gphy_val);
        rtl8101_mdio_write(tp, 0x1f, 0x0002);
        gphy_val = rtl8101_mdio_read(tp, 0x08);
        gphy_val &= ~(BIT_7);
        rtl8101_mdio_write(tp, 0x08, gphy_val);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0023);
        rtl8101_mdio_write(tp, 0x16, 0x0306);
        rtl8101_mdio_write(tp, 0x16, 0x0307);
        rtl8101_mdio_write(tp, 0x15, 0x000e);
        rtl8101_mdio_write(tp, 0x19, 0x000a);
        rtl8101_mdio_write(tp, 0x15, 0x0010);
        rtl8101_mdio_write(tp, 0x19, 0x0008);
        rtl8101_mdio_write(tp, 0x15, 0x0018);
        rtl8101_mdio_write(tp, 0x19, 0x4801);
        rtl8101_mdio_write(tp, 0x15, 0x0019);
        rtl8101_mdio_write(tp, 0x19, 0x6801);
        rtl8101_mdio_write(tp, 0x15, 0x001a);
        rtl8101_mdio_write(tp, 0x19, 0x66a1);
        rtl8101_mdio_write(tp, 0x15, 0x001f);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0020);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0021);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0022);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0023);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0024);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0025);
        rtl8101_mdio_write(tp, 0x19, 0x64a1);
        rtl8101_mdio_write(tp, 0x15, 0x0026);
        rtl8101_mdio_write(tp, 0x19, 0x40ea);
        rtl8101_mdio_write(tp, 0x15, 0x0027);
        rtl8101_mdio_write(tp, 0x19, 0x4503);
        rtl8101_mdio_write(tp, 0x15, 0x0028);
        rtl8101_mdio_write(tp, 0x19, 0x9f00);
        rtl8101_mdio_write(tp, 0x15, 0x0029);
        rtl8101_mdio_write(tp, 0x19, 0xa631);
        rtl8101_mdio_write(tp, 0x15, 0x002a);
        rtl8101_mdio_write(tp, 0x19, 0x9717);
        rtl8101_mdio_write(tp, 0x15, 0x002b);
        rtl8101_mdio_write(tp, 0x19, 0x302c);
        rtl8101_mdio_write(tp, 0x15, 0x002c);
        rtl8101_mdio_write(tp, 0x19, 0x4802);
        rtl8101_mdio_write(tp, 0x15, 0x002d);
        rtl8101_mdio_write(tp, 0x19, 0x58da);
        rtl8101_mdio_write(tp, 0x15, 0x002e);
        rtl8101_mdio_write(tp, 0x19, 0x400d);
        rtl8101_mdio_write(tp, 0x15, 0x002f);
        rtl8101_mdio_write(tp, 0x19, 0x4488);
        rtl8101_mdio_write(tp, 0x15, 0x0030);
        rtl8101_mdio_write(tp, 0x19, 0x9e00);
        rtl8101_mdio_write(tp, 0x15, 0x0031);
        rtl8101_mdio_write(tp, 0x19, 0x63c8);
        rtl8101_mdio_write(tp, 0x15, 0x0032);
        rtl8101_mdio_write(tp, 0x19, 0x6481);
        rtl8101_mdio_write(tp, 0x15, 0x0033);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0034);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0035);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0036);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0037);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0038);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0039);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x003a);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x003b);
        rtl8101_mdio_write(tp, 0x19, 0x63e8);
        rtl8101_mdio_write(tp, 0x15, 0x003c);
        rtl8101_mdio_write(tp, 0x19, 0x7d00);
        rtl8101_mdio_write(tp, 0x15, 0x003d);
        rtl8101_mdio_write(tp, 0x19, 0x59d4);
        rtl8101_mdio_write(tp, 0x15, 0x003e);
        rtl8101_mdio_write(tp, 0x19, 0x63f8);
        rtl8101_mdio_write(tp, 0x15, 0x0040);
        rtl8101_mdio_write(tp, 0x19, 0x64a1);
        rtl8101_mdio_write(tp, 0x15, 0x0041);
        rtl8101_mdio_write(tp, 0x19, 0x30de);
        rtl8101_mdio_write(tp, 0x15, 0x0044);
        rtl8101_mdio_write(tp, 0x19, 0x480f);
        rtl8101_mdio_write(tp, 0x15, 0x0045);
        rtl8101_mdio_write(tp, 0x19, 0x6800);
        rtl8101_mdio_write(tp, 0x15, 0x0046);
        rtl8101_mdio_write(tp, 0x19, 0x6680);
        rtl8101_mdio_write(tp, 0x15, 0x0047);
        rtl8101_mdio_write(tp, 0x19, 0x7c10);
        rtl8101_mdio_write(tp, 0x15, 0x0048);
        rtl8101_mdio_write(tp, 0x19, 0x63c8);
        rtl8101_mdio_write(tp, 0x15, 0x0049);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004a);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004b);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004c);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004d);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004e);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x004f);
        rtl8101_mdio_write(tp, 0x19, 0x40ea);
        rtl8101_mdio_write(tp, 0x15, 0x0050);
        rtl8101_mdio_write(tp, 0x19, 0x4503);
        rtl8101_mdio_write(tp, 0x15, 0x0051);
        rtl8101_mdio_write(tp, 0x19, 0x58ca);
        rtl8101_mdio_write(tp, 0x15, 0x0052);
        rtl8101_mdio_write(tp, 0x19, 0x63c8);
        rtl8101_mdio_write(tp, 0x15, 0x0053);
        rtl8101_mdio_write(tp, 0x19, 0x63d8);
        rtl8101_mdio_write(tp, 0x15, 0x0054);
        rtl8101_mdio_write(tp, 0x19, 0x66a0);
        rtl8101_mdio_write(tp, 0x15, 0x0055);
        rtl8101_mdio_write(tp, 0x19, 0x9f00);
        rtl8101_mdio_write(tp, 0x15, 0x0056);
        rtl8101_mdio_write(tp, 0x19, 0x3000);
        rtl8101_mdio_write(tp, 0x15, 0x006E);
        rtl8101_mdio_write(tp, 0x19, 0x9afa);
        rtl8101_mdio_write(tp, 0x15, 0x00a1);
        rtl8101_mdio_write(tp, 0x19, 0x3044);
        rtl8101_mdio_write(tp, 0x15, 0x00ab);
        rtl8101_mdio_write(tp, 0x19, 0x5820);
        rtl8101_mdio_write(tp, 0x15, 0x00ac);
        rtl8101_mdio_write(tp, 0x19, 0x5e04);
        rtl8101_mdio_write(tp, 0x15, 0x00ad);
        rtl8101_mdio_write(tp, 0x19, 0xb60c);
        rtl8101_mdio_write(tp, 0x15, 0x00af);
        rtl8101_mdio_write(tp, 0x19, 0x000a);
        rtl8101_mdio_write(tp, 0x15, 0x00b2);
        rtl8101_mdio_write(tp, 0x19, 0x30b9);
        rtl8101_mdio_write(tp, 0x15, 0x00b9);
        rtl8101_mdio_write(tp, 0x19, 0x4408);
        rtl8101_mdio_write(tp, 0x15, 0x00ba);
        rtl8101_mdio_write(tp, 0x19, 0x480b);
        rtl8101_mdio_write(tp, 0x15, 0x00bb);
        rtl8101_mdio_write(tp, 0x19, 0x5e00);
        rtl8101_mdio_write(tp, 0x15, 0x00bc);
        rtl8101_mdio_write(tp, 0x19, 0x405f);
        rtl8101_mdio_write(tp, 0x15, 0x00bd);
        rtl8101_mdio_write(tp, 0x19, 0x4448);
        rtl8101_mdio_write(tp, 0x15, 0x00be);
        rtl8101_mdio_write(tp, 0x19, 0x4020);
        rtl8101_mdio_write(tp, 0x15, 0x00bf);
        rtl8101_mdio_write(tp, 0x19, 0x4468);
        rtl8101_mdio_write(tp, 0x15, 0x00c0);
        rtl8101_mdio_write(tp, 0x19, 0x9c02);
        rtl8101_mdio_write(tp, 0x15, 0x00c1);
        rtl8101_mdio_write(tp, 0x19, 0x58a0);
        rtl8101_mdio_write(tp, 0x15, 0x00c2);
        rtl8101_mdio_write(tp, 0x19, 0xb605);
        rtl8101_mdio_write(tp, 0x15, 0x00c3);
        rtl8101_mdio_write(tp, 0x19, 0xc0d3);
        rtl8101_mdio_write(tp, 0x15, 0x00c4);
        rtl8101_mdio_write(tp, 0x19, 0x00e6);
        rtl8101_mdio_write(tp, 0x15, 0x00c5);
        rtl8101_mdio_write(tp, 0x19, 0xdaec);
        rtl8101_mdio_write(tp, 0x15, 0x00c6);
        rtl8101_mdio_write(tp, 0x19, 0x00fa);
        rtl8101_mdio_write(tp, 0x15, 0x00c7);
        rtl8101_mdio_write(tp, 0x19, 0x9df9);
        rtl8101_mdio_write(tp, 0x15, 0x00c8);
        rtl8101_mdio_write(tp, 0x19, 0x307a);
        rtl8101_mdio_write(tp, 0x15, 0x0112);
        rtl8101_mdio_write(tp, 0x19, 0x6421);
        rtl8101_mdio_write(tp, 0x15, 0x0113);
        rtl8101_mdio_write(tp, 0x19, 0x7c08);
        rtl8101_mdio_write(tp, 0x15, 0x0114);
        rtl8101_mdio_write(tp, 0x19, 0x63f0);
        rtl8101_mdio_write(tp, 0x15, 0x0115);
        rtl8101_mdio_write(tp, 0x19, 0x4003);
        rtl8101_mdio_write(tp, 0x15, 0x0116);
        rtl8101_mdio_write(tp, 0x19, 0x4418);
        rtl8101_mdio_write(tp, 0x15, 0x0117);
        rtl8101_mdio_write(tp, 0x19, 0x9b00);
        rtl8101_mdio_write(tp, 0x15, 0x0118);
        rtl8101_mdio_write(tp, 0x19, 0x6461);
        rtl8101_mdio_write(tp, 0x15, 0x0119);
        rtl8101_mdio_write(tp, 0x19, 0x64e1);
        rtl8101_mdio_write(tp, 0x15, 0x011a);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0150);
        rtl8101_mdio_write(tp, 0x19, 0x6461);
        rtl8101_mdio_write(tp, 0x15, 0x0151);
        rtl8101_mdio_write(tp, 0x19, 0x4003);
        rtl8101_mdio_write(tp, 0x15, 0x0152);
        rtl8101_mdio_write(tp, 0x19, 0x4540);
        rtl8101_mdio_write(tp, 0x15, 0x0153);
        rtl8101_mdio_write(tp, 0x19, 0x9f00);
        rtl8101_mdio_write(tp, 0x15, 0x0155);
        rtl8101_mdio_write(tp, 0x19, 0x6421);
        rtl8101_mdio_write(tp, 0x15, 0x0156);
        rtl8101_mdio_write(tp, 0x19, 0x64a1);
        rtl8101_mdio_write(tp, 0x15, 0x021e);
        rtl8101_mdio_write(tp, 0x19, 0x5410);
        rtl8101_mdio_write(tp, 0x15, 0x0225);
        rtl8101_mdio_write(tp, 0x19, 0x5400);
        rtl8101_mdio_write(tp, 0x15, 0x023D);
        rtl8101_mdio_write(tp, 0x19, 0x4050);
        rtl8101_mdio_write(tp, 0x15, 0x0295);
        rtl8101_mdio_write(tp, 0x19, 0x6c08);
        rtl8101_mdio_write(tp, 0x15, 0x02bd);
        rtl8101_mdio_write(tp, 0x19, 0xa523);
        rtl8101_mdio_write(tp, 0x15, 0x02be);
        rtl8101_mdio_write(tp, 0x19, 0x32ca);
        rtl8101_mdio_write(tp, 0x15, 0x02ca);
        rtl8101_mdio_write(tp, 0x19, 0x48b3);
        rtl8101_mdio_write(tp, 0x15, 0x02cb);
        rtl8101_mdio_write(tp, 0x19, 0x4020);
        rtl8101_mdio_write(tp, 0x15, 0x02cc);
        rtl8101_mdio_write(tp, 0x19, 0x4823);
        rtl8101_mdio_write(tp, 0x15, 0x02cd);
        rtl8101_mdio_write(tp, 0x19, 0x4510);
        rtl8101_mdio_write(tp, 0x15, 0x02ce);
        rtl8101_mdio_write(tp, 0x19, 0xb63a);
        rtl8101_mdio_write(tp, 0x15, 0x02cf);
        rtl8101_mdio_write(tp, 0x19, 0x7dc8);
        rtl8101_mdio_write(tp, 0x15, 0x02d6);
        rtl8101_mdio_write(tp, 0x19, 0x9bf8);
        rtl8101_mdio_write(tp, 0x15, 0x02d8);
        rtl8101_mdio_write(tp, 0x19, 0x85f6);
        rtl8101_mdio_write(tp, 0x15, 0x02d9);
        rtl8101_mdio_write(tp, 0x19, 0x32e0);
        rtl8101_mdio_write(tp, 0x15, 0x02e0);
        rtl8101_mdio_write(tp, 0x19, 0x4834);
        rtl8101_mdio_write(tp, 0x15, 0x02e1);
        rtl8101_mdio_write(tp, 0x19, 0x6c08);
        rtl8101_mdio_write(tp, 0x15, 0x02e2);
        rtl8101_mdio_write(tp, 0x19, 0x4020);
        rtl8101_mdio_write(tp, 0x15, 0x02e3);
        rtl8101_mdio_write(tp, 0x19, 0x4824);
        rtl8101_mdio_write(tp, 0x15, 0x02e4);
        rtl8101_mdio_write(tp, 0x19, 0x4520);
        rtl8101_mdio_write(tp, 0x15, 0x02e5);
        rtl8101_mdio_write(tp, 0x19, 0x4008);
        rtl8101_mdio_write(tp, 0x15, 0x02e6);
        rtl8101_mdio_write(tp, 0x19, 0x4560);
        rtl8101_mdio_write(tp, 0x15, 0x02e7);
        rtl8101_mdio_write(tp, 0x19, 0x9d04);
        rtl8101_mdio_write(tp, 0x15, 0x02e8);
        rtl8101_mdio_write(tp, 0x19, 0x48c4);
        rtl8101_mdio_write(tp, 0x15, 0x02e9);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x02ea);
        rtl8101_mdio_write(tp, 0x19, 0x4844);
        rtl8101_mdio_write(tp, 0x15, 0x02eb);
        rtl8101_mdio_write(tp, 0x19, 0x7dc8);
        rtl8101_mdio_write(tp, 0x15, 0x02f0);
        rtl8101_mdio_write(tp, 0x19, 0x9cf7);
        rtl8101_mdio_write(tp, 0x15, 0x02f1);
        rtl8101_mdio_write(tp, 0x19, 0xdf94);
        rtl8101_mdio_write(tp, 0x15, 0x02f2);
        rtl8101_mdio_write(tp, 0x19, 0x0002);
        rtl8101_mdio_write(tp, 0x15, 0x02f3);
        rtl8101_mdio_write(tp, 0x19, 0x6810);
        rtl8101_mdio_write(tp, 0x15, 0x02f4);
        rtl8101_mdio_write(tp, 0x19, 0xb614);
        rtl8101_mdio_write(tp, 0x15, 0x02f5);
        rtl8101_mdio_write(tp, 0x19, 0xc42b);
        rtl8101_mdio_write(tp, 0x15, 0x02f6);
        rtl8101_mdio_write(tp, 0x19, 0x00d4);
        rtl8101_mdio_write(tp, 0x15, 0x02f7);
        rtl8101_mdio_write(tp, 0x19, 0xc455);
        rtl8101_mdio_write(tp, 0x15, 0x02f8);
        rtl8101_mdio_write(tp, 0x19, 0x0093);
        rtl8101_mdio_write(tp, 0x15, 0x02f9);
        rtl8101_mdio_write(tp, 0x19, 0x92ee);
        rtl8101_mdio_write(tp, 0x15, 0x02fa);
        rtl8101_mdio_write(tp, 0x19, 0xefed);
        rtl8101_mdio_write(tp, 0x15, 0x02fb);
        rtl8101_mdio_write(tp, 0x19, 0x3312);
        rtl8101_mdio_write(tp, 0x15, 0x0312);
        rtl8101_mdio_write(tp, 0x19, 0x49b5);
        rtl8101_mdio_write(tp, 0x15, 0x0313);
        rtl8101_mdio_write(tp, 0x19, 0x7d00);
        rtl8101_mdio_write(tp, 0x15, 0x0314);
        rtl8101_mdio_write(tp, 0x19, 0x4d00);
        rtl8101_mdio_write(tp, 0x15, 0x0315);
        rtl8101_mdio_write(tp, 0x19, 0x6810);
        rtl8101_mdio_write(tp, 0x15, 0x031e);
        rtl8101_mdio_write(tp, 0x19, 0x404f);
        rtl8101_mdio_write(tp, 0x15, 0x031f);
        rtl8101_mdio_write(tp, 0x19, 0x44c8);
        rtl8101_mdio_write(tp, 0x15, 0x0320);
        rtl8101_mdio_write(tp, 0x19, 0xd64f);
        rtl8101_mdio_write(tp, 0x15, 0x0321);
        rtl8101_mdio_write(tp, 0x19, 0x00e7);
        rtl8101_mdio_write(tp, 0x15, 0x0322);
        rtl8101_mdio_write(tp, 0x19, 0x7c08);
        rtl8101_mdio_write(tp, 0x15, 0x0323);
        rtl8101_mdio_write(tp, 0x19, 0x8203);
        rtl8101_mdio_write(tp, 0x15, 0x0324);
        rtl8101_mdio_write(tp, 0x19, 0x4d48);
        rtl8101_mdio_write(tp, 0x15, 0x0325);
        rtl8101_mdio_write(tp, 0x19, 0x3327);
        rtl8101_mdio_write(tp, 0x15, 0x0326);
        rtl8101_mdio_write(tp, 0x19, 0x4d40);
        rtl8101_mdio_write(tp, 0x15, 0x0327);
        rtl8101_mdio_write(tp, 0x19, 0xc8d7);
        rtl8101_mdio_write(tp, 0x15, 0x0328);
        rtl8101_mdio_write(tp, 0x19, 0x0003);
        rtl8101_mdio_write(tp, 0x15, 0x0329);
        rtl8101_mdio_write(tp, 0x19, 0x7c20);
        rtl8101_mdio_write(tp, 0x15, 0x032a);
        rtl8101_mdio_write(tp, 0x19, 0x4c20);
        rtl8101_mdio_write(tp, 0x15, 0x032b);
        rtl8101_mdio_write(tp, 0x19, 0xc8ed);
        rtl8101_mdio_write(tp, 0x15, 0x032c);
        rtl8101_mdio_write(tp, 0x19, 0x00f4);
        rtl8101_mdio_write(tp, 0x15, 0x032d);
        rtl8101_mdio_write(tp, 0x19, 0x82b3);
        rtl8101_mdio_write(tp, 0x15, 0x032e);
        rtl8101_mdio_write(tp, 0x19, 0xd11d);
        rtl8101_mdio_write(tp, 0x15, 0x032f);
        rtl8101_mdio_write(tp, 0x19, 0x00b1);
        rtl8101_mdio_write(tp, 0x15, 0x0330);
        rtl8101_mdio_write(tp, 0x19, 0xde18);
        rtl8101_mdio_write(tp, 0x15, 0x0331);
        rtl8101_mdio_write(tp, 0x19, 0x0008);
        rtl8101_mdio_write(tp, 0x15, 0x0332);
        rtl8101_mdio_write(tp, 0x19, 0x91ee);
        rtl8101_mdio_write(tp, 0x15, 0x0333);
        rtl8101_mdio_write(tp, 0x19, 0x3339);
        rtl8101_mdio_write(tp, 0x15, 0x033a);
        rtl8101_mdio_write(tp, 0x19, 0x4064);
        rtl8101_mdio_write(tp, 0x15, 0x0340);
        rtl8101_mdio_write(tp, 0x19, 0x9e06);
        rtl8101_mdio_write(tp, 0x15, 0x0341);
        rtl8101_mdio_write(tp, 0x19, 0x7c08);
        rtl8101_mdio_write(tp, 0x15, 0x0342);
        rtl8101_mdio_write(tp, 0x19, 0x8203);
        rtl8101_mdio_write(tp, 0x15, 0x0343);
        rtl8101_mdio_write(tp, 0x19, 0x4d48);
        rtl8101_mdio_write(tp, 0x15, 0x0344);
        rtl8101_mdio_write(tp, 0x19, 0x3346);
        rtl8101_mdio_write(tp, 0x15, 0x0345);
        rtl8101_mdio_write(tp, 0x19, 0x4d40);
        rtl8101_mdio_write(tp, 0x15, 0x0346);
        rtl8101_mdio_write(tp, 0x19, 0xd11d);
        rtl8101_mdio_write(tp, 0x15, 0x0347);
        rtl8101_mdio_write(tp, 0x19, 0x0099);
        rtl8101_mdio_write(tp, 0x15, 0x0348);
        rtl8101_mdio_write(tp, 0x19, 0xbb17);
        rtl8101_mdio_write(tp, 0x15, 0x0349);
        rtl8101_mdio_write(tp, 0x19, 0x8102);
        rtl8101_mdio_write(tp, 0x15, 0x034a);
        rtl8101_mdio_write(tp, 0x19, 0x334d);
        rtl8101_mdio_write(tp, 0x15, 0x034b);
        rtl8101_mdio_write(tp, 0x19, 0xa22c);
        rtl8101_mdio_write(tp, 0x15, 0x034c);
        rtl8101_mdio_write(tp, 0x19, 0x3397);
        rtl8101_mdio_write(tp, 0x15, 0x034d);
        rtl8101_mdio_write(tp, 0x19, 0x91f2);
        rtl8101_mdio_write(tp, 0x15, 0x034e);
        rtl8101_mdio_write(tp, 0x19, 0xc218);
        rtl8101_mdio_write(tp, 0x15, 0x034f);
        rtl8101_mdio_write(tp, 0x19, 0x00f0);
        rtl8101_mdio_write(tp, 0x15, 0x0350);
        rtl8101_mdio_write(tp, 0x19, 0x3397);
        rtl8101_mdio_write(tp, 0x15, 0x0351);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0364);
        rtl8101_mdio_write(tp, 0x19, 0xbc05);
        rtl8101_mdio_write(tp, 0x15, 0x0367);
        rtl8101_mdio_write(tp, 0x19, 0xa1fc);
        rtl8101_mdio_write(tp, 0x15, 0x0368);
        rtl8101_mdio_write(tp, 0x19, 0x3377);
        rtl8101_mdio_write(tp, 0x15, 0x0369);
        rtl8101_mdio_write(tp, 0x19, 0x328b);
        rtl8101_mdio_write(tp, 0x15, 0x036a);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x15, 0x0377);
        rtl8101_mdio_write(tp, 0x19, 0x4b97);
        rtl8101_mdio_write(tp, 0x15, 0x0378);
        rtl8101_mdio_write(tp, 0x19, 0x6818);
        rtl8101_mdio_write(tp, 0x15, 0x0379);
        rtl8101_mdio_write(tp, 0x19, 0x4b07);
        rtl8101_mdio_write(tp, 0x15, 0x037a);
        rtl8101_mdio_write(tp, 0x19, 0x40ac);
        rtl8101_mdio_write(tp, 0x15, 0x037b);
        rtl8101_mdio_write(tp, 0x19, 0x4445);
        rtl8101_mdio_write(tp, 0x15, 0x037c);
        rtl8101_mdio_write(tp, 0x19, 0x404e);
        rtl8101_mdio_write(tp, 0x15, 0x037d);
        rtl8101_mdio_write(tp, 0x19, 0x4461);
        rtl8101_mdio_write(tp, 0x15, 0x037e);
        rtl8101_mdio_write(tp, 0x19, 0x9c09);
        rtl8101_mdio_write(tp, 0x15, 0x037f);
        rtl8101_mdio_write(tp, 0x19, 0x63da);
        rtl8101_mdio_write(tp, 0x15, 0x0380);
        rtl8101_mdio_write(tp, 0x19, 0x5440);
        rtl8101_mdio_write(tp, 0x15, 0x0381);
        rtl8101_mdio_write(tp, 0x19, 0x4b98);
        rtl8101_mdio_write(tp, 0x15, 0x0382);
        rtl8101_mdio_write(tp, 0x19, 0x7c60);
        rtl8101_mdio_write(tp, 0x15, 0x0383);
        rtl8101_mdio_write(tp, 0x19, 0x4c00);
        rtl8101_mdio_write(tp, 0x15, 0x0384);
        rtl8101_mdio_write(tp, 0x19, 0x4b08);
        rtl8101_mdio_write(tp, 0x15, 0x0385);
        rtl8101_mdio_write(tp, 0x19, 0x63d8);
        rtl8101_mdio_write(tp, 0x15, 0x0386);
        rtl8101_mdio_write(tp, 0x19, 0x338d);
        rtl8101_mdio_write(tp, 0x15, 0x0387);
        rtl8101_mdio_write(tp, 0x19, 0xd64f);
        rtl8101_mdio_write(tp, 0x15, 0x0388);
        rtl8101_mdio_write(tp, 0x19, 0x0080);
        rtl8101_mdio_write(tp, 0x15, 0x0389);
        rtl8101_mdio_write(tp, 0x19, 0x820c);
        rtl8101_mdio_write(tp, 0x15, 0x038a);
        rtl8101_mdio_write(tp, 0x19, 0xa10b);
        rtl8101_mdio_write(tp, 0x15, 0x038b);
        rtl8101_mdio_write(tp, 0x19, 0x9df3);
        rtl8101_mdio_write(tp, 0x15, 0x038c);
        rtl8101_mdio_write(tp, 0x19, 0x3395);
        rtl8101_mdio_write(tp, 0x15, 0x038d);
        rtl8101_mdio_write(tp, 0x19, 0xd64f);
        rtl8101_mdio_write(tp, 0x15, 0x038e);
        rtl8101_mdio_write(tp, 0x19, 0x00f9);
        rtl8101_mdio_write(tp, 0x15, 0x038f);
        rtl8101_mdio_write(tp, 0x19, 0xc017);
        rtl8101_mdio_write(tp, 0x15, 0x0390);
        rtl8101_mdio_write(tp, 0x19, 0x0005);
        rtl8101_mdio_write(tp, 0x15, 0x0391);
        rtl8101_mdio_write(tp, 0x19, 0x6c0b);
        rtl8101_mdio_write(tp, 0x15, 0x0392);
        rtl8101_mdio_write(tp, 0x19, 0xa103);
        rtl8101_mdio_write(tp, 0x15, 0x0393);
        rtl8101_mdio_write(tp, 0x19, 0x6c08);
        rtl8101_mdio_write(tp, 0x15, 0x0394);
        rtl8101_mdio_write(tp, 0x19, 0x9df9);
        rtl8101_mdio_write(tp, 0x15, 0x0395);
        rtl8101_mdio_write(tp, 0x19, 0x6c08);
        rtl8101_mdio_write(tp, 0x15, 0x0396);
        rtl8101_mdio_write(tp, 0x19, 0x3397);
        rtl8101_mdio_write(tp, 0x15, 0x0399);
        rtl8101_mdio_write(tp, 0x19, 0x6810);
        rtl8101_mdio_write(tp, 0x15, 0x03a4);
        rtl8101_mdio_write(tp, 0x19, 0x7c08);
        rtl8101_mdio_write(tp, 0x15, 0x03a5);
        rtl8101_mdio_write(tp, 0x19, 0x8203);
        rtl8101_mdio_write(tp, 0x15, 0x03a6);
        rtl8101_mdio_write(tp, 0x19, 0x4d08);
        rtl8101_mdio_write(tp, 0x15, 0x03a7);
        rtl8101_mdio_write(tp, 0x19, 0x33a9);
        rtl8101_mdio_write(tp, 0x15, 0x03a8);
        rtl8101_mdio_write(tp, 0x19, 0x4d00);
        rtl8101_mdio_write(tp, 0x15, 0x03a9);
        rtl8101_mdio_write(tp, 0x19, 0x9bfa);
        rtl8101_mdio_write(tp, 0x15, 0x03aa);
        rtl8101_mdio_write(tp, 0x19, 0x33b6);
        rtl8101_mdio_write(tp, 0x15, 0x03bb);
        rtl8101_mdio_write(tp, 0x19, 0x4056);
        rtl8101_mdio_write(tp, 0x15, 0x03bc);
        rtl8101_mdio_write(tp, 0x19, 0x44e9);
        rtl8101_mdio_write(tp, 0x15, 0x03bd);
        rtl8101_mdio_write(tp, 0x19, 0x4054);
        rtl8101_mdio_write(tp, 0x15, 0x03be);
        rtl8101_mdio_write(tp, 0x19, 0x44f8);
        rtl8101_mdio_write(tp, 0x15, 0x03bf);
        rtl8101_mdio_write(tp, 0x19, 0xd64f);
        rtl8101_mdio_write(tp, 0x15, 0x03c0);
        rtl8101_mdio_write(tp, 0x19, 0x0037);
        rtl8101_mdio_write(tp, 0x15, 0x03c1);
        rtl8101_mdio_write(tp, 0x19, 0xbd37);
        rtl8101_mdio_write(tp, 0x15, 0x03c2);
        rtl8101_mdio_write(tp, 0x19, 0x9cfd);
        rtl8101_mdio_write(tp, 0x15, 0x03c3);
        rtl8101_mdio_write(tp, 0x19, 0xc639);
        rtl8101_mdio_write(tp, 0x15, 0x03c4);
        rtl8101_mdio_write(tp, 0x19, 0x0011);
        rtl8101_mdio_write(tp, 0x15, 0x03c5);
        rtl8101_mdio_write(tp, 0x19, 0x9b03);
        rtl8101_mdio_write(tp, 0x15, 0x03c6);
        rtl8101_mdio_write(tp, 0x19, 0x7c01);
        rtl8101_mdio_write(tp, 0x15, 0x03c7);
        rtl8101_mdio_write(tp, 0x19, 0x4c01);
        rtl8101_mdio_write(tp, 0x15, 0x03c8);
        rtl8101_mdio_write(tp, 0x19, 0x9e03);
        rtl8101_mdio_write(tp, 0x15, 0x03c9);
        rtl8101_mdio_write(tp, 0x19, 0x7c20);
        rtl8101_mdio_write(tp, 0x15, 0x03ca);
        rtl8101_mdio_write(tp, 0x19, 0x4c20);
        rtl8101_mdio_write(tp, 0x15, 0x03cb);
        rtl8101_mdio_write(tp, 0x19, 0x9af4);
        rtl8101_mdio_write(tp, 0x15, 0x03cc);
        rtl8101_mdio_write(tp, 0x19, 0x7c12);
        rtl8101_mdio_write(tp, 0x15, 0x03cd);
        rtl8101_mdio_write(tp, 0x19, 0x4c52);
        rtl8101_mdio_write(tp, 0x15, 0x03ce);
        rtl8101_mdio_write(tp, 0x19, 0x4470);
        rtl8101_mdio_write(tp, 0x15, 0x03cf);
        rtl8101_mdio_write(tp, 0x19, 0x7c12);
        rtl8101_mdio_write(tp, 0x15, 0x03d0);
        rtl8101_mdio_write(tp, 0x19, 0x4c40);
        rtl8101_mdio_write(tp, 0x15, 0x03d1);
        rtl8101_mdio_write(tp, 0x19, 0x33bf);
        rtl8101_mdio_write(tp, 0x15, 0x03d6);
        rtl8101_mdio_write(tp, 0x19, 0x4047);
        rtl8101_mdio_write(tp, 0x15, 0x03d7);
        rtl8101_mdio_write(tp, 0x19, 0x4469);
        rtl8101_mdio_write(tp, 0x15, 0x03d8);
        rtl8101_mdio_write(tp, 0x19, 0x492b);
        rtl8101_mdio_write(tp, 0x15, 0x03d9);
        rtl8101_mdio_write(tp, 0x19, 0x4479);
        rtl8101_mdio_write(tp, 0x15, 0x03da);
        rtl8101_mdio_write(tp, 0x19, 0x7c09);
        rtl8101_mdio_write(tp, 0x15, 0x03db);
        rtl8101_mdio_write(tp, 0x19, 0x8203);
        rtl8101_mdio_write(tp, 0x15, 0x03dc);
        rtl8101_mdio_write(tp, 0x19, 0x4d48);
        rtl8101_mdio_write(tp, 0x15, 0x03dd);
        rtl8101_mdio_write(tp, 0x19, 0x33df);
        rtl8101_mdio_write(tp, 0x15, 0x03de);
        rtl8101_mdio_write(tp, 0x19, 0x4d40);
        rtl8101_mdio_write(tp, 0x15, 0x03df);
        rtl8101_mdio_write(tp, 0x19, 0xd64f);
        rtl8101_mdio_write(tp, 0x15, 0x03e0);
        rtl8101_mdio_write(tp, 0x19, 0x0017);
        rtl8101_mdio_write(tp, 0x15, 0x03e1);
        rtl8101_mdio_write(tp, 0x19, 0xbd17);
        rtl8101_mdio_write(tp, 0x15, 0x03e2);
        rtl8101_mdio_write(tp, 0x19, 0x9b03);
        rtl8101_mdio_write(tp, 0x15, 0x03e3);
        rtl8101_mdio_write(tp, 0x19, 0x7c20);
        rtl8101_mdio_write(tp, 0x15, 0x03e4);
        rtl8101_mdio_write(tp, 0x19, 0x4c20);
        rtl8101_mdio_write(tp, 0x15, 0x03e5);
        rtl8101_mdio_write(tp, 0x19, 0x88f5);
        rtl8101_mdio_write(tp, 0x15, 0x03e6);
        rtl8101_mdio_write(tp, 0x19, 0xc428);
        rtl8101_mdio_write(tp, 0x15, 0x03e7);
        rtl8101_mdio_write(tp, 0x19, 0x0008);
        rtl8101_mdio_write(tp, 0x15, 0x03e8);
        rtl8101_mdio_write(tp, 0x19, 0x9af2);
        rtl8101_mdio_write(tp, 0x15, 0x03e9);
        rtl8101_mdio_write(tp, 0x19, 0x7c12);
        rtl8101_mdio_write(tp, 0x15, 0x03ea);
        rtl8101_mdio_write(tp, 0x19, 0x4c52);
        rtl8101_mdio_write(tp, 0x15, 0x03eb);
        rtl8101_mdio_write(tp, 0x19, 0x4470);
        rtl8101_mdio_write(tp, 0x15, 0x03ec);
        rtl8101_mdio_write(tp, 0x19, 0x7c12);
        rtl8101_mdio_write(tp, 0x15, 0x03ed);
        rtl8101_mdio_write(tp, 0x19, 0x4c40);
        rtl8101_mdio_write(tp, 0x15, 0x03ee);
        rtl8101_mdio_write(tp, 0x19, 0x33da);
        rtl8101_mdio_write(tp, 0x15, 0x03ef);
        rtl8101_mdio_write(tp, 0x19, 0x3312);
        rtl8101_mdio_write(tp, 0x16, 0x0306);
        rtl8101_mdio_write(tp, 0x16, 0x0300);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x17, 0x2179);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0040);
        rtl8101_mdio_write(tp, 0x18, 0x0645);
        rtl8101_mdio_write(tp, 0x19, 0xe200);
        rtl8101_mdio_write(tp, 0x18, 0x0655);
        rtl8101_mdio_write(tp, 0x19, 0x9000);
        rtl8101_mdio_write(tp, 0x18, 0x0d05);
        rtl8101_mdio_write(tp, 0x19, 0xbe00);
        rtl8101_mdio_write(tp, 0x18, 0x0d15);
        rtl8101_mdio_write(tp, 0x19, 0xd300);
        rtl8101_mdio_write(tp, 0x18, 0x0d25);
        rtl8101_mdio_write(tp, 0x19, 0xfe00);
        rtl8101_mdio_write(tp, 0x18, 0x0d35);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x0d45);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x0d55);
        rtl8101_mdio_write(tp, 0x19, 0x1000);
        rtl8101_mdio_write(tp, 0x18, 0x0d65);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x0d75);
        rtl8101_mdio_write(tp, 0x19, 0x8200);
        rtl8101_mdio_write(tp, 0x18, 0x0d85);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x0d95);
        rtl8101_mdio_write(tp, 0x19, 0x7000);
        rtl8101_mdio_write(tp, 0x18, 0x0da5);
        rtl8101_mdio_write(tp, 0x19, 0x0f00);
        rtl8101_mdio_write(tp, 0x18, 0x0db5);
        rtl8101_mdio_write(tp, 0x19, 0x0100);
        rtl8101_mdio_write(tp, 0x18, 0x0dc5);
        rtl8101_mdio_write(tp, 0x19, 0x9b00);
        rtl8101_mdio_write(tp, 0x18, 0x0dd5);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x0de5);
        rtl8101_mdio_write(tp, 0x19, 0xe000);
        rtl8101_mdio_write(tp, 0x18, 0x0df5);
        rtl8101_mdio_write(tp, 0x19, 0xef00);
        rtl8101_mdio_write(tp, 0x18, 0x16d5);
        rtl8101_mdio_write(tp, 0x19, 0xe200);
        rtl8101_mdio_write(tp, 0x18, 0x16e5);
        rtl8101_mdio_write(tp, 0x19, 0xab00);
        rtl8101_mdio_write(tp, 0x18, 0x2904);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x2914);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x2924);
        rtl8101_mdio_write(tp, 0x19, 0x0100);
        rtl8101_mdio_write(tp, 0x18, 0x2934);
        rtl8101_mdio_write(tp, 0x19, 0x2000);
        rtl8101_mdio_write(tp, 0x18, 0x2944);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2954);
        rtl8101_mdio_write(tp, 0x19, 0x4600);
        rtl8101_mdio_write(tp, 0x18, 0x2964);
        rtl8101_mdio_write(tp, 0x19, 0xfc00);
        rtl8101_mdio_write(tp, 0x18, 0x2974);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2984);
        rtl8101_mdio_write(tp, 0x19, 0x5000);
        rtl8101_mdio_write(tp, 0x18, 0x2994);
        rtl8101_mdio_write(tp, 0x19, 0x9d00);
        rtl8101_mdio_write(tp, 0x18, 0x29a4);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x29b4);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x29c4);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x29d4);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x29e4);
        rtl8101_mdio_write(tp, 0x19, 0x2000);
        rtl8101_mdio_write(tp, 0x18, 0x29f4);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2a04);
        rtl8101_mdio_write(tp, 0x19, 0xe600);
        rtl8101_mdio_write(tp, 0x18, 0x2a14);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x2a24);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2a34);
        rtl8101_mdio_write(tp, 0x19, 0x5000);
        rtl8101_mdio_write(tp, 0x18, 0x2a44);
        rtl8101_mdio_write(tp, 0x19, 0x8500);
        rtl8101_mdio_write(tp, 0x18, 0x2a54);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x2a64);
        rtl8101_mdio_write(tp, 0x19, 0xac00);
        rtl8101_mdio_write(tp, 0x18, 0x2a74);
        rtl8101_mdio_write(tp, 0x19, 0x0800);
        rtl8101_mdio_write(tp, 0x18, 0x2a84);
        rtl8101_mdio_write(tp, 0x19, 0xfc00);
        rtl8101_mdio_write(tp, 0x18, 0x2a94);
        rtl8101_mdio_write(tp, 0x19, 0xe000);
        rtl8101_mdio_write(tp, 0x18, 0x2aa4);
        rtl8101_mdio_write(tp, 0x19, 0x7400);
        rtl8101_mdio_write(tp, 0x18, 0x2ab4);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x2ac4);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x2ad4);
        rtl8101_mdio_write(tp, 0x19, 0x0100);
        rtl8101_mdio_write(tp, 0x18, 0x2ae4);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x2af4);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2b04);
        rtl8101_mdio_write(tp, 0x19, 0x4400);
        rtl8101_mdio_write(tp, 0x18, 0x2b14);
        rtl8101_mdio_write(tp, 0x19, 0xfc00);
        rtl8101_mdio_write(tp, 0x18, 0x2b24);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2b34);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x2b44);
        rtl8101_mdio_write(tp, 0x19, 0x9d00);
        rtl8101_mdio_write(tp, 0x18, 0x2b54);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x2b64);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x2b74);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x2b84);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2b94);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x2ba4);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2bb4);
        rtl8101_mdio_write(tp, 0x19, 0xfc00);
        rtl8101_mdio_write(tp, 0x18, 0x2bc4);
        rtl8101_mdio_write(tp, 0x19, 0xff00);
        rtl8101_mdio_write(tp, 0x18, 0x2bd4);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2be4);
        rtl8101_mdio_write(tp, 0x19, 0x4000);
        rtl8101_mdio_write(tp, 0x18, 0x2bf4);
        rtl8101_mdio_write(tp, 0x19, 0x8900);
        rtl8101_mdio_write(tp, 0x18, 0x2c04);
        rtl8101_mdio_write(tp, 0x19, 0x8300);
        rtl8101_mdio_write(tp, 0x18, 0x2c14);
        rtl8101_mdio_write(tp, 0x19, 0xe000);
        rtl8101_mdio_write(tp, 0x18, 0x2c24);
        rtl8101_mdio_write(tp, 0x19, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x2c34);
        rtl8101_mdio_write(tp, 0x19, 0xac00);
        rtl8101_mdio_write(tp, 0x18, 0x2c44);
        rtl8101_mdio_write(tp, 0x19, 0x0800);
        rtl8101_mdio_write(tp, 0x18, 0x2c54);
        rtl8101_mdio_write(tp, 0x19, 0xfa00);
        rtl8101_mdio_write(tp, 0x18, 0x2c64);
        rtl8101_mdio_write(tp, 0x19, 0xe100);
        rtl8101_mdio_write(tp, 0x18, 0x2c74);
        rtl8101_mdio_write(tp, 0x19, 0x7f00);
        rtl8101_mdio_write(tp, 0x18, 0x0001);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x17, 0x2100);
        rtl8101_mdio_write(tp, 0x1f, 0x0005);
        rtl8101_mdio_write(tp, 0x05, 0xfff6);
        rtl8101_mdio_write(tp, 0x06, 0x0080);
        rtl8101_mdio_write(tp, 0x05, 0x8000);
        rtl8101_mdio_write(tp, 0x06, 0xd480);
        rtl8101_mdio_write(tp, 0x06, 0xc1e4);
        rtl8101_mdio_write(tp, 0x06, 0x8b9a);
        rtl8101_mdio_write(tp, 0x06, 0xe58b);
        rtl8101_mdio_write(tp, 0x06, 0x9bee);
        rtl8101_mdio_write(tp, 0x06, 0x8b83);
        rtl8101_mdio_write(tp, 0x06, 0x41bf);
        rtl8101_mdio_write(tp, 0x06, 0x8b88);
        rtl8101_mdio_write(tp, 0x06, 0xec00);
        rtl8101_mdio_write(tp, 0x06, 0x19a9);
        rtl8101_mdio_write(tp, 0x06, 0x8b90);
        rtl8101_mdio_write(tp, 0x06, 0xf9ee);
        rtl8101_mdio_write(tp, 0x06, 0xfff6);
        rtl8101_mdio_write(tp, 0x06, 0x00ee);
        rtl8101_mdio_write(tp, 0x06, 0xfff7);
        rtl8101_mdio_write(tp, 0x06, 0xffe0);
        rtl8101_mdio_write(tp, 0x06, 0xe140);
        rtl8101_mdio_write(tp, 0x06, 0xe1e1);
        rtl8101_mdio_write(tp, 0x06, 0x41f7);
        rtl8101_mdio_write(tp, 0x06, 0x2ff6);
        rtl8101_mdio_write(tp, 0x06, 0x28e4);
        rtl8101_mdio_write(tp, 0x06, 0xe140);
        rtl8101_mdio_write(tp, 0x06, 0xe5e1);
        rtl8101_mdio_write(tp, 0x06, 0x41f7);
        rtl8101_mdio_write(tp, 0x06, 0x0002);
        rtl8101_mdio_write(tp, 0x06, 0x020c);
        rtl8101_mdio_write(tp, 0x06, 0x0202);
        rtl8101_mdio_write(tp, 0x06, 0x1d02);
        rtl8101_mdio_write(tp, 0x06, 0x0230);
        rtl8101_mdio_write(tp, 0x06, 0x0202);
        rtl8101_mdio_write(tp, 0x06, 0x4002);
        rtl8101_mdio_write(tp, 0x06, 0x028b);
        rtl8101_mdio_write(tp, 0x06, 0x0280);
        rtl8101_mdio_write(tp, 0x06, 0x6c02);
        rtl8101_mdio_write(tp, 0x06, 0x8085);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x88e1);
        rtl8101_mdio_write(tp, 0x06, 0x8b89);
        rtl8101_mdio_write(tp, 0x06, 0x1e01);
        rtl8101_mdio_write(tp, 0x06, 0xe18b);
        rtl8101_mdio_write(tp, 0x06, 0x8a1e);
        rtl8101_mdio_write(tp, 0x06, 0x01e1);
        rtl8101_mdio_write(tp, 0x06, 0x8b8b);
        rtl8101_mdio_write(tp, 0x06, 0x1e01);
        rtl8101_mdio_write(tp, 0x06, 0xe18b);
        rtl8101_mdio_write(tp, 0x06, 0x8c1e);
        rtl8101_mdio_write(tp, 0x06, 0x01e1);
        rtl8101_mdio_write(tp, 0x06, 0x8b8d);
        rtl8101_mdio_write(tp, 0x06, 0x1e01);
        rtl8101_mdio_write(tp, 0x06, 0xe18b);
        rtl8101_mdio_write(tp, 0x06, 0x8e1e);
        rtl8101_mdio_write(tp, 0x06, 0x01a0);
        rtl8101_mdio_write(tp, 0x06, 0x00c7);
        rtl8101_mdio_write(tp, 0x06, 0xaec3);
        rtl8101_mdio_write(tp, 0x06, 0xf8e0);
        rtl8101_mdio_write(tp, 0x06, 0x8b8d);
        rtl8101_mdio_write(tp, 0x06, 0xad20);
        rtl8101_mdio_write(tp, 0x06, 0x10ee);
        rtl8101_mdio_write(tp, 0x06, 0x8b8d);
        rtl8101_mdio_write(tp, 0x06, 0x0002);
        rtl8101_mdio_write(tp, 0x06, 0x1310);
        rtl8101_mdio_write(tp, 0x06, 0x0280);
        rtl8101_mdio_write(tp, 0x06, 0xc602);
        rtl8101_mdio_write(tp, 0x06, 0x1f0c);
        rtl8101_mdio_write(tp, 0x06, 0x0227);
        rtl8101_mdio_write(tp, 0x06, 0x49fc);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x8ead);
        rtl8101_mdio_write(tp, 0x06, 0x200b);
        rtl8101_mdio_write(tp, 0x06, 0xf620);
        rtl8101_mdio_write(tp, 0x06, 0xe48b);
        rtl8101_mdio_write(tp, 0x06, 0x8e02);
        rtl8101_mdio_write(tp, 0x06, 0x852d);
        rtl8101_mdio_write(tp, 0x06, 0x021b);
        rtl8101_mdio_write(tp, 0x06, 0x67ad);
        rtl8101_mdio_write(tp, 0x06, 0x2211);
        rtl8101_mdio_write(tp, 0x06, 0xf622);
        rtl8101_mdio_write(tp, 0x06, 0xe48b);
        rtl8101_mdio_write(tp, 0x06, 0x8e02);
        rtl8101_mdio_write(tp, 0x06, 0x2ba5);
        rtl8101_mdio_write(tp, 0x06, 0x022a);
        rtl8101_mdio_write(tp, 0x06, 0x2402);
        rtl8101_mdio_write(tp, 0x06, 0x82e5);
        rtl8101_mdio_write(tp, 0x06, 0x022a);
        rtl8101_mdio_write(tp, 0x06, 0xf0ad);
        rtl8101_mdio_write(tp, 0x06, 0x2511);
        rtl8101_mdio_write(tp, 0x06, 0xf625);
        rtl8101_mdio_write(tp, 0x06, 0xe48b);
        rtl8101_mdio_write(tp, 0x06, 0x8e02);
        rtl8101_mdio_write(tp, 0x06, 0x8445);
        rtl8101_mdio_write(tp, 0x06, 0x0204);
        rtl8101_mdio_write(tp, 0x06, 0x0302);
        rtl8101_mdio_write(tp, 0x06, 0x19cc);
        rtl8101_mdio_write(tp, 0x06, 0x022b);
        rtl8101_mdio_write(tp, 0x06, 0x5bfc);
        rtl8101_mdio_write(tp, 0x06, 0x04ee);
        rtl8101_mdio_write(tp, 0x06, 0x8b8d);
        rtl8101_mdio_write(tp, 0x06, 0x0105);
        rtl8101_mdio_write(tp, 0x06, 0xf8f9);
        rtl8101_mdio_write(tp, 0x06, 0xfae0);
        rtl8101_mdio_write(tp, 0x06, 0x8b81);
        rtl8101_mdio_write(tp, 0x06, 0xac26);
        rtl8101_mdio_write(tp, 0x06, 0x08e0);
        rtl8101_mdio_write(tp, 0x06, 0x8b81);
        rtl8101_mdio_write(tp, 0x06, 0xac21);
        rtl8101_mdio_write(tp, 0x06, 0x02ae);
        rtl8101_mdio_write(tp, 0x06, 0x6bee);
        rtl8101_mdio_write(tp, 0x06, 0xe0ea);
        rtl8101_mdio_write(tp, 0x06, 0x00ee);
        rtl8101_mdio_write(tp, 0x06, 0xe0eb);
        rtl8101_mdio_write(tp, 0x06, 0x00e2);
        rtl8101_mdio_write(tp, 0x06, 0xe07c);
        rtl8101_mdio_write(tp, 0x06, 0xe3e0);
        rtl8101_mdio_write(tp, 0x06, 0x7da5);
        rtl8101_mdio_write(tp, 0x06, 0x1111);
        rtl8101_mdio_write(tp, 0x06, 0x15d2);
        rtl8101_mdio_write(tp, 0x06, 0x60d6);
        rtl8101_mdio_write(tp, 0x06, 0x6666);
        rtl8101_mdio_write(tp, 0x06, 0x0207);
        rtl8101_mdio_write(tp, 0x06, 0x6cd2);
        rtl8101_mdio_write(tp, 0x06, 0xa0d6);
        rtl8101_mdio_write(tp, 0x06, 0xaaaa);
        rtl8101_mdio_write(tp, 0x06, 0x0207);
        rtl8101_mdio_write(tp, 0x06, 0x6c02);
        rtl8101_mdio_write(tp, 0x06, 0x201d);
        rtl8101_mdio_write(tp, 0x06, 0xae44);
        rtl8101_mdio_write(tp, 0x06, 0xa566);
        rtl8101_mdio_write(tp, 0x06, 0x6602);
        rtl8101_mdio_write(tp, 0x06, 0xae38);
        rtl8101_mdio_write(tp, 0x06, 0xa5aa);
        rtl8101_mdio_write(tp, 0x06, 0xaa02);
        rtl8101_mdio_write(tp, 0x06, 0xae32);
        rtl8101_mdio_write(tp, 0x06, 0xeee0);
        rtl8101_mdio_write(tp, 0x06, 0xea04);
        rtl8101_mdio_write(tp, 0x06, 0xeee0);
        rtl8101_mdio_write(tp, 0x06, 0xeb06);
        rtl8101_mdio_write(tp, 0x06, 0xe2e0);
        rtl8101_mdio_write(tp, 0x06, 0x7ce3);
        rtl8101_mdio_write(tp, 0x06, 0xe07d);
        rtl8101_mdio_write(tp, 0x06, 0xe0e0);
        rtl8101_mdio_write(tp, 0x06, 0x38e1);
        rtl8101_mdio_write(tp, 0x06, 0xe039);
        rtl8101_mdio_write(tp, 0x06, 0xad2e);
        rtl8101_mdio_write(tp, 0x06, 0x21ad);
        rtl8101_mdio_write(tp, 0x06, 0x3f13);
        rtl8101_mdio_write(tp, 0x06, 0xe0e4);
        rtl8101_mdio_write(tp, 0x06, 0x14e1);
        rtl8101_mdio_write(tp, 0x06, 0xe415);
        rtl8101_mdio_write(tp, 0x06, 0x6880);
        rtl8101_mdio_write(tp, 0x06, 0xe4e4);
        rtl8101_mdio_write(tp, 0x06, 0x14e5);
        rtl8101_mdio_write(tp, 0x06, 0xe415);
        rtl8101_mdio_write(tp, 0x06, 0x0220);
        rtl8101_mdio_write(tp, 0x06, 0x1dae);
        rtl8101_mdio_write(tp, 0x06, 0x0bac);
        rtl8101_mdio_write(tp, 0x06, 0x3e02);
        rtl8101_mdio_write(tp, 0x06, 0xae06);
        rtl8101_mdio_write(tp, 0x06, 0x0281);
        rtl8101_mdio_write(tp, 0x06, 0x4602);
        rtl8101_mdio_write(tp, 0x06, 0x2057);
        rtl8101_mdio_write(tp, 0x06, 0xfefd);
        rtl8101_mdio_write(tp, 0x06, 0xfc04);
        rtl8101_mdio_write(tp, 0x06, 0xf8e0);
        rtl8101_mdio_write(tp, 0x06, 0x8b81);
        rtl8101_mdio_write(tp, 0x06, 0xad26);
        rtl8101_mdio_write(tp, 0x06, 0x0302);
        rtl8101_mdio_write(tp, 0x06, 0x20a7);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x81ad);
        rtl8101_mdio_write(tp, 0x06, 0x2109);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x2eac);
        rtl8101_mdio_write(tp, 0x06, 0x2003);
        rtl8101_mdio_write(tp, 0x06, 0x0281);
        rtl8101_mdio_write(tp, 0x06, 0x61fc);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x81ac);
        rtl8101_mdio_write(tp, 0x06, 0x2505);
        rtl8101_mdio_write(tp, 0x06, 0x0222);
        rtl8101_mdio_write(tp, 0x06, 0xaeae);
        rtl8101_mdio_write(tp, 0x06, 0x0302);
        rtl8101_mdio_write(tp, 0x06, 0x8172);
        rtl8101_mdio_write(tp, 0x06, 0xfc04);
        rtl8101_mdio_write(tp, 0x06, 0xf8f9);
        rtl8101_mdio_write(tp, 0x06, 0xfaef);
        rtl8101_mdio_write(tp, 0x06, 0x69fa);
        rtl8101_mdio_write(tp, 0x06, 0xe086);
        rtl8101_mdio_write(tp, 0x06, 0x20a0);
        rtl8101_mdio_write(tp, 0x06, 0x8016);
        rtl8101_mdio_write(tp, 0x06, 0xe086);
        rtl8101_mdio_write(tp, 0x06, 0x21e1);
        rtl8101_mdio_write(tp, 0x06, 0x8b33);
        rtl8101_mdio_write(tp, 0x06, 0x1b10);
        rtl8101_mdio_write(tp, 0x06, 0x9e06);
        rtl8101_mdio_write(tp, 0x06, 0x0223);
        rtl8101_mdio_write(tp, 0x06, 0x91af);
        rtl8101_mdio_write(tp, 0x06, 0x8252);
        rtl8101_mdio_write(tp, 0x06, 0xee86);
        rtl8101_mdio_write(tp, 0x06, 0x2081);
        rtl8101_mdio_write(tp, 0x06, 0xaee4);
        rtl8101_mdio_write(tp, 0x06, 0xa081);
        rtl8101_mdio_write(tp, 0x06, 0x1402);
        rtl8101_mdio_write(tp, 0x06, 0x2399);
        rtl8101_mdio_write(tp, 0x06, 0xbf25);
        rtl8101_mdio_write(tp, 0x06, 0xcc02);
        rtl8101_mdio_write(tp, 0x06, 0x2d21);
        rtl8101_mdio_write(tp, 0x06, 0xee86);
        rtl8101_mdio_write(tp, 0x06, 0x2100);
        rtl8101_mdio_write(tp, 0x06, 0xee86);
        rtl8101_mdio_write(tp, 0x06, 0x2082);
        rtl8101_mdio_write(tp, 0x06, 0xaf82);
        rtl8101_mdio_write(tp, 0x06, 0x52a0);
        rtl8101_mdio_write(tp, 0x06, 0x8232);
        rtl8101_mdio_write(tp, 0x06, 0xe086);
        rtl8101_mdio_write(tp, 0x06, 0x21e1);
        rtl8101_mdio_write(tp, 0x06, 0x8b32);
        rtl8101_mdio_write(tp, 0x06, 0x1b10);
        rtl8101_mdio_write(tp, 0x06, 0x9e06);
        rtl8101_mdio_write(tp, 0x06, 0x0223);
        rtl8101_mdio_write(tp, 0x06, 0x91af);
        rtl8101_mdio_write(tp, 0x06, 0x8252);
        rtl8101_mdio_write(tp, 0x06, 0xee86);
        rtl8101_mdio_write(tp, 0x06, 0x2100);
        rtl8101_mdio_write(tp, 0x06, 0xd000);
        rtl8101_mdio_write(tp, 0x06, 0x0282);
        rtl8101_mdio_write(tp, 0x06, 0x5910);
        rtl8101_mdio_write(tp, 0x06, 0xa004);
        rtl8101_mdio_write(tp, 0x06, 0xf9e0);
        rtl8101_mdio_write(tp, 0x06, 0x861f);
        rtl8101_mdio_write(tp, 0x06, 0xa000);
        rtl8101_mdio_write(tp, 0x06, 0x07ee);
        rtl8101_mdio_write(tp, 0x06, 0x8620);
        rtl8101_mdio_write(tp, 0x06, 0x83af);
        rtl8101_mdio_write(tp, 0x06, 0x8178);
        rtl8101_mdio_write(tp, 0x06, 0x0224);
        rtl8101_mdio_write(tp, 0x06, 0x0102);
        rtl8101_mdio_write(tp, 0x06, 0x2399);
        rtl8101_mdio_write(tp, 0x06, 0xae72);
        rtl8101_mdio_write(tp, 0x06, 0xa083);
        rtl8101_mdio_write(tp, 0x06, 0x4b1f);
        rtl8101_mdio_write(tp, 0x06, 0x55d0);
        rtl8101_mdio_write(tp, 0x06, 0x04bf);
        rtl8101_mdio_write(tp, 0x06, 0x8615);
        rtl8101_mdio_write(tp, 0x06, 0x1a90);
        rtl8101_mdio_write(tp, 0x06, 0x0c54);
        rtl8101_mdio_write(tp, 0x06, 0xd91e);
        rtl8101_mdio_write(tp, 0x06, 0x31b0);
        rtl8101_mdio_write(tp, 0x06, 0xf4e0);
        rtl8101_mdio_write(tp, 0x06, 0xe022);
        rtl8101_mdio_write(tp, 0x06, 0xe1e0);
        rtl8101_mdio_write(tp, 0x06, 0x23ad);
        rtl8101_mdio_write(tp, 0x06, 0x2e0c);
        rtl8101_mdio_write(tp, 0x06, 0xef02);
        rtl8101_mdio_write(tp, 0x06, 0xef12);
        rtl8101_mdio_write(tp, 0x06, 0x0e44);
        rtl8101_mdio_write(tp, 0x06, 0xef23);
        rtl8101_mdio_write(tp, 0x06, 0x0e54);
        rtl8101_mdio_write(tp, 0x06, 0xef21);
        rtl8101_mdio_write(tp, 0x06, 0xe6e4);
        rtl8101_mdio_write(tp, 0x06, 0x2ae7);
        rtl8101_mdio_write(tp, 0x06, 0xe42b);
        rtl8101_mdio_write(tp, 0x06, 0xe2e4);
        rtl8101_mdio_write(tp, 0x06, 0x28e3);
        rtl8101_mdio_write(tp, 0x06, 0xe429);
        rtl8101_mdio_write(tp, 0x06, 0x6d20);
        rtl8101_mdio_write(tp, 0x06, 0x00e6);
        rtl8101_mdio_write(tp, 0x06, 0xe428);
        rtl8101_mdio_write(tp, 0x06, 0xe7e4);
        rtl8101_mdio_write(tp, 0x06, 0x29bf);
        rtl8101_mdio_write(tp, 0x06, 0x25ca);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0x21ee);
        rtl8101_mdio_write(tp, 0x06, 0x8620);
        rtl8101_mdio_write(tp, 0x06, 0x84ee);
        rtl8101_mdio_write(tp, 0x06, 0x8621);
        rtl8101_mdio_write(tp, 0x06, 0x00af);
        rtl8101_mdio_write(tp, 0x06, 0x8178);
        rtl8101_mdio_write(tp, 0x06, 0xa084);
        rtl8101_mdio_write(tp, 0x06, 0x19e0);
        rtl8101_mdio_write(tp, 0x06, 0x8621);
        rtl8101_mdio_write(tp, 0x06, 0xe18b);
        rtl8101_mdio_write(tp, 0x06, 0x341b);
        rtl8101_mdio_write(tp, 0x06, 0x109e);
        rtl8101_mdio_write(tp, 0x06, 0x0602);
        rtl8101_mdio_write(tp, 0x06, 0x2391);
        rtl8101_mdio_write(tp, 0x06, 0xaf82);
        rtl8101_mdio_write(tp, 0x06, 0x5202);
        rtl8101_mdio_write(tp, 0x06, 0x241f);
        rtl8101_mdio_write(tp, 0x06, 0xee86);
        rtl8101_mdio_write(tp, 0x06, 0x2085);
        rtl8101_mdio_write(tp, 0x06, 0xae08);
        rtl8101_mdio_write(tp, 0x06, 0xa085);
        rtl8101_mdio_write(tp, 0x06, 0x02ae);
        rtl8101_mdio_write(tp, 0x06, 0x0302);
        rtl8101_mdio_write(tp, 0x06, 0x2442);
        rtl8101_mdio_write(tp, 0x06, 0xfeef);
        rtl8101_mdio_write(tp, 0x06, 0x96fe);
        rtl8101_mdio_write(tp, 0x06, 0xfdfc);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xf9fa);
        rtl8101_mdio_write(tp, 0x06, 0xef69);
        rtl8101_mdio_write(tp, 0x06, 0xfad1);
        rtl8101_mdio_write(tp, 0x06, 0x801f);
        rtl8101_mdio_write(tp, 0x06, 0x66e2);
        rtl8101_mdio_write(tp, 0x06, 0xe0ea);
        rtl8101_mdio_write(tp, 0x06, 0xe3e0);
        rtl8101_mdio_write(tp, 0x06, 0xeb5a);
        rtl8101_mdio_write(tp, 0x06, 0xf81e);
        rtl8101_mdio_write(tp, 0x06, 0x20e6);
        rtl8101_mdio_write(tp, 0x06, 0xe0ea);
        rtl8101_mdio_write(tp, 0x06, 0xe5e0);
        rtl8101_mdio_write(tp, 0x06, 0xebd3);
        rtl8101_mdio_write(tp, 0x06, 0x05b3);
        rtl8101_mdio_write(tp, 0x06, 0xfee2);
        rtl8101_mdio_write(tp, 0x06, 0xe07c);
        rtl8101_mdio_write(tp, 0x06, 0xe3e0);
        rtl8101_mdio_write(tp, 0x06, 0x7dad);
        rtl8101_mdio_write(tp, 0x06, 0x3703);
        rtl8101_mdio_write(tp, 0x06, 0x7dff);
        rtl8101_mdio_write(tp, 0x06, 0xff0d);
        rtl8101_mdio_write(tp, 0x06, 0x581c);
        rtl8101_mdio_write(tp, 0x06, 0x55f8);
        rtl8101_mdio_write(tp, 0x06, 0xef46);
        rtl8101_mdio_write(tp, 0x06, 0x0282);
        rtl8101_mdio_write(tp, 0x06, 0xc7ef);
        rtl8101_mdio_write(tp, 0x06, 0x65ef);
        rtl8101_mdio_write(tp, 0x06, 0x54fc);
        rtl8101_mdio_write(tp, 0x06, 0xac30);
        rtl8101_mdio_write(tp, 0x06, 0x2b11);
        rtl8101_mdio_write(tp, 0x06, 0xa188);
        rtl8101_mdio_write(tp, 0x06, 0xcabf);
        rtl8101_mdio_write(tp, 0x06, 0x860e);
        rtl8101_mdio_write(tp, 0x06, 0xef10);
        rtl8101_mdio_write(tp, 0x06, 0x0c11);
        rtl8101_mdio_write(tp, 0x06, 0x1a91);
        rtl8101_mdio_write(tp, 0x06, 0xda19);
        rtl8101_mdio_write(tp, 0x06, 0xdbf8);
        rtl8101_mdio_write(tp, 0x06, 0xef46);
        rtl8101_mdio_write(tp, 0x06, 0x021e);
        rtl8101_mdio_write(tp, 0x06, 0x17ef);
        rtl8101_mdio_write(tp, 0x06, 0x54fc);
        rtl8101_mdio_write(tp, 0x06, 0xad30);
        rtl8101_mdio_write(tp, 0x06, 0x0fef);
        rtl8101_mdio_write(tp, 0x06, 0x5689);
        rtl8101_mdio_write(tp, 0x06, 0xde19);
        rtl8101_mdio_write(tp, 0x06, 0xdfe2);
        rtl8101_mdio_write(tp, 0x06, 0x861f);
        rtl8101_mdio_write(tp, 0x06, 0xbf86);
        rtl8101_mdio_write(tp, 0x06, 0x161a);
        rtl8101_mdio_write(tp, 0x06, 0x90de);
        rtl8101_mdio_write(tp, 0x06, 0xfeef);
        rtl8101_mdio_write(tp, 0x06, 0x96fe);
        rtl8101_mdio_write(tp, 0x06, 0xfdfc);
        rtl8101_mdio_write(tp, 0x06, 0x04ac);
        rtl8101_mdio_write(tp, 0x06, 0x2707);
        rtl8101_mdio_write(tp, 0x06, 0xac37);
        rtl8101_mdio_write(tp, 0x06, 0x071a);
        rtl8101_mdio_write(tp, 0x06, 0x54ae);
        rtl8101_mdio_write(tp, 0x06, 0x11ac);
        rtl8101_mdio_write(tp, 0x06, 0x3707);
        rtl8101_mdio_write(tp, 0x06, 0xae00);
        rtl8101_mdio_write(tp, 0x06, 0x1a54);
        rtl8101_mdio_write(tp, 0x06, 0xac37);
        rtl8101_mdio_write(tp, 0x06, 0x07d0);
        rtl8101_mdio_write(tp, 0x06, 0x01d5);
        rtl8101_mdio_write(tp, 0x06, 0xffff);
        rtl8101_mdio_write(tp, 0x06, 0xae02);
        rtl8101_mdio_write(tp, 0x06, 0xd000);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x83ad);
        rtl8101_mdio_write(tp, 0x06, 0x2444);
        rtl8101_mdio_write(tp, 0x06, 0xe0e0);
        rtl8101_mdio_write(tp, 0x06, 0x22e1);
        rtl8101_mdio_write(tp, 0x06, 0xe023);
        rtl8101_mdio_write(tp, 0x06, 0xad22);
        rtl8101_mdio_write(tp, 0x06, 0x3be0);
        rtl8101_mdio_write(tp, 0x06, 0x8abe);
        rtl8101_mdio_write(tp, 0x06, 0xa000);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x28de);
        rtl8101_mdio_write(tp, 0x06, 0xae42);
        rtl8101_mdio_write(tp, 0x06, 0xa001);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x28f1);
        rtl8101_mdio_write(tp, 0x06, 0xae3a);
        rtl8101_mdio_write(tp, 0x06, 0xa002);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x8344);
        rtl8101_mdio_write(tp, 0x06, 0xae32);
        rtl8101_mdio_write(tp, 0x06, 0xa003);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x299a);
        rtl8101_mdio_write(tp, 0x06, 0xae2a);
        rtl8101_mdio_write(tp, 0x06, 0xa004);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x29ae);
        rtl8101_mdio_write(tp, 0x06, 0xae22);
        rtl8101_mdio_write(tp, 0x06, 0xa005);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x29d7);
        rtl8101_mdio_write(tp, 0x06, 0xae1a);
        rtl8101_mdio_write(tp, 0x06, 0xa006);
        rtl8101_mdio_write(tp, 0x06, 0x0502);
        rtl8101_mdio_write(tp, 0x06, 0x29fe);
        rtl8101_mdio_write(tp, 0x06, 0xae12);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xc000);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xc100);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xc600);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xbe00);
        rtl8101_mdio_write(tp, 0x06, 0xae00);
        rtl8101_mdio_write(tp, 0x06, 0xfc04);
        rtl8101_mdio_write(tp, 0x06, 0xf802);
        rtl8101_mdio_write(tp, 0x06, 0x2a67);
        rtl8101_mdio_write(tp, 0x06, 0xe0e0);
        rtl8101_mdio_write(tp, 0x06, 0x22e1);
        rtl8101_mdio_write(tp, 0x06, 0xe023);
        rtl8101_mdio_write(tp, 0x06, 0x0d06);
        rtl8101_mdio_write(tp, 0x06, 0x5803);
        rtl8101_mdio_write(tp, 0x06, 0xa002);
        rtl8101_mdio_write(tp, 0x06, 0x02ae);
        rtl8101_mdio_write(tp, 0x06, 0x2da0);
        rtl8101_mdio_write(tp, 0x06, 0x0102);
        rtl8101_mdio_write(tp, 0x06, 0xae2d);
        rtl8101_mdio_write(tp, 0x06, 0xa000);
        rtl8101_mdio_write(tp, 0x06, 0x4de0);
        rtl8101_mdio_write(tp, 0x06, 0xe200);
        rtl8101_mdio_write(tp, 0x06, 0xe1e2);
        rtl8101_mdio_write(tp, 0x06, 0x01ad);
        rtl8101_mdio_write(tp, 0x06, 0x2444);
        rtl8101_mdio_write(tp, 0x06, 0xe08a);
        rtl8101_mdio_write(tp, 0x06, 0xc2e4);
        rtl8101_mdio_write(tp, 0x06, 0x8ac4);
        rtl8101_mdio_write(tp, 0x06, 0xe08a);
        rtl8101_mdio_write(tp, 0x06, 0xc3e4);
        rtl8101_mdio_write(tp, 0x06, 0x8ac5);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xbe03);
        rtl8101_mdio_write(tp, 0x06, 0xe08b);
        rtl8101_mdio_write(tp, 0x06, 0x83ad);
        rtl8101_mdio_write(tp, 0x06, 0x253a);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xbe05);
        rtl8101_mdio_write(tp, 0x06, 0xae34);
        rtl8101_mdio_write(tp, 0x06, 0xe08a);
        rtl8101_mdio_write(tp, 0x06, 0xceae);
        rtl8101_mdio_write(tp, 0x06, 0x03e0);
        rtl8101_mdio_write(tp, 0x06, 0x8acf);
        rtl8101_mdio_write(tp, 0x06, 0xe18a);
        rtl8101_mdio_write(tp, 0x06, 0xc249);
        rtl8101_mdio_write(tp, 0x06, 0x05e5);
        rtl8101_mdio_write(tp, 0x06, 0x8ac4);
        rtl8101_mdio_write(tp, 0x06, 0xe18a);
        rtl8101_mdio_write(tp, 0x06, 0xc349);
        rtl8101_mdio_write(tp, 0x06, 0x05e5);
        rtl8101_mdio_write(tp, 0x06, 0x8ac5);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xbe05);
        rtl8101_mdio_write(tp, 0x06, 0x022a);
        rtl8101_mdio_write(tp, 0x06, 0xb6ac);
        rtl8101_mdio_write(tp, 0x06, 0x2012);
        rtl8101_mdio_write(tp, 0x06, 0x0283);
        rtl8101_mdio_write(tp, 0x06, 0xbaac);
        rtl8101_mdio_write(tp, 0x06, 0x200c);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xc100);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xc600);
        rtl8101_mdio_write(tp, 0x06, 0xee8a);
        rtl8101_mdio_write(tp, 0x06, 0xbe02);
        rtl8101_mdio_write(tp, 0x06, 0xfc04);
        rtl8101_mdio_write(tp, 0x06, 0xd000);
        rtl8101_mdio_write(tp, 0x06, 0x0283);
        rtl8101_mdio_write(tp, 0x06, 0xcc59);
        rtl8101_mdio_write(tp, 0x06, 0x0f39);
        rtl8101_mdio_write(tp, 0x06, 0x02aa);
        rtl8101_mdio_write(tp, 0x06, 0x04d0);
        rtl8101_mdio_write(tp, 0x06, 0x01ae);
        rtl8101_mdio_write(tp, 0x06, 0x02d0);
        rtl8101_mdio_write(tp, 0x06, 0x0004);
        rtl8101_mdio_write(tp, 0x06, 0xf9fa);
        rtl8101_mdio_write(tp, 0x06, 0xe2e2);
        rtl8101_mdio_write(tp, 0x06, 0xd2e3);
        rtl8101_mdio_write(tp, 0x06, 0xe2d3);
        rtl8101_mdio_write(tp, 0x06, 0xf95a);
        rtl8101_mdio_write(tp, 0x06, 0xf7e6);
        rtl8101_mdio_write(tp, 0x06, 0xe2d2);
        rtl8101_mdio_write(tp, 0x06, 0xe7e2);
        rtl8101_mdio_write(tp, 0x06, 0xd3e2);
        rtl8101_mdio_write(tp, 0x06, 0xe02c);
        rtl8101_mdio_write(tp, 0x06, 0xe3e0);
        rtl8101_mdio_write(tp, 0x06, 0x2df9);
        rtl8101_mdio_write(tp, 0x06, 0x5be0);
        rtl8101_mdio_write(tp, 0x06, 0x1e30);
        rtl8101_mdio_write(tp, 0x06, 0xe6e0);
        rtl8101_mdio_write(tp, 0x06, 0x2ce7);
        rtl8101_mdio_write(tp, 0x06, 0xe02d);
        rtl8101_mdio_write(tp, 0x06, 0xe2e2);
        rtl8101_mdio_write(tp, 0x06, 0xcce3);
        rtl8101_mdio_write(tp, 0x06, 0xe2cd);
        rtl8101_mdio_write(tp, 0x06, 0xf95a);
        rtl8101_mdio_write(tp, 0x06, 0x0f6a);
        rtl8101_mdio_write(tp, 0x06, 0x50e6);
        rtl8101_mdio_write(tp, 0x06, 0xe2cc);
        rtl8101_mdio_write(tp, 0x06, 0xe7e2);
        rtl8101_mdio_write(tp, 0x06, 0xcde0);
        rtl8101_mdio_write(tp, 0x06, 0xe03c);
        rtl8101_mdio_write(tp, 0x06, 0xe1e0);
        rtl8101_mdio_write(tp, 0x06, 0x3def);
        rtl8101_mdio_write(tp, 0x06, 0x64fd);
        rtl8101_mdio_write(tp, 0x06, 0xe0e2);
        rtl8101_mdio_write(tp, 0x06, 0xcce1);
        rtl8101_mdio_write(tp, 0x06, 0xe2cd);
        rtl8101_mdio_write(tp, 0x06, 0x580f);
        rtl8101_mdio_write(tp, 0x06, 0x5af0);
        rtl8101_mdio_write(tp, 0x06, 0x1e02);
        rtl8101_mdio_write(tp, 0x06, 0xe4e2);
        rtl8101_mdio_write(tp, 0x06, 0xcce5);
        rtl8101_mdio_write(tp, 0x06, 0xe2cd);
        rtl8101_mdio_write(tp, 0x06, 0xfde0);
        rtl8101_mdio_write(tp, 0x06, 0xe02c);
        rtl8101_mdio_write(tp, 0x06, 0xe1e0);
        rtl8101_mdio_write(tp, 0x06, 0x2d59);
        rtl8101_mdio_write(tp, 0x06, 0xe05b);
        rtl8101_mdio_write(tp, 0x06, 0x1f1e);
        rtl8101_mdio_write(tp, 0x06, 0x13e4);
        rtl8101_mdio_write(tp, 0x06, 0xe02c);
        rtl8101_mdio_write(tp, 0x06, 0xe5e0);
        rtl8101_mdio_write(tp, 0x06, 0x2dfd);
        rtl8101_mdio_write(tp, 0x06, 0xe0e2);
        rtl8101_mdio_write(tp, 0x06, 0xd2e1);
        rtl8101_mdio_write(tp, 0x06, 0xe2d3);
        rtl8101_mdio_write(tp, 0x06, 0x58f7);
        rtl8101_mdio_write(tp, 0x06, 0x5a08);
        rtl8101_mdio_write(tp, 0x06, 0x1e02);
        rtl8101_mdio_write(tp, 0x06, 0xe4e2);
        rtl8101_mdio_write(tp, 0x06, 0xd2e5);
        rtl8101_mdio_write(tp, 0x06, 0xe2d3);
        rtl8101_mdio_write(tp, 0x06, 0xef46);
        rtl8101_mdio_write(tp, 0x06, 0xfefd);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xf9fa);
        rtl8101_mdio_write(tp, 0x06, 0xef69);
        rtl8101_mdio_write(tp, 0x06, 0xe0e0);
        rtl8101_mdio_write(tp, 0x06, 0x22e1);
        rtl8101_mdio_write(tp, 0x06, 0xe023);
        rtl8101_mdio_write(tp, 0x06, 0x58c4);
        rtl8101_mdio_write(tp, 0x06, 0xe18b);
        rtl8101_mdio_write(tp, 0x06, 0x6e1f);
        rtl8101_mdio_write(tp, 0x06, 0x109e);
        rtl8101_mdio_write(tp, 0x06, 0x58e4);
        rtl8101_mdio_write(tp, 0x06, 0x8b6e);
        rtl8101_mdio_write(tp, 0x06, 0xad22);
        rtl8101_mdio_write(tp, 0x06, 0x22ac);
        rtl8101_mdio_write(tp, 0x06, 0x2755);
        rtl8101_mdio_write(tp, 0x06, 0xac26);
        rtl8101_mdio_write(tp, 0x06, 0x02ae);
        rtl8101_mdio_write(tp, 0x06, 0x1ad1);
        rtl8101_mdio_write(tp, 0x06, 0x06bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bba);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x07bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bbd);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x07bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc0);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1ae);
        rtl8101_mdio_write(tp, 0x06, 0x30d1);
        rtl8101_mdio_write(tp, 0x06, 0x03bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc3);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x00bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc6);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x00bf);
        rtl8101_mdio_write(tp, 0x06, 0x84e9);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x0fbf);
        rtl8101_mdio_write(tp, 0x06, 0x3bba);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x01bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bbd);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x01bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc0);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1ef);
        rtl8101_mdio_write(tp, 0x06, 0x96fe);
        rtl8101_mdio_write(tp, 0x06, 0xfdfc);
        rtl8101_mdio_write(tp, 0x06, 0x04d1);
        rtl8101_mdio_write(tp, 0x06, 0x00bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc3);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d0);
        rtl8101_mdio_write(tp, 0x06, 0x1102);
        rtl8101_mdio_write(tp, 0x06, 0x2bfb);
        rtl8101_mdio_write(tp, 0x06, 0x5903);
        rtl8101_mdio_write(tp, 0x06, 0xef01);
        rtl8101_mdio_write(tp, 0x06, 0xd100);
        rtl8101_mdio_write(tp, 0x06, 0xa000);
        rtl8101_mdio_write(tp, 0x06, 0x02d1);
        rtl8101_mdio_write(tp, 0x06, 0x01bf);
        rtl8101_mdio_write(tp, 0x06, 0x3bc6);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1d1);
        rtl8101_mdio_write(tp, 0x06, 0x11ad);
        rtl8101_mdio_write(tp, 0x06, 0x2002);
        rtl8101_mdio_write(tp, 0x06, 0x0c11);
        rtl8101_mdio_write(tp, 0x06, 0xad21);
        rtl8101_mdio_write(tp, 0x06, 0x020c);
        rtl8101_mdio_write(tp, 0x06, 0x12bf);
        rtl8101_mdio_write(tp, 0x06, 0x84e9);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1ae);
        rtl8101_mdio_write(tp, 0x06, 0xc870);
        rtl8101_mdio_write(tp, 0x06, 0xe426);
        rtl8101_mdio_write(tp, 0x06, 0x0284);
        rtl8101_mdio_write(tp, 0x06, 0xf005);
        rtl8101_mdio_write(tp, 0x06, 0xf8fa);
        rtl8101_mdio_write(tp, 0x06, 0xef69);
        rtl8101_mdio_write(tp, 0x06, 0xe0e2);
        rtl8101_mdio_write(tp, 0x06, 0xfee1);
        rtl8101_mdio_write(tp, 0x06, 0xe2ff);
        rtl8101_mdio_write(tp, 0x06, 0xad2d);
        rtl8101_mdio_write(tp, 0x06, 0x1ae0);
        rtl8101_mdio_write(tp, 0x06, 0xe14e);
        rtl8101_mdio_write(tp, 0x06, 0xe1e1);
        rtl8101_mdio_write(tp, 0x06, 0x4fac);
        rtl8101_mdio_write(tp, 0x06, 0x2d22);
        rtl8101_mdio_write(tp, 0x06, 0xf603);
        rtl8101_mdio_write(tp, 0x06, 0x0203);
        rtl8101_mdio_write(tp, 0x06, 0x3bf7);
        rtl8101_mdio_write(tp, 0x06, 0x03f7);
        rtl8101_mdio_write(tp, 0x06, 0x06bf);
        rtl8101_mdio_write(tp, 0x06, 0x8561);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0x21ae);
        rtl8101_mdio_write(tp, 0x06, 0x11e0);
        rtl8101_mdio_write(tp, 0x06, 0xe14e);
        rtl8101_mdio_write(tp, 0x06, 0xe1e1);
        rtl8101_mdio_write(tp, 0x06, 0x4fad);
        rtl8101_mdio_write(tp, 0x06, 0x2d08);
        rtl8101_mdio_write(tp, 0x06, 0xbf85);
        rtl8101_mdio_write(tp, 0x06, 0x6c02);
        rtl8101_mdio_write(tp, 0x06, 0x2d21);
        rtl8101_mdio_write(tp, 0x06, 0xf606);
        rtl8101_mdio_write(tp, 0x06, 0xef96);
        rtl8101_mdio_write(tp, 0x06, 0xfefc);
        rtl8101_mdio_write(tp, 0x06, 0x04f8);
        rtl8101_mdio_write(tp, 0x06, 0xfaef);
        rtl8101_mdio_write(tp, 0x06, 0x69e0);
        rtl8101_mdio_write(tp, 0x06, 0xe000);
        rtl8101_mdio_write(tp, 0x06, 0xe1e0);
        rtl8101_mdio_write(tp, 0x06, 0x01ad);
        rtl8101_mdio_write(tp, 0x06, 0x271f);
        rtl8101_mdio_write(tp, 0x06, 0xd101);
        rtl8101_mdio_write(tp, 0x06, 0xbf85);
        rtl8101_mdio_write(tp, 0x06, 0x5e02);
        rtl8101_mdio_write(tp, 0x06, 0x2dc1);
        rtl8101_mdio_write(tp, 0x06, 0xe0e0);
        rtl8101_mdio_write(tp, 0x06, 0x20e1);
        rtl8101_mdio_write(tp, 0x06, 0xe021);
        rtl8101_mdio_write(tp, 0x06, 0xad20);
        rtl8101_mdio_write(tp, 0x06, 0x0ed1);
        rtl8101_mdio_write(tp, 0x06, 0x00bf);
        rtl8101_mdio_write(tp, 0x06, 0x855e);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0xc1bf);
        rtl8101_mdio_write(tp, 0x06, 0x3b96);
        rtl8101_mdio_write(tp, 0x06, 0x022d);
        rtl8101_mdio_write(tp, 0x06, 0x21ef);
        rtl8101_mdio_write(tp, 0x06, 0x96fe);
        rtl8101_mdio_write(tp, 0x06, 0xfc04);
        rtl8101_mdio_write(tp, 0x06, 0x00e2);
        rtl8101_mdio_write(tp, 0x06, 0x34a7);
        rtl8101_mdio_write(tp, 0x06, 0x25e5);
        rtl8101_mdio_write(tp, 0x06, 0x0a1d);
        rtl8101_mdio_write(tp, 0x06, 0xe50a);
        rtl8101_mdio_write(tp, 0x06, 0x2ce5);
        rtl8101_mdio_write(tp, 0x06, 0x0a6d);
        rtl8101_mdio_write(tp, 0x06, 0xe50a);
        rtl8101_mdio_write(tp, 0x06, 0x1de5);
        rtl8101_mdio_write(tp, 0x06, 0x0a1c);
        rtl8101_mdio_write(tp, 0x06, 0xe50a);
        rtl8101_mdio_write(tp, 0x06, 0x2da7);
        rtl8101_mdio_write(tp, 0x06, 0x5500);
        rtl8101_mdio_write(tp, 0x05, 0x8b94);
        rtl8101_mdio_write(tp, 0x06, 0x84ec);
        gphy_val = rtl8101_mdio_read(tp, 0x01);
        gphy_val |= BIT_0;
        rtl8101_mdio_write(tp, 0x01, gphy_val);
        rtl8101_mdio_write(tp, 0x00, 0x0005);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, 0x1f, 0x0005);
        for (i = 0; i < 200; i++) {
                udelay(100);
                gphy_val = rtl8101_mdio_read(tp, 0x00);
                if (gphy_val & BIT_7)
                        break;
        }
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0023);
        rtl8101_mdio_write(tp, 0x17, 0x0116);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0028);
        rtl8101_mdio_write(tp, 0x15, 0x0010);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0020);
        rtl8101_mdio_write(tp, 0x15, 0x0100);
        rtl8101_mdio_write(tp, 0x1f, 0x0007);
        rtl8101_mdio_write(tp, 0x1e, 0x0041);
        rtl8101_mdio_write(tp, 0x15, 0x0802);
        rtl8101_mdio_write(tp, 0x16, 0x2185);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
}

static void
rtl8101_set_phy_mcu_8105e_2(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x0310);
        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        mdelay(20);

        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x19, 0x7070);
        rtl8101_mdio_write(tp, 0x1c, 0x0600);
        rtl8101_mdio_write(tp, 0x1d, 0x9700);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6900);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x4899);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x4800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5ffb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x301e);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0xa6fc);
        rtl8101_mdio_write(tp, 0x1d, 0xdcdb);
        rtl8101_mdio_write(tp, 0x1d, 0x0014);
        rtl8101_mdio_write(tp, 0x1d, 0xd9a9);
        rtl8101_mdio_write(tp, 0x1d, 0x0013);
        rtl8101_mdio_write(tp, 0x1d, 0xd16b);
        rtl8101_mdio_write(tp, 0x1d, 0x0011);
        rtl8101_mdio_write(tp, 0x1d, 0xb40e);
        rtl8101_mdio_write(tp, 0x1d, 0xd06b);
        rtl8101_mdio_write(tp, 0x1d, 0x000c);
        rtl8101_mdio_write(tp, 0x1d, 0xb206);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x301a);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x301e);
        rtl8101_mdio_write(tp, 0x1d, 0x314d);
        rtl8101_mdio_write(tp, 0x1d, 0x31f0);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c20);
        rtl8101_mdio_write(tp, 0x1d, 0x6004);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x4833);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c08);
        rtl8101_mdio_write(tp, 0x1d, 0x8300);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6600);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xb90c);
        rtl8101_mdio_write(tp, 0x1d, 0x30d3);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4de0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c60);
        rtl8101_mdio_write(tp, 0x1d, 0x6803);
        rtl8101_mdio_write(tp, 0x1d, 0x6520);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xaf03);
        rtl8101_mdio_write(tp, 0x1d, 0x6015);
        rtl8101_mdio_write(tp, 0x1d, 0x3059);
        rtl8101_mdio_write(tp, 0x1d, 0x6017);
        rtl8101_mdio_write(tp, 0x1d, 0x57e0);
        rtl8101_mdio_write(tp, 0x1d, 0x580c);
        rtl8101_mdio_write(tp, 0x1d, 0x588c);
        rtl8101_mdio_write(tp, 0x1d, 0x7ffc);
        rtl8101_mdio_write(tp, 0x1d, 0x5fa3);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c30);
        rtl8101_mdio_write(tp, 0x1d, 0x6020);
        rtl8101_mdio_write(tp, 0x1d, 0x48bf);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0xad09);
        rtl8101_mdio_write(tp, 0x1d, 0x7c03);
        rtl8101_mdio_write(tp, 0x1d, 0x5c03);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0xad2c);
        rtl8101_mdio_write(tp, 0x1d, 0xd6cf);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0x80f4);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c80);
        rtl8101_mdio_write(tp, 0x1d, 0x7c20);
        rtl8101_mdio_write(tp, 0x1d, 0x5c20);
        rtl8101_mdio_write(tp, 0x1d, 0x481e);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c02);
        rtl8101_mdio_write(tp, 0x1d, 0xad0a);
        rtl8101_mdio_write(tp, 0x1d, 0x7c03);
        rtl8101_mdio_write(tp, 0x1d, 0x5c03);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x8d02);
        rtl8101_mdio_write(tp, 0x1d, 0x4401);
        rtl8101_mdio_write(tp, 0x1d, 0x81f4);
        rtl8101_mdio_write(tp, 0x1d, 0x3114);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d00);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0xa4b7);
        rtl8101_mdio_write(tp, 0x1d, 0xd9b3);
        rtl8101_mdio_write(tp, 0x1d, 0xfffe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d20);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x3045);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d40);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x4401);
        rtl8101_mdio_write(tp, 0x1d, 0x5210);
        rtl8101_mdio_write(tp, 0x1d, 0x4833);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x4c08);
        rtl8101_mdio_write(tp, 0x1d, 0x8300);
        rtl8101_mdio_write(tp, 0x1d, 0x5f80);
        rtl8101_mdio_write(tp, 0x1d, 0x55e0);
        rtl8101_mdio_write(tp, 0x1d, 0xc06f);
        rtl8101_mdio_write(tp, 0x1d, 0x0005);
        rtl8101_mdio_write(tp, 0x1d, 0xd9b3);
        rtl8101_mdio_write(tp, 0x1d, 0xfffd);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x6040);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d60);
        rtl8101_mdio_write(tp, 0x1d, 0x57e0);
        rtl8101_mdio_write(tp, 0x1d, 0x4814);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7c03);
        rtl8101_mdio_write(tp, 0x1d, 0x5c03);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xad02);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0xc0e9);
        rtl8101_mdio_write(tp, 0x1d, 0x0003);
        rtl8101_mdio_write(tp, 0x1d, 0xadd8);
        rtl8101_mdio_write(tp, 0x1d, 0x30c6);
        rtl8101_mdio_write(tp, 0x1d, 0x3078);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4dc0);
        rtl8101_mdio_write(tp, 0x1d, 0x6730);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xd09d);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0xb4fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d80);
        rtl8101_mdio_write(tp, 0x1d, 0x6802);
        rtl8101_mdio_write(tp, 0x1d, 0x6600);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x486c);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x9503);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x30e9);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0xcdab);
        rtl8101_mdio_write(tp, 0x1d, 0xff5b);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0xff59);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0xff57);
        rtl8101_mdio_write(tp, 0x1d, 0xd0a0);
        rtl8101_mdio_write(tp, 0x1d, 0xffdb);
        rtl8101_mdio_write(tp, 0x1d, 0xcba0);
        rtl8101_mdio_write(tp, 0x1d, 0x0003);
        rtl8101_mdio_write(tp, 0x1d, 0x80f0);
        rtl8101_mdio_write(tp, 0x1d, 0x30f6);
        rtl8101_mdio_write(tp, 0x1d, 0x3109);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x7d30);
        rtl8101_mdio_write(tp, 0x1d, 0x6530);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c08);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6008);
        rtl8101_mdio_write(tp, 0x1d, 0x8300);
        rtl8101_mdio_write(tp, 0x1d, 0xb902);
        rtl8101_mdio_write(tp, 0x1d, 0x30d3);
        rtl8101_mdio_write(tp, 0x1d, 0x308f);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4da0);
        rtl8101_mdio_write(tp, 0x1d, 0x57a0);
        rtl8101_mdio_write(tp, 0x1d, 0x590c);
        rtl8101_mdio_write(tp, 0x1d, 0x5fa2);
        rtl8101_mdio_write(tp, 0x1d, 0xcba4);
        rtl8101_mdio_write(tp, 0x1d, 0x0005);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0x0003);
        rtl8101_mdio_write(tp, 0x1d, 0x80fc);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ca0);
        rtl8101_mdio_write(tp, 0x1d, 0xb603);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6010);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x541f);
        rtl8101_mdio_write(tp, 0x1d, 0x7ffc);
        rtl8101_mdio_write(tp, 0x1d, 0x5fb3);
        rtl8101_mdio_write(tp, 0x1d, 0x9403);
        rtl8101_mdio_write(tp, 0x1d, 0x7c03);
        rtl8101_mdio_write(tp, 0x1d, 0x5c03);
        rtl8101_mdio_write(tp, 0x1d, 0xaa05);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x3128);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4cc0);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6400);
        rtl8101_mdio_write(tp, 0x1d, 0x7ffc);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6a00);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x30f6);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x315c);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0x9402);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0xcda3);
        rtl8101_mdio_write(tp, 0x1d, 0x009d);
        rtl8101_mdio_write(tp, 0x1d, 0xcd85);
        rtl8101_mdio_write(tp, 0x1d, 0x009b);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0x0099);
        rtl8101_mdio_write(tp, 0x1d, 0x96e9);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e20);
        rtl8101_mdio_write(tp, 0x1d, 0x96e4);
        rtl8101_mdio_write(tp, 0x1d, 0x8b04);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5008);
        rtl8101_mdio_write(tp, 0x1d, 0xab03);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5000);
        rtl8101_mdio_write(tp, 0x1d, 0x6801);
        rtl8101_mdio_write(tp, 0x1d, 0x6776);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xdb7c);
        rtl8101_mdio_write(tp, 0x1d, 0xfff0);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x4837);
        rtl8101_mdio_write(tp, 0x1d, 0x4418);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8fc9);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a0);
        rtl8101_mdio_write(tp, 0x1d, 0x004a);
        rtl8101_mdio_write(tp, 0x1d, 0x9203);
        rtl8101_mdio_write(tp, 0x1d, 0xa041);
        rtl8101_mdio_write(tp, 0x1d, 0x3184);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x489c);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x7e28);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8fb0);
        rtl8101_mdio_write(tp, 0x1d, 0xb241);
        rtl8101_mdio_write(tp, 0x1d, 0xa02a);
        rtl8101_mdio_write(tp, 0x1d, 0x319d);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ea0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c02);
        rtl8101_mdio_write(tp, 0x1d, 0x4402);
        rtl8101_mdio_write(tp, 0x1d, 0x4448);
        rtl8101_mdio_write(tp, 0x1d, 0x4894);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c03);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x41ef);
        rtl8101_mdio_write(tp, 0x1d, 0x41ff);
        rtl8101_mdio_write(tp, 0x1d, 0x4891);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c17);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x8ef8);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8f95);
        rtl8101_mdio_write(tp, 0x1d, 0x92d5);
        rtl8101_mdio_write(tp, 0x1d, 0xa10f);
        rtl8101_mdio_write(tp, 0x1d, 0xd480);
        rtl8101_mdio_write(tp, 0x1d, 0x0008);
        rtl8101_mdio_write(tp, 0x1d, 0xd580);
        rtl8101_mdio_write(tp, 0x1d, 0xffb9);
        rtl8101_mdio_write(tp, 0x1d, 0xa202);
        rtl8101_mdio_write(tp, 0x1d, 0x31b8);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x4404);
        rtl8101_mdio_write(tp, 0x1d, 0x31b8);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0xfff3);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0xfff1);
        rtl8101_mdio_write(tp, 0x1d, 0x314d);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ee0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x4488);
        rtl8101_mdio_write(tp, 0x1d, 0x41cf);
        rtl8101_mdio_write(tp, 0x1d, 0x314d);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ec0);
        rtl8101_mdio_write(tp, 0x1d, 0x48f3);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c09);
        rtl8101_mdio_write(tp, 0x1d, 0x4508);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8f24);
        rtl8101_mdio_write(tp, 0x1d, 0xd218);
        rtl8101_mdio_write(tp, 0x1d, 0x0022);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a4);
        rtl8101_mdio_write(tp, 0x1d, 0xff9f);
        rtl8101_mdio_write(tp, 0x1d, 0x31d9);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e80);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c11);
        rtl8101_mdio_write(tp, 0x1d, 0x4428);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5440);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0xa4b3);
        rtl8101_mdio_write(tp, 0x1d, 0x31ee);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x31fa);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0xbcf6);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1d, 0x314d);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1c, 0x0200);
        rtl8101_mdio_write(tp, 0x19, 0x7030);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
}

static void
rtl8101_set_phy_mcu_8402_1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x0310);
        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        mdelay(20);

        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x19, 0x7070);
        rtl8101_mdio_write(tp, 0x1c, 0x0600);
        rtl8101_mdio_write(tp, 0x1d, 0x9700);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6900);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x4899);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x4800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5ffb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x301e);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0xa6fc);
        rtl8101_mdio_write(tp, 0x1d, 0xdcdb);
        rtl8101_mdio_write(tp, 0x1d, 0x0015);
        rtl8101_mdio_write(tp, 0x1d, 0xb915);
        rtl8101_mdio_write(tp, 0x1d, 0xb511);
        rtl8101_mdio_write(tp, 0x1d, 0xd16b);
        rtl8101_mdio_write(tp, 0x1d, 0x000f);
        rtl8101_mdio_write(tp, 0x1d, 0xb40f);
        rtl8101_mdio_write(tp, 0x1d, 0xd06b);
        rtl8101_mdio_write(tp, 0x1d, 0x000d);
        rtl8101_mdio_write(tp, 0x1d, 0xb206);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x301a);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x301e);
        rtl8101_mdio_write(tp, 0x1d, 0x3079);
        rtl8101_mdio_write(tp, 0x1d, 0x30f1);
        rtl8101_mdio_write(tp, 0x1d, 0x3199);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c60);
        rtl8101_mdio_write(tp, 0x1d, 0x6803);
        rtl8101_mdio_write(tp, 0x1d, 0x6420);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xaf03);
        rtl8101_mdio_write(tp, 0x1d, 0x6015);
        rtl8101_mdio_write(tp, 0x1d, 0x3040);
        rtl8101_mdio_write(tp, 0x1d, 0x6017);
        rtl8101_mdio_write(tp, 0x1d, 0x57e0);
        rtl8101_mdio_write(tp, 0x1d, 0x580c);
        rtl8101_mdio_write(tp, 0x1d, 0x588c);
        rtl8101_mdio_write(tp, 0x1d, 0x5fa3);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c30);
        rtl8101_mdio_write(tp, 0x1d, 0x6020);
        rtl8101_mdio_write(tp, 0x1d, 0x48bf);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0xd6cf);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0x80fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c80);
        rtl8101_mdio_write(tp, 0x1d, 0x7c20);
        rtl8101_mdio_write(tp, 0x1d, 0x5c20);
        rtl8101_mdio_write(tp, 0x1d, 0x481e);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c02);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x81ff);
        rtl8101_mdio_write(tp, 0x1d, 0x30ba);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d00);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0xa4cc);
        rtl8101_mdio_write(tp, 0x1d, 0xd9b3);
        rtl8101_mdio_write(tp, 0x1d, 0xfffe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d20);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4dc0);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xd09d);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0xb4fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d80);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x6004);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6802);
        rtl8101_mdio_write(tp, 0x1d, 0x6720);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x486c);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x9503);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x3092);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0xcdab);
        rtl8101_mdio_write(tp, 0x1d, 0xff78);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0xff76);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0xff74);
        rtl8101_mdio_write(tp, 0x1d, 0xd0a0);
        rtl8101_mdio_write(tp, 0x1d, 0xffd9);
        rtl8101_mdio_write(tp, 0x1d, 0xcba0);
        rtl8101_mdio_write(tp, 0x1d, 0x0003);
        rtl8101_mdio_write(tp, 0x1d, 0x80f0);
        rtl8101_mdio_write(tp, 0x1d, 0x309f);
        rtl8101_mdio_write(tp, 0x1d, 0x30ac);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c08);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6008);
        rtl8101_mdio_write(tp, 0x1d, 0x8300);
        rtl8101_mdio_write(tp, 0x1d, 0xb902);
        rtl8101_mdio_write(tp, 0x1d, 0x3079);
        rtl8101_mdio_write(tp, 0x1d, 0x3061);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4da0);
        rtl8101_mdio_write(tp, 0x1d, 0x6400);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x57a0);
        rtl8101_mdio_write(tp, 0x1d, 0x590c);
        rtl8101_mdio_write(tp, 0x1d, 0x5fa3);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xcba4);
        rtl8101_mdio_write(tp, 0x1d, 0x0004);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0x80fc);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ca0);
        rtl8101_mdio_write(tp, 0x1d, 0xb603);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6010);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x541f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fb3);
        rtl8101_mdio_write(tp, 0x1d, 0xaa05);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x30ca);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4cc0);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x6720);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6a00);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x309f);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x3100);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0x9403);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0xcda3);
        rtl8101_mdio_write(tp, 0x1d, 0x002d);
        rtl8101_mdio_write(tp, 0x1d, 0xcd85);
        rtl8101_mdio_write(tp, 0x1d, 0x002b);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0x0029);
        rtl8101_mdio_write(tp, 0x1d, 0x9629);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x9624);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e20);
        rtl8101_mdio_write(tp, 0x1d, 0x8b04);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5008);
        rtl8101_mdio_write(tp, 0x1d, 0xab03);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5000);
        rtl8101_mdio_write(tp, 0x1d, 0x6801);
        rtl8101_mdio_write(tp, 0x1d, 0x6776);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xdb7c);
        rtl8101_mdio_write(tp, 0x1d, 0xffee);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x4837);
        rtl8101_mdio_write(tp, 0x1d, 0x4418);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8f07);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a0);
        rtl8101_mdio_write(tp, 0x1d, 0x004c);
        rtl8101_mdio_write(tp, 0x1d, 0x9205);
        rtl8101_mdio_write(tp, 0x1d, 0xa043);
        rtl8101_mdio_write(tp, 0x1d, 0x312b);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1d, 0x30f1);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x489c);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x7e28);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8fec);
        rtl8101_mdio_write(tp, 0x1d, 0xb241);
        rtl8101_mdio_write(tp, 0x1d, 0xa02a);
        rtl8101_mdio_write(tp, 0x1d, 0x3146);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ea0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c02);
        rtl8101_mdio_write(tp, 0x1d, 0x4402);
        rtl8101_mdio_write(tp, 0x1d, 0x4448);
        rtl8101_mdio_write(tp, 0x1d, 0x4894);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c03);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x41ef);
        rtl8101_mdio_write(tp, 0x1d, 0x41ff);
        rtl8101_mdio_write(tp, 0x1d, 0x4891);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c17);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x8ef8);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8fd1);
        rtl8101_mdio_write(tp, 0x1d, 0x92d5);
        rtl8101_mdio_write(tp, 0x1d, 0xa10f);
        rtl8101_mdio_write(tp, 0x1d, 0xd480);
        rtl8101_mdio_write(tp, 0x1d, 0x0008);
        rtl8101_mdio_write(tp, 0x1d, 0xd580);
        rtl8101_mdio_write(tp, 0x1d, 0xffb7);
        rtl8101_mdio_write(tp, 0x1d, 0xa202);
        rtl8101_mdio_write(tp, 0x1d, 0x3161);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x4404);
        rtl8101_mdio_write(tp, 0x1d, 0x3161);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0xfff3);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0xfff1);
        rtl8101_mdio_write(tp, 0x1d, 0x30f1);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ee0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x4488);
        rtl8101_mdio_write(tp, 0x1d, 0x41cf);
        rtl8101_mdio_write(tp, 0x1d, 0x30f1);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ec0);
        rtl8101_mdio_write(tp, 0x1d, 0x48f3);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c09);
        rtl8101_mdio_write(tp, 0x1d, 0x4508);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8fb0);
        rtl8101_mdio_write(tp, 0x1d, 0xd218);
        rtl8101_mdio_write(tp, 0x1d, 0xffae);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a4);
        rtl8101_mdio_write(tp, 0x1d, 0xff9d);
        rtl8101_mdio_write(tp, 0x1d, 0x3182);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e80);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c11);
        rtl8101_mdio_write(tp, 0x1d, 0x4428);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5440);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0xa4b3);
        rtl8101_mdio_write(tp, 0x1d, 0x3197);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4f20);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x6736);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa03);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x31a5);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0xbcf4);
        rtl8101_mdio_write(tp, 0x1d, 0x300b);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1c, 0x0200);
        rtl8101_mdio_write(tp, 0x19, 0x7030);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
}

static void
rtl8101_set_phy_mcu_8106e_2(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mdio_write(tp, 0x1F, 0x0000);
        rtl8101_mdio_write(tp, 0x18, 0x0310);

        mdelay(20);

        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x19, 0x7070);
        rtl8101_mdio_write(tp, 0x1c, 0x0600);
        rtl8101_mdio_write(tp, 0x1d, 0x9700);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x4800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x673e);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5ffb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa04);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x6100);
        rtl8101_mdio_write(tp, 0x1d, 0x3016);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0x6080);
        rtl8101_mdio_write(tp, 0x1d, 0xa6fa);
        rtl8101_mdio_write(tp, 0x1d, 0xdcdb);
        rtl8101_mdio_write(tp, 0x1d, 0x0015);
        rtl8101_mdio_write(tp, 0x1d, 0xb915);
        rtl8101_mdio_write(tp, 0x1d, 0xb511);
        rtl8101_mdio_write(tp, 0x1d, 0xd16b);
        rtl8101_mdio_write(tp, 0x1d, 0x000f);
        rtl8101_mdio_write(tp, 0x1d, 0xb40f);
        rtl8101_mdio_write(tp, 0x1d, 0xd06b);
        rtl8101_mdio_write(tp, 0x1d, 0x000d);
        rtl8101_mdio_write(tp, 0x1d, 0xb206);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x3010);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x3016);
        rtl8101_mdio_write(tp, 0x1d, 0x307e);
        rtl8101_mdio_write(tp, 0x1d, 0x30f4);
        rtl8101_mdio_write(tp, 0x1d, 0x319f);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c60);
        rtl8101_mdio_write(tp, 0x1d, 0x6803);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6900);
        rtl8101_mdio_write(tp, 0x1d, 0x6520);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xaf03);
        rtl8101_mdio_write(tp, 0x1d, 0x6115);
        rtl8101_mdio_write(tp, 0x1d, 0x303a);
        rtl8101_mdio_write(tp, 0x1d, 0x6097);
        rtl8101_mdio_write(tp, 0x1d, 0x57e0);
        rtl8101_mdio_write(tp, 0x1d, 0x580c);
        rtl8101_mdio_write(tp, 0x1d, 0x588c);
        rtl8101_mdio_write(tp, 0x1d, 0x5f80);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c30);
        rtl8101_mdio_write(tp, 0x1d, 0x6020);
        rtl8101_mdio_write(tp, 0x1d, 0x48bf);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0xb802);
        rtl8101_mdio_write(tp, 0x1d, 0x3053);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6808);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6810);
        rtl8101_mdio_write(tp, 0x1d, 0xd6cf);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0x80fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4c80);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7c23);
        rtl8101_mdio_write(tp, 0x1d, 0x5c23);
        rtl8101_mdio_write(tp, 0x1d, 0x481e);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c02);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x81ff);
        rtl8101_mdio_write(tp, 0x1d, 0x30c1);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d00);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0xa4bd);
        rtl8101_mdio_write(tp, 0x1d, 0xd9b3);
        rtl8101_mdio_write(tp, 0x1d, 0x00fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d20);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x3001);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4dc0);
        rtl8101_mdio_write(tp, 0x1d, 0xd09d);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0xb4fe);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4d80);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x6004);
        rtl8101_mdio_write(tp, 0x1d, 0x6802);
        rtl8101_mdio_write(tp, 0x1d, 0x6728);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x486c);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x9503);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0x571f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0xaa05);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x7d80);
        rtl8101_mdio_write(tp, 0x1d, 0x6100);
        rtl8101_mdio_write(tp, 0x1d, 0x309a);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0x7d80);
        rtl8101_mdio_write(tp, 0x1d, 0x6080);
        rtl8101_mdio_write(tp, 0x1d, 0xcdab);
        rtl8101_mdio_write(tp, 0x1d, 0x0058);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0x0056);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0x0054);
        rtl8101_mdio_write(tp, 0x1d, 0xd0a0);
        rtl8101_mdio_write(tp, 0x1d, 0x00d8);
        rtl8101_mdio_write(tp, 0x1d, 0xcba0);
        rtl8101_mdio_write(tp, 0x1d, 0x0003);
        rtl8101_mdio_write(tp, 0x1d, 0x80ec);
        rtl8101_mdio_write(tp, 0x1d, 0x30a7);
        rtl8101_mdio_write(tp, 0x1d, 0x30b4);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c08);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6008);
        rtl8101_mdio_write(tp, 0x1d, 0x8300);
        rtl8101_mdio_write(tp, 0x1d, 0xb902);
        rtl8101_mdio_write(tp, 0x1d, 0x307e);
        rtl8101_mdio_write(tp, 0x1d, 0x3068);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4da0);
        rtl8101_mdio_write(tp, 0x1d, 0x6628);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x56a0);
        rtl8101_mdio_write(tp, 0x1d, 0x590c);
        rtl8101_mdio_write(tp, 0x1d, 0x5fa0);
        rtl8101_mdio_write(tp, 0x1d, 0xcba4);
        rtl8101_mdio_write(tp, 0x1d, 0x0004);
        rtl8101_mdio_write(tp, 0x1d, 0xcd8d);
        rtl8101_mdio_write(tp, 0x1d, 0x0002);
        rtl8101_mdio_write(tp, 0x1d, 0x80fc);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ca0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x6408);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0xb603);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6010);
        rtl8101_mdio_write(tp, 0x1d, 0x7d1f);
        rtl8101_mdio_write(tp, 0x1d, 0x551f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fb3);
        rtl8101_mdio_write(tp, 0x1d, 0xaa05);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b58);
        rtl8101_mdio_write(tp, 0x1d, 0x30d7);
        rtl8101_mdio_write(tp, 0x1d, 0x7c80);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x5b64);
        rtl8101_mdio_write(tp, 0x1d, 0x4827);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c10);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c10);
        rtl8101_mdio_write(tp, 0x1d, 0x6000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4cc0);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6400);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x5fbb);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c00);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c04);
        rtl8101_mdio_write(tp, 0x1d, 0x8200);
        rtl8101_mdio_write(tp, 0x1d, 0x7ce0);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7d00);
        rtl8101_mdio_write(tp, 0x1d, 0x6500);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x30a7);
        rtl8101_mdio_write(tp, 0x1d, 0x3001);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e00);
        rtl8101_mdio_write(tp, 0x1d, 0x4007);
        rtl8101_mdio_write(tp, 0x1d, 0x4400);
        rtl8101_mdio_write(tp, 0x1d, 0x5310);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x673e);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa05);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x7d80);
        rtl8101_mdio_write(tp, 0x1d, 0x6100);
        rtl8101_mdio_write(tp, 0x1d, 0x3107);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0x7d80);
        rtl8101_mdio_write(tp, 0x1d, 0x6080);
        rtl8101_mdio_write(tp, 0x1d, 0x9403);
        rtl8101_mdio_write(tp, 0x1d, 0x7e00);
        rtl8101_mdio_write(tp, 0x1d, 0x6200);
        rtl8101_mdio_write(tp, 0x1d, 0xcda3);
        rtl8101_mdio_write(tp, 0x1d, 0x00e8);
        rtl8101_mdio_write(tp, 0x1d, 0xcd85);
        rtl8101_mdio_write(tp, 0x1d, 0x00e6);
        rtl8101_mdio_write(tp, 0x1d, 0xd96b);
        rtl8101_mdio_write(tp, 0x1d, 0x00e4);
        rtl8101_mdio_write(tp, 0x1d, 0x96e4);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x673e);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e20);
        rtl8101_mdio_write(tp, 0x1d, 0x96dd);
        rtl8101_mdio_write(tp, 0x1d, 0x8b04);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5008);
        rtl8101_mdio_write(tp, 0x1d, 0xab03);
        rtl8101_mdio_write(tp, 0x1d, 0x7c08);
        rtl8101_mdio_write(tp, 0x1d, 0x5000);
        rtl8101_mdio_write(tp, 0x1d, 0x6801);
        rtl8101_mdio_write(tp, 0x1d, 0x677e);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0xdb7c);
        rtl8101_mdio_write(tp, 0x1d, 0x00ee);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x4837);
        rtl8101_mdio_write(tp, 0x1d, 0x4418);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e40);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8fc2);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a0);
        rtl8101_mdio_write(tp, 0x1d, 0x004b);
        rtl8101_mdio_write(tp, 0x1d, 0x9204);
        rtl8101_mdio_write(tp, 0x1d, 0xa042);
        rtl8101_mdio_write(tp, 0x1d, 0x3132);
        rtl8101_mdio_write(tp, 0x1d, 0x30f4);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe1);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x489c);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e60);
        rtl8101_mdio_write(tp, 0x1d, 0x7e28);
        rtl8101_mdio_write(tp, 0x1d, 0x4628);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5800);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c00);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x8fa8);
        rtl8101_mdio_write(tp, 0x1d, 0xb241);
        rtl8101_mdio_write(tp, 0x1d, 0xa02a);
        rtl8101_mdio_write(tp, 0x1d, 0x314c);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ea0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c02);
        rtl8101_mdio_write(tp, 0x1d, 0x4402);
        rtl8101_mdio_write(tp, 0x1d, 0x4448);
        rtl8101_mdio_write(tp, 0x1d, 0x4894);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c03);
        rtl8101_mdio_write(tp, 0x1d, 0x4824);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x41ef);
        rtl8101_mdio_write(tp, 0x1d, 0x41ff);
        rtl8101_mdio_write(tp, 0x1d, 0x4891);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c07);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c17);
        rtl8101_mdio_write(tp, 0x1d, 0x8400);
        rtl8101_mdio_write(tp, 0x1d, 0x8ef8);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8f8d);
        rtl8101_mdio_write(tp, 0x1d, 0x92d5);
        rtl8101_mdio_write(tp, 0x1d, 0xa10f);
        rtl8101_mdio_write(tp, 0x1d, 0xd480);
        rtl8101_mdio_write(tp, 0x1d, 0x0008);
        rtl8101_mdio_write(tp, 0x1d, 0xd580);
        rtl8101_mdio_write(tp, 0x1d, 0x00b8);
        rtl8101_mdio_write(tp, 0x1d, 0xa202);
        rtl8101_mdio_write(tp, 0x1d, 0x3167);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x4404);
        rtl8101_mdio_write(tp, 0x1d, 0x3167);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0x00f3);
        rtl8101_mdio_write(tp, 0x1d, 0xd484);
        rtl8101_mdio_write(tp, 0x1d, 0x00f1);
        rtl8101_mdio_write(tp, 0x1d, 0x30f4);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ee0);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5400);
        rtl8101_mdio_write(tp, 0x1d, 0x4488);
        rtl8101_mdio_write(tp, 0x1d, 0x41cf);
        rtl8101_mdio_write(tp, 0x1d, 0x30f4);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4ec0);
        rtl8101_mdio_write(tp, 0x1d, 0x48f3);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c09);
        rtl8101_mdio_write(tp, 0x1d, 0x4508);
        rtl8101_mdio_write(tp, 0x1d, 0x41c7);
        rtl8101_mdio_write(tp, 0x1d, 0x8fb0);
        rtl8101_mdio_write(tp, 0x1d, 0xd218);
        rtl8101_mdio_write(tp, 0x1d, 0x00ae);
        rtl8101_mdio_write(tp, 0x1d, 0xd2a4);
        rtl8101_mdio_write(tp, 0x1d, 0x009e);
        rtl8101_mdio_write(tp, 0x1d, 0x3188);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4e80);
        rtl8101_mdio_write(tp, 0x1d, 0x4832);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c01);
        rtl8101_mdio_write(tp, 0x1d, 0x7c1f);
        rtl8101_mdio_write(tp, 0x1d, 0x4c11);
        rtl8101_mdio_write(tp, 0x1d, 0x4428);
        rtl8101_mdio_write(tp, 0x1d, 0x7c40);
        rtl8101_mdio_write(tp, 0x1d, 0x5440);
        rtl8101_mdio_write(tp, 0x1d, 0x7c01);
        rtl8101_mdio_write(tp, 0x1d, 0x5801);
        rtl8101_mdio_write(tp, 0x1d, 0x7c04);
        rtl8101_mdio_write(tp, 0x1d, 0x5c04);
        rtl8101_mdio_write(tp, 0x1d, 0x41e8);
        rtl8101_mdio_write(tp, 0x1d, 0xa4b3);
        rtl8101_mdio_write(tp, 0x1d, 0x319d);
        rtl8101_mdio_write(tp, 0x1d, 0x7fe0);
        rtl8101_mdio_write(tp, 0x1d, 0x4f20);
        rtl8101_mdio_write(tp, 0x1d, 0x6800);
        rtl8101_mdio_write(tp, 0x1d, 0x673e);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x0000);
        rtl8101_mdio_write(tp, 0x1d, 0x570f);
        rtl8101_mdio_write(tp, 0x1d, 0x5fff);
        rtl8101_mdio_write(tp, 0x1d, 0xaa04);
        rtl8101_mdio_write(tp, 0x1d, 0x585b);
        rtl8101_mdio_write(tp, 0x1d, 0x6100);
        rtl8101_mdio_write(tp, 0x1d, 0x31ad);
        rtl8101_mdio_write(tp, 0x1d, 0x5867);
        rtl8101_mdio_write(tp, 0x1d, 0x6080);
        rtl8101_mdio_write(tp, 0x1d, 0xbcf2);
        rtl8101_mdio_write(tp, 0x1d, 0x3001);
        rtl8101_mdio_write(tp, 0x1f, 0x0004);
        rtl8101_mdio_write(tp, 0x1c, 0x0200);
        rtl8101_mdio_write(tp, 0x19, 0x7030);
        rtl8101_mdio_write(tp, 0x1f, 0x0000);
}

static void
rtl8101_set_phy_mcu_8106eus_1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int gphy_val;

        rtl8101_set_phy_mcu_patch_request(tp);
        rtl8101_mdio_write(tp, 0x1f, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8146);
        rtl8101_mdio_write(tp, 0x14, 0x0300);
        rtl8101_mdio_write(tp, 0x13, 0xB82E);
        rtl8101_mdio_write(tp, 0x14, 0x0001);
        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0xb820);
        rtl8101_mdio_write(tp, 0x14, 0x0290);
        rtl8101_mdio_write(tp, 0x13, 0xa012);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xa014);
        rtl8101_mdio_write(tp, 0x14, 0x2c04);
        rtl8101_mdio_write(tp, 0x14, 0x2c07);
        rtl8101_mdio_write(tp, 0x14, 0x2c07);
        rtl8101_mdio_write(tp, 0x14, 0x2c07);
        rtl8101_mdio_write(tp, 0x14, 0xa304);
        rtl8101_mdio_write(tp, 0x14, 0xa301);
        rtl8101_mdio_write(tp, 0x14, 0x207e);
        rtl8101_mdio_write(tp, 0x13, 0xa01a);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xa006);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xa004);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xa002);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xa000);
        rtl8101_mdio_write(tp, 0x14, 0x107c);
        rtl8101_mdio_write(tp, 0x13, 0xb820);
        rtl8101_mdio_write(tp, 0x14, 0x0210);
        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x0000);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x1f, 0x0B82);
        gphy_val = rtl8101_mdio_read(tp, 0x17);
        gphy_val &= ~(BIT_0);
        rtl8101_mdio_write(tp, 0x17, gphy_val);
        rtl8101_mdio_write(tp, 0x1f, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8146);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_clear_phy_mcu_patch_request(tp);
}

static void
rtl8101_set_phy_mcu_8107e_1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int gphy_val;

        rtl8101_set_phy_mcu_patch_request(tp);

        rtl8101_mdio_write(tp, 0x1f, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8028);
        rtl8101_mdio_write(tp, 0x14, 0x6200);
        rtl8101_mdio_write(tp, 0x13, 0xB82E);
        rtl8101_mdio_write(tp, 0x14, 0x0001);


        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0xB820);
        rtl8101_mdio_write(tp, 0x14, 0x0290);
        rtl8101_mdio_write(tp, 0x13, 0xA012);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xA014);
        rtl8101_mdio_write(tp, 0x14, 0x2c04);
        rtl8101_mdio_write(tp, 0x14, 0x2c10);
        rtl8101_mdio_write(tp, 0x14, 0x2c10);
        rtl8101_mdio_write(tp, 0x14, 0x2c10);
        rtl8101_mdio_write(tp, 0x14, 0xa210);
        rtl8101_mdio_write(tp, 0x14, 0xa101);
        rtl8101_mdio_write(tp, 0x14, 0xce10);
        rtl8101_mdio_write(tp, 0x14, 0xe070);
        rtl8101_mdio_write(tp, 0x14, 0x0f40);
        rtl8101_mdio_write(tp, 0x14, 0xaf01);
        rtl8101_mdio_write(tp, 0x14, 0x8f01);
        rtl8101_mdio_write(tp, 0x14, 0x183e);
        rtl8101_mdio_write(tp, 0x14, 0x8e10);
        rtl8101_mdio_write(tp, 0x14, 0x8101);
        rtl8101_mdio_write(tp, 0x14, 0x8210);
        rtl8101_mdio_write(tp, 0x14, 0x28da);
        rtl8101_mdio_write(tp, 0x13, 0xA01A);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xA006);
        rtl8101_mdio_write(tp, 0x14, 0x0017);
        rtl8101_mdio_write(tp, 0x13, 0xA004);
        rtl8101_mdio_write(tp, 0x14, 0x0015);
        rtl8101_mdio_write(tp, 0x13, 0xA002);
        rtl8101_mdio_write(tp, 0x14, 0x0013);
        rtl8101_mdio_write(tp, 0x13, 0xA000);
        rtl8101_mdio_write(tp, 0x14, 0x18d1);
        rtl8101_mdio_write(tp, 0x13, 0xB820);
        rtl8101_mdio_write(tp, 0x14, 0x0210);


        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x0000);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x1f, 0x0B82);
        gphy_val = rtl8101_mdio_read(tp,  0x17);
        gphy_val &= ~(BIT_0);
        rtl8101_mdio_write(tp, 0x17, gphy_val);
        rtl8101_mdio_write(tp, 0x1f, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8028);
        rtl8101_mdio_write(tp, 0x14, 0x0000);

        rtl8101_clear_phy_mcu_patch_request(tp);
}

static void
rtl8101_set_phy_mcu_8107e_2(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int gphy_val;

        rtl8101_set_phy_mcu_patch_request(tp);

        rtl8101_mdio_write(tp, 0x1f, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8028);
        rtl8101_mdio_write(tp, 0x14, 0x6201);
        rtl8101_mdio_write(tp, 0x13, 0xB82E);
        rtl8101_mdio_write(tp, 0x14, 0x0001);


        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0xB820);
        rtl8101_mdio_write(tp, 0x14, 0x0290);
        rtl8101_mdio_write(tp, 0x13, 0xA012);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xA014);
        rtl8101_mdio_write(tp, 0x14, 0x2c04);
        rtl8101_mdio_write(tp, 0x14, 0x2c09);
        rtl8101_mdio_write(tp, 0x14, 0x2c09);
        rtl8101_mdio_write(tp, 0x14, 0x2c09);
        rtl8101_mdio_write(tp, 0x14, 0xad01);
        rtl8101_mdio_write(tp, 0x14, 0xad01);
        rtl8101_mdio_write(tp, 0x14, 0xad01);
        rtl8101_mdio_write(tp, 0x14, 0xad01);
        rtl8101_mdio_write(tp, 0x14, 0x236c);
        rtl8101_mdio_write(tp, 0x13, 0xA01A);
        rtl8101_mdio_write(tp, 0x14, 0x0000);
        rtl8101_mdio_write(tp, 0x13, 0xA006);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xA004);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xA002);
        rtl8101_mdio_write(tp, 0x14, 0x0fff);
        rtl8101_mdio_write(tp, 0x13, 0xA000);
        rtl8101_mdio_write(tp, 0x14, 0x136b);
        rtl8101_mdio_write(tp, 0x13, 0xB820);
        rtl8101_mdio_write(tp, 0x14, 0x0210);


        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
        rtl8101_mdio_write(tp, 0x13, 0x8323);
        rtl8101_mdio_write(tp, 0x14, 0xaf83);
        rtl8101_mdio_write(tp, 0x14, 0x2faf);
        rtl8101_mdio_write(tp, 0x14, 0x853d);
        rtl8101_mdio_write(tp, 0x14, 0xaf85);
        rtl8101_mdio_write(tp, 0x14, 0x3daf);
        rtl8101_mdio_write(tp, 0x14, 0x853d);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x45ad);
        rtl8101_mdio_write(tp, 0x14, 0x2052);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7ae3);
        rtl8101_mdio_write(tp, 0x14, 0x85fe);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f6);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7a1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fa);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7be3);
        rtl8101_mdio_write(tp, 0x14, 0x85fe);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f7);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7b1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fb);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7ce3);
        rtl8101_mdio_write(tp, 0x14, 0x85fe);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f8);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7c1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fc);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7de3);
        rtl8101_mdio_write(tp, 0x14, 0x85fe);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f9);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7d1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fd);
        rtl8101_mdio_write(tp, 0x14, 0xae50);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7ee3);
        rtl8101_mdio_write(tp, 0x14, 0x85ff);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f6);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7e1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fa);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7fe3);
        rtl8101_mdio_write(tp, 0x14, 0x85ff);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f7);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x7f1b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fb);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x80e3);
        rtl8101_mdio_write(tp, 0x14, 0x85ff);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f8);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x801b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fc);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x81e3);
        rtl8101_mdio_write(tp, 0x14, 0x85ff);
        rtl8101_mdio_write(tp, 0x14, 0x1a03);
        rtl8101_mdio_write(tp, 0x14, 0x10e4);
        rtl8101_mdio_write(tp, 0x14, 0x85f9);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x811b);
        rtl8101_mdio_write(tp, 0x14, 0x03e4);
        rtl8101_mdio_write(tp, 0x14, 0x85fd);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf6ad);
        rtl8101_mdio_write(tp, 0x14, 0x2404);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xf610);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf7ad);
        rtl8101_mdio_write(tp, 0x14, 0x2404);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xf710);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf8ad);
        rtl8101_mdio_write(tp, 0x14, 0x2404);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xf810);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf9ad);
        rtl8101_mdio_write(tp, 0x14, 0x2404);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xf910);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfaad);
        rtl8101_mdio_write(tp, 0x14, 0x2704);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xfa00);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfbad);
        rtl8101_mdio_write(tp, 0x14, 0x2704);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xfb00);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfcad);
        rtl8101_mdio_write(tp, 0x14, 0x2704);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xfc00);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfdad);
        rtl8101_mdio_write(tp, 0x14, 0x2704);
        rtl8101_mdio_write(tp, 0x14, 0xee85);
        rtl8101_mdio_write(tp, 0x14, 0xfd00);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x44ad);
        rtl8101_mdio_write(tp, 0x14, 0x203f);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf6e4);
        rtl8101_mdio_write(tp, 0x14, 0x8288);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfae4);
        rtl8101_mdio_write(tp, 0x14, 0x8289);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x440d);
        rtl8101_mdio_write(tp, 0x14, 0x0458);
        rtl8101_mdio_write(tp, 0x14, 0x01bf);
        rtl8101_mdio_write(tp, 0x14, 0x8264);
        rtl8101_mdio_write(tp, 0x14, 0x0215);
        rtl8101_mdio_write(tp, 0x14, 0x38bf);
        rtl8101_mdio_write(tp, 0x14, 0x824e);
        rtl8101_mdio_write(tp, 0x14, 0x0213);
        rtl8101_mdio_write(tp, 0x14, 0x06a0);
        rtl8101_mdio_write(tp, 0x14, 0x010f);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x44f6);
        rtl8101_mdio_write(tp, 0x14, 0x20e4);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x580f);
        rtl8101_mdio_write(tp, 0x14, 0xe582);
        rtl8101_mdio_write(tp, 0x14, 0x5aae);
        rtl8101_mdio_write(tp, 0x14, 0x0ebf);
        rtl8101_mdio_write(tp, 0x14, 0x825e);
        rtl8101_mdio_write(tp, 0x14, 0xe382);
        rtl8101_mdio_write(tp, 0x14, 0x44f7);
        rtl8101_mdio_write(tp, 0x14, 0x3ce7);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x0212);
        rtl8101_mdio_write(tp, 0x14, 0xf0ad);
        rtl8101_mdio_write(tp, 0x14, 0x213f);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf7e4);
        rtl8101_mdio_write(tp, 0x14, 0x8288);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfbe4);
        rtl8101_mdio_write(tp, 0x14, 0x8289);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x440d);
        rtl8101_mdio_write(tp, 0x14, 0x0558);
        rtl8101_mdio_write(tp, 0x14, 0x01bf);
        rtl8101_mdio_write(tp, 0x14, 0x826b);
        rtl8101_mdio_write(tp, 0x14, 0x0215);
        rtl8101_mdio_write(tp, 0x14, 0x38bf);
        rtl8101_mdio_write(tp, 0x14, 0x824f);
        rtl8101_mdio_write(tp, 0x14, 0x0213);
        rtl8101_mdio_write(tp, 0x14, 0x06a0);
        rtl8101_mdio_write(tp, 0x14, 0x010f);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x44f6);
        rtl8101_mdio_write(tp, 0x14, 0x21e4);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x580f);
        rtl8101_mdio_write(tp, 0x14, 0xe582);
        rtl8101_mdio_write(tp, 0x14, 0x5bae);
        rtl8101_mdio_write(tp, 0x14, 0x0ebf);
        rtl8101_mdio_write(tp, 0x14, 0x8265);
        rtl8101_mdio_write(tp, 0x14, 0xe382);
        rtl8101_mdio_write(tp, 0x14, 0x44f7);
        rtl8101_mdio_write(tp, 0x14, 0x3de7);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x0212);
        rtl8101_mdio_write(tp, 0x14, 0xf0ad);
        rtl8101_mdio_write(tp, 0x14, 0x223f);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf8e4);
        rtl8101_mdio_write(tp, 0x14, 0x8288);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfce4);
        rtl8101_mdio_write(tp, 0x14, 0x8289);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x440d);
        rtl8101_mdio_write(tp, 0x14, 0x0658);
        rtl8101_mdio_write(tp, 0x14, 0x01bf);
        rtl8101_mdio_write(tp, 0x14, 0x8272);
        rtl8101_mdio_write(tp, 0x14, 0x0215);
        rtl8101_mdio_write(tp, 0x14, 0x38bf);
        rtl8101_mdio_write(tp, 0x14, 0x8250);
        rtl8101_mdio_write(tp, 0x14, 0x0213);
        rtl8101_mdio_write(tp, 0x14, 0x06a0);
        rtl8101_mdio_write(tp, 0x14, 0x010f);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x44f6);
        rtl8101_mdio_write(tp, 0x14, 0x22e4);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x580f);
        rtl8101_mdio_write(tp, 0x14, 0xe582);
        rtl8101_mdio_write(tp, 0x14, 0x5cae);
        rtl8101_mdio_write(tp, 0x14, 0x0ebf);
        rtl8101_mdio_write(tp, 0x14, 0x826c);
        rtl8101_mdio_write(tp, 0x14, 0xe382);
        rtl8101_mdio_write(tp, 0x14, 0x44f7);
        rtl8101_mdio_write(tp, 0x14, 0x3ee7);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x0212);
        rtl8101_mdio_write(tp, 0x14, 0xf0ad);
        rtl8101_mdio_write(tp, 0x14, 0x233f);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xf9e4);
        rtl8101_mdio_write(tp, 0x14, 0x8288);
        rtl8101_mdio_write(tp, 0x14, 0xe085);
        rtl8101_mdio_write(tp, 0x14, 0xfde4);
        rtl8101_mdio_write(tp, 0x14, 0x8289);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x440d);
        rtl8101_mdio_write(tp, 0x14, 0x0758);
        rtl8101_mdio_write(tp, 0x14, 0x01bf);
        rtl8101_mdio_write(tp, 0x14, 0x8279);
        rtl8101_mdio_write(tp, 0x14, 0x0215);
        rtl8101_mdio_write(tp, 0x14, 0x38bf);
        rtl8101_mdio_write(tp, 0x14, 0x8251);
        rtl8101_mdio_write(tp, 0x14, 0x0213);
        rtl8101_mdio_write(tp, 0x14, 0x06a0);
        rtl8101_mdio_write(tp, 0x14, 0x010f);
        rtl8101_mdio_write(tp, 0x14, 0xe082);
        rtl8101_mdio_write(tp, 0x14, 0x44f6);
        rtl8101_mdio_write(tp, 0x14, 0x23e4);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x580f);
        rtl8101_mdio_write(tp, 0x14, 0xe582);
        rtl8101_mdio_write(tp, 0x14, 0x5dae);
        rtl8101_mdio_write(tp, 0x14, 0x0ebf);
        rtl8101_mdio_write(tp, 0x14, 0x8273);
        rtl8101_mdio_write(tp, 0x14, 0xe382);
        rtl8101_mdio_write(tp, 0x14, 0x44f7);
        rtl8101_mdio_write(tp, 0x14, 0x3fe7);
        rtl8101_mdio_write(tp, 0x14, 0x8244);
        rtl8101_mdio_write(tp, 0x14, 0x0212);
        rtl8101_mdio_write(tp, 0x14, 0xf0ee);
        rtl8101_mdio_write(tp, 0x14, 0x8288);
        rtl8101_mdio_write(tp, 0x14, 0x10ee);
        rtl8101_mdio_write(tp, 0x14, 0x8289);
        rtl8101_mdio_write(tp, 0x14, 0x00af);
        rtl8101_mdio_write(tp, 0x14, 0x14aa);
        rtl8101_mdio_write(tp, 0x13, 0xb818);
        rtl8101_mdio_write(tp, 0x14, 0x13cf);
        rtl8101_mdio_write(tp, 0x13, 0xb81a);
        rtl8101_mdio_write(tp, 0x14, 0xfffd);
        rtl8101_mdio_write(tp, 0x13, 0xb81c);
        rtl8101_mdio_write(tp, 0x14, 0xfffd);
        rtl8101_mdio_write(tp, 0x13, 0xb81e);
        rtl8101_mdio_write(tp, 0x14, 0xfffd);
        rtl8101_mdio_write(tp, 0x13, 0xb832);
        rtl8101_mdio_write(tp, 0x14, 0x0001);


        rtl8101_mdio_write(tp,0x1F, 0x0A43);
        rtl8101_mdio_write(tp,0x13, 0x0000);
        rtl8101_mdio_write(tp,0x14, 0x0000);
        rtl8101_mdio_write(tp,0x1f, 0x0B82);
        gphy_val = rtl8101_mdio_read(tp, 0x17);
        gphy_val &= ~(BIT_0);
        rtl8101_mdio_write(tp,0x17, gphy_val);
        rtl8101_mdio_write(tp,0x1f, 0x0A43);
        rtl8101_mdio_write(tp,0x13, 0x8028);
        rtl8101_mdio_write(tp,0x14, 0x0000);

        rtl8101_clear_phy_mcu_patch_request(tp);

        if (tp->RequiredSecLanDonglePatch) {
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                gphy_val = rtl8101_mdio_read(tp, 0x11);
                gphy_val &= ~BIT_6;
                rtl8101_mdio_write(tp, 0x11, gphy_val);
        }
}

static void
rtl8101_init_hw_phy_mcu(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        if (tp->NotWrRamCodeToMicroP == TRUE) return;
        if(rtl8101_check_hw_phy_mcu_code_ver(dev)) return;

        switch (tp->mcfg) {
        case CFG_METHOD_10:
                rtl8101_set_phy_mcu_8105e_1(dev);
                break;
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
                rtl8101_set_phy_mcu_8105e_2(dev);
                break;
        case CFG_METHOD_14:
                rtl8101_set_phy_mcu_8402_1(dev);
                break;
        case CFG_METHOD_16:
                rtl8101_set_phy_mcu_8106e_2(dev);
                break;
        case CFG_METHOD_17:
                rtl8101_set_phy_mcu_8106eus_1(dev);
                break;
        case CFG_METHOD_18:
                rtl8101_set_phy_mcu_8107e_1(dev);
                break;
        case CFG_METHOD_19:
                rtl8101_set_phy_mcu_8107e_2(dev);
                break;
        case CFG_METHOD_20:
                break;
        }

        rtl8101_write_hw_phy_mcu_code_ver(dev);

        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        tp->HwHasWrRamCodeToMicroP = TRUE;
}

static void
rtl8101_hw_phy_config(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u16	gphy_val;

        tp->phy_reset_enable(dev);

        rtl8101_init_hw_phy_mcu(dev);

        if (tp->mcfg == CFG_METHOD_4) {
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | 0x1000);
                rtl8101_mdio_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) | 0x2000);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | 0x8000);

                rtl8101_mdio_write(tp, 0x1f, 0x0003);
                rtl8101_mdio_write(tp, 0x08, 0x441D);
                rtl8101_mdio_write(tp, 0x01, 0x9100);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
        } else if (tp->mcfg == CFG_METHOD_5) {
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | 0x1000);
                rtl8101_mdio_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) | 0x2000);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | 0x8000);

                rtl8101_mdio_write(tp, 0x1f, 0x0003);
                rtl8101_mdio_write(tp, 0x08, 0x441D);
                rtl8101_mdio_write(tp, 0x01, 0x9100);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
        } else if (tp->mcfg == CFG_METHOD_6) {
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | 0x1000);
                rtl8101_mdio_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) | 0x2000);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | 0x8000);

                rtl8101_mdio_write(tp, 0x1f, 0x0003);
                rtl8101_mdio_write(tp, 0x08, 0x441D);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
        } else if (tp->mcfg == CFG_METHOD_7 || tp->mcfg == CFG_METHOD_8) {
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | 0x1000);
                rtl8101_mdio_write(tp, 0x19, rtl8101_mdio_read(tp, 0x19) | 0x2000);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | 0x8000);
        } else if (tp->mcfg == CFG_METHOD_9) {
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | BIT_12);
                rtl8101_mdio_write(tp, 0x1F, 0x0002);
                rtl8101_mdio_write(tp, 0x0F, rtl8101_mdio_read(tp, 0x0F) | BIT_0 | BIT_1);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                rtl8101_phyio_write(tp, 0x0E, 0x0068);
                rtl8101_phyio_write(tp, 0x0E, 0x0069);
                rtl8101_phyio_write(tp, 0x0E, 0x006A);
                rtl8101_phyio_write(tp, 0x0E, 0x006B);
                rtl8101_phyio_write(tp, 0x0E, 0x006C);
        } else if (tp->mcfg == CFG_METHOD_10) {
                rtl8101_mdio_write(tp, 0x1F, 0x0007);
                rtl8101_mdio_write(tp, 0x1E, 0x0023);
                gphy_val = rtl8101_mdio_read(tp, 0x17) | BIT_1;
                if (tp->RequiredSecLanDonglePatch)
                        gphy_val &= ~(BIT_2);
                else
                        gphy_val |= (BIT_2);
                rtl8101_mdio_write(tp, 0x17, gphy_val);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1f, 0x0005);
                rtl8101_mdio_write(tp, 0x05, 0x8b80);
                rtl8101_mdio_write(tp, 0x06, 0xc896);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0001);
                rtl8101_mdio_write(tp, 0x0B, 0x8C60);
                rtl8101_mdio_write(tp, 0x07, 0x2872);
                rtl8101_mdio_write(tp, 0x1C, 0xEFFF);
                rtl8101_mdio_write(tp, 0x1F, 0x0003);
                rtl8101_mdio_write(tp, 0x14, 0x94B0);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0002);
                gphy_val = rtl8101_mdio_read(tp, 0x08) & 0x00FF;
                rtl8101_mdio_write(tp, 0x08, gphy_val | 0x8000);

                rtl8101_mdio_write(tp, 0x1F, 0x0007);
                rtl8101_mdio_write(tp, 0x1E, 0x002D);
                gphy_val = rtl8101_mdio_read(tp, 0x18);
                rtl8101_mdio_write(tp, 0x18, gphy_val | 0x0010);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                gphy_val = rtl8101_mdio_read(tp, 0x14);
                rtl8101_mdio_write(tp, 0x14, gphy_val | 0x8000);

                rtl8101_mdio_write(tp, 0x1F, 0x0002);
                rtl8101_mdio_write(tp, 0x00, 0x080B);
                rtl8101_mdio_write(tp, 0x0B, 0x09D7);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, 0x15, 0x1006);

                rtl8101_mdio_write(tp, 0x1F, 0x0003);
                rtl8101_mdio_write(tp, 0x19, 0x7F46);
                rtl8101_mdio_write(tp, 0x1F, 0x0005);
                rtl8101_mdio_write(tp, 0x05, 0x8AD2);
                rtl8101_mdio_write(tp, 0x06, 0x6810);
                rtl8101_mdio_write(tp, 0x05, 0x8AD4);
                rtl8101_mdio_write(tp, 0x06, 0x8002);
                rtl8101_mdio_write(tp, 0x05, 0x8ADE);
                rtl8101_mdio_write(tp, 0x06, 0x8025);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
        } else if (tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
                   tp->mcfg == CFG_METHOD_13) {
                if (RTL_R8(tp, 0xEF) & 0x08) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0005);
                        rtl8101_mdio_write(tp, 0x1A, 0x0004);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                } else {
                        rtl8101_mdio_write(tp, 0x1F, 0x0005);
                        rtl8101_mdio_write(tp, 0x1A, 0x0000);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }

                if (RTL_R8(tp, 0xEF) & 0x010) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0004);
                        rtl8101_mdio_write(tp, 0x1C, 0x0000);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                } else {
                        rtl8101_mdio_write(tp, 0x1F, 0x0004);
                        rtl8101_mdio_write(tp, 0x1C, 0x0200);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }

                rtl8101_mdio_write(tp, 0x1F, 0x0001);
                rtl8101_mdio_write(tp, 0x15, 0x7701);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                gphy_val = rtl8101_mdio_read(tp, 0x1A);
                rtl8101_mdio_write(tp, 0x1A, gphy_val & ~BIT_14);

                if(aspm) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        rtl8101_mdio_write(tp, 0x18, 0x8310);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
        } else if (tp->mcfg == CFG_METHOD_14) {
                if(aspm) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        rtl8101_mdio_write(tp, 0x18, 0x8310);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
        } else if (tp->mcfg == CFG_METHOD_15 || tp->mcfg == CFG_METHOD_16) {
                rtl8101_mdio_write(tp, 0x1F, 0x0001);
                rtl8101_mdio_write(tp, 0x11, 0x83BA);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0005);
                gphy_val = rtl8101_mdio_read(tp, 0x1A);
                rtl8101_mdio_write(tp, 0x1A, gphy_val & ~BIT_2);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if(aspm) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        rtl8101_mdio_write(tp, 0x18, 0x8310);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
        } else if (tp->mcfg == CFG_METHOD_17) {
                rtl8101_mdio_write(tp, 0x1F, 0x0BCC);
                rtl8101_mdio_write(tp, 0x14, rtl8101_mdio_read(tp, 0x14) & ~BIT_8);
                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | BIT_7);
                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | BIT_6);
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x8084);
                rtl8101_mdio_write(tp, 0x14, rtl8101_mdio_read(tp, 0x14) & ~(BIT_14 | BIT_13));
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | BIT_12);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | BIT_1);
                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | BIT_0);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x8012);
                rtl8101_mdio_write(tp, 0x14, rtl8101_mdio_read(tp, 0x14) | BIT_15);

                rtl8101_mdio_write(tp, 0x1F, 0x0BCE);
                rtl8101_mdio_write(tp, 0x12, 0x8860);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x80F3);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x8B00);
                rtl8101_mdio_write(tp, 0x13, 0x80F0);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x3A00);
                rtl8101_mdio_write(tp, 0x13, 0x80EF);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x0500);
                rtl8101_mdio_write(tp, 0x13, 0x80F6);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x6E00);
                rtl8101_mdio_write(tp, 0x13, 0x80EC);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x6800);
                rtl8101_mdio_write(tp, 0x13, 0x80ED);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x7C00);
                rtl8101_mdio_write(tp, 0x13, 0x80F2);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0xF400);
                rtl8101_mdio_write(tp, 0x13, 0x80F4);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x8500);
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x8110);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0xA800);
                rtl8101_mdio_write(tp, 0x13, 0x810F);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x1D00);
                rtl8101_mdio_write(tp, 0x13, 0x8111);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0xF500);
                rtl8101_mdio_write(tp, 0x13, 0x8113);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x6100);
                rtl8101_mdio_write(tp, 0x13, 0x8115);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x9200);
                rtl8101_mdio_write(tp, 0x13, 0x810E);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x0400);
                rtl8101_mdio_write(tp, 0x13, 0x810C);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x7C00);
                rtl8101_mdio_write(tp, 0x13, 0x810B);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x5A00);
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x80D1);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0xFF00);
                rtl8101_mdio_write(tp, 0x13, 0x80CD);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x9E00);
                rtl8101_mdio_write(tp, 0x13, 0x80D3);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x0E00);
                rtl8101_mdio_write(tp, 0x13, 0x80D5);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0xCA00);
                rtl8101_mdio_write(tp, 0x13, 0x80D7);
                rtl8101_mdio_write(tp, 0x14, (rtl8101_mdio_read(tp, 0x14) & ~0xFF00) | 0x8400);

                if (aspm) {
                        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                                rtl8101_mdio_write(tp, 0x10, rtl8101_mdio_read(tp, 0x10) | BIT_2);
                        }
                }
        } else if (tp->mcfg == CFG_METHOD_18) {
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x809b);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xF800 ,
                                      0x8000
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x80A2);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x8000
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x80A4);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x8500
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x809C);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0xbd00
                                    );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x80AD);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xF800 ,
                                      0x7000
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x80B4);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x5000
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x80AC);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x4000
                                    );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x808E);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x1200
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x8090);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0xE500
                                    );
                rtl8101_mdio_write(tp, 0x13, 0x8092);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      0xFF00 ,
                                      0x9F00
                                    );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        u16 dout_tapbin;

                        dout_tapbin = 0x0000;
                        rtl8101_mdio_write( tp, 0x1F, 0x0A46 );
                        gphy_val = rtl8101_mdio_read( tp, 0x13 );
                        gphy_val &= (BIT_1|BIT_0);
                        gphy_val <<= 2;
                        dout_tapbin |= gphy_val;

                        gphy_val = rtl8101_mdio_read( tp, 0x12 );
                        gphy_val &= (BIT_15|BIT_14);
                        gphy_val >>= 14;
                        dout_tapbin |= gphy_val;

                        dout_tapbin = ~( dout_tapbin^BIT_3 );
                        dout_tapbin <<= 12;
                        dout_tapbin &= 0xF000;

                        rtl8101_mdio_write( tp, 0x1F, 0x0A43 );

                        rtl8101_mdio_write( tp, 0x13, 0x827A );
                        ClearAndSetEthPhyBit( tp,
                                              0x14,
                                              BIT_15|BIT_14|BIT_13|BIT_12,
                                              dout_tapbin
                                            );


                        rtl8101_mdio_write( tp, 0x13, 0x827B );
                        ClearAndSetEthPhyBit( tp,
                                              0x14,
                                              BIT_15|BIT_14|BIT_13|BIT_12,
                                              dout_tapbin
                                            );


                        rtl8101_mdio_write( tp, 0x13, 0x827C );
                        ClearAndSetEthPhyBit( tp,
                                              0x14,
                                              BIT_15|BIT_14|BIT_13|BIT_12,
                                              dout_tapbin
                                            );


                        rtl8101_mdio_write( tp, 0x13, 0x827D );
                        ClearAndSetEthPhyBit( tp,
                                              0x14,
                                              BIT_15|BIT_14|BIT_13|BIT_12,
                                              dout_tapbin
                                            );

                        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                        rtl8101_mdio_write(tp, 0x13, 0x8011);
                        rtl8101_set_eth_phy_bit(tp, 0x14, BIT_11);
                        rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                        rtl8101_set_eth_phy_bit(tp, 0x16, BIT_1);
                }

                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                rtl8101_set_eth_phy_bit( tp, 0x11, BIT_11 );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);


                rtl8101_mdio_write(tp, 0x1F, 0x0BCA);
                ClearAndSetEthPhyBit( tp,
                                      0x17,
                                      (BIT_13 | BIT_12) ,
                                      BIT_14
                                    );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x803F);
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x8047);
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x804F);
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x8057);
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x805F);
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x8067 );
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x13, 0x806F );
                rtl8101_clear_eth_phy_bit( tp, 0x14, (BIT_13 | BIT_12));
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if (aspm) {
                        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                                rtl8101_set_eth_phy_bit( tp, 0x10, BIT_2 );
                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        }
                }
        } else if (tp->mcfg == CFG_METHOD_19) {
                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                rtl8101_mdio_write(tp, 0x13, 0x808A);
                ClearAndSetEthPhyBit( tp,
                                      0x14,
                                      BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0,
                                      0x0A );

                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                        rtl8101_mdio_write(tp, 0x13, 0x8011);
                        rtl8101_set_eth_phy_bit(tp, 0x14, BIT_11);
                        rtl8101_mdio_write(tp, 0x1F, 0x0A42);
                        rtl8101_set_eth_phy_bit(tp, 0x16, BIT_1);
                }

                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                rtl8101_set_eth_phy_bit( tp, 0x11, BIT_11 );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if(tp->RequireAdcBiasPatch) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0BCF);
                        rtl8101_mdio_write(tp, 0x16, tp->AdcBiasPatchIoffset);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }

                {
                        u16 rlen;

                        rtl8101_mdio_write(tp, 0x1F, 0x0BCD);
                        gphy_val = rtl8101_mdio_read( tp, 0x16 );
                        gphy_val &= 0x000F;

                        if( gphy_val > 3 ) {
                                rlen = gphy_val - 3;
                        } else {
                                rlen = 0;
                        }

                        gphy_val = rlen | (rlen<<4) | (rlen<<8) | (rlen<<12);

                        rtl8101_mdio_write(tp, 0x1F, 0x0BCD);
                        rtl8101_mdio_write(tp, 0x17, gphy_val);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }

                if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                        rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                        rtl8101_mdio_write(tp, 0x13, 0x85FE);
                        ClearAndSetEthPhyBit(
                                tp,
                                0x14,
                                BIT_15|BIT_14|BIT_13|BIT_12|BIT_11|BIT_10|BIT_8,
                                BIT_9);
                        rtl8101_mdio_write(tp, 0x13, 0x85FF);
                        ClearAndSetEthPhyBit(
                                tp,
                                0x14,
                                BIT_15|BIT_14|BIT_13|BIT_12,
                                BIT_11|BIT_10|BIT_9|BIT_8);
                        rtl8101_mdio_write(tp, 0x13, 0x814B);
                        ClearAndSetEthPhyBit(
                                tp,
                                0x14,
                                BIT_15|BIT_14|BIT_13|BIT_11|BIT_10|BIT_9|BIT_8,
                                BIT_12);
                }

                if (aspm) {
                        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                                rtl8101_set_eth_phy_bit( tp, 0x10, BIT_2 );
                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        }
                }
        } else if (tp->mcfg == CFG_METHOD_20) {
                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                rtl8101_set_eth_phy_bit( tp, 0x11, BIT_11 );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if (aspm) {
                        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                                rtl8101_mdio_write(tp, 0x1F, 0x0A43);
                                rtl8101_set_eth_phy_bit( tp, 0x10, BIT_2 );
                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        }
                }
        }

        //EthPhyPPSW
        if (tp->mcfg == CFG_METHOD_17) {
                //disable EthPhyPPSW
                rtl8101_mdio_write(tp, 0x1F, 0x0BCD);
                rtl8101_mdio_write(tp, 0x14, 0x5065);
                rtl8101_mdio_write(tp, 0x14, 0xD065);
                rtl8101_mdio_write(tp, 0x1F, 0x0BC8);
                rtl8101_mdio_write(tp, 0x11, 0x5655);
                rtl8101_mdio_write(tp, 0x1F, 0x0BCD);
                rtl8101_mdio_write(tp, 0x14, 0x1065);
                rtl8101_mdio_write(tp, 0x14, 0x9065);
                rtl8101_mdio_write(tp, 0x14, 0x1065);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
        } else if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                   tp->mcfg == CFG_METHOD_20) {
                //enable EthPhyPPSW
                rtl8101_mdio_write(tp, 0x1F, 0x0A44);
                rtl8101_set_eth_phy_bit( tp, 0x11, BIT_7 );
                rtl8101_mdio_write(tp, 0x1F, 0x0000);
        }

        /*ocp phy power saving*/
        if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
            tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                if (aspm)
                        rtl8101_enable_ocp_phy_power_saving(dev);
        }

        rtl8101_mdio_write(tp, 0x1F, 0x0000);

        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
                if (tp->eee_enabled == 1)
                        rtl8101_enable_eee(tp);
                else
                        rtl8101_disable_eee(tp);
        }
}

static inline void rtl8101_delete_link_timer(struct net_device *dev, struct timer_list *timer)
{
        del_timer_sync(timer);
}

static inline void rtl8101_request_link_timer(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->link_timer;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        setup_timer(timer, rtl8101_link_timer, (unsigned long)dev);
#else
        timer_setup(timer, rtl8101_link_timer, 0);
#endif
        mod_timer(timer, jiffies + RTL8101_LINK_TIMEOUT);
}

static inline void rtl8101_delete_esd_timer(struct net_device *dev, struct timer_list *timer)
{
        del_timer_sync(timer);
}

static inline void rtl8101_request_esd_timer(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->esd_timer;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        setup_timer(timer, rtl8101_esd_timer, (unsigned long)dev);
#else
        timer_setup(timer, rtl8101_esd_timer, 0);
#endif
        mod_timer(timer, jiffies + RTL8101_ESD_TIMEOUT);
}


#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void
rtl8101_netpoll(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        disable_irq(tp->irq);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
        rtl8101_interrupt(tp->irq, dev, NULL);
#else
        rtl8101_interrupt(tp->irq, dev);
#endif
        enable_irq(tp->irq);
}
#endif

static void
rtl8101_get_bios_setting(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        tp->bios_setting = 0;
        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                tp->bios_setting = RTL_R32(tp, 0x8c);
                break;
        }
}

static void
rtl8101_set_bios_setting(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W32(tp, 0x8C, tp->bios_setting);
                break;
        }
}

static void
rtl8101_init_software_variable(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;

        rtl8101_get_bios_setting(dev);

        tp->num_rx_desc = NUM_RX_DESC;
        tp->num_tx_desc = NUM_TX_DESC;

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                tp->HwSuppNowIsOobVer = 1;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
                tp->intr_mask = RxDescUnavail | TxDescUnavail | TxOK | RxOK | SWInt;
                break;
        default:
                tp->intr_mask = RxDescUnavail | TxOK | RxOK | SWInt;
                break;
        }

        if (aspm) {
                switch (tp->mcfg) {
                case CFG_METHOD_15:
                case CFG_METHOD_16:
                case CFG_METHOD_17:
                case CFG_METHOD_18:
                case CFG_METHOD_19:
                case CFG_METHOD_20:
                        tp->org_pci_offset_99 = rtl8101_csi_fun0_read_byte(tp, 0x99);
                        tp->org_pci_offset_99 &= ~(BIT_5|BIT_6);
                        break;
                }
                switch (tp->mcfg) {
                case CFG_METHOD_17:
                case CFG_METHOD_18:
                case CFG_METHOD_19:
                case CFG_METHOD_20:
                        tp->org_pci_offset_180 = rtl8101_csi_fun0_read_byte(tp, 0x180);
                        break;
                }

                switch (tp->mcfg) {
                case CFG_METHOD_15:
                case CFG_METHOD_16:
                case CFG_METHOD_17:
                        if (tp->org_pci_offset_99 & BIT_2)
                                tp->issue_offset_99_event = TRUE;
                        break;
                }
        }

        pci_read_config_byte(pdev, 0x80, &tp->org_pci_offset_80);
        pci_read_config_byte(pdev, 0x81, &tp->org_pci_offset_81);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                if ((tp->features & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX)) &&
                    (tp->org_pci_offset_80 & BIT_1))
                        tp->use_timer_interrrupt = FALSE;
                else
                        tp->use_timer_interrrupt = TRUE;
                break;
        default:
                tp->use_timer_interrrupt = TRUE;
                break;
        }

        if (timer_count == 0 || tp->mcfg == CFG_METHOD_DEFAULT)
                tp->use_timer_interrrupt = FALSE;

        switch (tp->mcfg) {
        case CFG_METHOD_19: {
                u16 ioffset_p3, ioffset_p2, ioffset_p1, ioffset_p0;
                u16 TmpUshort;

                rtl8101_mac_ocp_write( tp, 0xDD02, 0x807D);
                TmpUshort = rtl8101_mac_ocp_read( tp, 0xDD02 );
                ioffset_p3 = ( (TmpUshort & BIT_7) >>7 );
                ioffset_p3 <<= 3;
                TmpUshort = rtl8101_mac_ocp_read( tp, 0xDD00 );

                ioffset_p3 |= ((TmpUshort & (BIT_15 | BIT_14 | BIT_13))>>13);

                ioffset_p2 = ((TmpUshort & (BIT_12|BIT_11|BIT_10|BIT_9))>>9);
                ioffset_p1 = ((TmpUshort & (BIT_8|BIT_7|BIT_6|BIT_5))>>5);

                ioffset_p0 = ( (TmpUshort & BIT_4) >>4 );
                ioffset_p0 <<= 3;
                ioffset_p0 |= (TmpUshort & (BIT_2| BIT_1 | BIT_0));

                if((ioffset_p3 == 0x0F) && (ioffset_p2 == 0x0F) && (ioffset_p1 == 0x0F) && (ioffset_p0 == 0x0F)) {
                        tp->RequireAdcBiasPatch = FALSE;
                } else {
                        tp->RequireAdcBiasPatch = TRUE;
                        tp->AdcBiasPatchIoffset = (ioffset_p3<<12)|(ioffset_p2<<8)|(ioffset_p1<<4)|(ioffset_p0);
                }
        }
        break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19: {
                u16 rg_saw_cnt;

                rtl8101_mdio_write(tp, 0x1F, 0x0C42);
                rg_saw_cnt = rtl8101_mdio_read(tp, 0x13);
                rg_saw_cnt &= ~(BIT_15|BIT_14);
                rtl8101_mdio_write(tp, 0x1F, 0x0000);

                if ( rg_saw_cnt > 0) {
                        tp->SwrCnt1msIni = 16000000/rg_saw_cnt;
                        tp->SwrCnt1msIni &= 0x0FFF;

                        tp->RequireAdjustUpsTxLinkPulseTiming = TRUE;
                }
        }
        break;
        }
        switch (tp->mcfg) {
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
                tp->RequireResetNctlBfrPhyResetOrNway = TRUE;
                break;
        }

        if (pdev->subsystem_vendor == 0x144d) {
                if (pdev->subsystem_device == 0xc098 ||
                    pdev->subsystem_device == 0xc0b1 ||
                    pdev->subsystem_device == 0xc0b8)
                        hwoptimize |= HW_PATCH_SAMSUNG_LAN_DONGLE;
        }

#ifdef CONFIG_CTAP_SHORT_OFF
        hwoptimize |= HW_PATCH_SAMSUNG_LAN_DONGLE;
#endif //CONFIG_CTAP_SHORT_OFF

        if (hwoptimize & HW_PATCH_SAMSUNG_LAN_DONGLE) {
                switch (tp->mcfg) {
                case CFG_METHOD_10:
                case CFG_METHOD_19:
                        tp->RequiredSecLanDonglePatch = TRUE;
                        break;
                }
        }

        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        case CFG_METHOD_9:
                tp->RequireResetPhyToChgSpd = TRUE;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_14:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_V2;
                break;
        case CFG_METHOD_DEFAULT:
                tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_NOT_SUPPORT;
                break;
        default:
                tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_V1;
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_17;
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_18;
                break;
        case CFG_METHOD_20:
                tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_20;
                break;
        }

        if (tp->HwIcVerUnknown) {
                tp->NotWrRamCodeToMicroP = TRUE;
                tp->NotWrMcuPatchCode = TRUE;
        }

        rtl8101_get_hw_wol(dev);

        rtl8101_link_option((u8*)&autoneg_mode, (u32*)&speed_mode, (u8*)&duplex_mode, (u32*)&advertising_mode);

        tp->autoneg = autoneg_mode;
        tp->speed = speed_mode;
        tp->duplex = duplex_mode;
        tp->advertising = advertising_mode;
        tp->fcpause = rtl8101_fc_full;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
        /* MTU range: 60 - hw-specific max */
        dev->min_mtu = ETH_MIN_MTU;
        dev->max_mtu = ETH_DATA_LEN;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)

        if (rtl8101_support_eee(tp)) {
                tp->eee_enabled = eee_enable;
                tp->eee_adv_t = MDIO_EEE_100TX;
        }
}

static void
rtl8101_release_board(struct pci_dev *pdev,
                      struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        void __iomem *ioaddr = tp->mmio_addr;

        rtl8101_set_bios_setting(dev);
        rtl8101_rar_set(tp, tp->org_mac_addr);
        tp->wol_enabled = WOL_DISABLED;

        rtl8101_phy_power_down(dev);

        iounmap(ioaddr);
        pci_release_regions(pdev);
        pci_clear_mwi(pdev);
        pci_disable_device(pdev);
        free_netdev(dev);
}

static void
rtl8101_hw_address_set(struct net_device *dev, u8 mac_addr[MAC_ADDR_LEN])
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
        eth_hw_addr_set(dev, mac_addr);
#else
        memcpy(dev->dev_addr, mac_addr, MAC_ADDR_LEN);
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
}

static int
rtl8101_get_mac_address(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int i;
        u8 mac_addr[MAC_ADDR_LEN];

        for (i = 0; i < MAC_ADDR_LEN; i++)
                mac_addr[i] = RTL_R8(tp, MAC0 + i);

        if (tp->mcfg == CFG_METHOD_14 || tp->mcfg == CFG_METHOD_17 ||
            tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
            tp->mcfg == CFG_METHOD_20) {
                *(u32*)&mac_addr[0] = rtl8101_eri_read(tp, 0xE0, 4, ERIAR_ExGMAC);
                *(u16*)&mac_addr[4] = rtl8101_eri_read(tp, 0xE4, 2, ERIAR_ExGMAC);
        } else {
                if (tp->eeprom_type != EEPROM_TYPE_NONE) {
                        u16 *pUshort = (u16*)mac_addr;
                        /* Get MAC address from EEPROM */
                        *pUshort++ = rtl8101_eeprom_read_sc(tp, 7);
                        *pUshort++ = rtl8101_eeprom_read_sc(tp, 8);
                        *pUshort = rtl8101_eeprom_read_sc(tp, 9);
                }
        }

        if (!is_valid_ether_addr(mac_addr)) {
                netif_err(tp, probe, dev, "Invalid ether addr %pM\n",
                          mac_addr);
                eth_random_addr(mac_addr);
                dev->addr_assign_type = NET_ADDR_RANDOM;
                netif_info(tp, probe, dev, "Random ether addr %pM\n",
                           mac_addr);
                tp->random_mac = 1;
        }

        rtl8101_hw_address_set(dev, mac_addr);
        rtl8101_rar_set(tp, mac_addr);

        /* keep the original MAC address */
        memcpy(tp->org_mac_addr, dev->dev_addr, MAC_ADDR_LEN);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        memcpy(dev->perm_addr, dev->dev_addr, MAC_ADDR_LEN);
#endif
        return 0;
}

/**
 * rtl8101_set_mac_address - Change the Ethernet Address of the NIC
 * @dev: network interface device structure
 * @p:   pointer to an address structure
 *
 * Return 0 on success, negative on failure
 **/
static int
rtl8101_set_mac_address(struct net_device *dev,
                        void *p)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct sockaddr *addr = p;
        unsigned long flags;

        if (!is_valid_ether_addr(addr->sa_data))
                return -EADDRNOTAVAIL;

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_hw_address_set(dev, addr->sa_data);

        rtl8101_rar_set(tp, dev->dev_addr);

        spin_unlock_irqrestore(&tp->lock, flags);

        return 0;
}

/******************************************************************************
 * rtl8101_rar_set - Puts an ethernet address into a receive address register.
 *
 * tp - The private data structure for driver
 * addr - Address to put into receive address register
 *****************************************************************************/
void
rtl8101_rar_set(struct rtl8101_private *tp,
                const u8 *addr)
{
        uint32_t rar_low = 0;
        uint32_t rar_high = 0;

        rar_low = ((uint32_t) addr[0] |
                   ((uint32_t) addr[1] << 8) |
                   ((uint32_t) addr[2] << 16) |
                   ((uint32_t) addr[3] << 24));

        rar_high = ((uint32_t) addr[4] |
                    ((uint32_t) addr[5] << 8));

        rtl8101_enable_cfg9346_write(tp);
        RTL_W32(tp, MAC0, rar_low);
        RTL_W32(tp, MAC4, rar_high);

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
                RTL_W32(tp, SecMAC0, rar_low);
                RTL_W16(tp, SecMAC4, (uint16_t)rar_high);
                break;
        }

        rtl8101_disable_cfg9346_write(tp);
}

#ifdef ETHTOOL_OPS_COMPAT
static int ethtool_get_settings(struct net_device *dev, void *useraddr)
{
        struct ethtool_cmd cmd = { ETHTOOL_GSET };
        int err;

        if (!ethtool_ops->get_settings)
                return -EOPNOTSUPP;

        err = ethtool_ops->get_settings(dev, &cmd);
        if (err < 0)
                return err;

        if (copy_to_user(useraddr, &cmd, sizeof(cmd)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_settings(struct net_device *dev, void *useraddr)
{
        struct ethtool_cmd cmd;

        if (!ethtool_ops->set_settings)
                return -EOPNOTSUPP;

        if (copy_from_user(&cmd, useraddr, sizeof(cmd)))
                return -EFAULT;

        return ethtool_ops->set_settings(dev, &cmd);
}

static int ethtool_get_drvinfo(struct net_device *dev, void *useraddr)
{
        struct ethtool_drvinfo info;
        struct ethtool_ops *ops = ethtool_ops;

        if (!ops->get_drvinfo)
                return -EOPNOTSUPP;

        memset(&info, 0, sizeof(info));
        info.cmd = ETHTOOL_GDRVINFO;
        ops->get_drvinfo(dev, &info);

        if (ops->self_test_count)
                info.testinfo_len = ops->self_test_count(dev);
        if (ops->get_stats_count)
                info.n_stats = ops->get_stats_count(dev);
        if (ops->get_regs_len)
                info.regdump_len = ops->get_regs_len(dev);
        if (ops->get_eeprom_len)
                info.eedump_len = ops->get_eeprom_len(dev);

        if (copy_to_user(useraddr, &info, sizeof(info)))
                return -EFAULT;
        return 0;
}

static int ethtool_get_regs(struct net_device *dev, char *useraddr)
{
        struct ethtool_regs regs;
        struct ethtool_ops *ops = ethtool_ops;
        void *regbuf;
        int reglen, ret;

        if (!ops->get_regs || !ops->get_regs_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&regs, useraddr, sizeof(regs)))
                return -EFAULT;

        reglen = ops->get_regs_len(dev);
        if (regs.len > reglen)
                regs.len = reglen;

        regbuf = kmalloc(reglen, GFP_USER);
        if (!regbuf)
                return -ENOMEM;

        ops->get_regs(dev, &regs, regbuf);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &regs, sizeof(regs)))
                goto out;
        useraddr += offsetof(struct ethtool_regs, data);
        if (copy_to_user(useraddr, regbuf, reglen))
                goto out;
        ret = 0;

out:
        kfree(regbuf);
        return ret;
}

static int ethtool_get_wol(struct net_device *dev, char *useraddr)
{
        struct ethtool_wolinfo wol = { ETHTOOL_GWOL };

        if (!ethtool_ops->get_wol)
                return -EOPNOTSUPP;

        ethtool_ops->get_wol(dev, &wol);

        if (copy_to_user(useraddr, &wol, sizeof(wol)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_wol(struct net_device *dev, char *useraddr)
{
        struct ethtool_wolinfo wol;

        if (!ethtool_ops->set_wol)
                return -EOPNOTSUPP;

        if (copy_from_user(&wol, useraddr, sizeof(wol)))
                return -EFAULT;

        return ethtool_ops->set_wol(dev, &wol);
}

static int ethtool_get_msglevel(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GMSGLVL };

        if (!ethtool_ops->get_msglevel)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_msglevel(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_msglevel(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_msglevel)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        ethtool_ops->set_msglevel(dev, edata.data);
        return 0;
}

static int ethtool_nway_reset(struct net_device *dev)
{
        if (!ethtool_ops->nway_reset)
                return -EOPNOTSUPP;

        return ethtool_ops->nway_reset(dev);
}

static int ethtool_get_link(struct net_device *dev, void *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GLINK };

        if (!ethtool_ops->get_link)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_link(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_get_eeprom(struct net_device *dev, void *useraddr)
{
        struct ethtool_eeprom eeprom;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->get_eeprom || !ops->get_eeprom_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
                return -EFAULT;

        /* Check for wrap and zero */
        if (eeprom.offset + eeprom.len <= eeprom.offset)
                return -EINVAL;

        /* Check for exceeding total eeprom len */
        if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
                return -EINVAL;

        data = kmalloc(eeprom.len, GFP_USER);
        if (!data)
                return -ENOMEM;

        ret = -EFAULT;
        if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
                goto out;

        ret = ops->get_eeprom(dev, &eeprom, data);
        if (ret)
                goto out;

        ret = -EFAULT;
        if (copy_to_user(useraddr, &eeprom, sizeof(eeprom)))
                goto out;
        if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_set_eeprom(struct net_device *dev, void *useraddr)
{
        struct ethtool_eeprom eeprom;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->set_eeprom || !ops->get_eeprom_len)
                return -EOPNOTSUPP;

        if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
                return -EFAULT;

        /* Check for wrap and zero */
        if (eeprom.offset + eeprom.len <= eeprom.offset)
                return -EINVAL;

        /* Check for exceeding total eeprom len */
        if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
                return -EINVAL;

        data = kmalloc(eeprom.len, GFP_USER);
        if (!data)
                return -ENOMEM;

        ret = -EFAULT;
        if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
                goto out;

        ret = ops->set_eeprom(dev, &eeprom, data);
        if (ret)
                goto out;

        if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
                ret = -EFAULT;

out:
        kfree(data);
        return ret;
}

static int ethtool_get_coalesce(struct net_device *dev, void *useraddr)
{
        struct ethtool_coalesce coalesce = { ETHTOOL_GCOALESCE };

        if (!ethtool_ops->get_coalesce)
                return -EOPNOTSUPP;

        ethtool_ops->get_coalesce(dev, &coalesce);

        if (copy_to_user(useraddr, &coalesce, sizeof(coalesce)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_coalesce(struct net_device *dev, void *useraddr)
{
        struct ethtool_coalesce coalesce;

        if (!ethtool_ops->get_coalesce)
                return -EOPNOTSUPP;

        if (copy_from_user(&coalesce, useraddr, sizeof(coalesce)))
                return -EFAULT;

        return ethtool_ops->set_coalesce(dev, &coalesce);
}

static int ethtool_get_ringparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_ringparam ringparam = { ETHTOOL_GRINGPARAM };

        if (!ethtool_ops->get_ringparam)
                return -EOPNOTSUPP;

        ethtool_ops->get_ringparam(dev, &ringparam);

        if (copy_to_user(useraddr, &ringparam, sizeof(ringparam)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_ringparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_ringparam ringparam;

        if (!ethtool_ops->get_ringparam)
                return -EOPNOTSUPP;

        if (copy_from_user(&ringparam, useraddr, sizeof(ringparam)))
                return -EFAULT;

        return ethtool_ops->set_ringparam(dev, &ringparam);
}

static int ethtool_get_pauseparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_pauseparam pauseparam = { ETHTOOL_GPAUSEPARAM };

        if (!ethtool_ops->get_pauseparam)
                return -EOPNOTSUPP;

        ethtool_ops->get_pauseparam(dev, &pauseparam);

        if (copy_to_user(useraddr, &pauseparam, sizeof(pauseparam)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_pauseparam(struct net_device *dev, void *useraddr)
{
        struct ethtool_pauseparam pauseparam;

        if (!ethtool_ops->get_pauseparam)
                return -EOPNOTSUPP;

        if (copy_from_user(&pauseparam, useraddr, sizeof(pauseparam)))
                return -EFAULT;

        return ethtool_ops->set_pauseparam(dev, &pauseparam);
}

static int ethtool_get_rx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GRXCSUM };

        if (!ethtool_ops->get_rx_csum)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_rx_csum(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_rx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_rx_csum)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        ethtool_ops->set_rx_csum(dev, edata.data);
        return 0;
}

static int ethtool_get_tx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GTXCSUM };

        if (!ethtool_ops->get_tx_csum)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_tx_csum(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_tx_csum(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_tx_csum)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_tx_csum(dev, edata.data);
}

static int ethtool_get_sg(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GSG };

        if (!ethtool_ops->get_sg)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_sg(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_sg(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_sg)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_sg(dev, edata.data);
}

static int ethtool_get_tso(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata = { ETHTOOL_GTSO };

        if (!ethtool_ops->get_tso)
                return -EOPNOTSUPP;

        edata.data = ethtool_ops->get_tso(dev);

        if (copy_to_user(useraddr, &edata, sizeof(edata)))
                return -EFAULT;
        return 0;
}

static int ethtool_set_tso(struct net_device *dev, char *useraddr)
{
        struct ethtool_value edata;

        if (!ethtool_ops->set_tso)
                return -EOPNOTSUPP;

        if (copy_from_user(&edata, useraddr, sizeof(edata)))
                return -EFAULT;

        return ethtool_ops->set_tso(dev, edata.data);
}

static int ethtool_self_test(struct net_device *dev, char *useraddr)
{
        struct ethtool_test test;
        struct ethtool_ops *ops = ethtool_ops;
        u64 *data;
        int ret;

        if (!ops->self_test || !ops->self_test_count)
                return -EOPNOTSUPP;

        if (copy_from_user(&test, useraddr, sizeof(test)))
                return -EFAULT;

        test.len = ops->self_test_count(dev);
        data = kmalloc(test.len * sizeof(u64), GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->self_test(dev, &test, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &test, sizeof(test)))
                goto out;
        useraddr += sizeof(test);
        if (copy_to_user(useraddr, data, test.len * sizeof(u64)))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_get_strings(struct net_device *dev, void *useraddr)
{
        struct ethtool_gstrings gstrings;
        struct ethtool_ops *ops = ethtool_ops;
        u8 *data;
        int ret;

        if (!ops->get_strings)
                return -EOPNOTSUPP;

        if (copy_from_user(&gstrings, useraddr, sizeof(gstrings)))
                return -EFAULT;

        switch (gstrings.string_set) {
        case ETH_SS_TEST:
                if (!ops->self_test_count)
                        return -EOPNOTSUPP;
                gstrings.len = ops->self_test_count(dev);
                break;
        case ETH_SS_STATS:
                if (!ops->get_stats_count)
                        return -EOPNOTSUPP;
                gstrings.len = ops->get_stats_count(dev);
                break;
        default:
                return -EINVAL;
        }

        data = kmalloc(gstrings.len * ETH_GSTRING_LEN, GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->get_strings(dev, gstrings.string_set, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &gstrings, sizeof(gstrings)))
                goto out;
        useraddr += sizeof(gstrings);
        if (copy_to_user(useraddr, data, gstrings.len * ETH_GSTRING_LEN))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_phys_id(struct net_device *dev, void *useraddr)
{
        struct ethtool_value id;

        if (!ethtool_ops->phys_id)
                return -EOPNOTSUPP;

        if (copy_from_user(&id, useraddr, sizeof(id)))
                return -EFAULT;

        return ethtool_ops->phys_id(dev, id.data);
}

static int ethtool_get_stats(struct net_device *dev, void *useraddr)
{
        struct ethtool_stats stats;
        struct ethtool_ops *ops = ethtool_ops;
        u64 *data;
        int ret;

        if (!ops->get_ethtool_stats || !ops->get_stats_count)
                return -EOPNOTSUPP;

        if (copy_from_user(&stats, useraddr, sizeof(stats)))
                return -EFAULT;

        stats.n_stats = ops->get_stats_count(dev);
        data = kmalloc(stats.n_stats * sizeof(u64), GFP_USER);
        if (!data)
                return -ENOMEM;

        ops->get_ethtool_stats(dev, &stats, data);

        ret = -EFAULT;
        if (copy_to_user(useraddr, &stats, sizeof(stats)))
                goto out;
        useraddr += sizeof(stats);
        if (copy_to_user(useraddr, data, stats.n_stats * sizeof(u64)))
                goto out;
        ret = 0;

out:
        kfree(data);
        return ret;
}

static int ethtool_ioctl(struct ifreq *ifr)
{
        struct net_device *dev = __dev_get_by_name(ifr->ifr_name);
        void *useraddr = (void *) ifr->ifr_data;
        u32 ethcmd;

        /*
         * XXX: This can be pushed down into the ethtool_* handlers that
         * need it.  Keep existing behaviour for the moment.
         */
        if (!capable(CAP_NET_ADMIN))
                return -EPERM;

        if (!dev || !netif_device_present(dev))
                return -ENODEV;

        if (copy_from_user(&ethcmd, useraddr, sizeof (ethcmd)))
                return -EFAULT;

        switch (ethcmd) {
        case ETHTOOL_GSET:
                return ethtool_get_settings(dev, useraddr);
        case ETHTOOL_SSET:
                return ethtool_set_settings(dev, useraddr);
        case ETHTOOL_GDRVINFO:
                return ethtool_get_drvinfo(dev, useraddr);
        case ETHTOOL_GREGS:
                return ethtool_get_regs(dev, useraddr);
        case ETHTOOL_GWOL:
                return ethtool_get_wol(dev, useraddr);
        case ETHTOOL_SWOL:
                return ethtool_set_wol(dev, useraddr);
        case ETHTOOL_GMSGLVL:
                return ethtool_get_msglevel(dev, useraddr);
        case ETHTOOL_SMSGLVL:
                return ethtool_set_msglevel(dev, useraddr);
        case ETHTOOL_NWAY_RST:
                return ethtool_nway_reset(dev);
        case ETHTOOL_GLINK:
                return ethtool_get_link(dev, useraddr);
        case ETHTOOL_GEEPROM:
                return ethtool_get_eeprom(dev, useraddr);
        case ETHTOOL_SEEPROM:
                return ethtool_set_eeprom(dev, useraddr);
        case ETHTOOL_GCOALESCE:
                return ethtool_get_coalesce(dev, useraddr);
        case ETHTOOL_SCOALESCE:
                return ethtool_set_coalesce(dev, useraddr);
        case ETHTOOL_GRINGPARAM:
                return ethtool_get_ringparam(dev, useraddr);
        case ETHTOOL_SRINGPARAM:
                return ethtool_set_ringparam(dev, useraddr);
        case ETHTOOL_GPAUSEPARAM:
                return ethtool_get_pauseparam(dev, useraddr);
        case ETHTOOL_SPAUSEPARAM:
                return ethtool_set_pauseparam(dev, useraddr);
        case ETHTOOL_GRXCSUM:
                return ethtool_get_rx_csum(dev, useraddr);
        case ETHTOOL_SRXCSUM:
                return ethtool_set_rx_csum(dev, useraddr);
        case ETHTOOL_GTXCSUM:
                return ethtool_get_tx_csum(dev, useraddr);
        case ETHTOOL_STXCSUM:
                return ethtool_set_tx_csum(dev, useraddr);
        case ETHTOOL_GSG:
                return ethtool_get_sg(dev, useraddr);
        case ETHTOOL_SSG:
                return ethtool_set_sg(dev, useraddr);
        case ETHTOOL_GTSO:
                return ethtool_get_tso(dev, useraddr);
        case ETHTOOL_STSO:
                return ethtool_set_tso(dev, useraddr);
        case ETHTOOL_TEST:
                return ethtool_self_test(dev, useraddr);
        case ETHTOOL_GSTRINGS:
                return ethtool_get_strings(dev, useraddr);
        case ETHTOOL_PHYS_ID:
                return ethtool_phys_id(dev, useraddr);
        case ETHTOOL_GSTATS:
                return ethtool_get_stats(dev, useraddr);
        default:
                return -EOPNOTSUPP;
        }

        return -EOPNOTSUPP;
}
#endif //ETHTOOL_OPS_COMPAT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)
static int rtl8101_siocdevprivate(struct net_device *dev, struct ifreq *ifr,
                                  void __user *data, int cmd)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int ret = 0;

        switch (cmd) {
        case SIOCRTLTOOL:
                if (!capable(CAP_NET_ADMIN))
                        return -EPERM;

                return rtl8101_tool_ioctl(tp, ifr);

        default:
                ret = -EOPNOTSUPP;
        }

        return ret;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0)

static int
rtl8101_do_ioctl(struct net_device *dev,
                 struct ifreq *ifr,
                 int cmd)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct mii_ioctl_data *data = if_mii(ifr);
        unsigned long flags;

        if (!netif_running(dev))
                return -ENODEV;

        switch (cmd) {
        case SIOCGMIIPHY:
                data->phy_id = 32; /* Internal PHY */
                return 0;

        case SIOCGMIIREG:
                spin_lock_irqsave(&tp->lock, flags);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                data->val_out = rtl8101_mdio_read(tp, data->reg_num & 0x1f);
                spin_unlock_irqrestore(&tp->lock, flags);
                return 0;

        case SIOCSMIIREG:
                if (!capable(CAP_NET_ADMIN))
                        return -EPERM;
                spin_lock_irqsave(&tp->lock, flags);
                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                rtl8101_mdio_write(tp, data->reg_num & 0x1f, data->val_in);
                spin_unlock_irqrestore(&tp->lock, flags);
                return 0;
#ifdef ETHTOOL_OPS_COMPAT
        case SIOCETHTOOL:
                return ethtool_ioctl(ifr);
#endif

        case SIOCRTLTOOL:
                if (!capable(CAP_NET_ADMIN))
                        return -EPERM;

                return rtl8101_tool_ioctl(tp, ifr);

        default:
                return -EOPNOTSUPP;
        }

        return -EOPNOTSUPP;
}

static void
rtl8101_phy_power_up(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        rtl8101_mdio_write(tp, MII_BMCR, BMCR_ANENABLE);

        //wait ups resume (phy state 3)
        switch (tp->mcfg) {
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_wait_phy_ups_resume(dev, 3);
                break;
        };
}

static void
rtl8101_phy_power_down(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_mdio_write(tp, 0x1f, 0x0000);
        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mdio_write(tp, MII_BMCR, BMCR_ANENABLE | BMCR_PDOWN);
                break;
        default:
                rtl8101_mdio_write(tp, MII_BMCR, BMCR_PDOWN);
                break;
        }
}

static int __devinit
rtl8101_init_board(struct pci_dev *pdev,
                   struct net_device **dev_out,
                   void __iomem **ioaddr_out)
{
        void __iomem *ioaddr;
        struct net_device *dev;
        struct rtl8101_private *tp;
        int rc = -ENOMEM, i, acpi_idle_state = 0, pm_cap;

        assert(ioaddr_out != NULL);

        /* dev zeroed in alloc_etherdev */
        dev = alloc_etherdev(sizeof (*tp));
        if (dev == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_drv(&debug))
                        dev_err(&pdev->dev, "unable to alloc new ethernet\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out;
        }

        SET_MODULE_OWNER(dev);
        SET_NETDEV_DEV(dev, &pdev->dev);
        tp = netdev_priv(dev);
        tp->dev = dev;
        tp->pci_dev = pdev;
        tp->msg_enable = netif_msg_init(debug.msg_enable, R8101_MSG_DEFAULT);

        /* enable device (incl. PCI PM wakeup and hotplug setup) */
        rc = pci_enable_device(pdev);
        if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "enable failure\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out_free_dev;
        }

        if (pci_set_mwi(pdev) < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_drv(&debug))
                        dev_info(&pdev->dev, "Mem-Wr-Inval unavailable.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        }

        /* save power state before pci_enable_device overwrites it */
        pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
        if (pm_cap) {
                u16 pwr_command;

                pci_read_config_word(pdev, pm_cap + PCI_PM_CTRL, &pwr_command);
                acpi_idle_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
        } else {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp)) {
                        dev_err(&pdev->dev, "PowerManagement capability not found.\n");
                }
#else
                printk("PowerManagement capability not found.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        }

        /* make sure PCI base addr 1 is MMIO */
        if (!(pci_resource_flags(pdev, 2) & IORESOURCE_MEM)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev,
                                "region #1 not an MMIO resource, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -ENODEV;
                goto err_out_mwi;
        }
        /* check for weird/broken PCI region reporting */
        if (pci_resource_len(pdev, 2) < R8101_REGS_SIZE) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev,
                                "Invalid PCI region size(s), aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -ENODEV;
                goto err_out_mwi;
        }

        rc = pci_request_regions(pdev, MODULENAME);
        if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "could not request regions.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                goto err_out_mwi;
        }

        if ((sizeof(dma_addr_t) > 4) &&
            use_dac &&
            !dma_set_mask(&pdev->dev, DMA_BIT_MASK(64)) &&
            !dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(64))) {
                dev->features |= NETIF_F_HIGHDMA;
        } else {
                rc = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
                if (rc < 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        if (netif_msg_probe(tp))
                                dev_err(&pdev->dev, "DMA configuration failed.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        goto err_out_free_res;
                }
        }

        /* ioremap MMIO region */
        ioaddr = ioremap(pci_resource_start(pdev, 2), R8101_REGS_SIZE);
        if (ioaddr == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp))
                        dev_err(&pdev->dev, "cannot remap MMIO, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                rc = -EIO;
                goto err_out_free_res;
        }

        tp->mmio_addr = ioaddr;

        pci_write_config_dword(pdev, 0x30, 0);

        /* Unneeded ? Don't mess with Mrs. Murphy. */
        rtl8101_irq_mask_and_ack(tp);

        /* Soft reset the chip. */
        RTL_W8(tp, ChipCmd, CmdReset);

        /* Check that the chip has finished the reset. */
        for (i = 1000; i > 0; i--) {
                if ((RTL_R8(tp, ChipCmd) & CmdReset) == 0)
                        break;
                udelay(10);
        }

        /* Identify chip attached to board */
        rtl8101_get_mac_version(tp);

        rtl8101_print_mac_version(tp);

        for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
                if (tp->mcfg == rtl_chip_info[i].mcfg)
                        break;
        }

        if (i < 0) {
                /* Unknown chip: assume array element #0, original RTL-8101 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                if (netif_msg_probe(tp)) {
                        dev_printk(KERN_DEBUG, &pdev->dev, "unknown chip version, assuming %s\n", rtl_chip_info[0].name);
                }
#else
                printk("Realtek unknown chip version, assuming %s\n", rtl_chip_info[0].name);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
                i++;
        }

        tp->chipset = i;

        *ioaddr_out = ioaddr;
        *dev_out = dev;
out:
        return rc;

err_out_free_res:
        pci_release_regions(pdev);
err_out_mwi:
        pci_clear_mwi(pdev);
        pci_disable_device(pdev);
err_out_free_dev:
        free_netdev(dev);
err_out:
        *ioaddr_out = NULL;
        *dev_out = NULL;
        goto out;
}

#define PCI_DEVICE_SERIAL_NUMBER (0x0164)

static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
rtl8101_esd_timer(unsigned long __opaque)
#else
rtl8101_esd_timer(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        struct net_device *dev = (struct net_device *)__opaque;
        struct rtl8101_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->esd_timer;
#else
        struct rtl8101_private *tp = from_timer(tp, t, esd_timer);
        struct net_device *dev = tp->dev;
        struct timer_list *timer = t;
#endif
        struct pci_dev *pdev = tp->pci_dev;
        unsigned long timeout = RTL8101_ESD_TIMEOUT;
        unsigned long flags;
        u8 cmd;
        u16 io_base_l;
        u16 mem_base_l;
        u16 mem_base_h;
        u8 ilr;
        u16 resv_0x1c_h;
        u16 resv_0x1c_l;
        u16 resv_0x20_l;
        u16 resv_0x20_h;
        u16 resv_0x24_l;
        u16 resv_0x24_h;
        u16 resv_0x2c_h;
        u16 resv_0x2c_l;
        u32 pci_sn_l;
        u32 pci_sn_h;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(tp->rtk_enable_diag))
                goto out_unlock;

        tp->esd_flag = 0;

        pci_read_config_byte(pdev, PCI_COMMAND, &cmd);
        if (cmd != tp->pci_cfg_space.cmd) {
                printk(KERN_ERR "%s: cmd = 0x%02x, should be 0x%02x \n.", dev->name, cmd, tp->pci_cfg_space.cmd);
                pci_write_config_byte(pdev, PCI_COMMAND, tp->pci_cfg_space.cmd);
                tp->esd_flag |= BIT_0;

                pci_read_config_byte(pdev, PCI_COMMAND, &cmd);
                if (cmd == 0xff) {
                        netif_err(tp, drv, dev, "pci link is down \n");
                        goto out_unlock;
                }
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &io_base_l);
        if (io_base_l != tp->pci_cfg_space.io_base_l) {
                printk(KERN_ERR "%s: io_base_l = 0x%04x, should be 0x%04x \n.", dev->name, io_base_l, tp->pci_cfg_space.io_base_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_0, tp->pci_cfg_space.io_base_l);
                tp->esd_flag |= BIT_1;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &mem_base_l);
        if (mem_base_l != tp->pci_cfg_space.mem_base_l) {
                printk(KERN_ERR "%s: mem_base_l = 0x%04x, should be 0x%04x \n.", dev->name, mem_base_l, tp->pci_cfg_space.mem_base_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_2, tp->pci_cfg_space.mem_base_l);
                tp->esd_flag |= BIT_2;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &mem_base_h);
        if (mem_base_h!= tp->pci_cfg_space.mem_base_h) {
                printk(KERN_ERR "%s: mem_base_h = 0x%04x, should be 0x%04x \n.", dev->name, mem_base_h, tp->pci_cfg_space.mem_base_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, tp->pci_cfg_space.mem_base_h);
                tp->esd_flag |= BIT_3;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_3, &resv_0x1c_l);
        if (resv_0x1c_l != tp->pci_cfg_space.resv_0x1c_l) {
                printk(KERN_ERR "%s: resv_0x1c_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x1c_l, tp->pci_cfg_space.resv_0x1c_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_3, tp->pci_cfg_space.resv_0x1c_l);
                tp->esd_flag |= BIT_4;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, &resv_0x1c_h);
        if (resv_0x1c_h != tp->pci_cfg_space.resv_0x1c_h) {
                printk(KERN_ERR "%s: resv_0x1c_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x1c_h, tp->pci_cfg_space.resv_0x1c_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, tp->pci_cfg_space.resv_0x1c_h);
                tp->esd_flag |= BIT_5;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &resv_0x20_l);
        if (resv_0x20_l != tp->pci_cfg_space.resv_0x20_l) {
                printk(KERN_ERR "%s: resv_0x20_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x20_l, tp->pci_cfg_space.resv_0x20_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_4, tp->pci_cfg_space.resv_0x20_l);
                tp->esd_flag |= BIT_6;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &resv_0x20_h);
        if (resv_0x20_h != tp->pci_cfg_space.resv_0x20_h) {
                printk(KERN_ERR "%s: resv_0x20_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x20_h, tp->pci_cfg_space.resv_0x20_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, tp->pci_cfg_space.resv_0x20_h);
                tp->esd_flag |= BIT_7;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &resv_0x24_l);
        if (resv_0x24_l != tp->pci_cfg_space.resv_0x24_l) {
                printk(KERN_ERR "%s: resv_0x24_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x24_l, tp->pci_cfg_space.resv_0x24_l);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_5, tp->pci_cfg_space.resv_0x24_l);
                tp->esd_flag |= BIT_8;
        }

        pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &resv_0x24_h);
        if (resv_0x24_h != tp->pci_cfg_space.resv_0x24_h) {
                printk(KERN_ERR "%s: resv_0x24_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x24_h, tp->pci_cfg_space.resv_0x24_h);
                pci_write_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, tp->pci_cfg_space.resv_0x24_h);
                tp->esd_flag |= BIT_9;
        }

        pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &ilr);
        if (ilr != tp->pci_cfg_space.ilr) {
                printk(KERN_ERR "%s: ilr = 0x%02x, should be 0x%02x \n.", dev->name, ilr, tp->pci_cfg_space.ilr);
                pci_write_config_byte(pdev, PCI_INTERRUPT_LINE, tp->pci_cfg_space.ilr);
                tp->esd_flag |= BIT_10;
        }

        pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &resv_0x2c_l);
        if (resv_0x2c_l != tp->pci_cfg_space.resv_0x2c_l) {
                printk(KERN_ERR "%s: resv_0x2c_l = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x2c_l, tp->pci_cfg_space.resv_0x2c_l);
                pci_write_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, tp->pci_cfg_space.resv_0x2c_l);
                tp->esd_flag |= BIT_11;
        }

        pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, &resv_0x2c_h);
        if (resv_0x2c_h != tp->pci_cfg_space.resv_0x2c_h) {
                printk(KERN_ERR "%s: resv_0x2c_h = 0x%04x, should be 0x%04x \n.", dev->name, resv_0x2c_h, tp->pci_cfg_space.resv_0x2c_h);
                pci_write_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, tp->pci_cfg_space.resv_0x2c_h);
                tp->esd_flag |= BIT_12;
        }

        pci_sn_l = rtl8101_csi_read(tp, PCI_DEVICE_SERIAL_NUMBER);
        if (pci_sn_l != tp->pci_cfg_space.pci_sn_l) {
                printk(KERN_ERR "%s: pci_sn_l = 0x%08x, should be 0x%08x \n.", dev->name, pci_sn_l, tp->pci_cfg_space.pci_sn_l);
                rtl8101_csi_write(tp, PCI_DEVICE_SERIAL_NUMBER, tp->pci_cfg_space.pci_sn_l);
                tp->esd_flag |= BIT_13;
        }

        pci_sn_h = rtl8101_csi_read(tp, PCI_DEVICE_SERIAL_NUMBER + 4);
        if (pci_sn_h != tp->pci_cfg_space.pci_sn_h) {
                printk(KERN_ERR "%s: pci_sn_h = 0x%08x, should be 0x%08x \n.", dev->name, pci_sn_h, tp->pci_cfg_space.pci_sn_h);
                rtl8101_csi_write(tp, PCI_DEVICE_SERIAL_NUMBER + 4, tp->pci_cfg_space.pci_sn_h);
                tp->esd_flag |= BIT_14;
        }

        if (tp->esd_flag != 0) {
                printk(KERN_ERR "%s: esd_flag = 0x%04x\n.\n", dev->name, tp->esd_flag);
                netif_stop_queue(dev);
                netif_carrier_off(dev);
                rtl8101_hw_reset(dev);
                rtl8101_tx_clear(tp);
                rtl8101_rx_clear(tp);
                rtl8101_init_ring(dev);
                rtl8101_hw_init(dev);
                rtl8101_powerup_pll(dev);
                rtl8101_hw_ephy_config(dev);
                rtl8101_hw_phy_config(dev);
                rtl8101_hw_config(dev);
                rtl8101_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
                tp->esd_flag = 0;
        }

out_unlock:
        spin_unlock_irqrestore(&tp->lock, flags);

        mod_timer(timer, jiffies + timeout);
}

static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
rtl8101_link_timer(unsigned long __opaque)
#else
rtl8101_link_timer(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
        struct net_device *dev = (struct net_device *)__opaque;
        struct rtl8101_private *tp = netdev_priv(dev);
        struct timer_list *timer = &tp->link_timer;
#else
        struct rtl8101_private *tp = from_timer(tp, t, link_timer);
        struct net_device *dev = tp->dev;
        struct timer_list *timer = t;
#endif
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
        rtl8101_check_link_status(dev);
        spin_unlock_irqrestore(&tp->lock, flags);

        mod_timer(timer, jiffies + RTL8101_LINK_TIMEOUT);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
static int pci_enable_msix_range(struct pci_dev *dev, struct msix_entry *entries,
                                 int minvec, int maxvec)
{
        int nvec = maxvec;
        int rc;

        if (maxvec < minvec)
                return -ERANGE;

        do {
                rc = pci_enable_msix(dev, entries, nvec);
                if (rc < 0) {
                        return rc;
                } else if (rc > 0) {
                        if (rc < minvec)
                                return -ENOSPC;
                        nvec = rc;
                }
        } while (rc);

        return nvec;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0) */

static int rtl8101_enable_msix(struct rtl8101_private *tp)
{
        int nvecs = 0;
        struct msix_entry msix_ent[1] = {{0}};

        nvecs = pci_enable_msix_range(tp->pci_dev, msix_ent,
                                      1, 1);
        if (nvecs < 0)
                goto out;

        tp->irq = msix_ent[0].vector;
out:
        return nvecs;
}

static unsigned rtl8101_try_msi(struct pci_dev *pdev, struct rtl8101_private *tp)
{
        unsigned msi = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        switch (tp->mcfg) {
        case CFG_METHOD_1:
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        case CFG_METHOD_6:
        case CFG_METHOD_7:
        case CFG_METHOD_8:
        case CFG_METHOD_9:
                dev_info(&pdev->dev, "Default use INTx.\n");
                break;
        default:
                if (rtl8101_enable_msix(tp) > 0)
                        msi |= RTL_FEATURE_MSIX;
                else if (!pci_enable_msi(pdev))
                        msi |= RTL_FEATURE_MSI;
                else
                        dev_info(&pdev->dev, "no MSI. Back to INTx.\n");
                break;
        }
#endif

        if (msi & RTL_FEATURE_MSIX)
                goto out;

        tp->irq = pdev->irq;

out:

        return msi;
}

static void rtl8101_disable_msi(struct pci_dev *pdev, struct rtl8101_private *tp)
{
        if (tp->features & RTL_FEATURE_MSIX) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                pci_disable_msix(pdev);
#endif
                tp->features &= ~RTL_FEATURE_MSIX;
        }

        if (tp->features & RTL_FEATURE_MSI) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
                pci_disable_msi(pdev);
#endif
                tp->features &= ~RTL_FEATURE_MSI;
        }
}

static void
rtl8101_aspm_fix1(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        int data;

        data = rtl8101_csi_read(tp, 0x110);

        if ((data & (1 << 7)) && (data & (1 << 8))) {
                rtl8101_ephy_write(tp, 0x01, 0x2e65);
                rtl8101_ephy_write(tp, 0x01, 0x6e65);
        }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static const struct net_device_ops rtl8101_netdev_ops = {
        .ndo_open		= rtl8101_open,
        .ndo_stop		= rtl8101_close,
        .ndo_get_stats		= rtl8101_get_stats,
        .ndo_start_xmit		= rtl8101_start_xmit,
        .ndo_tx_timeout		= rtl8101_tx_timeout,
        .ndo_change_mtu		= rtl8101_change_mtu,
        .ndo_set_mac_address	= rtl8101_set_mac_address,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
        .ndo_do_ioctl       = rtl8101_do_ioctl,
#else
        .ndo_siocdevprivate = rtl8101_siocdevprivate,
        .ndo_eth_ioctl      = rtl8101_do_ioctl,
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,1,0)
        .ndo_set_multicast_list	= rtl8101_set_rx_mode,
#else
        .ndo_set_rx_mode	= rtl8101_set_rx_mode,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
#ifdef CONFIG_R8101_VLAN
        .ndo_vlan_rx_register	= rtl8101_vlan_rx_register,
#endif
#else
        .ndo_fix_features	= rtl8101_fix_features,
        .ndo_set_features	= rtl8101_set_features,
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller	= rtl8101_netpoll,
#endif
};
#endif

static int __devinit
rtl8101_init_one(struct pci_dev *pdev,
                 const struct pci_device_id *ent)
{
        struct net_device *dev = NULL;
        struct rtl8101_private *tp;
        void __iomem *ioaddr = NULL;
        static int board_idx = -1;
        int rc;

        assert(pdev != NULL);
        assert(ent != NULL);

        board_idx++;

        if (netif_msg_drv(&debug)) {
                printk(KERN_INFO "%s Fast Ethernet driver %s loaded\n",
                       MODULENAME, RTL8101_VERSION);
        }

        rc = rtl8101_init_board(pdev, &dev, &ioaddr);
        if (rc)
                goto out;

        tp = netdev_priv(dev);
        assert(ioaddr != NULL);

        tp->set_speed = rtl8101_set_speed_xmii;
        tp->get_settings = rtl8101_gset_xmii;
        tp->phy_reset_enable = rtl8101_xmii_reset_enable;
        tp->phy_reset_pending = rtl8101_xmii_reset_pending;
        tp->link_ok = rtl8101_xmii_link_ok;

        tp->features |= rtl8101_try_msi(pdev, tp);

        RTL_NET_DEVICE_OPS(rtl8101_netdev_ops);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
        SET_ETHTOOL_OPS(dev, &rtl8101_ethtool_ops);
#endif

        dev->watchdog_timeo = RTL8101_TX_TIMEOUT;
        dev->irq = tp->irq;
        dev->base_addr = (unsigned long) ioaddr;

#ifdef CONFIG_R8101_NAPI
        RTL_NAPI_CONFIG(dev, tp, rtl8101_poll, R8101_NAPI_WEIGHT);
#endif

#ifdef CONFIG_R8101_VLAN
        if (tp->mcfg != CFG_METHOD_DEFAULT) {
                dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
                dev->vlan_rx_kill_vid = rtl8101_vlan_rx_kill_vid;
#endif//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
        }
#endif

        tp->cp_cmd |= RTL_R16(tp, CPlusCmd);
        if (tp->mcfg != CFG_METHOD_DEFAULT) {
                dev->features |= NETIF_F_IP_CSUM;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
                tp->cp_cmd |= RxChkSum;
#else
                dev->features |= NETIF_F_RXCSUM;
                dev->hw_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_TSO |
                                   NETIF_F_RXCSUM | NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
                dev->vlan_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_TSO |
                                     NETIF_F_HIGHDMA;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
                dev->hw_features |= NETIF_F_RXALL;
                dev->hw_features |= NETIF_F_RXFCS;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
                if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3)) {
                        dev->hw_features &= ~NETIF_F_IPV6_CSUM;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                        netif_set_tso_max_size(dev, LSO_64K);
                        netif_set_tso_max_segs(dev, NIC_MAX_PHYS_BUF_COUNT_LSO2);
#else //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                        netif_set_gso_max_size(dev, LSO_32K);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
                        dev->gso_max_segs = NIC_MAX_PHYS_BUF_COUNT_LSO_64K;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
                        dev->gso_min_segs = NIC_MIN_PHYS_BUF_COUNT;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                } else {
                        dev->hw_features |= NETIF_F_IPV6_CSUM | NETIF_F_TSO6;
                        dev->features |=  NETIF_F_IPV6_CSUM;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                        netif_set_tso_max_size(dev, LSO_64K);
                        netif_set_tso_max_segs(dev, NIC_MAX_PHYS_BUF_COUNT_LSO2);
#else //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                        netif_set_gso_max_size(dev, LSO_64K);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
                        dev->gso_max_segs = NIC_MAX_PHYS_BUF_COUNT_LSO2;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
                        dev->gso_min_segs = NIC_MIN_PHYS_BUF_COUNT;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
                }
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        }

        spin_lock_init(&tp->lock);

        rtl8101_init_software_variable(dev);

        rtl8101_exit_oob(dev);

        rtl8101_hw_init(dev);

        rtl8101_hw_reset(dev);

        /* Get production from EEPROM */
        if ((tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
             tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) &&
            (rtl8101_mac_ocp_read(tp, 0xDC00) & BIT_3))
                tp->eeprom_type = EEPROM_TYPE_NONE;
        else
                rtl8101_eeprom_type(tp);

        if (tp->eeprom_type == EEPROM_TYPE_93C46 || tp->eeprom_type == EEPROM_TYPE_93C56)
                rtl8101_set_eeprom_sel_low(tp);

        rtl8101_get_mac_address(dev);

        tp->tally_vaddr = dma_alloc_coherent(&pdev->dev,
                                             sizeof(*tp->tally_vaddr),
                                             &tp->tally_paddr,
                                             GFP_KERNEL);
        if (!tp->tally_vaddr) {
                rc = -ENOMEM;
                goto err_out;
        }

        rtl8101_tally_counter_clear(tp);

        pci_set_drvdata(pdev, dev);

        rc = register_netdev(dev);
        if (rc)
                goto err_out;

        printk(KERN_INFO "%s: This product is covered by one or more of the following patents: US6,570,884, US6,115,776, and US6,327,625.\n", MODULENAME);

        rtl8101_disable_rxdvgate(dev);

        device_set_wakeup_enable(&pdev->dev, tp->wol_enabled);

        if (netif_msg_probe(tp)) {
                printk(KERN_DEBUG "%s: Identified chip type is '%s'.\n",
                       dev->name, rtl_chip_info[tp->chipset].name);
        }

        netif_carrier_off(dev);

        printk("%s", GPL_CLAIM);

out:
        return rc;

err_out:
        if (tp->tally_vaddr != NULL) {
                dma_free_coherent(&pdev->dev,
                                  sizeof(*tp->tally_vaddr),
                                  tp->tally_vaddr,
                                  tp->tally_paddr);
                tp->tally_vaddr = NULL;
        }
#ifdef  CONFIG_R8101_NAPI
        RTL_NAPI_DEL(tp);
#endif
        rtl8101_disable_msi(pdev, tp);
        rtl8101_release_board(pdev, dev);

        goto out;
}

static void __devexit
rtl8101_remove_one(struct pci_dev *pdev)
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8101_private *tp = netdev_priv(dev);

        assert(dev != NULL);
        assert(tp != NULL);

#ifdef  CONFIG_R8101_NAPI
        RTL_NAPI_DEL(tp);
#endif

        unregister_netdev(dev);

        rtl8101_disable_msi(pdev, tp);
#ifdef ENABLE_R8101_PROCFS
        rtl8101_proc_remove(dev);
#endif
        if (tp->tally_vaddr != NULL) {
                dma_free_coherent(&pdev->dev,
                                  sizeof(*tp->tally_vaddr),
                                  tp->tally_vaddr,
                                  tp->tally_paddr);
                tp->tally_vaddr = NULL;
        }

        rtl8101_release_board(pdev, dev);
        pci_set_drvdata(pdev, NULL);
}

static void rtl8101_set_rxbufsize(struct rtl8101_private *tp,
                                  struct net_device *dev)
{
        switch (tp->mcfg) {
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                tp->rx_buf_sz = 0x05F3;
                break;
        default:
                tp->rx_buf_sz = 0x05EF;
                break;
        }
}

static int rtl8101_open(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;
        unsigned long flags;
        int retval;

        retval = -ENOMEM;

#ifdef ENABLE_R8101_PROCFS
        rtl8101_proc_init(dev);
#endif
        rtl8101_set_rxbufsize(tp, dev);

        /*
         * Rx and Tx descriptors needs 256 bytes alignment.
         * dma_alloc_coherent provides more.
         */
        tp->TxDescAllocSize = (tp->num_tx_desc + 1) * sizeof(struct TxDesc);
        tp->TxDescArray = dma_alloc_coherent(&pdev->dev,
                                             tp->TxDescAllocSize,
                                             &tp->TxPhyAddr, GFP_KERNEL);
        if (!tp->TxDescArray)
                goto out;

        tp->RxDescAllocSize = (tp->num_rx_desc + 1) * sizeof(struct RxDesc);
        tp->RxDescArray = dma_alloc_coherent(&pdev->dev,
                                             tp->RxDescAllocSize,
                                             &tp->RxPhyAddr, GFP_KERNEL);
        if (!tp->RxDescArray)
                goto err_free_tx;

        retval = rtl8101_init_ring(dev);
        if (retval < 0)
                goto err_free_rx;

        retval = request_irq(tp->irq, rtl8101_interrupt, (tp->features &
                             (RTL_FEATURE_MSI | RTL_FEATURE_MSIX)) ? 0 : SA_SHIRQ, dev->name, dev);

        if (retval < 0)
                goto err_free_rx;

        if (netif_msg_probe(tp)) {
                printk(KERN_INFO "%s: 0x%lx, "
                       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, "
                       "IRQ %d\n",
                       dev->name,
                       dev->base_addr,
                       dev->dev_addr[0], dev->dev_addr[1],
                       dev->dev_addr[2], dev->dev_addr[3],
                       dev->dev_addr[4], dev->dev_addr[5], tp->irq);
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
        INIT_WORK(&tp->task, rtl8101_reset_task, dev);
#else
        INIT_DELAYED_WORK(&tp->task, rtl8101_reset_task);
#endif

        pci_set_master(pdev);

#ifdef	CONFIG_R8101_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        RTL_NAPI_ENABLE(dev, &tp->napi);
#endif
#endif

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_exit_oob(dev);

        rtl8101_hw_init(dev);

        rtl8101_hw_reset(dev);

        rtl8101_powerup_pll(dev);

        rtl8101_hw_ephy_config(dev);

        rtl8101_hw_phy_config(dev);

        rtl8101_hw_config(dev);

        rtl8101_dsm(dev, DSM_IF_UP);

        rtl8101_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);

        spin_unlock_irqrestore(&tp->lock, flags);

        if (tp->esd_flag == 0)
                rtl8101_request_esd_timer(dev);

        rtl8101_request_link_timer(dev);
out:

        return retval;

err_free_rx:
        dma_free_coherent(&pdev->dev,
                          tp->RxDescAllocSize,
                          tp->RxDescArray,
                          tp->RxPhyAddr);
        tp->RxDescArray = NULL;
err_free_tx:
        dma_free_coherent(&pdev->dev,
                          tp->TxDescAllocSize,
                          tp->TxDescArray,
                          tp->TxPhyAddr);
        tp->TxDescArray = NULL;
        goto out;
}

static void
rtl8101_dsm(struct net_device *dev, int dev_state)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        switch (dev_state) {
        case DSM_MAC_INIT:
                if ((tp->mcfg == CFG_METHOD_4) ||
                    (tp->mcfg == CFG_METHOD_5) ||
                    (tp->mcfg == CFG_METHOD_6)) {
                        if (RTL_R8(tp, MACDBG) & 0x80) {
                                rtl8101_mdio_write(tp, 0x1f, 0x0000);
                                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) & ~(1 << 12));
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) | GPIO_en);
                        } else {
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) & ~GPIO_en);
                        }
                }

                break;
        case DSM_NIC_GOTO_D3:
        case DSM_IF_DOWN:
                if (RTL_R8(tp, MACDBG) & 0x80) {
                        if ((tp->mcfg == CFG_METHOD_4) || (tp->mcfg == CFG_METHOD_5)) {
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) | GPIO_en);
                                rtl8101_mdio_write(tp, 0x11, rtl8101_mdio_read(tp, 0x11) | (1 << 12));
                        } else if (tp->mcfg == CFG_METHOD_6) {
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) & ~GPIO_en);
                        }
                }
                break;
        case DSM_NIC_RESUME_D3:
        case DSM_IF_UP:
                if (RTL_R8(tp, MACDBG) & 0x80) {
                        if ((tp->mcfg == CFG_METHOD_4) || (tp->mcfg == CFG_METHOD_5)) {
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) & ~GPIO_en);
                        } else if (tp->mcfg == CFG_METHOD_6) {
                                RTL_W8(tp, GPIO, RTL_R8(tp, GPIO) | GPIO_en);
                        }
                }

                break;
        }

}

static void
rtl8101_hw_set_rx_packet_filter(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 mc_filter[2];	/* Multicast hash filter */
        int i, j, rx_mode;
        u32 tmp = 0;

        if (dev->flags & IFF_PROMISC) {
                /* Unconditionally log net taps. */
                if (netif_msg_link(tp)) {
                        printk(KERN_NOTICE "%s: Promiscuous mode enabled.\n",
                               dev->name);
                }
                rx_mode =
                        AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
                        AcceptAllPhys;
                mc_filter[1] = mc_filter[0] = 0xffffffff;
        } else if (dev->flags & IFF_ALLMULTI) {
                /* accept all multicasts. */
                rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0xffffffff;
        } else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
                struct dev_mc_list *mclist;
                rx_mode = AcceptBroadcast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0;
                for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
                     i++, mclist = mclist->next) {
                        int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
                        mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
                        rx_mode |= AcceptMulticast;
                }
#else
                struct netdev_hw_addr *ha;

                rx_mode = AcceptBroadcast | AcceptMyPhys;
                mc_filter[1] = mc_filter[0] = 0;
                netdev_for_each_mc_addr(ha, dev) {
                        int bit_nr = ether_crc(ETH_ALEN, ha->addr) >> 26;
                        mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
                        rx_mode |= AcceptMulticast;
                }
#endif
        }

        if (dev->features & NETIF_F_RXALL)
                rx_mode |= (AcceptErr | AcceptRunt);

        if (tp->mcfg == CFG_METHOD_10) {
                tmp = rtl8101_rx_config_V2 | rx_mode |
                      (RTL_R32(tp, RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);
        } else if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
                   tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                tmp = rtl8101_rx_config_V3 | rx_mode |
                      (RTL_R32(tp, RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);
        } else {
                tmp = rtl8101_rx_config_V1 | rx_mode |
                      (RTL_R32(tp, RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);
        }

        for (i = 0; i < 2; i++) {
                u32 mask = 0x000000ff;
                u32 tmp1 = 0;
                u32 tmp2 = 0;
                int x = 0;
                int y = 0;

                for (j = 0; j < 4; j++) {
                        tmp1 = mc_filter[i] & mask;
                        x = 32 - (8 + 16 * j);
                        y = x - 2 * x;

                        if (x > 0)
                                tmp2 = tmp2 | (tmp1 << x);
                        else
                                tmp2 = tmp2 | (tmp1 >> y);

                        mask = mask << 8;
                }
                mc_filter[i] = tmp2;
        }

        RTL_W32(tp, RxConfig, tmp);
        RTL_W32(tp, MAR0 + 0, mc_filter[1]);
        RTL_W32(tp, MAR0 + 4, mc_filter[0]);
}

static void
rtl8101_set_rx_mode(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_hw_set_rx_packet_filter(dev);

        spin_unlock_irqrestore(&tp->lock, flags);
}

static void
rtl8101_hw_config(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;
        u8 link_control;
        u16 mac_ocp_data;
        u32 csi_tmp;

        RTL_W32(tp, RxConfig, (RX_DMA_BURST << RxCfgDMAShift));

        rtl8101_hw_reset(dev);

        rtl8101_enable_cfg9346_write(tp);
        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) & ~BIT_7);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                break;
        }

        //clear io_rdy_l23
        switch (tp->mcfg) {
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~BIT_1);
                break;
        }

        RTL_W8(tp, MTPS, Reserved1_data);

        /* Set DMA burst size and Interframe Gap Time */
        RTL_W32(tp, TxConfig, (TX_DMA_BURST << TxDMAShift) |
                (InterFrameGap << TxInterFrameGapShift));

        tp->cp_cmd &= 0x2063;

        RTL_W16(tp, IntrMitigate, 0x0000);

        rtl8101_tally_counter_addr_fill(tp);

        rtl8101_desc_addr_fill(tp);

        if (tp->mcfg == CFG_METHOD_4) {
                set_offset70F(tp, 0x27);
                set_offset79(tp, 0x50);

                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);
                }

                RTL_W8(tp, Config1, 0x0f);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_5) {
                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);
                }

                set_offset79(tp, 0x50);

                RTL_W8(tp, Config1, 0x0f);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_6) {
                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);
                }

                set_offset79(tp, 0x50);

//		RTL_W8(tp, Config1, 0xDF);

                RTL_W8(tp, 0xF4, 0x01);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_7) {
                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);
                }

                set_offset79(tp, 0x50);

//		RTL_W8(tp, Config1, (RTL_R8(tp, Config1)&0xC0)|0x1F);

                RTL_W8(tp, 0xF4, 0x01);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

                RTL_W8(tp, 0xF5, RTL_R8(tp, 0xF5) | BIT_2);
        } else if (tp->mcfg == CFG_METHOD_8) {
                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);
                        RTL_W8(tp, 0xF4, RTL_R8(tp, 0xF4) | BIT_3);
                        RTL_W8(tp, 0xF5, RTL_R8(tp, 0xF5) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);

                        if (rtl8101_ephy_read(tp, 0x10)==0x0008) {
                                rtl8101_ephy_write(tp, 0x10, 0x000C);
                        }
                }

                pci_read_config_byte(pdev, 0x80, &link_control);
                if (link_control & 3)
                        rtl8101_ephy_write(tp, 0x02, 0x011F);

                set_offset79(tp, 0x50);

//		RTL_W8(tp, Config1, (RTL_R8(tp, Config1)&0xC0)|0x1F);

                RTL_W8(tp, 0xF4, RTL_R8(tp, 0xF4) | BIT_0);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_9) {
                pci_read_config_byte(pdev, 0x81, &link_control);
                if (link_control == 1) {
                        pci_write_config_byte(pdev, 0x81, 0);

                        RTL_W8(tp, DBG_reg, 0x98);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                        RTL_W8(tp, Config4, RTL_R8(tp, Config4) | BIT_2);

                        pci_write_config_byte(pdev, 0x81, 1);
                }

                set_offset79(tp, 0x50);

//		RTL_W8(tp, Config1, 0xDF);

                RTL_W8(tp, 0xF4, 0x01);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_10) {
                set_offset70F(tp, 0x27);
                set_offset79(tp, 0x50);

                RTL_W8(tp, 0xF3, RTL_R8(tp, 0xF3) | BIT_5);
                RTL_W8(tp, 0xF3, RTL_R8(tp, 0xF3) & ~BIT_5);

                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_7 | BIT_6);

                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_6 | BIT_5 | BIT_4 | BIT_2 | BIT_1);

                /*
                if (aspm)
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_7);
                */

                RTL_W8(tp, Config5, (RTL_R8(tp, Config5)&~0x08) | BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);
        } else if (tp->mcfg == CFG_METHOD_11 || tp->mcfg == CFG_METHOD_12 ||
                   tp->mcfg == CFG_METHOD_13) {
                u8	pci_config;

                tp->cp_cmd &= 0x2063;

                pci_read_config_byte(pdev, 0x80, &pci_config);
                if (pci_config & 0x03) {
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) | BIT_0);
                        RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_7);
                        /*
                        if (aspm)
                        RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_7);
                        */
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                }

                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_5 | BIT_3);
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_0);
                RTL_W8(tp, 0xD3, RTL_R8(tp, 0xD3) | BIT_3 | BIT_2);
                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_6);
                RTL_W16(tp, 0xE0, RTL_R16(tp, 0xE0) & ~0xDF9C);

                if (tp->mcfg == CFG_METHOD_11)
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
        } else if (tp->mcfg == CFG_METHOD_14) {
                set_offset70F(tp, 0x27);
                set_offset79(tp, 0x50);

                rtl8101_eri_write(tp, 0xC8, 4, 0x00000002, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xE8, 4, 0x00000006, ERIAR_ExGMAC);
                RTL_W32(tp, TxConfig, RTL_R32(tp, TxConfig) | BIT_7);
                RTL_W8(tp, 0xD3, RTL_R8(tp, 0xD3) & ~BIT_7);
                csi_tmp = rtl8101_eri_read(tp, 0xDC, 1, ERIAR_ExGMAC);
                csi_tmp &= ~BIT_0;
                rtl8101_eri_write(tp, 0xDC, 1, csi_tmp, ERIAR_ExGMAC);
                csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0xDC, 1, csi_tmp, ERIAR_ExGMAC);

                rtl8101_ephy_write(tp, 0x19, 0xff64);

                RTL_W8(tp, Config5, RTL_R8(tp, Config5) | BIT_0);
                RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);

                rtl8101_eri_write(tp, 0xC0, 2, 0x00000000, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xB8, 2, 0x00000000, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xD5, 1, 0x0000000E, ERIAR_ExGMAC);
        } else if (tp->mcfg == CFG_METHOD_15 || tp->mcfg == CFG_METHOD_16) {
                u8	pci_config;

                tp->cp_cmd &= 0x2063;

                pci_read_config_byte(pdev, 0x80, &pci_config);
                if (pci_config & 0x03) {
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) | BIT_0);
                        RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_7);
                        /*
                        if (aspm)
                        RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_7);
                        */
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                }

                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_5 | BIT_3);
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) & ~BIT_0);
                RTL_W8(tp, 0xD3, RTL_R8(tp, 0xD3) | BIT_3 | BIT_2);
                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) & ~BIT_6);
                RTL_W16(tp, 0xE0, RTL_R16(tp, 0xE0) & ~0xDF9C);
        } else if (tp->mcfg == CFG_METHOD_17 || tp->mcfg == CFG_METHOD_18 ||
                   tp->mcfg == CFG_METHOD_19 || tp->mcfg == CFG_METHOD_20) {
                set_offset70F(tp, 0x27);
                set_offset79(tp, 0x50);

                rtl8101_eri_write(tp, 0xC8, 4, 0x00080002, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xCC, 1, 0x38, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xD0, 1, 0x48, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xE8, 4, 0x00100006, ERIAR_ExGMAC);

                RTL_W32(tp, TxConfig, RTL_R32(tp, TxConfig) | BIT_7);

                csi_tmp = rtl8101_eri_read(tp, 0xDC, 1, ERIAR_ExGMAC);
                csi_tmp &= ~BIT_0;
                rtl8101_eri_write(tp, 0xDC, 1, csi_tmp, ERIAR_ExGMAC);
                csi_tmp |= BIT_0;
                rtl8101_eri_write(tp, 0xDC, 1, csi_tmp, ERIAR_ExGMAC);

                if (tp->mcfg == CFG_METHOD_20)
                        rtl8101_set_mcu_ocp_bit(tp, 0xD438, (BIT_1 | BIT_0));

                if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                    tp->mcfg == CFG_METHOD_20) {
                        if(tp->RequireAdjustUpsTxLinkPulseTiming) {
                                mac_ocp_data = rtl8101_mac_ocp_read(tp, 0xD412);
                                mac_ocp_data &= ~(0x0FFF);
                                mac_ocp_data |= tp->SwrCnt1msIni ;
                                rtl8101_mac_ocp_write(tp, 0xD412, mac_ocp_data);
                        }

                        mac_ocp_data = rtl8101_mac_ocp_read(tp, 0xE056);
                        mac_ocp_data &= ~(BIT_7 | BIT_6 | BIT_5 | BIT_4);
                        rtl8101_mac_ocp_write(tp, 0xE056, mac_ocp_data);

                        mac_ocp_data = rtl8101_mac_ocp_read(tp, 0xE052);
                        mac_ocp_data &= ~(BIT_15 | BIT_14 | BIT_13 | BIT_3);
                        mac_ocp_data |= BIT_15;
                        rtl8101_mac_ocp_write(tp, 0xE052, mac_ocp_data);

                        mac_ocp_data = rtl8101_mac_ocp_read(tp, 0xD420);
                        mac_ocp_data &= ~(BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
                        mac_ocp_data |= 0x45F;
                        rtl8101_mac_ocp_write(tp, 0xD420, mac_ocp_data);

                        mac_ocp_data = rtl8101_mac_ocp_read(tp, 0xE0D6);
                        mac_ocp_data &= ~(BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
                        mac_ocp_data |= 0x17F;
                        rtl8101_mac_ocp_write(tp, 0xE0D6, mac_ocp_data);
                }

                RTL_W8(tp, Config3, RTL_R8(tp, Config3) & ~Beacon_en);

                tp->cp_cmd = RTL_R16(tp, CPlusCmd) &
                             ~(EnableBist | Macdbgo_oe | Force_halfdup |
                               Force_rxflow_en | Force_txflow_en |
                               Cxpl_dbg_sel | ASF | PktCntrDisable |
                               Macdbgo_sel);

                RTL_W8(tp, 0x1B, RTL_R8(tp, 0x1B) & ~0x07);

                RTL_W8(tp, TDFNR, 0x4);

                /*
                if (aspm)
                RTL_W8(tp, 0xF1, RTL_R8(tp, 0xF1) | BIT_7);
                */

                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_6);
                RTL_W8(tp, 0xF2, RTL_R8(tp, 0xF2) | BIT_6);

                RTL_W8(tp, 0xD0, RTL_R8(tp, 0xD0) | BIT_7);

                rtl8101_eri_write(tp, 0xC0, 2, 0x0000, ERIAR_ExGMAC);
                rtl8101_eri_write(tp, 0xB8, 4, 0x00000000, ERIAR_ExGMAC);

                if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                    tp->mcfg == CFG_METHOD_20) {
                        rtl8101_mac_ocp_write(tp, 0xE054, 0x0000);

                        csi_tmp = rtl8101_eri_read(tp, 0x5F0, 4, ERIAR_ExGMAC);
                        csi_tmp &= ~(BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
                        rtl8101_eri_write(tp, 0x5F0, 4, csi_tmp, ERIAR_ExGMAC);
                } else {
                        rtl8101_eri_write(tp, 0x5F0, 2, 0x4F87, ERIAR_ExGMAC);
                }

                if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                    tp->mcfg == CFG_METHOD_20) {
                        csi_tmp = rtl8101_eri_read(tp, 0xDC, 4, ERIAR_ExGMAC);
                        csi_tmp |= BIT_4;
                        rtl8101_eri_write(tp, 0xDC, 4, csi_tmp, ERIAR_ExGMAC);
                }

                if (tp->mcfg == CFG_METHOD_17) {
                        rtl8101_mac_ocp_write(tp, 0xC140, 0xFFFF);
                } else if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                           tp->mcfg == CFG_METHOD_20) {
                        rtl8101_mac_ocp_write(tp, 0xC140, 0xFFFF);
                        rtl8101_mac_ocp_write(tp, 0xC142, 0xFFFF);
                }

                csi_tmp = rtl8101_eri_read(tp, 0x1B0, 4, ERIAR_ExGMAC);
                csi_tmp &= ~BIT_12;
                rtl8101_eri_write(tp, 0x1B0, 4, csi_tmp, ERIAR_ExGMAC);

                if (tp->mcfg == CFG_METHOD_18 || tp->mcfg == CFG_METHOD_19 ||
                    tp->mcfg == CFG_METHOD_20) {
                        csi_tmp = rtl8101_eri_read(tp, 0x2FC, 1, ERIAR_ExGMAC);
                        csi_tmp &= ~(BIT_2);
                        rtl8101_eri_write(tp, 0x2FC, 1, csi_tmp, ERIAR_ExGMAC);
                } else {
                        csi_tmp = rtl8101_eri_read(tp, 0x2FC, 1, ERIAR_ExGMAC);
                        csi_tmp &= ~(BIT_0 | BIT_1 | BIT_2);
                        csi_tmp |= BIT_0;
                        rtl8101_eri_write(tp, 0x2FC, 1, csi_tmp, ERIAR_ExGMAC);
                }

                csi_tmp = rtl8101_eri_read(tp, 0x1D0, 1, ERIAR_ExGMAC);
                csi_tmp |= BIT_1;
                rtl8101_eri_write(tp, 0x1D0, 1, csi_tmp, ERIAR_ExGMAC);
        }

        if ((tp->mcfg == CFG_METHOD_1) ||
            (tp->mcfg == CFG_METHOD_2) ||
            (tp->mcfg == CFG_METHOD_3)) {
                /* csum offload command for RTL8101E */
                tp->tx_tcp_csum_cmd = TxTCPCS;
                tp->tx_udp_csum_cmd = TxUDPCS;
                tp->tx_ip_csum_cmd = TxIPCS;
                tp->tx_ipv6_csum_cmd = 0;
        } else {
                /* csum offload command for RTL8102E */
                tp->tx_tcp_csum_cmd = TxTCPCS_C;
                tp->tx_udp_csum_cmd = TxUDPCS_C;
                tp->tx_ip_csum_cmd = TxIPCS_C;
                tp->tx_ipv6_csum_cmd = TxIPV6F_C;
        }

        //other hw parameters
        if (tp->mcfg == CFG_METHOD_17)
                rtl8101_eri_write(tp, 0x2F8, 2, 0x1D8F, ERIAR_ExGMAC);

        if (tp->bios_setting & BIT_28) {
                if (tp->mcfg == CFG_METHOD_13) {
                        if (RTL_R8(tp, 0xEF) & BIT_2) {
                                u32 gphy_val;

                                rtl8101_mdio_write(tp, 0x1F, 0x0001);
                                gphy_val = rtl8101_mdio_read(tp, 0x1B);
                                gphy_val |= BIT_2;
                                rtl8101_mdio_write(tp, 0x1B, gphy_val);
                                rtl8101_mdio_write(tp, 0x1F, 0x0000);
                        }
                }

                if (tp->mcfg == CFG_METHOD_14) {
                        u32 gphy_val;

                        rtl8101_mdio_write(tp, 0x1F, 0x0001);
                        gphy_val = rtl8101_mdio_read(tp, 0x13);
                        gphy_val |= BIT_15;
                        rtl8101_mdio_write(tp, 0x13, gphy_val);
                        rtl8101_mdio_write(tp, 0x1F, 0x0000);
                }
        }

        rtl8101_hw_clear_timer_int(dev);

        rtl8101_enable_exit_l1_mask(tp);

        switch (tp->mcfg) {
        case CFG_METHOD_17:
                rtl8101_mac_ocp_write(tp, 0xD3C0, 0x0B00);
                rtl8101_mac_ocp_write(tp, 0xD3C2, 0x0000);
                break;
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_mac_ocp_write(tp, 0xE098, 0x0AA2);
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_disable_pci_offset_99(tp);
                if (aspm) {
                        if (tp->org_pci_offset_99 & (BIT_2 | BIT_5 | BIT_6))
                                rtl8101_init_pci_offset_99(tp);
                }
                break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                rtl8101_disable_pci_offset_180(tp);
                if (aspm) {
                        if (tp->org_pci_offset_180 & (BIT_0|BIT_1))
                                rtl8101_init_pci_offset_180(tp);
                }
                break;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        RTL_W16(tp, CPlusCmd, tp->cp_cmd);
#else
        rtl8101_hw_set_features(dev, dev->features);
#endif

        switch (tp->mcfg) {
        case CFG_METHOD_17: {
                int timeout;
                for (timeout = 0; timeout < 10; timeout++) {
                        if ((rtl8101_eri_read(tp, 0x1AE, 2, ERIAR_ExGMAC) & BIT_13)==0)
                                break;
                        mdelay(1);
                }
        }
        break;
        }

        RTL_W16(tp, RxMaxSize, tp->rx_buf_sz);

        rtl8101_disable_rxdvgate(dev);

        if (!tp->pci_cfg_is_read) {
                pci_read_config_byte(pdev, PCI_COMMAND, &tp->pci_cfg_space.cmd);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &tp->pci_cfg_space.io_base_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, &tp->pci_cfg_space.io_base_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &tp->pci_cfg_space.mem_base_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &tp->pci_cfg_space.mem_base_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_3, &tp->pci_cfg_space.resv_0x1c_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_3 + 2, &tp->pci_cfg_space.resv_0x1c_h);
                pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &tp->pci_cfg_space.ilr);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &tp->pci_cfg_space.resv_0x20_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &tp->pci_cfg_space.resv_0x20_h);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &tp->pci_cfg_space.resv_0x24_l);
                pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &tp->pci_cfg_space.resv_0x24_h);
                pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &tp->pci_cfg_space.resv_0x2c_l);
                pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID + 2, &tp->pci_cfg_space.resv_0x2c_h);
                tp->pci_cfg_space.pci_sn_l = rtl8101_csi_read(tp, PCI_DEVICE_SERIAL_NUMBER);
                tp->pci_cfg_space.pci_sn_h = rtl8101_csi_read(tp, PCI_DEVICE_SERIAL_NUMBER + 4);

                tp->pci_cfg_is_read = 1;
        }

        rtl8101_dsm(dev, DSM_MAC_INIT);

        /* Set Rx packet filter */
        rtl8101_hw_set_rx_packet_filter(dev);

        switch (tp->mcfg) {
        case CFG_METHOD_10:
        case CFG_METHOD_11:
        case CFG_METHOD_12:
        case CFG_METHOD_13:
        case CFG_METHOD_14:
        case CFG_METHOD_15:
        case CFG_METHOD_16:
        case CFG_METHOD_17:
        case CFG_METHOD_18:
        case CFG_METHOD_19:
        case CFG_METHOD_20:
                if (aspm) {
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) | BIT_0);
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) | BIT_7);
                } else {
                        RTL_W8(tp, Config2, RTL_R8(tp, Config2) & ~BIT_7);
                        RTL_W8(tp, Config5, RTL_R8(tp, Config5) & ~BIT_0);
                }
                break;
        }

        rtl8101_disable_cfg9346_write(tp);

        udelay(10);
}

static void
rtl8101_hw_start(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        RTL_W8(tp, ChipCmd, CmdTxEnb | CmdRxEnb);

        rtl8101_enable_hw_interrupt(tp);
}

static int
rtl8101_change_mtu(struct net_device *dev,
                   int new_mtu)
{
        int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
        if (new_mtu < ETH_MIN_MTU || new_mtu > ETH_DATA_LEN)
                return -EINVAL;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)

        dev->mtu = new_mtu;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
        netdev_update_features(dev);
#endif

        return ret;
}

static inline void
rtl8101_make_unusable_by_asic(struct RxDesc *desc)
{
        desc->addr = 0x0badbadbadbadbadull;
        desc->opts1 &= ~cpu_to_le32(DescOwn | RsvdMask);
}

static void
rtl8101_free_rx_skb(struct rtl8101_private *tp,
                    struct sk_buff **sk_buff,
                    struct RxDesc *desc)
{
        struct pci_dev *pdev = tp->pci_dev;

        dma_unmap_single(&pdev->dev, le64_to_cpu(desc->addr), tp->rx_buf_sz,
                         DMA_FROM_DEVICE);
        dev_kfree_skb(*sk_buff);
        *sk_buff = NULL;
        rtl8101_make_unusable_by_asic(desc);
}

static inline void
rtl8101_mark_to_asic(struct RxDesc *desc,
                     u32 rx_buf_sz)
{
        u32 eor = le32_to_cpu(desc->opts1) & RingEnd;

        desc->opts1 = cpu_to_le32(DescOwn | eor | rx_buf_sz);
}

static inline void
rtl8101_map_to_asic(struct RxDesc *desc,
                    dma_addr_t mapping,
                    u32 rx_buf_sz)
{
        desc->addr = cpu_to_le64(mapping);
        wmb();
        rtl8101_mark_to_asic(desc, rx_buf_sz);
}

static int
rtl8101_alloc_rx_skb(struct rtl8101_private *tp,
                     struct sk_buff **sk_buff,
                     struct RxDesc *desc,
                     int rx_buf_sz,
                     u8 in_intr)
{
        struct sk_buff *skb;
        dma_addr_t mapping;
        int ret = 0;

        if (in_intr)
                skb = RTL_ALLOC_SKB_INTR(tp, rx_buf_sz + RTK_RX_ALIGN);
        else
                skb = dev_alloc_skb(rx_buf_sz + RTK_RX_ALIGN);

        if (unlikely(!skb))
                goto err_out;

        skb_reserve(skb, RTK_RX_ALIGN);

        mapping = dma_map_single(&tp->pci_dev->dev, skb->data, rx_buf_sz,
                                 DMA_FROM_DEVICE);
        if (unlikely(dma_mapping_error(&tp->pci_dev->dev, mapping))) {
                if (unlikely(net_ratelimit()))
                        netif_err(tp, drv, tp->dev, "Failed to map RX DMA!\n");
                goto err_out;
        }

        *sk_buff = skb;
        rtl8101_map_to_asic(desc, mapping, rx_buf_sz);

out:
        return ret;

err_out:
        if (skb)
                dev_kfree_skb(skb);
        ret = -ENOMEM;
        rtl8101_make_unusable_by_asic(desc);
        goto out;
}

static void
rtl8101_rx_clear(struct rtl8101_private *tp)
{
        int i;

        for (i = 0; i < tp->num_rx_desc; i++) {
                if (tp->Rx_skbuff[i]) {
                        rtl8101_free_rx_skb(tp, tp->Rx_skbuff + i,
                                            tp->RxDescArray + i);
                }
        }
}

static u32
rtl8101_rx_fill(struct rtl8101_private *tp,
                struct net_device *dev,
                u32 start,
                u32 end,
                u8 in_intr)
{
        u32 cur;

        for (cur = start; end - cur > 0; cur++) {
                int ret, i = cur % tp->num_rx_desc;

                if (tp->Rx_skbuff[i])
                        continue;

                ret = rtl8101_alloc_rx_skb(tp, tp->Rx_skbuff + i,
                                           tp->RxDescArray + i,
                                           tp->rx_buf_sz,
                                           in_intr);
                if (ret < 0)
                        break;
        }
        return cur - start;
}

static inline void
rtl8101_mark_as_last_descriptor(struct RxDesc *desc)
{
        desc->opts1 |= cpu_to_le32(RingEnd);
}

static void
rtl8101_desc_addr_fill(struct rtl8101_private *tp)
{
        if (!tp->TxPhyAddr || !tp->RxPhyAddr)
                return;

        RTL_W32(tp, TxDescStartAddrLow, ((u64) tp->TxPhyAddr & DMA_BIT_MASK(32)));
        RTL_W32(tp, TxDescStartAddrHigh, ((u64) tp->TxPhyAddr >> 32));
        RTL_W32(tp, RxDescAddrLow, ((u64) tp->RxPhyAddr & DMA_BIT_MASK(32)));
        RTL_W32(tp, RxDescAddrHigh, ((u64) tp->RxPhyAddr >> 32));
}

static void
rtl8101_tx_desc_init(struct rtl8101_private *tp)
{
        int i = 0;

        memset(tp->TxDescArray, 0x0, tp->TxDescAllocSize);

        for (i = 0; i < tp->num_tx_desc; i++) {
                if (i == (tp->num_tx_desc - 1))
                        tp->TxDescArray[i].opts1 = cpu_to_le32(RingEnd);
        }
}

static void
rtl8101_rx_desc_init(struct rtl8101_private *tp)
{
        memset(tp->RxDescArray, 0x0, tp->RxDescAllocSize);
}

static int
rtl8101_init_ring(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_init_ring_indexes(tp);

        memset(tp->tx_skb, 0x0, sizeof(tp->tx_skb));
        memset(tp->Rx_skbuff, 0x0, sizeof(tp->Rx_skbuff));

        rtl8101_tx_desc_init(tp);
        rtl8101_rx_desc_init(tp);

        if (rtl8101_rx_fill(tp, dev, 0, tp->num_rx_desc, 0) != tp->num_rx_desc)
                goto err_out;

        rtl8101_mark_as_last_descriptor(tp->RxDescArray + tp->num_rx_desc - 1);

        return 0;

err_out:
        rtl8101_rx_clear(tp);
        return -ENOMEM;
}

static void
rtl8101_unmap_tx_skb(struct pci_dev *pdev,
                     struct ring_info *tx_skb,
                     struct TxDesc *desc)
{
        unsigned int len = tx_skb->len;

        dma_unmap_single(&pdev->dev, le64_to_cpu(desc->addr), len, DMA_TO_DEVICE);

        desc->opts1 = cpu_to_le32(RTK_MAGIC_DEBUG_VALUE);
        desc->opts2 = 0x00;
        desc->addr = 0x00;
        tx_skb->len = 0;
}

static void rtl8101_tx_clear_range(struct rtl8101_private *tp, u32 start,
                                   unsigned int n)
{
        unsigned int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
        struct net_device *dev = tp->dev;
#endif

        for (i = 0; i < n; i++) {
                unsigned int entry = (start + i) % tp->num_tx_desc;
                struct ring_info *tx_skb = tp->tx_skb + entry;
                unsigned int len = tx_skb->len;

                if (len) {
                        struct sk_buff *skb = tx_skb->skb;

                        rtl8101_unmap_tx_skb(tp->pci_dev, tx_skb,
                                             tp->TxDescArray + entry);
                        if (skb) {
                                RTLDEV->stats.tx_dropped++;
                                dev_kfree_skb_any(skb);
                                tx_skb->skb = NULL;
                        }
                }
        }
}

static void
rtl8101_tx_clear(struct rtl8101_private *tp)
{
        rtl8101_tx_clear_range(tp, tp->dirty_tx, tp->num_tx_desc);
        tp->cur_tx = tp->dirty_tx = 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8101_schedule_work(struct net_device *dev, void (*task)(void *))
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        struct rtl8101_private *tp = netdev_priv(dev);

        INIT_WORK(&tp->task, task, dev);
        schedule_delayed_work(&tp->task, 4);
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
}

#define rtl8101_cancel_schedule_work(a)

#else
static void rtl8101_schedule_work(struct net_device *dev, work_func_t task)
{
        struct rtl8101_private *tp = netdev_priv(dev);

        INIT_DELAYED_WORK(&tp->task, task);
        schedule_delayed_work(&tp->task, 4);
}

static void rtl8101_cancel_schedule_work(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct work_struct *work = &tp->task.work;

        if (!work->func) return;

        cancel_delayed_work_sync(&tp->task);
}
#endif

#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8101_reinit_task(void *_data)
#else
static void rtl8101_reinit_task(struct work_struct *work)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
        struct net_device *dev = _data;
#else
        struct rtl8101_private *tp =
                container_of(work, struct rtl8101_private, task.work);
        struct net_device *dev = tp->dev;
#endif
        int ret;

        if (netif_running(dev)) {
                rtl8101_wait_for_quiescence(dev);
                rtl8101_close(dev);
        }

        ret = rtl8101_open(dev);
        if (unlikely(ret < 0)) {
                if (unlikely(net_ratelimit())) {
                        struct rtl8101_private *tp = netdev_priv(dev);

                        if (netif_msg_drv(tp)) {
                                printk(PFX KERN_ERR
                                       "%s: reinit failure (status = %d)."
                                       " Rescheduling.\n", dev->name, ret);
                        }
                }
                rtl8101_schedule_work(dev, rtl8101_reinit_task);
        }
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8101_reset_task(void *_data)
{
        struct net_device *dev = _data;
        struct rtl8101_private *tp = netdev_priv(dev);
#else
static void rtl8101_reset_task(struct work_struct *work)
{
        struct rtl8101_private *tp =
                container_of(work, struct rtl8101_private, task.work);
        struct net_device *dev = tp->dev;
#endif
        u32 budget = ~(u32)0;
        unsigned long flags;

        if (!netif_running(dev))
                return;

        rtl8101_wait_for_quiescence(dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
        rtl8101_rx_interrupt(dev, tp, &budget);
#else
        rtl8101_rx_interrupt(dev, tp, budget);
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_tx_clear(tp);

        if (tp->dirty_rx == tp->cur_rx) {
                rtl8101_rx_clear(tp);
                rtl8101_init_ring(dev);
                rtl8101_set_speed(dev, tp->autoneg, tp->speed, tp->duplex, tp->advertising);
                spin_unlock_irqrestore(&tp->lock, flags);
        } else {
                spin_unlock_irqrestore(&tp->lock, flags);
                if (unlikely(net_ratelimit())) {
                        struct rtl8101_private *tp = netdev_priv(dev);

                        if (netif_msg_intr(tp)) {
                                printk(PFX KERN_EMERG
                                       "%s: Rx buffers shortage\n", dev->name);
                        }
                }
                rtl8101_schedule_work(dev, rtl8101_reset_task);
        }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static void
rtl8101_tx_timeout(struct net_device *dev, unsigned int txqueue)
#else
static void
rtl8101_tx_timeout(struct net_device *dev)
#endif
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
        netif_stop_queue(dev);
        netif_carrier_off(dev);
        rtl8101_hw_reset(dev);
        spin_unlock_irqrestore(&tp->lock, flags);

        /* Let's wait a bit while any (async) irq lands on */
        rtl8101_schedule_work(dev, rtl8101_reset_task);
}

static u32
rtl8101_get_txd_opts1(struct rtl8101_private *tp,
                      u32 opts1,
                      u32 len,
                      unsigned int entry)
{
        u32 status = opts1 | len;

        if (entry == tp->num_tx_desc - 1)
                status |= RingEnd;

        return status;
}

static int
rtl8101_xmit_frags(struct rtl8101_private *tp,
                   struct sk_buff *skb,
                   const u32 *opts)
{
        struct skb_shared_info *info = skb_shinfo(skb);
        unsigned int cur_frag, entry;
        struct TxDesc *txd = NULL;
        const unsigned char nr_frags = info->nr_frags;

        entry = tp->cur_tx;
        for (cur_frag = 0; cur_frag < nr_frags; cur_frag++) {
                skb_frag_t *frag = info->frags + cur_frag;
                dma_addr_t mapping;
                u32 status, len;
                void *addr;

                entry = (entry + 1) % tp->num_tx_desc;

                txd = tp->TxDescArray + entry;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
                len = frag->size;
                addr = ((void *) page_address(frag->page)) + frag->page_offset;
#else
                len = skb_frag_size(frag);
                addr = skb_frag_address(frag);
#endif
                mapping = dma_map_single(&tp->pci_dev->dev, addr, len, DMA_TO_DEVICE);

                if (unlikely(dma_mapping_error(&tp->pci_dev->dev, mapping))) {
                        if (unlikely(net_ratelimit()))
                                netif_err(tp, drv, tp->dev,
                                          "Failed to map TX fragments DMA!\n");
                        goto err_out;
                }

                /* anti gcc 2.95.3 bugware (sic) */
                status = rtl8101_get_txd_opts1(tp, opts[0], len, entry);
                if (cur_frag == (nr_frags - 1)) {
                        tp->tx_skb[entry].skb = skb;
                        status |= LastFrag;
                }

                txd->addr = cpu_to_le64(mapping);

                tp->tx_skb[entry].len = len;

                txd->opts2 = cpu_to_le32(opts[1]);
                wmb();
                txd->opts1 = cpu_to_le32(status);
        }

        return cur_frag;

err_out:
        rtl8101_tx_clear_range(tp, tp->cur_tx + 1, cur_frag);
        return -EIO;
}

static inline
__be16 get_protocol(struct sk_buff *skb)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
        return vlan_get_protocol(skb);
#else
        __be16 protocol;

        if (skb->protocol == htons(ETH_P_8021Q))
                protocol = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
        else
                protocol = skb->protocol;

        return protocol;
#endif
}

static inline bool
rtl8101_tx_csum(struct sk_buff *skb,
                struct net_device *dev,
                u32 *opts)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        u32 csum_cmd = 0;
        u8 sw_calc_csum = FALSE;

        if (skb->ip_summed == CHECKSUM_PARTIAL) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
                const struct iphdr *ip = skb->nh.iph;

                if (dev->features & NETIF_F_IP_CSUM) {
                        if (ip->protocol == IPPROTO_TCP)
                                csum_cmd = tp->tx_ip_csum_cmd | tp->tx_tcp_csum_cmd;
                        else if (ip->protocol == IPPROTO_UDP)
                                csum_cmd = tp->tx_ip_csum_cmd | tp->tx_udp_csum_cmd;
                        else if (ip->protocol == IPPROTO_IP)
                                csum_cmd = tp->tx_ip_csum_cmd;
                }
#else
                u8 ip_protocol = IPPROTO_RAW;

                switch (get_protocol(skb)) {
                case __constant_htons(ETH_P_IP):
                        if (dev->features & NETIF_F_IP_CSUM) {
                                ip_protocol = ip_hdr(skb)->protocol;
                                csum_cmd = tp->tx_ip_csum_cmd;
                        }
                        break;
                case __constant_htons(ETH_P_IPV6):
                        if (dev->features & NETIF_F_IPV6_CSUM) {
                                if (skb_transport_offset(skb) > 0 && skb_transport_offset(skb) <= TCPHO_MAX) {
                                        ip_protocol = ipv6_hdr(skb)->nexthdr;
                                        csum_cmd = tp->tx_ipv6_csum_cmd;
                                        csum_cmd |= skb_transport_offset(skb) << TCPHO_SHIFT;
                                }
                        }
                        break;
                default:
                        if (unlikely(net_ratelimit()))
                                dprintk("checksum_partial proto=%x!\n", skb->protocol);
                        break;
                }

                if (ip_protocol == IPPROTO_TCP)
                        csum_cmd |= tp->tx_tcp_csum_cmd;
                else if (ip_protocol == IPPROTO_UDP)
                        csum_cmd |= tp->tx_udp_csum_cmd;
#endif
                if (csum_cmd == 0) {
                        sw_calc_csum = TRUE;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
                        WARN_ON(1); /* we need a WARN() */
#endif
                }
        }

        if (csum_cmd != 0) {
                if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3))
                        opts[0] |= csum_cmd;
                else
                        opts[1] |= csum_cmd;
        }

        if (sw_calc_csum) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,7)
                skb_checksum_help(&skb, 0);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
                skb_checksum_help(skb, 0);
#else
                skb_checksum_help(skb);
#endif
        }

        return true;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
/* r8169_csum_workaround()
  * The hw limites the value the transport offset. When the offset is out of the
  * range, calculate the checksum by sw.
  */
static void r8101_csum_workaround(struct rtl8101_private *tp,
                                  struct sk_buff *skb)
{
        if (skb_shinfo(skb)->gso_size) {
                netdev_features_t features = tp->dev->features;
                struct sk_buff *segs, *nskb;

                features &= ~(NETIF_F_SG | NETIF_F_IPV6_CSUM | NETIF_F_TSO6);
                segs = skb_gso_segment(skb, features);
                if (IS_ERR(segs) || !segs)
                        goto drop;

                do {
                        nskb = segs;
                        segs = segs->next;
                        nskb->next = NULL;
                        rtl8101_start_xmit(nskb, tp->dev);
                } while (segs);

                dev_consume_skb_any(skb);
        } else if (skb->ip_summed == CHECKSUM_PARTIAL) {
                if (skb_checksum_help(skb) < 0)
                        goto drop;

                rtl8101_start_xmit(skb, tp->dev);
        } else {
                struct net_device_stats *stats;

drop:
                stats = &tp->dev->stats;
                stats->tx_dropped++;
                dev_kfree_skb_any(skb);
        }
}

/* msdn_giant_send_check()
 * According to the document of microsoft, the TCP Pseudo Header excludes the
 * packet length for IPv6 TCP large packets.
 */
static int msdn_giant_send_check(struct sk_buff *skb)
{
        const struct ipv6hdr *ipv6h;
        struct tcphdr *th;
        int ret;

        ret = skb_cow_head(skb, 0);
        if (ret)
                return ret;

        ipv6h = ipv6_hdr(skb);
        th = tcp_hdr(skb);

        th->check = 0;
        th->check = ~tcp_v6_check(0, &ipv6h->saddr, &ipv6h->daddr, 0);

        return ret;
}
#endif

static bool rtl8101_tx_slots_avail(struct rtl8101_private *tp,
                                   unsigned int nr_frags)
{
        unsigned int slots_avail = tp->dirty_tx + tp->num_tx_desc - tp->cur_tx;

        /* A skbuff with nr_frags needs nr_frags+1 entries in the tx queue */
        return slots_avail > nr_frags;
}

static netdev_tx_t
rtl8101_start_xmit(struct sk_buff *skb,
                   struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned int entry;
        struct TxDesc *txd;
        dma_addr_t mapping;
        u32 len;
        u32 opts[2];
        netdev_tx_t ret = NETDEV_TX_OK;
        unsigned long flags, large_send;
        int frags;

        spin_lock_irqsave(&tp->lock, flags);

        if (unlikely(!rtl8101_tx_slots_avail(tp, skb_shinfo(skb)->nr_frags))) {
                if (netif_msg_drv(tp)) {
                        printk(KERN_ERR
                               "%s: BUG! Tx Ring full when queue awake!\n",
                               dev->name);
                }
                goto err_stop;
        }

        entry = tp->cur_tx % tp->num_tx_desc;
        txd = tp->TxDescArray + entry;

        if (unlikely(le32_to_cpu(txd->opts1) & DescOwn)) {
                if (netif_msg_drv(tp)) {
                        printk(KERN_ERR
                               "%s: BUG! Tx Desc is own by hardware!\n",
                               dev->name);
                }
                goto err_stop;
        }

        opts[0] = DescOwn;
        opts[1] = rtl8101_tx_vlan_tag(tp, skb);

        large_send = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        if (dev->features & (NETIF_F_TSO | NETIF_F_TSO6)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
                u32 mss = skb_shinfo(skb)->tso_size;
#else
                u32 mss = skb_shinfo(skb)->gso_size;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)

                /* TCP Segmentation Offload (or TCP Large Send) */
                if (mss) {
                        if ((tp->mcfg == CFG_METHOD_1) ||
                            (tp->mcfg == CFG_METHOD_2) ||
                            (tp->mcfg == CFG_METHOD_3)) {
                                opts[0] |= LargeSend | (min(mss, MSS_MAX) << 16);
                                large_send = 1;
                        } else {
                                switch (get_protocol(skb)) {
                                case __constant_htons(ETH_P_IP):
                                        if (skb_transport_offset(skb) <= GTTCPHO_MAX) {
                                                opts[0] |= GiantSendv4;
                                                opts[0] |= skb_transport_offset(skb) << GTTCPHO_SHIFT;
                                                opts[1] |= min(mss, MSS_MAX) << 18;
                                                large_send = 1;
                                        }
                                        break;
                                case __constant_htons(ETH_P_IPV6):
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
                                        if (msdn_giant_send_check(skb)) {
                                                spin_unlock_irqrestore(&tp->lock, flags);
                                                r8101_csum_workaround(tp, skb);
                                                goto out;
                                        }
#endif
                                        if (skb_transport_offset(skb) <= GTTCPHO_MAX) {
                                                opts[0] |= GiantSendv6;
                                                opts[0] |= skb_transport_offset(skb) << GTTCPHO_SHIFT;
                                                opts[1] |= min(mss, MSS_MAX) << 18;
                                                large_send = 1;
                                        }
                                        break;
                                default:
                                        if (unlikely(net_ratelimit()))
                                                dprintk("tso proto=%x!\n", skb->protocol);
                                        break;
                                }
                        }

                        if (large_send == 0)
                                goto err_dma_0;
                }
        }
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

        if (large_send == 0) {
                if (unlikely(!rtl8101_tx_csum(skb, dev, opts)))
                        goto err_dma_0;
        }

        frags = rtl8101_xmit_frags(tp, skb, opts);
        if (unlikely(frags < 0))
                goto err_dma_0;
        if (frags) {
                len = skb_headlen(skb);
                opts[0] |= FirstFrag;
        } else {
                len = skb->len;

                opts[0] |= FirstFrag | LastFrag;

                tp->tx_skb[entry].skb = skb;
        }

        opts[0] = rtl8101_get_txd_opts1(tp, opts[0], len, entry);
        mapping = dma_map_single(&tp->pci_dev->dev, skb->data, len, DMA_TO_DEVICE);
        if (unlikely(dma_mapping_error(&tp->pci_dev->dev, mapping))) {
                if (unlikely(net_ratelimit()))
                        netif_err(tp, drv, dev, "Failed to map TX DMA!\n");
                goto err_dma_1;
        }
        tp->tx_skb[entry].len = len;
        txd->addr = cpu_to_le64(mapping);
        txd->opts2 = cpu_to_le32(opts[1]);
        wmb();
        txd->opts1 = cpu_to_le32(opts[0]);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)
        dev->trans_start = jiffies;
#else
        skb_tx_timestamp(skb);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)

        tp->cur_tx += frags + 1;

        wmb();

        RTL_W8(tp, TxPoll, NPQ);    /* set polling bit */

        if (!rtl8101_tx_slots_avail(tp, MAX_SKB_FRAGS)) {
                netif_stop_queue(dev);
                smp_rmb();
                if (rtl8101_tx_slots_avail(tp, MAX_SKB_FRAGS))
                        netif_wake_queue(dev);
        }

        spin_unlock_irqrestore(&tp->lock, flags);

out:
        return ret;

err_dma_1:
        tp->tx_skb[entry].skb = NULL;
        rtl8101_tx_clear_range(tp, tp->cur_tx + 1, frags);
err_dma_0:
        RTLDEV->stats.tx_dropped++;
        spin_unlock_irqrestore(&tp->lock, flags);
        dev_kfree_skb_any(skb);
        ret = NETDEV_TX_OK;
        goto out;
err_stop:
        netif_stop_queue(dev);
        ret = NETDEV_TX_BUSY;
        RTLDEV->stats.tx_dropped++;

        spin_unlock_irqrestore(&tp->lock, flags);
        goto out;
}

static void
rtl8101_tx_interrupt(struct net_device *dev,
                     struct rtl8101_private *tp)
{
        unsigned int dirty_tx, tx_left;

        assert(dev != NULL);
        assert(tp != NULL);
        assert(ioaddr != NULL);

        dirty_tx = tp->dirty_tx;
        smp_rmb();
        tx_left = tp->cur_tx - dirty_tx;

        while (tx_left > 0) {
                unsigned int entry = dirty_tx % tp->num_tx_desc;
                struct ring_info *tx_skb = tp->tx_skb + entry;
                u32 len = tx_skb->len;
                u32 status;

                rmb();
                status = le32_to_cpu(tp->TxDescArray[entry].opts1);
                if (status & DescOwn)
                        break;

                RTLDEV->stats.tx_bytes += len;
                RTLDEV->stats.tx_packets++;

                rtl8101_unmap_tx_skb(tp->pci_dev, tx_skb, tp->TxDescArray + entry);

                if (status & LastFrag) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0)
                        dev_consume_skb_any(tx_skb->skb);
#else
                        dev_kfree_skb_any(tx_skb->skb);
#endif
                        tx_skb->skb = NULL;
                }
                dirty_tx++;
                tx_left--;
        }

        if (tp->dirty_tx != dirty_tx) {
                tp->dirty_tx = dirty_tx;
                smp_wmb();
                if (netif_queue_stopped(dev) &&
                    (rtl8101_tx_slots_avail(tp, MAX_SKB_FRAGS))) {
                        netif_wake_queue(dev);
                }
                smp_wmb();
                if (tp->cur_tx != dirty_tx)
                        RTL_W8(tp, TxPoll, NPQ);
        }
}

static inline int
rtl8101_fragmented_frame(u32 status)
{
        return (status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag);
}

static inline void
rtl8101_rx_csum(struct rtl8101_private *tp,
                struct sk_buff *skb,
                struct RxDesc *desc)
{
        u32 opts1 = le32_to_cpu(desc->opts1);
        u32 opts2 = le32_to_cpu(desc->opts2);

        if ((tp->mcfg == CFG_METHOD_1) ||
            (tp->mcfg == CFG_METHOD_2) ||
            (tp->mcfg == CFG_METHOD_3)) {
                u32 status = opts1 & RxProtoMask;

                /* rx csum offload for RTL8101E */
                if (((status == RxProtoTCP) && !(opts1 & (RxTCPF | RxIPF))) ||
                    ((status == RxProtoUDP) && !(opts1 & (RxUDPF | RxIPF))))
                        skb->ip_summed = CHECKSUM_UNNECESSARY;
                else
                        skb->ip_summed = CHECKSUM_NONE;
        } else {
                /* rx csum offload for RTL8102E */
                if (((opts2 & RxV4F) && !(opts1 & RxIPF)) || (opts2 & RxV6F)) {
                        if (((opts1 & RxTCPT) && !(opts1 & RxTCPF)) ||
                            ((opts1 & RxUDPT) && !(opts1 & RxUDPF)))
                                skb->ip_summed = CHECKSUM_UNNECESSARY;
                        else
                                skb->ip_summed = CHECKSUM_NONE;
                } else
                        skb->ip_summed = CHECKSUM_NONE;
        }
}

static inline int
rtl8101_try_rx_copy(struct rtl8101_private *tp,
                    struct sk_buff **sk_buff,
                    int pkt_size,
                    struct RxDesc *desc,
                    int rx_buf_sz)
{
        int ret = -1;

        if (pkt_size < rx_copybreak) {
                struct sk_buff *skb;

                skb = RTL_ALLOC_SKB_INTR(tp, pkt_size + NET_IP_ALIGN);
                if (skb) {
                        u8 *data;

                        data = sk_buff[0]->data;
                        skb_reserve(skb, NET_IP_ALIGN);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
                        prefetch(data - NET_IP_ALIGN);
#endif
                        eth_copy_and_sum(skb, data, pkt_size, 0);
                        *sk_buff = skb;
                        rtl8101_mark_to_asic(desc, rx_buf_sz);
                        ret = 0;
                }
        }
        return ret;
}

static inline void
rtl8101_rx_skb(struct rtl8101_private *tp,
               struct sk_buff *skb)
{
#ifdef CONFIG_R8101_NAPI
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
        netif_receive_skb(skb);
#else
        napi_gro_receive(&tp->napi, skb);
#endif
#else
        netif_rx(skb);
#endif
}

static int
rtl8101_rx_interrupt(struct net_device *dev,
                     struct rtl8101_private *tp,
                     napi_budget budget)
{
        unsigned int cur_rx, rx_left;
        unsigned int delta, count = 0;
        unsigned int entry;
        struct RxDesc *desc;
        u32 status;
        u32 rx_quota;

        assert(dev != NULL);
        assert(tp != NULL);

        if (tp->RxDescArray == NULL)
                goto rx_out;

        rx_quota = RTL_RX_QUOTA(budget);
        cur_rx = tp->cur_rx;
        entry = cur_rx % tp->num_rx_desc;
        desc = tp->RxDescArray + entry;
        rx_left = tp->num_rx_desc + tp->dirty_rx - cur_rx;
        rx_left = rtl8101_rx_quota(rx_left, (u32) rx_quota);

        for (; rx_left > 0; rx_left--) {
                status = le32_to_cpu(desc->opts1);
                if (status & DescOwn)
                        break;

                rmb();

                if (unlikely(status & RxRES)) {
                        if (netif_msg_rx_err(tp)) {
                                printk(KERN_INFO
                                       "%s: Rx ERROR. status = %08x\n",
                                       dev->name, status);
                        }
                        RTLDEV->stats.rx_errors++;

                        if (status & (RxRWT | RxRUNT))
                                RTLDEV->stats.rx_length_errors++;
                        if (status & RxCRC)
                                RTLDEV->stats.rx_crc_errors++;
                        if (dev->features & NETIF_F_RXALL)
                                goto process_pkt;
                        rtl8101_mark_to_asic(desc, tp->rx_buf_sz);
                } else {
                        struct sk_buff *skb;
                        int pkt_size;

process_pkt:
                        pkt_size = status & 0x00003fff;
                        if (likely(!(dev->features & NETIF_F_RXFCS)))
                                pkt_size -= ETH_FCS_LEN;

                        /*
                         * The driver does not support incoming fragmented
                         * frames. They are seen as a symptom of over-mtu
                         * sized frames.
                         */
                        if (unlikely(rtl8101_fragmented_frame(status)) ||
                            unlikely(pkt_size > tp->rx_buf_sz)) {
                                RTLDEV->stats.rx_dropped++;
                                RTLDEV->stats.rx_length_errors++;
                                rtl8101_mark_to_asic(desc, tp->rx_buf_sz);
                                goto release_descriptor;
                        }

                        skb = tp->Rx_skbuff[entry];

                        dma_sync_single_for_cpu(&tp->pci_dev->dev,
                                                le64_to_cpu(desc->addr), tp->rx_buf_sz,
                                                DMA_FROM_DEVICE);

                        if (rtl8101_try_rx_copy(tp, &skb, pkt_size,
                                                desc, tp->rx_buf_sz)) {
                                tp->Rx_skbuff[entry] = NULL;
                                dma_unmap_single(&tp->pci_dev->dev, le64_to_cpu(desc->addr),
                                                 tp->rx_buf_sz, DMA_FROM_DEVICE);
                        } else {
                                dma_sync_single_for_device(&tp->pci_dev->dev, le64_to_cpu(desc->addr),
                                                           tp->rx_buf_sz, DMA_FROM_DEVICE);
                        }

                        if (tp->cp_cmd & RxChkSum)
                                rtl8101_rx_csum(tp, skb, desc);

                        skb->dev = dev;
                        skb_put(skb, pkt_size);
                        skb->protocol = eth_type_trans(skb, dev);

                        if (skb->pkt_type == PACKET_MULTICAST)
                                RTLDEV->stats.multicast++;

                        if (rtl8101_rx_vlan_skb(tp, desc, skb) < 0)
                                rtl8101_rx_skb(tp, skb);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
                        dev->last_rx = jiffies;
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
                        RTLDEV->stats.rx_bytes += pkt_size;
                        RTLDEV->stats.rx_packets++;
                }

release_descriptor:
                cur_rx++;
                entry = cur_rx % tp->num_rx_desc;
                desc = tp->RxDescArray + entry;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,37)
                prefetch(desc);
#endif
        }

        count = cur_rx - tp->cur_rx;
        tp->cur_rx = cur_rx;

        delta = rtl8101_rx_fill(tp, dev, tp->dirty_rx, tp->cur_rx, 1);
        if (!delta && count && netif_msg_intr(tp))
                printk(KERN_INFO "%s: no Rx buffer allocated\n", dev->name);
        tp->dirty_rx += delta;

        /*
         * FIXME: until there is periodic timer to try and refill the ring,
         * a temporary shortage may definitely kill the Rx process.
         * - disable the asic to try and avoid an overflow and kick it again
         *   after refill ?
         * - how do others driver handle this condition (Uh oh...).
         */
        if ((tp->dirty_rx + tp->num_rx_desc == tp->cur_rx) && netif_msg_intr(tp))
                printk(KERN_EMERG "%s: Rx buffers exhausted\n", dev->name);

rx_out:

        return count;
}

/* The interrupt handler does all of the Rx thread work and cleans up after the Tx thread. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8101_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t rtl8101_interrupt(int irq, void *dev_instance)
#endif
{
        struct net_device *dev = (struct net_device *) dev_instance;
        struct rtl8101_private *tp = netdev_priv(dev);
        int status;
        int handled = 0;

        do {
                status = RTL_R16(tp, IntrStatus);

                if(!(tp->features & (RTL_FEATURE_MSI | RTL_FEATURE_MSIX))) {
                        /* hotplug/major error/no more work/shared irq */
                        if ((status == 0xFFFF) || !status)
                                break;

                        if (!(status & (tp->intr_mask | PCSTimeout)))
                                break;
                }

                handled = 1;

                rtl8101_disable_hw_interrupt(tp);

                RTL_W16(tp, IntrStatus, status);

#ifdef CONFIG_R8101_NAPI
                if (status & tp->intr_mask || tp->keep_intr_cnt-- > 0) {
                        if (status & tp->intr_mask)
                                tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;

                        if (likely(RTL_NETIF_RX_SCHEDULE_PREP(dev, &tp->napi)))
                                __RTL_NETIF_RX_SCHEDULE(dev, &tp->napi);
                        else if (netif_msg_intr(tp))
                                printk(KERN_INFO "%s: interrupt %04x in poll\n",
                                       dev->name, status);
                } else {
                        tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;
                        rtl8101_switch_to_hw_interrupt(tp);
                }
#else
                if (status & tp->intr_mask || tp->keep_intr_cnt-- > 0) {
                        u32 budget = ~(u32)0;

                        if (status & tp->intr_mask)
                                tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;

                        rtl8101_tx_interrupt(dev, tp);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
                        rtl8101_rx_interrupt(dev, tp, &budget);
#else
                        rtl8101_rx_interrupt(dev, tp, budget);
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)

                        rtl8101_switch_to_timer_interrupt(tp);
                } else {
                        tp->keep_intr_cnt = RTK_KEEP_INTERRUPT_COUNT;
                        rtl8101_switch_to_hw_interrupt(tp);
                }
#endif
        } while (false);

        return IRQ_RETVAL(handled);
}

#ifdef CONFIG_R8101_NAPI
static int rtl8101_poll(napi_ptr napi, napi_budget budget)
{
        struct rtl8101_private *tp = RTL_GET_PRIV(napi, struct rtl8101_private);
        RTL_GET_NETDEV(tp)
        unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
        unsigned int work_done;
        unsigned long flags;

        spin_lock_irqsave(&tp->lock, flags);
        rtl8101_tx_interrupt(dev, tp);
        spin_unlock_irqrestore(&tp->lock, flags);

        work_done = rtl8101_rx_interrupt(dev, tp, budget);

        RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

        if (work_done < work_to_do) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
                if (RTL_NETIF_RX_COMPLETE(dev, napi, work_done) == FALSE) return RTL_NAPI_RETURN_VALUE;
#else
                RTL_NETIF_RX_COMPLETE(dev, napi, work_done);
#endif
                /*
                 * 20040426: the barrier is not strictly required but the
                 * behavior of the irq handler could be less predictable
                 * without it. Btw, the lack of flush for the posted pci
                 * write is safe - FR
                 */
                smp_wmb();
                rtl8101_switch_to_timer_interrupt(tp);
        }

        return RTL_NAPI_RETURN_VALUE;
}
#endif//CONFIG_R8101_NAPI

static void
rtl8101_down(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        rtl8101_delete_link_timer(dev, &tp->link_timer);

        rtl8101_delete_esd_timer(dev, &tp->esd_timer);

#ifdef CONFIG_R8101_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        RTL_NAPI_DISABLE(dev, &tp->napi);
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#endif

        netif_stop_queue(dev);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
        /* Give a racing hard_start_xmit a few cycles to complete. */
        synchronize_rcu();  /* FIXME: should this be synchronize_irq()? */
#endif

        spin_lock_irqsave(&tp->lock, flags);

        netif_carrier_off(dev);

        rtl8101_dsm(dev, DSM_IF_DOWN);

        rtl8101_hw_reset(dev);

        spin_unlock_irqrestore(&tp->lock, flags);

        synchronize_irq(tp->irq);

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_tx_clear(tp);

        rtl8101_rx_clear(tp);

        spin_unlock_irqrestore(&tp->lock, flags);
}

static int rtl8101_close(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        struct pci_dev *pdev = tp->pci_dev;
        u8 options;
        unsigned long flags;

        options = RTL_R8(tp, Config1);
        if (((tp->mcfg == CFG_METHOD_4) || (tp->mcfg == CFG_METHOD_5)) &&
            !(options & PMEnable)) {
                RTL_W8(tp, Config4, RTL_R8(tp, Config4) | (1 << 0));
                RTL_W8(tp, DBG_reg, RTL_R8(tp, DBG_reg) | (1 << 3));
                RTL_W8(tp, PMCH, RTL_R8(tp, PMCH) & ~BIT_7);
                RTL_W8(tp, CPlusCmd, RTL_R8(tp, CPlusCmd) | (1 << 1));
        }

        if (tp->TxDescArray!=NULL && tp->RxDescArray!=NULL) {
                rtl8101_cancel_schedule_work(dev);

                rtl8101_down(dev);

                pci_clear_master(tp->pci_dev);

                spin_lock_irqsave(&tp->lock, flags);

                rtl8101_hw_d3_para(dev);

                rtl8101_powerdown_pll(dev);

                spin_unlock_irqrestore(&tp->lock, flags);

                free_irq(tp->irq, dev);

                dma_free_coherent(&pdev->dev,
                                  tp->RxDescAllocSize,
                                  tp->RxDescArray,
                                  tp->RxPhyAddr);

                dma_free_coherent(&pdev->dev,
                                  tp->TxDescAllocSize,
                                  tp->TxDescArray,
                                  tp->TxPhyAddr);

                tp->TxDescArray = NULL;
                tp->RxDescArray = NULL;
        }

        return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
static void rtl8101_shutdown(struct pci_dev *pdev)
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8101_private *tp = netdev_priv(dev);

        rtl8101_set_bios_setting(dev);
        if (s5_keep_curr_mac == 0 && tp->random_mac == 0)
                rtl8101_rar_set(tp, tp->org_mac_addr);

        if (s5wol == 0)
                tp->wol_enabled = WOL_DISABLED;

        rtl8101_close(dev);
        rtl8101_disable_msi(pdev, tp);

        if (system_state == SYSTEM_POWER_OFF) {
                pci_clear_master(tp->pci_dev);
                pci_wake_from_d3(pdev, tp->wol_enabled);
                pci_set_power_state(pdev, PCI_D3hot);
        }
}
#endif

/**
 *  rtl8101_get_stats - Get rtl8101 read/write statistics
 *  @dev: The Ethernet Device to get statistics for
 *
 *  Get TX/RX statistics for rtl8101
 */
static struct net_device_stats *
rtl8101_get_stats(struct net_device *dev)
{
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;

        if (netif_running(dev)) {
                spin_lock_irqsave(&tp->lock, flags);
//		tp->stats.rx_missed_errors += RTL_R32(tp, RxMissed);
//		RTL_W32(tp, RxMissed, 0);
                spin_unlock_irqrestore(&tp->lock, flags);
        }

        return &RTLDEV->stats;
}

#ifdef CONFIG_PM

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static int
rtl8101_suspend(struct pci_dev *pdev,
                u32 state)
#else
static int
rtl8101_suspend(struct pci_dev *pdev,
                pm_message_t state)
#endif
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8101_private *tp = netdev_priv(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        u32 pci_pm_state = pci_choose_state(pdev, state);
#endif
        unsigned long flags;


        if (!netif_running(dev))
                goto out;

        rtl8101_cancel_schedule_work(dev);

        rtl8101_delete_esd_timer(dev, &tp->esd_timer);

        rtl8101_delete_link_timer(dev, &tp->link_timer);

        netif_stop_queue(dev);

        netif_carrier_off(dev);

        rtl8101_dsm(dev, DSM_NIC_GOTO_D3);

        netif_device_detach(dev);

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_hw_reset(dev);

        pci_clear_master(pdev);

        rtl8101_hw_d3_para(dev);

        rtl8101_powerdown_pll(dev);

        spin_unlock_irqrestore(&tp->lock, flags);

out:

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        pci_save_state(pdev, &pci_pm_state);
#else
        pci_save_state(pdev);
#endif
        pci_enable_wake(pdev, pci_choose_state(pdev, state), tp->wol_enabled);
        pci_set_power_state(pdev, pci_choose_state(pdev, state));

        return 0;
}

static int
rtl8101_resume(struct pci_dev *pdev)
{
        struct net_device *dev = pci_get_drvdata(pdev);
        struct rtl8101_private *tp = netdev_priv(dev);
        unsigned long flags;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        u32 pci_pm_state = PCI_D0;
#endif

        pci_set_power_state(pdev, PCI_D0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
        pci_restore_state(pdev, &pci_pm_state);
#else
        pci_restore_state(pdev);
#endif
        pci_enable_wake(pdev, PCI_D0, 0);

        spin_lock_irqsave(&tp->lock, flags);

        /* restore last modified mac address */
        rtl8101_rar_set(tp, dev->dev_addr);

        spin_unlock_irqrestore(&tp->lock, flags);

        if (!netif_running(dev))
                goto out;

        pci_set_master(pdev);

        spin_lock_irqsave(&tp->lock, flags);

        rtl8101_exit_oob(dev);

        rtl8101_dsm(dev, DSM_NIC_RESUME_D3);

        rtl8101_hw_init(dev);

        rtl8101_powerup_pll(dev);

        rtl8101_hw_ephy_config(dev);

        rtl8101_hw_phy_config(dev);

        rtl8101_hw_config(dev);

        spin_unlock_irqrestore(&tp->lock, flags);

        rtl8101_schedule_work(dev, rtl8101_reset_task);

        netif_device_attach(dev);

        mod_timer(&tp->esd_timer, jiffies + RTL8101_ESD_TIMEOUT);
        mod_timer(&tp->link_timer, jiffies + RTL8101_LINK_TIMEOUT);

out:
        return 0;
}

#endif /* CONFIG_PM */

static struct pci_driver rtl8101_pci_driver = {
        .name		= MODULENAME,
        .id_table	= rtl8101_pci_tbl,
        .probe		= rtl8101_init_one,
        .remove		= __devexit_p(rtl8101_remove_one),
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
        .shutdown	= rtl8101_shutdown,
#endif
#ifdef CONFIG_PM
        .suspend	= rtl8101_suspend,
        .resume		= rtl8101_resume,
#endif
};

static int __init
rtl8101_init_module(void)
{
#ifdef ENABLE_R8101_PROCFS
        rtl8101_proc_module_init();
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        return pci_register_driver(&rtl8101_pci_driver);
#else
        return pci_module_init(&rtl8101_pci_driver);
#endif
}

static void __exit
rtl8101_cleanup_module(void)
{
        pci_unregister_driver(&rtl8101_pci_driver);
#ifdef ENABLE_R8101_PROCFS
        if (rtl8101_proc) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                remove_proc_subtree(MODULENAME, init_net.proc_net);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
                remove_proc_entry(MODULENAME, init_net.proc_net);
#else
                remove_proc_entry(MODULENAME, proc_net);
#endif  //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#endif  //LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
                rtl8101_proc = NULL;
        }
#endif
}

module_init(rtl8101_init_module);
module_exit(rtl8101_cleanup_module);
