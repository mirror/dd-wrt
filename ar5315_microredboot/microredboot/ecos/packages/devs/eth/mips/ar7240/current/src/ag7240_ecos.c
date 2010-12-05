//==========================================================================
//
//      dev/ag7100ecos.c
//
//      Ethernet driver for AG7100
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AG7100 ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include <string.h>

#include "ag7240_ecos.h"
#include "ag7240.h"
#include "ag7240_phy.h"

#ifndef CYGPKG_REDBOOT
//#define DEBUG
#endif

typedef struct ag7100RxBuf_s {
    struct ag7100RxBuf_s *next;
    uint8_t              *data;
} ag7100RxBuf_t;

#define AG7100_RX_DATA_BUF_COUNT 2*AG7100_RX_DESC_CNT
#define AG7100_TX_DATA_BUF_COUNT AG7100_TX_DESC_CNT
#define AG7100_LINK_CHK_INTVL 0xffff
typedef struct ag7100TxBuf_s {
    struct ag7100TxBuf_s *next;
    unsigned long        key;
    uint8_t              *data;
} ag7100TxBuf_t;

typedef struct ag7100_priv_data_s {
    int            enetUnit;
    uint8_t        enet_addr[ETHER_ADDR_LEN];
    ag7100_mac_t   MACInfo;
    ag7100TxBuf_t  txDataBuf[AG7100_TX_DESC_CNT];
    ag7100TxBuf_t  *txFreeList;
    unsigned int   txAvail;
    ag7100RxBuf_t  rxDataBuf[AG7100_RX_DATA_BUF_COUNT];
    uint8_t        rxBuf[AG7100_RX_DATA_BUF_COUNT * AG7100_RX_BUF_SIZE + HAL_DCACHE_LINE_SIZE - 1];
    uint8_t        txBuf[AG7100_TX_DATA_BUF_COUNT * AG7100_TX_BUF_SIZE + HAL_DCACHE_LINE_SIZE - 1];
    ag7100RxBuf_t *rxFreeList;
    ag7100RxBuf_t *rxRecvdListHead;
    ag7100RxBuf_t *rxRecvdListTail;
    struct eth_drv_sc       *sc;
    uint32_t       linkChkCntr;
} ag7100_priv_data_t;

struct os_tx_pkt {
    struct eth_drv_sg *sg_list;
    int sg_len;
    int total_len;
    unsigned long key;
};

static void *ag7100_rxbuf_alloc(ag7100_priv_data_t *ag7100_priv, uint8_t **rxBptr);
static void ag7100_rxbuf_free(ag7100_priv_data_t *ag7100_priv, void *buf);
static int  ag7100_check_link(ag7100_mac_t *mac);
void ag7100_get_macaddr(ag7100_mac_t *mac, uint8_t *mac_addr);

ag7100_mac_t *ag7100_macs[2];
int ag7100_debug = AG7100_DEBUG_ERROR;

static int
ag7100_alloc_fifo(int ndesc, ag7100_desc_t **fifo)
{
    int i;
    uint32_t size;
    uint8_t *p = NULL;

    size  = sizeof(ag7100_desc_t) * ndesc;
    size += HAL_DCACHE_LINE_SIZE - 1;

    if ((p = malloc(size)) == NULL) {
        AG7100_PRINT(AG7100_DEBUG_ERROR, ("Cant allocate fifos\n"));
        return -1;
    }

    p = (uint8_t*)(((uint32_t)p + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1));
    p = CYGARC_UNCACHED_ADDRESS(p);

    for(i = 0; i < ndesc; i++)
        fifo[i] = (ag7100_desc_t *)p + i;

    return 0;
}

void *
ag7100_rxbuf_alloc(ag7100_priv_data_t *ag7100_priv, uint8_t **rxBptr)
{
    ag7100RxBuf_t *rxbuf;
    unsigned long oldIntrState;

    intDisable(oldIntrState);
    if (ag7100_priv->rxFreeList) {
        rxbuf = ag7100_priv->rxFreeList;
        ag7100_priv->rxFreeList = ag7100_priv->rxFreeList->next;
        intEnable(oldIntrState);

        *rxBptr = rxbuf->data;
        return (void *)rxbuf;
    }
    intEnable(oldIntrState);

    *rxBptr = NULL;

    return (void *)NULL;
}

static int
ag7100_setup_fifos(ag7100_mac_t *mac)
{
    int i; 
    uint8_t *rxBufData;
    ag7100_priv_data_t *ag7100_priv = (ag7100_priv_data_t *)mac->mac_osdep;
    ag7100_desc_t *fr;
    uint8_t *p;

    /* Setup Transmit buffer free list */
    ag7100_priv->txFreeList = 0;
    for (i=0; i<AG7100_TX_DESC_CNT; i++) {
        ag7100_priv->txDataBuf[i].next = ag7100_priv->txFreeList;
        ag7100_priv->txFreeList = &(ag7100_priv->txDataBuf[i]);
    }
    ag7100_priv->txAvail = AG7100_TX_DESC_CNT;

    p = (uint8_t*)(((uint32_t)(ag7100_priv->rxBuf) + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1));
    /* Setup Receive buffer free list */
    ag7100_priv->rxFreeList = 0;
    for (i=0; i<AG7100_RX_DATA_BUF_COUNT; i++) {
        ag7100_priv->rxDataBuf[i].next = ag7100_priv->rxFreeList;
        ag7100_priv->rxFreeList = &(ag7100_priv->rxDataBuf[i]);
        ag7100_priv->rxDataBuf[i].data = p + i * AG7100_RX_BUF_SIZE;
    }

    if (ag7100_alloc_fifo(AG7100_TX_DESC_CNT, mac->fifo_tx))
        return 1;

    p = (uint8_t*)(((uint32_t)(ag7100_priv->txBuf) + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1));
    for(i = 0; i < AG7100_TX_DESC_CNT; i++) {
        mac->fifo_tx[i]->next_desc =   (i == AG7100_TX_DESC_CNT - 1) ?
                                        virt_to_bus(mac->fifo_tx[0]) :
                                        virt_to_bus(mac->fifo_tx[i+1]);
        ag7100_tx_own(mac->fifo_tx[i]);
        ag7100_priv->txDataBuf[i].data = p + i * AG7100_TX_BUF_SIZE;
    }

    if (ag7100_alloc_fifo(AG7100_RX_DESC_CNT, mac->fifo_rx))
        return 1;

    for(i = 0; i < AG7100_RX_DESC_CNT; i++) {
        fr                  =    mac->fifo_rx[i];
        fr->next_desc =   (i == AG7100_RX_DESC_CNT - 1) ?
                                        virt_to_bus(mac->fifo_rx[0]) :
                                        virt_to_bus(mac->fifo_rx[i+1]);
        fr->sw_buf = ag7100_rxbuf_alloc(ag7100_priv, &rxBufData);
        if(fr->sw_buf == NULL) {
            AG7100_PRINT(AG7100_DEBUG_ERROR, ("Rx buffer alloc failed\n"));
            return 1;
        }
        fr->pkt_start_addr  =    virt_to_bus(rxBufData);
        A_DATA_CACHE_FLUSH(rxBufData, AG7100_RX_BUF_SIZE);
        ag7100_rx_give_to_dma(fr);
    }
    mac->next_rx = 0;
    mac->next_tx = 0;

    return 0;
}


/*
 * Standard eCos initialization routine.  Call the upper-level ethernet
 * driver and let it know our MAC address.
 */
static bool
ag7100_init(struct cyg_netdevtab_entry *tab)
{
    unsigned char enaddr[ETHER_ADDR_LEN];
    struct eth_drv_sc *sc;
    ag7100_priv_data_t *ag7100_priv;
    ag7100_mac_t *mac;
    int unit;

    ARRIVE();

    sc = (struct eth_drv_sc *)tab->device_instance;
    ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    unit = ag7100_priv->enetUnit;

    mac = &ag7100_priv->MACInfo;
    ag7100_macs[unit]=mac;

    /* Get ethernet's MAC address from board configuration data */
    ag7100_get_macaddr(mac, enaddr);

    ag7100_priv->sc = sc;
    ag7100_priv->rxRecvdListHead = NULL;
    ag7100_priv->rxRecvdListTail = NULL;

    /* Initialize MACInfo */
    mac->mac_unit = unit;
    mac->mac_osdep = (void *)ag7100_priv;

    mac->mac_base = ag7100_mac_base(unit);

#if 0
    AG7100_PRINT(AG7100_DEBUG_RESET,
              ("ag7100_init eth%d macBase=0x%x\n",
               mac->mac_unit,
               mac->mac_base));
#endif

    (sc->funs->eth_drv->init)(sc, enaddr);
    LEAVE();

    return true;
}

/*
 * program the usb pll (misnomer) to genrate appropriate clock
 * Write 2 into control field
 * Write pll value
 * Write 3 into control field
 * Write 0 into control field
 */
#define ag7100_pll_shift(_mac)      (((_mac)->mac_unit) ? 19: 17)
#define ag7100_pll_offset(_mac)     \
    (((_mac)->mac_unit) ? AR7240_USB_PLL_GE1_OFFSET : \
                          AR7240_USB_PLL_GE0_OFFSET)
static void
ag7100_set_pll(ag7100_mac_t *mac, unsigned int pll)
{
  uint32_t shift, reg, val;

  shift = ag7100_pll_shift(mac);
  reg   = ag7100_pll_offset(mac);

  val  = ar7240_reg_rd(AR7240_USB_PLL_CONFIG);
  val &= ~(3 << shift);
  val |=  (2 << shift);
  ar7240_reg_wr(AR7240_USB_PLL_CONFIG, val);
  A_UDELAY(100);

  ar7240_reg_wr(reg, pll);

  val |=  (3 << shift);
  ar7240_reg_wr(AR7240_USB_PLL_CONFIG, val);
  A_UDELAY(100);

  val &= ~(3 << shift);
  ar7240_reg_wr(AR7240_USB_PLL_CONFIG, val);
  A_UDELAY(100);
}


void ag7100_reset(ag7100_mac_t *mac) 
{
    uint32_t mask;

    //ag7100_reg_rmw_set(mac, AG7100_MAC_CFG1, AG7100_MAC_CFG1_SOFT_RST);
    mask = ag7100_reset_mask(mac->mac_unit);
        
    /*
     * put into reset, hold, pull out.
     */
         
    ar7240_reg_rmw_set(AR7240_RESET, mask);
    A_MDELAY(1000);
    ar7240_reg_rmw_clear(AR7240_RESET, mask);
    A_MDELAY(1000);
    A_UDELAY(20);
}

void
ag7100_hw_setup(ag7100_mac_t *mac)
{
    uint32_t mii_ctrl, mii_ctrl_val = 0;

    /* Select interface based on redboot build options */
    if (!mac->mac_unit) {
        mii_ctrl = AR7240_MII0_CTRL;

#if (CYGNUM_GE0_IFTYPE == AR7240_IF_GMII)
        mii_ctrl_val = 0x10;
#elif (CYGNUM_GE0_IFTYPE == AR7240_IF_MII)
        mii_ctrl_val = 0x11;
#elif (CYGNUM_GE0_IFTYPE == AR7240_IF_RGMII)
        mii_ctrl_val = 0x12;
#elif (CYGNUM_GE0_IFTYPE == AR7240_IF_RMII)
        mii_ctrl_val = 0x13;
#else
#error "Invalid interface type"
#endif

    } else {

#if (CYGNUM_GE1_IFTYPE == AR7240_IF_RGMII)
        mii_ctrl_val = 0x10;
#elif (CYGNUM_GE1_IFTYPE == AR7240_IF_RMII)
        mii_ctrl_val = 0x11;
#else
#error "Invalid interface type"
#endif

        mii_ctrl = AR7240_MII1_CTRL;
    }

    /* From Linux driver */
    ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
                  AG7100_MAC_CFG1_TX_EN));
    ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, (AG7100_MAC_CFG2_PAD_CRC_EN |
                       AG7100_MAC_CFG2_LEN_CHECK));

    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_0, 0x1f00);

    ar7240_reg_wr(mii_ctrl, mii_ctrl_val); 
    ag7100_reg_wr(mac, AG7100_MAC_MII_MGMT_CFG, AG7100_MGMT_CFG_CLK_DIV_20); 

    ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_4, 0x3ffff);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);
}

void
ag7100_hw_stop(ag7100_mac_t *mac)
{
    ag7100_rx_stop(mac);
    ag7100_tx_stop(mac);
    ag7100_int_disable(mac);
    /*
     * put everything into reset.
     */
    ag7100_reg_rmw_set(mac, AG7100_MAC_CFG1, AG7100_MAC_CFG1_SOFT_RST);
}

static void
ag7100_start(struct eth_drv_sc *sc,
             unsigned char *enet_addr,
             int flags)
{
    ag7100_priv_data_t *ag7100_priv;
    ag7100_mac_t *mac;
    uint32_t mac_l, mac_h;

    ARRIVE();

    ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    mac = (ag7100_mac_t *)&ag7100_priv->MACInfo;

    memcpy(ag7100_priv->enet_addr, enet_addr, ETHER_ADDR_LEN);

    /* Attach interrupt handler */
    /* TBDXXX */

    if(ag7100_setup_fifos(mac)) {
        AG7100_PRINT(AG7100_DEBUG_ERROR,("Fifo setup failed\n"));
        goto skip_start;
    }

    /* Initialize the device */
    ag7100_reset(mac);
    ag7100_hw_setup(mac);
    ag7100_phy_setup(mac->mac_unit);

    /*
     * XXX get the mac addr from flash?
     */
    mac_l = (enet_addr[4] << 8)  | (enet_addr[5]);
    mac_h = (enet_addr[0] << 24) | (enet_addr[1] << 16) |
            (enet_addr[2] << 8)  | (enet_addr[3] << 0);


    ag7100_reg_wr(mac, AG7100_GE_MAC_ADDR1, mac_l);
    ag7100_reg_wr(mac, AG7100_GE_MAC_ADDR2, mac_h);

    ag7100_priv->linkChkCntr = 1; /* Force link check on first poll */
    ag7100_check_link(mac);
    ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_bus(mac->fifo_rx[0]));
    ag7100_rx_start(mac);

skip_start:
    LEAVE();
}

static void
ag7100_stop(struct eth_drv_sc *sc)
{
    ag7100_priv_data_t *ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    ag7100_mac_t *MACInfo = (ag7100_mac_t *)&ag7100_priv->MACInfo;

    ARRIVE();
    ag7100_hw_stop(MACInfo);
    LEAVE();
}

static int
ag7100_control(struct eth_drv_sc *sc,
               unsigned long key,
               void *data,
               int data_length)
{
    ARRIVE();
    /* TBDXXX */
    LEAVE();
    return 0;
}

static int
ag7100_can_send(struct eth_drv_sc *sc)
{
    ag7100_priv_data_t *ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    ag7100_mac_t *mac = &ag7100_priv->MACInfo;

    if(!mac->link)
        if(ag7100_check_link(mac))
            AG7100_PRINT(AG7100_DEBUG_TX, ("Link is down\n"));
    
    return mac->link;
}

static void
ag7100_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list,
            int sg_len,
            int total_len,
            unsigned long key)
{
    ag7100_priv_data_t *ag7100_priv;
    ag7100_mac_t *mac;
    ag7100TxBuf_t *TxBuf;
    uint8_t *TxBufDataStart, *TxBufData;
    struct eth_drv_sg *sg_item;
    int i, length;
    ag7100_desc_t *ds;

    ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    mac = (ag7100_mac_t *)&ag7100_priv->MACInfo;

    TxBuf = &ag7100_priv->txDataBuf[mac->next_tx];
    if(TxBuf == NULL) {
        AG7100_PRINT(AG7100_DEBUG_ERROR, ("Out of Tx buffers\n"));
        return;
    }
    TxBufDataStart = TxBufData = TxBuf->data;

    /* Copy and coalesce from sg_list to buf/length */
    length = 0;
    
#if defined(CONFIG_ATHRS26_PHY) && defined(HEADER_EN)
    /* add header to normal frames */
    if (ag7100_priv->enetUnit == 0) {
        TxBuf->data[0] = 0x10; /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */ 
        TxBuf->data[1] = 0x80; /* reserved = 0b10; priority = 0; type = 0 (normal) */
        length = 2;
        TxBufData += 2;
        total_len += 2;
    }
#endif
    
    sg_item = sg_list;
    for (i=0; i<sg_len; sg_item++, i++) {
        char *segment_addr;
        int segment_len;

        segment_addr = (char *)sg_item->buf;
        segment_len = sg_item->len;

        length += segment_len;
        if (length > total_len) {
            DEBUG_PRINTF("ag7100_send: scatter/gather error.  copied=%d total=%d\n",
                         length, total_len);
            break;
        }
        memcpy(TxBufData, segment_addr, segment_len);
        TxBufData += segment_len;
    }

    ds = mac->fifo_tx[mac->next_tx];
    ds->pkt_size       = length;
    ds->pkt_start_addr = virt_to_bus(TxBufDataStart);

    AG7100_PRINT(AG7100_DEBUG_TX, ("SEND: txDesc=%p   TxBuf=%p   TxBufData=%p\n",
                 ds, TxBuf, TxBufData));

    ag7100_tx_give_to_dma(ds);
    A_DATA_CACHE_FLUSH((uint32_t)packet, length);
    ag7100_reg_wr(mac, AG7100_DMA_TX_DESC, virt_to_bus(ds));
    ag7100_reg_wr(mac, AG7100_DMA_TX_CTRL, AG7100_TXE);
    A_WR_FLUSH();

    for(i = 0; i < MAX_WAIT; i++) {
        A_UDELAY(10);
        if (!ag7100_tx_owned_by_dma(ds))
            break;
    }
    if (i == MAX_WAIT)
        AG7100_PRINT(AG7100_DEBUG_TX, ("Tx Timed out\n"));

    ds->pkt_start_addr = 0;
    ds->pkt_size       = 0;

    if(++mac->next_tx >= AG7100_TX_DESC_CNT) mac->next_tx = 0;

    /* Let upper layers know that we're done with this transmit */
    (sc->funs->eth_drv->tx_done)(sc, key, 0);

    return;
}

#if defined(DEBUG)
struct eth_drv_sg *saved_sg_list;
int saved_sg_len;
#endif

static void
ag7100_recv(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list,
            int sg_len)
{
    ag7100_priv_data_t *ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    ag7100RxBuf_t *rxBuf;
    unsigned long oldIntrState;
    unsigned char *RxBufData;
    struct eth_drv_sg *sg_item;
    int i;

    ARRIVE();

#if defined(DEBUG)
    saved_sg_list = sg_list;
    saved_sg_len = sg_len;
#endif

    intDisable(oldIntrState);
    rxBuf = ag7100_priv->rxRecvdListHead;
    ag7100_priv->rxRecvdListHead = ag7100_priv->rxRecvdListHead->next;
    intEnable(oldIntrState);
    
    if ((char *)sg_list->buf != NULL) {
        /* Copy out from driver Rx buffer to sg_list */
        RxBufData = rxBuf->data;
        sg_item = sg_list;
        for (i=0; i<sg_len; sg_item++, i++) {
            char *segment_addr;
            int segment_len;
    
            segment_addr = (char *)sg_item->buf;
            segment_len = sg_item->len;
    
#if defined(CONFIG_ATHRS26_PHY) && defined(HEADER_EN)
            /* remove header */
            if ((ag7100_priv->enetUnit == 0) && (i == 0)) {
                RxBufData += 2;
                segment_len -= 2;
            }
#endif    
            memcpy(segment_addr, RxBufData, segment_len);
            RxBufData += segment_len;
        }
    } else {
        /* Handle according to convention: bit bucket the packet */
    }

    /* Free driver Rx buffer */
    ag7100_rxbuf_free(ag7100_priv, rxBuf);
    LEAVE();
}

static void
ag7100_deliver(struct eth_drv_sc *sc)
{
    ARRIVE();
    /* TBDXXX */
    LEAVE();
}

static void
ag7100_poll(struct eth_drv_sc *sc)
{
    int length;
    ag7100RxBuf_t *rxBuf, *newRxBuf;
    ag7100_desc_t *f;
    ag7100_mac_t *mac;
    ag7100_priv_data_t *ag7100_priv;
    uint8_t *rxBufData;

    ag7100_priv = (ag7100_priv_data_t *)sc->driver_private;
    mac = (ag7100_mac_t *)&ag7100_priv->MACInfo;

    if(--ag7100_priv->linkChkCntr == 0) {
        ag7100_priv->linkChkCntr = AG7100_LINK_CHK_INTVL;
        if(ag7100_check_link(mac) == 0) {
            AG7100_PRINT(AG7100_DEBUG_TX, ("Link is down\n"));
            return;
        }
    }
    
    for(;;){
        f = mac->fifo_rx[mac->next_rx];
        A_DATA_CACHE_INVAL(f, sizeof(*f));
        if(ag7100_rx_owned_by_dma(f))
            goto rx_done;

        length = f->pkt_size;

        rxBuf = f->sw_buf;
        /*
         * Allocate a replacement buffer
         * We want to get another buffer ready for Rx ASAP.
         */
        newRxBuf = (ag7100RxBuf_t *)ag7100_rxbuf_alloc(ag7100_priv, &rxBufData);
        if(newRxBuf == NULL ) {
            /*
             * Give this descriptor back to the DMA engine,
             * and drop the received packet.
             */
            AG7100_PRINT(AG7100_DEBUG_ERROR,
                              ("Can't allocate new rx buf\n"));
        } else {
            //diag_printf("rxBufData=%p\n",rxBufData);
            f->pkt_start_addr = virt_to_bus(rxBufData);
            f->sw_buf = newRxBuf;
        }

        ag7100_rx_give_to_dma(f);
        A_WR_FLUSH();

        if(newRxBuf == NULL) {
            goto no_rx_bufs;
        } else {
            unsigned long oldIntrState;

            /* Sync data cache w.r.t. DMA */
            A_DATA_CACHE_INVAL(rxBuf->data, length);

            /* Send the data up the stack */
            AG7100_PRINT(AG7100_DEBUG_RX,
                ("Send data up stack: rxBuf=%p data=%p length=%d\n",
                (void *)rxBuf, (void *)rxBuf->data, length));

            rxBuf->next = NULL;
            intDisable(oldIntrState);
            if (ag7100_priv->rxRecvdListHead) {
                ag7100_priv->rxRecvdListTail->next = rxBuf;
                ag7100_priv->rxRecvdListTail = rxBuf;
            } else {
                ag7100_priv->rxRecvdListHead = rxBuf;
                ag7100_priv->rxRecvdListTail = rxBuf;
            }
            intEnable(oldIntrState);

            (sc->funs->eth_drv->recv)(sc, length);
        } 

        if(++mac->next_rx >= AG7100_RX_DESC_CNT) mac->next_rx = 0;
    }

rx_done:
    if (!(ag7100_reg_rd(mac, AG7100_DMA_RX_CTRL))) {
        ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_bus(f));
        ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, AG7100_RXE);
    }

no_rx_bufs:
    return;
}

static int
ag7100_int_vector(struct eth_drv_sc *sc)
{
    /* TBDXXX */
    return 0;
}

static void
ag7100_rxbuf_free(ag7100_priv_data_t *ag7100_priv, void *buf)
{
    unsigned long oldIntrState;
    ag7100RxBuf_t *rxBuf = (ag7100RxBuf_t *)buf;

    intDisable(oldIntrState);
    if (rxBuf) {
        rxBuf->next = ag7100_priv->rxFreeList;
        ag7100_priv->rxFreeList = rxBuf;
    }
    intEnable(oldIntrState);
}

void
ag7100_get_macaddr(ag7100_mac_t *mac, uint8_t *mac_addr)
{
    flash_get_mac_address(mac->mac_unit,mac_addr);
}
    
static void
ag7100_set_mac_from_link(ag7100_mac_t *mac, int speed, int fdx)
{
#ifdef CONFIG_ATHRS26_PHY
    int change_flag = 0;
    
    if(mac->mac_speed !=  speed)
        change_flag = 1;

    if(change_flag) {
        athrs26_phy_off(mac);
        athrs26_mac_speed_set(mac, speed);        
    }
#endif

    mac->mac_speed    =  speed;
    mac->mac_duplex   =  fdx;

    ag7100_set_mii_ctrl_speed(mac, speed);
    ag7100_set_mac_duplex(mac, fdx);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x7801ff /* Was 0x4001ff */);

    switch (speed) {

    case AG7100_PHY_SPEED_1000T:
      ag7100_set_mac_if(mac, 1);
      ag7100_set_pll(mac, 0x0110000);
      ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
      break;

    case AG7100_PHY_SPEED_100TX:
      ag7100_set_mac_if(mac, 0);
      ag7100_set_mac_speed(mac, 1);
      ag7100_set_pll(mac, 0x0001099);  /* was commented out */
      ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
      break;

    case AG7100_PHY_SPEED_10T:
      ag7100_set_mac_if(mac, 0);
      ag7100_set_mac_speed(mac, 0);
      ag7100_set_pll(mac, 0x00991099);
      ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
      break;
    }
        
#ifdef CONFIG_ATHRS26_PHY
    if(change_flag) 
        athrs26_phy_on(mac);
#endif
    mac->link   = 1;
}

static int
ag7100_check_link(ag7100_mac_t *mac)
{
    uint32_t link, duplex, speed;
    
    /* The vitesse switch uses an indirect method to communicate phy status
     * so it is best to limit the number of calls to what is necessary.
     * However a single call returns all three pieces of status information.
     * 
     * This is a trivial change to the other PHYs ergo this change.
     *
     */
    
    ag7100_get_link_status(mac->mac_unit, &link, &duplex, &speed);
    
    if (!link) {
      mac->link = 0;
      AG7100_PRINT(AG7100_DEBUG_VERBOSE,("link down\n"));
      return 0;
    }
    
    if (mac->link && (duplex == mac->mac_duplex) && (speed == mac->mac_speed))
      return 1;
    
    ag7100_set_mac_from_link(mac, speed, duplex);
    return 1;
}

uint16_t
ag7100_mii_read(int unit, uint32_t phy_addr, uint8_t reg)
{
    ag7100_mac_t *mac   = ag7100_unit2mac(0);
    uint16_t      addr  = (phy_addr << AG7100_ADDR_SHIFT) | reg, val;
    volatile int  rddata;
    int           ii=1024;

    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr); 
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, AG7100_MGMT_CMD_READ);

    do {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);

    val = ag7100_reg_rd(mac, AG7100_MII_MGMT_STATUS);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);

    return val;
}

void
ag7100_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data)
{
    ag7100_mac_t *mac   = ag7100_unit2mac(0);
    uint16_t      addr  = (phy_addr << AG7100_ADDR_SHIFT) | reg;
    volatile int  rddata;
    int           ii=1024;

    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr); 
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CTRL, data);

    do {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);
}

/* Ethernet Unit 0 */
ag7100_priv_data_t ag7100_priv_data0 = {enetUnit: 0};

ETH_DRV_SC(ag7100_sc0,
           &ag7100_priv_data0,
           "eth0",
           ag7100_start,
           ag7100_stop,
           ag7100_control,
           ag7100_can_send,
           ag7100_send,
           ag7100_recv,
           ag7100_deliver,
           ag7100_poll,
           ag7100_int_vector);

NETDEVTAB_ENTRY(ag7100_netdev0, 
                "ag7100_eth0", 
                ag7100_init, 
                &ag7100_sc0);

#if 1
/* Ethernet Unit 1 */
ag7100_priv_data_t ag7100_priv_data1 = {enetUnit: 1};

ETH_DRV_SC(ag7100_sc1,
           &ag7100_priv_data1,
           "eth1",
           ag7100_start,
           ag7100_stop,
           ag7100_control,
           ag7100_can_send,
           ag7100_send,
           ag7100_recv,
           ag7100_deliver,
           ag7100_poll,
           ag7100_int_vector);

NETDEVTAB_ENTRY(ag7100_netdev1, 
                "ag7100_eth1", 
                ag7100_init, 
                &ag7100_sc1);
#endif

