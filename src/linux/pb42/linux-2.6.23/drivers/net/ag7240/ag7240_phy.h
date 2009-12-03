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

#define phy_reg_read        ag7240_mii_read
#define phy_reg_write       ag7240_mii_write

#include "ag7240.h"

#ifdef __BDI

/* Empty */

#else
#ifdef __ECOS

/* ecos will set the value of CYGNUM_USE_ENET_PHY to one of the following strings
 * based on the cdl. These are defined here in no particuilar way so the
 * #if statements that follow will have something to compare to.
 */
#define AR7240_VSC_ENET_PHY             1
#define AR7240_VSC8601_ENET_PHY         2
#define AR7240_VSC8601_VSC8601_ENET_PHY 3
#define AR7240_VSC8601_VSC73XX_ENET_PHY 4
#define AR7240_ICPLUS_ENET_PHY          5
#define AR7240_REALTEK_ENET_PHY         6
#define AR7240_ADMTEK_ENET_PHY          7  
#define AR7240_ATHRF1_ENET_PHY          8
#define AR7240_ATHRS26_ENET_PHY         9

#if (CYGNUM_USE_ENET_PHY == AR7240_VSC_ENET_PHY) 
#   define CONFIG_VITESSE_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_VSC8601_ENET_PHY) 
#   define CONFIG_VITESSE_8601_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_VSC8601_VSC73XX_ENET_PHY)
#   define CONFIG_VITESSE_8601_7395_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_ICPLUS_ENET_PHY)
#   define CONFIG_ICPLUS_PHY 
#elif (CYGNUM_USE_ENET_PHY == AR7240_REALTEK_ENET_PHY)
#   define CONFIG_REALTEK_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_ADMTEK_ENET_PHY)
#   define CONFIG_ADM6996FC_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_ATHRF1_ENET_PHY)
#   define CONFIG_ATHRF1_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7240_ATHRS26_ENET_PHY)
#   define CONFIG_ATHRS26_PHY
#else
#error unknown PHY type CYGNUM_USE_ENET_PHY
#endif

#include "vsc8601_phy.h"
#include "vsc73xx.h"
#include "ipPhy.h"
#include "rtPhy.h"
#include "adm_phy.h"
#include "athr_phy.h"
#include "ar7240_s26_phy.h"

#define in_interrupt(x)    0
#define schedule_work(x)
#define INIT_WORK(x,y)

#else /* Must be Linux, CONFIGs are defined in .config */

/* Empty */

#endif
#endif

#ifdef CONFIG_AR7240_S26_PHY

#include "ar7240_s26_phy.h"

#define ag7240_phy_ioctl(unit, args)    athr_ioctl(unit,args)
#define ag7240_phy_setup(unit)          athrs26_phy_setup (unit)
#define ag7240_phy_is_up(unit)          athrs26_phy_is_up (unit)
#define ag7240_phy_speed(unit,phyUnit)  athrs26_phy_speed (unit,phyUnit)
#define ag7240_phy_is_fdx(unit,phyUnit) athrs26_phy_is_fdx(unit,phyUnit)
#define ag7240_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7240_phy_set_pkt_port         athr_set_pkt_port
#define ag7240_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7240_phy_get_counters         athrs26_get_counters

static inline unsigned int 
ag7240_get_link_status(int unit, int *link, int *fdx, ag7240_phy_speed_t *speed,int phyUnit)
{
  *link=ag7240_phy_is_up(unit);
  *fdx=ag7240_phy_is_fdx(unit, phyUnit);
  *speed=ag7240_phy_speed(unit, phyUnit);
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

