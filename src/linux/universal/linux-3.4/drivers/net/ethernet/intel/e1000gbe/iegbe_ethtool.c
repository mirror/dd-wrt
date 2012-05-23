/********************************************************************
 
GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98

  Contact Information:
  
  Intel Corporation, 5000 W. Chandler Blvd, Chandler, AZ 85226 

********************************************************************/

/**************************************************************************
 * @ingroup ETHTOOL_GENERAL
 *
 * @file iegbe_ethtool.c
 *
 * @description
 *   This module contains ethtool support for IEGBE.
 *
 **************************************************************************/

#include "iegbe.h"

#ifdef    SIOCETHTOOL
#include <asm/uaccess.h>
#endif

#ifdef ETHTOOL_OPS_COMPAT
#include "kcompat_ethtool.c"
#endif

extern char iegbe_driver_name[];
extern char iegbe_driver_version[];

extern int iegbe_up(struct iegbe_adapter *adapter);
extern void iegbe_down(struct iegbe_adapter *adapter);
extern void iegbe_reset(struct iegbe_adapter *adapter);
extern int iegbe_set_spd_dplx(struct iegbe_adapter *adapter, uint16_t spddplx);
extern int iegbe_setup_all_rx_resources(struct iegbe_adapter *adapter);
extern int iegbe_setup_all_tx_resources(struct iegbe_adapter *adapter);
extern void iegbe_free_all_rx_resources(struct iegbe_adapter *adapter);
extern void iegbe_free_all_tx_resources(struct iegbe_adapter *adapter);
extern void iegbe_update_stats(struct iegbe_adapter *adapter);

#ifndef ETH_GSTRING_LEN
#define ETH_GSTRING_LEN 0x20
#endif

#ifdef ETHTOOL_GSTATS
struct iegbe_stats {
    char stat_string[ETH_GSTRING_LEN];
    int sizeof_stat;
    int stat_offset;
};

#define E1000_STAT(m) sizeof(((struct iegbe_adapter *)0)->m), \
              offsetof(struct iegbe_adapter, m)
static const struct iegbe_stats iegbe_gstrings_stats[] = {
    { "rx_packets", E1000_STAT(net_stats.rx_packets) },
    { "tx_packets", E1000_STAT(net_stats.tx_packets) },
    { "rx_bytes", E1000_STAT(net_stats.rx_bytes) },
    { "tx_bytes", E1000_STAT(net_stats.tx_bytes) },
    { "rx_errors", E1000_STAT(net_stats.rx_errors) },
    { "tx_errors", E1000_STAT(net_stats.tx_errors) },
    { "rx_dropped", E1000_STAT(net_stats.rx_dropped) },
    { "tx_dropped", E1000_STAT(net_stats.tx_dropped) },
    { "multicast", E1000_STAT(net_stats.multicast) },
    { "collisions", E1000_STAT(net_stats.collisions) },
    { "rx_length_errors", E1000_STAT(net_stats.rx_length_errors) },
    { "rx_over_errors", E1000_STAT(net_stats.rx_over_errors) },
    { "rx_crc_errors", E1000_STAT(net_stats.rx_crc_errors) },
    { "rx_frame_errors", E1000_STAT(net_stats.rx_frame_errors) },
    { "rx_fifo_errors", E1000_STAT(net_stats.rx_fifo_errors) },
    { "rx_no_buffer_count", E1000_STAT(stats.rnbc) },
    { "rx_missed_errors", E1000_STAT(net_stats.rx_missed_errors) },
    { "tx_aborted_errors", E1000_STAT(net_stats.tx_aborted_errors) },
    { "tx_carrier_errors", E1000_STAT(net_stats.tx_carrier_errors) },
    { "tx_fifo_errors", E1000_STAT(net_stats.tx_fifo_errors) },
    { "tx_heartbeat_errors", E1000_STAT(net_stats.tx_heartbeat_errors) },
    { "tx_window_errors", E1000_STAT(net_stats.tx_window_errors) },
    { "tx_abort_late_coll", E1000_STAT(stats.latecol) },
    { "tx_deferred_ok", E1000_STAT(stats.dc) },
    { "tx_single_coll_ok", E1000_STAT(stats.scc) },
    { "tx_multi_coll_ok", E1000_STAT(stats.mcc) },
    { "rx_long_length_errors", E1000_STAT(stats.roc) },
    { "rx_short_length_errors", E1000_STAT(stats.ruc) },
    { "rx_align_errors", E1000_STAT(stats.algnerrc) },
    { "tx_tcp_seg_good", E1000_STAT(stats.tsctc) },
    { "tx_tcp_seg_failed", E1000_STAT(stats.tsctfc) },
    { "rx_flow_control_xon", E1000_STAT(stats.xonrxc) },
    { "rx_flow_control_xoff", E1000_STAT(stats.xoffrxc) },
    { "tx_flow_control_xon", E1000_STAT(stats.xontxc) },
    { "tx_flow_control_xoff", E1000_STAT(stats.xofftxc) },
    { "rx_long_byte_count", E1000_STAT(stats.gorcl) },
    { "rx_csum_offload_good", E1000_STAT(hw_csum_good) },
    { "rx_csum_offload_errors", E1000_STAT(hw_csum_err) },
    { "rx_header_split", E1000_STAT(rx_hdr_split) },
#ifdef E1000_COUNT_ICR
    { "txdw", E1000_STAT(icr_txdw) },
    { "txqe", E1000_STAT(icr_txqe) },
    { "lsc", E1000_STAT(icr_lsc) },
    { "rxseq", E1000_STAT(icr_rxseq) },
    { "rxdmt", E1000_STAT(icr_rxdmt) },
    { "rxo", E1000_STAT(icr_rxo) },
    { "rxt", E1000_STAT(icr_rxt) },
    { "mdac", E1000_STAT(icr_mdac) },
    { "rxcfg", E1000_STAT(icr_rxcfg) },
    { "gpi", E1000_STAT(icr_gpi) },
    { "pb", E1000_STAT(icr_pb) },
    { "intmem", E1000_STAT(icr_intmem_icp_xxxx) },
    { "cpp_target", E1000_STAT(icr_cpp_target) },
    { "cpp_master", E1000_STAT(icr_cpp_master) },
    { "stat", E1000_STAT(icr_stat) },
#endif
};
#define E1000_STATS_LEN    \
    sizeof(iegbe_gstrings_stats) / sizeof(struct iegbe_stats)
#endif /* ETHTOOL_GSTATS */
#ifdef ETHTOOL_TEST
static const char iegbe_gstrings_test[][ETH_GSTRING_LEN] = {
    "Register test  (offline)", "Eeprom test    (offline)",
    "Interrupt test (offline)", "Loopback test  (offline)",
    "Link test   (on/offline)"
};
#define E1000_TEST_LEN (sizeof(iegbe_gstrings_test) / ETH_GSTRING_LEN)
#endif /* ETHTOOL_TEST */

#define E1000_REGS_LEN 0x20

static int
iegbe_get_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;

    if((hw->media_type == iegbe_media_type_copper) || 
       (hw->media_type == iegbe_media_type_oem)) {

        ecmd->supported = (SUPPORTED_10baseT_Half |
                           SUPPORTED_10baseT_Full |
                           SUPPORTED_100baseT_Half |
                           SUPPORTED_100baseT_Full |
                           SUPPORTED_1000baseT_Full|
                           SUPPORTED_Autoneg |
                           SUPPORTED_TP);

        ecmd->advertising = ADVERTISED_TP;

        if(hw->autoneg == 1) {
            ecmd->advertising |= ADVERTISED_Autoneg;

            /* the iegbe autoneg seems to match ethtool nicely */

            ecmd->advertising |= hw->autoneg_advertised;
        }

        ecmd->port = PORT_TP;
        ecmd->phy_address = hw->phy_addr;

        if(hw->mac_type == iegbe_82543){
            ecmd->transceiver = XCVR_EXTERNAL;
        } else {
            ecmd->transceiver = XCVR_INTERNAL;
        }
    } else {
        ecmd->supported   = (SUPPORTED_1000baseT_Full |
                     SUPPORTED_FIBRE |
                     SUPPORTED_Autoneg);

        ecmd->advertising = (ADVERTISED_1000baseT_Full |
                     ADVERTISED_FIBRE |
                     ADVERTISED_Autoneg);

        ecmd->port = PORT_FIBRE;

        if(hw->mac_type >= iegbe_82545){
            ecmd->transceiver = XCVR_INTERNAL;
        }else {
            ecmd->transceiver = XCVR_EXTERNAL;
          }        
    }

    if(netif_carrier_ok(adapter->netdev)) {

        iegbe_get_speed_and_duplex(hw, &adapter->link_speed,
                                           &adapter->link_duplex);
        ecmd->speed = adapter->link_speed;

        /* unfortunatly FULL_DUPLEX != DUPLEX_FULL
         *          and HALF_DUPLEX != DUPLEX_HALF */

        if(adapter->link_duplex == FULL_DUPLEX){
            ecmd->duplex = DUPLEX_FULL;
        } else{
            ecmd->duplex = DUPLEX_HALF;
          }        
    } else {
        ecmd->speed = -1;
        ecmd->duplex = -1;
    }

    ecmd->autoneg = ((hw->media_type == iegbe_media_type_fiber) ||
             hw->autoneg) ? AUTONEG_ENABLE : AUTONEG_DISABLE;
    return 0;
}

static int
iegbe_set_settings(struct net_device *netdev, struct ethtool_cmd *ecmd)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;

    if(ecmd->autoneg == AUTONEG_ENABLE) {
        hw->autoneg = 1;
        if (hw->media_type == iegbe_media_type_fiber){
            hw->autoneg_advertised = ADVERTISED_1000baseT_Full |
                     ADVERTISED_FIBRE |
                     ADVERTISED_Autoneg;
        } else {
            hw->autoneg_advertised = ADVERTISED_10baseT_Half |
                          ADVERTISED_10baseT_Full |
                          ADVERTISED_100baseT_Half |
                          ADVERTISED_100baseT_Full |
                          ADVERTISED_1000baseT_Full|
                          ADVERTISED_Autoneg |
                          ADVERTISED_TP;
                          ecmd->advertising = hw->autoneg_advertised;
          }
    } else
        if(iegbe_set_spd_dplx(adapter, ecmd->speed + ecmd->duplex)){
            return -EINVAL;
    }
    /* reset the link */

    if(netif_running(adapter->netdev)) {
        iegbe_down(adapter);
        iegbe_reset(adapter);
        iegbe_up(adapter);
    } else{
        iegbe_reset(adapter);
    }
    return 0;
}

static void
iegbe_get_pauseparam(struct net_device *netdev,
                     struct ethtool_pauseparam *pause)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;

    pause->autoneg = 
        (adapter->fc_autoneg ? AUTONEG_ENABLE : AUTONEG_DISABLE);
    
    if(hw->fc == iegbe_fc_rx_pause){
        pause->rx_pause = 1;
    }          
    else if(hw->fc == iegbe_fc_tx_pause){
        pause->tx_pause = 1;
    }          
    else if(hw->fc == iegbe_fc_full) {
        pause->rx_pause = 1;
        pause->tx_pause = 1;
    }
}

static int
iegbe_set_pauseparam(struct net_device *netdev,
                     struct ethtool_pauseparam *pause)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;
    int ret_val;

    adapter->fc_autoneg = pause->autoneg;

    if(pause->rx_pause && pause->tx_pause){
        hw->fc = iegbe_fc_full;
    }          
    else if(pause->rx_pause && !pause->tx_pause){
        hw->fc = iegbe_fc_rx_pause;
    }          
    else if(!pause->rx_pause && pause->tx_pause){
        hw->fc = iegbe_fc_tx_pause;
    }          
    else if(!pause->rx_pause && !pause->tx_pause){
        hw->fc = iegbe_fc_none;
    }
    hw->original_fc = hw->fc;

    if(adapter->fc_autoneg == AUTONEG_ENABLE) {
        if(netif_running(adapter->netdev)) {
            iegbe_down(adapter);
            iegbe_up(adapter);
        } else {
            iegbe_reset(adapter);
        }
    }
    else {
        ret_val = ((hw->media_type == iegbe_media_type_fiber)
                || (hw->media_type == iegbe_media_type_oem
                    && !iegbe_oem_phy_is_copper(&adapter->hw)) ?
            iegbe_setup_link(hw) : iegbe_force_mac_fc(hw));
        return ret_val;
    }
    
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
static uint32_t
iegbe_get_rx_csum(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    return adapter->rx_csum;
}

static int
iegbe_set_rx_csum(struct net_device *netdev, uint32_t data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    adapter->rx_csum = data;

    if(netif_running(netdev)) {
        iegbe_down(adapter);
        iegbe_up(adapter);
    } else{
        iegbe_reset(adapter);
    }
    return 0;
}

static uint32_t
iegbe_get_tx_csum(struct net_device *netdev)
{
    return (netdev->features & NETIF_F_HW_CSUM) != 0;
}

static int
iegbe_set_tx_csum(struct net_device *netdev, uint32_t data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);

    if(adapter->hw.mac_type < iegbe_82543) {
        if(!data){
            return -EINVAL;
        }                
        return 0;
    }

    if(data){
        netdev->features |= NETIF_F_HW_CSUM;  
    } else {
        netdev->features &= ~NETIF_F_HW_CSUM;
    }
    return 0;
}

#ifdef NETIF_F_TSO
static int
iegbe_set_tso(struct net_device *netdev, uint32_t data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    if ((adapter->hw.mac_type < iegbe_82544) ||
        (adapter->hw.mac_type == iegbe_82547)){
        return data ? -EINVAL : 0;
    }
    if(data){
        netdev->features |= NETIF_F_TSO;
    } else {
        netdev->features &= ~NETIF_F_TSO;
    }          
    return 0;
} 
#endif /* NETIF_F_TSO */
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)) */

static uint32_t
iegbe_get_msglevel(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    return adapter->msg_enable;
}

static void
iegbe_set_msglevel(struct net_device *netdev, uint32_t data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    adapter->msg_enable = data;
}

static int 
iegbe_get_regs_len(struct net_device *netdev)
{
    return E1000_REGS_LEN * sizeof(uint32_t);
}

static void
iegbe_get_regs(struct net_device *netdev,
           struct ethtool_regs *regs, void *p)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;
    uint32_t *regs_buff = p;
    uint16_t phy_data;

    memset(p, 0, E1000_REGS_LEN * sizeof(uint32_t));

    regs->version = (1 << 0x18) | (hw->revision_id << 0x10) | hw->device_id;

    regs_buff[0x0]  = E1000_READ_REG(hw, CTRL);
    regs_buff[0x1]  = E1000_READ_REG(hw, STATUS);

    regs_buff[0x2]  = E1000_READ_REG(hw, RCTL);
    regs_buff[0x3]  = E1000_READ_REG(hw, RDLEN);
    regs_buff[0x4]  = E1000_READ_REG(hw, RDH);
    regs_buff[0x5]  = E1000_READ_REG(hw, RDT);
    regs_buff[0x6]  = E1000_READ_REG(hw, RDTR);

    regs_buff[0x7]  = E1000_READ_REG(hw, TCTL);
    regs_buff[0x8]  = E1000_READ_REG(hw, TDLEN);
    regs_buff[0x9]  = E1000_READ_REG(hw, TDH);
    regs_buff[0xa] = E1000_READ_REG(hw, TDT);
    regs_buff[0xb] = E1000_READ_REG(hw, TIDV);

    regs_buff[0xc] = adapter->hw.phy_type;  /* PHY type (IGP=1, M88=0, OEM=3) */
    /* 
     * Fill elements 13 though 23 of regs_buff[]
     *
     * [13] = cable length
     * [14] = cable length
     * [15] = cable length
     * [16] = cable length
     * [17] = extended 10bt distance
     * [18] = cable polarity
     * [19] = cable polarity
     * [20] = polarity correction enabled
     * [21] =
     * [22] = phy receive errors
     * [23] = mdix mode
     */
    switch(hw->phy_type){
    case iegbe_phy_igp:

        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT,
                    IGP01E1000_PHY_AGC_A);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_AGC_A &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0xd] = (uint32_t)phy_data; /* cable length */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT,
                    IGP01E1000_PHY_AGC_B);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_AGC_B &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0xe] = (uint32_t)phy_data; /* cable length */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT,
                    IGP01E1000_PHY_AGC_C);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_AGC_C &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0xf] = (uint32_t)phy_data; /* cable length */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT,
                    IGP01E1000_PHY_AGC_D);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_AGC_D &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0x10] = (uint32_t)phy_data; /* cable length */
        regs_buff[0x11] = 0; /* extended 10bt distance (not needed) */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT, 0x0);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_PORT_STATUS &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0x12] = (uint32_t)phy_data; /* cable polarity */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT,
                    IGP01E1000_PHY_PCS_INIT_REG);
        iegbe_read_phy_reg(hw, IGP01E1000_PHY_PCS_INIT_REG &
                   IGP01E1000_PHY_PAGE_SELECT, &phy_data);
        regs_buff[0x13] = (uint32_t)phy_data; /* cable polarity */
        regs_buff[0x14] = 0; /* polarity correction enabled (always) */
        regs_buff[0x15] = 0; /* phy receive errors (unavailable) */
        regs_buff[0x16] = regs_buff[0x12]; /* mdix mode */
        iegbe_write_phy_reg(hw, IGP01E1000_PHY_PAGE_SELECT, 0x0);
        break;

    case iegbe_phy_igp_2:
    case iegbe_phy_m88:

            iegbe_read_phy_reg(hw, M88E1000_PHY_SPEC_STATUS, &phy_data);
        regs_buff[0xd] = (uint32_t)phy_data; /* cable length */
        regs_buff[0xe] = 0;  /* Dummy (to align w/ IGP phy reg dump) */
        regs_buff[0xf] = 0;  /* Dummy (to align w/ IGP phy reg dump) */
        regs_buff[0x10] = 0;  /* Dummy (to align w/ IGP phy reg dump) */
            iegbe_read_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, &phy_data);
        regs_buff[0x11] = (uint32_t)phy_data; /* extended 10bt distance */
        regs_buff[0x12] = regs_buff[0xd]; /* cable polarity */
        regs_buff[0x13] = 0;  /* Dummy (to align w/ IGP phy reg dump) */
        regs_buff[0x14] = regs_buff[0x11]; /* polarity correction */
        /* phy receive errors */
        regs_buff[0x16] = adapter->phy_stats.receive_errors;
        regs_buff[0x17] = regs_buff[0xd]; /* mdix mode */
        break;

    case iegbe_phy_oem:

        iegbe_oem_get_phy_regs(adapter, &regs_buff[0xd], (0xb));
        break;

    case iegbe_phy_undefined:
    default:

        memset(&regs_buff[0xd], 0, sizeof(uint32_t)*(0xb));
    }
    regs_buff[0x15] = adapter->phy_stats.idle_errors;  /* phy idle errors */
    iegbe_read_phy_reg(hw, PHY_1000T_STATUS, &phy_data);
    regs_buff[0x18] = (uint32_t)phy_data;  /* phy local receiver status */
    regs_buff[0x19] = regs_buff[0x18];  /* phy remote receiver status */
    if(hw->mac_type >= iegbe_82540 &&
       hw->media_type == iegbe_media_type_copper) {
        regs_buff[0x20] = E1000_READ_REG(hw, MANC);
    }
}

static int
iegbe_get_eeprom_len(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    return adapter->hw.eeprom.word_size * 0x2;
}

static int
iegbe_get_eeprom(struct net_device *netdev,
                      struct ethtool_eeprom *eeprom, uint8_t *bytes)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;
    uint16_t *eeprom_buff;
    int first_word, last_word;
    int ret_val = 0;
    uint16_t i;

    if(eeprom->len == 0){
        return -EINVAL;
    }
    eeprom->magic = hw->vendor_id | (hw->device_id << (0x4 * 0x4));

    first_word = eeprom->offset >> 1;
    last_word = (eeprom->offset + eeprom->len - 1) >> 1;

    eeprom_buff = kmalloc(sizeof(uint16_t) *
            (last_word - first_word + 1), GFP_KERNEL);
    if(!eeprom_buff){
        return -ENOMEM;
    }
    if(hw->eeprom.type == iegbe_eeprom_spi){
        ret_val = iegbe_read_eeprom(hw, first_word,
                        last_word - first_word + 1,
                        eeprom_buff);
    }
    else {
        for(i = 0; i < last_word - first_word + 1; i++)
            if((ret_val = iegbe_read_eeprom(hw, first_word + i, 1,
                            &eeprom_buff[i]))){
                break;
            }
    }

    /* Device's eeprom is always little-endian, word addressable */
    for(i = 0; i < last_word - first_word + 1; i++)
        le16_to_cpus(&eeprom_buff[i]);

    memcpy(bytes, (uint8_t *)eeprom_buff + (eeprom->offset & 1),
            eeprom->len);
    kfree(eeprom_buff);

    return ret_val;
}

static int
iegbe_set_eeprom(struct net_device *netdev,
                      struct ethtool_eeprom *eeprom, uint8_t *bytes)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;
    uint16_t *eeprom_buff;
    void *ptr;
    int max_len, first_word, last_word, ret_val = 0;
    uint16_t i;
    uint16_t checksum_reg = hw->mac_type != iegbe_icp_xxxx
                            ? EEPROM_CHECKSUM_REG
                            : EEPROM_CHECKSUM_REG_ICP_xxxx;

    if(eeprom->len == 0){
        return -EOPNOTSUPP;
    }
    if(eeprom->magic != (hw->vendor_id | (hw->device_id << 0x10))){
        return -EFAULT;
    }
    max_len = hw->eeprom.word_size * 0x2;

    first_word = eeprom->offset >> 1;
    last_word = (eeprom->offset + eeprom->len - 1) >> 1;
    eeprom_buff = kmalloc(max_len, GFP_KERNEL);
    if(!eeprom_buff){
        return -ENOMEM;
    }
    ptr = (void *)eeprom_buff;

    if(eeprom->offset & 1) {
        /* need read/modify/write of first changed EEPROM word */
        /* only the second byte of the word is being modified */
        ret_val = iegbe_read_eeprom(hw, first_word, 1,
                        &eeprom_buff[0]);
        ptr++;
    }
    if(((eeprom->offset + eeprom->len) & 1) && (ret_val == 0)) {
        /* need read/modify/write of last changed EEPROM word */
        /* only the first byte of the word is being modified */
        ret_val = iegbe_read_eeprom(hw, last_word, 1,
                          &eeprom_buff[last_word - first_word]);
    }

    /* Device's eeprom is always little-endian, word addressable */
    for(i = 0; i < last_word - first_word + 1; i++)
        le16_to_cpus(&eeprom_buff[i]);

    memcpy(ptr, bytes, eeprom->len);

    for(i = 0; i < last_word - first_word + 1; i++)
        eeprom_buff[i] = cpu_to_le16(eeprom_buff[i]);

    ret_val = iegbe_write_eeprom(hw, first_word,
                     last_word - first_word + 1, eeprom_buff);

    /* Update the checksum over the first part of the EEPROM if needed 
     * and flush shadow RAM for 82573 conrollers */
    if((ret_val == 0) && ((first_word <= checksum_reg) || 
                (hw->mac_type == iegbe_82573))){
        iegbe_update_eeprom_checksum(hw);
    }
    kfree(eeprom_buff);
    return ret_val;
}

static void
iegbe_get_drvinfo(struct net_device *netdev,
                       struct ethtool_drvinfo *drvinfo)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);

    strncpy(drvinfo->driver,  iegbe_driver_name, 0x20);
    strncpy(drvinfo->version, iegbe_driver_version, 0x20);
    strncpy(drvinfo->fw_version, "N/A", 0x20);
    strncpy(drvinfo->bus_info, pci_name(adapter->pdev), 0x20);
    drvinfo->n_stats = E1000_STATS_LEN;
    drvinfo->testinfo_len = E1000_TEST_LEN;
    drvinfo->regdump_len = iegbe_get_regs_len(netdev);
    drvinfo->eedump_len = iegbe_get_eeprom_len(netdev);
}

static void
iegbe_get_ringparam(struct net_device *netdev,
                    struct ethtool_ringparam *ring)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    iegbe_mac_type mac_type = adapter->hw.mac_type;
    struct iegbe_tx_ring *txdr = adapter->tx_ring;
    struct iegbe_rx_ring *rxdr = adapter->rx_ring;

    ring->rx_max_pending = (mac_type < iegbe_82544) ? E1000_MAX_RXD :
        E1000_MAX_82544_RXD;
    ring->tx_max_pending = (mac_type < iegbe_82544) ? E1000_MAX_TXD :
        E1000_MAX_82544_TXD;
    ring->rx_mini_max_pending = 0;
    ring->rx_jumbo_max_pending = 0;
    ring->rx_pending = rxdr->count;
    ring->tx_pending = txdr->count;
    ring->rx_mini_pending = 0;
    ring->rx_jumbo_pending = 0;
}

static int 
iegbe_set_ringparam(struct net_device *netdev,
                    struct ethtool_ringparam *ring)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    iegbe_mac_type mac_type = adapter->hw.mac_type;
    struct iegbe_tx_ring *txdr, *tx_old, *tx_new;
    struct iegbe_rx_ring *rxdr, *rx_old, *rx_new;
    int i, err, tx_ring_size, rx_ring_size;

    tx_ring_size = sizeof(struct iegbe_tx_ring) * adapter->num_tx_queues;
    rx_ring_size = sizeof(struct iegbe_rx_ring) * adapter->num_rx_queues;

    if (netif_running(adapter->netdev)){
        iegbe_down(adapter);
    }
    tx_old = adapter->tx_ring;
    rx_old = adapter->rx_ring;

    adapter->tx_ring = kmalloc(tx_ring_size, GFP_KERNEL);
    if (!adapter->tx_ring) {
        err = -ENOMEM;
        goto err_setup_rx;
    }
    memset(adapter->tx_ring, 0, tx_ring_size);

    adapter->rx_ring = kmalloc(rx_ring_size, GFP_KERNEL);
    if (!adapter->rx_ring) {
        kfree(adapter->tx_ring);
        err = -ENOMEM;
        goto err_setup_rx;
    }
    memset(adapter->rx_ring, 0, rx_ring_size);

    txdr = adapter->tx_ring;
    rxdr = adapter->rx_ring;

    if((ring->rx_mini_pending) || (ring->rx_jumbo_pending)){
        return -EINVAL;
    }
    rxdr->count = max(ring->rx_pending,(uint32_t)E1000_MIN_RXD);
    rxdr->count = min(rxdr->count,(uint32_t)(mac_type < iegbe_82544 ?
        E1000_MAX_RXD : E1000_MAX_82544_RXD));
    E1000_ROUNDUP(rxdr->count, REQ_RX_DESCRIPTOR_MULTIPLE); 

    txdr->count = max(ring->tx_pending,(uint32_t)E1000_MIN_TXD);
    txdr->count = min(txdr->count,(uint32_t)(mac_type < iegbe_82544 ?
        E1000_MAX_TXD : E1000_MAX_82544_TXD));
    E1000_ROUNDUP(txdr->count, REQ_TX_DESCRIPTOR_MULTIPLE); 

	for (i = 0; i < adapter->num_tx_queues; i++)
		txdr[i].count = txdr->count;
	for (i = 0; i < adapter->num_rx_queues; i++)
		rxdr[i].count = rxdr->count;

    if(netif_running(adapter->netdev)) {
        /* Try to get new resources before deleting old */
        if ((err = iegbe_setup_all_rx_resources(adapter))){
            goto err_setup_rx;
        }                
        if ((err = iegbe_setup_all_tx_resources(adapter))){
            goto err_setup_tx;
        }                

        /* save the new, restore the old in order to free it,
         * then restore the new back again */

        rx_new = adapter->rx_ring;
        tx_new = adapter->tx_ring;
        adapter->rx_ring = rx_old;
        adapter->tx_ring = tx_old;
        iegbe_free_all_rx_resources(adapter);
        iegbe_free_all_tx_resources(adapter);
        kfree(tx_old);
        kfree(rx_old);
        adapter->rx_ring = rx_new;
        adapter->tx_ring = tx_new;
        if((err = iegbe_up(adapter))){
            return err;
        }
    }

    return 0;
err_setup_tx:
    iegbe_free_all_rx_resources(adapter);
err_setup_rx:
    adapter->rx_ring = rx_old;
    adapter->tx_ring = tx_old;
    iegbe_up(adapter);
    return err;
}
/* Klockwork complained that the following two macros were longer than 9 
 * lines, hence changed to this ugly formating */
#define REG_PATTERN_TEST(R, M, W)                                            \
{uint32_t pat,value,test[]={0x5A5A5A5A, 0xA5A5A5A5, 0x00000000, 0xFFFFFFFF}; \
 for(pat = 0; pat < sizeof(test)/sizeof(test[0]); pat++) {              \
   E1000_WRITE_REG(&adapter->hw, R, (test[pat] & W));             \
   value = E1000_READ_REG(&adapter->hw, R);                       \
   if(value != (test[pat] & W & M)) {                             \
      DPRINTK(DRV, ERR, "pattern test reg %04X failed: got " \
      "0x%08X expected 0x%08X\n", E1000_##R, value, (test[pat] & W & M)); \
  *data = (adapter->hw.mac_type < iegbe_82543) ? E1000_82542_##R : E1000_##R; \
            return 1; } } }

#define REG_SET_AND_CHECK(R, M, W)                                            \
{   uint32_t value;                                                        \
    E1000_WRITE_REG(&adapter->hw, R, W & M);                               \
    value = E1000_READ_REG(&adapter->hw, R);                               \
    if ((W & M) != (value & M)) {                                          \
        DPRINTK(DRV, ERR, "set/check reg %04X test failed: got 0x%08X "\
                "expected 0x%08X\n", E1000_##R, (value & M), (W & M)); \
        *data = (adapter->hw.mac_type < iegbe_82543) ?                 \
            E1000_82542_##R : E1000_##R;                           \
        return 1;         }  }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
static int
iegbe_reg_test(struct iegbe_adapter *adapter, uint64_t *data)
{
    uint32_t value, before, after;
    uint32_t i, toggle;

    /* The status register is Read Only, so a write should fail.
     * Some bits that get toggled are ignored.
     */
        switch (adapter->hw.mac_type) {
    /* there are several bits on newer hardware that are r/w */
    case iegbe_82571:
    case iegbe_82572:
        toggle = 0x7FFFF3FF;
        break;
    case iegbe_82573:
        toggle = 0x7FFFF033;
        break;
    default:
        toggle = 0xFFFFF833;
        break;
    }

    before = E1000_READ_REG(&adapter->hw, STATUS);
    value = (E1000_READ_REG(&adapter->hw, STATUS) & toggle);
    E1000_WRITE_REG(&adapter->hw, STATUS, toggle);
    after = E1000_READ_REG(&adapter->hw, STATUS) & toggle;
    if(value != after) {
        DPRINTK(DRV, ERR, "failed STATUS register test got: "
                "0x%08X expected: 0x%08X\n", after, value);
        *data = 1;
        return 1;
    }
    /* restore previous status */
    E1000_WRITE_REG(&adapter->hw, STATUS, before);

    REG_PATTERN_TEST(FCAL, 0xFFFFFFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(FCAH, 0x0000FFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(FCT, 0x0000FFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(VET, 0x0000FFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(RDTR, 0x0000FFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(RDBAH, 0xFFFFFFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(RDLEN, 0x000FFF80, 0x000FFFFF);
    REG_PATTERN_TEST(RDH, 0x0000FFFF, 0x0000FFFF);
    REG_PATTERN_TEST(RDT, 0x0000FFFF, 0x0000FFFF);
    REG_PATTERN_TEST(FCRTH, 0x0000FFF8, 0x0000FFF8);
    REG_PATTERN_TEST(FCTTV, 0x0000FFFF, 0x0000FFFF);
    REG_PATTERN_TEST(TIPG, 0x3FFFFFFF, 0x3FFFFFFF);
    REG_PATTERN_TEST(TDBAH, 0xFFFFFFFF, 0xFFFFFFFF);
    REG_PATTERN_TEST(TDLEN, 0x000FFF80, 0x000FFFFF);

    REG_SET_AND_CHECK(RCTL, 0xFFFFFFFF, 0x00000000);
    REG_SET_AND_CHECK(RCTL, 0x06DFB3FE, 0x003FFFFB);
    REG_SET_AND_CHECK(TCTL, 0xFFFFFFFF, 0x00000000);

    if(adapter->hw.mac_type >= iegbe_82543) {

        REG_SET_AND_CHECK(RCTL, 0x06DFB3FE, 0xFFFFFFFF);
        REG_PATTERN_TEST(RDBAL, 0xFFFFFFF0, 0xFFFFFFFF);
        if(adapter->hw.mac_type != iegbe_icp_xxxx){
            REG_PATTERN_TEST(TXCW, 0xC000FFFF, 0x0000FFFF);
        }
        REG_PATTERN_TEST(TDBAL, 0xFFFFFFF0, 0xFFFFFFFF);
        REG_PATTERN_TEST(TIDV, 0x0000FFFF, 0x0000FFFF);

        for(i = 0; i < E1000_RAR_ENTRIES; i++) {
            REG_PATTERN_TEST(RA + ((i << 0x1) << 0x2), 0xFFFFFFFF,
                     0xFFFFFFFF);
            REG_PATTERN_TEST(RA + (((i << 0x1) + 0x1) << 0x2), 0x8003FFFF,
                     0xFFFFFFFF);
        }

    } else {

        REG_SET_AND_CHECK(RCTL, 0xFFFFFFFF, 0x01FFFFFF);
        REG_PATTERN_TEST(RDBAL, 0xFFFFF000, 0xFFFFFFFF);
        REG_PATTERN_TEST(TXCW, 0x0000FFFF, 0x0000FFFF);
        REG_PATTERN_TEST(TDBAL, 0xFFFFF000, 0xFFFFFFFF);

    }

    for(i = 0; i < E1000_MC_TBL_SIZE; i++)
        REG_PATTERN_TEST(MTA + (i << 0x2), 0xFFFFFFFF, 0xFFFFFFFF);

    *data = 0;
    return 0;
}

static int
iegbe_eeprom_test(struct iegbe_adapter *adapter, uint64_t *data)
{
    uint16_t temp;
    uint16_t checksum = 0;
    uint16_t i;
    uint16_t checksum_reg = adapter->hw.mac_type != iegbe_icp_xxxx
                            ? EEPROM_CHECKSUM_REG
                            : EEPROM_CHECKSUM_REG_ICP_xxxx;

    *data = 0;
    /* Read and add up the contents of the EEPROM */
    for(i = 0; i < (checksum_reg + 1); i++) {
        if((iegbe_read_eeprom(&adapter->hw, i, 1, &temp)) < 0) {
            *data = 1;
            break;
        }
        checksum += temp;
    }

    /* If Checksum is not Correct return error else test passed */
    if((checksum != (uint16_t) EEPROM_SUM) && !(*data)){
        *data = 0x2;
    }
    return *data;
}

static irqreturn_t
iegbe_test_intr(int irq,
        void *data)
{
    struct net_device *netdev = (struct net_device *) data;
    struct iegbe_adapter *adapter = netdev_priv(netdev);

    adapter->test_icr |= E1000_READ_REG(&adapter->hw, ICR);

    return IRQ_HANDLED;
}

static int
iegbe_intr_test(struct iegbe_adapter *adapter, uint64_t *data)
{
    struct net_device *netdev = adapter->netdev;
     uint32_t mask, i=0, shared_int = TRUE;
     uint32_t irq = adapter->pdev->irq;

    *data = 0;

    /* Hook up test interrupt handler just for this test */
     if(!request_irq(irq, &iegbe_test_intr, IRQF_PROBE_SHARED, netdev->name,
			netdev)) {
         shared_int = FALSE;
     } else if(request_irq(irq, &iegbe_test_intr, IRQF_SHARED,
                  netdev->name, netdev)){
        *data = 1;
        return -1;
    }

    /* Disable all the interrupts */
    E1000_WRITE_REG(&adapter->hw, IMC, 0xFFFFFFFF);
    msec_delay(0xa);

    /* Test each interrupt */
    for(; i < 0xa; i++) {

        /* Interrupt to test */
        mask = 1 << i;

         if(!shared_int) {
             /* Disable the interrupt to be reported in
              * the cause register and then force the same
              * interrupt and see if one gets posted.  If
              * an interrupt was posted to the bus, the
              * test failed.
              */
             adapter->test_icr = 0;
             E1000_WRITE_REG(&adapter->hw, IMC, mask);
             E1000_WRITE_REG(&adapter->hw, ICS, mask);
             msec_delay(0xa);
 
             if(adapter->test_icr & mask) {
                 *data = 0x3;
                 break;
             }
        }

        /* Enable the interrupt to be reported in
         * the cause register and then force the same
         * interrupt and see if one gets posted.  If
         * an interrupt was not posted to the bus, the
         * test failed.
         */
        adapter->test_icr = 0;
        E1000_WRITE_REG(&adapter->hw, IMS, mask);
        E1000_WRITE_REG(&adapter->hw, ICS, mask);
        msec_delay(0xa);

        if(!(adapter->test_icr & mask)) {
            *data = 0x4;
            break;
        }

         if(!shared_int) {
            /* Disable the other interrupts to be reported in
             * the cause register and then force the other
             * interrupts and see if any get posted.  If
             * an interrupt was posted to the bus, the
             * test failed.
             */
            adapter->test_icr = 0;
            E1000_WRITE_REG(&adapter->hw, IMC, ~mask & 0x00007FFF);
            E1000_WRITE_REG(&adapter->hw, ICS, ~mask & 0x00007FFF);
            msec_delay(0xa);

            if(adapter->test_icr) {
                *data = 0x5;
                break;
            }
        }
    }

    /* Disable all the interrupts */
    E1000_WRITE_REG(&adapter->hw, IMC, 0xFFFFFFFF);
    msec_delay(0xa);

    /* Unhook test interrupt handler */
    free_irq(irq, netdev);

    return *data;
}

static void
iegbe_free_desc_rings(struct iegbe_adapter *adapter)
{
    struct iegbe_tx_ring *txdr = &adapter->test_tx_ring;
    struct iegbe_rx_ring *rxdr = &adapter->test_rx_ring;
    struct pci_dev *pdev = adapter->pdev;
    int i;

    if(txdr->desc && txdr->buffer_info) {
        for(i = 0; i < txdr->count; i++) {
            if(txdr->buffer_info[i].dma){
                pci_unmap_single(pdev, txdr->buffer_info[i].dma,
                         txdr->buffer_info[i].length,
                         PCI_DMA_TODEVICE);
            }
            if(txdr->buffer_info[i].skb){
                dev_kfree_skb(txdr->buffer_info[i].skb);
            }                     
        }
    }

    if(rxdr->desc && rxdr->buffer_info) {
        for(i = 0; i < rxdr->count; i++) {
            if(rxdr->buffer_info[i].dma){
                pci_unmap_single(pdev, rxdr->buffer_info[i].dma,
                         rxdr->buffer_info[i].length,
                         PCI_DMA_FROMDEVICE);
            }                                 
            if(rxdr->buffer_info[i].skb){
                dev_kfree_skb(rxdr->buffer_info[i].skb);
            }
        }
    }

    if(txdr->desc){
        pci_free_consistent(pdev, txdr->size, txdr->desc, txdr->dma);
    }
    if(rxdr->desc){
        pci_free_consistent(pdev, rxdr->size, rxdr->desc, rxdr->dma);
    }
    if(txdr->buffer_info){
        kfree(txdr->buffer_info);
    }
    if(rxdr->buffer_info){
        kfree(rxdr->buffer_info);
    }
    return;
}

static int
iegbe_setup_desc_rings(struct iegbe_adapter *adapter)
{
    struct iegbe_tx_ring *txdr = &adapter->test_tx_ring;
    struct iegbe_rx_ring *rxdr = &adapter->test_rx_ring;
    struct pci_dev *pdev = adapter->pdev;
    uint32_t rctl;
    int size, i, ret_val;

    /* Setup Tx descriptor ring and Tx buffers */

    if(!txdr->count){
        txdr->count = E1000_DEFAULT_TXD;   
    }
    size = txdr->count * sizeof(struct iegbe_buffer);
    if(!(txdr->buffer_info = kmalloc(size, GFP_KERNEL))) {
        ret_val = 1;
        goto err_nomem;
    }
    memset(txdr->buffer_info, 0, size);

    txdr->size = txdr->count * sizeof(struct iegbe_tx_desc);
    E1000_ROUNDUP(txdr->size, 0x1000);
    if(!(txdr->desc = pci_alloc_consistent(pdev, txdr->size, &txdr->dma))) {
        ret_val = 0x2;
        goto err_nomem;
    }
    memset(txdr->desc, 0, txdr->size);
    txdr->next_to_use = txdr->next_to_clean = 0;

    E1000_WRITE_REG(&adapter->hw, TDBAL,
            ((uint64_t) txdr->dma & 0x00000000FFFFFFFF));
    E1000_WRITE_REG(&adapter->hw, TDBAH, ((uint64_t) txdr->dma >> 0x20));
    E1000_WRITE_REG(&adapter->hw, TDLEN,
            txdr->count * sizeof(struct iegbe_tx_desc));
    E1000_WRITE_REG(&adapter->hw, TDH, 0);
    E1000_WRITE_REG(&adapter->hw, TDT, 0);
    E1000_WRITE_REG(&adapter->hw, TCTL,
            E1000_TCTL_PSP | E1000_TCTL_EN |
            E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT |
            E1000_FDX_COLLISION_DISTANCE << E1000_COLD_SHIFT);

    for(i = 0; i < txdr->count; i++) {
        struct iegbe_tx_desc *tx_desc = E1000_TX_DESC(*txdr, i);
        struct sk_buff *skb;
        unsigned int size = 0x400;

        if(!(skb = alloc_skb(size, GFP_KERNEL))) {
            ret_val = 0x3;
            goto err_nomem;
        }
        skb_put(skb, size);
        txdr->buffer_info[i].skb = skb;
        txdr->buffer_info[i].length = skb->len;
        txdr->buffer_info[i].dma =
            pci_map_single(pdev, skb->data, skb->len,
                       PCI_DMA_TODEVICE);
        tx_desc->buffer_addr = cpu_to_le64(txdr->buffer_info[i].dma);
        tx_desc->lower.data = cpu_to_le32(skb->len);
        tx_desc->lower.data |= cpu_to_le32(E1000_TXD_CMD_EOP |
                           E1000_TXD_CMD_IFCS |
                           E1000_TXD_CMD_RPS);
        tx_desc->upper.data = 0;
    }

    /* Setup Rx descriptor ring and Rx buffers */

    if(!rxdr->count){
        rxdr->count = E1000_DEFAULT_RXD;   
    }
    size = rxdr->count * sizeof(struct iegbe_buffer);
    if(!(rxdr->buffer_info = kmalloc(size, GFP_KERNEL))) {
        ret_val = 0x4;
        goto err_nomem;
    }
    memset(rxdr->buffer_info, 0, size);

    rxdr->size = rxdr->count * sizeof(struct iegbe_rx_desc);
    if(!(rxdr->desc = pci_alloc_consistent(pdev, rxdr->size, &rxdr->dma))) {
        ret_val = 0x5;
        goto err_nomem;
    }
    memset(rxdr->desc, 0, rxdr->size);
    rxdr->next_to_use = rxdr->next_to_clean = 0;

    rctl = E1000_READ_REG(&adapter->hw, RCTL);
    E1000_WRITE_REG(&adapter->hw, RCTL, rctl & ~E1000_RCTL_EN);
    E1000_WRITE_REG(&adapter->hw, RDBAL,
            ((uint64_t) rxdr->dma & 0xFFFFFFFF));
    E1000_WRITE_REG(&adapter->hw, RDBAH, ((uint64_t) rxdr->dma >> 0x20));
    E1000_WRITE_REG(&adapter->hw, RDLEN, rxdr->size);
    E1000_WRITE_REG(&adapter->hw, RDH, 0);
    E1000_WRITE_REG(&adapter->hw, RDT, 0);
    rctl = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SZ_2048 |
        E1000_RCTL_LBM_NO | E1000_RCTL_RDMTS_HALF |
        (adapter->hw.mc_filter_type << E1000_RCTL_MO_SHIFT);
    E1000_WRITE_REG(&adapter->hw, RCTL, rctl);

    for(i = 0; i < rxdr->count; i++) {
        struct iegbe_rx_desc *rx_desc = E1000_RX_DESC(*rxdr, i);
        struct sk_buff *skb;

        if(!(skb = alloc_skb(E1000_RXBUFFER_2048 + NET_IP_ALIGN,
                GFP_KERNEL))) {
            ret_val = 0x6;
            goto err_nomem;
        }
        skb_reserve(skb, NET_IP_ALIGN);
        rxdr->buffer_info[i].skb = skb;
        rxdr->buffer_info[i].length = E1000_RXBUFFER_2048;
        rxdr->buffer_info[i].dma =
            pci_map_single(pdev, skb->data, E1000_RXBUFFER_2048,
                       PCI_DMA_FROMDEVICE);
        rx_desc->buffer_addr = cpu_to_le64(rxdr->buffer_info[i].dma);
        memset(skb->data, 0x00, skb->len);
    }

    return 0;

err_nomem:
    iegbe_free_desc_rings(adapter);
    return ret_val;
}

static void
iegbe_phy_disable_receiver(struct iegbe_adapter *adapter)
{
    /* Write out to PHY registers 29 and 30 to disable the Receiver. */
    iegbe_write_phy_reg(&adapter->hw, 0x1d, 0x001F);
    iegbe_write_phy_reg(&adapter->hw, 0x1e, 0x8FFC);
    iegbe_write_phy_reg(&adapter->hw, 0x1d, 0x001A);
    iegbe_write_phy_reg(&adapter->hw, 0x1e, 0x8FF0);
}

static void
iegbe_phy_reset_clk_and_crs(struct iegbe_adapter *adapter)
{
    uint16_t phy_reg;

    /* Because we reset the PHY above, we need to re-force TX_CLK in the
     * Extended PHY Specific Control Register to 25MHz clock.  This
     * value defaults back to a 2.5MHz clock when the PHY is reset.
     */
    iegbe_read_phy_reg(&adapter->hw, M88E1000_EXT_PHY_SPEC_CTRL, &phy_reg);
    phy_reg |= M88E1000_EPSCR_TX_CLK_25;
    iegbe_write_phy_reg(&adapter->hw,
        M88E1000_EXT_PHY_SPEC_CTRL, phy_reg);

    /* In addition, because of the s/w reset above, we need to enable
     * CRS on TX.  This must be set for both full and half duplex
     * operation.
     */
    iegbe_read_phy_reg(&adapter->hw, M88E1000_PHY_SPEC_CTRL, &phy_reg);
    phy_reg &= ~M88E1000_PSCR_ASSERT_CRS_ON_TX;
    iegbe_write_phy_reg(&adapter->hw,
        M88E1000_PHY_SPEC_CTRL, phy_reg);
}

static int
iegbe_nonintegrated_phy_loopback(struct iegbe_adapter *adapter)
{
    uint32_t ctrl_reg;
    uint16_t phy_reg;

    /* Setup the Device Control Register for PHY loopback test. */

    ctrl_reg = E1000_READ_REG(&adapter->hw, CTRL);
    ctrl_reg |= (E1000_CTRL_ILOS |        /* Invert Loss-Of-Signal */
             E1000_CTRL_FRCSPD |    /* Set the Force Speed Bit */
             E1000_CTRL_FRCDPX |    /* Set the Force Duplex Bit */
             E1000_CTRL_SPD_1000 |    /* Force Speed to 1000 */
             E1000_CTRL_FD);        /* Force Duplex to FULL */

    E1000_WRITE_REG(&adapter->hw, CTRL, ctrl_reg);

    /* Read the PHY Specific Control Register (0x10) */
    iegbe_read_phy_reg(&adapter->hw, M88E1000_PHY_SPEC_CTRL, &phy_reg);

    /* Clear Auto-Crossover bits in PHY Specific Control Register
     * (bits 6:5).
     */
    phy_reg &= ~M88E1000_PSCR_AUTO_X_MODE;
    iegbe_write_phy_reg(&adapter->hw, M88E1000_PHY_SPEC_CTRL, phy_reg);

    /* Perform software reset on the PHY */
    iegbe_phy_reset(&adapter->hw);

    /* Have to setup TX_CLK and TX_CRS after software reset */
    iegbe_phy_reset_clk_and_crs(adapter);

    iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, 0x8100);

    /* Wait for reset to complete. */
    usec_delay(0x1f4);

    /* Have to setup TX_CLK and TX_CRS after software reset */
    iegbe_phy_reset_clk_and_crs(adapter);

    /* Write out to PHY registers 29 and 30 to disable the Receiver. */
    iegbe_phy_disable_receiver(adapter);

    /* Set the loopback bit in the PHY control register. */
    iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_reg);
    phy_reg |= MII_CR_LOOPBACK;
    iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, phy_reg);

    /* Setup TX_CLK and TX_CRS one more time. */
    iegbe_phy_reset_clk_and_crs(adapter);

    /* Check Phy Configuration */
    iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_reg);
    if(phy_reg != 0x4100){
         return 0x9;
    }
    iegbe_read_phy_reg(&adapter->hw, M88E1000_EXT_PHY_SPEC_CTRL, &phy_reg);
    if(phy_reg != 0x0070){
        return 0xa;
    }
    iegbe_read_phy_reg(&adapter->hw, 0x1d, &phy_reg);
    if(phy_reg != 0x001A){
        return 0xb;
    }
    return 0;
}

static int
iegbe_integrated_phy_loopback(struct iegbe_adapter *adapter)
{
    uint32_t ctrl_reg = 0;
    uint32_t stat_reg = 0;

    adapter->hw.autoneg = FALSE;

    if(adapter->hw.phy_type == iegbe_phy_m88) {
        /* Auto-MDI/MDIX Off */
        iegbe_write_phy_reg(&adapter->hw,
                    M88E1000_PHY_SPEC_CTRL, 0x0808);
        /* reset to update Auto-MDI/MDIX */
        iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, 0x9140);
        /* autoneg off */
        iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, 0x8140);
    }
    /* force 1000, set loopback */
    iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, 0x4140);

    /* Now set up the MAC to the same speed/duplex as the PHY. */
    ctrl_reg = E1000_READ_REG(&adapter->hw, CTRL);
    ctrl_reg &= ~E1000_CTRL_SPD_SEL; /* Clear the speed sel bits */
    ctrl_reg |= (E1000_CTRL_FRCSPD | /* Set the Force Speed Bit */
             E1000_CTRL_FRCDPX | /* Set the Force Duplex Bit */
             E1000_CTRL_SPD_1000 |/* Force Speed to 1000 */
             E1000_CTRL_FD);     /* Force Duplex to FULL */

    if(adapter->hw.media_type == iegbe_media_type_copper &&
       adapter->hw.phy_type == iegbe_phy_m88) {
        ctrl_reg |= E1000_CTRL_ILOS; /* Invert Loss of Signal */
    } else {
        /* Set the ILOS bit on the fiber Nic is half
         * duplex link is detected. */
        stat_reg = E1000_READ_REG(&adapter->hw, STATUS);
        if((stat_reg & E1000_STATUS_FD) == 0){
            ctrl_reg |= (E1000_CTRL_ILOS | E1000_CTRL_SLU);
        }
    }

    E1000_WRITE_REG(&adapter->hw, CTRL, ctrl_reg);

    /* Disable the receiver on the PHY so when a cable is plugged in, the
     * PHY does not begin to autoneg when a cable is reconnected to the NIC.
     */
    if(adapter->hw.phy_type == iegbe_phy_m88){
        iegbe_phy_disable_receiver(adapter);
    }
    usec_delay(0x1f4);

    return 0;
}

static int
iegbe_set_phy_loopback(struct iegbe_adapter *adapter)
{
    uint16_t phy_reg = 0;
    uint16_t count = 0;

    switch (adapter->hw.mac_type) {
    case iegbe_82543:
        if(adapter->hw.media_type == iegbe_media_type_copper) {
            /* Attempt to setup Loopback mode on Non-integrated PHY.
             * Some PHY registers get corrupted at random, so
             * attempt this 10 times.
             */
            while(iegbe_nonintegrated_phy_loopback(adapter) &&
                  count++ < 0xa);
            if(count < 0xb) {
                return 0;
            }                     
        }
        break;

    case iegbe_82544:
    case iegbe_82540:
    case iegbe_82545:
    case iegbe_82545_rev_3:
        return iegbe_integrated_phy_loopback(adapter);
        break;        

    case iegbe_icp_xxxx:
        return iegbe_oem_phy_loopback(adapter);
        break;

    case iegbe_82546:
    case iegbe_82546_rev_3:
    case iegbe_82541:
    case iegbe_82541_rev_2:
    case iegbe_82547:
    case iegbe_82547_rev_2:
    case iegbe_82571:
    case iegbe_82572:
    case iegbe_82573:
        return iegbe_integrated_phy_loopback(adapter);
        break;

    default:
        /* Default PHY loopback work is to read the MII
         * control register and assert bit 14 (loopback mode).
         */
        iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_reg);
        phy_reg |= MII_CR_LOOPBACK;
        iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, phy_reg);
        return 0;
        break;
    }

    return 0x8;
}

static int
iegbe_setup_loopback_test(struct iegbe_adapter *adapter)
{
    uint32_t rctl;

    if(adapter->hw.media_type == iegbe_media_type_fiber ||
       adapter->hw.media_type == iegbe_media_type_internal_serdes) {
        if(adapter->hw.mac_type == iegbe_82545 ||
           adapter->hw.mac_type == iegbe_82546 ||
           adapter->hw.mac_type == iegbe_82545_rev_3 ||
           adapter->hw.mac_type == iegbe_82546_rev_3){
            return iegbe_set_phy_loopback(adapter);
           }                
        else {
            rctl = E1000_READ_REG(&adapter->hw, RCTL);
            rctl |= E1000_RCTL_LBM_TCVR;
            E1000_WRITE_REG(&adapter->hw, RCTL, rctl);
            return 0;
        }
    } else if(adapter->hw.media_type == iegbe_media_type_copper
              || (adapter->hw.media_type == iegbe_media_type_oem
          && iegbe_oem_phy_is_copper(&adapter->hw))){
        return iegbe_set_phy_loopback(adapter);
        }
    return 0x7;
}

static void
iegbe_loopback_cleanup(struct iegbe_adapter *adapter)
{
    uint32_t rctl;
    uint16_t phy_reg;

    rctl = E1000_READ_REG(&adapter->hw, RCTL);
    rctl &= ~(E1000_RCTL_LBM_TCVR | E1000_RCTL_LBM_MAC);
    E1000_WRITE_REG(&adapter->hw, RCTL, rctl);

    if(adapter->hw.media_type == iegbe_media_type_copper ||
       ((adapter->hw.media_type == iegbe_media_type_fiber ||
         adapter->hw.media_type == iegbe_media_type_internal_serdes) &&
        (adapter->hw.mac_type == iegbe_82545 ||
         adapter->hw.mac_type == iegbe_82546 ||
         adapter->hw.mac_type == iegbe_82545_rev_3 ||
         adapter->hw.mac_type == iegbe_82546_rev_3))) {
        adapter->hw.autoneg = TRUE;
        iegbe_read_phy_reg(&adapter->hw, PHY_CTRL, &phy_reg);
        if(phy_reg & MII_CR_LOOPBACK) {
            phy_reg &= ~MII_CR_LOOPBACK;
            iegbe_write_phy_reg(&adapter->hw, PHY_CTRL, phy_reg);
            iegbe_phy_reset(&adapter->hw);
        }
    }

    if(adapter->hw.media_type == iegbe_media_type_oem){
        iegbe_oem_loopback_cleanup(adapter);
    }
}

static void
iegbe_create_lbtest_frame(struct sk_buff *skb, unsigned int frame_size)
{
    memset(skb->data, 0xFF, frame_size);
    frame_size = (frame_size % 0x2) ? (frame_size - 1) : frame_size;
    memset(&skb->data[frame_size / 0x2], 0xAA, frame_size / 0x2 - 1);
    memset(&skb->data[frame_size / 0x2 + 0xa], 0xBE, 1);
    memset(&skb->data[frame_size / 0x2 + 0xc], 0xAF, 1);
}

static int
iegbe_check_lbtest_frame(struct sk_buff *skb, unsigned int frame_size)
{
    frame_size = (frame_size % 0x2) ? (frame_size - 1) : frame_size;
    if(*(skb->data + 0x3) == 0xFF) {
        if((*(skb->data + frame_size / 0xc) == 0xBE) &&
           (*(skb->data + frame_size / 0xe) == 0xAF)) {
            return 0;
        }
    }
    return 0xd;
}

static int
iegbe_run_loopback_test(struct iegbe_adapter *adapter)
{
    struct iegbe_tx_ring *txdr = &adapter->test_tx_ring;
    struct iegbe_rx_ring *rxdr = &adapter->test_rx_ring;
    struct pci_dev *pdev = adapter->pdev;
    int i, j, k, l, lc, good_cnt, ret_val=0;
    unsigned long time;

    E1000_WRITE_REG(&adapter->hw, RDT, rxdr->count - 1);

    /* Calculate the loop count based on the largest descriptor ring 
     * The idea is to wrap the largest ring a number of times using 64
     * send/receive pairs during each loop
     */

    if(rxdr->count <= txdr->count){
        lc = ((txdr->count / 0x40) * 0x2) + 1;
    }  
    else{
        lc = ((rxdr->count / 0x40) * 0x2) + 1;
    }
    k = l = 0;
    for(j = 0; j <= lc; j++) { /* loop count loop */
        for(i = 0; i < 0x40; i++) { /* send the packets */
            iegbe_create_lbtest_frame(txdr->buffer_info[i].skb, 
                    0x400);
            pci_dma_sync_single_for_device(pdev, 
                    txdr->buffer_info[k].dma,
                        txdr->buffer_info[k].length,
                        PCI_DMA_TODEVICE);
            if(unlikely(++k == txdr->count)) k = 0;
        }
        E1000_WRITE_REG(&adapter->hw, TDT, k);
        msec_delay(0xc8);
        time = jiffies; /* set the start time for the receive */
        good_cnt = 0;
        do { /* receive the sent packets */
            pci_dma_sync_single_for_cpu(pdev, 
                    rxdr->buffer_info[l].dma,
                        rxdr->buffer_info[l].length,
                        PCI_DMA_FROMDEVICE);
    
            ret_val = iegbe_check_lbtest_frame(
                    rxdr->buffer_info[l].skb,
                       0x400);
            if(!ret_val){
                good_cnt++;
            }     
            if(unlikely(++l == rxdr->count)) l = 0;
            /* time + 20 msecs (200 msecs on 2.4) is more than 
             * enough time to complete the receives, if it's 
             * exceeded, break and error off
             */
        } while(good_cnt < (0x40) && jiffies < (time + (0x14)));
        if(good_cnt != (0x40)) {
            ret_val = (0xd); /* ret_val is the same as mis-compare */
            break; 
        }
        if(jiffies >= (time + 0x2)) {
            ret_val = (0x1e); /* error code for time out error */
            break;
        }
    } /* end loop count loop */
    return ret_val;
}

static int
iegbe_loopback_test(struct iegbe_adapter *adapter, uint64_t *data)
{

    /* PHY loopback cannot be performed if SoL/IDER
     * sessions are active */
    if (iegbe_check_phy_reset_block(&adapter->hw)) {
        DPRINTK(DRV, ERR, "Cannot do PHY loopback test "
                "when SoL/IDER is active.\n");
        *data = 0;
        goto out;
    }

    if ((*data = iegbe_setup_desc_rings(adapter))){
        goto out;
    }
    if ((*data = iegbe_setup_loopback_test(adapter))){
        goto err_loopback;
    }
    *data = iegbe_run_loopback_test(adapter);
    iegbe_loopback_cleanup(adapter);

err_loopback:
    iegbe_free_desc_rings(adapter);
out:
    return *data;


}

static int
iegbe_link_test(struct iegbe_adapter *adapter, uint64_t *data)
{
    *data = 0;
    if(adapter->hw.media_type == iegbe_media_type_internal_serdes) {
        int i = 0;
        adapter->hw.serdes_link_down = TRUE;

        /* On some blade server designs, link establishment
         * could take as long as 2-3 minutes */
        do {
            iegbe_check_for_link(&adapter->hw);
            if(adapter->hw.serdes_link_down == FALSE){
                return *data;
            }
            msec_delay(0x14);
        } while(i++ < 0xea6);

        *data = 1;
    } else {
        iegbe_check_for_link(&adapter->hw);
        if(adapter->hw.autoneg) {  /* if auto_neg is set wait for it */
            msec_delay(0xfa0);
        }
        /*
         * ICP_XXXX style MAC's do not have a link up bit int the STATUS
         * register, so query the PHY directly
         */
        if(adapter->hw.mac_type != iegbe_icp_xxxx) {
            if(!(E1000_READ_REG(&adapter->hw, STATUS) & E1000_STATUS_LU)) {
                *data = 1;
            }
        } else {
            int isUp = 0;
            if(iegbe_oem_phy_is_link_up(&adapter->hw, &isUp) != E1000_SUCCESS)
            {
                printk("unable to determine Link Status!\n");
            }
            else
            {
                if(isUp){
                    *data = 0;
                }
                else{
                    *data = 1;
                }
            }    
        }
    }
    return *data;
}

static int 
iegbe_diag_test_count(struct net_device *netdev)
{
    return E1000_TEST_LEN;
}

static void
iegbe_diag_test(struct net_device *netdev,
           struct ethtool_test *eth_test, uint64_t *data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    boolean_t if_running = netif_running(netdev);

    if(eth_test->flags == ETH_TEST_FL_OFFLINE) {
        /* Offline tests */

        /* save speed, duplex, autoneg settings */
        uint16_t autoneg_advertised = adapter->hw.autoneg_advertised;
        uint8_t forced_speed_duplex = adapter->hw.forced_speed_duplex;
        uint8_t autoneg = adapter->hw.autoneg;

        /* Link test performed before hardware reset so autoneg doesn't
         * interfere with test result */
        if(iegbe_link_test(adapter, &data[0x4])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        if(if_running){
            iegbe_down(adapter);
        }
        else{
            iegbe_reset(adapter);
        }
        if(iegbe_reg_test(adapter, &data[0x0])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        iegbe_reset(adapter);
        if(iegbe_eeprom_test(adapter, &data[0x1])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        iegbe_reset(adapter);
        if(iegbe_intr_test(adapter, &data[0x2])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        iegbe_reset(adapter);
        if(iegbe_loopback_test(adapter, &data[0x3])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        /* restore speed, duplex, autoneg settings */
        adapter->hw.autoneg_advertised = autoneg_advertised;
        adapter->hw.forced_speed_duplex = forced_speed_duplex;
        adapter->hw.autoneg = autoneg;

        iegbe_reset(adapter);
        if(if_running){
            iegbe_up(adapter);
        }
    } else {
        /* Online tests */
        if(iegbe_link_test(adapter, &data[0x4])){
            eth_test->flags |= ETH_TEST_FL_FAILED;
        }
        /* Offline tests aren't run; pass by default */
        data[0] = 0;
        data[0x1] = 0;
        data[0x2] = 0;
        data[0x3] = 0;
    }
    msleep_interruptible(0xfa0);
}
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)) */

static void
iegbe_get_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;

    switch(adapter->hw.device_id) {
    case E1000_DEV_ID_82542:
    case E1000_DEV_ID_82543GC_FIBER:
    case E1000_DEV_ID_82543GC_COPPER:
    case E1000_DEV_ID_82544EI_FIBER:
    case E1000_DEV_ID_82546EB_QUAD_COPPER:
    case E1000_DEV_ID_82545EM_FIBER:
    case E1000_DEV_ID_82545EM_COPPER:
        wol->supported = 0;
        wol->wolopts   = 0;
        return;

    case E1000_DEV_ID_82546EB_FIBER:
    case E1000_DEV_ID_82546GB_FIBER:
        /* Wake events only supported on port A for dual fiber */
        if(E1000_READ_REG(hw, STATUS) & E1000_STATUS_FUNC_1) {
            wol->supported = 0;
            wol->wolopts   = 0;
            return;
        }
        /* Fall Through */

    default:
        wol->supported = WAKE_UCAST | WAKE_MCAST |
                 WAKE_BCAST | WAKE_MAGIC;

        wol->wolopts = 0;
        if(adapter->wol & E1000_WUFC_EX){
            wol->wolopts |= WAKE_UCAST;
        }
        if(adapter->wol & E1000_WUFC_MC){
            wol->wolopts |= WAKE_MCAST;
        }
        if(adapter->wol & E1000_WUFC_BC){
            wol->wolopts |= WAKE_BCAST;
        }
        if(adapter->wol & E1000_WUFC_MAG){
            wol->wolopts |= WAKE_MAGIC;
        }
        return;
    }
}

static int
iegbe_set_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    struct iegbe_hw *hw = &adapter->hw;

    switch(adapter->hw.device_id) {
    case E1000_DEV_ID_82542:
    case E1000_DEV_ID_82543GC_FIBER:
    case E1000_DEV_ID_82543GC_COPPER:
    case E1000_DEV_ID_82544EI_FIBER:
    case E1000_DEV_ID_82546EB_QUAD_COPPER:
    case E1000_DEV_ID_82545EM_FIBER:
    case E1000_DEV_ID_82545EM_COPPER:
        return wol->wolopts ? -EOPNOTSUPP : 0;

    case E1000_DEV_ID_82546EB_FIBER:
    case E1000_DEV_ID_82546GB_FIBER:
        /* Wake events only supported on port A for dual fiber */
        if(E1000_READ_REG(hw, STATUS) & E1000_STATUS_FUNC_1){
            return wol->wolopts ? -EOPNOTSUPP : 0;
        }/* Fall Through */

    default:
        if(wol->wolopts & (WAKE_PHY | WAKE_ARP | WAKE_MAGICSECURE)){
            return -EOPNOTSUPP;
        }
        adapter->wol = 0;

        if(wol->wolopts & WAKE_UCAST){
            adapter->wol |= E1000_WUFC_EX;
        }
        if(wol->wolopts & WAKE_MCAST){
            adapter->wol |= E1000_WUFC_MC;
        }
        if(wol->wolopts & WAKE_BCAST){
            adapter->wol |= E1000_WUFC_BC;
        }
        if(wol->wolopts & WAKE_MAGIC){
            adapter->wol |= E1000_WUFC_MAG;
        }
    }

    return 0;
}

/* toggle LED 4 times per second = 2 "blinks" per second */
#define E1000_ID_INTERVAL    (HZ/0x4)

/* bit defines for adapter->led_status */
#define E1000_LED_ON        0

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
static void
iegbe_led_blink_callback(unsigned long data)
{
    struct iegbe_adapter *adapter = (struct iegbe_adapter *) data;

    if(test_and_change_bit(E1000_LED_ON, &adapter->led_status)){
        iegbe_led_off(&adapter->hw);
    }
    else{
        iegbe_led_on(&adapter->hw);
    }
    mod_timer(&adapter->blink_timer, jiffies + E1000_ID_INTERVAL);
}

static int
iegbe_phys_id(struct net_device *netdev, uint32_t data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);

    if(adapter->hw.mac_type == iegbe_icp_xxxx){
        /* No LED control on ICP family of gigE controllers */
        return 0;
    }
    if(!data || data > (uint32_t)(MAX_SCHEDULE_TIMEOUT / HZ)){
        data = (uint32_t)(MAX_SCHEDULE_TIMEOUT / HZ);
    }
    if(adapter->hw.mac_type < iegbe_82571) {
        if(!adapter->blink_timer.function) {
            init_timer(&adapter->blink_timer);
            adapter->blink_timer.function = iegbe_led_blink_callback;
            adapter->blink_timer.data = (unsigned long) adapter;
        }
        iegbe_setup_led(&adapter->hw);
        mod_timer(&adapter->blink_timer, jiffies);
        msleep_interruptible(data * 0x3e8);
        del_timer_sync(&adapter->blink_timer);
    }
    else {
        E1000_WRITE_REG(&adapter->hw, LEDCTL, (E1000_LEDCTL_LED2_BLINK_RATE |
            E1000_LEDCTL_LED1_BLINK | E1000_LEDCTL_LED2_BLINK | 
            (E1000_LEDCTL_MODE_LED_ON << E1000_LEDCTL_LED2_MODE_SHIFT) |
            (E1000_LEDCTL_MODE_LINK_ACTIVITY << E1000_LEDCTL_LED1_MODE_SHIFT) |
            (E1000_LEDCTL_MODE_LED_OFF << E1000_LEDCTL_LED0_MODE_SHIFT)));
        msleep_interruptible(data * 0x3e8);
    }

    iegbe_led_off(&adapter->hw);
    clear_bit(E1000_LED_ON, &adapter->led_status);
    iegbe_cleanup_led(&adapter->hw);

    return 0;
}
#endif

static int
iegbe_nway_reset(struct net_device *netdev)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    if(netif_running(netdev)) {
        iegbe_down(adapter);
        iegbe_up(adapter);
    }
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
static int 
iegbe_get_stats_count(struct net_device *netdev)
{
    return E1000_STATS_LEN;
}
#endif

static void 
iegbe_get_ethtool_stats(struct net_device *netdev, 
        struct ethtool_stats *stats, uint64_t *data)
{
    struct iegbe_adapter *adapter = netdev_priv(netdev);
    int i;

    iegbe_update_stats(adapter);
    for(i = 0; i < E1000_STATS_LEN; i++) {
        char *p = (char *)adapter+iegbe_gstrings_stats[i].stat_offset;    
        data[i] = (iegbe_gstrings_stats[i].sizeof_stat == 
            sizeof(uint64_t)) ? *(uint64_t *)p : *(uint32_t *)p;
    }
}

static void 
iegbe_get_strings(struct net_device *netdev, uint32_t stringset, uint8_t *data)
{
    int i;

    switch(stringset) {
    case ETH_SS_TEST:
        memcpy(data, *iegbe_gstrings_test, 
            E1000_TEST_LEN*ETH_GSTRING_LEN);
        break;
    case ETH_SS_STATS:
        for(i=0; i < E1000_STATS_LEN; i++) {
            memcpy(data + i * ETH_GSTRING_LEN, 
            iegbe_gstrings_stats[i].stat_string,
            ETH_GSTRING_LEN);
        }
        break;
    }
}

struct ethtool_ops iegbe_ethtool_ops = {
    .get_settings           = iegbe_get_settings,
    .set_settings           = iegbe_set_settings,
    .get_drvinfo            = iegbe_get_drvinfo,
    .get_regs_len           = iegbe_get_regs_len,
    .get_regs               = iegbe_get_regs,
    .get_wol                = iegbe_get_wol,
    .set_wol                = iegbe_set_wol,
    .get_msglevel            = iegbe_get_msglevel,
    .set_msglevel            = iegbe_set_msglevel,
    .nway_reset             = iegbe_nway_reset,
    .get_link               = ethtool_op_get_link,
    .get_eeprom_len         = iegbe_get_eeprom_len,
    .get_eeprom             = iegbe_get_eeprom,
    .set_eeprom             = iegbe_set_eeprom,
    .get_ringparam          = iegbe_get_ringparam,
    .set_ringparam          = iegbe_set_ringparam,
    .get_pauseparam        = iegbe_get_pauseparam,
    .set_pauseparam        = iegbe_set_pauseparam,

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
    .get_rx_csum        = iegbe_get_rx_csum,
    .set_rx_csum        = iegbe_set_rx_csum,
    .get_tx_csum            = iegbe_get_tx_csum,
    .set_tx_csum        = iegbe_set_tx_csum,
    .get_sg            = ethtool_op_get_sg,
    .set_sg            = ethtool_op_set_sg,
#ifdef NETIF_F_TSO
    .get_tso        = ethtool_op_get_tso,
    .set_tso        = iegbe_set_tso,
#endif
//    .self_test_count        = iegbe_diag_test_count,
    .self_test              = iegbe_diag_test,
    .set_phys_id                = iegbe_phys_id,
    .get_stats_count        = iegbe_get_stats_count,
#endif    /* SIOCETHTOOL */
    .get_strings            = iegbe_get_strings,
    .get_ethtool_stats      = iegbe_get_ethtool_stats,
};

void set_ethtool_ops(struct net_device *netdev)
{
    SET_ETHTOOL_OPS(netdev, &iegbe_ethtool_ops);
}
