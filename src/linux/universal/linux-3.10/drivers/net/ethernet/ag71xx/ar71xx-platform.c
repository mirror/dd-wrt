/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#ifndef CONFIG_MACH_AR7240
#ifndef CONFIG_MACH_HORNET

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#include <asm/mach-ar71xx/ar71xx.h>

static DEFINE_SPINLOCK(ar71xx_device_lock);

enum ar71xx_soc_type ar71xx_soc;

void ar71xx_device_stop(u32 mask)
{
	unsigned long flags;
	u32 mask_inv;
	u32 t;

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7130:
	case AR71XX_SOC_AR7141:
	case AR71XX_SOC_AR7161:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR71XX_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR71XX_RESET_REG_RESET_MODULE, t | mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR7242:
		mask_inv = mask & RESET_MODULE_USB_OHCI_DLL_7240;
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR724X_RESET_REG_RESET_MODULE);
		t |= mask;
		t &= ~mask_inv;
		ar71xx_reset_wr(AR724X_RESET_REG_RESET_MODULE, t);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9130:
	case AR71XX_SOC_AR9132:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR91XX_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR91XX_RESET_REG_RESET_MODULE, t | mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR933X_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR933X_RESET_REG_RESET_MODULE, t | mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR934X_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR934X_RESET_REG_RESET_MODULE, t | mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	default:
		BUG();
	}
}
EXPORT_SYMBOL_GPL(ar71xx_device_stop);
void ar71xx_device_start(u32 mask)
{
	unsigned long flags;
	u32 mask_inv;
	u32 t;

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7130:
	case AR71XX_SOC_AR7141:
	case AR71XX_SOC_AR7161:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR71XX_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR71XX_RESET_REG_RESET_MODULE, t & ~mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR7242:
		mask_inv = mask & RESET_MODULE_USB_OHCI_DLL_7240;
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR724X_RESET_REG_RESET_MODULE);
		t &= ~mask;
		t |= mask_inv;
		ar71xx_reset_wr(AR724X_RESET_REG_RESET_MODULE, t);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9130:
	case AR71XX_SOC_AR9132:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR91XX_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR91XX_RESET_REG_RESET_MODULE, t & ~mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR933X_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR933X_RESET_REG_RESET_MODULE, t & ~mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		spin_lock_irqsave(&ar71xx_device_lock, flags);
		t = ar71xx_reset_rr(AR934X_RESET_REG_RESET_MODULE);
		ar71xx_reset_wr(AR934X_RESET_REG_RESET_MODULE, t & ~mask);
		spin_unlock_irqrestore(&ar71xx_device_lock, flags);
		break;

	default:
		BUG();
	}
}
EXPORT_SYMBOL_GPL(ar71xx_device_start);
void ar71xx_ddr_flush(u32 reg)
{
	ar71xx_ddr_wr(reg, 1);
	while ((ar71xx_ddr_rr(reg) & 0x1))
		;

	ar71xx_ddr_wr(reg, 1);
	while ((ar71xx_ddr_rr(reg) & 0x1))
		;
}
EXPORT_SYMBOL_GPL(ar71xx_ddr_flush);
#endif
#endif
