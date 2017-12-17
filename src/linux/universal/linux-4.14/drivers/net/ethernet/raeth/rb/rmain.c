#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <asm/uaccess.h>

#include "smi.h"
#include "rtl_ioctl.h"
#include "rtl8368s_types.h"

//linux 2.6
#include "rtl8366rb_api.h"
#include "rtl8366rb_api_ext.h"
//linux 2.6
extern void SetVLan(void);

#define RDM_DEVNAME	    "rtl"

void gemtek_eth_link_status_init(void);

int switch_reset(void)
{
    rtl8366rb_vlanConfig_t  vlan_cfg;
    rtl8366rb_macConfig_t   mac5_cfg;
    if(rtl8366rb_initChip() != SUCCESS)   /* Initial RTL8366RB */
	{
        return FAILED;
        }

    mac5_cfg.force  = MAC_FORCE;
    mac5_cfg.speed  = SPD_1000M;
    mac5_cfg.duplex = FULL_DUPLEX;
    mac5_cfg.link   = 1;
    mac5_cfg.txPause= 1;
    mac5_cfg.rxPause= 1;
    if(rtl8366rb_setMac5ForceLink(&mac5_cfg) != SUCCESS)
	{
        return FAILED;
        }

    if(rtl8366rb_setCPUPort(5, 1) != SUCCESS)  /* Set Port 5 as CPU port */
	{
        return FAILED;
        }
    if(rtl8366rb_initVlan() != SUCCESS)    /* Initial VLAN */
	{
        return FAILED;
        }

//    rtl8366rb_setVlan_with_fid(1,0x3e,0x1e,0);
//    rtl8366rb_setVlan_with_fid(2,0x21,0x01,1);

    vlan_cfg.vid        = 1;
    vlan_cfg.mbrmsk     = 0x3E;
    vlan_cfg.untagmsk   = 0x1E;
    vlan_cfg.fid        = 0;
    if(rtl8366rb_setVlan(&vlan_cfg) != SUCCESS)    /* Set LAN, VID = 1, FID = 0 */
        return FAILED;
    vlan_cfg.vid        = 2;
    vlan_cfg.mbrmsk     = 0x21;
    vlan_cfg.untagmsk   = 0x01;
    vlan_cfg.fid        = 1;
    if(rtl8366rb_setVlan(&vlan_cfg) != SUCCESS)    /* Set WAN, VID = 2, FID = 1 */
        return FAILED;
    if(rtl8366rb_setVlanPVID(0, 2, 0) != SUCCESS)
        return FAILED;
    if(rtl8366rb_setVlanPVID(1, 1, 0) != SUCCESS)
        return FAILED;
    if(rtl8366rb_setVlanPVID(2, 1, 0) != SUCCESS)
        return FAILED;
    if(rtl8366rb_setVlanPVID(3, 1, 0) != SUCCESS)
        return FAILED;
    if(rtl8366rb_setVlanPVID(4, 1, 0) != SUCCESS)
        return FAILED;
    if(rtl8366rb_setVlanPVID(5, 1, 0) != SUCCESS)
        return FAILED;
    //if(rtl8366rb_setGreenEthernet(1, 1) != SUCCESS) /* Enable Green feature and power saving */
    //    return FAILED;
	return SUCCESS;        
}

