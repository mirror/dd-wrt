/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _sys1_reg_h
#define _sys1_reg_h

/** \addtogroup SYS1_REGISTER
   @{
*/
/* access macros */
#define sys1_r32(reg) reg_r32(&sys1->reg)
#define sys1_w32(val, reg) reg_w32(val, &sys1->reg)
#define sys1_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &sys1->reg)
#define sys1_r32_table(reg, idx) reg_r32_table(sys1->reg, idx)
#define sys1_w32_table(val, reg, idx) reg_w32_table(val, sys1->reg, idx)
#define sys1_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, sys1->reg, idx)
#define sys1_adr_table(reg, idx) adr_table(sys1->reg, idx)


/** SYS1 register structure */
struct gpon_reg_sys1
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
   unsigned int res_1[4]; /* 0x00000030 */
   /** CPU0 Clock Control Register
       Clock control register for CPU0 */
   unsigned int cpu0cc; /* 0x00000040 */
   /** Reserved */
   unsigned int res_2[7]; /* 0x00000044 */
   /** CPU0 Reset Source Register
       Via this register the CPU can find the the root cause for the boot it currently goes through, and take the appropriate measures. */
   unsigned int cpu0rs; /* 0x00000060 */
   /** Reserved */
   unsigned int res_3[7]; /* 0x00000064 */
   /** CPU0 Wakeup Configuration Register
       Controls the wakeup condition for CPU0. Note: The upper 16 bit of this register have to be set to the same value as the mask bits within the yield-resume interface block. If the yield-resume interface is not used at all, set the upper 16 bit to 0. */
   unsigned int cpu0wcfg; /* 0x00000080 */
   /** Reserved */
   unsigned int res_4[7]; /* 0x00000084 */
   /** Bootmode Control Register
       Reflects the bootmode for the CPU and provides means to manipulate it. */
   unsigned int bmc; /* 0x000000A0 */
   /** Reserved */
   unsigned int res_5[3]; /* 0x000000A4 */
   /** Sleep Configuration Register */
   unsigned int scfg; /* 0x000000B0 */
   /** Power Down Configuration Register
       Via this register the configuration is done whether in case of deactivation the power supply of the domain shall be switched off. */
   unsigned int pdcfg; /* 0x000000B4 */
   /** CLKO Pad Control Register
       Controls the behaviour of the CLKO pad/ball. */
   unsigned int clkoc; /* 0x000000B8 */
   /** Infrastructure Control Register
       Controls the behaviour of the components of the infrastructure block. */
   unsigned int infrac; /* 0x000000BC */
   /** HRST_OUT_N Control Register
       Controls the behaviour of the HRST_OUT_N pin. */
   unsigned int hrstoutc; /* 0x000000C0 */
   /** EBU Clock Control Register
       Clock control register for the EBU. */
   unsigned int ebucc; /* 0x000000C4 */
   /** Reserved */
   unsigned int res_6[2]; /* 0x000000C8 */
   /** NMI Status Register
       The Test NMI source is the GPTC counter 1A overflow bit. */
   unsigned int nmis; /* 0x000000D0 */
   /** NMI Set Register */
   unsigned int nmiset; /* 0x000000D4 */
   /** NMI Clear Register */
   unsigned int nmiclr; /* 0x000000D8 */
   /** NMI Test Configuration Register */
   unsigned int nmitcfg; /* 0x000000DC */
   /** NMI VPE1 Control Register */
   unsigned int nmivpe1c; /* 0x000000E0 */
   /** Reserved */
   unsigned int res_7[3]; /* 0x000000E4 */
   /** IRN Capture Register
       This register shows the currently active interrupt events masked with the corresponding enable bits of the IRNEN register. The interrupts can be acknowledged by a write operation. */
   unsigned int irncr; /* 0x000000F0 */
   /** IRN Interrupt Control Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int irnicr; /* 0x000000F4 */
   /** IRN Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IRNCR register and are not signalled via the interrupt line towards the controller. */
   unsigned int irnen; /* 0x000000F8 */
   /** Reserved */
   unsigned int res_8; /* 0x000000FC */
};


/* Fields of "Clock Status Register" */
/** STATUS Clock Enable
    Shows the clock enable bit for the STATUS domain. This domain contains the STATUS block. */
#define CLKS_STATUS 0x80000000
/* Disable
#define CLKS_STATUS_DIS 0x00000000 */
/** Enable */
#define CLKS_STATUS_EN 0x80000000
/** SHA1 Clock Enable
    Shows the clock enable bit for the SHA1 domain. This domain contains the SHA1 block. */
#define CLKS_SHA1 0x40000000
/* Disable
#define CLKS_SHA1_DIS 0x00000000 */
/** Enable */
#define CLKS_SHA1_EN 0x40000000
/** AES Clock Enable
    Shows the clock enable bit for the AES domain. This domain contains the AES block. */
#define CLKS_AES 0x20000000
/* Disable
#define CLKS_AES_DIS 0x00000000 */
/** Enable */
#define CLKS_AES_EN 0x20000000
/** PCM Clock Enable
    Shows the clock enable bit for the PCM domain. This domain contains the PCM interface block. */
#define CLKS_PCM 0x10000000
/* Disable
#define CLKS_PCM_DIS 0x00000000 */
/** Enable */
#define CLKS_PCM_EN 0x10000000
/** FSCT Clock Enable
    Shows the clock enable bit for the FSCT domain. This domain contains the FSCT block. */
#define CLKS_FSCT 0x08000000
/* Disable
#define CLKS_FSCT_DIS 0x00000000 */
/** Enable */
#define CLKS_FSCT_EN 0x08000000
/** GPTC Clock Enable
    Shows the clock enable bit for the GPTC domain. This domain contains the GPTC block. */
#define CLKS_GPTC 0x04000000
/* Disable
#define CLKS_GPTC_DIS 0x00000000 */
/** Enable */
#define CLKS_GPTC_EN 0x04000000
/** MPS Clock Enable
    Shows the clock enable bit for the MPS domain. This domain contains the MPS block. */
#define CLKS_MPS 0x02000000
/* Disable
#define CLKS_MPS_DIS 0x00000000 */
/** Enable */
#define CLKS_MPS_EN 0x02000000
/** DFEV0 Clock Enable
    Shows the clock enable bit for the DFEV0 domain. This domain contains the DFEV0 block. */
#define CLKS_DFEV0 0x01000000
/* Disable
#define CLKS_DFEV0_DIS 0x00000000 */
/** Enable */
#define CLKS_DFEV0_EN 0x01000000
/** PADCTRL4 Clock Enable
    Shows the clock enable bit for the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define CLKS_PADCTRL4 0x00400000
/* Disable
#define CLKS_PADCTRL4_DIS 0x00000000 */
/** Enable */
#define CLKS_PADCTRL4_EN 0x00400000
/** PADCTRL3 Clock Enable
    Shows the clock enable bit for the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define CLKS_PADCTRL3 0x00200000
/* Disable
#define CLKS_PADCTRL3_DIS 0x00000000 */
/** Enable */
#define CLKS_PADCTRL3_EN 0x00200000
/** PADCTRL1 Clock Enable
    Shows the clock enable bit for the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define CLKS_PADCTRL1 0x00100000
/* Disable
#define CLKS_PADCTRL1_DIS 0x00000000 */
/** Enable */
#define CLKS_PADCTRL1_EN 0x00100000
/** P4 Clock Enable
    Shows the clock enable bit for the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define CLKS_P4 0x00040000
/* Disable
#define CLKS_P4_DIS 0x00000000 */
/** Enable */
#define CLKS_P4_EN 0x00040000
/** P3 Clock Enable
    Shows the clock enable bit for the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define CLKS_P3 0x00020000
/* Disable
#define CLKS_P3_DIS 0x00000000 */
/** Enable */
#define CLKS_P3_EN 0x00020000
/** P1 Clock Enable
    Shows the clock enable bit for the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define CLKS_P1 0x00010000
/* Disable
#define CLKS_P1_DIS 0x00000000 */
/** Enable */
#define CLKS_P1_EN 0x00010000
/** HOST Clock Enable
    Shows the clock enable bit for the HOST domain. This domain contains the HOST interface block. */
#define CLKS_HOST 0x00008000
/* Disable
#define CLKS_HOST_DIS 0x00000000 */
/** Enable */
#define CLKS_HOST_EN 0x00008000
/** I2C Clock Enable
    Shows the clock enable bit for the I2C domain. This domain contains the I2C interface block. */
#define CLKS_I2C 0x00004000
/* Disable
#define CLKS_I2C_DIS 0x00000000 */
/** Enable */
#define CLKS_I2C_EN 0x00004000
/** SSC0 Clock Enable
    Shows the clock enable bit for the SSC0 domain. This domain contains the SSC0 interface block. */
#define CLKS_SSC0 0x00002000
/* Disable
#define CLKS_SSC0_DIS 0x00000000 */
/** Enable */
#define CLKS_SSC0_EN 0x00002000
/** ASC0 Clock Enable
    Shows the clock enable bit for the ASC0 domain. This domain contains the ASC0 interface block. */
#define CLKS_ASC0 0x00001000
/* Disable
#define CLKS_ASC0_DIS 0x00000000 */
/** Enable */
#define CLKS_ASC0_EN 0x00001000
/** ASC1 Clock Enable
    Shows the clock enable bit for the ASC1 domain. This domain contains the ASC1 block. */
#define CLKS_ASC1 0x00000800
/* Disable
#define CLKS_ASC1_DIS 0x00000000 */
/** Enable */
#define CLKS_ASC1_EN 0x00000800
/** DCDCAPD Clock Enable
    Shows the clock enable bit for the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define CLKS_DCDCAPD 0x00000400
/* Disable
#define CLKS_DCDCAPD_DIS 0x00000000 */
/** Enable */
#define CLKS_DCDCAPD_EN 0x00000400
/** DCDCDDR Clock Enable
    Shows the clock enable bit for the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define CLKS_DCDCDDR 0x00000200
/* Disable
#define CLKS_DCDCDDR_DIS 0x00000000 */
/** Enable */
#define CLKS_DCDCDDR_EN 0x00000200
/** DCDC1V0 Clock Enable
    Shows the clock enable bit for the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define CLKS_DCDC1V0 0x00000100
/* Disable
#define CLKS_DCDC1V0_DIS 0x00000000 */
/** Enable */
#define CLKS_DCDC1V0_EN 0x00000100
/** TRC2MEM Clock Enable
    Shows the clock enable bit for the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define CLKS_TRC2MEM 0x00000040
/* Disable
#define CLKS_TRC2MEM_DIS 0x00000000 */
/** Enable */
#define CLKS_TRC2MEM_EN 0x00000040
/** DDR Clock Enable
    Shows the clock enable bit for the DDR domain. This domain contains the DDR interface block. */
#define CLKS_DDR 0x00000020
/* Disable
#define CLKS_DDR_DIS 0x00000000 */
/** Enable */
#define CLKS_DDR_EN 0x00000020
/** EBU Clock Enable
    Shows the clock enable bit for the EBU domain. This domain contains the EBU interface block. */
#define CLKS_EBU 0x00000010
/* Disable
#define CLKS_EBU_DIS 0x00000000 */
/** Enable */
#define CLKS_EBU_EN 0x00000010

/* Fields of "Clock Enable Register" */
/** Set Clock Enable STATUS
    Sets the clock enable bit of the STATUS domain. This domain contains the STATUS block. */
#define CLKEN_STATUS 0x80000000
/* No-Operation
#define CLKEN_STATUS_NOP 0x00000000 */
/** Set */
#define CLKEN_STATUS_SET 0x80000000
/** Set Clock Enable SHA1
    Sets the clock enable bit of the SHA1 domain. This domain contains the SHA1 block. */
#define CLKEN_SHA1 0x40000000
/* No-Operation
#define CLKEN_SHA1_NOP 0x00000000 */
/** Set */
#define CLKEN_SHA1_SET 0x40000000
/** Set Clock Enable AES
    Sets the clock enable bit of the AES domain. This domain contains the AES block. */
#define CLKEN_AES 0x20000000
/* No-Operation
#define CLKEN_AES_NOP 0x00000000 */
/** Set */
#define CLKEN_AES_SET 0x20000000
/** Set Clock Enable PCM
    Sets the clock enable bit of the PCM domain. This domain contains the PCM interface block. */
#define CLKEN_PCM 0x10000000
/* No-Operation
#define CLKEN_PCM_NOP 0x00000000 */
/** Set */
#define CLKEN_PCM_SET 0x10000000
/** Set Clock Enable FSCT
    Sets the clock enable bit of the FSCT domain. This domain contains the FSCT block. */
#define CLKEN_FSCT 0x08000000
/* No-Operation
#define CLKEN_FSCT_NOP 0x00000000 */
/** Set */
#define CLKEN_FSCT_SET 0x08000000
/** Set Clock Enable GPTC
    Sets the clock enable bit of the GPTC domain. This domain contains the GPTC block. */
#define CLKEN_GPTC 0x04000000
/* No-Operation
#define CLKEN_GPTC_NOP 0x00000000 */
/** Set */
#define CLKEN_GPTC_SET 0x04000000
/** Set Clock Enable MPS
    Sets the clock enable bit of the MPS domain. This domain contains the MPS block. */
#define CLKEN_MPS 0x02000000
/* No-Operation
#define CLKEN_MPS_NOP 0x00000000 */
/** Set */
#define CLKEN_MPS_SET 0x02000000
/** Set Clock Enable DFEV0
    Sets the clock enable bit of the DFEV0 domain. This domain contains the DFEV0 block. */
#define CLKEN_DFEV0 0x01000000
/* No-Operation
#define CLKEN_DFEV0_NOP 0x00000000 */
/** Set */
#define CLKEN_DFEV0_SET 0x01000000
/** Set Clock Enable PADCTRL4
    Sets the clock enable bit of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define CLKEN_PADCTRL4 0x00400000
/* No-Operation
#define CLKEN_PADCTRL4_NOP 0x00000000 */
/** Set */
#define CLKEN_PADCTRL4_SET 0x00400000
/** Set Clock Enable PADCTRL3
    Sets the clock enable bit of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define CLKEN_PADCTRL3 0x00200000
/* No-Operation
#define CLKEN_PADCTRL3_NOP 0x00000000 */
/** Set */
#define CLKEN_PADCTRL3_SET 0x00200000
/** Set Clock Enable PADCTRL1
    Sets the clock enable bit of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define CLKEN_PADCTRL1 0x00100000
/* No-Operation
#define CLKEN_PADCTRL1_NOP 0x00000000 */
/** Set */
#define CLKEN_PADCTRL1_SET 0x00100000
/** Set Clock Enable P4
    Sets the clock enable bit of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define CLKEN_P4 0x00040000
/* No-Operation
#define CLKEN_P4_NOP 0x00000000 */
/** Set */
#define CLKEN_P4_SET 0x00040000
/** Set Clock Enable P3
    Sets the clock enable bit of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define CLKEN_P3 0x00020000
/* No-Operation
#define CLKEN_P3_NOP 0x00000000 */
/** Set */
#define CLKEN_P3_SET 0x00020000
/** Set Clock Enable P1
    Sets the clock enable bit of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define CLKEN_P1 0x00010000
/* No-Operation
#define CLKEN_P1_NOP 0x00000000 */
/** Set */
#define CLKEN_P1_SET 0x00010000
/** Set Clock Enable HOST
    Sets the clock enable bit of the HOST domain. This domain contains the HOST interface block. */
#define CLKEN_HOST 0x00008000
/* No-Operation
#define CLKEN_HOST_NOP 0x00000000 */
/** Set */
#define CLKEN_HOST_SET 0x00008000
/** Set Clock Enable I2C
    Sets the clock enable bit of the I2C domain. This domain contains the I2C interface block. */
#define CLKEN_I2C 0x00004000
/* No-Operation
#define CLKEN_I2C_NOP 0x00000000 */
/** Set */
#define CLKEN_I2C_SET 0x00004000
/** Set Clock Enable SSC0
    Sets the clock enable bit of the SSC0 domain. This domain contains the SSC0 interface block. */
#define CLKEN_SSC0 0x00002000
/* No-Operation
#define CLKEN_SSC0_NOP 0x00000000 */
/** Set */
#define CLKEN_SSC0_SET 0x00002000
/** Set Clock Enable ASC0
    Sets the clock enable bit of the ASC0 domain. This domain contains the ASC0 interface block. */
#define CLKEN_ASC0 0x00001000
/* No-Operation
#define CLKEN_ASC0_NOP 0x00000000 */
/** Set */
#define CLKEN_ASC0_SET 0x00001000
/** Set Clock Enable ASC1
    Sets the clock enable bit of the ASC1 domain. This domain contains the ASC1 block. */
#define CLKEN_ASC1 0x00000800
/* No-Operation
#define CLKEN_ASC1_NOP 0x00000000 */
/** Set */
#define CLKEN_ASC1_SET 0x00000800
/** Set Clock Enable DCDCAPD
    Sets the clock enable bit of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define CLKEN_DCDCAPD 0x00000400
/* No-Operation
#define CLKEN_DCDCAPD_NOP 0x00000000 */
/** Set */
#define CLKEN_DCDCAPD_SET 0x00000400
/** Set Clock Enable DCDCDDR
    Sets the clock enable bit of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define CLKEN_DCDCDDR 0x00000200
/* No-Operation
#define CLKEN_DCDCDDR_NOP 0x00000000 */
/** Set */
#define CLKEN_DCDCDDR_SET 0x00000200
/** Set Clock Enable DCDC1V0
    Sets the clock enable bit of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define CLKEN_DCDC1V0 0x00000100
/* No-Operation
#define CLKEN_DCDC1V0_NOP 0x00000000 */
/** Set */
#define CLKEN_DCDC1V0_SET 0x00000100
/** Set Clock Enable TRC2MEM
    Sets the clock enable bit of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define CLKEN_TRC2MEM 0x00000040
/* No-Operation
#define CLKEN_TRC2MEM_NOP 0x00000000 */
/** Set */
#define CLKEN_TRC2MEM_SET 0x00000040
/** Set Clock Enable DDR
    Sets the clock enable bit of the DDR domain. This domain contains the DDR interface block. */
#define CLKEN_DDR 0x00000020
/* No-Operation
#define CLKEN_DDR_NOP 0x00000000 */
/** Set */
#define CLKEN_DDR_SET 0x00000020
/** Set Clock Enable EBU
    Sets the clock enable bit of the EBU domain. This domain contains the EBU interface block. */
#define CLKEN_EBU 0x00000010
/* No-Operation
#define CLKEN_EBU_NOP 0x00000000 */
/** Set */
#define CLKEN_EBU_SET 0x00000010

/* Fields of "Clock Clear Register" */
/** Clear Clock Enable STATUS
    Clears the clock enable bit of the STATUS domain. This domain contains the STATUS block. */
#define CLKCLR_STATUS 0x80000000
/* No-Operation
#define CLKCLR_STATUS_NOP 0x00000000 */
/** Clear */
#define CLKCLR_STATUS_CLR 0x80000000
/** Clear Clock Enable SHA1
    Clears the clock enable bit of the SHA1 domain. This domain contains the SHA1 block. */
#define CLKCLR_SHA1 0x40000000
/* No-Operation
#define CLKCLR_SHA1_NOP 0x00000000 */
/** Clear */
#define CLKCLR_SHA1_CLR 0x40000000
/** Clear Clock Enable AES
    Clears the clock enable bit of the AES domain. This domain contains the AES block. */
#define CLKCLR_AES 0x20000000
/* No-Operation
#define CLKCLR_AES_NOP 0x00000000 */
/** Clear */
#define CLKCLR_AES_CLR 0x20000000
/** Clear Clock Enable PCM
    Clears the clock enable bit of the PCM domain. This domain contains the PCM interface block. */
#define CLKCLR_PCM 0x10000000
/* No-Operation
#define CLKCLR_PCM_NOP 0x00000000 */
/** Clear */
#define CLKCLR_PCM_CLR 0x10000000
/** Clear Clock Enable FSCT
    Clears the clock enable bit of the FSCT domain. This domain contains the FSCT block. */
#define CLKCLR_FSCT 0x08000000
/* No-Operation
#define CLKCLR_FSCT_NOP 0x00000000 */
/** Clear */
#define CLKCLR_FSCT_CLR 0x08000000
/** Clear Clock Enable GPTC
    Clears the clock enable bit of the GPTC domain. This domain contains the GPTC block. */
#define CLKCLR_GPTC 0x04000000
/* No-Operation
#define CLKCLR_GPTC_NOP 0x00000000 */
/** Clear */
#define CLKCLR_GPTC_CLR 0x04000000
/** Clear Clock Enable MPS
    Clears the clock enable bit of the MPS domain. This domain contains the MPS block. */
#define CLKCLR_MPS 0x02000000
/* No-Operation
#define CLKCLR_MPS_NOP 0x00000000 */
/** Clear */
#define CLKCLR_MPS_CLR 0x02000000
/** Clear Clock Enable DFEV0
    Clears the clock enable bit of the DFEV0 domain. This domain contains the DFEV0 block. */
#define CLKCLR_DFEV0 0x01000000
/* No-Operation
#define CLKCLR_DFEV0_NOP 0x00000000 */
/** Clear */
#define CLKCLR_DFEV0_CLR 0x01000000
/** Clear Clock Enable PADCTRL4
    Clears the clock enable bit of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define CLKCLR_PADCTRL4 0x00400000
/* No-Operation
#define CLKCLR_PADCTRL4_NOP 0x00000000 */
/** Clear */
#define CLKCLR_PADCTRL4_CLR 0x00400000
/** Clear Clock Enable PADCTRL3
    Clears the clock enable bit of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define CLKCLR_PADCTRL3 0x00200000
/* No-Operation
#define CLKCLR_PADCTRL3_NOP 0x00000000 */
/** Clear */
#define CLKCLR_PADCTRL3_CLR 0x00200000
/** Clear Clock Enable PADCTRL1
    Clears the clock enable bit of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define CLKCLR_PADCTRL1 0x00100000
/* No-Operation
#define CLKCLR_PADCTRL1_NOP 0x00000000 */
/** Clear */
#define CLKCLR_PADCTRL1_CLR 0x00100000
/** Clear Clock Enable P4
    Clears the clock enable bit of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define CLKCLR_P4 0x00040000
/* No-Operation
#define CLKCLR_P4_NOP 0x00000000 */
/** Clear */
#define CLKCLR_P4_CLR 0x00040000
/** Clear Clock Enable P3
    Clears the clock enable bit of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define CLKCLR_P3 0x00020000
/* No-Operation
#define CLKCLR_P3_NOP 0x00000000 */
/** Clear */
#define CLKCLR_P3_CLR 0x00020000
/** Clear Clock Enable P1
    Clears the clock enable bit of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define CLKCLR_P1 0x00010000
/* No-Operation
#define CLKCLR_P1_NOP 0x00000000 */
/** Clear */
#define CLKCLR_P1_CLR 0x00010000
/** Clear Clock Enable HOST
    Clears the clock enable bit of the HOST domain. This domain contains the HOST interface block. */
#define CLKCLR_HOST 0x00008000
/* No-Operation
#define CLKCLR_HOST_NOP 0x00000000 */
/** Clear */
#define CLKCLR_HOST_CLR 0x00008000
/** Clear Clock Enable I2C
    Clears the clock enable bit of the I2C domain. This domain contains the I2C interface block. */
#define CLKCLR_I2C 0x00004000
/* No-Operation
#define CLKCLR_I2C_NOP 0x00000000 */
/** Clear */
#define CLKCLR_I2C_CLR 0x00004000
/** Clear Clock Enable SSC0
    Clears the clock enable bit of the SSC0 domain. This domain contains the SSC0 interface block. */
#define CLKCLR_SSC0 0x00002000
/* No-Operation
#define CLKCLR_SSC0_NOP 0x00000000 */
/** Clear */
#define CLKCLR_SSC0_CLR 0x00002000
/** Clear Clock Enable ASC0
    Clears the clock enable bit of the ASC0 domain. This domain contains the ASC0 interface block. */
#define CLKCLR_ASC0 0x00001000
/* No-Operation
#define CLKCLR_ASC0_NOP 0x00000000 */
/** Clear */
#define CLKCLR_ASC0_CLR 0x00001000
/** Clear Clock Enable ASC1
    Clears the clock enable bit of the ASC1 domain. This domain contains the ASC1 block. */
#define CLKCLR_ASC1 0x00000800
/* No-Operation
#define CLKCLR_ASC1_NOP 0x00000000 */
/** Clear */
#define CLKCLR_ASC1_CLR 0x00000800
/** Clear Clock Enable DCDCAPD
    Clears the clock enable bit of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define CLKCLR_DCDCAPD 0x00000400
/* No-Operation
#define CLKCLR_DCDCAPD_NOP 0x00000000 */
/** Clear */
#define CLKCLR_DCDCAPD_CLR 0x00000400
/** Clear Clock Enable DCDCDDR
    Clears the clock enable bit of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define CLKCLR_DCDCDDR 0x00000200
/* No-Operation
#define CLKCLR_DCDCDDR_NOP 0x00000000 */
/** Clear */
#define CLKCLR_DCDCDDR_CLR 0x00000200
/** Clear Clock Enable DCDC1V0
    Clears the clock enable bit of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define CLKCLR_DCDC1V0 0x00000100
/* No-Operation
#define CLKCLR_DCDC1V0_NOP 0x00000000 */
/** Clear */
#define CLKCLR_DCDC1V0_CLR 0x00000100
/** Clear Clock Enable TRC2MEM
    Clears the clock enable bit of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define CLKCLR_TRC2MEM 0x00000040
/* No-Operation
#define CLKCLR_TRC2MEM_NOP 0x00000000 */
/** Clear */
#define CLKCLR_TRC2MEM_CLR 0x00000040
/** Clear Clock Enable DDR
    Clears the clock enable bit of the DDR domain. This domain contains the DDR interface block. */
#define CLKCLR_DDR 0x00000020
/* No-Operation
#define CLKCLR_DDR_NOP 0x00000000 */
/** Clear */
#define CLKCLR_DDR_CLR 0x00000020
/** Clear Clock Enable EBU
    Clears the clock enable bit of the EBU domain. This domain contains the EBU interface block. */
#define CLKCLR_EBU 0x00000010
/* No-Operation
#define CLKCLR_EBU_NOP 0x00000000 */
/** Clear */
#define CLKCLR_EBU_CLR 0x00000010

/* Fields of "Activation Status Register" */
/** STATUS Status
    Shows the activation status of the STATUS domain. This domain contains the STATUS block. */
#define ACTS_STATUS 0x80000000
/* The block is inactive.
#define ACTS_STATUS_INACT 0x00000000 */
/** The block is active. */
#define ACTS_STATUS_ACT 0x80000000
/** SHA1 Status
    Shows the activation status of the SHA1 domain. This domain contains the SHA1 block. */
#define ACTS_SHA1 0x40000000
/* The block is inactive.
#define ACTS_SHA1_INACT 0x00000000 */
/** The block is active. */
#define ACTS_SHA1_ACT 0x40000000
/** AES Status
    Shows the activation status of the AES domain. This domain contains the AES block. */
#define ACTS_AES 0x20000000
/* The block is inactive.
#define ACTS_AES_INACT 0x00000000 */
/** The block is active. */
#define ACTS_AES_ACT 0x20000000
/** PCM Status
    Shows the activation status of the PCM domain. This domain contains the PCM interface block. */
#define ACTS_PCM 0x10000000
/* The block is inactive.
#define ACTS_PCM_INACT 0x00000000 */
/** The block is active. */
#define ACTS_PCM_ACT 0x10000000
/** FSCT Status
    Shows the activation status of the FSCT domain. This domain contains the FSCT block. */
#define ACTS_FSCT 0x08000000
/* The block is inactive.
#define ACTS_FSCT_INACT 0x00000000 */
/** The block is active. */
#define ACTS_FSCT_ACT 0x08000000
/** GPTC Status
    Shows the activation status of the GPTC domain. This domain contains the GPTC block. */
#define ACTS_GPTC 0x04000000
/* The block is inactive.
#define ACTS_GPTC_INACT 0x00000000 */
/** The block is active. */
#define ACTS_GPTC_ACT 0x04000000
/** MPS Status
    Shows the activation status of the MPS domain. This domain contains the MPS block. */
#define ACTS_MPS 0x02000000
/* The block is inactive.
#define ACTS_MPS_INACT 0x00000000 */
/** The block is active. */
#define ACTS_MPS_ACT 0x02000000
/** DFEV0 Status
    Shows the activation status of the DFEV0 domain. This domain contains the DFEV0 block. */
#define ACTS_DFEV0 0x01000000
/* The block is inactive.
#define ACTS_DFEV0_INACT 0x00000000 */
/** The block is active. */
#define ACTS_DFEV0_ACT 0x01000000
/** PADCTRL4 Status
    Shows the activation status of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define ACTS_PADCTRL4 0x00400000
/* The block is inactive.
#define ACTS_PADCTRL4_INACT 0x00000000 */
/** The block is active. */
#define ACTS_PADCTRL4_ACT 0x00400000
/** PADCTRL3 Status
    Shows the activation status of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define ACTS_PADCTRL3 0x00200000
/* The block is inactive.
#define ACTS_PADCTRL3_INACT 0x00000000 */
/** The block is active. */
#define ACTS_PADCTRL3_ACT 0x00200000
/** PADCTRL1 Status
    Shows the activation status of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define ACTS_PADCTRL1 0x00100000
/* The block is inactive.
#define ACTS_PADCTRL1_INACT 0x00000000 */
/** The block is active. */
#define ACTS_PADCTRL1_ACT 0x00100000
/** P4 Status
    Shows the activation status of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define ACTS_P4 0x00040000
/* The block is inactive.
#define ACTS_P4_INACT 0x00000000 */
/** The block is active. */
#define ACTS_P4_ACT 0x00040000
/** P3 Status
    Shows the activation status of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define ACTS_P3 0x00020000
/* The block is inactive.
#define ACTS_P3_INACT 0x00000000 */
/** The block is active. */
#define ACTS_P3_ACT 0x00020000
/** P1 Status
    Shows the activation status of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define ACTS_P1 0x00010000
/* The block is inactive.
#define ACTS_P1_INACT 0x00000000 */
/** The block is active. */
#define ACTS_P1_ACT 0x00010000
/** HOST Status
    Shows the activation status of the HOST domain. This domain contains the HOST interface block. */
#define ACTS_HOST 0x00008000
/* The block is inactive.
#define ACTS_HOST_INACT 0x00000000 */
/** The block is active. */
#define ACTS_HOST_ACT 0x00008000
/** I2C Status
    Shows the activation status of the I2C domain. This domain contains the I2C interface block. */
#define ACTS_I2C 0x00004000
/* The block is inactive.
#define ACTS_I2C_INACT 0x00000000 */
/** The block is active. */
#define ACTS_I2C_ACT 0x00004000
/** SSC0 Status
    Shows the activation status of the SSC0 domain. This domain contains the SSC0 interface block. */
#define ACTS_SSC0 0x00002000
/* The block is inactive.
#define ACTS_SSC0_INACT 0x00000000 */
/** The block is active. */
#define ACTS_SSC0_ACT 0x00002000
/** ASC0 Status
    Shows the activation status of the ASC0 domain. This domain contains the ASC0 interface block. */
#define ACTS_ASC0 0x00001000
/* The block is inactive.
#define ACTS_ASC0_INACT 0x00000000 */
/** The block is active. */
#define ACTS_ASC0_ACT 0x00001000
/** ASC1 Status
    Shows the activation status of the ASC1 domain. This domain contains the ASC1 block. */
#define ACTS_ASC1 0x00000800
/* The block is inactive.
#define ACTS_ASC1_INACT 0x00000000 */
/** The block is active. */
#define ACTS_ASC1_ACT 0x00000800
/** DCDCAPD Status
    Shows the activation status of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define ACTS_DCDCAPD 0x00000400
/* The block is inactive.
#define ACTS_DCDCAPD_INACT 0x00000000 */
/** The block is active. */
#define ACTS_DCDCAPD_ACT 0x00000400
/** DCDCDDR Status
    Shows the activation status of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define ACTS_DCDCDDR 0x00000200
/* The block is inactive.
#define ACTS_DCDCDDR_INACT 0x00000000 */
/** The block is active. */
#define ACTS_DCDCDDR_ACT 0x00000200
/** DCDC1V0 Status
    Shows the activation status of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define ACTS_DCDC1V0 0x00000100
/* The block is inactive.
#define ACTS_DCDC1V0_INACT 0x00000000 */
/** The block is active. */
#define ACTS_DCDC1V0_ACT 0x00000100
/** TRC2MEM Status
    Shows the activation status of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define ACTS_TRC2MEM 0x00000040
/* The block is inactive.
#define ACTS_TRC2MEM_INACT 0x00000000 */
/** The block is active. */
#define ACTS_TRC2MEM_ACT 0x00000040
/** DDR Status
    Shows the activation status of the DDR domain. This domain contains the DDR interface block. */
#define ACTS_DDR 0x00000020
/* The block is inactive.
#define ACTS_DDR_INACT 0x00000000 */
/** The block is active. */
#define ACTS_DDR_ACT 0x00000020
/** EBU Status
    Shows the activation status of the EBU domain. This domain contains the EBU interface block. */
#define ACTS_EBU 0x00000010
/* The block is inactive.
#define ACTS_EBU_INACT 0x00000000 */
/** The block is active. */
#define ACTS_EBU_ACT 0x00000010

/* Fields of "Activation Register" */
/** Activate STATUS
    Sets the activation flag of the STATUS domain. This domain contains the STATUS block. */
#define ACT_STATUS 0x80000000
/* No-Operation
#define ACT_STATUS_NOP 0x00000000 */
/** Set */
#define ACT_STATUS_SET 0x80000000
/** Activate SHA1
    Sets the activation flag of the SHA1 domain. This domain contains the SHA1 block. */
#define ACT_SHA1 0x40000000
/* No-Operation
#define ACT_SHA1_NOP 0x00000000 */
/** Set */
#define ACT_SHA1_SET 0x40000000
/** Activate AES
    Sets the activation flag of the AES domain. This domain contains the AES block. */
#define ACT_AES 0x20000000
/* No-Operation
#define ACT_AES_NOP 0x00000000 */
/** Set */
#define ACT_AES_SET 0x20000000
/** Activate PCM
    Sets the activation flag of the PCM domain. This domain contains the PCM interface block. */
#define ACT_PCM 0x10000000
/* No-Operation
#define ACT_PCM_NOP 0x00000000 */
/** Set */
#define ACT_PCM_SET 0x10000000
/** Activate FSCT
    Sets the activation flag of the FSCT domain. This domain contains the FSCT block. */
#define ACT_FSCT 0x08000000
/* No-Operation
#define ACT_FSCT_NOP 0x00000000 */
/** Set */
#define ACT_FSCT_SET 0x08000000
/** Activate GPTC
    Sets the activation flag of the GPTC domain. This domain contains the GPTC block. */
#define ACT_GPTC 0x04000000
/* No-Operation
#define ACT_GPTC_NOP 0x00000000 */
/** Set */
#define ACT_GPTC_SET 0x04000000
/** Activate MPS
    Sets the activation flag of the MPS domain. This domain contains the MPS block. */
#define ACT_MPS 0x02000000
/* No-Operation
#define ACT_MPS_NOP 0x00000000 */
/** Set */
#define ACT_MPS_SET 0x02000000
/** Activate DFEV0
    Sets the activation flag of the DFEV0 domain. This domain contains the DFEV0 block. */
#define ACT_DFEV0 0x01000000
/* No-Operation
#define ACT_DFEV0_NOP 0x00000000 */
/** Set */
#define ACT_DFEV0_SET 0x01000000
/** Activate PADCTRL4
    Sets the activation flag of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define ACT_PADCTRL4 0x00400000
/* No-Operation
#define ACT_PADCTRL4_NOP 0x00000000 */
/** Set */
#define ACT_PADCTRL4_SET 0x00400000
/** Activate PADCTRL3
    Sets the activation flag of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define ACT_PADCTRL3 0x00200000
/* No-Operation
#define ACT_PADCTRL3_NOP 0x00000000 */
/** Set */
#define ACT_PADCTRL3_SET 0x00200000
/** Activate PADCTRL1
    Sets the activation flag of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define ACT_PADCTRL1 0x00100000
/* No-Operation
#define ACT_PADCTRL1_NOP 0x00000000 */
/** Set */
#define ACT_PADCTRL1_SET 0x00100000
/** Activate P4
    Sets the activation flag of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define ACT_P4 0x00040000
/* No-Operation
#define ACT_P4_NOP 0x00000000 */
/** Set */
#define ACT_P4_SET 0x00040000
/** Activate P3
    Sets the activation flag of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define ACT_P3 0x00020000
/* No-Operation
#define ACT_P3_NOP 0x00000000 */
/** Set */
#define ACT_P3_SET 0x00020000
/** Activate P1
    Sets the activation flag of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define ACT_P1 0x00010000
/* No-Operation
#define ACT_P1_NOP 0x00000000 */
/** Set */
#define ACT_P1_SET 0x00010000
/** Activate HOST
    Sets the activation flag of the HOST domain. This domain contains the HOST interface block. */
#define ACT_HOST 0x00008000
/* No-Operation
#define ACT_HOST_NOP 0x00000000 */
/** Set */
#define ACT_HOST_SET 0x00008000
/** Activate I2C
    Sets the activation flag of the I2C domain. This domain contains the I2C interface block. */
#define ACT_I2C 0x00004000
/* No-Operation
#define ACT_I2C_NOP 0x00000000 */
/** Set */
#define ACT_I2C_SET 0x00004000
/** Activate SSC0
    Sets the activation flag of the SSC0 domain. This domain contains the SSC0 interface block. */
#define ACT_SSC0 0x00002000
/* No-Operation
#define ACT_SSC0_NOP 0x00000000 */
/** Set */
#define ACT_SSC0_SET 0x00002000
/** Activate ASC0
    Sets the activation flag of the ASC0 domain. This domain contains the ASC0 interface block. */
#define ACT_ASC0 0x00001000
/* No-Operation
#define ACT_ASC0_NOP 0x00000000 */
/** Set */
#define ACT_ASC0_SET 0x00001000
/** Activate ASC1
    Sets the activation flag of the ASC1 domain. This domain contains the ASC1 block. */
#define ACT_ASC1 0x00000800
/* No-Operation
#define ACT_ASC1_NOP 0x00000000 */
/** Set */
#define ACT_ASC1_SET 0x00000800
/** Activate DCDCAPD
    Sets the activation flag of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define ACT_DCDCAPD 0x00000400
/* No-Operation
#define ACT_DCDCAPD_NOP 0x00000000 */
/** Set */
#define ACT_DCDCAPD_SET 0x00000400
/** Activate DCDCDDR
    Sets the activation flag of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define ACT_DCDCDDR 0x00000200
/* No-Operation
#define ACT_DCDCDDR_NOP 0x00000000 */
/** Set */
#define ACT_DCDCDDR_SET 0x00000200
/** Activate DCDC1V0
    Sets the activation flag of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define ACT_DCDC1V0 0x00000100
/* No-Operation
#define ACT_DCDC1V0_NOP 0x00000000 */
/** Set */
#define ACT_DCDC1V0_SET 0x00000100
/** Activate TRC2MEM
    Sets the activation flag of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define ACT_TRC2MEM 0x00000040
/* No-Operation
#define ACT_TRC2MEM_NOP 0x00000000 */
/** Set */
#define ACT_TRC2MEM_SET 0x00000040
/** Activate DDR
    Sets the activation flag of the DDR domain. This domain contains the DDR interface block. */
#define ACT_DDR 0x00000020
/* No-Operation
#define ACT_DDR_NOP 0x00000000 */
/** Set */
#define ACT_DDR_SET 0x00000020
/** Activate EBU
    Sets the activation flag of the EBU domain. This domain contains the EBU interface block. */
#define ACT_EBU 0x00000010
/* No-Operation
#define ACT_EBU_NOP 0x00000000 */
/** Set */
#define ACT_EBU_SET 0x00000010

/* Fields of "Deactivation Register" */
/** Deactivate STATUS
    Clears the activation flag of the STATUS domain. This domain contains the STATUS block. */
#define DEACT_STATUS 0x80000000
/* No-Operation
#define DEACT_STATUS_NOP 0x00000000 */
/** Clear */
#define DEACT_STATUS_CLR 0x80000000
/** Deactivate SHA1
    Clears the activation flag of the SHA1 domain. This domain contains the SHA1 block. */
#define DEACT_SHA1 0x40000000
/* No-Operation
#define DEACT_SHA1_NOP 0x00000000 */
/** Clear */
#define DEACT_SHA1_CLR 0x40000000
/** Deactivate AES
    Clears the activation flag of the AES domain. This domain contains the AES block. */
#define DEACT_AES 0x20000000
/* No-Operation
#define DEACT_AES_NOP 0x00000000 */
/** Clear */
#define DEACT_AES_CLR 0x20000000
/** Deactivate PCM
    Clears the activation flag of the PCM domain. This domain contains the PCM interface block. */
#define DEACT_PCM 0x10000000
/* No-Operation
#define DEACT_PCM_NOP 0x00000000 */
/** Clear */
#define DEACT_PCM_CLR 0x10000000
/** Deactivate FSCT
    Clears the activation flag of the FSCT domain. This domain contains the FSCT block. */
#define DEACT_FSCT 0x08000000
/* No-Operation
#define DEACT_FSCT_NOP 0x00000000 */
/** Clear */
#define DEACT_FSCT_CLR 0x08000000
/** Deactivate GPTC
    Clears the activation flag of the GPTC domain. This domain contains the GPTC block. */
#define DEACT_GPTC 0x04000000
/* No-Operation
#define DEACT_GPTC_NOP 0x00000000 */
/** Clear */
#define DEACT_GPTC_CLR 0x04000000
/** Deactivate MPS
    Clears the activation flag of the MPS domain. This domain contains the MPS block. */
#define DEACT_MPS 0x02000000
/* No-Operation
#define DEACT_MPS_NOP 0x00000000 */
/** Clear */
#define DEACT_MPS_CLR 0x02000000
/** Deactivate DFEV0
    Clears the activation flag of the DFEV0 domain. This domain contains the DFEV0 block. */
#define DEACT_DFEV0 0x01000000
/* No-Operation
#define DEACT_DFEV0_NOP 0x00000000 */
/** Clear */
#define DEACT_DFEV0_CLR 0x01000000
/** Deactivate PADCTRL4
    Clears the activation flag of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define DEACT_PADCTRL4 0x00400000
/* No-Operation
#define DEACT_PADCTRL4_NOP 0x00000000 */
/** Clear */
#define DEACT_PADCTRL4_CLR 0x00400000
/** Deactivate PADCTRL3
    Clears the activation flag of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define DEACT_PADCTRL3 0x00200000
/* No-Operation
#define DEACT_PADCTRL3_NOP 0x00000000 */
/** Clear */
#define DEACT_PADCTRL3_CLR 0x00200000
/** Deactivate PADCTRL1
    Clears the activation flag of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define DEACT_PADCTRL1 0x00100000
/* No-Operation
#define DEACT_PADCTRL1_NOP 0x00000000 */
/** Clear */
#define DEACT_PADCTRL1_CLR 0x00100000
/** Deactivate P4
    Clears the activation flag of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define DEACT_P4 0x00040000
/* No-Operation
#define DEACT_P4_NOP 0x00000000 */
/** Clear */
#define DEACT_P4_CLR 0x00040000
/** Deactivate P3
    Clears the activation flag of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define DEACT_P3 0x00020000
/* No-Operation
#define DEACT_P3_NOP 0x00000000 */
/** Clear */
#define DEACT_P3_CLR 0x00020000
/** Deactivate P1
    Clears the activation flag of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define DEACT_P1 0x00010000
/* No-Operation
#define DEACT_P1_NOP 0x00000000 */
/** Clear */
#define DEACT_P1_CLR 0x00010000
/** Deactivate HOST
    Clears the activation flag of the HOST domain. This domain contains the HOST interface block. */
#define DEACT_HOST 0x00008000
/* No-Operation
#define DEACT_HOST_NOP 0x00000000 */
/** Clear */
#define DEACT_HOST_CLR 0x00008000
/** Deactivate I2C
    Clears the activation flag of the I2C domain. This domain contains the I2C interface block. */
#define DEACT_I2C 0x00004000
/* No-Operation
#define DEACT_I2C_NOP 0x00000000 */
/** Clear */
#define DEACT_I2C_CLR 0x00004000
/** Deactivate SSC0
    Clears the activation flag of the SSC0 domain. This domain contains the SSC0 interface block. */
#define DEACT_SSC0 0x00002000
/* No-Operation
#define DEACT_SSC0_NOP 0x00000000 */
/** Clear */
#define DEACT_SSC0_CLR 0x00002000
/** Deactivate ASC0
    Clears the activation flag of the ASC0 domain. This domain contains the ASC0 interface block. */
#define DEACT_ASC0 0x00001000
/* No-Operation
#define DEACT_ASC0_NOP 0x00000000 */
/** Clear */
#define DEACT_ASC0_CLR 0x00001000
/** Deactivate ASC1
    Clears the activation flag of the ASC1 domain. This domain contains the ASC1 block. */
#define DEACT_ASC1 0x00000800
/* No-Operation
#define DEACT_ASC1_NOP 0x00000000 */
/** Clear */
#define DEACT_ASC1_CLR 0x00000800
/** Deactivate DCDCAPD
    Clears the activation flag of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define DEACT_DCDCAPD 0x00000400
/* No-Operation
#define DEACT_DCDCAPD_NOP 0x00000000 */
/** Clear */
#define DEACT_DCDCAPD_CLR 0x00000400
/** Deactivate DCDCDDR
    Clears the activation flag of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define DEACT_DCDCDDR 0x00000200
/* No-Operation
#define DEACT_DCDCDDR_NOP 0x00000000 */
/** Clear */
#define DEACT_DCDCDDR_CLR 0x00000200
/** Deactivate DCDC1V0
    Clears the activation flag of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define DEACT_DCDC1V0 0x00000100
/* No-Operation
#define DEACT_DCDC1V0_NOP 0x00000000 */
/** Clear */
#define DEACT_DCDC1V0_CLR 0x00000100
/** Deactivate TRC2MEM
    Clears the activation flag of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define DEACT_TRC2MEM 0x00000040
/* No-Operation
#define DEACT_TRC2MEM_NOP 0x00000000 */
/** Clear */
#define DEACT_TRC2MEM_CLR 0x00000040
/** Deactivate DDR
    Clears the activation flag of the DDR domain. This domain contains the DDR interface block. */
#define DEACT_DDR 0x00000020
/* No-Operation
#define DEACT_DDR_NOP 0x00000000 */
/** Clear */
#define DEACT_DDR_CLR 0x00000020
/** Deactivate EBU
    Clears the activation flag of the EBU domain. This domain contains the EBU interface block. */
#define DEACT_EBU 0x00000010
/* No-Operation
#define DEACT_EBU_NOP 0x00000000 */
/** Clear */
#define DEACT_EBU_CLR 0x00000010

/* Fields of "Reboot Trigger Register" */
/** Reboot STATUS
    Triggers a reboot of the STATUS domain. This domain contains the STATUS block. */
#define RBT_STATUS 0x80000000
/* No-Operation
#define RBT_STATUS_NOP 0x00000000 */
/** Trigger */
#define RBT_STATUS_TRIG 0x80000000
/** Reboot SHA1
    Triggers a reboot of the SHA1 domain. This domain contains the SHA1 block. */
#define RBT_SHA1 0x40000000
/* No-Operation
#define RBT_SHA1_NOP 0x00000000 */
/** Trigger */
#define RBT_SHA1_TRIG 0x40000000
/** Reboot AES
    Triggers a reboot of the AES domain. This domain contains the AES block. */
#define RBT_AES 0x20000000
/* No-Operation
#define RBT_AES_NOP 0x00000000 */
/** Trigger */
#define RBT_AES_TRIG 0x20000000
/** Reboot PCM
    Triggers a reboot of the PCM domain. This domain contains the PCM interface block. */
#define RBT_PCM 0x10000000
/* No-Operation
#define RBT_PCM_NOP 0x00000000 */
/** Trigger */
#define RBT_PCM_TRIG 0x10000000
/** Reboot FSCT
    Triggers a reboot of the FSCT domain. This domain contains the FSCT block. */
#define RBT_FSCT 0x08000000
/* No-Operation
#define RBT_FSCT_NOP 0x00000000 */
/** Trigger */
#define RBT_FSCT_TRIG 0x08000000
/** Reboot GPTC
    Triggers a reboot of the GPTC domain. This domain contains the GPTC block. */
#define RBT_GPTC 0x04000000
/* No-Operation
#define RBT_GPTC_NOP 0x00000000 */
/** Trigger */
#define RBT_GPTC_TRIG 0x04000000
/** Reboot MPS
    Triggers a reboot of the MPS domain. This domain contains the MPS block. */
#define RBT_MPS 0x02000000
/* No-Operation
#define RBT_MPS_NOP 0x00000000 */
/** Trigger */
#define RBT_MPS_TRIG 0x02000000
/** Reboot DFEV0
    Triggers a reboot of the DFEV0 domain. This domain contains the DFEV0 block. */
#define RBT_DFEV0 0x01000000
/* No-Operation
#define RBT_DFEV0_NOP 0x00000000 */
/** Trigger */
#define RBT_DFEV0_TRIG 0x01000000
/** Reboot PADCTRL4
    Triggers a reboot of the PADCTRL4 domain. This domain contains the PADCTRL4 block. */
#define RBT_PADCTRL4 0x00400000
/* No-Operation
#define RBT_PADCTRL4_NOP 0x00000000 */
/** Trigger */
#define RBT_PADCTRL4_TRIG 0x00400000
/** Reboot PADCTRL3
    Triggers a reboot of the PADCTRL3 domain. This domain contains the PADCTRL3 block. */
#define RBT_PADCTRL3 0x00200000
/* No-Operation
#define RBT_PADCTRL3_NOP 0x00000000 */
/** Trigger */
#define RBT_PADCTRL3_TRIG 0x00200000
/** Reboot PADCTRL1
    Triggers a reboot of the PADCTRL1 domain. This domain contains the PADCTRL1 block. */
#define RBT_PADCTRL1 0x00100000
/* No-Operation
#define RBT_PADCTRL1_NOP 0x00000000 */
/** Trigger */
#define RBT_PADCTRL1_TRIG 0x00100000
/** Reboot P4
    Triggers a reboot of the P4 domain. This domain contains the P4 instance of the GPIO block. */
#define RBT_P4 0x00040000
/* No-Operation
#define RBT_P4_NOP 0x00000000 */
/** Trigger */
#define RBT_P4_TRIG 0x00040000
/** Reboot P3
    Triggers a reboot of the P3 domain. This domain contains the P3 instance of the GPIO block. */
#define RBT_P3 0x00020000
/* No-Operation
#define RBT_P3_NOP 0x00000000 */
/** Trigger */
#define RBT_P3_TRIG 0x00020000
/** Reboot P1
    Triggers a reboot of the P1 domain. This domain contains the P1 instance of the GPIO block. */
#define RBT_P1 0x00010000
/* No-Operation
#define RBT_P1_NOP 0x00000000 */
/** Trigger */
#define RBT_P1_TRIG 0x00010000
/** Reboot HOST
    Triggers a reboot of the HOST domain. This domain contains the HOST interface block. */
#define RBT_HOST 0x00008000
/* No-Operation
#define RBT_HOST_NOP 0x00000000 */
/** Trigger */
#define RBT_HOST_TRIG 0x00008000
/** Reboot I2C
    Triggers a reboot of the I2C domain. This domain contains the I2C interface block. */
#define RBT_I2C 0x00004000
/* No-Operation
#define RBT_I2C_NOP 0x00000000 */
/** Trigger */
#define RBT_I2C_TRIG 0x00004000
/** Reboot SSC0
    Triggers a reboot of the SSC0 domain. This domain contains the SSC0 interface block. */
#define RBT_SSC0 0x00002000
/* No-Operation
#define RBT_SSC0_NOP 0x00000000 */
/** Trigger */
#define RBT_SSC0_TRIG 0x00002000
/** Reboot ASC0
    Triggers a reboot of the ASC0 domain. This domain contains the ASC0 interface block. */
#define RBT_ASC0 0x00001000
/* No-Operation
#define RBT_ASC0_NOP 0x00000000 */
/** Trigger */
#define RBT_ASC0_TRIG 0x00001000
/** Reboot ASC1
    Triggers a reboot of the ASC1 domain. This domain contains the ASC1 block. */
#define RBT_ASC1 0x00000800
/* No-Operation
#define RBT_ASC1_NOP 0x00000000 */
/** Trigger */
#define RBT_ASC1_TRIG 0x00000800
/** Reboot DCDCAPD
    Triggers a reboot of the DCDCAPD domain. This domain contains the digital part of the 60 volts DCDC converter. */
#define RBT_DCDCAPD 0x00000400
/* No-Operation
#define RBT_DCDCAPD_NOP 0x00000000 */
/** Trigger */
#define RBT_DCDCAPD_TRIG 0x00000400
/** Reboot DCDCDDR
    Triggers a reboot of the DCDCDDR domain. This domain contains the digital part of the DCDC converter dedicated to the DDR interface. */
#define RBT_DCDCDDR 0x00000200
/* No-Operation
#define RBT_DCDCDDR_NOP 0x00000000 */
/** Trigger */
#define RBT_DCDCDDR_TRIG 0x00000200
/** Reboot DCDC1V0
    Triggers a reboot of the DCDC1V0 domain. This domain contains the digital part of the 1.0 volts DCDC converter. */
#define RBT_DCDC1V0 0x00000100
/* No-Operation
#define RBT_DCDC1V0_NOP 0x00000000 */
/** Trigger */
#define RBT_DCDC1V0_TRIG 0x00000100
/** Reboot TRC2MEM
    Triggers a reboot of the TRC2MEM domain. This domain contains the TRC2MEM block. */
#define RBT_TRC2MEM 0x00000040
/* No-Operation
#define RBT_TRC2MEM_NOP 0x00000000 */
/** Trigger */
#define RBT_TRC2MEM_TRIG 0x00000040
/** Reboot DDR
    Triggers a reboot of the DDR domain. This domain contains the DDR interface block. */
#define RBT_DDR 0x00000020
/* No-Operation
#define RBT_DDR_NOP 0x00000000 */
/** Trigger */
#define RBT_DDR_TRIG 0x00000020
/** Reboot EBU
    Triggers a reboot of the EBU domain. This domain contains the EBU interface block. */
#define RBT_EBU 0x00000010
/* No-Operation
#define RBT_EBU_NOP 0x00000000 */
/** Trigger */
#define RBT_EBU_TRIG 0x00000010
/** Reboot XBAR
    Triggers a reboot of the XBAR. */
#define RBT_XBAR 0x00000002
/* No-Operation
#define RBT_XBAR_NOP 0x00000000 */
/** Trigger */
#define RBT_XBAR_TRIG 0x00000002
/** Reboot CPU
    Triggers a reboot of the CPU. */
#define RBT_CPU 0x00000001
/* No-Operation
#define RBT_CPU_NOP 0x00000000 */
/** Trigger */
#define RBT_CPU_TRIG 0x00000001

/* Fields of "CPU0 Clock Control Register" */
/** CPU Clock Divider
    Via this bit the divider and therefore the frequency of the clock of CPU0 can be selected. */
#define CPU0CC_CPUDIV 0x00000001
/* Frequency set to the nominal value.
#define CPU0CC_CPUDIV_SELFNOM 0x00000000 */
/** Frequency set to half of its nominal value. */
#define CPU0CC_CPUDIV_SELFHALF 0x00000001

/* Fields of "CPU0 Reset Source Register" */
/** Software Reboot Request Occurred
    This bit can be acknowledged by a write operation. */
#define CPU0RS_SWRRO 0x00000004
/* Nothing
#define CPU0RS_SWRRO_NULL 0x00000000 */
/** Write: Acknowledge the event. */
#define CPU0RS_SWRRO_EVACK 0x00000004
/** Read: Event occurred. */
#define CPU0RS_SWRRO_EVOCC 0x00000004
/** Hardware Reset Source
    Reflects the root cause for the last hardware reset. The infrastructure-block is only reset in case of POR. For all other blocks there is no difference between the three HW-reset sources. */
#define CPU0RS_HWRS_MASK 0x00000003
/** field offset */
#define CPU0RS_HWRS_OFFSET 0
/** Power-on reset. */
#define CPU0RS_HWRS_POR 0x00000000
/** RST pin. */
#define CPU0RS_HWRS_RST 0x00000001
/** Watchdog reset request. */
#define CPU0RS_HWRS_WDT 0x00000002

/* Fields of "CPU0 Wakeup Configuration Register" */
/** Wakeup Request Source Yield Resume 15
    Select the signal connected to the yield/resume interface pin 15 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR15 0x80000000
/* Not selected
#define CPU0WCFG_WRSYR15_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR15_SEL 0x80000000
/** Wakeup Request Source Yield Resume 14
    Select the signal connected to the yield/resume interface pin 14 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR14 0x40000000
/* Not selected
#define CPU0WCFG_WRSYR14_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR14_SEL 0x40000000
/** Wakeup Request Source Yield Resume 13
    Select the signal connected to the yield/resume interface pin 13 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR13 0x20000000
/* Not selected
#define CPU0WCFG_WRSYR13_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR13_SEL 0x20000000
/** Wakeup Request Source Yield Resume 12
    Select the signal connected to the yield/resume interface pin 12 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR12 0x10000000
/* Not selected
#define CPU0WCFG_WRSYR12_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR12_SEL 0x10000000
/** Wakeup Request Source Yield Resume 11
    Select the signal connected to the yield/resume interface pin 11 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR11 0x08000000
/* Not selected
#define CPU0WCFG_WRSYR11_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR11_SEL 0x08000000
/** Wakeup Request Source Yield Resume 10
    Select the signal connected to the yield/resume interface pin 10 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR10 0x04000000
/* Not selected
#define CPU0WCFG_WRSYR10_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR10_SEL 0x04000000
/** Wakeup Request Source Yield Resume 9
    Select the signal connected to the yield/resume interface pin 9 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR9 0x02000000
/* Not selected
#define CPU0WCFG_WRSYR9_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR9_SEL 0x02000000
/** Wakeup Request Source Yield Resume 8
    Select the signal connected to the yield/resume interface pin 8 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR8 0x01000000
/* Not selected
#define CPU0WCFG_WRSYR8_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR8_SEL 0x01000000
/** Wakeup Request Source Yield Resume 7
    Select the signal connected to the yield/resume interface pin 7 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR7 0x00800000
/* Not selected
#define CPU0WCFG_WRSYR7_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR7_SEL 0x00800000
/** Wakeup Request Source Yield Resume 6
    Select the signal connected to the yield/resume interface pin 6 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR6 0x00400000
/* Not selected
#define CPU0WCFG_WRSYR6_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR6_SEL 0x00400000
/** Wakeup Request Source Yield Resume 5
    Select the signal connected to the yield/resume interface pin 5 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR5 0x00200000
/* Not selected
#define CPU0WCFG_WRSYR5_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR5_SEL 0x00200000
/** Wakeup Request Source Yield Resume 4
    Select the signal connected to the yield/resume interface pin 4 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR4 0x00100000
/* Not selected
#define CPU0WCFG_WRSYR4_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR4_SEL 0x00100000
/** Wakeup Request Source Yield Resume 3
    Select the signal connected to the yield/resume interface pin 3 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR3 0x00080000
/* Not selected
#define CPU0WCFG_WRSYR3_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR3_SEL 0x00080000
/** Wakeup Request Source Yield Resume 2
    Select the signal connected to the yield/resume interface pin 2 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR2 0x00040000
/* Not selected
#define CPU0WCFG_WRSYR2_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR2_SEL 0x00040000
/** Wakeup Request Source Yield Resume 1
    Select the signal connected to the yield/resume interface pin 1 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR1 0x00020000
/* Not selected
#define CPU0WCFG_WRSYR1_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR1_SEL 0x00020000
/** Wakeup Request Source Yield Resume 0
    Select the signal connected to the yield/resume interface pin 0 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSYR0 0x00010000
/* Not selected
#define CPU0WCFG_WRSYR0_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSYR0_SEL 0x00010000
/** Wakeup Request Source Debug
    Select signal EJ_DINT as source for wakeup from sleep state. */
#define CPU0WCFG_WRSDBG 0x00000100
/* Not selected
#define CPU0WCFG_WRSDBG_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSDBG_SEL 0x00000100
/** Wakeup Request Source ICU of VPE1
    Select signal ICU_IRQ of VPE1 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSICUVPE1 0x00000002
/* Not selected
#define CPU0WCFG_WRSICUVPE1_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSICUVPE1_SEL 0x00000002
/** Wakeup Request Source ICU of VPE0
    Select signal ICU_IRQ of VPE0 as source for wakeup from sleep state. */
#define CPU0WCFG_WRSICUVPE0 0x00000001
/* Not selected
#define CPU0WCFG_WRSICUVPE0_NSEL 0x00000000 */
/** Selected */
#define CPU0WCFG_WRSICUVPE0_SEL 0x00000001

/* Fields of "Bootmode Control Register" */
/** Software Bootmode Select
    Enables SW writing of Bootmode and shows whether or not the SW-programmed bootmode is reflected in field Bootmode instead of the hardware given value. */
#define BMC_BMSW 0x80000000
/* Disable
#define BMC_BMSW_DIS 0x00000000 */
/** Enable */
#define BMC_BMSW_EN 0x80000000
/** Bootmode
    Initially this field holds the value of the pinstraps LED_BMODEx on positions 5:0, and the value of the corresponding JTAG register bit on position 6. Writing is enabled by setting Software Bootmode Select to 1 during the write cycle. */
#define BMC_BM_MASK 0x0000007F
/** field offset */
#define BMC_BM_OFFSET 0

/* Fields of "Sleep Configuration Register" */
/** Enable XBAR Clockoff When All XBAR masters Clockoff
    Enable XBAR clock shutdown in case all XBAR masters are in clockoff mode. This bit has no effect if bit CPU0 is not enabled. */
#define SCFG_XBAR 0x00010000
/* Disable
#define SCFG_XBAR_DIS 0x00000000 */
/** Enable */
#define SCFG_XBAR_EN 0x00010000
/** CPU0 Clockoff On Sleep
    Enable CPU0 clock shutdown in case its SI_SLEEP signal becomes active. */
#define SCFG_CPU0 0x00000001
/* Disable
#define SCFG_CPU0_DIS 0x00000000 */
/** Enable */
#define SCFG_CPU0_EN 0x00000001

/* Fields of "Power Down Configuration Register" */
/** Enable Power Down STATUS
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_STATUS 0x80000000
/* Disable
#define PDCFG_STATUS_DIS 0x00000000 */
/** Enable */
#define PDCFG_STATUS_EN 0x80000000
/** Enable Power Down SHA1
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_SHA1 0x40000000
/* Disable
#define PDCFG_SHA1_DIS 0x00000000 */
/** Enable */
#define PDCFG_SHA1_EN 0x40000000
/** Enable Power Down AES
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_AES 0x20000000
/* Disable
#define PDCFG_AES_DIS 0x00000000 */
/** Enable */
#define PDCFG_AES_EN 0x20000000
/** Enable Power Down PCM
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_PCM 0x10000000
/* Disable
#define PDCFG_PCM_DIS 0x00000000 */
/** Enable */
#define PDCFG_PCM_EN 0x10000000
/** Enable Power Down FSCT
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_FSCT 0x08000000
/* Disable
#define PDCFG_FSCT_DIS 0x00000000 */
/** Enable */
#define PDCFG_FSCT_EN 0x08000000
/** Enable Power Down GPTC
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_GPTC 0x04000000
/* Disable
#define PDCFG_GPTC_DIS 0x00000000 */
/** Enable */
#define PDCFG_GPTC_EN 0x04000000
/** Enable Power Down MPS
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_MPS 0x02000000
/* Disable
#define PDCFG_MPS_DIS 0x00000000 */
/** Enable */
#define PDCFG_MPS_EN 0x02000000
/** Enable Power Down DFEV0
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_DFEV0 0x01000000
/* Disable
#define PDCFG_DFEV0_DIS 0x00000000 */
/** Enable */
#define PDCFG_DFEV0_EN 0x01000000
/** Enable Power Down PADCTRL4
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_PADCTRL4 0x00400000
/* Disable
#define PDCFG_PADCTRL4_DIS 0x00000000 */
/** Enable */
#define PDCFG_PADCTRL4_EN 0x00400000
/** Enable Power Down PADCTRL3
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_PADCTRL3 0x00200000
/* Disable
#define PDCFG_PADCTRL3_DIS 0x00000000 */
/** Enable */
#define PDCFG_PADCTRL3_EN 0x00200000
/** Enable Power Down PADCTRL1
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_PADCTRL1 0x00100000
/* Disable
#define PDCFG_PADCTRL1_DIS 0x00000000 */
/** Enable */
#define PDCFG_PADCTRL1_EN 0x00100000
/** Enable Power Down P4
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_P4 0x00040000
/* Disable
#define PDCFG_P4_DIS 0x00000000 */
/** Enable */
#define PDCFG_P4_EN 0x00040000
/** Enable Power Down P3
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_P3 0x00020000
/* Disable
#define PDCFG_P3_DIS 0x00000000 */
/** Enable */
#define PDCFG_P3_EN 0x00020000
/** Enable Power Down P1
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_P1 0x00010000
/* Disable
#define PDCFG_P1_DIS 0x00000000 */
/** Enable */
#define PDCFG_P1_EN 0x00010000
/** Enable Power Down HOST
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_HOST 0x00008000
/* Disable
#define PDCFG_HOST_DIS 0x00000000 */
/** Enable */
#define PDCFG_HOST_EN 0x00008000
/** Enable Power Down I2C
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_I2C 0x00004000
/* Disable
#define PDCFG_I2C_DIS 0x00000000 */
/** Enable */
#define PDCFG_I2C_EN 0x00004000
/** Enable Power Down SSC0
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_SSC0 0x00002000
/* Disable
#define PDCFG_SSC0_DIS 0x00000000 */
/** Enable */
#define PDCFG_SSC0_EN 0x00002000
/** Enable Power Down ASC0
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_ASC0 0x00001000
/* Disable
#define PDCFG_ASC0_DIS 0x00000000 */
/** Enable */
#define PDCFG_ASC0_EN 0x00001000
/** Enable Power Down ASC1
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_ASC1 0x00000800
/* Disable
#define PDCFG_ASC1_DIS 0x00000000 */
/** Enable */
#define PDCFG_ASC1_EN 0x00000800
/** Enable Power Down DCDCAPD
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_DCDCAPD 0x00000400
/* Disable
#define PDCFG_DCDCAPD_DIS 0x00000000 */
/** Enable */
#define PDCFG_DCDCAPD_EN 0x00000400
/** Enable Power Down DCDCDDR
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_DCDCDDR 0x00000200
/* Disable
#define PDCFG_DCDCDDR_DIS 0x00000000 */
/** Enable */
#define PDCFG_DCDCDDR_EN 0x00000200
/** Enable Power Down DCDC1V0
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_DCDC1V0 0x00000100
/* Disable
#define PDCFG_DCDC1V0_DIS 0x00000000 */
/** Enable */
#define PDCFG_DCDC1V0_EN 0x00000100
/** Enable Power Down TRC2MEM
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_TRC2MEM 0x00000040
/* Disable
#define PDCFG_TRC2MEM_DIS 0x00000000 */
/** Enable */
#define PDCFG_TRC2MEM_EN 0x00000040
/** Enable Power Down DDR
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_DDR 0x00000020
/* Disable
#define PDCFG_DDR_DIS 0x00000000 */
/** Enable */
#define PDCFG_DDR_EN 0x00000020
/** Enable Power Down EBU
    Ignore this bit as power-gating is not supported for this chip. */
#define PDCFG_EBU 0x00000010
/* Disable
#define PDCFG_EBU_DIS 0x00000000 */
/** Enable */
#define PDCFG_EBU_EN 0x00000010

/* Fields of "CLKO Pad Control Register" */
/** Ethernet Reference Clock CLKO Select
    Selects the CLKO pad's input as source for the GPHY, SGMII PLLs. */
#define CLKOC_ETHREF 0x00000002
/* Not selected
#define CLKOC_ETHREF_NSEL 0x00000000 */
/** Selected */
#define CLKOC_ETHREF_SEL 0x00000002
/** Output Enable
    Enables the output driver of the CLKO pad. */
#define CLKOC_OEN 0x00000001
/* Disable
#define CLKOC_OEN_DIS 0x00000000 */
/** Enable */
#define CLKOC_OEN_EN 0x00000001

/* Fields of "Infrastructure Control Register" */
/** General Purpose Control
    Backup bits. Currently they are connected as: bit 0 : connected to the configmode_on pin of the pinstrapping block. bit 1 : clock enable of the GPE primary clock. bits 3:2 : frequency select of the GPE primary clock. 00 = 769.2MHz, 01 = 625MHz, 10 = 555.6MHz, 11 = 500MHz All other bits are unconnected. */
#define INFRAC_GP_MASK 0x1F000000
/** field offset */
#define INFRAC_GP_OFFSET 24
/** CMOS2CML Ethernet Control
    CMOS2CML Ethernet Control. */
#define INFRAC_CMOS2CML_GPON_MASK 0x0000F000
/** field offset */
#define INFRAC_CMOS2CML_GPON_OFFSET 12
/** CMOS2CML Ethernet Control
    CMOS2CML Ethernet Control. */
#define INFRAC_CMOS2CML_ETH_MASK 0x00000F00
/** field offset */
#define INFRAC_CMOS2CML_ETH_OFFSET 8
/** Dying Gasp Enable
    Enables the dying gasp detector. */
#define INFRAC_DGASPEN 0x00000040
/* Disable
#define INFRAC_DGASPEN_DIS 0x00000000 */
/** Enable */
#define INFRAC_DGASPEN_EN 0x00000040
/** Dying Gasp Hysteresis Control
    Dying Gasp Hysteresis Control. */
#define INFRAC_DGASPHYS_MASK 0x00000030
/** field offset */
#define INFRAC_DGASPHYS_OFFSET 4
/** Linear Regulator 1.5V Enable
    Enables 1.5V linear regulator. */
#define INFRAC_LIN1V5EN 0x00000008
/* Disable
#define INFRAC_LIN1V5EN_DIS 0x00000000 */
/** Enable */
#define INFRAC_LIN1V5EN_EN 0x00000008
/** Linear Regulator 1.5V Control
    Linear regulator 1.5V control. */
#define INFRAC_LIN1V5C_MASK 0x00000007
/** field offset */
#define INFRAC_LIN1V5C_OFFSET 0

/* Fields of "HRST_OUT_N Control Register" */
/** HRST_OUT_N Pin Value
    Controls the value of the HRST_OUT_N pin. */
#define HRSTOUTC_VALUE 0x00000001

/* Fields of "EBU Clock Control Register" */
/** EBU Clock Divider
    Via this bit the frequency of the clock of the EBU can be selected. */
#define EBUCC_EBUDIV 0x00000001
/* Frequency set to 50MHz.
#define EBUCC_EBUDIV_SELF50 0x00000000 */
/** Frequency set to 100MHz. */
#define EBUCC_EBUDIV_SELF100 0x00000001

/* Fields of "NMI Status Register" */
/** NMI Status Flag TEST
    Shows whether the event NMI TEST occurred. */
#define NMIS_TEST 0x00000100
/* Nothing
#define NMIS_TEST_NULL 0x00000000 */
/** Read: Event occurred. */
#define NMIS_TEST_EVOCC 0x00000100
/** NMI Status Flag DGASP
    Shows whether the event NMI DGASP occurred. */
#define NMIS_DGASP 0x00000004
/* Nothing
#define NMIS_DGASP_NULL 0x00000000 */
/** Read: Event occurred. */
#define NMIS_DGASP_EVOCC 0x00000004
/** NMI Status Flag HOST
    Shows whether the event NMI HOST occurred. */
#define NMIS_HOST 0x00000002
/* Nothing
#define NMIS_HOST_NULL 0x00000000 */
/** Read: Event occurred. */
#define NMIS_HOST_EVOCC 0x00000002
/** NMI Status Flag PIN
    Shows whether the event NMI PIN occurred. */
#define NMIS_PIN 0x00000001
/* Nothing
#define NMIS_PIN_NULL 0x00000000 */
/** Read: Event occurred. */
#define NMIS_PIN_EVOCC 0x00000001

/* Fields of "NMI Set Register" */
/** Set NMI Status Flag TEST
    Sets the corresponding NMI status flag. */
#define NMISET_TEST 0x00000100
/* Nothing
#define NMISET_TEST_NULL 0x00000000 */
/** Set */
#define NMISET_TEST_SET 0x00000100
/** Set NMI Status Flag DGASP
    Sets the corresponding NMI status flag. */
#define NMISET_DGASP 0x00000004
/* Nothing
#define NMISET_DGASP_NULL 0x00000000 */
/** Set */
#define NMISET_DGASP_SET 0x00000004
/** Set NMI Status Flag HOST
    Sets the corresponding NMI status flag. */
#define NMISET_HOST 0x00000002
/* Nothing
#define NMISET_HOST_NULL 0x00000000 */
/** Set */
#define NMISET_HOST_SET 0x00000002
/** Set NMI Status Flag PIN
    Sets the corresponding NMI status flag. */
#define NMISET_PIN 0x00000001
/* Nothing
#define NMISET_PIN_NULL 0x00000000 */
/** Set */
#define NMISET_PIN_SET 0x00000001

/* Fields of "NMI Clear Register" */
/** Clear NMI Status Flag TEST
    Clears the corresponding NMI status flag. */
#define NMICLR_TEST 0x00000100
/* Nothing
#define NMICLR_TEST_NULL 0x00000000 */
/** Clear */
#define NMICLR_TEST_CLR 0x00000100
/** Clear NMI Status Flag DGASP
    Clears the corresponding NMI status flag. */
#define NMICLR_DGASP 0x00000004
/* Nothing
#define NMICLR_DGASP_NULL 0x00000000 */
/** Clear */
#define NMICLR_DGASP_CLR 0x00000004
/** Clear NMI Status Flag HOST
    Clears the corresponding NMI status flag. */
#define NMICLR_HOST 0x00000002
/* Nothing
#define NMICLR_HOST_NULL 0x00000000 */
/** Clear */
#define NMICLR_HOST_CLR 0x00000002
/** Clear NMI Status Flag PIN
    Clears the corresponding NMI status flag. */
#define NMICLR_PIN 0x00000001
/* Nothing
#define NMICLR_PIN_NULL 0x00000000 */
/** Clear */
#define NMICLR_PIN_CLR 0x00000001

/* Fields of "NMI Test Configuration Register" */
/** Enable NMI Test Feature
    Enables the operation of the NMI TEST flag. This is the mask for the Non-Maskable-Interrupt dedicated to SW tests. All others cannot be masked. */
#define NMITCFG_TEN 0x00000100
/* Disable
#define NMITCFG_TEN_DIS 0x00000000 */
/** Enable */
#define NMITCFG_TEN_EN 0x00000100

/* Fields of "NMI VPE1 Control Register" */
/** NMI VPE1 State
    Reflects the state of the NMI signal towards VPE1. This bit is controlled by software only, there is no hardware NMI source dedicated to VPE1. So VPE0 could trigger an NMI at VPE1 using this bit in its own NMI-routine. */
#define NMIVPE1C_NMI 0x00000001
/* False
#define NMIVPE1C_NMI_FALSE 0x00000000 */
/** True */
#define NMIVPE1C_NMI_TRUE 0x00000001

/* Fields of "IRN Capture Register" */
/** DCDCAPD Alarm
    The DCDC Converter for the APD Supply submitted an alarm. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define IRNCR_DCDCAPD 0x00400000
/* Nothing
#define IRNCR_DCDCAPD_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define IRNCR_DCDCAPD_INTACK 0x00400000
/** Read: Interrupt occurred. */
#define IRNCR_DCDCAPD_INTOCC 0x00400000
/** DCDCDDR Alarm
    The DCDC Converter for the DDR Supply submitted an alarm. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define IRNCR_DCDCDDR 0x00200000
/* Nothing
#define IRNCR_DCDCDDR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define IRNCR_DCDCDDR_INTACK 0x00200000
/** Read: Interrupt occurred. */
#define IRNCR_DCDCDDR_INTOCC 0x00200000
/** DCDC1V0 Alarm
    The DCDC Converter for the 1.0 Volts submitted an alarm. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define IRNCR_DCDC1V0 0x00100000
/* Nothing
#define IRNCR_DCDC1V0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define IRNCR_DCDC1V0_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define IRNCR_DCDC1V0_INTOCC 0x00100000
/** SIF0 wakeup request
    SmartSlic Interface 0 submitted a wakeup request. This bit is edge-sensitive. This bit contributes to the indirect interrupt. */
#define IRNCR_SIF0 0x00010000
/* Nothing
#define IRNCR_SIF0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define IRNCR_SIF0_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define IRNCR_SIF0_INTOCC 0x00010000

/* Fields of "IRN Interrupt Control Register" */
/** DCDCAPD Alarm
    Interrupt control bit for the corresponding bit in the IRNCR register. */
#define IRNICR_DCDCAPD 0x00400000
/** DCDCDDR Alarm
    Interrupt control bit for the corresponding bit in the IRNCR register. */
#define IRNICR_DCDCDDR 0x00200000
/** DCDC1V0 Alarm
    Interrupt control bit for the corresponding bit in the IRNCR register. */
#define IRNICR_DCDC1V0 0x00100000
/** SIF0 wakeup request
    Interrupt control bit for the corresponding bit in the IRNCR register. */
#define IRNICR_SIF0 0x00010000

/* Fields of "IRN Interrupt Enable Register" */
/** DCDCAPD Alarm
    Interrupt enable bit for the corresponding bit in the IRNCR register. */
#define IRNEN_DCDCAPD 0x00400000
/* Disable
#define IRNEN_DCDCAPD_DIS 0x00000000 */
/** Enable */
#define IRNEN_DCDCAPD_EN 0x00400000
/** DCDCDDR Alarm
    Interrupt enable bit for the corresponding bit in the IRNCR register. */
#define IRNEN_DCDCDDR 0x00200000
/* Disable
#define IRNEN_DCDCDDR_DIS 0x00000000 */
/** Enable */
#define IRNEN_DCDCDDR_EN 0x00200000
/** DCDC1V0 Alarm
    Interrupt enable bit for the corresponding bit in the IRNCR register. */
#define IRNEN_DCDC1V0 0x00100000
/* Disable
#define IRNEN_DCDC1V0_DIS 0x00000000 */
/** Enable */
#define IRNEN_DCDC1V0_EN 0x00100000
/** SIF0 wakeup request
    Interrupt enable bit for the corresponding bit in the IRNCR register. */
#define IRNEN_SIF0 0x00010000
/* Disable
#define IRNEN_SIF0_DIS 0x00000000 */
/** Enable */
#define IRNEN_SIF0_EN 0x00010000

/*! @} */ /* SYS1_REGISTER */

#endif /* _sys1_reg_h */
