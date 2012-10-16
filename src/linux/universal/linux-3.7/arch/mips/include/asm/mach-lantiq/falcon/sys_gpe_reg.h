/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _sys_gpe_reg_h
#define _sys_gpe_reg_h

/** \addtogroup SYS_GPE_REGISTER
   @{
*/
/* access macros */
#define sys_gpe_r32(reg) reg_r32(&sys_gpe->reg)
#define sys_gpe_w32(val, reg) reg_w32(val, &sys_gpe->reg)
#define sys_gpe_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &sys_gpe->reg)
#define sys_gpe_r32_table(reg, idx) reg_r32_table(sys_gpe->reg, idx)
#define sys_gpe_w32_table(val, reg, idx) reg_w32_table(val, sys_gpe->reg, idx)
#define sys_gpe_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, sys_gpe->reg, idx)
#define sys_gpe_adr_table(reg, idx) adr_table(sys_gpe->reg, idx)


/** SYS_GPE register structure */
struct gpon_reg_sys_gpe
{
   /** Clock Status Register
       The clock status reflects the actual clocking mode as a function of the SW settings and the hardware sleep mode. */
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
   unsigned int res_1[33]; /* 0x00000030 */
   /** Power Down Configuration Register
       Via this register the configuration is done whether in case of deactivation the power supply of the domain shall be removed. */
   unsigned int pdcfg; /* 0x000000B4 */
   /** Sleep Source Configuration Register
       All sleep/wakeup conditions selected in this register contribute to the generation of the hardware sleep/wakeup request. Unselected conditions are ignored for sleep and wakeup. If no bit is selected, HW sleep is disabled. */
   unsigned int sscfg; /* 0x000000B8 */
   /** Sleep Source Timer Register */
   unsigned int sst; /* 0x000000BC */
   /** Sleep Destination Status Register
       Shows the status of the sleep destination vector. All clock domains selected in this register will be shutoff in case of a hardware sleep request. These clocks will be automatically reenabled in case of a hardware wakeup request. */
   unsigned int sds; /* 0x000000C0 */
   /** Sleep Destination Set Register
       Via this register the the domains to be shutoff in case of a hardware sleep request can be selected. */
   unsigned int sdset; /* 0x000000C4 */
   /** Sleep Destination Clear Register
       Via this register the the domains to be shutoff in case of a hardware sleep request can be deselected. */
   unsigned int sdclr; /* 0x000000C8 */
   /** Reserved */
   unsigned int res_2[9]; /* 0x000000CC */
   /** IRNCS Capture Register
       This register shows the currently active interrupt events masked with the corresponding enable bits of the IRNCSEN register. The interrupts can be acknowledged by a write operation. */
   unsigned int irncscr; /* 0x000000F0 */
   /** IRNCS Interrupt Control Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int irncsicr; /* 0x000000F4 */
   /** IRNCS Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IRNCSCR register and are not signalled via the interrupt line towards the controller. */
   unsigned int irncsen; /* 0x000000F8 */
   /** Reserved */
   unsigned int res_3; /* 0x000000FC */
};


/* Fields of "Clock Status Register" */
/** COP7 Clock Enable
    Shows the clock enable bit for the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_CLKS_COP7 0x80000000
/* Disable
#define SYS_GPE_CLKS_COP7_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP7_EN 0x80000000
/** COP6 Clock Enable
    Shows the clock enable bit for the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_CLKS_COP6 0x40000000
/* Disable
#define SYS_GPE_CLKS_COP6_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP6_EN 0x40000000
/** COP5 Clock Enable
    Shows the clock enable bit for the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_CLKS_COP5 0x20000000
/* Disable
#define SYS_GPE_CLKS_COP5_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP5_EN 0x20000000
/** COP4 Clock Enable
    Shows the clock enable bit for the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_CLKS_COP4 0x10000000
/* Disable
#define SYS_GPE_CLKS_COP4_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP4_EN 0x10000000
/** COP3 Clock Enable
    Shows the clock enable bit for the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_CLKS_COP3 0x08000000
/* Disable
#define SYS_GPE_CLKS_COP3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP3_EN 0x08000000
/** COP2 Clock Enable
    Shows the clock enable bit for the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_CLKS_COP2 0x04000000
/* Disable
#define SYS_GPE_CLKS_COP2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP2_EN 0x04000000
/** COP1 Clock Enable
    Shows the clock enable bit for the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_CLKS_COP1 0x02000000
/* Disable
#define SYS_GPE_CLKS_COP1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP1_EN 0x02000000
/** COP0 Clock Enable
    Shows the clock enable bit for the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_CLKS_COP0 0x01000000
/* Disable
#define SYS_GPE_CLKS_COP0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_COP0_EN 0x01000000
/** PE5 Clock Enable
    Shows the clock enable bit for the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_CLKS_PE5 0x00200000
/* Disable
#define SYS_GPE_CLKS_PE5_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE5_EN 0x00200000
/** PE4 Clock Enable
    Shows the clock enable bit for the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_CLKS_PE4 0x00100000
/* Disable
#define SYS_GPE_CLKS_PE4_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE4_EN 0x00100000
/** PE3 Clock Enable
    Shows the clock enable bit for the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_CLKS_PE3 0x00080000
/* Disable
#define SYS_GPE_CLKS_PE3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE3_EN 0x00080000
/** PE2 Clock Enable
    Shows the clock enable bit for the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_CLKS_PE2 0x00040000
/* Disable
#define SYS_GPE_CLKS_PE2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE2_EN 0x00040000
/** PE1 Clock Enable
    Shows the clock enable bit for the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_CLKS_PE1 0x00020000
/* Disable
#define SYS_GPE_CLKS_PE1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE1_EN 0x00020000
/** PE0 Clock Enable
    Shows the clock enable bit for the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_CLKS_PE0 0x00010000
/* Disable
#define SYS_GPE_CLKS_PE0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_PE0_EN 0x00010000
/** ARB Clock Enable
    Shows the clock enable bit for the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_CLKS_ARB 0x00002000
/* Disable
#define SYS_GPE_CLKS_ARB_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_ARB_EN 0x00002000
/** FSQM Clock Enable
    Shows the clock enable bit for the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_CLKS_FSQM 0x00001000
/* Disable
#define SYS_GPE_CLKS_FSQM_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_FSQM_EN 0x00001000
/** TMU Clock Enable
    Shows the clock enable bit for the TMU domain. This domain contains the TMU. */
#define SYS_GPE_CLKS_TMU 0x00000800
/* Disable
#define SYS_GPE_CLKS_TMU_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_TMU_EN 0x00000800
/** MRG Clock Enable
    Shows the clock enable bit for the MRG domain. This domain contains the Merger. */
#define SYS_GPE_CLKS_MRG 0x00000400
/* Disable
#define SYS_GPE_CLKS_MRG_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_MRG_EN 0x00000400
/** DISP Clock Enable
    Shows the clock enable bit for the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_CLKS_DISP 0x00000200
/* Disable
#define SYS_GPE_CLKS_DISP_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_DISP_EN 0x00000200
/** IQM Clock Enable
    Shows the clock enable bit for the IQM domain. This domain contains the IQM. */
#define SYS_GPE_CLKS_IQM 0x00000100
/* Disable
#define SYS_GPE_CLKS_IQM_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_IQM_EN 0x00000100
/** CPUE Clock Enable
    Shows the clock enable bit for the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_CLKS_CPUE 0x00000080
/* Disable
#define SYS_GPE_CLKS_CPUE_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_CPUE_EN 0x00000080
/** CPUI Clock Enable
    Shows the clock enable bit for the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_CLKS_CPUI 0x00000040
/* Disable
#define SYS_GPE_CLKS_CPUI_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_CPUI_EN 0x00000040
/** GPONE Clock Enable
    Shows the clock enable bit for the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_CLKS_GPONE 0x00000020
/* Disable
#define SYS_GPE_CLKS_GPONE_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_GPONE_EN 0x00000020
/** GPONI Clock Enable
    Shows the clock enable bit for the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_CLKS_GPONI 0x00000010
/* Disable
#define SYS_GPE_CLKS_GPONI_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_GPONI_EN 0x00000010
/** LAN3 Clock Enable
    Shows the clock enable bit for the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_CLKS_LAN3 0x00000008
/* Disable
#define SYS_GPE_CLKS_LAN3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_LAN3_EN 0x00000008
/** LAN2 Clock Enable
    Shows the clock enable bit for the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_CLKS_LAN2 0x00000004
/* Disable
#define SYS_GPE_CLKS_LAN2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_LAN2_EN 0x00000004
/** LAN1 Clock Enable
    Shows the clock enable bit for the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_CLKS_LAN1 0x00000002
/* Disable
#define SYS_GPE_CLKS_LAN1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_LAN1_EN 0x00000002
/** LAN0 Clock Enable
    Shows the clock enable bit for the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_CLKS_LAN0 0x00000001
/* Disable
#define SYS_GPE_CLKS_LAN0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_CLKS_LAN0_EN 0x00000001

/* Fields of "Clock Enable Register" */
/** Set Clock Enable COP7
    Sets the clock enable bit of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_CLKEN_COP7 0x80000000
/* No-Operation
#define SYS_GPE_CLKEN_COP7_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP7_SET 0x80000000
/** Set Clock Enable COP6
    Sets the clock enable bit of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_CLKEN_COP6 0x40000000
/* No-Operation
#define SYS_GPE_CLKEN_COP6_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP6_SET 0x40000000
/** Set Clock Enable COP5
    Sets the clock enable bit of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_CLKEN_COP5 0x20000000
/* No-Operation
#define SYS_GPE_CLKEN_COP5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP5_SET 0x20000000
/** Set Clock Enable COP4
    Sets the clock enable bit of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_CLKEN_COP4 0x10000000
/* No-Operation
#define SYS_GPE_CLKEN_COP4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP4_SET 0x10000000
/** Set Clock Enable COP3
    Sets the clock enable bit of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_CLKEN_COP3 0x08000000
/* No-Operation
#define SYS_GPE_CLKEN_COP3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP3_SET 0x08000000
/** Set Clock Enable COP2
    Sets the clock enable bit of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_CLKEN_COP2 0x04000000
/* No-Operation
#define SYS_GPE_CLKEN_COP2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP2_SET 0x04000000
/** Set Clock Enable COP1
    Sets the clock enable bit of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_CLKEN_COP1 0x02000000
/* No-Operation
#define SYS_GPE_CLKEN_COP1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP1_SET 0x02000000
/** Set Clock Enable COP0
    Sets the clock enable bit of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_CLKEN_COP0 0x01000000
/* No-Operation
#define SYS_GPE_CLKEN_COP0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_COP0_SET 0x01000000
/** Set Clock Enable PE5
    Sets the clock enable bit of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_CLKEN_PE5 0x00200000
/* No-Operation
#define SYS_GPE_CLKEN_PE5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE5_SET 0x00200000
/** Set Clock Enable PE4
    Sets the clock enable bit of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_CLKEN_PE4 0x00100000
/* No-Operation
#define SYS_GPE_CLKEN_PE4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE4_SET 0x00100000
/** Set Clock Enable PE3
    Sets the clock enable bit of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_CLKEN_PE3 0x00080000
/* No-Operation
#define SYS_GPE_CLKEN_PE3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE3_SET 0x00080000
/** Set Clock Enable PE2
    Sets the clock enable bit of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_CLKEN_PE2 0x00040000
/* No-Operation
#define SYS_GPE_CLKEN_PE2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE2_SET 0x00040000
/** Set Clock Enable PE1
    Sets the clock enable bit of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_CLKEN_PE1 0x00020000
/* No-Operation
#define SYS_GPE_CLKEN_PE1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE1_SET 0x00020000
/** Set Clock Enable PE0
    Sets the clock enable bit of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_CLKEN_PE0 0x00010000
/* No-Operation
#define SYS_GPE_CLKEN_PE0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_PE0_SET 0x00010000
/** Set Clock Enable ARB
    Sets the clock enable bit of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_CLKEN_ARB 0x00002000
/* No-Operation
#define SYS_GPE_CLKEN_ARB_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_ARB_SET 0x00002000
/** Set Clock Enable FSQM
    Sets the clock enable bit of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_CLKEN_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_CLKEN_FSQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_FSQM_SET 0x00001000
/** Set Clock Enable TMU
    Sets the clock enable bit of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_CLKEN_TMU 0x00000800
/* No-Operation
#define SYS_GPE_CLKEN_TMU_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_TMU_SET 0x00000800
/** Set Clock Enable MRG
    Sets the clock enable bit of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_CLKEN_MRG 0x00000400
/* No-Operation
#define SYS_GPE_CLKEN_MRG_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_MRG_SET 0x00000400
/** Set Clock Enable DISP
    Sets the clock enable bit of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_CLKEN_DISP 0x00000200
/* No-Operation
#define SYS_GPE_CLKEN_DISP_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_DISP_SET 0x00000200
/** Set Clock Enable IQM
    Sets the clock enable bit of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_CLKEN_IQM 0x00000100
/* No-Operation
#define SYS_GPE_CLKEN_IQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_IQM_SET 0x00000100
/** Set Clock Enable CPUE
    Sets the clock enable bit of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_CLKEN_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_CLKEN_CPUE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_CPUE_SET 0x00000080
/** Set Clock Enable CPUI
    Sets the clock enable bit of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_CLKEN_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_CLKEN_CPUI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_CPUI_SET 0x00000040
/** Set Clock Enable GPONE
    Sets the clock enable bit of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_CLKEN_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_CLKEN_GPONE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_GPONE_SET 0x00000020
/** Set Clock Enable GPONI
    Sets the clock enable bit of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_CLKEN_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_CLKEN_GPONI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_GPONI_SET 0x00000010
/** Set Clock Enable LAN3
    Sets the clock enable bit of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_CLKEN_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_CLKEN_LAN3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_LAN3_SET 0x00000008
/** Set Clock Enable LAN2
    Sets the clock enable bit of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_CLKEN_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_CLKEN_LAN2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_LAN2_SET 0x00000004
/** Set Clock Enable LAN1
    Sets the clock enable bit of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_CLKEN_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_CLKEN_LAN1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_LAN1_SET 0x00000002
/** Set Clock Enable LAN0
    Sets the clock enable bit of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_CLKEN_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_CLKEN_LAN0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_CLKEN_LAN0_SET 0x00000001

/* Fields of "Clock Clear Register" */
/** Clear Clock Enable COP7
    Clears the clock enable bit of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_CLKCLR_COP7 0x80000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP7_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP7_CLR 0x80000000
/** Clear Clock Enable COP6
    Clears the clock enable bit of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_CLKCLR_COP6 0x40000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP6_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP6_CLR 0x40000000
/** Clear Clock Enable COP5
    Clears the clock enable bit of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_CLKCLR_COP5 0x20000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP5_CLR 0x20000000
/** Clear Clock Enable COP4
    Clears the clock enable bit of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_CLKCLR_COP4 0x10000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP4_CLR 0x10000000
/** Clear Clock Enable COP3
    Clears the clock enable bit of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_CLKCLR_COP3 0x08000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP3_CLR 0x08000000
/** Clear Clock Enable COP2
    Clears the clock enable bit of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_CLKCLR_COP2 0x04000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP2_CLR 0x04000000
/** Clear Clock Enable COP1
    Clears the clock enable bit of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_CLKCLR_COP1 0x02000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP1_CLR 0x02000000
/** Clear Clock Enable COP0
    Clears the clock enable bit of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_CLKCLR_COP0 0x01000000
/* No-Operation
#define SYS_GPE_CLKCLR_COP0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_COP0_CLR 0x01000000
/** Clear Clock Enable PE5
    Clears the clock enable bit of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_CLKCLR_PE5 0x00200000
/* No-Operation
#define SYS_GPE_CLKCLR_PE5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE5_CLR 0x00200000
/** Clear Clock Enable PE4
    Clears the clock enable bit of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_CLKCLR_PE4 0x00100000
/* No-Operation
#define SYS_GPE_CLKCLR_PE4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE4_CLR 0x00100000
/** Clear Clock Enable PE3
    Clears the clock enable bit of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_CLKCLR_PE3 0x00080000
/* No-Operation
#define SYS_GPE_CLKCLR_PE3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE3_CLR 0x00080000
/** Clear Clock Enable PE2
    Clears the clock enable bit of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_CLKCLR_PE2 0x00040000
/* No-Operation
#define SYS_GPE_CLKCLR_PE2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE2_CLR 0x00040000
/** Clear Clock Enable PE1
    Clears the clock enable bit of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_CLKCLR_PE1 0x00020000
/* No-Operation
#define SYS_GPE_CLKCLR_PE1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE1_CLR 0x00020000
/** Clear Clock Enable PE0
    Clears the clock enable bit of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_CLKCLR_PE0 0x00010000
/* No-Operation
#define SYS_GPE_CLKCLR_PE0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_PE0_CLR 0x00010000
/** Clear Clock Enable ARB
    Clears the clock enable bit of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_CLKCLR_ARB 0x00002000
/* No-Operation
#define SYS_GPE_CLKCLR_ARB_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_ARB_CLR 0x00002000
/** Clear Clock Enable FSQM
    Clears the clock enable bit of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_CLKCLR_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_CLKCLR_FSQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_FSQM_CLR 0x00001000
/** Clear Clock Enable TMU
    Clears the clock enable bit of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_CLKCLR_TMU 0x00000800
/* No-Operation
#define SYS_GPE_CLKCLR_TMU_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_TMU_CLR 0x00000800
/** Clear Clock Enable MRG
    Clears the clock enable bit of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_CLKCLR_MRG 0x00000400
/* No-Operation
#define SYS_GPE_CLKCLR_MRG_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_MRG_CLR 0x00000400
/** Clear Clock Enable DISP
    Clears the clock enable bit of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_CLKCLR_DISP 0x00000200
/* No-Operation
#define SYS_GPE_CLKCLR_DISP_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_DISP_CLR 0x00000200
/** Clear Clock Enable IQM
    Clears the clock enable bit of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_CLKCLR_IQM 0x00000100
/* No-Operation
#define SYS_GPE_CLKCLR_IQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_IQM_CLR 0x00000100
/** Clear Clock Enable CPUE
    Clears the clock enable bit of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_CLKCLR_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_CLKCLR_CPUE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_CPUE_CLR 0x00000080
/** Clear Clock Enable CPUI
    Clears the clock enable bit of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_CLKCLR_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_CLKCLR_CPUI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_CPUI_CLR 0x00000040
/** Clear Clock Enable GPONE
    Clears the clock enable bit of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_CLKCLR_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_CLKCLR_GPONE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_GPONE_CLR 0x00000020
/** Clear Clock Enable GPONI
    Clears the clock enable bit of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_CLKCLR_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_CLKCLR_GPONI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_GPONI_CLR 0x00000010
/** Clear Clock Enable LAN3
    Clears the clock enable bit of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_CLKCLR_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_CLKCLR_LAN3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_LAN3_CLR 0x00000008
/** Clear Clock Enable LAN2
    Clears the clock enable bit of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_CLKCLR_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_CLKCLR_LAN2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_LAN2_CLR 0x00000004
/** Clear Clock Enable LAN1
    Clears the clock enable bit of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_CLKCLR_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_CLKCLR_LAN1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_LAN1_CLR 0x00000002
/** Clear Clock Enable LAN0
    Clears the clock enable bit of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_CLKCLR_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_CLKCLR_LAN0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_CLKCLR_LAN0_CLR 0x00000001

/* Fields of "Activation Status Register" */
/** COP7 Status
    Shows the activation status of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_ACTS_COP7 0x80000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP7_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP7_ACT 0x80000000
/** COP6 Status
    Shows the activation status of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_ACTS_COP6 0x40000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP6_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP6_ACT 0x40000000
/** COP5 Status
    Shows the activation status of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_ACTS_COP5 0x20000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP5_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP5_ACT 0x20000000
/** COP4 Status
    Shows the activation status of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_ACTS_COP4 0x10000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP4_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP4_ACT 0x10000000
/** COP3 Status
    Shows the activation status of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_ACTS_COP3 0x08000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP3_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP3_ACT 0x08000000
/** COP2 Status
    Shows the activation status of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_ACTS_COP2 0x04000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP2_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP2_ACT 0x04000000
/** COP1 Status
    Shows the activation status of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_ACTS_COP1 0x02000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP1_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP1_ACT 0x02000000
/** COP0 Status
    Shows the activation status of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_ACTS_COP0 0x01000000
/* The block is inactive.
#define SYS_GPE_ACTS_COP0_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_COP0_ACT 0x01000000
/** PE5 Status
    Shows the activation status of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_ACTS_PE5 0x00200000
/* The block is inactive.
#define SYS_GPE_ACTS_PE5_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE5_ACT 0x00200000
/** PE4 Status
    Shows the activation status of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_ACTS_PE4 0x00100000
/* The block is inactive.
#define SYS_GPE_ACTS_PE4_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE4_ACT 0x00100000
/** PE3 Status
    Shows the activation status of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_ACTS_PE3 0x00080000
/* The block is inactive.
#define SYS_GPE_ACTS_PE3_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE3_ACT 0x00080000
/** PE2 Status
    Shows the activation status of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_ACTS_PE2 0x00040000
/* The block is inactive.
#define SYS_GPE_ACTS_PE2_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE2_ACT 0x00040000
/** PE1 Status
    Shows the activation status of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_ACTS_PE1 0x00020000
/* The block is inactive.
#define SYS_GPE_ACTS_PE1_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE1_ACT 0x00020000
/** PE0 Status
    Shows the activation status of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_ACTS_PE0 0x00010000
/* The block is inactive.
#define SYS_GPE_ACTS_PE0_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_PE0_ACT 0x00010000
/** ARB Status
    Shows the activation status of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_ACTS_ARB 0x00002000
/* The block is inactive.
#define SYS_GPE_ACTS_ARB_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_ARB_ACT 0x00002000
/** FSQM Status
    Shows the activation status of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_ACTS_FSQM 0x00001000
/* The block is inactive.
#define SYS_GPE_ACTS_FSQM_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_FSQM_ACT 0x00001000
/** TMU Status
    Shows the activation status of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_ACTS_TMU 0x00000800
/* The block is inactive.
#define SYS_GPE_ACTS_TMU_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_TMU_ACT 0x00000800
/** MRG Status
    Shows the activation status of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_ACTS_MRG 0x00000400
/* The block is inactive.
#define SYS_GPE_ACTS_MRG_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_MRG_ACT 0x00000400
/** DISP Status
    Shows the activation status of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_ACTS_DISP 0x00000200
/* The block is inactive.
#define SYS_GPE_ACTS_DISP_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_DISP_ACT 0x00000200
/** IQM Status
    Shows the activation status of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_ACTS_IQM 0x00000100
/* The block is inactive.
#define SYS_GPE_ACTS_IQM_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_IQM_ACT 0x00000100
/** CPUE Status
    Shows the activation status of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_ACTS_CPUE 0x00000080
/* The block is inactive.
#define SYS_GPE_ACTS_CPUE_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_CPUE_ACT 0x00000080
/** CPUI Status
    Shows the activation status of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_ACTS_CPUI 0x00000040
/* The block is inactive.
#define SYS_GPE_ACTS_CPUI_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_CPUI_ACT 0x00000040
/** GPONE Status
    Shows the activation status of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_ACTS_GPONE 0x00000020
/* The block is inactive.
#define SYS_GPE_ACTS_GPONE_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_GPONE_ACT 0x00000020
/** GPONI Status
    Shows the activation status of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_ACTS_GPONI 0x00000010
/* The block is inactive.
#define SYS_GPE_ACTS_GPONI_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_GPONI_ACT 0x00000010
/** LAN3 Status
    Shows the activation status of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_ACTS_LAN3 0x00000008
/* The block is inactive.
#define SYS_GPE_ACTS_LAN3_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_LAN3_ACT 0x00000008
/** LAN2 Status
    Shows the activation status of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_ACTS_LAN2 0x00000004
/* The block is inactive.
#define SYS_GPE_ACTS_LAN2_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_LAN2_ACT 0x00000004
/** LAN1 Status
    Shows the activation status of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_ACTS_LAN1 0x00000002
/* The block is inactive.
#define SYS_GPE_ACTS_LAN1_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_LAN1_ACT 0x00000002
/** LAN0 Status
    Shows the activation status of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_ACTS_LAN0 0x00000001
/* The block is inactive.
#define SYS_GPE_ACTS_LAN0_INACT 0x00000000 */
/** The block is active. */
#define SYS_GPE_ACTS_LAN0_ACT 0x00000001

/* Fields of "Activation Register" */
/** Activate COP7
    Sets the activation flag of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_ACT_COP7 0x80000000
/* No-Operation
#define SYS_GPE_ACT_COP7_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP7_SET 0x80000000
/** Activate COP6
    Sets the activation flag of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_ACT_COP6 0x40000000
/* No-Operation
#define SYS_GPE_ACT_COP6_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP6_SET 0x40000000
/** Activate COP5
    Sets the activation flag of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_ACT_COP5 0x20000000
/* No-Operation
#define SYS_GPE_ACT_COP5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP5_SET 0x20000000
/** Activate COP4
    Sets the activation flag of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_ACT_COP4 0x10000000
/* No-Operation
#define SYS_GPE_ACT_COP4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP4_SET 0x10000000
/** Activate COP3
    Sets the activation flag of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_ACT_COP3 0x08000000
/* No-Operation
#define SYS_GPE_ACT_COP3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP3_SET 0x08000000
/** Activate COP2
    Sets the activation flag of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_ACT_COP2 0x04000000
/* No-Operation
#define SYS_GPE_ACT_COP2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP2_SET 0x04000000
/** Activate COP1
    Sets the activation flag of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_ACT_COP1 0x02000000
/* No-Operation
#define SYS_GPE_ACT_COP1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP1_SET 0x02000000
/** Activate COP0
    Sets the activation flag of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_ACT_COP0 0x01000000
/* No-Operation
#define SYS_GPE_ACT_COP0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_COP0_SET 0x01000000
/** Activate PE5
    Sets the activation flag of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_ACT_PE5 0x00200000
/* No-Operation
#define SYS_GPE_ACT_PE5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE5_SET 0x00200000
/** Activate PE4
    Sets the activation flag of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_ACT_PE4 0x00100000
/* No-Operation
#define SYS_GPE_ACT_PE4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE4_SET 0x00100000
/** Activate PE3
    Sets the activation flag of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_ACT_PE3 0x00080000
/* No-Operation
#define SYS_GPE_ACT_PE3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE3_SET 0x00080000
/** Activate PE2
    Sets the activation flag of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_ACT_PE2 0x00040000
/* No-Operation
#define SYS_GPE_ACT_PE2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE2_SET 0x00040000
/** Activate PE1
    Sets the activation flag of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_ACT_PE1 0x00020000
/* No-Operation
#define SYS_GPE_ACT_PE1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE1_SET 0x00020000
/** Activate PE0
    Sets the activation flag of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_ACT_PE0 0x00010000
/* No-Operation
#define SYS_GPE_ACT_PE0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_PE0_SET 0x00010000
/** Activate ARB
    Sets the activation flag of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_ACT_ARB 0x00002000
/* No-Operation
#define SYS_GPE_ACT_ARB_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_ARB_SET 0x00002000
/** Activate FSQM
    Sets the activation flag of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_ACT_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_ACT_FSQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_FSQM_SET 0x00001000
/** Activate TMU
    Sets the activation flag of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_ACT_TMU 0x00000800
/* No-Operation
#define SYS_GPE_ACT_TMU_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_TMU_SET 0x00000800
/** Activate MRG
    Sets the activation flag of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_ACT_MRG 0x00000400
/* No-Operation
#define SYS_GPE_ACT_MRG_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_MRG_SET 0x00000400
/** Activate DISP
    Sets the activation flag of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_ACT_DISP 0x00000200
/* No-Operation
#define SYS_GPE_ACT_DISP_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_DISP_SET 0x00000200
/** Activate IQM
    Sets the activation flag of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_ACT_IQM 0x00000100
/* No-Operation
#define SYS_GPE_ACT_IQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_IQM_SET 0x00000100
/** Activate CPUE
    Sets the activation flag of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_ACT_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_ACT_CPUE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_CPUE_SET 0x00000080
/** Activate CPUI
    Sets the activation flag of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_ACT_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_ACT_CPUI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_CPUI_SET 0x00000040
/** Activate GPONE
    Sets the activation flag of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_ACT_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_ACT_GPONE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_GPONE_SET 0x00000020
/** Activate GPONI
    Sets the activation flag of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_ACT_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_ACT_GPONI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_GPONI_SET 0x00000010
/** Activate LAN3
    Sets the activation flag of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_ACT_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_ACT_LAN3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_LAN3_SET 0x00000008
/** Activate LAN2
    Sets the activation flag of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_ACT_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_ACT_LAN2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_LAN2_SET 0x00000004
/** Activate LAN1
    Sets the activation flag of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_ACT_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_ACT_LAN1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_LAN1_SET 0x00000002
/** Activate LAN0
    Sets the activation flag of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_ACT_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_ACT_LAN0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_ACT_LAN0_SET 0x00000001

/* Fields of "Deactivation Register" */
/** Deactivate COP7
    Clears the activation flag of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_DEACT_COP7 0x80000000
/* No-Operation
#define SYS_GPE_DEACT_COP7_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP7_CLR 0x80000000
/** Deactivate COP6
    Clears the activation flag of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_DEACT_COP6 0x40000000
/* No-Operation
#define SYS_GPE_DEACT_COP6_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP6_CLR 0x40000000
/** Deactivate COP5
    Clears the activation flag of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_DEACT_COP5 0x20000000
/* No-Operation
#define SYS_GPE_DEACT_COP5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP5_CLR 0x20000000
/** Deactivate COP4
    Clears the activation flag of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_DEACT_COP4 0x10000000
/* No-Operation
#define SYS_GPE_DEACT_COP4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP4_CLR 0x10000000
/** Deactivate COP3
    Clears the activation flag of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_DEACT_COP3 0x08000000
/* No-Operation
#define SYS_GPE_DEACT_COP3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP3_CLR 0x08000000
/** Deactivate COP2
    Clears the activation flag of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_DEACT_COP2 0x04000000
/* No-Operation
#define SYS_GPE_DEACT_COP2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP2_CLR 0x04000000
/** Deactivate COP1
    Clears the activation flag of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_DEACT_COP1 0x02000000
/* No-Operation
#define SYS_GPE_DEACT_COP1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP1_CLR 0x02000000
/** Deactivate COP0
    Clears the activation flag of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_DEACT_COP0 0x01000000
/* No-Operation
#define SYS_GPE_DEACT_COP0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_COP0_CLR 0x01000000
/** Deactivate PE5
    Clears the activation flag of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_DEACT_PE5 0x00200000
/* No-Operation
#define SYS_GPE_DEACT_PE5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE5_CLR 0x00200000
/** Deactivate PE4
    Clears the activation flag of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_DEACT_PE4 0x00100000
/* No-Operation
#define SYS_GPE_DEACT_PE4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE4_CLR 0x00100000
/** Deactivate PE3
    Clears the activation flag of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_DEACT_PE3 0x00080000
/* No-Operation
#define SYS_GPE_DEACT_PE3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE3_CLR 0x00080000
/** Deactivate PE2
    Clears the activation flag of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_DEACT_PE2 0x00040000
/* No-Operation
#define SYS_GPE_DEACT_PE2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE2_CLR 0x00040000
/** Deactivate PE1
    Clears the activation flag of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_DEACT_PE1 0x00020000
/* No-Operation
#define SYS_GPE_DEACT_PE1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE1_CLR 0x00020000
/** Deactivate PE0
    Clears the activation flag of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_DEACT_PE0 0x00010000
/* No-Operation
#define SYS_GPE_DEACT_PE0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_PE0_CLR 0x00010000
/** Deactivate ARB
    Clears the activation flag of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_DEACT_ARB 0x00002000
/* No-Operation
#define SYS_GPE_DEACT_ARB_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_ARB_CLR 0x00002000
/** Deactivate FSQM
    Clears the activation flag of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_DEACT_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_DEACT_FSQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_FSQM_CLR 0x00001000
/** Deactivate TMU
    Clears the activation flag of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_DEACT_TMU 0x00000800
/* No-Operation
#define SYS_GPE_DEACT_TMU_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_TMU_CLR 0x00000800
/** Deactivate MRG
    Clears the activation flag of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_DEACT_MRG 0x00000400
/* No-Operation
#define SYS_GPE_DEACT_MRG_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_MRG_CLR 0x00000400
/** Deactivate DISP
    Clears the activation flag of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_DEACT_DISP 0x00000200
/* No-Operation
#define SYS_GPE_DEACT_DISP_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_DISP_CLR 0x00000200
/** Deactivate IQM
    Clears the activation flag of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_DEACT_IQM 0x00000100
/* No-Operation
#define SYS_GPE_DEACT_IQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_IQM_CLR 0x00000100
/** Deactivate CPUE
    Clears the activation flag of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_DEACT_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_DEACT_CPUE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_CPUE_CLR 0x00000080
/** Deactivate CPUI
    Clears the activation flag of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_DEACT_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_DEACT_CPUI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_CPUI_CLR 0x00000040
/** Deactivate GPONE
    Clears the activation flag of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_DEACT_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_DEACT_GPONE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_GPONE_CLR 0x00000020
/** Deactivate GPONI
    Clears the activation flag of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_DEACT_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_DEACT_GPONI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_GPONI_CLR 0x00000010
/** Deactivate LAN3
    Clears the activation flag of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_DEACT_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_DEACT_LAN3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_LAN3_CLR 0x00000008
/** Deactivate LAN2
    Clears the activation flag of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_DEACT_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_DEACT_LAN2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_LAN2_CLR 0x00000004
/** Deactivate LAN1
    Clears the activation flag of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_DEACT_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_DEACT_LAN1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_LAN1_CLR 0x00000002
/** Deactivate LAN0
    Clears the activation flag of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_DEACT_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_DEACT_LAN0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_DEACT_LAN0_CLR 0x00000001

/* Fields of "Reboot Trigger Register" */
/** Reboot COP7
    Triggers a reboot of the COP7 domain. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_RBT_COP7 0x80000000
/* No-Operation
#define SYS_GPE_RBT_COP7_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP7_TRIG 0x80000000
/** Reboot COP6
    Triggers a reboot of the COP6 domain. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_RBT_COP6 0x40000000
/* No-Operation
#define SYS_GPE_RBT_COP6_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP6_TRIG 0x40000000
/** Reboot COP5
    Triggers a reboot of the COP5 domain. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_RBT_COP5 0x20000000
/* No-Operation
#define SYS_GPE_RBT_COP5_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP5_TRIG 0x20000000
/** Reboot COP4
    Triggers a reboot of the COP4 domain. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_RBT_COP4 0x10000000
/* No-Operation
#define SYS_GPE_RBT_COP4_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP4_TRIG 0x10000000
/** Reboot COP3
    Triggers a reboot of the COP3 domain. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_RBT_COP3 0x08000000
/* No-Operation
#define SYS_GPE_RBT_COP3_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP3_TRIG 0x08000000
/** Reboot COP2
    Triggers a reboot of the COP2 domain. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_RBT_COP2 0x04000000
/* No-Operation
#define SYS_GPE_RBT_COP2_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP2_TRIG 0x04000000
/** Reboot COP1
    Triggers a reboot of the COP1 domain. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_RBT_COP1 0x02000000
/* No-Operation
#define SYS_GPE_RBT_COP1_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP1_TRIG 0x02000000
/** Reboot COP0
    Triggers a reboot of the COP0 domain. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_RBT_COP0 0x01000000
/* No-Operation
#define SYS_GPE_RBT_COP0_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_COP0_TRIG 0x01000000
/** Reboot PE5
    Triggers a reboot of the PE5 domain. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_RBT_PE5 0x00200000
/* No-Operation
#define SYS_GPE_RBT_PE5_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE5_TRIG 0x00200000
/** Reboot PE4
    Triggers a reboot of the PE4 domain. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_RBT_PE4 0x00100000
/* No-Operation
#define SYS_GPE_RBT_PE4_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE4_TRIG 0x00100000
/** Reboot PE3
    Triggers a reboot of the PE3 domain. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_RBT_PE3 0x00080000
/* No-Operation
#define SYS_GPE_RBT_PE3_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE3_TRIG 0x00080000
/** Reboot PE2
    Triggers a reboot of the PE2 domain. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_RBT_PE2 0x00040000
/* No-Operation
#define SYS_GPE_RBT_PE2_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE2_TRIG 0x00040000
/** Reboot PE1
    Triggers a reboot of the PE1 domain. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_RBT_PE1 0x00020000
/* No-Operation
#define SYS_GPE_RBT_PE1_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE1_TRIG 0x00020000
/** Reboot PE0
    Triggers a reboot of the PE0 domain. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_RBT_PE0 0x00010000
/* No-Operation
#define SYS_GPE_RBT_PE0_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_PE0_TRIG 0x00010000
/** Reboot ARB
    Triggers a reboot of the ARB domain. This domain contains the Arbiter. */
#define SYS_GPE_RBT_ARB 0x00002000
/* No-Operation
#define SYS_GPE_RBT_ARB_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_ARB_TRIG 0x00002000
/** Reboot FSQM
    Triggers a reboot of the FSQM domain. This domain contains the FSQM. */
#define SYS_GPE_RBT_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_RBT_FSQM_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_FSQM_TRIG 0x00001000
/** Reboot TMU
    Triggers a reboot of the TMU domain. This domain contains the TMU. */
#define SYS_GPE_RBT_TMU 0x00000800
/* No-Operation
#define SYS_GPE_RBT_TMU_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_TMU_TRIG 0x00000800
/** Reboot MRG
    Triggers a reboot of the MRG domain. This domain contains the Merger. */
#define SYS_GPE_RBT_MRG 0x00000400
/* No-Operation
#define SYS_GPE_RBT_MRG_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_MRG_TRIG 0x00000400
/** Reboot DISP
    Triggers a reboot of the DISP domain. This domain contains the Dispatcher. */
#define SYS_GPE_RBT_DISP 0x00000200
/* No-Operation
#define SYS_GPE_RBT_DISP_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_DISP_TRIG 0x00000200
/** Reboot IQM
    Triggers a reboot of the IQM domain. This domain contains the IQM. */
#define SYS_GPE_RBT_IQM 0x00000100
/* No-Operation
#define SYS_GPE_RBT_IQM_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_IQM_TRIG 0x00000100
/** Reboot CPUE
    Triggers a reboot of the CPUE domain. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_RBT_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_RBT_CPUE_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_CPUE_TRIG 0x00000080
/** Reboot CPUI
    Triggers a reboot of the CPUI domain. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_RBT_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_RBT_CPUI_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_CPUI_TRIG 0x00000040
/** Reboot GPONE
    Triggers a reboot of the GPONE domain. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_RBT_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_RBT_GPONE_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_GPONE_TRIG 0x00000020
/** Reboot GPONI
    Triggers a reboot of the GPONI domain. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_RBT_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_RBT_GPONI_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_GPONI_TRIG 0x00000010
/** Reboot LAN3
    Triggers a reboot of the LAN3 domain. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_RBT_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_RBT_LAN3_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_LAN3_TRIG 0x00000008
/** Reboot LAN2
    Triggers a reboot of the LAN2 domain. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_RBT_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_RBT_LAN2_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_LAN2_TRIG 0x00000004
/** Reboot LAN1
    Triggers a reboot of the LAN1 domain. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_RBT_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_RBT_LAN1_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_LAN1_TRIG 0x00000002
/** Reboot LAN0
    Triggers a reboot of the LAN0 domain. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_RBT_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_RBT_LAN0_NOP 0x00000000 */
/** Trigger */
#define SYS_GPE_RBT_LAN0_TRIG 0x00000001

/* Fields of "Power Down Configuration Register" */
/** Enable Power Down COP7
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP7 0x80000000
/* Disable
#define SYS_GPE_PDCFG_COP7_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP7_EN 0x80000000
/** Enable Power Down COP6
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP6 0x40000000
/* Disable
#define SYS_GPE_PDCFG_COP6_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP6_EN 0x40000000
/** Enable Power Down COP5
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP5 0x20000000
/* Disable
#define SYS_GPE_PDCFG_COP5_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP5_EN 0x20000000
/** Enable Power Down COP4
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP4 0x10000000
/* Disable
#define SYS_GPE_PDCFG_COP4_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP4_EN 0x10000000
/** Enable Power Down COP3
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP3 0x08000000
/* Disable
#define SYS_GPE_PDCFG_COP3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP3_EN 0x08000000
/** Enable Power Down COP2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP2 0x04000000
/* Disable
#define SYS_GPE_PDCFG_COP2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP2_EN 0x04000000
/** Enable Power Down COP1
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP1 0x02000000
/* Disable
#define SYS_GPE_PDCFG_COP1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP1_EN 0x02000000
/** Enable Power Down COP0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_COP0 0x01000000
/* Disable
#define SYS_GPE_PDCFG_COP0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_COP0_EN 0x01000000
/** Enable Power Down PE5
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE5 0x00200000
/* Disable
#define SYS_GPE_PDCFG_PE5_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE5_EN 0x00200000
/** Enable Power Down PE4
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE4 0x00100000
/* Disable
#define SYS_GPE_PDCFG_PE4_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE4_EN 0x00100000
/** Enable Power Down PE3
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE3 0x00080000
/* Disable
#define SYS_GPE_PDCFG_PE3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE3_EN 0x00080000
/** Enable Power Down PE2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE2 0x00040000
/* Disable
#define SYS_GPE_PDCFG_PE2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE2_EN 0x00040000
/** Enable Power Down PE1
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE1 0x00020000
/* Disable
#define SYS_GPE_PDCFG_PE1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE1_EN 0x00020000
/** Enable Power Down PE0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_PE0 0x00010000
/* Disable
#define SYS_GPE_PDCFG_PE0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_PE0_EN 0x00010000
/** Enable Power Down ARB
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_ARB 0x00002000
/* Disable
#define SYS_GPE_PDCFG_ARB_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_ARB_EN 0x00002000
/** Enable Power Down FSQM
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_FSQM 0x00001000
/* Disable
#define SYS_GPE_PDCFG_FSQM_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_FSQM_EN 0x00001000
/** Enable Power Down TMU
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_TMU 0x00000800
/* Disable
#define SYS_GPE_PDCFG_TMU_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_TMU_EN 0x00000800
/** Enable Power Down MRG
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_MRG 0x00000400
/* Disable
#define SYS_GPE_PDCFG_MRG_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_MRG_EN 0x00000400
/** Enable Power Down DISP
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_DISP 0x00000200
/* Disable
#define SYS_GPE_PDCFG_DISP_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_DISP_EN 0x00000200
/** Enable Power Down IQM
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_IQM 0x00000100
/* Disable
#define SYS_GPE_PDCFG_IQM_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_IQM_EN 0x00000100
/** Enable Power Down CPUE
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_CPUE 0x00000080
/* Disable
#define SYS_GPE_PDCFG_CPUE_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_CPUE_EN 0x00000080
/** Enable Power Down CPUI
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_CPUI 0x00000040
/* Disable
#define SYS_GPE_PDCFG_CPUI_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_CPUI_EN 0x00000040
/** Enable Power Down GPONE
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_GPONE 0x00000020
/* Disable
#define SYS_GPE_PDCFG_GPONE_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_GPONE_EN 0x00000020
/** Enable Power Down GPONI
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_GPONI 0x00000010
/* Disable
#define SYS_GPE_PDCFG_GPONI_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_GPONI_EN 0x00000010
/** Enable Power Down LAN3
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_LAN3 0x00000008
/* Disable
#define SYS_GPE_PDCFG_LAN3_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_LAN3_EN 0x00000008
/** Enable Power Down LAN2
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_LAN2 0x00000004
/* Disable
#define SYS_GPE_PDCFG_LAN2_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_LAN2_EN 0x00000004
/** Enable Power Down LAN1
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_LAN1 0x00000002
/* Disable
#define SYS_GPE_PDCFG_LAN1_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_LAN1_EN 0x00000002
/** Enable Power Down LAN0
    Ignore this bit as power-gating is not supported for this chip. */
#define SYS_GPE_PDCFG_LAN0 0x00000001
/* Disable
#define SYS_GPE_PDCFG_LAN0_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_PDCFG_LAN0_EN 0x00000001

/* Fields of "Sleep Source Configuration Register" */
/** Sleep/Wakeup Source CPU
    Selects the CPU access signal as sleep/wakeup source. */
#define SYS_GPE_SSCFG_CPU 0x00020000
/* Not selected
#define SYS_GPE_SSCFG_CPU_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_CPU_SEL 0x00020000
/** Sleep/Wakeup Source FSQM
    Selects the FSQM signal as sleep/wakeup source. */
#define SYS_GPE_SSCFG_FSQM 0x00008000
/* Not selected
#define SYS_GPE_SSCFG_FSQM_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_FSQM_SEL 0x00008000
/** Sleep/Wakeup Source GPONT
    Selects the FIFO empty signal of the TCONT Request FIFO of port GPON as sleep/wakeup source. */
#define SYS_GPE_SSCFG_GPONT 0x00002000
/* Not selected
#define SYS_GPE_SSCFG_GPONT_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_GPONT_SEL 0x00002000
/** Sleep/Wakeup Source GPONE
    Selects the FIFO empty signal of the EGRESS FIFO of port GPON as sleep/wakeup source. */
#define SYS_GPE_SSCFG_GPONE 0x00001000
/* Not selected
#define SYS_GPE_SSCFG_GPONE_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_GPONE_SEL 0x00001000
/** Sleep/Wakeup Source LAN3E
    Selects the FIFO empty signal of the EGRESS FIFO of port LAN3 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN3E 0x00000800
/* Not selected
#define SYS_GPE_SSCFG_LAN3E_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN3E_SEL 0x00000800
/** Sleep/Wakeup Source LAN2E
    Selects the FIFO empty signal of the EGRESS FIFO of port LAN2 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN2E 0x00000400
/* Not selected
#define SYS_GPE_SSCFG_LAN2E_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN2E_SEL 0x00000400
/** Sleep/Wakeup Source LAN1E
    Selects the FIFO empty signal of the EGRESS FIFO of port LAN1 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN1E 0x00000200
/* Not selected
#define SYS_GPE_SSCFG_LAN1E_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN1E_SEL 0x00000200
/** Sleep/Wakeup Source LAN0E
    Selects the FIFO empty signal of the EGRESS FIFO of port LAN0 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN0E 0x00000100
/* Not selected
#define SYS_GPE_SSCFG_LAN0E_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN0E_SEL 0x00000100
/** Sleep/Wakeup Source GPONI
    Selects the FIFO empty signal of the INGRESS FIFO of port GPON as sleep/wakeup source. */
#define SYS_GPE_SSCFG_GPONI 0x00000010
/* Not selected
#define SYS_GPE_SSCFG_GPONI_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_GPONI_SEL 0x00000010
/** Sleep/Wakeup Source LAN3I
    Selects the FIFO empty signal of the INGRESS FIFO of port LAN3 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN3I 0x00000008
/* Not selected
#define SYS_GPE_SSCFG_LAN3I_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN3I_SEL 0x00000008
/** Sleep/Wakeup Source LAN2I
    Selects the FIFO empty signal of the INGRESS FIFO of port LAN2 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN2I 0x00000004
/* Not selected
#define SYS_GPE_SSCFG_LAN2I_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN2I_SEL 0x00000004
/** Sleep/Wakeup Source LAN1I
    Selects the FIFO empty signal of the INGRESS FIFO of port LAN1 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN1I 0x00000002
/* Not selected
#define SYS_GPE_SSCFG_LAN1I_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN1I_SEL 0x00000002
/** Sleep/Wakeup Source LAN0I
    Selects the FIFO empty signal of the INGRESS FIFO of port LAN0 as sleep/wakeup source. */
#define SYS_GPE_SSCFG_LAN0I 0x00000001
/* Not selected
#define SYS_GPE_SSCFG_LAN0I_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SSCFG_LAN0I_SEL 0x00000001

/* Fields of "Sleep Source Timer Register" */
/** Sleep Delay Value
    A HW sleep request is delayed by this value multiplied by 3.2ns before it takes effect. A wakeup request is not delayed but takes effect immediately. Values lower than 256 are limited to 256. */
#define SYS_GPE_SST_SDV_MASK 0x7FFFFFFF
/** field offset */
#define SYS_GPE_SST_SDV_OFFSET 0

/* Fields of "Sleep Destination Status Register" */
/** Shutoff COP7 on HW Sleep
    If selected the domain COP7 is shutoff on a hardware sleep request. This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_SDS_COP7 0x80000000
/* Not selected
#define SYS_GPE_SDS_COP7_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP7_SEL 0x80000000
/** Shutoff COP6 on HW Sleep
    If selected the domain COP6 is shutoff on a hardware sleep request. This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_SDS_COP6 0x40000000
/* Not selected
#define SYS_GPE_SDS_COP6_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP6_SEL 0x40000000
/** Shutoff COP5 on HW Sleep
    If selected the domain COP5 is shutoff on a hardware sleep request. This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_SDS_COP5 0x20000000
/* Not selected
#define SYS_GPE_SDS_COP5_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP5_SEL 0x20000000
/** Shutoff COP4 on HW Sleep
    If selected the domain COP4 is shutoff on a hardware sleep request. This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_SDS_COP4 0x10000000
/* Not selected
#define SYS_GPE_SDS_COP4_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP4_SEL 0x10000000
/** Shutoff COP3 on HW Sleep
    If selected the domain COP3 is shutoff on a hardware sleep request. This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_SDS_COP3 0x08000000
/* Not selected
#define SYS_GPE_SDS_COP3_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP3_SEL 0x08000000
/** Shutoff COP2 on HW Sleep
    If selected the domain COP2 is shutoff on a hardware sleep request. This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_SDS_COP2 0x04000000
/* Not selected
#define SYS_GPE_SDS_COP2_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP2_SEL 0x04000000
/** Shutoff COP1 on HW Sleep
    If selected the domain COP1 is shutoff on a hardware sleep request. This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_SDS_COP1 0x02000000
/* Not selected
#define SYS_GPE_SDS_COP1_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP1_SEL 0x02000000
/** Shutoff COP0 on HW Sleep
    If selected the domain COP0 is shutoff on a hardware sleep request. This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_SDS_COP0 0x01000000
/* Not selected
#define SYS_GPE_SDS_COP0_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_COP0_SEL 0x01000000
/** Shutoff PE5 on HW Sleep
    If selected the domain PE5 is shutoff on a hardware sleep request. This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_SDS_PE5 0x00200000
/* Not selected
#define SYS_GPE_SDS_PE5_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE5_SEL 0x00200000
/** Shutoff PE4 on HW Sleep
    If selected the domain PE4 is shutoff on a hardware sleep request. This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_SDS_PE4 0x00100000
/* Not selected
#define SYS_GPE_SDS_PE4_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE4_SEL 0x00100000
/** Shutoff PE3 on HW Sleep
    If selected the domain PE3 is shutoff on a hardware sleep request. This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_SDS_PE3 0x00080000
/* Not selected
#define SYS_GPE_SDS_PE3_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE3_SEL 0x00080000
/** Shutoff PE2 on HW Sleep
    If selected the domain PE2 is shutoff on a hardware sleep request. This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_SDS_PE2 0x00040000
/* Not selected
#define SYS_GPE_SDS_PE2_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE2_SEL 0x00040000
/** Shutoff PE1 on HW Sleep
    If selected the domain PE1 is shutoff on a hardware sleep request. This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_SDS_PE1 0x00020000
/* Not selected
#define SYS_GPE_SDS_PE1_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE1_SEL 0x00020000
/** Shutoff PE0 on HW Sleep
    If selected the domain PE0 is shutoff on a hardware sleep request. This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_SDS_PE0 0x00010000
/* Not selected
#define SYS_GPE_SDS_PE0_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_PE0_SEL 0x00010000
/** Shutoff ARB on HW Sleep
    If selected the domain ARB is shutoff on a hardware sleep request. This domain contains the Arbiter. */
#define SYS_GPE_SDS_ARB 0x00002000
/* Not selected
#define SYS_GPE_SDS_ARB_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_ARB_SEL 0x00002000
/** Shutoff FSQM on HW Sleep
    If selected the domain FSQM is shutoff on a hardware sleep request. This domain contains the FSQM. */
#define SYS_GPE_SDS_FSQM 0x00001000
/* Not selected
#define SYS_GPE_SDS_FSQM_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_FSQM_SEL 0x00001000
/** Shutoff TMU on HW Sleep
    If selected the domain TMU is shutoff on a hardware sleep request. This domain contains the TMU. */
#define SYS_GPE_SDS_TMU 0x00000800
/* Not selected
#define SYS_GPE_SDS_TMU_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_TMU_SEL 0x00000800
/** Shutoff MRG on HW Sleep
    If selected the domain MRG is shutoff on a hardware sleep request. This domain contains the Merger. */
#define SYS_GPE_SDS_MRG 0x00000400
/* Not selected
#define SYS_GPE_SDS_MRG_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_MRG_SEL 0x00000400
/** Shutoff DISP on HW Sleep
    If selected the domain DISP is shutoff on a hardware sleep request. This domain contains the Dispatcher. */
#define SYS_GPE_SDS_DISP 0x00000200
/* Not selected
#define SYS_GPE_SDS_DISP_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_DISP_SEL 0x00000200
/** Shutoff IQM on HW Sleep
    If selected the domain IQM is shutoff on a hardware sleep request. This domain contains the IQM. */
#define SYS_GPE_SDS_IQM 0x00000100
/* Not selected
#define SYS_GPE_SDS_IQM_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_IQM_SEL 0x00000100
/** Shutoff CPUE on HW Sleep
    If selected the domain CPUE is shutoff on a hardware sleep request. This domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_SDS_CPUE 0x00000080
/* Not selected
#define SYS_GPE_SDS_CPUE_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_CPUE_SEL 0x00000080
/** Shutoff CPUI on HW Sleep
    If selected the domain CPUI is shutoff on a hardware sleep request. This domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_SDS_CPUI 0x00000040
/* Not selected
#define SYS_GPE_SDS_CPUI_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_CPUI_SEL 0x00000040
/** Shutoff GPONE on HW Sleep
    If selected the domain GPONE is shutoff on a hardware sleep request. This domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_SDS_GPONE 0x00000020
/* Not selected
#define SYS_GPE_SDS_GPONE_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_GPONE_SEL 0x00000020
/** Shutoff GPONI on HW Sleep
    If selected the domain GPONI is shutoff on a hardware sleep request. This domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_SDS_GPONI 0x00000010
/* Not selected
#define SYS_GPE_SDS_GPONI_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_GPONI_SEL 0x00000010
/** Shutoff LAN3 on HW Sleep
    If selected the domain LAN3 is shutoff on a hardware sleep request. This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_SDS_LAN3 0x00000008
/* Not selected
#define SYS_GPE_SDS_LAN3_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_LAN3_SEL 0x00000008
/** Shutoff LAN2 on HW Sleep
    If selected the domain LAN2 is shutoff on a hardware sleep request. This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_SDS_LAN2 0x00000004
/* Not selected
#define SYS_GPE_SDS_LAN2_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_LAN2_SEL 0x00000004
/** Shutoff LAN1 on HW Sleep
    If selected the domain LAN1 is shutoff on a hardware sleep request. This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_SDS_LAN1 0x00000002
/* Not selected
#define SYS_GPE_SDS_LAN1_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_LAN1_SEL 0x00000002
/** Shutoff LAN0 on HW Sleep
    If selected the domain LAN0 is shutoff on a hardware sleep request. This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_SDS_LAN0 0x00000001
/* Not selected
#define SYS_GPE_SDS_LAN0_NSEL 0x00000000 */
/** Selected */
#define SYS_GPE_SDS_LAN0_SEL 0x00000001

/* Fields of "Sleep Destination Set Register" */
/** Set Sleep Selection COP7
    Sets the selection bit for domain COP7This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_SDSET_COP7 0x80000000
/* No-Operation
#define SYS_GPE_SDSET_COP7_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP7_SET 0x80000000
/** Set Sleep Selection COP6
    Sets the selection bit for domain COP6This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_SDSET_COP6 0x40000000
/* No-Operation
#define SYS_GPE_SDSET_COP6_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP6_SET 0x40000000
/** Set Sleep Selection COP5
    Sets the selection bit for domain COP5This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_SDSET_COP5 0x20000000
/* No-Operation
#define SYS_GPE_SDSET_COP5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP5_SET 0x20000000
/** Set Sleep Selection COP4
    Sets the selection bit for domain COP4This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_SDSET_COP4 0x10000000
/* No-Operation
#define SYS_GPE_SDSET_COP4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP4_SET 0x10000000
/** Set Sleep Selection COP3
    Sets the selection bit for domain COP3This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_SDSET_COP3 0x08000000
/* No-Operation
#define SYS_GPE_SDSET_COP3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP3_SET 0x08000000
/** Set Sleep Selection COP2
    Sets the selection bit for domain COP2This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_SDSET_COP2 0x04000000
/* No-Operation
#define SYS_GPE_SDSET_COP2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP2_SET 0x04000000
/** Set Sleep Selection COP1
    Sets the selection bit for domain COP1This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_SDSET_COP1 0x02000000
/* No-Operation
#define SYS_GPE_SDSET_COP1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP1_SET 0x02000000
/** Set Sleep Selection COP0
    Sets the selection bit for domain COP0This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_SDSET_COP0 0x01000000
/* No-Operation
#define SYS_GPE_SDSET_COP0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_COP0_SET 0x01000000
/** Set Sleep Selection PE5
    Sets the selection bit for domain PE5This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_SDSET_PE5 0x00200000
/* No-Operation
#define SYS_GPE_SDSET_PE5_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE5_SET 0x00200000
/** Set Sleep Selection PE4
    Sets the selection bit for domain PE4This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_SDSET_PE4 0x00100000
/* No-Operation
#define SYS_GPE_SDSET_PE4_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE4_SET 0x00100000
/** Set Sleep Selection PE3
    Sets the selection bit for domain PE3This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_SDSET_PE3 0x00080000
/* No-Operation
#define SYS_GPE_SDSET_PE3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE3_SET 0x00080000
/** Set Sleep Selection PE2
    Sets the selection bit for domain PE2This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_SDSET_PE2 0x00040000
/* No-Operation
#define SYS_GPE_SDSET_PE2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE2_SET 0x00040000
/** Set Sleep Selection PE1
    Sets the selection bit for domain PE1This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_SDSET_PE1 0x00020000
/* No-Operation
#define SYS_GPE_SDSET_PE1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE1_SET 0x00020000
/** Set Sleep Selection PE0
    Sets the selection bit for domain PE0This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_SDSET_PE0 0x00010000
/* No-Operation
#define SYS_GPE_SDSET_PE0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_PE0_SET 0x00010000
/** Set Sleep Selection ARB
    Sets the selection bit for domain ARBThis domain contains the Arbiter. */
#define SYS_GPE_SDSET_ARB 0x00002000
/* No-Operation
#define SYS_GPE_SDSET_ARB_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_ARB_SET 0x00002000
/** Set Sleep Selection FSQM
    Sets the selection bit for domain FSQMThis domain contains the FSQM. */
#define SYS_GPE_SDSET_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_SDSET_FSQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_FSQM_SET 0x00001000
/** Set Sleep Selection TMU
    Sets the selection bit for domain TMUThis domain contains the TMU. */
#define SYS_GPE_SDSET_TMU 0x00000800
/* No-Operation
#define SYS_GPE_SDSET_TMU_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_TMU_SET 0x00000800
/** Set Sleep Selection MRG
    Sets the selection bit for domain MRGThis domain contains the Merger. */
#define SYS_GPE_SDSET_MRG 0x00000400
/* No-Operation
#define SYS_GPE_SDSET_MRG_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_MRG_SET 0x00000400
/** Set Sleep Selection DISP
    Sets the selection bit for domain DISPThis domain contains the Dispatcher. */
#define SYS_GPE_SDSET_DISP 0x00000200
/* No-Operation
#define SYS_GPE_SDSET_DISP_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_DISP_SET 0x00000200
/** Set Sleep Selection IQM
    Sets the selection bit for domain IQMThis domain contains the IQM. */
#define SYS_GPE_SDSET_IQM 0x00000100
/* No-Operation
#define SYS_GPE_SDSET_IQM_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_IQM_SET 0x00000100
/** Set Sleep Selection CPUE
    Sets the selection bit for domain CPUEThis domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_SDSET_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_SDSET_CPUE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_CPUE_SET 0x00000080
/** Set Sleep Selection CPUI
    Sets the selection bit for domain CPUIThis domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_SDSET_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_SDSET_CPUI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_CPUI_SET 0x00000040
/** Set Sleep Selection GPONE
    Sets the selection bit for domain GPONEThis domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_SDSET_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_SDSET_GPONE_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_GPONE_SET 0x00000020
/** Set Sleep Selection GPONI
    Sets the selection bit for domain GPONIThis domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_SDSET_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_SDSET_GPONI_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_GPONI_SET 0x00000010
/** Set Sleep Selection LAN3
    Sets the selection bit for domain LAN3This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_SDSET_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_SDSET_LAN3_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_LAN3_SET 0x00000008
/** Set Sleep Selection LAN2
    Sets the selection bit for domain LAN2This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_SDSET_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_SDSET_LAN2_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_LAN2_SET 0x00000004
/** Set Sleep Selection LAN1
    Sets the selection bit for domain LAN1This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_SDSET_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_SDSET_LAN1_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_LAN1_SET 0x00000002
/** Set Sleep Selection LAN0
    Sets the selection bit for domain LAN0This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_SDSET_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_SDSET_LAN0_NOP 0x00000000 */
/** Set */
#define SYS_GPE_SDSET_LAN0_SET 0x00000001

/* Fields of "Sleep Destination Clear Register" */
/** Clear Sleep Selection COP7
    Clears the selection bit for domain COP7This domain contains the Coprocessor 7 of the SCE. */
#define SYS_GPE_SDCLR_COP7 0x80000000
/* No-Operation
#define SYS_GPE_SDCLR_COP7_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP7_CLR 0x80000000
/** Clear Sleep Selection COP6
    Clears the selection bit for domain COP6This domain contains the Coprocessor 6 of the SCE. */
#define SYS_GPE_SDCLR_COP6 0x40000000
/* No-Operation
#define SYS_GPE_SDCLR_COP6_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP6_CLR 0x40000000
/** Clear Sleep Selection COP5
    Clears the selection bit for domain COP5This domain contains the Coprocessor 5 of the SCE. */
#define SYS_GPE_SDCLR_COP5 0x20000000
/* No-Operation
#define SYS_GPE_SDCLR_COP5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP5_CLR 0x20000000
/** Clear Sleep Selection COP4
    Clears the selection bit for domain COP4This domain contains the Coprocessor 4 of the SCE. */
#define SYS_GPE_SDCLR_COP4 0x10000000
/* No-Operation
#define SYS_GPE_SDCLR_COP4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP4_CLR 0x10000000
/** Clear Sleep Selection COP3
    Clears the selection bit for domain COP3This domain contains the Coprocessor 3 of the SCE. */
#define SYS_GPE_SDCLR_COP3 0x08000000
/* No-Operation
#define SYS_GPE_SDCLR_COP3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP3_CLR 0x08000000
/** Clear Sleep Selection COP2
    Clears the selection bit for domain COP2This domain contains the Coprocessor 2 of the SCE. */
#define SYS_GPE_SDCLR_COP2 0x04000000
/* No-Operation
#define SYS_GPE_SDCLR_COP2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP2_CLR 0x04000000
/** Clear Sleep Selection COP1
    Clears the selection bit for domain COP1This domain contains the Coprocessor 1 of the SCE. */
#define SYS_GPE_SDCLR_COP1 0x02000000
/* No-Operation
#define SYS_GPE_SDCLR_COP1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP1_CLR 0x02000000
/** Clear Sleep Selection COP0
    Clears the selection bit for domain COP0This domain contains the Coprocessor 0 of the SCE. */
#define SYS_GPE_SDCLR_COP0 0x01000000
/* No-Operation
#define SYS_GPE_SDCLR_COP0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_COP0_CLR 0x01000000
/** Clear Sleep Selection PE5
    Clears the selection bit for domain PE5This domain contains the Processing Element 5 of the SCE. */
#define SYS_GPE_SDCLR_PE5 0x00200000
/* No-Operation
#define SYS_GPE_SDCLR_PE5_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE5_CLR 0x00200000
/** Clear Sleep Selection PE4
    Clears the selection bit for domain PE4This domain contains the Processing Element 4 of the SCE. */
#define SYS_GPE_SDCLR_PE4 0x00100000
/* No-Operation
#define SYS_GPE_SDCLR_PE4_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE4_CLR 0x00100000
/** Clear Sleep Selection PE3
    Clears the selection bit for domain PE3This domain contains the Processing Element 3 of the SCE. */
#define SYS_GPE_SDCLR_PE3 0x00080000
/* No-Operation
#define SYS_GPE_SDCLR_PE3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE3_CLR 0x00080000
/** Clear Sleep Selection PE2
    Clears the selection bit for domain PE2This domain contains the Processing Element 2 of the SCE. */
#define SYS_GPE_SDCLR_PE2 0x00040000
/* No-Operation
#define SYS_GPE_SDCLR_PE2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE2_CLR 0x00040000
/** Clear Sleep Selection PE1
    Clears the selection bit for domain PE1This domain contains the Processing Element 1 of the SCE. */
#define SYS_GPE_SDCLR_PE1 0x00020000
/* No-Operation
#define SYS_GPE_SDCLR_PE1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE1_CLR 0x00020000
/** Clear Sleep Selection PE0
    Clears the selection bit for domain PE0This domain contains the Processing Element 0 of the SCE. */
#define SYS_GPE_SDCLR_PE0 0x00010000
/* No-Operation
#define SYS_GPE_SDCLR_PE0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_PE0_CLR 0x00010000
/** Clear Sleep Selection ARB
    Clears the selection bit for domain ARBThis domain contains the Arbiter. */
#define SYS_GPE_SDCLR_ARB 0x00002000
/* No-Operation
#define SYS_GPE_SDCLR_ARB_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_ARB_CLR 0x00002000
/** Clear Sleep Selection FSQM
    Clears the selection bit for domain FSQMThis domain contains the FSQM. */
#define SYS_GPE_SDCLR_FSQM 0x00001000
/* No-Operation
#define SYS_GPE_SDCLR_FSQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_FSQM_CLR 0x00001000
/** Clear Sleep Selection TMU
    Clears the selection bit for domain TMUThis domain contains the TMU. */
#define SYS_GPE_SDCLR_TMU 0x00000800
/* No-Operation
#define SYS_GPE_SDCLR_TMU_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_TMU_CLR 0x00000800
/** Clear Sleep Selection MRG
    Clears the selection bit for domain MRGThis domain contains the Merger. */
#define SYS_GPE_SDCLR_MRG 0x00000400
/* No-Operation
#define SYS_GPE_SDCLR_MRG_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_MRG_CLR 0x00000400
/** Clear Sleep Selection DISP
    Clears the selection bit for domain DISPThis domain contains the Dispatcher. */
#define SYS_GPE_SDCLR_DISP 0x00000200
/* No-Operation
#define SYS_GPE_SDCLR_DISP_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_DISP_CLR 0x00000200
/** Clear Sleep Selection IQM
    Clears the selection bit for domain IQMThis domain contains the IQM. */
#define SYS_GPE_SDCLR_IQM 0x00000100
/* No-Operation
#define SYS_GPE_SDCLR_IQM_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_IQM_CLR 0x00000100
/** Clear Sleep Selection CPUE
    Clears the selection bit for domain CPUEThis domain contains all parts related to the CPU EGRESS interface. */
#define SYS_GPE_SDCLR_CPUE 0x00000080
/* No-Operation
#define SYS_GPE_SDCLR_CPUE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_CPUE_CLR 0x00000080
/** Clear Sleep Selection CPUI
    Clears the selection bit for domain CPUIThis domain contains all parts related to the CPU INGRESS interface. */
#define SYS_GPE_SDCLR_CPUI 0x00000040
/* No-Operation
#define SYS_GPE_SDCLR_CPUI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_CPUI_CLR 0x00000040
/** Clear Sleep Selection GPONE
    Clears the selection bit for domain GPONEThis domain contains all parts related to the GPON (GTC) EGRESS interface. */
#define SYS_GPE_SDCLR_GPONE 0x00000020
/* No-Operation
#define SYS_GPE_SDCLR_GPONE_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_GPONE_CLR 0x00000020
/** Clear Sleep Selection GPONI
    Clears the selection bit for domain GPONIThis domain contains all parts related to the GPON (GTC) INGRESS interface. */
#define SYS_GPE_SDCLR_GPONI 0x00000010
/* No-Operation
#define SYS_GPE_SDCLR_GPONI_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_GPONI_CLR 0x00000010
/** Clear Sleep Selection LAN3
    Clears the selection bit for domain LAN3This domain contains all parts related to the LAN3 interface. */
#define SYS_GPE_SDCLR_LAN3 0x00000008
/* No-Operation
#define SYS_GPE_SDCLR_LAN3_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_LAN3_CLR 0x00000008
/** Clear Sleep Selection LAN2
    Clears the selection bit for domain LAN2This domain contains all parts related to the LAN2 interface. */
#define SYS_GPE_SDCLR_LAN2 0x00000004
/* No-Operation
#define SYS_GPE_SDCLR_LAN2_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_LAN2_CLR 0x00000004
/** Clear Sleep Selection LAN1
    Clears the selection bit for domain LAN1This domain contains all parts related to the LAN1 interface. */
#define SYS_GPE_SDCLR_LAN1 0x00000002
/* No-Operation
#define SYS_GPE_SDCLR_LAN1_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_LAN1_CLR 0x00000002
/** Clear Sleep Selection LAN0
    Clears the selection bit for domain LAN0This domain contains all parts related to the LAN0 interface. */
#define SYS_GPE_SDCLR_LAN0 0x00000001
/* No-Operation
#define SYS_GPE_SDCLR_LAN0_NOP 0x00000000 */
/** Clear */
#define SYS_GPE_SDCLR_LAN0_CLR 0x00000001

/* Fields of "IRNCS Capture Register" */
/** FSQM wakeup request
    The FSQM submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_FSQMWR 0x80000000
/* Nothing
#define SYS_GPE_IRNCSCR_FSQMWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_FSQMWR_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_FSQMWR_INTOCC 0x80000000
/** GPONT wakeup request
    The TCONT Request FIFO of port GPON submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONTWR 0x20000000
/* Nothing
#define SYS_GPE_IRNCSCR_GPONTWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONTWR_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONTWR_INTOCC 0x20000000
/** GPONE wakeup request
    The EGRESS FIFO of port GPON submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONEWR 0x10000000
/* Nothing
#define SYS_GPE_IRNCSCR_GPONEWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONEWR_INTACK 0x10000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONEWR_INTOCC 0x10000000
/** LAN3E wakeup request
    The EGRESS FIFO of port LAN3 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN3EWR 0x08000000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN3EWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN3EWR_INTACK 0x08000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN3EWR_INTOCC 0x08000000
/** LAN2E wakeup requestThe ENGRESS FIFO of port LAN2 submitted a wakeup request.
    This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN2EWR 0x04000000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN2EWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN2EWR_INTACK 0x04000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN2EWR_INTOCC 0x04000000
/** LAN1E wakeup request
    The EGRESS FIFO of port LAN1 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN1EWR 0x02000000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN1EWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN1EWR_INTACK 0x02000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN1EWR_INTOCC 0x02000000
/** LAN0E wakeup request
    The EGRESS FIFO of port LAN0 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN0EWR 0x01000000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN0EWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN0EWR_INTACK 0x01000000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN0EWR_INTOCC 0x01000000
/** GPONI wakeup request
    The INGRESS FIFO of port GPON submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONIWR 0x00100000
/* Nothing
#define SYS_GPE_IRNCSCR_GPONIWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONIWR_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONIWR_INTOCC 0x00100000
/** LAN3I wakeup request
    The INGRESS FIFO of port LAN3 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN3IWR 0x00080000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN3IWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN3IWR_INTACK 0x00080000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN3IWR_INTOCC 0x00080000
/** LAN2I wakeup request
    The INGRESS FIFO of port LAN2 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN2IWR 0x00040000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN2IWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN2IWR_INTACK 0x00040000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN2IWR_INTOCC 0x00040000
/** LAN1I wakeup request
    The INGRESS FIFO of port LAN1 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN1IWR 0x00020000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN1IWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN1IWR_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN1IWR_INTOCC 0x00020000
/** LAN0I wakeup request
    The INGRESS FIFO of port LAN0 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN0IWR 0x00010000
/* Nothing
#define SYS_GPE_IRNCSCR_LAN0IWR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN0IWR_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN0IWR_INTOCC 0x00010000
/** FSQM sleep request
    The FSQM submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_FSQMSR 0x00008000
/* Nothing
#define SYS_GPE_IRNCSCR_FSQMSR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_FSQMSR_INTACK 0x00008000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_FSQMSR_INTOCC 0x00008000
/** GPONT sleep request
    The TCONT Request FIFO of port GPON submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONTSR 0x00002000
/* Nothing
#define SYS_GPE_IRNCSCR_GPONTSR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONTSR_INTACK 0x00002000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONTSR_INTOCC 0x00002000
/** GPONE sleep request
    The EGRESS FIFO of port GPON submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONESR 0x00001000
/* Nothing
#define SYS_GPE_IRNCSCR_GPONESR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONESR_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONESR_INTOCC 0x00001000
/** LAN3E sleep request
    The EGRESS FIFO of port LAN3 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN3ESR 0x00000800
/* Nothing
#define SYS_GPE_IRNCSCR_LAN3ESR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN3ESR_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN3ESR_INTOCC 0x00000800
/** LAN2E sleep requestThe ENGRESS FIFO of port LAN2 submitted a sleep request.
    This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN2ESR 0x00000400
/* Nothing
#define SYS_GPE_IRNCSCR_LAN2ESR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN2ESR_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN2ESR_INTOCC 0x00000400
/** LAN1E sleep request
    The EGRESS FIFO of port LAN1 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN1ESR 0x00000200
/* Nothing
#define SYS_GPE_IRNCSCR_LAN1ESR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN1ESR_INTACK 0x00000200
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN1ESR_INTOCC 0x00000200
/** LAN0E sleep request
    The EGRESS FIFO of port LAN0 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN0ESR 0x00000100
/* Nothing
#define SYS_GPE_IRNCSCR_LAN0ESR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN0ESR_INTACK 0x00000100
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN0ESR_INTOCC 0x00000100
/** GPONI sleep request
    The INGRESS FIFO of port GPON submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_GPONISR 0x00000010
/* Nothing
#define SYS_GPE_IRNCSCR_GPONISR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_GPONISR_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_GPONISR_INTOCC 0x00000010
/** LAN3I sleep request
    The INGRESS FIFO of port LAN3 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN3ISR 0x00000008
/* Nothing
#define SYS_GPE_IRNCSCR_LAN3ISR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN3ISR_INTACK 0x00000008
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN3ISR_INTOCC 0x00000008
/** LAN2I sleep request
    The INGRESS FIFO of port LAN2 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN2ISR 0x00000004
/* Nothing
#define SYS_GPE_IRNCSCR_LAN2ISR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN2ISR_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN2ISR_INTOCC 0x00000004
/** LAN1I sleep request
    The INGRESS FIFO of port LAN1 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN1ISR 0x00000002
/* Nothing
#define SYS_GPE_IRNCSCR_LAN1ISR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN1ISR_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN1ISR_INTOCC 0x00000002
/** LAN0I sleep request
    The INGRESS FIFO of port LAN0 submitted a sleep request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define SYS_GPE_IRNCSCR_LAN0ISR 0x00000001
/* Nothing
#define SYS_GPE_IRNCSCR_LAN0ISR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define SYS_GPE_IRNCSCR_LAN0ISR_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define SYS_GPE_IRNCSCR_LAN0ISR_INTOCC 0x00000001

/* Fields of "IRNCS Interrupt Control Register" */
/** FSQM wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_FSQMWR 0x80000000
/** GPONT wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONTWR 0x20000000
/** GPONE wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONEWR 0x10000000
/** LAN3E wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN3EWR 0x08000000
/** LAN2E wakeup requestThe ENGRESS FIFO of port LAN2 submitted a wakeup request.
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN2EWR 0x04000000
/** LAN1E wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN1EWR 0x02000000
/** LAN0E wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN0EWR 0x01000000
/** GPONI wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONIWR 0x00100000
/** LAN3I wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN3IWR 0x00080000
/** LAN2I wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN2IWR 0x00040000
/** LAN1I wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN1IWR 0x00020000
/** LAN0I wakeup request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN0IWR 0x00010000
/** FSQM sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_FSQMSR 0x00008000
/** GPONT sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONTSR 0x00002000
/** GPONE sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONESR 0x00001000
/** LAN3E sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN3ESR 0x00000800
/** LAN2E sleep requestThe ENGRESS FIFO of port LAN2 submitted a sleep request.
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN2ESR 0x00000400
/** LAN1E sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN1ESR 0x00000200
/** LAN0E sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN0ESR 0x00000100
/** GPONI sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_GPONISR 0x00000010
/** LAN3I sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN3ISR 0x00000008
/** LAN2I sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN2ISR 0x00000004
/** LAN1I sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN1ISR 0x00000002
/** LAN0I sleep request
    Interrupt control bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSICR_LAN0ISR 0x00000001

/* Fields of "IRNCS Interrupt Enable Register" */
/** FSQM wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_FSQMWR 0x80000000
/* Disable
#define SYS_GPE_IRNCSEN_FSQMWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_FSQMWR_EN 0x80000000
/** GPONT wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONTWR 0x20000000
/* Disable
#define SYS_GPE_IRNCSEN_GPONTWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONTWR_EN 0x20000000
/** GPONE wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONEWR 0x10000000
/* Disable
#define SYS_GPE_IRNCSEN_GPONEWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONEWR_EN 0x10000000
/** LAN3E wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN3EWR 0x08000000
/* Disable
#define SYS_GPE_IRNCSEN_LAN3EWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN3EWR_EN 0x08000000
/** LAN2E wakeup requestThe ENGRESS FIFO of port LAN2 submitted a wakeup request.
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN2EWR 0x04000000
/* Disable
#define SYS_GPE_IRNCSEN_LAN2EWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN2EWR_EN 0x04000000
/** LAN1E wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN1EWR 0x02000000
/* Disable
#define SYS_GPE_IRNCSEN_LAN1EWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN1EWR_EN 0x02000000
/** LAN0E wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN0EWR 0x01000000
/* Disable
#define SYS_GPE_IRNCSEN_LAN0EWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN0EWR_EN 0x01000000
/** GPONI wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONIWR 0x00100000
/* Disable
#define SYS_GPE_IRNCSEN_GPONIWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONIWR_EN 0x00100000
/** LAN3I wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN3IWR 0x00080000
/* Disable
#define SYS_GPE_IRNCSEN_LAN3IWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN3IWR_EN 0x00080000
/** LAN2I wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN2IWR 0x00040000
/* Disable
#define SYS_GPE_IRNCSEN_LAN2IWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN2IWR_EN 0x00040000
/** LAN1I wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN1IWR 0x00020000
/* Disable
#define SYS_GPE_IRNCSEN_LAN1IWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN1IWR_EN 0x00020000
/** LAN0I wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN0IWR 0x00010000
/* Disable
#define SYS_GPE_IRNCSEN_LAN0IWR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN0IWR_EN 0x00010000
/** FSQM sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_FSQMSR 0x00008000
/* Disable
#define SYS_GPE_IRNCSEN_FSQMSR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_FSQMSR_EN 0x00008000
/** GPONT sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONTSR 0x00002000
/* Disable
#define SYS_GPE_IRNCSEN_GPONTSR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONTSR_EN 0x00002000
/** GPONE sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONESR 0x00001000
/* Disable
#define SYS_GPE_IRNCSEN_GPONESR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONESR_EN 0x00001000
/** LAN3E sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN3ESR 0x00000800
/* Disable
#define SYS_GPE_IRNCSEN_LAN3ESR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN3ESR_EN 0x00000800
/** LAN2E sleep requestThe ENGRESS FIFO of port LAN2 submitted a sleep request.
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN2ESR 0x00000400
/* Disable
#define SYS_GPE_IRNCSEN_LAN2ESR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN2ESR_EN 0x00000400
/** LAN1E sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN1ESR 0x00000200
/* Disable
#define SYS_GPE_IRNCSEN_LAN1ESR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN1ESR_EN 0x00000200
/** LAN0E sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN0ESR 0x00000100
/* Disable
#define SYS_GPE_IRNCSEN_LAN0ESR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN0ESR_EN 0x00000100
/** GPONI sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_GPONISR 0x00000010
/* Disable
#define SYS_GPE_IRNCSEN_GPONISR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_GPONISR_EN 0x00000010
/** LAN3I sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN3ISR 0x00000008
/* Disable
#define SYS_GPE_IRNCSEN_LAN3ISR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN3ISR_EN 0x00000008
/** LAN2I sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN2ISR 0x00000004
/* Disable
#define SYS_GPE_IRNCSEN_LAN2ISR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN2ISR_EN 0x00000004
/** LAN1I sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN1ISR 0x00000002
/* Disable
#define SYS_GPE_IRNCSEN_LAN1ISR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN1ISR_EN 0x00000002
/** LAN0I sleep request
    Interrupt enable bit for the corresponding bit in the IRNCSCR register. */
#define SYS_GPE_IRNCSEN_LAN0ISR 0x00000001
/* Disable
#define SYS_GPE_IRNCSEN_LAN0ISR_DIS 0x00000000 */
/** Enable */
#define SYS_GPE_IRNCSEN_LAN0ISR_EN 0x00000001

/*! @} */ /* SYS_GPE_REGISTER */

#endif /* _sys_gpe_reg_h */
