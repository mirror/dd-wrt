/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LTQ_XWAY_H__
#define _LTQ_XWAY_H__

#ifdef CONFIG_SOC_TYPE_XWAY

#include <lantiq.h>

/* Chip IDs */
#define SOC_ID_DANUBE1		0x129
#define SOC_ID_DANUBE2		0x12B
#define SOC_ID_TWINPASS		0x12D
#define SOC_ID_AMAZON_SE	0x152
#define SOC_ID_ARX188		0x16C
#define SOC_ID_ARX168		0x16D
#define SOC_ID_ARX182		0x16F
#define SOC_ID_VRX288		0x1C0

/* SoC Types */
#define SOC_TYPE_DANUBE		0x01
#define SOC_TYPE_TWINPASS	0x02
#define SOC_TYPE_AR9		0x03
#define SOC_TYPE_VR9		0x04
#define SOC_TYPE_AMAZON_SE	0x05

/* ASC0/1 - serial port */
#define LTQ_ASC0_BASE_ADDR	0x1E100400
#define LTQ_ASC1_BASE_ADDR	0x1E100C00
#define LTQ_ASC_SIZE		0x400

/* during early_printk no ioremap is possible
   lets use KSEG1 instead  */
#define LTQ_EARLY_ASC		KSEG1ADDR(LTQ_ASC1_BASE_ADDR)

/* RCU - reset control unit */
#define LTQ_RCU_BASE_ADDR	0x1F203000
#define LTQ_RCU_SIZE		0x1000

/* GPTU - general purpose timer unit */
#define LTQ_GPTU_BASE_ADDR	0x18000300
#define LTQ_GPTU_SIZE		0x100

/* EBU - external bus unit */
#define LTQ_EBU_GPIO_START	0x14000000
#define LTQ_EBU_GPIO_SIZE	0x1000

#define LTQ_EBU_BASE_ADDR	0x1E105300
#define LTQ_EBU_SIZE		0x100

#define LTQ_EBU_BUSCON0		0x0060
#define LTQ_EBU_PCC_CON		0x0090
#define LTQ_EBU_PCC_IEN		0x00A4
#define LTQ_EBU_PCC_ISTAT	0x00A0
#define LTQ_EBU_BUSCON1		0x0064
#define LTQ_EBU_ADDRSEL1	0x0024
#define EBU_WRDIS		0x80000000

/* CGU - clock generation unit */
#define LTQ_CGU_BASE_ADDR	0x1F103000
#define LTQ_CGU_SIZE		0x1000

#define CGU_EPHY		0x10

/* ICU - interrupt control unit */
#define LTQ_ICU_BASE_ADDR	0x1F880200
#define LTQ_ICU_SIZE		0x100

/* EIU - external interrupt unit */
#define LTQ_EIU_BASE_ADDR	0x1F101000
#define LTQ_EIU_SIZE		0x1000

/* PMU - power management unit */
#define LTQ_PMU_BASE_ADDR	0x1F102000
#define LTQ_PMU_SIZE		0x1000

#define PMU_DMA			0x0020
#define PMU_EPHY		0x0080
#define PMU_USB			0x8041
#define PMU_SPI			0x0100
#define PMU_LED			0x0800
#define PMU_GPT			0x1000
#define PMU_PPE			0x2000
#define PMU_FPI			0x4000
#define PMU_SWITCH		0x10000000

/* ETOP - ethernet */
#define LTQ_ETOP_BASE_ADDR	0x1E180000
#define LTQ_ETOP_BASE_ADDR_VR9	0x1E200000
#define LTQ_ETOP_SIZE		0x40000

/* GBIT - gigabit switch */
#define LTQ_GBIT_BASE_ADDR	0x1E108000
#define LTQ_GBIT_SIZE		0x200

/* DMA */
#define LTQ_DMA_BASE_ADDR	0x1E104100
#define LTQ_DMA_SIZE		0x800

/* PCI */
#define PCI_CR_BASE_ADDR	0x1E105400
#define PCI_CR_SIZE		0x400

/* WDT */
#define LTQ_WDT_BASE_ADDR	0x1F8803F0
#define LTQ_WDT_SIZE		0x10

#define LTQ_RST_CAUSE_WDTRST	0x20

/* STP - serial to parallel conversion unit */
#define LTQ_STP_BASE_ADDR	0x1E100BB0
#define LTQ_STP_SIZE		0x40

/* GPIO */
#define LTQ_GPIO0_BASE_ADDR	0x1E100B10
#define LTQ_GPIO1_BASE_ADDR	0x1E100B40
#define LTQ_GPIO2_BASE_ADDR	0x1E100B70
#define LTQ_GPIO3_BASE_ADDR	0x1E100BA0
#define LTQ_GPIO_SIZE		0x30
#define LTQ_GPIO3_SIZE		0x10

/* SSC */
#define LTQ_SSC_BASE_ADDR	0x1e100800
#define LTQ_SSC_SIZE		0x100

/* MEI - dsl core */
#define LTQ_MEI_BASE_ADDR	0x1E116000

/* DEU - data encryption unit */
#define LTQ_DEU_BASE_ADDR	0x1E103100

/* MPS - multi processor unit (voice) */
#define LTQ_MPS_BASE_ADDR	(KSEG1 + 0x1F107000)
#define LTQ_MPS_CHIPID		((u32 *)(LTQ_MPS_BASE_ADDR + 0x0344))

/* register access macros for EBU and CGU */
#define ltq_ebu_w32(x, y)	ltq_w32((x), ltq_ebu_membase + (y))
#define ltq_ebu_r32(x)		ltq_r32(ltq_ebu_membase + (x))
#define ltq_ebu_w32_mask(x, y, z) \
	ltq_w32_mask(x, y, ltq_ebu_membase + (z))
#define ltq_cgu_w32(x, y)	ltq_w32((x), ltq_cgu_membase + (y))
#define ltq_cgu_r32(x)		ltq_r32(ltq_cgu_membase + (x))

extern __iomem void *ltq_ebu_membase;
extern __iomem void *ltq_cgu_membase;

/* allow booting xrx200 phys */
int xrx200_gphy_boot(struct device *dev, unsigned int id, dma_addr_t dev_addr);

/* request a non-gpio and set the PIO config */
extern int  ltq_gpio_request(unsigned int pin, unsigned int alt0,
	unsigned int alt1, unsigned int dir, const char *name);
extern void ltq_pmu_enable(unsigned int module);
extern void ltq_pmu_disable(unsigned int module);
extern void ltq_cgu_enable(unsigned int clk);

static inline int ltq_is_ase(void)
{
	return (ltq_get_soc_type() == SOC_TYPE_AMAZON_SE);
}

static inline int ltq_is_ar9(void)
{
	return (ltq_get_soc_type() == SOC_TYPE_AR9);
}

static inline int ltq_is_vr9(void)
{
	return (ltq_get_soc_type() == SOC_TYPE_VR9);
}

#endif /* CONFIG_SOC_TYPE_XWAY */
#endif /* _LTQ_XWAY_H__ */
