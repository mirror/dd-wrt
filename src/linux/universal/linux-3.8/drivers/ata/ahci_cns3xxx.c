/*
 * AHCI support for CNS3xxx SoC
 *
 * Copyright 2010 MontaVista Software, LLC.
 * Copyright 2010 Cavium Networks
 *
 * Authors: Anton Vorontsov <avorontsov@xxxxxxxxxx>
 *	    Mac Lin <mkl0301@xxxxxxxxx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include "ahci.h"

/*
 * TODO: move cns3xxx_ahci_init to here after cns3xxx_pwr*() calls are
 * thread-safe
 */

static int cns3xxx_ahci_softreset(struct ata_link *link, unsigned int *class,
			  unsigned long deadline)
{
	int pmp = sata_srst_pmp(link);
	int ret;

	ret = ahci_do_softreset(link, class, pmp, deadline, ahci_check_ready);
	if (pmp && ret)
		return ahci_do_softreset(link, class, 0, deadline,
			ahci_check_ready);
	return ret;
}

static struct ata_port_operations cns3xxx_ahci_ops = {
	.inherits		= &ahci_ops,
	.softreset		= cns3xxx_ahci_softreset,
};

static const struct ata_port_info cns3xxx_ata_port_info = {
	.flags		= AHCI_FLAG_COMMON,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &cns3xxx_ahci_ops,
};

struct ahci_platform_data cns3xxx_ahci_platform_data = {
	.ata_port_info	= &cns3xxx_ata_port_info,
};

