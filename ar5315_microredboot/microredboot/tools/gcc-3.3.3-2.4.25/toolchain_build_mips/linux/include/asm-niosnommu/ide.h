//FIXME - copied from armnommu version - needs to be ported
/*
 *  linux/include/asm-niosnommu/ide.h
 *
 *  Copyright (C) 1994-1996  Linus Torvalds & authors
 */

/*
 *  This file contains the nios architecture specific IDE code.
 */

#ifndef __ASMNIOS_IDE_H
#define __ASMNIOS_IDE_H

#ifdef __KERNEL__

#include <asm/nios.h>


#undef MAX_HWIFS		/* we're going to force it */

#ifndef MAX_HWIFS
#define MAX_HWIFS	1
#endif


#define ide__sti()	__sti()


typedef union {
	unsigned all			: 8;	/* all of the bits together */
	struct {
		unsigned head		: 4;	/* always zeros here */
		unsigned unit		: 1;	/* drive select number, 0 or 1 */
		unsigned bit5		: 1;	/* always 1 */
		unsigned lba		: 1;	/* using LBA instead of CHS */
		unsigned bit7		: 1;	/* always 1 */
	} b;
	} select_t;


static __inline__ int ide_default_irq(ide_ioreg_t base)
{
	switch (base) {
		case ((int) na_ide_interface)+0x40:
			return na_ide_interface_irq;
		default:
			return 0;
	}
}

static __inline__ ide_ioreg_t ide_default_io_base(int index)
{
	switch (index) {
		case 0:
			return ((int) na_ide_interface)+0x40;
		default:
			return 0;
	}
}

static __inline__ void ide_init_hwif_ports(hw_regs_t *hw,
					   ide_ioreg_t data_port,
					   ide_ioreg_t ctrl_port, int *irq)
{
	ide_ioreg_t reg = data_port;
	int i;

	for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
		hw->io_ports[i] = reg;
		reg += 4;
	}
	if (ctrl_port) {
		hw->io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
	} else {
		hw->io_ports[IDE_CONTROL_OFFSET] = data_port + (0xE*4);
	}
	if (irq != NULL)
		*irq = 0;
	hw->io_ports[IDE_IRQ_OFFSET] = 0;
}

static __inline__ void ide_init_default_hwifs(void)
{
	hw_regs_t hw;
	int index;
	ide_ioreg_t base;

	for (index = 0; index < MAX_HWIFS; index++) {
		base = ide_default_io_base(index);
		if (base == 0)
			continue;
		ide_init_hwif_ports(&hw, base, 0, NULL);
		hw.irq = ide_default_irq(base);
		ide_register_hw(&hw, NULL);
	}
}

#define ide_request_irq(irq,hand,flg,dev,id)	request_irq((irq),(hand),(flg),(dev),(id))
#define ide_free_irq(irq,dev_id)		free_irq((irq), (dev_id))
#define ide_check_region(from,extent)		check_region((from), (extent))
#define ide_request_region(from,extent,name)	request_region((from), (extent), (name))
#define ide_release_region(from,extent)		release_region((from), (extent))

/*
 * The following are not needed for the non-m68k ports
 */
#define ide_ack_intr(hwif)		(1)
#define ide_fix_driveid(id)		do {} while (0)
#define ide_release_lock(lock)		do {} while (0)
#define ide_get_lock(lock, hdlr, data)	do {} while (0)

/*
 * We always use the new IDE port registering,
 * so these are fixed here.
 */
//vic#define ide_default_io_base(i)		((ide_ioreg_t)0)
//vic#define ide_default_irq(b)		(0)

/* now for some arch-specific io functions */

#define HAVE_ARCH_OUT_BYTE
#define OUT_BYTE(b,p) outl((b),(p))

#endif /* __KERNEL__ */

#endif /* __ASMNIOS_IDE_H */
