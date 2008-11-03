/*
 * include/asm-arm/arch-ixp4xx/pronghornmetro.h
 *
 * ADI Engineering platform specific definitions
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

#if	defined(CONFIG_MACH_PRONGHORNMETRO)
#define	PRONGHORNMETRO_SDA_PIN		9
#define	PRONGHORNMETRO_SCL_PIN		10

/* PCI controller GPIO to IRQ pin mappings */
#define	PCI_SLOT0_PIN	 1
#define	PCI_SLOT1_PIN	 11
#define	PCI_SLOT2_PIN	 6
#define	PCI_SLOT3_PIN	 4

#define	PCI_SLOT0_DEVID  13
#define	PCI_SLOT1_DEVID  14
#define	PCI_SLOT2_DEVID  15
#define	PCI_SLOT3_DEVID  16

#define	TASKFILE_CS 	 3
#define	TASKFILE_CS_REG	IXP4XX_EXP_CS3
#define	ALTSTAT_CS  	 4

#elif defined(CONFIG_MACH_PRONGHORN)
/* PCI controller GPIO to IRQ pin mappings */
#define	PCI_SLOT0_PIN     11
#define	PCI_SLOT1_PIN     6

#define	PCI_SLOT0_DEVID   15
#define	PCI_SLOT1_DEVID   14

#define	TASKFILE_CS       2
#define	TASKFILE_CS_REG   IXP4XX_EXP_CS2
#define	ALTSTAT_CS		  3
#endif
