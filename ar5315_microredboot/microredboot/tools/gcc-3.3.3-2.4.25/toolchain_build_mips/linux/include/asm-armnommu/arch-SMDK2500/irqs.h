/*
 * irqs.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the interupt controller definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */
 
#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

#define	INTMOD		0xF0140000	/* Internal interrupt mode R */
#define	EXTMOD		0xF0140004	/* External interrupt mode R */
#define	INTMASK		0xF0140008	/* Internal interrupt mask R */
#define	EXTMASK		0xF014000C	/* External interrupt mask R */
#define IPRIORHI	0xF0140010	/* HIgh bits, 5-0 bit, Interrupt by priority R */
#define IPRIORLO	0xF0140014	/* LOw bits, 31-0 bit, Interrupt by priority R */
#define INTOFFSET_FIQ	0xF0140018	/* FIQ interrupt offset R */
#define INTOFFSET_IRQ	0xF014001C	/* IRQ interrupt offset R */
#define INTPRIOR0	0xF0140020	/* Interrupt priority R 0 */
#define INTPRIOR1	0xF0140024	/* Interrupt priority R 1 */
#define INTPRIOR2	0xF0140028	/* Interrupt priority R 2 */
#define INTPRIOR3	0xF014002C	/* Interrupt priority R 3 */
#define INTPRIOR4	0xF0140030	/* Interrupt priority R 4 */
#define INTPRIOR5	0xF0140034	/* Interrupt priority R 5 */
#define INTPRIOR6	0xF0140038	/* Interrupt priority R 6 */
#define INTPRIOR7	0xF014003C	/* Interrupt priority R 7 */
#define INTPRIOR8	0xF0140040	/* Interrupt priority R 8 */
#define INTPRIOR9	0xF0140044	/* Interrupt priority R 9 */
#define INTTSTHI	0xF0140048	/* HIgh bits, 5-0 bit, Interrupt test R */
#define INTTSTLO	0xF014004C	/* LOw bits, 31-0 bit, Interrupt test R */


#define	NR_IRQS		39		/* There are 39 sources of the interrupts */

#define	SRC_IRQ_0		0
#define	SRC_IRQ_1		1
#define	SRC_IRQ_2		2
#define	SRC_IRQ_3		3
#define	SRC_IRQ_4		4
#define	SRC_IRQ_5		5
#define	SRC_IRQ_IOM2		6
#define	SRC_IRQ_IICC		7
#define	SRC_IRQ_HUART0_TX	8
#define	SRC_IRQ_HUART0_RX	9
#define	SRC_IRQ_HUART1_TX	10
#define	SRC_IRQ_HUART1_RX	11
#define	SRC_IRQ_CUART_TX	12
#define	SRC_IRQ_CUART_RX	13
#define	SRC_IRQ_USB		14
#define	SRC_IRQ_HDLC_TX0	15
#define	SRC_IRQ_HDLC_RX0	16
#define	SRC_IRQ_HDLC_TX1	17
#define	SRC_IRQ_HDLC_RX1	18
#define	SRC_IRQ_HDLC_TX2	19
#define	SRC_IRQ_HDLC_RX2	20
#define	SRC_IRQ_ETH_TX0		21
#define	SRC_IRQ_ETH_RX0		22
#define	SRC_IRQ_ETH_TX1		23
#define	SRC_IRQ_ETH_RX1		24
#define	SRC_IRQ_DES		25
#define	SRC_IRQ_GDMA0		26
#define	SRC_IRQ_GDMA1		27
#define	SRC_IRQ_GDMA2		28
#define	SRC_IRQ_GDMA3		29
#define	SRC_IRQ_GDMA4		30
#define	SRC_IRQ_GDMA5		31
#define	SRC_IRQ_TIMER0		32
#define	SRC_IRQ_TIMER1		33
#define	SRC_IRQ_TIMER2		34
#define	SRC_IRQ_TIMER3		35
#define	SRC_IRQ_TIMER4		36
#define	SRC_IRQ_TIMER5		37
#define	SRC_IRQ_TIMER_WD	38


#define NR_INT_IRQS	32		/* Internal interrupt sources number */

#define	INT_IRQ_IICC		0
#define	INT_IRQ_HUART0_TX	1
#define	INT_IRQ_HUART0_RX	2
#define	INT_IRQ_HUART1_TX	3
#define	INT_IRQ_HUART1_RX	4
#define	INT_IRQ_CUART_TX	5
#define	INT_IRQ_CUART_RX	6
#define	INT_IRQ_USB		7
#define	INT_IRQ_HDLC_TX0	8
#define	INT_IRQ_HDLC_RX0	9
#define	INT_IRQ_HDLC_TX1	10
#define	INT_IRQ_HDLC_RX1	11
#define	INT_IRQ_HDLC_TX2	12
#define	INT_IRQ_HDLC_RX2	13
#define	INT_IRQ_ETH_TX0		14
#define	INT_IRQ_ETH_RX0		15
#define	INT_IRQ_ETH_TX1		16
#define	INT_IRQ_ETH_RX1		17
#define	INT_IRQ_DES		18
#define	INT_IRQ_GDMA0		19
#define	INT_IRQ_GDMA1		20
#define	INT_IRQ_GDMA2		21
#define	INT_IRQ_GDMA3		22
#define	INT_IRQ_GDMA4		23
#define	INT_IRQ_GDMA5		24
#define	INT_IRQ_TIMER0		25
#define	INT_IRQ_TIMER1		26
#define	INT_IRQ_TIMER2		27
#define	INT_IRQ_TIMER3		28
#define	INT_IRQ_TIMER4		29
#define	INT_IRQ_TIMER5		30
#define	INT_IRQ_TIMER_WD	31

#define	MASK_IRQ_IICC		0x00000001
#define	MASK_IRQ_HUART0_TX	0x00000002
#define	MASK_IRQ_HUART0_RX	0x00000004
#define	MASK_IRQ_HUART1_TX	0x00000008
#define	MASK_IRQ_HUART1_RX	0x00000010
#define	MASK_IRQ_CUART_TX	0x00000020
#define	MASK_IRQ_CUART_RX	0x00000040
#define	MASK_IRQ_USB		0x00000080
#define	MASK_IRQ_HDLC_TX0	0x00000100
#define	MASK_IRQ_HDLC_RX0	0x00000200
#define	MASK_IRQ_HDLC_TX1	0x00000400
#define	MASK_IRQ_HDLC_RX1	0x00000800
#define	MASK_IRQ_HDLC_TX2	0x00001000
#define	MASK_IRQ_HDLC_RX2	0x00002000
#define	MASK_IRQ_ETH_TX0	0x00004000
#define	MASK_IRQ_ETH_RX0	0x00008000
#define	MASK_IRQ_ETH_TX1	0x00010000
#define	MASK_IRQ_ETH_RX1	0x00020000
#define	MASK_IRQ_DES		0x00040000
#define	MASK_IRQ_GDMA0		0x00080000
#define	MASK_IRQ_GDMA1		0x00100000
#define	MASK_IRQ_GDMA2		0x00200000
#define	MASK_IRQ_GDMA3		0x00400000
#define	MASK_IRQ_GDMA4		0x00800000
#define	MASK_IRQ_GDMA5		0x01000000
#define	MASK_IRQ_TIMER0		0x02000000
#define	MASK_IRQ_TIMER1		0x04000000
#define	MASK_IRQ_TIMER2		0x08000000
#define	MASK_IRQ_TIMER3		0x10000000
#define	MASK_IRQ_TIMER4		0x20000000
#define	MASK_IRQ_TIMER5		0x40000000
#define	MASK_IRQ_TIMER_WD	0x80000000


#define	NR_EXT_IRQS	7		/* External interrupt sources number */

#define	EXT_IRQ_0		0
#define	EXT_IRQ_1		1
#define	EXT_IRQ_2		2
#define	EXT_IRQ_3		3
#define	EXT_IRQ_4		4
#define	EXT_IRQ_5		5
#define	EXT_IRQ_IOM2		6

#define	MASK_IRQ_0		0x00000001
#define	MASK_IRQ_1		0x00000002
#define	MASK_IRQ_2		0x00000004
#define	MASK_IRQ_3		0x00000008
#define	MASK_IRQ_4		0x00000010
#define	MASK_IRQ_5		0x00000020
#define	MASK_IRQ_IOM2		0x00000040
#define	MASK_IRQ_GLOBAL		0x80000000

	
#endif	/* __ASM_ARCH_IRQS_H */
