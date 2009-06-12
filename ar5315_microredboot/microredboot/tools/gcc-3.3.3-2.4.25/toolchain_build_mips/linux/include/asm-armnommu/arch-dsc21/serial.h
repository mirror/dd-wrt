/*
 *  linux/include/asm-arm/arch-arc/serial.h
 *
 *  Copyright (C) 1996 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Changelog:
 *   15-10-1996	RMK	Created
 *   04-04-1998	PJB	Merged `arc' and `a5k' architectures
 *   02-28-2001 GJM     Modified for dsc21
 */
#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>

/* setup */
#define BASE_BAUD      (DSC21_CLOCK_RATE / 16)
#define STD_COM_FLAGS  (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)
#define RS_TABLE_SIZE  2
#define STD_SERIAL_PORT_DEFNS \
	{  \
	magic: 0, \
        baud_base: BASE_BAUD, \
	port: 0, \
	irq: DSC21_IRQ_UART0, \
	flags: STD_COM_FLAGS, \
	type: PORT_UNKNOWN, \
	iomem_base: (u8*)DSC21_UART0, \
	iomem_reg_shift: 1, \
	io_type: SERIAL_IO_MEM \
	},	/* ttyS0 */ \
	{  \
	magic: 0, \
        baud_base: BASE_BAUD, \
	port: 0, \
	irq: DSC21_IRQ_UART1, \
	flags: STD_COM_FLAGS, \
	type: PORT_UNKNOWN, \
	iomem_base: (u8*)DSC21_UART1, \
	iomem_reg_shift: 1, \
	io_type: SERIAL_IO_MEM \
	}	/* ttyS1 */ 
#define EXTRA_SERIAL_PORT_DEFNS


/* registers */
#define UART_DTRR  0  /* Data Transmission/Reception Register */
#define UART_BRSR  1  /* Bit Rate Set Register */
#define UART_MSR   2  /* Mode Set Register */
#define UART_RFCR  3  /* Reception FIFO Control Register */
#define UART_TFCR  4  /* Transmission FIFO Control Register */
#define UART_LCR   5  /* Line Control Register */
#define UART_SR    6  /* Status Register */

/* bitmasks */
#define UART_MSR_CLS    0x0001    // Character length select (1=7bit, 0=8bit)
#define UART_MSR_RSVD0  0x0002    // Reserved
#define UART_MSR_SBLD   0x0004    // Stop bit length select (1=2bits, 0=1bit)
#define UART_MSR_PSB 	0x0008	  // Parity select bit (1=odd, 0=even)
#define UART_MSR_PEB    0x0010	  // Parity enable bit (1=enable, 0=disable)
#define UART_MSR_TFTIE  0x4000    // Transmission FIFO trigger interrupt enable
#define UART_MSR_RFTIE  0x8000    // Reception FIFO trigger interrupt enable
#define UART_MSR_INTS   0xfc00    // All interrupt bits
#define UART_MSR_MODE_BITS 0x001f // data length, stop & parity

#define UART_SR_TREF    0x0001    // Transmission register empty flag
#define UART_SR_TFEF    0x0002    // Transmission FIFO empty flag
#define UART_SR_RFNEF   0x0004    // Reception FIFO not empty flag
#define UART_SR_RSVD0   0x00f8    // Reserved
#define UART_SR_TOIF    0x0100    // Time out interrupt flag
#define UART_SR_RFER    0x0200    // Reception FIFO error flag
#define UART_SR_TFTI    0x0400    // Transmission FIFO trigger indicator
#define UART_SR_RFTI    0x0800    // Reception FIFO trigger indicator
#define UART_SR_RSVD1   0xf000    // Reserved

#define UART_LCR_BOC    0x0100    // Break output control (1=send break)

#define UART_FCR_FTL1   0x0000    /* FIFO trigger level 1 byte */
#define UART_FCR_FTL4   0x0100    /* FIFO trigger level 4 bytes */
#define UART_FCR_FTL8   0x0200    /* FIFO trigger level 8 bytes */
#define UART_FCR_FTL16  0x0300    /* FIFO trigger level 16 bytes */
#define UART_FCR_FTL24  0x0400    /* FIFO trigger level 24 bytes */
#define UART_FCR_FTL32  0x0500    /* FIFO trigger level 32 bytes */

struct dsc21_async_struct {
	int			magic;
	unsigned long		port;
	int			flags;
	int			xmit_fifo_size;
	struct serial_state	*state;
	struct tty_struct 	*tty;
	int			timeout;
	int			quot;
	int			x_char;	/* xon/xoff character */
	int			close_delay;
	unsigned short		closing_wait;
	unsigned short		closing_wait2;
	u16			MSR; 	/* Interrupt Enable Register */
	u16			LCR; 	/* Line control register */
	u16                     RFCR;
	u16                     TFCR;
	unsigned long		event;
	unsigned long		last_active;
	int			line;
	int			blocked_open; /* # of blocked opens */
	long			session; /* Session of opening process */
	long			pgrp; /* pgrp of opening process */
 	struct circ_buf		xmit;
 	spinlock_t		xmit_lock;
	u8			*iomem_base;
	u16			iomem_reg_shift;
	int			io_type;
	struct tq_struct	tqueue;
#ifdef DECLARE_WAITQUEUE
	wait_queue_head_t	open_wait;
	wait_queue_head_t	close_wait;
	wait_queue_head_t	delta_msr_wait;
#else	
	struct wait_queue	*open_wait;
	struct wait_queue	*close_wait;
	struct wait_queue	*delta_msr_wait;
#endif	
	struct dsc21_async_struct *next_port;
	struct dsc21_async_struct *prev_port;
	
};



#endif
