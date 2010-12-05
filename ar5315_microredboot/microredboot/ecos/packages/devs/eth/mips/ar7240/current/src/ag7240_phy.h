#ifndef _AG7100_PHY_H
#define _AG7100_PHY_H

#define phy_reg_read        ag7100_mii_read
#define phy_reg_write       ag7100_mii_write

#ifndef CONFIG_ATHRS26_PHY
#define ag7100_phy_ioctl(unit, args)
#endif

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
#   define CONFIG_ATHRS26_PHY
#endif

#include "vsc8601_phy.h"
#include "vsc73xx.h"
#include "ipPhy.h"
#include "rtPhy.h"
#include "adm_phy.h"
#include "athr_phy.h"
#include "athrs26_phy.h"

#define in_interrupt(x)    0
#define schedule_work(x)
#define INIT_WORK(x,y)

#else /* Must be Linux, CONFIGs are defined in .config */

/* Empty */

#endif
#endif

/*
** Implements various interfaces depending on the PHY selected.
*/


#if defined(CONFIG_VITESSE_PHY)

#include "vscgen_phy.h"

#define ag7100_phy_is_up(unit)          vscgen_phy_is_up(unit)
#define ag7100_phy_speed(unit)          vscgen_phy_speed(unit)
#define ag7100_phy_is_fdx(unit)         vscgen_phy_is_fdx(unit)

static inline int 
ag7100_phy_setup(int unit)
{
  vscgen_phy_setup(unit);
  return (0);
}

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  return vscgen_phy_get_link_status(unit, link, fdx, speed, 0);
}

static inline int 
ag7100_print_link_status(int unit)
{
   return vscgen_phy_print_link_status(unit);
}

#elif defined(CONFIG_VITESSE_8601_PHY)

#include "vsc8601_phy.h"

static inline int 
ag7100_phy_setup(int unit)
{
  return vsc8601_phy_setup(unit);
}

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  return vsc8601_phy_get_link_status(unit, link, fdx, speed, 0);
}

static inline int 
ag7100_print_link_status(int unit)
{
  if (0==unit)
    return vsc8601_phy_print_link_status(unit);
  else 
    if (0 == in_interrupt())
      return vsc73xx_print_link_status(unit);
  return -1;  
}

#elif defined(CONFIG_VITESSE_8601_7395_PHY)

#include "vsc8601_phy.h"
#include "vsc73xx.h"

static inline int 
ag7100_phy_setup(int unit)
{
  if (0==unit) {
    return vsc8601_phy_setup(unit);
  } else { 
    if (1 == unit) {
      return vsc73xx_setup(unit);
    }
  }
  return -1;
}

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed) 
{
  if (0==unit)
    return vsc8601_phy_get_link_status(unit, link, fdx, speed, 0);
  else 
    if (0 == in_interrupt())
      return vsc73xx_get_link_status(unit, link, fdx, speed, 0);
  
  return -1;
}

static inline int 
ag7100_print_link_status(int unit)
{
  if (0==unit)
    return vsc8601_phy_print_link_status(unit);
  else
    if (0 == in_interrupt())
      return vsc73xx_phy_print_link_status(unit);
  return -1;  
}

#elif defined(CONFIG_ICPLUS_PHY)

#include "ipPhy.h"

#define ag7100_phy_setup(unit)          ip_phySetup(unit)
#define ag7100_phy_is_up(unit)          ip_phyIsUp(unit)
#define ag7100_phy_speed(unit)          ip_phySpeed(unit)
#define ag7100_phy_is_fdx(unit)         ip_phyIsFullDuplex(unit)

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int
ag7100_print_link_status(int unit)
{
  return -1;
}

#elif defined(CONFIG_REALTEK_PHY)

#include "rtPhy.h"

#define ag7100_phy_setup(unit)          rt_phySetup(unit, 0)
#define ag7100_phy_is_up(unit)          rt_phyIsUp(unit)
#define ag7100_phy_speed(unit)          rt_phySpeed(unit)
#define ag7100_phy_is_fdx(unit)         rt_phyIsFullDuplex(unit)

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int 
ag7100_print_link_status(int unit)
{
  return -1;
}

#elif defined(CONFIG_ADM6996FC_PHY)

#include "adm_phy.h"

#define ag7100_phy_setup(unit)          adm_phySetup(unit)
#define ag7100_phy_is_up(unit)          adm_phyIsUp(unit)
#define ag7100_phy_speed(unit)          adm_phySpeed(unit)
#define ag7100_phy_is_fdx(unit)         adm_phyIsFullDuplex(unit)
#define ag7100_phy_is_lan_pkt           adm_is_lan_pkt
#define ag7100_phy_set_pkt_port         adm_set_pkt_port
#define ag7100_phy_tag_len              ADM_VLAN_TAG_SIZE
#define ag7100_phy_get_counters         adm_get_counters

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int 
ag7100_print_link_status(int unit)
{
  return -1;
}

#elif defined(CONFIG_ATHRS26_PHY)

#include "athrs26_phy.h"

#define ag7100_phy_ioctl(unit, args)    athr_ioctl(unit,args)
#define ag7100_phy_setup(unit)          athrs26_phy_setup (unit)
#define ag7100_phy_is_up(unit)          athrs26_phy_is_up (unit)
#define ag7100_phy_speed(unit)          athrs26_phy_speed (unit)
#define ag7100_phy_is_fdx(unit)         athrs26_phy_is_fdx (unit)
#define ag7100_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7100_phy_set_pkt_port         athr_set_pkt_port
#define ag7100_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7100_phy_get_counters         athrs26_get_counters

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int
ag7100_print_link_status(int unit)
{
  return -1;
}

#elif defined(CONFIG_ATHRF1_PHY)

#include "athr_phy.h"

#define ag7100_phy_setup(unit)          athr_phy_setup(unit)
#define ag7100_phy_is_up(unit)          athr_phy_is_up(unit)
#define ag7100_phy_speed(unit)          athr_phy_speed(unit)
#define ag7100_phy_is_fdx(unit)         athr_phy_is_fdx(unit)
#define ag7100_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7100_phy_set_pkt_port         athr_set_pkt_port
#define ag7100_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7100_phy_get_counters         athr_get_counters

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int
ag7100_print_link_status(int unit)
{
  return -1;
}

#else
#error unknown PHY type PHY not configured in config.h
#endif

#endif

