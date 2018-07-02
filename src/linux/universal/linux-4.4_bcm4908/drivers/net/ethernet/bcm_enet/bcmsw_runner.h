/*
    Copyright 2004-2010 Broadcom Corporation

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

#ifndef _BCMSW_RUNNER_H_
#define _BCMSW_RUNNER_H_

#include <bcm/bcmswapitypes.h>
#include <bcm/bcmswapistat.h>
#include "bcmenet_common.h"

#define bcmeapi_ioctl_ethsw_control(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_prio_control(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_vlan(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_pmdioaccess(dev, e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cosq_txchannel_mapping(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cosq_sched(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_spiaccess(bus_num, spi_id, chip_id, e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cosq_config(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_jumbo_control(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_default_tag_config(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_clear_stats(portmap) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cosq_txq_sel(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_counter_get(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cosq_rxchannel_mapping(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_info(dev, e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_regaccess(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_pbvlan(e) -(EOPNOTSUPP)

#define bcmeapi_ioctl_debug_conf(e)
#define bcmeapi_ethsw_dump_page(page) {}
#define bcmeapi_ioctl_ethsw_cosq_port_mapping(e) -(EOPNOTSUPP)
#define enet_arl_read( mac, vid, val ) FALSE
#define enet_arl_write(mac, vid, val) {}
#define enet_arl_dump() {}  /* This should return status actually ?? */
#define enet_arl_dump_multiport_arl() {}
#define fast_age_all(age_static) {}
void bcmeapi_ethsw_init_hw(int unit, uint32_t portMap,  int wanPortMap);

#define bcmsw_spi_rreg(bus_num, spi_ss, chip_id, page, reg, data, len) {}
#define bcmsw_spi_wreg(bus_num, spi_ss, chip_id, page, reg, data, len) {}
static inline int enet_arl_remove(char *mac) {return 0;}
#define bcmeapi_set_multiport_address(addr) {}
int bcmeapi_init_ext_sw_if(extsw_info_t *extSwInfo);
int bcmeapi_ioctl_ethsw_clear_port_emac(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_get_port_emac(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_clear_port_stats(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_get_port_stats(struct ethswctl_data *e);
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
#define bcmeapi_ioctl_que_map(pDevCtrl, e) 0
#define bcmeapi_ioctl_que_mon(pDevCtrl, e) 0
int bcmeapi_ioctl_ethsw_port_pause_capability(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_traffic_control(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_port_loopback(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_phy_autoneg_info(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_phy_autoneg_cap_adv_set(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_link_status(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_port_transparent_set (struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_vlanlist_set (struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_vlan_isolation_set (struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_irc_set(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_irc_get(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_cpu_meter_set(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_phy_mode(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_port_erc_set(struct ethswctl_data *e);
#define bcmeapi_ioctl_ethsw_combophy_mode(ethswctl_data, phy_id, phy_id_ext) 0
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
int bcmeapi_ioctl_que_map(BcmEnet_devctrl *pDevCtrl, struct ethswctl_data *e);
int bcmeapi_ioctl_que_mon(BcmEnet_devctrl *pDevCtrl, struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_pause_capability(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_phy_mode(struct ethswctl_data *e, int phy_id);
int bcmeapi_ioctl_ethsw_combophy_mode(struct ethswctl_data *e, int phy_id, int phy_id_ext);
int bcmsw_unimac_rxtx_op(int port, int get, int *disable);
int bcmeapi_ioctl_ethsw_port_irc_get(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_port_irc_set(struct ethswctl_data *e);
#define bcmeapi_ioctl_ethsw_port_erc_get(ethswctl_data) (-EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_erc_set(ethswctl_data) (-EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_loopback(e, phy_id) (-EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cpu_meter_set(e) (-EOPNOTSUPP)
#else
#define bcmeapi_ioctl_que_map(pDevCtrl, e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_que_mon(pDevCtrl, e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_pause_capability(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_traffic_control(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_loopback(e, phy_id) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_irc_set(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_irc_get(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_cpu_meter_set(e) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_phy_mode(e, phy_id) -(EOPNOTSUPP)
#define bcmeapi_ioctl_ethsw_port_erc_set(e) -(EOPNOTSUPP)
#endif
#define bcmeapi_link_check(unit, link, speed) {}
int bcmeapi_ethsw_dump_mib(int port, int type, int queue);
int ethsw_get_hw_stats(int port, struct net_device_stats *stats);
int bcmeapi_ioctl_ethsw_sal_dal_set (struct ethswctl_data *e);
void extsw_rreg(int page, int reg, uint8 *data, int len);
void extsw_wreg(int page, int reg, uint8 *data_in, int len);
int bcmeapi_ioctl_ethsw_mtu_set (struct ethswctl_data *e);
int ethsw_set_mac_hw(int port, PHY_STAT ps);
int bcmeapi_ioctl_ethsw_phys_port_to_rdpa_port(int unit, int port, int *rdpa_if);
int bcmeapi_ioctl_ethsw_ifname_to_rdpaif(struct ethswctl_data *e);
void bcmeapi_ethsw_set_stp_mode(unsigned int unit, unsigned int port, unsigned char stpState);
#define ethsw_get_pbvlan(port) 0
#define ethsw_set_pbvlan(port, fwdMap) {}

#endif /* _BCMSW_RUNNER_H_ */
