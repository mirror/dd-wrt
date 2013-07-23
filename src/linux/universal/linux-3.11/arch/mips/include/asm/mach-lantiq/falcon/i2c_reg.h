/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _i2c_reg_h
#define _i2c_reg_h

/** \addtogroup I2C_REGISTER
   @{
*/
/* access macros */
#define i2c_r32(reg) reg_r32(&i2c->reg)
#define i2c_w32(val, reg) reg_w32(val, &i2c->reg)
#define i2c_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &i2c->reg)
#define i2c_r32_table(reg, idx) reg_r32_table(i2c->reg, idx)
#define i2c_w32_table(val, reg, idx) reg_w32_table(val, i2c->reg, idx)
#define i2c_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, i2c->reg, idx)
#define i2c_adr_table(reg, idx) adr_table(i2c->reg, idx)


/** I2C register structure */
struct gpon_reg_i2c
{
   /** I2C Kernel Clock Control Register */
   unsigned int clc; /* 0x00000000 */
   /** Reserved */
   unsigned int res_0; /* 0x00000004 */
   /** I2C Identification Register */
   unsigned int id; /* 0x00000008 */
   /** Reserved */
   unsigned int res_1; /* 0x0000000C */
   /** I2C RUN Control Register
       This register enables and disables the I2C peripheral. Before enabling, the I2C has to be configured properly. After enabling no configuration is possible */
   unsigned int run_ctrl; /* 0x00000010 */
   /** I2C End Data Control Register
       This register is used to either turn around the data transmission direction or to address another slave without sending a stop condition. Also the software can stop the slave-transmitter by sending a not-accolade when working as master-receiver or even stop data transmission immediately when operating as master-transmitter. The writing to the bits of this control register is only effective when in MASTER RECEIVES BYTES, MASTER TRANSMITS BYTES, MASTER RESTART or SLAVE RECEIVE BYTES state */
   unsigned int endd_ctrl; /* 0x00000014 */
   /** I2C Fractional Divider Configuration Register
       These register is used to program the fractional divider of the I2C bus. Before the peripheral is switched on by setting the RUN-bit the two (fixed) values for the two operating frequencies are programmed into these (configuration) registers. The Register FDIV_HIGH_CFG has the same layout as I2C_FDIV_CFG. */
   unsigned int fdiv_cfg; /* 0x00000018 */
   /** I2C Fractional Divider (highspeed mode) Configuration Register
       These register is used to program the fractional divider of the I2C bus. Before the peripheral is switched on by setting the RUN-bit the two (fixed) values for the two operating frequencies are programmed into these (configuration) registers. The Register FDIV_CFG has the same layout as I2C_FDIV_CFG. */
   unsigned int fdiv_high_cfg; /* 0x0000001C */
   /** I2C Address Configuration Register */
   unsigned int addr_cfg; /* 0x00000020 */
   /** I2C Bus Status Register
       This register gives a status information of the I2C. This additional information can be used by the software to start proper actions. */
   unsigned int bus_stat; /* 0x00000024 */
   /** I2C FIFO Configuration Register */
   unsigned int fifo_cfg; /* 0x00000028 */
   /** I2C Maximum Received Packet Size Register */
   unsigned int mrps_ctrl; /* 0x0000002C */
   /** I2C Received Packet Size Status Register */
   unsigned int rps_stat; /* 0x00000030 */
   /** I2C Transmit Packet Size Register */
   unsigned int tps_ctrl; /* 0x00000034 */
   /** I2C Filled FIFO Stages Status Register */
   unsigned int ffs_stat; /* 0x00000038 */
   /** Reserved */
   unsigned int res_2; /* 0x0000003C */
   /** I2C Timing Configuration Register */
   unsigned int tim_cfg; /* 0x00000040 */
   /** Reserved */
   unsigned int res_3[7]; /* 0x00000044 */
   /** I2C Error Interrupt Request Source Mask Register */
   unsigned int err_irqsm; /* 0x00000060 */
   /** I2C Error Interrupt Request Source Status Register */
   unsigned int err_irqss; /* 0x00000064 */
   /** I2C Error Interrupt Request Source Clear Register */
   unsigned int err_irqsc; /* 0x00000068 */
   /** Reserved */
   unsigned int res_4; /* 0x0000006C */
   /** I2C Protocol Interrupt Request Source Mask Register */
   unsigned int p_irqsm; /* 0x00000070 */
   /** I2C Protocol Interrupt Request Source Status Register */
   unsigned int p_irqss; /* 0x00000074 */
   /** I2C Protocol Interrupt Request Source Clear Register */
   unsigned int p_irqsc; /* 0x00000078 */
   /** Reserved */
   unsigned int res_5; /* 0x0000007C */
   /** I2C Raw Interrupt Status Register */
   unsigned int ris; /* 0x00000080 */
   /** I2C Interrupt Mask Control Register */
   unsigned int imsc; /* 0x00000084 */
   /** I2C Masked Interrupt Status Register */
   unsigned int mis; /* 0x00000088 */
   /** I2C Interrupt Clear Register */
   unsigned int icr; /* 0x0000008C */
   /** I2C Interrupt Set Register */
   unsigned int isr; /* 0x00000090 */
   /** I2C DMA Enable Register */
   unsigned int dmae; /* 0x00000094 */
   /** Reserved */
   unsigned int res_6[8154]; /* 0x00000098 */
   /** I2C Transmit Data Register */
   unsigned int txd; /* 0x00008000 */
   /** Reserved */
   unsigned int res_7[4095]; /* 0x00008004 */
   /** I2C Receive Data Register */
   unsigned int rxd; /* 0x0000C000 */
   /** Reserved */
   unsigned int res_8[4095]; /* 0x0000C004 */
};


/* Fields of "I2C Kernel Clock Control Register" */
/** Clock Divider for Optional Run Mode (AHB peripherals)
    Max 8-bit divider value. Note: As long as the new divider value ORMC is not valid, the register returns 0x0000 00xx on reading. */
#define I2C_CLC_ORMC_MASK 0x00FF0000
/** field offset */
#define I2C_CLC_ORMC_OFFSET 16
/** Clock Divider for Normal Run Mode
    Max 8-bit divider value. IF RMC is 0 the module is disabled. Note: As long as the new divider value RMC is not valid, the register returns 0x0000 00xx on reading. */
#define I2C_CLC_RMC_MASK 0x0000FF00
/** field offset */
#define I2C_CLC_RMC_OFFSET 8
/** Fast Shut-Off Enable Bit */
#define I2C_CLC_FSOE 0x00000020
/* Disable
#define I2C_CLC_FSOE_DIS 0x00000000 */
/** Enable */
#define I2C_CLC_FSOE_EN 0x00000020
/** Suspend Bit Write Enable for OCDS */
#define I2C_CLC_SBWE 0x00000010
/* Disable
#define I2C_CLC_SBWE_DIS 0x00000000 */
/** Enable */
#define I2C_CLC_SBWE_EN 0x00000010
/** Disable External Request Disable */
#define I2C_CLC_EDIS 0x00000008
/* Enable
#define I2C_CLC_EDIS_EN 0x00000000 */
/** Disable */
#define I2C_CLC_EDIS_DIS 0x00000008
/** Suspend Enable Bit for OCDS */
#define I2C_CLC_SPEN 0x00000004
/* Disable
#define I2C_CLC_SPEN_DIS 0x00000000 */
/** Enable */
#define I2C_CLC_SPEN_EN 0x00000004
/** Disable Status Bit
    Bit DISS can be modified only by writing to bit DISR */
#define I2C_CLC_DISS 0x00000002
/* Enable
#define I2C_CLC_DISS_EN 0x00000000 */
/** Disable */
#define I2C_CLC_DISS_DIS 0x00000002
/** Disable Request Bit */
#define I2C_CLC_DISR 0x00000001
/* Module disable not requested
#define I2C_CLC_DISR_OFF 0x00000000 */
/** Module disable requested */
#define I2C_CLC_DISR_ON 0x00000001

/* Fields of "I2C Identification Register" */
/** Module ID */
#define I2C_ID_ID_MASK 0x0000FF00
/** field offset */
#define I2C_ID_ID_OFFSET 8
/** Revision */
#define I2C_ID_REV_MASK 0x000000FF
/** field offset */
#define I2C_ID_REV_OFFSET 0

/* Fields of "I2C RUN Control Register" */
/** Enabling I2C Interface
    Only when this bit is set to zero, the configuration registers of the I2C peripheral are writable by SW. */
#define I2C_RUN_CTRL_RUN 0x00000001
/* Disable
#define I2C_RUN_CTRL_RUN_DIS 0x00000000 */
/** Enable */
#define I2C_RUN_CTRL_RUN_EN 0x00000001

/* Fields of "I2C End Data Control Register" */
/** Set End of Transmission
    Note:Do not write '1' to this bit when bus is free. This will cause an abort after the first byte when a new transfer is started. */
#define I2C_ENDD_CTRL_SETEND 0x00000002
/* No-Operation
#define I2C_ENDD_CTRL_SETEND_NOP 0x00000000 */
/** Master Receives Bytes */
#define I2C_ENDD_CTRL_SETEND_MRB 0x00000002
/** Set Restart Condition */
#define I2C_ENDD_CTRL_SETRSC 0x00000001
/* No-Operation
#define I2C_ENDD_CTRL_SETRSC_NOP 0x00000000 */
/** Master Restart */
#define I2C_ENDD_CTRL_SETRSC_RESTART 0x00000001

/* Fields of "I2C Fractional Divider Configuration Register" */
/** Decrement Value of fractional divider */
#define I2C_FDIV_CFG_INC_MASK 0x00FF0000
/** field offset */
#define I2C_FDIV_CFG_INC_OFFSET 16
/** Increment Value of fractional divider */
#define I2C_FDIV_CFG_DEC_MASK 0x000007FF
/** field offset */
#define I2C_FDIV_CFG_DEC_OFFSET 0

/* Fields of "I2C Fractional Divider (highspeed mode) Configuration Register" */
/** Decrement Value of fractional divider */
#define I2C_FDIV_HIGH_CFG_INC_MASK 0x00FF0000
/** field offset */
#define I2C_FDIV_HIGH_CFG_INC_OFFSET 16
/** Increment Value of fractional divider */
#define I2C_FDIV_HIGH_CFG_DEC_MASK 0x000007FF
/** field offset */
#define I2C_FDIV_HIGH_CFG_DEC_OFFSET 0

/* Fields of "I2C Address Configuration Register" */
/** Stop on Packet End
    If device works as receiver a not acknowledge is generated in both cases. After successful transmission of a master code (during high speed mode) SOPE is not considered till a stop condition is manually generated by SETEND. */
#define I2C_ADDR_CFG_SOPE 0x00200000
/* Disable
#define I2C_ADDR_CFG_SOPE_DIS 0x00000000 */
/** Enable */
#define I2C_ADDR_CFG_SOPE_EN 0x00200000
/** Stop on Not Acknowledge
    After successful transmission of a master code (during high speed mode) SONA is not considered till a stop condition is manually generated by SETEND. */
#define I2C_ADDR_CFG_SONA 0x00100000
/* Disable
#define I2C_ADDR_CFG_SONA_DIS 0x00000000 */
/** Enable */
#define I2C_ADDR_CFG_SONA_EN 0x00100000
/** Master Enable */
#define I2C_ADDR_CFG_MnS 0x00080000
/* Disable
#define I2C_ADDR_CFG_MnS_DIS 0x00000000 */
/** Enable */
#define I2C_ADDR_CFG_MnS_EN 0x00080000
/** Master Code Enable */
#define I2C_ADDR_CFG_MCE 0x00040000
/* Disable
#define I2C_ADDR_CFG_MCE_DIS 0x00000000 */
/** Enable */
#define I2C_ADDR_CFG_MCE_EN 0x00040000
/** General Call Enable */
#define I2C_ADDR_CFG_GCE 0x00020000
/* Disable
#define I2C_ADDR_CFG_GCE_DIS 0x00000000 */
/** Enable */
#define I2C_ADDR_CFG_GCE_EN 0x00020000
/** Ten Bit Address Mode */
#define I2C_ADDR_CFG_TBAM 0x00010000
/* 7-bit address mode enabled.
#define I2C_ADDR_CFG_TBAM_7bit 0x00000000 */
/** 10-bit address mode enabled. */
#define I2C_ADDR_CFG_TBAM_10bit 0x00010000
/** I2C Bus device address
    This is the address of this device. (Watch out for reserved addresses by referring to Phillips Spec V2.1) This could either be a 7bit- address (bits [7:1]) or a 10bit- address (bits [9:0]). Note:The validity of the bits are in accordance with the TBAM bit. Bit-1 (Bit-0) is the LSB of the device address. */
#define I2C_ADDR_CFG_ADR_MASK 0x000003FF
/** field offset */
#define I2C_ADDR_CFG_ADR_OFFSET 0

/* Fields of "I2C Bus Status Register" */
/** Read / not Write */
#define I2C_BUS_STAT_RNW 0x00000004
/* Write to I2C Bus.
#define I2C_BUS_STAT_RNW_WRITE 0x00000000 */
/** Read from I2C Bus. */
#define I2C_BUS_STAT_RNW_READ 0x00000004
/** Bus Status */
#define I2C_BUS_STAT_BS_MASK 0x00000003
/** field offset */
#define I2C_BUS_STAT_BS_OFFSET 0
/** I2C Bus is free. */
#define I2C_BUS_STAT_BS_FREE 0x00000000
/** A start condition has been detected on the bus (bus busy). */
#define I2C_BUS_STAT_BS_SC 0x00000001
/** The device is working as master and has claimed the control on the I2C-bus (busy master). */
#define I2C_BUS_STAT_BS_BM 0x00000002
/** A remote master has accessed this device as slave. */
#define I2C_BUS_STAT_BS_RM 0x00000003

/* Fields of "I2C FIFO Configuration Register" */
/** TX FIFO Flow Control */
#define I2C_FIFO_CFG_TXFC 0x00020000
/* TX FIFO not as Flow Controller
#define I2C_FIFO_CFG_TXFC_TXNFC 0x00000000 */
/** RX FIFO Flow Control */
#define I2C_FIFO_CFG_RXFC 0x00010000
/* RX FIFO not as Flow Controller
#define I2C_FIFO_CFG_RXFC_RXNFC 0x00000000 */
/** The reset value depends on the used character sizes of the peripheral. The maximum selectable alignment depends on the maximum number of characters per stage. */
#define I2C_FIFO_CFG_TXFA_MASK 0x00003000
/** field offset */
#define I2C_FIFO_CFG_TXFA_OFFSET 12
/** Byte aligned (character alignment) */
#define I2C_FIFO_CFG_TXFA_TXFA0 0x00000000
/** Half word aligned (character alignment of two characters) */
#define I2C_FIFO_CFG_TXFA_TXFA1 0x00001000
/** Word aligned (character alignment of four characters) */
#define I2C_FIFO_CFG_TXFA_TXFA2 0x00002000
/** Double word aligned (character alignment of eight */
#define I2C_FIFO_CFG_TXFA_TXFA3 0x00003000
/** The reset value depends on the used character sizes of the peripheral. The maximum selectable alignment depends on the maximum number of characters per stage. */
#define I2C_FIFO_CFG_RXFA_MASK 0x00000300
/** field offset */
#define I2C_FIFO_CFG_RXFA_OFFSET 8
/** Byte aligned (character alignment) */
#define I2C_FIFO_CFG_RXFA_RXFA0 0x00000000
/** Half word aligned (character alignment of two characters) */
#define I2C_FIFO_CFG_RXFA_RXFA1 0x00000100
/** Word aligned (character alignment of four characters) */
#define I2C_FIFO_CFG_RXFA_RXFA2 0x00000200
/** Double word aligned (character alignment of eight */
#define I2C_FIFO_CFG_RXFA_RXFA3 0x00000300
/** DMA controller does not support a burst size of 2 words. The reset value is the half of the FIFO size. The maximum selectable burst size is smaller than the FIFO size. */
#define I2C_FIFO_CFG_TXBS_MASK 0x00000030
/** field offset */
#define I2C_FIFO_CFG_TXBS_OFFSET 4
/** 1 word */
#define I2C_FIFO_CFG_TXBS_TXBS0 0x00000000
/** 2 words */
#define I2C_FIFO_CFG_TXBS_TXBS1 0x00000010
/** 4 words */
#define I2C_FIFO_CFG_TXBS_TXBS2 0x00000020
/** 8 words */
#define I2C_FIFO_CFG_TXBS_TXBS3 0x00000030
/** DMA controller does not support a burst size of 2 words. The reset value is the half of the FIFO size. The maximum selectable burst size is smaller than the FIFO size. */
#define I2C_FIFO_CFG_RXBS_MASK 0x00000003
/** field offset */
#define I2C_FIFO_CFG_RXBS_OFFSET 0
/** 1 word */
#define I2C_FIFO_CFG_RXBS_RXBS0 0x00000000
/** 2 words */
#define I2C_FIFO_CFG_RXBS_RXBS1 0x00000001
/** 4 words */
#define I2C_FIFO_CFG_RXBS_RXBS2 0x00000002
/** 8 words */
#define I2C_FIFO_CFG_RXBS_RXBS3 0x00000003

/* Fields of "I2C Maximum Received Packet Size Register" */
/** MRPS */
#define I2C_MRPS_CTRL_MRPS_MASK 0x00003FFF
/** field offset */
#define I2C_MRPS_CTRL_MRPS_OFFSET 0

/* Fields of "I2C Received Packet Size Status Register" */
/** RPS */
#define I2C_RPS_STAT_RPS_MASK 0x00003FFF
/** field offset */
#define I2C_RPS_STAT_RPS_OFFSET 0

/* Fields of "I2C Transmit Packet Size Register" */
/** TPS */
#define I2C_TPS_CTRL_TPS_MASK 0x00003FFF
/** field offset */
#define I2C_TPS_CTRL_TPS_OFFSET 0

/* Fields of "I2C Filled FIFO Stages Status Register" */
/** FFS */
#define I2C_FFS_STAT_FFS_MASK 0x0000000F
/** field offset */
#define I2C_FFS_STAT_FFS_OFFSET 0

/* Fields of "I2C Timing Configuration Register" */
/** SDA Delay Stages for Start/Stop bit in High Speed Mode
    The actual delay is calculated as the value of this field + 3 */
#define I2C_TIM_CFG_HS_SDA_DEL_MASK 0x00070000
/** field offset */
#define I2C_TIM_CFG_HS_SDA_DEL_OFFSET 16
/** Enable Fast Mode SCL Low period timing */
#define I2C_TIM_CFG_FS_SCL_LOW 0x00008000
/* Disable
#define I2C_TIM_CFG_FS_SCL_LOW_DIS 0x00000000 */
/** Enable */
#define I2C_TIM_CFG_FS_SCL_LOW_EN 0x00008000
/** SCL Delay Stages for Hold Time Start (Restart) Bit.
    The actual delay is calculated as the value of this field + 2 */
#define I2C_TIM_CFG_SCL_DEL_HD_STA_MASK 0x00000E00
/** field offset */
#define I2C_TIM_CFG_SCL_DEL_HD_STA_OFFSET 9
/** SDA Delay Stages for Start/Stop bit in High Speed Mode
    The actual delay is calculated as the value of this field + 3 */
#define I2C_TIM_CFG_HS_SDA_DEL_HD_DAT_MASK 0x000001C0
/** field offset */
#define I2C_TIM_CFG_HS_SDA_DEL_HD_DAT_OFFSET 6
/** SDA Delay Stages for Start/Stop bit in High Speed Mode
    The actual delay is calculated as the value of this field + 3 */
#define I2C_TIM_CFG_SDA_DEL_HD_DAT_MASK 0x0000003F
/** field offset */
#define I2C_TIM_CFG_SDA_DEL_HD_DAT_OFFSET 0

/* Fields of "I2C Error Interrupt Request Source Mask Register" */
/** Enables the corresponding error interrupt. */
#define I2C_ERR_IRQSM_TXF_OFL 0x00000008
/* Disable
#define I2C_ERR_IRQSM_TXF_OFL_DIS 0x00000000 */
/** Enable */
#define I2C_ERR_IRQSM_TXF_OFL_EN 0x00000008
/** Enables the corresponding error interrupt. */
#define I2C_ERR_IRQSM_TXF_UFL 0x00000004
/* Disable
#define I2C_ERR_IRQSM_TXF_UFL_DIS 0x00000000 */
/** Enable */
#define I2C_ERR_IRQSM_TXF_UFL_EN 0x00000004
/** Enables the corresponding error interrupt. */
#define I2C_ERR_IRQSM_RXF_OFL 0x00000002
/* Disable
#define I2C_ERR_IRQSM_RXF_OFL_DIS 0x00000000 */
/** Enable */
#define I2C_ERR_IRQSM_RXF_OFL_EN 0x00000002
/** Enables the corresponding error interrupt. */
#define I2C_ERR_IRQSM_RXF_UFL 0x00000001
/* Disable
#define I2C_ERR_IRQSM_RXF_UFL_DIS 0x00000000 */
/** Enable */
#define I2C_ERR_IRQSM_RXF_UFL_EN 0x00000001

/* Fields of "I2C Error Interrupt Request Source Status Register" */
/** TXF_OFL */
#define I2C_ERR_IRQSS_TXF_OFL 0x00000008
/* Nothing
#define I2C_ERR_IRQSS_TXF_OFL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_ERR_IRQSS_TXF_OFL_INTOCC 0x00000008
/** TXF_UFL */
#define I2C_ERR_IRQSS_TXF_UFL 0x00000004
/* Nothing
#define I2C_ERR_IRQSS_TXF_UFL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_ERR_IRQSS_TXF_UFL_INTOCC 0x00000004
/** RXF_OFL */
#define I2C_ERR_IRQSS_RXF_OFL 0x00000002
/* Nothing
#define I2C_ERR_IRQSS_RXF_OFL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_ERR_IRQSS_RXF_OFL_INTOCC 0x00000002
/** RXF_UFL */
#define I2C_ERR_IRQSS_RXF_UFL 0x00000001
/* Nothing
#define I2C_ERR_IRQSS_RXF_UFL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_ERR_IRQSS_RXF_UFL_INTOCC 0x00000001

/* Fields of "I2C Error Interrupt Request Source Clear Register" */
/** TXF_OFL */
#define I2C_ERR_IRQSC_TXF_OFL 0x00000008
/* No-Operation
#define I2C_ERR_IRQSC_TXF_OFL_NOP 0x00000000 */
/** Clear */
#define I2C_ERR_IRQSC_TXF_OFL_CLR 0x00000008
/** TXF_UFL */
#define I2C_ERR_IRQSC_TXF_UFL 0x00000004
/* No-Operation
#define I2C_ERR_IRQSC_TXF_UFL_NOP 0x00000000 */
/** Clear */
#define I2C_ERR_IRQSC_TXF_UFL_CLR 0x00000004
/** RXF_OFL */
#define I2C_ERR_IRQSC_RXF_OFL 0x00000002
/* No-Operation
#define I2C_ERR_IRQSC_RXF_OFL_NOP 0x00000000 */
/** Clear */
#define I2C_ERR_IRQSC_RXF_OFL_CLR 0x00000002
/** RXF_UFL */
#define I2C_ERR_IRQSC_RXF_UFL 0x00000001
/* No-Operation
#define I2C_ERR_IRQSC_RXF_UFL_NOP 0x00000000 */
/** Clear */
#define I2C_ERR_IRQSC_RXF_UFL_CLR 0x00000001

/* Fields of "I2C Protocol Interrupt Request Source Mask Register" */
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_RX 0x00000040
/* Disable
#define I2C_P_IRQSM_RX_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_RX_EN 0x00000040
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_TX_END 0x00000020
/* Disable
#define I2C_P_IRQSM_TX_END_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_TX_END_EN 0x00000020
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_NACK 0x00000010
/* Disable
#define I2C_P_IRQSM_NACK_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_NACK_EN 0x00000010
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_AL 0x00000008
/* Disable
#define I2C_P_IRQSM_AL_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_AL_EN 0x00000008
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_MC 0x00000004
/* Disable
#define I2C_P_IRQSM_MC_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_MC_EN 0x00000004
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_GC 0x00000002
/* Disable
#define I2C_P_IRQSM_GC_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_GC_EN 0x00000002
/** Enables the corresponding interrupt. */
#define I2C_P_IRQSM_AM 0x00000001
/* Disable
#define I2C_P_IRQSM_AM_DIS 0x00000000 */
/** Enable */
#define I2C_P_IRQSM_AM_EN 0x00000001

/* Fields of "I2C Protocol Interrupt Request Source Status Register" */
/** RX */
#define I2C_P_IRQSS_RX 0x00000040
/* Nothing
#define I2C_P_IRQSS_RX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_RX_INTOCC 0x00000040
/** TX_END */
#define I2C_P_IRQSS_TX_END 0x00000020
/* Nothing
#define I2C_P_IRQSS_TX_END_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_TX_END_INTOCC 0x00000020
/** NACK */
#define I2C_P_IRQSS_NACK 0x00000010
/* Nothing
#define I2C_P_IRQSS_NACK_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_NACK_INTOCC 0x00000010
/** AL */
#define I2C_P_IRQSS_AL 0x00000008
/* Nothing
#define I2C_P_IRQSS_AL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_AL_INTOCC 0x00000008
/** MC */
#define I2C_P_IRQSS_MC 0x00000004
/* Nothing
#define I2C_P_IRQSS_MC_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_MC_INTOCC 0x00000004
/** GC */
#define I2C_P_IRQSS_GC 0x00000002
/* Nothing
#define I2C_P_IRQSS_GC_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_GC_INTOCC 0x00000002
/** AM */
#define I2C_P_IRQSS_AM 0x00000001
/* Nothing
#define I2C_P_IRQSS_AM_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_P_IRQSS_AM_INTOCC 0x00000001

/* Fields of "I2C Protocol Interrupt Request Source Clear Register" */
/** RX */
#define I2C_P_IRQSC_RX 0x00000040
/* No-Operation
#define I2C_P_IRQSC_RX_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_RX_CLR 0x00000040
/** TX_END */
#define I2C_P_IRQSC_TX_END 0x00000020
/* No-Operation
#define I2C_P_IRQSC_TX_END_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_TX_END_CLR 0x00000020
/** NACK */
#define I2C_P_IRQSC_NACK 0x00000010
/* No-Operation
#define I2C_P_IRQSC_NACK_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_NACK_CLR 0x00000010
/** AL */
#define I2C_P_IRQSC_AL 0x00000008
/* No-Operation
#define I2C_P_IRQSC_AL_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_AL_CLR 0x00000008
/** MC */
#define I2C_P_IRQSC_MC 0x00000004
/* No-Operation
#define I2C_P_IRQSC_MC_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_MC_CLR 0x00000004
/** GC */
#define I2C_P_IRQSC_GC 0x00000002
/* No-Operation
#define I2C_P_IRQSC_GC_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_GC_CLR 0x00000002
/** AM */
#define I2C_P_IRQSC_AM 0x00000001
/* No-Operation
#define I2C_P_IRQSC_AM_NOP 0x00000000 */
/** Clear */
#define I2C_P_IRQSC_AM_CLR 0x00000001

/* Fields of "I2C Raw Interrupt Status Register" */
/** This is the combined interrupt bit for indication of an protocol event in the I2C kernel. */
#define I2C_RIS_I2C_P_INT 0x00000020
/* Nothing
#define I2C_RIS_I2C_P_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_I2C_P_INT_INTOCC 0x00000020
/** This is the combined interrupt bit for indication of FIFO errors due to overflow and underrun. */
#define I2C_RIS_I2C_ERR_INT 0x00000010
/* Nothing
#define I2C_RIS_I2C_ERR_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_I2C_ERR_INT_INTOCC 0x00000010
/** BREQ_INT */
#define I2C_RIS_BREQ_INT 0x00000008
/* Nothing
#define I2C_RIS_BREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_BREQ_INT_INTOCC 0x00000008
/** LBREQ_INT */
#define I2C_RIS_LBREQ_INT 0x00000004
/* Nothing
#define I2C_RIS_LBREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_LBREQ_INT_INTOCC 0x00000004
/** SREQ_INT */
#define I2C_RIS_SREQ_INT 0x00000002
/* Nothing
#define I2C_RIS_SREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_SREQ_INT_INTOCC 0x00000002
/** LSREQ_INT */
#define I2C_RIS_LSREQ_INT 0x00000001
/* Nothing
#define I2C_RIS_LSREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_RIS_LSREQ_INT_INTOCC 0x00000001

/* Fields of "I2C Interrupt Mask Control Register" */
/** This is the combined interrupt bit for indication of an protocol event in the I2C kernel. */
#define I2C_IMSC_I2C_P_INT 0x00000020
/* Disable
#define I2C_IMSC_I2C_P_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_I2C_P_INT_EN 0x00000020
/** This is the combined interrupt bit for indication of FIFO errors due to overflow and underrun. */
#define I2C_IMSC_I2C_ERR_INT 0x00000010
/* Disable
#define I2C_IMSC_I2C_ERR_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_I2C_ERR_INT_EN 0x00000010
/** BREQ_INT */
#define I2C_IMSC_BREQ_INT 0x00000008
/* Disable
#define I2C_IMSC_BREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_BREQ_INT_EN 0x00000008
/** LBREQ_INT */
#define I2C_IMSC_LBREQ_INT 0x00000004
/* Disable
#define I2C_IMSC_LBREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_LBREQ_INT_EN 0x00000004
/** SREQ_INT */
#define I2C_IMSC_SREQ_INT 0x00000002
/* Disable
#define I2C_IMSC_SREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_SREQ_INT_EN 0x00000002
/** LSREQ_INT */
#define I2C_IMSC_LSREQ_INT 0x00000001
/* Disable
#define I2C_IMSC_LSREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_IMSC_LSREQ_INT_EN 0x00000001

/* Fields of "I2C Masked Interrupt Status Register" */
/** This is the combined interrupt bit for indication of an protocol event in the I2C kernel. */
#define I2C_MIS_I2C_P_INT 0x00000020
/* Nothing
#define I2C_MIS_I2C_P_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_I2C_P_INT_INTOCC 0x00000020
/** This is the combined interrupt bit for indication of FIFO errors due to overflow and underrun. */
#define I2C_MIS_I2C_ERR_INT 0x00000010
/* Nothing
#define I2C_MIS_I2C_ERR_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_I2C_ERR_INT_INTOCC 0x00000010
/** BREQ_INT */
#define I2C_MIS_BREQ_INT 0x00000008
/* Nothing
#define I2C_MIS_BREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_BREQ_INT_INTOCC 0x00000008
/** LBREQ_INT */
#define I2C_MIS_LBREQ_INT 0x00000004
/* Nothing
#define I2C_MIS_LBREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_LBREQ_INT_INTOCC 0x00000004
/** SREQ_INT */
#define I2C_MIS_SREQ_INT 0x00000002
/* Nothing
#define I2C_MIS_SREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_SREQ_INT_INTOCC 0x00000002
/** LSREQ_INT */
#define I2C_MIS_LSREQ_INT 0x00000001
/* Nothing
#define I2C_MIS_LSREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define I2C_MIS_LSREQ_INT_INTOCC 0x00000001

/* Fields of "I2C Interrupt Clear Register" */
/** This is the combined interrupt bit for indication of an protocol event in the I2C kernel. */
#define I2C_ICR_I2C_P_INT 0x00000020
/* No-Operation
#define I2C_ICR_I2C_P_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_I2C_P_INT_CLR 0x00000020
/** This is the combined interrupt bit for indication of FIFO errors due to overflow and underrun. */
#define I2C_ICR_I2C_ERR_INT 0x00000010
/* No-Operation
#define I2C_ICR_I2C_ERR_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_I2C_ERR_INT_CLR 0x00000010
/** BREQ_INT */
#define I2C_ICR_BREQ_INT 0x00000008
/* No-Operation
#define I2C_ICR_BREQ_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_BREQ_INT_CLR 0x00000008
/** LBREQ_INT */
#define I2C_ICR_LBREQ_INT 0x00000004
/* No-Operation
#define I2C_ICR_LBREQ_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_LBREQ_INT_CLR 0x00000004
/** SREQ_INT */
#define I2C_ICR_SREQ_INT 0x00000002
/* No-Operation
#define I2C_ICR_SREQ_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_SREQ_INT_CLR 0x00000002
/** LSREQ_INT */
#define I2C_ICR_LSREQ_INT 0x00000001
/* No-Operation
#define I2C_ICR_LSREQ_INT_NOP 0x00000000 */
/** Clear */
#define I2C_ICR_LSREQ_INT_CLR 0x00000001

/* Fields of "I2C Interrupt Set Register" */
/** This is the combined interrupt bit for indication of an protocol event in the I2C kernel. */
#define I2C_ISR_I2C_P_INT 0x00000020
/* No-Operation
#define I2C_ISR_I2C_P_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_I2C_P_INT_SET 0x00000020
/** This is the combined interrupt bit for indication of FIFO errors due to overflow and underrun. */
#define I2C_ISR_I2C_ERR_INT 0x00000010
/* No-Operation
#define I2C_ISR_I2C_ERR_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_I2C_ERR_INT_SET 0x00000010
/** BREQ_INT */
#define I2C_ISR_BREQ_INT 0x00000008
/* No-Operation
#define I2C_ISR_BREQ_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_BREQ_INT_SET 0x00000008
/** LBREQ_INT */
#define I2C_ISR_LBREQ_INT 0x00000004
/* No-Operation
#define I2C_ISR_LBREQ_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_LBREQ_INT_SET 0x00000004
/** SREQ_INT */
#define I2C_ISR_SREQ_INT 0x00000002
/* No-Operation
#define I2C_ISR_SREQ_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_SREQ_INT_SET 0x00000002
/** LSREQ_INT */
#define I2C_ISR_LSREQ_INT 0x00000001
/* No-Operation
#define I2C_ISR_LSREQ_INT_NOP 0x00000000 */
/** Set */
#define I2C_ISR_LSREQ_INT_SET 0x00000001

/* Fields of "I2C DMA Enable Register" */
/** BREQ_INT */
#define I2C_DMAE_BREQ_INT 0x00000008
/* Disable
#define I2C_DMAE_BREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_DMAE_BREQ_INT_EN 0x00000008
/** LBREQ_INT */
#define I2C_DMAE_LBREQ_INT 0x00000004
/* Disable
#define I2C_DMAE_LBREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_DMAE_LBREQ_INT_EN 0x00000004
/** SREQ_INT */
#define I2C_DMAE_SREQ_INT 0x00000002
/* Disable
#define I2C_DMAE_SREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_DMAE_SREQ_INT_EN 0x00000002
/** LSREQ_INT */
#define I2C_DMAE_LSREQ_INT 0x00000001
/* Disable
#define I2C_DMAE_LSREQ_INT_DIS 0x00000000 */
/** Enable */
#define I2C_DMAE_LSREQ_INT_EN 0x00000001

/* Fields of "I2C Transmit Data Register" */
/** Characters to be transmitted */
#define I2C_TXD_TXD_MASK 0xFFFFFFFF
/** field offset */
#define I2C_TXD_TXD_OFFSET 0

/* Fields of "I2C Receive Data Register" */
/** Received characters */
#define I2C_RXD_RXD_MASK 0xFFFFFFFF
/** field offset */
#define I2C_RXD_RXD_OFFSET 0

/*! @} */ /* I2C_REGISTER */

#endif /* _i2c_reg_h */
