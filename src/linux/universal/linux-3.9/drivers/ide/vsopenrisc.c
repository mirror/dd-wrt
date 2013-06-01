#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-irq.h>

#define IDE_ARCH_OBSOLETE_DEFAULTS
#include <linux/ide.h>


#undef  MAX_HWIFS
#define MAX_HWIFS	1


typedef unsigned long ide_ioreg_t;


static int __init vsopenrisc_ide_default_irq(int base)
{
	switch (base) {
		case VSOPENRISC_EPLD_IDE_DATA: return KS8695_INTEPLD_IDE;
		default:
			return 0;
	}
	return 0;
}

static ide_ioreg_t __init vsopenrisc_ide_default_io_base(int index)
{
	switch (index) {
		case 0:	return VSOPENRISC_EPLD_IDE_DATA;
		default:
			return 0;
	}
}

void vsopenrisc_ide_init_hwif_ports(hw_regs_t *hw, ide_ioreg_t data_port, ide_ioreg_t ctrl_port, int *irq)
{
	ide_ioreg_t reg = data_port;
	int i;

	if (reg)
	{
		for (i = 0; i <= 7; i++) {
			hw->io_ports[i] = (ide_ioreg_t)reg;
			reg += 4;
		}
		if (ctrl_port) {
			hw->io_ports[8] = (ide_ioreg_t)ctrl_port;
		} else {
			hw->io_ports[8] = (ide_ioreg_t)VSOPENRISC_EPLD_IDE_CTRL;
		}
	}
	if (irq != NULL)
		*irq = 0;
	hw->io_ports[9] = 0;
}

/*static void __init tune_epld(ide_drive_t *drive, unsigned char pio)
{
	unsigned long flags, addr;


	spin_lock_irqsave(&ide_lock, flags);
	
	// FIXME: we should identify the supported PIO from the devices
	// and setting the EPLD to the minimum value of them both.

	outb(0x03, IDE_FEATURE_REG);
	if (pio >= 3)
		outb(0x0b, IDE_NSECTOR_REG);
	else
		outb(0x08, IDE_NSECTOR_REG);
	// FIXME: didn't find a field, which shows the state of the device
	if (drive->name[2] == 'a')	// master
		outb(0x00, IDE_SELECT_REG);
	else
		outb(0x10, IDE_SELECT_REG);
	outb(0xef, IDE_COMMAND_REG);

	// configure the "controller" (EPLD)
	addr = VSOPENRISC_VA_EPLD_IDE_BASE;
	
	if (pio >= 3)
		outb(0x03, addr);
	else
		outb(0x00, addr);
	
	spin_unlock_irqrestore(&ide_lock, flags);
}*/

static int __init vsopenrisc_ide_register(hw_regs_t *hw)
{
	ide_hwif_t *hwif;
	int i, rc;
	u8 idx[4] = { 0xff, 0xff, 0xff, 0xff };

//	hwif = ide_find_port();
//	if (hwif == NULL)
//		goto out;
//

	ide_register_hw(hw, 0, &hwif);

//	hwif->port_ops = NULL;
//	i = hwif->index;

//	idx[0] = i;
//	ide_device_add(idx, NULL);
	
out:
	return 0;

}

static int __init vsopenrisc_ide_init_default_hwifs(void)
{
	ide_hwif_t *hwif;
	hw_regs_t hw;
	int index = 0;

	printk(KERN_INFO "VScom OpenRISC IDE init\n");

	//for(index = 0; index < MAX_HWIFS; index++) {
		memset(&hw, 0, sizeof(hw));
		vsopenrisc_ide_init_hwif_ports(&hw, vsopenrisc_ide_default_io_base(index), 0, NULL);
		hw.irq = vsopenrisc_ide_default_irq(vsopenrisc_ide_default_io_base(index));
		vsopenrisc_ide_register(&hw);

		//ide_register_hw(&hw, NULL, 0, &hwif);
		//hwif->tuneproc = &tune_epld;
	//}

	return 0;
}

module_init(vsopenrisc_ide_init_default_hwifs);

MODULE_AUTHOR("VScom <info@vscom.de>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OpenRISC IDE driver");

