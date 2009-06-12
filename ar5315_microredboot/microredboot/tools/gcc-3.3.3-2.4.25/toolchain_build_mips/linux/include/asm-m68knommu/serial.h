/*
 * linux/include/asm-m68knommu/serial.h
 *
 * Copyright (C) 2003 Develer S.r.l. (http://www.develer.com/)
 * Author: Bernardo Innocenti <bernie@codewiz.org>
 *
 * Based on linux/include/asm-i386/serial.h
 */
#include <linux/config.h>

/*
 * This assumes you have a 1.8432 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires the faster clock.
 */
#define BASE_BAUD ( 1843200 / 16 )

#ifdef CONFIG_SERIAL_CDB4

#define CONFIG_SERIAL_SHARE_IRQ

#define CDB4_COM_BASE		((u8 *)0x40000000)
#define CDB4_COM_IRQ		67	/* external IRQ3 */
#define CDB4_COM_IRQPRI		5	/* interrupt priority */
#define CDB4_COM_RESET_BIT	13	/* PA13 hooked to 16C554 RESET line, active high */
#define STD_COM_FLAGS		ASYNC_BOOT_AUTOCONF

#define SERIAL_PORT_DFNS				\
	{						\
		.baud_base	= BASE_BAUD,		\
		.iomem_base	= CDB4_COM_BASE + 0x10,	\
		.io_type	= SERIAL_IO_MEM,	\
		.irq		= CDB4_COM_IRQ,		\
		.flags		= STD_COM_FLAGS		\
	},						\
	{						\
		.baud_base	= BASE_BAUD,		\
		.iomem_base	= CDB4_COM_BASE + 0x18,	\
		.io_type	= SERIAL_IO_MEM,	\
		.irq		= CDB4_COM_IRQ,		\
		.flags		= STD_COM_FLAGS		\
	},						\
	{						\
		.baud_base	= BASE_BAUD,		\
		.iomem_base	= CDB4_COM_BASE + 0x20,	\
		.io_type	= SERIAL_IO_MEM,	\
		.irq		= CDB4_COM_IRQ,		\
		.flags		= STD_COM_FLAGS		\
	},						\
	{						\
		.baud_base	= BASE_BAUD,		\
		.iomem_base	= CDB4_COM_BASE + 0x28,	\
		.io_type	= SERIAL_IO_MEM,	\
		.irq		= CDB4_COM_IRQ,		\
		.flags		= STD_COM_FLAGS		\
	}

#define RS_TABLE_SIZE  4

#else /* !CONFIG_SERIAL_CDB4 */

#error serial port not supported on this board

#endif /* !CONFIG_SERIAL_CDB4 */

