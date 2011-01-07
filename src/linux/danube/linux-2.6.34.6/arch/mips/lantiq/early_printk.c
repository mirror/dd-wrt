/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/cpu.h>

#include <lantiq.h>

#ifdef CONFIG_SOC_LANTIQ_XWAY
#include <xway.h>
#ifdef CONFIG_LANTIQ_PROM_ASC0
#define LQ_ASC_BASE	KSEG1ADDR(LQ_ASC0_BASE)
#else
#define LQ_ASC_BASE	KSEG1ADDR(LQ_ASC1_BASE)
#endif

#elif CONFIG_SOC_LANTIQ_FALCON
#include <falcon/gpon_reg_base.h>
#ifdef CONFIG_LANTIQ_PROM_ASC0
#define LQ_ASC_BASE	GPON_ASC0_BASE
#else
#define LQ_ASC_BASE	GPON_ASC1_BASE
#endif

#endif

#define ASC_BUF				1024
#define LQ_ASC_FSTAT		0x0048
#define LQ_ASC_TBUF			0x0020
#define TXMASK				0x3F00
#define TXOFFSET			8

static char buf[ASC_BUF];

void
prom_putchar(char c)
{
	unsigned long flags;

	local_irq_save(flags);
	while ((lq_r32((u32 *)(LQ_ASC_BASE + LQ_ASC_FSTAT)) & TXMASK) >> TXOFFSET);

	if (c == '\n')
		lq_w32('\r', (u32 *)(LQ_ASC_BASE + LQ_ASC_TBUF));
	lq_w32(c, (u32 *)(LQ_ASC_BASE + LQ_ASC_TBUF));
	local_irq_restore(flags);
}

void
early_printf(const char *fmt, ...)
{
	va_list args;
	int l;
	char *p, *buf_end;

	va_start(args, fmt);
	l = vsnprintf(buf, ASC_BUF, fmt, args);
	va_end(args);
	buf_end = buf + l;

	for (p = buf; p < buf_end; p++)
		prom_putchar(*p);
}
