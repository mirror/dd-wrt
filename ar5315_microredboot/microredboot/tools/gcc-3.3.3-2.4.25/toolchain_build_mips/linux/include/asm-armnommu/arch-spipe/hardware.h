/*
 * linux/include/asm-arm/arch-trio/hardware.h
 *
 * Copyright (C) 1996 Russell King.
 *
 * This file contains the hardware definitions of the TI DSC21 series machines
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

/* ARM clock rate (MHz) */
#define DSC21_CLOCK_RATE     37125000

/* external bus */
#define DSC21_EXTBUS_RESET    0x00030a00
#define DSC21_EXTBUS_MODE     0x00030a04
#define DSC21_EXTBUS_INTEN    0x00030a08

/* uarts (note that the register fields aren't needed by the standard linux
   serial driver -- we only use them for our initial debugging console) */
#define DSC21_UART0       0x00030380
#define DSC21_UART0_DTRR  0x00030380
#define DSC21_UART0_BRSR  0x00030382
#define DSC21_UART0_MSR   0x00030384
#define DSC21_UART0_RFCR  0x00030386
#define DSC21_UART0_TFCR  0x00030388
#define DSC21_UART0_LCR	  0x0003038a
#define DSC21_UART0_SR 	  0x0003038c

#define DSC21_UART1       0x00030400
#define DSC21_UART1_DTRR  0x00030400
#define DSC21_UART1_BRSR  0x00030402
#define DSC21_UART1_MSR   0x00030404
#define DSC21_UART1_RFCR  0x00030406
#define DSC21_UART1_TFCR  0x00030408
#define DSC21_UART1_LCR	  0x0003040a
#define DSC21_UART1_SR 	  0x0003040c

/* timers */
#define DSC21_TIMER0_MODE    0x00030000
#define DSC21_TIMER0_SEL     0x00030002
#define DSC21_TIMER0_SCAL    0x00030004
#define DSC21_TIMER0_DIV     0x00030006
#define DSC21_TIMER0_TRG     0x00030008
#define DSC21_TIMER0_CNT     0x0003000a

#define DSC21_TIMER1_MODE    0x00030080
#define DSC21_TIMER1_SEL     0x00030082
#define DSC21_TIMER1_SCAL    0x00030084
#define DSC21_TIMER1_DIV     0x00030086
#define DSC21_TIMER1_TRG     0x00030088
#define DSC21_TIMER1_CNT     0x0003008a

#define DSC21_TIMER2_MODE    0x00030100
#define DSC21_TIMER2_SEL     0x00030102
#define DSC21_TIMER2_SCAL    0x00030104
#define DSC21_TIMER2_DIV     0x00030106
#define DSC21_TIMER2_TRG     0x00030108
#define DSC21_TIMER2_CNT     0x0003010a

#define DSC21_TIMER3_MODE    0x00030180
#define DSC21_TIMER3_SEL     0x00030182
#define DSC21_TIMER3_SCAL    0x00030184
#define DSC21_TIMER3_DIV     0x00030186
#define DSC21_TIMER3_TRG     0x00030188
#define DSC21_TIMER3_CNT     0x0003018a

/* interrupt */
#define DSC21_FIQ_STATUS     0x00030580
#define DSC21_IRQ0_STATUS    0x00030582
#define DSC21_IRQ1_STATUS    0x00030584
#define DSC21_FIQ_ENABLE     0x000305a0
#define DSC21_IRQ0_ENABLE    0x000305a2
#define DSC21_IRQ1_ENABLE    0x000305a4


/*
 * HARD_RESET_NOW -- used in blkmem.c. Should call arch_hard_reset(), but I 
 * don't appear to have one ;).
 * --gmcnutt
 */
#define HARD_RESET_NOW()


/* reference */
/////////////////////////////////////////////////
//
//	register and SDRAM map
//
//  (c) VSA/TITRDC   06/1999
//
/////////////////////////////////////////////////

// memory mapped registers
#define TIMER0_REGISTER_BASE    0x00030000		// timer0
#define TIMER1_REGISTER_BASE	0x00030080		// timer1
#define TIMER2_REGISTER_BASE	0x00030100		// timer2
#define TIMER3_REGISTER_BASE	0x00030180		// timer3
#define SERIAL0_REGISTER_BASE	0x00030200		// serial0
#define SERIAL1_REGISTER_BASE	0x00030280		// serial1
#define WDT_REGISTER_BASE	0x00030300		// watch dog timer
#define UART0_REGISTER_BASE	0x00030380		// UART0
#define UART1_REGISTER_BASE	0x00030400		// UART0
#define USB_REGISTER_BASE	0x00030480		// USB
#define IRDA_REGISTER_BASE	0x00030500		// IrDA
#define INTCTRL_REGISTER_BASE	0x00030580		// INTC
#define GIO_REGISTER_BASE	0x00030600		// GIO
#define OSD_REGISTER_BASE	0x00030680		// OSD
#define DSPC_REGISTER_BASE	0x00030700		// DSP controller
#define CCDC_REGISTER_BASE	0x00030780		// CCD controller
#define PREVIEW_REGISTER_BASE	0x00030800		// preview engine
#define BURSTC_REGISTER_BASE	0x00030880		// burst mode compression
#define VIDEOENC_REGISTER_BASE	0x00030900		// NTSC/PAL encoder
#define SDRAMC_REGISTER_BASE	0x00030980		// SDRAM controller
#define EXBC_REGISTER_BASE	0x00030A00		// external bus controller
#define CLOCKC_REGISTER_BASE	0x00030A80		// clock controller
#define BUSC_REGISTER_BASE	0x00030B00		// bus controller
#define DMAIF_REGISTER_BASE	0x00030B80		// external bus DMA I/F

// DSPRAM
#define DSPRAM_BASE         0x00100000  // DSPRAM base address


// SDRAM address (real byte address)
#define SDRAM_BASE          0x08000000  // sdram base address
#define SDRAM_SIZE          0x01000000  // hard-code 16MB for now
#define SDRAM_PREVIEW       0x00000000  // output from preview engine
#define SDRAM_RAWDATA       0x00100000  // output from CCDC
#define SDRAM_WORK1         0x00100000  // WORK area #1 for burst-capture & decoded data
#define SDRAM_WORK2         0x00500000  // WORK area #2 for burst-capture & decoded data
#define SDRAM_ENCODED1      0x00900000  // work area #1 for encoded data
#define SDRAM_ENCODED2      0x00B00000  // work area #2 for encoded data
#define SDRAM_BURSTDATA     0x00B00000  // BURST-compressed data start address
#define SDRAM_BURSTDATA_END 0x00EFFFFF  // BURST-compressed data end address
#define SDRAM_OSD           0x00F00000  // OSD bitmap start address

// CARD map (tentative)
#define CARD_BASE           0x04000000		// CFC card base address
#define CARD_ENCODED_START	0x00000800		// encoded image start address
#define CARD_ENCODED_SLOT_SIZE	0x00044000		// encoded image slot size

// IRQ defines  (Offset listed in register document is in words, so double offset)
#define IRQ0_STATUS (INTCTRL_REGISTER_BASE + 0x02)
#define IRQ1_STATUS (INTCTRL_REGISTER_BASE + 0x04)
#define IRQ0_ENABLE (INTCTRL_REGISTER_BASE + 0x22)
#define IRQ1_ENABLE (INTCTRL_REGISTER_BASE + 0x24)

#endif  /* _ASM_ARCH_HARDWARE_H */
