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

#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/workqueue.h>
//#include <asm/system.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include "ar7240.h"
#include "ag7240_trc.h"

#ifdef CONFIG_AR7240_EMULATION
#undef CONFIG_AR9100
#endif


#include <net/inet_ecn.h>                /* XXX for TOS */
#include <linux/if_ether.h>

#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP     0x0800          /* IP protocol */
#endif

#define IP_PRI_SHIFT     5

#define ENET_NUM_AC      4               /* 4 AC categories */
/* QOS stream classes */
#define ENET_AC_BE       0               /* best effort */
#define ENET_AC_BK       1               /* background */
#define ENET_AC_VI       2               /* video */
#define ENET_AC_VO       3               /* voice */

#define HDR_PACKET_TYPE_MASK    0x0F            
#define HDR_PRIORITY_SHIFT      0x4
#define HDR_PRIORITY_MASK       0x3
#define TOS_ECN_SHIFT           0x2
#define TOS_ECN_MASK            0xFC

#define TOS_TO_ENET_AC(_tos) (      \
    (((_tos) == 0) || ((_tos) == 3)) ? ENET_AC_BE : \
    (((_tos) == 1) || ((_tos) == 2)) ? ENET_AC_BK : \
    (((_tos) == 4) || ((_tos) == 5)) ? ENET_AC_VI : \
        ENET_AC_VO)

#define CONFIG_CHECK_DMA_STATUS 1
#define CONFIG_ETH_SOFT_LED 1

#ifdef AG7240_DEBUG
#define DPRINTF(_fmt,...) do {         \
                printk(MODULE_NAME":"_fmt, __VA_ARGS__);      \
} while (0)
#else
#define DPRINTF(_fmt,...)
#endif

/*
 * h/w descriptor
 */
typedef struct {
    uint32_t    pkt_start_addr;

    uint32_t    is_empty       :  1;
    uint32_t    res1           :  6;
    uint32_t    more           :  1;
    uint32_t    res2           :  3;
    uint32_t    ftpp_override  :  5;
    uint32_t    res3           :  4;
    uint32_t    pkt_size       : 12;

    uint32_t    next_desc      ;
    uint32_t 	pad;
}ag7240_desc_t;

/*
 * s/w descriptor
 */
typedef struct {
    struct sk_buff *buf_pkt;        /*ptr to skb*/
    int             buf_nds;        /*no. of desc for this skb*/
    ag7240_desc_t  *buf_lastds;     /*the last desc. (for convenience)*/
    unsigned long   trans_start;    /*  descriptor time stamp */
}ag7240_buffer_t;

/*
 * Tx and Rx descriptor rings;
 */
typedef struct {
    ag7240_desc_t     *ring_desc;           /* hardware descriptors */
    dma_addr_t         ring_desc_dma;       /* dma addresses of desc*/
    ag7240_buffer_t   *ring_buffer;         /* OS buffer info       */
    int                ring_head;           /* producer index       */
    int                ring_tail;           /* consumer index       */
    int                ring_nelem;          /* nelements            */
}ag7240_ring_t;

typedef struct {
    int stats;
}ag7240_stats_t;

/*
 * 0, 1, 2: based on hardware values for mii ctrl bits [5,4]
 */
typedef enum {
    AG7240_PHY_SPEED_10T,
    AG7240_PHY_SPEED_100TX,
    AG7240_PHY_SPEED_1000T,
}ag7240_phy_speed_t;

/*
 * Represents an ethernet MAC. Contains ethernet devices (LAN and WAN)
 */
#define AG7240_NVDEVS   2

struct eth_led_control;

typedef struct {
    struct net_device      *mac_dev;
    uint32_t                mac_unit;
    uint32_t                mac_base;
    int                     mac_irq;
    uint8_t                 mac_ifup;
    uint8_t                 mac_noacs;
    ag7240_ring_t           mac_txring[4];
    ag7240_ring_t           mac_rxring;
    ag7240_stats_t          mac_stats;
    spinlock_t              mac_lock;
    struct timer_list       mac_oom_timer;
    struct work_struct      mac_tx_timeout;
    struct net_device_stats mac_net_stats;
    ag7240_phy_speed_t      mac_speed;
    int                     mac_fdx;
    struct timer_list       mac_phy_timer;
    ag7240_trc_t            tb;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
    struct napi_struct mac_napi;
#endif
    uint16_t                mac_flags;
}ag7240_mac_t;

#define net_rx_packets      mac_net_stats.rx_packets
#define net_rx_fifo_errors  mac_net_stats.rx_fifo_errors
#define net_tx_packets      mac_net_stats.tx_packets
#define net_rx_bytes        mac_net_stats.rx_bytes
#define net_tx_bytes        mac_net_stats.tx_bytes
#define net_rx_over_errors  mac_net_stats.rx_over_errors
#define net_tx_dropped      mac_net_stats.tx_dropped;

#define ag7240_dev_up(_dev)                                     \
    (((_dev)->flags & (IFF_RUNNING|IFF_UP)) != (IFF_RUNNING|IFF_UP))

typedef enum {
    AG7240_RX_STATUS_DONE,
    AG7240_RX_STATUS_NOT_DONE,
    AG7240_RX_STATUS_OOM,
    AG7240_RX_DMA_HANG
}ag7240_rx_status_t;

/*
 * This defines the interconnects between MAC and PHY at compile time
 * There are several constraints - the Kconfig largely takes care of them
 * at compile time.
 */
#if defined (CONFIG_AG7240_GE0_GMII)
    #define     AG7240_MII0_INTERFACE   0
#elif defined (CONFIG_AG7240_GE0_MII)
    #define     AG7240_MII0_INTERFACE   1
#elif defined (CONFIG_AG7240_GE0_RGMII)
    #define     AG7240_MII0_INTERFACE   2
#elif defined (CONFIG_AG7240_GE0_RMII)
    #define     AG7240_MII0_INTERFACE   3
#endif /*defined (AG7240_GE0_GMII)*/

/*
 * Port 1 may or may not be connected
 */
#if defined(CONFIG_AG7240_GE1_IS_CONNECTED)

    #define AG7240_NMACS            2

    #if defined (CONFIG_AG7240_GE1_GMII)
        #define AG7240_MII1_INTERFACE   0
    #elif defined (CONFIG_AG7240_GE1_MII)
        #define AG7240_MII1_INTERFACE   1
    #elif defined (CONFIG_AG7240_GE1_RGMII)
        #define AG7240_MII1_INTERFACE   2
    #elif defined (CONFIG_AG7240_GE1_RMII)
        #define AG7240_MII1_INTERFACE   3
    #endif /*AG7240_GE1_RGMII*/
#else
    #define AG7240_NMACS            1
    #define AG7240_MII1_INTERFACE   0xff        /*not connected*/
#endif  /*AG7240_GE1_IS_CONNECTED*/

#define mii_reg(_mac)   (AR7240_MII0_CTRL + ((_mac)->mac_unit * 4))
#define mii_if(_mac)    (((_mac)->mac_unit == 0) ? mii0_if : mii1_if)
#define phy_reg_read    ag7240_mii_read
#define phy_reg_write   ag7240_mii_write

#define ag7240_set_mii_ctrl_speed(_mac, _spd)   do {                        \
    ar7240_reg_rmw_clear(mii_reg(_mac), (3 << 4));                          \
    ar7240_reg_rmw_set(mii_reg(_mac), ((_spd) << 4));                       \
}while(0);

/*
 * IP needs 16 bit alignment. But RX DMA needs 4 bit alignment. We sacrifice IP
 * Plus Reserve extra head room for wmac
 */
#define ETHERNET_FCS_SIZE            4
#define AG71XX_TX_FIFO_LEN	2048
#define AG71XX_TX_MTU_LEN	1544
#define AG7240_RX_RESERVE           (64)
#define AG7240_RX_BUF_SIZE      \
    (AG7240_RX_RESERVE + ETH_HLEN + ETH_FRAME_LEN + ETHERNET_FCS_SIZE)



#define ag7240_mac_base(_no)    (_no) ? AR7240_GE1_BASE    : AR7240_GE0_BASE
#define ag7240_mac_irq(_no)     (_no) ? AR7240_CPU_IRQ_GE1 : AR7240_CPU_IRQ_GE0

#define ag7240_reset_mask(_no) (_no) ? (AR7240_RESET_GE1_MAC)   \
                                     : (AR7240_RESET_GE0_MAC)  

#define ag7240_unit2mac(_unit)     ag7240_macs[(_unit)]

#define assert(_cond)   do {                                     \
    if(!(_cond)) {                                               \
        ag7240_trc_dump();                                       \
        printk("%s:%d: assertion failed\n", __func__, __LINE__); \
        BUG();                                                   \
    }                                                            \
}while(0);


/*
 * Config/Mac Register definitions
 */
#define AG7240_MAC_CFG1             0x00
#define AG7240_MAC_CFG2             0x04
#define AG7240_MAC_IFCTL            0x38
#define AG71XX_REG_MAC_IPG	  0x0008
#define AG71XX_REG_MAC_HDX	  0x000c
#define AG71XX_REG_MAC_MFL	  0x0010

/*
 * fifo control registers
 */
#define AG7240_MAC_FIFO_CFG_0      0x48
#define AG7240_MAC_FIFO_CFG_1      0x4c
#define AG7240_MAC_FIFO_CFG_2      0x50
#define AG7240_MAC_FIFO_CFG_3      0x54
#define AG7240_MAC_FIFO_CFG_4      0x58

#define AG7240_MAC_FIFO_CFG_5      0x5c
#define AG7240_BYTE_PER_CLK_EN     (1 << 19)

#define AG7240_MAC_FIFO_RAM_0      0x60
#define AG7240_MAC_FIFO_RAM_1      0x64
#define AG7240_MAC_FIFO_RAM_2      0x68
#define AG7240_MAC_FIFO_RAM_3      0x6c
#define AG7240_MAC_FIFO_RAM_4      0x70
#define AG7240_MAC_FIFO_RAM_5      0x74
#define AG7240_MAC_FIFO_RAM_6      0x78
#define AG7240_MAC_FIFO_RAM_7      0x7c

/*
 * fields
 */
#define AG7240_MAC_CFG1_SOFT_RST       (1 << 31)
#define AG7240_MAC_CFG1_RX_RST         (1 << 19)
#define AG7240_MAC_CFG1_TX_RST         (1 << 18)
#define AG7240_MAC_CFG1_LOOPBACK       (1 << 8)
#define AG7240_MAC_CFG1_RX_EN          (1 << 2)
#define AG7240_MAC_CFG1_TX_EN          (1 << 0)
#define AG7240_MAC_CFG1_RX_FCTL        (1 << 5)
#define AG7240_MAC_CFG1_TX_FCTL        (1 << 4)


#define AG7240_MAC_CFG2_FDX            (1 << 0)
#define AG7240_MAC_CFG2_CRC_EN         (1 << 1)
#define AG7240_MAC_CFG2_PAD_CRC_EN     (1 << 2)
#define AG7240_MAC_CFG2_LEN_CHECK      (1 << 4)
#define AG7240_MAC_CFG2_HUGE_FRAME_EN  (1 << 5)
#define AG7240_MAC_CFG2_IF_1000        (1 << 9)
#define AG7240_MAC_CFG2_IF_10_100      (1 << 8)

#define AG7240_MAC_IFCTL_SPEED         (1 << 16)

/*
 * DMA (tx/rx) register defines
 */
#define AG7240_DMA_TX_CTRL              0x180
#define AG7240_DMA_TX_DESC              0x184
#define AG7240_DMA_TX_STATUS            0x188
#define AG7240_DMA_RX_CTRL              0x18c
#define AG7240_DMA_RX_DESC              0x190
#define AG7240_DMA_RX_STATUS            0x194
#define AG7240_DMA_INTR_MASK            0x198
#define AG7240_DMA_INTR                 0x19c
#define AG7240_DMA_RXFSM		0x1b0
#define AG7240_DMA_TXFSM		0x1b4
#define AG7240_DMA_XFIFO_DEPTH		0x1a8


/*
 * DMA status mask.
 */

#define AG7240_DMA_DMA_STATE 	       0x3
#define AG7240_DMA_AHB_STATE 	       0x7

/*
 * QOS register Defines 
 */

#define AG7240_DMA_TX_CTRL_Q0             0x180
#define AG7240_DMA_TX_CTRL_Q1             0x1C0
#define AG7240_DMA_TX_CTRL_Q2             0x1C8
#define AG7240_DMA_TX_CTRL_Q3             0x1D0

#define AG7240_DMA_TX_DESC_Q0             0x184
#define AG7240_DMA_TX_DESC_Q1             0x1C4
#define AG7240_DMA_TX_DESC_Q2             0x1CC
#define AG7240_DMA_TX_DESC_Q3             0x1D4

#define AG7240_DMA_TX_ARB_CFG             0x1D8
#define AG7240_TX_QOS_MODE_FIXED          0x0
#define AG7240_TX_QOS_MODE_WEIGHTED       0x1
#define AG7240_TX_QOS_WGT_0(x)		  ((x & 0x3F) << 8)
#define AG7240_TX_QOS_WGT_1(x)		  ((x & 0x3F) << 14)
#define AG7240_TX_QOS_WGT_2(x)		  ((x & 0x3F) << 20)
#define AG7240_TX_QOS_WGT_3(x)		  ((x & 0x3F) << 26)

/*
 * tx/rx ctrl and status bits
 */
#define AG7240_TXE                      (1 << 0)
#define AG7240_TX_STATUS_PKTCNT_SHIFT   16
#define AG7240_TX_STATUS_PKT_SENT       0x1
#define AG7240_TX_STATUS_URN            0x2
#define AG7240_TX_STATUS_BUS_ERROR      0x8

#define AG7240_RXE                      (1 << 0)

#define AG7240_RX_STATUS_PKTCNT_MASK    0xff0000
#define AG7240_RX_STATUS_PKT_RCVD       (1 << 0)
#define AG7240_RX_STATUS_OVF            (1 << 2)
#define AG7240_RX_STATUS_BUS_ERROR      (1 << 3)

/*
 * Int and int mask
 */
#define AG7240_INTR_TX                  (1 << 0)
#define AG7240_INTR_TX_URN              (1 << 1)
#define AG7240_INTR_TX_BUS_ERROR        (1 << 3)
#define AG7240_INTR_RX                  (1 << 4)
#define AG7240_INTR_RX_OVF              (1 << 6)
#define AG7240_INTR_RX_BUS_ERROR        (1 << 7)

/*
 * MII registers
 */
#define AG7240_MAC_MII_MGMT_CFG         0x20
#define AG7240_MGMT_CFG_CLK_DIV_20      0x06
#define AG7240_MII_MGMT_CMD             0x24
#define AG7240_MGMT_CMD_READ            0x1
#define AG7240_MII_MGMT_ADDRESS         0x28
#define AG7240_ADDR_SHIFT               8
#define AG7240_MII_MGMT_CTRL            0x2c
#define AG7240_MII_MGMT_STATUS          0x30
#define AG7240_MII_MGMT_IND             0x34
#define AG7240_MGMT_IND_BUSY            (1 << 0)
#define AG7240_MGMT_IND_INVALID         (1 << 2)
#define AG7240_GE_MAC_ADDR1             0x40
#define AG7240_GE_MAC_ADDR2             0x44
#define AG7240_MII0_CONTROL             0x18070000

/*
 * MIB Registers
 */
#define AG7240_RX_PKT_CNTR		0xa0
#define AG7240_TX_PKT_CNTR		0xe4
#define AG7240_RX_BYTES_CNTR		0x9c
#define AG7240_TX_BYTES_CNTR		0xe0
#define AG7240_RX_LEN_ERR_CNTR		0xc0
#define AG7240_RX_OVL_ERR_CNTR		0xd0
#define AG7240_RX_CRC_ERR_CNTR		0xa4
#define AG7240_RX_FRM_ERR_CNTR		0xbc
#define AG7240_RX_CODE_ERR_CNTR		0xc4
#define AG7240_RX_CRS_ERR_CNTR		0xc8
#define AG7240_RX_DROP_CNTR		0xdc
#define AG7240_TX_DROP_CNTR		0x114
#define AG7240_RX_MULT_CNTR		0xa8
#define AG7240_TX_MULT_CNTR		0xe8
#define AG7240_TOTAL_COL_CNTR		0x10c
#define AG7240_TX_CRC_ERR_CNTR		0x11c


#define AG7240_ETH_CFG                  0x18070000
#define AG7240_ETH_CFG_RGMII_GE0        (1<<0)
#define AG7240_ETH_CFG_MII_GE0          (1<<1)
#define AG7240_ETH_CFG_GMII_GE0         (1<<2)
#define AG7240_ETH_CFG_MII_GE0_MASTER   (1<<3)
#define AG7240_ETH_CFG_MII_GE0_SLAVE    (1<<4)
#define AG7240_ETH_CFG_GE0_ERR_EN       (1<<5)
#define AG7240_ETH_CFG_SW_ONLY_MODE     (1<<6)
#define AG7240_ETH_CFG_SW_PHY_SWAP      (1<<7)
#define AG7240_ETH_CFG_SW_PHY_ADDR_SWAP (1<<8)

/*
 * Everything but TX
 */
#define AG7240_INTR_MASK    (AG7240_INTR_RX | \
                             AG7240_INTR_RX_BUS_ERROR |             \
                             AG7240_INTR_TX_BUS_ERROR              \
                             /*| AG7240_INTR_TX_URN | AG7240_INTR_TX*/)

#define ag7240_reg_rd(_mac, _reg)                                       \
            (ar7240_reg_rd((_mac)->mac_base + (_reg)))

#define ag7240_reg_wr(_mac, _reg, _val)                                 \
                ar7240_reg_wr((_mac)->mac_base + (_reg), (_val));

/*
 * The no flush version
 */
#define ag7240_reg_wr_nf(_mac, _reg, _val)                             \
                ar7240_reg_wr_nf((_mac)->mac_base + (_reg), (_val));

#define ag7240_reg_rmw_set(_mac, _reg, _mask)                           \
                 ar7240_reg_rmw_set((_mac)->mac_base + (_reg), (_mask));

#define ag7240_reg_rmw_clear(_mac, _reg, _mask)                          \
                 ar7240_reg_rmw_clear((_mac)->mac_base + (_reg), (_mask));


#define ag7240_desc_dma_addr(_r, _ds)                                   \
    (u32)((ag7240_desc_t *)(_r)->ring_desc_dma + ((_ds) - ((_r)->ring_desc)))


/*
 * tx/rx stop start
 */
#define ag7240_tx_stopped(_mac)                                         \
    (!(ag7240_reg_rd((_mac), AG7240_DMA_TX_CTRL) & AG7240_TXE))

#define ag7240_rx_start(_mac)                                           \
    ag7240_reg_wr((_mac), AG7240_DMA_RX_CTRL, AG7240_RXE)

#define ag7240_rx_stop(_mac)                                            \
    ag7240_reg_wr((_mac), AG7240_DMA_RX_CTRL, 0)


#define ag7240_tx_start_qos(_mac,ac)                                       \
switch(ac) {  								   \
	case ENET_AC_VO: 							   \
    		ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL_Q0, AG7240_TXE);  \
		break;							   \
	case ENET_AC_VI: 						   \
    		ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL_Q1, AG7240_TXE);  \
		break;							   \
	case ENET_AC_BK: 						   \
    		ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL_Q2, AG7240_TXE);  \
		break;							   \
	case ENET_AC_BE: 						   \
    		ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL_Q3, AG7240_TXE);  \
		break;							   \
}


#define ag7240_tx_start(_mac)                                           \
    ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL, AG7240_TXE)


#define ag7240_tx_stop(_mac)						\
    ag7240_reg_wr((_mac), AG7240_DMA_TX_CTRL, 0)

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
    ag7240_ring_t *r    = &mac->mac_rxring;
    int            tail = r->ring_tail;

    return ((r->ring_head == tail) && !r->ring_buffer[tail].buf_pkt);
}

#define ag7240_ring_incr(_idx)                                             \
        if(unlikely(++(_idx) == r->ring_nelem)) (_idx) = 0;

/*
 * ownership of descriptors between DMA and cpu
 */
#define ag7240_rx_owned_by_dma(_ds)     ((_ds)->is_empty == 1)
#define ag7240_rx_give_to_dma(_ds)      ((_ds)->is_empty = 1)
#define ag7240_tx_owned_by_dma(_ds)     ((_ds)->is_empty == 0)
#define ag7240_tx_give_to_dma(_ds)      ((_ds)->is_empty = 0)
#define ag7240_tx_own(_ds)              ((_ds)->is_empty = 1)

/*
 * Interrupts 
 * ----------
 */
#define ag7240_get_isr(_mac) ag7240_reg_rd((_mac), AG7240_DMA_INTR);
#define ag7240_int_enable(_mac)                                         \
    ag7240_reg_wr(_mac, AG7240_DMA_INTR_MASK, AG7240_INTR_MASK)

#define ag7240_int_disable(_mac)                                        \
    ag7240_reg_wr(_mac, AG7240_DMA_INTR_MASK, 0)
/*
 * ACKS:
 * - We just write our bit - its write 1 to clear.
 * - These are not rmw's so we dont need locking around these.
 * - Txurn and rxovf are not fastpath and need consistency, so we use the flush
 *   version of reg write.
 * - ack_rx is done every packet, and is largely only for statistical purposes;
 *   so we use the no flush version and later cause an explicite flush.
 */
#define ag7240_intr_ack_txurn(_mac)                                           \
    ag7240_reg_wr((_mac), AG7240_DMA_TX_STATUS, AG7240_TX_STATUS_URN);
#define ag7240_intr_ack_rx(_mac)                                              \
    ag7240_reg_wr_nf((_mac), AG7240_DMA_RX_STATUS, AG7240_RX_STATUS_PKT_RCVD);
#define ag7240_intr_ack_rxovf(_mac)                                           \
    ag7240_reg_wr((_mac), AG7240_DMA_RX_STATUS, AG7240_RX_STATUS_OVF);

/*
 * Not used currently
 */
#define ag7240_intr_ack_tx(_mac)                                              \
    ag7240_reg_wr((_mac), AG7240_DMA_TX_STATUS, AG7240_TX_STATUS_PKT_SENT);
#define ag7240_intr_ack_txbe(_mac)                                            \
    ag7240_reg_wr((_mac), AG7240_DMA_TX_STATUS, AG7240_TX_STATUS_BUS_ERROR);
#define ag7240_intr_ack_rxbe(_mac)                                            \
    ag7240_reg_wr((_mac), AG7240_DMA_RX_STATUS, AG7240_RX_STATUS_BUS_ERROR);

/*
 * Enable Disable. These are Read-Modify-Writes. Sometimes called from ISR
 * sometimes not. So the caller explicitely handles locking.
 */
#define ag7240_intr_disable_txurn(_mac)                                     \
    ag7240_reg_rmw_clear((_mac), AG7240_DMA_INTR_MASK, AG7240_INTR_TX_URN);

#define ag7240_intr_enable_txurn(_mac)                                      \
    ag7240_reg_rmw_set((_mac), AG7240_DMA_INTR_MASK, AG7240_INTR_TX_URN);

#define ag7240_intr_enable_tx(_mac)                                      \
    ag7240_reg_rmw_set((_mac), AG7240_DMA_INTR_MASK, AG7240_INTR_TX);

#define ag7240_intr_disable_tx(_mac)                                     \
    ag7240_reg_rmw_clear((_mac), AG7240_DMA_INTR_MASK, AG7240_INTR_TX);

#define ag7240_intr_disable_recv(_mac)                                      \
    ag7240_reg_rmw_clear(mac, AG7240_DMA_INTR_MASK,                         \
                        (AG7240_INTR_RX ));

#define ag7240_intr_enable_rxovf(_mac)                                      \
    ag7240_reg_rmw_set((_mac), AG7240_DMA_INTR_MASK, AG7240_INTR_RX_OVF);

#define ag7240_intr_disable_rxovf(_mac)                                      \
    ag7240_reg_rmw_clear(mac, AG7240_DMA_INTR_MASK,                         \
                        (AG7240_INTR_RX_OVF));


#define ag7240_intr_enable_recv(_mac)                                      \
    ag7240_reg_rmw_set(mac, AG7240_DMA_INTR_MASK,                          \
                        (AG7240_INTR_RX | AG7240_INTR_RX_OVF));

/*
 * link settings
 */
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

uint16_t ag7240_mii_read(int unit, uint32_t phy_addr, uint8_t reg);
void ag7240_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data);
unsigned int s26_rd_phy(unsigned int phy_addr, unsigned int reg_addr);
int  ag7240_check_link(ag7240_mac_t *mac,int phyUnit);
int  ag7242_check_link(ag7240_mac_t *mac);

#define CHECK_DMA_STATUS    0x0001
#define ETH_SOFT_LED        0x0002
#define WAN_QOS_SOFT_CLASS      0x0004
#define ETH_SWONLY_MODE         0x0008
#define ATHR_S26_HEADER         0x0010
#define ATHR_S16_HEADER         0x0020
#define ETH_PKT_INSPECT         0x0040

static inline int
mac_has_flag(ag7240_mac_t *mac, u_int16_t flag)
{
      return ((mac->mac_flags & flag) != 0);
}

static inline void
mac_set_flag(ag7240_mac_t *mac, u_int16_t flag)
{
    mac->mac_flags |= flag;
    return;
    
}

static inline void
mac_clear_flag(ag7240_mac_t *mac, u_int16_t flag)
{
    mac->mac_flags &= ~flag;
    return;
}

/**
 * Added for customizing LED control operations
 */

typedef struct {
    u_int32_t    rate;        // Rate per second
    u_int32_t    timeOn;      // LED ON time in ms
    u_int32_t    timeOff;     // LED OFF time in ms
} LED_BLINK_RATES;

typedef struct eth_led_control {
    uint8_t                ledlink[5];                 // current link state
    ag7240_phy_speed_t     speed[5];                   // current link speed 
    struct timer_list      led_timer;
} ATH_LED_CONTROL;


#define MB(x) ((x * 100000) / 8)  /* Mbits/sec */
static const
LED_BLINK_RATES BlinkRateTable_100M[] = {
    {  MB(1),   5,  5 }, /* on:168ms off:670ms */
    {  MB(5),   4,  3 }, /* on:84ms  off:168ms */
    {  MB(10),  2,  2 }, /* on:21ms  off:84ms */
    {  MB(50),  2,  1 }, /* on:21ms  off:42ms */
    {  MB(100), 1,  1 }, /* on:10ms  off:42ms */
    {  0xffffffff, 0 , 1 }
};

static const
LED_BLINK_RATES BlinkRateTable_10M[] = {
    {  MB(1),   5,  5 }, /* on:168ms off:670ms */
    {  MB(3),   4,  3 }, /* on:84ms  off:168ms */
    {  MB(5),   2,  2 }, /* on:21ms  off:84ms */
    {  MB(7),   2,  1 }, /* on:21ms  off:42ms */
    {  MB(10),  1,  1 }, /* on:10ms  off:42ms */
    {  0xffffffff, 0 , 1 }
};

#endif
