/****************************************************************************/

/*
 *	m5249sim.h -- ColdFire 5249 System Integration Module support.
 *
 *	(C) Copyright 2002, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef	m5249sim_h
#define	m5249sim_h
/****************************************************************************/

/*
 *	Define the 5249 SIM register set addresses.
 */
#define	MCFSIM_RSR		0x00		/* Reset Status reg (r/w) */
#define	MCFSIM_SYPCR		0x01		/* System Protection reg (r/w)*/
#define	MCFSIM_SWIVR		0x02		/* SW Watchdog intr reg (r/w) */
#define	MCFSIM_SWSR		0x03		/* SW Watchdog service (r/w) */
#define	MCFSIM_PAR		0x04		/* Pin Assignment reg (r/w) */
#define	MCFSIM_IRQPAR		0x06		/* Interrupt Assignment reg (r/w) */
#define	MCFSIM_MPARK		0x0C		/* BUS Master Control Reg*/
#define	MCFSIM_IPR		0x40		/* Interrupt Pend reg (r/w) */
#define	MCFSIM_IMR		0x44		/* Interrupt Mask reg (r/w) */
#define	MCFSIM_AVR		0x4b		/* Autovector Ctrl reg (r/w) */
#define	MCFSIM_ICR0		0x4c		/* Intr Ctrl reg 0 (r/w) */
#define	MCFSIM_ICR1		0x4d		/* Intr Ctrl reg 1 (r/w) */
#define	MCFSIM_ICR2		0x4e		/* Intr Ctrl reg 2 (r/w) */
#define	MCFSIM_ICR3		0x4f		/* Intr Ctrl reg 3 (r/w) */
#define	MCFSIM_ICR4		0x50		/* Intr Ctrl reg 4 (r/w) */
#define	MCFSIM_ICR5		0x51		/* Intr Ctrl reg 5 (r/w) */
#define	MCFSIM_ICR6		0x52		/* Intr Ctrl reg 6 (r/w) */
#define	MCFSIM_ICR7		0x53		/* Intr Ctrl reg 7 (r/w) */
#define	MCFSIM_ICR8		0x54		/* Intr Ctrl reg 8 (r/w) */
#define	MCFSIM_ICR9		0x55		/* Intr Ctrl reg 9 (r/w) */
#define	MCFSIM_ICR10		0x56		/* Intr Ctrl reg 10 (r/w) */
#define	MCFSIM_ICR11		0x57		/* Intr Ctrl reg 11 (r/w) */

#define MCFSIM_CSAR0		0x80		/* CS 0 Address 0 reg (r/w) */
#define MCFSIM_CSMR0		0x84		/* CS 0 Mask 0 reg (r/w) */
#define MCFSIM_CSCR0		0x8a		/* CS 0 Control reg (r/w) */
#define MCFSIM_CSAR1		0x8c		/* CS 1 Address reg (r/w) */
#define MCFSIM_CSMR1		0x90		/* CS 1 Mask reg (r/w) */
#define MCFSIM_CSCR1		0x96		/* CS 1 Control reg (r/w) */
#define MCFSIM_CSAR2		0x98		/* CS 2 Adress reg (r/w) */
#define MCFSIM_CSMR2		0x9c		/* CS 2 Mask reg (r/w) */
#define MCFSIM_CSCR2		0xa2		/* CS 2 Control reg (r/w) */
#define MCFSIM_CSAR3		0xa4		/* CS 3 Adress reg (r/w) */
#define MCFSIM_CSMR3		0xa8		/* CS 3 Mask reg (r/w) */
#define MCFSIM_CSCR3		0xae		/* CS 3 Control reg (r/w) */

#define MCFSIM_DCR		0x100		/* DRAM Control reg (r/w) */
#define MCFSIM_DACR0		0x108		/* DRAM 0 Addr and Ctrl (r/w) */
#define MCFSIM_DMR0		0x10c		/* DRAM 0 Mask reg (r/w) */
#define MCFSIM_DACR1		0x110		/* DRAM 1 Addr and Ctrl (r/w) */
#define MCFSIM_DMR1		0x114		/* DRAM 1 Mask reg (r/w) */


/*
 *	Some symbol defines for the above...
 */
#define	MCFSIM_SWDICR		MCFSIM_ICR0	/* Watchdog timer ICR */
#define	MCFSIM_TIMER1ICR	MCFSIM_ICR1	/* Timer 1 ICR */
#define	MCFSIM_TIMER2ICR	MCFSIM_ICR2	/* Timer 2 ICR */
#define	MCFSIM_UART1ICR		MCFSIM_ICR4	/* UART 1 ICR */
#define	MCFSIM_UART2ICR		MCFSIM_ICR5	/* UART 2 ICR */
#define	MCFSIM_DMA0ICR		MCFSIM_ICR6	/* DMA 0 ICR */
#define	MCFSIM_DMA1ICR		MCFSIM_ICR7	/* DMA 1 ICR */
#define	MCFSIM_DMA2ICR		MCFSIM_ICR8	/* DMA 2 ICR */
#define	MCFSIM_DMA3ICR		MCFSIM_ICR9	/* DMA 3 ICR */

/*
 *	General purpose IO registers (in MBAR2).
 */
#define	MCFSIM2_GPIOREAD	0x0		/* GPIO read values */
#define	MCFSIM2_GPIOWRITE	0x4		/* GPIO write values */
#define	MCFSIM2_GPIOENABLE	0x8		/* GPIO enabled */
#define	MCFSIM2_GPIOFUNC	0xc		/* GPIO function */
#define	MCFSIM2_GPIO1READ	0xb0		/* GPIO1 read values */
#define	MCFSIM2_GPIO1WRITE	0xb4		/* GPIO1 write values */
#define	MCFSIM2_GPIO1ENABLE	0xb8		/* GPIO1 enabled */
#define	MCFSIM2_GPIO1FUNC	0xbc		/* GPIO1 function */

#define	MCFSIM2_GPIOINTSTAT	0xc0		/* GPIO interrupt status */
#define	MCFSIM2_GPIOINTCLEAR	0xc0		/* GPIO interrupt clear */
#define	MCFSIM2_GPIOINTENABLE	0xc4		/* GPIO interrupt enable */

#define	MCFSIM2_INTLEVEL1	0x140		/* Interrupt level reg 1 */
#define	MCFSIM2_INTLEVEL2	0x144		/* Interrupt level reg 2 */
#define	MCFSIM2_INTLEVEL3	0x148		/* Interrupt level reg 3 */
#define	MCFSIM2_INTLEVEL4	0x14c		/* Interrupt level reg 4 */
#define	MCFSIM2_INTLEVEL5	0x150		/* Interrupt level reg 5 */
#define	MCFSIM2_INTLEVEL6	0x154		/* Interrupt level reg 6 */
#define	MCFSIM2_INTLEVEL7	0x158		/* Interrupt level reg 7 */
#define	MCFSIM2_INTLEVEL8	0x15c		/* Interrupt level reg 8 */

#define	MCFSIM2_DMAROUTE	0x188		/* DMA routing */

#define	MCFSIM2_IDECONFIG1	0x18c		/* IDEconfig1 */
#define	MCFSIM2_IDECONFIG2	0x190		/* IDEconfig2 */


/*
 *	Macro to set IMR register. It is 32 bits on the 5249.
 */
#define	MCFSIM_IMR_MASKALL	0x7fffe		/* All SIM intr sources */

#define	mcf_getimr()		\
	*((volatile unsigned long *) (MCF_MBAR + MCFSIM_IMR))

#define	mcf_setimr(imr)		\
	*((volatile unsigned long *) (MCF_MBAR + MCFSIM_IMR)) = (imr);

#define	mcf_getipr()		\
	*((volatile unsigned long *) (MCF_MBAR + MCFSIM_IPR))

/****************************************************************************/
#endif	/* m5249sim_h */
