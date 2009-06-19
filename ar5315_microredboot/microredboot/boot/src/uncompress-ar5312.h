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

#define UART			0xbc000003
#define AR531X_RESET		0xbc003020
#define AR5312_CLOCKCTL1	0xbc003064
#define AR531X_REV		0xbc003090

#define AR531X_REV_MAJ		0x00f0
#define AR531X_REV_MAJ_AR5312	0x40
#define AR531X_REV_MAJ_AR2313	0x50

#define AR531X_RESET_UART0      		0x00000100
#define AR5312_CLOCKCTL1_PREDIVIDE_MASK		0x00000030
#define AR5312_CLOCKCTL1_PREDIVIDE_SHIFT	4
#define AR5312_CLOCKCTL1_MULTIPLIER_MASK	0x00001f00
#define AR5312_CLOCKCTL1_MULTIPLIER_SHIFT	8
#define AR5312_CLOCKCTL1_DOUBLER_MASK		0x00010000

#define AR2313_CLOCKCTL1_PREDIVIDE_MASK		0x00003000
#define AR2313_CLOCKCTL1_PREDIVIDE_SHIFT	12
#define AR2313_CLOCKCTL1_MULTIPLIER_MASK	0x001f0000
#define AR2313_CLOCKCTL1_MULTIPLIER_SHIFT	16
#define AR2313_CLOCKCTL1_DOUBLER_MASK		0x00000000

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
		putc(c);
		if (c == '\n')
			putc('\r');
	}
}

static int ar531x_cpu_frequency(void)
{
	static const int CLOCKCTL1_PREDIVIDE_TABLE[4] = {
		1,
		2,
		4,
		5
	};

	int preDivideSelect;
	int preDivisor;
	int multiplier;
	int doublerMask;

	unsigned int clockCtl1 = SYS_REG_READ(AR5312_CLOCKCTL1);

	unsigned int cpu_version = SYS_REG_READ(AR531X_REV) & AR531X_REV_MAJ;

	if (cpu_version == AR531X_REV_MAJ_AR5312) {

		preDivideSelect =
		    (clockCtl1 & AR5312_CLOCKCTL1_PREDIVIDE_MASK) >>
		    AR5312_CLOCKCTL1_PREDIVIDE_SHIFT;
		preDivisor = CLOCKCTL1_PREDIVIDE_TABLE[preDivideSelect];
		multiplier = (clockCtl1 & AR5312_CLOCKCTL1_MULTIPLIER_MASK) >>
		    AR5312_CLOCKCTL1_MULTIPLIER_SHIFT;
		doublerMask = AR5312_CLOCKCTL1_DOUBLER_MASK;

	} else {

		preDivideSelect =
		    (clockCtl1 & AR2313_CLOCKCTL1_PREDIVIDE_MASK) >>
		    AR2313_CLOCKCTL1_PREDIVIDE_SHIFT;
		preDivisor = CLOCKCTL1_PREDIVIDE_TABLE[preDivideSelect];
		multiplier = (clockCtl1 & AR2313_CLOCKCTL1_MULTIPLIER_MASK) >>
		    AR2313_CLOCKCTL1_MULTIPLIER_SHIFT;
		doublerMask = AR2313_CLOCKCTL1_DOUBLER_MASK;

	}

	if (clockCtl1 & doublerMask) {
		multiplier = multiplier << 1;
	}

	return ((40000000 / preDivisor) * multiplier);
}

static inline void arch_decomp_setup(void)
{
	/* Initialise the serial port here */

	SYS_REG_WRITE(AR531X_RESET,
		      SYS_REG_READ(AR531X_RESET) & ~(AR531X_RESET_UART0));

	/* disable interrupts */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	UART16550_WRITE(OFS_INTR_ENABLE, 0);

	/* set up buad rate */
	{
		u32 divisor;
		u32 uart_clock_rate = ar531x_cpu_frequency() / 4;
		u32 base_baud = uart_clock_rate / 16;

		/* set DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x80);

		/* set divisor */
		divisor = base_baud / CONFIG_DEFAULT_BAUDRATE;
		UART16550_WRITE(OFS_DIVISOR_LSB, divisor & 0xff);
		UART16550_WRITE(OFS_DIVISOR_MSB, (divisor & 0xff00) >> 8);

		/* clear DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	}

	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, DEFAULT_DATA_FORMAT);
}

#define arch_decomp_wdog()

#endif
