/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LANTIQ_PLATFORM_H__
#define _LANTIQ_PLATFORM_H__

#include <linux/mtd/partitions.h>

/* struct used to pass info to network drivers */
enum {
	MII_MODE,
	REV_MII_MODE,
};

struct lq_eth_data {
	unsigned char *mac;
	int mii_mode;
};

#define PCI_EXIN0	0x0001
#define PCI_EXIN1	0x0002
#define PCI_EXIN2	0x0004
#define PCI_EXIN_SHIFT	0

#define PCI_GNT1	0x0008
#define PCI_GNT2	0x0010
#define PCI_GNT3	0x0020
#define PCI_GNT_SHIFT	3

#define PCI_REQ1	0x0040
#define PCI_REQ2	0x0080
#define PCI_REQ3	0x0100
#define PCI_REQ_SHIFT	6

#define	PCI_CLOCK_INT	0
#define	PCI_CLOCK_EXT	1

struct lq_pci_data {
	int clock;
	int gpio;
	int irq[16];
};

extern int (*ifxmips_pci_plat_dev_init)(struct pci_dev *dev);

#endif
