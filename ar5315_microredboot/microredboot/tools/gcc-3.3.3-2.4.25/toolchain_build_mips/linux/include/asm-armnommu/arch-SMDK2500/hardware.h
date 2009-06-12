/*
 * hardware.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the hardware definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */
 
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>

#ifndef __ASSEMBLER__
extern	unsigned int CONFIG_ARM_CLK;
#endif

#define PCLK		CONFIG_ARM_CLK

#define MCLK2		(PCLK >> 1)

#define Vol32	*(volatile unsigned int *)
#define Vol16	*(volatile unsigned short *)
#define Vol08	*(volatile unsigned char *)

#define REG_WRITE(addr,data)	(Vol32(addr)=(data))
#define REG_READ(addr)		(Vol32(addr))

#define HARD_RESET_NOW()

/*
---	Memory map
*/
#define BASE_MEM_BANK0	0x00000000	/* EXT I/O Bank 0 base address */
#define BASE_MEM_BANK1	0x01000000	/* EXT I/O Bank 1 base address */
#define BASE_MEM_BANK2	0x02000000	/* EXT I/O Bank 2 base address */
#define BASE_MEM_BANK3	0x03000000	/* EXT I/O Bank 3 base address */
#define BASE_MEM_BANK4	0x04000000	/* EXT I/O Bank 4 base address */
#define BASE_MEM_BANK5	0x05000000	/* EXT I/O Bank 5 base address */
#define BASE_MEM_BANK6	0x06000000	/* EXT I/O Bank 6 base address */
#define BASE_MEM_BANK7	0x07000000	/* EXT I/O Bank 7 base address */
#define BASE_SDRAM_BANK0 0x40000000	/* SDRAM   Bank 0 base address */
#define BASE_SDRAM_BANK1 0x80000000	/* SDRAM   Bank 1 base address */

#define MAP_MEM_BANK0	0x80000000	/* EXT I/O Bank 0 remaped address */
#define MAP_MEM_BANK1	0x81000000	/* EXT I/O Bank 1 remaped address */
#define MAP_MEM_BANK2	0x82000000	/* EXT I/O Bank 2 remaped address */
#define MAP_MEM_BANK3	0x83000000	/* EXT I/O Bank 3 remaped address */
#define MAP_MEM_BANK4	0x84000000	/* EXT I/O Bank 4 remaped address */
#define MAP_MEM_BANK5	0x85000000	/* EXT I/O Bank 5 remaped address */
#define MAP_MEM_BANK6	0x86000000	/* EXT I/O Bank 6 remaped address */
#define MAP_MEM_BANK7	0x87000000	/* EXT I/O Bank 7 remaped address */
#define MAP_SDRAM_BANK0	0x00000000	/* SDRAM   Bank 0 remaped address */
#define MAP_SDRAM_BANK1	0x04000000	/* SDRAM   Bank 1 remaped address */

#define	BASE_REG	0xF0000000	/* Internal register base address */


/*	
---	System configuration special registers
*/
#define	SYSCFG		0xF0000000	/* System configuration R */
#define	PDCODE		0xF0000004	/* Product code and revision number R */
#define CLKCON		0xF0000008	/* System clock control R */
#define PCLKDIS		0xF000000C	/* Pirepheral clock disable R */
#define CLKST		0xF0000010	/* Clock status R */
#define HPRIF		0xF0000014	/* AHB bus master fixed priority R */
#define HPRIR		0xF0000018	/* AHB bus master round-robin priority R */
#define CPLL		0xF000001C	/* Core PLL configuration R */
#define SPLL		0xF0000020	/* System PLL configuration R */
#define UPLL		0xF0000024	/* USB PLL configuration R */
#define PPLL		0xF0000028	/* PHY PLL configuration R */

/*
---	External I/O bank controller
*/
#define	B0CON		0xF0010000	/* Bank 0 control R */
#define	B1CON		0xF0010004	/* Bank 1 control R */
#define	B2CON		0xF0010008	/* Bank 2 control R */
#define	B3CON		0xF001000C	/* Bank 3 control R */
#define	B4CON		0xF0010010	/* Bank 4 control R */
#define	B5CON		0xF0010014	/* Bank 5 control R */
#define	B6CON		0xF0010018	/* Bank 6 control R */
#define	B7CON		0xF001001C	/* Bank 7 control R */
#define	MUXBCON		0xF0010020	/* Muxed bus control R */
#define	WAITCON		0xF0010024	/* Wait control R */

/*
---	SDRAM controller
*/
#define	RegO		0xF0020000	/* Configuration R 0 */
#define	Reg1		0xF0020004	/* Configuration R 1 */
#define	Reg3		0xF0020008	/* Refresh timer R */
#define	Reg4		0xF002000C	/* Write buffer time-out R*/

/*
---	HDLC controller
*/
#define HMODE_A		0xF0100000	/* HDLC Channel A mode */
#define HMODE_B		0xF0110000	/* HDLC Channel B mode */
#define HMODE_C		0xF0120000	/* HDLC Channel C mode */

/*
---	Interrupt controller
*/
#include <asm/arch/irqs.h>

/*
---	Timers
*/
#define	ARM_CLK	CONFIG_ARM_CLK
#include <asm/arch/timers.h>

/*
---	I/O ports
*/
#include <asm/arch/ios.h>

/*
---	UART controller
*/
#include <asm/arch/uart.h>

/*
---	DMA controller
*/
#include <asm/arch/dma.h>

/*
---	I2C BUS controller
*/
#include <asm/arch/i2c.h>

	
#endif	/* __ASM_ARCH_HARDWARE_H */
