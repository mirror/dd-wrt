/*
 *  linux/include/asm-armnommu/arch-netarm/serial.h
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
 *   05-31-2001 RWP     Modified for Net+ARM
 *   06-14-2003 AES     Altered so it didn't use a definition that conflicts
 *                      with the 16550-oriented serial.c driver.
 */

#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>
#include <asm/arch/netarm_ser_module.h>
#include <linux/termios.h>

#undef SERIAL_DEBUG_OPEN	/* info on open/close */
#undef NAS_DEBUG_VERBOSE	/* even more debug info */
#define SERIAL_NETARM_INLINE 1

/* setup */
#define STD_COM_FLAGS  (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)
#define NR_NAS_PORTS  2				/* NetARM's got 2 serial ports */
#define SERIAL_DEV_OFFSET  0
#define NAS_MAGIC  0x58414c4e			/* NALX (NetArm LinuX) */
#define BASE_BAUD  (NETARM_XTAL_FREQ / 16)	/* actually not needed, just in case... */

/* we might need a port type (for the serial_state struct),
   but make sure it's out of reach of the standard serial driver  */
#define PORT_NETARM	(PORT_MAX + 7)

/* keep in mind that this is the serial driver default setting */
/* there is also a default baud in <asm/arch/netarm_ser_module.h> */
/* which is used by the bootloader */
#define DEFAULT_CFLAGS (B9600 | CS8 | CREAD | HUPCL | CLOCAL)
#define NETARM_SERIAL_PORT_DFNS \
	{  \
	magic: 0, \
        baud_base: BASE_BAUD, \
	port: 1, \
	irq: IRQ_SER1_RX, \
	flags: STD_COM_FLAGS, \
	type: PORT_UNKNOWN, \
	iomem_base: (u8*)NETARM_SER_MODULE_BASE, \
	iomem_reg_shift: 0, \
	io_type: SERIAL_IO_MEM \
	},	/* ttyS0 */ \
	{  \
	magic: 0, \
        baud_base: BASE_BAUD, \
	port: 2, \
	irq: IRQ_SER2_RX, \
	flags: STD_COM_FLAGS, \
	type: PORT_UNKNOWN, \
	iomem_base: (u8*)(NETARM_SER_MODULE_BASE+NETARM_SER_CH2_CTRL_A), \
	iomem_reg_shift: 0, \
	io_type: SERIAL_IO_MEM \
	}	/* ttyS1 */ 
#define EXTRA_SERIAL_PORT_DEFNS

typedef enum { NoMode, TtyMode, CuaMode, SpiMode, HdlcMode } port_mode_t;

struct netarm_async_struct {
	int			magic;
	unsigned long		port;
	port_mode_t		mode;
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
	unsigned long		SCSRA; 	/* Status Register A (at time of last interrupt) */
	unsigned long		event;
	unsigned long		last_active;
	int			line;
	int			blocked_open; /* # of blocked opens */
	long			session; /* Session of opening process */
	long			pgrp; /* pgrp of opening process */
 	struct circ_buf		xmit;
 	spinlock_t		xmit_lock;
	netarm_serial_channel_t	*registers; /* address of control registers */
	int			io_type;
	int			baud;	/* bitrate */
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
};

#define CONFIGURED_NAS_PORT(info) ((info)->port || ((info)->registers))

/* Function prototypes: */
static void nas_rx_interrupt_1(int irq, void *dev_id, struct pt_regs *regs);
static void nas_rx_interrupt_2(int irq, void *dev_id, struct pt_regs *regs);
static void nas_rx_interrupt(struct netarm_async_struct *info, struct pt_regs *regs);

#endif
