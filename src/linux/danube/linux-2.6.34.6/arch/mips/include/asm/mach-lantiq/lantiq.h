/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LANTIQ_H__
#define _LANTIQ_H__

/* generic reg access functions */
#define lq_r32(reg)						__raw_readl(reg)
#define lq_w32(val, reg)				__raw_writel(val, reg)
#define lq_w32_mask(clear, set, reg)	lq_w32((lq_r32(reg) & ~clear) | set, reg)

extern unsigned int lq_get_cpu_ver(void);
extern unsigned int lq_get_soc_type(void);

/* clock speeds */
#define CLOCK_60M			60000000
#define CLOCK_83M			83333333
#define CLOCK_111M			111111111
#define CLOCK_111M			111111111
#define CLOCK_133M			133333333
#define CLOCK_167M			166666667
#define CLOCK_200M			200000000
#define CLOCK_333M			333333333
#define CLOCK_400M			400000000

/* spinlock all ebu i/o */
extern spinlock_t ebu_lock;

/* some irq helpers */
extern void lq_disable_irq(unsigned int irq_nr);
extern void lq_mask_and_ack_irq(unsigned int irq_nr);
extern void lq_enable_irq(unsigned int irq_nr);

#define IOPORT_RESOURCE_START		0x10000000
#define IOPORT_RESOURCE_END		0xffffffff
#define IOMEM_RESOURCE_START		0x10000000
#define IOMEM_RESOURCE_END		0xffffffff

#define LQ_FLASH_START		0x10000000
#define LQ_FLASH_MAX		0x04000000

#endif
