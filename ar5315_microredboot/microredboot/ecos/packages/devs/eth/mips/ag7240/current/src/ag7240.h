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

#ifndef _AG7240_H
#define _AG7240_H

#define AG7240_TX_DESC_CNT          CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_TX_DESC_CNT
#define AG7240_RX_DESC_CNT          CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_RX_DESC_CNT

#ifdef __ECOS
#define uint8_t  cyg_uint8
#define uint16_t cyg_uint16
#define uint32_t cyg_uint32
#endif

#include "ag7240_mac.h"
#include "ag7240_phy.h"

/*
 * Function definitions. 
 */

typedef void void_fun(void *);


//ag7240_mac_t *ag7240_macs[2];
#if 0
static void ag7240_hw_setup(ag7240_mac_t *mac);
static void ag7240_hw_stop(ag7240_mac_t *mac);
static void ag7240_oom_timer(unsigned long data);
int  ag7240_check_link(ag7240_mac_t *mac,int phyUnit);
int  ag7242_check_link(ag7240_mac_t *mac);
static int  check_for_dma_status(ag7240_mac_t *mac,int ac);
static int  ag7240_tx_alloc(ag7240_mac_t *mac);
static int  ag7240_rx_alloc(ag7240_mac_t *mac);
static void ag7240_rx_free(ag7240_mac_t *mac);
static void ag7240_tx_free(ag7240_mac_t *mac);
static int  ag7240_ring_alloc(ag7240_ring_t *r, int count);
static int  ag7240_rx_replenish(ag7240_mac_t *mac);
static void ag7240_get_default_macaddr(ag7240_mac_t *mac, cyg_uint8 *mac_addr);
static int  ag7240_tx_reap(ag7240_mac_t *mac,int ac);
static void ag7240_ring_release(ag7240_mac_t *mac, ag7240_ring_t  *r);
static void ag7240_ring_free(ag7240_ring_t *r);
static void ag7240_tx_timeout_task(ag7240_mac_t *mac);
static int  ag7240_poll(struct net_device *dev, int *budget);
#endif

#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_S26_VLAN_IGMP
#define ETH_VLAN_HLEN 18
#endif
  
#if 0
static int  ag7240_recv_packets(struct net_device *dev, ag7240_mac_t *mac,
    int max_work, int *work_done);
static irqreturn_t ag7240_intr(int cpl, void *dev_id, struct pt_regs *regs);
static irqreturn_t ag7240_link_intr(int cpl, void *dev_id, struct pt_regs *regs);
static struct sk_buff * ag7240_buffer_alloc(void);
#endif

static uint32_t prev_dma_chk_ts;
static int tx_len_per_ds = 0;
static int tx_max_desc_per_ds_pkt=1;


static char *mii_str[2][4] = {
    {"GMii", "Mii", "RGMii", "RMii"},
    {"GMii","Mii","RGMii", "RMii"}
};

/* 
 * Global Defines 
 */
/*
ATH_LED_CONTROL    PLedCtrl;
atomic_t Ledstatus;
int phy_in_reset = 0;
int rg_phy_speed = -1 , rg_phy_duplex = -1;
char *spd_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
char *dup_str[] = {"half duplex", "full duplex"}; */

/*
 * Defines specific to this implemention
 */

#define addr_to_words(addr, w1, w2)  {                                 \
    w1 = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | addr[3]; \
    w2 = (addr[4] << 24) | (addr[5] << 16) | 0;                        \
}

#define AG7240_TX_FIFO_LEN          2048
#define AG7240_TX_MIN_DS_LEN        128
#define AG7240_TX_MAX_DS_LEN        AG7240_TX_FIFO_LEN

#define AG7240_TX_MTU_LEN           1536

#define AG7240_TX_REAP_THRESH       AG7240_TX_DESC_CNT/2
#define AG7240_TX_QSTART_THRESH     4*tx_max_desc_per_ds_pkt


#define AG7240_NAPI_WEIGHT          64
#define AG7240_PHY_POLL_SECONDS     2
#define AG7240_LED_POLL_SECONDS    (HZ/10)

#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0

#define SW_PLL 0x1f000000ul
/*
 * Externs
 */

extern uint32_t ar7240_ahb_freq;
extern void ar7240_s26_intr(void);

/*
 * Inline functions
 */

static inline int
ag7240_ndesc_unused(ag7240_mac_t *mac, ag7240_ring_t *ring)
{
    int head = ring->ring_head, tail = ring->ring_tail;

    return ((tail > head ? 0 : ring->ring_nelem) + tail - head);
}

static inline uint32_t
ag7240_get_diff(uint32_t t1,uint32_t t2)
{
    return (t1 > t2 ? (0xffffffff - (t1 - t2)) : t2 - t1);
}

static inline int ag7240_rx_ring_full(ag7240_mac_t *mac)
{
    ag7240_ring_t *r = &mac->mac_rxring;
    int tail = r->ring_tail;

    /* TODO */
//    return ((r->ring_head == tail) && !r->ring_buffer[tail].buf_pkt);
    return ((r->ring_head == tail) && (r->ring_desc[tail].packet == NULL));
}

static inline void ag7240_set_mac_duplex(ag7240_mac_t *mac, int fdx)
{
    if (fdx) {
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_FDX);
    }
    else {
        ag7240_reg_rmw_clear(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_FDX);
    }
}

static inline void ag7240_set_mac_if(ag7240_mac_t *mac, int is_1000)
{
    ag7240_reg_rmw_clear(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_IF_1000|
                         AG7240_MAC_CFG2_IF_10_100));
    if (is_1000) {
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_IF_1000);
        ag7240_reg_rmw_set(mac, AG7240_MAC_FIFO_CFG_5, AG7240_BYTE_PER_CLK_EN);
    }
    else {
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, AG7240_MAC_CFG2_IF_10_100);
        ag7240_reg_rmw_clear(mac,AG7240_MAC_FIFO_CFG_5, AG7240_BYTE_PER_CLK_EN);
    }
}

static inline void ag7240_set_mac_speed(ag7240_mac_t *mac, int is100)
{
    if (is100) {
        ag7240_reg_rmw_set(mac, AG7240_MAC_IFCTL, AG7240_MAC_IFCTL_SPEED);
    }
    else {
        ag7240_reg_rmw_clear(mac, AG7240_MAC_IFCTL, AG7240_MAC_IFCTL_SPEED);
    }
}

static inline int ag7240_tx_reap_thresh(ag7240_mac_t *mac,int ac)
{
    ag7240_ring_t *r = &mac->mac_txring[ac];

    return (ag7240_ndesc_unused(mac, r) < AG7240_TX_REAP_THRESH);
}

static inline int ag7240_tx_ring_full(ag7240_mac_t *mac,int ac)
{
    ag7240_ring_t *r = &mac->mac_txring[ac];

    return (ag7240_ndesc_unused(mac, r) < tx_max_desc_per_ds_pkt + 2);
}

#endif
