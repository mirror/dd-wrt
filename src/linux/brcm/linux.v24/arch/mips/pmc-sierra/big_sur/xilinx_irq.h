/*
 * Copyright 2003 PMC-Sierra
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * Board specific definititions for the PMC-Sierra Big Sur
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __XILINX_IRQ_H__
#define	__XILINX_IRQ_H__

#define BIG_SUR_BASE	0xfb000000

#define BIG_SUR_WRITE(ofs, data)	\
	*(volatile u32 *)(BIG_SUR_BASE + ofs) = data

#define BIG_SUR_READ(ofs)	*(volatile u32 *)(BIG_SUR_BASE + ofs)

#define	BIG_SUR_INTERRUPT_MASK_1	0x0c00
#define	BIG_SUR_INTERRUPT_STATUS_1	0x0d00
#define	BIG_SUR_UART1_IRQ
#define	BIG_SUR_UART2_IRQ
#define	BIG_SUR_TIMER_IRQ
#define	BIG_SUR_PCI_IRQ
#define	BIG_SUR_IDE_IRQ

#endif
