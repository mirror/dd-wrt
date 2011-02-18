/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2005 infineon
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifdef CONFIG_SOC_LANTIQ_XWAY

#ifndef _LQ_XWAY_H__
#define _LQ_XWAY_H__

#include <lantiq.h>

/* request a non-gpio and set the PIO config */
extern int  lq_gpio_request(unsigned int pin, unsigned int alt0,
    unsigned int alt1, unsigned int dir, const char *name);
extern int lq_gpio_setconfig(unsigned int pin, unsigned int reg, unsigned int val);

extern void lq_pmu_enable(unsigned int module);
extern void lq_pmu_disable(unsigned int module);

extern unsigned int lq_get_fpi_bus_clock(int bus);

#define BOARD_SYSTEM_TYPE		"LANTIQ"

/*------------ Chip IDs */
#define SOC_ID_DANUBE1		0x129
#define SOC_ID_DANUBE2		0x12B
#define SOC_ID_TWINPASS		0x12D
#define SOC_ID_ARX188		0x16C
#define SOC_ID_ARX168		0x16D
#define SOC_ID_ARX182		0x16F

/*------------ SoC Types */
#define SOC_TYPE_DANUBE		0x01
#define SOC_TYPE_TWINPASS	0x02
#define SOC_TYPE_AR9		0x03

/*------------ ASC0/1 */
#define LQ_ASC0_BASE		0x1E100400
#define LQ_ASC1_BASE		0x1E100C00
#define LQ_ASC_SIZE			0x400

/*------------ RCU */
#define LQ_RCU_BASE_ADDR	0xBF203000
#define LQ_RCU_PPE_CONF 		((u32 *)(LQ_RCU_BASE_ADDR + 0x002C))

/*------------ GPTU */
#define LQ_GPTU_BASE_ADDR	0xB8000300

/*------------ EBU */
#define LQ_EBU_GPIO_START	0x14000000
#define LQ_EBU_GPIO_SIZE	0x1000

#define LQ_EBU_BASE_ADDR	0xBE105300

#define LQ_EBU_BUSCON0		((u32 *)(LQ_EBU_BASE_ADDR + 0x0060))
#define LQ_EBU_PCC_CON		((u32 *)(LQ_EBU_BASE_ADDR + 0x0090))
#define LQ_EBU_PCC_IEN		((u32 *)(LQ_EBU_BASE_ADDR + 0x00A4))
#define LQ_EBU_PCC_ISTAT	((u32 *)(LQ_EBU_BASE_ADDR + 0x00A0))
#define LQ_EBU_BUSCON1		((u32 *)(LQ_EBU_BASE_ADDR + 0x0064))
#define LQ_EBU_ADDRSEL1		((u32 *)(LQ_EBU_BASE_ADDR + 0x0024))

#define EBU_WRDIS			0x80000000

/*------------ CGU */
#define LQ_CGU_BASE_ADDR	(KSEG1 + 0x1F103000)

/*------------ PMU */
#define LQ_PMU_BASE_ADDR	(KSEG1 + 0x1F102000)

#define PMU_DMA				0x0020
#define PMU_USB				0x8041
#define PMU_LED				0x0800
#define PMU_GPT				0x1000
#define PMU_PPE				0x2000
#define PMU_FPI				0x4000
#define PMU_SWITCH			0x10000000

/*------------ ETOP */
#define LQ_PPE32_BASE_ADDR	0xBE180000
#define LQ_PPE32_SIZE		0x40000

/*------------ DMA */
#define LQ_DMA_BASE_ADDR	0xBE104100

/*------------ PCI */
#define PCI_CR_PR_BASE_ADDR	(KSEG1 + 0x1E105400)
#define PCI_CS_PR_BASE_ADDR	(KSEG1 + 0x17000000)

/*------------ WDT */
#define LQ_WDT_BASE			0x1F880000
#define LQ_WDT_SIZE			0x400

/*------------ Serial To Parallel conversion  */
#define LQ_STP_BASE			0x1E100BB0
#define LQ_STP_SIZE			0x40

/*------------ GPIO */
#define LQ_GPIO0_BASE_ADDR	0x1E100B10
#define LQ_GPIO1_BASE_ADDR	0x1E100B40
#define LQ_GPIO2_BASE_ADDR	0x1E100B70
#define LQ_GPIO3_BASE_ADDR	0x1E100BA0
#define LQ_GPIO_SIZE		0x30

/*------------ SSC */
#define LQ_SSC_BASE_ADDR	(KSEG1 + 0x1e100800)

/*------------ MEI */
#define LQ_MEI_BASE_ADDR	(KSEG1 + 0x1E116000)

/*------------ DEU */
#define LQ_DEU_BASE			(KSEG1 + 0x1E103100)

/*------------ MPS */
#define LQ_MPS_BASE_ADDR	(KSEG1 + 0x1F107000)
#define LQ_MPS_CHIPID		((u32 *)(LQ_MPS_BASE_ADDR + 0x0344))

#endif

#endif
