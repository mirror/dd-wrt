/*
 * uart.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the Serial I/O definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */
 
#ifndef __ASM_ARCH_UART_H
#define __ASM_ARCH_UART_H

 /*
 *	Console UART special registers
*/
#define CUCON		0xF0060000
#define CUSTAT		0xF0060004
#define CUINT		0xF0060008
#define CUTXBUF		0xF006000C
#define CURXBUF		0xF0060010
#define CUBRD		0xF0060014
#define CUCHAR1		0xF0060018
#define CUCHAR2		0xF006001C

 /*
 *	High-speed UART special registers
*/
#define HUCON0		0xF0070000
#define HUSTAT0		0xF0070004
#define HUINT0		0xF0070008
#define HUTXBUF0	0xF007000C
#define HURXBUF0	0xF0070010
#define HUBRD0		0xF0070014
#define HUCHAR10	0xF0070018
#define UUCHAR20	0xF007001C
#define HUABB0		0xF0070100
#define HUABT0		0xF0070104

#define HUCON1		0xF0080000
#define HUSTAT1		0xF0080004
#define HUINT1		0xF0080008
#define HUTXBUF1	0xF008000C
#define HURXBUF1	0xF0080010
#define HUBRD1		0xF0080014
#define HUCHAR11	0xF0080018
#define UUCHAR21	0xF008001C
#define HUABB1		0xF0080100
#define HUABT1		0xF0080104

#define CUART_BASE 	CUCON
#define HUART0_BASE 	HUCON0
#define HUART1_BASE 	HUCON1


#ifndef __ASSEMBLER__
/****************************************  S3C2500 Samsung  UART registers */
struct s3c2500_usart_regs{
	unsigned int cr;		/* control		*/
	unsigned int csr;		/* channel status	*/
	unsigned int ier;		/* interrupt enable	*/
	unsigned int thr;		/* tramsmit holding	*/
	unsigned int rhr;		/* receive holding	*/
	unsigned int brgr;		/* baud rate generator	*/
	unsigned int ucc1;		/* contorl character 1	*/
	unsigned int ucc2;		/* contorl character 2	*/
};
#endif

/****************************************  UART control register */
#define	U_DISABLE_TMODE	0xFFFFFFFC
#define	U_IRQ_TMODE	0x00000001
#define	U_GDMA_TMODE	0x00000002
#define	U_TMODE		0x00000003

#define	U_DISABLE_RMODE	0xFFFFFFF3
#define	U_IRQ_RMODE	0x00000004
#define	U_GDMA_RMODE	0x00000008
#define	U_RMODE		0x0000000C

#define	U_SBR		0x00000010
#define	U_UCLK		0x00000020
#define	U_AUBD		0x00000040
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
#define	U_ECHO		0x40000000
#define	U_RTSRTR	0x80000000

/****************************************  UART status register */
#define	U_TFFUL		(1<<20)
#define	U_TFEMT		(1<<19)
#define	U_THE		(1<<18)
#define	U_TI		(1<<17)
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
#define	U_TIIE		(1<<17)
#define	U_E_CTSIE	(1<<16)
#define	U_E_RxTOIE	(1<<12)
#define	U_OVFFIE	(1<<10)
#define	U_RFREAIE	(1<<7)
#define	U_DCDLIE	(1<<6)
#define	U_CCDIE		(1<<5)
#define	U_OERIE		(1<<4)
#define	U_PERIE		(1<<3)
#define	U_FERIE		(1<<2)
#define	U_BSDIE		(1<<1)
#define	U_RDVIE		(1)

#endif
