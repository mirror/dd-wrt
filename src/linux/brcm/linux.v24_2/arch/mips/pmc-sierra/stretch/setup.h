/*
 * Copyright 2004 PMC-Sierra
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * Board specific definititions for the PMC-Sierra Stretch
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __SETUP_H__
#define __SETUP_H__

/* 
 * Define for the base addreses 
 */

#define	PMC_STRETCH_BASE			(0x1A000000 + KSEG1) /* FIXME */

/* 
 * PCI defines
 */
#define	PMC_STRETCH_PCI_BASE			(0x1A000000 + KSEG1) /* FIXME */

#define	PMC_STRETCH_READ_DATA(ofs)				\
	*(volatile u32 *)(PMC_STRETCH_PCI_BASE + ofs)

/*
 * PCI Config space accesses
 */
#define	PMC_STRETCH_PCI_0_CONFIG_ADDRESS	0x678
#define	PMC_STRETCH_PCI_0_DATA_ADDRESS		0x6fc

#define	PMC_STRETCH_WRITE(ofs, data)				\
	*(volatile u32 *)(PMC_STRETCH_PCI_BASE + ofs) = data

#define	PMC_STRETCH_READ(ofs, data)				\
	*(data) = *(volatile u32 *)(PMC_STRETCH_PCI_BASE + ofs)

#define	PMC_STRETCH_WRITE_16(ofs, data)				\
	*(volatile u16 *)(PMC_STRETCH_PCI_BASE + ofs) = data

#define	PMC_STRETCH_READ_16(ofs, data)				\
	*(data) =  *(volatile u16 *)(PMC_STRETCH_PCI_BASE + ofs)

#define	PMC_STRETCH_WRITE_8(ofs, data)				\
	*(volatile u8 *)(PMC_STRETCH_PCI_BASE + ofs) = data

#define	PMC_STRETCH_READ_8(ofs, data)				\
	*(data) = *(volatile u8 *)(PMC_STRETCH_PCI_BASE + ofs)

/* 
 * RTC/NVRAM 
 */
#define	PMC_STRETCH_NVRAM_BASE	0xfc800000	/* FIXME */

#endif /* __SETUP_H__ */
