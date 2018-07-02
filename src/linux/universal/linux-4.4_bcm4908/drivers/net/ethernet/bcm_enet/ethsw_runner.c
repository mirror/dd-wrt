/*
 Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard

 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:

    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.

 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.

 :>
*/

#define _BCMENET_LOCAL_

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmenet.h"
#include "bcmmii.h"
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
#include "phys_common_drv.h"
#endif
#include "hwapi_mac.h"
#include <rdpa_api.h>
#include "bcmswshared.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#include "eth_pwrmngt.h"
#include "bcmsw_runner.h"

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
#define RUNNER_PORT_MIRROR_SUPPORT
#endif

extern struct semaphore bcm_ethlock_switch_config;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
PHY_STAT ethsw_phy_stat(int unit, int port, int cb_port) /* FIXME  - very similar code/functionality; Merge together with the above */
{
    PHY_STAT phys;
    phy_rate_t curr_phy_rate;
    int phyId;

    memset(&phys, 0, sizeof(phys));
    phyId = enet_sw_port_to_phyid(0, port);
    if (!IsPhyConnected(phyId))
    {
        // 0xff PHY ID means no PHY on this port.
        phys.lnk = 1;
        phys.fdx = 1;
        if (IsMII(phyId))
            phys.spd100 = 1;
        else
            phys.spd1000 = 1;
        return phys;
    }

    curr_phy_rate = port_get_link_speed(port);
    phys.lnk = curr_phy_rate < PHY_RATE_LINK_DOWN;
    switch (curr_phy_rate)
    {
        case PHY_RATE_10_FULL:
            phys.fdx = 1;
            phys.spd10 = 1;
            break;
        case PHY_RATE_10_HALF:
            phys.spd10 = 1;
            break;
        case PHY_RATE_100_FULL:
            phys.fdx = 1;
            phys.spd100 = 1;
            break;
        case PHY_RATE_100_HALF:
            phys.spd100 = 1;
            break;
        case PHY_RATE_1000_FULL:
            phys.spd1000 = 1;
            phys.fdx = 1;
            break;
        case PHY_RATE_1000_HALF:
            phys.spd1000 = 1;
            break;
        case PHY_RATE_2500_FULL:
            phys.spd2500 = 1;
            phys.fdx = 1;
            break;
        case PHY_RATE_LINK_DOWN:
            break;
        default:
            break;
    }

    return phys;
}
#endif

int ethsw_reset_ports(struct net_device *dev)
{
    return 0;
}

int ethsw_add_proc_files(struct net_device *dev)
{
    return 0;
}

int ethsw_del_proc_files(void)
{
    return 0;
}

int ethsw_disable_hw_switching(void)
{
    return 0;
}

void ethsw_dump_page(int page)
{
}

void fast_age_port(uint8_t port, uint8_t age_static)
{
}

int ethsw_counter_collect(uint32_t portmap, int discard)
{
    return 0;
}

void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);

int ethsw_phy_intr_ctrl(int port, int on)
{
    return 0;
}
void ethsw_phy_apply_init_bp(void)
{
}
int ethsw_setup_led(void)
{
    return 0;
}
int ethsw_setup_phys(void)
{
    ethsw_shutdown_unused_phys();
    return 0;
}
int ethsw_enable_hw_switching(void)
{
    return 0;
}

/* Code to handle exceptions chip specific cases */
void ethsw_phy_handle_exception_cases(void)
{
}

#if defined(RUNNER_PORT_MIRROR_SUPPORT)
static int convert_pmap2portNum(unsigned int pmap)
{
    int portNum = 0xFFFF;

    if (pmap == 0)
        return portNum;

    for (portNum = 0;;portNum++) {
        if (pmap & (1 << portNum))
            break;
    }

    return portNum;
}
#endif

void ethsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap,
                           unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                           int *tx_port, int *rx_port)
{
#if defined(RUNNER_PORT_MIRROR_SUPPORT)
    bdmf_object_handle port_obj = NULL;
    rdpa_port_mirror_cfg_t mirror_cfg;
    rdpa_if port_index;

    *enable = 0;
    *mirror_port = 0;
    *ing_pmap = 0;
    *eg_pmap = 0;
    *blk_no_mrr = 0;
    *tx_port = -1;
    *rx_port = -1;

    if (rdpa_port_get(rdpa_if_wan0, &port_obj) != 0)
        return;

    memset(&mirror_cfg, 0, sizeof(mirror_cfg));

    if (rdpa_port_mirror_cfg_get(port_obj, &mirror_cfg) != 0)
        return;

    if (mirror_cfg.rx_dst_port != NULL)
    {
        if (rdpa_port_index_get(mirror_cfg.rx_dst_port, &port_index) == 0)
        {
            *ing_pmap = 1 << EPON_PORT_ID;
            *enable = 1;
            *rx_port = port_index - rdpa_if_lan0;
        }
    }

    if (mirror_cfg.tx_dst_port != NULL)
    {
        if (rdpa_port_index_get(mirror_cfg.tx_dst_port, &port_index) == 0)
        {
            *eg_pmap = 1 << EPON_PORT_ID;
            *enable = 1;
            *tx_port = port_index - rdpa_if_lan0;
        }
    }

    if (port_obj)
        bdmf_put(port_obj);

#else
    printk("Runner port mirroring is not supported\n");
#endif
}

void ethsw_port_mirror_set(int enable, int mirror_port, unsigned int ing_pmap,
                           unsigned int eg_pmap, unsigned int blk_no_mrr,
                           int tx_port, int rx_port)
{
#if defined(RUNNER_PORT_MIRROR_SUPPORT)
    bdmf_object_handle port_obj = NULL, tx_port_obj = NULL, rx_port_obj = NULL;
    rdpa_port_mirror_cfg_t mirror_cfg;
    int src_rx_port = convert_pmap2portNum(ing_pmap);
    int dst_rx_port = (rx_port == -1)?mirror_port:rx_port;
    int src_tx_port = convert_pmap2portNum(eg_pmap);
    int dst_tx_port = (tx_port == -1)?mirror_port:tx_port;

    /* Only support mirror WAN port now */
    if ((ing_pmap ==0) && (eg_pmap == 0)) {
        printk("Invalid ingress and egress port map %x - %x\n", ing_pmap, eg_pmap);
        return;
    }

    if ((ing_pmap != 0) && (src_rx_port != EPON_PORT_ID)) {
        printk("Invalid ingress port map %x\n", ing_pmap);
        return;
    }

    if ((eg_pmap != 0) && (src_tx_port != EPON_PORT_ID)) {
        printk("Invalid egress port map %x\n", eg_pmap);
        return;
    }

    if (rdpa_port_get(rdpa_if_wan0, &port_obj) != 0)
        return;

    memset(&mirror_cfg, 0, sizeof(mirror_cfg));

    if (rdpa_port_mirror_cfg_get(port_obj, &mirror_cfg) != 0)
        goto free_all;

    /* Get the port which mirrors Rx traffic */
    if (src_rx_port == EPON_PORT_ID) {
        if(!rdpa_if_is_lan(rdpa_if_lan0 + dst_rx_port) ||
           (rdpa_port_get(rdpa_if_lan0 + dst_rx_port, &rx_port_obj) != 0)) {
            printk("Invalid mirror port(Rx) %d\n", dst_rx_port);
            goto free_all;
        }
    }

    /* Get the port which mirrors Tx traffic */
    if (src_tx_port == EPON_PORT_ID) {
        if(!rdpa_if_is_lan(rdpa_if_lan0 + dst_tx_port) ||
           (rdpa_port_get(rdpa_if_lan0 + dst_tx_port, &tx_port_obj) != 0)) {
            printk("Invalid mirror port(Tx) %d\n", dst_tx_port);
            goto free_all;
        }
    }

    if (!enable) {
        if (mirror_cfg.rx_dst_port == rx_port_obj)
            mirror_cfg.rx_dst_port = NULL;

        if (mirror_cfg.tx_dst_port == tx_port_obj)
            mirror_cfg.tx_dst_port = NULL;
    }
    else {
        if (rx_port_obj != NULL)
            mirror_cfg.rx_dst_port = rx_port_obj;

        if (tx_port_obj != NULL )
            mirror_cfg.tx_dst_port = tx_port_obj;
    }

    if (rdpa_port_mirror_cfg_set(port_obj, &mirror_cfg) != 0)
        printk("Set port mirror failed!\n");

free_all:
    if (rx_port_obj)
        bdmf_put(rx_port_obj);

    if (tx_port_obj)
        bdmf_put(tx_port_obj);

    if (port_obj)
        bdmf_put(port_obj);

#else
    printk("Runner port mirroring is not supported\n");
#endif
}

