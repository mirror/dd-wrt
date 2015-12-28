#ifndef _AG7100_PHY_H
#define _AG7100_PHY_H

#define phy_reg_read        ag7100_mii_read
#define phy_reg_write       ag7100_mii_write

#if !defined(CONFIG_ATHRS26_PHY) && !defined(CONFIG_ATHRS16_PHY)
#define ag7100_phy_ioctl(unit, args)
#endif

#include "ag7100.h"

#ifdef __BDI

/* Empty */

#else
#ifdef __ECOS

/* ecos will set the value of CYGNUM_USE_ENET_PHY to one of the following strings
 * based on the cdl. These are defined here in no particuilar way so the
 * #if statements that follow will have something to compare to.
 */
#define AR7100_VSC_ENET_PHY             1
#define AR7100_VSC8601_ENET_PHY         2
#define AR7100_VSC8601_VSC8601_ENET_PHY 3
#define AR7100_VSC8601_VSC73XX_ENET_PHY 4
#define AR7100_ICPLUS_ENET_PHY          5
#define AR7100_REALTEK_ENET_PHY         6
#define AR7100_ADMTEK_ENET_PHY          7  
#define AR7100_ATHRF1_ENET_PHY          8
#define AR7100_ATHRS26_ENET_PHY         9
#define AR7100_ATHRS16_ENET_PHY         10

#if (CYGNUM_USE_ENET_PHY == AR7100_VSC_ENET_PHY) 
#   define CONFIG_ATHR_VITESSE_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_VSC8601_ENET_PHY) 
#   define CONFIG_VITESSE_8601_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_VSC8601_VSC73XX_ENET_PHY)
#   define CONFIG_VITESSE_8601_7395_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_ICPLUS_ENET_PHY)
#   define CONFIG_ATHR_ICPLUS_PHY 
#elif (CYGNUM_USE_ENET_PHY == AR7100_REALTEK_ENET_PHY)
#   define CONFIG_ATHR_REALTEK_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_ADMTEK_ENET_PHY)
#   define CONFIG_ADM6996FC_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_ATHRF1_ENET_PHY)
#   define CONFIG_ATHRF1_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_ATHRS26_ENET_PHY)
#   define CONFIG_ATHRS26_PHY
#elif (CYGNUM_USE_ENET_PHY == AR7100_ATHRS16_ENET_PHY)
#   define CONFIG_ATHRS16_PHY
#else
#error unknown PHY type CYGNUM_USE_ENET_PHY
#endif

#include "vsc8601_phy.h"
#include "vsc73xx.h"
#include "ipPhy.h"
#include "rtPhy.h"
#include "adm_phy.h"
#include "athr_phy.h"
#include "athrs26_phy.h"
#include "athrs16_phy.h"

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

#if defined (CONFIG_CAMEO_REALTEK_PHY)
#include "rtl8366sr_phy.h"

#ifdef CONFIG_BUFFALO
#include "rtl8366rb_phy.h"

enum
{
	CHIP_TYPE_RTL8366SR,
	CHIP_TYPE_RTL8366RB,
	CHIP_TYPE_UNKNOWN
};

typedef struct
{
	BOOL (*phy_setup)(int);
	int (*phy_is_up)(int);
	int (*phy_speed)(int);
	int (*phy_is_fdx)(int);
	int (*get_link_status)(int, int *, int *, ag7100_phy_speed_t *);
}
RTL8366_FUNCS;

extern RTL8366_FUNCS rtl_funcs;

#define ag7100_phy_is_up(unit) rtl_funcs.phy_is_up(unit)
#define ag7100_phy_speed(unit) rtl_funcs.phy_speed(unit)
#define ag7100_phy_is_fdx(unit) rtl_funcs.phy_is_fdx(unit)

static inline void
ag7100_phy_setup(int unit)
{
	rtl_funcs.phy_setup(unit);
}

static inline unsigned int
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  
  rtl_funcs.get_link_status(unit, link, fdx, speed);
  return 0;
}
#else
#include "rtl8366sr_phy.h"

enum
{
	CHIP_TYPE_RTL8366SR,
	CHIP_TYPE_RTL8366RB,
	CHIP_TYPE_UNKNOWN
};


#define ag7100_phy_is_up(unit)          rtl8366sr_phy_is_up(unit)
#define ag7100_phy_speed(unit)          rtl8366sr_phy_speed(unit)
#define ag7100_phy_is_fdx(unit)         rtl8366sr_phy_is_fdx(unit)

static inline int
ag7100_phy_setup(int unit)
{
  return rtl8366sr_phy_setup(unit); 
}

static inline unsigned int
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
    rtl8366sr_get_link_status(unit, link, fdx, speed);
    return 0;
}
#endif // CONFIG_BUFFALO //

#elif defined(CONFIG_ATHR_VITESSE_PHY)

#include "vsc_phy.h"

#define ag7100_phy_is_up(unit)          vsc_phy_is_up(unit)
#define ag7100_phy_speed(unit)          vsc_phy_speed(unit)
#define ag7100_phy_is_fdx(unit)         vsc_phy_is_fdx(unit)

static inline int 
ag7100_phy_setup(int unit)
{
  vsc_phy_setup(unit);
  return (0);
}

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  return vsc_phy_get_link_status(unit, link, fdx, speed, 0);
}

static inline int 
ag7100_print_link_status(int unit)
{
   return vsc_phy_print_link_status(unit);
}
#elif defined(CONFIG_PHY_LAYER)
#include <linux/phy.h>

#define ag7100_phy_is_up(unit)          1
#ifdef CONFIG_RTL8366RB_SMI
#define ag7100_phy_speed(unit)          AG7100_PHY_SPEED_1000T
#elif CONFIG_RTL8366_SMI_MODULE
#define ag7100_phy_speed(unit)          AG7100_PHY_SPEED_1000T
#else
#define ag7100_phy_speed(unit)          AG7100_PHY_SPEED_100TX
#endif
#define ag7100_phy_is_fdx(unit)         1
static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
*fdx = 1;
#ifdef CONFIG_RTL8366RB_SMI
*speed = AG7100_PHY_SPEED_1000T;
#elif defined(CONFIG_RTL8366_SMI_MODULE)
*speed = AG7100_PHY_SPEED_1000T;
#else
*speed = AG7100_PHY_SPEED_100TX;
#endif
*link = 1;
return 0;
}


static int ag71xx_mdio_read(struct mii_bus *bus, int addr, int reg)
{
	return ag7100_mii_read(0, addr, reg);
}

static int ag71xx_mdio_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	ag7100_mii_write(0, addr, reg, val);
	return 0;
}



static inline int 
ag7100_phy_setup(int unit)
{
    return 0;
}

static void ag7100_adjust_link(struct net_device *dev)
{

}

static inline struct mii_bus *ag7100_mdiobus_setup(int unit,struct net_device *dev)
{
  int i;
  struct phy_device *phydev = NULL;
  ag7100_mac_t *mac = (ag7100_mac_t *)netdev_priv(dev);
  struct mii_bus *mii_bus = kzalloc(sizeof(struct mii_bus),GFP_KERNEL);

  snprintf(mii_bus->id, MII_BUS_ID_SIZE, "%d", unit);
  mii_bus->phy_mask=0;
  mii_bus->priv = dev;
  mii_bus->name = "ag7100_mii";
  mii_bus->read = ag71xx_mdio_read;
  mii_bus->write = ag71xx_mdio_write;
  mii_bus->irq = kmalloc(sizeof(int) * PHY_MAX_ADDR, GFP_KERNEL);
  for (i = 0; i < PHY_MAX_ADDR; i++)
	    mii_bus->irq[i] = PHY_POLL;
  mdiobus_register(mii_bus);
   if (unit==0)
   {
//   #ifdef CONFIG_MVSWITCH_PHY
//    phydev = mii_bus->phy_map[31];
//    #else
    phydev = mii_bus->phy_map[0];
//    #endif
   }
   else
    phydev = mii_bus->phy_map[4];

    if (phydev!=NULL)
    {
    phydev = phy_connect(dev, dev_name(&phydev->dev), &ag7100_adjust_link,PHY_INTERFACE_MODE_RMII);
    mac->rx = phydev->netif_receive_skb;
    phydev->supported &= PHY_BASIC_FEATURES;
    phydev->advertising = phydev->supported;
    }else{
    printk(KERN_EMERG "phymap is null\n");
    }

  return mii_bus;
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

#elif defined(CONFIG_ATHR_ICPLUS_PHY)

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

#elif defined(CONFIG_ATHR_REALTEK_PHY)

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

#elif defined(CONFIG_ATHRS16_PHY)

#include "athrs16_phy.h"

#define ag7100_phy_ioctl(unit, args)    athr_ioctl(unit,args)
#define ag7100_phy_setup(unit)          athrs16_phy_setup (unit)
#define ag7100_phy_is_up(unit)          athrs16_phy_is_up (unit)
#define ag7100_phy_speed(unit)          athrs16_phy_speed (unit)
#define ag7100_phy_is_fdx(unit)         athrs16_phy_is_fdx (unit)
#define ag7100_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7100_phy_set_pkt_port         athr_set_pkt_port
#define ag7100_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7100_phy_get_counters         athrs16_get_counters

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

#elif defined(CONFIG_ATHRF1_PHY) || defined(CONFIG_ATHR_PHY)

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
  if (*link == 0)
    return 0;
    
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

