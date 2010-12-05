#ifndef _AG7100_H
#define _AG7100_H

#include <cyg/hal/ar7240_soc.h>

#ifdef __ECOS
#include "ag7240_ecos.h" 
#define uint8_t  cyg_uint8
#define uint16_t cyg_uint16
#define uint32_t cyg_uint32
#endif

#define AR7240_IF_GMII  0
#define AR7240_IF_MII   1
#define AR7240_IF_RGMII 2
#define AR7240_IF_RMII  3

/*
 * DEBUG switches to control verbosity.
 * Just modify the value of ag7100_debug.
 */
#define AG7100_DEBUG_ALL         0xffffffff
#define AG7100_DEBUG_ERROR       0x00000001 /* Unusual conditions and Errors */
#define AG7100_DEBUG_ARRIVE      0x00000002 /* Arrive into a function */
#define AG7100_DEBUG_LEAVE       0x00000004 /* Leave a function */
#define AG7100_DEBUG_RESET       0x00000008 /* Reset */
#define AG7100_DEBUG_TX          0x00000010 /* Transmit */
#define AG7100_DEBUG_TX_REAP     0x00000020 /* Transmit Descriptor Reaping */
#define AG7100_DEBUG_RX          0x00000040 /* Receive */
#define AG7100_DEBUG_RX_STOP     0x00000080 /* Receive Early Stop */
#define AG7100_DEBUG_INT         0x00000100 /* Interrupts */
#define AG7100_DEBUG_LINK_CHANGE 0x00000200 /* PHY Link status changed */
#define AG7100_DEBUG_VERBOSE     0x00000400 /* Verbose output */

extern int ag7100_debug;
#if 0
#define AG7100_PRINT(FLG, X)                       \
{                                                   \
    if (ag7100_debug & (FLG)) {                     \
        DEBUG_PRINTF("%s#%d:%s ",                   \
                     __FILE__,                      \
                     __LINE__,                      \
                     __FUNCTION__);                 \
        DEBUG_PRINTF X;                             \
    }                                               \
}
#else
#define AG7100_PRINT(FLG, X) 
#endif

#define ARRIVE() AG7100_PRINT(AG7100_DEBUG_ARRIVE, ("Arrive{\n"))
#define LEAVE() AG7100_PRINT(AG7100_DEBUG_LEAVE, ("}Leave\n"))

/*
 * h/w descriptor
 */
typedef struct {
    uint32_t    pkt_start_addr;

    uint32_t    is_empty       :  1;
    uint32_t    res1           : 10;
    uint32_t    ftpp_override  :  5;
    uint32_t    res2           :  4;
    uint32_t    pkt_size       : 12;

    uint32_t    next_desc      ;
    void        *sw_buf;
}ag7100_desc_t;

/*
 * s/w descriptor
 */
typedef struct {
    void *buf_pkt;
}ag7100_buffer_t;

/*
 * Tx and Rx descriptor rings;
 */
typedef struct ag7100_ring{
    ag7100_desc_t     *ring_desc;           /* hardware descriptors */
    a_dma_addr_t      ring_desc_dma;        /* dma addresses of desc*/
    ag7100_buffer_t   *ring_buffer;         /* OS buffer info       */
    int                ring_head;           /* producer index       */
    int                ring_tail;           /* consumer index       */
    int                ring_nelem;          /* nelements            */
}ag7100_ring_t;

typedef struct {
    int stats;
}ag7100_stats_t;

typedef enum {
    AG7100_PHY_SPEED_NONE,      /* not initialized yet */
    AG7100_PHY_SPEED_1000T,
    AG7100_PHY_SPEED_100TX,
    AG7100_PHY_SPEED_10T
}ag7100_phy_speed_t;

/*
 * Represents an ethernet MAC. Contains ethernet devices (LAN and WAN)
 */
typedef struct {
    ag7100_desc_t          *fifo_tx[AG7100_TX_DESC_CNT];
    ag7100_desc_t          *fifo_rx[AG7100_RX_DESC_CNT];
    uint32_t                next_tx;
    uint32_t                next_rx;
    void                   *mac_osdep;
    uint32_t                mac_unit;
    uint32_t                mac_base;
    int                     mac_irq;
    ag7100_stats_t          mac_stats;
    ag7100_phy_speed_t      mac_speed;
    int                     mac_duplex;
    int                     link;
}ag7100_mac_t;

typedef enum {
    AG7100_RX_STATUS_DONE,
    AG7100_RX_STATUS_NOT_DONE,
    AG7100_RX_STATUS_OOM
}ag7100_rx_status_t;

#define AG7100_NMACS            2

#define AG7100_PHY_POLL_SECONDS 2

#define AG7100_TX_REAP_THRESH   AG7100_TX_DESC_CNT/2

#define ag7100_unit2mac(_unit)     ag7100_macs[(_unit)]
#define ag7100_mac_base(_no)    (_no) ? AR7240_GE1_BASE    : AR7240_GE0_BASE

#define ag7100_reset_mask(_no) (_no) ? (AR7240_RESET_GE1_MAC |  \
                                        AR7240_RESET_GE1_PHY)   \
                                     : (AR7240_RESET_GE0_MAC |  \
                                        AR7240_RESET_GE0_PHY)

#define MAX_WAIT        1000

#if 0
#define assert(_cond)   do           {                          \
    if(!(_cond))       {                                        \
        printk("assert %s %d\n", __FILE__, __LINE__);           \
        ag7100_trc_dump();                                      \
        panic("shit");                                          \
    }                                                           \
}while(0);
#endif

/*
 * Config/Mac Register definitions
 */
#define AG7100_MAC_CFG1             0x00
#define AG7100_MAC_CFG2             0x04
#define AG7100_MAC_IFCTL            0x38

/*
 * fifo control registers
 */
#define AG7100_MAC_FIFO_CFG_0      0x48
#define AG7100_MAC_FIFO_CFG_1      0x4c
#define AG7100_MAC_FIFO_CFG_2      0x50
#define AG7100_MAC_FIFO_CFG_3      0x54
#define AG7100_MAC_FIFO_CFG_4      0x58

#define AG7100_MAC_FIFO_CFG_5      0x5c
#define AG7100_BYTE_PER_CLK_EN     (1 << 19)

#define AG7100_MAC_FIFO_RAM_0      0x60
#define AG7100_MAC_FIFO_RAM_1      0x64
#define AG7100_MAC_FIFO_RAM_2      0x68
#define AG7100_MAC_FIFO_RAM_3      0x6c
#define AG7100_MAC_FIFO_RAM_4      0x70
#define AG7100_MAC_FIFO_RAM_5      0x74
#define AG7100_MAC_FIFO_RAM_6      0x78
#define AG7100_MAC_FIFO_RAM_7      0x7c

/*
 * fields
 */
#define AG7100_MAC_CFG1_SOFT_RST       (1 << 31)
#define AG7100_MAC_CFG1_LOOPBACK       (1 << 8)
#define AG7100_MAC_CFG1_RX_EN          (1 << 2)
#define AG7100_MAC_CFG1_TX_EN          (1 << 0)


#define AG7100_MAC_CFG2_FDX            (1 << 0)
#define AG7100_MAC_CFG2_PAD_CRC_EN     (1 << 2)
#define AG7100_MAC_CFG2_LEN_CHECK      (1 << 4)
#define AG7100_MAC_CFG2_HUGE_FRAME_EN  (1 << 5)
#define AG7100_MAC_CFG2_IF_1000        (1 << 9)
#define AG7100_MAC_CFG2_IF_10_100      (1 << 8)

#define AG7100_MAC_IFCTL_SPEED         (1 << 16)

/*
 * DMA (tx/rx) register defines
 */
#define AG7100_DMA_TX_CTRL              0x180
#define AG7100_DMA_TX_DESC              0x184
#define AG7100_DMA_TX_STATUS            0x188
#define AG7100_DMA_RX_CTRL              0x18c
#define AG7100_DMA_RX_DESC              0x190
#define AG7100_DMA_RX_STATUS            0x194
#define AG7100_DMA_INTR_MASK            0x198
#define AG7100_DMA_INTR                 0x19c

/*
 * tx/rx ctrl and status bits
 */
#define AG7100_TXE                      (1 << 0)
#define AG7100_TX_STATUS_PKTCNT_SHIFT   16
#define AG7100_TX_STATUS_PKT_SENT       0x1
#define AG7100_TX_STATUS_URN            0x2
#define AG7100_TX_STATUS_BUS_ERROR      0x8

#define AG7100_RXE                      (1 << 0)

#define AG7100_RX_STATUS_PKTCNT_MASK    0xff0000
#define AG7100_RX_STATUS_PKT_RCVD       (1 << 0)
#define AG7100_RX_STATUS_OVF            (1 << 2)
#define AG7100_RX_STATUS_BUS_ERROR      (1 << 3)

/*
 * Int and int mask
 */
#define AG7100_INTR_TX                  (1 << 0)
#define AG7100_INTR_TX_URN              (1 << 1)
#define AG7100_INTR_TX_BUS_ERROR        (1 << 3)
#define AG7100_INTR_RX                  (1 << 4)
#define AG7100_INTR_RX_OVF              (1 << 6)
#define AG7100_INTR_RX_BUS_ERROR        (1 << 7)

/*
 * MII registers
 */
#define AG7100_MAC_MII_MGMT_CFG         0x20
#define AG7100_MGMT_CFG_CLK_DIV_20      0x06
#define AG7100_MII_MGMT_CMD             0x24
#define AG7100_MGMT_CMD_READ            0x1
#define AG7100_MII_MGMT_ADDRESS         0x28
#define AG7100_ADDR_SHIFT               8
#define AG7100_MII_MGMT_CTRL            0x2c
#define AG7100_MII_MGMT_STATUS          0x30
#define AG7100_MII_MGMT_IND             0x34
#define AG7100_MGMT_IND_BUSY            (1 << 0)
#define AG7100_MGMT_IND_INVALID         (1 << 2)
#define AG7100_GE_MAC_ADDR1             0x40
#define AG7100_GE_MAC_ADDR2             0x44
#define AG7100_MII0_CONTROL             0x18070000

/*
 * Everything but TX
 */
#define AG7100_INTR_MASK    (AG7100_INTR_RX | AG7100_INTR_RX_OVF |  \
                             AG7100_INTR_RX_BUS_ERROR |             \
                             AG7100_INTR_TX_BUS_ERROR              \
                             /*| AG7100_INTR_TX_URN | AG7100_INTR_TX*/)

#define ag7100_reg_rd(_mac, _reg)                                       \
            (ar7240_reg_rd((_mac)->mac_base + (_reg)))

#define ag7100_reg_wr(_mac, _reg, _val)                                 \
                ar7240_reg_wr((_mac)->mac_base + (_reg), (_val));

/*
 * The no flush version
 */
#define ag7100_reg_wr_nf(_mac, _reg, _val)                             \
                ar7240_reg_wr_nf((_mac)->mac_base + (_reg), (_val));

#define ag7100_reg_rmw_set(_mac, _reg, _mask)                           \
                 ar7240_reg_rmw_set((_mac)->mac_base + (_reg), (_mask));

#define ag7100_reg_rmw_clear(_mac, _reg, _mask)                          \
                 ar7240_reg_rmw_clear((_mac)->mac_base + (_reg), (_mask));


#define ag7100_desc_dma_addr(_r, _ds)                                   \
    (uint32_t)((ag7100_desc_t *)(_r)->ring_desc_dma + ((_ds) - ((_r)->ring_desc)))


/*
 * tx/rx stop start
 */
#define ag7100_tx_stopped(_mac)                                         \
    (!(ag7100_reg_rd((_mac), AG7100_DMA_TX_CTRL) & AG7100_TXE))

#define ag7100_rx_start(_mac)                                           \
    ag7100_reg_wr((_mac), AG7100_DMA_RX_CTRL, AG7100_RXE)

#define ag7100_rx_stop(_mac)                                            \
    ag7100_reg_wr((_mac), AG7100_DMA_RX_CTRL, 0)

#define ag7100_tx_start(_mac)                                           \
    ag7100_reg_wr((_mac), AG7100_DMA_TX_CTRL, AG7100_TXE)

#define ag7100_tx_stop(_mac)                                            \
    ag7100_reg_wr((_mac), AG7100_DMA_TX_CTRL, 0)

#if 0
/*
 * Clean only when at least half the ring is full
 */
static inline int ag7100_tx_reap_thresh(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_txring;

    return(((r->ring_tail + AG7100_TX_REAP_THRESH) % r->ring_nelem) <= 
            r->ring_head);
}

static inline int ag7100_tx_ring_full(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_txring;

    return (((r->ring_head + 2) % r->ring_nelem) == r->ring_tail);
}

static inline int ag7100_rx_ring_oom(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_rxring;

    return (((r->ring_head + 1) % r->ring_nelem) == r->ring_tail);
}

#define ag7100_ring_incr(_r, _idx)                                             \
        if(++(_idx) == _r->ring_nelem) (_idx) = 0;
#endif

/*
 * ownership of descriptors between DMA and cpu
 */
#define ag7100_rx_owned_by_dma(_ds)     ((_ds)->is_empty == 1)
#define ag7100_rx_give_to_dma(_ds)      ((_ds)->is_empty = 1)
#define ag7100_tx_owned_by_dma(_ds)     ((_ds)->is_empty == 0)
#define ag7100_tx_give_to_dma(_ds)      ((_ds)->is_empty = 0)
#define ag7100_tx_own(_ds)              ((_ds)->is_empty = 1)

/*
 * Interrupts 
 * ----------
 */
#define ag7100_get_isr(_mac) ag7100_reg_rd((_mac), AG7100_DMA_INTR);
#define ag7100_int_enable(_mac)                                         \
    ag7100_reg_wr(_mac, AG7100_DMA_INTR_MASK, AG7100_INTR_MASK)

#define ag7100_int_disable(_mac)                                        \
    ag7100_reg_wr(_mac, AG7100_DMA_INTR_MASK, 0)
/*
 * ACKS:
 * - We just write our bit - its write 1 to clear.
 * - These are not rmw's so we dont need locking around these.
 * - Txurn and rxovf are not fastpath and need consistency, so we use the flush
 *   version of reg write.
 * - ack_rx is done every packet, and is largely only for statistical purposes;
 *   so we use the no flush version and later cause an explicite flush.
 */
#define ag7100_intr_ack_txurn(_mac)                                           \
    ag7100_reg_wr((_mac), AG7100_DMA_TX_STATUS, AG7100_TX_STATUS_URN);
#define ag7100_intr_ack_rx(_mac)                                              \
    ag7100_reg_wr_nf((_mac), AG7100_DMA_RX_STATUS, AG7100_RX_STATUS_PKT_RCVD);
#define ag7100_intr_ack_rxovf(_mac)                                           \
    ag7100_reg_wr((_mac), AG7100_DMA_RX_STATUS, AG7100_RX_STATUS_OVF);
/*
 * Not used currently
 */
#define ag7100_intr_ack_tx(_mac)                                              \
    ag7100_reg_wr((_mac), AG7100_DMA_TX_STATUS, AG7100_TX_STATUS_PKT_SENT);
#define ag7100_intr_ack_txbe(_mac)                                            \
    ag7100_reg_wr((_mac), AG7100_DMA_TX_STATUS, AG7100_TX_STATUS_BUS_ERROR);
#define ag7100_intr_ack_rxbe(_mac)                                            \
    ag7100_reg_wr((_mac), AG7100_DMA_RX_STATUS, AG7100_RX_STATUS_BUS_ERROR);

/*
 * Enable Disable. These are Read-Modify-Writes. Sometimes called from ISR
 * sometimes not. So the caller explicitely handles locking.
 */
#define ag7100_intr_disable_txurn(_mac)                                     \
    ag7100_reg_rmw_clear((_mac), AG7100_DMA_INTR_MASK, AG7100_INTR_TX_URN);

#define ag7100_intr_enable_txurn(_mac)                                      \
    ag7100_reg_rmw_set((_mac), AG7100_DMA_INTR_MASK, AG7100_INTR_TX_URN);

#define ag7100_intr_disable_recv(_mac)                                      \
    ag7100_reg_rmw_clear(_mac, AG7100_DMA_INTR_MASK,                         \
                        (AG7100_INTR_RX | AG7100_INTR_RX_OVF));

#define ag7100_intr_enable_recv(_mac)                                      \
    ag7100_reg_rmw_set(_mac, AG7100_DMA_INTR_MASK,                          \
                        (AG7100_INTR_RX | AG7100_INTR_RX_OVF));
/*
 * link settings
 */
static inline void ag7100_set_mac_duplex(ag7100_mac_t *mac, int fdx)
{
    if (fdx) {
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, AG7100_MAC_CFG2_FDX);
    }
    else {
        ag7100_reg_rmw_clear(mac, AG7100_MAC_CFG2, AG7100_MAC_CFG2_FDX);
    }
}

static inline void ag7100_set_mac_if(ag7100_mac_t *mac, int is_xgmii)
{
    if (is_xgmii) {                                                        
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, AG7100_MAC_CFG2_IF_1000);
        ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_5, AG7100_BYTE_PER_CLK_EN);                          
    }                                                                        
    else {                                                                   
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, AG7100_MAC_CFG2_IF_10_100);
        ag7100_reg_rmw_clear(mac,AG7100_MAC_FIFO_CFG_5, AG7100_BYTE_PER_CLK_EN);
    }                                                                        
}

static inline void ag7100_set_mac_speed(ag7100_mac_t *mac, int is100)
{
    if (is100) {
        ag7100_reg_rmw_set(mac, AG7100_MAC_IFCTL, AG7100_MAC_IFCTL_SPEED); 
    }
    else {
        ag7100_reg_rmw_clear(mac, AG7100_MAC_IFCTL, AG7100_MAC_IFCTL_SPEED);
    }
}

static inline void ag7100_set_mii_ctrl_speed(ag7100_mac_t *mac, ag7100_phy_speed_t spd)
{
    uint32_t mii_ctrl = (mac->mac_unit)?AR7240_MII1_CTRL:AR7240_MII0_CTRL;

    ar7240_reg_rmw_clear(mii_ctrl, (3 << 4));
    if(spd == AG7100_PHY_SPEED_100TX) { 
        ar7240_reg_rmw_set(mii_ctrl,  0x10);    
    } else if(spd == AG7100_PHY_SPEED_1000T) {
        ar7240_reg_rmw_set(mii_ctrl, 0x20);
    }
}

uint16_t ag7100_mii_read(int unit, uint32_t phy_addr, uint8_t reg);
void ag7100_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data);


void ag7100_reset(ag7100_mac_t *mac); 
int ag7100_devopen(ag7100_mac_t *mac);
int ag7100_devclose(ag7100_mac_t *mac);
int ag7100_xmit(void *pktbuf, ag7100_mac_t *mac);
int ag7100_handle_intr(ag7100_mac_t *mac);
int ag7100_process_rx(ag7100_mac_t *mac, int quota);
void ag7100_get_macaddr(ag7100_mac_t *mac, uint8_t *mac_addr);

#endif
