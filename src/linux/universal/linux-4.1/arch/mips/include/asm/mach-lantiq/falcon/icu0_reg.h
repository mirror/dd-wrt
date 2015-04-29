/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _icu0_reg_h
#define _icu0_reg_h

/** \addtogroup ICU0_REGISTER
   @{
*/
/* access macros */
#define icu0_r32(reg) reg_r32(&icu0->reg)
#define icu0_w32(val, reg) reg_w32(val, &icu0->reg)
#define icu0_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &icu0->reg)
#define icu0_r32_table(reg, idx) reg_r32_table(icu0->reg, idx)
#define icu0_w32_table(val, reg, idx) reg_w32_table(val, icu0->reg, idx)
#define icu0_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, icu0->reg, idx)
#define icu0_adr_table(reg, idx) adr_table(icu0->reg, idx)


/** ICU0 register structure */
struct gpon_reg_icu0
{
   /** IM0 Interrupt Status Register
       A read action to this register delivers the unmasked captured status of the interrupt request lines. Each bit can be cleared by a write operation. */
   unsigned int im0_isr; /* 0x00000000 */
   /** Reserved */
   unsigned int res_0; /* 0x00000004 */
   /** IM0 Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IM0_IOSR register and are not signalled via the interrupt line towards the controller. */
   unsigned int im0_ier; /* 0x00000008 */
   /** Reserved */
   unsigned int res_1; /* 0x0000000C */
   /** IM0 Interrupt Output Status Register
       This register shows the currently active interrupt requests masked with the corresponding enable bits of the IM0_IER register. */
   unsigned int im0_iosr; /* 0x00000010 */
   /** Reserved */
   unsigned int res_2; /* 0x00000014 */
   /** IM0 Interrupt Request Set Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int im0_irsr; /* 0x00000018 */
   /** Reserved */
   unsigned int res_3; /* 0x0000001C */
   /** IM0 Interrupt Mode Register
       This register shows the type of interrupt for each bit. */
   unsigned int im0_imr; /* 0x00000020 */
   /** Reserved */
   unsigned int res_4; /* 0x00000024 */
   /** IM1 Interrupt Status Register
       A read action to this register delivers the unmasked captured status of the interrupt request lines. Each bit can be cleared by a write operation. */
   unsigned int im1_isr; /* 0x00000028 */
   /** Reserved */
   unsigned int res_5; /* 0x0000002C */
   /** IM1 Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IM1_IOSR register and are not signalled via the interrupt line towards the controller. */
   unsigned int im1_ier; /* 0x00000030 */
   /** Reserved */
   unsigned int res_6; /* 0x00000034 */
   /** IM1 Interrupt Output Status Register
       This register shows the currently active interrupt requests masked with the corresponding enable bits of the IM1_IER register. */
   unsigned int im1_iosr; /* 0x00000038 */
   /** Reserved */
   unsigned int res_7; /* 0x0000003C */
   /** IM1 Interrupt Request Set Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int im1_irsr; /* 0x00000040 */
   /** Reserved */
   unsigned int res_8; /* 0x00000044 */
   /** IM1 Interrupt Mode Register
       This register shows the type of interrupt for each bit. */
   unsigned int im1_imr; /* 0x00000048 */
   /** Reserved */
   unsigned int res_9; /* 0x0000004C */
   /** IM2 Interrupt Status Register
       A read action to this register delivers the unmasked captured status of the interrupt request lines. Each bit can be cleared by a write operation. */
   unsigned int im2_isr; /* 0x00000050 */
   /** Reserved */
   unsigned int res_10; /* 0x00000054 */
   /** IM2 Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IM2_IOSR register and are not signalled via the interrupt line towards the controller. */
   unsigned int im2_ier; /* 0x00000058 */
   /** Reserved */
   unsigned int res_11; /* 0x0000005C */
   /** IM2 Interrupt Output Status Register
       This register shows the currently active interrupt requests masked with the corresponding enable bits of the IM2_IER register. */
   unsigned int im2_iosr; /* 0x00000060 */
   /** Reserved */
   unsigned int res_12; /* 0x00000064 */
   /** IM2 Interrupt Request Set Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int im2_irsr; /* 0x00000068 */
   /** Reserved */
   unsigned int res_13; /* 0x0000006C */
   /** IM2 Interrupt Mode Register
       This register shows the type of interrupt for each bit. */
   unsigned int im2_imr; /* 0x00000070 */
   /** Reserved */
   unsigned int res_14; /* 0x00000074 */
   /** IM3 Interrupt Status Register
       A read action to this register delivers the unmasked captured status of the interrupt request lines. Each bit can be cleared by a write operation. */
   unsigned int im3_isr; /* 0x00000078 */
   /** Reserved */
   unsigned int res_15; /* 0x0000007C */
   /** IM3 Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IM3_IOSR register and are not signalled via the interrupt line towards the controller. */
   unsigned int im3_ier; /* 0x00000080 */
   /** Reserved */
   unsigned int res_16; /* 0x00000084 */
   /** IM3 Interrupt Output Status Register
       This register shows the currently active interrupt requests masked with the corresponding enable bits of the IM3_IER register. */
   unsigned int im3_iosr; /* 0x00000088 */
   /** Reserved */
   unsigned int res_17; /* 0x0000008C */
   /** IM3 Interrupt Request Set Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int im3_irsr; /* 0x00000090 */
   /** Reserved */
   unsigned int res_18; /* 0x00000094 */
   /** IM3 Interrupt Mode Register
       This register shows the type of interrupt for each bit. */
   unsigned int im3_imr; /* 0x00000098 */
   /** Reserved */
   unsigned int res_19; /* 0x0000009C */
   /** IM4 Interrupt Status Register
       A read action to this register delivers the unmasked captured status of the interrupt request lines. Each bit can be cleared by a write operation. */
   unsigned int im4_isr; /* 0x000000A0 */
   /** Reserved */
   unsigned int res_20; /* 0x000000A4 */
   /** IM4 Interrupt Enable Register
       This register contains the enable (or mask) bits for the interrupts. Disabled interrupts are not visible in the IM4_IOSR register and are not signalled via the interrupt line towards the controller. */
   unsigned int im4_ier; /* 0x000000A8 */
   /** Reserved */
   unsigned int res_21; /* 0x000000AC */
   /** IM4 Interrupt Output Status Register
       This register shows the currently active interrupt requests masked with the corresponding enable bits of the IM4_IER register. */
   unsigned int im4_iosr; /* 0x000000B0 */
   /** Reserved */
   unsigned int res_22; /* 0x000000B4 */
   /** IM4 Interrupt Request Set Register
       A write operation directly effects the interrupts. This can be used to trigger events under software control for testing purposes. A read operation returns the unmasked interrupt events. */
   unsigned int im4_irsr; /* 0x000000B8 */
   /** Reserved */
   unsigned int res_23; /* 0x000000BC */
   /** IM4 Interrupt Mode Register
       This register shows the type of interrupt for each bit. */
   unsigned int im4_imr; /* 0x000000C0 */
   /** Reserved */
   unsigned int res_24; /* 0x000000C4 */
   /** ICU Interrupt Vector Register (5 bit variant)
       Shows the leftmost pending interrupt request. If e.g. bit 14 of the IOSR register is set, 15 is reported, because the 15th interrupt request is active. */
   unsigned int icu_ivec; /* 0x000000C8 */
   /** Reserved */
   unsigned int res_25; /* 0x000000CC */
   /** ICU Interrupt Vector Register (6 bit variant)
       Shows the leftmost pending interrupt request. If e.g. bit 14 of the IOSR register is set, 15 is reported, because the 15th interrupt request is active. */
   unsigned int icu_ivec_6; /* 0x000000D0 */
   /** Reserved */
   unsigned int res_26[3]; /* 0x000000D4 */
};


/* Fields of "IM0 Interrupt Status Register" */
/** PCM Transmit Crash Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_PCM_HW2_CRASH 0x80000000
/* Nothing
#define ICU0_IM0_ISR_PCM_HW2_CRASH_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_PCM_HW2_CRASH_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_PCM_HW2_CRASH_INTOCC 0x80000000
/** PCM Transmit Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_PCM_TX 0x40000000
/* Nothing
#define ICU0_IM0_ISR_PCM_TX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_PCM_TX_INTACK 0x40000000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_PCM_TX_INTOCC 0x40000000
/** PCM Receive Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_PCM_RX 0x20000000
/* Nothing
#define ICU0_IM0_ISR_PCM_RX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_PCM_RX_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_PCM_RX_INTOCC 0x20000000
/** Secure Hash Algorithm Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_SHA1_HASH 0x10000000
/* Nothing
#define ICU0_IM0_ISR_SHA1_HASH_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_SHA1_HASH_INTACK 0x10000000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_SHA1_HASH_INTOCC 0x10000000
/** Advanced Encryption Standard Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_AES_AES 0x08000000
/* Nothing
#define ICU0_IM0_ISR_AES_AES_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_AES_AES_INTACK 0x08000000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_AES_AES_INTOCC 0x08000000
/** SSC Frame Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_SSC0_F 0x00020000
/* Nothing
#define ICU0_IM0_ISR_SSC0_F_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_SSC0_F_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_SSC0_F_INTOCC 0x00020000
/** SSC Error Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_SSC0_E 0x00010000
/* Nothing
#define ICU0_IM0_ISR_SSC0_E_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_SSC0_E_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_SSC0_E_INTOCC 0x00010000
/** SSC Receive Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_SSC0_R 0x00008000
/* Nothing
#define ICU0_IM0_ISR_SSC0_R_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_SSC0_R_INTACK 0x00008000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_SSC0_R_INTOCC 0x00008000
/** SSC Transmit Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM0_ISR_SSC0_T 0x00004000
/* Nothing
#define ICU0_IM0_ISR_SSC0_T_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_SSC0_T_INTACK 0x00004000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_SSC0_T_INTOCC 0x00004000
/** I2C Peripheral Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_I2C_P_INT 0x00002000
/* Nothing
#define ICU0_IM0_ISR_I2C_I2C_P_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_I2C_P_INT_INTACK 0x00002000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_I2C_P_INT_INTOCC 0x00002000
/** I2C Error Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_I2C_ERR_INT 0x00001000
/* Nothing
#define ICU0_IM0_ISR_I2C_I2C_ERR_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_I2C_ERR_INT_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_I2C_ERR_INT_INTOCC 0x00001000
/** I2C Burst Data Transfer Request
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_BREQ_INT 0x00000800
/* Nothing
#define ICU0_IM0_ISR_I2C_BREQ_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_BREQ_INT_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_BREQ_INT_INTOCC 0x00000800
/** I2C Last Burst Data Transfer Request
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_LBREQ_INT 0x00000400
/* Nothing
#define ICU0_IM0_ISR_I2C_LBREQ_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_LBREQ_INT_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_LBREQ_INT_INTOCC 0x00000400
/** I2C Single Data Transfer Request
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_SREQ_INT 0x00000200
/* Nothing
#define ICU0_IM0_ISR_I2C_SREQ_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_SREQ_INT_INTACK 0x00000200
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_SREQ_INT_INTOCC 0x00000200
/** I2C Last Single Data Transfer Request
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_I2C_LSREQ_INT 0x00000100
/* Nothing
#define ICU0_IM0_ISR_I2C_LSREQ_INT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_I2C_LSREQ_INT_INTACK 0x00000100
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_I2C_LSREQ_INT_INTOCC 0x00000100
/** HOST IF Mailbox1 Transmit Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_HOST_MB1_TIR 0x00000010
/* Nothing
#define ICU0_IM0_ISR_HOST_MB1_TIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_HOST_MB1_TIR_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_HOST_MB1_TIR_INTOCC 0x00000010
/** HOST IF Mailbox1 Receive Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_HOST_MB1_RIR 0x00000008
/* Nothing
#define ICU0_IM0_ISR_HOST_MB1_RIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_HOST_MB1_RIR_INTACK 0x00000008
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_HOST_MB1_RIR_INTOCC 0x00000008
/** HOST IF Mailbox0 Transmit Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_HOST_MB0_TIR 0x00000004
/* Nothing
#define ICU0_IM0_ISR_HOST_MB0_TIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_HOST_MB0_TIR_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_HOST_MB0_TIR_INTOCC 0x00000004
/** HOST IF Mailbox0 Receive Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_HOST_MB0_RIR 0x00000002
/* Nothing
#define ICU0_IM0_ISR_HOST_MB0_RIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_HOST_MB0_RIR_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_HOST_MB0_RIR_INTOCC 0x00000002
/** HOST IF Event Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM0_ISR_HOST_EIR 0x00000001
/* Nothing
#define ICU0_IM0_ISR_HOST_EIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM0_ISR_HOST_EIR_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define ICU0_IM0_ISR_HOST_EIR_INTOCC 0x00000001

/* Fields of "IM0 Interrupt Enable Register" */
/** PCM Transmit Crash Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_PCM_HW2_CRASH 0x80000000
/* Disable
#define ICU0_IM0_IER_PCM_HW2_CRASH_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_PCM_HW2_CRASH_EN 0x80000000
/** PCM Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_PCM_TX 0x40000000
/* Disable
#define ICU0_IM0_IER_PCM_TX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_PCM_TX_EN 0x40000000
/** PCM Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_PCM_RX 0x20000000
/* Disable
#define ICU0_IM0_IER_PCM_RX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_PCM_RX_EN 0x20000000
/** Secure Hash Algorithm Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_SHA1_HASH 0x10000000
/* Disable
#define ICU0_IM0_IER_SHA1_HASH_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_SHA1_HASH_EN 0x10000000
/** Advanced Encryption Standard Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_AES_AES 0x08000000
/* Disable
#define ICU0_IM0_IER_AES_AES_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_AES_AES_EN 0x08000000
/** SSC Frame Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_SSC0_F 0x00020000
/* Disable
#define ICU0_IM0_IER_SSC0_F_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_SSC0_F_EN 0x00020000
/** SSC Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_SSC0_E 0x00010000
/* Disable
#define ICU0_IM0_IER_SSC0_E_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_SSC0_E_EN 0x00010000
/** SSC Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_SSC0_R 0x00008000
/* Disable
#define ICU0_IM0_IER_SSC0_R_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_SSC0_R_EN 0x00008000
/** SSC Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_SSC0_T 0x00004000
/* Disable
#define ICU0_IM0_IER_SSC0_T_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_SSC0_T_EN 0x00004000
/** I2C Peripheral Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_I2C_P_INT 0x00002000
/* Disable
#define ICU0_IM0_IER_I2C_I2C_P_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_I2C_P_INT_EN 0x00002000
/** I2C Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_I2C_ERR_INT 0x00001000
/* Disable
#define ICU0_IM0_IER_I2C_I2C_ERR_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_I2C_ERR_INT_EN 0x00001000
/** I2C Burst Data Transfer Request
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_BREQ_INT 0x00000800
/* Disable
#define ICU0_IM0_IER_I2C_BREQ_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_BREQ_INT_EN 0x00000800
/** I2C Last Burst Data Transfer Request
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_LBREQ_INT 0x00000400
/* Disable
#define ICU0_IM0_IER_I2C_LBREQ_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_LBREQ_INT_EN 0x00000400
/** I2C Single Data Transfer Request
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_SREQ_INT 0x00000200
/* Disable
#define ICU0_IM0_IER_I2C_SREQ_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_SREQ_INT_EN 0x00000200
/** I2C Last Single Data Transfer Request
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_I2C_LSREQ_INT 0x00000100
/* Disable
#define ICU0_IM0_IER_I2C_LSREQ_INT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_I2C_LSREQ_INT_EN 0x00000100
/** HOST IF Mailbox1 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_HOST_MB1_TIR 0x00000010
/* Disable
#define ICU0_IM0_IER_HOST_MB1_TIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_HOST_MB1_TIR_EN 0x00000010
/** HOST IF Mailbox1 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_HOST_MB1_RIR 0x00000008
/* Disable
#define ICU0_IM0_IER_HOST_MB1_RIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_HOST_MB1_RIR_EN 0x00000008
/** HOST IF Mailbox0 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_HOST_MB0_TIR 0x00000004
/* Disable
#define ICU0_IM0_IER_HOST_MB0_TIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_HOST_MB0_TIR_EN 0x00000004
/** HOST IF Mailbox0 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_HOST_MB0_RIR 0x00000002
/* Disable
#define ICU0_IM0_IER_HOST_MB0_RIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_HOST_MB0_RIR_EN 0x00000002
/** HOST IF Event Interrupt
    Interrupt enable bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IER_HOST_EIR 0x00000001
/* Disable
#define ICU0_IM0_IER_HOST_EIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM0_IER_HOST_EIR_EN 0x00000001

/* Fields of "IM0 Interrupt Output Status Register" */
/** PCM Transmit Crash Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_PCM_HW2_CRASH 0x80000000
/* Nothing
#define ICU0_IM0_IOSR_PCM_HW2_CRASH_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_PCM_HW2_CRASH_INTOCC 0x80000000
/** PCM Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_PCM_TX 0x40000000
/* Nothing
#define ICU0_IM0_IOSR_PCM_TX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_PCM_TX_INTOCC 0x40000000
/** PCM Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_PCM_RX 0x20000000
/* Nothing
#define ICU0_IM0_IOSR_PCM_RX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_PCM_RX_INTOCC 0x20000000
/** Secure Hash Algorithm Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_SHA1_HASH 0x10000000
/* Nothing
#define ICU0_IM0_IOSR_SHA1_HASH_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_SHA1_HASH_INTOCC 0x10000000
/** Advanced Encryption Standard Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_AES_AES 0x08000000
/* Nothing
#define ICU0_IM0_IOSR_AES_AES_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_AES_AES_INTOCC 0x08000000
/** SSC Frame Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_SSC0_F 0x00020000
/* Nothing
#define ICU0_IM0_IOSR_SSC0_F_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_SSC0_F_INTOCC 0x00020000
/** SSC Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_SSC0_E 0x00010000
/* Nothing
#define ICU0_IM0_IOSR_SSC0_E_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_SSC0_E_INTOCC 0x00010000
/** SSC Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_SSC0_R 0x00008000
/* Nothing
#define ICU0_IM0_IOSR_SSC0_R_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_SSC0_R_INTOCC 0x00008000
/** SSC Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_SSC0_T 0x00004000
/* Nothing
#define ICU0_IM0_IOSR_SSC0_T_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_SSC0_T_INTOCC 0x00004000
/** I2C Peripheral Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_I2C_P_INT 0x00002000
/* Nothing
#define ICU0_IM0_IOSR_I2C_I2C_P_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_I2C_P_INT_INTOCC 0x00002000
/** I2C Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_I2C_ERR_INT 0x00001000
/* Nothing
#define ICU0_IM0_IOSR_I2C_I2C_ERR_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_I2C_ERR_INT_INTOCC 0x00001000
/** I2C Burst Data Transfer Request
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_BREQ_INT 0x00000800
/* Nothing
#define ICU0_IM0_IOSR_I2C_BREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_BREQ_INT_INTOCC 0x00000800
/** I2C Last Burst Data Transfer Request
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_LBREQ_INT 0x00000400
/* Nothing
#define ICU0_IM0_IOSR_I2C_LBREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_LBREQ_INT_INTOCC 0x00000400
/** I2C Single Data Transfer Request
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_SREQ_INT 0x00000200
/* Nothing
#define ICU0_IM0_IOSR_I2C_SREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_SREQ_INT_INTOCC 0x00000200
/** I2C Last Single Data Transfer Request
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_I2C_LSREQ_INT 0x00000100
/* Nothing
#define ICU0_IM0_IOSR_I2C_LSREQ_INT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_I2C_LSREQ_INT_INTOCC 0x00000100
/** HOST IF Mailbox1 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_HOST_MB1_TIR 0x00000010
/* Nothing
#define ICU0_IM0_IOSR_HOST_MB1_TIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_HOST_MB1_TIR_INTOCC 0x00000010
/** HOST IF Mailbox1 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_HOST_MB1_RIR 0x00000008
/* Nothing
#define ICU0_IM0_IOSR_HOST_MB1_RIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_HOST_MB1_RIR_INTOCC 0x00000008
/** HOST IF Mailbox0 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_HOST_MB0_TIR 0x00000004
/* Nothing
#define ICU0_IM0_IOSR_HOST_MB0_TIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_HOST_MB0_TIR_INTOCC 0x00000004
/** HOST IF Mailbox0 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_HOST_MB0_RIR 0x00000002
/* Nothing
#define ICU0_IM0_IOSR_HOST_MB0_RIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_HOST_MB0_RIR_INTOCC 0x00000002
/** HOST IF Event Interrupt
    Masked interrupt bit for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IOSR_HOST_EIR 0x00000001
/* Nothing
#define ICU0_IM0_IOSR_HOST_EIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM0_IOSR_HOST_EIR_INTOCC 0x00000001

/* Fields of "IM0 Interrupt Request Set Register" */
/** PCM Transmit Crash Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_PCM_HW2_CRASH 0x80000000
/** PCM Transmit Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_PCM_TX 0x40000000
/** PCM Receive Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_PCM_RX 0x20000000
/** Secure Hash Algorithm Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_SHA1_HASH 0x10000000
/** Advanced Encryption Standard Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_AES_AES 0x08000000
/** SSC Frame Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_SSC0_F 0x00020000
/** SSC Error Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_SSC0_E 0x00010000
/** SSC Receive Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_SSC0_R 0x00008000
/** SSC Transmit Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_SSC0_T 0x00004000
/** I2C Peripheral Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_I2C_P_INT 0x00002000
/** I2C Error Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_I2C_ERR_INT 0x00001000
/** I2C Burst Data Transfer Request
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_BREQ_INT 0x00000800
/** I2C Last Burst Data Transfer Request
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_LBREQ_INT 0x00000400
/** I2C Single Data Transfer Request
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_SREQ_INT 0x00000200
/** I2C Last Single Data Transfer Request
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_I2C_LSREQ_INT 0x00000100
/** HOST IF Mailbox1 Transmit Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_HOST_MB1_TIR 0x00000010
/** HOST IF Mailbox1 Receive Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_HOST_MB1_RIR 0x00000008
/** HOST IF Mailbox0 Transmit Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_HOST_MB0_TIR 0x00000004
/** HOST IF Mailbox0 Receive Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_HOST_MB0_RIR 0x00000002
/** HOST IF Event Interrupt
    Software control for the corresponding bit in the IM0_ISR register. */
#define ICU0_IM0_IRSR_HOST_EIR 0x00000001

/* Fields of "IM0 Interrupt Mode Register" */
/** PCM Transmit Crash Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_PCM_HW2_CRASH 0x80000000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_PCM_HW2_CRASH_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_PCM_HW2_CRASH_DIR 0x80000000
/** PCM Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_PCM_TX 0x40000000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_PCM_TX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_PCM_TX_DIR 0x40000000
/** PCM Receive Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_PCM_RX 0x20000000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_PCM_RX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_PCM_RX_DIR 0x20000000
/** Secure Hash Algorithm Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_SHA1_HASH 0x10000000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_SHA1_HASH_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_SHA1_HASH_DIR 0x10000000
/** Advanced Encryption Standard Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_AES_AES 0x08000000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_AES_AES_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_AES_AES_DIR 0x08000000
/** SSC Frame Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_SSC0_F 0x00020000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_SSC0_F_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_SSC0_F_DIR 0x00020000
/** SSC Error Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_SSC0_E 0x00010000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_SSC0_E_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_SSC0_E_DIR 0x00010000
/** SSC Receive Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_SSC0_R 0x00008000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_SSC0_R_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_SSC0_R_DIR 0x00008000
/** SSC Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_SSC0_T 0x00004000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_SSC0_T_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_SSC0_T_DIR 0x00004000
/** I2C Peripheral Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_I2C_P_INT 0x00002000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_I2C_P_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_I2C_P_INT_DIR 0x00002000
/** I2C Error Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_I2C_ERR_INT 0x00001000
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_I2C_ERR_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_I2C_ERR_INT_DIR 0x00001000
/** I2C Burst Data Transfer Request
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_BREQ_INT 0x00000800
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_BREQ_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_BREQ_INT_DIR 0x00000800
/** I2C Last Burst Data Transfer Request
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_LBREQ_INT 0x00000400
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_LBREQ_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_LBREQ_INT_DIR 0x00000400
/** I2C Single Data Transfer Request
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_SREQ_INT 0x00000200
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_SREQ_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_SREQ_INT_DIR 0x00000200
/** I2C Last Single Data Transfer Request
    Type of interrupt. */
#define ICU0_IM0_IMR_I2C_LSREQ_INT 0x00000100
/* Indirect Interrupt.
#define ICU0_IM0_IMR_I2C_LSREQ_INT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_I2C_LSREQ_INT_DIR 0x00000100
/** HOST IF Mailbox1 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_HOST_MB1_TIR 0x00000010
/* Indirect Interrupt.
#define ICU0_IM0_IMR_HOST_MB1_TIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_HOST_MB1_TIR_DIR 0x00000010
/** HOST IF Mailbox1 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_HOST_MB1_RIR 0x00000008
/* Indirect Interrupt.
#define ICU0_IM0_IMR_HOST_MB1_RIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_HOST_MB1_RIR_DIR 0x00000008
/** HOST IF Mailbox0 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_HOST_MB0_TIR 0x00000004
/* Indirect Interrupt.
#define ICU0_IM0_IMR_HOST_MB0_TIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_HOST_MB0_TIR_DIR 0x00000004
/** HOST IF Mailbox0 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_HOST_MB0_RIR 0x00000002
/* Indirect Interrupt.
#define ICU0_IM0_IMR_HOST_MB0_RIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_HOST_MB0_RIR_DIR 0x00000002
/** HOST IF Event Interrupt
    Type of interrupt. */
#define ICU0_IM0_IMR_HOST_EIR 0x00000001
/* Indirect Interrupt.
#define ICU0_IM0_IMR_HOST_EIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM0_IMR_HOST_EIR_DIR 0x00000001

/* Fields of "IM1 Interrupt Status Register" */
/** Crossbar Error Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_XBAR_ERROR 0x80000000
/* Nothing
#define ICU0_IM1_ISR_XBAR_ERROR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_XBAR_ERROR_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_XBAR_ERROR_INTOCC 0x80000000
/** DDR Controller Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_DDR 0x40000000
/* Nothing
#define ICU0_IM1_ISR_DDR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_DDR_INTACK 0x40000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_DDR_INTOCC 0x40000000
/** FPI Bus Control Unit Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM1_ISR_BCU0 0x20000000
/* Nothing
#define ICU0_IM1_ISR_BCU0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_BCU0_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_BCU0_INTOCC 0x20000000
/** SBIU interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_SBIU0 0x08000000
/* Nothing
#define ICU0_IM1_ISR_SBIU0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_SBIU0_INTACK 0x08000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_SBIU0_INTOCC 0x08000000
/** Watchdog Prewarning Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_WDT_PIR 0x02000000
/* Nothing
#define ICU0_IM1_ISR_WDT_PIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_WDT_PIR_INTACK 0x02000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_WDT_PIR_INTOCC 0x02000000
/** Watchdog Access Error Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_WDT_AEIR 0x01000000
/* Nothing
#define ICU0_IM1_ISR_WDT_AEIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_WDT_AEIR_INTACK 0x01000000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_WDT_AEIR_INTOCC 0x01000000
/** SYS GPE Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_SYS_GPE 0x00200000
/* Nothing
#define ICU0_IM1_ISR_SYS_GPE_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_SYS_GPE_INTACK 0x00200000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_SYS_GPE_INTOCC 0x00200000
/** SYS1 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_SYS1 0x00100000
/* Nothing
#define ICU0_IM1_ISR_SYS1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_SYS1_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_SYS1_INTOCC 0x00100000
/** PMA Interrupt from IntNode of the RX Clk Domain
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_PMA_RX 0x00020000
/* Nothing
#define ICU0_IM1_ISR_PMA_RX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_PMA_RX_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_PMA_RX_INTOCC 0x00020000
/** PMA Interrupt from IntNode of the TX Clk Domain
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_PMA_TX 0x00010000
/* Nothing
#define ICU0_IM1_ISR_PMA_TX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_PMA_TX_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_PMA_TX_INTOCC 0x00010000
/** PMA Interrupt from IntNode of the 200MHz Domain
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_PMA_200M 0x00008000
/* Nothing
#define ICU0_IM1_ISR_PMA_200M_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_PMA_200M_INTACK 0x00008000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_PMA_200M_INTOCC 0x00008000
/** Time of Day
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_TOD 0x00004000
/* Nothing
#define ICU0_IM1_ISR_TOD_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_TOD_INTACK 0x00004000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_TOD_INTOCC 0x00004000
/** 8kHz root interrupt derived from GPON interface
    This bit is a direct interrupt. */
#define ICU0_IM1_ISR_FSC_ROOT 0x00002000
/* Nothing
#define ICU0_IM1_ISR_FSC_ROOT_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_FSC_ROOT_INTACK 0x00002000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_FSC_ROOT_INTOCC 0x00002000
/** FSC Timer Interrupt 1
    Delayed version of FSCROOT. This bit is a direct interrupt. */
#define ICU0_IM1_ISR_FSCT_CMP1 0x00001000
/* Nothing
#define ICU0_IM1_ISR_FSCT_CMP1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_FSCT_CMP1_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_FSCT_CMP1_INTOCC 0x00001000
/** FSC Timer Interrupt 0
    Delayed version of FSCROOT. This bit is a direct interrupt. */
#define ICU0_IM1_ISR_FSCT_CMP0 0x00000800
/* Nothing
#define ICU0_IM1_ISR_FSCT_CMP0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_FSCT_CMP0_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_FSCT_CMP0_INTOCC 0x00000800
/** 8kHz backup interrupt derived from core-PLL
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_FSC_BKP 0x00000400
/* Nothing
#define ICU0_IM1_ISR_FSC_BKP_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_FSC_BKP_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_FSC_BKP_INTOCC 0x00000400
/** External Interrupt from GPIO P4
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_P4 0x00000100
/* Nothing
#define ICU0_IM1_ISR_P4_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_P4_INTACK 0x00000100
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_P4_INTOCC 0x00000100
/** External Interrupt from GPIO P3
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_P3 0x00000080
/* Nothing
#define ICU0_IM1_ISR_P3_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_P3_INTACK 0x00000080
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_P3_INTOCC 0x00000080
/** External Interrupt from GPIO P2
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_P2 0x00000040
/* Nothing
#define ICU0_IM1_ISR_P2_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_P2_INTACK 0x00000040
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_P2_INTOCC 0x00000040
/** External Interrupt from GPIO P1
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_P1 0x00000020
/* Nothing
#define ICU0_IM1_ISR_P1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_P1_INTACK 0x00000020
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_P1_INTOCC 0x00000020
/** External Interrupt from GPIO P0
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_P0 0x00000010
/* Nothing
#define ICU0_IM1_ISR_P0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_P0_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_P0_INTOCC 0x00000010
/** EBU Serial Flash Busy
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_EBU_SF_BUSY 0x00000004
/* Nothing
#define ICU0_IM1_ISR_EBU_SF_BUSY_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_EBU_SF_BUSY_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_EBU_SF_BUSY_INTOCC 0x00000004
/** EBU Serial Flash Command Overwrite Error
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_EBU_SF_COVERR 0x00000002
/* Nothing
#define ICU0_IM1_ISR_EBU_SF_COVERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_EBU_SF_COVERR_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_EBU_SF_COVERR_INTOCC 0x00000002
/** EBU Serial Flash Command Error
    This bit is an indirect interrupt. */
#define ICU0_IM1_ISR_EBU_SF_CMDERR 0x00000001
/* Nothing
#define ICU0_IM1_ISR_EBU_SF_CMDERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM1_ISR_EBU_SF_CMDERR_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define ICU0_IM1_ISR_EBU_SF_CMDERR_INTOCC 0x00000001

/* Fields of "IM1 Interrupt Enable Register" */
/** Crossbar Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_XBAR_ERROR 0x80000000
/* Disable
#define ICU0_IM1_IER_XBAR_ERROR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_XBAR_ERROR_EN 0x80000000
/** DDR Controller Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_DDR 0x40000000
/* Disable
#define ICU0_IM1_IER_DDR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_DDR_EN 0x40000000
/** FPI Bus Control Unit Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_BCU0 0x20000000
/* Disable
#define ICU0_IM1_IER_BCU0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_BCU0_EN 0x20000000
/** SBIU interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_SBIU0 0x08000000
/* Disable
#define ICU0_IM1_IER_SBIU0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_SBIU0_EN 0x08000000
/** Watchdog Prewarning Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_WDT_PIR 0x02000000
/* Disable
#define ICU0_IM1_IER_WDT_PIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_WDT_PIR_EN 0x02000000
/** Watchdog Access Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_WDT_AEIR 0x01000000
/* Disable
#define ICU0_IM1_IER_WDT_AEIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_WDT_AEIR_EN 0x01000000
/** SYS GPE Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_SYS_GPE 0x00200000
/* Disable
#define ICU0_IM1_IER_SYS_GPE_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_SYS_GPE_EN 0x00200000
/** SYS1 Interrupt
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_SYS1 0x00100000
/* Disable
#define ICU0_IM1_IER_SYS1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_SYS1_EN 0x00100000
/** PMA Interrupt from IntNode of the RX Clk Domain
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_PMA_RX 0x00020000
/* Disable
#define ICU0_IM1_IER_PMA_RX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_PMA_RX_EN 0x00020000
/** PMA Interrupt from IntNode of the TX Clk Domain
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_PMA_TX 0x00010000
/* Disable
#define ICU0_IM1_IER_PMA_TX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_PMA_TX_EN 0x00010000
/** PMA Interrupt from IntNode of the 200MHz Domain
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_PMA_200M 0x00008000
/* Disable
#define ICU0_IM1_IER_PMA_200M_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_PMA_200M_EN 0x00008000
/** Time of Day
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_TOD 0x00004000
/* Disable
#define ICU0_IM1_IER_TOD_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_TOD_EN 0x00004000
/** 8kHz root interrupt derived from GPON interface
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_FSC_ROOT 0x00002000
/* Disable
#define ICU0_IM1_IER_FSC_ROOT_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_FSC_ROOT_EN 0x00002000
/** FSC Timer Interrupt 1
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_FSCT_CMP1 0x00001000
/* Disable
#define ICU0_IM1_IER_FSCT_CMP1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_FSCT_CMP1_EN 0x00001000
/** FSC Timer Interrupt 0
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_FSCT_CMP0 0x00000800
/* Disable
#define ICU0_IM1_IER_FSCT_CMP0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_FSCT_CMP0_EN 0x00000800
/** 8kHz backup interrupt derived from core-PLL
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_FSC_BKP 0x00000400
/* Disable
#define ICU0_IM1_IER_FSC_BKP_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_FSC_BKP_EN 0x00000400
/** External Interrupt from GPIO P4
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_P4 0x00000100
/* Disable
#define ICU0_IM1_IER_P4_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_P4_EN 0x00000100
/** External Interrupt from GPIO P3
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_P3 0x00000080
/* Disable
#define ICU0_IM1_IER_P3_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_P3_EN 0x00000080
/** External Interrupt from GPIO P2
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_P2 0x00000040
/* Disable
#define ICU0_IM1_IER_P2_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_P2_EN 0x00000040
/** External Interrupt from GPIO P1
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_P1 0x00000020
/* Disable
#define ICU0_IM1_IER_P1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_P1_EN 0x00000020
/** External Interrupt from GPIO P0
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_P0 0x00000010
/* Disable
#define ICU0_IM1_IER_P0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_P0_EN 0x00000010
/** EBU Serial Flash Busy
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_EBU_SF_BUSY 0x00000004
/* Disable
#define ICU0_IM1_IER_EBU_SF_BUSY_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_EBU_SF_BUSY_EN 0x00000004
/** EBU Serial Flash Command Overwrite Error
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_EBU_SF_COVERR 0x00000002
/* Disable
#define ICU0_IM1_IER_EBU_SF_COVERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_EBU_SF_COVERR_EN 0x00000002
/** EBU Serial Flash Command Error
    Interrupt enable bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IER_EBU_SF_CMDERR 0x00000001
/* Disable
#define ICU0_IM1_IER_EBU_SF_CMDERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM1_IER_EBU_SF_CMDERR_EN 0x00000001

/* Fields of "IM1 Interrupt Output Status Register" */
/** Crossbar Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_XBAR_ERROR 0x80000000
/* Nothing
#define ICU0_IM1_IOSR_XBAR_ERROR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_XBAR_ERROR_INTOCC 0x80000000
/** DDR Controller Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_DDR 0x40000000
/* Nothing
#define ICU0_IM1_IOSR_DDR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_DDR_INTOCC 0x40000000
/** FPI Bus Control Unit Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_BCU0 0x20000000
/* Nothing
#define ICU0_IM1_IOSR_BCU0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_BCU0_INTOCC 0x20000000
/** SBIU interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_SBIU0 0x08000000
/* Nothing
#define ICU0_IM1_IOSR_SBIU0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_SBIU0_INTOCC 0x08000000
/** Watchdog Prewarning Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_WDT_PIR 0x02000000
/* Nothing
#define ICU0_IM1_IOSR_WDT_PIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_WDT_PIR_INTOCC 0x02000000
/** Watchdog Access Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_WDT_AEIR 0x01000000
/* Nothing
#define ICU0_IM1_IOSR_WDT_AEIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_WDT_AEIR_INTOCC 0x01000000
/** SYS GPE Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_SYS_GPE 0x00200000
/* Nothing
#define ICU0_IM1_IOSR_SYS_GPE_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_SYS_GPE_INTOCC 0x00200000
/** SYS1 Interrupt
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_SYS1 0x00100000
/* Nothing
#define ICU0_IM1_IOSR_SYS1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_SYS1_INTOCC 0x00100000
/** PMA Interrupt from IntNode of the RX Clk Domain
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_PMA_RX 0x00020000
/* Nothing
#define ICU0_IM1_IOSR_PMA_RX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_PMA_RX_INTOCC 0x00020000
/** PMA Interrupt from IntNode of the TX Clk Domain
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_PMA_TX 0x00010000
/* Nothing
#define ICU0_IM1_IOSR_PMA_TX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_PMA_TX_INTOCC 0x00010000
/** PMA Interrupt from IntNode of the 200MHz Domain
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_PMA_200M 0x00008000
/* Nothing
#define ICU0_IM1_IOSR_PMA_200M_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_PMA_200M_INTOCC 0x00008000
/** Time of Day
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_TOD 0x00004000
/* Nothing
#define ICU0_IM1_IOSR_TOD_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_TOD_INTOCC 0x00004000
/** 8kHz root interrupt derived from GPON interface
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_FSC_ROOT 0x00002000
/* Nothing
#define ICU0_IM1_IOSR_FSC_ROOT_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_FSC_ROOT_INTOCC 0x00002000
/** FSC Timer Interrupt 1
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_FSCT_CMP1 0x00001000
/* Nothing
#define ICU0_IM1_IOSR_FSCT_CMP1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_FSCT_CMP1_INTOCC 0x00001000
/** FSC Timer Interrupt 0
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_FSCT_CMP0 0x00000800
/* Nothing
#define ICU0_IM1_IOSR_FSCT_CMP0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_FSCT_CMP0_INTOCC 0x00000800
/** 8kHz backup interrupt derived from core-PLL
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_FSC_BKP 0x00000400
/* Nothing
#define ICU0_IM1_IOSR_FSC_BKP_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_FSC_BKP_INTOCC 0x00000400
/** External Interrupt from GPIO P4
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_P4 0x00000100
/* Nothing
#define ICU0_IM1_IOSR_P4_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_P4_INTOCC 0x00000100
/** External Interrupt from GPIO P3
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_P3 0x00000080
/* Nothing
#define ICU0_IM1_IOSR_P3_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_P3_INTOCC 0x00000080
/** External Interrupt from GPIO P2
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_P2 0x00000040
/* Nothing
#define ICU0_IM1_IOSR_P2_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_P2_INTOCC 0x00000040
/** External Interrupt from GPIO P1
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_P1 0x00000020
/* Nothing
#define ICU0_IM1_IOSR_P1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_P1_INTOCC 0x00000020
/** External Interrupt from GPIO P0
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_P0 0x00000010
/* Nothing
#define ICU0_IM1_IOSR_P0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_P0_INTOCC 0x00000010
/** EBU Serial Flash Busy
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_EBU_SF_BUSY 0x00000004
/* Nothing
#define ICU0_IM1_IOSR_EBU_SF_BUSY_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_EBU_SF_BUSY_INTOCC 0x00000004
/** EBU Serial Flash Command Overwrite Error
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_EBU_SF_COVERR 0x00000002
/* Nothing
#define ICU0_IM1_IOSR_EBU_SF_COVERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_EBU_SF_COVERR_INTOCC 0x00000002
/** EBU Serial Flash Command Error
    Masked interrupt bit for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IOSR_EBU_SF_CMDERR 0x00000001
/* Nothing
#define ICU0_IM1_IOSR_EBU_SF_CMDERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM1_IOSR_EBU_SF_CMDERR_INTOCC 0x00000001

/* Fields of "IM1 Interrupt Request Set Register" */
/** Crossbar Error Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_XBAR_ERROR 0x80000000
/** DDR Controller Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_DDR 0x40000000
/** FPI Bus Control Unit Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_BCU0 0x20000000
/** SBIU interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_SBIU0 0x08000000
/** Watchdog Prewarning Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_WDT_PIR 0x02000000
/** Watchdog Access Error Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_WDT_AEIR 0x01000000
/** SYS GPE Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_SYS_GPE 0x00200000
/** SYS1 Interrupt
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_SYS1 0x00100000
/** PMA Interrupt from IntNode of the RX Clk Domain
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_PMA_RX 0x00020000
/** PMA Interrupt from IntNode of the TX Clk Domain
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_PMA_TX 0x00010000
/** PMA Interrupt from IntNode of the 200MHz Domain
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_PMA_200M 0x00008000
/** Time of Day
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_TOD 0x00004000
/** 8kHz root interrupt derived from GPON interface
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_FSC_ROOT 0x00002000
/** FSC Timer Interrupt 1
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_FSCT_CMP1 0x00001000
/** FSC Timer Interrupt 0
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_FSCT_CMP0 0x00000800
/** 8kHz backup interrupt derived from core-PLL
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_FSC_BKP 0x00000400
/** External Interrupt from GPIO P4
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_P4 0x00000100
/** External Interrupt from GPIO P3
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_P3 0x00000080
/** External Interrupt from GPIO P2
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_P2 0x00000040
/** External Interrupt from GPIO P1
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_P1 0x00000020
/** External Interrupt from GPIO P0
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_P0 0x00000010
/** EBU Serial Flash Busy
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_EBU_SF_BUSY 0x00000004
/** EBU Serial Flash Command Overwrite Error
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_EBU_SF_COVERR 0x00000002
/** EBU Serial Flash Command Error
    Software control for the corresponding bit in the IM1_ISR register. */
#define ICU0_IM1_IRSR_EBU_SF_CMDERR 0x00000001

/* Fields of "IM1 Interrupt Mode Register" */
/** Crossbar Error Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_XBAR_ERROR 0x80000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_XBAR_ERROR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_XBAR_ERROR_DIR 0x80000000
/** DDR Controller Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_DDR 0x40000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_DDR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_DDR_DIR 0x40000000
/** FPI Bus Control Unit Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_BCU0 0x20000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_BCU0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_BCU0_DIR 0x20000000
/** SBIU interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_SBIU0 0x08000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_SBIU0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_SBIU0_DIR 0x08000000
/** Watchdog Prewarning Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_WDT_PIR 0x02000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_WDT_PIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_WDT_PIR_DIR 0x02000000
/** Watchdog Access Error Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_WDT_AEIR 0x01000000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_WDT_AEIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_WDT_AEIR_DIR 0x01000000
/** SYS GPE Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_SYS_GPE 0x00200000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_SYS_GPE_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_SYS_GPE_DIR 0x00200000
/** SYS1 Interrupt
    Type of interrupt. */
#define ICU0_IM1_IMR_SYS1 0x00100000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_SYS1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_SYS1_DIR 0x00100000
/** PMA Interrupt from IntNode of the RX Clk Domain
    Type of interrupt. */
#define ICU0_IM1_IMR_PMA_RX 0x00020000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_PMA_RX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_PMA_RX_DIR 0x00020000
/** PMA Interrupt from IntNode of the TX Clk Domain
    Type of interrupt. */
#define ICU0_IM1_IMR_PMA_TX 0x00010000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_PMA_TX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_PMA_TX_DIR 0x00010000
/** PMA Interrupt from IntNode of the 200MHz Domain
    Type of interrupt. */
#define ICU0_IM1_IMR_PMA_200M 0x00008000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_PMA_200M_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_PMA_200M_DIR 0x00008000
/** Time of Day
    Type of interrupt. */
#define ICU0_IM1_IMR_TOD 0x00004000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_TOD_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_TOD_DIR 0x00004000
/** 8kHz root interrupt derived from GPON interface
    Type of interrupt. */
#define ICU0_IM1_IMR_FSC_ROOT 0x00002000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_FSC_ROOT_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_FSC_ROOT_DIR 0x00002000
/** FSC Timer Interrupt 1
    Type of interrupt. */
#define ICU0_IM1_IMR_FSCT_CMP1 0x00001000
/* Indirect Interrupt.
#define ICU0_IM1_IMR_FSCT_CMP1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_FSCT_CMP1_DIR 0x00001000
/** FSC Timer Interrupt 0
    Type of interrupt. */
#define ICU0_IM1_IMR_FSCT_CMP0 0x00000800
/* Indirect Interrupt.
#define ICU0_IM1_IMR_FSCT_CMP0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_FSCT_CMP0_DIR 0x00000800
/** 8kHz backup interrupt derived from core-PLL
    Type of interrupt. */
#define ICU0_IM1_IMR_FSC_BKP 0x00000400
/* Indirect Interrupt.
#define ICU0_IM1_IMR_FSC_BKP_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_FSC_BKP_DIR 0x00000400
/** External Interrupt from GPIO P4
    Type of interrupt. */
#define ICU0_IM1_IMR_P4 0x00000100
/* Indirect Interrupt.
#define ICU0_IM1_IMR_P4_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_P4_DIR 0x00000100
/** External Interrupt from GPIO P3
    Type of interrupt. */
#define ICU0_IM1_IMR_P3 0x00000080
/* Indirect Interrupt.
#define ICU0_IM1_IMR_P3_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_P3_DIR 0x00000080
/** External Interrupt from GPIO P2
    Type of interrupt. */
#define ICU0_IM1_IMR_P2 0x00000040
/* Indirect Interrupt.
#define ICU0_IM1_IMR_P2_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_P2_DIR 0x00000040
/** External Interrupt from GPIO P1
    Type of interrupt. */
#define ICU0_IM1_IMR_P1 0x00000020
/* Indirect Interrupt.
#define ICU0_IM1_IMR_P1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_P1_DIR 0x00000020
/** External Interrupt from GPIO P0
    Type of interrupt. */
#define ICU0_IM1_IMR_P0 0x00000010
/* Indirect Interrupt.
#define ICU0_IM1_IMR_P0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_P0_DIR 0x00000010
/** EBU Serial Flash Busy
    Type of interrupt. */
#define ICU0_IM1_IMR_EBU_SF_BUSY 0x00000004
/* Indirect Interrupt.
#define ICU0_IM1_IMR_EBU_SF_BUSY_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_EBU_SF_BUSY_DIR 0x00000004
/** EBU Serial Flash Command Overwrite Error
    Type of interrupt. */
#define ICU0_IM1_IMR_EBU_SF_COVERR 0x00000002
/* Indirect Interrupt.
#define ICU0_IM1_IMR_EBU_SF_COVERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_EBU_SF_COVERR_DIR 0x00000002
/** EBU Serial Flash Command Error
    Type of interrupt. */
#define ICU0_IM1_IMR_EBU_SF_CMDERR 0x00000001
/* Indirect Interrupt.
#define ICU0_IM1_IMR_EBU_SF_CMDERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM1_IMR_EBU_SF_CMDERR_DIR 0x00000001

/* Fields of "IM2 Interrupt Status Register" */
/** EIM Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_EIM 0x80000000
/* Nothing
#define ICU0_IM2_ISR_EIM_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_EIM_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_EIM_INTOCC 0x80000000
/** GTC Upstream Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_GTC_US 0x40000000
/* Nothing
#define ICU0_IM2_ISR_GTC_US_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_GTC_US_INTACK 0x40000000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_GTC_US_INTOCC 0x40000000
/** GTC Downstream Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_GTC_DS 0x20000000
/* Nothing
#define ICU0_IM2_ISR_GTC_DS_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_GTC_DS_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_GTC_DS_INTOCC 0x20000000
/** TBM Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_TBM 0x00400000
/* Nothing
#define ICU0_IM2_ISR_TBM_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_TBM_INTACK 0x00400000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_TBM_INTOCC 0x00400000
/** Dispatcher Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_DISP 0x00200000
/* Nothing
#define ICU0_IM2_ISR_DISP_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_DISP_INTACK 0x00200000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_DISP_INTOCC 0x00200000
/** CONFIG Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_CONFIG 0x00100000
/* Nothing
#define ICU0_IM2_ISR_CONFIG_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_CONFIG_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_CONFIG_INTOCC 0x00100000
/** CONFIG Break Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_CONFIG_BREAK 0x00080000
/* Nothing
#define ICU0_IM2_ISR_CONFIG_BREAK_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_CONFIG_BREAK_INTACK 0x00080000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_CONFIG_BREAK_INTOCC 0x00080000
/** OCTRLC Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLC 0x00040000
/* Nothing
#define ICU0_IM2_ISR_OCTRLC_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLC_INTACK 0x00040000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLC_INTOCC 0x00040000
/** ICTRLC 1 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLC1 0x00020000
/* Nothing
#define ICU0_IM2_ISR_ICTRLC1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLC1_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLC1_INTOCC 0x00020000
/** ICTRLC 0 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLC0 0x00010000
/* Nothing
#define ICU0_IM2_ISR_ICTRLC0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLC0_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLC0_INTOCC 0x00010000
/** LINK 1 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_LINK1 0x00004000
/* Nothing
#define ICU0_IM2_ISR_LINK1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_LINK1_INTACK 0x00004000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_LINK1_INTOCC 0x00004000
/** TMU Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_TMU 0x00001000
/* Nothing
#define ICU0_IM2_ISR_TMU_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_TMU_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_TMU_INTOCC 0x00001000
/** FSQM Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_FSQM 0x00000800
/* Nothing
#define ICU0_IM2_ISR_FSQM_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_FSQM_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_FSQM_INTOCC 0x00000800
/** IQM Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_IQM 0x00000400
/* Nothing
#define ICU0_IM2_ISR_IQM_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_IQM_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_IQM_INTOCC 0x00000400
/** OCTRLG Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLG 0x00000200
/* Nothing
#define ICU0_IM2_ISR_OCTRLG_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLG_INTACK 0x00000200
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLG_INTOCC 0x00000200
/** OCTRLL 3 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLL3 0x00000080
/* Nothing
#define ICU0_IM2_ISR_OCTRLL3_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLL3_INTACK 0x00000080
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLL3_INTOCC 0x00000080
/** OCTRLL 2 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLL2 0x00000040
/* Nothing
#define ICU0_IM2_ISR_OCTRLL2_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLL2_INTACK 0x00000040
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLL2_INTOCC 0x00000040
/** OCTRLL 1 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLL1 0x00000020
/* Nothing
#define ICU0_IM2_ISR_OCTRLL1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLL1_INTACK 0x00000020
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLL1_INTOCC 0x00000020
/** OCTRLL 0 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_OCTRLL0 0x00000010
/* Nothing
#define ICU0_IM2_ISR_OCTRLL0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_OCTRLL0_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_OCTRLL0_INTOCC 0x00000010
/** ICTRLL 3 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLL3 0x00000008
/* Nothing
#define ICU0_IM2_ISR_ICTRLL3_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLL3_INTACK 0x00000008
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLL3_INTOCC 0x00000008
/** ICTRLL 2 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLL2 0x00000004
/* Nothing
#define ICU0_IM2_ISR_ICTRLL2_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLL2_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLL2_INTOCC 0x00000004
/** ICTRLL 1 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLL1 0x00000002
/* Nothing
#define ICU0_IM2_ISR_ICTRLL1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLL1_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLL1_INTOCC 0x00000002
/** ICTRLL 0 Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM2_ISR_ICTRLL0 0x00000001
/* Nothing
#define ICU0_IM2_ISR_ICTRLL0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM2_ISR_ICTRLL0_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define ICU0_IM2_ISR_ICTRLL0_INTOCC 0x00000001

/* Fields of "IM2 Interrupt Enable Register" */
/** EIM Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_EIM 0x80000000
/* Disable
#define ICU0_IM2_IER_EIM_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_EIM_EN 0x80000000
/** GTC Upstream Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_GTC_US 0x40000000
/* Disable
#define ICU0_IM2_IER_GTC_US_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_GTC_US_EN 0x40000000
/** GTC Downstream Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_GTC_DS 0x20000000
/* Disable
#define ICU0_IM2_IER_GTC_DS_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_GTC_DS_EN 0x20000000
/** TBM Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_TBM 0x00400000
/* Disable
#define ICU0_IM2_IER_TBM_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_TBM_EN 0x00400000
/** Dispatcher Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_DISP 0x00200000
/* Disable
#define ICU0_IM2_IER_DISP_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_DISP_EN 0x00200000
/** CONFIG Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_CONFIG 0x00100000
/* Disable
#define ICU0_IM2_IER_CONFIG_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_CONFIG_EN 0x00100000
/** CONFIG Break Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_CONFIG_BREAK 0x00080000
/* Disable
#define ICU0_IM2_IER_CONFIG_BREAK_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_CONFIG_BREAK_EN 0x00080000
/** OCTRLC Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLC 0x00040000
/* Disable
#define ICU0_IM2_IER_OCTRLC_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLC_EN 0x00040000
/** ICTRLC 1 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLC1 0x00020000
/* Disable
#define ICU0_IM2_IER_ICTRLC1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLC1_EN 0x00020000
/** ICTRLC 0 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLC0 0x00010000
/* Disable
#define ICU0_IM2_IER_ICTRLC0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLC0_EN 0x00010000
/** LINK 1 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_LINK1 0x00004000
/* Disable
#define ICU0_IM2_IER_LINK1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_LINK1_EN 0x00004000
/** TMU Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_TMU 0x00001000
/* Disable
#define ICU0_IM2_IER_TMU_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_TMU_EN 0x00001000
/** FSQM Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_FSQM 0x00000800
/* Disable
#define ICU0_IM2_IER_FSQM_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_FSQM_EN 0x00000800
/** IQM Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_IQM 0x00000400
/* Disable
#define ICU0_IM2_IER_IQM_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_IQM_EN 0x00000400
/** OCTRLG Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLG 0x00000200
/* Disable
#define ICU0_IM2_IER_OCTRLG_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLG_EN 0x00000200
/** OCTRLL 3 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLL3 0x00000080
/* Disable
#define ICU0_IM2_IER_OCTRLL3_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLL3_EN 0x00000080
/** OCTRLL 2 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLL2 0x00000040
/* Disable
#define ICU0_IM2_IER_OCTRLL2_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLL2_EN 0x00000040
/** OCTRLL 1 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLL1 0x00000020
/* Disable
#define ICU0_IM2_IER_OCTRLL1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLL1_EN 0x00000020
/** OCTRLL 0 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_OCTRLL0 0x00000010
/* Disable
#define ICU0_IM2_IER_OCTRLL0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_OCTRLL0_EN 0x00000010
/** ICTRLL 3 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLL3 0x00000008
/* Disable
#define ICU0_IM2_IER_ICTRLL3_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLL3_EN 0x00000008
/** ICTRLL 2 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLL2 0x00000004
/* Disable
#define ICU0_IM2_IER_ICTRLL2_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLL2_EN 0x00000004
/** ICTRLL 1 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLL1 0x00000002
/* Disable
#define ICU0_IM2_IER_ICTRLL1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLL1_EN 0x00000002
/** ICTRLL 0 Interrupt
    Interrupt enable bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IER_ICTRLL0 0x00000001
/* Disable
#define ICU0_IM2_IER_ICTRLL0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM2_IER_ICTRLL0_EN 0x00000001

/* Fields of "IM2 Interrupt Output Status Register" */
/** EIM Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_EIM 0x80000000
/* Nothing
#define ICU0_IM2_IOSR_EIM_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_EIM_INTOCC 0x80000000
/** GTC Upstream Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_GTC_US 0x40000000
/* Nothing
#define ICU0_IM2_IOSR_GTC_US_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_GTC_US_INTOCC 0x40000000
/** GTC Downstream Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_GTC_DS 0x20000000
/* Nothing
#define ICU0_IM2_IOSR_GTC_DS_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_GTC_DS_INTOCC 0x20000000
/** TBM Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_TBM 0x00400000
/* Nothing
#define ICU0_IM2_IOSR_TBM_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_TBM_INTOCC 0x00400000
/** Dispatcher Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_DISP 0x00200000
/* Nothing
#define ICU0_IM2_IOSR_DISP_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_DISP_INTOCC 0x00200000
/** CONFIG Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_CONFIG 0x00100000
/* Nothing
#define ICU0_IM2_IOSR_CONFIG_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_CONFIG_INTOCC 0x00100000
/** CONFIG Break Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_CONFIG_BREAK 0x00080000
/* Nothing
#define ICU0_IM2_IOSR_CONFIG_BREAK_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_CONFIG_BREAK_INTOCC 0x00080000
/** OCTRLC Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLC 0x00040000
/* Nothing
#define ICU0_IM2_IOSR_OCTRLC_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLC_INTOCC 0x00040000
/** ICTRLC 1 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLC1 0x00020000
/* Nothing
#define ICU0_IM2_IOSR_ICTRLC1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLC1_INTOCC 0x00020000
/** ICTRLC 0 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLC0 0x00010000
/* Nothing
#define ICU0_IM2_IOSR_ICTRLC0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLC0_INTOCC 0x00010000
/** LINK 1 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_LINK1 0x00004000
/* Nothing
#define ICU0_IM2_IOSR_LINK1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_LINK1_INTOCC 0x00004000
/** TMU Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_TMU 0x00001000
/* Nothing
#define ICU0_IM2_IOSR_TMU_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_TMU_INTOCC 0x00001000
/** FSQM Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_FSQM 0x00000800
/* Nothing
#define ICU0_IM2_IOSR_FSQM_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_FSQM_INTOCC 0x00000800
/** IQM Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_IQM 0x00000400
/* Nothing
#define ICU0_IM2_IOSR_IQM_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_IQM_INTOCC 0x00000400
/** OCTRLG Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLG 0x00000200
/* Nothing
#define ICU0_IM2_IOSR_OCTRLG_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLG_INTOCC 0x00000200
/** OCTRLL 3 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLL3 0x00000080
/* Nothing
#define ICU0_IM2_IOSR_OCTRLL3_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLL3_INTOCC 0x00000080
/** OCTRLL 2 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLL2 0x00000040
/* Nothing
#define ICU0_IM2_IOSR_OCTRLL2_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLL2_INTOCC 0x00000040
/** OCTRLL 1 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLL1 0x00000020
/* Nothing
#define ICU0_IM2_IOSR_OCTRLL1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLL1_INTOCC 0x00000020
/** OCTRLL 0 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_OCTRLL0 0x00000010
/* Nothing
#define ICU0_IM2_IOSR_OCTRLL0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_OCTRLL0_INTOCC 0x00000010
/** ICTRLL 3 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLL3 0x00000008
/* Nothing
#define ICU0_IM2_IOSR_ICTRLL3_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLL3_INTOCC 0x00000008
/** ICTRLL 2 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLL2 0x00000004
/* Nothing
#define ICU0_IM2_IOSR_ICTRLL2_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLL2_INTOCC 0x00000004
/** ICTRLL 1 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLL1 0x00000002
/* Nothing
#define ICU0_IM2_IOSR_ICTRLL1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLL1_INTOCC 0x00000002
/** ICTRLL 0 Interrupt
    Masked interrupt bit for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IOSR_ICTRLL0 0x00000001
/* Nothing
#define ICU0_IM2_IOSR_ICTRLL0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM2_IOSR_ICTRLL0_INTOCC 0x00000001

/* Fields of "IM2 Interrupt Request Set Register" */
/** EIM Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_EIM 0x80000000
/** GTC Upstream Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_GTC_US 0x40000000
/** GTC Downstream Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_GTC_DS 0x20000000
/** TBM Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_TBM 0x00400000
/** Dispatcher Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_DISP 0x00200000
/** CONFIG Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_CONFIG 0x00100000
/** CONFIG Break Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_CONFIG_BREAK 0x00080000
/** OCTRLC Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLC 0x00040000
/** ICTRLC 1 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLC1 0x00020000
/** ICTRLC 0 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLC0 0x00010000
/** LINK 1 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_LINK1 0x00004000
/** TMU Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_TMU 0x00001000
/** FSQM Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_FSQM 0x00000800
/** IQM Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_IQM 0x00000400
/** OCTRLG Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLG 0x00000200
/** OCTRLL 3 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLL3 0x00000080
/** OCTRLL 2 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLL2 0x00000040
/** OCTRLL 1 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLL1 0x00000020
/** OCTRLL 0 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_OCTRLL0 0x00000010
/** ICTRLL 3 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLL3 0x00000008
/** ICTRLL 2 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLL2 0x00000004
/** ICTRLL 1 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLL1 0x00000002
/** ICTRLL 0 Interrupt
    Software control for the corresponding bit in the IM2_ISR register. */
#define ICU0_IM2_IRSR_ICTRLL0 0x00000001

/* Fields of "IM2 Interrupt Mode Register" */
/** EIM Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_EIM 0x80000000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_EIM_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_EIM_DIR 0x80000000
/** GTC Upstream Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_GTC_US 0x40000000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_GTC_US_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_GTC_US_DIR 0x40000000
/** GTC Downstream Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_GTC_DS 0x20000000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_GTC_DS_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_GTC_DS_DIR 0x20000000
/** TBM Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_TBM 0x00400000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_TBM_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_TBM_DIR 0x00400000
/** Dispatcher Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_DISP 0x00200000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_DISP_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_DISP_DIR 0x00200000
/** CONFIG Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_CONFIG 0x00100000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_CONFIG_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_CONFIG_DIR 0x00100000
/** CONFIG Break Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_CONFIG_BREAK 0x00080000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_CONFIG_BREAK_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_CONFIG_BREAK_DIR 0x00080000
/** OCTRLC Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLC 0x00040000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLC_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLC_DIR 0x00040000
/** ICTRLC 1 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLC1 0x00020000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLC1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLC1_DIR 0x00020000
/** ICTRLC 0 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLC0 0x00010000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLC0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLC0_DIR 0x00010000
/** LINK 1 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_LINK1 0x00004000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_LINK1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_LINK1_DIR 0x00004000
/** TMU Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_TMU 0x00001000
/* Indirect Interrupt.
#define ICU0_IM2_IMR_TMU_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_TMU_DIR 0x00001000
/** FSQM Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_FSQM 0x00000800
/* Indirect Interrupt.
#define ICU0_IM2_IMR_FSQM_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_FSQM_DIR 0x00000800
/** IQM Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_IQM 0x00000400
/* Indirect Interrupt.
#define ICU0_IM2_IMR_IQM_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_IQM_DIR 0x00000400
/** OCTRLG Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLG 0x00000200
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLG_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLG_DIR 0x00000200
/** OCTRLL 3 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLL3 0x00000080
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLL3_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLL3_DIR 0x00000080
/** OCTRLL 2 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLL2 0x00000040
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLL2_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLL2_DIR 0x00000040
/** OCTRLL 1 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLL1 0x00000020
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLL1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLL1_DIR 0x00000020
/** OCTRLL 0 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_OCTRLL0 0x00000010
/* Indirect Interrupt.
#define ICU0_IM2_IMR_OCTRLL0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_OCTRLL0_DIR 0x00000010
/** ICTRLL 3 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLL3 0x00000008
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLL3_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLL3_DIR 0x00000008
/** ICTRLL 2 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLL2 0x00000004
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLL2_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLL2_DIR 0x00000004
/** ICTRLL 1 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLL1 0x00000002
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLL1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLL1_DIR 0x00000002
/** ICTRLL 0 Interrupt
    Type of interrupt. */
#define ICU0_IM2_IMR_ICTRLL0 0x00000001
/* Indirect Interrupt.
#define ICU0_IM2_IMR_ICTRLL0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM2_IMR_ICTRLL0_DIR 0x00000001

/* Fields of "IM3 Interrupt Status Register" */
/** DFEV0, Channel 0 General Purpose Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_1GP 0x80000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_1GP_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_1GP_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_1GP_INTOCC 0x80000000
/** DFEV0, Channel 0 Receive Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_1RX 0x40000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_1RX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_1RX_INTACK 0x40000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_1RX_INTOCC 0x40000000
/** DFEV0, Channel 0 Transmit Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_1TX 0x20000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_1TX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_1TX_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_1TX_INTOCC 0x20000000
/** DFEV0, Channel 1 General Purpose Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_2GP 0x10000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_2GP_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_2GP_INTACK 0x10000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_2GP_INTOCC 0x10000000
/** DFEV0, Channel 1 Receive Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_2RX 0x08000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_2RX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_2RX_INTACK 0x08000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_2RX_INTOCC 0x08000000
/** DFEV0, Channel 1 Transmit Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM3_ISR_DFEV0_2TX 0x04000000
/* Nothing
#define ICU0_IM3_ISR_DFEV0_2TX_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_DFEV0_2TX_INTACK 0x04000000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_DFEV0_2TX_INTOCC 0x04000000
/** GPTC Timer/Counter 3B Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC3B 0x00200000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC3B_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC3B_INTACK 0x00200000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC3B_INTOCC 0x00200000
/** GPTC Timer/Counter 3A Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC3A 0x00100000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC3A_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC3A_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC3A_INTOCC 0x00100000
/** GPTC Timer/Counter 2B Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC2B 0x00080000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC2B_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC2B_INTACK 0x00080000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC2B_INTOCC 0x00080000
/** GPTC Timer/Counter 2A Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC2A 0x00040000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC2A_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC2A_INTACK 0x00040000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC2A_INTOCC 0x00040000
/** GPTC Timer/Counter 1B Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC1B 0x00020000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC1B_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC1B_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC1B_INTOCC 0x00020000
/** GPTC Timer/Counter 1A Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_GPTC_TC1A 0x00010000
/* Nothing
#define ICU0_IM3_ISR_GPTC_TC1A_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_GPTC_TC1A_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_GPTC_TC1A_INTOCC 0x00010000
/** ASC1 Soft Flow Control Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_SFC 0x00008000
/* Nothing
#define ICU0_IM3_ISR_ASC1_SFC_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_SFC_INTACK 0x00008000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_SFC_INTOCC 0x00008000
/** ASC1 Modem Status Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_MS 0x00004000
/* Nothing
#define ICU0_IM3_ISR_ASC1_MS_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_MS_INTACK 0x00004000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_MS_INTOCC 0x00004000
/** ASC1 Autobaud Detection Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_ABDET 0x00002000
/* Nothing
#define ICU0_IM3_ISR_ASC1_ABDET_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_ABDET_INTACK 0x00002000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_ABDET_INTOCC 0x00002000
/** ASC1 Autobaud Start Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_ABST 0x00001000
/* Nothing
#define ICU0_IM3_ISR_ASC1_ABST_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_ABST_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_ABST_INTOCC 0x00001000
/** ASC1 Transmit Buffer Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_TB 0x00000800
/* Nothing
#define ICU0_IM3_ISR_ASC1_TB_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_TB_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_TB_INTOCC 0x00000800
/** ASC1 Error Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_E 0x00000400
/* Nothing
#define ICU0_IM3_ISR_ASC1_E_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_E_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_E_INTOCC 0x00000400
/** ASC1 Receive Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_R 0x00000200
/* Nothing
#define ICU0_IM3_ISR_ASC1_R_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_R_INTACK 0x00000200
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_R_INTOCC 0x00000200
/** ASC1 Transmit Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC1_T 0x00000100
/* Nothing
#define ICU0_IM3_ISR_ASC1_T_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC1_T_INTACK 0x00000100
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC1_T_INTOCC 0x00000100
/** ASC0 Soft Flow Control Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_SFC 0x00000080
/* Nothing
#define ICU0_IM3_ISR_ASC0_SFC_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_SFC_INTACK 0x00000080
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_SFC_INTOCC 0x00000080
/** ASC1 Modem Status Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_MS 0x00000040
/* Nothing
#define ICU0_IM3_ISR_ASC0_MS_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_MS_INTACK 0x00000040
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_MS_INTOCC 0x00000040
/** ASC0 Autobaud Detection Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_ABDET 0x00000020
/* Nothing
#define ICU0_IM3_ISR_ASC0_ABDET_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_ABDET_INTACK 0x00000020
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_ABDET_INTOCC 0x00000020
/** ASC0 Autobaud Start Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_ABST 0x00000010
/* Nothing
#define ICU0_IM3_ISR_ASC0_ABST_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_ABST_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_ABST_INTOCC 0x00000010
/** ASC0 Transmit Buffer Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_TB 0x00000008
/* Nothing
#define ICU0_IM3_ISR_ASC0_TB_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_TB_INTACK 0x00000008
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_TB_INTOCC 0x00000008
/** ASC0 Error Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_E 0x00000004
/* Nothing
#define ICU0_IM3_ISR_ASC0_E_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_E_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_E_INTOCC 0x00000004
/** ASC0 Receive Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_R 0x00000002
/* Nothing
#define ICU0_IM3_ISR_ASC0_R_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_R_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_R_INTOCC 0x00000002
/** ASC0 Transmit Interrupt
    This bit is a direct interrupt. */
#define ICU0_IM3_ISR_ASC0_T 0x00000001
/* Nothing
#define ICU0_IM3_ISR_ASC0_T_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM3_ISR_ASC0_T_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define ICU0_IM3_ISR_ASC0_T_INTOCC 0x00000001

/* Fields of "IM3 Interrupt Enable Register" */
/** DFEV0, Channel 0 General Purpose Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_1GP 0x80000000
/* Disable
#define ICU0_IM3_IER_DFEV0_1GP_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_1GP_EN 0x80000000
/** DFEV0, Channel 0 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_1RX 0x40000000
/* Disable
#define ICU0_IM3_IER_DFEV0_1RX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_1RX_EN 0x40000000
/** DFEV0, Channel 0 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_1TX 0x20000000
/* Disable
#define ICU0_IM3_IER_DFEV0_1TX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_1TX_EN 0x20000000
/** DFEV0, Channel 1 General Purpose Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_2GP 0x10000000
/* Disable
#define ICU0_IM3_IER_DFEV0_2GP_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_2GP_EN 0x10000000
/** DFEV0, Channel 1 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_2RX 0x08000000
/* Disable
#define ICU0_IM3_IER_DFEV0_2RX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_2RX_EN 0x08000000
/** DFEV0, Channel 1 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_DFEV0_2TX 0x04000000
/* Disable
#define ICU0_IM3_IER_DFEV0_2TX_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_DFEV0_2TX_EN 0x04000000
/** GPTC Timer/Counter 3B Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC3B 0x00200000
/* Disable
#define ICU0_IM3_IER_GPTC_TC3B_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC3B_EN 0x00200000
/** GPTC Timer/Counter 3A Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC3A 0x00100000
/* Disable
#define ICU0_IM3_IER_GPTC_TC3A_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC3A_EN 0x00100000
/** GPTC Timer/Counter 2B Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC2B 0x00080000
/* Disable
#define ICU0_IM3_IER_GPTC_TC2B_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC2B_EN 0x00080000
/** GPTC Timer/Counter 2A Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC2A 0x00040000
/* Disable
#define ICU0_IM3_IER_GPTC_TC2A_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC2A_EN 0x00040000
/** GPTC Timer/Counter 1B Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC1B 0x00020000
/* Disable
#define ICU0_IM3_IER_GPTC_TC1B_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC1B_EN 0x00020000
/** GPTC Timer/Counter 1A Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_GPTC_TC1A 0x00010000
/* Disable
#define ICU0_IM3_IER_GPTC_TC1A_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_GPTC_TC1A_EN 0x00010000
/** ASC1 Soft Flow Control Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_SFC 0x00008000
/* Disable
#define ICU0_IM3_IER_ASC1_SFC_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_SFC_EN 0x00008000
/** ASC1 Modem Status Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_MS 0x00004000
/* Disable
#define ICU0_IM3_IER_ASC1_MS_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_MS_EN 0x00004000
/** ASC1 Autobaud Detection Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_ABDET 0x00002000
/* Disable
#define ICU0_IM3_IER_ASC1_ABDET_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_ABDET_EN 0x00002000
/** ASC1 Autobaud Start Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_ABST 0x00001000
/* Disable
#define ICU0_IM3_IER_ASC1_ABST_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_ABST_EN 0x00001000
/** ASC1 Transmit Buffer Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_TB 0x00000800
/* Disable
#define ICU0_IM3_IER_ASC1_TB_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_TB_EN 0x00000800
/** ASC1 Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_E 0x00000400
/* Disable
#define ICU0_IM3_IER_ASC1_E_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_E_EN 0x00000400
/** ASC1 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_R 0x00000200
/* Disable
#define ICU0_IM3_IER_ASC1_R_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_R_EN 0x00000200
/** ASC1 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC1_T 0x00000100
/* Disable
#define ICU0_IM3_IER_ASC1_T_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC1_T_EN 0x00000100
/** ASC0 Soft Flow Control Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_SFC 0x00000080
/* Disable
#define ICU0_IM3_IER_ASC0_SFC_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_SFC_EN 0x00000080
/** ASC1 Modem Status Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_MS 0x00000040
/* Disable
#define ICU0_IM3_IER_ASC0_MS_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_MS_EN 0x00000040
/** ASC0 Autobaud Detection Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_ABDET 0x00000020
/* Disable
#define ICU0_IM3_IER_ASC0_ABDET_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_ABDET_EN 0x00000020
/** ASC0 Autobaud Start Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_ABST 0x00000010
/* Disable
#define ICU0_IM3_IER_ASC0_ABST_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_ABST_EN 0x00000010
/** ASC0 Transmit Buffer Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_TB 0x00000008
/* Disable
#define ICU0_IM3_IER_ASC0_TB_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_TB_EN 0x00000008
/** ASC0 Error Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_E 0x00000004
/* Disable
#define ICU0_IM3_IER_ASC0_E_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_E_EN 0x00000004
/** ASC0 Receive Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_R 0x00000002
/* Disable
#define ICU0_IM3_IER_ASC0_R_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_R_EN 0x00000002
/** ASC0 Transmit Interrupt
    Interrupt enable bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IER_ASC0_T 0x00000001
/* Disable
#define ICU0_IM3_IER_ASC0_T_DIS 0x00000000 */
/** Enable */
#define ICU0_IM3_IER_ASC0_T_EN 0x00000001

/* Fields of "IM3 Interrupt Output Status Register" */
/** DFEV0, Channel 0 General Purpose Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_1GP 0x80000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_1GP_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_1GP_INTOCC 0x80000000
/** DFEV0, Channel 0 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_1RX 0x40000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_1RX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_1RX_INTOCC 0x40000000
/** DFEV0, Channel 0 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_1TX 0x20000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_1TX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_1TX_INTOCC 0x20000000
/** DFEV0, Channel 1 General Purpose Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_2GP 0x10000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_2GP_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_2GP_INTOCC 0x10000000
/** DFEV0, Channel 1 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_2RX 0x08000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_2RX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_2RX_INTOCC 0x08000000
/** DFEV0, Channel 1 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_DFEV0_2TX 0x04000000
/* Nothing
#define ICU0_IM3_IOSR_DFEV0_2TX_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_DFEV0_2TX_INTOCC 0x04000000
/** GPTC Timer/Counter 3B Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC3B 0x00200000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC3B_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC3B_INTOCC 0x00200000
/** GPTC Timer/Counter 3A Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC3A 0x00100000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC3A_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC3A_INTOCC 0x00100000
/** GPTC Timer/Counter 2B Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC2B 0x00080000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC2B_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC2B_INTOCC 0x00080000
/** GPTC Timer/Counter 2A Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC2A 0x00040000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC2A_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC2A_INTOCC 0x00040000
/** GPTC Timer/Counter 1B Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC1B 0x00020000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC1B_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC1B_INTOCC 0x00020000
/** GPTC Timer/Counter 1A Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_GPTC_TC1A 0x00010000
/* Nothing
#define ICU0_IM3_IOSR_GPTC_TC1A_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_GPTC_TC1A_INTOCC 0x00010000
/** ASC1 Soft Flow Control Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_SFC 0x00008000
/* Nothing
#define ICU0_IM3_IOSR_ASC1_SFC_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_SFC_INTOCC 0x00008000
/** ASC1 Modem Status Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_MS 0x00004000
/* Nothing
#define ICU0_IM3_IOSR_ASC1_MS_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_MS_INTOCC 0x00004000
/** ASC1 Autobaud Detection Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_ABDET 0x00002000
/* Nothing
#define ICU0_IM3_IOSR_ASC1_ABDET_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_ABDET_INTOCC 0x00002000
/** ASC1 Autobaud Start Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_ABST 0x00001000
/* Nothing
#define ICU0_IM3_IOSR_ASC1_ABST_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_ABST_INTOCC 0x00001000
/** ASC1 Transmit Buffer Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_TB 0x00000800
/* Nothing
#define ICU0_IM3_IOSR_ASC1_TB_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_TB_INTOCC 0x00000800
/** ASC1 Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_E 0x00000400
/* Nothing
#define ICU0_IM3_IOSR_ASC1_E_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_E_INTOCC 0x00000400
/** ASC1 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_R 0x00000200
/* Nothing
#define ICU0_IM3_IOSR_ASC1_R_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_R_INTOCC 0x00000200
/** ASC1 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC1_T 0x00000100
/* Nothing
#define ICU0_IM3_IOSR_ASC1_T_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC1_T_INTOCC 0x00000100
/** ASC0 Soft Flow Control Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_SFC 0x00000080
/* Nothing
#define ICU0_IM3_IOSR_ASC0_SFC_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_SFC_INTOCC 0x00000080
/** ASC1 Modem Status Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_MS 0x00000040
/* Nothing
#define ICU0_IM3_IOSR_ASC0_MS_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_MS_INTOCC 0x00000040
/** ASC0 Autobaud Detection Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_ABDET 0x00000020
/* Nothing
#define ICU0_IM3_IOSR_ASC0_ABDET_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_ABDET_INTOCC 0x00000020
/** ASC0 Autobaud Start Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_ABST 0x00000010
/* Nothing
#define ICU0_IM3_IOSR_ASC0_ABST_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_ABST_INTOCC 0x00000010
/** ASC0 Transmit Buffer Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_TB 0x00000008
/* Nothing
#define ICU0_IM3_IOSR_ASC0_TB_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_TB_INTOCC 0x00000008
/** ASC0 Error Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_E 0x00000004
/* Nothing
#define ICU0_IM3_IOSR_ASC0_E_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_E_INTOCC 0x00000004
/** ASC0 Receive Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_R 0x00000002
/* Nothing
#define ICU0_IM3_IOSR_ASC0_R_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_R_INTOCC 0x00000002
/** ASC0 Transmit Interrupt
    Masked interrupt bit for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IOSR_ASC0_T 0x00000001
/* Nothing
#define ICU0_IM3_IOSR_ASC0_T_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM3_IOSR_ASC0_T_INTOCC 0x00000001

/* Fields of "IM3 Interrupt Request Set Register" */
/** DFEV0, Channel 0 General Purpose Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_1GP 0x80000000
/** DFEV0, Channel 0 Receive Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_1RX 0x40000000
/** DFEV0, Channel 0 Transmit Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_1TX 0x20000000
/** DFEV0, Channel 1 General Purpose Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_2GP 0x10000000
/** DFEV0, Channel 1 Receive Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_2RX 0x08000000
/** DFEV0, Channel 1 Transmit Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_DFEV0_2TX 0x04000000
/** GPTC Timer/Counter 3B Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC3B 0x00200000
/** GPTC Timer/Counter 3A Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC3A 0x00100000
/** GPTC Timer/Counter 2B Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC2B 0x00080000
/** GPTC Timer/Counter 2A Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC2A 0x00040000
/** GPTC Timer/Counter 1B Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC1B 0x00020000
/** GPTC Timer/Counter 1A Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_GPTC_TC1A 0x00010000
/** ASC1 Soft Flow Control Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_SFC 0x00008000
/** ASC1 Modem Status Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_MS 0x00004000
/** ASC1 Autobaud Detection Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_ABDET 0x00002000
/** ASC1 Autobaud Start Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_ABST 0x00001000
/** ASC1 Transmit Buffer Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_TB 0x00000800
/** ASC1 Error Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_E 0x00000400
/** ASC1 Receive Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_R 0x00000200
/** ASC1 Transmit Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC1_T 0x00000100
/** ASC0 Soft Flow Control Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_SFC 0x00000080
/** ASC1 Modem Status Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_MS 0x00000040
/** ASC0 Autobaud Detection Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_ABDET 0x00000020
/** ASC0 Autobaud Start Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_ABST 0x00000010
/** ASC0 Transmit Buffer Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_TB 0x00000008
/** ASC0 Error Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_E 0x00000004
/** ASC0 Receive Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_R 0x00000002
/** ASC0 Transmit Interrupt
    Software control for the corresponding bit in the IM3_ISR register. */
#define ICU0_IM3_IRSR_ASC0_T 0x00000001

/* Fields of "IM3 Interrupt Mode Register" */
/** DFEV0, Channel 0 General Purpose Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_1GP 0x80000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_1GP_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_1GP_DIR 0x80000000
/** DFEV0, Channel 0 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_1RX 0x40000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_1RX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_1RX_DIR 0x40000000
/** DFEV0, Channel 0 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_1TX 0x20000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_1TX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_1TX_DIR 0x20000000
/** DFEV0, Channel 1 General Purpose Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_2GP 0x10000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_2GP_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_2GP_DIR 0x10000000
/** DFEV0, Channel 1 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_2RX 0x08000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_2RX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_2RX_DIR 0x08000000
/** DFEV0, Channel 1 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_DFEV0_2TX 0x04000000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_DFEV0_2TX_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_DFEV0_2TX_DIR 0x04000000
/** GPTC Timer/Counter 3B Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC3B 0x00200000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC3B_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC3B_DIR 0x00200000
/** GPTC Timer/Counter 3A Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC3A 0x00100000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC3A_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC3A_DIR 0x00100000
/** GPTC Timer/Counter 2B Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC2B 0x00080000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC2B_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC2B_DIR 0x00080000
/** GPTC Timer/Counter 2A Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC2A 0x00040000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC2A_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC2A_DIR 0x00040000
/** GPTC Timer/Counter 1B Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC1B 0x00020000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC1B_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC1B_DIR 0x00020000
/** GPTC Timer/Counter 1A Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_GPTC_TC1A 0x00010000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_GPTC_TC1A_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_GPTC_TC1A_DIR 0x00010000
/** ASC1 Soft Flow Control Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_SFC 0x00008000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_SFC_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_SFC_DIR 0x00008000
/** ASC1 Modem Status Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_MS 0x00004000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_MS_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_MS_DIR 0x00004000
/** ASC1 Autobaud Detection Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_ABDET 0x00002000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_ABDET_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_ABDET_DIR 0x00002000
/** ASC1 Autobaud Start Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_ABST 0x00001000
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_ABST_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_ABST_DIR 0x00001000
/** ASC1 Transmit Buffer Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_TB 0x00000800
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_TB_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_TB_DIR 0x00000800
/** ASC1 Error Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_E 0x00000400
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_E_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_E_DIR 0x00000400
/** ASC1 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_R 0x00000200
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_R_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_R_DIR 0x00000200
/** ASC1 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC1_T 0x00000100
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC1_T_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC1_T_DIR 0x00000100
/** ASC0 Soft Flow Control Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_SFC 0x00000080
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_SFC_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_SFC_DIR 0x00000080
/** ASC1 Modem Status Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_MS 0x00000040
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_MS_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_MS_DIR 0x00000040
/** ASC0 Autobaud Detection Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_ABDET 0x00000020
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_ABDET_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_ABDET_DIR 0x00000020
/** ASC0 Autobaud Start Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_ABST 0x00000010
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_ABST_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_ABST_DIR 0x00000010
/** ASC0 Transmit Buffer Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_TB 0x00000008
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_TB_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_TB_DIR 0x00000008
/** ASC0 Error Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_E 0x00000004
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_E_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_E_DIR 0x00000004
/** ASC0 Receive Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_R 0x00000002
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_R_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_R_DIR 0x00000002
/** ASC0 Transmit Interrupt
    Type of interrupt. */
#define ICU0_IM3_IMR_ASC0_T 0x00000001
/* Indirect Interrupt.
#define ICU0_IM3_IMR_ASC0_T_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM3_IMR_ASC0_T_DIR 0x00000001

/* Fields of "IM4 Interrupt Status Register" */
/** VPE0 Performance Monitoring Counter Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_VPE0_PMCIR 0x80000000
/* Nothing
#define ICU0_IM4_ISR_VPE0_PMCIR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_VPE0_PMCIR_INTACK 0x80000000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_VPE0_PMCIR_INTOCC 0x80000000
/** VPE0 Error Level Flag Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_VPE0_ERL 0x40000000
/* Nothing
#define ICU0_IM4_ISR_VPE0_ERL_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_VPE0_ERL_INTACK 0x40000000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_VPE0_ERL_INTOCC 0x40000000
/** VPE0 Exception Level Flag Interrupt
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_VPE0_EXL 0x20000000
/* Nothing
#define ICU0_IM4_ISR_VPE0_EXL_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_VPE0_EXL_INTACK 0x20000000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_VPE0_EXL_INTOCC 0x20000000
/** MPS Bin. Sem Interrupt to VPE0
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR8 0x00400000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR8_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR8_INTACK 0x00400000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR8_INTOCC 0x00400000
/** MPS Global Interrupt to VPE0
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR7 0x00200000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR7_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR7_INTACK 0x00200000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR7_INTOCC 0x00200000
/** MPS Status Interrupt #6 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR6 0x00100000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR6_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR6_INTACK 0x00100000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR6_INTOCC 0x00100000
/** MPS Status Interrupt #5 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR5 0x00080000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR5_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR5_INTACK 0x00080000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR5_INTOCC 0x00080000
/** MPS Status Interrupt #4 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR4 0x00040000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR4_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR4_INTACK 0x00040000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR4_INTOCC 0x00040000
/** MPS Status Interrupt #3 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR3 0x00020000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR3_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR3_INTACK 0x00020000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR3_INTOCC 0x00020000
/** MPS Status Interrupt #2 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR2 0x00010000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR2_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR2_INTACK 0x00010000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR2_INTOCC 0x00010000
/** MPS Status Interrupt #1 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR1 0x00008000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR1_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR1_INTACK 0x00008000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR1_INTOCC 0x00008000
/** MPS Status Interrupt #0 (VPE1 to VPE0)
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_MPS_IR0 0x00004000
/* Nothing
#define ICU0_IM4_ISR_MPS_IR0_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_MPS_IR0_INTACK 0x00004000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_MPS_IR0_INTOCC 0x00004000
/** TMU Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_TMU_ERR 0x00001000
/* Nothing
#define ICU0_IM4_ISR_TMU_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_TMU_ERR_INTACK 0x00001000
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_TMU_ERR_INTOCC 0x00001000
/** FSQM Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_FSQM_ERR 0x00000800
/* Nothing
#define ICU0_IM4_ISR_FSQM_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_FSQM_ERR_INTACK 0x00000800
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_FSQM_ERR_INTOCC 0x00000800
/** IQM Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_IQM_ERR 0x00000400
/* Nothing
#define ICU0_IM4_ISR_IQM_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_IQM_ERR_INTACK 0x00000400
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_IQM_ERR_INTOCC 0x00000400
/** OCTRLG Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_OCTRLG_ERR 0x00000200
/* Nothing
#define ICU0_IM4_ISR_OCTRLG_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_OCTRLG_ERR_INTACK 0x00000200
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_OCTRLG_ERR_INTOCC 0x00000200
/** ICTRLG Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_ICTRLG_ERR 0x00000100
/* Nothing
#define ICU0_IM4_ISR_ICTRLG_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_ICTRLG_ERR_INTACK 0x00000100
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_ICTRLG_ERR_INTOCC 0x00000100
/** OCTRLL 3 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_OCTRLL3_ERR 0x00000080
/* Nothing
#define ICU0_IM4_ISR_OCTRLL3_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_OCTRLL3_ERR_INTACK 0x00000080
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_OCTRLL3_ERR_INTOCC 0x00000080
/** OCTRLL 2 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_OCTRLL2_ERR 0x00000040
/* Nothing
#define ICU0_IM4_ISR_OCTRLL2_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_OCTRLL2_ERR_INTACK 0x00000040
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_OCTRLL2_ERR_INTOCC 0x00000040
/** OCTRLL 1 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_OCTRLL1_ERR 0x00000020
/* Nothing
#define ICU0_IM4_ISR_OCTRLL1_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_OCTRLL1_ERR_INTACK 0x00000020
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_OCTRLL1_ERR_INTOCC 0x00000020
/** OCTRLL 0 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_OCTRLL0_ERR 0x00000010
/* Nothing
#define ICU0_IM4_ISR_OCTRLL0_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_OCTRLL0_ERR_INTACK 0x00000010
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_OCTRLL0_ERR_INTOCC 0x00000010
/** ICTRLL 3 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_ICTRLL3_ERR 0x00000008
/* Nothing
#define ICU0_IM4_ISR_ICTRLL3_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_ICTRLL3_ERR_INTACK 0x00000008
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_ICTRLL3_ERR_INTOCC 0x00000008
/** ICTRLL 2 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_ICTRLL2_ERR 0x00000004
/* Nothing
#define ICU0_IM4_ISR_ICTRLL2_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_ICTRLL2_ERR_INTACK 0x00000004
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_ICTRLL2_ERR_INTOCC 0x00000004
/** ICTRLL 1 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_ICTRLL1_ERR 0x00000002
/* Nothing
#define ICU0_IM4_ISR_ICTRLL1_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_ICTRLL1_ERR_INTACK 0x00000002
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_ICTRLL1_ERR_INTOCC 0x00000002
/** ICTRLL 0 Error
    This bit is an indirect interrupt. */
#define ICU0_IM4_ISR_ICTRLL0_ERR 0x00000001
/* Nothing
#define ICU0_IM4_ISR_ICTRLL0_ERR_NULL 0x00000000 */
/** Write: Acknowledge the interrupt. */
#define ICU0_IM4_ISR_ICTRLL0_ERR_INTACK 0x00000001
/** Read: Interrupt occurred. */
#define ICU0_IM4_ISR_ICTRLL0_ERR_INTOCC 0x00000001

/* Fields of "IM4 Interrupt Enable Register" */
/** VPE0 Performance Monitoring Counter Interrupt
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_VPE0_PMCIR 0x80000000
/* Disable
#define ICU0_IM4_IER_VPE0_PMCIR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_VPE0_PMCIR_EN 0x80000000
/** VPE0 Error Level Flag Interrupt
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_VPE0_ERL 0x40000000
/* Disable
#define ICU0_IM4_IER_VPE0_ERL_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_VPE0_ERL_EN 0x40000000
/** VPE0 Exception Level Flag Interrupt
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_VPE0_EXL 0x20000000
/* Disable
#define ICU0_IM4_IER_VPE0_EXL_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_VPE0_EXL_EN 0x20000000
/** MPS Bin. Sem Interrupt to VPE0
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR8 0x00400000
/* Disable
#define ICU0_IM4_IER_MPS_IR8_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR8_EN 0x00400000
/** MPS Global Interrupt to VPE0
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR7 0x00200000
/* Disable
#define ICU0_IM4_IER_MPS_IR7_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR7_EN 0x00200000
/** MPS Status Interrupt #6 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR6 0x00100000
/* Disable
#define ICU0_IM4_IER_MPS_IR6_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR6_EN 0x00100000
/** MPS Status Interrupt #5 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR5 0x00080000
/* Disable
#define ICU0_IM4_IER_MPS_IR5_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR5_EN 0x00080000
/** MPS Status Interrupt #4 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR4 0x00040000
/* Disable
#define ICU0_IM4_IER_MPS_IR4_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR4_EN 0x00040000
/** MPS Status Interrupt #3 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR3 0x00020000
/* Disable
#define ICU0_IM4_IER_MPS_IR3_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR3_EN 0x00020000
/** MPS Status Interrupt #2 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR2 0x00010000
/* Disable
#define ICU0_IM4_IER_MPS_IR2_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR2_EN 0x00010000
/** MPS Status Interrupt #1 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR1 0x00008000
/* Disable
#define ICU0_IM4_IER_MPS_IR1_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR1_EN 0x00008000
/** MPS Status Interrupt #0 (VPE1 to VPE0)
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_MPS_IR0 0x00004000
/* Disable
#define ICU0_IM4_IER_MPS_IR0_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_MPS_IR0_EN 0x00004000
/** TMU Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_TMU_ERR 0x00001000
/* Disable
#define ICU0_IM4_IER_TMU_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_TMU_ERR_EN 0x00001000
/** FSQM Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_FSQM_ERR 0x00000800
/* Disable
#define ICU0_IM4_IER_FSQM_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_FSQM_ERR_EN 0x00000800
/** IQM Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_IQM_ERR 0x00000400
/* Disable
#define ICU0_IM4_IER_IQM_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_IQM_ERR_EN 0x00000400
/** OCTRLG Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_OCTRLG_ERR 0x00000200
/* Disable
#define ICU0_IM4_IER_OCTRLG_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_OCTRLG_ERR_EN 0x00000200
/** ICTRLG Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_ICTRLG_ERR 0x00000100
/* Disable
#define ICU0_IM4_IER_ICTRLG_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_ICTRLG_ERR_EN 0x00000100
/** OCTRLL 3 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_OCTRLL3_ERR 0x00000080
/* Disable
#define ICU0_IM4_IER_OCTRLL3_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_OCTRLL3_ERR_EN 0x00000080
/** OCTRLL 2 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_OCTRLL2_ERR 0x00000040
/* Disable
#define ICU0_IM4_IER_OCTRLL2_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_OCTRLL2_ERR_EN 0x00000040
/** OCTRLL 1 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_OCTRLL1_ERR 0x00000020
/* Disable
#define ICU0_IM4_IER_OCTRLL1_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_OCTRLL1_ERR_EN 0x00000020
/** OCTRLL 0 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_OCTRLL0_ERR 0x00000010
/* Disable
#define ICU0_IM4_IER_OCTRLL0_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_OCTRLL0_ERR_EN 0x00000010
/** ICTRLL 3 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_ICTRLL3_ERR 0x00000008
/* Disable
#define ICU0_IM4_IER_ICTRLL3_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_ICTRLL3_ERR_EN 0x00000008
/** ICTRLL 2 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_ICTRLL2_ERR 0x00000004
/* Disable
#define ICU0_IM4_IER_ICTRLL2_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_ICTRLL2_ERR_EN 0x00000004
/** ICTRLL 1 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_ICTRLL1_ERR 0x00000002
/* Disable
#define ICU0_IM4_IER_ICTRLL1_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_ICTRLL1_ERR_EN 0x00000002
/** ICTRLL 0 Error
    Interrupt enable bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IER_ICTRLL0_ERR 0x00000001
/* Disable
#define ICU0_IM4_IER_ICTRLL0_ERR_DIS 0x00000000 */
/** Enable */
#define ICU0_IM4_IER_ICTRLL0_ERR_EN 0x00000001

/* Fields of "IM4 Interrupt Output Status Register" */
/** VPE0 Performance Monitoring Counter Interrupt
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_VPE0_PMCIR 0x80000000
/* Nothing
#define ICU0_IM4_IOSR_VPE0_PMCIR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_VPE0_PMCIR_INTOCC 0x80000000
/** VPE0 Error Level Flag Interrupt
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_VPE0_ERL 0x40000000
/* Nothing
#define ICU0_IM4_IOSR_VPE0_ERL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_VPE0_ERL_INTOCC 0x40000000
/** VPE0 Exception Level Flag Interrupt
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_VPE0_EXL 0x20000000
/* Nothing
#define ICU0_IM4_IOSR_VPE0_EXL_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_VPE0_EXL_INTOCC 0x20000000
/** MPS Bin. Sem Interrupt to VPE0
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR8 0x00400000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR8_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR8_INTOCC 0x00400000
/** MPS Global Interrupt to VPE0
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR7 0x00200000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR7_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR7_INTOCC 0x00200000
/** MPS Status Interrupt #6 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR6 0x00100000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR6_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR6_INTOCC 0x00100000
/** MPS Status Interrupt #5 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR5 0x00080000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR5_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR5_INTOCC 0x00080000
/** MPS Status Interrupt #4 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR4 0x00040000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR4_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR4_INTOCC 0x00040000
/** MPS Status Interrupt #3 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR3 0x00020000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR3_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR3_INTOCC 0x00020000
/** MPS Status Interrupt #2 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR2 0x00010000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR2_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR2_INTOCC 0x00010000
/** MPS Status Interrupt #1 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR1 0x00008000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR1_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR1_INTOCC 0x00008000
/** MPS Status Interrupt #0 (VPE1 to VPE0)
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_MPS_IR0 0x00004000
/* Nothing
#define ICU0_IM4_IOSR_MPS_IR0_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_MPS_IR0_INTOCC 0x00004000
/** TMU Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_TMU_ERR 0x00001000
/* Nothing
#define ICU0_IM4_IOSR_TMU_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_TMU_ERR_INTOCC 0x00001000
/** FSQM Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_FSQM_ERR 0x00000800
/* Nothing
#define ICU0_IM4_IOSR_FSQM_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_FSQM_ERR_INTOCC 0x00000800
/** IQM Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_IQM_ERR 0x00000400
/* Nothing
#define ICU0_IM4_IOSR_IQM_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_IQM_ERR_INTOCC 0x00000400
/** OCTRLG Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_OCTRLG_ERR 0x00000200
/* Nothing
#define ICU0_IM4_IOSR_OCTRLG_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_OCTRLG_ERR_INTOCC 0x00000200
/** ICTRLG Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_ICTRLG_ERR 0x00000100
/* Nothing
#define ICU0_IM4_IOSR_ICTRLG_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_ICTRLG_ERR_INTOCC 0x00000100
/** OCTRLL 3 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_OCTRLL3_ERR 0x00000080
/* Nothing
#define ICU0_IM4_IOSR_OCTRLL3_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_OCTRLL3_ERR_INTOCC 0x00000080
/** OCTRLL 2 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_OCTRLL2_ERR 0x00000040
/* Nothing
#define ICU0_IM4_IOSR_OCTRLL2_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_OCTRLL2_ERR_INTOCC 0x00000040
/** OCTRLL 1 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_OCTRLL1_ERR 0x00000020
/* Nothing
#define ICU0_IM4_IOSR_OCTRLL1_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_OCTRLL1_ERR_INTOCC 0x00000020
/** OCTRLL 0 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_OCTRLL0_ERR 0x00000010
/* Nothing
#define ICU0_IM4_IOSR_OCTRLL0_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_OCTRLL0_ERR_INTOCC 0x00000010
/** ICTRLL 3 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_ICTRLL3_ERR 0x00000008
/* Nothing
#define ICU0_IM4_IOSR_ICTRLL3_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_ICTRLL3_ERR_INTOCC 0x00000008
/** ICTRLL 2 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_ICTRLL2_ERR 0x00000004
/* Nothing
#define ICU0_IM4_IOSR_ICTRLL2_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_ICTRLL2_ERR_INTOCC 0x00000004
/** ICTRLL 1 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_ICTRLL1_ERR 0x00000002
/* Nothing
#define ICU0_IM4_IOSR_ICTRLL1_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_ICTRLL1_ERR_INTOCC 0x00000002
/** ICTRLL 0 Error
    Masked interrupt bit for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IOSR_ICTRLL0_ERR 0x00000001
/* Nothing
#define ICU0_IM4_IOSR_ICTRLL0_ERR_NULL 0x00000000 */
/** Read: Interrupt occurred. */
#define ICU0_IM4_IOSR_ICTRLL0_ERR_INTOCC 0x00000001

/* Fields of "IM4 Interrupt Request Set Register" */
/** VPE0 Performance Monitoring Counter Interrupt
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_VPE0_PMCIR 0x80000000
/** VPE0 Error Level Flag Interrupt
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_VPE0_ERL 0x40000000
/** VPE0 Exception Level Flag Interrupt
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_VPE0_EXL 0x20000000
/** MPS Bin. Sem Interrupt to VPE0
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR8 0x00400000
/** MPS Global Interrupt to VPE0
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR7 0x00200000
/** MPS Status Interrupt #6 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR6 0x00100000
/** MPS Status Interrupt #5 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR5 0x00080000
/** MPS Status Interrupt #4 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR4 0x00040000
/** MPS Status Interrupt #3 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR3 0x00020000
/** MPS Status Interrupt #2 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR2 0x00010000
/** MPS Status Interrupt #1 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR1 0x00008000
/** MPS Status Interrupt #0 (VPE1 to VPE0)
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_MPS_IR0 0x00004000
/** TMU Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_TMU_ERR 0x00001000
/** FSQM Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_FSQM_ERR 0x00000800
/** IQM Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_IQM_ERR 0x00000400
/** OCTRLG Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_OCTRLG_ERR 0x00000200
/** ICTRLG Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_ICTRLG_ERR 0x00000100
/** OCTRLL 3 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_OCTRLL3_ERR 0x00000080
/** OCTRLL 2 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_OCTRLL2_ERR 0x00000040
/** OCTRLL 1 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_OCTRLL1_ERR 0x00000020
/** OCTRLL 0 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_OCTRLL0_ERR 0x00000010
/** ICTRLL 3 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_ICTRLL3_ERR 0x00000008
/** ICTRLL 2 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_ICTRLL2_ERR 0x00000004
/** ICTRLL 1 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_ICTRLL1_ERR 0x00000002
/** ICTRLL 0 Error
    Software control for the corresponding bit in the IM4_ISR register. */
#define ICU0_IM4_IRSR_ICTRLL0_ERR 0x00000001

/* Fields of "IM4 Interrupt Mode Register" */
/** VPE0 Performance Monitoring Counter Interrupt
    Type of interrupt. */
#define ICU0_IM4_IMR_VPE0_PMCIR 0x80000000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_VPE0_PMCIR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_VPE0_PMCIR_DIR 0x80000000
/** VPE0 Error Level Flag Interrupt
    Type of interrupt. */
#define ICU0_IM4_IMR_VPE0_ERL 0x40000000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_VPE0_ERL_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_VPE0_ERL_DIR 0x40000000
/** VPE0 Exception Level Flag Interrupt
    Type of interrupt. */
#define ICU0_IM4_IMR_VPE0_EXL 0x20000000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_VPE0_EXL_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_VPE0_EXL_DIR 0x20000000
/** MPS Bin. Sem Interrupt to VPE0
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR8 0x00400000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR8_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR8_DIR 0x00400000
/** MPS Global Interrupt to VPE0
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR7 0x00200000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR7_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR7_DIR 0x00200000
/** MPS Status Interrupt #6 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR6 0x00100000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR6_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR6_DIR 0x00100000
/** MPS Status Interrupt #5 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR5 0x00080000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR5_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR5_DIR 0x00080000
/** MPS Status Interrupt #4 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR4 0x00040000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR4_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR4_DIR 0x00040000
/** MPS Status Interrupt #3 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR3 0x00020000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR3_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR3_DIR 0x00020000
/** MPS Status Interrupt #2 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR2 0x00010000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR2_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR2_DIR 0x00010000
/** MPS Status Interrupt #1 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR1 0x00008000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR1_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR1_DIR 0x00008000
/** MPS Status Interrupt #0 (VPE1 to VPE0)
    Type of interrupt. */
#define ICU0_IM4_IMR_MPS_IR0 0x00004000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_MPS_IR0_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_MPS_IR0_DIR 0x00004000
/** TMU Error
    Type of interrupt. */
#define ICU0_IM4_IMR_TMU_ERR 0x00001000
/* Indirect Interrupt.
#define ICU0_IM4_IMR_TMU_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_TMU_ERR_DIR 0x00001000
/** FSQM Error
    Type of interrupt. */
#define ICU0_IM4_IMR_FSQM_ERR 0x00000800
/* Indirect Interrupt.
#define ICU0_IM4_IMR_FSQM_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_FSQM_ERR_DIR 0x00000800
/** IQM Error
    Type of interrupt. */
#define ICU0_IM4_IMR_IQM_ERR 0x00000400
/* Indirect Interrupt.
#define ICU0_IM4_IMR_IQM_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_IQM_ERR_DIR 0x00000400
/** OCTRLG Error
    Type of interrupt. */
#define ICU0_IM4_IMR_OCTRLG_ERR 0x00000200
/* Indirect Interrupt.
#define ICU0_IM4_IMR_OCTRLG_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_OCTRLG_ERR_DIR 0x00000200
/** ICTRLG Error
    Type of interrupt. */
#define ICU0_IM4_IMR_ICTRLG_ERR 0x00000100
/* Indirect Interrupt.
#define ICU0_IM4_IMR_ICTRLG_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_ICTRLG_ERR_DIR 0x00000100
/** OCTRLL 3 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_OCTRLL3_ERR 0x00000080
/* Indirect Interrupt.
#define ICU0_IM4_IMR_OCTRLL3_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_OCTRLL3_ERR_DIR 0x00000080
/** OCTRLL 2 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_OCTRLL2_ERR 0x00000040
/* Indirect Interrupt.
#define ICU0_IM4_IMR_OCTRLL2_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_OCTRLL2_ERR_DIR 0x00000040
/** OCTRLL 1 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_OCTRLL1_ERR 0x00000020
/* Indirect Interrupt.
#define ICU0_IM4_IMR_OCTRLL1_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_OCTRLL1_ERR_DIR 0x00000020
/** OCTRLL 0 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_OCTRLL0_ERR 0x00000010
/* Indirect Interrupt.
#define ICU0_IM4_IMR_OCTRLL0_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_OCTRLL0_ERR_DIR 0x00000010
/** ICTRLL 3 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_ICTRLL3_ERR 0x00000008
/* Indirect Interrupt.
#define ICU0_IM4_IMR_ICTRLL3_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_ICTRLL3_ERR_DIR 0x00000008
/** ICTRLL 2 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_ICTRLL2_ERR 0x00000004
/* Indirect Interrupt.
#define ICU0_IM4_IMR_ICTRLL2_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_ICTRLL2_ERR_DIR 0x00000004
/** ICTRLL 1 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_ICTRLL1_ERR 0x00000002
/* Indirect Interrupt.
#define ICU0_IM4_IMR_ICTRLL1_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_ICTRLL1_ERR_DIR 0x00000002
/** ICTRLL 0 Error
    Type of interrupt. */
#define ICU0_IM4_IMR_ICTRLL0_ERR 0x00000001
/* Indirect Interrupt.
#define ICU0_IM4_IMR_ICTRLL0_ERR_IND 0x00000000 */
/** Direct Interrupt. */
#define ICU0_IM4_IMR_ICTRLL0_ERR_DIR 0x00000001

/* Fields of "ICU Interrupt Vector Register (5 bit variant)" */
/** IM4 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_IM4_vec_MASK 0x01F00000
/** field offset */
#define ICU0_ICU_IVEC_IM4_vec_OFFSET 20
/** Interrupt pending at bit 31 or no pending interrupt */
#define ICU0_ICU_IVEC_IM4_vec_NOINTorBit31 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_IM4_vec_BIT0 0x00100000
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_IM4_vec_BIT1 0x00200000
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_IM4_vec_BIT30 0x01F00000
/** IM3 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_IM3_vec_MASK 0x000F8000
/** field offset */
#define ICU0_ICU_IVEC_IM3_vec_OFFSET 15
/** Interrupt pending at bit 31 or no pending interrupt */
#define ICU0_ICU_IVEC_IM3_vec_NOINTorBit31 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_IM3_vec_BIT0 0x00008000
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_IM3_vec_BIT1 0x00010000
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_IM3_vec_BIT30 0x000F8000
/** IM2 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_IM2_vec_MASK 0x00007C00
/** field offset */
#define ICU0_ICU_IVEC_IM2_vec_OFFSET 10
/** Interrupt pending at bit 31 or no pending interrupt */
#define ICU0_ICU_IVEC_IM2_vec_NOINTorBit31 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_IM2_vec_BIT0 0x00000400
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_IM2_vec_BIT1 0x00000800
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_IM2_vec_BIT30 0x00007C00
/** IM1 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_IM1_vec_MASK 0x000003E0
/** field offset */
#define ICU0_ICU_IVEC_IM1_vec_OFFSET 5
/** Interrupt pending at bit 31 or no pending interrupt */
#define ICU0_ICU_IVEC_IM1_vec_NOINTorBit31 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_IM1_vec_BIT0 0x00000020
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_IM1_vec_BIT1 0x00000040
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_IM1_vec_BIT30 0x000003E0
/** IM0 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_IM0_vec_MASK 0x0000001F
/** field offset */
#define ICU0_ICU_IVEC_IM0_vec_OFFSET 0
/** Interrupt pending at bit 31 or no pending interrupt */
#define ICU0_ICU_IVEC_IM0_vec_NOINTorBit31 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_IM0_vec_BIT0 0x00000001
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_IM0_vec_BIT1 0x00000002
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_IM0_vec_BIT30 0x0000001F

/* Fields of "ICU Interrupt Vector Register (6 bit variant)" */
/** IM4 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_6_IM4_vec_MASK 0x3F000000
/** field offset */
#define ICU0_ICU_IVEC_6_IM4_vec_OFFSET 24
/** No pending interrupt */
#define ICU0_ICU_IVEC_6_IM4_vec_NOINT 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_6_IM4_vec_BIT0 0x01000000
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_6_IM4_vec_BIT1 0x02000000
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_6_IM4_vec_BIT30 0x1F000000
/** Interrupt pending at bit 31. */
#define ICU0_ICU_IVEC_6_IM4_vec_BIT31 0x20000000
/** IM3 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_6_IM3_vec_MASK 0x00FC0000
/** field offset */
#define ICU0_ICU_IVEC_6_IM3_vec_OFFSET 18
/** No pending interrupt */
#define ICU0_ICU_IVEC_6_IM3_vec_NOINT 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_6_IM3_vec_BIT0 0x00040000
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_6_IM3_vec_BIT1 0x00080000
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_6_IM3_vec_BIT30 0x007C0000
/** Interrupt pending at bit 31. */
#define ICU0_ICU_IVEC_6_IM3_vec_BIT31 0x00800000
/** IM2 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_6_IM2_vec_MASK 0x0003F000
/** field offset */
#define ICU0_ICU_IVEC_6_IM2_vec_OFFSET 12
/** No pending interrupt */
#define ICU0_ICU_IVEC_6_IM2_vec_NOINT 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_6_IM2_vec_BIT0 0x00001000
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_6_IM2_vec_BIT1 0x00002000
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_6_IM2_vec_BIT30 0x0001F000
/** Interrupt pending at bit 31. */
#define ICU0_ICU_IVEC_6_IM2_vec_BIT31 0x00020000
/** IM1 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_6_IM1_vec_MASK 0x00000FC0
/** field offset */
#define ICU0_ICU_IVEC_6_IM1_vec_OFFSET 6
/** No pending interrupt */
#define ICU0_ICU_IVEC_6_IM1_vec_NOINT 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_6_IM1_vec_BIT0 0x00000040
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_6_IM1_vec_BIT1 0x00000080
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_6_IM1_vec_BIT30 0x000007C0
/** Interrupt pending at bit 31. */
#define ICU0_ICU_IVEC_6_IM1_vec_BIT31 0x00000800
/** IM0 Interrupt Vector Value
    Returns the highest priority pending interrupt vector. */
#define ICU0_ICU_IVEC_6_IM0_vec_MASK 0x0000003F
/** field offset */
#define ICU0_ICU_IVEC_6_IM0_vec_OFFSET 0
/** No pending interrupt */
#define ICU0_ICU_IVEC_6_IM0_vec_NOINT 0x00000000
/** Interrupt pending at bit 0. */
#define ICU0_ICU_IVEC_6_IM0_vec_BIT0 0x00000001
/** Interrupt pending at bit 1. */
#define ICU0_ICU_IVEC_6_IM0_vec_BIT1 0x00000002
/** Interrupt pending at bit 30. */
#define ICU0_ICU_IVEC_6_IM0_vec_BIT30 0x0000001F
/** Interrupt pending at bit 31. */
#define ICU0_ICU_IVEC_6_IM0_vec_BIT31 0x00000020

/*! @} */ /* ICU0_REGISTER */

#endif /* _icu0_reg_h */
