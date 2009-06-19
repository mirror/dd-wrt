/*
 *  uncompress-ar531x.h
 *
 *  Copyright (C) 2003 Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __UNCOMPRESS_H
#define __UNCOMPRESS_H

#include <linux/autoconf.h>

#ifndef CONFIG_DEFAULT_BAUDRATE
#define CONFIG_DEFAULT_BAUDRATE	115200
#endif

#define DEFAULT_DATA_FORMAT	UART16550_DATA_8BIT | \
				UART16550_PARITY_NONE | \
				UART16550_STOP_1BIT

#define UART			0xB1100003

#define REG_OFFSET		4
#define OFS_RCV_BUFFER          (0*REG_OFFSET)
#define OFS_TRANS_HOLD          (0*REG_OFFSET)
#define OFS_SEND_BUFFER         (0*REG_OFFSET)
#define OFS_INTR_ENABLE         (1*REG_OFFSET)
#define OFS_INTR_ID             (2*REG_OFFSET)
#define OFS_DATA_FORMAT         (3*REG_OFFSET)
#define OFS_LINE_CONTROL        (3*REG_OFFSET)
#define OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define OFS_LINE_STATUS         (5*REG_OFFSET)
#define OFS_MODEM_STATUS        (6*REG_OFFSET)
#define OFS_RS232_INPUT         (6*REG_OFFSET)
#define OFS_SCRATCH_PAD         (7*REG_OFFSET)
#define OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define UART16550_PARITY_NONE	0
#define UART16550_PARITY_ODD    0x08
#define UART16550_PARITY_EVEN   0x18
#define UART16550_PARITY_MARK   0x28
#define UART16550_PARITY_SPACE  0x38

#define UART16550_DATA_5BIT     0x0
#define UART16550_DATA_6BIT     0x1
#define UART16550_DATA_7BIT     0x2
#define UART16550_DATA_8BIT     0x3

#define UART16550_STOP_1BIT     0x0
#define UART16550_STOP_2BIT     0x4

#define SYS_REG_READ(p)		(*((volatile unsigned long *) (p)))
#define SYS_REG_WRITE(p,v)	(*((volatile unsigned long *) (p)) = (v))
#define UART16550_READ(p)	(*((volatile u8*)(UART + (p))))
#define UART16550_WRITE(p,v)	((*((volatile u8*)(UART + (p)))) = (v))

static void putc(int c)
{
	while ((UART16550_READ(OFS_LINE_STATUS) & 0x20) == 0) ;
	UART16550_WRITE(OFS_SEND_BUFFER, c);
}

static void puts(const char *s)
{
	int c;

	while ((c = *s++)) {
		if (c == '\n')
			putc('\r');
		putc(c);
	}
}

#if defined(COBRA_EMUL)
#define AR2316_AMBA_CLOCK_RATE  20000000
#define AR2316_CPU_CLOCK_RATE   40000000
#else
#if defined(DEFAULT_PLL)
#define AR2316_AMBA_CLOCK_RATE  40000000
#define AR2316_CPU_CLOCK_RATE   40000000
#else
#define AR2316_AMBA_CLOCK_RATE  92000000
#define AR2316_CPU_CLOCK_RATE   184000000
#endif				/* ! DEFAULT_PLL */
#endif				/* ! COBRA_EMUL */

static inline void arch_decomp_setup(void)
{
	/* Initialise the serial port here */

	/* disable interrupts */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	UART16550_WRITE(OFS_INTR_ENABLE, 0);

	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, DEFAULT_DATA_FORMAT);
}

#define arch_decomp_wdog()

#endif
