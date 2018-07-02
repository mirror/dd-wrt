/*
 * include/asm-arm/arch-ixp4xx/ap71.h
 *
 * AP71 platform specific definitions
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright 2004 (c) MontaVista, Software, Inc. 
 * 
 * This file is licensed under  the terms of the GNU General Public 
 * License version 2. This program is licensed "as is" without any 
 * warranty of any kind, whether express or implied.
 */

#ifndef __ASM_ARCH_HARDWARE_H__
#error "Do not include this directly, instead #include <asm/hardware.h>"
#endif

#define	AP71_FLASH_BASE	IXP4XX_EXP_BUS_CS0_BASE_PHYS
#define	AP71_FLASH_SIZE	IXP4XX_EXP_BUS_CSX_REGION_SIZE

/*
 * AP71 PCI IRQs
 */
#define AP71_PCI_MAX_DEV	2
#define AP71_PCI_IRQ_LINES	3

/*
 * Reset Button connected to GPIO Pin 8 (GPIO8 )
 */
#define	AP71_SW_PUSHBUTTON_RESET 8
/* IRQ mappings of the above pin */
#define AP71_SW_PUSHBUTTON_RESET_IRQ	25

/* 
 * PCI controller GPIO to IRQ pin mappings 
 */
#define AP71_PCI_INTA_PIN	11
#define AP71_PCI_INTB_PIN	10
#define	AP71_PCI_INTC_PIN	9

/* IRQ mappings of the above pins */
#define AP71_PCI_INTA_IRQ	28
#define AP71_PCI_INTB_IRQ	27
#define AP71_PCI_INTC_IRQ	26
/*
 * PCI reset is tied to GPIO 13
 */
#define AP71_PCI_RESET_LINE 13


