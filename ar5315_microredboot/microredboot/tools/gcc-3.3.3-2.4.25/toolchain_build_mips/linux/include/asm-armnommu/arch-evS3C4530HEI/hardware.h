/*
 * uclinux/include/asm-armnommu/arch-evS3C4530HEI/hardware.h
 *
 * OZH, 2001 Oleksandr Zhadan.
 *
 * This file includes the hardware definitions of EV-S3C4530-HEI board.
 */
 
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>
#ifndef __ASSEMBLER__
typedef unsigned long u_32;
#define ARM_CLK ((u_32)(CONFIG_ARM_CLK))	/*(50000000))*/
#else
#define ARM_CLK CONFIG_ARM_CLK
#endif

#define MCLK2 (ARM_CLK/2)
#define Vol32	*(volatile unsigned int *)
#define Vol16	*(volatile unsigned short *)
#define Vol08	*(volatile unsigned char *)

#define REG_WRITE(addr,data)	(Vol32(addr)=(data))
#define REG_READ(addr)		(Vol32(addr))


#ifndef __ASSEMBLER__

/**************************************** RAM definitions */
#define MAPTOPHYS(a)      ((unsigned long)a)
#define KERNTOPHYS(a)     ((unsigned long)(&a))
#define GET_MEMORY_END(p) ((p->u1.s.page_size) * (p->u1.s.nr_pages))
#define PARAMS_BASE       0x7000
#define HARD_RESET_NOW()  { arch_hard_reset(); }
#endif

#define IO_BASE		0x00000000
#define EIO_BASE	0x03600000		/* External I/O banks base address */

#define SRB_BASE 	0x03FF0000

#define BDMA_OWNER 	0x80000000
#define NON_CACHEABLE	0x04000000


/**************************************** System Manager */
#define SYSCFG 		(SRB_BASE+0x0000)
#define CLKCON		(SRB_BASE+0x3000)
#define EXTACON0	(SRB_BASE+0x3008)
#define EXTACON1	(SRB_BASE+0x300C)
#define EXTDBWTH	(SRB_BASE+0x300C)
#define ROMCON0		(SRB_BASE+0x3014)
#define ROMCON1		(SRB_BASE+0x3018)
#define ROMCON2		(SRB_BASE+0x301C)
#define ROMCON3		(SRB_BASE+0x3020)
#define ROMCON4		(SRB_BASE+0x3024)
#define ROMCON5		(SRB_BASE+0x3028)
#define DRAMCON0	(SRB_BASE+0x302C)
#define DRAMCON1	(SRB_BASE+0x3030)
#define DRAMCON2	(SRB_BASE+0x3034)
#define DRAMCON4	(SRB_BASE+0x3038)
#define REFEXTCON	(SRB_BASE+0x303C)

/**************************************** Interrupt Controller */
#define INTMOD		(SRB_BASE+0x4000)
#define INTPND		(SRB_BASE+0x4004)
#define INTMSK		(SRB_BASE+0x4008)
#define INTPRI0		(SRB_BASE+0x400C)
#define INTPRI1		(SRB_BASE+0x4010)
#define INTPRI2		(SRB_BASE+0x4014)
#define INTPRI3		(SRB_BASE+0x4018)
#define INTPRI4		(SRB_BASE+0x401C)
#define INTPRI5		(SRB_BASE+0x4020)
#define INTOFFSET	(SRB_BASE+0x4024)
#define INTOSET_FIQ	(SRB_BASE+0x4030)
#define INTOSET_IRQ	(SRB_BASE+0x4034)

/**************************************** I/O Ports */
#define IOPMOD		(SRB_BASE+0x5000)
#define IOPCON0		(SRB_BASE+0x5004)
#define IOPDATA		(SRB_BASE+0x5008)
#define IOPCON1		(SRB_BASE+0x500C)

/**************************************** Timers */
#define TMOD		(SRB_BASE+0x6000)
#define TDATA0		(SRB_BASE+0x6004)
#define TDATA1		(SRB_BASE+0x6008)
#define TCNT0		(SRB_BASE+0x600C)
#define TCNT1		(SRB_BASE+0x600C)

/**************************************** HDLC Channel A */
#define HMODE_A		(SRB_BASE+0x7000)
#define HCON_A		(SRB_BASE+0x7004)
#define HSTAT_A		(SRB_BASE+0x7008)
#define HINTEN_A	(SRB_BASE+0x700C)
#define HTXFIFOC_A	(SRB_BASE+0x7010)
#define HTXFIFOT_A	(SRB_BASE+0x7014)
#define HRXFIFO_A	(SRB_BASE+0x7018)
#define HBRGTC_A	(SRB_BASE+0x701C)
#define HTPRMB_A	(SRB_BASE+0x7020)
#define HSAR0_A		(SRB_BASE+0x7024)
#define HSAR1_A		(SRB_BASE+0x7028)
#define HSAR2_A		(SRB_BASE+0x702C)
#define HSAR3_A		(SRB_BASE+0x7030)
#define HMASK_A		(SRB_BASE+0x7034)
#define HDMATxPTR_A	(SRB_BASE+0x7038)
#define HDMARxPTR_A	(SRB_BASE+0x703C)
#define HMFLR_A		(SRB_BASE+0x7040)
#define HRBSR_A		(SRB_BASE+0x7044)
#define HSYNC_A		(SRB_BASE+0x7048)
#define TCON_A		(SRB_BASE+0x704C)

/**************************************** HDLC Channel B */
#define HMODE_B		(SRB_BASE+0x8000)
#define HCON_B		(SRB_BASE+0x8004)
#define HSTAT_B		(SRB_BASE+0x8008)
#define HINTEN_B	(SRB_BASE+0x800C)
#define HTXFIFOC_B	(SRB_BASE+0x8010)
#define HTXFIFOT_B	(SRB_BASE+0x8014)
#define HRXFIFO_B	(SRB_BASE+0x8018)
#define HBRGTC_B	(SRB_BASE+0x801C)
#define HTPRMB_B	(SRB_BASE+0x8020)
#define HSAR0_B		(SRB_BASE+0x8024)
#define HSAR1_B		(SRB_BASE+0x8028)
#define HSAR2_B		(SRB_BASE+0x802C)
#define HSAR3_B		(SRB_BASE+0x8030)
#define HMASK_B		(SRB_BASE+0x8034)
#define HDMATxPTR_B	(SRB_BASE+0x8038)
#define HDMARxPTR_B	(SRB_BASE+0x803C)
#define HMFLR_B		(SRB_BASE+0x8040)
#define HRBSR_B		(SRB_BASE+0x8044)
#define HSYNC_B		(SRB_BASE+0x8048)
#define TCON_B		(SRB_BASE+0x804C)

/**************************************** Ethernet (BDMA) */
#define BDMATXCON	(SRB_BASE+0x9000)
#define BDMARXCON	(SRB_BASE+0x9004)
#define BDMATXPTR	(SRB_BASE+0x9008)
#define BDMARXPTR	(SRB_BASE+0x900C)
#define BDMARXLSZ	(SRB_BASE+0x9010)
#define BDMASTAT	(SRB_BASE+0x9014)
#define ETXSTAT		(SRB_BASE+0x9040) 	/* Transmit control frame status */
#define CAM		(SRB_BASE+0x9100)
#define BDMATXBUF	(SRB_BASE+0x9200)
#define BDMARXBUF	(SRB_BASE+0x9800)

/**************************************** Ethernet (MAC) */
#define MACON		(SRB_BASE+0xA000)  /* MAC control */
#define CAMCON		(SRB_BASE+0xA004)  /* CAM control */
#define MATXCON		(SRB_BASE+0xA008)  /* Transmit control */
#define MATXSTAT	(SRB_BASE+0xA00C)  /* Transmit status */
#define MARXCON		(SRB_BASE+0xA010)  /* Receive control */
#define MARXSTAT	(SRB_BASE+0xA014)  /* Receive status */
#define STADATA		(SRB_BASE+0xA018)  /* Station management data */
#define STACON		(SRB_BASE+0xA01C)  /* Station management control and address */
#define CAMEN		(SRB_BASE+0xA028)  /* CAM enable */
#define EMISSCNT	(SRB_BASE+0xA03C)  /* Missed error count */
#define EPZCNT		(SRB_BASE+0xA040)  /* Pause count */
#define ERMPZCNT	(SRB_BASE+0xA044)  /* Remote pause count */


/**************************************** Ethernet BDMATXCON register map */
#define BTxEn           0x00004000         /* BDMA tx enable */
#define BTxRs           0x00008000         /* BDMA tx reset */

/**************************************** Ethernet BDMARXCON register map */
#define BRxEn           0x00004000         /* BDMA rx enable */
#define BRxRs           0x00008000         /* BDMA rx reset */

/**************************************** Ethernet BDMASTAT register map */
#define BRxRDF          0x00000001         /* BDMA received data frame  */
#define BRxNL           0x00000002         /* BDMA rx null list */
#define BRxNO           0x00000004         /* BDMA rx not owner of current frame */
#define BRxMSO          0x00000008         /* BDMA rx max size over */
#define BRxEmpty        0x00000010         /* BDMA rx buffer empty */
#define BRxEarly        0x00000020         /* BDMA rx early length notify */
/* reserved */
#define BRxFRF          0x00000080         /* BDMA rx early length notify */

#define BRxNFR_mask     0x0000ff00         /* BDMA no. frames in rx buf (mask) */
#define BRxNFR_shift             8         /* (shift) */

#define BTxCCP          0x00010000         /* BDMA tx control packet */
#define BTxNL           0x00020000         /* BDMA tx null list */
#define BTxNO           0x00040000         /* BDMA tx not owner of current frame */
/* reserved */
#define BTxEmpty        0x00100000         /* BDMA tx buffer empty */


/**************************************** Ethernet tx frame descriptor bits */
#define TxCollCnt_mask  0x000f             /* # tx collisions */
#define TxCollCnt_shift      0
#define TxExColl        0x0010             /* 16 collisions occured */
#define TxDefer         0x0020             /* transmission defered */
#define TxPaused        0x0040             /* transmission paused */
#define TxInt           0x0080             /* pkt tx caused an interrupt */
#define TxUnder         0x0100             /* tx underrun */
#define TxMaxDefer      0x0200             /* tx defered for max_defer */
#define TxNCarr         0x0400             /* carrier sense not detected */
#define TxSQErr         0x0800             /* SQE error */
#define TxLateColl      0x1000             /* tx coll after 512 bit times */
#define TxPar           0x2000             /* tx FIFO parity error */
#define TxComp          0x4000             /* tx complete or discarded */
#define TxHalted        0x8000             /* tx halted OR error */


/**************************************** Ethernet MACON register map */
#define SwRESET         0x00000040         /* MAC reset */

/**************************************** Ethernet CAMCON register map */
#define StationAcc      0x00000001         /* Accept Station/Unicast addresses */
#define GroupAcc        0x00000002         /* Accept Group/Multicast addresses */
#define BroadAcc        0x00000004         /* Accept the Broadcast addresses */
#define NegCAM          0x00000008         /* Reject, rather than accept
					    * CAM-recognized entries */
#define CompEn          0x00000010         /* Enable compare mode */

#define HW_MAX_ADDRS    21                 /* CAM consists of 32 words */


/**************************************** GDMA Channel 0 */
#define GDMACON0	(SRB_BASE+0xB000)
#define GDMASRC0	(SRB_BASE+0xB004)
#define GDMADST0	(SRB_BASE+0xB008)
#define GDMACNT0	(SRB_BASE+0xB00C)

/**************************************** GDMA Channel 1 */
#define GDMACON1	(SRB_BASE+0xC000)
#define GDMASRC1	(SRB_BASE+0xC004)
#define GDMADST1	(SRB_BASE+0xC008)
#define GDMACNT1	(SRB_BASE+0xC00C)

/**************************************** UART Channel 0 */
#define UCON0		(SRB_BASE+0xD000)
#define USTAT0		(SRB_BASE+0xD004)
#define UINTEN0		(SRB_BASE+0xD008)
#define UTXBUF0		(SRB_BASE+0xD00C)
#define URXBUF0		(SRB_BASE+0xD010)
#define UBRDIV0		(SRB_BASE+0xD014)
#define UCC1_0		(SRB_BASE+0xD018)
#define UCC2_0		(SRB_BASE+0xD01C)

/**************************************** UART Channel 1 */
#define UCON1		(SRB_BASE+0xE000)
#define USTAT1		(SRB_BASE+0xE004)
#define UINTEN1		(SRB_BASE+0xE008)
#define UTXBUF1		(SRB_BASE+0xE00C)
#define URXBUF1		(SRB_BASE+0xE010)
#define UBRDIV1		(SRB_BASE+0xE014)
#define UCC1_1		(SRB_BASE+0xE018)
#define UCC2_1		(SRB_BASE+0xE01C)

/**************************************** I2C Bus */
#include <asm/arch/i2c.h>


/* FIXME USART0_BASE and USART1_BASE should be set in .config */

   #define USART0_BASE 	UCON0
   #define USART1_BASE 	UCON1


#ifndef __ASSEMBLER__
/****************************************  S3C4530 Samsung  UART registers */
struct s3c4530_usart_regs {
  volatile u_32 cr;		/* control		*/
  volatile u_32 csr;		/* channel status	*/
  volatile u_32 ier;		/* interrupt enable	*/
  volatile u_32 thr;		/* tramsmit holding	*/
  volatile u_32 rhr;		/* receive holding	*/
  volatile u_32 brgr;		/* baud rate generator	*/
  volatile u_32 ucc1;		/* control character 1	*/
  volatile u_32 ucc2;		/* control character 2	*/
};
#endif

#define TX_FIFO_DEPTH 32
#define RX_FIFO_DEPTH 32

/****************************************  UART control register */
#define	U_DISABLE_TMODE	0xFFFFFFFC
#define	U_IRQ_TMODE	0x00000001
#define	U_DMA0_TMODE	0x00000002
#define	U_DMA1_TMODE	0x00000003
#define	U_TMODE		0x00000003

#define	U_DISABLE_RMODE	0xFFFFFFF3
#define	U_IRQ_RMODE	0x00000004
#define	U_DMA0_RMODE	0x00000008
#define	U_DMA1_RMODE	0x0000000C
#define	U_RMODE		0x0000000C

#define	U_SBR		0x00000010
#define	U_UCLK		0x00000020
#define	U_ABRD		0x00000040
#define	U_LOOPB		0x00000080

#define	U_NO_PMD	0xFFFFF8FF
#define	U_ODD_PMD	0x00000400
#define	U_EVEN_PMD	0x00000500
#define	U_FC1_PMD	0x00000600
#define	U_FC0_PMD	0x00000700
#define	U_PMD		0x00000700

#define	U_STB		0x00000800

#define	U_WL5		0xFFFFCFFF
#define	U_WL6		0x00001000
#define	U_WL7		0x00002000
#define	U_WL8		0x00003000
#define	U_WL		0x00003000

#define	U_IR		0x00004000
#define	U_TFEN		0x00010000
#define	U_RFEN		0x00020000
#define	U_TFRST		0x00040000
#define	U_RFRST		0x00080000

#define	U_TFTL30	0xFFCFFFFF
#define	U_TFTL24	0x00100000
#define	U_TFTL16	0x00200000
#define	U_TFTL8		0x00300000
#define	U_TFTL		0x00300000

#define	U_RFTL1		0xFF3FFFFF
#define	U_RFTL8		0x00400000
#define	U_RFTL18	0x00800000
#define	U_RFTL28	0x00C00000
#define	U_RFTL		0x00C00000

#define	U_DTR		0x01000000
#define	U_RTS		0x02000000
#define	U_HFEN		0x10000000
#define	U_SFEN		0x20000000

/****************************************  UART status register */
#define	U_TFFUL		(1<<20)
#define	U_TFEMT		(1<<19)
#define	U_THE		(1<<18)
#define	U_TC		(1<<17)
#define	U_E_CTS		(1<<16)
#define	U_CTS		(1<<15)
#define	U_DSR		(1<<14)
#define	U_E_RxTO	(1<<12)
#define	U_RIDLE		(1<<11
#define	U_RFOV		(1<<10)
#define	U_RFFUL		(1<<9)
#define	U_RFEMT		(1<<8)
#define	U_RFREA		(1<<7)
#define	U_DCD		(1<<6)
#define	U_CCD		(1<<5)
#define	U_OER		(1<<4)
#define	U_PER		(1<<3)
#define	U_FER		(1<<2)
#define	U_BSD		(1<<1)
#define	U_RDV		(1)

/****************************************  UART Interrupt Enable register */
#define	U_THEIE		(1<<18)
#define	U_E_CTSIE	(1<<16)
#define	U_E_RxTOIE	(1<<12)
#define	U_OVFFIE	(1<<10)
#define	U_RFREAIE	(1<<7)
#define	U_DCDLIE	(1<<6)
#define	U_CCDIE		(1<<5)
#define	U_OVEIE		(1<<4)
#define	U_PERIE		(1<<3)
#define	U_FERIE		(1<<2)
#define	U_BKDIE		(1<<1)
#define	U_RDRIE		(1)

#define US_ALL_INTS (U_THEIE|U_E_CTSIE|U_E_RxTOIE|U_OVFFIE|U_RFREAIE|U_DCDLIE|U_CCDIE|U_OERIE|U_PERIE|U_FERIE|U_BSDIE|U_RDVIE)

/****************************************  IO -- IO controller */
#define PIO_BASE	IOPMOD


/**************** Bit definitions for IOPMOD, IOPDATA, IOPCON1 */
#define DTR1_PIN	(1<<25)
#define TxD1_PIN	(1<<24)
#define DSR1_PIN	(1<<23)
#define RxD1_PIN	(1<<22)
#define DTR0_PIN	(1<<21)
#define TxD0_PIN	(1<<20)
#define DSR0_PIN	(1<<19)
#define RxD0_PIN	(1<<18)
#define BZ1_PIN		(1<<17)
#define TOUT1_PIN	(1<<17)
#define TOUT0_PIN	(1<<16)
#define nXDACK1_PIN	(1<<15)
#define nXDACK0_PIN	(1<<14)
#define nXDREQ1_PIN	(1<<13)
#define nXDREQ0_PIN	(1<<12)
#define xINTREQ3_PIN	(1<<11)
#define xINTREQ2_PIN	(1<<10)
#define xINTREQ1_PIN	(1<<9)
#define xINTREQ0_PIN	(1<<8)
#define RTS1_PIN	(1<<7)
#define CTS1_PIN	(1<<6)
#define DCD1_PIN	(1<<5)
#define RTS0_PIN	(1<<4)
#define CTS0_PIN	(1<<3)
#define DCD0_PIN	(1<<2)
#define LED_PIN		(1<<1)

/*************************** Bit definitions for IOPCON0 */

#define xINTREQ0_MODE	(3)	/* 0 = level, 1 = rising, 2 = falling, 3 = both */
#define xINTREQ0_FILTER	(1<<2)	/* assert irq if state holds for 3 clocks */
#define xINTREQ0_ACT_HI	(1<<3)	/* 1 = irq active high */
#define xINTREQ0_ENABLE	(1<<4)	/* 1 = enable irq0 on port 8 */

#define xINTREQ1_MODE	(3<<5)
#define xINTREQ1_FILTER	(1<<7)
#define xINTREQ1_ACT_HI	(1<<8)
#define xINTREQ1_ENABLE	(1<<9)	/* 1 = enable irq1 on port 9 */

#define xINTREQ2_MODE	(3<<10)
#define xINTREQ2_FILTER	(1<<12)
#define xINTREQ2_ACT_HI	(1<<13)
#define xINTREQ2_ENABLE	(1<<14)	/* 1 = enable irq2 on port 10 */

#define xINTREQ3_MODE	(3<<15)
#define xINTREQ3_FILTER	(1<<17)
#define xINTREQ3_ACT_HI	(1<<18)
#define xINTREQ3_ENABLE	(1<<19)	/* 1 = enable irq3 on port 11 */

#define nXDREQ0_ACT_HI	(1<<20)	/* 1 = active high */
#define nXDREQ0_FILTER	(1<<21)	/* 1 = 3 clock filtering */
#define nXDREQ0_ENABLE	(1<<22)	/* 1 = enable drq0 on port 12 */

#define nXDREQ1_ACT_HI	(1<<23)
#define nXDREQ1_FILTER	(1<<24)
#define nXDREQ1_ENABLE	(1<<25)	/* 1 = enable drq1 on port 13 */

#define nXDACK0_ACT_HI	(1<<26)
#define nXDACK0_ENABLE	(1<<27)	/* 1 = enable dack0 on port 14 */

#define nXDACK1_ACT_HI	(1<<28)
#define nXDACK1_ENABLE	(1<<29)	/* 1 = enable dack1 on port 15 */



/****************************************  Timer */

#define TIMER_BASE 	TMOD

/* The  S3C4530A has 2 internel 32-bit timers, TC0 and TC1.
 * One of these must be used to drive the kernel's internal
 * timer (the thing that updates jiffies).  Pick a timer channel
 * here.  */
 
#define	KERNEL_TIMER	0

/*  TC mode register */
#define TCLR1		(1<<5)
#define TDM1		(1<<4)
#define TE1		(1<<3)
#define TCLR0		(1<<2)
#define TDM0		(1<<1)
#define TE0		(1)

#ifndef __ASSEMBLER__
struct s3c4530_timer
{
  unsigned long tmr;        // timers mode register      (RW)
  unsigned long tdr0;       // timer0 data register      (RW)
  unsigned long tdr1;       // timer1 data register      (RW)
  unsigned long tcr0;       // timer0 count register      (RW)
  unsigned long tcr1;       // timer1 count register      (RW)
};
#endif

/****************************************  Interrupt Controller */
#define IC_BASE		INTMOD

#define IC_xIRQ0  	(1<<0)
#define IC_xIRQ1  	(1<<1)
#define IC_xIRQ2  	(1<<2)
#define IC_xIRQ3  	(1<<3)
#define IC_UARTTx0  	(1<<4)
#define IC_UARTRxE0  	(1<<5)
#define IC_UARTTx1  	(1<<6)
#define IC_UARTRxE1  	(1<<7)
#define IC_GDMA0  	(1<<8)
#define IC_GDMA1  	(1<<9)
#define IC_TC0  	(1<<10)
#define IC_TC1  	(1<<11)
#define IC_HDLCATx  	(1<<12)
#define IC_HDLCARx  	(1<<13)
#define IC_HDLCBTx  	(1<<14)
#define IC_HDLCBRx  	(1<<15)
#define IC_BDMATx  	(1<<16)
#define IC_BDMARx  	(1<<17)
#define IC_MACTx  	(1<<18)
#define IC_MACRx  	(1<<19)
#define IC_I2C  	(1<<20)
#define IC_GLOBAL  	(1<<21)

#define _IRQ0		0
#define _IRQ1		1
#define _IRQ2		2
#define _IRQ3		3
#define _URTTx0  	4
#define _URTRx0  	5
#define _URTTx1  	6
#define _URTRx1  	7
#define _GDMA0  	8
#define _GDMA1  	9
#define _TC0  		10
#define _TC1  		11
#define _HDLCATx  	12
#define _HDLCARx  	13
#define _HDLCBTx  	14
#define _HDLCBRx  	15
#define _BDMATx  	16
#define _BDMARx  	17
#define _MACTx  	18
#define _MACRx  	19
#define _I2C  		20
#define _GLOBAL		21

#define HW_ETHERNET_ADDR 0x1007fd0

#endif
