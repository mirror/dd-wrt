/* pip.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * Adapted for actual PIP5 Version and PIP7 and PIP8 by
 * Stefan Peter, MPL AG, Switzerland
 * email:support@mpl.ch
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */
/* This is the CAN driver for the PIP5,PIP6,PIP7 and PIP8 
 * Packaged Industrial PCs of MPL AG, Switzerland. 
 */


#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/pip.h"
#include "../include/i82527.h"
/* PIP Specific Extension registers */
#define PIP_CANRES_REG 0x804	/* CAN Resources */
#define PIP_CANCTRL_REG (PIP_CANRES_REG+1)	/* CAN Control */
/* Interrupt maps for the various PIP variants, see user manual */
#define PIP5_IRQ_MAP 0x4F6D
#define PIP6_IRQ_MAP 0xDEF8
#define PIP7_IRQ_MAP 0x3768
#define PIP8_IRQ_MAP 0x3768

int pip_request_io(struct candevice_t *candev)
{
	if ((candev->io_addr != 0x1000) && (candev->io_addr != 0x8000)
	    && (candev->io_addr != 0xe000)) {
		CANMSG("Invalid base io address\n");
		CANMSG
		    ("Valid values for the PIP are: 0x1000, 0x8000 or 0xe000\n");
		CANMSG("Please consult your user manual.\n");
		return -ENODEV;
	}
	if (!can_request_io_region(candev->io_addr, 0x100, DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n", candev->io_addr);
		return -ENODEV;
	} else
	    if (!can_request_io_region(PIP_CANRES_REG, 0x02, DEVICE_NAME))
	{
		can_release_io_region(candev->io_addr, 0x100);
		CANMSG("Unable to open port: 0x%x\n", PIP_CANRES_REG);
		return -ENODEV;
	} else {
		DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n",
			 candev->io_addr, candev->io_addr + 0x100 - 1);
		DEBUGMSG("Registered IO-memory : 0x%x - 0x%x\n",
			 PIP_CANRES_REG, PIP_CANCTRL_REG);
	}
	return 0;
}

int pip_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr, 0x100);
	can_release_io_region(PIP_CANRES_REG, 0x02);

	return 0;
}

int pip_reset(struct candevice_t *candev)
{
	int i = 0;

	DEBUGMSG("Resetting %s hardware ...\n", candev->hwname);
	while (i < 1000000) {
		i++;
		can_outb(0x01, candev->res_addr);
	}
	can_outb(0x0, candev->res_addr);

	/* Check hardware reset status */
	i = 0;
	while ((can_inb(candev->io_addr + iCPU) & iCPU_RST) && (i <= 15)) {
		udelay(20000);
		i++;
	}
	if (i >= 15) {
		CANMSG("Reset status timeout!\n");
		CANMSG("Please check your hardware.\n");
		return -ENODEV;
	} else
		DEBUGMSG("Chip0 reset status ok.\n");


	return 0;
}

int pip_init_hw_data(struct candevice_t *candev)
{
	candev->res_addr = PIP_CANCTRL_REG;
	candev->nr_82527_chips = 1;
	candev->nr_sja1000_chips = 0;
	candev->nr_all_chips = 1;
	candev->flags |= CANDEV_PROGRAMMABLE_IRQ;

	return 0;
}

int pip_init_chip_data(struct candevice_t *candev, int chipnr)
{
	i82527_fill_chipspecops(candev->chip[chipnr]);
	candev->chip[chipnr]->chip_base_addr = can_ioport2ioptr(candev->io_addr);
	candev->chip[chipnr]->clock = 8000000;
	candev->chip[chipnr]->int_cpu_reg = 0;
	candev->chip[chipnr]->int_clk_reg = iCLK_SL1;
	candev->chip[chipnr]->int_bus_reg = iBUS_CBY;
	candev->chip[chipnr]->sja_cdr_reg = 0;
	candev->chip[chipnr]->sja_ocr_reg = 0;

	return 0;
}

int pip_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr =
	    chip->chip_base_addr + (objnr + 1) * 0x10;

	return 0;
}

int pip_program_irq(struct candevice_t *candev)
{
	unsigned int irq_mask;
	unsigned char can_addr = 0, can_reg = 0;
	DEBUGMSG("pip_program_irq\n");
	/* Reset can controller */
	can_outb(0x01, candev->res_addr);
	if (strcmp(candev->hwname, "pip5") == 0) {
		irq_mask = PIP5_IRQ_MAP;
	} else if (strcmp(candev->hwname, "pip6") == 0) {
		irq_mask = PIP6_IRQ_MAP;
	} else if (strcmp(candev->hwname, "pip7") == 0) {
		irq_mask = PIP7_IRQ_MAP;
	} else if (strcmp(candev->hwname, "pip8") == 0) {
		irq_mask = PIP8_IRQ_MAP;
	} else {
		CANMSG("Unsupported PIP specified.\n");
		return -ENODEV;
	}
	if ((candev->chip[0]->chip_irq < 1)
	    || (candev->chip[0]->chip_irq > 15)) {
		CANMSG("Interrupt specified does not exist.\n");
		return -ENODEV;
	}
	if (((0x01 << (candev->chip[0]->chip_irq - 1)) & irq_mask) == 0) {
		CANMSG("Invalid Interrupt specified %d\n",
		       candev->chip[0]->chip_irq);
		return -ENODEV;
	}

	/* set the IRQ routing of the board accordingly */
	switch (candev->io_addr) {
	case 0x1000:{
			can_addr = 0x01;
			break;
		}
	case 0x8000:{
			can_addr = 0x02;
			break;
		}
	case 0xe000:{
			can_addr = 0x03;
			break;
		}
	default:{
			CANMSG
			    ("Supplied io address is not valid, please check your manual\n");
			return -ENODEV;
		}
	}
	can_reg = can_inb(PIP_CANRES_REG);
	DEBUGMSG("PIP_CANRES was 0x%x\n", can_reg);
	can_reg = (candev->chip[0]->chip_irq << 4) | can_addr;
	DEBUGMSG("Setting PIP_CANRES_REG to 0x%x\n", can_reg);
	can_outb((candev->chip[0]->chip_irq << 4) | can_addr, PIP_CANRES_REG);
	/* re-enable the chip */
	can_outb(0x00, candev->res_addr);

	return 0;
}


void pip_write_register(unsigned data, can_ioptr_t address)
{
	can_outb(data, address);
}

unsigned pip_read_register(can_ioptr_t address)
{
	return can_inb(address);
}

/* !!! Don't change these functions !!! */
int pip_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = pip_request_io;
	hwspecops->release_io = pip_release_io;
	hwspecops->reset = pip_reset;
	hwspecops->init_hw_data = pip_init_hw_data;
	hwspecops->init_chip_data = pip_init_chip_data;
	hwspecops->init_obj_data = pip_init_obj_data;
	hwspecops->write_register = pip_write_register;
	hwspecops->read_register = pip_read_register;
	hwspecops->program_irq = pip_program_irq;
	return 0;
}
