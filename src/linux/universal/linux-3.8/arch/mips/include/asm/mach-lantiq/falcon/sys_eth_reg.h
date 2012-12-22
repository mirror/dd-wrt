/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _sys_eth_reg_h
#define _sys_eth_reg_h

/** \addtogroup SYS_ETH_REGISTER
   @{
*/
/* access macros */
#define sys_eth_r32(reg) reg_r32(&sys_eth->reg)
#define sys_eth_w32(val, reg) reg_w32(val, &sys_eth->reg)
#define sys_eth_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &sys_eth->reg)
#define sys_eth_r32_table(reg, idx) reg_r32_table(sys_eth->reg, idx)
#define sys_eth_w32_table(val, reg, idx) reg_w32_table(val, sys_eth->reg, idx)
#define sys_eth_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, sys_eth->reg, idx)
#define sys_eth_adr_table(reg, idx) adr_table(sys_eth->reg, idx)


/** SYS_ETH register structure */
struct gpon_reg_sys_eth
{
   /** Clock Status Register */
   unsigned int clks; /* 0x00000000 */
   /** Clock Enable Register
       Via this register the clocks for the domains can be enabled. */
   unsigned int clken; /* 0x00000004 */
   /** Clock Clear Register
       Via this register the clocks for the domains can be disabled. */
   unsigned int clkclr; /* 0x00000008 */
   /** Reserved */
   unsigned int res_0[5]; /* 0x0000000C */
   /** Activation Status Register */
   unsigned int acts; /* 0x00000020 */
   /** Activation Register
       Via this register the domains can be activated. */
   unsigned int act; /* 0x00000024 */
   /** Deactivation Register
       Via this register the domains can be deactivated. */
   unsigned int deact; /* 0x00000028 */
   /** Reboot Trigger Register
       Via this register the domains can be rebooted (sent through reset). */
   unsigned int rbt; /* 0x0000002C */
   /** Reserved */
   unsigned int res_1[32]; /* 0x00000030 */
   /** External PHY Control Register */
   unsigned int extphyc; /* 0x000000B0 */
   /** Power Down Configuration Register
       Via this register the configuration is done whether in case of deactivation the power supply of the domain shall be removed. */
   unsigned int pdcfg; /* 0x000000B4 */
   /** Datarate Control Register
       Controls the datarate of the various physical layers. The contents of the writeable fields of this register shall not be changed during operation. */
   unsigned int drc; /* 0x000000B8 */
   /** GMAC Multiplexer Control Register
       Controls the interconnect between GMACs and the various physical layers. All fields need to have a different content. If two GMACs are muxed to the same PHY unpredictable results may occur. The contents of this register shall not be changed during operation. */
   unsigned int gmuxc; /* 0x000000BC */
   /** Datarate Status Register
       Shows the datarate of the GMACs. The datarate of a GMAC is derived from the datarate of the physical layer it is multiplexed to. This register is for debugging only. */
   unsigned int drs; /* 0x000000C0 */
   /** SGMII Control Register */
   unsigned int sgmiic; /* 0x000000C4 */
   /** Reserved */
   unsigned int res_2[14]; /* 0x000000C8 */
};


/* Fields of "Clock Status Register" */
/** GPHY1MII2 Clock Enable
    Shows the clock enable bit for GPHY1MII2. */
#define SYS_ETH_CLKS_GPHY1MII2 0x02000000
/* Disable
#define SYS_ETH_CLKS_GPHY1MII2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GPHY1MII2_EN 0x02000000
/** GPHY0MII2 Clock Enable
    Shows the clock enable bit for GPHY0MII2. */
#define SYS_ETH_CLKS_GPHY0MII2 0x01000000
/* Disable
#define SYS_ETH_CLKS_GPHY0MII2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GPHY0MII2_EN 0x01000000
/** PADCTRL2 Clock Enable
    Shows the clock enable bit for the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_CLKS_PADCTRL2 0x00200000
/* Disable
#define SYS_ETH_CLKS_PADCTRL2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_PADCTRL2_EN 0x00200000
/** PADCTRL0 Clock Enable
    Shows the clock enable bit for the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_CLKS_PADCTRL0 0x00100000
/* Disable
#define SYS_ETH_CLKS_PADCTRL0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_PADCTRL0_EN 0x00100000
/** P2 Clock Enable
    Shows the clock enable bit for the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_CLKS_P2 0x00020000
/* Disable
#define SYS_ETH_CLKS_P2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_P2_EN 0x00020000
/** P0 Clock Enable
    Shows the clock enable bit for the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_CLKS_P0 0x00010000
/* Disable
#define SYS_ETH_CLKS_P0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_P0_EN 0x00010000
/** xMII Clock Enable
    Shows the clock enable bit for the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_CLKS_xMII 0x00000800
/* Disable
#define SYS_ETH_CLKS_xMII_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_xMII_EN 0x00000800
/** SGMII Clock Enable
    Shows the clock enable bit for the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKS_SGMII 0x00000400
/* Disable
#define SYS_ETH_CLKS_SGMII_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_SGMII_EN 0x00000400
/** GPHY1 Clock Enable
    Shows the clock enable bit for the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKS_GPHY1 0x00000200
/* Disable
#define SYS_ETH_CLKS_GPHY1_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GPHY1_EN 0x00000200
/** GPHY0 Clock Enable
    Shows the clock enable bit for the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKS_GPHY0 0x00000100
/* Disable
#define SYS_ETH_CLKS_GPHY0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GPHY0_EN 0x00000100
/** MDIO Clock Enable
    Shows the clock enable bit for the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_CLKS_MDIO 0x00000080
/* Disable
#define SYS_ETH_CLKS_MDIO_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_MDIO_EN 0x00000080
/** GMAC3 Clock Enable
    Shows the clock enable bit for the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_CLKS_GMAC3 0x00000008
/* Disable
#define SYS_ETH_CLKS_GMAC3_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GMAC3_EN 0x00000008
/** GMAC2 Clock Enable
    Shows the clock enable bit for the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_CLKS_GMAC2 0x00000004
/* Disable
#define SYS_ETH_CLKS_GMAC2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GMAC2_EN 0x00000004
/** GMAC1 Clock Enable
    Shows the clock enable bit for the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_CLKS_GMAC1 0x00000002
/* Disable
#define SYS_ETH_CLKS_GMAC1_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GMAC1_EN 0x00000002
/** GMAC0 Clock Enable
    Shows the clock enable bit for the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_CLKS_GMAC0 0x00000001
/* Disable
#define SYS_ETH_CLKS_GMAC0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_CLKS_GMAC0_EN 0x00000001

/* Fields of "Clock Enable Register" */
/** Set Clock Enable GPHY1MII2
    Sets the clock enable bit of the GPHY1MII2. */
#define SYS_ETH_CLKEN_GPHY1MII2 0x02000000
/* No-Operation
#define SYS_ETH_CLKEN_GPHY1MII2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GPHY1MII2_SET 0x02000000
/** Set Clock Enable GPHY0MII2
    Sets the clock enable bit of the GPHY0MII2. */
#define SYS_ETH_CLKEN_GPHY0MII2 0x01000000
/* No-Operation
#define SYS_ETH_CLKEN_GPHY0MII2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GPHY0MII2_SET 0x01000000
/** Set Clock Enable PADCTRL2
    Sets the clock enable bit of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_CLKEN_PADCTRL2 0x00200000
/* No-Operation
#define SYS_ETH_CLKEN_PADCTRL2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_PADCTRL2_SET 0x00200000
/** Set Clock Enable PADCTRL0
    Sets the clock enable bit of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_CLKEN_PADCTRL0 0x00100000
/* No-Operation
#define SYS_ETH_CLKEN_PADCTRL0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_PADCTRL0_SET 0x00100000
/** Set Clock Enable P2
    Sets the clock enable bit of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_CLKEN_P2 0x00020000
/* No-Operation
#define SYS_ETH_CLKEN_P2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_P2_SET 0x00020000
/** Set Clock Enable P0
    Sets the clock enable bit of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_CLKEN_P0 0x00010000
/* No-Operation
#define SYS_ETH_CLKEN_P0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_P0_SET 0x00010000
/** Set Clock Enable xMII
    Sets the clock enable bit of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_CLKEN_xMII 0x00000800
/* No-Operation
#define SYS_ETH_CLKEN_xMII_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_xMII_SET 0x00000800
/** Set Clock Enable SGMII
    Sets the clock enable bit of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKEN_SGMII 0x00000400
/* No-Operation
#define SYS_ETH_CLKEN_SGMII_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_SGMII_SET 0x00000400
/** Set Clock Enable GPHY1
    Sets the clock enable bit of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKEN_GPHY1 0x00000200
/* No-Operation
#define SYS_ETH_CLKEN_GPHY1_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GPHY1_SET 0x00000200
/** Set Clock Enable GPHY0
    Sets the clock enable bit of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKEN_GPHY0 0x00000100
/* No-Operation
#define SYS_ETH_CLKEN_GPHY0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GPHY0_SET 0x00000100
/** Set Clock Enable MDIO
    Sets the clock enable bit of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_CLKEN_MDIO 0x00000080
/* No-Operation
#define SYS_ETH_CLKEN_MDIO_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_MDIO_SET 0x00000080
/** Set Clock Enable GMAC3
    Sets the clock enable bit of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_CLKEN_GMAC3 0x00000008
/* No-Operation
#define SYS_ETH_CLKEN_GMAC3_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GMAC3_SET 0x00000008
/** Set Clock Enable GMAC2
    Sets the clock enable bit of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_CLKEN_GMAC2 0x00000004
/* No-Operation
#define SYS_ETH_CLKEN_GMAC2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GMAC2_SET 0x00000004
/** Set Clock Enable GMAC1
    Sets the clock enable bit of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_CLKEN_GMAC1 0x00000002
/* No-Operation
#define SYS_ETH_CLKEN_GMAC1_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GMAC1_SET 0x00000002
/** Set Clock Enable GMAC0
    Sets the clock enable bit of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_CLKEN_GMAC0 0x00000001
/* No-Operation
#define SYS_ETH_CLKEN_GMAC0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_CLKEN_GMAC0_SET 0x00000001

/* Fields of "Clock Clear Register" */
/** Clear Clock Enable GPHY1MII2
    Clears the clock enable bit of the GPHY1MII2. */
#define SYS_ETH_CLKCLR_GPHY1MII2 0x02000000
/* No-Operation
#define SYS_ETH_CLKCLR_GPHY1MII2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GPHY1MII2_CLR 0x02000000
/** Clear Clock Enable GPHY0MII2
    Clears the clock enable bit of the GPHY0MII2. */
#define SYS_ETH_CLKCLR_GPHY0MII2 0x01000000
/* No-Operation
#define SYS_ETH_CLKCLR_GPHY0MII2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GPHY0MII2_CLR 0x01000000
/** Clear Clock Enable PADCTRL2
    Clears the clock enable bit of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_CLKCLR_PADCTRL2 0x00200000
/* No-Operation
#define SYS_ETH_CLKCLR_PADCTRL2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_PADCTRL2_CLR 0x00200000
/** Clear Clock Enable PADCTRL0
    Clears the clock enable bit of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_CLKCLR_PADCTRL0 0x00100000
/* No-Operation
#define SYS_ETH_CLKCLR_PADCTRL0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_PADCTRL0_CLR 0x00100000
/** Clear Clock Enable P2
    Clears the clock enable bit of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_CLKCLR_P2 0x00020000
/* No-Operation
#define SYS_ETH_CLKCLR_P2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_P2_CLR 0x00020000
/** Clear Clock Enable P0
    Clears the clock enable bit of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_CLKCLR_P0 0x00010000
/* No-Operation
#define SYS_ETH_CLKCLR_P0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_P0_CLR 0x00010000
/** Clear Clock Enable xMII
    Clears the clock enable bit of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_CLKCLR_xMII 0x00000800
/* No-Operation
#define SYS_ETH_CLKCLR_xMII_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_xMII_CLR 0x00000800
/** Clear Clock Enable SGMII
    Clears the clock enable bit of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKCLR_SGMII 0x00000400
/* No-Operation
#define SYS_ETH_CLKCLR_SGMII_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_SGMII_CLR 0x00000400
/** Clear Clock Enable GPHY1
    Clears the clock enable bit of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKCLR_GPHY1 0x00000200
/* No-Operation
#define SYS_ETH_CLKCLR_GPHY1_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GPHY1_CLR 0x00000200
/** Clear Clock Enable GPHY0
    Clears the clock enable bit of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_CLKCLR_GPHY0 0x00000100
/* No-Operation
#define SYS_ETH_CLKCLR_GPHY0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GPHY0_CLR 0x00000100
/** Clear Clock Enable MDIO
    Clears the clock enable bit of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_CLKCLR_MDIO 0x00000080
/* No-Operation
#define SYS_ETH_CLKCLR_MDIO_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_MDIO_CLR 0x00000080
/** Clear Clock Enable GMAC3
    Clears the clock enable bit of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_CLKCLR_GMAC3 0x00000008
/* No-Operation
#define SYS_ETH_CLKCLR_GMAC3_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GMAC3_CLR 0x00000008
/** Clear Clock Enable GMAC2
    Clears the clock enable bit of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_CLKCLR_GMAC2 0x00000004
/* No-Operation
#define SYS_ETH_CLKCLR_GMAC2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GMAC2_CLR 0x00000004
/** Clear Clock Enable GMAC1
    Clears the clock enable bit of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_CLKCLR_GMAC1 0x00000002
/* No-Operation
#define SYS_ETH_CLKCLR_GMAC1_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GMAC1_CLR 0x00000002
/** Clear Clock Enable GMAC0
    Clears the clock enable bit of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_CLKCLR_GMAC0 0x00000001
/* No-Operation
#define SYS_ETH_CLKCLR_GMAC0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_CLKCLR_GMAC0_CLR 0x00000001

/* Fields of "Activation Status Register" */
/** PADCTRL2 Status
    Shows the activation status of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_ACTS_PADCTRL2 0x00200000
/* The block is inactive.
#define SYS_ETH_ACTS_PADCTRL2_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_PADCTRL2_ACT 0x00200000
/** PADCTRL0 Status
    Shows the activation status of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_ACTS_PADCTRL0 0x00100000
/* The block is inactive.
#define SYS_ETH_ACTS_PADCTRL0_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_PADCTRL0_ACT 0x00100000
/** P2 Status
    Shows the activation status of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_ACTS_P2 0x00020000
/* The block is inactive.
#define SYS_ETH_ACTS_P2_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_P2_ACT 0x00020000
/** P0 Status
    Shows the activation status of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_ACTS_P0 0x00010000
/* The block is inactive.
#define SYS_ETH_ACTS_P0_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_P0_ACT 0x00010000
/** xMII Status
    Shows the activation status of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_ACTS_xMII 0x00000800
/* The block is inactive.
#define SYS_ETH_ACTS_xMII_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_xMII_ACT 0x00000800
/** SGMII Status
    Shows the activation status of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACTS_SGMII 0x00000400
/* The block is inactive.
#define SYS_ETH_ACTS_SGMII_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_SGMII_ACT 0x00000400
/** GPHY1 Status
    Shows the activation status of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACTS_GPHY1 0x00000200
/* The block is inactive.
#define SYS_ETH_ACTS_GPHY1_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GPHY1_ACT 0x00000200
/** GPHY0 Status
    Shows the activation status of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACTS_GPHY0 0x00000100
/* The block is inactive.
#define SYS_ETH_ACTS_GPHY0_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GPHY0_ACT 0x00000100
/** MDIO Status
    Shows the activation status of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_ACTS_MDIO 0x00000080
/* The block is inactive.
#define SYS_ETH_ACTS_MDIO_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_MDIO_ACT 0x00000080
/** GMAC3 Status
    Shows the activation status of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_ACTS_GMAC3 0x00000008
/* The block is inactive.
#define SYS_ETH_ACTS_GMAC3_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GMAC3_ACT 0x00000008
/** GMAC2 Status
    Shows the activation status of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_ACTS_GMAC2 0x00000004
/* The block is inactive.
#define SYS_ETH_ACTS_GMAC2_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GMAC2_ACT 0x00000004
/** GMAC1 Status
    Shows the activation status of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_ACTS_GMAC1 0x00000002
/* The block is inactive.
#define SYS_ETH_ACTS_GMAC1_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GMAC1_ACT 0x00000002
/** GMAC0 Status
    Shows the activation status of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_ACTS_GMAC0 0x00000001
/* The block is inactive.
#define SYS_ETH_ACTS_GMAC0_INACT 0x00000000 */
/** The block is active. */
#define SYS_ETH_ACTS_GMAC0_ACT 0x00000001

/* Fields of "Activation Register" */
/** Activate PADCTRL2
    Sets the activation flag of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_ACT_PADCTRL2 0x00200000
/* No-Operation
#define SYS_ETH_ACT_PADCTRL2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_PADCTRL2_SET 0x00200000
/** Activate PADCTRL0
    Sets the activation flag of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_ACT_PADCTRL0 0x00100000
/* No-Operation
#define SYS_ETH_ACT_PADCTRL0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_PADCTRL0_SET 0x00100000
/** Activate P2
    Sets the activation flag of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_ACT_P2 0x00020000
/* No-Operation
#define SYS_ETH_ACT_P2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_P2_SET 0x00020000
/** Activate P0
    Sets the activation flag of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_ACT_P0 0x00010000
/* No-Operation
#define SYS_ETH_ACT_P0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_P0_SET 0x00010000
/** Activate xMII
    Sets the activation flag of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_ACT_xMII 0x00000800
/* No-Operation
#define SYS_ETH_ACT_xMII_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_xMII_SET 0x00000800
/** Activate SGMII
    Sets the activation flag of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACT_SGMII 0x00000400
/* No-Operation
#define SYS_ETH_ACT_SGMII_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_SGMII_SET 0x00000400
/** Activate GPHY1
    Sets the activation flag of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACT_GPHY1 0x00000200
/* No-Operation
#define SYS_ETH_ACT_GPHY1_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GPHY1_SET 0x00000200
/** Activate GPHY0
    Sets the activation flag of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_ACT_GPHY0 0x00000100
/* No-Operation
#define SYS_ETH_ACT_GPHY0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GPHY0_SET 0x00000100
/** Activate MDIO
    Sets the activation flag of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_ACT_MDIO 0x00000080
/* No-Operation
#define SYS_ETH_ACT_MDIO_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_MDIO_SET 0x00000080
/** Activate GMAC3
    Sets the activation flag of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_ACT_GMAC3 0x00000008
/* No-Operation
#define SYS_ETH_ACT_GMAC3_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GMAC3_SET 0x00000008
/** Activate GMAC2
    Sets the activation flag of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_ACT_GMAC2 0x00000004
/* No-Operation
#define SYS_ETH_ACT_GMAC2_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GMAC2_SET 0x00000004
/** Activate GMAC1
    Sets the activation flag of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_ACT_GMAC1 0x00000002
/* No-Operation
#define SYS_ETH_ACT_GMAC1_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GMAC1_SET 0x00000002
/** Activate GMAC0
    Sets the activation flag of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_ACT_GMAC0 0x00000001
/* No-Operation
#define SYS_ETH_ACT_GMAC0_NOP 0x00000000 */
/** Set */
#define SYS_ETH_ACT_GMAC0_SET 0x00000001

/* Fields of "Deactivation Register" */
/** Deactivate PADCTRL2
    Clears the activation flag of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_DEACT_PADCTRL2 0x00200000
/* No-Operation
#define SYS_ETH_DEACT_PADCTRL2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_PADCTRL2_CLR 0x00200000
/** Deactivate PADCTRL0
    Clears the activation flag of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_DEACT_PADCTRL0 0x00100000
/* No-Operation
#define SYS_ETH_DEACT_PADCTRL0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_PADCTRL0_CLR 0x00100000
/** Deactivate P2
    Clears the activation flag of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_DEACT_P2 0x00020000
/* No-Operation
#define SYS_ETH_DEACT_P2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_P2_CLR 0x00020000
/** Deactivate P0
    Clears the activation flag of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_DEACT_P0 0x00010000
/* No-Operation
#define SYS_ETH_DEACT_P0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_P0_CLR 0x00010000
/** Deactivate xMII
    Clears the activation flag of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_DEACT_xMII 0x00000800
/* No-Operation
#define SYS_ETH_DEACT_xMII_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_xMII_CLR 0x00000800
/** Deactivate SGMII
    Clears the activation flag of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_DEACT_SGMII 0x00000400
/* No-Operation
#define SYS_ETH_DEACT_SGMII_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_SGMII_CLR 0x00000400
/** Deactivate GPHY1
    Clears the activation flag of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_DEACT_GPHY1 0x00000200
/* No-Operation
#define SYS_ETH_DEACT_GPHY1_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GPHY1_CLR 0x00000200
/** Deactivate GPHY0
    Clears the activation flag of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_DEACT_GPHY0 0x00000100
/* No-Operation
#define SYS_ETH_DEACT_GPHY0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GPHY0_CLR 0x00000100
/** Deactivate MDIO
    Clears the activation flag of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_DEACT_MDIO 0x00000080
/* No-Operation
#define SYS_ETH_DEACT_MDIO_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_MDIO_CLR 0x00000080
/** Deactivate GMAC3
    Clears the activation flag of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_DEACT_GMAC3 0x00000008
/* No-Operation
#define SYS_ETH_DEACT_GMAC3_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GMAC3_CLR 0x00000008
/** Deactivate GMAC2
    Clears the activation flag of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_DEACT_GMAC2 0x00000004
/* No-Operation
#define SYS_ETH_DEACT_GMAC2_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GMAC2_CLR 0x00000004
/** Deactivate GMAC1
    Clears the activation flag of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_DEACT_GMAC1 0x00000002
/* No-Operation
#define SYS_ETH_DEACT_GMAC1_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GMAC1_CLR 0x00000002
/** Deactivate GMAC0
    Clears the activation flag of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_DEACT_GMAC0 0x00000001
/* No-Operation
#define SYS_ETH_DEACT_GMAC0_NOP 0x00000000 */
/** Clear */
#define SYS_ETH_DEACT_GMAC0_CLR 0x00000001

/* Fields of "Reboot Trigger Register" */
/** Reboot PADCTRL2
    Triggers a reboot of the PADCTRL2 domain. This domain contains the PADCTRL2 block. */
#define SYS_ETH_RBT_PADCTRL2 0x00200000
/* No-Operation
#define SYS_ETH_RBT_PADCTRL2_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_PADCTRL2_TRIG 0x00200000
/** Reboot PADCTRL0
    Triggers a reboot of the PADCTRL0 domain. This domain contains the PADCTRL0 block. */
#define SYS_ETH_RBT_PADCTRL0 0x00100000
/* No-Operation
#define SYS_ETH_RBT_PADCTRL0_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_PADCTRL0_TRIG 0x00100000
/** Reboot P2
    Triggers a reboot of the P2 domain. This domain contains the P2 instance of the GPIO block. */
#define SYS_ETH_RBT_P2 0x00020000
/* No-Operation
#define SYS_ETH_RBT_P2_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_P2_TRIG 0x00020000
/** Reboot P0
    Triggers a reboot of the P0 domain. This domain contains the P0 instance of the GPIO block. */
#define SYS_ETH_RBT_P0 0x00010000
/* No-Operation
#define SYS_ETH_RBT_P0_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_P0_TRIG 0x00010000
/** Reboot xMII
    Triggers a reboot of the xMII domain. This domain contains the XMII block. If any of the digital LAN interfaces shall be used, this domain has to be active. */
#define SYS_ETH_RBT_xMII 0x00000800
/* No-Operation
#define SYS_ETH_RBT_xMII_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_xMII_TRIG 0x00000800
/** Reboot SGMII
    Triggers a reboot of the SGMII domain. This domain contains all parts of the EIM related to the SGMII block. The SGMII block itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_RBT_SGMII 0x00000400
/* No-Operation
#define SYS_ETH_RBT_SGMII_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_SGMII_TRIG 0x00000400
/** Reboot GPHY1
    Triggers a reboot of the GPHY1 domain. This domain contains all parts of the EIM related to GPHY1. The GPHY1 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_RBT_GPHY1 0x00000200
/* No-Operation
#define SYS_ETH_RBT_GPHY1_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GPHY1_TRIG 0x00000200
/** Reboot GPHY0
    Triggers a reboot of the GPHY0 domain. This domain contains all parts of the EIM related to GPHY0. The GPHY0 itself is not contained, as it has its own clock/reset/power management. */
#define SYS_ETH_RBT_GPHY0 0x00000100
/* No-Operation
#define SYS_ETH_RBT_GPHY0_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GPHY0_TRIG 0x00000100
/** Reboot MDIO
    Triggers a reboot of the MDIO domain. This domain contains the MDIO block. */
#define SYS_ETH_RBT_MDIO 0x00000080
/* No-Operation
#define SYS_ETH_RBT_MDIO_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_MDIO_TRIG 0x00000080
/** Reboot GMAC3
    Triggers a reboot of the GMAC3 domain. This domain contains the GMAC3 block. */
#define SYS_ETH_RBT_GMAC3 0x00000008
/* No-Operation
#define SYS_ETH_RBT_GMAC3_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GMAC3_TRIG 0x00000008
/** Reboot GMAC2
    Triggers a reboot of the GMAC2 domain. This domain contains the GMAC2 block. */
#define SYS_ETH_RBT_GMAC2 0x00000004
/* No-Operation
#define SYS_ETH_RBT_GMAC2_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GMAC2_TRIG 0x00000004
/** Reboot GMAC1
    Triggers a reboot of the GMAC1 domain. This domain contains the GMAC1 block. */
#define SYS_ETH_RBT_GMAC1 0x00000002
/* No-Operation
#define SYS_ETH_RBT_GMAC1_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GMAC1_TRIG 0x00000002
/** Reboot GMAC0
    Triggers a reboot of the GMAC0 domain. This domain contains the GMAC0 block. */
#define SYS_ETH_RBT_GMAC0 0x00000001
/* No-Operation
#define SYS_ETH_RBT_GMAC0_NOP 0x00000000 */
/** Trigger */
#define SYS_ETH_RBT_GMAC0_TRIG 0x00000001

/* Fields of "External PHY Control Register" */
/** PHY_CLKO Output Enable
    Enables the output driver of the PHY_CLKO pin. */
#define SYS_ETH_EXTPHYC_CLKEN 0x80000000
/* Disable
#define SYS_ETH_EXTPHYC_CLKEN_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_EXTPHYC_CLKEN_EN 0x80000000
/** PHY_CLKO Frequency Select
    Selects the frequency of the PHY_CLKO pin. */
#define SYS_ETH_EXTPHYC_CLKSEL_MASK 0x00000007
/** field offset */
#define SYS_ETH_EXTPHYC_CLKSEL_OFFSET 0
/** 25 MHz. */
#define SYS_ETH_EXTPHYC_CLKSEL_F25 0x00000001
/** 125 MHz. */
#define SYS_ETH_EXTPHYC_CLKSEL_F125 0x00000002
/** 50 MHz. */
#define SYS_ETH_EXTPHYC_CLKSEL_F50 0x00000005

/* Fields of "Power Down Configuration Register" */
/** Enable Power Down PADCTRL2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_PADCTRL2 0x00200000
/* Disable
#define SYS_ETH_PDCFG_PADCTRL2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_PADCTRL2_EN 0x00200000
/** Enable Power Down PADCTRL0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_PADCTRL0 0x00100000
/* Disable
#define SYS_ETH_PDCFG_PADCTRL0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_PADCTRL0_EN 0x00100000
/** Enable Power Down P2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_P2 0x00020000
/* Disable
#define SYS_ETH_PDCFG_P2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_P2_EN 0x00020000
/** Enable Power Down P0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_P0 0x00010000
/* Disable
#define SYS_ETH_PDCFG_P0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_P0_EN 0x00010000
/** Enable Power Down xMII
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_xMII 0x00000800
/* Disable
#define SYS_ETH_PDCFG_xMII_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_xMII_EN 0x00000800
/** Enable Power Down SGMII
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_SGMII 0x00000400
/* Disable
#define SYS_ETH_PDCFG_SGMII_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_SGMII_EN 0x00000400
/** Enable Power Down GPHY1
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GPHY1 0x00000200
/* Disable
#define SYS_ETH_PDCFG_GPHY1_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GPHY1_EN 0x00000200
/** Enable Power Down GPHY0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GPHY0 0x00000100
/* Disable
#define SYS_ETH_PDCFG_GPHY0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GPHY0_EN 0x00000100
/** Enable Power Down MDIO
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_MDIO 0x00000080
/* Disable
#define SYS_ETH_PDCFG_MDIO_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_MDIO_EN 0x00000080
/** Enable Power Down GMAC3
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GMAC3 0x00000008
/* Disable
#define SYS_ETH_PDCFG_GMAC3_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GMAC3_EN 0x00000008
/** Enable Power Down GMAC2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GMAC2 0x00000004
/* Disable
#define SYS_ETH_PDCFG_GMAC2_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GMAC2_EN 0x00000004
/** Enable Power Down GMAC1
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GMAC1 0x00000002
/* Disable
#define SYS_ETH_PDCFG_GMAC1_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GMAC1_EN 0x00000002
/** Enable Power Down GMAC0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_ETH_PDCFG_GMAC0 0x00000001
/* Disable
#define SYS_ETH_PDCFG_GMAC0_DIS 0x00000000 */
/** Enable */
#define SYS_ETH_PDCFG_GMAC0_EN 0x00000001

/* Fields of "Datarate Control Register" */
/** MDC Clockrate
    Selects the clockrate of the MDIO interface. */
#define SYS_ETH_DRC_MDC_MASK 0x30000000
/** field offset */
#define SYS_ETH_DRC_MDC_OFFSET 28
/** 312.5/128 = appr. 2.44 MHz. */
#define SYS_ETH_DRC_MDC_F2M44 0x00000000
/** 312.5/64 = appr. 4.88 MHz. */
#define SYS_ETH_DRC_MDC_F4M88 0x10000000
/** 312.5/32 = appr. 9.77 MHz. */
#define SYS_ETH_DRC_MDC_F9M77 0x20000000
/** 312.5/16 = appr. 19.5 MHz. */
#define SYS_ETH_DRC_MDC_F19M5 0x30000000
/** xMII1 Datarate
    Selects the datarate of the xMII1 interface. */
#define SYS_ETH_DRC_xMII1_MASK 0x07000000
/** field offset */
#define SYS_ETH_DRC_xMII1_OFFSET 24
/** 10 MBit/s. */
#define SYS_ETH_DRC_xMII1_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_xMII1_DR100 0x01000000
/** 1000 MBit/s. */
#define SYS_ETH_DRC_xMII1_DR1000 0x02000000
/** 200 MBit/s. */
#define SYS_ETH_DRC_xMII1_DR200 0x05000000
/** xMII0 Datarate
    Selects the datarate of the xMII0 interface. */
#define SYS_ETH_DRC_xMII0_MASK 0x00700000
/** field offset */
#define SYS_ETH_DRC_xMII0_OFFSET 20
/** 10 MBit/s. */
#define SYS_ETH_DRC_xMII0_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_xMII0_DR100 0x00100000
/** 1000 MBit/s. */
#define SYS_ETH_DRC_xMII0_DR1000 0x00200000
/** 200 MBit/s. */
#define SYS_ETH_DRC_xMII0_DR200 0x00500000
/** SGMII Datarate
    Selects the datarate of the SGMII interface. */
#define SYS_ETH_DRC_SGMII_MASK 0x00070000
/** field offset */
#define SYS_ETH_DRC_SGMII_OFFSET 16
/** 10 MBit/s. */
#define SYS_ETH_DRC_SGMII_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_SGMII_DR100 0x00010000
/** 1000 MBit/s. */
#define SYS_ETH_DRC_SGMII_DR1000 0x00020000
/** 2500 MBit/s. */
#define SYS_ETH_DRC_SGMII_DR2500 0x00040000
/** GPHY1_MII2 Datarate
    Shows the datarate of the GPHY1_MII2 interface. */
#define SYS_ETH_DRC_GPHY1_MII2_MASK 0x00007000
/** field offset */
#define SYS_ETH_DRC_GPHY1_MII2_OFFSET 12
/** 10 MBit/s. */
#define SYS_ETH_DRC_GPHY1_MII2_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_GPHY1_MII2_DR100 0x00001000
/** GPHY1_GMII Datarate
    Shows the datarate of the GPHY1_GMII interface. */
#define SYS_ETH_DRC_GPHY1_GMII_MASK 0x00000700
/** field offset */
#define SYS_ETH_DRC_GPHY1_GMII_OFFSET 8
/** 10 MBit/s. */
#define SYS_ETH_DRC_GPHY1_GMII_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_GPHY1_GMII_DR100 0x00000100
/** 1000 MBit/s. */
#define SYS_ETH_DRC_GPHY1_GMII_DR1000 0x00000200
/** GPHY0_MII2 Datarate
    Shows the datarate of the GPHY0_MII2 interface. */
#define SYS_ETH_DRC_GPHY0_MII2_MASK 0x00000070
/** field offset */
#define SYS_ETH_DRC_GPHY0_MII2_OFFSET 4
/** 10 MBit/s. */
#define SYS_ETH_DRC_GPHY0_MII2_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_GPHY0_MII2_DR100 0x00000010
/** GPHY0_GMII Datarate
    Shows the datarate of the GPHY0_GMII interface. */
#define SYS_ETH_DRC_GPHY0_GMII_MASK 0x00000007
/** field offset */
#define SYS_ETH_DRC_GPHY0_GMII_OFFSET 0
/** 10 MBit/s. */
#define SYS_ETH_DRC_GPHY0_GMII_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRC_GPHY0_GMII_DR100 0x00000001
/** 1000 MBit/s. */
#define SYS_ETH_DRC_GPHY0_GMII_DR1000 0x00000002

/* Fields of "GMAC Multiplexer Control Register" */
/** GMAC 3 MUX setting
    Selects the physical layer to be connected to GMAC3 */
#define SYS_ETH_GMUXC_GMAC3_MASK 0x00007000
/** field offset */
#define SYS_ETH_GMUXC_GMAC3_OFFSET 12
/** GMAC connects to GPHY0_GMII interface */
#define SYS_ETH_GMUXC_GMAC3_GPHY0_GMII 0x00000000
/** GMAC connects to GPHY0_MII2 interface */
#define SYS_ETH_GMUXC_GMAC3_GPHY0_MII2 0x00001000
/** GMAC connects to GPHY1_GMII interface */
#define SYS_ETH_GMUXC_GMAC3_GPHY1_GMII 0x00002000
/** GMAC connects to GPHY1_MII2 interface */
#define SYS_ETH_GMUXC_GMAC3_GPHY1_MII2 0x00003000
/** GMAC connects to SGMII interface */
#define SYS_ETH_GMUXC_GMAC3_SGMII 0x00004000
/** GMAC connects to xMII0 interface */
#define SYS_ETH_GMUXC_GMAC3_xMII0 0x00005000
/** GMAC connects to xMII1 interface */
#define SYS_ETH_GMUXC_GMAC3_xMII1 0x00006000
/** GMAC 2 MUX setting
    Selects the physical layer to be connected to GMAC2 */
#define SYS_ETH_GMUXC_GMAC2_MASK 0x00000700
/** field offset */
#define SYS_ETH_GMUXC_GMAC2_OFFSET 8
/** GMAC connects to GPHY0_GMII interface */
#define SYS_ETH_GMUXC_GMAC2_GPHY0_GMII 0x00000000
/** GMAC connects to GPHY0_MII2 interface */
#define SYS_ETH_GMUXC_GMAC2_GPHY0_MII2 0x00000100
/** GMAC connects to GPHY1_GMII interface */
#define SYS_ETH_GMUXC_GMAC2_GPHY1_GMII 0x00000200
/** GMAC connects to GPHY1_MII2 interface */
#define SYS_ETH_GMUXC_GMAC2_GPHY1_MII2 0x00000300
/** GMAC connects to SGMII interface */
#define SYS_ETH_GMUXC_GMAC2_SGMII 0x00000400
/** GMAC connects to xMII0 interface */
#define SYS_ETH_GMUXC_GMAC2_xMII0 0x00000500
/** GMAC connects to xMII1 interface */
#define SYS_ETH_GMUXC_GMAC2_xMII1 0x00000600
/** GMAC 1 MUX setting
    Selects the physical layer to be connected to GMAC1 */
#define SYS_ETH_GMUXC_GMAC1_MASK 0x00000070
/** field offset */
#define SYS_ETH_GMUXC_GMAC1_OFFSET 4
/** GMAC connects to GPHY0_GMII interface */
#define SYS_ETH_GMUXC_GMAC1_GPHY0_GMII 0x00000000
/** GMAC connects to GPHY0_MII2 interface */
#define SYS_ETH_GMUXC_GMAC1_GPHY0_MII2 0x00000010
/** GMAC connects to GPHY1_GMII interface */
#define SYS_ETH_GMUXC_GMAC1_GPHY1_GMII 0x00000020
/** GMAC connects to GPHY1_MII2 interface */
#define SYS_ETH_GMUXC_GMAC1_GPHY1_MII2 0x00000030
/** GMAC connects to SGMII interface */
#define SYS_ETH_GMUXC_GMAC1_SGMII 0x00000040
/** GMAC connects to xMII0 interface */
#define SYS_ETH_GMUXC_GMAC1_xMII0 0x00000050
/** GMAC connects to xMII1 interface */
#define SYS_ETH_GMUXC_GMAC1_xMII1 0x00000060
/** GMAC 0 MUX setting
    Selects the physical layer to be connected to GMAC0 */
#define SYS_ETH_GMUXC_GMAC0_MASK 0x00000007
/** field offset */
#define SYS_ETH_GMUXC_GMAC0_OFFSET 0
/** GMAC connects to GPHY0_GMII interface */
#define SYS_ETH_GMUXC_GMAC0_GPHY0_GMII 0x00000000
/** GMAC connects to GPHY0_MII2 interface */
#define SYS_ETH_GMUXC_GMAC0_GPHY0_MII2 0x00000001
/** GMAC connects to GPHY1_GMII interface */
#define SYS_ETH_GMUXC_GMAC0_GPHY1_GMII 0x00000002
/** GMAC connects to GPHY1_MII2 interface */
#define SYS_ETH_GMUXC_GMAC0_GPHY1_MII2 0x00000003
/** GMAC connects to SGMII interface */
#define SYS_ETH_GMUXC_GMAC0_SGMII 0x00000004
/** GMAC connects to xMII0 interface */
#define SYS_ETH_GMUXC_GMAC0_xMII0 0x00000005
/** GMAC connects to xMII1 interface */
#define SYS_ETH_GMUXC_GMAC0_xMII1 0x00000006

/* Fields of "Datarate Status Register" */
/** GMAC 3 datarate
    Shows the datarate of GMAC3 */
#define SYS_ETH_DRS_GMAC3_MASK 0x00007000
/** field offset */
#define SYS_ETH_DRS_GMAC3_OFFSET 12
/** 10 MBit/s. */
#define SYS_ETH_DRS_GMAC3_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRS_GMAC3_DR100 0x00001000
/** 1000 MBit/s. */
#define SYS_ETH_DRS_GMAC3_DR1000 0x00002000
/** 2500 MBit/s. */
#define SYS_ETH_DRS_GMAC3_DR2500 0x00004000
/** 200 MBit/s. */
#define SYS_ETH_DRS_GMAC3_DR200 0x00005000
/** GMAC 2 datarate
    Shows the datarate of GMAC2 */
#define SYS_ETH_DRS_GMAC2_MASK 0x00000700
/** field offset */
#define SYS_ETH_DRS_GMAC2_OFFSET 8
/** 10 MBit/s. */
#define SYS_ETH_DRS_GMAC2_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRS_GMAC2_DR100 0x00000100
/** 1000 MBit/s. */
#define SYS_ETH_DRS_GMAC2_DR1000 0x00000200
/** 2500 MBit/s. */
#define SYS_ETH_DRS_GMAC2_DR2500 0x00000400
/** 200 MBit/s. */
#define SYS_ETH_DRS_GMAC2_DR200 0x00000500
/** GMAC 1 datarate
    Shows the datarate of GMAC1 */
#define SYS_ETH_DRS_GMAC1_MASK 0x00000070
/** field offset */
#define SYS_ETH_DRS_GMAC1_OFFSET 4
/** 10 MBit/s. */
#define SYS_ETH_DRS_GMAC1_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRS_GMAC1_DR100 0x00000010
/** 1000 MBit/s. */
#define SYS_ETH_DRS_GMAC1_DR1000 0x00000020
/** 2500 MBit/s. */
#define SYS_ETH_DRS_GMAC1_DR2500 0x00000040
/** 200 MBit/s. */
#define SYS_ETH_DRS_GMAC1_DR200 0x00000050
/** GMAC 0 datarate
    Shows the datarate of GMAC0 */
#define SYS_ETH_DRS_GMAC0_MASK 0x00000007
/** field offset */
#define SYS_ETH_DRS_GMAC0_OFFSET 0
/** 10 MBit/s. */
#define SYS_ETH_DRS_GMAC0_DR10 0x00000000
/** 100 MBit/s. */
#define SYS_ETH_DRS_GMAC0_DR100 0x00000001
/** 1000 MBit/s. */
#define SYS_ETH_DRS_GMAC0_DR1000 0x00000002
/** 2500 MBit/s. */
#define SYS_ETH_DRS_GMAC0_DR2500 0x00000004
/** 200 MBit/s. */
#define SYS_ETH_DRS_GMAC0_DR200 0x00000005

/* Fields of "SGMII Control Register" */
/** Auto Negotiation Protocol
    Selects the TBX/SGMII mode for the autonegotiation of the SGMII interface. */
#define SYS_ETH_SGMIIC_ANP 0x00000002
/* TBX Mode (IEEE 802.3 Clause 37 ANEG)
#define SYS_ETH_SGMIIC_ANP_TBXM 0x00000000 */
/** SGMII Mode (Cisco Aneg) */
#define SYS_ETH_SGMIIC_ANP_SGMIIM 0x00000002
/** Auto Negotiation MAC/PHY
    Selects the MAC/PHY mode for the autonegotiation of the SGMII interface. */
#define SYS_ETH_SGMIIC_ANMP 0x00000001
/* MAC Mode
#define SYS_ETH_SGMIIC_ANMP_MAC 0x00000000 */
/** PHY Mode */
#define SYS_ETH_SGMIIC_ANMP_PHY 0x00000001

/*! @} */ /* SYS_ETH_REGISTER */

#endif /* _sys_eth_reg_h */
