/*

 * Copyright (c) 2008, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <net/sch_generic.h>
#include <linux/if_pppox.h>
#include <linux/ip.h>

#include "ag7240.h"
#include "ag7240_phy.h"
#include "ag7240_trc.h"

ag7240_mac_t *ag7240_macs[2];
static void ag7240_hw_setup(ag7240_mac_t *mac);
static void ag7240_hw_stop(ag7240_mac_t *mac);
static void ag7240_oom_timer(unsigned long data);
static int  check_for_dma_status(ag7240_mac_t *mac,int ac);
static int  ag7240_tx_alloc(ag7240_mac_t *mac);
static int  ag7240_rx_alloc(ag7240_mac_t *mac);
static void ag7240_rx_free(ag7240_mac_t *mac);
static void ag7240_tx_free(ag7240_mac_t *mac);
static int  ag7240_ring_alloc(ag7240_ring_t *r, int count);
static int  ag7240_rx_replenish(ag7240_mac_t *mac);
static void ag7240_get_default_macaddr(ag7240_mac_t *mac, u8 *mac_addr);
static int  ag7240_tx_reap(ag7240_mac_t *mac,int ac);
static void ag7240_ring_release(ag7240_mac_t *mac, ag7240_ring_t  *r);
static void ag7240_ring_free(ag7240_ring_t *r);
static void ag7240_tx_timeout_task(struct work_struct *work);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
static int ag7240_poll(struct napi_struct *napi, int budget);
#else
static int ag7240_poll(struct net_device *dev, int *budget);
#endif
#define ETH_VLAN_HLEN 18
#ifdef CONFIG_AR7240_S26_VLAN_IGMP
int athr_ioctl(struct net_device *dev,uint32_t *args, int cmd);
#else
int athr_ioctl(uint32_t *args, int cmd);
#endif
void ar7240_s26_intr(void);
void ag7240_dma_reset(ag7240_mac_t *mac);

int  ag7240_recv_packets(struct net_device *dev, ag7240_mac_t *mac,
    int max_work, int *work_done);
static irqreturn_t ag7240_intr(int cpl, void *dev_id);
static irqreturn_t ag7240_link_intr(int cpl, void *dev_id);
static struct sk_buff * ag7240_buffer_alloc(void);
#ifdef ETH_SOFT_LED
ATH_LED_CONTROL    PLedCtrl;
atomic_t Ledstatus;
#endif

extern uint32_t ar7240_ahb_freq;
extern void athrs26_reg_dev(ag7240_mac_t **mac);
extern void athrs26_enable_linkIntrs(int ethUnit);
extern void athrs26_disable_linkIntrs(int ethUnit);
extern int athrs26_phy_is_link_alive(int phyUnit);
extern void athrs26_phy_stab_wr(int phy_id, int phy_up, int phy_speed);
extern uint32_t athrs26_reg_read(unsigned int s26_addr);
extern void athrs26_reg_write(unsigned int s26_addr, unsigned int s26_write_data);
extern void s26_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);

char *mii_str[2][4] = {
    {"GMii", "Mii", "RGMii", "RMii"},
    {"GMii","Mii","RGMii", "RMii"}
};
int rg_phy_speed = -1 , rg_phy_duplex = -1;
char *spd_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
char *dup_str[] = {"half duplex", "full duplex"};

#define MODULE_NAME "AG7240"
MODULE_LICENSE("Dual BSD/GPL");

static uint32_t prev_dma_chk_ts;
/* if 0 compute in init */
int tx_len_per_ds = 0;
int phy_in_reset = 0;
int enet_ac = 0;
module_param(tx_len_per_ds, int, 0);
MODULE_PARM_DESC(tx_len_per_ds, "Size of DMA chunk");

/* if 0 compute in init */
int tx_max_desc_per_ds_pkt=0;

/* if 0 compute in init */
int fifo_3 = 0x1f00140;
module_param(fifo_3, int, 0);
MODULE_PARM_DESC(fifo_3, "fifo cfg 3 settings");

int mii0_if = AG7240_MII0_INTERFACE;
module_param(mii0_if, int, 0);
MODULE_PARM_DESC(mii0_if, "mii0 connect");

int mii1_if = AG7240_MII1_INTERFACE;
module_param(mii1_if, int, 0);
MODULE_PARM_DESC(mii1_if, "mii1 connect");

#define SW_PLL 0x1f000000ul
int gige_pll = 0x1a000000;
module_param(gige_pll, int, 0);
MODULE_PARM_DESC(gige_pll, "Pll for (R)GMII if");

int fifo_5 = 0xbefff;
module_param(fifo_5, int, 0);
MODULE_PARM_DESC(fifo_5, "fifo cfg 5 settings");

int xmii_val = 0x16000000;

int ignore_packet_inspection = 0;

void set_packet_inspection_flag(int flag)
{
	if(flag==0)
		ignore_packet_inspection = 0;
	else
		ignore_packet_inspection = 1;
}

#define addr_to_words(addr, w1, w2)  {                                 \
    w1 = (addr[5] << 24) | (addr[4] << 16) | (addr[3] << 8) | addr[2]; \
    w2 = (addr[1] << 24) | (addr[0] << 16) | 0;                        \
}

/*
 * Defines specific to this implemention
 */

#ifndef CONFIG_AG7240_LEN_PER_TX_DS
#error Please run menuconfig and define CONFIG_AG7240_LEN_PER_TX_DS
#endif

#ifndef CONFIG_AG7240_NUMBER_TX_PKTS
#error Please run menuconfig and define CONFIG_AG7240_NUMBER_TX_PKTS
#endif

#ifndef CONFIG_AG7240_NUMBER_RX_PKTS
#error Please run menuconfig and define CONFIG_AG7240_NUMBER_RX_PKTS
#endif
#define AG7240_TX_FIFO_LEN          2048
#define AG7240_TX_MIN_DS_LEN        128
#define AG7240_TX_MAX_DS_LEN        AG7240_TX_FIFO_LEN

#define AG7240_TX_MTU_LEN           AG71XX_TX_MTU_LEN

#define AG7240_TX_DESC_CNT          CONFIG_AG7240_NUMBER_TX_PKTS*tx_max_desc_per_ds_pkt
#define AG7240_TX_REAP_THRESH       AG7240_TX_DESC_CNT/2
#define AG7240_TX_QSTART_THRESH     4*tx_max_desc_per_ds_pkt

#define AG7240_RX_DESC_CNT          CONFIG_AG7240_NUMBER_RX_PKTS

#define AG7240_NAPI_WEIGHT          64
#define AG7240_PHY_POLL_SECONDS     2
#define AG7240_LED_POLL_SECONDS    (HZ/10)

#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0


static inline int ag7240_tx_reap_thresh(ag7240_mac_t *mac,int ac)
{
    ag7240_ring_t *r = &mac->mac_txring[ac];

    return (ag7240_ndesc_unused(mac, r) < AG7240_TX_REAP_THRESH);
}

static inline int ag7240_tx_ring_full(ag7240_mac_t *mac,int ac)
{
    ag7240_ring_t *r = &mac->mac_txring[ac];

    ag7240_trc_new(ag7240_ndesc_unused(mac, r),"tx ring full");
    return (ag7240_ndesc_unused(mac, r) < tx_max_desc_per_ds_pkt + 2);
}
static int
ag7240_open(struct net_device *dev)
{
    unsigned int w1 = 0, w2 = 0;
    ag7240_mac_t *mac = (ag7240_mac_t *)netdev_priv(dev);
    int st,flags;
    uint32_t mask;

#ifdef SWITCH_AHB_FREQ
    u32 tmp_pll, pll;
#endif

    assert(mac);
    if (mac_has_flag(mac,WAN_QOS_SOFT_CLASS))
        mac->mac_noacs = 4;
    else
        mac->mac_noacs = 1;

    st = request_irq(mac->mac_irq, ag7240_intr, 0, dev->name, dev);
    if (st < 0)
    {
        printk(MODULE_NAME ": request irq %d failed %d\n", mac->mac_irq, st);
        return 1;
    }
    if (ag7240_tx_alloc(mac)) goto tx_failed;
    if (ag7240_rx_alloc(mac)) goto rx_failed;

    udelay(20);
    mask = ag7240_reset_mask(mac->mac_unit);

    /*
    * put into reset, hold, pull out.
    */
    spin_lock_irqsave(&mac->mac_lock, flags);
    ar7240_reg_rmw_set(AR7240_RESET, mask);
    udelay(10);
    ar7240_reg_rmw_clear(AR7240_RESET, mask);
    udelay(10);
    spin_unlock_irqrestore(&mac->mac_lock, flags);

    ag7240_hw_setup(mac);
#ifdef CONFIG_AR7242_VIR_PHY
 #ifndef SWITCH_AHB_FREQ
    u32 tmp_pll ;
 #endif
    tmp_pll = 0x62000000 ;
    ar7240_reg_wr_nf(AR7242_ETH_XMII_CONFIG, tmp_pll);
    udelay(100*1000);
#endif

    mac->mac_speed = -1;
    printk(KERN_INFO "reg init for %d\n",mac->mac_unit);
    ag7240_phy_reg_init(mac->mac_unit);


    printk("Setting PHY...\n");

    if (mac_has_flag(mac,ETH_SOFT_LED)) {
    /* Resetting PHY will disable MDIO access needed by soft LED task.
     * Hence, Do not reset PHY until Soft LED task get completed.
     */
        while(atomic_read(&Ledstatus) == 1);
    }
    phy_in_reset = 1;
#ifndef CONFIG_AR7242_VIR_PHY
     ag7240_phy_setup(mac->mac_unit);
#else
     athr_vir_phy_setup(mac->mac_unit);
#endif 
    phy_in_reset = 0;

#ifdef SWITCH_AHB_FREQ
    ar7240_reg_wr_nf(AR7240_PLL_CONFIG, pll);
    udelay(100*1000);
#endif
    /*
    * set the mac addr
    */
    addr_to_words(dev->dev_addr, w1, w2);
    ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR1, w1);
    ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR2, w2);

    dev->trans_start = jiffies;

    /*
     * Keep carrier off while initialization and switch it once the link is up.
     */
    netif_carrier_off(dev);
    napi_enable(&mac->mac_napi);

 
    mac->mac_ifup = 1;
    ag7240_int_enable(mac);

        ag7240_intr_enable_rxovf(mac);

#if defined(CONFIG_AR7242_RGMII_PHY)||defined(CONFIG_AR7242_S16_PHY)||defined(CONFIG_AR7242_VIR_PHY) || defined(CONFIG_AR7242_RTL8309G_PHY)
    
    if(is_ar7242() && mac->mac_unit == 0) {

        init_timer(&mac->mac_phy_timer);
        mac->mac_phy_timer.data     = (unsigned long)mac;
        mac->mac_phy_timer.function = (void *)ag7242_check_link;
        ag7242_check_link(mac);

    }
#endif
    
    if (is_ar7240() || is_ar7241() || is_ar933x() || (is_ar7242() && mac->mac_unit == 1))
	athrs26_enable_linkIntrs(mac->mac_unit);

    ag7240_rx_start(mac);	
    netif_start_queue(dev);
   
    return 0;

rx_failed:
    ag7240_tx_free(mac);
tx_failed:
    free_irq(mac->mac_irq, dev);
    return 1;
}

static int
ag7240_stop(struct net_device *dev)
{
    ag7240_mac_t *mac = (ag7240_mac_t *)netdev_priv(dev);
    int flags,i;

    spin_lock_irqsave(&mac->mac_lock, flags);
    mac->mac_ifup = 0;
    napi_disable(&mac->mac_napi);
    netif_carrier_off(dev);
    netif_stop_queue(dev);

    ag7240_hw_stop(mac);
    free_irq(mac->mac_irq, dev);


    ag7240_tx_free(mac);
    ag7240_rx_free(mac);

/* During interface up on PHY reset the link status will be updated.
 * Clearing the software link status while bringing the interface down
 * will avoid the race condition during PHY RESET.
 */
    if (mac_has_flag(mac,ETH_SOFT_LED)) {
        if (mac->mac_unit == ENET_UNIT_LAN) {
            for(i = 0;i < 4; i++)
                PLedCtrl.ledlink[i] = 0;
        }
        else {
            PLedCtrl.ledlink[4] = 0;
        }
    }
    if (is_ar7242())
        del_timer(&mac->mac_phy_timer);
    spin_unlock_irqrestore(&mac->mac_lock, flags);

    /*ag7240_trc_dump();*/

    return 0;
}

#define FIFO_CFG0_WTM		BIT(0)	/* Watermark Module */
#define FIFO_CFG0_RXS		BIT(1)	/* Rx System Module */
#define FIFO_CFG0_RXF		BIT(2)	/* Rx Fabric Module */
#define FIFO_CFG0_TXS		BIT(3)	/* Tx System Module */
#define FIFO_CFG0_TXF		BIT(4)	/* Tx Fabric Module */
#define FIFO_CFG0_ALL	(FIFO_CFG0_WTM | FIFO_CFG0_RXS | FIFO_CFG0_RXF \
			| FIFO_CFG0_TXS | FIFO_CFG0_TXF)

#define FIFO_CFG0_ENABLE_SHIFT	8

#define FIFO_CFG4_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG4_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG4_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG4_CE		BIT(3)	/* Code Error */
#define FIFO_CFG4_CR		BIT(4)	/* CRC error */
#define FIFO_CFG4_LM		BIT(5)	/* Length Mismatch */
#define FIFO_CFG4_LO		BIT(6)	/* Length out of range */
#define FIFO_CFG4_OK		BIT(7)	/* Packet is OK */
#define FIFO_CFG4_MC		BIT(8)	/* Multicast Packet */
#define FIFO_CFG4_BC		BIT(9)	/* Broadcast Packet */
#define FIFO_CFG4_DR		BIT(10)	/* Dribble */
#define FIFO_CFG4_LE		BIT(11)	/* Long Event */
#define FIFO_CFG4_CF		BIT(12)	/* Control Frame */
#define FIFO_CFG4_PF		BIT(13)	/* Pause Frame */
#define FIFO_CFG4_UO		BIT(14)	/* Unsupported Opcode */
#define FIFO_CFG4_VT		BIT(15)	/* VLAN tag detected */
#define FIFO_CFG4_FT		BIT(16)	/* Frame Truncated */
#define FIFO_CFG4_UC		BIT(17)	/* Unicast Packet */

#define FIFO_CFG5_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG5_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG5_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG5_CE		BIT(3)	/* Code Error */
#define FIFO_CFG5_LM		BIT(4)	/* Length Mismatch */
#define FIFO_CFG5_LO		BIT(5)	/* Length Out of Range */
#define FIFO_CFG5_OK		BIT(6)	/* Packet is OK */
#define FIFO_CFG5_MC		BIT(7)	/* Multicast Packet */
#define FIFO_CFG5_BC		BIT(8)	/* Broadcast Packet */
#define FIFO_CFG5_DR		BIT(9)	/* Dribble */
#define FIFO_CFG5_CF		BIT(10)	/* Control Frame */
#define FIFO_CFG5_PF		BIT(11)	/* Pause Frame */
#define FIFO_CFG5_UO		BIT(12)	/* Unsupported Opcode */
#define FIFO_CFG5_VT		BIT(13)	/* VLAN tag detected */
#define FIFO_CFG5_LE		BIT(14)	/* Long Event */
#define FIFO_CFG5_FT		BIT(15)	/* Frame Truncated */
#define FIFO_CFG5_16		BIT(16)	/* unknown */
#define FIFO_CFG5_17		BIT(17)	/* unknown */
#define FIFO_CFG5_SF		BIT(18)	/* Short Frame */
#define FIFO_CFG5_BM		BIT(19)	/* Byte Mode */


#define FIFO_CFG0_INIT	(FIFO_CFG0_ALL << FIFO_CFG0_ENABLE_SHIFT)

#define FIFO_CFG4_INIT	(FIFO_CFG4_DE | FIFO_CFG4_DV | FIFO_CFG4_FC | \
			 FIFO_CFG4_CE | FIFO_CFG4_CR | FIFO_CFG4_LM | \
			 FIFO_CFG4_LO | FIFO_CFG4_OK | FIFO_CFG4_MC | \
			 FIFO_CFG4_BC | FIFO_CFG4_DR | FIFO_CFG4_LE | \
			 FIFO_CFG4_CF | FIFO_CFG4_PF | FIFO_CFG4_UO | \
			 FIFO_CFG4_VT)

#define FIFO_CFG5_INIT	(FIFO_CFG5_DE | FIFO_CFG5_DV | FIFO_CFG5_FC | \
			 FIFO_CFG5_CE | FIFO_CFG5_LO | FIFO_CFG5_OK | \
			 FIFO_CFG5_MC | FIFO_CFG5_BC | FIFO_CFG5_DR | \
			 FIFO_CFG5_CF | FIFO_CFG5_PF | FIFO_CFG5_VT | \
			 FIFO_CFG5_LE | FIFO_CFG5_FT | FIFO_CFG5_16 | \
			 FIFO_CFG5_17 | FIFO_CFG5_SF)



static void
ag7240_hw_setup(ag7240_mac_t *mac)
{
    ag7240_ring_t *tx, *rx = &mac->mac_rxring;
    ag7240_desc_t *r0, *t0;
    uint32_t mgmt_cfg_val,ac;
    u32 check_cnt=0;

#ifdef CONFIG_AR7242_RTL8309G_PHY
    ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN | AG7240_MAC_CFG1_TX_EN));
    ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_PAD_CRC_EN | AG7240_MAC_CFG2_LEN_CHECK | AG7240_MAC_CFG2_IF_10_100));
#else
    if(mac->mac_unit)
    {
        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN | AG7240_MAC_CFG1_TX_EN));
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_PAD_CRC_EN | AG7240_MAC_CFG2_IF_1000 | AG7240_MAC_CFG2_LEN_CHECK));
    } 
    else 
    {
        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN | AG7240_MAC_CFG1_TX_EN));
//        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN | AG7240_MAC_CFG1_TX_EN));
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_PAD_CRC_EN | AG7240_MAC_CFG2_LEN_CHECK));
    }
#endif
    ag7240_reg_wr(mac, AG71XX_REG_MAC_MFL, AG71XX_TX_MTU_LEN);

    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_0, 0x1f00);
//    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_0, FIFO_CFG0_INIT);

    if (mac_has_flag(mac,ATHR_S26_HEADER) || mac_has_flag(mac,ATHR_S16_HEADER))
        ag7240_reg_rmw_clear(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_LEN_CHECK)
    /*
    * set the mii if type - NB reg not in the gigE space
    */
     if ((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR7240_REV_1_2) {
        mgmt_cfg_val = 0x2;
        if (mac->mac_unit == 0) {
            ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
            ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
        }
    }
    else {
        switch (ar7240_ahb_freq/1000000) {
            case 150:
                     mgmt_cfg_val = 0x7;
                     break;
            case 175: 
                     mgmt_cfg_val = 0x5;
                     break;
            case 200: 
                     mgmt_cfg_val = 0x4;
                     break;
            case 210: 
                      mgmt_cfg_val = 0x9;
                      break;
            case 220: 
                      mgmt_cfg_val = 0x9;
                      break;
            case 40:
            case 24:
                /* hornet emulation...ahb is either 24 or 40Mhz */
                mgmt_cfg_val = 0x6;
            default:
                     mgmt_cfg_val = 0x7;
        }

#ifdef CONFIG_AR7242_RTL8309G_PHY
		mgmt_cfg_val = 0x4;
     if ( is_ar7242() && (mac->mac_unit == 0)) {
		ar7240_reg_rmw_set(AG7240_ETH_CFG, AG7240_ETH_CFG_MII_GE0 | AG7240_ETH_CFG_MII_GE0_SLAVE);
		ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
		ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
	} else if(is_ar7242() && mac->mac_unit == 1) {
		ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
		ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
	}
#else

        if ((is_ar7241() || is_ar7242())) {

            /* External MII mode */
            if (mac->mac_unit == 0 && is_ar7242()) {
                mgmt_cfg_val = 0x6;
                ar7240_reg_rmw_set(AG7240_ETH_CFG, AG7240_ETH_CFG_RGMII_GE0); 
                ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
                ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
            }
            /* Virian */
            mgmt_cfg_val = 0x4;

            if (mac_has_flag(mac,ETH_SWONLY_MODE)) {
                ar7240_reg_rmw_set(AG7240_ETH_CFG, AG7240_ETH_CFG_SW_ONLY_MODE); 
                ag7240_reg_rmw_set(ag7240_macs[0], AG7240_MAC_CFG1, AG7240_MAC_CFG1_SOFT_RST);;
            }
            ag7240_reg_wr(ag7240_macs[1], AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
            ag7240_reg_wr(ag7240_macs[1], AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
            printk("Virian MDC CFG Value ==> %x\n",mgmt_cfg_val);
        }
#endif
        else
          if(is_ar933x()){
            ar7240_reg_wr(AG7240_ETH_CFG, AG7240_ETH_CFG_MII_GE0_SLAVE); 
            mgmt_cfg_val = 0xF;
            if (mac->mac_unit == 1) {
                while (check_cnt++ < 10) {
                    ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
                    ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
                    break;
                }
                if(check_cnt == 11)
                    printk("%s: MDC check failed (Hornet)\n", __func__);
            }
        }
        else { /* Python 1.0 & 1.1 */
            if (mac->mac_unit == 0) {
                check_cnt = 0;
                while (check_cnt++ < 10) {
                    ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val | (1 << 31));
                    ag7240_reg_wr(mac, AG7240_MAC_MII_MGMT_CFG, mgmt_cfg_val);
                    if(athrs26_mdc_check() == 0) 
                        break;
                }
                if(check_cnt == 11)
                    printk("%s: MDC check failed\n", __func__);
            }
        }
    }
        
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_1, 0x10ffff);
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_2, 0x015500aa);

    /*
     * Weed out junk frames (CRC errored, short collision'ed frames etc.)
     */
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_4, 0x3ffff);

    /*
     * Drop CRC Errors, Pause Frames ,Length Error frames, Truncated Frames
     * dribble nibble and rxdv error frames.
     */
    DPRINTF("Setting Drop CRC Errors, Pause Frames and Length Error frames \n");

#ifdef CONFIG_AR7242_RTL8309G_PHY
	if(is_ar7242() && (mac->mac_unit == 0)) {
		ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 18));
		ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
	} else {
		ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 18));

	}
#else
    if(mac_has_flag(mac,ATHR_S26_HEADER)){
        ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0xe6bc0);
    } else if (mac->mac_unit == 0 && is_ar7242()){
       ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0xe6be2);
    } else{
        ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, FIFO_CFG5_INIT);
    }
    if (mac->mac_unit == 0 && is_ar7242()){
       ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0xe6be2);
    }
#endif    
    if (mac_has_flag(mac,WAN_QOS_SOFT_CLASS)) {
    /* Enable Fixed priority */
#if 1
        ag7240_reg_wr(mac,AG7240_DMA_TX_ARB_CFG,AG7240_TX_QOS_MODE_WEIGHTED
                    | AG7240_TX_QOS_WGT_0(0x7)
                    | AG7240_TX_QOS_WGT_1(0x5) 
                    | AG7240_TX_QOS_WGT_2(0x3)
                    | AG7240_TX_QOS_WGT_3(0x1));
#else
        ag7240_reg_wr(mac,AG7240_DMA_TX_ARB_CFG,AG7240_TX_QOS_MODE_FIXED);
#endif
        for(ac = 0;ac < mac->mac_noacs; ac++) {
            tx = &mac->mac_txring[ac];
            t0  =  &tx->ring_desc[0];
            switch(ac) {
                case ENET_AC_VO:
                    ag7240_reg_wr(mac, AG7240_DMA_TX_DESC_Q0, ag7240_desc_dma_addr(tx, t0));
                    break;
                case ENET_AC_VI:
                    ag7240_reg_wr(mac, AG7240_DMA_TX_DESC_Q1, ag7240_desc_dma_addr(tx, t0));
                    break;
                case ENET_AC_BK:
                    ag7240_reg_wr(mac, AG7240_DMA_TX_DESC_Q2, ag7240_desc_dma_addr(tx, t0));
                    break;
                case ENET_AC_BE:
                    ag7240_reg_wr(mac, AG7240_DMA_TX_DESC_Q3, ag7240_desc_dma_addr(tx, t0));
                    break;
            }
        }
    }
    else {
        tx = &mac->mac_txring[0];
        t0  =  &tx->ring_desc[0];
        ag7240_reg_wr(mac, AG7240_DMA_TX_DESC_Q0, ag7240_desc_dma_addr(tx, t0));
    }
    r0  =  &rx->ring_desc[0];
    ag7240_reg_wr(mac, AG7240_DMA_RX_DESC, ag7240_desc_dma_addr(rx, r0));

    DPRINTF(MODULE_NAME ": cfg1 %#x cfg2 %#x\n", ag7240_reg_rd(mac, AG7240_MAC_CFG1),
        ag7240_reg_rd(mac, AG7240_MAC_CFG2));
}

static void
ag7240_hw_stop(ag7240_mac_t *mac)
{
    ag7240_rx_stop(mac);
    ag7240_tx_stop(mac);
    ag7240_int_disable(mac);
    athrs26_disable_linkIntrs(mac->mac_unit);
    /*
    * put everything into reset.
    * Dont Reset WAN MAC as we are using eth0 MDIO to access S26 Registers.
    */
    if(mac->mac_unit == 1 || is_ar7241() || is_ar7242())
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG1, AG7240_MAC_CFG1_SOFT_RST);
}

/*
 * Several fields need to be programmed based on what the PHY negotiated
 * Ideally we should quiesce everything before touching the pll, but:
 * 1. If its a linkup/linkdown, we dont care about quiescing the traffic.
 * 2. If its a single gigE PHY, this can only happen on lup/ldown.
 * 3. If its a 100Mpbs switch, the link will always remain at 100 (or nothing)
 * 4. If its a gigE switch then the speed should always be set at 1000Mpbs, 
 *    and the switch should provide buffering for slower devices.
 *
 * XXX Only gigE PLL can be changed as a parameter for now. 100/10 is hardcoded.
 * XXX Need defines for them -
 * XXX FIFO settings based on the mode
 */
extern void ar7100_set_gpio(int gpio, int val);
extern int ar7100_get_gpio(int gpio);


static void
ag7240_set_mac_from_link(ag7240_mac_t *mac, ag7240_phy_speed_t speed, int fdx)
{
    if(mac->mac_unit == 1)
	speed = AG7240_PHY_SPEED_1000T;

    mac->mac_speed =  speed;
    mac->mac_fdx   =  fdx;

    ag7240_set_mac_duplex(mac, fdx);
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_3, fifo_3);

#ifdef CONFIG_AR7242_RTL8309G_PHY
    switch (speed)
	{
		case AG7240_PHY_SPEED_1000T:
		case AG7240_PHY_SPEED_100TX:
//			printf("Speed is 100 or 100M \n");
			if(mac->mac_unit == 0) { //RTL8309G 100M only 
				ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
				ar7240_reg_wr(AR7242_ETH_XMII_CONFIG,0x0101);
			} else {
			    ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_IF_1000);
			}
			break;
		case AG7240_PHY_SPEED_10T:
//		    printf("Spped is 10M \n");
			ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
			if(mac->mac_unit == 0) { //RTL8309G 10M only
				ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
				ar7240_reg_wr(AR7242_ETH_XMII_CONFIG,0x1616);
			}
			break;
		default:
			printk(KERN_INFO "Invalid speed detected\n");
			return 0;

	}
#else
    switch (speed)
    {
    case AG7240_PHY_SPEED_1000T:
        ag7240_set_mac_if(mac, 1);
        ag7240_reg_rmw_set(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
        if (is_ar7242() &&( mac->mac_unit == 0))
            ar7240_reg_wr(AR7242_ETH_XMII_CONFIG,xmii_val);
        break;

    case AG7240_PHY_SPEED_100TX:
        ag7240_set_mac_if(mac, 0);
        ag7240_set_mac_speed(mac, 1);
        ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
        if (is_ar7242() &&( mac->mac_unit == 0))
            ar7240_reg_wr(AR7242_ETH_XMII_CONFIG,0x0101);
        break;

    case AG7240_PHY_SPEED_10T:
        ag7240_set_mac_if(mac, 0);
        ag7240_set_mac_speed(mac, 0);
        ag7240_reg_rmw_clear(mac, AG7240_MAC_FIFO_CFG_5, (1 << 19));
        if (is_ar7242() &&( mac->mac_unit == 0))
            ar7240_reg_wr(AR7242_ETH_XMII_CONFIG,0x1616);
        break;

    default:
        assert(0);
    }
#endif
    DPRINTF(MODULE_NAME ": cfg_1: %#x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_1));
    DPRINTF(MODULE_NAME ": cfg_2: %#x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_2));
    DPRINTF(MODULE_NAME ": cfg_3: %#x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_3));
    DPRINTF(MODULE_NAME ": cfg_4: %#x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_4));
    DPRINTF(MODULE_NAME ": cfg_5: %#x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_5));
}

static int 
led_control_func(ATH_LED_CONTROL *pledctrl) 
{
    uint32_t i=0,cnt,reg_addr;
    const LED_BLINK_RATES  *bRateTab; 
    static uint32_t pkt_count;
    ag7240_mac_t *mac;
    mac = ag7240_macs[1];

    if (mac_has_flag(mac,ETH_SOFT_LED)) {
        atomic_inc(&Ledstatus);

    atomic_inc(&Ledstatus);
  
    /* 
     *  MDIO access will fail While PHY is in RESET phase.
     */
    if(phy_in_reset)
          goto done;

    if (mac->mac_ifup) {
        for (i = 0 ; i < 4 ; i ++) {
            bRateTab = BlinkRateTable_100M;
            if (pledctrl->ledlink[i]) {   
                reg_addr = 0x2003c + ((i + 1) << 8);
                /* Rx good byte count */
                cnt = athrs26_reg_read(reg_addr);
                
                reg_addr = 0x20084 + ((i + 1) << 8);
                /* Tx good byte count */
                cnt = cnt + athrs26_reg_read(reg_addr);

                if (cnt == 0) {
                #ifdef CONFIG_DIR615E
            	    ar7100_set_gpio(13+i,0);
                #endif
                        if(is_ar933x()) {
                                s26_wr_phy(i,0x19, 0x3c0);
                                s26_wr_phy(i,0x18,(0x4));
                        } else {
                                s26_wr_phy(i,0x19,(s26_rd_phy( i,0x19) | (0x3c0)));
                        }
                    continue;
                }
                if (pledctrl->speed[i] == AG7240_PHY_SPEED_10T) {
                    bRateTab = BlinkRateTable_10M;
                }
                while (bRateTab++) {
                    if (cnt <= bRateTab->rate) {
                #ifdef CONFIG_DIR615E
            		ar7100_set_gpio(13+i,1);
                #endif
                            if(is_ar933x()&& bRateTab->rate <= MB(5)){
    				            s26_wr_phy(i,0x18, 0);
                            } else {
                        s26_wr_phy(i,0x18,((bRateTab->timeOn << 12)|(bRateTab->timeOff << 8)));
                            }
                            if(is_ar933x()) {
                                s26_wr_phy(i,0x19, 0x140);                            
                            } else {
                        s26_wr_phy(i,0x19,(s26_rd_phy(i,0x19) & ~(0x280)));                
                            }
                        break;
                    }
                }
            } else {
                #ifdef CONFIG_DIR615E
            	    ar7100_set_gpio(13+i,1);
            	#endif
            	    s26_wr_phy(i,0x19,0x0);
            }
        }
            if(!is_ar933x()) {
        /* Flush all LAN MIB counters */
        athrs26_reg_write(0x80,((1 << 17) | (1 << 24))); 
        while ((athrs26_reg_read(0x80) & 0x20000) == 0x20000);
        athrs26_reg_write(0x80,0);
        }
    }

    mac = ag7240_macs[0];
    bRateTab = BlinkRateTable_100M;
    cnt = 0;
    if (mac->mac_ifup) {
        if (pledctrl->ledlink[4]) {
            cnt += ag7240_reg_rd(mac,AG7240_RX_BYTES_CNTR) +
                           ag7240_reg_rd(mac,AG7240_TX_BYTES_CNTR);

            if (ag7240_get_diff(pkt_count,cnt) == 0) {
//                #ifdef CONFIG_DIR615E
//            	ar7100_set_gpio(17,0);                
//                #endif
                    if(is_ar933x()) {
                s26_wr_phy(4,0x19,((0x3c0)));
                    
                    }else
                    {
                s26_wr_phy(4,0x19,(s26_rd_phy(4,0x19) | (0x3c0)));
                }
                goto done;
            }
            if (pledctrl->speed[4] == AG7240_PHY_SPEED_10T) {
                bRateTab = BlinkRateTable_10M;
            }
            while (bRateTab++) {
                if (ag7240_get_diff(pkt_count,cnt) <= bRateTab->rate) {
//                #ifdef CONFIG_DIR615E
//            	    ar7100_set_gpio(17,1);
//                #endif
                    s26_wr_phy(4,0x18,((bRateTab->timeOn << 12)|(bRateTab->timeOff << 8)));
                        if(is_ar933x()) {
                    s26_wr_phy(4,0x19,0x140);
                        } else {
                    s26_wr_phy(4,0x19,(s26_rd_phy(4,0x19) & ~(0x280)));
                        }

                    break;
                }
            }
            pkt_count = cnt;
        } else {
//            #ifdef CONFIG_DIR615E
//            ar7100_set_gpio(17,1);            
//            #endif
            s26_wr_phy(4,0x19,0x0);
        }
    }
}
done: 
    if (mac_has_flag(mac,CHECK_DMA_STATUS)) { 
        if(ag7240_get_diff(prev_dma_chk_ts,jiffies) >= (1*HZ / 2)) {
            if (ag7240_macs[0]->mac_ifup) check_for_dma_status(ag7240_macs[0],0);      
            if (ag7240_macs[1]->mac_ifup) check_for_dma_status(ag7240_macs[1],0);
            prev_dma_chk_ts = jiffies;
        }
    }
    mod_timer(&PLedCtrl.led_timer,(jiffies + AG7240_LED_POLL_SECONDS));
    atomic_dec(&Ledstatus);
    return 0;
}

static int check_dma_status_pause(ag7240_mac_t *mac) { 

    int RxFsm,TxFsm,RxFD,RxCtrl,TxCtrl;

    /*
     * If get called by tx timeout for other chips we assume
     * the DMA is in pause state and update the watchdog
     * timer to avoid MAC reset.
     */

    if(!is_ar7240())
       return 1;

    RxFsm = ag7240_reg_rd(mac,AG7240_DMA_RXFSM);
    TxFsm = ag7240_reg_rd(mac,AG7240_DMA_TXFSM);
    RxFD  = ag7240_reg_rd(mac,AG7240_DMA_XFIFO_DEPTH);
    RxCtrl = ag7240_reg_rd(mac,AG7240_DMA_RX_CTRL);
    TxCtrl = ag7240_reg_rd(mac,AG7240_DMA_TX_CTRL);


    if ((RxFsm & AG7240_DMA_DMA_STATE) == 0x3 
        && ((RxFsm >> 4) & AG7240_DMA_AHB_STATE) == 0x6) {
        return 0;
    }
    else if ((((TxFsm >> 4) & AG7240_DMA_AHB_STATE) <= 0x0) && 
             ((RxFsm & AG7240_DMA_DMA_STATE) == 0x0) && 
             (((RxFsm >> 4) & AG7240_DMA_AHB_STATE) == 0x0) && 
             (RxFD  == 0x0) && (RxCtrl == 1) && (TxCtrl == 1)) {
        return 0;
    }
    else if (((((TxFsm >> 4) & AG7240_DMA_AHB_STATE) <= 0x4) && 
            ((RxFsm & AG7240_DMA_DMA_STATE) == 0x0) && 
            (((RxFsm >> 4) & AG7240_DMA_AHB_STATE) == 0x0)) || 
            (((RxFD >> 16) <= 0x20) && (RxCtrl == 1)) ) {
        return 1;
    }
    else {
        DPRINTF(" FIFO DEPTH = %x",RxFD);
        DPRINTF(" RX DMA CTRL = %x",RxCtrl);
        DPRINTF("mac:%d RxFsm:%x TxFsm:%x\n",mac->mac_unit,RxFsm,TxFsm);
        return 2;
    }
}

static int check_for_dma_status(ag7240_mac_t *mac,int ac) {

    ag7240_ring_t   *r     = &mac->mac_txring[ac];
    int              head  = r->ring_head, tail = r->ring_tail;
    ag7240_desc_t   *ds;
    uint32_t rx_ds;
    ag7240_buffer_t *bp;
    int flags,mask,int_mask;
    unsigned int w1 = 0, w2 = 0;

    /* If Tx hang is asserted reset the MAC and restore the descriptors
     * and interrupt state.
     */
    while (tail != head)
    {
        ds   = &r->ring_desc[tail];
        bp   =  &r->ring_buffer[tail];

        if(ag7240_tx_owned_by_dma(ds)) {
            if ((ag7240_get_diff(bp->trans_start,jiffies)) > ((1 * HZ/10))) {

                 /*
                  * If the DMA is in pause state reset kernel watchdog timer
                  */
        
                if(check_dma_status_pause(mac)) {
                    mac->mac_dev->trans_start = jiffies;
                    return 0;
                } 
                printk(MODULE_NAME ": Tx Dma status eth%d : %s\n",mac->mac_unit,
                            ag7240_tx_stopped(mac) ? "inactive" : "active");                               

                spin_lock_irqsave(&mac->mac_lock, flags);
                                                                                                
                int_mask = ag7240_reg_rd(mac,AG7240_DMA_INTR_MASK);

                ag7240_tx_stop(mac);
                ag7240_rx_stop(mac);

                rx_ds = ag7240_reg_rd(mac,AG7240_DMA_RX_DESC);
                mask = ag7240_reset_mask(mac->mac_unit);

                /*
                 * put into reset, hold, pull out.
                 */
                ar7240_reg_rmw_set(AR7240_RESET, mask);
                udelay(10);
                ar7240_reg_rmw_clear(AR7240_RESET, mask);
                udelay(10);

                ag7240_hw_setup(mac);

                ag7240_reg_wr(mac,AG7240_DMA_TX_DESC,ag7240_desc_dma_addr(r,ds));
                ag7240_reg_wr(mac,AG7240_DMA_RX_DESC,rx_ds);
                /*
                 * set the mac addr
                 */
                 
                addr_to_words(mac->mac_dev->dev_addr, w1, w2);
                ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR1, w1);
                ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR2, w2);

                ag7240_set_mac_from_link(mac, mac->mac_speed, mac->mac_fdx);
                

                if (mac_has_flag(mac,WAN_QOS_SOFT_CLASS)) {
                    ag7240_tx_start_qos(mac,ac);
                }
                else {
                    ag7240_tx_start(mac);
                }

                ag7240_rx_start(mac);
               
                /*
                 * Restore interrupts
                 */
                ag7240_reg_wr(mac,AG7240_DMA_INTR_MASK,int_mask);

                spin_unlock_irqrestore(&mac->mac_lock,flags);
                break;
            }
        }
        ag7240_ring_incr(tail);
    }
    return 0;
}


/*
 *
 * phy link state management
 */
int
ag7240_check_link(ag7240_mac_t *mac,int phyUnit)
{
    struct net_device  *dev     = mac->mac_dev;
    int                 carrier = netif_carrier_ok(dev), fdx=0, phy_up=0;
    ag7240_phy_speed_t  speed = 0;
    int                 rc;

    /* The vitesse switch uses an indirect method to communicate phy status
     * so it is best to limit the number of calls to what is necessary.
     * However a single call returns all three pieces of status information.
     * 
     * This is a trivial change to the other PHYs ergo this change.
     *
     */
  
    rc = ag7240_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed, phyUnit);

    athrs26_phy_stab_wr(phyUnit,phy_up,speed);

    if (rc < 0)
        goto done;

    if (!phy_up)
    {
        if (carrier)
        {
            printk(MODULE_NAME ":unit %d: phy %0d not up carrier %d\n", mac->mac_unit, phyUnit, carrier);

            /* A race condition is hit when the queue is switched on while tx interrupts are enabled.
             * To avoid that disable tx interrupts when phy is down.
             */
            ag7240_intr_disable_tx(mac);

            netif_carrier_off(dev);
            netif_stop_queue(dev);
            if (mac_has_flag(mac,ETH_SOFT_LED)) {
                PLedCtrl.ledlink[phyUnit] = 0;
                s26_wr_phy(phyUnit,0x19,0x0);
            }
        }
        goto done;
    }
   
    if(!mac->mac_ifup)
        goto done; 
    /*
     * phy is up. Either nothing changed or phy setttings changed while we 
     * were sleeping.
     */

    if ((fdx < 0) || (speed < 0))
    {
        printk(MODULE_NAME ": phy not connected?\n");
        return 0;
    }

    if (carrier && (speed == mac->mac_speed) && (fdx == mac->mac_fdx)) 
        goto done;

    if (athrs26_phy_is_link_alive(phyUnit)) 
    {
        printk(MODULE_NAME ": enet unit:%d phy:%d is up...", mac->mac_unit,phyUnit);
        printk("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)], 
           spd_str[speed], dup_str[fdx]);

        ag7240_set_mac_from_link(mac, speed, fdx);

        printk(MODULE_NAME ": done cfg2 %#x ifctl %#x miictrl  \n", 
           ag7240_reg_rd(mac, AG7240_MAC_CFG2), 
           ag7240_reg_rd(mac, AG7240_MAC_IFCTL));
        /*
         * in business
         */
        netif_carrier_on(dev);
        netif_start_queue(dev);
        /* 
         * WAR: Enable link LED to glow if speed is negotiated as 10 Mbps 
         */
        if (mac_has_flag(mac,ETH_SOFT_LED)) {
            PLedCtrl.ledlink[phyUnit] = 1;
            PLedCtrl.speed[phyUnit] = speed;

            s26_wr_phy(phyUnit,0x19,0x3c0);
        }
    }
    else {
        if (mac_has_flag(mac,ETH_SOFT_LED)) {
            PLedCtrl.ledlink[phyUnit] = 0;
            s26_wr_phy(phyUnit,0x19,0x0);
        }
        printk(MODULE_NAME ": enet unit:%d phy:%d is down...\n", mac->mac_unit,phyUnit);
    }

done:
    return 0;
}

#if defined(CONFIG_AR7242_RGMII_PHY)||defined(CONFIG_AR7242_S16_PHY)||defined(CONFIG_AR7242_VIR_PHY) || defined(CONFIG_AR7242_RTL8309G_PHY)
/*
 * phy link state management
 */
int
ag7242_check_link(ag7240_mac_t *mac)
{
    struct net_device  *dev     = mac->mac_dev;
    int                 carrier = netif_carrier_ok(dev), fdx = 0, phy_up = 0;
    ag7240_phy_speed_t  speed = 0;
    int                 rc,phyUnit = 0;


    rc = ag7240_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed, phyUnit);

    if (rc < 0)
        goto done;

    if (!phy_up)
    {
        if (carrier)
        {
            printk(MODULE_NAME ": unit %d: phy %0d not up carrier %d\n", mac->mac_unit, phyUnit, carrier);

            /* A race condition is hit when the queue is switched on while tx interrupts are enabled.
             * To avoid that disable tx interrupts when phy is down.
             */
            ag7240_intr_disable_tx(mac);

            netif_carrier_off(dev);
            netif_stop_queue(dev);
       }
       goto done;
    }

    if(!mac->mac_ifup) {
        goto done;
    }

    if ((fdx < 0) || (speed < 0))
    {
        printk(MODULE_NAME ": phy not connected?\n");
        return 0;
    }

    if (carrier && (speed == rg_phy_speed ) && (fdx == rg_phy_duplex)) {
        goto done;
    }
#ifdef CONFIG_AR7242_RGMII_PHY
    if (athr_phy_is_link_alive(phyUnit)) 
#endif
    {
        printk(MODULE_NAME ": enet unit:%d is up...\n", mac->mac_unit);
        printk("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)],
           spd_str[speed], dup_str[fdx]);

        rg_phy_speed = speed;
        rg_phy_duplex = fdx;
        /* Set GEO to be always at Giga Bit */
        speed = AG7240_PHY_SPEED_1000T;
        ag7240_set_mac_from_link(mac, speed, fdx);

        printk(MODULE_NAME ": done cfg2 %#x ifctl %#x miictrl  \n",
           ag7240_reg_rd(mac, AG7240_MAC_CFG2),
           ag7240_reg_rd(mac, AG7240_MAC_IFCTL));
        /*
         * in business
         */
        netif_carrier_on(dev);
        netif_start_queue(dev);
    }
    
done:
    mod_timer(&mac->mac_phy_timer, jiffies + AG7240_PHY_POLL_SECONDS*HZ);
    return 0;
}

#endif

uint16_t
ag7240_mii_read(int unit, uint32_t phy_addr, uint8_t reg)
{
    ag7240_mac_t *mac   = ag7240_unit2mac(unit);
    uint16_t      addr  = (phy_addr << AG7240_ADDR_SHIFT) | reg, val;
    volatile int           rddata;
    uint16_t      ii = 0x1000;


    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, 0x0);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_ADDRESS, addr);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, AG7240_MGMT_CMD_READ);

    do
    {
        udelay(5);
        rddata = ag7240_reg_rd(mac, AG7240_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);

    val = ag7240_reg_rd(mac, AG7240_MII_MGMT_STATUS);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, 0x0);

    return val;
}

void
ag7240_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data)
{
    ag7240_mac_t *mac   = ag7240_unit2mac(unit);
    uint16_t      addr  = (phy_addr << AG7240_ADDR_SHIFT) | reg;
    volatile int rddata;
    uint16_t      ii = 0x1000;


    ag7240_reg_wr(mac, AG7240_MII_MGMT_ADDRESS, addr);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CTRL, data);

    do
    {
        rddata = ag7240_reg_rd(mac, AG7240_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);

}

/*
 * Tx operation:
 * We do lazy reaping - only when the ring is "thresh" full. If the ring is 
 * full and the hardware is not even done with the first pkt we q'd, we turn
 * on the tx interrupt, stop all q's and wait for h/w to
 * tell us when its done with a "few" pkts, and then turn the Qs on again.
 *
 * Locking:
 * The interrupt only touches the ring when Q's stopped  => Tx is lockless, 
 * except when handling ring full.
 *
 * Desc Flushing: Flushing needs to be handled at various levels, broadly:
 * - The DDr FIFOs for desc reads.
 * - WB's for desc writes.
 */
static void
ag7240_handle_tx_full(ag7240_mac_t *mac)
{
    u32         flags;

    assert(!netif_queue_stopped(mac->mac_dev));

    mac->mac_net_stats.tx_fifo_errors ++;
    netif_stop_queue(mac->mac_dev);


    spin_lock_irqsave(&mac->mac_lock, flags);
    ag7240_intr_enable_tx(mac);
    spin_unlock_irqrestore(&mac->mac_lock, flags);
}

/* ******************************
 * 
 * Code under test - do not use
 *
 * ******************************
 */
static ag7240_desc_t *
ag7240_get_tx_ds(ag7240_mac_t *mac, int *len, unsigned char **start,int ac)
{
    ag7240_desc_t      *ds;
    int                len_this_ds;
    ag7240_ring_t      *r   = &mac->mac_txring[ac];
    ag7240_buffer_t    *bp;



    /* force extra pkt if remainder less than 4 bytes */
    if (*len > tx_len_per_ds)
        if (*len < (tx_len_per_ds + 4))
            len_this_ds = tx_len_per_ds - 4;
        else
            len_this_ds = tx_len_per_ds;
    else
        len_this_ds    = *len;

    ds = &r->ring_desc[r->ring_head];

    ag7240_trc_new(ds,"ds addr");
    ag7240_trc_new(ds,"ds len");
    if (ag7240_tx_owned_by_dma(ds))
	ag7240_dma_reset(mac);

    ds->pkt_size       = len_this_ds;
    ds->pkt_start_addr = virt_to_phys(*start);
    ds->more           = 1;

    *len   -= len_this_ds;
    *start += len_this_ds;

    if (mac_has_flag(mac,CHECK_DMA_STATUS)) {
        bp = &r->ring_buffer[r->ring_head];
        bp->trans_start = jiffies; 
    }

    ag7240_ring_incr(r->ring_head);

    return ds;
}

int
ag7240_hard_start(struct sk_buff *skb, struct net_device *dev)
{
    ag7240_mac_t       *mac = (ag7240_mac_t *)netdev_priv(dev);
    ag7240_ring_t      *r;
    struct ethhdr *eh = (struct ethhdr *) skb->data;
    int                ac;
    ag7240_buffer_t    *bp;
    ag7240_desc_t      *ds, *fds;
    unsigned char      *start;
    int                len;
    int                nds_this_pkt;
//#ifdef CONFIG_AR7240_S26_VLAN_IGMP
    if(unlikely((skb->len <= 0) || (skb->len > (dev->mtu + ETH_VLAN_HLEN +6 ))))
//#else
//    if(unlikely((skb->len <= 0) || (skb->len > (dev->mtu + ETH_HLEN + 4))))
//#endif
    {
        printk(MODULE_NAME ": bad skb, len %d\n", skb->len);
        goto dropit;
    }

    for (ac = 0;ac < mac->mac_noacs; ac++) {
        if (ag7240_tx_reap_thresh(mac,ac)) 
            ag7240_tx_reap(mac,ac);
    }

    /* 
     * Select the TX based on access category 
     */
    ac = ENET_AC_BE;
    if ( (mac_has_flag(mac,WAN_QOS_SOFT_CLASS)) || (mac_has_flag(mac,ATHR_S26_HEADER))
        || (mac_has_flag(mac,ATHR_S16_HEADER)))  { 
        /* Default priority */
        eh = (struct ethhdr *) skb->data;

        if (eh->h_proto  == __constant_htons(ETHERTYPE_IP))
        {
            const struct iphdr *ip = (struct iphdr *)
                        (skb->data + sizeof (struct ethhdr));
            /*
             * IP frame: exclude ECN bits 0-1 and map DSCP bits 2-7
             * from TOS byte.
             */
            ac = TOS_TO_ENET_AC ((ip->tos >> TOS_ECN_SHIFT) & 0x3F);
        }
    }
    skb->priority=ac;
    /* add header to normal frames sent to LAN*/
    if (mac_has_flag(mac,ATHR_S26_HEADER))
    {
        skb_push(skb, ATHR_HEADER_LEN);
        skb->data[0] = 0x30; /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */
        skb->data[1] = (0x40 | (ac << HDR_PRIORITY_SHIFT)); /* reserved = 0b10; priority = 0; type = 0 (normal) */
        skb->priority=ENET_AC_BE;
    }

    if (mac_has_flag(mac,ATHR_S16_HEADER))
    {
        skb_push(skb, ATHR_HEADER_LEN);
        memcpy(skb->data,skb->data + ATHR_HEADER_LEN, 12);

        skb->data[12] = 0x30; /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */
        skb->data[13] = 0x40 | ((ac << HDR_PRIORITY_SHIFT)); /* reserved = 0b10; priority = 0; type = 0 (normal) */
        skb->priority=ENET_AC_BE;
    }
    /*hdr_dump("Tx",mac->mac_unit,skb->data,ac,0);*/
    r = &mac->mac_txring[skb->priority];

    assert(r);
    ag7240_trc_new(r->ring_head,"hard-stop hd");
    ag7240_trc_new(r->ring_tail,"hard-stop tl");

    ag7240_trc_new(skb->len,    "len this pkt");
    ag7240_trc_new(skb->data,   "ptr 2 pkt");

    dma_cache_sync(NULL, (void *)skb->data, skb->len, DMA_TO_DEVICE);

    bp          = &r->ring_buffer[r->ring_head];
    bp->buf_pkt = skb;
    len         = skb->len;
    start       = skb->data;

    assert(len>4);

    nds_this_pkt = 1;
    fds = ds = ag7240_get_tx_ds(mac, &len, &start,skb->priority);

    while (len>0)
    {
        ds = ag7240_get_tx_ds(mac, &len, &start,skb->priority);
        nds_this_pkt++;
        ag7240_tx_give_to_dma(ds);
    }

    ds->res1           = 0;
    ds->res2           = 0;
    ds->ftpp_override  = 0;
    ds->res3           = 0;

    ds->more        = 0;
    ag7240_tx_give_to_dma(fds);

    bp->buf_lastds  = ds;
    bp->buf_nds     = nds_this_pkt;

    ag7240_trc_new(ds,"last ds");
    ag7240_trc_new(nds_this_pkt,"nmbr ds for this pkt");

    wmb();

    mac->net_tx_packets ++;
    mac->net_tx_bytes += skb->len;

    ag7240_trc(ag7240_reg_rd(mac, AG7240_DMA_TX_CTRL),"dma idle");

    if (mac_has_flag(mac,WAN_QOS_SOFT_CLASS)) {
        ag7240_tx_start_qos(mac,skb->priority);
    }
    else {
        ag7240_tx_start(mac);
    }
    for ( ac = 0;ac < mac->mac_noacs; ac++) {
        if (unlikely(ag7240_tx_ring_full(mac,ac))) 
            ag7240_handle_tx_full(mac);
    }


    dev->trans_start = jiffies;

    return NETDEV_TX_OK;

dropit:
    printk(MODULE_NAME ": dropping skb %08x\n", (unsigned int)skb);
    kfree_skb(skb);
    return NETDEV_TX_OK;
}

/*
 * Interrupt handling:
 * - Recv NAPI style (refer to Documentation/networking/NAPI)
 *
 *   2 Rx interrupts: RX and Overflow (OVF).
 *   - If we get RX and/or OVF, schedule a poll. Turn off _both_ interurpts. 
 *
 *   - When our poll's called, we
 *     a) Have one or more packets to process and replenish
 *     b) The hardware may have stopped because of an OVF.
 *
 *   - We process and replenish as much as we can. For every rcvd pkt 
 *     indicated up the stack, the head moves. For every such slot that we
 *     replenish with an skb, the tail moves. If head catches up with the tail
 *     we're OOM. When all's done, we consider where we're at:
 *
 *      if no OOM:
 *      - if we're out of quota, let the ints be disabled and poll scheduled.
 *      - If we've processed everything, enable ints and cancel poll.
 *
 *      If OOM:
 *      - Start a timer. Cancel poll. Ints still disabled. 
 *        If the hardware's stopped, no point in restarting yet. 
 *
 *      Note that in general, whether we're OOM or not, we still try to
 *      indicate everything recvd, up.
 *
 * Locking: 
 * The interrupt doesnt touch the ring => Rx is lockless
 *
 */
static irqreturn_t
ag7240_intr(int cpl, void *dev_id)
{
    struct net_device *dev  = (struct net_device *)dev_id;
    ag7240_mac_t      *mac  = (ag7240_mac_t *)netdev_priv(dev);
    int   isr, imr, handled = 0,ac;

    isr   = ag7240_get_isr(mac);
    imr   = ag7240_reg_rd(mac, AG7240_DMA_INTR_MASK);

    ag7240_trc(isr,"isr");
    ag7240_trc(imr,"imr");

    assert(isr == (isr & imr));
    if (isr & (AG7240_INTR_RX_OVF))
    {
        handled = 1;

        ag7240_reg_wr(mac,AG7240_MAC_CFG1,(ag7240_reg_rd(mac,AG7240_MAC_CFG1)&0xfffffff3));

        ag7240_intr_ack_rxovf(mac);
    }
    if (likely(isr & AG7240_INTR_RX))
    {
        handled = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	if (napi_schedule_prep(&mac->mac_napi))
#else
	if (netif_rx_schedule_prep(dev))
#endif
        {
            ag7240_intr_disable_recv(mac);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
            __napi_schedule(&mac->mac_napi);
#else
            __netif_rx_schedule(dev);
#endif
        }
        else
        {
            printk(MODULE_NAME ": driver bug! interrupt while in poll\n");
//            assert(0);
//            ag7240_intr_disable_recv(mac);
        }
        /*ag7240_recv_packets(dev, mac, 200, &budget);*/
    }
    if (likely(isr & AG7240_INTR_TX))
    {
        handled = 1;
        ag7240_intr_ack_tx(mac);
	/* Which queue to reap ??????????? */
        for(ac = 0; ac < mac->mac_noacs;ac++)
            ag7240_tx_reap(mac,ac);
    }
    if (unlikely(isr & AG7240_INTR_RX_BUS_ERROR))
    {
        assert(0);
        handled = 1;
        ag7240_intr_ack_rxbe(mac);
    }
    if (unlikely(isr & AG7240_INTR_TX_BUS_ERROR))
    {
        assert(0);
        handled = 1;
        ag7240_intr_ack_txbe(mac);
    }

    if (!handled)
    {
        assert(0);
        printk(MODULE_NAME ": unhandled intr isr %#x\n", isr);
    }

    return IRQ_HANDLED;
}

static irqreturn_t
ag7240_link_intr(int cpl, void *dev_id) {

	ar7240_s26_intr();
	return IRQ_HANDLED;
}

void ag7240_dma_reset(ag7240_mac_t *mac)
{
    uint32_t mask;

    if(mac->mac_unit)
        mask = AR7240_RESET_GE1_MAC;
    else
        mask = AR7240_RESET_GE0_MAC;

    ar7240_reg_rmw_set(AR7240_RESET, mask);
    mdelay(100);
    ar7240_reg_rmw_clear(AR7240_RESET, mask);
    mdelay(100);

    ag7240_intr_disable_recv(mac);
    schedule_work(&mac->mac_tx_timeout);
}


static int
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
ag7240_poll(struct napi_struct *napi, int budget)
#else
ag7240_poll(struct net_device *dev, int *budget)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	ag7240_mac_t *mac = container_of(napi, ag7240_mac_t, mac_napi);
	struct net_device *dev = mac->mac_dev;
	int work_done=0,      max_work  = budget, status = 0;
#else
	ag7240_mac_t       *mac       = (ag7240_mac_t *)netdev_priv(dev);
	int work_done=0,      max_work  = min(*budget, dev->quota), status = 0;
#endif


    ag7240_rx_status_t  ret;
    u32                 flags;
    spin_lock_irqsave(&mac->mac_lock, flags);

    ret = ag7240_recv_packets(dev, mac, max_work, &work_done);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	if (likely(ret == AG7240_RX_STATUS_DONE) && work_done < budget )
		{
    		napi_complete(napi);
    		ag7240_intr_enable_recv(mac);
    		}
#else
    dev->quota  -= work_done;
    *budget     -= work_done;
    if (likely(ret == AG7240_RX_STATUS_DONE))
    {
	netif_rx_complete(dev);
    }
#endif
    if(ret == AG7240_RX_DMA_HANG)
    {
        status = 0;
        ag7240_dma_reset(mac);
    }
    if (likely(ret == AG7240_RX_STATUS_NOT_DONE))
    {
        /*
        * We have work left
        */
        status = 1;
    	napi_complete(napi);
    	napi_reschedule(napi);
    }
    else if (ret == AG7240_RX_STATUS_OOM)
    {
        printk(MODULE_NAME ": oom..?\n");
        /* 
        * Start timer, stop polling, but do not enable rx interrupts.
        */
        mod_timer(&mac->mac_oom_timer, jiffies+1);
    }
spin_unlock_irqrestore(&mac->mac_lock, flags);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	return work_done;
#else
	return status;
#endif

}

int ag7240_recv_packets(struct net_device *dev, ag7240_mac_t *mac, 
    int quota, int *work_done)
{
    ag7240_ring_t       *r     = &mac->mac_rxring;
    ag7240_desc_t       *ds;
    ag7240_buffer_t     *bp;
    struct sk_buff      *skb;
    ag7240_rx_status_t   ret   = AG7240_RX_STATUS_DONE;
    int head = r->ring_head, len, status, iquota = quota, more_pkts, rep;

    ag7240_trc(iquota,"iquota");
    status = ag7240_reg_rd(mac, AG7240_DMA_RX_STATUS);

process_pkts:

    ag7240_trc(status,"status");

    /*
    * Flush the DDR FIFOs for our gmac
    */
    ar7240_flush_ge(mac->mac_unit);

    assert(quota > 0); /* WCL */

    while(quota)
    {
        ds    = &r->ring_desc[head];

        ag7240_trc(head,"hd");
        ag7240_trc(ds,  "ds");

        if (ag7240_rx_owned_by_dma(ds))
        {
    	    break;
/*            if(quota == iquota)
            {
                *work_done = quota = 0;
                return AG7240_RX_DMA_HANG;
            }
            break;*/
        }
        ag7240_intr_ack_rx(mac);

        bp                  = &r->ring_buffer[head];
        len                 = ds->pkt_size;
        skb                 = bp->buf_pkt;

        assert(skb);
        skb_put(skb, len - ETHERNET_FCS_SIZE);

#if defined(CONFIG_ATHEROS_HEADER_EN)
	//01:00:5e:X:X:X is a multicast MAC address.
	if(mac->mac_unit == 1){
		int offset = 14;
		if(ignore_packet_inspection){
			if((skb->data[2]==0x01)&&(skb->data[3]==0x00)&&(skb->data[4]==0x5e)){
		
				//If vlan, calculate the offset of the vlan header.
				if((skb->data[14]==0x81)&&(skb->data[15]==0x00)){
					offset +=4;				
				}

				//If pppoe exists, calculate the offset of the pppoe header.
				if((skb->data[offset]==0x88)&&((skb->data[offset+1]==0x63)|(skb->data[offset+1]==0x64))){
					offset += sizeof(struct pppoe_hdr);				
				}

				//Parse IP layer. 
				if((skb->data[offset]==0x08)&&(skb->data[offset+1]==0x00)){
					struct iphdr * iph;
					struct igmphdr *igh;

					offset +=2;
					iph = skb->data+offset;
					offset += (iph->ihl<<2);
					igh = skb->data + offset; 

					//Sanity Check and insert port information into igmp checksum.
					if((iph->ihl>4)&&(iph->version==4)&&(iph->protocol==2)){
						igh->csum = 0x7d50|(skb->data[0]&0xf);
						printk("[eth1] Type %x port %x.\n",igh->type,skb->data[0]&0xf);
					}
			
				}								
			}
		}
		skb_pull(skb,2);
	}

#endif

#ifdef CONFIG_AR7240_S26_VLAN_IGMP	
	//For swith S26, if vlan_id == 0, we will remove the tag header.
	if(mac->mac_unit==1)
	{
		if((skb->data[12]==0x81)&&(skb->data[13]==0x00)&&((skb->data[14]&0xf)==0x00)&&(skb->data[15]==0x00))
		{
			memmove(&skb->data[4],&skb->data[0],12);
			skb_pull(skb,4);
		}	
	}
#endif
        dma_cache_sync(NULL, (void *)skb->data,  skb->len, DMA_FROM_DEVICE);

        mac->net_rx_packets ++;
        mac->net_rx_bytes += skb->len;
        /*
        * also pulls the ether header
        */
        skb->dev            = dev;
        bp->buf_pkt         = NULL;
        dev->last_rx        = jiffies;

        work_done[0]++;
        quota--;
        skb->protocol       = eth_type_trans(skb, dev);
        netif_receive_skb(skb);
        ag7240_ring_incr(head);
    }

    r->ring_head   =  head;
    rep = ag7240_rx_replenish(mac);

    /*
    * let's see what changed while we were slogging.
    * ack Rx in the loop above is no flush version. It will get flushed now.
    */
    status       =  ag7240_reg_rd(mac, AG7240_DMA_RX_STATUS);
    more_pkts    =  (status & AG7240_RX_STATUS_PKT_RCVD);

    ag7240_trc(more_pkts,"more_pkts");

    if (!more_pkts) goto done;
    /*
    * more pkts arrived; if we have quota left, get rolling again
    */
    /*
    * out of quota
    */
    ret = AG7240_RX_STATUS_NOT_DONE;

done:

    if (unlikely(ag7240_rx_ring_full(mac))) 
        return AG7240_RX_STATUS_OOM;
    /*
    * !oom; if stopped, restart h/w
    */

    if (unlikely(status & AG7240_RX_STATUS_OVF))
    {
        mac->net_rx_over_errors ++;
        ag7240_intr_ack_rxovf(mac);
        ag7240_rx_start(mac);
    }

    return ret;
}

static struct sk_buff *
    ag7240_buffer_alloc(void)
{
    struct sk_buff *skb;

    skb = dev_alloc_skb(AG7240_RX_BUF_SIZE + AG7240_RX_RESERVE);
    if (unlikely(!skb))
        return NULL;
    skb_reserve(skb, AG7240_RX_RESERVE);

    return skb;
}

static void
ag7240_buffer_free(struct sk_buff *skb)
{
    if (in_irq())
        dev_kfree_skb_irq(skb);
    else
        dev_kfree_skb(skb);
}

/*
 * Head is the first slot with a valid buffer. Tail is the last slot 
 * replenished. Tries to refill buffers from tail to head.
 */
static int
ag7240_rx_replenish(ag7240_mac_t *mac)
{
    ag7240_ring_t   *r     = &mac->mac_rxring;
    int              head  = r->ring_head, tail = r->ring_tail, refilled = 0;
    ag7240_desc_t   *ds;
    ag7240_buffer_t *bf;

    ag7240_trc(head,"hd");
    ag7240_trc(tail,"tl");

    do
    {
        bf                  = &r->ring_buffer[tail];
        ds                  = &r->ring_desc[tail];

        ag7240_trc(ds,"ds");

        if(ag7240_rx_owned_by_dma(ds))
    	{
    	    return -1;
    	}
        assert(!bf->buf_pkt);

        bf->buf_pkt         = ag7240_buffer_alloc();
        if (!bf->buf_pkt)
        {
            printk(MODULE_NAME ": outta skbs!\n");
            break;
        }
        dma_cache_sync(NULL, (void *)bf->buf_pkt->data, AG7240_RX_BUF_SIZE, DMA_FROM_DEVICE);
        ds->pkt_start_addr  = virt_to_phys(bf->buf_pkt->data);

        ag7240_rx_give_to_dma(ds);
        refilled ++;

        ag7240_ring_incr(tail);

    } while(tail != head);
    /*
    * Flush descriptors
    */
    wmb();

        ag7240_reg_wr(mac,AG7240_MAC_CFG1,(ag7240_reg_rd(mac,AG7240_MAC_CFG1)|0xc));
        ag7240_rx_start(mac);

    r->ring_tail = tail;
    ag7240_trc(refilled,"refilled");

    return refilled;
}

/* 
 * Reap from tail till the head or whenever we encounter an unxmited packet.
 */
static int
ag7240_tx_reap(ag7240_mac_t *mac,int ac)
{    
    ag7240_ring_t   *r;
    int              head, tail, reaped = 0, i;
    ag7240_desc_t   *ds;
    ag7240_buffer_t *bf;
    uint32_t    flags;

    r = &mac->mac_txring[ac];
    head = r->ring_head;
    tail = r->ring_tail;
    ag7240_trc_new(head,"hd");
    ag7240_trc_new(tail,"tl");

    ar7240_flush_ge(mac->mac_unit);
    spin_lock_irqsave(&mac->mac_lock, flags);
    while(tail != head)
    {
        ds   = &r->ring_desc[tail];

        ag7240_trc_new(ds,"ds");

        if(ag7240_tx_owned_by_dma(ds))
            break;

        bf      = &r->ring_buffer[tail];
        assert(bf->buf_pkt);

        ag7240_trc_new(bf->buf_lastds,"lastds");

        if(ag7240_tx_owned_by_dma(bf->buf_lastds))
            break;

        for(i = 0; i < bf->buf_nds; i++)
        {
            ag7240_intr_ack_tx(mac);
            ag7240_ring_incr(tail);
        }

        ag7240_buffer_free(bf->buf_pkt);
        bf->buf_pkt = NULL;

        reaped ++;
    }
    spin_unlock_irqrestore(&mac->mac_lock, flags);

    r->ring_tail = tail;

    if (netif_queue_stopped(mac->mac_dev) &&
        (ag7240_ndesc_unused(mac, r) >= AG7240_TX_QSTART_THRESH) &&
        netif_carrier_ok(mac->mac_dev))
    {
        if (ag7240_reg_rd(mac, AG7240_DMA_INTR_MASK) & AG7240_INTR_TX)
        {
            spin_lock_irqsave(&mac->mac_lock, flags);
            ag7240_intr_disable_tx(mac);
            spin_unlock_irqrestore(&mac->mac_lock, flags);
        }
        netif_wake_queue(mac->mac_dev);
    }

    return reaped;
}

/*
 * allocate and init rings, descriptors etc.
 */
static int
ag7240_tx_alloc(ag7240_mac_t *mac)
{
    ag7240_ring_t *r ;
    int ac;
    ag7240_desc_t *ds;
    int i, next;
    for(ac = 0;ac < mac->mac_noacs; ac++) {

        r  = &mac->mac_txring[ac];
        if (ag7240_ring_alloc(r, AG7240_TX_DESC_CNT))
            return 1;

        ag7240_trc(r->ring_desc,"ring_desc");

       ds = r->ring_desc;
       for(i = 0; i < r->ring_nelem; i++ )
       {
            ag7240_trc_new(ds,"tx alloc ds");
            next                =   (i == (r->ring_nelem - 1)) ? 0 : (i + 1);
            ds[i].next_desc     =   ag7240_desc_dma_addr(r, &ds[next]);
            ag7240_tx_own(&ds[i]);
       }
   }
    return 0;
}

static int
ag7240_rx_alloc(ag7240_mac_t *mac)
{
    ag7240_ring_t *r  = &mac->mac_rxring;
    ag7240_desc_t *ds;
    int i, next, tail = r->ring_tail;
    ag7240_buffer_t *bf;

    if (ag7240_ring_alloc(r, AG7240_RX_DESC_CNT))
        return 1;

    ag7240_trc(r->ring_desc,"ring_desc");

    ds = r->ring_desc;
    for(i = 0; i < r->ring_nelem; i++ )
    {
        next                =   (i == (r->ring_nelem - 1)) ? 0 : (i + 1);
        ds[i].next_desc     =   ag7240_desc_dma_addr(r, &ds[next]);
    }

    for (i = 0; i < AG7240_RX_DESC_CNT; i++)
    {
        bf                  = &r->ring_buffer[tail];
        ds                  = &r->ring_desc[tail];

        bf->buf_pkt         = ag7240_buffer_alloc();
        if (!bf->buf_pkt) 
            goto error;

        dma_cache_sync(NULL, (void *)bf->buf_pkt->data, AG7240_RX_BUF_SIZE, DMA_FROM_DEVICE);
        ds->pkt_start_addr  = virt_to_phys(bf->buf_pkt->data);

        ds->res1           = 0;
        ds->res2           = 0;
        ds->ftpp_override  = 0;
        ds->res3           = 0;
	ds->more	= 0;
        ag7240_rx_give_to_dma(ds);
        ag7240_ring_incr(tail);
    }

    return 0;
error:
    printk(MODULE_NAME ": unable to allocate rx\n");
    ag7240_rx_free(mac);
    return 1;
}

static void
ag7240_tx_free(ag7240_mac_t *mac)
{
    int ac;
    
    for(ac = 0;ac < mac->mac_noacs; ac++) {
        ag7240_ring_release(mac, &mac->mac_txring[ac]);
        ag7240_ring_free(&mac->mac_txring[ac]);
   }
}

static void
ag7240_rx_free(ag7240_mac_t *mac)
{
    ag7240_ring_release(mac, &mac->mac_rxring);
    ag7240_ring_free(&mac->mac_rxring);
}

static int
ag7240_ring_alloc(ag7240_ring_t *r, int count)
{
    int desc_alloc_size, buf_alloc_size;

    desc_alloc_size = sizeof(ag7240_desc_t)   * count;
    buf_alloc_size  = sizeof(ag7240_buffer_t) * count;

    memset(r, 0, sizeof(ag7240_ring_t));

    r->ring_buffer = (ag7240_buffer_t *)kmalloc(buf_alloc_size, GFP_KERNEL);
    printk("%s Allocated %d at 0x%lx\n",__func__,buf_alloc_size,(unsigned long) r->ring_buffer);
    if (!r->ring_buffer)
    {
        printk(MODULE_NAME ": unable to allocate buffers\n");
        return 1;
    }

    r->ring_desc  =  (ag7240_desc_t *)dma_alloc_coherent(NULL, 
        desc_alloc_size,
        &r->ring_desc_dma, 
        GFP_DMA);
    if (! r->ring_desc)
    {
        printk(MODULE_NAME ": unable to allocate coherent descs\n");
        kfree(r->ring_buffer);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) r->ring_buffer);
        return 1;
    }

    memset(r->ring_buffer, 0, buf_alloc_size);
    memset(r->ring_desc,   0, desc_alloc_size);
    r->ring_nelem   = count;

    return 0;
}

static void
ag7240_ring_release(ag7240_mac_t *mac, ag7240_ring_t  *r)
{
    int i;

    for(i = 0; i < r->ring_nelem; i++)
        if (r->ring_buffer[i].buf_pkt)
            ag7240_buffer_free(r->ring_buffer[i].buf_pkt);
}

static void
ag7240_ring_free(ag7240_ring_t *r)
{
    dma_free_coherent(NULL, sizeof(ag7240_desc_t)*r->ring_nelem, r->ring_desc,
        r->ring_desc_dma);
    kfree(r->ring_buffer);
    printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) r->ring_buffer);
}

/*
 * Error timers
 */
static void
ag7240_oom_timer(unsigned long data)
{
    ag7240_mac_t *mac = (ag7240_mac_t *)data;
    int val;

    ag7240_trc(data,"data");
    ag7240_rx_replenish(mac);
    if (ag7240_rx_ring_full(mac))
    {
        val = mod_timer(&mac->mac_oom_timer, jiffies+1);
        assert(!val);
    }
    else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	napi_schedule(&mac->mac_napi);
#else
	netif_rx_schedule(mac->mac_dev);
#endif
}

static void
ag7240_tx_timeout(struct net_device *dev)
{
    ag7240_mac_t *mac = (ag7240_mac_t *)netdev_priv(dev);
    ag7240_trc(dev,"dev");
    printk("%s\n",__func__);
    /* 
    * Do the reset outside of interrupt context 
    */
    schedule_work(&mac->mac_tx_timeout);
}

static void
ag7240_tx_timeout_task(struct work_struct *work)
{
    ag7240_mac_t *mac = container_of(work, ag7240_mac_t, mac_tx_timeout);
    ag7240_trc(mac,"mac");
    check_for_dma_status(mac,0);
    ag7240_stop(mac->mac_dev);
    ag7240_open(mac->mac_dev);
}

static void
ag7240_get_default_macaddr(ag7240_mac_t *mac, u8 *mac_addr)
{
    /* Use MAC address stored in Flash */
    
#ifdef CONFIG_AG7240_MAC_LOCATION
    u8 *eep_mac_addr = (u8 *)( CONFIG_AG7240_MAC_LOCATION + (mac->mac_unit)*6);
#else
    u8 *eep_mac_addr = (u8 *)((mac->mac_unit) ? AR7240_EEPROM_GE1_MAC_ADDR:
        AR7240_EEPROM_GE0_MAC_ADDR);
#endif

    printk(MODULE_NAME "CHH: Mac address for unit %d\n",mac->mac_unit);
    printk(MODULE_NAME "CHH: %02x:%02x:%02x:%02x:%02x:%02x \n",
        eep_mac_addr[0],eep_mac_addr[1],eep_mac_addr[2],
        eep_mac_addr[3],eep_mac_addr[4],eep_mac_addr[5]);
    memcpy(mac_addr,eep_mac_addr,6);    
}

static int
ag7240_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    return athrs_do_ioctl(dev,ifr, cmd);
}
static struct net_device_stats 
    *ag7240_get_stats(struct net_device *dev)
{

    ag7240_mac_t *mac = netdev_priv(dev);
    int carrier = netif_carrier_ok(dev);

    if (mac->mac_speed < 1) {
        return &mac->mac_net_stats;
    }

    /* 
     *  MIB registers reads will fail if link is lost while resetting PHY.
     */
    if (carrier && !phy_in_reset)
    {

        mac->mac_net_stats.rx_packets = ag7240_reg_rd(mac,AG7240_RX_PKT_CNTR);
        mac->mac_net_stats.tx_packets = ag7240_reg_rd(mac,AG7240_TX_PKT_CNTR);
        mac->mac_net_stats.rx_bytes   = ag7240_reg_rd(mac,AG7240_RX_BYTES_CNTR);
        mac->mac_net_stats.tx_bytes   = ag7240_reg_rd(mac,AG7240_TX_BYTES_CNTR);
        mac->mac_net_stats.tx_errors  = ag7240_reg_rd(mac,AG7240_TX_CRC_ERR_CNTR);
        mac->mac_net_stats.rx_dropped = ag7240_reg_rd(mac,AG7240_RX_DROP_CNTR);
        mac->mac_net_stats.tx_dropped = ag7240_reg_rd(mac,AG7240_TX_DROP_CNTR);
        mac->mac_net_stats.collisions = ag7240_reg_rd(mac,AG7240_TOTAL_COL_CNTR);
    
            /* detailed rx_errors: */
        mac->mac_net_stats.rx_length_errors = ag7240_reg_rd(mac,AG7240_RX_LEN_ERR_CNTR);
        mac->mac_net_stats.rx_over_errors 	= ag7240_reg_rd(mac,AG7240_RX_OVL_ERR_CNTR);
        mac->mac_net_stats.rx_crc_errors 	= ag7240_reg_rd(mac,AG7240_RX_CRC_ERR_CNTR);
        mac->mac_net_stats.rx_frame_errors 	= ag7240_reg_rd(mac,AG7240_RX_FRM_ERR_CNTR);
    	
        mac->mac_net_stats.rx_errors  = ag7240_reg_rd(mac,AG7240_RX_CODE_ERR_CNTR) + 
                                        ag7240_reg_rd(mac,AG7240_RX_CRS_ERR_CNTR) +      
                                        mac->mac_net_stats.rx_length_errors +
                                        mac->mac_net_stats.rx_over_errors +
                                        mac->mac_net_stats.rx_crc_errors +
                                        mac->mac_net_stats.rx_frame_errors; 

        mac->mac_net_stats.multicast  = ag7240_reg_rd(mac,AG7240_RX_MULT_CNTR) +
                                        ag7240_reg_rd(mac,AG7240_TX_MULT_CNTR);

    }
    return &mac->mac_net_stats;
}

static void
ag7240_vet_tx_len_per_pkt(unsigned int *len)
{
    unsigned int l;

    /* make it into words */
    l = *len & ~3;

    /* 
    * Not too small 
    */
    if (l < AG7240_TX_MIN_DS_LEN) {
        l = AG7240_TX_MIN_DS_LEN;
    }
    else {
    /* Avoid len where we know we will deadlock, that
    * is the range between fif_len/2 and the MTU size
    */
        if (l > AG7240_TX_FIFO_LEN/2) {
            if (l < AG7240_TX_MTU_LEN)
                l = AG7240_TX_MTU_LEN;
            else if (l > AG7240_TX_MAX_DS_LEN)
                l = AG7240_TX_MAX_DS_LEN;
            *len = l;
        }
    }
}
static struct net_device_ops mac_net_ops;

static int ag7240_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu<=(AG71XX_TX_MTU_LEN-18))
	    dev->mtu = new_mtu;
	return 0;
}

/*
 * All allocations (except irq and rings).
 */
static int __init
ag7240_init(void)
{
    int i,st;
    struct net_device *dev;
    ag7240_mac_t      *mac;
    uint32_t mask;

    /* 
    * tx_len_per_ds is the number of bytes per data transfer in word increments.
    * 
    * If the value is 0 than we default the value to a known good value based
    * on benchmarks. Otherwise we use the value specified - within some 
    * cosntraints of course.
    *
    * Tested working values are 256, 512, 768, 1024 & 1536.
    *
    * A value of 256 worked best in all benchmarks. That is the default.
    *
    */

    /* Tested 256, 512, 768, 1024, 1536 OK, 1152 and 1280 failed*/
    if (0 == tx_len_per_ds)
        tx_len_per_ds = CONFIG_AG7240_LEN_PER_TX_DS;

    ag7240_vet_tx_len_per_pkt( &tx_len_per_ds);

    printk(MODULE_NAME ": Length per segment %d\n", tx_len_per_ds);

    /* 
    * Compute the number of descriptors for an MTU 
    */
    tx_max_desc_per_ds_pkt =1;


    printk(MODULE_NAME ": Max segments per packet %d\n", tx_max_desc_per_ds_pkt);
    printk(MODULE_NAME ": Max tx descriptor count    %d\n", AG7240_TX_DESC_CNT);
    printk(MODULE_NAME ": Max rx descriptor count    %d\n", AG7240_RX_DESC_CNT);

    /* 
    * Let hydra know how much to put into the fifo in words (for tx) 
    */
    if (0 == fifo_3)
        fifo_3 = 0x000001ff | ((AG7240_TX_FIFO_LEN-tx_len_per_ds)/4)<<16;

    printk(MODULE_NAME ": fifo cfg 3 %08x\n", fifo_3);

    /* 
    ** Do the rest of the initializations 
    */

    for(i = 0; i < AG7240_NMACS; i++)
    {
        dev = alloc_etherdev(sizeof(ag7240_mac_t));
        mac =  (ag7240_mac_t *)netdev_priv(dev);
        if (!mac)
        {
            printk(MODULE_NAME ": unable to allocate mac\n");
            return 1;
        }
        memset(mac, 0, sizeof(ag7240_mac_t));

        mac->mac_unit               =  i;
        mac->mac_base               =  ag7240_mac_base(i);
        mac->mac_irq                =  ag7240_mac_irq(i);
        mac->mac_noacs              =  1;
        ag7240_macs[i]              =  mac;
#ifdef CONFIG_ATHEROS_HEADER_EN
        if (mac->mac_unit == ENET_UNIT_LAN)    
	    mac_set_flag(mac,ATHR_S26_HEADER);
#endif 
#ifdef CONFIG_ETH_SOFT_LED
    if (is_ar7240() || is_ar933x()) 
            mac_set_flag(mac,ETH_SOFT_LED);
#endif
#ifdef CONFIG_CHECK_DMA_STATUS 
        mac_set_flag(mac,CHECK_DMA_STATUS);
#endif
#ifdef CONFIG_S26_SWITCH_ONLY_MODE
        if (is_ar7241()) {
            if(i) {
                mac_set_flag(mac,ETH_SWONLY_MODE);
                }
            else {
                continue;
            }
        }
#endif
        spin_lock_init(&mac->mac_lock);
        /*
        * out of memory timer
        */
        init_timer(&mac->mac_oom_timer);
        mac->mac_oom_timer.data     = (unsigned long)mac;
        mac->mac_oom_timer.function = (void *)ag7240_oom_timer;
        /*
        * watchdog task
        */

        INIT_WORK(&mac->mac_tx_timeout, ag7240_tx_timeout_task);

        if (!dev)
        {
            kfree(mac);
            printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) mac);
            printk(MODULE_NAME ": unable to allocate etherdev\n");
            return 1;
        }

        mac_net_ops.ndo_open      = ag7240_open;
        mac_net_ops.ndo_stop      = ag7240_stop;
        mac_net_ops.ndo_start_xmit= ag7240_hard_start;
        mac_net_ops.ndo_get_stats = ag7240_get_stats;
        mac_net_ops.ndo_tx_timeout= ag7240_tx_timeout;
        mac_net_ops.ndo_do_ioctl        =  ag7240_do_ioctl;
	mac_net_ops.ndo_change_mtu		= ag7240_change_mtu;
	mac_net_ops.ndo_set_mac_address	= eth_mac_addr;
	mac_net_ops.ndo_validate_addr	= eth_validate_addr;
	netif_napi_add(dev, &mac->mac_napi, ag7240_poll, AG7240_NAPI_WEIGHT);
        dev->netdev_ops = (const struct net_device_ops *)&mac_net_ops;             
        mac->mac_dev         =  dev;        

        ag7240_get_default_macaddr(mac, dev->dev_addr);

        if (register_netdev(dev))
        {
            printk(MODULE_NAME ": register netdev failed\n");
            goto failed;
        }
	netif_carrier_off(dev);
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG1, AG7240_MAC_CFG1_SOFT_RST 
				| AG7240_MAC_CFG1_RX_RST | AG7240_MAC_CFG1_TX_RST);

        udelay(20);
        mask = ag7240_reset_mask(mac->mac_unit);

        /*
        * put into reset, hold, pull out.
        */
        ar7240_reg_rmw_set(AR7240_RESET, mask);
        mdelay(100);
        ar7240_reg_rmw_clear(AR7240_RESET, mask);
        mdelay(100);
    }

    /*
     * Enable link interrupts for PHYs 
     */
    dev = ag7240_macs[0]->mac_dev;
    st = request_irq(AR7240_MISC_IRQ_ENET_LINK, ag7240_link_intr, 0, dev->name, dev);

    if (st < 0)
    {
        printk(MODULE_NAME ": request irq %d failed %d\n", AR7240_MISC_IRQ_ENET_LINK, st);
        goto failed;
    }

    ag7240_trc_init();

    athrs_reg_dev(ag7240_macs);
    if (mac_has_flag(mac,CHECK_DMA_STATUS))
        prev_dma_chk_ts = jiffies;

    if (mac_has_flag(mac,ETH_SOFT_LED)) {
        init_timer(&PLedCtrl.led_timer);
        PLedCtrl.led_timer.data     = (unsigned long)(&PLedCtrl);
        PLedCtrl.led_timer.function = (void *)led_control_func;
        mod_timer(&PLedCtrl.led_timer,(jiffies + AG7240_LED_POLL_SECONDS));
    }
    return 0;
failed:
    free_irq(AR7240_MISC_IRQ_ENET_LINK, ag7240_macs[0]->mac_dev);
    for(i = 0; i < AG7240_NMACS; i++)
    {
#ifdef CONFIG_S26_SWITCH_ONLY_MODE
        if (is_ar7241() && i == 0)
            continue;
#endif
        if (!ag7240_macs[i]) 
            continue;
        if (ag7240_macs[i]->mac_dev) 
            free_netdev(ag7240_macs[i]->mac_dev);
        kfree(ag7240_macs[i]);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) ag7240_macs[i]);
    }
    return 1;
}

static void __exit
ag7240_cleanup(void)
{
    int i;

    free_irq(AR7240_MISC_IRQ_ENET_LINK, ag7240_macs[0]->mac_dev);
    for(i = 0; i < AG7240_NMACS; i++)
    {
        if (mac_has_flag(ag7240_macs[1],ETH_SWONLY_MODE) && i == 0) {
            kfree(ag7240_macs[i]);
            continue;
        }
        unregister_netdev(ag7240_macs[i]->mac_dev);
        free_netdev(ag7240_macs[i]->mac_dev);
        kfree(ag7240_macs[i]);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) ag7240_macs[i]);
    }
    if (mac_has_flag(ag7240_macs[0],ETH_SOFT_LED)) 
        del_timer(&PLedCtrl.led_timer);
    printk(MODULE_NAME ": cleanup done\n");
}

module_init(ag7240_init);
module_exit(ag7240_cleanup);
