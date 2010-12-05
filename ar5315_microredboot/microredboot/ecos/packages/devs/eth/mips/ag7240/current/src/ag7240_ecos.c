//==========================================================================
//
//      dev/ag7240ecos.c
//
//      Ethernet driver for AG7240
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>
#include <pkgconf/devs_eth_mips_mips32_ag7240.h>

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


#include "ag7240.h"
#include "ag7240_phy.h"

#ifndef CYGPKG_REDBOOT
//#define DEBUG
#endif

#if 1
static int ag7240_debug = 0;
#define CONSOLE_PRINTF(fmt, arg...)
#else
#define CONSOLE_PRINTF(fmt, arg...) \
                          do {                                                                    \
                               if (ag7240_debug) {                                                \
                                   diag_printf("%s[%4d]: "fmt"\n", __func__, __LINE__, ##arg);    \
                               }                                                                  \
                          } while(0)
#endif


#define CONSOLE_DB_PRINTF(fmt, arg...) do {                                                         \
                                diag_printf("<%llu> %s[%04d]: "fmt"\n", cyg_current_time(), __func__, __LINE__, ##arg);  \
                          } while(0)



#define ARRIVE() CONSOLE_PRINTF("begin");
//#define ARRIVE()
#define LEAVE() CONSOLE_PRINTF("end");
//#define LEAVE()

#define AG7240_LINK_CHK_INTVL       0xffff
#define AG7240_PHY_POLL_SECONDS     2

void ag7240_show_info(void);
void ag7240_get_macaddr(ag7240_mac_t *mac, cyg_uint8 *mac_addr);
static cyg_uint32 ag7240_isr(cyg_vector_t vector, cyg_addrword_t data);
static void ag7240_hw_setup(ag7240_mac_t *mac);
static int ag7240_tx_reap(ag7240_mac_t *mac, struct eth_drv_sc *sc , int ac);

ag7240_mac_t *ag7240_macs[2];
static cyg_handle_t  link_interrupt_handle;
static cyg_interrupt link_interrupt_object;
ATH_LED_CONTROL    PLedCtrl;
int phy_in_reset = 0;
int fifo_3 = 0x1f00140;
int mii0_if = AG7240_MII0_INTERFACE;
int mii1_if = AG7240_MII1_INTERFACE;
int gige_pll = 0x1a000000;
int fifo_5 = 0xbefff;
int xmii_val = 0x16000000;
int rg_phy_speed = -1 , rg_phy_duplex = -1;
char *spd_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
char *dup_str[] = {"half duplex", "full duplex"};


#define TICKS_PER_SEC ((1000000000 / CYGNUM_HAL_RTC_NUMERATOR) * CYGNUM_HAL_RTC_DENOMINATOR)

static void
mac_init_timer(mac_timer_t* timer, void* arg, cyg_alarm_t* fn)
{
    cyg_handle_t clock_hdl = cyg_real_time_clock();

    cyg_clock_to_counter(clock_hdl, &(timer->counter_hdl));
    cyg_alarm_create(timer->counter_hdl, fn, (cyg_addrword_t)arg,
                     &(timer->alarm_hdl), &(timer->alarm_obj));
}

static void
mac_set_timer(mac_timer_t* timer, u_int32_t secs)
{
    if(timer->alarm_hdl != NULL) {
        cyg_alarm_initialize(timer->alarm_hdl,
                             (cyg_current_time() + (cyg_tick_count_t)(TICKS_PER_SEC * secs)), 0);
    }
}

static void
mac_cancel_timer(mac_timer_t* timer)
{
    if(timer->alarm_hdl != NULL) {
        cyg_alarm_disable(timer->alarm_hdl);
    }
    //cyg_alarm_delete(timer->alarm_hdl);
    //cyg_counter_delete(timer->counter_hdl);
}

static void
mac_free_timer(mac_timer_t* timer)
{
    if(timer->alarm_hdl != NULL) {
        cyg_alarm_delete(timer->alarm_hdl);
        timer->alarm_hdl = NULL;
    }

}

#define MIN_HEAD_ROOM  64

static void*
ag7240_alloc_pkt(void)
{
    struct mbuf *m;

    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL) return NULL;

    MCLGET(m, M_DONTWAIT);
    if ((m->m_flags & M_EXT) == 0) {
        m_free(m);
        return NULL;
    }
    m->m_len = 0;
    //edward
    M_ALIGN(m, MIN_HEAD_ROOM);
    m->m_nextpkt = NULL;
    m->m_next = NULL;

    return m;
}

static inline void
ag7240_free_pkt(void* m)
{
    m_freem(m);
}


static cyg_uint8*
ag7240_get_pkt_buf(void* m)
{
    return mtod((struct mbuf *)m, cyg_uint8*);
}


static void
ag7240_rings_clean(ag7240_mac_t *mac)
{
    int i;
    ag7240_desc_t* rxdesc = mac->mac_rxdesc;
    ag7240_desc_t* rxdesc_tmp;
    struct mbuf *m;

    CONSOLE_PRINTF("begin");
    ag7240_reg_wr(mac, AG7240_DMA_RX_DESC, (cyg_uint32)NULL);
    ag7240_reg_wr(mac, AG7240_DMA_TX_DESC, (cyg_uint32)NULL);

    for (i = 0, rxdesc_tmp = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(rxdesc);
         i < AG7240_RX_DESC_CNT;
         i++, rxdesc_tmp++) {

        if (rxdesc_tmp->packet) {
            m = rxdesc_tmp->packet;
            ag7240_free_pkt(m);
            rxdesc_tmp->packet = NULL;
        }
    }
#if 1
{
    int ac;
    ag7240_desc_t* txdesc;
    ag7240_desc_t* txdesc_tmp;
    //remove tx ring
    /* free Tx descriptors */
    for (ac = 0; ac < mac->mac_noacs; ac++) {
        txdesc = &mac->mac_txdesc[ac];
        for (i = 0, txdesc_tmp = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(txdesc);
             i < (AG7240_TX_DESC_CNT);
             i++, txdesc_tmp++) {

            if (txdesc_tmp->packet) {
                m = txdesc_tmp->packet;
                ag7240_free_pkt(m);
                txdesc_tmp->packet = NULL;
            }
        }
    }
}
#endif
    CONSOLE_PRINTF("end");
}

static int
ag7240_rings_init(ag7240_mac_t *mac)
{
    int i, ac;
    struct mbuf *m;
    ag7240_ring_t* r;
    ag7240_desc_t* ds;


    ARRIVE();
    bzero(mac->mac_txdesc, sizeof(mac->mac_txdesc));
    bzero(&mac->mac_rxdesc, sizeof(mac->mac_rxdesc));
    bzero(mac->mac_txring, sizeof(mac->mac_txring));
    bzero(&mac->mac_rxring, sizeof(mac->mac_rxring));

    /* Init Tx descriptors */
    for (ac = 0; ac < mac->mac_noacs; ac++) {
        ds = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(mac->mac_txdesc[ac]);
        for (i = 0; i < AG7240_TX_DESC_CNT-1; i++, ds++) {
            ds->next_desc = CYGARC_PHYSICAL_ADDRESS(ds + 1);
            ds->is_empty = 1;
            ds->packet = NULL;
        }
        ds->next_desc = CYGARC_PHYSICAL_ADDRESS(mac->mac_txdesc[ac]);
        ds->is_empty = 1;

        r = &mac->mac_txring[ac];
        r->ring_desc = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(mac->mac_txdesc[ac]);
        r->ring_desc_dma = CYGARC_PHYSICAL_ADDRESS(r->ring_desc);
        r->ring_nelem = AG7240_TX_DESC_CNT;
        r->ring_head = 0;
        r->ring_tail = 0;
        r->ring_full = 0;
    }


    /* Init Rx descriptors and allocate packet buf */
    ds = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(mac->mac_rxdesc);
    for (i = 0; i < AG7240_RX_DESC_CNT-1; i++, ds++) {
        ds->next_desc = CYGARC_PHYSICAL_ADDRESS(ds + 1);
        ds->is_empty = 1;
        if ((m = ds->packet = ag7240_alloc_pkt()) == NULL) {
            diag_printf("%s:%d:no buffer i:%d\n", __func__, __LINE__, i);
            goto fail;
        }
        ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(ag7240_get_pkt_buf(m));
    }
    ds->next_desc = CYGARC_PHYSICAL_ADDRESS(mac->mac_rxdesc);
    ds->is_empty = 1;
    if ((m = ds->packet = ag7240_alloc_pkt()) == NULL) {
        diag_printf("%s:%d:no buffer i:%d\n", __func__, __LINE__, i);
        goto fail;
    }
    ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(ag7240_get_pkt_buf(m));

    r = &mac->mac_rxring;
    r->ring_desc = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(mac->mac_rxdesc);
    r->ring_desc_dma = CYGARC_PHYSICAL_ADDRESS(r->ring_desc);
    r->ring_nelem = AG7240_RX_DESC_CNT;
    r->ring_head = 0;
    r->ring_tail = 0;
    r->ring_full = 0;


    LEAVE();
    return 0;

fail:
    ds = (ag7240_desc_t*)CYGARC_UNCACHED_ADDRESS(mac->mac_rxdesc);
    for (i = 0; i < AG7240_RX_DESC_CNT; i++, ds++) {
        ag7240_free_pkt(ds->packet);
    }

    LEAVE();
    return -1;
}

cyg_uint16
ag7240_mii_read(int unit, cyg_uint32 phy_addr, cyg_uint8 reg)
{
    ag7240_mac_t *mac = ag7240_unit2mac(unit);
    cyg_uint16 addr = (phy_addr << AG7240_ADDR_SHIFT) | reg, val;
    volatile int rddata;
    cyg_uint16 ii = 0x1000;

    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, 0x0);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_ADDRESS, addr);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, AG7240_MGMT_CMD_READ);

    do {
        HAL_DELAY_US(5);
        rddata = ag7240_reg_rd(mac, AG7240_MII_MGMT_IND) & 0x1;
    } while (rddata && --ii);

    val = ag7240_reg_rd(mac, AG7240_MII_MGMT_STATUS);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CMD, 0x0);

    return val;
}

void
ag7240_mii_write(int unit, cyg_uint32 phy_addr, cyg_uint8 reg, cyg_uint16 data)
{
    ag7240_mac_t *mac = ag7240_unit2mac(unit);
    cyg_uint16 addr  = (phy_addr << AG7240_ADDR_SHIFT) | reg;
    volatile int rddata;
    cyg_uint16 ii = 0x1000;

    ag7240_reg_wr(mac, AG7240_MII_MGMT_ADDRESS, addr);
    ag7240_reg_wr(mac, AG7240_MII_MGMT_CTRL, data);

    do {
        rddata = ag7240_reg_rd(mac, AG7240_MII_MGMT_IND) & 0x1;
    } while (rddata && --ii);
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

static void
ag7240_hw_setup(ag7240_mac_t *mac)
{
    ag7240_ring_t *tx, *rx = &mac->mac_rxring;
    ag7240_desc_t *r0, *t0;
    cyg_uint32 mgmt_cfg_val,ac;
    cyg_uint32 check_cnt;

    if(mac->mac_unit) {
        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN |
            AG7240_MAC_CFG1_TX_EN | AG7240_MAC_CFG1_RX_FCTL | AG7240_MAC_CFG1_TX_FCTL));
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_PAD_CRC_EN |
            AG7240_MAC_CFG2_LEN_CHECK | AG7240_MAC_CFG2_IF_1000));
    } else {
        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (AG7240_MAC_CFG1_RX_EN |
            AG7240_MAC_CFG1_TX_EN));
        ag7240_reg_rmw_set(mac, AG7240_MAC_CFG2, (AG7240_MAC_CFG2_PAD_CRC_EN |
            AG7240_MAC_CFG2_LEN_CHECK));
    }
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_0, 0x1f00);

    /* Disable AG7240_MAC_CFG2_LEN_CHECK to fix the bug that
     * the mac address is mistaken as length when enabling Atheros header
     */
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
            default:
                mgmt_cfg_val = 0x7;
        }
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
            diag_printf("Virian MDC CFG Value ==> %x\n",mgmt_cfg_val);
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
                    diag_printf("%s: MDC check failed\n", __func__);
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
    if(mac_has_flag(mac,ATHR_S26_HEADER)){
        ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0xe6bc0);
    }else{
        ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0x66b82);
    }
    if (mac->mac_unit == 0 && is_ar7242()){
       ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_5, 0xe6be2);
    }
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

    DPRINTF(MODULE_NAME ": cfg1 0x%x cfg2 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_CFG1),
        ag7240_reg_rd(mac, AG7240_MAC_CFG2));
}

static void
ag7240_set_mac_from_link(ag7240_mac_t *mac, ag7240_phy_speed_t speed, int fdx)
{
    if(mac->mac_unit == 1)
        speed = AG7240_PHY_SPEED_1000T;

    mac->mac_speed =  speed;
    mac->mac_fdx   =  fdx;

    ag7240_set_mac_duplex(mac, fdx);
    ag7240_reg_wr(mac, AG7240_MAC_FIFO_CFG_3, fifo_3);

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
        diag_printf("%s:%d: assert\n", __func__, __LINE__);
    }
    diag_printf("cfg_1: 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_1));
    diag_printf("cfg_2: 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_2));
    diag_printf("cfg_3: 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_3));
    diag_printf("cfg_4: 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_4));
    diag_printf("cfg_5: 0x%x\n", ag7240_reg_rd(mac, AG7240_MAC_FIFO_CFG_5));
}

static int mac_link_up = 0;
int
ag7240_check_link(ag7240_mac_t *mac, int phyUnit)
{
    u_int32_t carrier = mac->link_up;
    int fdx = 0, phy_up = 0, rc;
    ag7240_phy_speed_t  speed = AG7240_PHY_SPEED_10T;

    /* The vitesse switch uses an indirect method to communicate phy status
     * so it is best                 to limit the number of calls to what is necessary.
     * However a single call returns all three pieces of status information.
     *
     * This is a trivial change to the other PHYs ergo this change.
     *
     */

    rc = ag7240_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed, phyUnit);

    athrs26_phy_stab_wr(phyUnit,phy_up,speed);

    if (rc < 0)
        goto done;

    if (!phy_up) {
        if (carrier) {
            diag_printf("unit %d: phy %0d not up carrier %d\n", mac->mac_unit, phyUnit, carrier);

            /* A race condition is hit when the queue is switched on while tx interrupts are enabled.
             * To avoid that disable tx interrupts when phy is down.
             */
            ag7240_intr_disable_tx(mac);

            mac->link_up = 0;
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
        diag_printf("phy not connected?\n");
        return 0;
    }

    if (carrier && (speed == mac->mac_speed) && (fdx == mac->mac_fdx))
        goto done;

    if (athrs26_phy_is_link_alive(phyUnit))
    {
        diag_printf("enet unit:%d phy:%d is up...", mac->mac_unit,phyUnit);
        diag_printf("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)], spd_str[speed], dup_str[fdx]);

        ag7240_set_mac_from_link(mac, speed, fdx);

        diag_printf("done cfg2 0x%x ifctl 0x%x miictrl  \n",
                    ag7240_reg_rd(mac, AG7240_MAC_CFG2),
                    ag7240_reg_rd(mac, AG7240_MAC_IFCTL));
        /*
         * in business
         */
        mac->link_up = 1;
    }
    else {
        diag_printf("enet unit:%d phy:%d is down...\n", mac->mac_unit,phyUnit);
    }

done:
    return 0;
}
int ag7240_get_link_up(){
    printf("ag7240_get_link_up=%d\n\r",mac_link_up);
    return mac_link_up;
}
#if defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_RGMII_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_S16_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_VIR_PHY)
void
ag7242_check_link(cyg_handle_t alarm, cyg_addrword_t data)
{
    ag7240_mac_t *mac = (ag7240_mac_t*)data;
    u_int32_t carrier = mac->link_up;
    int fdx = 0, phy_up = 0, rc, phyUnit = 0;
    ag7240_phy_speed_t  speed = AG7240_PHY_SPEED_10T;

    rc = ag7240_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed, phyUnit);

    if (rc < 0) {
        goto done;
    }

    if (!phy_up)
    {
        if (carrier)
        {
            diag_printf("unit %d: phy %0d not up carrier %d\n", mac->mac_unit, phyUnit, carrier);

            /* A race condition is hit when the queue is switched on while tx interrupts are enabled.
             * To avoid that disable tx interrupts when phy is down.
             */
            ag7240_intr_disable_tx(mac);

            mac->link_up = 0;
//            netif_carrier_off(dev);
//            netif_stop_queue(dev);
        }
        goto done;
    }

    if(!mac->mac_ifup) {
        goto done;
    }

    if ((fdx < 0) || (speed < 0))
    {
        diag_printf("phy not connected?\n");
        return;
//        return 0;
    }

    if (carrier && (speed == rg_phy_speed ) && (fdx == rg_phy_duplex)) {
        goto done;
    }
#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_RGMII_PHY
    if (athr_phy_is_link_alive(phyUnit))
#endif
    {
        diag_printf("enet unit:%d is up...\n", mac->mac_unit);
        diag_printf("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)], spd_str[speed], dup_str[fdx]);

        rg_phy_speed = speed;
        rg_phy_duplex = fdx;
        /* Set GEO to be always at Giga Bit */
        speed = AG7240_PHY_SPEED_1000T;
        ag7240_set_mac_from_link(mac, speed, fdx);

        diag_printf("done cfg2 0x%x ifctl 0x%x miictrl  \n",
                    ag7240_reg_rd(mac, AG7240_MAC_CFG2),
                    ag7240_reg_rd(mac, AG7240_MAC_IFCTL));
        /*
         * in business
         */
        mac->link_up = 1;
//        netif_carrier_on(dev);
//        netif_start_queue(dev);
    }

done:
//    mod_timer(&mac->mac_phy_timer, jiffies + AG7240_PHY_POLL_SECONDS*HZ);
    mac_set_timer(&mac->mac_phy_timer, AG7240_PHY_POLL_SECONDS);
    return;
}
#endif



#include <cyg/io/eth/eth_drv.h>
static int
null_can_send(struct eth_drv_sc *sc)
{
    #if 1
    return 1;
    #else
    int unused = 0;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    ag7240_ring_t* r = &mac->mac_txring[0];
    unused = ag7240_ndesc_unused(mac, r);
    return unused;
    #endif
}

static void
null_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, int total, unsigned long key)
{
    #if 1//test by randy
    return;
    #else
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    ag7240_ring_t* r = &mac->mac_txring[0];
    ag7240_desc_t* ds;
    struct mbuf *m0, *m;
    int len;
    unsigned char *data;
    int unused;

    struct mbuf *m_big;
    int total_len = 0;
    unsigned char *tmp_ptr;
    unsigned char *tmp2_ptr;

    /* TODO */
    //printf("unuse=%d,%d,%d\n\r",unused,AG7240_TX_DESC_CNT,AG7240_RX_DESC_CNT);
    unused = ag7240_ndesc_unused(mac, r);
    //printf("unuse=%d,%d,%d\n\r",unused,AG7240_TX_DESC_CNT,AG7240_RX_DESC_CNT);
    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_tx_reap(mac, sc, 0);
    }


    if(1) {
        //if (unused < AG7240_TX_QSTART_THRESH) {
        //    break;
        //}

        m_big = ag7240_alloc_pkt();
        if(m_big == 0)
            goto fail_1;
        tmp_ptr = mtod(m_big, unsigned char *);
        tmp2_ptr = mtod(m_big, unsigned char *);

m0 = (struct mbuf *)key;
        if(m0->m_next == 0) {
            ag7240_free_pkt(m_big);


            if ((m0->m_flags & M_PKTHDR) == 0) {
                panic("eth_drv_send: no header mbuf");
            }


            // Extract data pointers (don't actually move data here)
            for (m = m0; m ; m = m->m_next) {
                ds = &r->ring_desc[r->ring_head];
                data = mtod(m, unsigned char *);
                len = m->m_len;
                HAL_DCACHE_FLUSH((cyg_uint32)data, len);
                ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(data);
    //            ds->pkt_size = len;
                //((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit = AG7240_DESC_MORE_T | len;
                ds->pkt_size = len;
                ds->more = 1;
                ds->packet = (m == m0)? m0: NULL;
                HAL_REORDER_BARRIER();
                ds->more = 0;
                ag7240_tx_give_to_dma(ds);
    //            HAL_DCACHE_FLUSH(ds, sizeof(*ds));
                ag7240_ring_incr(r->ring_head);
                unused--;
            }
    //        ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit &= AG7240_DESC_CLEAR_MORE_T;
           // ds->more = 0;
            mac->mac_net_stats.tx_packets++;
        } else {

            if ((m0->m_flags & M_PKTHDR) == 0) {
                panic("eth_drv_send: no header mbuf");
            }

            total_len =0;
            // Extract data pointers (don't actually move data here)
            for (m = m0; m ; m = m->m_next) {
                if(m == m0)
                    ds = &r->ring_desc[r->ring_head];
                data = mtod(m, unsigned char *);
                len = m->m_len;
                memcpy(tmp2_ptr,data,len);
                tmp2_ptr = tmp2_ptr+len;
                total_len=total_len+len;
            }
            m_freem(m0);
            m_big->m_len = total_len;
            ds->pkt_size = total_len;
            ds->more = 1;
            HAL_DCACHE_FLUSH((cyg_uint32)tmp_ptr, total_len);
            ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(tmp_ptr);
            ds->packet = m_big;
            HAL_REORDER_BARRIER();
            ds->more = 0;
            ag7240_tx_give_to_dma(ds);
            ag7240_ring_incr(r->ring_head);
            unused--;
            mac->mac_net_stats.tx_packets++;
        }
    }

    ar7240_flush_ge(mac->mac_unit);

    ag7240_tx_start(mac);

    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_intr_enable_tx(mac);
    }
fail_1:
    return;
    #endif
}
static void
null_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
}

static void
null_poll(struct eth_drv_sc *sc)
{
}

static void
null_deliver(struct eth_drv_sc *sc)
{
}



static int
ag7240_if_ioctl(struct eth_drv_sc *sc,
               unsigned long key,
               void *data,
               int data_length)
{

     printf("%s[%04u]\n\r",__func__,__LINE__);
    ARRIVE();
    /* TBDXXX */
    LEAVE();
    return 0;
}

static void
ag7240_if_stop(struct eth_drv_sc *sc)
{
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;

    ARRIVE();
    cyg_drv_interrupt_mask(mac->mac_vector);
    mac->mac_ifup = 0;
    mac->link_up = 0;
    ag7240_hw_stop(mac);
    ag7240_rings_clean(mac);
	
    mac->mac_net_stats.rx_packets = 0;
    mac->mac_net_stats.tx_packets = 0;
    mac->mac_net_stats.rx_error_packets = 0;
    mac->mac_net_stats.tx_error_packets = 0;
    mac->mac_net_stats.rx_drop_packets = 0;
    mac->mac_net_stats.tx_drop_packets = 0;
	
#if defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_RGMII_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_S16_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_VIR_PHY)
    if(is_ar7242() && mac->mac_unit == 0) {
        mac_free_timer(&mac->mac_phy_timer);
    }
#endif
    LEAVE();
}


static void
ag7240_if_start(struct eth_drv_sc *sc,
                unsigned char *enet_addr,
                int flags)
{
    cyg_uint32 mask;
    cyg_uint32 w1 = 0, w2 = 0;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    if(mac->mac_ifup == 1) {
        ag7240_if_stop(sc);
    }
    ARRIVE();

    /* Initialize rings */
    if (ag7240_rings_init(mac) < 0) {
        LEAVE();
        return;
    }

    HAL_DELAY_US(20);
    mask = ag7240_reset_mask(mac->mac_unit);
    ar7240_reg_rmw_set(AR7240_RESET, mask);
    HAL_DELAY_US(10);
    ar7240_reg_rmw_clear(AR7240_RESET, mask);
    HAL_DELAY_US(10);

    ag7240_hw_setup(mac);

#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_VIR_PHY
 #ifndef SWITCH_AHB_FREQ
    u32 tmp_pll ;
 #endif
    tmp_pll = 0x62000000 ;
    ar7240_reg_wr_nf(AR7242_ETH_XMII_CONFIG, tmp_pll);
    HAL_DELAY_US(100*1000);
#endif

    mac->mac_speed = -1;

    ag7240_phy_reg_init(mac->mac_unit);

    diag_printf("Setting PHY...\n");

    phy_in_reset = 1;

#ifndef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_VIR_PHY
     ag7240_phy_setup(mac->mac_unit);
#else
     athr_vir_phy_setup(mac->mac_unit);
#endif
    phy_in_reset = 0;


    /*
    * set the mac addr
    */
    addr_to_words(enet_addr, w1, w2);
    ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR1, w1);
    ag7240_reg_wr(mac, AG7240_GE_MAC_ADDR2, w2);

    mac->mac_ifup = 1;
    mac->link_up = 0;
    ag7240_int_enable(mac);

    if (is_ar7240()) {
        ag7240_intr_enable_rxovf(mac);
    }

#if defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_RGMII_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_S16_PHY) || \
    defined(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7242_VIR_PHY)
    if(is_ar7242() && mac->mac_unit == 0) {
        mac_init_timer(&mac->mac_phy_timer, mac, ag7242_check_link);
        ag7242_check_link((cyg_handle_t)NULL, (cyg_addrword_t)mac);
    }
#endif


    if (is_ar7240() || is_ar7241() || (is_ar7242() && mac->mac_unit == 1)) {
       athrs26_enable_linkIntrs(mac->mac_unit);
    }

    /* Attach interrupt handler */
    cyg_drv_interrupt_unmask(mac->mac_vector);

    ag7240_rx_start(mac);

    LEAVE();
}

static int
ag7240_if_poll_xmit(struct ifnet *ifp, int* budget)
{
#if 1
    struct eth_drv_sc *sc = ifp->if_softc;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    ag7240_ring_t* r = &mac->mac_txring[0];
    ag7240_desc_t* ds;
    struct mbuf *m0, *m;
    int len;
    int qlen;
    unsigned char *data;
    int unused;

    struct mbuf *m_big;
    int total_len = 0;
    unsigned char *tmp_ptr;
    unsigned char *tmp2_ptr;

    /* TODO */
//    if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
//         return 0;
//    }
    if ((ifp->if_flags & (IFF_RUNNING|IFF_UP)) != (IFF_RUNNING|IFF_UP)) {
        while(1) {//free all out packet when ifp down
            IF_DEQUEUE(&ifp->if_snd, m0);
            if (m0 == 0) {
                break;
            }
            m_freem(m0);
            //printf("%s[%04u]\n\r",__func__,__LINE__);
        }
        return 0;
    }
    unused = ag7240_ndesc_unused(mac, r);
    //printf("unuse=%d,%d,%d\n\r",unused,AG7240_TX_DESC_CNT,AG7240_RX_DESC_CNT);
    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_tx_reap(mac, sc, 0);
    }

    qlen = ifp->if_snd.ifq_len;

    while (1) {
        if (unused < AG7240_TX_QSTART_THRESH) {
            break;
        }
        if(*budget <= 0) {
            break;
        }
        *budget = *budget - 1;

        IF_DEQUEUE(&ifp->if_snd, m0);
        if (m0 == 0) {
            break;
        }

        if(m0->m_next == 0) {
            qlen--;

            if ((m0->m_flags & M_PKTHDR) == 0) {
                panic("eth_drv_send: no header mbuf");
            }


            // Extract data pointers (don't actually move data here)
            for (m = m0; m ; m = m->m_next) {
                ds = &r->ring_desc[r->ring_head];
                data = mtod(m, unsigned char *);
                len = m->m_len;
                HAL_DCACHE_FLUSH((cyg_uint32)data, len);
                ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(data);
    //            ds->pkt_size = len;
                ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit = AG7240_DESC_MORE_T | len;
                ds->packet = (m == m0)? m0: NULL;
                HAL_REORDER_BARRIER();
                ag7240_tx_give_to_dma(ds);
    //            HAL_DCACHE_FLUSH(ds, sizeof(*ds));
                ag7240_ring_incr(r->ring_head);
                unused--;
            }
    //        ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit &= AG7240_DESC_CLEAR_MORE_T;
            ds->more = 0;
            mac->mac_net_stats.tx_packets++;
        } else {
            qlen--;
            m_big = ag7240_alloc_pkt();
            if(m_big == 0) {
				mac->mac_net_stats.tx_drop_packets++;
                break;
			}
            tmp_ptr = mtod(m_big, unsigned char *);
            tmp2_ptr = mtod(m_big, unsigned char *);

            if ((m0->m_flags & M_PKTHDR) == 0) {
                panic("eth_drv_send: no header mbuf");
            }

            total_len =0;
            // Extract data pointers (don't actually move data here)
            for (m = m0; m ; m = m->m_next) {
                if(m == m0)
                    ds = &r->ring_desc[r->ring_head];
                data = mtod(m, unsigned char *);
                len = m->m_len;
                memcpy(tmp2_ptr,data,len);
                tmp2_ptr = tmp2_ptr+len;
                total_len=total_len+len;
            }
            m_freem(m0);
            // ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit = AG7240_DESC_MORE_T | total_len;
             ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit = total_len;
            HAL_DCACHE_FLUSH((cyg_uint32)tmp_ptr, total_len);
            m_big->m_len = total_len;
            ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(tmp_ptr);
            //            ds->pkt_size = len;
            ds->packet = m_big;
            ag7240_tx_give_to_dma(ds);
    //            HAL_DCACHE_FLUSH(ds, sizeof(*ds));
            ag7240_ring_incr(r->ring_head);
            unused--;
    //        ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit &= AG7240_DESC_CLEAR_MORE_T;
            ds->more = 0;
            mac->mac_net_stats.tx_packets++;
        }
    }

    ar7240_flush_ge(mac->mac_unit);

    ag7240_tx_start(mac);

    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_intr_enable_tx(mac);
    }

    return ifp->if_snd.ifq_len;
#else
    struct eth_drv_sc *sc = ifp->if_softc;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    ag7240_ring_t* r = &mac->mac_txring[0];
    ag7240_desc_t* ds;
    struct mbuf *m0, *m;
    int len;
    int qlen;
    unsigned char *data;
    int unused;


    /* TODO */
//    if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
//         return 0;
//    }

    unused = ag7240_ndesc_unused(mac, r);

    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_tx_reap(mac, sc, 0);
    }

    qlen = ifp->if_snd.ifq_len;

    while (1) {
        if (unused < AG7240_TX_QSTART_THRESH) {
            break;
        }

        IF_DEQUEUE(&ifp->if_snd, m0);
        if (m0 == 0) {
            break;
        }
        qlen--;

        if ((m0->m_flags & M_PKTHDR) == 0) {
            panic("eth_drv_send: no header mbuf");
        }


        // Extract data pointers (don't actually move data here)
        for (m = m0; m ; m = m->m_next) {
            ds = &r->ring_desc[r->ring_head];
            data = mtod(m, unsigned char *);
            len = m->m_len;
            HAL_DCACHE_FLUSH((cyg_uint32)data, len);
            ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(data);
//            ds->pkt_size = len;
            ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit = AG7240_DESC_MORE_T | len;
            ds->packet = (m == m0)? m0: NULL;
            HAL_REORDER_BARRIER();
            ag7240_tx_give_to_dma(ds);
//            HAL_DCACHE_FLUSH(ds, sizeof(*ds));
            ag7240_ring_incr(r->ring_head);
            unused--;
        }
//        ((ag7240_desc_tmp_t*)ds)->for_more_pktlen_bit &= AG7240_DESC_CLEAR_MORE_T;
        ds->more = 0;
    }

    ar7240_flush_ge(mac->mac_unit);

    ag7240_tx_start(mac);

    if (unused < AG7240_TX_REAP_THRESH) {
        ag7240_intr_enable_tx(mac);
    }

    return qlen;
#endif
}



static int
ag7240_int_vector(struct eth_drv_sc *sc)
{
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    return  mac->mac_vector;
}



static int
ag7240_rx_replenish(ag7240_mac_t *mac)
{
    ag7240_ring_t* r = &mac->mac_rxring;
    int head = r->ring_head, tail = r->ring_tail, refilled = 0;
    ag7240_desc_t* ds;
    void* data;
    struct mbuf *m;

    CONSOLE_PRINTF("begin mac:%u", mac->mac_unit);
    do {
        ds = &r->ring_desc[tail];
        if (ds->packet != NULL) {
            break;
        }

        m = ag7240_alloc_pkt();
        if (m == NULL) {
            diag_printf("%s:%d: no mbuf\n", __func__, __LINE__);
            break;
        }
        ds->packet = m;
        CYG_ASSERT(ds->packet != NULL, "Null packet");
        data = mtod(m, void*);
        ds->pkt_start_addr = CYGARC_PHYSICAL_ADDRESS(data);
        HAL_DCACHE_INVALIDATE(data, 1600);

        HAL_REORDER_BARRIER();
        ag7240_rx_give_to_dma(ds);
        refilled++;

        ag7240_ring_incr(tail);
    } while (tail != head);
    /*
     * Flush descriptors
     */
    if (is_ar7240()) {
        ag7240_reg_wr(mac, AG7240_MAC_CFG1, (ag7240_reg_rd(mac, AG7240_MAC_CFG1)|0xc));
        ag7240_rx_start(mac);
    }

    r->ring_tail = tail;

    LEAVE();
    return refilled;
}

static int
ag7240_rx_reap(ag7240_mac_t *mac, struct eth_drv_sc *sc, int* quota)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    ag7240_ring_t* r = &mac->mac_rxring;
    int head = r->ring_head;
    ag7240_desc_t* ds;
    cyg_uint32 status;
    cyg_uint32 pkt_size;
    cyg_uint32 more_pkts;
    struct ether_header *eh;
    struct mbuf *m;
    int rep;
    u_int32_t cnt;


    CONSOLE_PRINTF("begin mac:%u", mac->mac_unit);
	status = ag7240_reg_rd(mac, AG7240_DMA_RX_STATUS);  
 
process_pkts:
    cnt = (status & AG7240_RX_STATUS_PKTCNT_MASK) >> AG7240_RX_STATUS_PKTCNT_SHIFT;

    ar7240_flush_ge(mac->mac_unit);
//    while (1) {
    while (cnt) {
        ds = &r->ring_desc[head];
        CONSOLE_PRINTF("status:0x%x head:%d ds:%p pkt_start_addr:0:%x packet:0x%x size:%u is_empty:%u",
                       status, head, ds, ds->pkt_start_addr, ds->packet, ds->pkt_size, ds->is_empty);

//        if (ag7240_rx_owned_by_dma(ds) || ds->packet == NULL) {
        if (ds->packet == NULL) {
            break;
        }
        ag7240_intr_ack_rx(mac);

        pkt_size = ds->pkt_size;
        m = ds->packet;
        CYG_ASSERT(m != NULL, "Null packet");
        ds->packet = NULL;
        m->m_len = pkt_size;
        m->m_pkthdr.rcvif = ifp;
        m->m_pkthdr.len = m->m_len = pkt_size -  sizeof(*eh) - ETHERNET_FCS_SIZE;

        eh = mtod(m, struct ether_header*);
        m->m_data += sizeof(*eh);
        ether_input(ifp, eh, m);
        ifp->if_ipackets++;
        mac->mac_net_stats.rx_packets++;
        ag7240_ring_incr(head);
        cnt--;
        (*quota)--;
        if ((*quota) <= 0) {
            break;
        }
    }
    r->ring_head = head;

    rep = ag7240_rx_replenish(mac);

    status = ag7240_reg_rd(mac, AG7240_DMA_RX_STATUS);
    more_pkts = (status & AG7240_RX_STATUS_PKT_RCVD);

    if (!more_pkts) goto done;

    if ((*quota)) goto process_pkts;

done:
    if (ag7240_rx_ring_full(mac)) {
        diag_printf("ring is full!!\n\n");
        goto end;
    }

    if (status & AG7240_RX_STATUS_OVF) {
        ag7240_intr_ack_rxovf(mac);
        ag7240_rx_start(mac);
    }
end:
    ag7240_intr_enable_recv(mac);
    LEAVE();

    return more_pkts;
}

static int
ag7240_tx_reap(ag7240_mac_t *mac, struct eth_drv_sc *sc , int ac)
{
    ag7240_ring_t *r;
    ag7240_desc_t *ds;
    int head, tail, reaped = 0;
    struct mbuf* m;
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    u_int32_t status;
    u_int32_t cnt;

    ARRIVE();
    r = &mac->mac_txring[ac];
    head = r->ring_head;
    tail = r->ring_tail;

    ar7240_flush_ge(mac->mac_unit);
    status = ag7240_reg_rd(mac, AG7240_DMA_TX_STATUS);
    cnt = (status & AG7240_TX_STATUS_PKTCNT_MASK) >> AG7240_TX_STATUS_PKTCNT_SHIFT;
    reaped = cnt;

    while ((tail != head) && cnt) {
        ds = CYGARC_UNCACHED_ADDRESS(&r->ring_desc[tail]);
//        if(ag7240_tx_owned_by_dma(ds))
//            break;
        CONSOLE_PRINTF("head:%d tail:%d ds:%p pkt_start_addr:0:%x packet:0x%x size:%u is_empty:%u",
                       head, tail, ds, ds->pkt_start_addr, ds->packet, ds->pkt_size, ds->is_empty);

        m = ds->packet;
        if(m != NULL)
        ag7240_free_pkt(m);

        ds->pkt_start_addr = 0;
        ds->packet = NULL;
//        ds->packet = NULL;
//        ds->more = 0;
//        ds->pkt_start_addr = NULL;

        ag7240_intr_ack_tx(mac);
        ag7240_ring_incr(tail);

        cnt--;
    }
    r->ring_tail = tail;
    reaped = reaped - cnt;
    ifp->if_opackets += reaped;


    if ((ag7240_ndesc_unused(mac, r) >= AG7240_TX_REAP_THRESH)) {
        if (ag7240_reg_rd(mac, AG7240_DMA_INTR_MASK) & AG7240_INTR_TX) {
            ag7240_intr_disable_tx(mac);
        }
        CONSOLE_PRINTF("tx ring start");
    }
    LEAVE();
    return reaped;
}


static cyg_uint32
ag7240_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 isr, imr;
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;

    ARRIVE();

    cyg_interrupt_mask(vector);
    cyg_interrupt_acknowledge(vector);

    isr = ag7240_get_isr(mac);

    if (isr & (AG7240_INTR_RX_OVF)) {
        if (is_ar7240()) {
            ag7240_reg_wr(mac, AG7240_MAC_CFG1, (ag7240_reg_rd(mac, AG7240_MAC_CFG1) & 0xfffffff3));
        }
        ag7240_intr_ack_rxovf(mac);
    }

    if (isr & AG7240_INTR_RX_BUS_ERROR) {
        ag7240_intr_ack_rxbe(mac);
    }

    if (isr & AG7240_INTR_TX_BUS_ERROR) {
        ag7240_intr_ack_txbe(mac);
    }

    if (isr & ~(AG7240_INTR_RX_OVF | AG7240_INTR_RX_BUS_ERROR | AG7240_INTR_TX_BUS_ERROR)) {
        LEAVE();
        return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
    }
    else {
        cyg_interrupt_unmask(vector);
        LEAVE();
        return (CYG_ISR_HANDLED);
    }
}


static cyg_uint32
ag7240_link_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    ar7240_s26_intr();
    return CYG_ISR_HANDLED;
}


static int
ag7240_if_poll_recv(struct ifnet *ifp, int* budget)
{
    cyg_uint32 isr, imr;
    int ac;
    int ret = 0;
    struct eth_drv_sc *sc = ifp->if_softc;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;


    CONSOLE_PRINTF("begin mac:%u", mac->mac_unit);
    isr = ag7240_get_isr(mac);
//    imr = ag7240_reg_rd(mac, AG7240_DMA_INTR_MASK);

    if (isr & AG7240_INTR_RX) {
        ret = ag7240_rx_reap(mac, sc, budget);
    }

    if (isr & AG7240_INTR_TX) {
//        ag7240_intr_ack_tx(mac);
        /* Which queue to reap ??????????? */
            ag7240_tx_reap(mac, sc, 0);
/*
        for (ac = 0; ac < mac->mac_noacs; ac++) {
            ag7240_tx_reap(mac, sc, ac);
        }
*/
    }
    cyg_interrupt_unmask(mac->mac_vector);
    LEAVE();

    return ret;
}


static void
ag7240_get_default_macaddr(ag7240_mac_t *mac, cyg_uint8 *mac_addr)
{
    /* Use MAC address stored in Flash */

#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_MAC_LOCATION
    cyg_uint8 *eep_mac_addr = (cyg_uint8 *)(CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_MAC_LOCATION + (mac->mac_unit)*6);
#else
    cyg_uint8 *eep_mac_addr = (cyg_uint8 *)((mac->mac_unit) ? AR7240_EEPROM_GE1_MAC_ADDR: AR7240_EEPROM_GE0_MAC_ADDR);
#endif

    /*
    ** Check for a valid manufacturer prefix.  If not, then use the defaults
    */

    if(eep_mac_addr[0] == 0x00 &&
       eep_mac_addr[1] == 0x03 &&
       eep_mac_addr[2] == 0x7f)
    {
        mac_addr[0] = eep_mac_addr[0];
        mac_addr[1] = eep_mac_addr[1];
        mac_addr[2] = eep_mac_addr[2];
        mac_addr[3] = eep_mac_addr[3];
        mac_addr[4] = eep_mac_addr[4];
        mac_addr[5] = eep_mac_addr[5];
    }
    else
    {
        /* Use Default address at top of range */
        mac_addr[0] = 0x00;
        mac_addr[1] = 0x03;
        mac_addr[2] = 0x7F;
        mac_addr[3] = 0xFF;
        mac_addr[4] = 0xFF;
        mac_addr[5] = 0xFF - mac->mac_unit;
    }
}

void ag7240_get_stats(struct ifnet *ifp,void *ifr);
static void
ag7240_reset_hw(ag7240_mac_t *mac)
{
    cyg_uint32 mask;

    ARRIVE();
    ag7240_reg_rmw_set(mac, AG7240_MAC_CFG1, AG7240_MAC_CFG1_SOFT_RST |
                       AG7240_MAC_CFG1_RX_RST | AG7240_MAC_CFG1_TX_RST);

    HAL_DELAY_US(20);
    mask = ag7240_reset_mask(mac->mac_unit);

    ar7240_reg_rmw_set(AR7240_RESET, mask);
    HAL_DELAY_US(100*1000);

    ar7240_reg_rmw_clear(AR7240_RESET, mask);
    HAL_DELAY_US(100*1000);
    LEAVE();
}
static int
ag7240_control(struct eth_drv_sc *sc,
               unsigned long key,
               void *data,
               int data_length)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    int error =0;
    ARRIVE();
    /* TBDXXX */
    switch (key) {
    case ETH_DRV_GET_IF_STATS_UD:
    case ETH_DRV_GET_IF_STATS:
        ag7240_get_stats(ifp,data);
        error = 0;
        break;
    case ETH_DRV_SET_MC_ALL :
    case ETH_DRV_SET_MC_LIST:
        //printf("%s[%04u]\n\r",__func__,__LINE__);
        error = 0;
        break;
    default:
        error = -1;
        break;
    }
    LEAVE();
    return error;
}

void ag7240_get_stats(struct ifnet *ifp,void *ifr)
{
    struct eth_drv_sc *sc = ifp->if_softc;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    struct ifreq *ifr_ptr = ifr;

    copyout(&mac->mac_net_stats.rx_packets, ifr_ptr->ifr_ifru.ifru_data, sizeof(struct net_device_stats));
    return;
}

static bool
ag7240_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    ag7240_mac_t *mac = (ag7240_mac_t *)sc->driver_private;
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    int unit;
    unsigned char mac_addr[ETHER_ADDR_LEN];

    ARRIVE();

    unit = mac->mac_unit;
    ag7240_macs[unit] = mac;
#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_S26_PHY_SWONLY
    if (is_ar7241()) {
        if (unit) {
            mac_set_flag(mac, ETH_SWONLY_MODE);
        }
    }
#endif

    /* Get ethernet's MAC address from board configuration data */
    ag7240_get_default_macaddr(mac, mac_addr);



    /* Reset the device */
    ag7240_reset_hw(mac);

    (sc->funs->eth_drv->init)(sc, mac_addr);
    ifp->if_poll_xmit = ag7240_if_poll_xmit;
    ifp->if_poll_recv = ag7240_if_poll_recv;

    if (unit == 0) {
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC,
                                 0,
                                 (CYG_ADDRWORD)sc,
                                 ag7240_link_isr,
                                 NULL,
                                 &link_interrupt_handle,
                                 &link_interrupt_object);
        cyg_drv_interrupt_attach(link_interrupt_handle);
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC);
    }

    cyg_drv_interrupt_create(mac->mac_vector,
                             0,
                             (CYG_ADDRWORD)sc,
                             ag7240_isr,
                             eth_drv_dsr,
                             &mac->mac_interrupt_handle,
                             &mac->mac_interrupt_object);
    cyg_drv_interrupt_attach(mac->mac_interrupt_handle);


    mac->link_up = 0;
    mac->mac_speed = 0xff;

    memset(&mac->mac_phy_timer,0x00,sizeof(mac->mac_phy_timer));
    mac->mac_net_stats.rx_packets = 0;
    mac->mac_net_stats.tx_packets = 0;
    mac->mac_net_stats.rx_error_packets = 0;
    mac->mac_net_stats.tx_error_packets = 0;
    mac->mac_net_stats.rx_drop_packets = 0;
    mac->mac_net_stats.tx_drop_packets = 0;

    LEAVE();

    return true;
}


/* Ethernet Unit 0 */
static ag7240_mac_t ag7240_priv_data0 = {
                                           .mac_name = CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_GE0_NAME,
                                           .mac_unit = 0,
                                           .mac_base = AR7240_GE0_BASE,
                                           .mac_vector = CYGNUM_HAL_INTERRUPT_ETH0_VEC,
                                           .mac_noacs = 1,
                                       };

ETH_DRV_SC(ag7240_sc0,
           &ag7240_priv_data0,
           CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_GE0_NAME,
           ag7240_if_start,
           ag7240_if_stop,
           ag7240_control,
           null_can_send,
           null_send,
           null_recv,
           null_deliver,
           null_poll,
           ag7240_int_vector);


NETDEVTAB_ENTRY(ag7240_netdev0,
                "ag7240_eth0",
                ag7240_init,
                &ag7240_sc0);

/* Ethernet Unit 1 */
static ag7240_mac_t ag7240_priv_data1 = {
                                           .mac_name = CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_GE1_NAME,
                                           .mac_unit = 1,
                                           .mac_base = AR7240_GE1_BASE,
                                           .mac_vector = CYGNUM_HAL_INTERRUPT_ETH1_VEC,
                                           .mac_noacs = 1,
                                       };


ETH_DRV_SC(ag7240_sc1,
           &ag7240_priv_data1,
           CYGPKG_DEVS_ETH_MIPS_MIPS32_AG7240_GE1_NAME,
           ag7240_if_start,
           ag7240_if_stop,
           ag7240_control,
           null_can_send,
           null_send,
           null_recv,
           null_deliver,
           null_poll,
           ag7240_int_vector);


NETDEVTAB_ENTRY(ag7240_netdev1,
                "ag7240_eth1",
                ag7240_init,
                &ag7240_sc1);


