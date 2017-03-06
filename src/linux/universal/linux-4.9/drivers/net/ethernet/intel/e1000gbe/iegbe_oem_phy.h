/*******************************************************************************

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
  
  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226 

*******************************************************************************/
#ifndef _IEGBE_OEM_PHY_H_
#define _IEGBE_OEM_PHY_H_

#include <linux/types.h>

struct iegbe_hw;
struct iegbe_adapter;
struct ifreq;
struct iegbe_phy_info;

int32_t iegbe_oem_setup_link(struct iegbe_hw *hw);
int32_t iegbe_oem_set_trans_gasket(struct iegbe_hw *hw);

uint32_t iegbe_oem_get_tipg(struct iegbe_hw *hw);
int iegbe_oem_phy_is_copper(struct iegbe_hw *hw);
uint32_t iegbe_oem_get_phy_dev_number(struct iegbe_hw *hw);
int iegbe_oem_mii_ioctl(struct iegbe_adapter *adapter, unsigned long flags, 
                        struct ifreq *ifr, int cmd);
void iegbe_oem_fiber_live_in_suspend(struct iegbe_hw *hw);
void iegbe_oem_get_phy_regs(struct iegbe_adapter *adapter, uint32_t *data, 
                            uint32_t data_length);
int iegbe_oem_phy_loopback(struct iegbe_adapter *adapter);
void iegbe_oem_loopback_cleanup(struct iegbe_adapter *adapter);
uint32_t iegbe_oem_phy_speed_downgraded(struct iegbe_hw *hw, uint16_t *isDowngraded);
int32_t iegbe_oem_check_polarity(struct iegbe_hw *hw, uint16_t *polarity);

int32_t iegbe_oem_phy_is_full_duplex(struct iegbe_hw *hw, int *isFD);
int32_t iegbe_oem_phy_is_speed_1000(struct iegbe_hw *hw, int *is1000);
int32_t iegbe_oem_phy_is_speed_100(struct iegbe_hw *hw, int *is100);

int32_t iegbe_oem_force_mdi(struct iegbe_hw *hw, int *resetPhy);
int32_t iegbe_oem_phy_reset_dsp(struct iegbe_hw *hw);
int32_t iegbe_oem_cleanup_after_phy_reset(struct iegbe_hw *hw);

int32_t iegbe_oem_phy_get_info(struct iegbe_hw *hw,
                               struct iegbe_phy_info *phy_info);

int32_t iegbe_oem_phy_hw_reset(struct iegbe_hw *hw);
void iegbe_oem_phy_init_script(struct iegbe_hw *hw);

int32_t iegbe_oem_read_phy_reg_ex(struct iegbe_hw *hw,
                                  uint32_t reg_addr,
                                  uint16_t *phy_data);
int32_t iegbe_oem_write_phy_reg_ex(struct iegbe_hw *hw,
                                   uint32_t reg_addr,
                                   uint16_t phy_data);

int iegbe_oem_phy_needs_reset_with_mac(struct iegbe_hw *hw);

int32_t iegbe_oem_config_dsp_after_link_change(struct iegbe_hw *hw,
                                               int link_up);

int32_t iegbe_oem_get_cable_length(struct iegbe_hw *hw,
		        				   uint16_t *min_length,
			        			   uint16_t *max_length);

int32_t iegbe_oem_phy_is_link_up(struct iegbe_hw *hw, int *isUp);

/* Default Register Macros */

#define ICP_XXXX_MAC_0 0       /* PCI Device numbers associated with MACs on */
#define ICP_XXXX_MAC_1 1       /* ICP_XXXX family of controllers */
#define ICP_XXXX_MAC_2 2

#define DEFAULT_ICP_XXXX_TIPG_IPGT 8      /* Inter Packet Gap Transmit Time */
#define ICP_XXXX_TIPG_IPGT_MASK 0x000003FFUL 
#define BCM5481_PHY_ID		0x0143BCA0
#define BCM5395S_PHY_ID		0x0143BCF0

/* Miscellaneous defines */
#ifdef IEGBE_10_100_ONLY
    #define ICP_XXXX_AUTONEG_ADV_DEFAULT	0x0F 
#else
    #define ICP_XXXX_AUTONEG_ADV_DEFAULT	0x2F
#endif

/* BCM5481 specifics */

#define BCM5481_ECTRL		(0x10)
#define BCM5481_ESTAT		(0x11)
#define BCM5481_RXERR		(0x12)
#define BCM5481_EXPRW		(0x15)
#define BCM5481_EXPACC		(0x17)
#define BCM5481_ASTAT		(0x19)
#define BCM5481_R18H		(0x18)
#define BCM5481_R1CH		(0x1c)

/* indirect register access via register 18h */

#define BCM5481_R18H_SV_MASK	(7) // Mask for SV bits.
#define BCM5481_R18H_SV_ACTRL	(0) // SV000 Aux. control
#define BCM5481_R18H_SV_10BT	(1) // SV001 10Base-T
#define BCM5481_R18H_SV_PMII	(2) // SV010 Power/MII control
#define BCM5481_R18H_SV_MTEST	(4) // SV100 Misc. test
#define BCM5481_R18H_SV_MCTRL	(7) // SV111 Misc. control

#define BCM5481_R18H_SV001_POL	(1 << 13) // Polarity
#define BCM5481_R18H_SV010_LPM	(1 << 6)
#define BCM5481_R18H_SV111_SKEW	(1 << 8)
#define BCM5481_R18H_WE		(1 << 15) // Write enable

// 0x1c registers
#define BCM5481_R1CH_SV_SHIFT	(10)
#define BCM5481_R1CH_SV_MASK	(0x1f)
#define BCM5481_R1CH_SC1	(0x02) // sv00010 Spare control 1
#define BCM5481_R1CH_CACR	(0x03) // sv00011 Clock alignment control
#define BCM5481_R1CH_LCTRL	(0x09) // sv01001 LED control
#define BCM5481_R1CH_LEDS1	(0x0d) // sv01101 LED selector 1

// 0x1c common
#define BCM5481_R1CH_WE		(1 << 15) // Write enable

// 0x1c, sv 00010
#define BCM5481_R1CH_SC1_LINK	(1 << 2) // sv00010 Linkspeed

// 0x1c, sv 00011
#define BCM5481_R1CH_CACR_TCD	(1 << 9) // sv00011 RGMII tx clock delay

// 0x1c, sv 01001
#define BCM5481_R1CH_LCTRL_ALEN	(1 << 4) // Activity/Link enable on ACTIVITY LED
#define BCM5481_R1CH_LCTRL_AEN	(1 << 3) // Activity enable on ACTIVITY LED

#define BCM5481_ECTRL_DISMDIX	(1 <<14)

#define BCM5481_MCTRL_AUTOMDIX	(1 <<9)

#define BCM5481_ESTAT_LINK	(1 << 8)

#define BCM5481_ASTAT_ANC	(1 << 15)
#define BCM5481_ASTAT_ANHCD	(7 << 8)
#define BCM5481_ASTAT_HCD(x)	((x >> 8) & 7)
#define BCM5481_ASTAT_1KBTFD	(0x7)
#define BCM5481_ASTAT_1KBTHD	(0x6)
#define BCM5481_ASTAT_100BTXFD	(0x5)
#define BCM5481_ASTAT_100BTXHD	(0x3)

#endif /* ifndef _IEGBE_OEM_PHY_H_ */
 
