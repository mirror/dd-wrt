#ifndef _ATHRS_QOPS_H
#define _ATHRS_QOPS_H 1

#include "athrs_ioctl.h"

#if defined(CONFIG_MACH_AR7240)

#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0

#include "ag7240_mac.h"
extern ag7240_mac_t *ag7240_macs[2];

#ifdef CONFIG_AR7240_S26_PHY
#include "ar7240_s26_phy.h"
#endif

#ifdef CONFIG_AR7242_RGMII_PHY
#include "athrf1_phy.h"
#endif

#ifdef CONFIG_AR7242_S16_PHY
#include "athrs16_phy.h"
#endif

static uint32_t
ag7240_mac_reg_read(char *ad_name,uint32_t Reg) {

#ifdef CONFIG_AR7242_S16_PHY
    ag7240_mac_t *mac = ag7240_name2mac(ad_name);

    if (mac->mac_unit == 1 && is_ar7242()) 
        return (athrs16_reg_read(Reg));
    else
#endif
        return (athrs26_reg_read(Reg));

}

static void
ag7240_mac_reg_write(char *ad_name,uint32_t Reg, uint32_t Val) 
{

    ag7240_mac_t *mac = ag7240_name2mac(ad_name);
#ifdef CONFIG_AR7242_S16_PHY

    if (mac->mac_unit == 1 && is_ar7242()) 
        athrs16_reg_write(Reg,Val);
    else
#endif
    printk(KERN_INFO "[%x] QoS Reg write %x=%x\n",mac->mac_unit,Reg,Val);
        athrs26_reg_write(Reg,Val);
    return;

}

static void
ag7240_mac_reg_rmw_set(char *ad_name,uint32_t Reg, uint32_t Val) 
{
    int tval = ag7240_mac_reg_read(ad_name,Reg);

    ag7240_mac_reg_write(ad_name,Reg,(tval | Val));
}

static void
ag7240_mac_reg_rmw_clear(char *ad_name,uint32_t Reg, uint32_t Val) 
{
    int tval = ag7240_mac_reg_read(ad_name,Reg);

    ag7240_mac_reg_write(ad_name,Reg,(tval & ~Val));
}

static int 
ag7240_enable_mac_qos(char *ad_name) {

    ag7240_mac_t *mac = ag7240_name2mac(ad_name);
    struct net_device *dev = mac->mac_dev;
    int val = 0;

    dev->stop(dev);
    if(mac_has_flag(mac,ETH_SWONLY_MODE))
        return -1;

    mac_set_flag(mac,WAN_QOS_SOFT_CLASS);

    if (mac->mac_unit == ENET_UNIT_LAN) {
        val = ag7240_mac_reg_read(ad_name, ATHR_CTRL_PORT_0);
        if( (val & ATHR_HDR_EN) == 0){
            ag7240_mac_reg_rmw_set(ad_name,ATHR_CTRL_PORT_0, ATHR_HDR_EN);
            mac->qos->qos_flag |= 0x1 << 0;
        } 

        val = ag7240_mac_reg_read(ad_name, ATHR_CPU_PORT); 
        if( (val & ATHR_CPU_EN) == 0){
            ag7240_mac_reg_rmw_set(ad_name,ATHR_CPU_PORT, ATHR_CPU_EN);
            mac->qos->qos_flag |= 0x1 << 1;
        } 
        ag7240_mac_reg_rmw_set(ad_name,ATHR_QOS_MODE_REGISTER,ATHR_QOS_WEIGHTED);

#ifdef CONFIG_AR7242_S16_PHY
        if (is_ar7242() && mac->mac_unit == 0)
            mac_set_flag(mac,ATHR_S16_HEADER);
        else
#endif
#ifndef CONFIG_ATHEROS_HEADER_EN
            mac_set_flag(mac,ATHR_S26_HEADER);
#endif
        for (val = 1; val <= 4; val++){
            ag7240_mac_reg_rmw_clear(ad_name,ATHR_PRI_CTRL_PORT_0 +
              (val * 256) , (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));
        }
    }
    else {
     /* WAN MAC setting */
        ag7240_mac_reg_rmw_clear(ad_name,ATHR_PRI_CTRL_PORT_0 +
               (5 * 256) , (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));
    }
    mac->qos->qos_cap = 1;
    dev->open(dev);
    return 0;
}

static void 
ag7240_disable_mac_qos(char *ad_name) {

    ag7240_mac_t *mac = ag7240_name2mac(ad_name);
    struct net_device *dev = mac->mac_dev;
    int val = 0;

    dev->stop(dev);

    mac_clear_flag(mac,WAN_QOS_SOFT_CLASS);

    if (mac->mac_unit == ENET_UNIT_LAN) {
        if(mac->qos->qos_flag != 0){
            if( (0x1 & mac->qos->qos_flag) != 0)
                ag7240_mac_reg_rmw_clear(ad_name,ATHR_CTRL_PORT_0 ,ATHR_HDR_EN);
            if( ( (0x1 << 1) & mac->qos->qos_flag) != 0)
                ag7240_mac_reg_rmw_clear(ad_name,ATHR_CPU_PORT, ATHR_CPU_EN);
        }
        ag7240_mac_reg_rmw_clear(ad_name,ATHR_QOS_MODE_REGISTER,ATHR_QOS_WEIGHTED);

#ifdef CONFIG_AR7242_S16_PHY
        if (is_ar7242() && mac->mac_unit == 0)
            mac_clear_flag(mac,ATHR_S16_HEADER);
        else
#endif
#ifndef CONFIG_ATHEROS_HEADER_EN
            mac_clear_flag(mac,ATHR_S26_HEADER);
#endif
        for (val = 1; val <= 4; val++){
            ag7240_mac_reg_rmw_clear(ad_name,ATHR_PRI_CTRL_PORT_0 +
                 (val * 256) , (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));
        }
    }
    else {
        /* WAN MAC setting */
        ag7240_mac_reg_rmw_clear(ad_name,ATHR_PRI_CTRL_PORT_0 +
                 (5 * 256) , (ATHR_DA_PRI_EN|ATHR_TOS_PRI_EN|ATHR_VLAN_PRI_EN|ATHR_PORT_PRI_EN));

    }
    mac->qos->qos_cap = 0;
	mac->qos->qos_flag = 0;
    dev->open(dev);
}

struct ath_qops ath_qos_ops = {
    ag7240_mac_reg_read,
    ag7240_mac_reg_write,
    ag7240_mac_reg_rmw_set,
    ag7240_mac_reg_rmw_clear,
    ag7240_enable_mac_qos,
    ag7240_disable_mac_qos,
    0,
};

static inline struct ath_qops
*get_qmac(char *ad_name)
{
    ag7240_mac_t *mac = ag7240_name2mac(ad_name);

    if (mac_has_flag(mac,ETH_SWONLY_MODE))
        mac = ag7240_unit2mac(1);

    return (mac->qos);
}

static inline  struct net_device 
*get_ndev(char *ad_name)
{
    ag7240_mac_t *mac = ag7240_name2mac(ad_name);

    if (mac_has_flag(mac,ETH_SWONLY_MODE))
        mac = ag7240_unit2mac(1);

    return (mac->mac_dev);
}

static inline void
set_mac_qops(void *mac)
{
    ag7240_mac_t *qmac = (ag7240_mac_t *)mac;

    qmac->qos = &ath_qos_ops;
    printk("%s : %p\n",__func__,qmac->mac_dev);
}

#elif defined(CONFIG_MACH_AR7100)

#include "ar7100.h"

#define #define ag7100_name2mac(name)      \
    strcmp(name,"eth0") ? ag7100_unit2mac(1) : ag7100_unit2mac(0)

#if defined(CONFIG_ATHRS26_PHY)
#include "athrs26_phy.h"
#elif  defined(CONFIG_ATHRS16_PHY)
#include "athrs16_phy.h"
#endif

static uint32_t
ag7100_mac_reg_read(char *ad_name,uint32_t Reg) {

#if defined(CONFIG_ATHRS26_PHY)
    return (athrs26_reg_read(Reg));
#elif defined(CONFIG_ATHRS16_PHY)
    return (athrs16_reg_read(Reg));
#endif

}

static void
ag7100_mac_reg_write(char *ad_name,uint32_t Reg, uint32_t Val) {

#if defined(CONFIG_ATHRS26_PHY)
    return (athrs26_reg_write(Reg,Val));
#elif defined(CONFIG_ATHRS16_PHY)
    return (athrs16_reg_write(Reg,Val));
#endif

}

static void
ag7100_mac_reg_rmw_set(char *ad_name,uint32_t Reg, uint32_t Val) {

    int tval = ag7100_mac_reg_read(ad_name,Reg);
    ag7100_mac_reg_write(ad_name,Reg,(tval | Val));
    return;
}

static void
ag7100_mac_reg_rmw_clear(char *ad_name,uint32_t Reg, uint32_t Val) {

    int tval = ag7100_mac_reg_read(ad_name,Reg);
    ag7100_mac_reg_write(ad_name,Reg,(tval & ~Val));
    return;
}

struct ath_qops ath_qos_ops = {
    ag7100_mac_reg_read,
    ag7100_mac_reg_write,
    ag7100_mac_reg_rmw_set,
    ag7100_mac_reg_rmw_clear,
};

static inline struct ath_qops
*get_qmac(char *ad_name)
{
    ag7100_mac_t *mac = ag7100_name2mac(ad_name);
    return (mac->qos);
}

static inline void
set_mac_qops(void *mac)
{
    ag7100_mac_t *qmac = (ag7100_mac_t *)mac;
    qmac->qos = &ath_qos_ops;
}

static inline  struct net_device 
*get_ndev(char *ad_name)
{
    ag7100_mac_t *mac = ag7100_name2mac(ad_name);
    return (mac->mac_dev);
}

#endif

#endif  //_ATHRS_QOS_H
