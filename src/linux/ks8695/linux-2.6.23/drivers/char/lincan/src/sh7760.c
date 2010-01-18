/* sh7760.c
* Linux CAN-bus device driver.
* This software is released under the GPL-License.
*/ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/sh7760.h"
#include "../include/hcan2.h"

int sh7760_request_io(struct candevice_t *candev)
{
	if (!can_request_io_region(candev->io_addr, candev->nr_all_chips * IO_RANGE, DEVICE_NAME)) {
		CANMSG("Unable to open port: 0x%lx\n",candev->io_addr);
		return -ENODEV;
	}

	DEBUGMSG("Registered IO-memory: 0x%lx - 0x%lx\n", candev->io_addr, candev->io_addr + candev->nr_all_chips * IO_RANGE - 1);
	return 0;
}

int sh7760_release_io(struct candevice_t *candev)
{
	can_release_io_region(candev->io_addr, candev->nr_all_chips * IO_RANGE);

	return 0;
}

int sh7760_reset(struct candevice_t *candev)
{
	int i; 
	DEBUGMSG("Resetting HCAN2 chips ...\n");

	for (i = 0; i < candev->nr_all_chips; i++)
	{
	    /* !!! Assuming this card has ONLY HCAN2 chips !!! */
	    if (hcan2_reset_chip(candev->chip[i])) return -ENODEV;
	}

	return 0;
}

int sh7760_init_hw_data(struct candevice_t *candev) 
{
	/* candev->res_addr = RESET_ADDR; */
	candev->nr_82527_chips = NR_82527;
	candev->nr_sja1000_chips = NR_SJA1000;
	candev->nr_all_chips = NR_ALL;
	/* candev->flags |= CANDEV_PROGRAMMABLE_IRQ; */

	return 0;
}
int sh7760_init_chip_data(struct candevice_t *candev, int chipnr)
{
	hcan2_fill_chipspecops(candev->chip[chipnr]);

	candev->chip[chipnr]->chip_base_addr = can_ioport2ioptr(candev->io_addr) + chipnr * SH7760_CAN_CHIP_OFFSET; /* one chip with 2 interfaces */
	candev->chip[chipnr]->clock = SH7760_CAN_CLOCK;
	candev->chip[chipnr]->chip_irq = SH7760_CAN_IRQ + chipnr;
	candev->chip[chipnr]->hostdevice = candev;

	return 0;
}

int sh7760_init_obj_data(struct canchip_t *chip, int objnr)
{
	chip->msgobj[objnr]->obj_base_addr = (can_ioptr_t) HCAN2_MB0 + HCAN2_MB_OFFSET * objnr;

	return 0;
}

int sh7760_program_irq(struct candevice_t *candev)
{
	/* sh7760 doesn't use programmable interrupt */ 
	return 0;
}


void sh7760_write_register(unsigned data, can_ioptr_t address)
{
	/* address is an absolute address */
	writew(data, address);
}

unsigned sh7760_read_register(can_ioptr_t address)
{
	/* address is an absolute address */
	return readw(address);
}

int sh7760_register(struct hwspecops_t *hwspecops)
{
	hwspecops->request_io = sh7760_request_io;
	hwspecops->release_io = sh7760_release_io;
	hwspecops->reset = sh7760_reset;
	hwspecops->init_hw_data = sh7760_init_hw_data;
	hwspecops->init_chip_data = sh7760_init_chip_data;
	hwspecops->init_obj_data = sh7760_init_obj_data;
	hwspecops->program_irq = sh7760_program_irq;
	hwspecops->write_register = sh7760_write_register;
	hwspecops->read_register = sh7760_read_register;
	return 0;
}
