#ifndef _AG7100_RTL8366RB_COMPAT_H_
#define _AG7100_RTL8366RB_COMPAT_H_

#include "rtl8366rb_api.h"

int32 rtl8366rb_initChip(void);
int32 rtl8366rb_setMac5ForceLink(rtl8366rb_macConfig_t *ptr_maccfg);
int32 rtl8368s_getAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 *data );
int32 rtl8366rb_initVlan(void);
int32 rtl8366rb_setVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg);
int32 rtl8366rb_setVlanPVID(uint32 port, uint32 vid, uint32 priority);
int32 rtl8366rb_getPHYLinkStatus(uint32 phy, uint32 *ptr_linkStatus);
int32 rtl8366rb_setGreenEthernet(uint32 greenFeature, uint32 powerSaving);
int32 rtl8368s_setAsicReg(uint32 reg, uint32 value);

#endif /* _AG7100_RTL8366RB_COMPAT_H_ */

