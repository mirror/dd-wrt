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

#ifndef _AG7240_PHY_H
#define _AG7240_PHY_H

#include "ag7240.h"
#include "athrs_ioctl.h"

#ifdef CONFIG_AR7240_S26_PHY

#include "ar7240_s26_phy.h" 
#ifdef CONFIG_AR7242_RGMII_PHY
#include "athrf1_phy.h"
#endif
#ifdef CONFIG_AR7242_S16_PHY
#include "athrs16_phy.h"
#endif

#ifdef CONFIG_AR7242_RTL8309G_PHY
#include "rtl8309g_phy.h"
#endif
extern ag7240_mac_t *ag7240_macs[2];
#define ag7240_phy_is_up(unit)          athrs26_phy_is_up (unit)
#define ag7240_phy_speed(unit,phyUnit)  athrs26_phy_speed (unit,phyUnit)
#define ag7240_phy_is_fdx(unit,phyUnit) athrs26_phy_is_fdx(unit,phyUnit)
#define ag7240_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7240_phy_set_pkt_port         athr_set_pkt_port
#define ag7240_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7240_phy_get_counters         athrs26_get_counters

static inline void athrs_reg_dev(ag7240_mac_t **ag7240_macs)
{
#if defined(CONFIG_AR7242_S16_PHY)
  if (is_ar7242())
    athrs16_reg_dev(ag7240_macs);
#endif
  athrs26_reg_dev(ag7240_macs);

  return ;

}

static inline int athrs_do_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd)
{
  ag7240_mac_t *mac = (ag7240_mac_t *)netdev_priv(dev);
  int ret = -1;

  if (is_ar7240() || mac->mac_unit == 1)
    ret = athrs26_ioctl(dev,ifr, cmd);
#ifdef CONFIG_AR7242_S16_PHY
  else if(is_ar7242())
    ret = athrs16_ioctl(ifr->ifr_data, cmd);
#endif
#ifdef CONFIG_ATHRS_QOS
  if(ret < 0) 
    ret = athrs_config_qos(ifr->ifr_data,cmd);
#endif
  return ret;
}

static inline void ag7240_phy_reg_init(int unit)
{
#ifdef CONFIG_AR7242_RTL8309G_PHY
  if (unit == 0 && is_ar7242())
    rtl8309g_reg_init(unit);
  else
#else
#ifndef CONFIG_AR7242_S16_PHY
  if (unit == 0)
    athrs26_reg_init(unit);
#else
  if (unit == 0 && is_ar7242())
    athrs16_reg_init(unit);
#endif
  else
#endif
    athrs26_reg_init_lan(unit);
} 

static inline void ag7240_phy_setup(int unit)
{

#ifdef CONFIG_AR7242_RTL8309G_PHY
    if (is_ar7242() && (unit==0)) {
    	 rtl8309g_phy_setup(unit);
    } else 
#endif 
  if (is_ar7241() || is_ar7240() || is_ar933x())
    athrs26_phy_setup (unit);
  else if (is_ar7242() && unit == 1) 
    athrs26_phy_setup (unit);
#ifdef CONFIG_AR7242_RGMII_PHY
  else if (is_ar7242() && unit == 0)
    athr_phy_setup(unit);
#endif
#ifdef CONFIG_AR7242_S16_PHY
  else if (is_ar7242() && unit == 0)
    athrs16_phy_setup(unit);
#endif
}

static inline unsigned int 
ag7240_get_link_status(int unit, int *link, int *fdx, ag7240_phy_speed_t *speed,int phyUnit)
{

#ifdef CONFIG_AR7242_RTL8309G_PHY
    if (is_ar7242() && (unit==0)) {
    *link=rtl8309g_phy_is_up(unit);
    *fdx=rtl8309g_phy_is_fdx(unit);
    *speed=rtl8309g_phy_speed(unit);
    }
    else
#endif

  if (is_ar7240() || is_ar7241() || (is_ar7242() && unit == 1) || is_ar933x()) {
    *link=ag7240_phy_is_up(unit);
    *fdx=ag7240_phy_is_fdx(unit, phyUnit);
    *speed=ag7240_phy_speed(unit, phyUnit);
  } 
#ifdef CONFIG_AR7242_RGMII_PHY
  else if(is_ar7242() && unit == 0){
    *link=athr_phy_is_up(unit);
    *fdx=athr_phy_is_fdx(unit,phyUnit);
    *speed=athr_phy_speed(unit,phyUnit);
  }
#endif
#ifdef CONFIG_AR7242_VIR_PHY
  else if(is_ar7242() && unit == 0){
    *link=athr_vir_phy_is_up(unit);
    *fdx=athr_vir_phy_is_fdx(unit);
    *speed=athr_vir_phy_speed(unit);
  }
#endif

#ifdef CONFIG_AR7242_S16_PHY
  else if(is_ar7242() && unit == 0){
    *link=athrs16_phy_is_up(unit);
    *fdx=athrs16_phy_is_fdx(unit);
    *speed=athrs16_phy_speed(unit);
  }
#endif
  return 0;
}

static inline int
ag7240_print_link_status(int unit)
{
  return -1;
}
#else
#error unknown PHY type PHY not configured in config.h
#endif

#endif

