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

/* struct used to pass info to the pci core */
enum {
	PCI_CLOCK_INT = 0,
	PCI_CLOCK_EXT
};

struct lq_pci_data {
	int clock;
	int req_mask;
};

#endif
